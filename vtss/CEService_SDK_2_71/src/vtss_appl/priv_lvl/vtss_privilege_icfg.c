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
#include "vtss_privilege_api.h"
#include "vtss_privilege_icfg.h"

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
static BOOL diffConf(vtss_priv_module_conf_t *conf_ptr, vtss_priv_module_conf_t *defconf_ptr)
{
    return (conf_ptr->cro != defconf_ptr->cro ||
            conf_ptr->crw != defconf_ptr->crw ||
            conf_ptr->sro != defconf_ptr->sro ||
            conf_ptr->srw != defconf_ptr->srw);
}

/* ICFG callback functions */
static vtss_rc PRIV_ICFG_global_conf(const vtss_icfg_query_request_t *req,
                                     vtss_icfg_query_result_t *result)
{
    vtss_priv_conf_t            conf, def_conf;
    vtss_priv_module_conf_t     *conf_ptr = &conf.privilege_level[0], *defconf_ptr = &def_conf.privilege_level[0]; /**< Privilege level */
    vtss_module_id_t            idx = 0;
    i8                          name[VTSS_PRIV_LVL_NAME_LEN_MAX] = "";
    vtss_rc                     rc = VTSS_OK;

    if ((rc = vtss_priv_mgmt_conf_get(&conf)) != VTSS_OK) {
        return rc;
    }
    VTSS_PRIVILEGE_default_get(&def_conf);

    /* command: snmp-server
                web privilege group <group_name> level { [cro <cro:0-15>]  [crw <crw:0-15>] [sro <sro:0-15>] [srw <srw:0-15>] }*1
       */
    while (TRUE == vtss_privilege_group_name_get(name, &idx, TRUE)) {
        if (req->all_defaults || diffConf(&conf_ptr[idx], &defconf_ptr[idx])) {
            rc += vtss_icfg_printf(result, "web privilege group %s level cro %d crw %d sro %d srw %d\n", name,
                                   conf_ptr[idx].cro, conf_ptr[idx].crw, conf_ptr[idx].sro, conf_ptr[idx].srw);
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
vtss_rc priv_icfg_init(void)
{
    vtss_rc rc;

    /* Register callback functions to ICFG module.
       The configuration divided into two groups for this module.
       1. Global configuration
       2. Port configuration
    */
    if ((rc = vtss_icfg_query_register(VTSS_ICFG_WEB_PRIV_LVL_GLOBAL_CONF, "web-privilege-group-level", PRIV_ICFG_global_conf)) != VTSS_OK) {
        return rc;
    }

    return rc;
}
