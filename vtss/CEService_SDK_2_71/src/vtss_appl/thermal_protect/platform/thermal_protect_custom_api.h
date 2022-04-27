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

#ifndef _THERMAL_PROTECT_CUSTOM_API_H_
#define _THERMAL_PROTECT_CUSTOM_API_H__

#include "vtss_phy_api.h"

/**
 * \file thermal_protect_custom_api.h
 * \brief This file defines the data for the specific thermal protection.
 *
 */


#ifdef BOARD_LUTON26_REF
/**
  * \brief Thermal protection/board specific api function
  *  for getting the chips temperature.
  */

// The Luton26 board has a temperature sensor in Luton26 (ports 0-11) and one in Atom12 (ports 12-24)
#define thermal_protect_get_temperture(iport, temp)  vtss_phy_chip_temp_get(NULL, (iport < 12) ? 0: 12, temp)

/**
  * \brief Thermal protection/board specific api function
  *  for initialising temperature controller.
  */

// The Luton26 board has a temperature sensor in Luton26 (ports 0-11) and one in Atom12 (ports 12-24)
#define thermal_protect_init_temperature_sensor (vtss_phy_chip_temp_init(NULL, 0) == VTSS_OK && \
                                                 vtss_phy_chip_temp_init(NULL, 12) == VTSS_OK) ? \
                                                 VTSS_OK : VTSS_RC_ERROR

#elif defined(BOARD_LUTON10_REF)
// Only one temperature sensor for Luton10 boards

/**
  * \brief Thermal protection/board specific api function
  *  for getting the chips temperature.
  */

#define thermal_protect_get_temperture(iport, temp)  vtss_phy_chip_temp_get(NULL, 0, temp)

/**
  * \brief Thermal protection/board specific api function
  *  for initialising temperature controller.
  */


#define thermal_protect_init_temperature_sensor vtss_phy_chip_temp_init(NULL, 0)

#elif defined(BOARD_SERVAL_REF)
// For the Serval board we only have temperature sensor at port 0.

/**
  * \brief Thermal protection/board specific api function
  *  for getting the chips temperature.
  */
#define thermal_protect_get_temperture(iport, temp)  vtss_phy_chip_temp_get(NULL, 0, temp)

/**
  * \brief Thermal protection/board specific api function
  *  for initialising temperature controller.
  */
#define thermal_protect_init_temperature_sensor vtss_phy_chip_temp_init(NULL, 0)

#else

#error("No function for getting temperature");

#endif

#endif
