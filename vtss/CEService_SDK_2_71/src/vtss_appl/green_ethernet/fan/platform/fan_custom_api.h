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

#ifndef _FAN_CUSTOM_API_H_
#define _FAN_CUSTOM_API_H_


/**
 * \file fan_custom_api.h
 * \brief This file defines the data for the specific fan used for colling the chip.
 *
 */


// Macro returning TRUE if the board for the given isid is a Jaguar 48 board
//#define IS_BOARD_JAG_48(isid) (port_isid_info_board_type_get(isid) == VTSS_BOARD_JAG_CU48_REF)
#define IS_BOARD_JAG_48(isid) (port_isid_info_board_type_get(isid) == VTSS_BOARD_JAG_CU48_REF)

// Macro returning TRUE if the board for the given isid is a Jaguar 24 board
#define IS_BOARD_JAG_24(isid) ((port_isid_info_board_type_get(isid) == VTSS_BOARD_JAG_CU24_REF) || \
                              (port_isid_info_board_type_get(isid) == VTSS_BOARD_JAG_SFP24_REF))

// Macro returning TRUE if the board for the given isid is a Luton26 board
#define IS_BOARD_LUTON26(isid) (port_isid_info_board_type_get(isid) == VTSS_BOARD_LUTON26_REF)

// Macro returning TRUE if the board for the given isid is a Serval  board
#define IS_BOARD_SERVAL(isid) (port_isid_info_board_type_get(isid) == VTSS_BOARD_SERVAL_PCB106_REF)


/**
  * \brief Fan controller initialization
  */
// JR 24 ports boards fan controller is pure analog and doesn't need to be initialized - just return VTSS_OK
// JR 48 ports board fan controller doesn't need to be initialized - just return VTSS_OK
#define fan_initialize(isid, fan_spec) IS_BOARD_LUTON26(isid) || IS_BOARD_SERVAL(isid) ? vtss_fan_controller_init(NULL, &fan_spec) : \
                                       IS_BOARD_JAG_24(isid)  ? VTSS_OK :  \
                                       IS_BOARD_JAG_48(isid)  ? VTSS_OK : VTSS_RC_ERROR



/**
  * \brief Defining the number of temperature sensors at the board.
  */
// The Luton26(VTSS_BOARD_LUTON26_REF) reference board has 2 temperature sensors (One in Luton26 and one in Atom12)

// Jaguar 24 boards have one sensor (The sensor in jaguar)

// Jaguar 48 board (VTSS_BOARD_JAG_CU48_REF) has the following temperature sensors
// 1 - Jaguar temperature sensor
// 2 - Temperature sensor for ATOM12 (port2 1-12)
// 3 - Temperature sensor for ATOM12 (port2 1-12)
// 4 - Temperature sensor for ATOM12 (port2 1-12)
// 5 - Temperature sensor for ATOM12 (port2 1-12)

// Serval board has one sensor (in Tesla PHY)
#define FAN_TEMPERATURE_SENSOR_CNT(isid) IS_BOARD_JAG_48(isid)  ? 5 :   \
                                         IS_BOARD_JAG_24(isid)   ? 1 :  \
                                         IS_BOARD_LUTON26(isid) ? 2 :   \
                                         IS_BOARD_SERVAL(isid) ? 1 : 0

// Define the board to the largest number of sensors
#define FAN_TEMPERATURE_SENSOR_CNT_MAX 5

/**
  * \brief Temperature sensor initialization
  */

#include "vtss_misc_api.h" // For vtss_temp_sensor_init
#define FAN_INIT_BOARD_JAG_CU48 ((vtss_temp_sensor_init(NULL, TRUE) == VTSS_OK && \
                                                vtss_phy_chip_temp_init(NULL, 0) == VTSS_OK && \
                                                vtss_phy_chip_temp_init(NULL, 12) == VTSS_OK && \
                                                vtss_phy_chip_temp_init(NULL, 24) == VTSS_OK && \
                                                vtss_phy_chip_temp_init(NULL, 36) == VTSS_OK) ? VTSS_RC_OK : VTSS_RC_ERROR)


