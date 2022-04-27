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
#include "ipmc_lib.h"
#include "ipmc_lib_icfg.h"


/*
******************************************************************************

    Constant and Macro definition

******************************************************************************
*/
#define VTSS_TRACE_MODULE_ID        VTSS_MODULE_ID_IPMC_LIB

#define IPMC_LIB_ICFG_REG(x, y, z, w)  (((x) = vtss_icfg_query_register((y), (z), (w))) == VTSS_OK)

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
static vtss_rc _ipmc_lib_icfg_profile_state(const vtss_icfg_query_request_t *req,
                                            vtss_icfg_query_result_t *result)
{
    vtss_rc rc = VTSS_OK;
#if ((defined(VTSS_SW_OPTION_SMB_IPMC) || defined(VTSS_SW_OPTION_MVR)) && defined(VTSS_SW_OPTION_IPMC_LIB))
    BOOL    state;

    if (req && result) {
        /*
            COMMAND = ipmc profile
        */
        if (ipmc_lib_mgmt_profile_state_get(&state) == VTSS_OK) {
            if (req->all_defaults) {
                rc = vtss_icfg_printf(result, "%s\n",
                                      state ? "ipmc profile" : "no ipmc profile");
            } else {
                if (state != IPMC_LIB_FLTR_PROFILE_DEF_STATE) {
                    rc = vtss_icfg_printf(result, "%s\n",
                                          state ? "ipmc profile" : "no ipmc profile");
                }
            }
        }
    }
#endif /* (VTSS_SW_OPTION_SMB_IPMC || VTSS_SW_OPTION_MVR) && VTSS_SW_OPTION_IPMC_LIB */

    return rc;
}

static vtss_rc _ipmc_lib_icfg_profile_table(const vtss_icfg_query_request_t *req,
                                            vtss_icfg_query_result_t *result)
{
    vtss_rc                 rc;
    ipmc_lib_profile_mem_t  *pfm;

    if (!IPMC_MEM_PROFILE_MTAKE(pfm)) {
        return VTSS_RC_ERROR;
    }

    rc = VTSS_OK;
#if ((defined(VTSS_SW_OPTION_SMB_IPMC) || defined(VTSS_SW_OPTION_MVR)) && defined(VTSS_SW_OPTION_IPMC_LIB))
    if (req && result) {
        ipmc_lib_grp_fltr_profile_t *fltr_profile;
        ipmc_lib_profile_t          *data;
        ipmc_lib_rule_t             fltr_rule;
        ipmc_lib_grp_fltr_entry_t   fltr_entry;

        /*
            COMMAND = description <profile_desc:line64>
            COMMAND = range <entry_name:word16> {permit | deny} [log] [next <next_entry:word16>]
        */
        fltr_profile = &pfm->profile;
        memset(fltr_profile, 0x0, sizeof(ipmc_lib_grp_fltr_profile_t));
        memcpy(fltr_profile->data.name, req->instance_id.string, sizeof(fltr_profile->data.name));
        if (ipmc_lib_mgmt_fltr_profile_get(fltr_profile, TRUE) == VTSS_OK) {
            data = &fltr_profile->data;
            if (strlen(data->desc)) {
                rc = vtss_icfg_printf(result, " description %s\n", data->desc);
            } else {
                if (req->all_defaults) {
                    rc = vtss_icfg_printf(result, " no description\n");
                }
            }

            if ((rc == VTSS_OK) && (ipmc_lib_mgmt_fltr_profile_rule_get_first(data->index, &fltr_rule) == VTSS_OK)) {
                do {
                    fltr_entry.index = fltr_rule.entry_index;
                    if (ipmc_lib_mgmt_fltr_entry_get(&fltr_entry, FALSE) == VTSS_OK) {
                        if (fltr_rule.log) {
                            rc = vtss_icfg_printf(result, " range %s %s log\n",
                                                  fltr_entry.name,
                                                  ipmc_lib_fltr_action_txt(fltr_rule.action, IPMC_TXT_CASE_LOWER));
                        } else {
                            rc = vtss_icfg_printf(result, " range %s %s\n",
                                                  fltr_entry.name,
                                                  ipmc_lib_fltr_action_txt(fltr_rule.action, IPMC_TXT_CASE_LOWER));
                        }
                    }

                    if (rc != VTSS_OK) {
                        break;
                    }
                } while (ipmc_lib_mgmt_fltr_profile_rule_get_next(data->index, &fltr_rule) == VTSS_OK);
            }
        }
    }
#endif /* (VTSS_SW_OPTION_SMB_IPMC || VTSS_SW_OPTION_MVR) && VTSS_SW_OPTION_IPMC_LIB */

    IPMC_MEM_PROFILE_MGIVE(pfm);
    return rc;
}

