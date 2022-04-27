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
 * \brief vcl icli functions
 * \details This header file describes vcl icli functions
 */

#ifndef VTSS_ICLI_VCL_H
#define VTSS_ICLI_VCL_H

#include "icli_api.h"

/**
 * \brief Function for showing vcl mac status
 * \param session_id  [IN] For ICLI_PRINTF
 * \param has_address [IN] TRUE to look up a specific MAC address entry. If FALSE all MAC address entries are looked up.
 * \param mac_addr    [IN] Only valid when has_address is TRUE. The MAC address to look up.

 **/
vtss_rc vcl_icli_show_vlan(const i32 session_id, const BOOL has_address, const vtss_mac_t *mac_addr);

/**
 * \brief Function for showing vcl ip-subnet status
 * \param session_id  [IN] For ICLI_PRINTF
 * \param has_address [IN] TRUE to look up a specific subnet id address entry. If FALSE all subnet entries are looked up.
 * \param mac_addr    [IN] Only valid when has_id is TRUE. The subnet to look up.

 **/
vtss_rc vcl_icli_show_ipsubnet(const i32 session_id, const BOOL has_id, const u8 subnet_id);
#endif /* VTSS_ICLI_VCL_H */



/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