// Any port can be used for luton26 - We use port 0 for luton26 and port 12 for ATom12
#define FAN_INIT_BOARD_LUTON26  (vtss_phy_chip_temp_init(NULL, 0) == VTSS_OK && \
                                 vtss_phy_chip_temp_init(NULL, 12) == VTSS_OK) ? VTSS_OK : VTSS_RC_ERROR \
 

// Any port can be used for Tesla chip - We use port 0
#define FAN_INIT_BOARD_SERVAL  vtss_phy_chip_temp_init(NULL, 0)


#define fan_init_temperature_sensor(isid)  IS_BOARD_JAG_24(isid)  ? vtss_temp_sensor_init(NULL, TRUE) :\
                                           IS_BOARD_JAG_48(isid) ? FAN_INIT_BOARD_JAG_CU48 : \
                                           IS_BOARD_SERVAL(isid) ? FAN_INIT_BOARD_SERVAL :\
                                           IS_BOARD_LUTON26(isid) ? FAN_INIT_BOARD_LUTON26 : VTSS_RC_ERROR



/**
  * \brief Getting the chip temperature
  */

// Getting temperature for both Luton 26 (phy 0) and Atom 12 (phy 12)
#define FAN_GET_TEMP_VTSS_BOARD_LUTON26_REF(temp) (vtss_phy_chip_temp_get(NULL, 0, &temp[0]) == VTSS_OK && \
                                                   vtss_phy_chip_temp_get(NULL, 12, &temp[1]) == VTSS_OK) ? \
                                                   VTSS_OK : VTSS_RC_ERROR

// See comment at FAN_TEMPERATURE_SENSOR_CNT
#define FAN_GET_TEMP_BOARD_JAG_CU48(temp) (vtss_temp_sensor_get(NULL, &temp[0]) == VTSS_OK && \
                                           vtss_phy_chip_temp_get(NULL, 0, &temp[1]) == VTSS_OK && \
                                           vtss_phy_chip_temp_get(NULL, 12, &temp[2]) == VTSS_OK && \
                                           vtss_phy_chip_temp_get(NULL, 24, &temp[3]) == VTSS_OK && \
                                           vtss_phy_chip_temp_get(NULL, 36, &temp[4]) == VTSS_OK) ? \
                                           VTSS_OK : VTSS_RC_ERROR


#define fan_get_temperture(isid, temp) IS_BOARD_LUTON26(isid) ? FAN_GET_TEMP_VTSS_BOARD_LUTON26_REF(temp) : \
                                       IS_BOARD_JAG_24(isid)  ? vtss_temp_sensor_get(NULL, temp) :     \
                                       IS_BOARD_SERVAL(isid)  ? vtss_phy_chip_temp_get(NULL, 0, temp) : \
                                       IS_BOARD_JAG_48(isid)  ? FAN_GET_TEMP_BOARD_JAG_CU48(temp) : VTSS_RC_ERROR






/**
  * \brief Getting the fan speed
  */

// JR boards don't support getting the FAN speed. By returning VTSS_RC_ERROR the speed is set to 0.
#define fan_speed_level_get(isid, level) IS_BOARD_LUTON26(isid) || IS_BOARD_SERVAL(isid)  ? vtss_fan_cool_lvl_get(NULL, level) : \
                                          VTSS_RC_ERROR



/**
  * \brief Setting the fan speed
  */

#if defined(VTSS_CHIP_E_STAX_III_68) || defined(VTSS_CHIP_E_STAX_III_68_DUAL) || defined(VTSS_CHIP_LYNX_1) || defined(VTSS_CHIP_JAGUAR_1) || defined(VTSS_CHIP_CE_MAX_24) || defined(VTSS_CHIP_CE_MAX_12)
extern vtss_rc jr1_fan_control_set(u8 level);
#define JR_FAN_CONTROL(level) jr1_fan_control_set(level)
#else
#define JR_FAN_CONTROL(level) VTSS_RC_ERROR
#endif




