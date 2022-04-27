#ifndef CYGONCE_HAL_PLATFORM_INTS_H
#define CYGONCE_HAL_PLATFORM_INTS_H

//==========================================================================
//
//      hal_platform_ints.h
//
//      HAL Interrupt and clock support
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Lars Povlsen
// Contributors:
// Date:         2006-03-22
// Purpose:      Define Interrupt support
// Description:  The interrupt details for the Vitesse VCore-II ARM926EJ-S are defined here.
// Usage:
//               #include <cyg/hal/hal_platform_ints.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

// These are interrupts on the ARM926EJS board

#define CYGNUM_HAL_INT_I2C                    7
#define CYGNUM_HAL_INT_FDMA                   6
#define CYGNUM_HAL_INT_TIMER_2                5
#define CYGNUM_HAL_INT_TIMER_1                4
#define CYGNUM_HAL_INT_TIMER_0                3
#define CYGNUM_HAL_INT_UART                   2
#define CYGNUM_HAL_INT_PI_1                   1
#define CYGNUM_HAL_INT_PI_0                   0

#define CYGNUM_HAL_INTERRUPT_NONE    -1

#define CYGNUM_HAL_ISR_MIN            0
#define CYGNUM_HAL_ISR_MAX            (CYGNUM_HAL_INT_I2C)

#define CYGNUM_HAL_ISR_COUNT          (CYGNUM_HAL_ISR_MAX-CYGNUM_HAL_ISR_MIN+1)

// The vector used by the Real time clock
#define CYGNUM_HAL_INTERRUPT_RTC      CYGNUM_HAL_INT_TIMER_0

//----------------------------------------------------------------------------
// Microsecond delay support.

externC void hal_delay_us(cyg_uint32 usecs);
#define HAL_DELAY_US(n)          hal_delay_us(n)

//----------------------------------------------------------------------------
// Reset.

//----------------------------------------------------------------------------
// Reset.
externC void hal_reset(void);
#define HAL_PLATFORM_RESET() hal_reset()
#define HAL_PLATFORM_RESET_ENTRY VCOREII_FLASH_PHYS_BASE

#endif // CYGONCE_HAL_PLATFORM_INTS_H
