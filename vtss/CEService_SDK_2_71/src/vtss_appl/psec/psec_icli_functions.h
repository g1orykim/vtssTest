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
 * \brief PSEC icli functions
 * \details This header file describes PSEC iCLI functions
 */

/**
 * \brief Function for printing Port Security status.
 *
 * \param session_id [IN]  Needed for being able to print
 * \param plist [IN] List of interfaces to print statistics for.
 * \return Error code.
 **/
vtss_rc psec_icli_show_port(i32 session_id, icli_stack_port_range_t *plist);

/**
 * \brief Function for printing MAC Addresses learned by Port Security.
 *
 * \param session_id [IN]  Needed for being able to print
 * \param plist [IN] List of interfaces to print statistics for.
 * \return Error code.
 **/
vtss_rc psec_icli_show_switch(i32 session_id, icli_stack_port_range_t *plist);
