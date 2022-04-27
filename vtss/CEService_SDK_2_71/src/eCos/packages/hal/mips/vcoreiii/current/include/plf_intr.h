#ifndef CYGONCE_HAL_PLF_INTR_H
#define CYGONCE_HAL_PLF_INTR_H

//==========================================================================
//
//      plf_intr.h
//
//      VCore-III Interrupt and clock support
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    lpovlsen
// Contributors: (based on MIPS Malta code)
// Date:         2000-09-01
// Purpose:      Define Interrupt support
// Description:  The macros defined here provide the HAL APIs for handling
//               interrupts and the clock for the VCore-III board.
//              
// Usage:
//              #include <cyg/hal/plf_intr.h>
//              ...
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/plf_misc.h>

// First an assembly safe part

//--------------------------------------------------------------------------
// Interrupt vectors.

#ifndef CYGHWR_HAL_INTERRUPT_VECTORS_DEFINED

// These are decoded via the IP bits of the cause
// register when an external interrupt is delivered.

// Internal to MIPS
#define CYGNUM_HAL_INTERRUPT_HW0           0
#define CYGNUM_HAL_INTERRUPT_HW1           1
#define CYGNUM_HAL_INTERRUPT_HW2           2
#define CYGNUM_HAL_INTERRUPT_HW3           3
#define CYGNUM_HAL_INTERRUPT_HW4           4
#define CYGNUM_HAL_INTERRUPT_HW5           5

// Via IRQ0
#define PRI_IRQ_BASE                       (CYGNUM_HAL_INTERRUPT_HW5 + 1) /* 6 */
#define PRI_ICPU_IRQ0(x)                   (PRI_IRQ_BASE + (x))
#define CYGNUM_HAL_INTERRUPT_EXT0          PRI_ICPU_IRQ0(0)
#define CYGNUM_HAL_INTERRUPT_EXT1          PRI_ICPU_IRQ0(1)
#define CYGNUM_HAL_INTERRUPT_SW0           PRI_ICPU_IRQ0(2)
#define CYGNUM_HAL_INTERRUPT_SW1           PRI_ICPU_IRQ0(3)
#define CYGNUM_HAL_INTERRUPT_PI_SD0        PRI_ICPU_IRQ0(4)
#define CYGNUM_HAL_INTERRUPT_PI_SD1        PRI_ICPU_IRQ0(5)
#define CYGNUM_HAL_INTERRUPT_UART          PRI_ICPU_IRQ0(6)
#define CYGNUM_HAL_INTERRUPT_TIMER(_tmr_)  PRI_ICPU_IRQ0(7 + (_tmr_))
#define CYGNUM_HAL_INTERRUPT_TIMER0        PRI_ICPU_IRQ0(7)
#define CYGNUM_HAL_INTERRUPT_TIMER1        PRI_ICPU_IRQ0(8)
#define CYGNUM_HAL_INTERRUPT_TIMER2        PRI_ICPU_IRQ0(9)
#define CYGNUM_HAL_INTERRUPT_FDMA          PRI_ICPU_IRQ0(10)
#define CYGNUM_HAL_INTERRUPT_TWI           PRI_ICPU_IRQ0(11)
#define CYGNUM_HAL_INTERRUPT_GPIO          PRI_ICPU_IRQ0(12)
#define CYGNUM_HAL_INTERRUPT_SGPIO         PRI_ICPU_IRQ0(13)
#define CYGNUM_HAL_INTERRUPT_DEV_ALL       PRI_ICPU_IRQ0(14)
#define CYGNUM_HAL_INTERRUPT_BLK_ANA       PRI_ICPU_IRQ0(15)
#define CYGNUM_HAL_INTERRUPT_XTR_RDY0      PRI_ICPU_IRQ0(16)
#define CYGNUM_HAL_INTERRUPT_XTR_RDY1      PRI_ICPU_IRQ0(17)
#define CYGNUM_HAL_INTERRUPT_XTR_RDY2      PRI_ICPU_IRQ0(18)
#define CYGNUM_HAL_INTERRUPT_XTR_RDY3      PRI_ICPU_IRQ0(19)
#define CYGNUM_HAL_INTERRUPT_INJ_RDY0      PRI_ICPU_IRQ0(20)
#define CYGNUM_HAL_INTERRUPT_INJ_RDY1      PRI_ICPU_IRQ0(21)
#define CYGNUM_HAL_INTERRUPT_INJ_RDY2      PRI_ICPU_IRQ0(22)
#define CYGNUM_HAL_INTERRUPT_INJ_RDY3      PRI_ICPU_IRQ0(23)
#define CYGNUM_HAL_INTERRUPT_INJ_RDY4      PRI_ICPU_IRQ0(24)
#define CYGNUM_HAL_INTERRUPT_INTEGRITY     PRI_ICPU_IRQ0(25)
#define CYGNUM_HAL_INTERRUPT_PTP_SYNC      PRI_ICPU_IRQ0(26)
#define CYGNUM_HAL_INTERRUPT_MIIM0_INTR    PRI_ICPU_IRQ0(27)
#define CYGNUM_HAL_INTERRUPT_MIIM1_INTR    PRI_ICPU_IRQ0(28) /* 34 */

