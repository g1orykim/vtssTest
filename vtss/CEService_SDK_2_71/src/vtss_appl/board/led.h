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
 
 $Id$
 $Revision$

*/

#ifndef _LED_H_
#define _LED_H_

/* =================
 * Trace definitions
 * -------------- */
#include <vtss_trace_lvl_api.h>

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_LED

#define VTSS_TRACE_GRP_DEFAULT 0
#define TRACE_GRP_CNT          1

#include <vtss_trace_api.h>
/* ============== */

#define LED_FRONT_LED_MAX_SUBSTATES 3 /* Defines the maximum number of sub-states one state can undergo before starting over */

/****************************************************************************/
// This defines the configurations of the LED, i.e. the color(s) and
// blinking rate.
/****************************************************************************/
typedef struct {
  LED_led_colors_t          colors[LED_FRONT_LED_MAX_SUBSTATES+1];
  unsigned long            timeout_ms[LED_FRONT_LED_MAX_SUBSTATES];
  led_front_led_state_t least_next_state;
} LED_front_led_state_cfg_t;

#endif /* _LED_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
