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

#ifndef _VTSS_PERF_MON_ICFG_H_
#define _VTSS_PERF_MON_ICFG_H_

/*
******************************************************************************

    Include files

******************************************************************************
*/

/*
******************************************************************************

    Constant and Macro definition

******************************************************************************
*/
#define VTSS_PM_TEXT                                "perf-mon"

#define VTSS_PM_SESSION_TEXT                        "session"
#define VTSS_PM_STORAGE_TEXT                        "storage"
#define VTSS_PM_INTERVAL_TEXT                       "interval"
#define VTSS_PM_TRANSFER_TEXT                       "transfer"

#define VTSS_PM_TRANSFER_HOURS_TEXT                 "hour"
#define VTSS_PM_TRANSFER_MINUTE_TEXT                "minute"
#define VTSS_PM_TRANSFER_URL_TEXT                   "url"
#define VTSS_PM_TRANSFER_INTERVAL_MODE_TEXT         "mode"
#define VTSS_PM_TRANSFER_RANDOM_TEXT                "random-offset"
#define VTSS_PM_TRANSFER_FIXED_TEXT                 "fixed-offset"
#define VTSS_PM_TRANSFER_INCOMPLETE_TEXT            "incomplete"

#define VTSS_PM_TRANSFER_INTERVAL_MODE_ALL_TEXT     "all"
#define VTSS_PM_TRANSFER_INTERVAL_MODE_NEW_TEXT     "new"
#define VTSS_PM_TRANSFER_INTERVAL_MODE_FIXED_TEXT   "fixed"

#define VTSS_PM_SESSION_LM_TEXT                     "lm"
#define VTSS_PM_SESSION_DM_TEXT                     "dm"
#define VTSS_PM_SESSION_EVC_TEXT                    "evc"
#define VTSS_PM_SESSION_ECE_TEXT                    "ece"
#define VTSS_PM_STORAGE_LM_TEXT                     "lm"
#define VTSS_PM_STORAGE_DM_TEXT                     "dm"
#define VTSS_PM_STORAGE_EVC_TEXT                    "evc"
#define VTSS_PM_STORAGE_ECE_TEXT                    "ece"

/*
******************************************************************************

    Data structure type definition

******************************************************************************
*/

/**
 * \file perf_mon_icfg.h
 * \brief This file defines the interface to the Performance Monitor module's ICFG commands.
 */

/**
  * \brief Initialization function.
  *
  * Call once, preferably from the INIT_CMD_INIT section of
  * the module's _init() function.
  */
vtss_rc vtss_perf_mon_icfg_init(void);

#endif /* _VTSS_PERF_MON_ICFG_H_ */
