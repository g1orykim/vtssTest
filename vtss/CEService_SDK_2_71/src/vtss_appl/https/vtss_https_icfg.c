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
#include "vtss_https_api.h"
#include "vtss_https_icfg.h"

/*
******************************************************************************

    Constant and Macro definition

******************************************************************************
*/
#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_HTTPS

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
static vtss_rc VTSS_HTTPS_ICFG_global_conf(const vtss_icfg_query_request_t *req,
                                           vtss_icfg_query_result_t *result)
{
    vtss_rc         rc = VTSS_RC_ERROR;
    https_conf_t    *conf, *def_conf;
    int             conf_changed = 0;

    conf     = (https_conf_t *) VTSS_MALLOC(sizeof(https_conf_t));
    def_conf = (https_conf_t *) VTSS_MALLOC(sizeof(https_conf_t));

    if (!conf || !def_conf) {
        goto out;
    }

    if ((rc = https_mgmt_conf_get(conf)) != VTSS_OK) {
        goto out;
    }

    https_conf_mgmt_get_default(def_conf);
    conf_changed = https_mgmt_conf_changed(def_conf, conf);
    if (req->all_defaults ||
        (conf_changed && conf->mode != def_conf->mode)) {
        rc = vtss_icfg_printf(result, "%s%s\n",
                              conf->mode == HTTPS_MGMT_ENABLED ? "" : VTSS_HTTPS_NO_FORM_TEXT,
                              VTSS_HTTPS_GLOBAL_MODE_ENABLE_TEXT);
    }

    if (req->all_defaults ||
        (conf_changed && conf->redirect != def_conf->redirect)) {
        rc = vtss_icfg_printf(result, "%s%s\n",
                              conf->redirect == HTTPS_MGMT_ENABLED ? "" : VTSS_HTTPS_NO_FORM_TEXT,
                              VTSS_HTTPS_REDIRECT_MODE_ENABLE_TEXT);
    }

out:
    if (conf) {
        VTSS_FREE(conf);
    }
    if (def_conf) {
        VTSS_FREE(def_conf);
    }

    return rc;
}

/*
******************************************************************************

    Public functions

******************************************************************************
*/

/* Initialization function */
vtss_rc vtss_https_icfg_init(void)
{
    /* Register callback functions to ICFG module. */
    return vtss_icfg_query_register(VTSS_HTTPS_ICFG_GLOBAL_CONF, "http", VTSS_HTTPS_ICFG_global_conf);
}
