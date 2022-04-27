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
#include "syslog_api.h"
#include "syslog_icfg.h"
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
static vtss_rc SYSLOG_ICFG_global_conf(const vtss_icfg_query_request_t *req,
                                       vtss_icfg_query_result_t *result)
{
    vtss_rc             rc = VTSS_OK;
    syslog_conf_t         conf, def_conf;

    (void) syslog_mgmt_conf_get(&conf);
    syslog_mgmt_default_get(&def_conf);

    /* command: logging on
                logging host {<ipv4_ucast> | <word45>}
                logging level [info|warning|error]
       */
    if (req->all_defaults ||
        (conf.server_mode != def_conf.server_mode)) {
        rc = vtss_icfg_printf(result, "%s\n",
                              conf.server_mode == TRUE ? "logging on" : "no logging on");
    }

    if (req->all_defaults ||
        strcmp(conf.syslog_server, def_conf.syslog_server) ) {
        rc = vtss_icfg_printf(result, "%s%s%s%s\n",
                              strlen(conf.syslog_server) ? "" : "no ",
                              "logging host",
                              strlen(conf.syslog_server) ? " " : "",
                              strlen(conf.syslog_server) ? conf.syslog_server : "");
    }

    if (req->all_defaults ||
        (conf.syslog_level != def_conf.syslog_level)) {
        rc = vtss_icfg_printf(result, "%s%s\n",
                              "logging level ", (conf.syslog_level == SYSLOG_LVL_INFO) ? "info" : (conf.syslog_level == SYSLOG_LVL_WARNING) ? "warning" : "error");
    }

    return rc;
}

/*
******************************************************************************

    Public functions

******************************************************************************
*/

/* Initialization function */
vtss_rc syslog_icfg_init(void)
{
    vtss_rc rc;

    /* Register callback functions to ICFG module.
       The configuration divided into two groups for this module.
       1. Global configuration
       2. Port configuration
    */
    if ((rc = vtss_icfg_query_register(VTSS_ICFG_SYSLOG_GLOBAL_CONF, "logging", SYSLOG_ICFG_global_conf)) != VTSS_OK) {
        return rc;
    }

    return rc;
}

