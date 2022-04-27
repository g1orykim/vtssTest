#ifndef CYGONCE_VCOREIII_DIAG_H
#define CYGONCE_VCOREIII_DIAG_H

//=============================================================================
//
//      vcoreiii_diag.h
//
//      Platform header for HAL disgnostics
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
// Author(s):   lpovlsen
// Contributors:
// Date:        2010-11-23
// Purpose:     HAL POST diagnostics
// Usage:       #include <cyg/hal/vcoreiii_diag.h>
//              
//####DESCRIPTIONEND####
//
//=============================================================================
#include <pkgconf/hal.h>

enum {
    STATUS_LED_OFF,
    STATUS_LED_GREEN_ON,
    STATUS_LED_RED_ON,
    STATUS_LED_GREEN_BLINK1,
    STATUS_LED_RED_BLINK1,
    STATUS_LED_GREEN_BLINK2,
    STATUS_LED_RED_BLINK2,
};

externC void led_mode(int mode);

externC void gpio_out(int gpio);
externC void gpio_set(int gpio, int on);
externC void gpio_alt0(int gpio);

#if defined(CYG_HAL_VCOREIII_CHIPTYPE_LUTON26)
externC int vtss_phy_rd(cyg_uint32 miim_controller, cyg_uint8 miim_addr, cyg_uint8 addr, cyg_uint16 *value);
externC int vtss_phy_wr(cyg_uint32 miim_controller, cyg_uint8 miim_addr, cyg_uint8 addr, cyg_uint16 value);
externC void sgpio_port_bit_set(int port, int bit, int mode);
#endif

externC void cyg_plf_redboot_startup(void);

externC void vcoreiii_zero_mem(void *pStart, void *pEnd);

//-----------------------------------------------------------------------------
#endif // CYGONCE_VCOREIII_DIAG_H

