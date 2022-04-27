/* -*- Mode: C; c-basic-offset: 4; tab-width: 8; c-comment-only-line-offset: 0; -*- */
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

#include "main.h"
#include "led_api.h"
#include "vtss_misc_api.h"
#include "port_custom_api.h"
#include "port_api.h"

#ifdef VTSS_SW_OPTION_PHY
#include "misc_api.h"
#include "phy_api.h" // For PHY_INST
#endif

/**
 * \led_serval.c
 *
 * \brief This file contains the code for LED control for the Serval
 * hardware platform.
 *
 *  The status LED is attached through the SGPIO interface, bits p11.0
 *  and p11.1 (green, red).
 */

#define STATUS_GREEN_PORT 11
#define STATUS_GREEN_BIT  0

#define STATUS_RED_PORT   11
#define STATUS_RED_BIT    1

/****************************************************************************/
/*                                                                          */
/*  MODULE INTERNAL FUNCTIONS - LED board dependent functions               */
/*                                                                          */
/****************************************************************************/


/* The reference implementation - lunton26_ref */
/** \brief LEDs control initialization  */

void LED_led_init(void)
{
    vtss_phy_led_mode_select_t led_blink_mode;
    vtss_phy_enhanced_led_control_t conf;
    u8 port_no;

    /* For SERVAL_LITE, Tesla is not used */
    if (!port_phy(0))
	return;

    conf.ser_led_output_2 = FALSE; // Not used. 
    conf.ser_led_output_1 = TRUE; // Enhanced serial LED output enable
    conf.ser_led_frame_rate = 0x1; // Frame rate = 1000Hz
    conf.ser_led_select = 2; // For PCB105 this can be set to anything, since the serial pins are not used, but for PCB106 the serial pins are connected to shift registers that expect 2 LEDs per PHY.
    // This is not used for PCB105, but for PCB106 the TESLA PHY serial stream is used for SFPs. Set LED mode to SFP
    for (port_no = 0; port_no < 4; port_no++) {
        led_blink_mode.mode = LINK100BASE_FX_ACTIVITY ;
        led_blink_mode.number = LED0;
        vtss_phy_led_mode_set(NULL, port_no, led_blink_mode);

        led_blink_mode.mode = LINK1000BASE_X_ACTIVITY;                      ;
        led_blink_mode.number = LED1;
        vtss_phy_led_mode_set(NULL, port_no, led_blink_mode);
    }
    
    vtss_phy_enhanced_led_control_init(PHY_INST, 1, conf); // Initialize the API
}



/* The reference implementation -  serval reference board */
void LED_front_led_set(LED_led_colors_t color)
{
    static LED_led_colors_t old_color;
    vtss_sgpio_conf_t conf;

    if (old_color == color) {
        return;
    } else {
        old_color = color;
    }

    /* Make sure that the SGPIO initilization has been done */
    if (vtss_board_type() == VTSS_BOARD_UNKNOWN || 
        vtss_sgpio_conf_get(NULL, 0, 0, &conf) != VTSS_RC_OK)
        return;

    switch(color) {
    case lc_GREEN:
        conf.port_conf[STATUS_GREEN_PORT].mode[STATUS_GREEN_BIT] = VTSS_SGPIO_MODE_ON;
        conf.port_conf[STATUS_RED_PORT  ].mode[STATUS_RED_BIT  ] = VTSS_SGPIO_MODE_OFF;
        break;
      
    case lc_RED:
        conf.port_conf[STATUS_GREEN_PORT].mode[STATUS_GREEN_BIT] = VTSS_SGPIO_MODE_OFF;
        conf.port_conf[STATUS_RED_PORT  ].mode[STATUS_RED_BIT  ] = VTSS_SGPIO_MODE_ON;
        break;
        
    default:
        conf.port_conf[STATUS_GREEN_PORT].mode[STATUS_GREEN_BIT] = VTSS_SGPIO_MODE_OFF;
        conf.port_conf[STATUS_RED_PORT  ].mode[STATUS_RED_BIT  ] = VTSS_SGPIO_MODE_OFF;
        break;
    }

    vtss_sgpio_conf_set(NULL, 0, 0, &conf);
}
