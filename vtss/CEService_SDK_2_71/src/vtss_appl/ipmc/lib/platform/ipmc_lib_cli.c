/*

 Vitesse Switch API software.

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
#include "cli.h"
#include "cli_api.h"

#include "syslog_api.h"
#include "mgmt_api.h"

#include "ipmc_lib.h"
#include "ipmc_lib_cli.h"


#define VTSS_TRACE_MODULE_ID    VTSS_MODULE_ID_IPMC_LIB

typedef struct {
    /* Keywords */
    i8                          profile_name[VTSS_IPMC_NAME_MAX_LEN];
    i8                          entry_name[VTSS_IPMC_NAME_MAX_LEN];
    i8                          next_rule_name[VTSS_IPMC_NAME_MAX_LEN];
    i8                          desc[VTSS_IPMC_DESC_MAX_LEN];
    ipmc_operation_action_t     op;
    vtss_ipv6_t                 adr_bgn;
    vtss_ipv6_t                 adr_end;
    ipmc_ip_version_t           adr_ver;
    ipmc_action_t               fltr_action;
    BOOL                        fltr_log;

    BOOL                        addressing;
    BOOL                        boundary;
    BOOL                        action;
    BOOL                        logging;
    BOOL                        additional;
} ipmc_lib_cli_req_t;


void ipmc_lib_cli_req_init(void)
{
    /* register the size required for ipmc_lib req. structure */
    cli_req_size_register(sizeof(ipmc_lib_cli_req_t));
}