// For the Jaguar 48 ports board the fan is connected to the ATOM12 (ports 37-48) PWM (GPIO3). Unfortunately this doesn't work with the fan we
// have chosen, so we can only turn on/off

#define fan_speed_level_set(isid, level) IS_BOARD_LUTON26(isid) || IS_BOARD_SERVAL(isid) ? vtss_fan_cool_lvl_set(NULL, level) : \
                                         IS_BOARD_JAG_24(isid)  ? JR_FAN_CONTROL(level) :                \
                                         IS_BOARD_JAG_48(isid)  ? vtss_phy_write_masked_page(NULL, 47, 0x10, 25, ((0xff - level) << 8) + 0x40, 0xFF40)    : VTSS_RC_ERROR


/**
  * \brief Fan-specific type
  *
  * The fan type - 3 types of fans are supported.
  *                   a. VTSS_FAN_2_WIRE_TYPE
  *                   b. VTSS_FAN_3_WIRE_TYPE
  *                   c. VTSS_FAN_4_WIRE_TYPE

  */

#define FAN_CUSTOM_TYPE(isid)  IS_BOARD_LUTON26(isid) || IS_BOARD_SERVAL(isid) ? VTSS_FAN_3_WIRE_TYPE : \
                               VTSS_FAN_2_WIRE_TYPE


/**
  * \brief Fan-specific PPR (Pulses Per Rotation).
  *
  * This parameter is only valid for 3 and 4 wire fan types.
  * 3 and 4 wire fans have the possibility to signal their rotation
  * speed by sending pulses to the chip. To be able to calculate
  * the RPM (Rotation Per Minute) we must know the number of pulses
  * the fan gives for each rotation.
  *
  */
#define FAN_CUSTOM_PPR 2


/**
  * \brief Fan-specific polarity for PWM.
  *
  * 0: PWM is logic 1 when "on"
  * 1: PWM is logic 0 when "on"
  */
#define FAN_CUSTOM_POL IS_BOARD_SERVAL(0) ? 1 : 0


/**
  * \brief Fan-specific open collector
  *
  * 0: FAN is driven with "high/low" output.
  * 1: FAN is driven with "open collector" output. (Requires external pull up)
  */
#define FAN_CUSTOM_OC 1


/**
  * \brief Fan-specific minimum PWM
  *
  * Some fans can not run with very low pulse width. FAN_CUSTOM_MIN_PWM_PCT specifies
  * the minimum pulse width for the fan used. Specified in percent.
  *
  */

#define FAN_CUSTOM_MIN_PWM_PCT(isid)  (IS_BOARD_LUTON26(isid) || IS_BOARD_SERVAL(isid) ? 30 :    \
                                      IS_BOARD_JAG_24(isid) ? 70 :  \
                                       IS_BOARD_JAG_48(isid) ? 100 : 100)


/**
  * \brief Fan-specific kick-start level
  *
  * Some fans needs to be "kick-started" in order to start at low PWM, FAN_CUSTOM_KICK_START_LVL_PCT defines
  * at which PWM level the fan used shall be kick-started
  */

#define FAN_CUSTOM_KICK_START_LVL_PCT(isid)  (IS_BOARD_LUTON26(isid) || IS_BOARD_SERVAL(isid) ? 60 : \
                                             IS_BOARD_JAG_24(isid) ? 80 :  \
                                              IS_BOARD_JAG_48(isid) ? 0 : 0)



/**
  * \brief Fan-specific kick-start time
  *
  * Defines the time that the fan needs to be kick-started in mili seconds
  */
#define FAN_CUSTOM_KICK_START_ON_TIME 100

#endif
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
