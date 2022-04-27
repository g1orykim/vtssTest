/*

 Vitesse Switch Software.

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
/************************************************************-*- mode: C -*-*/
/*                                                                          */
/*           Copyright (C) 2007 Vitesse Semiconductor Corporation           */
/*                           All Rights Reserved.                           */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*                            Copyright Notice:                             */
/*                                                                          */
/*  This document contains confidential and proprietary information.        */
/*  Reproduction or usage of this document, in part or whole, by any means, */
/*  electrical, mechanical, optical, chemical or otherwise is prohibited,   */
/*  without written permission from Vitesse Semiconductor Corporation.      */
/*                                                                          */
/*  The information contained herein is protected by Danish and             */
/*  international copyright laws.                                           */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*  <DESCRIBE FILE CONTENTS HERE>                                           */
/*                                                                          */
/****************************************************************************/
/****************************************************************************/

/**
 * \file
 * \brief TI SLUS787 PoE driver
 * This header file contains the functions of driver
 */


/** \brief Define how many ports each PoE chip controls. */
#define PORTS_PER_POE_CHIP 4 /**< 4 ports per chip */

/**
 * \brief Get the current TI PoE module on system.
 *
 * - Re-entrant:\n
 *   No
 *
 * \param port_index [IN]:       The physical port to get the settings for.
 * \return
 *   1: TI PoE module is existed.
 *   0: No TI PoE Module.
 */
int slus787_is_chip_available(vtss_port_no_t port_index);

/**
 * \brief Set PoE port enable.
 *
 * Use this function to enable and disable PoE port
 *
 * - Re-entrant:\n
 *   No
 *
 * \param port_index [IN]:       The physical port to get the settings for.
 * \param enable [OUT]:          A BOOL that enable or disable.
 * \return
 *   No
 */
void slus787_poe_enable(vtss_port_no_t port_index, BOOL enable);
/**
 * \brief Set PoE port class/detect enable.
 *
 * Use this function to force auto class/detect signal
 *
 * - Re-entrant:\n
 *   No
 *
 * \param port_index [IN]:       The physical port to get the settings for.
 * \return
 *   No
 */
void slus787_poe_force_auto_detect(vtss_port_no_t port_index);
/**
 * \brief Set PoE port class/detect disable.
 *
 * Use this function to force off class/detect signal
 *
 * - Re-entrant:\n
 *   No
 *
 * \param port_index [IN]:       The physical port to get the settings for.
 * \return
 *   No
 */
void slus787_poe_force_off_detect(vtss_port_no_t port_index);
/**
 * \brief Set PoE module into known state.
 *
 * Use this function to initialize PoE module
 *
 * - Re-entrant:\n
 *   No
 *
  * \param port_index [IN]:       The physical port to get the settings for.
 * \return
 *   No
 */
void slus787_poe_init(vtss_port_no_t port_index);
/**
 * \brief Get PoE port poe_status.
 *
 * Use this function to update poe_status structure information
 *
 * - Re-entrant:\n
 *   No
 *
 * \param port_index [IN]:       The physical port to get the settings for.
 * \param poe_status [OUT]: PoE port status
 * \return
 *   No
 */
void slus787_get_port_measurement(poe_status_t *poe_status, vtss_port_no_t port_index);
/**
 * \brief Get PoE port poe_status.
 *
 * Use this function to update all poe_status structure information
 *
 * - Re-entrant:\n
 *   No
 *
 * \param port_index [IN]:       The physical port to get the settings for.
 * \param poe_status [OUT]: PoE port status
 * \return
 *   No
 */
void slus787_port_status_get(poe_port_status_t *port_status, vtss_port_no_t port_index);
/**
 * \brief Get PoE port PD class information
 *
 * Use this function to update class
 *
 * - Re-entrant:\n
 *   No
 *
 * \param classes [OUT]: PD class
 * \return
 *   No
 */
void slus787_get_all_port_class(char *classes, vtss_port_no_t port_index);
/**
 * \brief Set PoE port power alloaction
 *
 * Use this function to allocate PoE power on port.
 *
 * - Re-entrant:\n
 *   No
 *
 * \param port_index [IN]:       The physical port to get the settings for.
 * \param max_port_power [IN]:   The max power to get the settings for.
 * \return
 *   No
 */
void slus787_set_power_limit_channel(vtss_port_no_t port_index, int max_port_power) ;


/** \brief Debug function for writing to a PoE register
 *
 *
 *
 * - Re-entrant:\n
 *   No
 *
 * \param port_index [IN]:       The physical port to get the settings for.
 * \param addr [IN]:             The address.
 * \param data [OUT]:            The Data.
 * \return
 *   No
 */
void slus787_device_wr(vtss_port_no_t port_index, uchar reg_addr, uchar data);



/** \brief Debug function for writing to a PoE register
 *
 *
 *
 * - Re-entrant:\n
 *   No
 *
 * \param port_index [IN]:       The physical port to get the settings for.
 * \param addr [IN]:             The address.
 * \param data [OUT]:            The Data.
 * \param size [IN]:             The number of bytes to read
 * \return
 *   No
 */
void slus787_device_rd(vtss_port_no_t port_index, uchar reg_addr, uchar *data, size_t size);
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
