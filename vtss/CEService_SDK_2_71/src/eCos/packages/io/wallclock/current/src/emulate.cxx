//==========================================================================
//
//      io/wallclock/emulate.cxx
//
//      Wallclock emulation implementation
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
// Author(s):     nickg
// Contributors:  nickg
// Date:          1999-03-05
// Purpose:       Wallclock emulation
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/wallclock.h>          // Wallclock device config
#include <pkgconf/kernel.h>             // Kernel config

#include <cyg/infra/cyg_type.h>         // Common type definitions and support
#include <cyg/kernel/ktypes.h>

#include <cyg/io/wallclock.hxx>         // The WallClock API

#include <cyg/kernel/clock.hxx>         // Access to kernel clock
#include <cyg/kernel/clock.inl>


// Remember the tick count at the time of initialization.
static cyg_tick_count epoch_ticks;

//-----------------------------------------------------------------------------
// Called to initialize the hardware clock with a known valid
// time. Which of course isn't required when doing emulation using the
// kernel clock. But we set the private epoch base to "now".
void
Cyg_WallClock::init_hw_seconds( void )
{
    // Remember the current clock tick count.
    epoch_ticks = Cyg_Clock::real_time_clock->current_value();
}

//-----------------------------------------------------------------------------
// Returns the number of seconds elapsed since the init call.
cyg_uint32 
Cyg_WallClock::get_hw_seconds( void )
{
    Cyg_Clock::cyg_resolution res = 
        Cyg_Clock::real_time_clock->get_resolution();
    cyg_tick_count secs = Cyg_Clock::real_time_clock->current_value()
        - epoch_ticks;

    secs = secs / ( ( res.divisor * 1000000000LL ) / res.dividend );

    return secs;
}

//-----------------------------------------------------------------------------
// Returns the number of microseconds elapsed since the init call.
cyg_uint64 Cyg_WallClock::get_hw_usecs(void)
{
    // CYGNUM_HAL_RTC_NUMERATOR   = res.dividend               = nanoseconds per second = 1,000,000,000
    // CYGNUM_HAL_RTC_DENOMINATOR = res.divisor                = ticks per second       = e.g. 100
    // (CYGNUM_HAL_RTC_NUMERATOR / CYGNUM_HAL_RTC_DENOMINATOR) = (ns/s) / (ticks/s) = ns per tick.
    Cyg_Clock::cyg_resolution res         = Cyg_Clock::real_time_clock->get_resolution();
    cyg_uint64                usecs       = Cyg_Clock::real_time_clock->current_value() - epoch_ticks;
    cyg_uint64                us_per_tick = res.dividend / (1000LL * res.divisor);
    usecs = usecs * us_per_tick;
    return usecs;
}

//-----------------------------------------------------------------------------
// End of devs/wallclock/emulate.cxx