#if ((defined(VTSS_SW_OPTION_SMB_IPMC) || defined(VTSS_SW_OPTION_MVR)) && defined(VTSS_SW_OPTION_IPMC_LIB))
static void cli_cmd_ipmclib_conf(cli_req_t *req,
                                 BOOL show,
                                 BOOL mode,
                                 BOOL pf_entry,
                                 BOOL pf_rule,
                                 BOOL pf_obj)
{
    i8                          buf[80], *p;
    i8                          adrString[40];
    int                         str_len;
    vtss_ipv4_t                 adrs4;
    BOOL                        state, first;
    ipmc_lib_cli_req_t          *ipmclib_req = req->module_req;
    ipmc_lib_profile_mem_t      *pfm;
    ipmc_lib_grp_fltr_profile_t *fltr_profile;
    ipmc_lib_profile_t          *data;
    ipmc_lib_rule_t             fltr_rule;
    ipmc_lib_grp_fltr_entry_t   fltr_entry;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req) || !ipmclib_req) {
        return;
    }

    if (!IPMC_MEM_PROFILE_MTAKE(pfm)) {
        return;
    }

    fltr_profile = &pfm->profile;
    memset(buf, 0x0, sizeof(buf));

    if (mode) {
        if (req->set && !show) {
            state = req->enable;
            (void) ipmc_lib_mgmt_profile_state_set(state);
        } else if (ipmc_lib_mgmt_profile_state_get(&state) == VTSS_OK) {
            CPRINTF("Global Profile Control State: %s\n", cli_bool_txt(state));
            CPRINTF("\n");
        }
    }

    if (pf_obj) {
        str_len = strlen(ipmclib_req->profile_name);
        memset(fltr_profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));

        if (req->set && !show) {
            data = &fltr_profile->data;
            if (str_len) {
                strncpy(data->name, ipmclib_req->profile_name, str_len);
            } else {
                CPRINTF("Please specify correct filter profile name.\n");
                IPMC_MEM_PROFILE_MGIVE(pfm);
                return;
            }

            str_len = strlen(ipmclib_req->desc);
            if (ipmc_lib_mgmt_fltr_profile_get(fltr_profile, TRUE) == VTSS_OK) {
                if (ipmclib_req->additional) {
                    memset(data->desc, 0x0, sizeof(data->desc));
                    if (str_len) {
                        strncpy(data->desc, ipmclib_req->desc, str_len);
                    }
                }
            } else {
                if (ipmclib_req->additional && str_len) {
                    strncpy(data->desc, ipmclib_req->desc, str_len);
                }
            }

            if (ipmc_lib_mgmt_fltr_profile_set(ipmclib_req->op, fltr_profile) != VTSS_OK) {
                CPRINTF("Operation Failed!\n");
            }
        } else {
            first = TRUE;
            while (ipmc_lib_mgmt_fltr_profile_get_next(fltr_profile, TRUE) == VTSS_OK) {
                data = &fltr_profile->data;
                if (str_len) {
                    if (str_len != strlen(data->name)) {
                        continue;
                    }

                    if (strncmp(data->name, ipmclib_req->profile_name, str_len)) {
                        continue;
                    }
                }

                if (first) {
                    first = FALSE;
                    p = &buf[0];
                    p += sprintf(p, "IPMC Profile Table Setting");
                    cli_table_header(buf);
                }

                CPRINTF("Profile: %s (In %s Mode)\n",
                        data->name,
                        ipmc_lib_version_txt(data->version, IPMC_TXT_CASE_UPPER));
                CPRINTF("Description: %s\n", data->desc);

                if (ipmc_lib_mgmt_fltr_profile_rule_get_first(data->index, &fltr_rule) == VTSS_OK) {
                    BOOL    heading = TRUE;

                    do {
                        fltr_entry.index = fltr_rule.entry_index;
                        if (ipmc_lib_mgmt_fltr_entry_get(&fltr_entry, FALSE) == VTSS_OK) {
                            if (heading) {
                                heading = FALSE;

                                CPRINTF("HEAD-> %s\n", fltr_entry.name);
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

                            CPRINTF("Action: %s\n", ipmc_lib_fltr_action_txt(fltr_rule.action, IPMC_TXT_CASE_CAPITAL));
                            CPRINTF("Log   : %s\n", cli_bool_txt(fltr_rule.log));
                        }
                    } while (ipmc_lib_mgmt_fltr_profile_rule_get_next(data->index, &fltr_rule) == VTSS_OK);
                }

                CPRINTF("\n");
            }

            if (!first) {
                CPRINTF("\n");
            }
        }
    }

    if (pf_rule) {
        if (req->set && !show) {
            ipmc_lib_grp_fltr_entry_t   next_entry;
            u32                         pdx = 0, edx = 0, ndx = IPMC_LIB_FLTR_RULE_IDX_INIT;
            BOOL                        rule_sanity = TRUE;

            str_len = strlen(ipmclib_req->profile_name);
            if (!str_len) {
                CPRINTF("Please specify correct filter profile name.\n");
                rule_sanity = FALSE;
            } else {
                memset(fltr_profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
                data = &fltr_profile->data;
                strncpy(data->name, ipmclib_req->profile_name, str_len);
                if (ipmc_lib_mgmt_fltr_profile_get(fltr_profile, TRUE) != VTSS_OK) {
                    CPRINTF("Please specify correct filter profile name.\n");
                    rule_sanity = FALSE;
                } else {
                    pdx = data->index;
                }
            }
            str_len = strlen(ipmclib_req->entry_name);
            if (!str_len) {
                CPRINTF("Please specify correct filter entry name.\n");
                rule_sanity = FALSE;
            } else {
                memset(&fltr_entry, 0x0, sizeof(ipmc_lib_grp_fltr_entry_t));
                strncpy(fltr_entry.name, ipmclib_req->entry_name, str_len);
                if (ipmc_lib_mgmt_fltr_entry_get(&fltr_entry, TRUE) != VTSS_OK) {
                    CPRINTF("Please specify correct filter entry name.\n");
                    rule_sanity = FALSE;
                } else {
                    edx = fltr_entry.index;
                }
            }
            if (ipmclib_req->additional && strlen(ipmclib_req->next_rule_name)) {
                memset(&next_entry, 0x0, sizeof(ipmc_lib_grp_fltr_entry_t));
                strncpy(next_entry.name, ipmclib_req->next_rule_name, strlen(ipmclib_req->next_rule_name));
                if (ipmc_lib_mgmt_fltr_entry_get(&next_entry, TRUE) != VTSS_OK) {
                    CPRINTF("Please specify correct next filter entry name.\n");
                    rule_sanity = FALSE;
                } else {
                    if (pdx && (ipmc_lib_mgmt_fltr_profile_rule_get_first(pdx, &fltr_rule) == VTSS_OK)) {
                        BOOL    next_valid = FALSE;

                        do {
                            if (fltr_rule.entry_index == next_entry.index) {
                                next_valid = TRUE;
                                ndx = fltr_rule.idx;
                                break;
                            }
                        } while (ipmc_lib_mgmt_fltr_profile_rule_get_next(pdx, &fltr_rule) == VTSS_OK);

                        if (!next_valid) {
                            CPRINTF("Please add the expected next filter entry first.\n");
                            rule_sanity = FALSE;
                        }
                    }
                }
            }
            if (!rule_sanity) {
                IPMC_MEM_PROFILE_MGIVE(pfm);
                return;
            }

            if (ipmc_lib_mgmt_fltr_profile_rule_search(pdx, edx, &fltr_rule) != VTSS_OK) {
                memset(&fltr_rule, 0x0, sizeof(ipmc_lib_rule_t));
                fltr_rule.idx = IPMC_LIB_FLTR_RULE_IDX_INIT;
                fltr_rule.entry_index = edx;
                fltr_rule.next_rule_idx = IPMC_LIB_FLTR_RULE_IDX_INIT;
            }

            if (ipmclib_req->additional && strlen(ipmclib_req->next_rule_name) && IPMC_LIB_PF_RULE_ID_VALID(ndx)) {
                fltr_rule.next_rule_idx = ndx;
            }

            if (ipmclib_req->action) {
                fltr_rule.action = ipmclib_req->fltr_action;
            }
            if (ipmclib_req->logging) {
                fltr_rule.log = ipmclib_req->fltr_log;
            }

            if (ipmc_lib_mgmt_fltr_profile_rule_set(ipmclib_req->op, pdx, &fltr_rule) != VTSS_OK) {
                CPRINTF("Operation Failed!\n");
            }
        }
    }

    if (pf_entry) {
        str_len = strlen(ipmclib_req->entry_name);
        memset(&fltr_entry, 0x0, sizeof(ipmc_lib_grp_fltr_entry_t));

        if (req->set && !show) {
            if (str_len) {
                strncpy(fltr_entry.name, ipmclib_req->entry_name, str_len);
            } else {
                CPRINTF("Please specify correct filter entry name.\n");
                IPMC_MEM_PROFILE_MGIVE(pfm);
                return;
            }

            if (ipmc_lib_mgmt_fltr_entry_get(&fltr_entry, TRUE) == VTSS_OK) {
                if (ipmclib_req->addressing) {
                    if (IPMC_LIB_ADRS_CMP6(fltr_entry.grp_bgn, ipmclib_req->adr_bgn)) {
                        IPMC_LIB_ADRS_CPY(&fltr_entry.grp_bgn, &ipmclib_req->adr_bgn);
                    }
                    if (ipmclib_req->boundary) {
                        IPMC_LIB_ADRS_CPY(&fltr_entry.grp_end, &ipmclib_req->adr_end);
                    } else {
                        IPMC_LIB_ADRS_CPY(&fltr_entry.grp_end, &fltr_entry.grp_bgn);
                    }
                    if (fltr_entry.version != ipmclib_req->adr_ver) {
                        fltr_entry.version = ipmclib_req->adr_ver;
                    }
                }
            } else {
                fltr_entry.index = 0;
                fltr_entry.version = IPMC_IP_VERSION_INIT;
                if (ipmclib_req->addressing) {
                    IPMC_LIB_ADRS_CPY(&fltr_entry.grp_bgn, &ipmclib_req->adr_bgn);
                    if (ipmclib_req->boundary) {
                        IPMC_LIB_ADRS_CPY(&fltr_entry.grp_end, &ipmclib_req->adr_end);
                    } else {
                        IPMC_LIB_ADRS_CPY(&fltr_entry.grp_end, &fltr_entry.grp_bgn);
                    }
                    fltr_entry.version = ipmclib_req->adr_ver;
                }
            }

            if (ipmc_lib_mgmt_fltr_entry_set(ipmclib_req->op, &fltr_entry) != VTSS_OK) {
                CPRINTF("Operation Failed!\n");
            }
        } else {
            first = TRUE;
            while (ipmc_lib_mgmt_fltr_entry_get_next(&fltr_entry, TRUE) == VTSS_OK) {
                if (str_len) {
                    if (str_len != strlen(fltr_entry.name)) {
                        continue;
                    }

                    if (strncmp(fltr_entry.name, ipmclib_req->entry_name, str_len)) {
                        continue;
                    }
                }

                if (first) {
                    first = FALSE;
                    p = &buf[0];
                    p += sprintf(p, "IPMC Profile Entry Setting");
                    cli_table_header(buf);
                }

                CPRINTF("Entry Name   : %s\n", fltr_entry.name);

                if (fltr_entry.version == IPMC_IP_VERSION_IGMP) {
                    memset(adrString, 0x0, sizeof(adrString));
                    IPMC_LIB_ADRS_6TO4_SET(fltr_entry.grp_bgn, adrs4);
                    CPRINTF("Start Address: %s\n", misc_ipv4_txt(htonl(adrs4), adrString));
                    memset(adrString, 0x0, sizeof(adrString));
                    IPMC_LIB_ADRS_6TO4_SET(fltr_entry.grp_end, adrs4);
                    CPRINTF("End Address  : %s\n", misc_ipv4_txt(htonl(adrs4), adrString));
                } else {
                    memset(adrString, 0x0, sizeof(adrString));
                    CPRINTF("Start Address: %s\n", misc_ipv6_txt(&fltr_entry.grp_bgn, adrString));
                    memset(adrString, 0x0, sizeof(adrString));
                    CPRINTF("End Address  : %s\n", misc_ipv6_txt(&fltr_entry.grp_end, adrString));
                }

                CPRINTF("\n");
            }

            if (!first) {
                CPRINTF("\n");
            }
        }
    }

    IPMC_MEM_PROFILE_MGIVE(pfm);
}

