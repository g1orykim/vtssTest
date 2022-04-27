/*

 Vitesse Switch Software.

 Copyright (c) 2002-2010 Vitesse Semiconductor Corporation "Vitesse". All
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

*/
#ifndef CYGONCE_PLF_MISC_H
#define CYGONCE_PLF_MISC_H

/*=============================================================================
//
//      plf_misc.h
//
//      HAL Support for Board dependent functions
//
//=============================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 2009 Free Software Foundation, Inc.
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
// Date:        2009-09-03
// Purpose:     HAL Support for Board dependent functions
// Description: Misc board support
// Usage:       #include <cyg/hal/plf_misc.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#ifdef __ASSEMBLER__
#define C0_INX          $0 /* Index into TLB Array - 4Kc core */
#define C0_RANDOM       $1 /* Randomly generated index into TLB Array - 4Kc core */
#define C0_TLBLO0       $2 /* Low-order portion of the TLB entry for even-numbered virtual pages - 4Kc core */
#define C0_TLBLO1       $3 /* Low-order portion of the TLB entry for odd-numbered virtual pages - 4Kc core */
#define C0_PAGEMASK     $5 /* Pointer to page table entry in memory - 4Kc core */
#define C0_WIRED        $6 /* Number of fixed TLB entries - 4Kc core */
#define C0_TLBHI        $10 /* High-order portion of the TLB entry - 4Kc core */
#define C0_PRId         $15 /* Processor Identification and Revision */
#else
  // The following function is not available in RedBoot
  #ifdef CYGPKG_KERNEL
    // Perform a cool restart, i.e. first read addr_to_persist, then reset switch core,
    // then write back the previously read value to addr_to_persist, and finally
    // reset the CPU. After the system reboots, it can read the addr_to_persist
    // to figure out whether this was a power-on-reset or a software-initiated reset.
    void hal_cool_restart(cyg_uint32 *addr_to_persist);

    // ----------------------------------------------------------------------------
    // Test case exit support.
    extern void (*vtss_system_reset_hook)(void);

    #define CYGHWR_TEST_PROGRAM_EXIT()           \
     CYG_MACRO_START                             \
     if(vtss_system_reset_hook)                  \
         vtss_system_reset_hook();               \
     CYG_MACRO_END

  #endif

#if defined(CYG_HAL_TIMER_ADJUSTMENT_SUPPORT) && defined(CYGPKG_KERNEL)

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

void vcoreiii_gpio_set_alternate(int gpio, int is_alternate);
void vcoreiii_gpio_set_alternate2(int gpio, int is_alternate);
void vcoreiii_gpio_set_input(int gpio, int is_input);
void vcoreiii_gpio_set_output_level(int gpio, int output_level);

/* For guarding simultaneous SPI acesses */
void vcoreiii_spi_bus_lock(void);
void vcoreiii_spi_bus_unlock(void);

#endif

//-----------------------------------------------------------------------------
// end of plf_misc.h
#endif // CYGONCE_PLF_MISC_H
