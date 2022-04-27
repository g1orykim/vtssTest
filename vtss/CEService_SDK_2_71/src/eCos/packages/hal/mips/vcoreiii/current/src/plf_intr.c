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
    [CYGNUM_HAL_INTERRUPT_EXT0]       = "EXT0",
    [CYGNUM_HAL_INTERRUPT_EXT1]       = "EXT1",
    [CYGNUM_HAL_INTERRUPT_SW0]        = "SW0",
    [CYGNUM_HAL_INTERRUPT_SW1]        = "SW1",
    [CYGNUM_HAL_INTERRUPT_PI_SD0]     = "PI_SD0",
    [CYGNUM_HAL_INTERRUPT_PI_SD1]     = "PI_SD1",
    [CYGNUM_HAL_INTERRUPT_UART]       = "UART",
    [CYGNUM_HAL_INTERRUPT_TIMER0]     = "TIMER0",
    [CYGNUM_HAL_INTERRUPT_TIMER1]     = "TIMER1",
    [CYGNUM_HAL_INTERRUPT_TIMER2]     = "TIMER2",
    [CYGNUM_HAL_INTERRUPT_FDMA]       = "FDMA",
    [CYGNUM_HAL_INTERRUPT_TWI]        = "TWI",
    [CYGNUM_HAL_INTERRUPT_GPIO]       = "GPIO",
    [CYGNUM_HAL_INTERRUPT_SGPIO]      = "SGPIO",
    [CYGNUM_HAL_INTERRUPT_DEV_ALL]    = "DEV_ALL",
    [CYGNUM_HAL_INTERRUPT_BLK_ANA]    = "BLK_ANA",
    [CYGNUM_HAL_INTERRUPT_XTR_RDY0]   = "XTR_RDY0",
    [CYGNUM_HAL_INTERRUPT_XTR_RDY1]   = "XTR_RDY1",
    [CYGNUM_HAL_INTERRUPT_XTR_RDY2]   = "XTR_RDY2",
    [CYGNUM_HAL_INTERRUPT_XTR_RDY3]   = "XTR_RDY3",
    [CYGNUM_HAL_INTERRUPT_INJ_RDY0]   = "INJ_RDY0",
    [CYGNUM_HAL_INTERRUPT_INJ_RDY1]   = "INJ_RDY1",
    [CYGNUM_HAL_INTERRUPT_INJ_RDY2]   = "INJ_RDY2",
    [CYGNUM_HAL_INTERRUPT_INJ_RDY3]   = "INJ_RDY3",
    [CYGNUM_HAL_INTERRUPT_INJ_RDY4]   = "INJ_RDY4",
    [CYGNUM_HAL_INTERRUPT_INTEGRITY]  = "INTEGRITY",
    [CYGNUM_HAL_INTERRUPT_PTP_SYNC]   = "PTP_SYNC",
    [CYGNUM_HAL_INTERRUPT_MIIM0_INTR] = "MIIM0_INTR",
    [CYGNUM_HAL_INTERRUPT_MIIM1_INTR] = "MIIM1_INTR",
