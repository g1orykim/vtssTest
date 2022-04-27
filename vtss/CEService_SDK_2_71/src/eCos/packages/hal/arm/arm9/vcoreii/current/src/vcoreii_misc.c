//==========================================================================
//
//      vcoreii_misc.c
//
//      HAL misc board support code for ARM9/VCOREII
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
// Author(s):    Lars Povlsen
// Contributors:
// Date:         2006-03-22
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/system.h>
#include CYGBLD_HAL_PLATFORM_H
#include <cyg/hal/vcoreii_clockadj.h>

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#ifdef CYGPKG_IO_I2C_ARM_VCOREII
#include <cyg/io/i2c_vcoreii.h>
#endif

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // Interrupt names
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/plf_io.h>          // Platform specifics
#ifdef CYGPKG_PROFILE_GPROF
#include <cyg/profile/profile.h>
#endif

#include <cyg/infra/diag.h>             // diag_printf

#include <string.h> // memset




// -------------------------------------------------------------------------
// MMU initialization:
//
// These structures are laid down in memory to define the translation
// table.
//

// Vitesse Translation Table Base Bit Masks
#define ARM_TRANSLATION_TABLE_MASK               0xFFFFC000

// Vitesse Domain Access Control Bit Masks
#define ARM_ACCESS_TYPE_NO_ACCESS(domain_num)    (0x0 << (domain_num)*2)
#define ARM_ACCESS_TYPE_CLIENT(domain_num)       (0x1 << (domain_num)*2)
#define ARM_ACCESS_TYPE_MANAGER(domain_num)      (0x3 << (domain_num)*2)

struct ARM_MMU_FIRST_LEVEL_FAULT {
    int id : 2;
    int sbz : 30;
};
#define ARM_MMU_FIRST_LEVEL_FAULT_ID 0x0

struct ARM_MMU_FIRST_LEVEL_PAGE_TABLE {
    int id : 2;
    int imp : 2;
    int domain : 4;
    int sbz : 1;
    int base_address : 23;
};
#define ARM_MMU_FIRST_LEVEL_PAGE_TABLE_ID 0x1

struct ARM_MMU_FIRST_LEVEL_SECTION {
    int id : 2;
    int b : 1;
    int c : 1;
    int imp : 1;
    int domain : 4;
    int sbz0 : 1;
    int ap : 2;
    int sbz1 : 8;
    int base_address : 12;
};
#define ARM_MMU_FIRST_LEVEL_SECTION_ID 0x2

struct ARM_MMU_FIRST_LEVEL_RESERVED {
    int id : 2;
    int sbz : 30;
};
#define ARM_MMU_FIRST_LEVEL_RESERVED_ID 0x3

#define ARM_MMU_FIRST_LEVEL_DESCRIPTOR_ADDRESS(ttb_base, table_index) \
   (unsigned long *)((unsigned long)(ttb_base) + ((table_index) << 2))

#define ARM_FIRST_LEVEL_PAGE_TABLE_SIZE 0x4000

#define ARM_MMU_SECTION(ttb_base, actual_base, virtual_base,              \
                        cacheable, bufferable, perm)                      \
    CYG_MACRO_START                                                       \
        register union ARM_MMU_FIRST_LEVEL_DESCRIPTOR desc;               \
                                                                          \
        desc.word = 0;                                                    \
        desc.section.id = ARM_MMU_FIRST_LEVEL_SECTION_ID;                 \
        desc.section.imp = 1;                                             \
        desc.section.domain = 0;                                          \
        desc.section.c = (cacheable);                                     \
        desc.section.b = (bufferable);                                    \
        desc.section.ap = (perm);                                         \
        desc.section.base_address = (actual_base);                        \
        *ARM_MMU_FIRST_LEVEL_DESCRIPTOR_ADDRESS(ttb_base, (virtual_base)) \
                            = desc.word;                                  \
    CYG_MACRO_END

#define X_ARM_MMU_SECTION(abase,vbase,size,cache,buff,access)      \
    { int i; int j = abase; int k = vbase;                         \
      for (i = size; i > 0 ; i--,j++,k++)                          \
      {                                                            \
        ARM_MMU_SECTION(ttb_base, j, k, cache, buff, access);      \
      }                                                            \
    }

