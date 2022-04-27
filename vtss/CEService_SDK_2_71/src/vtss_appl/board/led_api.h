/* -*- Mode: C; c-basic-offset: 2; tab-width: 8; c-comment-only-line-offset: 0; -*- */
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

#ifndef _LED_API_H_
#define _LED_API_H_

#include <main.h>

/* Base colors */
typedef enum {lc_NULL, lc_OFF, lc_GREEN, lc_RED} LED_led_colors_t;

/****************************************************************************/
// The led_front_led_state() function takes an argument that tells this
// module how to exercise the front LED. This argument is one of the
// following.
// NOTE 1: If you sometime in the future need to add another state, make sure
// you also add a state configuration, which is set in the SL_led_state_cfg[]
// array within led.c.
// NOTE 2: Future states MUST be added after LED_FRONT_LED_NORMAL and before
// LED_FRONT_LED_ERROR due to an implicit severity/stickyness.
/****************************************************************************/
typedef enum {
  LED_FRONT_LED_NORMAL = 0, 
  LED_FRONT_LED_FLASHING_BOARD, 
  LED_FRONT_LED_STACK_FW_CHK_ERROR, // Stack neighbor has incompatible FW ver.
  // Add more states here, if needed.
  LED_FRONT_LED_ERROR, 
  LED_FRONT_LED_FATAL} led_front_led_state_t;

/* 7-segment LCD symbols */
typedef enum {
  LCD_SYMBOL_0 = 0,
  LCD_SYMBOL_1,
  LCD_SYMBOL_2,
  LCD_SYMBOL_3,
  LCD_SYMBOL_4,
  LCD_SYMBOL_5,
  LCD_SYMBOL_6,
  LCD_SYMBOL_7,
  LCD_SYMBOL_8,
  LCD_SYMBOL_9,
  LCD_SYMBOL_A,
  LCD_SYMBOL_B,
  LCD_SYMBOL_C,
  LCD_SYMBOL_D,
  LCD_SYMBOL_E,
  LCD_SYMBOL_F,
  LCD_SYMBOL_BLANK,             /* "space" */
  LCD_SYMBOL_DASH,              /* "-" */
  LCD_SYMBOL_PERIOD,            /* "." */
  LCD_SYMBOL_COUNT              /* Must be last */
} lcd_symbol_t;

/*
 * Overridable (board) functions
 */
void LED_led_init(void);
void LED_front_led_set(LED_led_colors_t color);
void LED_master_led_set(BOOL master); /* External module - if any */
void LED_lcd_set(lcd_symbol_t sym1, lcd_symbol_t sym2);

/****************************************************************************/
// led_front_led_state()
// Change the state of the front LED on boards that support this function.
// Note that some of the states are sticky in the sense that you cannot
// return to a state with a smaller significance, when once entered. E.g.
// you cannot change from LED_FRONT_LED_FATAL to LED_FRONT_LED_NORMAL.
/****************************************************************************/
void led_front_led_state(led_front_led_state_t state);

/****************************************************************************/
// led_usid_set()
// Set USID value to be shown on 7-segment LEDs.
// If usid==0, then "-" is shown. 
/****************************************************************************/
void led_usid_set(vtss_usid_t usid);

/****************************************************************************/
// led_init()
// Module initialization function.
/****************************************************************************/
vtss_rc led_init(vtss_init_data_t *data);


/****************************************************************************/
// led_front_led_in_error_state()
// Function for checking if the front LED is indicating error/fatal.
/****************************************************************************/
BOOL led_front_led_in_error_state(void);

/****************************************************************************/
// Debug functions
/****************************************************************************/

// Return current USID LED value
vtss_usid_t led_usid_get(void);

#endif /* _LED_API_H_ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
