/* Switch API software.

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

#include "icli_api.h"

/**
 * \file
 * \brief sFlow icli functions
 * \details This header file describes dot1x iCLI functions
 */

/**
 * \brief Function for printing dot1x statistics.
 *
 * \param session_id [IN] Needed for being able to print
 * \param has_eapol  [IN] TRUE to print eapol statistics
 * \param has_radius [IN] TRUE to print radius statistics
 * \param has_all    [IN] TRUE to print all dot1x statistics
 * \param plist      [IN] List of interfaces to print statistics for.
 * \return error code.
 **/
vtss_rc dot1x_icli_statistics(i32 session_id, BOOL has_eapol, BOOL has_radius, BOOL has_all, icli_stack_port_range_t *plist);

/**
 * \brief Function for printing dot1x status such as e.g. Admin state.
 *
 * \param session_id [IN] Needed for being able to print
 * \param plist      [IN] List of interfaces to print status for.
 * \param has_brief  [IN] TRUE to print status in brief format (In order to show everything within one line of 85 characters).
 * \return error code.
 **/
vtss_rc dot1x_icli_status(i32 session_id, icli_stack_port_range_t *plist, BOOL has_brief);

/**
 * \brief Function for clearing dot1x statistics.
 *
 * \param session_id [IN] Needed for being able to print
 * \param plist      [IN] List of interfaces to clear statistics for.
 * \return error code.
 **/
vtss_rc dot1x_statistics_clr(i32 session_id, icli_stack_port_range_t *plist);

/**
 * \brief Function for configuring reauth_period_secs.
 *
 * \param session_id [IN] Needed for being able to print
 * \param value      [IN] New value for reauth_period_secs.
 * \param plist      [IN] List of interfaces to configure
 * \param no         [IN] TRUE to set to default.
 * \return error code.
 **/
vtss_rc dot1x_icli_reauthenticate(i32 session_id, i32 value, BOOL no);

/**
 * \brief Init function ICLI CFG
 * \return Error code.
 **/
vtss_rc dot1x_icfg_init(void);

/**
 * \brief Function for configuring eapol_timeout_secs
 *
 * \param session_id [IN]  Needed for being able to print
 * \param value [IN] New value for eapol_timeout_secs                  .
 * \param plist [IN] List of interfaces to configure.
 * \param no    [IN] TRUE to set to default.
 * \return error code.
 **/
vtss_rc dot1x_icli_tx_period(i32 session_id, i32 value, BOOL no);

/**
 * \brief Function for configuring inactivity timer
 *
 * \param session_id [IN]  Needed for being able to print
 * \param value [IN] New value for inactivity timer                            .
 * \param plist [IN] List of interfaces to configure.
 * \param no    [IN] TRUE to set to default.
 * \return error code.
 **/
vtss_rc dot1x_icli_inactivity(i32 session_id, i32 value, BOOL no);

/**
* \brief Function for enable/disable reauthentication
*
* \param session_id [IN] Needed for being able to print
* \param no         [IN] TRUE to set to default (disabled), FALSE to enable.
* \return None.
**/
vtss_rc dot1x_icli_reauthentication(i32 session_id, BOOL no);

/**
 * \brief Function for configure quiet period
 *
 * \param session_id [IN] Needed for being able to print
 * \param value      [IN] New quiet value
 * \param no         [IN] TRUE to set to default.
 * \return error code.
 **/
vtss_rc dot1x_icli_quiet_period(i32 session_id, i32 value, BOOL no);

/**
* \brief Function for start re-authenticate
* \param session_id [IN] Needed for being able to print
* \param plist      [IN] List of interfaces to start re-authenticate at.
* \return None.
**/
vtss_rc dot1x_re_authenticate(i32 session_id, icli_stack_port_range_t *plist);

/**
* \brief Function for force re-authenticate
* \param session_id [IN] Needed for being able to print
* \param plist      [IN] List of interfaces to force re-authenticate at.
* \return None.
**/
vtss_rc dot1x_initialize(i32 session_id, icli_stack_port_range_t *plist);

/**
* \brief Function for enable/disable system auth control
*
* \param session_id [IN] Needed for being able to print
* \param no         [IN] TRUE to set to default (disabled), FALSE to enable.
* \return None.
**/
vtss_rc dot1x_icli_system_auth_control(i32 session_id, BOOL no);

/**
 * \brief Runtime function determining if macbased dot1x is supported
 *
 * \param session_id [IN] Needed for being able to print
 * \param ask        [IN] Asking
 * \param runtime    [IN] Pointer to where to put the "answer"
 * \return TRUE is runtime contains valid information.
 **/
BOOL dot1x_icli_runtime_macbased(u32                session_id,
                                 icli_runtime_ask_t ask,
                                 icli_runtime_t     *runtime);