union ARM_MMU_FIRST_LEVEL_DESCRIPTOR {
    unsigned long word;
    struct ARM_MMU_FIRST_LEVEL_FAULT fault;
    struct ARM_MMU_FIRST_LEVEL_PAGE_TABLE page_table;
    struct ARM_MMU_FIRST_LEVEL_SECTION section;
    struct ARM_MMU_FIRST_LEVEL_RESERVED reserved;
};

#define ARM_UNCACHEABLE                         0
#define ARM_CACHEABLE                           1
#define ARM_UNBUFFERABLE                        0
#define ARM_BUFFERABLE                          1

#define ARM_ACCESS_PERM_NONE_NONE               0
#define ARM_ACCESS_PERM_RO_NONE                 0
#define ARM_ACCESS_PERM_RO_RO                   0
#define ARM_ACCESS_PERM_RW_NONE                 1
#define ARM_ACCESS_PERM_RW_RO                   2
#define ARM_ACCESS_PERM_RW_RW                   3

extern unsigned long ttb_base_offset[];

static inline volatile cyg_uint32 rd_cp1(void)
{
    cyg_uint32 reg;
    asm volatile("MRC p15, 0, %[oreg], c1, c0, 0" : [oreg] "=r" (reg));
    return reg;
}

static inline void wr_cp1(cyg_uint32 reg)
{
    asm volatile("MCR p15, 0, %[ireg], c1, c0, 0" : : [ireg] "r" (reg));
}

void
hal_mmu_init(void)
{
    unsigned long ttb_base = VCOREII_SDRAM_PHYS_BASE + ((unsigned long) &ttb_base_offset[0]);
    unsigned long domac;

    // First clear all TT entries - ie Set them to Faulting
    memset((void *)ttb_base, 0, ARM_FIRST_LEVEL_PAGE_TABLE_SIZE);

    // Set the TTB register
    asm volatile ("mcr  p15,0,%[base],c2,c0,0" : : [base] "r" (ttb_base));

    // Set the Domain Access Control Register
    domac = ARM_ACCESS_TYPE_MANAGER(0) |
        ARM_ACCESS_TYPE_NO_ACCESS(1)  |
        ARM_ACCESS_TYPE_NO_ACCESS(2)  |
        ARM_ACCESS_TYPE_NO_ACCESS(3)  |
        ARM_ACCESS_TYPE_NO_ACCESS(4)  |
        ARM_ACCESS_TYPE_NO_ACCESS(5)  |
        ARM_ACCESS_TYPE_NO_ACCESS(6)  |
        ARM_ACCESS_TYPE_NO_ACCESS(7)  |
        ARM_ACCESS_TYPE_NO_ACCESS(8)  |
        ARM_ACCESS_TYPE_NO_ACCESS(9)  |
        ARM_ACCESS_TYPE_NO_ACCESS(10) |
        ARM_ACCESS_TYPE_NO_ACCESS(11) |
        ARM_ACCESS_TYPE_NO_ACCESS(12) |
        ARM_ACCESS_TYPE_NO_ACCESS(13) |
        ARM_ACCESS_TYPE_NO_ACCESS(14) |
        ARM_ACCESS_TYPE_NO_ACCESS(15);
    asm volatile ("mcr  p15,0,%0,c3,c0,0" : : "r"(domac) /*:*/);

    //               Actual  Virtual  Size   Attributes                                                    Function
    //           Base     Base     MB      cached?           buffered?        access permissions
    //             xxx00000  xxx00000
    // Alias - ROM or SDRAM
#ifdef CYGPKG_REDBOOT
    X_ARM_MMU_SECTION(0x000,  0x000,    64,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW);
#else
    /* Establish *possibility* of low RAM protection.
     * To enable, change DOMAC register from MANAGER => CLIENT.
     * This can only be done when system init is done!
     */
    extern char __ram_data_start[];
    size_t textsize = (size_t) (__ram_data_start - (char*)VCOREII_ALIAS_PHYS_BASE);
    int nRO = (textsize / (1024*1024));
    if(nRO > 0) {
        X_ARM_MMU_SECTION(  0,   0,    nRO, ARM_CACHEABLE, ARM_BUFFERABLE, ARM_ACCESS_PERM_RO_RO);
        X_ARM_MMU_SECTION(nRO, nRO, 64-nRO, ARM_CACHEABLE, ARM_BUFFERABLE, ARM_ACCESS_PERM_RW_RW);
    } else {
        X_ARM_MMU_SECTION(0x000, 0x000, 64, ARM_CACHEABLE, ARM_BUFFERABLE, ARM_ACCESS_PERM_RW_RW);
    }
#endif
    // SDRAM
    X_ARM_MMU_SECTION(0x400,  0x400,    64,  ARM_CACHEABLE,   ARM_BUFFERABLE,   ARM_ACCESS_PERM_RW_RW);
    // Flash
    X_ARM_MMU_SECTION(0x800,  0x800,    32,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW);
    // VCore-II SwC
    X_ARM_MMU_SECTION(0xa00,  0xa00,     1,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW);
    // VCore-II amba
    X_ARM_MMU_SECTION(0xc00,  0xc00,     2,  ARM_UNCACHEABLE, ARM_UNBUFFERABLE, ARM_ACCESS_PERM_RW_RW);
}

