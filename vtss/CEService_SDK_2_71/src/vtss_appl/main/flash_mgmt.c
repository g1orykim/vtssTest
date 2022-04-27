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

#include "flash_mgmt_api.h"
#include "main.h"            /* For ARRSZ */
#include <cyg/hal/hal_io.h>

#ifdef VTSS_ARCH_LUTON28
  #define FLASH_LEGACY_SUPPORT
#else
  #undef  FLASH_LEGACY_SUPPORT
#endif

#ifdef FLASH_LEGACY_SUPPORT
  #define FLASH_PHYS_BASE                    VTSS_FLASH_TO
  #define FLASH_LEGACY_FIRST_USABLE_ADDR     (FLASH_PHYS_BASE + 0x00800000)

  #define FLASH_LEGACY_STACK_CONF_BASE_ADDR  (FLASH_LEGACY_FIRST_USABLE_ADDR)
  #define FLASH_LEGACY_STACK_CONF_SIZE_BYTES (17 * 128 * 1024)

  #define FLASH_LEGACY_SYSLOG_BASE_ADDR      (FLASH_LEGACY_STACK_CONF_BASE_ADDR + FLASH_LEGACY_STACK_CONF_SIZE_BYTES)
  #define FLASH_LEGACY_SYSLOG_SIZE_BYTES     (512 * 1024)

  #define FLASH_LEGACY_CONF_BASE_ADDR        (FLASH_PHYS_BASE + 0x00FC0000)
  #define FLASH_LEGACY_CONF_SIZE_BYTES       (1 * 128 * 1024)
#endif

typedef struct {
  // Static contents.
  char            *name;
#ifdef FLASH_LEGACY_SUPPORT
  cyg_flashaddr_t legacy_addr;
  size_t          legacy_size;
#endif

  // Variable contents
  BOOL            initialized;
  cyg_flashaddr_t actual_addr;
  size_t          actual_size;
} flash_mgmt_list_t;

static flash_mgmt_list_t flash_mgmt_list[] = {
  {
    .name        = "conf",
#ifdef FLASH_LEGACY_SUPPORT
    .legacy_addr = FLASH_LEGACY_CONF_BASE_ADDR,
    .legacy_size = FLASH_LEGACY_CONF_SIZE_BYTES,
#endif
  },
  {
    .name        = "stackconf",
#ifdef FLASH_LEGACY_SUPPORT
    .legacy_addr = FLASH_LEGACY_STACK_CONF_BASE_ADDR,
    .legacy_size = FLASH_LEGACY_STACK_CONF_SIZE_BYTES,
#endif
  },
  {
    .name        = "syslog",
#ifdef FLASH_LEGACY_SUPPORT
    .legacy_addr = FLASH_LEGACY_SYSLOG_BASE_ADDR,
    .legacy_size = FLASH_LEGACY_SYSLOG_SIZE_BYTES,
#endif
  },
};

/****************************************************************************/
// flash_mgmt_fis_lookup()
// Low-level lookup.
// Lookup a named FIS index named @section_name. Result will be stored in pEntry.
// Returns -1 on error, the entry number on success.
/****************************************************************************/
int flash_mgmt_fis_lookup(const char *section_name, struct fis_table_entry *pEntry)
{
  int i, max;

  max = CYGACC_CALL_IF_FLASH_FIS_OP2(CYGNUM_CALL_IF_FLASH_FIS_GET_ENTRY_COUNT, 0, NULL);
  for (i = 0; i < max; i++) {
    CYGACC_CALL_IF_FLASH_FIS_OP2(CYGNUM_CALL_IF_FLASH_FIS_GET_ENTRY, i, pEntry);
    if(pEntry->name[0] != 0xff && strcmp((const char *) pEntry->name, section_name) == 0) {
      return i;
    }
  }

  return -1;
}

/****************************************************************************/
// flash_mgmt_lookup()
// High-level lookup.
// Given a FIS section name, lookup the base address and size in flash.
// If it doesn't exist in flash, fall back to statically defined addresses
// if the flash's size permits it, and return TRUE. Otherwise return FALSE.
/****************************************************************************/
BOOL flash_mgmt_lookup(const char *section_name, flash_mgmt_section_info_t *info)
{
  int i;

  for(i = 0; i < ARRSZ(flash_mgmt_list); i++) {
    flash_mgmt_list_t *item = &flash_mgmt_list[i];

    if(strcmp(section_name, item->name) == 0) {
      if(!item->initialized) {
        struct fis_table_entry fis_entry;
        if(flash_mgmt_fis_lookup(section_name, &fis_entry) >= 0) {
          item->actual_addr = fis_entry.flash_base;
          item->actual_size = fis_entry.size;
        } else {
#ifdef FLASH_LEGACY_SUPPORT
          // No such section in flash. Use the legacy address.
          item->actual_addr = item->legacy_addr;
          item->actual_size = item->legacy_size;
#else
          // No such section in flash, and we don't default to a legacy address
          return FALSE;
#endif
        }
        item->initialized = TRUE;
      }
      info->base_fladdr  = item->actual_addr;
      info->size_bytes = item->actual_size;
      return TRUE;
    }
  }

  return FALSE;
}