#if defined(CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR)
    [CYGNUM_HAL_INTERRUPT_SEC_EXT1]       = "SEC_EXT1",
    [CYGNUM_HAL_INTERRUPT_SEC_SW0]        = "SEC_SW0",
    [CYGNUM_HAL_INTERRUPT_SEC_SW1]        = "SEC_SW1",
    [CYGNUM_HAL_INTERRUPT_SEC_PI_SD0]     = "SEC_PI",
    [CYGNUM_HAL_INTERRUPT_SEC_PI_SD1]     = "SEC_PI",
    [CYGNUM_HAL_INTERRUPT_SEC_UART]       = "SEC_UART",
    [CYGNUM_HAL_INTERRUPT_SEC_TIMER0]     = "SEC_TIMER0",
    [CYGNUM_HAL_INTERRUPT_SEC_TIMER1]     = "SEC_TIMER1",
    [CYGNUM_HAL_INTERRUPT_SEC_TIMER2]     = "SEC_TIMER2",
    [CYGNUM_HAL_INTERRUPT_SEC_FDMA]       = "SEC_FDMA",
    [CYGNUM_HAL_INTERRUPT_SEC_TWI]        = "SEC_TWI",
    [CYGNUM_HAL_INTERRUPT_SEC_GPIO]       = "SEC_GPIO",
    [CYGNUM_HAL_INTERRUPT_SEC_SGPIO]      = "SEC_SGPIO",
    [CYGNUM_HAL_INTERRUPT_SEC_DEV_ALL]    = "SEC_DEV",
    [CYGNUM_HAL_INTERRUPT_SEC_BLK_ANA]    = "SEC_BLK",
    [CYGNUM_HAL_INTERRUPT_SEC_XTR_RDY0]   = "SEC_XTR",
    [CYGNUM_HAL_INTERRUPT_SEC_XTR_RDY1]   = "SEC_XTR",
    [CYGNUM_HAL_INTERRUPT_SEC_XTR_RDY2]   = "SEC_XTR",
    [CYGNUM_HAL_INTERRUPT_SEC_XTR_RDY3]   = "SEC_XTR",
    [CYGNUM_HAL_INTERRUPT_SEC_INJ_RDY0]   = "SEC_INJ",
    [CYGNUM_HAL_INTERRUPT_SEC_INJ_RDY1]   = "SEC_INJ",
    [CYGNUM_HAL_INTERRUPT_SEC_INJ_RDY2]   = "SEC_INJ",
    [CYGNUM_HAL_INTERRUPT_SEC_INJ_RDY3]   = "SEC_INJ",
    [CYGNUM_HAL_INTERRUPT_SEC_INJ_RDY4]   = "SEC_INJ",
    [CYGNUM_HAL_INTERRUPT_SEC_INTEGRITY]  = "SEC_INTEGRITY",
    [CYGNUM_HAL_INTERRUPT_SEC_PTP_SYNC]   = "SEC_PTP",
    [CYGNUM_HAL_INTERRUPT_SEC_MIIM0_INTR] = "SEC_MIIM0",
    [CYGNUM_HAL_INTERRUPT_SEC_MIIM1_INTR] = "SEC_MIIM1",
#endif  /* CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR */
};    

#if defined(CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR) && defined(CYGPKG_KERNEL)

typedef cyg_uint32 cyg_ISR(cyg_uint32 vector, CYG_ADDRWORD data);

extern void cyg_interrupt_post_dsr( CYG_ADDRWORD intr_obj );

static inline cyg_uint32
hal_call_isr (cyg_uint32 vector)
{
    cyg_ISR *isr;
    CYG_ADDRWORD data;
    cyg_uint32 isr_ret;

    isr = (cyg_ISR*) hal_interrupt_handlers[vector];
    data = hal_interrupt_data[vector];

    isr_ret = (*isr) (vector, data);

#ifdef CYGFUN_HAL_COMMON_KERNEL_SUPPORT
    if (isr_ret & CYG_ISR_CALL_DSR) {
        cyg_interrupt_post_dsr (hal_interrupt_objects[vector]);
    }
#endif

    return isr_ret & ~CYG_ISR_CALL_DSR;
}

externC cyg_uint32
hal_secondary_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data)
{
    cyg_uint32 irqstat = vcoreiii_io_sec_readl(&VTSS_ICPU_CFG_INTR_EXT_IRQ0_IDENT);
    if(irqstat) {
        cyg_uint32 ret, vector;
        HAL_MSBIT_INDEX(vector, irqstat);
        vector += SEC_IRQ_BASE;
#ifdef CYG_HAL_IRQCOUNT_SUPPORT
        hal_interrupt_counts[vector]++;
#endif /* CYG_HAL_IRQCOUNT_SUPPORT */
        ret = hal_call_isr (vector);
        if (ret & CYG_ISR_HANDLED)
            return ret;
    }

    return 0;
}
#endif  /* CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR && CYGPKG_KERNEL */

