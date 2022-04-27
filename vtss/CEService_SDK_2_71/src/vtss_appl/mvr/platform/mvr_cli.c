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

#include "mgmt_api.h"

#include "mvr.h"
#include "mvr_api.h"
#include "mvr_cli.h"


#define MVR_CLI_SORT_INTF_BY_NAME   VTSS_IPMC_DISABLE   /* VTSS_IPMC_ENABLE */

typedef struct {
    /* Keywords */
    BOOL                    naming;
    BOOL                    prival;
    BOOL                    taging;
    BOOL                    action;
    BOOL                    grpcnt;
    ipmc_operation_action_t op;
    mvr_intf_mode_t         mode;
    ipmc_intf_vtag_t        vtag;
    mvr_port_role_t         role;
    i8                      profile_name[VTSS_IPMC_NAME_MAX_LEN];
    i8                      vlan_name[MVR_NAME_MAX_LEN];
    i8                      grps_name[MVR_NAME_MAX_LEN];
    BOOL                    cnt_big;
    vtss_ipv6_t             grps_bound;
} mvr_cli_req_t;


void mvr_cli_req_init(void)
{
    /* register the size required for MVR cli_req structure */
    cli_req_size_register(sizeof(mvr_cli_req_t));
}

static void MVR_cli_cmd_set(cli_req_t *req,
                            BOOL mode,
                            BOOL vlan_basic,
                            BOOL vlan_mode,
                            BOOL vlan_llqi,
                            BOOL vlan_channel,
                            BOOL vlan_priority,
                            BOOL vlan_port,
                            BOOL fast_leave)
{
    switch_iter_t           sit;
    vtss_usid_t             usid;
    vtss_isid_t             isid;
    port_iter_t             pit;
    vtss_port_no_t          iport;
    vtss_uport_no_t         uport;
    vtss_rc                 mvr_cli_rc;
    mvr_cli_req_t           *mvr_req = req->module_req;

    i8                      buf[80], *ptr;
    BOOL                    state, flag;

    mvr_mgmt_interface_t    mvrif;
    mvr_conf_intf_entry_t   *p;
    mvr_conf_port_role_t    *q;
    mvr_conf_fast_leave_t   conf_fast_leave;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req) || !mvr_req) {
        return;
    }

    if (req->set) {
        if (mode) {
            state = req->enable;
            if (mvr_mgmt_set_mode(&state) != VTSS_OK) {
                CPRINTF("Failure\n");
                return;
            }
        }

        if (vlan_basic || vlan_mode || vlan_llqi || vlan_priority) {
            memset(&mvrif, 0x0, sizeof(mvr_mgmt_interface_t));
            mvrif.vid = req->vid;

            if (mvr_mgmt_get_intf_entry(VTSS_ISID_GLOBAL, &mvrif) == VTSS_OK) {
                if (vlan_basic && (mvr_req->op == IPMC_OP_ADD)) {
                    CPRINTF("MVR VLAN already exists!\n");
                    return;
                }
            } else {
                if (vlan_basic && (mvr_req->op != IPMC_OP_ADD)) {
                    CPRINTF("MVR VLAN doesn't exist!\n");
                    return;
                }
            }

            p = &mvrif.intf;
            if (mvr_req->op == IPMC_OP_ADD) {
                p->mode = MVR_CONF_DEF_INTF_MODE;
                p->vtag = MVR_CONF_DEF_INTF_VTAG;
                p->priority = MVR_CONF_DEF_INTF_PRIO;
                p->last_listener_query_interval = MVR_CONF_DEF_INTF_LLQI;
            }

            if (vlan_basic) {
                p->vid = mvrif.vid = req->vid;
                if (mvr_req->naming) {
                    memset(p->name, 0x0, sizeof(p->name));
                    strncpy(p->name, mvr_req->vlan_name, strlen(mvr_req->vlan_name));
                }
            }

            if (vlan_mode) {
                p->mode = mvr_req->mode;
            }

            if (vlan_llqi) {
                p->last_listener_query_interval = req->int_values[0];
            }

            if (vlan_priority) {
                if (mvr_req->taging) {
                    p->vtag = mvr_req->vtag;
                }

                if (mvr_req->prival) {
                    p->priority = req->value;
                }
            }

            mvr_cli_rc = mvr_mgmt_set_intf_entry(VTSS_ISID_GLOBAL, mvr_req->op, &mvrif);
            if (mvr_cli_rc != VTSS_OK) {
                if (mvr_cli_rc == IPMC_ERROR_TABLE_IS_FULL) {
                    CPRINTF("MVR VLAN Table Full\n");
                } else if (mvr_cli_rc == IPMC_ERROR_VLAN_ACTIVE) {
                    CPRINTF("Expected MVR VLAN is already in use\n");
                } else {
                    CPRINTF("Failure\n");
                }

                return;
            }
        }

        if (vlan_port) {
            (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
            while (switch_iter_getnext(&sit)) {
                usid = sit.usid;
                if (req->stack.isid[usid] == VTSS_ISID_END) {
                    continue;
                }
                isid = sit.isid;

                memset(&mvrif, 0x0, sizeof(mvr_mgmt_interface_t));
                mvrif.vid = req->vid;

                if (mvr_mgmt_get_intf_entry(isid, &mvrif) != VTSS_OK) {
                    continue;
                }

                q = &mvrif.role;

                (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                while (port_iter_getnext(&pit)) {
                    iport = pit.iport;

                    uport = iport2uport(iport);
                    if (req->uport_list[pit.uport] == 0 || port_isid_port_no_is_stack(isid, iport)) {
                        continue;
                    }

                    q->ports[iport] = mvr_req->role;
                }

                mvr_cli_rc = mvr_mgmt_set_intf_entry(isid, IPMC_OP_SET, &mvrif);
                if (mvr_cli_rc != VTSS_OK) {
                    if (mvr_cli_rc == IPMC_ERROR_TABLE_IS_FULL) {
                        CPRINTF("ISID-%d MVR VLAN Table Full\n", isid);
                    } else if (mvr_cli_rc == IPMC_ERROR_VLAN_ACTIVE) {
                        CPRINTF("ISID-%d Expected MVR VLAN is already in use\n", isid);
                    } else {
                        CPRINTF("ISID-%d Failure\n", isid);
                    }

                    return;
                }
            }
        }

        if (vlan_channel) {
            memset(&mvrif, 0x0, sizeof(mvr_mgmt_interface_t));
            mvrif.vid = req->vid;
            if (mvr_mgmt_get_intf_entry(VTSS_ISID_GLOBAL, &mvrif) == VTSS_OK) {
                int                         str_len = strlen(mvr_req->profile_name);
                ipmc_lib_profile_mem_t      *pfm;
                ipmc_lib_grp_fltr_profile_t *fltr_profile;
                u32                         profile_index;

                if (str_len && IPMC_MEM_PROFILE_MTAKE(pfm)) {
                    fltr_profile = &pfm->profile;
                    memset(fltr_profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
                    strncpy(fltr_profile->data.name, mvr_req->profile_name, str_len);
                    if (ipmc_lib_mgmt_fltr_profile_get(fltr_profile, TRUE) != VTSS_OK) {
                        CPRINTF("Please specify correct filter profile name.\n");
                        IPMC_MEM_PROFILE_MGIVE(pfm);
                        return;
                    }
                    profile_index = fltr_profile->data.index;
                    IPMC_MEM_PROFILE_MGIVE(pfm);

                    if (mvrif.intf.profile_index) {
                        if (mvr_req->op == IPMC_OP_ADD) {
                            CPRINTF("MVR VLAN channel profile already exists!\n");
                            return;
                        }
                    } else {
                        if (mvr_req->op != IPMC_OP_ADD) {
                            CPRINTF("MVR VLAN channel profile doesn't exist!\n");
                            return;
                        }
                    }

                    if (mvr_req->op == IPMC_OP_DEL) {
                        if (mvrif.intf.profile_index != profile_index) {
                            CPRINTF("Please specify correct filter profile name for deletion.\n");
                            return;
                        }

                        mvrif.intf.profile_index = MVR_CONF_DEF_INTF_PROFILE;
                    } else {
                        mvrif.intf.profile_index = profile_index;
                    }

                    mvr_cli_rc = mvr_mgmt_set_intf_entry(VTSS_ISID_GLOBAL, IPMC_OP_SET, &mvrif);
                    if (mvr_cli_rc != VTSS_OK) {
                        if (mvr_cli_rc == IPMC_ERROR_ENTRY_OVERLAPPED) {
                            CPRINTF("Expected profile has overlapped addresses used in other MVR VLAN\n");
                        } else {
                            if (mvr_cli_rc == IPMC_ERROR_ENTRY_NOT_FOUND) {
                                CPRINTF("Expected profile does not exist\n");
                            } else {
                                CPRINTF("Failure\n");
                            }
                        }

                        return;
                    }
                } else {
                    CPRINTF("Please specify correct filter profile name.\n");
                    return;
                }
            } else {
                CPRINTF("MVR VLAN doesn't exist!\n");
                return;
            }
        }
    } else {
        if (mode) {
            if (mvr_mgmt_get_mode(&state) == VTSS_OK) {
                CPRINTF("MVR Mode: %s\n\n", cli_bool_txt(state));
            }
        }

        (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
        while (switch_iter_getnext(&sit)) {
            usid = sit.usid;
            if (req->stack.isid[usid] == VTSS_ISID_END) {
                continue;
            }
            isid = sit.isid;

            if (vlan_basic || vlan_mode || vlan_llqi || vlan_priority || vlan_port || vlan_channel) {
                BOOL    need_hdr_print, mvid_match;

                cli_cmd_usid_print(usid, req, 0);
                mvr_cli_rc = mvr_mgmt_validate_intf_channel();
                if (mvr_cli_rc == VTSS_OK) {
                    CPRINTF("MVR Interface Setting\n");
                } else {
                    if (mvr_cli_rc == IPMC_ERROR_ENTRY_OVERLAPPED) {
                        CPRINTF("MVR Interface Setting (CAUTION: Overlapped channels are used!)\n");
                    } else {
                        if (mvr_cli_rc == IPMC_ERROR_ENTRY_NOT_FOUND) {
                            CPRINTF("MVR Interface Setting (CAUTION: Invalid channel profiles!)\n");
                        } else {
                            CPRINTF("MVR Interface Setting (CAUTION: Invalid channel settings!)\n");
                        }
                    }
                }
                ptr = &buf[0];
                ptr += sprintf(ptr, "VID   ");
                ptr += sprintf(ptr, "Name                              ");
                if (vlan_basic || vlan_mode) {
                    ptr += sprintf(ptr, "Mode        ");
                }
                if (vlan_basic || vlan_priority) {
                    ptr += sprintf(ptr, "Tagging   ");
                    ptr += sprintf(ptr, "Priority  ");
                }
                if (vlan_basic || vlan_llqi) {
                    ptr += sprintf(ptr, "LLQI   ");
                }

                cli_table_header(buf);
                need_hdr_print = FALSE;

                memset(&mvrif, 0x0, sizeof(mvr_mgmt_interface_t));
                if (req->vid_spec == CLI_SPEC_VAL) {
                    p = &mvrif.intf;
                    mvrif.vid = p->vid = req->vid - 1;

                    if (mvr_req->naming) {
                        strncpy(p->name, mvr_req->vlan_name, strlen(mvr_req->vlan_name));
                    }

                    mvid_match = FALSE;
                } else {
                    mvid_match = TRUE;
                }

#if MVR_CLI_SORT_INTF_BY_NAME
                while (mvr_mgmt_get_next_intf_entry_by_name(isid, &mvrif) == VTSS_OK) {
#else
                while (mvr_mgmt_get_next_intf_entry(isid, &mvrif) == VTSS_OK) {
#endif /* MVR_CLI_SORT_INTF_BY_NAME */
                    p = &mvrif.intf;
                    if ((req->vid_spec == CLI_SPEC_VAL) && (p->vid != req->vid)) {
                        continue;
                    }

                    mvid_match = TRUE;
                    if (need_hdr_print) {
                        CPRINTF("\n");
                        ptr = &buf[0];
                        ptr += sprintf(ptr, "VID   ");
                        ptr += sprintf(ptr, "Name                              ");
                        if (vlan_basic || vlan_mode) {
                            ptr += sprintf(ptr, "Mode        ");
                        }
                        if (vlan_basic || vlan_priority) {
                            ptr += sprintf(ptr, "Tagging   ");
                            ptr += sprintf(ptr, "Priority  ");
                        }
                        if (vlan_basic || vlan_llqi) {
                            ptr += sprintf(ptr, "LLQI   ");
                        }

                        cli_table_header(buf);
                        need_hdr_print = FALSE;
                    }

                    CPRINTF("%-4d  ", p->vid);
                    CPRINTF("%-32s  ", p->name);
                    if (vlan_basic || vlan_mode) {
                        if (p->mode == MVR_INTF_MODE_COMP) {
                            CPRINTF("%-10s  ", "Compatible");
                        } else {
                            CPRINTF("%-10s  ", "Dynamic");
                        }
                    }
                    if (vlan_basic || vlan_priority) {
                        if (p->vtag == IPMC_INTF_TAGED) {
                            CPRINTF("%-8s  ", "Tagged");
                        } else {
                            CPRINTF("%-8s  ", "Untagged");
                        }
                        CPRINTF("%-8d  ", p->priority);
                    }
                    if (vlan_basic || vlan_llqi) {
                        CPRINTF("%-5u  ", p->last_listener_query_interval);
                    }

                    if (vlan_basic || vlan_port) {
                        BOOL    psrc[VTSS_PORT_ARRAY_SIZE], fsrc;
                        BOOL    prcv[VTSS_PORT_ARRAY_SIZE], frcv;
                        BOOL    pina[VTSS_PORT_ARRAY_SIZE], fina;
                        char    pbuf[MGMT_PORT_BUF_SIZE];

                        CPRINTF("\n[Port Setting of %s(VID-%d)]",
                                p->name,
                                p->vid);
                        fsrc = frcv = fina = FALSE;
                        memset(psrc, 0x0, sizeof(psrc));
                        memset(prcv, 0x0, sizeof(prcv));
                        memset(pina, 0x0, sizeof(pina));
                        q = &mvrif.role;
                        (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_NORMAL);
                        while (port_iter_getnext(&pit)) {
                            iport = pit.iport;

                            uport = iport2uport(iport);
                            if (req->uport_list[uport] == 0) {
                                continue;
                            }

                            if (port_isid_port_no_is_stack(isid, iport)) {
                                psrc[iport] = prcv[iport] = pina[iport] = FALSE;
                                continue;
                            }

                            if (q->ports[iport] == MVR_PORT_ROLE_SOURC) {
                                fsrc = psrc[iport] = TRUE;
                                prcv[iport] = pina[iport] = FALSE;
                            } else if (q->ports[iport] == MVR_PORT_ROLE_RECVR) {
                                frcv = prcv[iport] = TRUE;
                                psrc[iport] = pina[iport] = FALSE;
                            } else {
                                fina = pina[iport] = TRUE;
                                psrc[iport] = prcv[iport] = FALSE;
                            }
                        }

                        if (!fsrc && !frcv && !fina) {
                            CPRINTF("\n<Empty Port Table>");
                        } else {
                            if (fsrc) {
                                CPRINTF("\nSource Port  : %s", cli_iport_list_txt(psrc, pbuf));
                            }
                            if (frcv) {
                                CPRINTF("\nReceiver Port: %s", cli_iport_list_txt(prcv, pbuf));
                            }
                            if (fina) {
                                CPRINTF("\nInactive Port: %s", cli_iport_list_txt(pina, pbuf));
                            }
                        }
                    }

                    if (vlan_basic || vlan_channel) {
                        u32                         pdx;
                        ipmc_lib_rule_t             rule;
                        ipmc_lib_profile_mem_t      *pfm;
                        ipmc_lib_grp_fltr_profile_t *fltr_profile;
                        i8                          profile_name[VTSS_IPMC_NAME_MAX_LEN];

                        pdx = p->profile_index;
                        memset(profile_name, 0x0, sizeof(profile_name));
                        if (IPMC_MEM_PROFILE_MTAKE(pfm)) {
                            fltr_profile = &pfm->profile;
                            fltr_profile->data.index = pdx;
                            if ((ipmc_lib_mgmt_fltr_profile_get(fltr_profile, FALSE) == VTSS_OK) &&
                                strlen(fltr_profile->data.name)) {
                                memcpy(profile_name, fltr_profile->data.name, sizeof(profile_name));
                            }
                            IPMC_MEM_PROFILE_MGIVE(pfm);
                        }
                        CPRINTF("\n[Channel Profile {%s} Used in %s(VID-%d)]",
                                profile_name,
                                p->name,
                                p->vid);

                        pdx = p->profile_index;
                        if (pdx && ipmc_lib_mgmt_fltr_profile_rule_get_first(pdx, &rule) == VTSS_OK) {
                            ipmc_lib_grp_fltr_entry_t   fltr_entry;
                            vtss_ipv4_t                 adrs4;
                            i8                          adrString[40];
                            BOOL                        heading = TRUE;

                            do {
                                fltr_entry.index = rule.entry_index;
                                if (ipmc_lib_mgmt_fltr_entry_get(&fltr_entry, FALSE) == VTSS_OK) {
                                    if (heading) {
                                        heading = FALSE;

                                        CPRINTF("\nHEAD-> %s\n", fltr_entry.name);
                                    } else {
                                        CPRINTF("NEXT-> %s\n", fltr_entry.name);
                                    }

                                    if (fltr_entry.version == IPMC_IP_VERSION_IGMP) {
                                        memset(adrString, 0x0, sizeof(adrString));
                                        IPMC_LIB_ADRS_6TO4_SET(fltr_entry.grp_bgn, adrs4);
                                        CPRINTF("Start : %s\n", misc_ipv4_txt(htonl(adrs4), adrString));
                                        memset(adrString, 0x0, sizeof(adrString));
                                        IPMC_LIB_ADRS_6TO4_SET(fltr_entry.grp_end, adrs4);
                                        CPRINTF("End   : %s\n", misc_ipv4_txt(htonl(adrs4), adrString));
                                    } else {
                                        memset(adrString, 0x0, sizeof(adrString));
                                        CPRINTF("Start : %s\n", misc_ipv6_txt(&fltr_entry.grp_bgn, adrString));
                                        memset(adrString, 0x0, sizeof(adrString));
                                        CPRINTF("End   : %s\n", misc_ipv6_txt(&fltr_entry.grp_end, adrString));
                                    }

                                    CPRINTF("Action: %s\n", ipmc_lib_fltr_action_txt(rule.action, IPMC_TXT_CASE_CAPITAL));
                                    CPRINTF("Log   : %s\n", cli_bool_txt(rule.log));
                                }
                            } while (ipmc_lib_mgmt_fltr_profile_rule_get_next(pdx, &rule) == VTSS_OK);
                        } else {
                            CPRINTF("\n<Empty Channel Table>\n");
                        }
                    }

                    need_hdr_print = TRUE;
                }

                if (!mvid_match) {
                    CPRINTF("\nInput MVR VID doesn't exist!\n");
                } else {
                    CPRINTF("\n");
                }
            }
        }
    }

    if (fast_leave) {
        (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
        while (switch_iter_getnext(&sit)) {
            usid = sit.usid;
            if (req->stack.isid[usid] == VTSS_ISID_END) {
                continue;
            }
            isid = sit.isid;

            if (mvr_mgmt_get_fast_leave_port(isid, &conf_fast_leave) != VTSS_OK) {
                continue;
            }

            flag = TRUE;
            (void) port_iter_init(&pit, NULL, isid, PORT_ITER_SORT_ORDER_UPORT, PORT_ITER_FLAGS_NORMAL);
            while (port_iter_getnext(&pit)) {
                iport = pit.iport;
                uport = pit.uport;

                if (req->uport_list[uport] == 0 || port_isid_port_no_is_stack(isid, iport)) {
                    continue;
                }

                if (req->set) {
                    if (fast_leave) {
                        VTSS_PORT_BF_SET(conf_fast_leave.ports, iport, req->enable);
                    }
                } else {
                    if (flag) {
                        flag = FALSE;
                        cli_cmd_usid_print(usid, req, 0);
                        CPRINTF("MVR Immediate Leave Setting\n");
                        ptr = &buf[0];
                        ptr += sprintf(ptr, "Port  ");
                        if (fast_leave) {
                            ptr += sprintf(ptr, "Immediate Leave");
                        }

                        cli_table_header(buf);
                    }

                    CPRINTF("%-2u    ", uport);
                    CPRINTF("%s\n", cli_bool_txt(VTSS_PORT_BF_GET(conf_fast_leave.ports, iport)));
                }
            }

            if (req->set) {
                if (mvr_mgmt_set_fast_leave_port(isid, &conf_fast_leave) != VTSS_OK) {
                    CPRINTF("\nFailure\n");
                }
            } else {
                CPRINTF("\n");
            }
        }
    }
}

/*
    cli_cmd_config_mvr_mode()
*/
static void cli_cmd_config_mvr_mode(cli_req_t *req)
{
    MVR_cli_cmd_set(req, 1, 0, 0, 0, 0, 0, 0, 0);
}

/*
    cli_cmd_config_mvr_vlan_basic()
*/
static void cli_cmd_config_mvr_vlan_basic(cli_req_t *req)
{
    MVR_cli_cmd_set(req, 0, 1, 0, 0, 0, 0, 0, 0);
}

/*
    cli_cmd_config_mvr_vlan_mode()
*/
static void cli_cmd_config_mvr_vlan_mode(cli_req_t *req)
{
    MVR_cli_cmd_set(req, 0, 0, 1, 0, 0, 0, 0, 0);
}

/*
    cli_cmd_config_mvr_vlan_llqi()
*/
static void cli_cmd_config_mvr_vlan_llqi(cli_req_t *req)
{
    MVR_cli_cmd_set(req, 0, 0, 0, 1, 0, 0, 0, 0);
}

/*
    cli_cmd_config_mvr_vlan_channel()
*/
static void cli_cmd_config_mvr_vlan_channel(cli_req_t *req)
{
    MVR_cli_cmd_set(req, 0, 0, 0, 0, 1, 0, 0, 0);
}

/*
    cli_cmd_config_mvr_vlan_priority()
*/
static void cli_cmd_config_mvr_vlan_priority(cli_req_t *req)
{
    MVR_cli_cmd_set(req, 0, 0, 0, 0, 0, 1, 0, 0);
}

/*
    cli_cmd_config_mvr_vlan_port()
*/
static void cli_cmd_config_mvr_vlan_port(cli_req_t *req)
{
    MVR_cli_cmd_set(req, 0, 0, 0, 0, 0, 0, 1, 0);
}

/*
    cli_cmd_config_mvr_fast_leave()
*/
static void cli_cmd_config_mvr_fast_leave(cli_req_t *req)
{
    MVR_cli_cmd_set(req, 0, 0, 0, 0, 0, 0, 0, 1);
}

/*
    cli_cmd_show_mvr_config()
*/
static void cli_cmd_show_mvr_config(cli_req_t *req)
{
    if (!req->set) {
        cli_header("MVR Configuration", 1);
    }

    MVR_cli_cmd_set(req, 1, 1, 1, 1, 1, 1, 1, 1);
}

/*
    cli_cmd_access_mvr_status()
*/
static void cli_cmd_access_mvr_status(cli_req_t *req)
{
    switch_iter_t                   sit;
    vtss_usid_t                     usid;
    vtss_isid_t                     isid;
    ipmc_ip_version_t               version;
    ipmc_prot_intf_entry_param_t    intf_param;
    BOOL                            mvr_mode, flag;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }

    if (mvr_mgmt_get_mode(&mvr_mode) != VTSS_OK) {
        mvr_mode = FALSE;
    }

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
    while (switch_iter_getnext(&sit)) {
        usid = sit.usid;
        if (req->stack.isid[usid] == VTSS_ISID_END) {
            continue;
        }
        isid = sit.isid;

        if (req->set) {
            if (mvr_mgmt_clear_stat_counter(isid, VTSS_IPMC_VID_ALL) != VTSS_OK) {
                CPRINTF("Clear Statistics Failed!\n");
            }

            continue;
        }

        flag = TRUE;
        version = IPMC_IP_VERSION_ALL;
        memset(&intf_param, 0x0, sizeof(ipmc_prot_intf_entry_param_t));
        while (mvr_mgmt_get_next_intf_info(isid, &version, &intf_param) == VTSS_OK) {
            if (req->vid_spec == CLI_SPEC_VAL && intf_param.vid != req->vid) {
                continue;
            }

            if (flag) {
                cli_cmd_usid_print(usid, req, 0);
                flag = FALSE;
            }

            if (version == IPMC_IP_VERSION_IGMP) {
                CPRINTF("IPv4  Querier  Rx         Tx         Rx         Rx         Rx         Rx\n");
                CPRINTF("VID   Status   Query      Query      V1 Join    V2 Join    V3 Join    V2 Leave\n");
                CPRINTF("----  -------  ---------- ---------- ---------- ---------- ---------- ----------\n");

                CPRINTF("%-4d  %-7s  %-10d %-10d %-10d %-10d %-10d %-10d\n",
                        intf_param.vid,
                        !mvr_mode ? "DISABLE" : (intf_param.querier.state == IPMC_QUERIER_IDLE ? "IDLE" : "ACTIVE"),
                        intf_param.stats.igmp_queries,
                        intf_param.querier.ipmc_queries_sent,
                        intf_param.stats.igmp_v1_membership_join,
                        intf_param.stats.igmp_v2_membership_join,
                        intf_param.stats.igmp_v3_membership_join,
                        intf_param.stats.igmp_v2_membership_leave);
            } else {
                CPRINTF("IPv6  Querier  Rx         Tx         Rx         Rx         Rx\n");
                CPRINTF("VID   Status   Query      Query      V1 Report  V2 Report  V1 Done\n");
                CPRINTF("----  -------  ---------- ---------- ---------- ---------- ----------\n");

                CPRINTF("%-4d  %-7s  %-10d %-10d %-10d %-10d %-10d\n",
                        intf_param.vid,
                        !mvr_mode ? "DISABLE" : (intf_param.querier.state == IPMC_QUERIER_IDLE ? "IDLE" : "ACTIVE"),
                        intf_param.stats.mld_queries,
                        intf_param.querier.ipmc_queries_sent,
                        intf_param.stats.mld_v1_membership_report,
                        intf_param.stats.mld_v2_membership_report,
                        intf_param.stats.mld_v1_membership_done);
            }

            if (req->vid_spec == CLI_SPEC_VAL) {
                break;
            }
            CPRINTF("\n");
        }
    }
}

/*
    cli_cmd_show_mvr_groups()
*/
static void cli_cmd_show_mvr_groups(cli_req_t *req )
{
    switch_iter_t                       sit;
    vtss_usid_t                         usid;
    vtss_isid_t                         isid;
    mvr_mgmt_interface_t                intf;
    u16                                 vid;
    ipmc_ip_version_t                   version;
    ipmc_prot_intf_group_entry_t        intf_group_entry;
    port_iter_t                         pit;
    BOOL                                iports[VTSS_PORT_ARRAY_SIZE], flag;
    u32                                 i, portCnt;
    char                                v6adrString[40] = {0};
    char                                plistString[MGMT_PORT_BUF_SIZE] = {0};

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
    while (switch_iter_getnext(&sit)) {
        usid = sit.usid;
        if (req->stack.isid[usid] == VTSS_ISID_END) {
            continue;
        }
        isid = sit.isid;

        flag = TRUE;
        memset(&intf, 0x0, sizeof(mvr_mgmt_interface_t));
        while (mvr_mgmt_get_next_intf_entry(isid, &intf) == VTSS_OK) {
            if (req->vid_spec == CLI_SPEC_VAL && intf.vid != req->vid) {
                continue;
            }

            version = IPMC_IP_VERSION_IGMP;
            vid = intf.vid;
            memset(&intf_group_entry, 0x0, sizeof(ipmc_prot_intf_group_entry_t));
            while (mvr_mgmt_get_next_intf_group(isid, &version, &vid, &intf_group_entry) == VTSS_OK ) {
                if (intf.vid != intf_group_entry.vid) {
                    break;
                }

                if (flag) {
                    flag = FALSE;
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

                if (version == IPMC_IP_VERSION_IGMP) {
                    u32 ip4addr;

                    memcpy((uchar *)&ip4addr, &intf_group_entry.group_addr.addr[12], sizeof(u32));
                    CPRINTF("%-4d  %-40s  %s\n",
                            intf_group_entry.vid,
                            misc_ipv4_txt(htonl(ip4addr), v6adrString),
                            cli_iport_list_txt(iports, plistString));
                } else {
                    CPRINTF("%-4d  %-40s  %s\n",
                            intf_group_entry.vid,
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

/*
    cli_cmd_show_mvr_sfm_info()
*/
static void cli_cmd_show_mvr_sfm_info(cli_req_t *req)
{
    switch_iter_t                       sit;
    vtss_usid_t                         usid;
    vtss_isid_t                         isid;
    port_iter_t                         pit;
    vtss_port_no_t                      iport;
    vtss_uport_no_t                     uport;
    mvr_mgmt_interface_t                intf;
    u16                                 vid;
    ipmc_ip_version_t                   version;
    ipmc_prot_intf_group_entry_t        intf_group_entry;
    ipmc_prot_group_srclist_t           group_srclist_entry;
    BOOL                                flag, deny_found, sfm_print, amir, smir;
    u32                                 ip4grp, ip4src;
    char                                buf[40] = {0};
    char                                buf1[16] = {0};
    char                                buf2[40] = {0};
    ipmc_sfm_srclist_t                  sfm_src;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
    while (switch_iter_getnext(&sit)) {
        usid = sit.usid;
        if (req->stack.isid[usid] == VTSS_ISID_END) {
            continue;
        }
        isid = sit.isid;

        flag = TRUE;
        memset(&intf, 0x0, sizeof(mvr_mgmt_interface_t));
        while (mvr_mgmt_get_next_intf_entry(isid, &intf) == VTSS_OK) {
            if (req->vid_spec == CLI_SPEC_VAL && intf.vid != req->vid) {
                continue;
            }

            version = IPMC_IP_VERSION_IGMP;
            vid = intf.vid;
            memset(&intf_group_entry, 0x0, sizeof(ipmc_prot_intf_group_entry_t));
            while (mvr_mgmt_get_next_intf_group(isid, &version, &vid, &intf_group_entry) == VTSS_OK ) {
                if (version != intf_group_entry.ipmc_version) {
                    continue;
                }
                if (intf.vid != intf_group_entry.vid) {
                    continue;
                }

                if (flag) {
                    flag = FALSE;
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
                        while (mvr_mgmt_get_next_group_srclist(
                                   isid, version, intf_group_entry.vid,
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
                            if (version == IPMC_IP_VERSION_IGMP) {
                                memcpy((uchar *)&ip4grp, &intf_group_entry.group_addr.addr[12], sizeof(u32));
                                memcpy((uchar *)&ip4src, &sfm_src.src_ip.addr[12], sizeof(u32));

                                if (!sfm_print) {
                                    sfm_print = TRUE;
                                    CPRINTF("%-4d  %-40s  %-7s  %-5u\n",
                                            intf_group_entry.vid,
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
                                            intf_group_entry.vid,
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
                        while (mvr_mgmt_get_next_group_srclist(
                                   isid, version, intf_group_entry.vid,
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
                            if (version == IPMC_IP_VERSION_IGMP) {
                                memcpy((uchar *)&ip4grp, &intf_group_entry.group_addr.addr[12], sizeof(u32));
                                memcpy((uchar *)&ip4src, &sfm_src.src_ip.addr[12], sizeof(u32));

                                if (!sfm_print) {
                                    sfm_print = TRUE;
                                    CPRINTF("%-4d  %-40s  %-7s  %-5u\n",
                                            intf_group_entry.vid,
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
                                            intf_group_entry.vid,
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
                                if (version == IPMC_IP_VERSION_IGMP) {
                                    memcpy((uchar *)&ip4grp, &intf_group_entry.group_addr.addr[12], sizeof(u32));

                                    if (!sfm_print) {
                                        CPRINTF("%-4d  %-40s  %-7s  %-5u\n",
                                                intf_group_entry.vid,
                                                misc_ipv4_txt(htonl(ip4grp), buf),
                                                buf1,
                                                iport + 1);
                                    }
                                } else {
                                    if (!sfm_print) {
                                        CPRINTF("%-4d  %-40s  %-7s  %-5u\n",
                                                intf_group_entry.vid,
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

/*
    cli_cmd_debug_mvr_intf_info()
*/
static void cli_cmd_debug_mvr_intf_info(cli_req_t *req)
{
    i8                      pbuf[MGMT_PORT_BUF_SIZE], buf[80], *ptr;
    mvr_local_interface_t   mvrif;
    mvr_conf_intf_entry_t   *p;
    port_iter_t             pit;
    u32                     iport, pdx;
    ipmc_lib_profile_mem_t  *pfm;
    ipmc_lib_rule_t         rule;
    BOOL                    pmvr[VTSS_PORT_ARRAY_SIZE], fmvr;
    BOOL                    psrc[VTSS_PORT_ARRAY_SIZE], fsrc;
    BOOL                    prcv[VTSS_PORT_ARRAY_SIZE], frcv;
    BOOL                    pina[VTSS_PORT_ARRAY_SIZE], fina;

    CPRINTF("Local MVR Interface Setting\n");

    memset(&mvrif, 0x0, sizeof(mvr_local_interface_t));
    while (mvr_mgmt_local_interface_get_next(&mvrif) == VTSS_OK) {
        ptr = &buf[0];
        ptr += sprintf(ptr, "VID   ");
        ptr += sprintf(ptr, "Name                              ");
        ptr += sprintf(ptr, "Mode        ");
        ptr += sprintf(ptr, "Tagging   ");
        ptr += sprintf(ptr, "Priority  ");
        ptr += sprintf(ptr, "LLQI   ");

        cli_table_header(buf);

        p = &mvrif.intf;

        CPRINTF("%-4d  ", p->vid);
        CPRINTF("%-32s  ", p->name);
        if (p->mode == MVR_INTF_MODE_COMP) {
            CPRINTF("%-10s  ", "Compatible");
        } else {
            CPRINTF("%-10s  ", "Dynamic");
        }
        if (p->vtag == IPMC_INTF_TAGED) {
            CPRINTF("%-8s  ", "Tagged");
        } else {
            CPRINTF("%-8s  ", "Untagged");
        }
        CPRINTF("%-8d  ", p->priority);
        CPRINTF("%-5u  ", p->last_listener_query_interval);

        CPRINTF("\n[%s Port Setting of %s(VID-%d)]",
                (mvrif.version == IPMC_IP_VERSION_IGMP) ? "IPv4" : "IPv6",
                p->name,
                p->vid);

        fmvr = fsrc = frcv = fina = FALSE;
        memset(pmvr, 0x0, sizeof(pmvr));
        memset(psrc, 0x0, sizeof(psrc));
        memset(prcv, 0x0, sizeof(prcv));
        memset(pina, 0x0, sizeof(pina));
        (void) port_iter_init(&pit, NULL, VTSS_ISID_LOCAL, PORT_ITER_SORT_ORDER_IPORT, PORT_ITER_FLAGS_ALL);
        while (port_iter_getnext(&pit)) {
            iport = pit.iport;

            if (VTSS_PORT_BF_GET(mvrif.vlan_ports, iport)) {
                fmvr = pmvr[iport] = TRUE;
            }

            if (mvrif.role_ports[iport] == MVR_PORT_ROLE_SOURC) {
                fsrc = psrc[iport] = TRUE;
                prcv[iport] = pina[iport] = FALSE;
            } else if (mvrif.role_ports[iport] == MVR_PORT_ROLE_RECVR) {
                frcv = prcv[iport] = TRUE;
                psrc[iport] = pina[iport] = FALSE;
            } else {
                fina = pina[iport] = TRUE;
                psrc[iport] = prcv[iport] = FALSE;
            }
        }

        if (!fmvr && !fsrc && !frcv && !fina) {
            CPRINTF("\n<Empty Port Table>");
        } else {
            if (fmvr) {
                CPRINTF("\nVLAN Port    : %s", cli_iport_list_txt(pmvr, pbuf));
            }

            if (fsrc) {
                CPRINTF("\nSource Port  : %s", cli_iport_list_txt(psrc, pbuf));
            }
            if (frcv) {
                CPRINTF("\nReceiver Port: %s", cli_iport_list_txt(prcv, pbuf));
            }
            if (fina) {
                CPRINTF("\nInactive Port: %s", cli_iport_list_txt(pina, pbuf));
            }
        }

        pdx = p->profile_index;
        if (pdx &&
            (ipmc_lib_mgmt_fltr_profile_rule_get_first(pdx, &rule) == VTSS_OK) &&
            IPMC_MEM_PROFILE_MTAKE(pfm)) {
            ipmc_lib_grp_fltr_profile_t *fltr_profile = &pfm->profile;
            ipmc_lib_grp_fltr_entry_t   fltr_entry;
            vtss_ipv4_t                 adrs4;
            i8                          adrString[40];
            BOOL                        heading = TRUE;

            fltr_profile->data.index = pdx;
            if (ipmc_lib_mgmt_fltr_profile_get(fltr_profile, FALSE) != VTSS_OK) {
                memset(fltr_profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
            }

            CPRINTF("\n[Channel Profile {%s} Used in %s(VID-%d)]",
                    fltr_profile->data.name,
                    p->name,
                    p->vid);

            do {
                fltr_entry.index = rule.entry_index;
                if (ipmc_lib_mgmt_fltr_entry_get(&fltr_entry, FALSE) == VTSS_OK) {
                    if (heading) {
                        heading = FALSE;

                        CPRINTF("\nHEAD-> %s\n", fltr_entry.name);
                    } else {
                        CPRINTF("NEXT-> %s\n", fltr_entry.name);
                    }

                    if (fltr_entry.version == IPMC_IP_VERSION_IGMP) {
                        memset(adrString, 0x0, sizeof(adrString));
                        IPMC_LIB_ADRS_6TO4_SET(fltr_entry.grp_bgn, adrs4);
                        CPRINTF("Start : %s\n", misc_ipv4_txt(htonl(adrs4), adrString));
                        memset(adrString, 0x0, sizeof(adrString));
                        IPMC_LIB_ADRS_6TO4_SET(fltr_entry.grp_end, adrs4);
                        CPRINTF("End   : %s\n", misc_ipv4_txt(htonl(adrs4), adrString));
                    } else {
                        memset(adrString, 0x0, sizeof(adrString));
                        CPRINTF("Start : %s\n", misc_ipv6_txt(&fltr_entry.grp_bgn, adrString));
                        memset(adrString, 0x0, sizeof(adrString));
                        CPRINTF("End   : %s\n", misc_ipv6_txt(&fltr_entry.grp_end, adrString));
                    }

                    CPRINTF("Action: %s\n", ipmc_lib_fltr_action_txt(rule.action, IPMC_TXT_CASE_CAPITAL));
                    CPRINTF("Log   : %s\n", cli_bool_txt(rule.log));
                }
            } while (ipmc_lib_mgmt_fltr_profile_rule_get_next(pdx, &rule) == VTSS_OK);

            IPMC_MEM_PROFILE_MGIVE(pfm);
        } else {
            CPRINTF("\n<Empty Channel Table>\n");
        }

        CPRINTF("\n");
    }

    CPRINTF("\n");
}

/*
    cli_cmd_debug_mvr_sfm_info()
*/
static void cli_cmd_debug_mvr_sfm_info(cli_req_t *req)
{
    switch_iter_t                       sit;
    vtss_usid_t                         usid;
    vtss_isid_t                         isid;
    port_iter_t                         pit;
    vtss_port_no_t                      iport;
    vtss_uport_no_t                     uport;
    mvr_mgmt_interface_t                intf;
    u16                                 vid;
    ipmc_ip_version_t                   version;
    ipmc_prot_intf_group_entry_t        intf_group_entry;
    ipmc_prot_group_srclist_t           group_srclist_entry;
    int                                 fto;
    BOOL                                flag, deny_found, sfm_print, amir, smir;
    u32                                 ip4grp, ip4src;
    char                                buf[40] = {0};
    char                                buf1[16] = {0};
    char                                buf2[40] = {0};
    ipmc_sfm_srclist_t                  sfm_src;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }

    (void) switch_iter_init(&sit, VTSS_ISID_GLOBAL, SWITCH_ITER_SORT_ORDER_USID);
    while (switch_iter_getnext(&sit)) {
        usid = sit.usid;
        if (req->stack.isid[usid] == VTSS_ISID_END) {
            continue;
        }
        isid = sit.isid;

        flag = TRUE;
        memset(&intf, 0x0, sizeof(mvr_mgmt_interface_t));
        while (mvr_mgmt_get_next_intf_entry(isid, &intf) == VTSS_OK) {
            if (req->vid_spec == CLI_SPEC_VAL && intf.vid != req->vid) {
                continue;
            }

            version = IPMC_IP_VERSION_IGMP;
            vid = intf.vid;
            memset(&intf_group_entry, 0x0, sizeof(ipmc_prot_intf_group_entry_t));
            while (mvr_mgmt_get_next_intf_group(isid, &version, &vid, &intf_group_entry) == VTSS_OK ) {
                if (version != intf_group_entry.ipmc_version) {
                    continue;
                }
                if (intf.vid != intf_group_entry.vid) {
                    continue;
                }

                if (flag) {
                    flag = FALSE;
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
                        while (mvr_mgmt_get_next_group_srclist(
                                   isid, version, intf_group_entry.vid,
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
                            if (version == IPMC_IP_VERSION_IGMP) {
                                memcpy((uchar *)&ip4grp, &intf_group_entry.group_addr.addr[12], sizeof(u32));
                                memcpy((uchar *)&ip4src, &sfm_src.src_ip.addr[12], sizeof(u32));

                                if (!sfm_print) {
                                    sfm_print = TRUE;
                                    CPRINTF("%-4d  %-40s  %-7s  %-9s  %-2s%-2s  %-5u\n",
                                            intf_group_entry.vid,
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
                                            intf_group_entry.vid,
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
                        while (mvr_mgmt_get_next_group_srclist(
                                   isid, version, intf_group_entry.vid,
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
                            if (version == IPMC_IP_VERSION_IGMP) {
                                memcpy((uchar *)&ip4grp, &intf_group_entry.group_addr.addr[12], sizeof(u32));
                                memcpy((uchar *)&ip4src, &sfm_src.src_ip.addr[12], sizeof(u32));

                                if (!sfm_print) {
                                    sfm_print = TRUE;
                                    CPRINTF("%-4d  %-40s  %-7s  %-9s  %-2s%-2s  %-5u\n",
                                            intf_group_entry.vid,
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
                                            intf_group_entry.vid,
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
                                if (version == IPMC_IP_VERSION_IGMP) {
                                    memcpy((uchar *)&ip4grp, &intf_group_entry.group_addr.addr[12], sizeof(u32));

                                    if (!sfm_print) {
                                        CPRINTF("%-4d  %-40s  %-7s  %-9s  %-2s%-2s  %-5u\n",
                                                intf_group_entry.vid,
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
                                                intf_group_entry.vid,
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
            }

            if (req->vid_spec == CLI_SPEC_VAL) {
                break;
            }
        }
    }
}

static int32_t mvr_cli_parse_priority(char *cmd, char *cmd2,
                                      char *stx,
                                      char *cmd_org,
                                      cli_req_t *req)
{
    mvr_cli_req_t   *mvr_req = req->module_req;

    req->parm_parsed = 1;

    if (cli_parse_ulong(cmd, &req->value, 0, 7)) {
        return 1;
    } else {
        mvr_req->prival = TRUE;
        return 0;
    }
}

static int32_t mvr_cli_parse_param_llqi(char *cmd, char *cmd2,
                                        char *stx,
                                        char *cmd_org,
                                        cli_req_t *req)
{
    int32_t error = 0;

    req->parm_parsed = 1;

    error = cli_parse_integer(cmd, req, stx);
    if (error) {
        return error;
    }

    if (req->int_values[0] < -1) {
        return 1;
    } else {
        if (req->int_values[0] == IPMC_PARAM_VALUE_INIT) {
            req->int_values[0] = MVR_CONF_DEF_INTF_LLQI;
        }
    }

    if ((req->int_values[0] < 0) || (req->int_values[0] > 31744)) {
        error = 1;
    }

    return error;
}

static int32_t mvr_cli_parse_vlan_vidname(char *cmd, char *cmd2,
                                          char *stx,
                                          char *cmd_org,
                                          cli_req_t *req)
{
    mvr_mgmt_interface_t    mvrif;
    mvr_cli_req_t           *mvr_req;
    BOOL                    has_alpha;
    int                     i, len;

    if (cli_parse_text(cmd_org, req->parm, MVR_NAME_MAX_LEN)) {
        return 1;
    }

    req->parm_parsed = 1;
    len = strlen(req->parm);
    if (len >= MVR_NAME_MAX_LEN) {
        return 1;
    }

    has_alpha = FALSE;
    for (i = 0; i < len; i++) {
        if (isalpha(req->parm[i])) {
            has_alpha = TRUE;
        } else {
            if (!isdigit(req->parm[i])) {
                if (req->parm[i] != 32) {
                    CPRINTF("Only space/blank, alphabets and numbers are allowed in MVR name.\n");
                    return 1;
                }
            }
        }
    }

    if (!has_alpha) {
        i = atoi(req->parm);
        if ((i < MVR_VID_MIN) || (i > MVR_VID_MAX)) {
            CPRINTF("At least one alphabet must be present.\n");

            return 1;
        } else {
            memset(&mvrif, 0x0, sizeof(mvr_mgmt_interface_t));
            mvrif.vid = mvrif.intf.vid = (vtss_vid_t)(i & 0xFFFF);

            if (mvr_mgmt_get_intf_entry(VTSS_ISID_GLOBAL, &mvrif) == VTSS_OK) {
                req->vid_spec = CLI_SPEC_VAL;
                req->vid = mvrif.vid;

                return 0;
            } else {
                CPRINTF("Input MVR VID doesn't exist!\n");

                return 1;
            }
        }
    }

    mvr_req = req->module_req;
    if (!mvr_req->action) {
        memset(&mvrif, 0x0, sizeof(mvr_mgmt_interface_t));
        strncpy(mvrif.intf.name, req->parm, len);

        if (mvr_mgmt_get_intf_entry_by_name(VTSS_ISID_GLOBAL, &mvrif) == VTSS_OK) {
            req->vid_spec = CLI_SPEC_VAL;
            req->vid = mvrif.vid;
        } else {
            CPRINTF("Input MVR Name doesn't exist!\n");

            return 1;
        }
    }

    strncpy(mvr_req->vlan_name, req->parm, len);
    mvr_req->naming = TRUE;

    return 0;
}

static int32_t mvr_cli_parse_vlan_name(char *cmd, char *cmd2,
                                       char *stx,
                                       char *cmd_org,
                                       cli_req_t *req)
{
    mvr_mgmt_interface_t    mvrif;
    mvr_cli_req_t           *mvr_req;
    BOOL                    has_alpha;
    int                     i, len;

    if (cli_parse_text(cmd_org, req->parm, MVR_NAME_MAX_LEN)) {
        return 1;
    }

    req->parm_parsed = 1;
    len = strlen(req->parm);
    if (len >= MVR_NAME_MAX_LEN) {
        return 1;
    }

    has_alpha = FALSE;
    for (i = 0; i < len; i++) {
        if (isalpha(req->parm[i])) {
            has_alpha = TRUE;
        } else {
            if (!isdigit(req->parm[i])) {
                if (req->parm[i] != 32) {
                    CPRINTF("Only space/blank, alphabets and numbers are allowed in MVR name.\n");
                    return 1;
                }
            }
        }
    }

    if (!has_alpha) {
        CPRINTF("At least one alphabet must be present.\n");
        return 1;
    }

    mvr_req = req->module_req;
    if (!mvr_req->action) {
        memset(&mvrif, 0x0, sizeof(mvr_mgmt_interface_t));
        strncpy(mvrif.intf.name, req->parm, len);

        if (mvr_mgmt_get_intf_entry_by_name(VTSS_ISID_GLOBAL, &mvrif) == VTSS_OK) {
            req->vid_spec = CLI_SPEC_VAL;
            req->vid = mvrif.vid;
        } else {
            return 1;
        }
    }

    strncpy(mvr_req->vlan_name, req->parm, len);
    mvr_req->naming = TRUE;

    return 0;
}

static int32_t mvr_cli_parse_vlan_vid(char *cmd, char *cmd2,
                                      char *stx,
                                      char *cmd_org,
                                      cli_req_t *req)
{
    u32 val;

    req->parm_parsed = 1;
    if (cli_parse_ulong(cmd, &val, MVR_VID_MIN, MVR_VID_MAX)) {
        return 1;
    } else {
        req->vid_spec = CLI_SPEC_VAL;
        req->vid = (vtss_vid_t)(val & 0xFFFF);

        return 0;
    }
}

static int32_t mvr_cli_parse_channel_profile_name(char *cmd, char *cmd2,
                                                  char *stx,
                                                  char *cmd_org,
                                                  cli_req_t *req)
{
    mvr_cli_req_t   *mvr_req;
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

    mvr_req = req->module_req;
    strncpy(mvr_req->profile_name, req->parm, len);

    return 0;
}

static int32_t mvr_cli_parse_keyword(char *cmd, char *cmd2,
                                     char *stx,
                                     char *cmd_org,
                                     cli_req_t *req)
{
    mvr_cli_req_t   *mvr_req = req->module_req;
    char            *found = cli_parse_find(cmd, stx);

    req->parm_parsed = 1;

    if (found && mvr_req) {
        if (!strncmp(found, "add", 3)) {
            mvr_req->op = IPMC_OP_ADD;
            mvr_req->action = TRUE;
        } else if (!strncmp(found, "del", 3)) {
            mvr_req->op = IPMC_OP_DEL;
            mvr_req->action = TRUE;
        } else if (!strncmp(found, "upd", 3)) {
            mvr_req->op = IPMC_OP_UPD;
            mvr_req->action = TRUE;
        } else if (!strncmp(found, "dynamic", 7)) {
            mvr_req->mode = MVR_INTF_MODE_DYNA;
        } else if (!strncmp(found, "compatible", 10)) {
            mvr_req->mode = MVR_INTF_MODE_COMP;
        } else if (!strncmp(found, "tagged", 6)) {
            mvr_req->vtag = IPMC_INTF_TAGED;
            mvr_req->taging = TRUE;
        } else if (!strncmp(found, "untagged", 8)) {
            mvr_req->vtag = IPMC_INTF_UNTAG;
            mvr_req->taging = TRUE;
        } else if (!strncmp(found, "source", 6)) {
            mvr_req->role = MVR_PORT_ROLE_SOURC;
        } else if (!strncmp(found, "receiver", 8)) {
            mvr_req->role = MVR_PORT_ROLE_RECVR;
        } else if (!strncmp(found, "inactive", 8)) {
            mvr_req->role = MVR_PORT_ROLE_INACT;
        } else {
            if (strncmp(found, "name", 4)) {
                found = NULL;
            }
        }
    }

    return (found == NULL ? 1 : 0);
}

/******************************************************************************/
// Parameters used in this module
// Note to myself, because I forget it all the time:
// CLI_PARM_FLAG_NO_TXT:
//   If included in flags, then help text will look like:
//      enable : bla-di-bla
//      disable: bla-di-bla
//      (default: bla-di-bla)
//
//   If excluded in flags, then help text will look like:
//     enable|disable: enable : bla-di-bla
//      disable: bla-di-bla
//      (default: bla-di-bla)
//
//   I.e., the parameter name is printed first if excluded. And it looks silly
//   in some cases, but is OK in other.
//   If it's a pipe-separated list of keywords (e.g. enable|disable), then
//   the flag should generally be included.
//   If it's a triangle-parenthesis-enclosed keyword (e.g. <age_time>), then
//   the flag should generally be excluded.
/******************************************************************************/
static cli_parm_t mvr_cli_parm_table[] = {
    {
        "add|del|upd",
        "add       : Add operation\n"
        "del       : Delete operation\n"
        "upd       : Update operation",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        mvr_cli_parse_keyword,
        NULL
    },
    {
        "name",
        "MVR Name keyword",
        CLI_PARM_FLAG_NONE,
        mvr_cli_parse_keyword,
        NULL
    },

    {
        "enable|disable",
        "enable    : Enable MVR Mode\n"
        "disable   : Disable MVR Mode\n"
        "(default: Show MVR mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_config_mvr_mode
    },

    {
        "<mvid>",
        "MVR VLAN ID (1-4095)",
        CLI_PARM_FLAG_NONE,
        mvr_cli_parse_vlan_vid,
        NULL
    },
    {
        "<mvr_name>",
        "MVR VLAN name (Maximum of 16 characters)",
        CLI_PARM_FLAG_NONE,
        mvr_cli_parse_vlan_name,
        NULL
    },
    {
        "<vid>|<mvr_name>",
        "MVR VLAN ID (1-4095) or Name (Maximum of 16 characters)",
        CLI_PARM_FLAG_NONE,
        mvr_cli_parse_vlan_vidname,
        NULL
    },

    {
        "dynamic|compatible",
        "dynamic   : Dynamic MVR mode\n"
        "compatible: Compatible MVR mode\n"
        "(default: Show MVR VLAN mode)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        mvr_cli_parse_keyword,
        cli_cmd_config_mvr_vlan_mode
    },

    {
        "mvr_param_llqi",
        "\n-1      : Default Value (5)\n"
        "0~31744   : Last Listener Query Interval in tenths of seconds\n"
        "(default: Show MVR Interface Last Listener Query Interval",
        CLI_PARM_FLAG_SET,
        mvr_cli_parse_param_llqi,
        cli_cmd_config_mvr_vlan_llqi
    },

    {
        "profile_name",
        "IPMC filtering profile name (Maximum of 16 characters)\n"
        "profile_name: Identifier of the designated profile",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        mvr_cli_parse_channel_profile_name,
        cli_cmd_config_mvr_vlan_channel
    },

    {
        "priority",
        "CoS priority value ranges from 0 ~ 7",
        CLI_PARM_FLAG_SET,
        mvr_cli_parse_priority,
        cli_cmd_config_mvr_vlan_priority
    },
    {
        "tagged|untagged",
        "tagged    : Tagged IGMP/MLD frames will be sent\n"
        "untagged  : Untagged IGMP/MLD frames will be sent",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        mvr_cli_parse_keyword,
        cli_cmd_config_mvr_vlan_priority
    },

    {
        "source|receiver|inactive",
        "source    : MVR source port\n"
        "receiver  : MVR receiver port\n"
        "inactive  : Disable MVR\n"
        "(default: Show MVR port role)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        mvr_cli_parse_keyword,
        cli_cmd_config_mvr_vlan_port
    },

    {
        "enable|disable",
        "enable    : Enable Immediate Leave\n"
        "disable   : Disable Immediate Leave\n"
        "(default: Show MVR Immediate Leave)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_config_mvr_fast_leave
    },

    {
        NULL,
        NULL,
        0,
        0,
        NULL
    },
};

/******************************************************************************
    The order that the commands shall appear.
*******************************************************************************/
enum {
    PRIO_MVR_CONF,

    PRIO_MVR_MODE,

    PRIO_MVR_VLAN_CONF,
    PRIO_MVR_VLAN_MODE,
    PRIO_MVR_VLAN_PORT,
    PRIO_MVR_VLAN_LLQI,
    PRIO_MVR_VLAN_GRPS,
    PRIO_MVR_VLAN_VTAG,

    PRIO_MVR_FAST_LEAVE,

    PRIO_MVR_STATUS,
    PRIO_MVR_GROUPS,
    PRIO_MVR_SFM_INFO,

    PRIO_MVR_DEBUG = CLI_CMD_SORT_KEY_DEFAULT
};


/******************************************************************************
    Commands defined in MVR
*******************************************************************************/
cli_cmd_tab_entry (
    "MVR Configuration",
    NULL,
    "Show MVR configuration",
    PRIO_MVR_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_MVR,
    cli_cmd_show_mvr_config,
    NULL,
    mvr_cli_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    "MVR Mode",
    "MVR Mode [enable|disable]",
    "Set or show system MVR mode",
    PRIO_MVR_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MVR,
    cli_cmd_config_mvr_mode,
    NULL,
    mvr_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "MVR VLAN Setup [<mvid>]",
    "MVR VLAN Setup [<mvid>] [add|del|upd] [(Name <mvr_name>)]",
    "Set or show per MVR VLAN configuration",
    PRIO_MVR_VLAN_CONF,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MVR,
    cli_cmd_config_mvr_vlan_basic,
    NULL,
    mvr_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "MVR VLAN Mode [<vid>|<mvr_name>]",
    "MVR VLAN Mode [<vid>|<mvr_name>] [dynamic|compatible]",
    "Set or show per MVR VLAN mode",
    PRIO_MVR_VLAN_MODE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MVR,
    cli_cmd_config_mvr_vlan_mode,
    NULL,
    mvr_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "MVR VLAN LLQI [<vid>|<mvr_name>]",
    "MVR VLAN LLQI [<vid>|<mvr_name>] [mvr_param_llqi]",
    "Set or show per MVR VLAN LLQI (Last Listener Query Interval)",
    PRIO_MVR_VLAN_LLQI,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MVR,
    cli_cmd_config_mvr_vlan_llqi,
    NULL,
    mvr_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "MVR VLAN Channel [<vid>|<mvr_name>]",
    "MVR VLAN Channel [<vid>|<mvr_name>] [add|del|upd] [profile_name]",
    "Set or show per MVR VLAN channel",
    PRIO_MVR_VLAN_GRPS,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MVR,
    cli_cmd_config_mvr_vlan_channel,
    NULL,
    mvr_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "MVR VLAN Priority [<vid>|<mvr_name>]",
    "MVR VLAN Priority [<vid>|<mvr_name>] [priority] [tagged|untagged]",
    "Set or show per MVR VLAN priority and VLAN tag",
    PRIO_MVR_VLAN_VTAG,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MVR,
    cli_cmd_config_mvr_vlan_priority,
    NULL,
    mvr_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "MVR VLAN Port [<vid>|<mvr_name>]",
    "MVR VLAN Port [<vid>|<mvr_name>] [<port_list>] [source|receiver|inactive]",
    "Set or show per MVR VLAN port role",
    PRIO_MVR_VLAN_PORT,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MVR,
    cli_cmd_config_mvr_vlan_port,
    NULL,
    mvr_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "MVR Immediate Leave [<port_list>]",
    "MVR Immediate Leave [<port_list>] [enable|disable]",
    "Set or show MVR immediate leave per port",
    PRIO_MVR_FAST_LEAVE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_MVR,
    cli_cmd_config_mvr_fast_leave,
    NULL,
    mvr_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "MVR Status [<vid>]",
    "MVR Status [<vid>] [clear]",
    "Show/Clear MVR operational status",
    PRIO_MVR_STATUS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_MVR,
    cli_cmd_access_mvr_status,
    NULL,
    mvr_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "MVR Groups [<vid>]",
    NULL,
    "Show MVR group addresses",
    PRIO_MVR_GROUPS,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_MVR,
    cli_cmd_show_mvr_groups,
    NULL,
    mvr_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "MVR SFM [<vid>] [<port_list>]",
    NULL,
    "Show SFM (including SSM) related information for MVR",
    PRIO_MVR_SFM_INFO,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_MVR,
    cli_cmd_show_mvr_sfm_info,
    NULL,
    mvr_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug MVR VLAN",
    NULL,
    "Debug local MVR Interface information",
    PRIO_MVR_DEBUG,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_mvr_intf_info,
    NULL,
    mvr_cli_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    "Debug MVR SFM [<vid>] [<port_list>]",
    NULL,
    "Debug SFM related information in MVR",
    PRIO_MVR_DEBUG,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_mvr_sfm_info,
    NULL,
    mvr_cli_parm_table,
    CLI_CMD_FLAG_NONE
);