static void cli_cmd_ipmclib_profile_config(cli_req_t *req)
{
    ipmc_lib_cli_req_t  *ipmclib_req = req->module_req;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req) || !ipmclib_req) {
        return;
    }

    cli_cmd_ipmclib_conf(req, 1, 1, 1, 1, 1);
}

static void cli_cmd_ipmclib_profile_entry_config(cli_req_t *req)
{
    ipmc_lib_cli_req_t  *ipmclib_req = req->module_req;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req) || !ipmclib_req) {
        return;
    }

    cli_cmd_ipmclib_conf(req, 1, 0, 1, 0, 0);
}

static void cli_cmd_ipmclib_profile_table_config(cli_req_t *req)
{
    ipmc_lib_cli_req_t  *ipmclib_req = req->module_req;

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req) || !ipmclib_req) {
        return;
    }

    cli_cmd_ipmclib_conf(req, 1, 0, 0, 1, 1);
}

static void cli_cmd_ipmclib_profile_state(cli_req_t *req)
{
    cli_cmd_ipmclib_conf(req, 0, 1, 0, 0, 0);
}

static void cli_cmd_ipmclib_entry_op(cli_req_t *req)
{
    cli_cmd_ipmclib_conf(req, 0, 0, 1, 0, 0);
}

static void cli_cmd_ipmclib_entry_op_add(cli_req_t *req)
{
    ipmc_lib_cli_req_t  *ipmclib_req = req->module_req;

    ipmclib_req->op = IPMC_OP_ADD;
    cli_cmd_ipmclib_entry_op(req);
}

static void cli_cmd_ipmclib_entry_op_del(cli_req_t *req)
{
    ipmc_lib_cli_req_t  *ipmclib_req = req->module_req;

    ipmclib_req->op = IPMC_OP_DEL;
    cli_cmd_ipmclib_entry_op(req);
}

