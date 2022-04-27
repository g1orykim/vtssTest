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

#include "vtss_ptp_local_clock.h"
#include "vtss_ptp_os.h"
#include "ptp_servo.h"
#include "ptp.h"
#include "vtss_tod_api.h"

#define VTSS_ALLOC_MODULE_ID VTSS_MODULE_ID_PTP

/*
 * Delay filter alforithm:
 * Average the two latest measurements, and do lowpass filtering
 */

typedef struct {
    Integer64  nsec_prev, y;
    Octet cnt;  /* = 0 if Nsec_prev is empty */
} offset_from_master_filter;

typedef struct one_way_delay_filter {
    Integer64  nsec_prev, y;
    Integer64  s_exp;
    Integer32 skipped;
} one_way_delay_filter;


/**
 * \brief Clock Servo Data Set structure
 */
typedef struct ptp_clock_servo_ds_t {
    const ptp_clock_servo_con_ds_t *con;
    offset_from_master_filter  ofm_filt;
} ptp_clock_servo_ds_t;

/**
 * \brief Delay Filter Data Set structure
 */
typedef struct ptp_delay_filter_ds_t {
    ptp_clock_servo_con_ds_t *con;
    one_way_delay_filter *owd_filt;
} ptp_delay_filter_ds_t;




static void my_offset_filter_reset(vtss_ptp_offset_filter_handle_t servo)
{
    T_NG(_S, "initClock");
    ptp_clock_servo_ds_t *s = servo->private_data;

    /* level clock */
    s->ofm_filt.nsec_prev = 0;
    s->ofm_filt.y = 0;
    s->ofm_filt.cnt = 0;
}

static void my_delay_filter_reset(vtss_ptp_delay_filter_handle_t df, int port)
{
    ptp_delay_filter_ds_t *filter = df->private_data;

    filter->owd_filt[port].nsec_prev = 0;
    filter->owd_filt[port].s_exp = 0;
    filter->owd_filt[port].y = 0;
    filter->owd_filt[port].skipped = 0;
}

static int my_delay_filter(vtss_ptp_delay_filter_handle_t df, vtss_ptp_delay_filter_param_t *delay, int port)
{
    Integer16 s;
    ptp_delay_filter_ds_t *filter = df->private_data;
    one_way_delay_filter *owd_filt = &filter->owd_filt[port];
    T_DG(_S, "delay_filter");

    if (VTSS_INTERVAL_SEC(*delay)) {
        /* cannot filter with secs, clear filter */
        owd_filt->s_exp = owd_filt->nsec_prev = 0;
        return 0;
    }
    /* avoid overflowing filter */
    s =  filter->con->delay_filter;
    while (VTSS_LLABS(owd_filt->y)>>(63-s)) {
        --s;
    }
    /* crank down filter cutoff by increasing 's_exp' */
    if (owd_filt->s_exp < 1)
        owd_filt->s_exp = 1;
    else if (owd_filt->s_exp < 1LL<<s)
        ++owd_filt->s_exp;
    else if (owd_filt->s_exp > 1LL<<s)
        owd_filt->s_exp = 1LL<<s;

    T_NG(_S, "delay before filtering %d", VTSS_INTERVAL_NS(*delay));
    T_NG(_S, "OWD filt: nsec_prev %d, s_exp %lld, y %d", VTSS_INTERVAL_NS(owd_filt->nsec_prev),
         owd_filt->s_exp,
         VTSS_INTERVAL_NS(owd_filt->y));

    /* filter 'one_way_delay' */
    if (owd_filt->nsec_prev == 0) owd_filt->nsec_prev = *delay; //first time
    owd_filt->y = (owd_filt->s_exp-1)*owd_filt->y/owd_filt->s_exp +
                  (*delay/2 + owd_filt->nsec_prev/2)/owd_filt->s_exp;

    owd_filt->nsec_prev = *delay;
    *delay = owd_filt->y;
    T_NG(_S, "delay after filtering %d", VTSS_INTERVAL_NS(*delay));
    return 1;
}

static int my_offset_filter(vtss_ptp_offset_filter_handle_t filter, vtss_ptp_offset_filter_param_t *offset, Integer8 logMsgInterval )
{
    char str [40];
    ptp_clock_servo_ds_t *servo = filter->private_data;

    /* filter 'offset_from_master' */
    if (servo->ofm_filt.cnt == 0) {
        servo->ofm_filt.nsec_prev = offset->offsetFromMaster;
        ++servo->ofm_filt.cnt;
    }

    servo->ofm_filt.y = offset->offsetFromMaster/2 + servo->ofm_filt.nsec_prev/2;
    servo->ofm_filt.nsec_prev = offset->offsetFromMaster;
    offset->offsetFromMaster = servo->ofm_filt.y;

    T_RG(_S, "offset filter %s, logMsgInterval %d", vtss_tod_TimeInterval_To_String (&servo->ofm_filt.y, str,0), logMsgInterval);
    return 1;
}

static Integer32 my_clock_servo(vtss_ptp_offset_filter_handle_t filter, const vtss_ptp_offset_filter_param_t *offset, Integer32 *observedParentClockPhaseChangeRate, int localClockId, int phase_lock)
{
    Integer32 adj = 0;
    Integer64 time_offset;
    T_RG(_S, "clock_servo");
    ptp_clock_servo_ds_t *servo = filter->private_data;

    time_offset = (offset->offsetFromMaster)/(1<<16);
    time_offset = time_offset/(1000000*ECOS_MSECS_PER_HWTICK);
    if (time_offset)
    {
        T_IG(_S, "offsetfromMaster %d ns, time_offset %lld timer ticks",VTSS_INTERVAL_NS(offset->offsetFromMaster), time_offset);
        /* if secs, reset clock or set freq adjustment to max */
        if (!servo->con->no_adjust) {
#if (DEFAULT_NO_RESET_CLOCK == FALSE)
            {
                *observedParentClockPhaseChangeRate = 0;  /* clears clock servo accumulator (the I term) */
                servo->offset_filter_reset(servo); //offsetFilterInit(servo);
                ptp_adjust_timeofday(-time_offset);
            }
#else
            {
                adj = VTSS_INTERVAL_NS(offset->offsetFromMaster) > 0 ? ADJ_FREQ_MAX : -ADJ_FREQ_MAX;
            }
#endif
        }
    } else {
        /* the PI controller */

        /* the accumulator for the I component */
        *observedParentClockPhaseChangeRate += VTSS_INTERVAL_NS(offset->offsetFromMaster)/servo->con->ai;

        /* clamp the accumulator to ADJ_FREQ_MAX for sanity */
        if (*observedParentClockPhaseChangeRate > ADJ_FREQ_MAX)
            *observedParentClockPhaseChangeRate = ADJ_FREQ_MAX;
        else if (*observedParentClockPhaseChangeRate < -ADJ_FREQ_MAX)
            *observedParentClockPhaseChangeRate = -ADJ_FREQ_MAX;

        adj = VTSS_INTERVAL_NS(offset->offsetFromMaster)/servo->con->ap + *observedParentClockPhaseChangeRate;

    }
    /* apply controller output as a clock tick rate adjustment */
    if (!servo->con->no_adjust)
        vtss_local_clock_ratio_set(-adj*10, localClockId);
    return adj;
}

static BOOL my_display_stats(vtss_ptp_offset_filter_handle_t filter)
{
    ptp_clock_servo_ds_t *servo = filter->private_data;
    return servo->con->display_stats;
}

vtss_ptp_offset_filter_handle_t ptp_servo_create(const ptp_clock_servo_con_ds_t *c)
{
    ptp_clock_servo_ds_t *servo;
    vtss_ptp_offset_filter_handle_t h = VTSS_MALLOC(sizeof(vtss_ptp_offset_filter_t) + sizeof(ptp_clock_servo_ds_t));
    if (h) {
        h->offset_filter_reset = my_offset_filter_reset;
        h->offset_filter = my_offset_filter;
        h->clock_servo = my_clock_servo;
        h->display_stats = my_display_stats;
        h->private_data = (uchar *)h + sizeof(vtss_ptp_offset_filter_t);
        servo = h->private_data;
        servo->con = c;
        // initialiser private data
        servo->ofm_filt.nsec_prev = 0;
        servo->ofm_filt.y = 0;
        servo->ofm_filt.cnt = 0;
        T_WG(_S, "servo->con: ap = %d, ai = %d", servo->con->ap, servo->con->ai);
    }
    return h;
}


vtss_ptp_delay_filter_handle_t ptp_delay_filter_create(ptp_clock_servo_con_ds_t *c, int port_count)
{
    ptp_delay_filter_ds_t *df;
    vtss_ptp_delay_filter_handle_t s = VTSS_MALLOC(sizeof(vtss_ptp_delay_filter_t) + sizeof(ptp_delay_filter_ds_t));
    if (s) {
        s->delay_filter_reset = my_delay_filter_reset;
        s->delay_filter = my_delay_filter;
        s->private_data = (uchar *)s + sizeof(vtss_ptp_delay_filter_t);
        df = s->private_data;
        df->owd_filt = (one_way_delay_filter*)VTSS_CALLOC(port_count+1, sizeof(one_way_delay_filter));
        df->con = c;
        T_WG(_S, "df->con: delay_filter = %d", df->con->delay_filter);
    }
    return s;
}

void ptp_servo_delete(vtss_ptp_offset_filter_handle_t servo)
{
    if (servo) VTSS_FREE(servo);
}

void ptp_delay_filter_delete(vtss_ptp_delay_filter_handle_t delay)
{
    if (delay) {
        ptp_delay_filter_ds_t *df = delay->private_data;
        if (df->owd_filt) VTSS_FREE(df->owd_filt);
        VTSS_FREE(delay);
    }
}



