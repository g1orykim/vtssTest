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
#include "dhcp_snooping_api.h"
#include "dhcp_snooping_icfg.h"
#include "misc_api.h"   //uport2iport(), iport2uport()
#include "topo_api.h"   //topo_usid2isid(), topo_isid2usid()

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
static vtss_rc DHCP_SNOOPING_ICFG_global_conf(const vtss_icfg_query_request_t *req,
                                              vtss_icfg_query_result_t *result)
{
    vtss_rc                 rc = VTSS_OK;
    dhcp_snooping_conf_t    conf, def_conf;
    int                     conf_changed = 0;

    if ((rc = dhcp_snooping_mgmt_conf_get(&conf)) != VTSS_OK) {
        return rc;
    }

    dhcp_snooping_mgmt_conf_get_default(&def_conf);
    conf_changed = dhcp_snooping_mgmt_conf_changed(&def_conf, &conf);
    if (req->all_defaults ||
        (conf_changed && conf.snooping_mode != def_conf.snooping_mode)) {
        rc = vtss_icfg_printf(result, "%s%s\n",
                              conf.snooping_mode == DHCP_SNOOPING_MGMT_ENABLED ? "" : DHCP_SNOOPING_NO_FORM_TEXT,
                              DHCP_SNOOPING_GLOBAL_MODE_ENABLE_TEXT);
    }

    return rc;
}

static vtss_rc DHCP_SNOOPING_ICFG_port_conf(const vtss_icfg_query_request_t *req,
                                            vtss_icfg_query_result_t *result)
{
    vtss_rc                     rc = VTSS_OK;
    dhcp_snooping_port_conf_t   conf, def_conf;
    vtss_isid_t                 isid = topo_usid2isid(req->instance_id.port.usid);
    vtss_port_no_t              iport = uport2iport(req->instance_id.port.begin_uport);
    int                         conf_changed = 0;

    if ((rc = dhcp_snooping_mgmt_port_conf_get(isid, &conf)) != VTSS_OK) {
        return rc;
    }

    dhcp_snooping_mgmt_port_get_default(isid, &def_conf);
    conf_changed = dhcp_snooping_mgmt_port_conf_changed(&def_conf, &conf);
    if (req->all_defaults ||
        (conf_changed && def_conf.port_mode[iport] != conf.port_mode[iport])) {
        rc = vtss_icfg_printf(result, " %s%s\n",
                              conf.port_mode[iport] == DHCP_HELPER_PORT_MODE_TRUSTED ? "" : DHCP_SNOOPING_NO_FORM_TEXT,
                              DHCP_SNOOPING_PORT_MODE_TRUST_TEXT);
    }

    return rc;
}

/*
******************************************************************************

    Public functions

******************************************************************************
*/

/* Initialization function */
vtss_rc dhcp_snooping_icfg_init(void)
{
    vtss_rc rc;

    /* Register callback functions to ICFG module.
       The configuration divided into two groups for this module.
       1. Global configuration
       2. Port configuration
    */
    if ((rc = vtss_icfg_query_register(VTSS_ICFG_DHCP_SNOOPING_GLOBAL_CONF, "dhcp-snooping", DHCP_SNOOPING_ICFG_global_conf)) != VTSS_OK) {
        return rc;
    }

    rc = vtss_icfg_query_register(VTSS_ICFG_DHCP_SNOOPING_PORT_CONF, "dhcp-snooping", DHCP_SNOOPING_ICFG_port_conf);

    return rc;
}