/*
 * Embedded I2C bus & devices
 */

#ifdef CYGPKG_IO_I2C_ARM_VCOREII

cyg_vcore_i2c_extra hal_vcore_i2c_bus_extra = {
    .i2c_wait = 100,     /* Default to maximum responds time 100 ms */
    .i2c_clk_sel = -1    /* Default use standard clk */
}; 

CYG_I2C_BUS(hal_vcore_i2c_bus,
            cyg_vcoreii_i2c_init,
            cyg_vcoreii_i2c_tx,
            cyg_vcoreii_i2c_rx,
            cyg_vcoreii_i2c_stop,
            (void *)&hal_vcore_i2c_bus_extra); 

#endif

//----------------------------------------------------------------------------
// Profiling support

#ifdef CYGPKG_PROFILE_GPROF

#define MIN_PROFILE_PERIOD	10 /* 10 usec minimum */

// Periodic timer ISR.
static cyg_uint32 
isr_pit(CYG_ADDRWORD vector, CYG_ADDRWORD data, HAL_SavedRegisters *regs)
{

    HAL_INTERRUPT_ACKNOWLEDGE (CYGNUM_HAL_INT_TIMER_2);
    __profile_hit(regs->pc);

    return 1;                   /* Cyg_InterruptHANDLED */
}

int
hal_enable_profile_timer(int resolution) /* resolution units = 1 usec  */
{
    cyg_uint32 profile_period;

    if(resolution < MIN_PROFILE_PERIOD)
        resolution = MIN_PROFILE_PERIOD; /* Lower bound */

    /* Calculate ticks from the main timer divider */
    profile_period = 
        (CYGNUM_HAL_ARM_VCOREII_TIMER_CLOCK / (2*CYGNUM_HAL_ARM_VCOREII_TIMER_PRESCALER))
        / (1000000 / resolution);

    // Attach pit isr.
    HAL_INTERRUPT_ATTACH (CYGNUM_HAL_INT_TIMER_2, &isr_pit, 0, 0);
    HAL_INTERRUPT_UNMASK (CYGNUM_HAL_INT_TIMER_2);

    /* Disable pit */
    VTSS_TIMERS_TIMER_CTRL_2 = 0;

    /* Clear intr pit */
    VTSS_INTR_CLR = VTSS_F_CLR_TIMER_2;

    /* Configure pit */
    VTSS_TIMERS_TIMER_RELOAD_VALUE_2 = (0x10000 - profile_period);
    VTSS_TIMERS_TIMER_CTRL_2 = (VTSS_F_TIMER_ENA|VTSS_F_FORCE_RELOAD); /* Enabled & reload now */

    return resolution;
}

void hal_suspend_profile_timer(void)
{
    VTSS_TIMERS_TIMER_CTRL_2 = 0; /* Disable pit */
}