#if defined(CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR)
// Via (optional) slave device on SEC_EXT0 -> PRI_EXT1
#define SEC_IRQ_BASE            (CYGNUM_HAL_INTERRUPT_MIIM1_INTR + 1) /* 35 */
#define SEC_ICPU_IRQ0(x)        (SEC_IRQ_BASE + (x))
//#define CYGNUM_HAL_INTERRUPT_SEC_EXT0        SEC_ICPU_IRQ0(0) - reserved
#define CYGNUM_HAL_INTERRUPT_SEC_EXT1          SEC_ICPU_IRQ0(1)
#define CYGNUM_HAL_INTERRUPT_SEC_SW0           SEC_ICPU_IRQ0(2)
#define CYGNUM_HAL_INTERRUPT_SEC_SW1           SEC_ICPU_IRQ0(3)
#define CYGNUM_HAL_INTERRUPT_SEC_PI_SD0        SEC_ICPU_IRQ0(4)
#define CYGNUM_HAL_INTERRUPT_SEC_PI_SD1        SEC_ICPU_IRQ0(5)
#define CYGNUM_HAL_INTERRUPT_SEC_UART          SEC_ICPU_IRQ0(6)
#define CYGNUM_HAL_INTERRUPT_SEC_TIMER(_tmr_)  SEC_ICPU_IRQ0(7 + (_tmr_))
#define CYGNUM_HAL_INTERRUPT_SEC_TIMER0        SEC_ICPU_IRQ0(7)
#define CYGNUM_HAL_INTERRUPT_SEC_TIMER1        SEC_ICPU_IRQ0(8)
#define CYGNUM_HAL_INTERRUPT_SEC_TIMER2        SEC_ICPU_IRQ0(9)
#define CYGNUM_HAL_INTERRUPT_SEC_FDMA          SEC_ICPU_IRQ0(10)
#define CYGNUM_HAL_INTERRUPT_SEC_TWI           SEC_ICPU_IRQ0(11)
#define CYGNUM_HAL_INTERRUPT_SEC_GPIO          SEC_ICPU_IRQ0(12)
#define CYGNUM_HAL_INTERRUPT_SEC_SGPIO         SEC_ICPU_IRQ0(13)
#define CYGNUM_HAL_INTERRUPT_SEC_DEV_ALL       SEC_ICPU_IRQ0(14)
#define CYGNUM_HAL_INTERRUPT_SEC_BLK_ANA       SEC_ICPU_IRQ0(15)
#define CYGNUM_HAL_INTERRUPT_SEC_XTR_RDY0      SEC_ICPU_IRQ0(16)
#define CYGNUM_HAL_INTERRUPT_SEC_XTR_RDY1      SEC_ICPU_IRQ0(17)
#define CYGNUM_HAL_INTERRUPT_SEC_XTR_RDY2      SEC_ICPU_IRQ0(18)
#define CYGNUM_HAL_INTERRUPT_SEC_XTR_RDY3      SEC_ICPU_IRQ0(19)
#define CYGNUM_HAL_INTERRUPT_SEC_INJ_RDY0      SEC_ICPU_IRQ0(20)
#define CYGNUM_HAL_INTERRUPT_SEC_INJ_RDY1      SEC_ICPU_IRQ0(21)
#define CYGNUM_HAL_INTERRUPT_SEC_INJ_RDY2      SEC_ICPU_IRQ0(22)
#define CYGNUM_HAL_INTERRUPT_SEC_INJ_RDY3      SEC_ICPU_IRQ0(23)
#define CYGNUM_HAL_INTERRUPT_SEC_INJ_RDY4      SEC_ICPU_IRQ0(24)
#define CYGNUM_HAL_INTERRUPT_SEC_INTEGRITY     SEC_ICPU_IRQ0(25)
#define CYGNUM_HAL_INTERRUPT_SEC_PTP_SYNC      SEC_ICPU_IRQ0(26)
#define CYGNUM_HAL_INTERRUPT_SEC_MIIM0_INTR    SEC_ICPU_IRQ0(27)
#define CYGNUM_HAL_INTERRUPT_SEC_MIIM1_INTR    SEC_ICPU_IRQ0(28)
#endif

