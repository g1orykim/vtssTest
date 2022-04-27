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

#ifndef _PSEC_RATE_LIMIT_H_
#define _PSEC_RATE_LIMIT_H_

#include <vtss_api.h> /* For u8, u16, u32, u64, vtss_vid_mac_t, etc */
#include <port_api.h> /* For VTSS_PORTS */

#define PSEC_RATE_LIMIT_FILTER_CNT 10 /**< The number of memorized historical MAC addresses */

/**
 * \brief Rate limiter configuration
 */
typedef struct {
    u32 fill_level_min; /**< Hysteresis: After the burst capacity has been reached, do not allow new frames until it reaches this amount.   */
    u32 fill_level_max; /**< Burst capacity. At most this amount of frames in a burst.                                                      */
    u32 rate;           /**< At most this amount of frames per second over time (better to pick a power-of-two for this number).            */
    u64 drop_age_ms;    /**< Drop the frame if it is less than this amount of milliseconds since last frame with this MAC address was seen. */
} psec_rate_limit_cfg_t;

/**
 * \brief Rate limiter statistics
 */
typedef struct {
    u64 forward_cnt;     /**< Number of not-dropped packets.                  */
    u64 drop_cnt;        /**< Number of dropped packets due to the shaper     */
    u64 filter_drop_cnt; /**< Number of dropped packets due to the MAC filter */
} psec_rate_limit_stat_t;

/**
 * \brief Check to see if we should drop this frame.
 *
 * Check to see if we should drop this frame from a rate-limiter perspective.
 *
 * - Reentrant: No. Always call it from the same thread (e.g. Packet Rx thread).
 *
 * \param port    [IN]: Port on which the frame was received.
 * \param vid_mac [IN]: Frame's MAC address and classified VID.
 *
 * \return TRUE if frame should be dropped.\n
 *         FALSE if it's OK - from the rate-limiter's perspective - to send the frame to the master.
 */
BOOL psec_rate_limit_drop(vtss_port_no_t port, vtss_vid_mac_t *vid_mac);

/**
 * \brief Configure shaper - Debug only
 *
 * \param cfg [IN]: The rate limiter configuration to use from now on.
 */
void psec_rate_limit_cfg_set(psec_rate_limit_cfg_t *cfg);

/**
 * \brief Get current shaper configuration - Debug only
 *
 * \param cfg [OUT]: Pointer to structure receiving current rate limiter configuration.
 */
void psec_rate_limit_cfg_get(psec_rate_limit_cfg_t *cfg);

/**
 * \brief Read statistics counters - Debug only
 *
 * \param stat              [OUT]: Pointer to structure receiving the current rate limiter statistics.
 * \param shaper_fill_level [OUT]: Pointer to u64 receiving the current shaper fill-level
 */
void psec_rate_limit_stat_get(psec_rate_limit_stat_t *stat, u64 *shaper_fill_level);

/**
 * \brief Clear statistics counters - Debug only
 */
void psec_rate_limit_stat_clr(void);

/**
 * \brief Clear current filter for \@port.
 *
 * Use VTSS_PORTS + VTSS_PORT_NO_START for \@port, if you
 * wish to clear the whole filter.
 */
void psec_rate_limit_filter_clr(vtss_port_no_t port);

/**
 * \brief Initialize rate-limiter
 *
 * Expected to be called only once.
 */
void psec_rate_limit_init(void);

#endif /* _PSEC_RATE_LIMIT_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
