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
 * \brief iCLI shared functions by LLDP and LLDP-MED
 * \details This header file describes LLDP shared iCLI functions
 */


#ifndef _VTSS_ICLI_SHARED_LLDP_H_
#define _VTSS_ICLI_SHARED_LLDP_H_

/**
 * \brief Function for getting local port as printable text
 *
 * \param  buf   [IN] Pointer to text buffer.
 * \param  entry [IN] Pointer to entry containing the information
 * \param  sit   [IN] Pointer to switch information'
 * \param  pit   [IN] Pointer to port information
 * return  Pointer to text buffer
 **/
i8 *lldp_local_interface_txt_get(i8 *buf, const lldp_remote_entry_t *entry, const switch_iter_t *sit, const port_iter_t *pit);


#endif /* _VTSS_ICLI_SHARED_LLDP_H_ */
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
