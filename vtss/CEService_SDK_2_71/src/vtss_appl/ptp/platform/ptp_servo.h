/*

 Vitesse Switch API software.

 Copyright (c) 2002-2010 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _PTP_SERVO_H_
#define _PTP_SERVO_H_

#include "vtss_ptp_api.h"
#include "vtss_ptp_offset_filter.h"
#include "vtss_ptp_delay_filter.h"


/**
 * \brief Configurable part of Clock Servo Data Set structure
 */
typedef struct {
	BOOL display_stats;
	BOOL csv_stats;
	BOOL no_adjust;
	short delay_filter;
	short ap;
	short ai;
} ptp_clock_servo_con_ds_t;


vtss_ptp_offset_filter_handle_t ptp_servo_create(const ptp_clock_servo_con_ds_t *c);
vtss_ptp_delay_filter_handle_t ptp_delay_filter_create(ptp_clock_servo_con_ds_t *c, int port_count);

void ptp_servo_delete(vtss_ptp_offset_filter_handle_t servo);
void ptp_delay_filter_delete(vtss_ptp_delay_filter_handle_t delay);



#endif // _PTP_SERVO_H_


// ***************************************************************************
//
//  End of file.
//
// ***************************************************************************