void hal_interrupt_configure(
    cyg_vector_t        vector,
    cyg_bool_t          edge,
    cyg_bool_t          activehigh)
{
    if(vector == CYGNUM_HAL_INTERRUPT_EXT0) {
        cyg_uint32 cfg = VTSS_ICPU_CFG_INTR_EXT_IRQ0_INTR_CFG;
        if(activehigh)
            cfg |= VTSS_F_ICPU_CFG_INTR_EXT_IRQ0_INTR_CFG_EXT_IRQ0_INTR_POL;
        else
            cfg &= ~VTSS_F_ICPU_CFG_INTR_EXT_IRQ0_INTR_CFG_EXT_IRQ0_INTR_POL;
#ifdef VTSS_F_ICPU_CFG_INTR_EXT_IRQ0_INTR_CFG_EXT_IRQ0_INTR_TRIGGER
        if(edge)
            cfg |= VTSS_F_ICPU_CFG_INTR_EXT_IRQ0_INTR_CFG_EXT_IRQ0_INTR_TRIGGER;
        else
            cfg &= ~VTSS_F_ICPU_CFG_INTR_EXT_IRQ0_INTR_CFG_EXT_IRQ0_INTR_TRIGGER;
#endif
        VTSS_ICPU_CFG_INTR_EXT_IRQ0_INTR_CFG = cfg;
    } else if(vector == CYGNUM_HAL_INTERRUPT_EXT1) {
        cyg_uint32 cfg = VTSS_ICPU_CFG_INTR_EXT_IRQ1_INTR_CFG;
        if(activehigh)
            cfg |= VTSS_F_ICPU_CFG_INTR_EXT_IRQ1_INTR_CFG_EXT_IRQ1_INTR_POL;
        else
            cfg &= ~VTSS_F_ICPU_CFG_INTR_EXT_IRQ1_INTR_CFG_EXT_IRQ1_INTR_POL;
#ifdef VTSS_F_ICPU_CFG_INTR_EXT_IRQ1_INTR_CFG_EXT_IRQ1_INTR_TRIGGER
        if(edge)
            cfg |= VTSS_F_ICPU_CFG_INTR_EXT_IRQ1_INTR_CFG_EXT_IRQ1_INTR_TRIGGER;
        else
            cfg &= ~VTSS_F_ICPU_CFG_INTR_EXT_IRQ1_INTR_CFG_EXT_IRQ1_INTR_TRIGGER;
#endif
        VTSS_ICPU_CFG_INTR_EXT_IRQ1_INTR_CFG = cfg;
    }
}

void cyg_hal_interrupt_mask(cyg_uint32 vector)
{
    if(vector < PRI_IRQ_BASE) {
        HAL_INTERRUPT_MASK_CPU(vector);
    } else if(vector <= CYGNUM_HAL_INTERRUPT_MIIM1_INTR) {
        cyg_uint32 __vec = vector - PRI_IRQ_BASE;
        VTSS_ICPU_CFG_INTR_INTR_ENA_CLR = (1 << __vec);
#if defined(CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR)
    } else if(vcoreiii_has_secondary) {
        cyg_uint32 __vec = vector - SEC_IRQ_BASE;
        vcoreiii_io_sec_writel((1 << __vec), &VTSS_ICPU_CFG_INTR_INTR_ENA_CLR);
#endif  /* CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR */
    }
}

