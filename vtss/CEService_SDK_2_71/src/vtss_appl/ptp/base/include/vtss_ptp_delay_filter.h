/*

 Vitesse Switch API software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _VTSS_PTP_DELAY_FILTER_H_
#define _VTSS_PTP_DELAY_FILTER_H_
/**
 * \file vtss_ptp_delay_filter.h
 * \brief Define Delay filter callouts.
 *
 * This file contain the definitions of Delay filter functions and associated
 * types.
 * The functions are indirectly called vis function pointers, defined in 
 * vtss_ptp_delay_filter_t structure, and can easily be exchanged with customer
 * defined functions.
 *
 */

#include "vtss_ptp_types.h"


/**
 * \brief Opaque Delay filter handle.
 */
typedef struct vtss_ptp_delay_filter_t *vtss_ptp_delay_filter_handle_t;

/**
 * \brief Clock Delay filter and servo parameter structure
 */
typedef vtss_timeinterval_t vtss_ptp_delay_filter_param_t;

// ***************************************************************************
//
//  General filter definitions.
//
// ***************************************************************************
/**
 * \brief Delay Filter Data structure
 */
typedef struct vtss_ptp_delay_filter_t {
	/**
	 * \brief Filter reset function pointer.
     *
	 * Reset the delay filter
	 *
	 * \param filter [IN]  instance data reference.
	 * \param port   [IN]  port number (0 = end to end delay instance,
	 *                      1..port_count = peer to peer delay instances.
	 * \return  None
	 */
	void (*delay_filter_reset)(vtss_ptp_delay_filter_handle_t filter, int port);

	/**
	 * \brief Filter execution function pointer.
     *
	 * Execute the delay filter operation (called each time the delay has been measured)
	 *
	 * \param filter [IN]  instance data reference.
	 * \param delay [IN/OUT]  pointer to delay data
	 *                          IN : measured delay
	 *                          OUT: filtered delay.
	 * \param port   [IN]  port number (0 = end to end delay instance,
	 *                      1..port_count = peer to peer delay instances.
	 * \return  0 : no delayupdate; 1 : do delayupdate.
	 */
	int (*delay_filter)(vtss_ptp_delay_filter_handle_t filter, vtss_ptp_delay_filter_param_t *delay, int port);

	void *private_data;
} vtss_ptp_delay_filter_t;

// ***************************************************************************
//
//  filter definitions specific for default filter.
//
// ***************************************************************************

/**
 * \brief Default Clock Filter config Data Set structure
 */
typedef struct vtss_ptp_default_delay_filter_config_t {
	short delay_filter;
} vtss_ptp_default_delay_filter_config_t;

/**
 * \brief Create a Default PTP delay filter instance.
 *
 * Create an instance of the default vtss_ptp filter
 *
 * \param of [IN]  pointer to a structure containing the default parameters for
 *                the delay filter
 * \param port_count [IN]  number og ports
 * \return (opaque) instance data reference or NULL.
 */
vtss_ptp_delay_filter_handle_t vtss_ptp_default_delay_filter_create(
    const vtss_ptp_default_delay_filter_config_t *of, int port_count);

/**
 * \brief Delete a Default PTP filter instance.
 *
 * Delete an instance of the default vtss_ptp filter
 *
 * \param filter [IN]  instance data reference
 * \return  None
 */
void vtss_ptp_default_delay_filter_delete(vtss_ptp_delay_filter_handle_t filter);

#endif // _VTSS_PTP_DELAY_FILTER_H_

// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
