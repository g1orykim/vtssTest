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

/**
 * \file
 * \brief Port icli functions
 * \details This header file describes port control functions
 */

#ifndef VTSS_ICLI_PORT_H
#define VTSS_ICLI_PORT_H

#include "icli_api.h"

/**
 * \brief Function for setting LED power reduction on event configuration
 *
 * \param session_id [IN]  Needed for being able to print error messages
 * \param has_link_change [IN]  TRUE to set the link change time.
 * \param v_0_to_65535 [IN] New time in seconds to turn LEDs 100% on. Only valid if has_link_change is TRUE.
 * \param error [IN]  TRUE to if LEDs shall be turned 100% on when errors occur.
 * \param no [IN]  TRUE to set the corresponding link_change or error configuration to default value.
 * \return Error code.
 **/
vtss_rc port_icli_statistics(i32 session_id, icli_stack_port_range_t *port_list, BOOL has_packets, BOOL has_bytes, BOOL has_errors,
                             BOOL has_discards, BOOL has_filtered, BOOL has_priority, icli_unsigned_range_t *priority_list, BOOL has_up, BOOL has_down);

/**
 * \brief Function for setting maximum frames size
 *
 * \param session_id [IN]  For printing error messages.
 * \param max_length [IN]  Maximum frame size
 * \param plist [IN]  Port list with ports to configure.
 * \param no [IN]  TRUE to set the maximum frame size to default
 * \return Error code.
 **/
vtss_rc port_icli_mtu(i32 session_id, u16 max_length, icli_stack_port_range_t *plist, BOOL no);

/**
 * \brief Function for enabling/disabling a port.
 *
 * \param session_id [IN]  For printing error messages.
 * \param plist [IN]  Port list with ports to configure.
 * \param no [IN]  TRUE to enable port. FALSE to disable port.
 * \return Error code.
 **/
vtss_rc port_icli_shutdown(i32 session_id, icli_stack_port_range_t *plist, BOOL no);

/**
 * \brief Function for configuring port speed
 *
 * \param session_id [IN]  For printing error messages.
 * \param plist [IN]  Port list with ports to configure.
 * \param has_speed_10g [IN]  TRUE to set port in 10G mode.
 * \param has_speed_2g5 [IN]  TRUE to set port in 2.5G mode.
 * \param has_speed_1g [IN]  TRUE to set port in 1G mode.
 * \param has_speed_100 [IN]  TRUE to set port in 100M mode.
 * \param has_speed_10 [IN]  TRUE to set port in 10M mode.
 * \param has_speed_auto [IN]  TRUE to set port in auto-neg mode.
 * \param has_neg_10 [IN]  TRUE to enable 10M auto-neg advertise
 * \param has_neg_100 [IN]  TRUE to enable 100M auto-neg advertise
 * \param has_neg_1000 [IN]  TRUE to enable 1G auto-neg advertise
 * \param no [IN]  TRUE to set speed to default.
 * \return Error code.
 **/
vtss_rc port_icli_speed(i32 session_id, icli_stack_port_range_t *plist, BOOL has_speed_10g, BOOL has_speed_2g5, BOOL has_speed_1g, BOOL has_speed_100m,
                        BOOL has_speed_10m, BOOL has_speed_auto, BOOL has_neg_10, BOOL has_neg_100, BOOL has_neg_1000, BOOL no);

/**
 * \brief Function for clearing port statistics
 *
 * \param session_id [IN]  For printing .
 * \param plist [IN]  Port list with ports to clear.
 * \return Error code.
 **/
vtss_rc port_icli_statistics_clear(i32 session_id, icli_stack_port_range_t *port_list);

/**
 * \brief Function for showing port capabilities
 *
 * \param session_id [IN]  For printing .
 * \param plist [IN]  Port list with ports to show.
 * \return Error code.
 **/
vtss_rc port_icli_capabilities(i32 session_id, icli_stack_port_range_t *port_list);

/**
 * \brief Function for showing port status
 *
 * \param session_id [IN]  For printing .
 * \param plist [IN]  Port list with ports to show.
 * \return Error code.
 **/
vtss_rc port_icli_status(i32 session_id, icli_stack_port_range_t *port_list);

