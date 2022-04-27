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
/* vtss_ptp_local_clock.h */

#ifndef VTSS_PTP_LOCAL_CLOCK_H
#define VTSS_PTP_LOCAL_CLOCK_H

#include "vtss_ptp_types.h"

#define ADJ_FREQ_MAX  100000 // +/- 100 ppm
#define ADJ_FREQ_MAX_LL  100000LL // +/- 100 ppm
#define ADJ_OFFSET_MAX  10000000LL  // Max offset used in the servo (measured in ns)

#define VTSS_PTP_LOCK_THRESHOLD 1000LL<<16   // Lock threshols is 1000 ns
#define VTSS_PTP_LOCK_PERIODS 5            // Lock periods is 5 periods

/**
 * \brief Get current PTP time.
 *
 * \param t [OUT] The variable to store the time in.
 *
 *
 * The TimeStamp format is defined by IEEE1588.
 */
void vtss_local_clock_time_get(vtss_timestamp_t *t, int instance, u32 *hw_time);

/**
 * \brief Set current PTP time.
 *
 * \param t [IN] The variable holding the time.
 * \param instance [IN]  PTP clock instance no.
 *
 */
void vtss_local_clock_time_set(vtss_timestamp_t *t, int instance);

/**
 * \brief Convert a timestamp from the packet module to PTP time.
 * \param cur_time [IN]  number of timer ticks since startup.
 * \param t        [OUT] The variable to store the time in.
 * \param instance [IN]  PTP clock instance no.
 */
void vtss_local_clock_convert_to_time( UInteger32 cur_time, vtss_timestamp_t *t, int instance);

/**
 * \brief Convert a nanosec value to HW timecounter.
 * \param t [IN] The nanosecond value.
 * \param cur_time [OUT] corresponding HW timecounter.
 * \param instance [IN]  PTP clock instance no.
 */
void vtss_local_clock_convert_to_hw_tc( UInteger32 ns, UInteger32 *cur_time);
/**
 * \brief Get offset between timer ticks and PTP time
 *
 * \return offset in number of timer ticks .
 *
 */
UInteger64 ptp_get_cyg_current_time_offset(void);

/**
 * \brief Adjust the clock timer ratio
 *
 * \param ratio Clock ratio frequency offset in 0,1 ppb (parts pr billion).
 *      ratio > 0 => clock runs faster
 * The function has a buildin slewrate function, that limits the adjustment change to max 1 PPM pr call.
 *
 */
void vtss_local_clock_ratio_set(Integer32 ratio, int instance);

/**
 * \brief Clear the clock timer ratio and the slewrate value
 *
 * \param instance Clock instance number
 *
 */
void vtss_local_clock_ratio_clear(int instance);

/**
 * \brief Adjust the clock timer offset
 *
 * \param offset Clock offset in ns.
 *      offset is subtracted from the actual time
 *
 */
void vtss_local_clock_adj_offset(Integer32 offset, int instance);

/**
 * \brief Get clock adjustment method
 *
 * \param instance Clock instance number.
 * \returns Clock option adjustment method (see values below)
 *
 */
#define CLOCK_OPTION_INTERNAL_TIMER 0
#define CLOCK_OPTION_SYNCE_XO 1
#define CLOCK_OPTION_DAC      2
#define CLOCK_OPTION_SOFTWARE 3


int vtss_ptp_adjustment_method(int instance);

#endif

