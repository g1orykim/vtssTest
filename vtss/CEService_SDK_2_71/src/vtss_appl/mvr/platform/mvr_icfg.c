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

#include "icfg_api.h"
#include "misc_api.h"
#include "topo_api.h"
#include "mvr.h"
#include "mvr_api.h"
#include "mvr_icfg.h"


/*
******************************************************************************

    Constant and Macro definition

******************************************************************************
*/
#define VTSS_TRACE_MODULE_ID        VTSS_MODULE_ID_MVR

#define IPMC_MVR_ICFG_REG(x, y, z, w)  (((x) = vtss_icfg_query_register((y), (z), (w))) == VTSS_OK)

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
/* ICFG callback functions */
static vtss_rc _ipmc_mvr_icfg_global(const vtss_icfg_query_request_t *req,
                                     vtss_icfg_query_result_t *result)
{
    vtss_rc rc = VTSS_OK;

    if (req && result) {
        BOOL    state;

        /*
            COMMAND = mvr
        */
        rc = mvr_mgmt_get_mode(&state);
        if (rc != VTSS_OK) {
            state = FALSE;
        }

        if (req->all_defaults || (state != MVR_CONF_DEF_GLOBAL_MODE)) {
            rc |= vtss_icfg_printf(result, "%smvr\n", state ? "" : "no ");
        }
    }

    return rc;
}

