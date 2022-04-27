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
#include "perf_mon_api.h"
#include "perf_mon_icfg.h"
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
static vtss_rc PERF_MON_ICFG_global_conf(const vtss_icfg_query_request_t *req,
                                         vtss_icfg_query_result_t *result)
{
    vtss_rc                 rc = VTSS_OK;
    perf_mon_conf_t         global_conf;
    int                     i;

    //get global configuration
    perf_mon_conf_get(&global_conf);

    /* session */
    // example: perf-mon session [ lm | dm | evc | ece ]
    if (global_conf.lm_session_mode) {
        rc = vtss_icfg_printf(result, "%s %s %s\n",
                              VTSS_PM_TEXT,
                              VTSS_PM_SESSION_TEXT,
                              VTSS_PM_SESSION_LM_TEXT);
    }

    if (global_conf.dm_session_mode) {
        rc = vtss_icfg_printf(result, "%s %s %s\n",
                              VTSS_PM_TEXT,
                              VTSS_PM_SESSION_TEXT,
                              VTSS_PM_SESSION_DM_TEXT);
    }

    if (global_conf.evc_session_mode) {
        rc = vtss_icfg_printf(result, "%s %s %s\n",
                              VTSS_PM_TEXT,
                              VTSS_PM_SESSION_TEXT,
                              VTSS_PM_SESSION_EVC_TEXT);
    }

    if (global_conf.ece_session_mode) {
        rc = vtss_icfg_printf(result, "%s %s %s\n",
                              VTSS_PM_TEXT,
                              VTSS_PM_SESSION_TEXT,
                              VTSS_PM_SESSION_ECE_TEXT);
    }

    /* storage */
    // example: perf-mon storage [ lm | dm | evc | ece ]
    if (global_conf.lm_storage_mode) {
        rc = vtss_icfg_printf(result, "%s %s %s\n",
                              VTSS_PM_TEXT,
                              VTSS_PM_STORAGE_TEXT,
                              VTSS_PM_STORAGE_LM_TEXT);
    }

    if (global_conf.dm_storage_mode) {
        rc = vtss_icfg_printf(result, "%s %s %s\n",
                              VTSS_PM_TEXT,
                              VTSS_PM_STORAGE_TEXT,
                              VTSS_PM_STORAGE_DM_TEXT);
    }

    if (global_conf.evc_storage_mode) {
        rc = vtss_icfg_printf(result, "%s %s %s\n",
                              VTSS_PM_TEXT,
                              VTSS_PM_STORAGE_TEXT,
                              VTSS_PM_STORAGE_EVC_TEXT);
    }

    if (global_conf.ece_storage_mode) {
        rc = vtss_icfg_printf(result, "%s %s %s\n",
                              VTSS_PM_TEXT,
                              VTSS_PM_STORAGE_TEXT,
                              VTSS_PM_STORAGE_ECE_TEXT);
    }

    /* interval */
    // example: perf-mon interval { lm | dm | evc | ece } <1-60>
    if (req->all_defaults || global_conf.lm_interval != VTSS_PM_DEFAULT_LM_INTERVAL) {
        rc = vtss_icfg_printf(result, "%s %s %s %d\n",
                              VTSS_PM_TEXT,
                              VTSS_PM_INTERVAL_TEXT,
                              VTSS_PM_STORAGE_LM_TEXT,
                              global_conf.lm_interval);
    }

    if (req->all_defaults || global_conf.dm_interval != VTSS_PM_DEFAULT_DM_INTERVAL) {
        rc = vtss_icfg_printf(result, "%s %s %s %d\n",
                              VTSS_PM_TEXT,
                              VTSS_PM_INTERVAL_TEXT,
                              VTSS_PM_STORAGE_DM_TEXT,
                              global_conf.dm_interval);
    }

    if (req->all_defaults || global_conf.evc_interval != VTSS_PM_DEFAULT_EVC_INTERVAL) {
        rc = vtss_icfg_printf(result, "%s %s %s %d\n",
                              VTSS_PM_TEXT,
                              VTSS_PM_INTERVAL_TEXT,
                              VTSS_PM_STORAGE_EVC_TEXT,
                              global_conf.evc_interval);
    }

    if (req->all_defaults || global_conf.ece_interval != VTSS_PM_DEFAULT_ECE_INTERVAL) {
        rc = vtss_icfg_printf(result, "%s %s %s %d\n",
                              VTSS_PM_TEXT,
                              VTSS_PM_INTERVAL_TEXT,
                              VTSS_PM_STORAGE_ECE_TEXT,
                              global_conf.ece_interval);
    }

    /* transfer mode */
    // example: perf-mon transfer */
    if (global_conf.transfer_mode) {
        rc = vtss_icfg_printf(result, "%s %s\n",
                              VTSS_PM_TEXT,
                              VTSS_PM_TRANSFER_TEXT);
    }

    /* transfer hour */
    // example: perf-mon transfer hour <0-23>
    for (i = 0; i < 24; ++i) {
        if (VTSS_PM_BF_GET(global_conf.transfer_scheduled_hours, i)) {
            rc = vtss_icfg_printf(result, "%s %s %s %d\n",
                                  VTSS_PM_TEXT,
                                  VTSS_PM_TRANSFER_TEXT,
                                  VTSS_PM_TRANSFER_HOURS_TEXT,
                                  i);
        }
    }

    /* transfer minute */
    // example: perf-mon transfer minute <0,15,30,45>
    for (i = 0; i < 4; ++i) {
        if (VTSS_PM_BF_GET(global_conf.transfer_scheduled_minutes, i)) {
            rc = vtss_icfg_printf(result, "%s %s %s %d\n",
                                  VTSS_PM_TEXT,
                                  VTSS_PM_TRANSFER_TEXT,
                                  VTSS_PM_TRANSFER_MINUTE_TEXT,
                                  (i * 15));
        }
    }

    /* transfer fixed-offset */
    // example: perf-mon transfer fixed-offset <1-15>
    if (global_conf.transfer_scheduled_offset != VTSS_PM_DEFAULT_FIXED_OFFSET) {
        rc = vtss_icfg_printf(result, "%s %s %s %d\n",
                              VTSS_PM_TEXT,
                              VTSS_PM_TRANSFER_TEXT,
                              VTSS_PM_TRANSFER_FIXED_TEXT,
                              global_conf.transfer_scheduled_offset);
    }

    /* transfer random-offset */
    // example: perf-mon transfer random-offset <1-900>
    if (global_conf.transfer_scheduled_random_offset != VTSS_PM_DEFAULT_RANDOM_OFFSET) {
        rc = vtss_icfg_printf(result, "%s %s %s %d\n",
                              VTSS_PM_TEXT,
                              VTSS_PM_TRANSFER_TEXT,
                              VTSS_PM_TRANSFER_RANDOM_TEXT,
                              global_conf.transfer_scheduled_random_offset);
    }

    /* transfer url */
    // example: perf-mon transfer url <word64>
    if (strlen(global_conf.transfer_server_url)) {
        rc = vtss_icfg_printf(result, "%s %s %s %s\n",
                              VTSS_PM_TEXT,
                              VTSS_PM_TRANSFER_TEXT,
                              VTSS_PM_TRANSFER_URL_TEXT,
                              global_conf.transfer_server_url);
    }

    /* transfer mode */
    // example: perf-mon transfer mode { all | new | fixed <1-96> }
    if (VTSS_PM_BF_GET(global_conf.transfer_interval_mode, VTSS_PM_TRANSFER_MODE_ALL)) {
        rc = vtss_icfg_printf(result, "%s %s %s %s\n",
                              VTSS_PM_TEXT,
                              VTSS_PM_TRANSFER_TEXT,
                              VTSS_PM_TRANSFER_INTERVAL_MODE_TEXT,
                              VTSS_PM_TRANSFER_INTERVAL_MODE_ALL_TEXT);
    } else if (VTSS_PM_BF_GET(global_conf.transfer_interval_mode, VTSS_PM_TRANSFER_MODE_NEW)) {
        rc = vtss_icfg_printf(result, "%s %s %s %s\n",
                              VTSS_PM_TEXT,
                              VTSS_PM_TRANSFER_TEXT,
                              VTSS_PM_TRANSFER_INTERVAL_MODE_TEXT,
                              VTSS_PM_TRANSFER_INTERVAL_MODE_NEW_TEXT);
    } else if (VTSS_PM_BF_GET(global_conf.transfer_interval_mode, VTSS_PM_TRANSFER_MODE_FIXED)) {
        rc = vtss_icfg_printf(result, "%s %s %s %s %d\n",
                              VTSS_PM_TEXT,
                              VTSS_PM_TRANSFER_TEXT,
                              VTSS_PM_TRANSFER_INTERVAL_MODE_TEXT,
                              VTSS_PM_TRANSFER_INTERVAL_MODE_FIXED_TEXT,
                              global_conf.transfer_interval_num);
    }

    /* transfer incomplete */
    // example: perf-mon transfer incomplete
    if (global_conf.transfer_incompleted) {
        rc = vtss_icfg_printf(result, "%s %s %s\n",
                              VTSS_PM_TEXT,
                              VTSS_PM_TRANSFER_TEXT,
                              VTSS_PM_TRANSFER_INCOMPLETE_TEXT);
    }

    return rc;
}

/*
******************************************************************************

    Public functions

******************************************************************************
*/

/* Initialization function */
vtss_rc vtss_perf_mon_icfg_init(void)
{
    vtss_rc rc;

    /* Register callback functions to ICFG module.
       The configuration divided into two groups for this module.
       1. Global configuration
       2. Port configuration
    */
    if ((rc = vtss_icfg_query_register(VTSS_ICFG_PERF_MON_GLOBAL_CONF, "perf-mon", PERF_MON_ICFG_global_conf)) != VTSS_OK) {
        return rc;
    }

    return rc;
}
