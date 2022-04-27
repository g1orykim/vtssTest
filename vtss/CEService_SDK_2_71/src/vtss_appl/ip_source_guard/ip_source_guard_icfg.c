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

/*
******************************************************************************

    Include files

******************************************************************************
*/
#include "icfg_api.h"
#include "ip_source_guard_api.h"
#include "ip_source_guard_icfg.h"
#include "misc_api.h"   //uport2iport(), iport2uport()
#include "topo_api.h"   //topo_usid2isid(), topo_isid2usid()
#include "icli_porting_util.h"

/*
******************************************************************************

    Constant and Macro definition

******************************************************************************
*/

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
/*lint -esym(459, IP_SOURCE_GUARD_ICFG_global_conf)                       */
static vtss_rc IP_SOURCE_GUARD_ICFG_global_conf(const vtss_icfg_query_request_t *req,
                                                vtss_icfg_query_result_t *result)
{
    vtss_rc                             rc = VTSS_OK;
    u32                                 mode;
    static ip_source_guard_conf_t       def_conf;
    ip_source_guard_entry_t             entry;
    char                                buf0[80], buf1[80], buf2[80];


    if ((rc = ip_source_guard_mgmt_conf_get_mode(&mode)) != VTSS_OK) {
        return rc;
    }

    ip_source_guard_default_set(&def_conf);

    /* global mode */
    // example: ip verify source
    if (req->all_defaults || mode != def_conf.mode) {
        rc = vtss_icfg_printf(result, "%s%s\n",
                              mode == IP_SOURCE_GUARD_MGMT_ENABLED ? "" : IP_SOURCE_GUARD_NO_FORM_TEXT,
                              IP_SOURCE_GUARD_GLOBAL_MODE_ENABLE_TEXT);
    }

    /* entries */
    // example: ip source binding interface <port_type> <port_id> <vlan_id> <ipv4_ucast> <mac_ucast>
    if (ip_source_guard_mgmt_conf_get_first_static_entry(&entry) == VTSS_OK) {

        rc = vtss_icfg_printf(result, "%s %s %s %d %s %s\n",
                              IP_SOURCE_GUARD_GLOBAL_MODE_ENTRY_TEXT,
                              IP_SOURCE_GUARD_INTERFACE_TEXT,
                              icli_port_info_txt(topo_isid2usid(entry.isid), iport2uport(entry.port_no), buf0),
                              entry.vid,
                              misc_ipv4_txt(entry.assigned_ip, buf1),
#if defined(VTSS_FEATURE_ACL_V2)
                              misc_mac_txt(entry.assigned_mac, buf2));
#else
                              misc_ipv4_txt(entry.ip_mask, buf2));
#endif

        while (ip_source_guard_mgmt_conf_get_next_static_entry(&entry) == VTSS_OK) {
            rc = vtss_icfg_printf(result, "%s %s %s %d %s %s\n",
                                  IP_SOURCE_GUARD_GLOBAL_MODE_ENTRY_TEXT,
                                  IP_SOURCE_GUARD_INTERFACE_TEXT,
                                  icli_port_info_txt(topo_isid2usid(entry.isid), iport2uport(entry.port_no), buf0),
                                  entry.vid,
                                  misc_ipv4_txt(entry.assigned_ip, buf1),
#if defined(VTSS_FEATURE_ACL_V2)
                                  misc_mac_txt(entry.assigned_mac, buf2));
#else
                                  misc_ipv4_txt(entry.ip_mask, buf2));
#endif
        }
    }

    return rc;
}

/*lint -esym(459, IP_SOURCE_GUARD_ICFG_port_conf)                       */
static vtss_rc IP_SOURCE_GUARD_ICFG_port_conf(const vtss_icfg_query_request_t *req,
                                              vtss_icfg_query_result_t *result)
{
    vtss_rc                                     rc = VTSS_OK;
    static ip_source_guard_conf_t               def_conf;
    ip_source_guard_port_mode_conf_t            conf;
    ip_source_guard_port_dynamic_entry_conf_t   entry_cnt_conf;
    vtss_isid_t                                 isid = topo_usid2isid(req->instance_id.port.usid);
    vtss_port_no_t                              iport = uport2iport(req->instance_id.port.begin_uport);


    if ((rc = ip_source_guard_mgmt_conf_get_port_mode(isid, &conf)) != VTSS_OK) {
        return rc;
    }

    ip_source_guard_default_set(&def_conf);

    /* port mode */
    // example: ip verify source
    if (req->all_defaults || conf.mode[iport] != def_conf.port_mode_conf[isid - VTSS_ISID_START].mode[iport]) {
        rc = vtss_icfg_printf(result, " %s%s\n",
                              conf.mode[iport] == IP_SOURCE_GUARD_MGMT_ENABLED ? "" : IP_SOURCE_GUARD_NO_FORM_TEXT,
                              IP_SOURCE_GUARD_PORT_MODE_ENABLE_TEXT);
    }

    if ((rc = ip_source_guard_mgmt_conf_get_port_dynamic_entry_cnt(isid, &entry_cnt_conf)) != VTSS_OK) {
        return rc;
    }

    /* port limit */
    // example: ip verify source limit <0-2>
    if (req->all_defaults || entry_cnt_conf.entry_cnt[iport] != def_conf.port_dynamic_entry_conf[isid - VTSS_ISID_START].entry_cnt[iport]) {
        if (entry_cnt_conf.entry_cnt[iport] == IP_SOURCE_GUARD_DYNAMIC_UNLIMITED) {
            rc = vtss_icfg_printf(result, " %s%s\n",
                                  IP_SOURCE_GUARD_NO_FORM_TEXT,
                                  IP_SOURCE_GUARD_PORT_MODE_LIMIT_TEXT);
        } else {
            rc = vtss_icfg_printf(result, " %s %u\n",
                                  IP_SOURCE_GUARD_PORT_MODE_LIMIT_TEXT,
                                  entry_cnt_conf.entry_cnt[iport]);
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
vtss_rc ip_source_guard_icfg_init(void)
{
    vtss_rc rc;

    /* Register callback functions to ICFG module.
       The configuration divided into two groups for this module.
       1. Global configuration
       2. Port configuration
    */
    if ((rc = vtss_icfg_query_register(VTSS_ICFG_IP_SOURCE_GUARD_GLOBAL_CONF, "source-guard", IP_SOURCE_GUARD_ICFG_global_conf)) != VTSS_OK) {
        return rc;
    }

    rc = vtss_icfg_query_register(VTSS_ICFG_IP_SOURCE_GUARD_PORT_CONF, "source-guard", IP_SOURCE_GUARD_ICFG_port_conf);

    return rc;
}
