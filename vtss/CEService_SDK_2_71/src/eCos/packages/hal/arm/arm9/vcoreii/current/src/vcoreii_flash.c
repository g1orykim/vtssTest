/*

 Vitesse Switch Software.

 #####ECOSGPLCOPYRIGHTBEGIN#####
 -------------------------------------------
 This file is part of eCos, the Embedded Configurable Operating System.
 Copyright (C) 1998-2012 Free Software Foundation, Inc.

 eCos is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free
 Software Foundation; either version 2 or (at your option) any later
 version.

 eCos is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.

 You should have received a copy of the GNU General Public License
 along with eCos; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 As a special exception, if other files instantiate templates or use
 macros or inline functions from this file, or you compile this file
 and link it with other works to produce a work based on this file,
 this file does not by itself cause the resulting work to be covered by
 the GNU General Public License. However the source code for this file
 must still be made available in accordance with section (3) of the GNU
 General Public License v2.

 This exception does not invalidate any other reasons why a work based
 on this file might be covered by the GNU General Public License.
 -------------------------------------------
 #####ECOSGPLCOPYRIGHTEND#####

*/
#include <pkgconf/system.h>
#ifdef CYGPKG_DEVS_FLASH_AMD_AM29XXXXX_V2

//--------------------------------------------------------------------------
// Device properties

#include <cyg/io/flash.h>
#include <cyg/io/flash_dev.h>
#include <cyg/io/am29xxxxx_dev.h>
#include <cyg/hal/vcoreii.h> /* For VCOREII_FLASH_PHYS_BASE */

static const CYG_FLASH_FUNS(
  hal_vcoreii_flash_amd_funs,                                                // _funs_
  &cyg_am29xxxxx_init_cfi_16as8,                                             // _init_
  &cyg_flash_devfn_query_nop,                                                // _query_
  &cyg_am29xxxxx_erase_16as8,                                                // _erase_
  &cyg_am29xxxxx_program_16as8,                                              // _prog_
  (int (*)(struct cyg_flash_dev *, const cyg_flashaddr_t, void *, size_t))0, // _read_
  &cyg_flash_devfn_lock_nop,                                                 // _lock_
  &cyg_flash_devfn_unlock_nop);                                              // _unlock_

// Provides a place holder for the flash device info (devid, blockinfo).
// This info is queried dynamically using the cyg_am29xxxxx_init_cfi_16as8()
// function before use of any other function.
// Other drivers declare this structure const, but the documentation
// recommends not to when using CFI, since the linker may place
// it in read-only memory (well, we have no read-only memory on vcoreii,
// but anyway...).
static cyg_am29xxxxx_dev hal_vcoreii_flash_priv;

CYG_FLASH_DRIVER(
  hal_vcoreii_flash,                 // _name_
  &hal_vcoreii_flash_amd_funs,       // _funs_
  0,                                 // _flags_
  VCOREII_FLASH_PHYS_BASE,           // _start_ (in principle the virtual address, but we use a one-to-one-mapping)
  0,                                 // _end_
  0,                                 // _num_block_infos_
  hal_vcoreii_flash_priv.block_info, // _block_info_
  &hal_vcoreii_flash_priv            // _priv_
);
#endif

