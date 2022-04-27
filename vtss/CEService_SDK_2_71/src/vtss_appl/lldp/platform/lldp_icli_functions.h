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

/**
 * \file
 * \brief LLDP iCLI functions
 * \details This header file describes LLDP iCLI functions
 */


#ifndef _VTSS_ICLI_LLDP_H_
#define _VTSS_ICLI_LLDP_H_

#include "icli_api.h"

/**
 * \brief Function for showing lldp status (chip temperature and port status)
 *
 * \param session_id      [IN] The session id.
 * \param show_neighbors  [IN] TRUE if neighbor information shall be printed
 * \param show_statistics [IN] TRUE if LLDP statistics shall be printed
 * \param has_interface   [IN] TRUE if user has specified a specific interface
 * \param port_list       [IN]  Port list in case user has specified a specific interface.
 * \return error code
 **/
vtss_rc lldp_icli_status(i32 session_id, BOOL show_neighbors, BOOL show_statistics, BOOL has_interface, icli_stack_port_range_t *list);

/**
 * \brief Function for setting lldp global configuration (hold time, interval, delay etc.)
 *
 * \param session_id         [IN] The session id for printing.
 * \param holdtime           [IN] TRUE when setting holdtime value.
 * \param timer              [IN] TRUE when setting the tx interval
 * \param reinit             [IN] TRUE when setting reinit.
 * \param transmission_delay [IN] TRUE when setting the tx delay configuration.
 * \param new_val            [IN] The new value to be set.
 * \param no                 [IN] TRUE if the no command is use
 * \return error code
 **/
vtss_rc lldp_icli_global_conf(i32 session_id, BOOL holdtime, BOOL timer, BOOL reinit, BOOL transmission_delay, u16 new_val, BOOL no);

/**
 * \brief Function for setting priority for ports
 *
 * \param prio_list [IN] Port list
 * \param prio      [IN] Priority to be set for the ports in the port list.
 * \param no        [IN] TRUE if the no command is used
 * \return error code
 **/
vtss_rc lldp_icli_prio(BOOL interface, icli_stack_port_range_t *port_list, u8 prio, BOOL no);


/**
 * \brief Function for enabling/disabling TX and RX mode.
 *
 * \param session_id [IN] The session id for printing.
 * \param port_list  [IN] Port list with ports to configure.
 * \param tx         [IN] TRUE if we shall transmit LLDP frames.
 * \param rx         [IN] TRUE if we shall add LLDP information received from neighbors into the entry table.
 * \param no         [IN] TRUE if the no command is used
 * \return error code
 **/
vtss_rc lldp_icli_mode(i32 session_id, icli_stack_port_range_t *port_list, BOOL tx, BOOL rx, BOOL no);

/**
 * \brief Function for setting when optional TLVs to transmit to the neighbors.
 *
 * \param session_id [IN] The session id for printing.
 * \param plist      [IN] Port list with ports to configure.
 * \param mgmt       [IN] TRUE when management TLV shall be transmitted to neighbors.
 * \param port       [IN] TRUE when port TLV shall be transmitted to neighbors.
 * \param sys_capa   [IN] TRUE when system capabilities TLV shall be transmitted to neighbors.
 * \param sys_des    [IN] TRUE when system description TLV shall be transmitted to neighbors.
 * \param sys_name   [IN] TRUE when system name TLV shall be transmitted to neighbors.
 * \param no         [IN] TRUE if the no command is use
 * \return error_code.
 **/
vtss_rc lldp_icli_tlv_select(i32 session_id, icli_stack_port_range_t *plist, BOOL mgmt, BOOL port, BOOL sys_capa, BOOL sys_des, BOOL sys_name, BOOL no);

/**
 * \brief Function for configuring CDP awareness
 *
 * \param session_id [IN] The session id for printing.
 * \param port_list  [IN] Port list with ports to configure.
 * \param no         [IN] TRUE if the no command is used
 * \return error code
 **/
vtss_rc lldp_icli_cdp(i32 session_id, icli_stack_port_range_t *plist, BOOL no);

/**
 * \brief Function for clearing the LLDP statistic.
 */
void lldp_icli_clear_counters(void);

/**
 * \brief Init function ICLI CFG
 * \return Error code.
 **/
vtss_rc lldp_icfg_init(void);
#endif /* _VTSS_ICLI_LLDP_H_ */
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
