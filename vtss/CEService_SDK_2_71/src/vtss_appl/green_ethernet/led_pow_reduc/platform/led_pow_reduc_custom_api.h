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

/**
 * \file led_pow_reduc_custom_api.h
 * \brief This file defines the custom setting for the LED power reduction module.
 *
 *
 */

#ifndef _LED_POW_REDUC_CUSTOM_API_H_
#define _LED_POW_REDUC_CUSTOM_API_H_



// **********************************************************************************************
// * Custom function for Vitesse Luton26 reference board                                                 *
// **********************************************************************************************
#ifdef BOARD_LUTON26_REF

#include "vtss_phy_api.h"
#include "led_api.h"

/**
  * \brief LED power reduction board/chip specific API function for setting the
  * LEDs intensity in percent.
  *
  */
// For Luton 26 reference board the LED pwm pin is connected at the ATOM12 chip (phy ports 13-24), GPIO 8. Therefore port 13 is used.
#define led_pow_reduc_custom_set_led_intensity(intensity) (void) vtss_phy_led_intensity_set(PHY_INST, 13, intensity)


/**
  * \brief LED power reduction board/chip specific API function initializing the LED power reduction API.
  *
  */
#define led_pow_reduc_custom_api_init // For Luton 26 reference board no initialization is needed

/**
  * \brief LED power reduction board/chip specific API function determining if an error as happen.
  *  The function MUST return a boolean (return TRUE if an error has happen else FALSE)
  */
#define led_pow_reduc_custom_on_at_error led_front_led_in_error_state()


// **********************************************************************************************
// * Custom functions for Vitesse Luton10 reference board                                                 *
// **********************************************************************************************
#elif defined(BOARD_LUTON10_REF)

#include "vtss_phy_api.h"
#include "led_api.h"

/**
  * \brief LED power reduction board/chip specific API function for setting the
  * LEDs intensity in percent.
  *
  * \param inst [IN]     LEDs intensity in percent.
  */
// For Luton 10 reference board the LED pwm is in fact the FAN controller PWM, so we need to used
// vtss_fan_cool_lvl_set which can be set from 0 to 255. We need to convert from percent to 0-255
#define led_pow_reduc_custom_set_led_intensity(intensity) (void) vtss_fan_cool_lvl_set(PHY_INST, intensity * 255 / 100)

/**
  * \brief LED power reduction board/chip specific API function initializing the LED power reduction API.
  *
  */
// For Luton 10 reference board the LED pwm is in fact the FAN controller PWM, so we need to initialize
// fan controller.
#define led_pow_reduc_custom_api_init {\
                                         vtss_fan_conf_t fan_spec;                          \
                                         fan_spec.fan_pwm_freq = VTSS_FAN_PWM_FREQ_25KHZ; \
                                         fan_spec.fan_low_pol  = TRUE;  \
                                         fan_spec.fan_open_col = FALSE; \
                                         (void) vtss_fan_controller_init(PHY_INST, &fan_spec ); \
                                      }




/**
  * \brief LED power reduction board/chip specific API function determining if an error as happen.
  *  The function MUST return a boolean (return TRUE if an error has happen else FALSE)
  */
#define led_pow_reduc_custom_on_at_error led_front_led_in_error_state()


// **********************************************************************************************
// * Custom functions for Vitesse Serval reference board                                       *
// **********************************************************************************************

#elif defined(BOARD_SERVAL_REF)

#include "led_api.h"
/**
  * \brief LED power reduction board/chip specific API function for setting the
  * LEDs intensity in percent.
  *
  */
// For Serval reference board the LED pwm pin is connected at the TESLA chip (phy ports 1-4), Therefore port 0 is used.
#define led_pow_reduc_custom_set_led_intensity(intensity) (void) vtss_phy_led_intensity_set(PHY_INST, 0, intensity)


/**
  * \brief LED power reduction board/chip specific API function initializing the LED power reduction API.
  *
  */
#define led_pow_reduc_custom_api_init // For Serval it is done in the led_serval.c file (function:LED_led_init)

/**
  * \brief LED power reduction board/chip specific API function determining if an error as happen.
  *  The function MUST return a boolean (return TRUE if an error has happen else FALSE)
  */
#define led_pow_reduc_custom_on_at_error led_front_led_in_error_state()


// **********************************************************************************************
// * Custom functions for Vitesse JR48 reference board                                       *
// **********************************************************************************************


#elif defined(BOARD_JAGUAR1_REF)
#include "led_api.h"
/**
  * \brief LED power reduction board/chip specific API function for setting the
  * LEDs intensity in percent.
  *
  */

// For Jaguar 24 we don't support LED power reduction and Jaguar48 board has 4 ATOM12 Phys, set intensity for each of them
#define led_pow_reduc_custom_set_led_intensity(intensity) if (port_isid_info_board_type_get(VTSS_ISID_LOCAL) == VTSS_BOARD_JAG_CU48_REF) { \
                                                              (void) vtss_phy_led_intensity_set(PHY_INST, 0, intensity); \
                                                              (void) vtss_phy_led_intensity_set(PHY_INST, 12, intensity); \
                                                              (void) vtss_phy_led_intensity_set(PHY_INST, 24, intensity);             \
                                                              (void) vtss_phy_led_intensity_set(PHY_INST, 36, intensity); \
                                                          }

/**
  * \brief LED power reduction board/chip specific API function initializing the LED power reduction API.
  *
  */
#define led_pow_reduc_custom_api_init // For Jaguar it is done in the led_jaguar1.c file (function:LED_led_init)

/**
  * \brief LED power reduction board/chip specific API function determining if an error as happen.
  *  The function MUST return a boolean (return TRUE if an error has happen else FALSE)
  */
#define led_pow_reduc_custom_on_at_error led_front_led_in_error_state()


// **********************************************************************************************
// * Custom functions not defined
// **********************************************************************************************

#else

#warning Custom functions not defined for this board

#endif // End board 

#endif // End _LED_POW_REDUC_CUSTOM_API_H_
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
