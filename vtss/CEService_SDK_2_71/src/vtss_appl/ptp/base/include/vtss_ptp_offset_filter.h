/*

 Vitesse Switch API software.

 Copyright (c) 2002-2012 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _VTSS_PTP_OFFSET_FILTER_H_
#define _VTSS_PTP_OFFSET_FILTER_H_
/**
 * \file vtss_ptp_offset_filter.h
 * \brief Define Offset filter callouts.
 *
 * This file contain the definitions of PTP Offset filter functions and
 * associated types.
 * The functions are indirectly called vis function pointers, defined in 
 * vtss_ptp_offset_filter_t structure, and can easily be exchanged with customer
 * defined functions.
 *
 */

#include "vtss_ptp_types.h"


/**
 * \brief Opaque Offset filter handle.
 */
typedef struct vtss_ptp_offset_filter_t *vtss_ptp_offset_filter_handle_t;

/**
 * \brief Clock Offset filter and servo parameter structure
 */
typedef struct vtss_ptp_offset_filter_param_t {
	vtss_timeinterval_t offsetFromMaster;
	vtss_timestamp_t rcvTime;
} vtss_ptp_offset_filter_param_t;

/**
 * \brief Clock servo status structure
 */
typedef struct vtss_ptp_servo_status_t {
    BOOL holdover_ok;
    i64 holdover_adj;
} vtss_ptp_servo_status_t;

// ***************************************************************************
//
//  General filter definitions.
//
// ***************************************************************************
/**
 * \brief Offset Filter Data structure
 */
typedef struct vtss_ptp_offset_filter_t {
	/**
	 * \brief Filter reset function pointer.
     *
	 * Reset the offset filter
	 *
	 * \param filter [IN]  instance data reference
	 * \return  None
	 */
	void (*offset_filter_reset)(vtss_ptp_offset_filter_handle_t filter);

	/**
	 * \brief Filter execution function pointer.
     *
	 * Execute the offset filter operation (called each time the offset has 
     * been measured)
	 *
	 * \param filter [IN]  instance data reference.
	 * \param offset [IN/OUT]  pointer to offset data
	 *                          IN : measured offset and time
	 *                          OUT: filtered offset and time.
	 * \return  0 : no clockupdate; 1 : do clockupdate
	 */
	int (*offset_filter)(vtss_ptp_offset_filter_handle_t filter, vtss_ptp_offset_filter_param_t *offset, Integer8 logMsgInterval);

	/**
	 * \brief Clock Servo reset function pointer.
     *
	 * Reset the clock servo operation (set the clock into FreeRun or Holdover
	 *
	 * \param filter [IN]  instance data reference.
     * \param localClockId local clock id.
	 * \return  clock adjustment rate in units of 0,1 ppb.
	 */
	void (*clock_servo_reset)(vtss_ptp_offset_filter_handle_t filter, int localClockId);

	/**
	 * \brief Clock Servo execution function pointer.
     *
	 * Execute the clock servo operation (called each time the offset has been
     * updated)
	 *
	 * \param filter [IN]  instance data reference.
	 * \param offset [IN]  pointer to filtered offset data.
	 * \param observedParentClockPhaseChangeRate [OUT] Parent clock rate difference.
     * \param localClockId local clock id.
     * \param phase_lock   0 if try to obtain freq lock, i.e. the integration parameter is omitted
     *                     1 if try to obtain phase lock,
     *                     2 if slave is in locked state, in this mode the holdover frequency can be calculated
	 * \return  clock adjustment rate in units of 0,1 ppb.
	 */
	Integer32 (*clock_servo)(vtss_ptp_offset_filter_handle_t filter, const vtss_ptp_offset_filter_param_t *offset, Integer32 *observedParentClockPhaseChangeRate, int localClockId, int phase_lock);

	/**
	 * \brief Clock Servo status get function pointer.
     *
	 * Get the clock servo status
	 *
	 * \param filter [IN]  instance data reference.
	 * \param status [OUT]  pointer to servo status data.
	 * \return  void.
	 */
	void (*clock_servo_status)(vtss_ptp_offset_filter_handle_t filter, vtss_ptp_servo_status_t *status);
	/**
    * \brief Test if logging is enabled.
    *
    * \return  TRUE if logging is enabled.
    */
	BOOL (*display_stats) (vtss_ptp_offset_filter_handle_t filter);
	/**
     * \brief Log internal servo parameters.
     *
	 * \param filter [IN]  instance data reference.
     * \param buf [IN/OUT]  pointer text buffer.
     * \return  size of text string generated.
     */
	int (*display_parm) (vtss_ptp_offset_filter_handle_t filter, char* buf);
	void *private_data;
} vtss_ptp_offset_filter_t;

// ***************************************************************************
//
//  filter definitions specific for default filter.
//
// ***************************************************************************

/**
 * \brief Default Clock Filter config Data Set structure
 */
typedef struct vtss_ptp_default_filter_config_t {
	u32 period;
	u32 dist;
} vtss_ptp_default_filter_config_t;

/**
 * \brief parameter describing PTP servo option.
 **/
typedef enum {
    VTSS_PTP_CLOCK_FREE,  /**< Oscillator is freerunning */
    VTSS_PTP_CLOCK_SYNCE,  /**< Oscillator is synce locked to master */
} vtss_ptp_srv_clock_option_t;
/**
 * \brief Default Clock Servo config Data Set structure
 */
typedef struct vtss_ptp_default_servo_config_t {
	BOOL display_stats;
	BOOL p_reg;
	BOOL i_reg;
	BOOL d_reg;
	short ap;
	short ai;
	short ad;
    vtss_ptp_srv_clock_option_t srv_option;
    short synce_threshold;
    short synce_ap;
    i32   ho_filter;                            /* Hold off low pass filter constant for calculation of holdover frequency */
    i64   stable_adj_threshold;                 /* The offset is assumed to be stable if the |adj_average - adj| < this value unit: ppb*10*/
} vtss_ptp_default_servo_config_t;


/**
 * \brief Create a Default PTP filter instance.
 *
 * Create an instance of the default vtss_ptp filter
 *
 * \param of [IN]  pointer to a structure containing the default parameters for
 *                the offset filter
 * \param s [IN]  pointer to a structure containing the default parameters for
 *                the servo
 * \return (opaque) instance data reference or NULL.
 */
vtss_ptp_offset_filter_handle_t vtss_ptp_default_filter_create(
    vtss_ptp_default_filter_config_t *of,
    const vtss_ptp_default_servo_config_t *s);

/**
 * \brief Delete a Default PTP filter instance.
 *
 * Delete an instance of the default vtss_ptp filter
 *
 * \param filter [IN]  instance data reference
 * \return  None
 */
void vtss_ptp_default_filter_delete(vtss_ptp_offset_filter_handle_t filter);


#endif // _VTSS_PTP_OFFSET_FILTER_H_

// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
