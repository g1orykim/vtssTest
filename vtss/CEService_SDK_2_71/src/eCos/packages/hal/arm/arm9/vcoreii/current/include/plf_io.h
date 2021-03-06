#ifndef CYGONCE_HAL_PLF_IO_H
#define CYGONCE_HAL_PLF_IO_H

//=============================================================================
//
//      plf_io.h
//
//      Platform specific registers
//
//=============================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under    
// the terms of the GNU General Public License as published by the Free     
// Software Foundation; either version 2 or (at your option) any later      
// version.                                                                 
//
// eCos is distributed in the hope that it will be useful, but WITHOUT      
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    
// for more details.                                                        
//
// You should have received a copy of the GNU General Public License        
// along with eCos; if not, write to the Free Software Foundation, Inc.,    
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.            
//
// As a special exception, if other files instantiate templates or use      
// macros or inline functions from this file, or you compile this file      
// and link it with other works to produce a work based on this file,       
// this file does not by itself cause the resulting work to be covered by   
// the GNU General Public License. However the source code for this file    
// must still be made available in accordance with section (3) of the GNU   
// General Public License v2.                                               
//
// This exception does not invalidate any other reasons why a work based    
// on this file might be covered by the GNU General Public License.         
// -------------------------------------------                              
// ####ECOSGPLCOPYRIGHTEND####                                              
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Lars Povlsen
// Contributors:
// Date:         2006-03-22
// Purpose:      Vitesse VCore-II platform specific registers
// Description: 
// Usage:        #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <cyg/hal/vcoreii.h>

#define CYGARC_PHYSICAL_ADDRESS(x) (x)

// This is used by the flash driver to get
// the address on which the flash is not cached.
// Since the MMU is configured not to cache the
// flash area, it's a one-to-one mapping.
// By having this macro, we can pursuade the
// AM29XXXXX_V2 driver to not disable the dcache
// while programming.
// Note that other drivers (e.g. eth) may use
// this macro, and if so, make sure that their
// address space is also uncached.
#ifndef CYGARC_UNCACHED_ADDRESS
  #define CYGARC_UNCACHED_ADDRESS(x) (x)
#endif

// ----------------------------------------------------------------------------
// exported I2C devices on OEM board
//
#define HAL_I2C_EXPORTED_DEVICES                                \
    extern cyg_i2c_bus            hal_vcore_i2c_bus;

//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_HAL_PLF_IO_H
