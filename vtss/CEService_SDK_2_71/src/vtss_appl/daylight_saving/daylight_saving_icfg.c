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
#include "daylight_saving_api.h"
#include "daylight_saving_icfg.h"
#include "misc_api.h"   //uport2iport(), iport2uport()
#include "icli_porting_util.h"

#define SECSPERMIN          60

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
static vtss_rc DAYLIGHT_SAVING_ICFG_global_conf(const vtss_icfg_query_request_t *req,
                                                vtss_icfg_query_result_t *result)
{
    vtss_rc                             rc = VTSS_OK;
    time_conf_t                         conf;


    if ((rc = time_dst_get_config(&conf)) != VTSS_OK) {
        return rc;
    }

    /* entries */
    // example 1:   clock summer-time <word> recurring [<1-5> <1-7> <1-12> <time> <1-5> <1-7> <1-12> <time> [<1-1440>]]
    // example 2:   clock summer-time <word> date [<1-12> <1-31> <2000-2097> <time> <1-12> <1-31> <2000-2097> <time> [<1-1440>]]
    switch (conf.dst_mode) {
    case TIME_DST_DISABLED:
        break;
    case TIME_DST_RECURRING:
        rc = vtss_icfg_printf(result, "%s %s %s %d %d %d %02d:%02d %d %d %d %02d:%02d %d\n",
                              VTSS_NTP_GLOBAL_MODE_CLOCK_TEXT,
                              strlen(conf.tz_acronym) > 0 ? conf.tz_acronym : "''",
                              VTSS_NTP_GLOBAL_MODE_RECURRING_TEXT,
                              conf.dst_start_time.week,
                              conf.dst_start_time.day,
                              conf.dst_start_time.month,
                              conf.dst_start_time.hour,
                              conf.dst_start_time.minute,
                              conf.dst_end_time.week,
                              conf.dst_end_time.day,
                              conf.dst_end_time.month,
                              conf.dst_end_time.hour,
                              conf.dst_end_time.minute,
                              conf.dst_offset);
        break;
    case TIME_DST_NON_RECURRING:
        rc = vtss_icfg_printf(result, "%s %s %s %d %d %d %02d:%02d %d %d %d %02d:%02d %d\n",
                              VTSS_NTP_GLOBAL_MODE_CLOCK_TEXT,
                              strlen(conf.tz_acronym) > 0 ? conf.tz_acronym : "''",
                              VTSS_NTP_GLOBAL_MODE_NO_RECURRING_TEXT,
                              conf.dst_start_time.month,
                              conf.dst_start_time.date,
                              conf.dst_start_time.year,
                              conf.dst_start_time.hour,
                              conf.dst_start_time.minute,
                              conf.dst_end_time.month,
                              conf.dst_end_time.date,
                              conf.dst_end_time.year,
                              conf.dst_end_time.hour,
                              conf.dst_end_time.minute,
                              conf.dst_offset);
        break;
    }

    /* timezone */
    // example: clock timezone <word> <-23-23> [<0-59>]
    if (conf.tz_offset != 0) {
        if ((conf.tz_offset % SECSPERMIN) != 0) {
            rc = vtss_icfg_printf(result, "%s %s %d %d\n",
                                  VTSS_NTP_GLOBAL_MODE_CLOCK_TIMEZONE_TEXT,
                                  strlen(conf.tz_acronym) > 0 ? conf.tz_acronym : "''",
                                  conf.tz_offset != 0 ? conf.tz_offset / SECSPERMIN : 0,
                                  (conf.tz_offset % SECSPERMIN));
        } else {
            rc = vtss_icfg_printf(result, "%s %s %d\n",
                                  VTSS_NTP_GLOBAL_MODE_CLOCK_TIMEZONE_TEXT,
                                  strlen(conf.tz_acronym) > 0 ? conf.tz_acronym : "''",
                                  conf.tz_offset != 0 ? conf.tz_offset / SECSPERMIN : 0);
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
vtss_rc vtss_daylight_saving_icfg_init(void)
{
    vtss_rc rc;

    /* Register callback functions to ICFG module.
       The configuration divided into two groups for this module.
       1. Global configuration
       2. Port configuration
    */
    if ((rc = vtss_icfg_query_register(VTSS_ICFG_DAYLIGHT_SAVING_GLOBAL_CONF, "clock", DAYLIGHT_SAVING_ICFG_global_conf)) != VTSS_OK) {
        return rc;
    }

    return rc;
}
