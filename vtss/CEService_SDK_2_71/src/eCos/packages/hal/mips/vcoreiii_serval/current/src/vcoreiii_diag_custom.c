//==========================================================================
//
//      vcoreiii_diag.c
//
//      Misc diagnostics (Customizable parts)
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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
// Author(s):    Lars Povlsen
// Contributors:
// Date:         2010-11-23
// Purpose:      Misc diagnostics (customizable parts)
// Description:
//
//####DESCRIPTIONEND####

#include <redboot.h>
#include <cyg/hal/plf_io.h>
#include <pkgconf/hal.h>
#include <cyg/hal/vcoreiii_diag.h>

/*
 * The following implementation is intended for customization support
 * for customer specific boards.
 */

#if 0
void led_mode(int mode)
{
    cyg_uint32 chipid = (VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_ID >> 12) & 0xFFFF;
    diag_printf("Chipid: %04x *** Custom\n", chipid);
}
#endif