static void cli_cmd_ipmclib_entry_op_upd(cli_req_t *req)
{
    ipmc_lib_cli_req_t  *ipmclib_req = req->module_req;

    ipmclib_req->op = IPMC_OP_UPD;
    cli_cmd_ipmclib_entry_op(req);
}

static void cli_cmd_ipmclib_rule_op(cli_req_t *req)
{
    cli_cmd_ipmclib_conf(req, 0, 0, 0, 1, 0);
}

static void cli_cmd_ipmclib_rule_op_add(cli_req_t *req)
{
    ipmc_lib_cli_req_t  *ipmclib_req = req->module_req;

    ipmclib_req->op = IPMC_OP_ADD;
    cli_cmd_ipmclib_rule_op(req);
}

static void cli_cmd_ipmclib_rule_op_del(cli_req_t *req)
{
    ipmc_lib_cli_req_t  *ipmclib_req = req->module_req;

    ipmclib_req->op = IPMC_OP_DEL;
    cli_cmd_ipmclib_rule_op(req);
}

static void cli_cmd_ipmclib_rule_op_upd(cli_req_t *req)
{
    ipmc_lib_cli_req_t  *ipmclib_req = req->module_req;

    ipmclib_req->op = IPMC_OP_UPD;
    cli_cmd_ipmclib_rule_op(req);
}

static void cli_cmd_ipmclib_profile_op(cli_req_t *req)
{
    cli_cmd_ipmclib_conf(req, 0, 0, 0, 0, 1);
}

static void cli_cmd_ipmclib_profile_op_add(cli_req_t *req)
{
    ipmc_lib_cli_req_t  *ipmclib_req = req->module_req;

    ipmclib_req->op = IPMC_OP_ADD;
    cli_cmd_ipmclib_profile_op(req);
}

static void cli_cmd_ipmclib_profile_op_del(cli_req_t *req)
{
    ipmc_lib_cli_req_t  *ipmclib_req = req->module_req;

    ipmclib_req->op = IPMC_OP_DEL;
    cli_cmd_ipmclib_profile_op(req);
}

static void cli_cmd_ipmclib_profile_op_upd(cli_req_t *req)
{
    ipmc_lib_cli_req_t  *ipmclib_req = req->module_req;

    ipmclib_req->op = IPMC_OP_UPD;
    cli_cmd_ipmclib_profile_op(req);
}
#endif /* (VTSS_SW_OPTION_SMB_IPMC || VTSS_SW_OPTION_MVR) && VTSS_SW_OPTION_IPMC_LIB */

static char *_ipmclib_cli_rule_id_txt(u32 idx, char *txt)
{
    switch ( idx ) {
    case IPMC_LIB_FLTR_RULE_IDX_INIT:
        sprintf(txt, "%s", "GND");
        break;
    case IPMC_LIB_FLTR_RULE_IDX_DFLT:
        sprintf(txt, "%s", "DEF");
        break;
    default:
        sprintf(txt, "%u", idx);
        break;
    }

    return txt;
}

