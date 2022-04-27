//==========================================================================
//
//      plf_io.c
//
//      HAL platform I/O functions
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    lpovlsen
// Contributors: 
// Date:         2011-10-18
// Purpose:      HAL I/O functions
// Description:  This file contains I/O functions provided by the
//               HAL.
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/system.h>
#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // Base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros
#include <cyg/infra/diag.h>             // diag_* functions
#include <cyg/hal/hal_arch.h>           // architectural definitions
#include <cyg/hal/hal_intr.h>           // Interrupt handling
#include <cyg/hal/hal_cache.h>          // Cache handling
#include <cyg/hal/hal_if.h>
#include <cyg/hal/plf_io.h>

cyg_uint32 vcoreiii_io_readl(volatile void *addr)
{
    return *((volatile cyg_uint32*)addr);
}

void vcoreiii_io_writel(cyg_uint32 val, volatile void *addr)
{
    *((volatile cyg_uint32*)addr) = val;
}

void vcoreiii_io_clr(cyg_uint32 mask, volatile void *addr)
{
    (*((volatile cyg_uint32*)addr)) &= ~mask;
}

void vcoreiii_io_set(cyg_uint32 mask, volatile void *addr)
{
    (*((volatile cyg_uint32*)addr)) |= mask;
}

void vcoreiii_io_mask_set(volatile void *addr, 
                          cyg_uint32 clr_mask, 
                          cyg_uint32 set_mask)
{
    cyg_uint32 val = (*((volatile cyg_uint32*)addr));
    val &= ~clr_mask;
    val |= set_mask;
    (*((volatile cyg_uint32*)addr)) = val;
}


