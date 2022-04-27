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
#include "vtss_upnp_api.h"
#include "upnp_icfg.h"
#include "cli.h"
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

/*
******************************************************************************

    Public functions

******************************************************************************
*/

/* ICFG callback functions */
static vtss_rc UPNP_ICFG_global_conf(const vtss_icfg_query_request_t *req,
                                     vtss_icfg_query_result_t *result)
{
    vtss_rc             rc = VTSS_OK;
    upnp_conf_t         conf, def_conf;

    (void) upnp_mgmt_conf_get(&conf);
    (void) upnp_default_get(&def_conf);
    /* command: upnp
                upnp ttl <1-255>
                upnp advertising-duration <100-86400>
       */

    if (req->all_defaults ||
        (conf.mode != def_conf.mode)) {
        rc = vtss_icfg_printf(result, "%s\n",
                              conf.mode == UPNP_MGMT_ENABLED ? "upnp" : "no upnp");
    }

    if (req->all_defaults ||
        (conf.ttl != def_conf.ttl)) {
        rc = vtss_icfg_printf(result, "%s%u\n",
                              "upnp ttl ", (u32)conf.ttl);
    }

    if (req->all_defaults ||
        (conf.adv_interval != def_conf.adv_interval) ) {
        rc = vtss_icfg_printf(result, "%s%lu\n",
                              "upnp advertising-duration ", conf.adv_interval);
    }

    return rc;
}

/* Initialization function */
vtss_rc upnp_icfg_init(void)
{
    vtss_rc rc;

    /* Register callback functions to ICFG module.
       The configuration divided into two groups for this module.
       1. Global configuration
       2. Port configuration
    */
    if ((rc = vtss_icfg_query_register(VTSS_ICFG_UPNP_GLOBAL_CONF, "upnp", UPNP_ICFG_global_conf)) != VTSS_OK) {
        return rc;
    }

    return rc;
}
