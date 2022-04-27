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

/******************************************************************************/
//
// Rate limiting filter needed to prevent DoS attacks, since Luton28 rate-
// limiter towards the CPU doesn't work.
//
/******************************************************************************/

#include "vtss_trace_lvl_api.h" /* For T_x() trace macros */
#include "main.h"               /* For ARRSZ()            */
#include "psec_rate_limit.h"    /* Ourselves              */

// Piggy-back on PSEC's trace system
#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_PSEC

/******************************************************************************/
//
// NOTE: ALL MUTUAL EXCLUSION MUST BE TAKEN CARE OF BY THE USER OF THIS API
//
/******************************************************************************/

// We need to shape the learn frames against the master for the following reason:
// If any slave could send an unlimited amount of learn frames to the master,
// the message queue on the master would queue them up, and send them to the
// DOT1X module one by one. The message queue is of unlimited size, so if the
// DOT1X module can't keep up the pace, it would just grow and grow until
// the DOT1X module finds out that all state machines are in use, or it's
// out of RADIUS identifiers. After this, it would send a message to the slave
// and tell it to stop copying learn frames to the CPU. The bad thing at this
// point is that if the MAC module uses the message protocol to tell this,
// then a real slave would get the message quite fast, but if it is sent
// to the master itself, it would be looped back and end up at the back of
// the message queue, and not get processed until all the learn frames that
// the message module has received thus far are processed. Therefore, it is
// of importance that the MAC module doesn't use the message protocol to send
// such requests to itself, but shortcut the message protocol and write it
// directly - when it's master and the message is also for the master.
// The shaper is implemented as a leaky bucket with some burst capacity and
// an emptying at a steady pace. Since the DOT1X_thread() is suspended when
// we're a slave and the shaper still needs to work, we can't use the thread
// to empty the leaky bucket, so we'll have to do it based on the current
// uptime whenever we're about to judge wether to drop the learn frame or
// forward it to the master. For that we need a current fill level, a burst
// capacity (constant), an emptying rate (constant) and the time of the last
// update of the bucket.
// One could ask: Why not use the learn frame policer, which is already
// enabled, when multi-clients are supported? The reason is that
// this policer affects wire-speed learning, and doesn't protect when a
// given MAC address has already been learned, and it is going to be aged.
static u64              PSEC_shaper_fill_level;
static cyg_tick_count_t PSEC_shaper_uptime_of_last_update_ms;
static BOOL             PSEC_shaper_burst_capacity_reached;

// And then the statistics we keep track of in here
static psec_rate_limit_stat_t PSEC_shaper_stat;

// Shaper configuration. This can be changed by a debug command.
static psec_rate_limit_cfg_t PSEC_cfg;

/******************************************************************************/
// List of last seen MAC addresses on this switch.
/******************************************************************************/
typedef struct {
    cyg_tick_count_t last_transmission_time_ms;
    vtss_vid_mac_t   vid_mac;
    vtss_port_no_t   port;
    BOOL             in_use;
} psec_rate_limit_filter_t;

static psec_rate_limit_filter_t PSEC_rate_limit_filter[PSEC_RATE_LIMIT_FILTER_CNT];