static vtss_rc _ipmc_lib_icfg_profile_entry(const vtss_icfg_query_request_t *req,
                                            vtss_icfg_query_result_t *result)
{
    vtss_rc                     rc = VTSS_OK;
#if ((defined(VTSS_SW_OPTION_SMB_IPMC) || defined(VTSS_SW_OPTION_MVR)) && defined(VTSS_SW_OPTION_IPMC_LIB))
    ipmc_lib_grp_fltr_entry_t   fltr_entry;
    i8                          bgnString[40], endString[40];
    vtss_ipv4_t                 bgn4, end4;

    if (req && result) {
        /*
            COMMAND = ipmc range <entry_name:word16> { <ipv4_mcast> [ <ipv4_mcast> ] | <ipv6_mcast> [ <ipv6_mcast> ] }
        */
        memset(&fltr_entry, 0x0, sizeof(ipmc_lib_grp_fltr_entry_t));
        while (ipmc_lib_mgmt_fltr_entry_get_next(&fltr_entry, FALSE) == VTSS_OK) {
            memset(bgnString, 0x0, sizeof(bgnString));
            memset(endString, 0x0, sizeof(endString));
            if (fltr_entry.version == IPMC_IP_VERSION_IGMP) {
                IPMC_LIB_ADRS_6TO4_SET(fltr_entry.grp_bgn, bgn4);
                IPMC_LIB_ADRS_6TO4_SET(fltr_entry.grp_end, end4);
                rc = vtss_icfg_printf(result, "ipmc range %s %s %s\n",
                                      fltr_entry.name,
                                      misc_ipv4_txt(htonl(bgn4), bgnString),
                                      misc_ipv4_txt(htonl(end4), endString));
            } else {
                rc = vtss_icfg_printf(result, "ipmc range %s %s %s\n",
                                      fltr_entry.name,
                                      misc_ipv6_txt(&fltr_entry.grp_bgn, bgnString),
                                      misc_ipv6_txt(&fltr_entry.grp_end, endString));
            }

            if (rc != VTSS_OK) {
                break;
            }
        }
    }
#endif /* (VTSS_SW_OPTION_SMB_IPMC || VTSS_SW_OPTION_MVR) && VTSS_SW_OPTION_IPMC_LIB */

    return rc;
}

/*
******************************************************************************

    Public functions

******************************************************************************
*/
/* Initialization function */
vtss_rc ipmc_lib_icfg_init(void)
{
    vtss_rc rc;

    /* Register callback functions to ICFG module. */
    if (IPMC_LIB_ICFG_REG(rc, VTSS_ICFG_IPMC_LIB_IPMC_RANGE, "ipmc-profile-range", _ipmc_lib_icfg_profile_entry)) {
        if (IPMC_LIB_ICFG_REG(rc, VTSS_ICFG_IPMC_LIB_PROFILE_DESC_RANGE, "ipmc-profile", _ipmc_lib_icfg_profile_table)) {
            if (IPMC_LIB_ICFG_REG(rc, VTSS_ICFG_IPMC_LIB_IPMC_PROFILE_CTRL, "ipmc-profile", _ipmc_lib_icfg_profile_state)) {
                T_I("ipmc_lib ICFG done");
            }
        }
    }

    return rc;
}