// The vector used by the Real time clock
#define CYGNUM_HAL_INTERRUPT_RTC           CYGNUM_HAL_INTERRUPT_TIMER0

// Min/Max ISR numbers
#define CYGNUM_HAL_ISR_MIN                 0
#if defined(CYGNUM_HAL_INTERRUPT_SEC_MIIM1_INTR)
#define CYGNUM_HAL_ISR_MAX                 CYGNUM_HAL_INTERRUPT_SEC_MIIM1_INTR
#else
#define CYGNUM_HAL_ISR_MAX                 CYGNUM_HAL_INTERRUPT_MIIM1_INTR
#endif

#define CYGNUM_HAL_ISR_COUNT               (CYGNUM_HAL_ISR_MAX - CYGNUM_HAL_ISR_MIN + 1)

#define CYGHWR_HAL_INTERRUPT_VECTORS_DEFINED
#endif // CYGHWR_HAL_INTERRUPT_VECTORS_DEFINED

//--------------------------------------------------------------------------
#ifndef __ASSEMBLER__

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/plf_io.h>
#include <cyg/hal/drv_api.h>

externC void hal_interrupt_configure(
    cyg_vector_t        vector,
    cyg_bool_t          edge,
    cyg_bool_t          up);

//--------------------------------------------------------------------------
// Interrupt controller access.

//--------------------------------------------------------------------------
// Interrupt controller access
// The default code here simply uses the fields present in the CP0 status
// and cause registers to implement this functionality.
// Beware of nops in this code. They fill delay slots and avoid CP0 hazards
// that might otherwise cause following code to run in the wrong state or
// cause a resource conflict.

#ifndef CYGHWR_HAL_INTERRUPT_CONTROLLER_ACCESS_DEFINED

#define HAL_INTERRUPT_MASK_CPU( _vector_ )      \
CYG_MACRO_START                                 \
    asm volatile (                              \
        "mfc0   $3,$12\n"                       \
        "la     $2,0x00000400\n"                \
        "sllv   $2,$2,%0\n"                     \
        "nor    $2,$2,$0\n"                     \
        "and    $3,$3,$2\n"                     \
        "mtc0   $3,$12\n"                       \
        "nop; nop; nop\n"                       \
        :                                       \
        : "r"(_vector_)                         \
        : "$2", "$3"                            \
        );                                      \
CYG_MACRO_END

#define HAL_INTERRUPT_UNMASK_CPU( _vector_ )    \
CYG_MACRO_START                                 \
    asm volatile (                              \
        "mfc0   $3,$12\n"                       \
        "la     $2,0x00000400\n"                \
        "sllv   $2,$2,%0\n"                     \
        "or     $3,$3,$2\n"                     \
        "mtc0   $3,$12\n"                       \
        "nop; nop; nop\n"                       \
        :                                       \
        : "r"(_vector_)                         \
        : "$2", "$3"                            \
        );                                      \
CYG_MACRO_END

#define HAL_INTERRUPT_ACKNOWLEDGE_CPU( _vector_ )       \
CYG_MACRO_START                                 \
    asm volatile (                              \
        "mfc0   $3,$13\n"                       \
        "la     $2,0x00000400\n"                \
        "sllv   $2,$2,%0\n"                     \
        "nor    $2,$2,$0\n"                     \
        "and    $3,$3,$2\n"                     \
        "mtc0   $3,$13\n"                       \
        "nop; nop; nop\n"                       \
        :                                       \
        : "r"(_vector_)                         \
        : "$2", "$3"                            \
        );                                      \
CYG_MACRO_END

#define HAL_INTERRUPT_CONFIGURE( _vector_, _level_, _up_ )      \
    hal_interrupt_configure(_vector_, !_level_, _up_)

/* Not available/possible on this platform */
#define HAL_INTERRUPT_SET_LEVEL( _vector_, _level_ )

externC void cyg_hal_interrupt_mask(cyg_uint32 vector);
externC void cyg_hal_interrupt_unmask(cyg_uint32 vector);
externC void cyg_hal_interrupt_acknowledge(cyg_uint32 vector);

#define HAL_INTERRUPT_MASK( _vector_ )          \
    CYG_MACRO_START                             \
    cyg_hal_interrupt_mask ( (_vector_) );	\
    CYG_MACRO_END