void hal_resume_profile_timer(void)
{
    /* Be sure we can resume the timer */
    if(VTSS_TIMERS_TIMER_RELOAD_VALUE_2) {
        VTSS_TIMERS_TIMER_CTRL_2 = (VTSS_F_TIMER_ENA|VTSS_F_FORCE_RELOAD); /* Enabled & reload now */
    }
}

#endif /* CYGPKG_PROFILE_GPROF */

#ifdef CYGPKG_PROFILE_CALLGRAPH
void
hal_mcount(CYG_ADDRWORD caller_pc, CYG_ADDRWORD callee_pc)
{
    static int  nested = 0;
    int         enabled;

    HAL_DISABLE_INTERRUPTS(enabled);
    if (!nested) {
        nested = 1;
        __profile_mcount(caller_pc, callee_pc);
        nested = 0;
    }
    HAL_RESTORE_INTERRUPTS(enabled);
}
#endif /* CYGPKG_PROFILE_CALLGRAPH */

//----------------------------------------------------------------------------
// Platform specific initialization

void
plf_hardware_init(void)
{
    VTSS_INTR_MASK = 0;         /* 0 is _disabled_ */

#ifndef CYGFUN_HAL_COMMON_KERNEL_SUPPORT
    HAL_CLOCK_INITIALIZE( CYGNUM_HAL_RTC_PERIOD );
#endif

    /* Enable IRQ's */
    VTSS_INTR_CTRL |= VTSS_F_GLBL_IRQ_ENA;

#if defined(CYGSEM_HAL_INSTALL_MMU_TABLES) && defined(CYG_HAL_STARTUP_RAM)
    wr_cp1(rd_cp1() & ~MMU_Control_M); /* Disable MMU */
    hal_mmu_init();
    wr_cp1(rd_cp1() | MMU_Control_M | MMU_Control_R); /* Enable MMU, ROM Protection */
#endif
}

// declarations
static cyg_uint32 _period;
#if defined(CYG_HAL_TIMER_ADJUSTMENT_SUPPORT)

static int _adj_enable = 0;
static timer_tick_cb _clock_callout = NULL;

// used for clock adjustment
// Initial clock offset = 0 => delta_max = 1, delta_t = 100, delta_h = 0
static  int delta_max = 1;
static  int delta_t = 100; /* number of periods */
static  int delta_h = 0; /* number of periods with delta_max offset */
static  int delta_period = 0;
static  int default_period; /* default value for _period */
static  int next_period;    /* value for _period in the nexe period */
#endif

// -------------------------------------------------------------------------
void
hal_clock_initialize(cyg_uint32 period)
{
    /* Disable timer 0 */
    VTSS_TIMERS_TIMER_CTRL_0 = 0;

    /* disable timer0 ints */
    hal_interrupt_mask(CYGNUM_HAL_INTERRUPT_RTC);

    /* Clear intr timer 0 */
    VTSS_INTR_CLR = VTSS_F_CLR_TIMER_0;

    /* Set Divider according to desired timer granularity */
    VTSS_TIMERS_TIMER_TICK_DIV = CYGNUM_HAL_ARM_VCOREII_TIMER_PRESCALER-1; // errata: register value is prescaler -1

    /* Configure timer 0 */
    _period = (0x10000 - period);
    VTSS_TIMERS_TIMER_RELOAD_VALUE_0 = _period;
    VTSS_TIMERS_TIMER_CTRL_0 = (VTSS_F_TIMER_ENA|VTSS_F_FORCE_RELOAD); /* Enabled & reload now */
#if defined(CYG_HAL_TIMER_ADJUSTMENT_SUPPORT)
    default_period = _period;
    next_period = _period;
#endif
}

#if defined(CYG_HAL_TIMER_ADJUSTMENT_SUPPORT)
// Enable the timer tick period period adjustment and callout.
void hal_clock_enable_set_adjtimer(int enable, timer_tick_cb callout)
{
    CYG_INTERRUPT_STATE old;

    HAL_DISABLE_INTERRUPTS(old);
    _adj_enable = enable;
    if (!enable)
    {
        _period = default_period;
        next_period = _period;
        VTSS_TIMERS_TIMER_RELOAD_VALUE_0 = _period;

    }
    _clock_callout = callout;
    HAL_RESTORE_INTERRUPTS(old);
}