/**
 * \brief Runtime function determining if single dot1x is supported
 *
 * \param session_id [IN] Needed for being able to print
 * \param ask        [IN] Asking
 * \param runtime    [IN] Pointer to where to put the "answer"
 * \return TRUE is runtime contains valid information.
 **/
BOOL dot1x_icli_runtime_single(u32                session_id,
                               icli_runtime_ask_t ask,
                               icli_runtime_t     *runtime);

/**
 * \brief Runtime function determining if multi dot1x is supported
 *
 * \param session_id [IN] Needed for being able to print
 * \param ask        [IN] Asking
 * \param runtime    [IN] Pointer to where to put the "answer"
 * \return TRUE is runtime contains valid information.
 **/
BOOL dot1x_icli_runtime_multi(u32                session_id,
                              icli_runtime_ask_t ask,
                              icli_runtime_t     *runtime);

/**
 * \brief Function for configure dot1x port control
 *
 * \param session_id             [IN] Needed for being able to print
 * \param has_force_unauthorized [IN] TRUE if mode shall be set to forced unauthorized
 * \param has_force_authorized   [IN] TRUE if mode shall be set to forced authorized
 * \param has_auto               [IN] TRUE if mode shall be set to auto
 * \param has_single             [IN] TRUE if mode shall be set to single
 * \param has_multi              [IN] TRUE if mode shall be set to multi
 * \param has_macbased           [IN] TRUE if mode shall be set to mac-based
 * \param plist                  [IN] List of interfaces to configure
 * \param no                     [IN] TRUE to set to default.
 * \return error code.
 **/
vtss_rc dot1x_icli_port_contol(i32 session_id, BOOL has_force_unauthorized, BOOL has_force_authorized, BOOL has_auto, BOOL has_single, BOOL has_multi, BOOL has_macbased, icli_stack_port_range_t *plist, BOOL no);

/**
 * \brief Function for enable/disable guest vlan
 *
 * \param session_id [IN] Needed for being able to print
 * \param plist      [IN] List of interfaces to configure
 * \param no         [IN] TRUE to set to default (disabled), FALSE to enable.
 * \return error code.
 **/
vtss_rc dot1x_icli_interface_guest_vlan(i32 session_id, icli_stack_port_range_t *plist, BOOL no);

/**
 * \brief Function for configuring max reauth req
 *
 * \param session_id [IN] Needed for being able to print
 * \param value      [IN] New value for max_reauth_req.
 * \param no         [IN] TRUE to set to default.
 * \return error code.
 **/
vtss_rc  dot1x_icli_max_reauth_req(i32 session_id, i32 value, BOOL no);

/**
 * \brief Function for configuring guest vlan
 *
 * \param session_id [IN] Needed for being able to print
 * \param value      [IN] New value for guest vid.
 * \param no         [IN] TRUE to set to default.
 * \return error code.
 **/
vtss_rc  dot1x_icli_guest_vlan(i32 session_id, i32 value, BOOL no);

/**
* \brief Function for enable/disable guest vlan supplicant
*
* \param session_id [IN] Needed for being able to print
* \param no         [IN] TRUE to set to default (disabled), FALSE to enable.
* \return None.
**/
vtss_rc dot1x_icli_guest_vlan_supplicant(i32 session_id, BOOL no);

/**
 * \brief Function for enable/disable radius QOS
 *
 * \param session_id [IN] Needed for being able to print
 * \param plist      [IN] List of interfaces to configure
 * \param no         [IN] TRUE to set to default (disabled), FALSE to enable.
 * \return error code.
 **/
vtss_rc dot1x_icli_radius_qos(i32 session_id, icli_stack_port_range_t *plist, BOOL no);

/**
 * \brief Function for enable/disable radius VLAN
 *
 * \param session_id [IN] Needed for being able to print
 * \param plist      [IN] List of interfaces to configure
 * \param no         [IN] TRUE to set to default (disabled), FALSE to enable.
 * \return error code.
 **/
vtss_rc dot1x_icli_radius_vlan(i32 session_id, icli_stack_port_range_t *plist, BOOL no);

/**
 * \brief Function for configure global enable/disable for different features
 *
 * \param session_id      [IN] Needed for being able to print
 * \param has_guest_vlan  [IN] TRUE to configure guest vlan feature
 * \param has_radius_qos  [IN] TRUE to configure radius qos feature
 * \param has_radius_vlan [IN] TRUE to configure radius qos feature
 * \param no              [IN] TRUE to set to default (disabled), FALSE to enable.
 * \return error code.
 **/
vtss_rc dot1x_icli_global(i32 session_id, BOOL has_guest_vlan, BOOL has_radius_qos, BOOL has_radius_vlan, BOOL no);