/* ICFG callback functions */
static vtss_rc _ipmc_mvr_icfg_intf(const vtss_icfg_query_request_t *req,
                                   vtss_icfg_query_result_t *result)
{
    vtss_rc rc = VTSS_OK;

    if (req && result) {
        mvr_mgmt_interface_t        mvr_icfg_intf;
        int                         mvif_strlen;
        i8                          adrString[40];
        mvr_conf_intf_entry_t       *mvif;
        ipmc_lib_profile_mem_t      *pfm;
        ipmc_lib_grp_fltr_profile_t *fltr_profile;

        /*
            COMMAND = mvr vlan <vlan_list> [name <mvr_name:word16>]

            COMMAND = mvr name <mvr_name:word16> mode {dynamic | compatible}
            COMMAND = mvr vlan <vlan_list> mode {dynamic | compatible}
            COMMAND = mvr name <mvr_name:word16> igmp-address <ipv4_ucast>
            COMMAND = mvr vlan <vlan_list> igmp-address <ipv4_ucast>
            COMMAND = mvr name <mvr_name:word16> frame priority <cos_priority:0-7>
            COMMAND = mvr name <mvr_name:word16> frame tagged
            COMMAND = mvr vlan <vlan_list> frame priority <cos_priority:0-7>
            COMMAND = mvr vlan <vlan_list> frame tagged
            COMMAND = mvr name <mvr_name:word16> last-member-query-interval <ipmc_lmqi:0-31744>
            COMMAND = mvr vlan <vlan_list> last-member-query-interval <ipmc_lmqi:0-31744>

            COMMAND = mvr name <mvr_name:word16> channel <profile_name:word16>
            COMMAND = mvr vlan <vlan_list> channel <profile_name:word16>
        */
        memset(&mvr_icfg_intf, 0x0, sizeof(mvr_mgmt_interface_t));
        while (mvr_mgmt_get_next_intf_entry(VTSS_ISID_GLOBAL, &mvr_icfg_intf) == VTSS_OK) {
            mvif = &mvr_icfg_intf.intf;
            mvif_strlen = strlen(mvif->name);
            if (mvif_strlen) {
                rc |= vtss_icfg_printf(result, "mvr vlan %u name %s\n",
                                       mvif->vid,
                                       mvif->name);
                if (req->all_defaults || (mvif->querier4_address != MVR_CONF_DEF_INTF_ADRS4)) {
                    memset(adrString, 0x0, sizeof(adrString));
                    rc |= vtss_icfg_printf(result, "%smvr name %s igmp-%s%s\n",
                                           mvif->querier4_address ? "" : "no ",
                                           mvif->name,
                                           mvif->querier4_address ? "address " : "address",
                                           mvif->querier4_address ? misc_ipv4_txt(mvif->querier4_address, adrString) : "");
                }
                if (req->all_defaults || (mvif->mode != MVR_CONF_DEF_INTF_MODE)) {
                    rc |= vtss_icfg_printf(result, "mvr name %s mode %s\n",
                                           mvif->name,
                                           (mvif->mode == MVR_INTF_MODE_DYNA) ? "dynamic" : "compatible");
                }
                if (req->all_defaults || (mvif->priority != MVR_CONF_DEF_INTF_PRIO)) {
                    rc |= vtss_icfg_printf(result, "mvr name %s frame priority %u\n",
                                           mvif->name,
                                           mvif->priority);
                }
                if (req->all_defaults || (mvif->vtag != MVR_CONF_DEF_INTF_VTAG)) {
                    rc |= vtss_icfg_printf(result, "%smvr name %s frame tagged\n",
                                           (mvif->vtag == IPMC_INTF_TAGED) ? "" : "no ",
                                           mvif->name);
                }
                if (req->all_defaults || (mvif->last_listener_query_interval != MVR_CONF_DEF_INTF_LLQI)) {
                    rc |= vtss_icfg_printf(result, "mvr name %s last-member-query-interval %u\n",
                                           mvif->name,
                                           mvif->last_listener_query_interval);
                }
            } else {
                rc |= vtss_icfg_printf(result, "mvr vlan %u\n", mvif->vid);
                if (req->all_defaults || (mvif->querier4_address != MVR_CONF_DEF_INTF_ADRS4)) {
                    memset(adrString, 0x0, sizeof(adrString));
                    rc |= vtss_icfg_printf(result, "%smvr vlan %u igmp-%s%s\n",
                                           mvif->querier4_address ? "" : "no ",
                                           mvif->vid,
                                           mvif->querier4_address ? "address " : "address",
                                           mvif->querier4_address ? misc_ipv4_txt(mvif->querier4_address, adrString) : "");
                }
                if (req->all_defaults || (mvif->mode != MVR_CONF_DEF_INTF_MODE)) {
                    rc |= vtss_icfg_printf(result, "mvr vlan %u mode %s\n",
                                           mvif->vid,
                                           (mvif->mode == MVR_INTF_MODE_DYNA) ? "dynamic" : "compatible");
                }
                if (req->all_defaults || (mvif->priority != MVR_CONF_DEF_INTF_PRIO)) {
                    rc |= vtss_icfg_printf(result, "mvr vlan %u frame priority %u\n",
                                           mvif->vid,
                                           mvif->priority);
                }
                if (req->all_defaults || (mvif->vtag != MVR_CONF_DEF_INTF_VTAG)) {
                    rc |= vtss_icfg_printf(result, "%smvr vlan %u frame tagged\n",
                                           (mvif->vtag == IPMC_INTF_TAGED) ? "" : "no ",
                                           mvif->vid);
                }
                if (req->all_defaults || (mvif->last_listener_query_interval != MVR_CONF_DEF_INTF_LLQI)) {
                    rc |= vtss_icfg_printf(result, "mvr vlan %u last-member-query-interval %u\n",
                                           mvif->vid,
                                           mvif->last_listener_query_interval);
                }
            }

            if (mvif->profile_index && IPMC_MEM_PROFILE_MTAKE(pfm)) {
                i8  profile_name[VTSS_IPMC_NAME_MAX_LEN];
                int str_len;

                fltr_profile = &pfm->profile;
                fltr_profile->data.index = mvif->profile_index;
                memset(profile_name, 0x0, sizeof(profile_name));
                str_len = 0;
                if ((ipmc_lib_mgmt_fltr_profile_get(fltr_profile, FALSE) == VTSS_OK) &&
                    ((str_len = strlen(fltr_profile->data.name)) > 0)) {
                    strncpy(profile_name, fltr_profile->data.name, str_len);
                }
                IPMC_MEM_PROFILE_MGIVE(pfm);

                if (str_len) {
                    if (mvif_strlen) {
                        rc |= vtss_icfg_printf(result, "mvr name %s channel %s\n", mvif->name, profile_name);
                    } else {
                        rc |= vtss_icfg_printf(result, "mvr vlan %u channel %s\n", mvif->vid, profile_name);
                    }
                } else {
                    if (req->all_defaults) {
                        if (mvif_strlen) {
                            rc |= vtss_icfg_printf(result, "no mvr name %s channel\n", mvif->name);
                        } else {
                            rc |= vtss_icfg_printf(result, "no mvr vlan %u channel\n", mvif->vid);
                        }
                    }
                }
            } else {
                if (req->all_defaults) {
                    if (mvif_strlen) {
                        rc |= vtss_icfg_printf(result, "no mvr name %s channel\n", mvif->name);
                    } else {
                        rc |= vtss_icfg_printf(result, "no mvr vlan %u channel\n", mvif->vid);
                    }
                }
            }
        }
    }

    return rc;
}

