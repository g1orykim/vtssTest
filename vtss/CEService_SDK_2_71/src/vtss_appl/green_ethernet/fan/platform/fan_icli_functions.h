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
 * \details This header file describes FAN iCLI
 */


#ifndef _VTSS_ICLI_FAN_H_
#define _VTSS_ICLI_FAN_H_

#include "icli_api.h"


/**
 * \brief Function for showing fan status (chip temperature and fan speed)
 *
 * \param session_id [IN]  The session id.
 * \return None.
 **/
void fan_status(i32 session_id);


/**
 * \brief Function for setting fan configuration
 *
 * \param session_id [IN]  Needed for being able to print error messages
 * \param t_on [IN]  TRUE to set the t_on parameter. If FALSE the t_max parameter is set
 * \param new_temp [IN] New temperature for either t_on or t_max
 * \param no [IN] TRUE if t_on/t_max shall be set to default value
 * \return None.
 **/
void fan_temp(u32 session_id, BOOL t_on, i8 new_temp, BOOL no);


/**
 * \brief Init function ICLI CFG
 * \return Error code.
 **/
vtss_rc fan_icfg_init(void);
#endif /* _VTSS_ICLI_FAN_H_ */



/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