static void cli_cmd_debug_ipmc_profile(cli_req_t *req)
{
    i8                          buf[80], *bp;
    u32                         pdx;
    int                         str_len;
    BOOL                        first, is_avl;
    vtss_vid_t                  pf_vid;
    ipmc_profile_rule_t         ptr;
    ipmc_lib_rule_t             *rule;
    ipmc_lib_grp_fltr_entry_t   entry, *p;
    ipmc_lib_profile_t          *data;
    ipmc_lib_profile_mem_t      *pfm;
    ipmc_lib_grp_fltr_profile_t *q;
    ipmc_lib_cli_req_t          *ipmclib_req = req->module_req;
    i8                          adrString[40], id_txt[11];

    if (cli_cmd_switch_none(req) || cli_cmd_conf_slave(req)) {
        return;
    }
    if (!IPMC_MEM_PROFILE_MTAKE(pfm)) {
        return;
    }

    memset(buf, 0x0, sizeof(buf));
    bp = &buf[0];
    bp += sprintf(bp, "Internal IPMC Profile Tree");
    cli_table_header(buf);

    str_len = strlen(ipmclib_req->profile_name);
    for (pdx = 1; pdx <= IPMC_LIB_FLTR_PROFILE_MAX_CNT; pdx++) {
        pfm->profile.data.index = pdx;
        if (ipmc_lib_mgmt_fltr_profile_get(&pfm->profile, FALSE) != VTSS_OK) {
            continue;
        }
        q = &pfm->profile;
        data = &q->data;

        if (str_len) {
            if (str_len != strlen(data->name)) {
                continue;
            }

            if (strncmp(data->name, ipmclib_req->profile_name, str_len)) {
                continue;
            }
        }

        first = TRUE;
        memset(&ptr, 0x0, sizeof(ipmc_profile_rule_t));
        is_avl = FALSE;
        while (ipmc_lib_mgmt_profile_tree_get_next(pdx, &ptr, &is_avl)) {
            rule = ptr.rule;
            if (first) {
                if (!ipmc_lib_mgmt_profile_tree_vid_get(pdx, &pf_vid)) {
                    pf_vid = VTSS_IPMC_VID_VOID;
                }
                memset(id_txt, 0x0, sizeof(id_txt));
                CPRINTF("\n\r%s %s ProfileTree:Index-%u(VID-%u)/HEAD-%s",
                        ipmc_lib_version_txt(data->version, IPMC_TXT_CASE_UPPER),
                        data->name,
                        data->index,
                        pf_vid,
                        _ipmclib_cli_rule_id_txt(data->rule_head_idx, id_txt));
                if (strlen(data->desc)) {
                    CPRINTF("\nDescription: %s", data->desc);
                } else {
                    CPRINTF("\nWithout Further Description");
                }

                CPRINTF("\n\rO(%s)\n\r|\n\r1ST->", is_avl ? "TREE" : "LIST");
                first = FALSE;
            } else {
                CPRINTF("\n\r|\n\rNXT->");
            }

            memset(id_txt, 0x0, sizeof(id_txt));
            memset(adrString, 0x0, sizeof(adrString));
            CPRINTF("IDX-%s/%s/%s/NXT-",
                    rule ? _ipmclib_cli_rule_id_txt(rule->idx, id_txt) : "INVALID",
                    ipmc_lib_version_txt(ptr.version, IPMC_TXT_CASE_UPPER),
                    misc_ipv6_txt(&ptr.grp_adr, adrString));
            memset(id_txt, 0x0, sizeof(id_txt));
            CPRINTF("%s", rule ? _ipmclib_cli_rule_id_txt(rule->next_rule_idx, id_txt) : "INVALID");

            if (!rule) {
                CPRINTF("\nInvalid Rule Entry!\n");
                continue;
            }

            entry.index = rule->entry_index;
            if (ipmc_lib_mgmt_fltr_entry_get(&entry, FALSE) == VTSS_OK) {
                p = &entry;
                CPRINTF("\n\rEntry %s(Index:%u) is in %s range.",
                        p->name,
                        p->index,
                        ipmc_lib_version_txt(p->version, IPMC_TXT_CASE_UPPER));

                memset(adrString, 0x0, sizeof(adrString));
                CPRINTF("\n\rStart   : %s", misc_ipv6_txt(&p->grp_bgn, adrString));
                memset(adrString, 0x0, sizeof(adrString));
                CPRINTF("\n\rBoundary: %s", misc_ipv6_txt(&p->grp_end, adrString));
            } else {
                if (IPMC_LIB_PF_RULE_ID_DFLT(rule->entry_index)) {
                    CPRINTF("\nDEFAULT Profile Entry.");
                } else {
                    CPRINTF("\nInvalid Profile Entry Association!");
                }

                memset(id_txt, 0x0, sizeof(id_txt));
                CPRINTF(" [Index is %s]", _ipmclib_cli_rule_id_txt(rule->entry_index, id_txt));
            }

            CPRINTF("\n%s matched (%s logging) filtering condition.",
                    ipmc_lib_fltr_action_txt(rule->action, IPMC_TXT_CASE_CAPITAL),
                    rule->log ? "with" : "without");
        }

        CPRINTF("\n\r");
    }

    IPMC_MEM_PROFILE_MGIVE(pfm);
}

static int32_t ipmclib_cli_parse_profile_name(char *cmd, char *cmd2,
                                              char *stx,
                                              char *cmd_org,
                                              cli_req_t *req)
{
    ipmc_lib_cli_req_t  *ipmclib_req;
    BOOL                has_alpha;
    int                 i, len;

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

    ipmclib_req = req->module_req;
    memset(ipmclib_req->profile_name, 0x0, sizeof(ipmclib_req->profile_name));
    if (len) {
        strncpy(ipmclib_req->profile_name, req->parm, len);
    }

    return 0;
}

static int32_t ipmclib_cli_parse_profile_desc(char *cmd, char *cmd2,
                                              char *stx,
                                              char *cmd_org,
                                              cli_req_t *req)
{
    ipmc_lib_cli_req_t  *ipmclib_req;
    BOOL                has_alpha;
    int                 i, len;

    if (cli_parse_text(cmd_org, req->parm, VTSS_IPMC_DESC_MAX_LEN)) {
        return 1;
    }

    req->parm_parsed = 1;
    ipmclib_req = req->module_req;

    len = strlen(req->parm);
    if (!len) {
        memset(ipmclib_req->desc, 0x0, sizeof(ipmclib_req->desc));
        ipmclib_req->additional = TRUE;

        return 0;
    }

    if (len >= VTSS_IPMC_DESC_MAX_LEN) {
        return 1;
    }

    has_alpha = FALSE;
    for (i = 0; i < len; i++) {
        if (isalpha(req->parm[i])) {
            has_alpha = TRUE;
        } else {
            if ((req->parm[i] < 45) || (req->parm[i] > 122)) {  /* ASCII[45:- 122:z] */
                if (req->parm[i] != 32) {
                    CPRINTF("Input character is not allowed in profile description.\n");
                    return 1;
                }
            } else {
                /* Accept only digits and - & _ */
                if (!isdigit(req->parm[i]) &&
                    (req->parm[i] != 45) &&
                    (req->parm[i] != 95)) {
                    CPRINTF("Input character is not allowed in profile description.\n");
                    return 1;
                }
            }
        }
    }

    if (!has_alpha) {
        CPRINTF("At least one alphabet must be present.\n");
        return 1;
    }

    ipmclib_req = req->module_req;
    memset(ipmclib_req->desc, 0x0, sizeof(ipmclib_req->desc));
    if (len) {
        strncpy(ipmclib_req->desc, req->parm, len);
    }
    ipmclib_req->additional = TRUE;

    return 0;
}

