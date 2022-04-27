//==========================================================================
//
//      plf_intr.c
//
//      HAL platform interrupt functions
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
// Purpose:      HAL interrupt functions
// Description:  This file contains interrupt functions provided by the
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

const char * const hal_interrupt_name[CYGNUM_HAL_ISR_COUNT] = {
    [CYGNUM_HAL_INTERRUPT_HW0]        = "MIPS_HW0",
    [CYGNUM_HAL_INTERRUPT_HW1]        = "MIPS_HW1",
    [CYGNUM_HAL_INTERRUPT_HW2]        = "MIPS_HW2",
    [CYGNUM_HAL_INTERRUPT_HW3]        = "MIPS_HW3",
    [CYGNUM_HAL_INTERRUPT_HW4]        = "MIPS_HW4",
    [CYGNUM_HAL_INTERRUPT_HW5]        = "MIPS_COMPARE",
    [CYGNUM_HAL_INTERRUPT_DEV_ALL]    = "DEV_ALL",
    [CYGNUM_HAL_INTERRUPT_EXT0]       = "EXT0",
    [CYGNUM_HAL_INTERRUPT_EXT1]       = "EXT1",
    [CYGNUM_HAL_INTERRUPT_TIMER0]     = "TIMER0",
    [CYGNUM_HAL_INTERRUPT_TIMER1]     = "TIMER1",
    [CYGNUM_HAL_INTERRUPT_TIMER2]     = "TIMER2",
    [CYGNUM_HAL_INTERRUPT_UART]       = "UART",
    [CYGNUM_HAL_INTERRUPT_UART2]      = "UART2",
    [CYGNUM_HAL_INTERRUPT_TWI]        = "TWI",
    [CYGNUM_HAL_INTERRUPT_SW0]        = "SW0",
    [CYGNUM_HAL_INTERRUPT_SW1]        = "SW1",
    [CYGNUM_HAL_INTERRUPT_SGPIO]      = "SGPIO",
    [CYGNUM_HAL_INTERRUPT_GPIO]       = "GPIO",
    [CYGNUM_HAL_INTERRUPT_MIIM0_INTR] = "MIIM0_INTR",
    [CYGNUM_HAL_INTERRUPT_MIIM1_INTR] = "MIIM1_INTR",
    [CYGNUM_HAL_INTERRUPT_FDMA]       = "FDMA",
    [CYGNUM_HAL_INTERRUPT_OAM_MEP]    = "OAM_MEP",
    [CYGNUM_HAL_INTERRUPT_PTP_RDY]    = "PTP_RDY",
    [CYGNUM_HAL_INTERRUPT_PTP_SYNC]   = "PTP_SYNC",
    [CYGNUM_HAL_INTERRUPT_INTEGRITY]  = "INTEGRITY",
    [CYGNUM_HAL_INTERRUPT_XTR_RDY0]   = "XTR_RDY0",
    [CYGNUM_HAL_INTERRUPT_XTR_RDY1]   = "XTR_RDY1",
    [CYGNUM_HAL_INTERRUPT_INJ_RDY0]   = "INJ_RDY0",
    [CYGNUM_HAL_INTERRUPT_INJ_RDY1]   = "INJ_RDY1",
    [CYGNUM_HAL_INTERRUPT_PCIE]       = "PCIE",
};    

void hal_interrupt_configure(
    cyg_vector_t        vector,
    cyg_bool_t          edge,
    cyg_bool_t          activehigh)
{
}

void cyg_hal_interrupt_mask(cyg_uint32 vector)
{
    if(vector < PRI_IRQ_BASE) {
        HAL_INTERRUPT_MASK_CPU(vector);
    } else {
        cyg_uint32 __vec = vector - PRI_IRQ_BASE;
        VTSS_ICPU_CFG_INTR_INTR_ENA_CLR = (1 << __vec);
    }
}

void cyg_hal_interrupt_unmask(cyg_uint32 vector)
{
    if(vector < PRI_IRQ_BASE) {
        HAL_INTERRUPT_UNMASK_CPU(vector);
    } else {
        cyg_uint32 __vec = vector - PRI_IRQ_BASE;
        /* Note: The below ACK will ruin edge triggered IRQ! */
        VTSS_ICPU_CFG_INTR_INTR_STICKY  = (1 << __vec);
        VTSS_ICPU_CFG_INTR_INTR_ENA_SET = (1 << __vec);
    }
}

void cyg_hal_interrupt_acknowledge(cyg_uint32 vector)
{
    if(vector < PRI_IRQ_BASE) {
        HAL_INTERRUPT_ACKNOWLEDGE_CPU(vector);
    } else {
        cyg_uint32 __vec = vector - PRI_IRQ_BASE;
        VTSS_ICPU_CFG_INTR_INTR_STICKY = (1 << __vec);
    }
}

//--------------------------------------------------------------------------
// IRQ init

static cyg_uint32
hal_spurious_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    return 0;
}

void
hal_init_irq(void)
{
    /* Disable all Jaguar IRQs (to IRQ0) */
    VTSS_ICPU_CFG_INTR_INTR_ENA = 0;

    /* Enable Routing of IRQs to IRQ0 */
    VTSS_ICPU_CFG_INTR_DST_INTR_MAP(0) = VTSS_BITMASK(25); /* MIPS_HW0 */
    VTSS_ICPU_CFG_INTR_DST_INTR_MAP(1) = 0; /* MIPS_HW1 */
    VTSS_ICPU_CFG_INTR_DST_INTR_MAP(2) = 0; /* EXT0 */
    VTSS_ICPU_CFG_INTR_DST_INTR_MAP(3) = 0; /* EXT1 */

    /* Soak up spurious IRQs */
    HAL_INTERRUPT_ATTACH (CYGNUM_HAL_INTERRUPT_HW0, &hal_spurious_isr, 0, 0);

    /* VCoreIII IRQs attached via HW0 */
    HAL_INTERRUPT_UNMASK_CPU( CYGNUM_HAL_INTERRUPT_HW0 );    
}

#ifdef CYG_HAL_IRQCOUNT_SUPPORT
cyg_uint64 hal_irqcount_read (cyg_vector_t vector)
{
    cyg_uint64 count = (cyg_uint64) -1;
    if(vector < CYGNUM_HAL_ISR_COUNT) {
        CYG_INTERRUPT_STATE old;
        HAL_DISABLE_INTERRUPTS(old);
        count = hal_interrupt_counts[vector];
        HAL_RESTORE_INTERRUPTS(old);
    }
    return count;
}

void hal_irqcount_clear(cyg_vector_t vector)
{
    if(vector < CYGNUM_HAL_ISR_COUNT && vector != CYGNUM_HAL_INTERRUPT_TIMER0) {
        CYG_INTERRUPT_STATE old;
        HAL_DISABLE_INTERRUPTS(old);
        hal_interrupt_counts[vector] = 0;
        HAL_RESTORE_INTERRUPTS(old);
    }
}
#endif /* CYG_HAL_IRQCOUNT_SUPPORT */