/**
 * \brief Function for configuring media type.
 *
 * \param session_id [IN]  For printing .
 * \param plist [IN]  Port list with ports to configure.
 * \param has_rj45 [IN]  TRUE if port supports rj45 (cu port).
 * \param has_sfp [IN]  TRUE if port supports sfp (fiber port).
 * \param no [IN]  TRUE to set media type to default.
 * \return Error code.
 **/
vtss_rc port_icli_media_type(i32 session_id, icli_stack_port_range_t *plist, BOOL has_rj45, BOOL has_sfp, BOOL no);

/**
 * \brief Function for configuring duplex
 *
 * \param session_id [IN]  For printing .
 * \param plist [IN]  Port list with ports to configure.
 * \param has_half [IN]  TRUE if port duplex mode to forced half duplex.
 * \param has_full [IN]  TRUE if port duplex mode to forced full duplex.
 * \param has_auto [IN]  TRUE if port duplex mode to auto negotiation.
 * \param has_advertise_hdx [IN]  TRUE to advertise half duplex in auto negotiation mode
 * \param has_advertise_fdx [IN]  TRUE to advertise full duplex in auto negotiation mode
 * \param no [IN]  TRUE to set duplex to default.
 * \return Error code.
 **/
vtss_rc port_icli_duplex(i32 session_id, icli_stack_port_range_t *plist, BOOL has_half, BOOL has_full, BOOL has_auto, BOOL has_advertise_hdx, BOOL has_advertise_fdx, BOOL no);

/**
 * \brief Function for configuring flow control
 *
 * \param session_id [IN]  For printing .
 * \param plist [IN]  Port list with ports to configure.
 * \param has_receive [IN]  TRUE to configure rx flow control.
 * \param has_send [IN] TRUE to configure tx flow control.
 * \param has_on [IN]  TRUE to enable the rx/tx flow control.
 * \param has_off [IN]  TRUE to disable the rx/tx flow control.
 * \param no [IN]  TRUE to set media type to default.
 * \return Error code.
 **/
vtss_rc port_icli_flow_control(i32 session_id, icli_stack_port_range_t *plist, BOOL has_receive, BOOL has_send, BOOL has_on, BOOL has_off, BOOL no);

/**
 * \brief Function for configuring excessive collisions
 *
 * \param session_id [IN]  For printing .
 * \param plist [IN]  Port list with ports to configure.
 * \param no [IN]  TRUE to set media type to default.
 * \return Error code.
 **/
vtss_rc port_icli_excessive_restart(i32 session_id, icli_stack_port_range_t *plist, BOOL no);

/**
 * \brief Function for running and showing VeriPHY
 *
 * \param session_id [IN]  For printing .
 * \param plist [IN]  Port list with ports to configure.
 * \return Error code.
 **/
vtss_rc port_icli_veriphy(i32 session_id, icli_stack_port_range_t *port_list);

/**
 * \brief Init function ICLI CFG
 * \return Error code.
 **/
vtss_rc port_icfg_init(void);

/**
 * \brief Function for at runtime getting information about QOS queues
 *
 * \param session_id [IN] The session id use by iCLI print.
 * \param ask [IN]  Asking
 * \param runtime [IN]  Pointer to where to put the "answer"
 * \return TRUE is runtime contains valid information.
 **/
BOOL port_icli_runtime_qos_queues(u32                session_id,
                                  icli_runtime_ask_t ask,
                                  icli_runtime_t     *runtime);

/**
 * \brief Function for at runtime getting information if the stack supports 10G
 *
 * \param session_id [IN] The session id use by iCLI print.
 * \param ask        [IN] Asking
 * \param runtime    [IN] Pointer to where to put the "answer"
 * \return TRUE is runtime contains valid information.
 **/
BOOL port_icli_runtime_10g(u32                session_id,
                           icli_runtime_ask_t ask,
                           icli_runtime_t     *runtime);

/**
 * \brief Function for at runtime getting information if the stack supports 2.5G
 *
 * \param session_id [IN] The session id use by iCLI print.
 * \param ask        [IN] Asking
 * \param runtime    [IN] Pointer to where to put the "answer"
 * \return TRUE is runtime contains valid information.
 **/
BOOL port_icli_runtime_2g5(u32                session_id,
                           icli_runtime_ask_t ask,
                           icli_runtime_t     *runtime);


#endif /* VTSS_ICLI_PORT_H */



/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