static int32_t ipmclib_cli_parse_entry_name(char *cmd, char *cmd2,
                                            char *stx,
                                            char *cmd_org,
                                            cli_req_t *req)
{
    ipmc_lib_cli_req_t  *ipmclib_req;
    BOOL                has_alpha;
    int                 i, len;

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
                    CPRINTF("Only space/blank, alphabets and numbers are allowed in profile entry name.\n");
                    return 1;
                }
            }
        }
    }

    if (!has_alpha) {
        CPRINTF("At least one alphabet must be present.\n");
        return 1;
    } else {
        BOOL    no_key_word = TRUE;

        if (!strncmp(cmd_org, "permit", 6) ||
            !strncmp(cmd_org, "deny", 4) ||
            !strncmp(cmd_org, "enable", 6) ||
            !strncmp(cmd_org, "disable", 7)) {
            no_key_word = FALSE;
        }

        if (!no_key_word) {
            CPRINTF("Invalid entry name that is the same as configuring keyword.\n");
            return 1;
        }
    }

    ipmclib_req = req->module_req;
    memset(ipmclib_req->entry_name, 0x0, sizeof(ipmclib_req->entry_name));
    if (len) {
        strncpy(ipmclib_req->entry_name, req->parm, len);
    }

    return 0;
}

static int32_t ipmclib_cli_parse_entry_next(char *cmd, char *cmd2,
                                            char *stx,
                                            char *cmd_org,
                                            cli_req_t *req)
{
    ipmc_lib_cli_req_t  *ipmclib_req;
    BOOL                has_alpha;
    int                 i, len;

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
                    CPRINTF("Only space/blank, alphabets and numbers are allowed in profile entry name.\n");
                    return 1;
                }
            }
        }
    }

    if (!has_alpha) {
        CPRINTF("At least one alphabet must be present.\n");
        return 1;
    } else {
        BOOL    no_key_word = TRUE;

        if (!strncmp(cmd_org, "permit", 6) ||
            !strncmp(cmd_org, "deny", 4) ||
            !strncmp(cmd_org, "enable", 6) ||
            !strncmp(cmd_org, "disable", 7)) {
            no_key_word = FALSE;
        }

        if (!no_key_word) {
            CPRINTF("Invalid entry name that is same as keyword.\n");
            return 1;
        }
    }

    ipmclib_req = req->module_req;
    memset(ipmclib_req->next_rule_name, 0x0, sizeof(ipmclib_req->next_rule_name));
    if (len) {
        strncpy(ipmclib_req->next_rule_name, req->parm, len);
    }
    ipmclib_req->additional = TRUE;

    return 0;
}

static int32_t ipmclib_cli_parse_adrs_bgn(char *cmd, char *cmd2,
                                          char *stx,
                                          char *cmd_org,
                                          cli_req_t *req)
{
    int32_t             error;
    cli_spec_t          spec_type;
    u32                 grp4;
    vtss_ipv6_t         grp6;
    ipmc_lib_cli_req_t  *ipmclib_req;

    ipmclib_req = req->module_req;
    ipmclib_req->adr_ver = IPMC_IP_VERSION_INIT;
    ipmclib_req->addressing = FALSE;

    error = cli_parse_ipmcv4_addr(cmd, &grp4, &spec_type);
    if (error) {
        error = cli_parse_ipmcv6_addr(cmd, &grp6, &spec_type);
        if (!error) {
            if (spec_type == CLI_SPEC_VAL) {
                IPMC_LIB_ADRS_CPY(&ipmclib_req->adr_bgn, &grp6);
                ipmclib_req->adr_ver = IPMC_IP_VERSION_MLD;
                ipmclib_req->addressing = TRUE;
            } else {
                error = 1;
            }
        }
    } else {
        if (spec_type == CLI_SPEC_VAL) {
            grp4 = htonl(grp4);
            IPMC_LIB_ADRS_4TO6_SET(grp4, ipmclib_req->adr_bgn);
            ipmclib_req->adr_ver = IPMC_IP_VERSION_IGMP;
            ipmclib_req->addressing = TRUE;
        } else {
            error = 1;
        }
    }

    if (error) {
        CPRINTF("Please input valid IPv4 or IPv6 multicast address.\n");
        return 1;
    } else {
        ipmclib_req->additional = TRUE;
        return 0;
    }
}