/* ICFG callback functions */
static vtss_rc _ipmc_mvr_icfg_port(const vtss_icfg_query_request_t *req,
                                   vtss_icfg_query_result_t *result)
{
    vtss_rc rc = VTSS_OK;

    if (req && result) {
        vtss_isid_t             isid;
        vtss_port_no_t          iport;
        mvr_mgmt_interface_t    mvr_icfg_intf;
        mvr_conf_intf_entry_t   *mvif;
        mvr_conf_port_role_t    *mvpt;
        mvr_conf_fast_leave_t   mvr_icfg_fast_leave;

        /*
            SUB-MODE: interface ethernet <port_id>
            COMMAND = mvr vlan <vlan_list> type {source | receiver}
            COMMAND = mvr name <mvr_name:word16> type {source | receiver}
            COMMAND = mvr immediate-leave
        */
        isid = topo_usid2isid(req->instance_id.port.usid);
        iport = uport2iport(req->instance_id.port.begin_uport);

        if (mvr_mgmt_get_fast_leave_port(isid, &mvr_icfg_fast_leave) == VTSS_OK) {
            BOOL    var_bol = VTSS_PORT_BF_GET(mvr_icfg_fast_leave.ports, iport);

            if (req->all_defaults || (var_bol != MVR_CONF_DEF_FAST_LEAVE)) {
                rc |= vtss_icfg_printf(result, " %smvr immediate-leave\n",
                                       var_bol ? "" : "no ");
            }
        } else {
            if (req->all_defaults) {
                rc |= vtss_icfg_printf(result, " no mvr immediate-leave\n");
            }
        }

        memset(&mvr_icfg_intf, 0x0, sizeof(mvr_mgmt_interface_t));
        while (mvr_mgmt_get_next_intf_entry(isid, &mvr_icfg_intf) == VTSS_OK) {
            mvif = &mvr_icfg_intf.intf;
            mvpt = &mvr_icfg_intf.role;

            if (req->all_defaults || (mvpt->ports[iport] != MVR_CONF_DEF_PORT_ROLE)) {
                if (mvpt->ports[iport] != MVR_PORT_ROLE_INACT) {
                    if (strlen(mvif->name)) {
                        rc |= vtss_icfg_printf(result, " mvr name %s type %s\n",
                                               mvif->name,
                                               ipmc_lib_port_role_txt(mvpt->ports[iport], IPMC_TXT_CASE_LOWER));
                    } else {
                        rc |= vtss_icfg_printf(result, " mvr vlan %u type %s\n",
                                               mvif->vid,
                                               ipmc_lib_port_role_txt(mvpt->ports[iport], IPMC_TXT_CASE_LOWER));
                    }
                } else {
                    if (strlen(mvif->name)) {
                        rc |= vtss_icfg_printf(result, " no mvr name %s type\n", mvif->name);
                    } else {
                        rc |= vtss_icfg_printf(result, " no mvr vlan %u type\n", mvif->vid);
                    }
                }
            }
        }
    }

    return rc;
}

/*
******************************************************************************

    Public functions

******************************************************************************
*/
/* Initialization function */
vtss_rc ipmc_mvr_icfg_init(void)
{
    vtss_rc rc;

    /* Register callback functions to ICFG module. */
    if (IPMC_MVR_ICFG_REG(rc, VTSS_ICFG_IPMC_MVR_INTF, "mvr", _ipmc_mvr_icfg_intf)) {
        if (IPMC_MVR_ICFG_REG(rc, VTSS_ICFG_IPMC_MVR_PORT, "mvr-port", _ipmc_mvr_icfg_port)) {
            if (IPMC_MVR_ICFG_REG(rc, VTSS_ICFG_IPMC_MVR_GLOBAL, "mvr", _ipmc_mvr_icfg_global)) {
                T_I("ipmc_mvr ICFG done");
            }
        }
    }

    return rc;
}
