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

#ifdef VTSS_SW_OPTION_IP2
#ifndef VTSS_SW_OPTION_NTP
#ifdef VTSS_SW_OPTION_SNTP

/*
******************************************************************************

    Include files

******************************************************************************
*/
#ifdef VTSS_SW_OPTION_SYSUTIL
#include "sysutil_api.h"
#endif
#include "icfg_api.h"
#include "vtss_sntp_api.h"
#include "vtss_sntp_icfg.h"
#include "misc_api.h"   //uport2iport(), iport2uport()
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
static vtss_rc VTSS_SNTP_ICFG_global_conf(const vtss_icfg_query_request_t *req,
                                          vtss_icfg_query_result_t *result)
{
    vtss_rc                             rc = VTSS_OK;
    sntp_conf_t                         conf;
    sntp_conf_t                         def_conf;
    u32                                 length;

    if ((rc = sntp_config_get(&conf)) != VTSS_OK) {
        return rc;
    }

    vtss_sntp_default_set(&def_conf);

    /* global mode */
    // example: sntp
    if (req->all_defaults || conf.mode != def_conf.mode) {
        rc = vtss_icfg_printf(result, "%s%s\n",
                              conf.mode == SNTP_MGMT_ENABLED ? "" : VTSS_SNTP_NO_FORM_TEXT,
                              VTSS_SNTP_GLOBAL_MODE_ENABLE_TEXT);
    }

    /* entries */
    // example: sntp server ip-address {<ipv4_ucast>}
    length = strlen((char *)conf.sntp_server);
    if (length > 0) {
        rc = vtss_icfg_printf(result, "%s %s %s\n",
                              VTSS_SNTP_GLOBAL_MODE_SERVER_TEXT,
                              VTSS_SNTP_GLOBAL_MODE_IP_TEXT,
                              conf.sntp_server);
    }

    return rc;
}

/*
******************************************************************************

    Public functions

******************************************************************************
*/

/* Initialization function */
vtss_rc vtss_sntp_icfg_init(void)
{
    vtss_rc rc;

    /* Register callback functions to ICFG module.
       The configuration divided into two groups for this module.
       1. Global configuration
       2. Port configuration
    */
    if ((rc = vtss_icfg_query_register(VTSS_ICFG_VTSS_SNTP_GLOBAL_CONF, "sntp", VTSS_SNTP_ICFG_global_conf)) != VTSS_OK) {
        return rc;
    }

    return rc;
}


#endif
#endif
#endif

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
