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

#ifndef _VTSS_PERF_MON_H_
#define _VTSS_PERF_MON_H_

#include "perf_mon_api.h"

/* ================================================================= *
 *  Trace definitions
 * ================================================================= */

#include "vtss_module_id.h"

#include <vtss_module_id.h>
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_PERF_MON

#define VTSS_TRACE_GRP_DEFAULT      0
#define TRACE_GRP_CRIT              1
#define TRACE_GRP_CNT               2

#include <vtss_trace_api.h>

/* ================================================================= *
 *  PERF_MON configuration blocks
 * ================================================================= */

#define PM_CURRENT_VERSION          2

/* ================================================================= *
 *  PERF_MON global structure
 * ================================================================= */

/* PERF_MON time tick structure */
typedef struct {
    u32 lm_tick;                        /* the tick for Loss Measurement Interval */
    u32 dm_tick;                        /* the tick for Delay Measurement Interval */
    u32 evc_tick;                       /* the tick for EVC Measurement Interval */
    u32 ece_tick;                       /* the tick for ECE Measurement Interval */
} perf_mon_time_tick_t;

/* PERF_MON lm data set */
typedef struct {
    vtss_perf_mon_lm_info_t             lm_data_set[PM_MEASUREMENT_INTERVAL_LIMIT][PM_LM_DATA_SET_LIMIT];
    vtss_perf_mon_measurement_info_t    lm_minfo[PM_MEASUREMENT_INTERVAL_LIMIT];
    u32                                 lm_current_interval_id;
    u32                                 lm_current_interval_cnt;
} perf_mon_lm_conf_t;

/* PERF_MON dm data set */
typedef struct {
    vtss_perf_mon_dm_info_t             dm_data_set[PM_MEASUREMENT_INTERVAL_LIMIT][PM_DM_DATA_SET_LIMIT];
    vtss_perf_mon_measurement_info_t    dm_minfo[PM_MEASUREMENT_INTERVAL_LIMIT];
    u32                                 dm_current_interval_id;
    u32                                 dm_current_interval_cnt;
} perf_mon_dm_conf_t;

/* PERF_MON evc data set */
typedef struct {
    vtss_perf_mon_evc_info_t            evc_data_set[PM_MEASUREMENT_INTERVAL_LIMIT][PM_EVC_DATA_SET_LIMIT];
    vtss_perf_mon_measurement_info_t    evc_minfo[PM_MEASUREMENT_INTERVAL_LIMIT];
    u32                                 evc_current_interval_id;
    u32                                 evc_current_interval_cnt;
} perf_mon_evc_conf_t;

/* PERF_MON ece data set */
typedef struct {
    vtss_perf_mon_evc_info_t            ece_data_set[PM_MEASUREMENT_INTERVAL_LIMIT][PM_ECE_DATA_SET_LIMIT];
    vtss_perf_mon_measurement_info_t    ece_minfo[PM_MEASUREMENT_INTERVAL_LIMIT];
    u32                                 ece_current_interval_id;
    u32                                 ece_current_interval_cnt;
} perf_mon_ece_conf_t;

/* PERF_MON lm report block */
typedef struct {
    unsigned long               version;                /* Block version */
    perf_mon_lm_conf_t          lm_reports;             /* LM reports */
} perf_mon_lm_conf_blk_t;

/* PERF_MON dm report block */
typedef struct {
    unsigned long               version;                /* Block version */
    perf_mon_dm_conf_t          dm_reports;             /* DM reports */
} perf_mon_dm_conf_blk_t;

/* PERF_MON evc report block */
typedef struct {
    unsigned long               version;                /* Block version */
    perf_mon_evc_conf_t         evc_reports;            /* EVC reports */
} perf_mon_evc_conf_blk_t;

/* PERF_MON ece report block */
typedef struct {
    unsigned long               version;                /* Block version */
    perf_mon_ece_conf_t         ece_reports;            /* ECE reports */
} perf_mon_ece_conf_blk_t;

/* PERF_MON global structure */
typedef struct {
    critd_t                         crit;
    perf_mon_conf_t                 perf_mon_conf;
    perf_mon_time_tick_t            pm_time;
    perf_mon_lm_conf_t              lm;
    perf_mon_dm_conf_t              dm;
    perf_mon_evc_conf_t             evc;
    perf_mon_ece_conf_t             ece;
} perf_mon_global_t;

#endif /* _VTSS_PERF_MON_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
