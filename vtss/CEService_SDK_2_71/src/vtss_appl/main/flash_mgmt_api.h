/*

 Vitesse Switch API software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
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

#ifndef _FLASH_MGMT_API_H_
#define _FLASH_MGMT_API_H_

#include <network.h>
#include <cyg/hal/hal_if.h>
#include <cyg/io/flash.h>
#include <vtss_types.h>

/****************************************************************************/
/****************************************************************************/
typedef struct {
    cyg_flashaddr_t base_fladdr; /* For flash read/write access */
    size_t          size_bytes;
} flash_mgmt_section_info_t;

/****************************************************************************/
// flash_mgmt_fis_lookup()
// Low-level lookup.
// Lookup a named FIS index named @section_name. Result will be stored in pEntry.
// Returns -1 on error, the entry number on success.
/****************************************************************************/
int flash_mgmt_fis_lookup(const char *section_name, struct fis_table_entry *pEntry);

/****************************************************************************/
// flash_mgmt_lookup()
// High-level lookup.
// Given a FIS section name, lookup the base address and size in flash.
// If it doesn't exist in flash, fall back to statically defined addresses
// if the flash's size permits it, and return TRUE. Otherwise return FALSE.
/****************************************************************************/
BOOL flash_mgmt_lookup(const char *section_name, flash_mgmt_section_info_t *info);

#endif /* _FLASH_MGMT_API_H_ */

//***************************************************************************
// 
//  End of file.
// 
//***************************************************************************
