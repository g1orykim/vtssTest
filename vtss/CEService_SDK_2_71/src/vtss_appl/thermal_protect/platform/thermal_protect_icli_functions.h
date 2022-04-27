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
 * \brief mirror icli functions
 * \details This header file describes PHY control functions
 */


#ifndef _VTSS_ICLI_THERMAL_PROTECT_H_
#define _VTSS_ICLI_THERMAL_PROTECT_H_

#include "icli_api.h"


/**
 * \brief Function for showing thermal_protect status (chip temperature and port status)
 *
 * \param session_id [IN]  The session id.
 * \param interface [IN]  TRUE if user has specified a specific interface(s)
 * \param port_list [IN]  Port list
 * \return None.
 **/
void thermal_protect_status(i32 session_id, BOOL interface, icli_stack_port_range_t *port_list);


/**
 * \brief Function for setting thermal_protect temperature for a specific priority
 *
 *
 * \param prio_list [IN]  List of priorities
 * \param new_temp [IN] New temperature for when to shut port down for the given priority(is)
 * \param no [IN] TRUE if the no command is used
 * \return Error code.
 **/
vtss_rc thermal_protect_temp(i32 session_id, icli_unsigned_range_t *prio_list, i8 new_temp, BOOL no);


/**
 * \brief Function for setting priority for ports
 *
 *
 * \param prio_list [IN]  Port list
 * \param prio [IN] Priority to be set for the ports in the port list.
 * \param no [IN] TRUE if the no command is used
 * \return None.
 **/
void thermal_protect_prio(icli_stack_port_range_t *port_list, u8 prio, BOOL no);

/**
 * \brief Init function ICLI CFG
 * \return Error code.
 **/
vtss_rc thermal_protect_icfg_init(void);
#endif /* _VTSS_ICLI_THERMAL_PROTECT_H_ */



/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