static int32_t ipmclib_cli_parse_adrs_end(char *cmd, char *cmd2,
                                          char *stx,
                                          char *cmd_org,
                                          cli_req_t *req)
{
    int32_t             error;
    cli_spec_t          spec_type;
    u32                 grp4;
    vtss_ipv6_t         grp6;
    ipmc_lib_cli_req_t  *ipmclib_req;

    ipmclib_req = req->module_req;

    if (!ipmclib_req->additional) {
        return 1;
    }

    error = cli_parse_ipmcv4_addr(cmd, &grp4, &spec_type);
    if (error) {
        error = cli_parse_ipmcv6_addr(cmd, &grp6, &spec_type);
        if (!error) {
            if ((spec_type == CLI_SPEC_VAL) &&
                (ipmclib_req->adr_ver == IPMC_IP_VERSION_MLD)) {
                IPMC_LIB_ADRS_CPY(&ipmclib_req->adr_end, &grp6);
            } else {
                CPRINTF("Start address and end address must be in the same IP version.\n");
                CPRINTF("Please input valid IPv4 multicast address.\n");
                error = 1;
            }
        } else {
            CPRINTF("Please input valid IPv4 or IPv6 multicast address.\n");
        }
    } else {
        if ((spec_type == CLI_SPEC_VAL) &&
            (ipmclib_req->adr_ver == IPMC_IP_VERSION_IGMP)) {
            grp4 = htonl(grp4);
            IPMC_LIB_ADRS_4TO6_SET(grp4, ipmclib_req->adr_end);
        } else {
            CPRINTF("Start address and end address must be in the same IP version.\n");
            CPRINTF("Please input valid IPv6 multicast address.\n");
            error = 1;
        }
    }

    if (!error) {
        if (IPMC_LIB_ADRS_GREATER(&ipmclib_req->adr_bgn, &ipmclib_req->adr_end)) {
            CPRINTF("End address is not allowed to be less than start address.\n");
            CPRINTF("Please input valid %s ending multicast address again.\n",
                    (ipmclib_req->adr_ver == IPMC_IP_VERSION_IGMP) ? "IPv4" : "IPv6");
            error = 1;
        } else {
            ipmclib_req->boundary = TRUE;
        }
    }

    return error;
}

static int32_t ipmclib_cli_parse_keyword(char *cmd, char *cmd2,
                                         char *stx,
                                         char *cmd_org,
                                         cli_req_t *req)
{
    ipmc_lib_cli_req_t  *ipmclib_req = req->module_req;
    i8                  *found = cli_parse_find(cmd, stx);

    req->parm_parsed = 1;

    if (found && ipmclib_req) {
        if (!strncmp(found, "permit", 6)) {
            ipmclib_req->fltr_action = IPMC_ACTION_PERMIT;
            ipmclib_req->action = TRUE;
        } else if (!strncmp(found, "deny", 4)) {
            ipmclib_req->fltr_action = IPMC_ACTION_DENY;
            ipmclib_req->action = TRUE;
        } else if (!strncmp(found, "enable", 6)) {
            ipmclib_req->fltr_log = TRUE;
            ipmclib_req->logging = TRUE;
        } else if (!strncmp(found, "disable", 7)) {
            ipmclib_req->fltr_log = FALSE;
            ipmclib_req->logging = TRUE;
        } else {
            found = NULL;
        }
    }

    return (found == NULL ? 1 : 0);
}

