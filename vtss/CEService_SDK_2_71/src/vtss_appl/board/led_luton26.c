/* -*- Mode: C; c-basic-offset: 2; tab-width: 8; c-comment-only-line-offset: 0; -*- */
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

#ifdef VTSS_SW_OPTION_PHY
#include "misc_api.h"
#include "phy_api.h" // For PHY_INST
#endif

/**
 * \led_luton26.h
 * \brief This file contains the code for LED control for the Luton26 hardware platform.
 *
 *  The Luton26 hardware platform consists of 2 chip sets. The Luton26 chip, and a Atom12
 *  chip set. Both chip sets are used for controlling their corresponding port LEDs.
 *
 *
 */


/* SGPIO LED mapping */
typedef struct {
    u8  port;
    u8  bit;
} sgpio_mapping_t;

#if defined(BOARD_LUTON26_REF)
static const sgpio_mapping_t status_led_mapping[2] = {
    {0, 2}, /* green */
    {1, 2}  /* red */
};
#endif /* BOARD_LUTON26_REF */

#if defined(BOARD_LUTON10_REF) 
static const sgpio_mapping_t status_led_mapping[2] = {
    {26, 0}, /* green */
    {26, 1}  /* red */
};
#endif


/****************************************************************************/
/*                                                                          */
/*  MODULE INTERNAL FUNCTIONS - LED board dependent functions               */
/*                                                                          */
/****************************************************************************/

/* The reference implementation - lunton26_ref */
/** \brief LEDs control initialization  */

void LED_led_init(void)
{
    vtss_phy_enhanced_led_control_t conf;
    conf.ser_led_output_2 = TRUE; // 
    conf.ser_led_output_1 = FALSE;
    conf.ser_led_frame_rate = 0x1;
    conf.ser_led_select = 0;

#if defined(BOARD_LUTON26_REF) 
    vtss_phy_enhanced_led_control_init(PHY_INST, 13, conf);
#endif
#if defined(BOARD_LUTON10_REF)    
    vtss_phy_enhanced_led_control_init(PHY_INST, 1, conf);
#endif


}


/* The reference implementation -  luton26_ref */
void LED_front_led_set(LED_led_colors_t color)
{
    static LED_led_colors_t old_color;

    if (old_color == color) {
        return;
    } else {
        old_color = color;
    }

#if defined(BOARD_LUTON26_REF) || defined(BOARD_LUTON10_REF)
    vtss_sgpio_conf_t conf;

    /* Make sure that the SGPIO initilization has been done */
    if (vtss_board_type() == VTSS_BOARD_UNKNOWN || vtss_sgpio_conf_get(NULL, 0, 0, &conf) != VTSS_RC_OK) {
      return;
    }

    switch(color) {
        case lc_GREEN:
            conf.port_conf[status_led_mapping[0].port].mode[status_led_mapping[0].bit] = VTSS_SGPIO_MODE_ON;
            conf.port_conf[status_led_mapping[1].port].mode[status_led_mapping[1].bit] = VTSS_SGPIO_MODE_OFF;
          break;
          
        case lc_RED:
            conf.port_conf[status_led_mapping[0].port].mode[status_led_mapping[0].bit] = VTSS_SGPIO_MODE_OFF;
            conf.port_conf[status_led_mapping[1].port].mode[status_led_mapping[1].bit] = VTSS_SGPIO_MODE_ON;
          break;
        
        default:
            conf.port_conf[status_led_mapping[0].port].mode[status_led_mapping[0].bit] = VTSS_SGPIO_MODE_OFF;
            conf.port_conf[status_led_mapping[1].port].mode[status_led_mapping[1].bit] = VTSS_SGPIO_MODE_OFF;
          break;
    }

    vtss_sgpio_conf_set(NULL, 0, 0, &conf);
#endif /* BOARD_LUTON26_REF || BOARD_LUTON10_REF */
}