// Change the timer tick period.
// The change takes place in the timer interrupt callback (hal_clock_reset)

// The function calculates these static variables, used by the interupt callback:
// delta_max = max counter offset (the counter changes between delta_max and delta_max-1
// delta_t   = number of periods for the change in offset
// delta_h   = number of periods with delta_max offset,
// (delta_t - delta_h = number of periods with delta_max-1 offset)
// delta_period = current period counter
void
hal_clock_set_adjtimer(int delta_ppm)
{
    int delta_dutycycle;
    int my_delta_t;
    int my_delta_h;
    CYG_INTERRUPT_STATE old;

    HAL_DISABLE_INTERRUPTS(old);
    if (delta_ppm >= 0)
    {
        delta_max = (delta_ppm / 100) + 1;
        delta_dutycycle = delta_ppm % 100;
    }
    else
    {
        delta_max = ((delta_ppm+1) / 100);
        delta_dutycycle = ((delta_ppm+1) % 100) + 99;
    }
    if (delta_dutycycle == 0)
    {
        my_delta_t = 100;
        my_delta_h = 0;
    }
    else if (delta_dutycycle < 50)
    {
        my_delta_t = 100 / delta_dutycycle;
        my_delta_h = 1;
    }
    else
    {
        my_delta_t = 100 / (100 - delta_dutycycle);
        my_delta_h = my_delta_t - 1;
    }
    if (my_delta_t != delta_t || my_delta_h != delta_h)
    {
        delta_t = my_delta_t;
        delta_h = my_delta_h;
        delta_period = 0;
    }
    HAL_RESTORE_INTERRUPTS(old);
}
#endif

// This routine is called during a clock interrupt.
void
hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
#if defined(CYG_HAL_TIMER_ADJUSTMENT_SUPPORT)
    int offset;
    if (_clock_callout) _clock_callout();
    if (_adj_enable)
    {
        /* update clock period */
        _period = next_period;
        if (delta_period < delta_h) offset = delta_max;
        else offset = delta_max-1;
        if (next_period != default_period - offset)
        {
            next_period = default_period - offset;
            VTSS_TIMERS_TIMER_RELOAD_VALUE_0 = next_period;
        }
        if (++delta_period >= delta_t) delta_period = 0;
    }
#endif

    /* Do none? - EOI register? */

}

// Read the current value of the clock, returning the number of hardware
// "ticks" that have occurred (i.e. how far away the current value is from
// the start)

void
hal_clock_read(cyg_uint32 *pvalue)
{
    /* Clock is 16 bit, counting up */
    *pvalue = VTSS_TIMERS_TIMER_VALUE_0 - _period;
}

/* 
 * Ripped from mips/arch/current/src/hal_misc.c - uses
 * HAL_CLOCK_READ() and is thread safe (by just running on timer0 -
 * which autoloads).
 */
/*------------------------------------------------------------------------*/
/* Delay for some number of useconds.                                     */
void 
hal_delay_us(cyg_uint32 us)
{
    cyg_uint32 val1, val2;
    int diff;
    long usticks;
    long ticks;

    // Calculate the number of counter register ticks per microsecond.
    
    usticks = (CYGNUM_HAL_RTC_PERIOD * CYGNUM_HAL_RTC_DENOMINATOR) / 1000000;

    // Make sure that the value is not zero. This will only happen if the
    // CPU is running at < 2MHz.
    if( usticks == 0 ) usticks = 1;
    
    while( us > 0 )
    {
        int us1 = us;

        // Wait in bursts of less than 10000us to avoid any overflow
        // problems in the multiply.
        if( us1 > 10000 )
            us1 = 10000;

        us -= us1;

        ticks = us1 * usticks;

        HAL_CLOCK_READ(&val1);
        while (ticks > 0) {
            do {
                HAL_CLOCK_READ(&val2);
            } while (val1 == val2);
            diff = val2 - val1;
            if (diff < 0) diff += CYGNUM_HAL_RTC_PERIOD;
            ticks -= diff;
            val1 = val2;
        }
    }
}

// -------------------------------------------------------------------------
//
//
//
void hal_reset(void)
{
    CYG_INTERRUPT_STATE old;

    HAL_DISABLE_INTERRUPTS(old);


    /* Use VTSS_CPU_SYSTEM_CTRL_RESET */
//     VTSS_CPU_SYSTEM_CTRL_RESET = VTSS_F_SOFT_RST;

    // This is e.g. called after a flash update.
    // The ports may be up and traffic be running, and the FDMA
    // may be active. Therefore it's not enough to perform a
    // "VCore-II only" reset. We should perform a full chip reset.
    VCOREII_SYSTEM_GLORESET = VCOREII_F_SYSTEM_GLORESET_MASTER_RESET;

    /* NOTREACHED */
    while (1)
        ;
}

// -------------------------------------------------------------------------

// This routine is called to respond to a hardware interrupt (IRQ).  It
// should interrogate the hardware and return the IRQ vector number.
int
hal_IRQ_handler(void)
{
    cyg_uint32 active = VTSS_INTR_IRQ_IDENT;
    if(active) {
#if defined(__ARM_ARCH_5TE__)
        cyg_uint32 bit;
        asm ("clz %[bit],%[active]"
             : [bit] "=r" (bit)
             : [active] "r" (active));
        return 31 - bit;
#else
        int i;
        for(i = 0; i <= CYGNUM_HAL_ISR_MAX; active >>= 1, i++)
            if(active & 1)
                return i;
#endif
    }

    return CYGNUM_HAL_INTERRUPT_NONE;
}

//----------------------------------------------------------------------------
// Interrupt control
//
void
hal_interrupt_mask(int vector)
{
    CYG_ASSERT(vector <= CYGNUM_HAL_ISR_MAX &&
               vector >= CYGNUM_HAL_ISR_MIN , "Invalid vector");

    VTSS_INTR_MASK_CLR = (1 << vector); /* Clear bit in mask => diabled */
}

void
hal_interrupt_unmask(int vector)
{
    CYG_ASSERT(vector <= CYGNUM_HAL_ISR_MAX &&
               vector >= CYGNUM_HAL_ISR_MIN , "Invalid vector");
    VTSS_INTR_CLR = (1 << vector); /* Be sure to clear the interrupt */
    VTSS_INTR_MASK_SET = (1 << vector); /* Set bit in mask => enabled */
}

void
hal_interrupt_acknowledge(int vector)
{
    CYG_ASSERT(vector <= CYGNUM_HAL_ISR_MAX &&
               vector >= CYGNUM_HAL_ISR_MIN , "Invalid vector");
    VTSS_INTR_CLR = (1 << vector); /* Be sure to clear the interrupt */
}

void
hal_interrupt_configure(int vector, int level, int up)
{
    CYG_ASSERT(vector <= CYGNUM_HAL_ISR_MAX &&
               vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");
    /* No need - all is in place by default register settings! */
}

void
hal_interrupt_set_level(int vector, int level)
{
    CYG_ASSERT(vector <= CYGNUM_HAL_ISR_MAX &&
               vector >= CYGNUM_HAL_ISR_MIN, "Invalid vector");
    CYG_ASSERT(level <= 63 && level >= 0, "Invalid level");
    /* No need - all is in place by default register settings! */
}

#include CYGHWR_MEMORY_LAYOUT_H
typedef void code_fun(void);
void vcoreii_program_new_stack(void *func)
{
    register CYG_ADDRESS stack_ptr asm("sp");
    register CYG_ADDRESS old_stack asm("r4");
    register code_fun *new_func asm("r0");
    old_stack = stack_ptr;
    stack_ptr = CYGMEM_REGION_ram + CYGMEM_REGION_ram_SIZE - sizeof(CYG_ADDRESS);
    new_func = (code_fun*)func;
    new_func();
    stack_ptr = old_stack;
    return;
}