#define HAL_INTERRUPT_UNMASK( _vector_ )        \
    CYG_MACRO_START                             \
    cyg_hal_interrupt_unmask ( (_vector_) );    \
    CYG_MACRO_END

#define HAL_INTERRUPT_ACKNOWLEDGE( _vector_ )             \
    CYG_MACRO_START                                       \
    cyg_hal_interrupt_acknowledge ( (_vector_) );         \
    CYG_MACRO_END

#define CYGHWR_HAL_INTERRUPT_CONTROLLER_ACCESS_DEFINED

#else

#warning Interrupt control overridden!?!?

#endif


//--------------------------------------------------------------------------
// Control-C support.

#if defined(CYGDBG_HAL_MIPS_DEBUG_GDB_CTRLC_SUPPORT)

# define CYGHWR_HAL_GDB_PORT_VECTOR CYGNUM_HAL_INTERRUPT_SER

externC cyg_uint32 hal_ctrlc_isr(CYG_ADDRWORD vector, CYG_ADDRWORD data);

# define HAL_CTRLC_ISR hal_ctrlc_isr

#endif


//----------------------------------------------------------------------------
// Reset.
#ifndef CYGHWR_HAL_RESET_DEFINED
externC void hal_vcoreiii_reset( void );
externC void hal_vcoreiii_cpu_reset( void );
#define CYGHWR_HAL_RESET_DEFINED
#define HAL_PLATFORM_RESET()             hal_vcoreiii_reset()

#define HAL_PLATFORM_RESET_ENTRY 0xbfc00000

#endif // CYGHWR_HAL_RESET_DEFINED

//--------------------------------------------------------------------------
// Clock control

#ifndef CYGHWR_HAL_CLOCK_CONTROL_DEFINED

externC void hal_clock_initialize(cyg_uint32);
externC void hal_clock_read(cyg_uint32 *);
externC void hal_clock_reset(cyg_uint32, cyg_uint32);

#define HAL_CLOCK_INITIALIZE( _period_ )   hal_clock_initialize( _period_ )
#define HAL_CLOCK_RESET( _vec_, _period_ ) hal_clock_reset( _vec_, _period_ )
#define HAL_CLOCK_READ( _pvalue_ )         hal_clock_read( _pvalue_ )

#ifdef CYGVAR_KERNEL_COUNTERS_CLOCK_LATENCY
# ifndef HAL_CLOCK_LATENCY
#  define HAL_CLOCK_LATENCY( _pvalue_ )    HAL_CLOCK_READ( (cyg_uint32 *)_pvalue_ )
# endif
#endif

#define CYGHWR_HAL_CLOCK_CONTROL_DEFINED

#endif

#define CYG_HAL_NAMES_SUPPORT
externC const char * const hal_interrupt_name[CYGNUM_HAL_ISR_COUNT];

#ifdef CYG_HAL_IRQCOUNT_SUPPORT
externC cyg_uint64     hal_interrupt_counts[CYGNUM_HAL_ISR_COUNT];
externC cyg_uint64     hal_irqcount_read (cyg_vector_t vector);
externC void           hal_irqcount_clear(cyg_vector_t vector);
#endif // CYG_HAL_IRQCOUNT_SUPPORT

#ifdef CYG_HAL_TIMER_SUPPORT
#define HAL_TIMER_COUNT 3
// It's up to the application to handle interrupts if required.
externC cyg_bool hal_timer_reserve  (cyg_uint32 timer_number);
externC void     hal_timer_release  (cyg_uint32 timer_number);
externC cyg_bool hal_timer_enable   (cyg_uint32 timer_number, cyg_uint32 period_us, cyg_bool one_shot);
externC cyg_bool hal_timer_disable  (cyg_uint32 timer_number);
externC cyg_bool hal_timer_time_left(cyg_uint32 timer_number, cyg_uint32 *time_left_us);
#ifdef CYG_HAL_IRQCOUNT_SUPPORT
externC cyg_uint64 hal_time_get(void); // Get current time since boot in microseconds
// If the threadload service package is installed, let it use hal_time_get() to get
// a better clock granularity than cyg_current_time().
#define THREADLOAD_CURRENT_TIME_GET hal_time_get()
#endif
#endif /* CYG_HAL_TIMER_SUPPORT */

externC void hal_init_irq(void);
#if defined(CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR)
externC void hal_init_irq_secondary(void);
#endif  /* CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR */

#endif // __ASSEMBLER__

//--------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PLF_INTR_H
// End of plf_intr.h
