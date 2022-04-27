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
#include "vtss_ntp_api.h"
#include "vtss_ntp_icfg.h"
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
static vtss_rc VTSS_NTP_ICFG_global_conf(const vtss_icfg_query_request_t *req,
                                         vtss_icfg_query_result_t *result)
{
    vtss_rc                             rc = VTSS_OK;
    ntp_conf_t                          conf;
    ntp_conf_t                          def_conf;
    int                                 i;
#ifdef VTSS_SW_OPTION_IPV6
    char                                ip_buf0[80];
#endif

    if ((rc = ntp_mgmt_conf_get(&conf)) != VTSS_OK) {
        return rc;
    }

    vtss_ntp_default_set(&def_conf);

    /* global mode */
    // example: ntp
    if (req->all_defaults || conf.mode_enabled != def_conf.mode_enabled) {
        rc = vtss_icfg_printf(result, "%s%s\n",
                              conf.mode_enabled == NTP_MGMT_ENABLED ? "" : VTSS_NTP_NO_FORM_TEXT,
                              VTSS_NTP_GLOBAL_MODE_ENABLE_TEXT);
    }

    /* entries */
    // example: ntp server <1-5> ip-address {<ipv4_ucast>|<ipv6_ucast>|<word>}
    for (i = 0; i < NTP_MAX_SERVER_COUNT; i++) {

        if (strlen(conf.server[i].ip_host_string) > 0
#ifdef VTSS_SW_OPTION_IPV6
            || conf.server[i].ipv6_addr.addr[0] > 0) {
            rc = vtss_icfg_printf(result, "%s %d %s %s\n",
                                  VTSS_NTP_GLOBAL_MODE_SERVER_TEXT,
                                  i + 1,
                                  VTSS_NTP_GLOBAL_MODE_IP_TEXT,
                                  conf.server[i].ip_type == NTP_IP_TYPE_IPV4 ? (char *)conf.server[i].ip_host_string : misc_ipv6_txt(&conf.server[i].ipv6_addr, ip_buf0));
#else
           ) {
            rc = vtss_icfg_printf(result, "%s %d %s %s\n",
                                  VTSS_NTP_GLOBAL_MODE_SERVER_TEXT,
                                  i + 1,
                                  VTSS_NTP_GLOBAL_MODE_IP_TEXT,
                                  conf.server[i].ip_host_string);
#endif
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
vtss_rc vtss_ntp_icfg_init(void)
{
    vtss_rc rc;

    /* Register callback functions to ICFG module.
       The configuration divided into two groups for this module.
       1. Global configuration
       2. Port configuration
    */
    if ((rc = vtss_icfg_query_register(VTSS_ICFG_VTSS_NTP_GLOBAL_CONF, "ntp", VTSS_NTP_ICFG_global_conf)) != VTSS_OK) {
        return rc;
    }

    return rc;
}
