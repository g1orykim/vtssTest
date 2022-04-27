/*

 Vitesse Switch Software.

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

#include "vtss_ptp_wl_delay_filter.h"
#include "vtss_ptp_os.h"

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_PTP

typedef struct one_way_wl_delay_filter {
    vtss_timeinterval_t act_min_delay;
    vtss_timeinterval_t act_max_delay;
    vtss_timeinterval_t act_mean_delay;
    vtss_timeinterval_t prev_delay;
    UInteger32 prev_cnt;
    UInteger32 actual_period;
    UInteger32 actual_dist;
} one_way_wl_delay_filter;

/**
 * \brief Delay Filter private data
 */
typedef struct vtss_ptp_wl_delay_filterprivate_data_t {

    const vtss_ptp_default_filter_config_t *filt_conf;
    one_way_wl_delay_filter *owd_filt;
} vtss_ptp_wl_delay_filterprivate_data_t;


/**
 * \brief Filter reset function.
 */
static void my_delay_filter_reset(vtss_ptp_delay_filter_handle_t df, int port)
{
    vtss_ptp_wl_delay_filterprivate_data_t *filter = df->private_data;

    if (filter->filt_conf->dist > 1) {
        filter->owd_filt[port].act_min_delay = VTSS_MAX_TIMEINTERVAL;
    } else {
        filter->owd_filt[port].act_min_delay = 0;
    }
    filter->owd_filt[port].actual_period = 0;
    filter->owd_filt[port].prev_delay = 0;
    filter->owd_filt[port].prev_cnt = 0;
    
    
}

/**
 * \brief Filter execution function.
 */
static int my_delay_filter(vtss_ptp_delay_filter_handle_t df, vtss_ptp_delay_filter_param_t *delay, int port)
{
    vtss_ptp_wl_delay_filterprivate_data_t *filter = df->private_data;
    one_way_wl_delay_filter *owd_filt = &filter->owd_filt[port];
    T_D("delay_filter (port %d)", port);

    if (VTSS_INTERVAL_SEC(*delay)) {
        /* cannot filter with secs, clear filter */
        my_delay_filter_reset(df, port);
        return 0;
    }
    if (*delay < 0LL) { /*cannot handle negative delays */
        *delay = 0LL;
    }
    if (owd_filt->actual_period == 0) {
        owd_filt->act_min_delay = VTSS_MAX_TIMEINTERVAL;
        owd_filt->act_max_delay = -VTSS_MAX_TIMEINTERVAL;
        owd_filt->act_mean_delay = 0;
    }
    if (owd_filt->act_min_delay > *delay) {
        owd_filt->act_min_delay = *delay;
    }
    if (owd_filt->act_max_delay < *delay) {
        owd_filt->act_max_delay = *delay;
    }
    owd_filt->act_mean_delay +=*delay;
    if (owd_filt->prev_cnt == 0) {
        owd_filt->prev_delay = *delay;
    }
    if (++owd_filt->prev_cnt > filter->filt_conf->dist) {
        owd_filt->prev_cnt = filter->filt_conf->dist;
    }
    if (filter->filt_conf->dist > 1) {
        /* min delay algorithm */
        if (++owd_filt->actual_period >= filter->filt_conf->period) {
            owd_filt->act_mean_delay = owd_filt->act_mean_delay/filter->filt_conf->period;
            *delay = (owd_filt->prev_delay*(owd_filt->prev_cnt-1) + owd_filt->act_min_delay)/(owd_filt->prev_cnt);
            owd_filt->prev_delay = *delay;
            owd_filt->actual_period = 0;
            T_I("delayfilter, min %d ns, max %d ns, delay %d ns, prev_cnt %d",
                    VTSS_INTERVAL_NS(owd_filt->act_min_delay), VTSS_INTERVAL_NS(owd_filt->act_max_delay), 
                    VTSS_INTERVAL_NS(*delay), owd_filt->prev_cnt);
            return 1;
        }
    } else {
        /* mean delay algorithm */
        if (++owd_filt->actual_period >= filter->filt_conf->period) {
            owd_filt->act_mean_delay = owd_filt->act_mean_delay/filter->filt_conf->period;
            *delay = owd_filt->act_mean_delay;
            owd_filt->actual_period = 0;
            T_I("delayfilter, min %d ns, max %d ns, delay %d ns",
                VTSS_INTERVAL_NS(owd_filt->act_min_delay), VTSS_INTERVAL_NS(owd_filt->act_max_delay), 
                VTSS_INTERVAL_NS(*delay));
            return 1;
        }
    }
    *delay = owd_filt->prev_delay;
    return 1;
}

/**
 * \brief Create a Default PTP filter instance.
 * Create an instance of the default vtss_ptp filter
 *
 * \param of [IN]  pointer to a structure containing the default parameters for
 *                the delay filter
 * \param s [IN]  pointer to a structure containing the default parameters for
 *                the servo
 * \return (opaque) instance data reference or NULL.
 */
vtss_ptp_delay_filter_handle_t vtss_ptp_wl_delay_filter_create(
    const vtss_ptp_default_filter_config_t *of, int port_count)
{
    vtss_ptp_wl_delay_filterprivate_data_t *filter;
    vtss_ptp_delay_filter_handle_t df = VTSS_MALLOC(sizeof(vtss_ptp_delay_filter_t) + sizeof(vtss_ptp_wl_delay_filterprivate_data_t));
    if (df) {
        df->delay_filter_reset = my_delay_filter_reset;
        df->delay_filter = my_delay_filter;
        df->private_data = (uchar *)df + sizeof(vtss_ptp_delay_filter_t);
        filter = df->private_data;
        filter->owd_filt = (one_way_wl_delay_filter*)VTSS_CALLOC(port_count+1, sizeof(one_way_wl_delay_filter));
        filter->filt_conf = of;
        T_R("filter->con: period = %d dist = %d", filter->filt_conf->period, filter->filt_conf->dist);
    }
    return df;
}

/**
 * \brief Delete a Default PTP filter instance.
 * Delete an instance of the default vtss_ptp filter
 *
 * \param filter [IN]  instance data reference
 * \return  None
 */
void vtss_ptp_wl_delay_filter_delete(vtss_ptp_delay_filter_handle_t df)
{
    if (df) {
        vtss_ptp_wl_delay_filterprivate_data_t *filter = df->private_data;
        if (filter->owd_filt) VTSS_FREE(filter->owd_filt);
        VTSS_FREE(df);
    }
}