cli_parm_t ipmc_lib_parm_table[] = {
#if ((defined(VTSS_SW_OPTION_SMB_IPMC) || defined(VTSS_SW_OPTION_MVR)) && defined(VTSS_SW_OPTION_IPMC_LIB))
    {
        "enable|disable",
        "enable : Enable IPMC profile filtering \n"
        "disable: Disable IPMC profile filtering \n"
        "(default: Show global IPMC profile filtering state)",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        cli_parm_parse_keyword,
        cli_cmd_ipmclib_profile_state
    },
#endif /* (VTSS_SW_OPTION_SMB_IPMC || VTSS_SW_OPTION_MVR) && VTSS_SW_OPTION_IPMC_LIB */

    {
        "<profile_name>",
        "IPMC filtering profile name (Maximum of 16 characters)\n"
        "profile_name: Identifier of the designated profile",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        ipmclib_cli_parse_profile_name,
        NULL
    },

    {
        "profile_desc",
        "Description for IPMC filtering profile (Maximum of 64 characters)\n"
        "profile_desc: Additional description about the profile\n"
        "              Use \"\" to clear description\n",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        ipmclib_cli_parse_profile_desc,
        NULL
    },

    {
        "<entry_name>",
        "IPMC filtering entry name (Maximum of 16 characters)\n"
        "entry_name: Identifier of the designated entry",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        ipmclib_cli_parse_entry_name,
        NULL
    },

    {
        "next_entry_name",
        "Name of the next IPMC filtering entry in a profile (Maximum of 16 characters)\n"
        "next_entry_name: Specify next entry used in profile; Default: Add entry last",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        ipmclib_cli_parse_entry_next,
        NULL
    },

    {
        "start_addr",
        "IPv4/IPv6 multicast group address\n"
        "start_addr: Valid IPv4 or IPv6 multicast address",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        ipmclib_cli_parse_adrs_bgn,
        NULL
    },

    {
        "end_addr",
        "The boundary IPv4/IPv6 multicast group address for filtering entry.\n"
        "end_addr: Valid IPv4/IPv6 multicast address that is not less than start address",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        ipmclib_cli_parse_adrs_end,
        NULL
    },

    {
        "permit|deny",
        "Set filtering action for the rule in profile; Default is deny.\n"
        "permit: Permit the group address to be registered\n"
        "deny  : Deny the group address to be registered",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        ipmclib_cli_parse_keyword,
        NULL
    },

    {
        "enable|disable",
        "Log capability when the rule is matched; Default is disable.\n"
        "enable : Enable logging the matched rule\n"
        "disable: Bypass logging the matched rule",
        CLI_PARM_FLAG_NO_TXT | CLI_PARM_FLAG_SET,
        ipmclib_cli_parse_keyword,
        NULL
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
    PRIO_IPMC_LIB_DEBUG = CLI_CMD_SORT_KEY_DEFAULT
};

#if ((defined(VTSS_SW_OPTION_SMB_IPMC) || defined(VTSS_SW_OPTION_MVR)) && defined(VTSS_SW_OPTION_IPMC_LIB))
/* Command table entries */
cli_cmd_tab_entry (
    "IPMC Profile Configuration",
    NULL,
    "Show entire IPMC profile configuration",
    PRIO_IPMC_LIB_PROFILE_CONF,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_IPMC_LIB,
    cli_cmd_ipmclib_profile_config,
    NULL,
    ipmc_lib_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    "IPMC Profile Entry Configuration [<entry_name>]",
    NULL,
    "Show IPMC profile entry configuration",
    PRIO_IPMC_LIB_FLTR_ENTRY_TBL,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_IPMC_LIB,
    cli_cmd_ipmclib_profile_entry_config,
    NULL,
    ipmc_lib_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    "IPMC Profile Table Configuration [<profile_name>]",
    NULL,
    "Show IPMC profile table configuration",
    PRIO_IPMC_LIB_FLTR_PROFILE_TBL,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_IPMC_LIB,
    cli_cmd_ipmclib_profile_table_config,
    NULL,
    ipmc_lib_parm_table,
    CLI_CMD_FLAG_SYS_CONF
);

cli_cmd_tab_entry (
    "IPMC Profile State",
    "IPMC Profile State [enable|disable]",
    "Set or show global IPMC profile state",
    PRIO_IPMC_LIB_PROFILE_STATE,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC_LIB,
    cli_cmd_ipmclib_profile_state,
    NULL,
    ipmc_lib_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "IPMC Profile Entry Add <entry_name> [start_addr] [end_addr]",
    "Add the specific IPMC profile entry",
    PRIO_IPMC_LIB_FLTR_ENTRY_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC_LIB,
    cli_cmd_ipmclib_entry_op_add,
    NULL,
    ipmc_lib_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    NULL,
    "IPMC Profile Entry Delete <entry_name>",
    "Delete the specific IPMC profile entry",
    PRIO_IPMC_LIB_FLTR_ENTRY_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC_LIB,
    cli_cmd_ipmclib_entry_op_del,
    NULL,
    ipmc_lib_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    NULL,
    "IPMC Profile Entry Update <entry_name> [start_addr] [end_addr]",
    "Update the specific IPMC profile entry",
    PRIO_IPMC_LIB_FLTR_ENTRY_UPD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC_LIB,
    cli_cmd_ipmclib_entry_op_upd,
    NULL,
    ipmc_lib_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "IPMC Profile Add <profile_name> [profile_desc]",
    "Add the specific IPMC profile",
    PRIO_IPMC_LIB_FLTR_PROFILE_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC_LIB,
    cli_cmd_ipmclib_profile_op_add,
    NULL,
    ipmc_lib_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    NULL,
    "IPMC Profile Delete <profile_name>",
    "Delete the specific IPMC profile",
    PRIO_IPMC_LIB_FLTR_PROFILE_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC_LIB,
    cli_cmd_ipmclib_profile_op_del,
    NULL,
    ipmc_lib_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    NULL,
    "IPMC Profile Update <profile_name> [profile_desc]",
    "Update the specific IPMC profile",
    PRIO_IPMC_LIB_FLTR_PROFILE_UPD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC_LIB,
    cli_cmd_ipmclib_profile_op_upd,
    NULL,
    ipmc_lib_parm_table,
    CLI_CMD_FLAG_NONE
);

cli_cmd_tab_entry (
    NULL,
    "IPMC Profile Rule Add <profile_name> <entry_name> [permit|deny] [enable|disable] [next_entry_name]",
    "Add the filtering rule in an IPMC profile",
    PRIO_IPMC_LIB_FLTR_RULE_ADD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC_LIB,
    cli_cmd_ipmclib_rule_op_add,
    NULL,
    ipmc_lib_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    NULL,
    "IPMC Profile Rule Delete <profile_name> <entry_name>",
    "Delete the filtering rule in an IPMC profile",
    PRIO_IPMC_LIB_FLTR_RULE_DEL,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC_LIB,
    cli_cmd_ipmclib_rule_op_del,
    NULL,
    ipmc_lib_parm_table,
    CLI_CMD_FLAG_NONE
);
cli_cmd_tab_entry (
    NULL,
    "IPMC Profile Rule Update <profile_name> <entry_name> [permit|deny] [enable|disable] [next_entry_name]",
    "Update the filtering rule in an IPMC profile",
    PRIO_IPMC_LIB_FLTR_RULE_UPD,
    CLI_CMD_TYPE_CONF,
    VTSS_MODULE_ID_IPMC_LIB,
    cli_cmd_ipmclib_rule_op_upd,
    NULL,
    ipmc_lib_parm_table,
    CLI_CMD_FLAG_NONE
);
#endif /* (VTSS_SW_OPTION_SMB_IPMC || VTSS_SW_OPTION_MVR) && VTSS_SW_OPTION_IPMC_LIB */

cli_cmd_tab_entry (
    "Debug IPMC Profile [<profile_name>]",
    NULL,
    "Debug IPMC Profile Internal Database",
    PRIO_IPMC_LIB_DEBUG,
    CLI_CMD_TYPE_STATUS,
    VTSS_MODULE_ID_DEBUG,
    cli_cmd_debug_ipmc_profile,
    NULL,
    ipmc_lib_parm_table,
    CLI_CMD_FLAG_NONE
);
