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

#ifndef _VTSS_ICLI_POE_H_
#define _VTSS_ICLI_POE_H_

#include "icli_api.h" // for icli_stack_port_range_t


/**
 * \file
 * \brief PoE iCLI functions
 * \details This header file describes PoE iCLI functions
 */



/**
 * \brief Function for displaying PoE status
 *
 * \param session_id [IN] The session id use by iCLI print.
 * \param has_interface [IN] TRUE if the user want to display a specific interface (port)
 * \param list [IN] port list of which interfaces to display PoE status.
 * \return None.
 **/
void poe_icli_show(i32 session_id, BOOL has_interface, icli_stack_port_range_t *list);


/**
 * \brief Function for configuring PoE mode
 *
 * \param session_id [IN] The session id use by iCLI print.
 * \param has_poe [IN] TRUE if the user want to set PoE to PoE mode (15.4W).
 * \param has_poe_plus [IN] TRUE if the user want to set PoE to PoE mode (30W).
 * \param plist [IN] List of interfaces to configure.
 * \param no [IN] TRUE is user want to set mode to default value
 * \return None.
 **/
void poe_icli_mode(i32 session_id, BOOL has_poe, BOOL has_poe_plus, icli_stack_port_range_t *plist, BOOL no);

/**
 * \brief Function for configuring PoE priority
 *
 * \param session_id [IN] The session id use by iCLI print.
 * \param has_low [IN] TRUE if the user want to priority to low.
 * \param has_high [IN] TRUE if the user want to priority to high.
 * \param has_critical [IN] TRUE if the user want to priority to critical.
 * \param plist [IN] List of interfaces to configure.
 * \param no [IN] TRUE is user want to set mode to default value
 * \return None.
 **/
void poe_icli_priority(i32 session_id, BOOL has_low, BOOL has_high, BOOL has_critical, icli_stack_port_range_t *plist, BOOL no);

/**
 * \brief Function for configuring PoE management mode
 *
 * \param session_id [IN] The session id use by iCLI print.
 * \param has_* [IN] TRUE for the mode that the user want.
 * \param no [IN] TRUE is user want to set mode to default value
 * \return None.
 **/
void poe_icli_management_mode(i32 session_id, BOOL has_class_consumption, BOOL has_class_reserved_power, BOOL has_allocation_consumption, BOOL has_alllocation_reserved_power, BOOL has_lldp_consumption, BOOL has_lldp_reserved_power, BOOL no);


/**
 * \brief Function for configuring power limit for port in allocation mode.
 *
 * \param session_id [IN] The session id use by iCLI print.
 * \param value [IN] New value
 * \param plist [IN] List of interfaces to configure.
 * \param no [IN] TRUE is user want to set mode to default value
 * \return None.
 **/
void poe_icli_power_limit(i32 session_id, char *value, icli_stack_port_range_t *plist, BOOL no);

/**
 * \brief Function for configuring maximum power for the power supply
 *
 * \param session_id [IN] The session id use by iCLI print.
 * \param has_sid[IN] TRUE if user has specified a specific sid
 * \param usid[IN] User switch ID.
 * \param no [IN] TRUE is user want to set mode to default value
 * \return None.
 **/
void poe_icli_power_supply(i32 session_id, icli_unsigned_range_t *usid_list, u32 value, BOOL no);

/**
 * \brief Function for initializing ICFG
 **/
vtss_rc poe_icfg_init(void);


#endif // _VTSS_ICLI_POE_H_