// The FDMA must not have interrupts ack'd just before unmask because
// of a bug in the Synopsys GPDMA when running with CCM frames (the
// GPDMA interrupt goes up and down without software intervention)
// See Bugzilla#6045 and Bugzilla#6089.
void cyg_hal_interrupt_unmask(cyg_uint32 vector)
{
    if(vector < PRI_IRQ_BASE) {
        HAL_INTERRUPT_UNMASK_CPU(vector);
    } else if(vector <= CYGNUM_HAL_INTERRUPT_MIIM1_INTR) {
        cyg_uint32 __vec = vector - PRI_IRQ_BASE;
        if (vector != CYGNUM_HAL_INTERRUPT_FDMA) {
            /* Note: The below ACK will ruin edge triggered IRQ! */
            VTSS_ICPU_CFG_INTR_INTR = (1 << __vec);
        }
        VTSS_ICPU_CFG_INTR_INTR_ENA_SET = (1 << __vec);
#if defined(CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR)
    } else if(vcoreiii_has_secondary) {
        cyg_uint32 __vec = vector - SEC_IRQ_BASE;
        if (vector != CYGNUM_HAL_INTERRUPT_SEC_FDMA) {
            /* Note: The below ACK will ruin edge triggered IRQ! */
            vcoreiii_io_sec_writel((1 << __vec), &VTSS_ICPU_CFG_INTR_INTR);
        }
        vcoreiii_io_sec_writel((1 << __vec), &VTSS_ICPU_CFG_INTR_INTR_ENA_SET);
#endif  /* CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR */
    }
}