/******************************************************************************/
// PSEC_rate_limit_filter_drop()
/******************************************************************************/
static inline BOOL PSEC_rate_limit_filter_drop(vtss_port_no_t port, vtss_vid_mac_t *vid_mac, cyg_tick_count_t now_ms)
{
    // Scenario: Suppose two clients are attached to the same switch, and one is
    // transmitting a lot of data, while the other (the friendly) is transmitting
    // only a little bit, and suppose we're currently software-aging both clients.
    // Both users' data would end up in the same extraction queue, but if the one
    // sending a lot of data saturates the shaper, the one sending not so much data
    // would not get a chance to get its frames through to the master. Therefore, we
    // need to build a filter on top of the shaper, so that a given user cannot
    // send frames to the master unless it hasn't sent anything the last, say,
    // 3 seconds. We can't have an array that is infinitely long, so we just save
    // the last X MAC addresses.
    psec_rate_limit_filter_t *mf, *temp_mf = &PSEC_rate_limit_filter[0];
    cyg_tick_count_t         time_of_oldest = PSEC_rate_limit_filter[0].last_transmission_time_ms;
    int                      i;

    // In case of a stack-topology change or configuration change, the mac filter gets cleared.
    for (i = 0; i < (int)ARRSZ(PSEC_rate_limit_filter); i++) {
        mf = &PSEC_rate_limit_filter[i];
        if (mf->in_use && memcmp(&mf->vid_mac, vid_mac, sizeof(mf->vid_mac)) == 0 && mf->port == port) {
            // Received this MAC address before. Check to see if we should drop it.
            if (now_ms - mf->last_transmission_time_ms >= PSEC_cfg.drop_age_ms) {
                mf->last_transmission_time_ms = now_ms;
                return FALSE; // Don't drop it.
            } else {
                return TRUE; // Drop it. It's not old enough
            }
        }

        if (mf->last_transmission_time_ms < time_of_oldest) {
            temp_mf        = mf;
            time_of_oldest = mf->last_transmission_time_ms;
        }
    }

    // New MAC address. Save it by overwriting the one that has been longest in the array
    temp_mf->in_use                    = TRUE; // Can never get cleared.
    temp_mf->last_transmission_time_ms = now_ms;
    temp_mf->vid_mac                   = *vid_mac;
    temp_mf->port                      = port;
    temp_mf->in_use                    = TRUE;
    return FALSE; // Don't drop it.
}

/******************************************************************************/
// psec_rate_limit_drop()
// Check to see if we should drop this frame.
/******************************************************************************/
BOOL psec_rate_limit_drop(vtss_port_no_t port, vtss_vid_mac_t *vid_mac)
{
    // This function is not re-entrant, and since it's only called from the Packet Rx
    // thread, it's ensured that it's not called twice.

    // NB:
    // Due to the granularity of an eCos tick (10 ms), all fill-levels are multiplied
    // by 1024, so that we count up and down more accurately.
    // If we didn't the fill-level-subtraction would occur in too high steps, or not
    // at all if two frames arrived within the same eCos tick.

    // Update the shaper's current fill level
    // @now_ms is the current uptime measured in milliseconds.
    cyg_tick_count_t now_ms = cyg_current_time() * ECOS_MSECS_PER_HWTICK;

    // @diff_ms is the amount of time elapsed since the last call of this
    // function (notice that we don't take into account if the stack topology
    // changes, which is fine, I think).
    cyg_tick_count_t diff_ms = now_ms - PSEC_shaper_uptime_of_last_update_ms;

    // In reality we should have divided the following by 1000 and multiplied by the
    // fill-level granularity (1024), but to speed things up, we don't do this, which
    // causes the emptying of the leaky bucket to happen 1 - 1000/1024 = 2.3% slower.
    // Stay in the 64-bit world (cyg_tick_count_t) when doing these computations.
    u64 subtract_from_fill_level = (diff_ms * PSEC_cfg.rate);

    // Update the shaper's current fill-level
    if (subtract_from_fill_level >= PSEC_shaper_fill_level) {
        PSEC_shaper_fill_level = 0;
    } else {
        PSEC_shaper_fill_level -= (ulong)subtract_from_fill_level;
    }

    PSEC_shaper_uptime_of_last_update_ms = now_ms;

    // Hysteresis: If the shaper has hit it's maximum, wait until it's below it's minimum
    // before allowing frames again.
    // The 1024 below is due to the fill-level granularity.
    if ((PSEC_shaper_burst_capacity_reached && PSEC_shaper_fill_level > (PSEC_cfg.fill_level_min * 1024ULL)) || (PSEC_shaper_fill_level >= (PSEC_cfg.fill_level_max * 1024ULL))) {
        if (PSEC_shaper_burst_capacity_reached == FALSE) {
            T_I("Turning on shaper");
        }
        // We could've stopped all traffic from multi-client ports here, but I think it's better
        // to leave that decision to the master, also because we have no way to trigger the
        // port(s) to start again, since we're only called when new frames arrive.
        PSEC_shaper_burst_capacity_reached = TRUE;
        PSEC_shaper_stat.drop_cnt++;
        return TRUE;
    }

    if (PSEC_shaper_burst_capacity_reached == TRUE) {
        T_I("Turning off shaper");
    }

    // Check to see if we need to filter this one out.
    if (PSEC_rate_limit_filter_drop(port, vid_mac, now_ms)) {
        // We do. Don't count it in the shaper.
        PSEC_shaper_stat.filter_drop_cnt++;
        return TRUE;
    }

    // Fill-level granularity is 1024 per frame.
    PSEC_shaper_fill_level             += 1024;
    PSEC_shaper_burst_capacity_reached  = FALSE;
    PSEC_shaper_stat.forward_cnt++;

    return FALSE; // Don't drop the frame - forward it to the master.
}

/******************************************************************************/
// psec_rate_limit_cfg_set()
/******************************************************************************/
void psec_rate_limit_cfg_set(psec_rate_limit_cfg_t *cfg)
{
    // Mutual exclusion is taken care of by the user of this API.
    // The same goes for valid ranges.
    PSEC_cfg = *cfg;
}

/******************************************************************************/
// psec_rate_limit_cfg_get()
/******************************************************************************/
void psec_rate_limit_cfg_get(psec_rate_limit_cfg_t *cfg)
{
    // Mutual exclusion is taken care of by the user of this API.
    // The same goes for valid ranges.
    *cfg = PSEC_cfg;
}

/******************************************************************************/
// psec_rate_limit_stat_get()
/******************************************************************************/
void psec_rate_limit_stat_get(psec_rate_limit_stat_t *stat, u64 *shaper_fill_level)
{
    *stat              = PSEC_shaper_stat;
    *shaper_fill_level = PSEC_shaper_fill_level / 1024;
}

/******************************************************************************/
// psec_rate_limit_stat_clr()
/******************************************************************************/
void psec_rate_limit_stat_clr(void)
{
    // Mutual exclusion is taken care of by the user of this API.
    memset(&PSEC_shaper_stat, 0, sizeof(PSEC_shaper_stat));
}

/******************************************************************************/
// psec_rate_limit_filter_clr()
// Set port to VTSS_PORTS + VTSS_PORT_NO_START to clear the whole filter.
/******************************************************************************/
void psec_rate_limit_filter_clr(vtss_port_no_t port)
{
    int i;

    // Mutual exclusion is taken care of by the user of this API.

    // In case someone changes VTSS_PORT_NO_START back to 1, we better survive that
    // so tell lint to not report "Relational operator '<' always evaluates to 'false'"
    // and "non-negative quantity is never less than zero".
    /*lint -e{685, 568} */
    if (port < VTSS_PORT_NO_START || port >= VTSS_PORTS + VTSS_PORT_NO_START) {
        memset(PSEC_rate_limit_filter, 0, sizeof(PSEC_rate_limit_filter));
    } else {
        for (i = 0; i < (int)ARRSZ(PSEC_rate_limit_filter); i++) {
            psec_rate_limit_filter_t *mf = &PSEC_rate_limit_filter[i];
            if (mf->in_use && mf->port == port) {
                mf->in_use = FALSE;
                mf->last_transmission_time_ms = 0;
            }
        }
    }
}

/******************************************************************************/
// psec_rate_limit_init()
/******************************************************************************/
void psec_rate_limit_init(void)
{
    PSEC_shaper_fill_level               =    0;
    PSEC_shaper_uptime_of_last_update_ms =    0;
    PSEC_shaper_burst_capacity_reached   = FALSE;
    PSEC_cfg.fill_level_max              =  200;    // Burst capacity. At most this amount of frames in a burst
    PSEC_cfg.fill_level_min              =   50;    // Hysteresis: After the burst capacity has been reached, do not allow new frames until it reaches 50
    PSEC_cfg.rate                        =   32;    // At most 32 frames per second over time (better to pick a power-of-two for this number)
    PSEC_cfg.drop_age_ms                 = 3000ULL; // 3 seconds per default.
    psec_rate_limit_stat_clr();
    psec_rate_limit_filter_clr(VTSS_PORTS + VTSS_PORT_NO_START);
}

