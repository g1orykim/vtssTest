#ifndef _VTSS_H_VCOREII_CLOCKADJ_
#define _VTSS_H_VCOREII_CLOCKADJ_

//=============================================================================
//
//      vcoreii_clockadj.h
//
//      Platform specific Hardware clock adjustment function
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
// Author(s):    Arne Kristoffersen
// Contributors:
// Date:         2009-06-18
// Purpose:      VCore-II Clock adjustment function
// Description:
// Usage:
//
//####DESCRIPTIONEND####
//
//=============================================================================
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H

#if defined(CYG_HAL_TIMER_ADJUSTMENT_SUPPORT)

/*
 * Inject completion callback
 */
typedef void (*timer_tick_cb)(void);

/**
 * \brief Enable the clock adjustment.
 * Enables or disables the clock adjustment feature
 *
 * \param enable [IN] != 0: enable clock adjustment.
 * \param callout [IN] pointer to call out function called from the timer tick interrupt.
 *                == NULL : no function callout is done.
 *
 */
void
hal_clock_enable_set_adjtimer(int enable, timer_tick_cb callout);

/**
 * \brief Change the timer tick period.
 * Adjust the clock period, erlative to the default clock period.
 *
 * \param delta_ppm [IN] period adjustment in ppm.
 * 						 > 0 : longer period, i.e. slower clock.
 * 						 < 0 : shorter period, i.e. faster clock.
 *
 */
void
hal_clock_set_adjtimer(int delta_ppm);

#endif
#endif  /* _VTSS_H_VCOREII_CLOCKADJ_ */
