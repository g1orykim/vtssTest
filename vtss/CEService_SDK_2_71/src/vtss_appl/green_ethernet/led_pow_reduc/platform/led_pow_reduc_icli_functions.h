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

 $Id$
 $Revision$

*/

/**
 * \file
 * \brief LED power reduction icli functions
 * \details This header file describes LED power reduction control functions
 */


#ifndef VTSS_ICLI_LED_POWER_REDUCTION_H
#define VTSS_ICLI_LED_POWER_REDUCTION_H

#include "icli_api.h"



/**
 * \brief Function for setting LED power reduction on event configuration
 *
 * \param session_id [IN]  Needed for being able to print error messages
 * \param has_link_change [IN]  TRUE to set the link change time.
 * \param v_0_to_65535 [IN] New time in seconds to turn LEDs 100% on. Only valid if has_link_change is TRUE.
 * \param error [IN]  TRUE to if LEDs shall be turned 100% on when errors occur.
 * \param no [IN]  TRUE to set the corresponding link_change or error configuration to default value.
 * \return None.
 **/
void led_pow_reduc_icli_on_event(u32 session_id, BOOL has_link_change, u16 v_0_to_65535, BOOL has_error, BOOL no);


/**
 * \brief Function for setting LED power reduction on event configuration
 *
 * \param session_id [IN]  Needed for being able to print error messages
 * \param led_interval [IN]  Pointer to stuct containing intervals.
 * \param v_0_to_100 [IN] LED intensity value.
 * \param no [IN]  TRUE to set led intensity for the corresponding interval to default value.
 * \return None.
 **/
void led_pow_reduc_icli_led_interval(u32 session_id, icli_unsigned_range_t *led_interval, u8 intensity,  BOOL no);


/**
 * \brief Init function ICLI CFG
 * \return Error code.
 **/
vtss_rc led_pow_reduc_icfg_init(void);
#endif /* VTSS_ICLI_LED_POWER_REDUCTION_H */



/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