void cyg_hal_interrupt_acknowledge(cyg_uint32 vector)
{
    if(vector < PRI_IRQ_BASE) {
        HAL_INTERRUPT_ACKNOWLEDGE_CPU(vector);
    } else if(vector <= CYGNUM_HAL_INTERRUPT_MIIM1_INTR) {
        cyg_uint32 __vec = vector - PRI_IRQ_BASE;
        VTSS_ICPU_CFG_INTR_INTR = (1 << __vec);
#if defined(CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR)
    } else if(vcoreiii_has_secondary) {
        cyg_uint32 __vec = vector - SEC_IRQ_BASE;
        vcoreiii_io_sec_writel((1 << __vec), &VTSS_ICPU_CFG_INTR_INTR);
        VTSS_ICPU_CFG_INTR_INTR = VTSS_F_ICPU_CFG_INTR_INTR_EXT_IRQ1_INTR;
#endif  /* CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR */
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

    /* Enable Routing of Jaguar IRQs to IRQ0 */
    VTSS_ICPU_CFG_INTR_ICPU_IRQ0_ENA = VTSS_F_ICPU_CFG_INTR_ICPU_IRQ0_ENA_ICPU_IRQ0_ENA;

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

#if defined(CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR)
void hal_init_irq_secondary(void)
{
#if defined(CYGPKG_KERNEL)
    vcoreiii_io_sec_writel(0xFFFFFFFF, &VTSS_ICPU_CFG_INTR_INTR_ENA_CLR);

    /* Be sure to mask while setting up */
    cyg_hal_interrupt_mask  (CYGNUM_HAL_INTERRUPT_EXT1);

    /* EXT1-primary (input) is connected to EXT0-secondary (output) */
    /* Secondary EXT0 is output, active low */
    vcoreiii_io_sec_writel(VTSS_F_ICPU_CFG_INTR_EXT_IRQ0_INTR_CFG_EXT_IRQ0_INTR_DRV | 
                           VTSS_F_ICPU_CFG_INTR_EXT_IRQ0_INTR_CFG_EXT_IRQ0_INTR_DIR,
                           &VTSS_ICPU_CFG_INTR_EXT_IRQ0_INTR_CFG);
    /* Primary EXT1 is input, active low */
    VTSS_ICPU_CFG_INTR_EXT_IRQ1_INTR_CFG = 0;
        
    /* Route EXT_IRQ1 (input) and all internal interrupts to EXT_IRQ0 (output). */

#define JR_INTR_CFG_EXTERNAL(intr) vcoreiii_io_sec_writel(VTSS_F_ICPU_CFG_INTR_##intr##_INTR_CFG_##intr##_INTR_SEL(2), &VTSS_ICPU_CFG_INTR_##intr##_INTR_CFG) /* External, active low input, to EXT_IRQ0 */

#define JR_INTR_CFG_INTERNAL(intr) vcoreiii_io_sec_writel(VTSS_F_ICPU_CFG_INTR_##intr##_INTR_CFG_##intr##_INTR_SEL(2), &VTSS_ICPU_CFG_INTR_##intr##_INTR_CFG) /* Internal, to EXT_IRQ0 */

    /* EXT_IRQ1 is an input on the secondary chip. Route it to EXT_IRQ0. */
    JR_INTR_CFG_EXTERNAL(EXT_IRQ1);
        
    /* Remaining interrupts are also routed to EXT_IRQ0. */
    JR_INTR_CFG_INTERNAL(SW0);
    JR_INTR_CFG_INTERNAL(SW1);
    JR_INTR_CFG_INTERNAL(MIIM1);
    JR_INTR_CFG_INTERNAL(MIIM0);
    JR_INTR_CFG_INTERNAL(PI_SD0);
    JR_INTR_CFG_INTERNAL(PI_SD1);
    JR_INTR_CFG_INTERNAL(UART);
    JR_INTR_CFG_INTERNAL(TIMER0);
    JR_INTR_CFG_INTERNAL(TIMER1);
    JR_INTR_CFG_INTERNAL(TIMER2);
    JR_INTR_CFG_INTERNAL(FDMA);
    JR_INTR_CFG_INTERNAL(TWI);
    JR_INTR_CFG_INTERNAL(GPIO);
    JR_INTR_CFG_INTERNAL(SGPIO);
    JR_INTR_CFG_INTERNAL(DEV_ALL);
    JR_INTR_CFG_INTERNAL(BLK_ANA);
    JR_INTR_CFG_INTERNAL(XTR_RDY0);
    JR_INTR_CFG_INTERNAL(XTR_RDY1);
    JR_INTR_CFG_INTERNAL(XTR_RDY2);
    JR_INTR_CFG_INTERNAL(XTR_RDY3);
    JR_INTR_CFG_INTERNAL(INJ_RDY0);
    JR_INTR_CFG_INTERNAL(INJ_RDY1);
    JR_INTR_CFG_INTERNAL(INJ_RDY2);
    JR_INTR_CFG_INTERNAL(INJ_RDY3);
    JR_INTR_CFG_INTERNAL(INJ_RDY4);
    JR_INTR_CFG_INTERNAL(INTEGRITY);
    JR_INTR_CFG_INTERNAL(PTP_SYNC);
        
    /* GPIO Alternate functions EXT1(primary) <-> EXT0(secondary) */
    vcoreiii_gpio_set_alternate(7, 1); /* Primary GPIO7 = EXT1 */
    vcoreiii_gpio_set_alternate_sec(6, 1); /* Secondary GPIO6 = EXT0 */
        
    /* Enable EXT0 to drive it */
    vcoreiii_io_sec_writel(VTSS_F_ICPU_CFG_INTR_EXT_IRQ0_ENA_EXT_IRQ0_ENA, 
                           &VTSS_ICPU_CFG_INTR_EXT_IRQ0_ENA);

    vcoreiii_io_sec_writel(0, &VTSS_ICPU_CFG_INTR_INTR_ENA); /* Mask all */
    vcoreiii_io_sec_writel(0xffffffff, &VTSS_ICPU_CFG_INTR_INTR); /* Ack pending */

    HAL_INTERRUPT_ATTACH    (CYGNUM_HAL_INTERRUPT_EXT1, &hal_secondary_isr, 0, 0);

    /* Unmask main EXT "umbrella" IRQ */
    cyg_hal_interrupt_unmask(CYGNUM_HAL_INTERRUPT_EXT1);
#endif  /* CYGPKG_KERNEL */
}
#endif	/* CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR */
