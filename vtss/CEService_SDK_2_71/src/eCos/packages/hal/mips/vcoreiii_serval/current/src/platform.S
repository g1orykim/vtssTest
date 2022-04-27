##
#=============================================================================
##	platform.S
##
##	MIPS Malta platform code
##
##=============================================================================
## ####ECOSGPLCOPYRIGHTBEGIN####                                            
## -------------------------------------------                              
## This file is part of eCos, the Embedded Configurable Operating System.   
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
##
## eCos is free software; you can redistribute it and/or modify it under    
## the terms of the GNU General Public License as published by the Free     
## Software Foundation; either version 2 or (at your option) any later      
## version.                                                                 
##
## eCos is distributed in the hope that it will be useful, but WITHOUT      
## ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    
## FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    
## for more details.                                                        
##
## You should have received a copy of the GNU General Public License        
## along with eCos; if not, write to the Free Software Foundation, Inc.,    
## 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.            
##
## As a special exception, if other files instantiate templates or use      
## macros or inline functions from this file, or you compile this file      
## and link it with other works to produce a work based on this file,       
## this file does not by itself cause the resulting work to be covered by   
## the GNU General Public License. However the source code for this file    
## must still be made available in accordance with section (3) of the GNU   
## General Public License v2.                                               
##
## This exception does not invalidate any other reasons why a work based    
## on this file might be covered by the GNU General Public License.         
## -------------------------------------------                              
## ####ECOSGPLCOPYRIGHTEND####                                              
##=============================================================================
#######DESCRIPTIONBEGIN####
##
## Author(s):	dmoseley
## Contributors:dmoseley, jskov
## Date:	2001-03-20
## Purpose:	MIPS Malta platform code
## Description:	Platform specific code for Malta board.
##
##
##
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <pkgconf/system.h>
#include <pkgconf/hal.h>

#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>
# ifdef CYGPKG_PROFILE_GPROF
#  include <pkgconf/profile_gprof.h>
# endif
#endif

#include <cyg/hal/arch.inc>
#include <cyg/hal/plf_io.h>
#include <cyg/hal/plf_intr.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/plf_defs.inc>
#include CYGBLD_HAL_PLATFORM_H

##-----------------------------------------------------------------------------

#ifdef at
#undef at
#endif

##-----------------------------------------------------------------------------
# Platform Initialization.
# This code performs platform specific initialization.

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)

	.set	noreorder
        .text
FUNC_START(pll_init)
	jr      ra
	nop
FUNC_END(pll_init)

#endif // defined(CYG_HAL_STARTUP_ROM / ROMRAM )

#if defined(CYGPKG_KERNEL)
FUNC_START(vcoreiii_cool_restart_asm_begin)
    /*
     * In this code snippet, we use all kinds of registers - even non-scratch.
     * This is acceptable, since the function ends with a reboot and never returns.
     * Parameters: a0 = addr of register to persist across switch core reset.
     */

    /*
     * Get the current value of the MBOX value. This is where the API has persisted its
     * restart state in the previous call to vtss_restart_conf_set().
     * Unfortunately, there's no other way to keep this persisted, because we reset
     * the switch core in just a split second,and therefore we need to remember it.
     */
    // register u32 mbox = VTSS_DEVCPU_GCB_SW_REGS_MAILBOX;
    lw a1,0(a0)

    /*
     * Prevent VCore-III from being reset with a global reset.
     */
    // VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_RESET = VTSS_F_ICPU_CFG_CPU_SYSTEM_CTRL_RESET_CORE_RST_PROTECT;
    li a2,VTSS_F_ICPU_CFG_CPU_SYSTEM_CTRL_RESET_CORE_RST_PROTECT;
    sw a2,VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_RESET;

    /*
     * Do the global reset (which only resets the switch core (and possibly PHYs)) now).
     */
#if defined(VTSS_DEVCPU_GCB_CHIP_REGS_SOFT_RST)
    li a2,VTSS_F_DEVCPU_GCB_CHIP_REGS_SOFT_RST_SOFT_CHIP_RST
    sw a2,VTSS_DEVCPU_GCB_CHIP_REGS_SOFT_RST
#else
    li a2,VTSS_F_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_CHIP_RST_SOFT_CHIP_RST
    sw a2,VTSS_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_CHIP_RST
#endif

    /*
     * Pause. The pause should be long enought to allow the CSR ring to stabilize after the reset,
     * i.e. the alignment block must have sent one round of syncs. A good guess is if we here
     * count to 1000. This will take 3000 CPU clock cycles or 1500 AHB clock cycles, which should
     * be enough despite the SwC-to-AHB clock relationship.
     */
     // register int i;
     // for(i = 0; i < 1000; i++); // Do nothing
     move v0,zero      # v0 = 0;
1:
     addiu v0,v0,1     # v0++;
     slti v1,v0,1000   # v1 = v0 < 1000 ? 1 : 0;
     bnez v1,1b        # if (v1 != 0) goto 1;
     nop               # Branch delay slot

    /*
     * Write the mbox value back to the switch core.
     */
    // VTSS_DEVCPU_GCB_SW_REGS_MAILBOX = mbox;
    sw a1,0(a0) # a1 holds the value previously read, which must be stored in a0.

    /*
     * Then reset VCore-III, only.
     */
    // VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_RESET = VTSS_F_ICPU_CFG_CPU_SYSTEM_CTRL_RESET_CORE_RST_FORCE;
    li a2,VTSS_F_ICPU_CFG_CPU_SYSTEM_CTRL_RESET_CORE_RST_FORCE
    sw a2,VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_RESET

    /*
     * Wait forever (unreachable).
     */
    // while(1);
2:  b 2b
    nop
FUNC_END(vcoreiii_cool_restart_asm_begin)

FUNC_START(vcoreiii_cool_restart_asm_end)
  // Dummy function that marks the end of the previous function.
  // MUST BE PLACED RIGHT AFTER vcoreiii_cool_restart_asm_begin().
  nop
FUNC_END(vcoreiii_cool_restart_asm_end)

#endif // CYGPKG_KERNEL

#ifdef CYGPKG_PROFILE_CALLGRAPH
	// See http://gcc.gnu.org/ml/gcc/2002-03/msg01373.html
        .text
	.set	noreorder
	.set	noat
	.extern hal_mcount
FUNC_START(_mcount)
	addiu	sp,sp,-(32*4)   # Store 32 regs (max)
	# store GPRs
#	sgpr	0,sp // zero
	sgpr	1,sp // at 
	sgpr	2,sp // v0 
	sgpr	3,sp // v1 
	sgpr	4,sp // a0 
	sgpr	5,sp // a1 
	sgpr	6,sp // a2 
	sgpr	7,sp // a3 
	sgpr	8,sp // t0 
	sgpr	9,sp // t1 
	sgpr	10,sp // t2
	sgpr	11,sp // t3
#	sgpr	12,sp // t4
#	sgpr	13,sp // t5
#	sgpr	14,sp // t6
#	sgpr	15,sp // t7
#	sgpr	16,sp // s0
#	sgpr	17,sp // s1
#	sgpr	18,sp // s2
#	sgpr	19,sp // s3
#	sgpr	20,sp // s4
#	sgpr	21,sp // s5
#	sgpr	22,sp // s6
#	sgpr	23,sp // s7
#	sgpr	24,sp // t8
#	sgpr	25,sp // t9
#	sgpr	26,sp	# == K0
#	sgpr	27,sp	# == K1
#	sgpr	28,sp	# == GP
#	sgpr	29,sp	# == SP
#	sgpr	30,sp	# == FP
	sgpr	31,sp	# == RA

	move    a0, $1          // Caller = at
        move    a1, ra          // Callee
        jal     hal_mcount      // Intermediate HAL layer
        nop

	# load GPRs
#	lgpr	0,sp // zero
	lgpr	1,sp // at
	lgpr	2,sp // v0
	lgpr	3,sp // v1
	lgpr	4,sp // a0
	lgpr	5,sp // a1
	lgpr	6,sp // a2
	lgpr	7,sp // a3
	lgpr	8,sp // t0
	lgpr	9,sp // t1
	lgpr	10,sp // t2
	lgpr	11,sp // t3
#	lgpr	12,sp // t4
#	lgpr	13,sp // t5
#	lgpr	14,sp // t6
#	lgpr	15,sp // t7
#	lgpr	16,sp // s0
#	lgpr	17,sp // s1
#	lgpr	18,sp // s2
#	lgpr	19,sp // s3
#	lgpr	20,sp // s4
#	lgpr	21,sp // s5
#	lgpr	22,sp // s6
#	lgpr	23,sp // s7
#	lgpr	24,sp // t8
#	lgpr	25,sp // t9
#	lgpr	26,sp	# == K0
#	lgpr	27,sp	# == K1
#	lgpr	28,sp	# == GP
#	lgpr	29,sp	# == SP
#	lgpr	30,sp	# == FP, S8
	lgpr	31,sp	# == RA
	
        # NB: The +8 comes from the mcount calling convention (historic).
        addiu	sp,sp,(32*4)+8          // Restore stack
	jr      ra
        move    ra,$1                   // Restore return address ra (from at)
        .set	at
FUNC_END(_mcount)
#endif /* CYGPKG_PROFILE_CALLGRAPH */

##-----------------------------------------------------------------------------
# Interrupt vector tables.
# These tables contain the isr, data and object pointers used to deliver
# interrupts to user code.

	.extern hal_default_isr

	.data

	.globl	hal_interrupt_handlers
hal_interrupt_handlers:
	.rept	CYGNUM_HAL_ISR_COUNT
	.long	hal_default_isr
	.endr

	.globl	hal_interrupt_data
hal_interrupt_data:
	.rept	CYGNUM_HAL_ISR_COUNT
	.long	0
	.endr

	.globl	hal_interrupt_objects
hal_interrupt_objects:
	.rept	CYGNUM_HAL_ISR_COUNT
	.long	0
	.endr

#ifdef CYG_HAL_IRQCOUNT_SUPPORT
	.globl	hal_interrupt_counts
hal_interrupt_counts:
	.rept	CYGNUM_HAL_ISR_COUNT
	.long	0
        .long   0
	.endr
#endif // CYG_HAL_IRQCOUNT_SUPPORT
	
##-----------------------------------------------------------------------------
## end of platform.S
