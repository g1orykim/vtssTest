#ifndef CYGONCE_HAL_PLATFORM_INC
#define CYGONCE_HAL_PLATFORM_INC
##=============================================================================
##
##  platform.inc
##
##  VCore-III board assembler header file
##
##=============================================================================
## ####ECOSGPLCOPYRIGHTBEGIN####                                            
## -------------------------------------------                              
## This file is part of eCos, the Embedded Configurable Operating System.   
## Copyright (C) 2009 Free Software Foundation, Inc.
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
## Author(s):    rbn
## Contributors: lpovlsen, based on MIPS Malta code
## Date:         2009-06-25
## Purpose:      VCore-III board definitions.
## Description:  This file contains various definitions and macros that are
##               useful for writing assembly code for the VCore-III board.
## Usage:
##      #include <cyg/hal/platform.inc>
##      ...
##
######DESCRIPTIONEND####
##
##=============================================================================

#include <cyg/hal/mips.inc>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/plf_io.h>
#include <cyg/hal/plf_intr.h>

#------------------------------------------------------------------------------
# Additional bits for status register.
# We set the IM[0] bit to accept all interrupts.

#define INITIAL_SR_PLF 0x00000400
#if defined(CYG_HAL_STARTUP_RAMBOOT)
#define INITIAL_SR_ARCH       0x1000ff00 /* CP0 usable, Ints enabled, master interrupt disable */
#endif

#define CYGPKG_HAL_EARLY_INIT
    .macro	hal_early_init
    .extern	hal_init_tlb
    .extern	hal_map_iospace
    .extern	pll_init

# Reset TLB
    jal	        hal_init_tlb
    nop
# MAP register spaces, Flash, DDR
    jal	        hal_map_iospace
    nop
    # Speed up SPI Flash Reads: Fast reads + desel x19 + div ~ 23Mhz
    li		v0, VTSS_F_ICPU_CFG_SPI_MST_SPI_MST_CFG_FAST_READ_ENA + \
                VTSS_F_ICPU_CFG_SPI_MST_SPI_MST_CFG_CS_DESELECT_TIME(0x19) + \
                VTSS_F_ICPU_CFG_SPI_MST_SPI_MST_CFG_CLK_DIV(9)
    sw          v0, VTSS_ICPU_CFG_SPI_MST_SPI_MST_CFG
#if !defined(CYG_HAL_STARTUP_RAMBOOT)
    jal	        pll_init
    nop
#endif
    .endm

#define CYGPKG_HAL_MIPS_MON_DEFINED

	.macro	hal_mon_init
	hal_mon_copy_trampoline
	hal_mon_init_vsr_table
	.endm

#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)
	.macro	hal_mon_copy_trampoline
	la	a0,utlb_vector
	la	a1,utlb_vector_end
	la	t0,0xa0000000
1:
	lw	v0,0(a0)
	sw	v0,0(t0)
	addiu	a0,a0,4
	bne	a0,a1,1b
	addiu	t0,t0,4
	.endm
#elif defined(CYG_HAL_STARTUP_RAMBOOT)
	.macro	hal_mon_copy_trampoline
	la	a0,utlb_vector
	la	a1,utlb_vector_end
	la	t0,0xa0000000
1:
	lw	v0,0(a0)
	sw	v0,0x180(t0)
	sw	v0,0(t0)
	addiu	a0,a0,4
	bne	a0,a1,1b
	addiu	t0,t0,4
	.endm
#else
	.macro	hal_mon_copy_trampoline
	.endm
#endif

	# Fill the VSR table with the default VSRs.
	# If we contain the stubs, the default VSR will pass
	# exceptions on to the stubs.

	.macro	hal_mon_init_vsr_table
	la	v0,__default_exception_vsr
	la	v1,hal_vsr_table

	sw	v0,(1*4)(v1)
	sw	v0,(2*4)(v1)
	sw	v0,(3*4)(v1)
	sw	v0,(4*4)(v1)
	sw	v0,(5*4)(v1)
	sw	v0,(6*4)(v1)
	sw	v0,(7*4)(v1)	# Bus error
	sw	v0,(8*4)(v1)
	sw	v0,(9*4)(v1)
	sw	v0,(10*4)(v1)
	sw	v0,(11*4)(v1)
	sw	v0,(12*4)(v1)
	sw	v0,(13*4)(v1)
	sw	v0,(14*4)(v1)
	sw	v0,(15*4)(v1)
	sw	v0,(23*4)(v1)
	sw	v0,(24*4)(v1)
	sw	v0,(32*4)(v1)	# debug
	sw	v0,(33*4)(v1)	# utlb
	sw	v0,(34*4)(v1)	# nmi

	la	v0,__default_interrupt_vsr
	sw	v0,(0*4)(v1)

	.endm

#------------------------------------------------------------------------------
# Cache macros.
	
#ifdef CYG_HAL_STARTUP_RAM

	.macro	hal_cache_init
	.endm

#define CYGPKG_HAL_MIPS_CACHE_DEFINED

#endif	

#------------------------------------------------------------------------------
# Interrupt macros

#ifndef CYGPKG_HAL_MIPS_INTC_INIT_DEFINED

	.macro	hal_intc_init
	mfc0	v0,status
	nop
	lui	v1,0xFFFF
	ori	v1,v1,0x00FF
	and	v0,v0,v1	# Clear the IntMask bits IM[7-0]
	mtc0	v0,status
	nop
	nop
	nop
	.endm

#define CYGPKG_HAL_MIPS_INTC_INIT_DEFINED

#endif

#ifndef CYGPKG_HAL_MIPS_INTC_DECODE_DEFINED
	.macro	hal_intc_decode vnum

        lw      v0,VTSS_ICPU_CFG_INTR_INTR_IDENT # (all) In service address
        move    t8,v0
        bnez    v0,0f                   # Any VCoreIII irqs?

    2:
	mfc0	v1,status		# get status register (interrupt mask)
	mfc0	v0,cause		# get cause register
	and	v0,v0,v1		# apply interrupt mask
	srl	v0,v0,11		# shift interrupt bits down - loose IM0
	andi	v0,v0,0x1f		# isolate remaining 5 interrupt bits
	clz	v1,v0			# count leading zeros into v1
	la	v0,32	                # v0 = 32
	b	1f
	sub	\vnum,v0,v1		# vnum = v0-v1 == vector number	
    0:  
    # vec = (31 - clz(irq_id)) + PRI_IRQ_BASE
	clz	v1,v0			# count leading zeros into v1
	li	v0,(31+PRI_IRQ_BASE)	# v0 = 31+PRI_IRQ_BASE
	sub	\vnum,v0,v1		# vnum = v0-v1 == vector number	

    1:
	.endm

#ifndef CYGPKG_HAL_MIPS_INTC_TRANSLATE_DEFINED
#ifdef CYGIMP_HAL_COMMON_INTERRUPTS_CHAIN
	.macro	hal_intc_translate inum,vnum
	move	\vnum,zero			# Just vector zero is supported
	.endm
#else			
	.macro	hal_intc_translate inum,vnum
	move	\vnum,\inum			# Vector == interrupt number
	.endm
#endif
#endif

	.macro	hal_intc_decode_data
	.endm

#define CYGPKG_HAL_MIPS_INTC_DECODE_DEFINED
#define CYGPKG_HAL_MIPS_INTC_DEFINED

// We also define our own interrupt tables in platform.S...
#define CYG_HAL_MIPS_ISR_TABLES_DEFINED

#endif

#------------------------------------------------------------------------------
# Diagnostic macros

#ifndef CYGPKG_HAL_MIPS_DIAG_DEFINED

#ifdef CYG_HAL_IRQCOUNT_SUPPORT

	.macro	hal_diag_init
	.endm

	.macro	hal_diag_excpt_start
	.endm

	.macro	hal_diag_intr_start
	.extern hal_interrupt_counts
        # 64-bit addition
	sll	t1,s1,3			    # s1 = byte offset of vector
        la      t2,hal_interrupt_counts
        addu    t1,t2,t1
	lw	t2,0(t1)
        addiu   t3,t2,1
        sltu    t2,t3,t2                    # t2 = t3 < t2 ? 1 : 0
        sw      t3,0(t1)
        beq     t2,zero,1f
        lw      t2,4(t1)
        addiu   t2,t2,1
        sw      t2,4(t1)
1:
	.endm

	.macro	hal_diag_restore
	.endm

#else

	.macro	hal_diag_init
	.endm

	.macro	hal_diag_excpt_start
	.endm

	.macro	hal_diag_intr_start
	.endm

	.macro	hal_diag_restore
	.endm

#endif // CYG_HAL_IRQCOUNT_SUPPORT

#define CYGPKG_HAL_MIPS_DIAG_DEFINED

#endif // ifndef CYGPKG_HAL_MIPS_DIAG_DEFINED
	
#------------------------------------------------------------------------------
# MEMC macros.
# 
	
#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_ROMRAM)

	.macro	hal_memc_init
	.extern hal_vcoreiii_init_memctl
	.extern hal_vcoreiii_wait_memctl
	.extern hal_vcoreiii_train_bytelane

        la	t1, VTSS_ICPU_CFG_MEMCTRL_MEMCTRL_STAT
        lw      v0, 0(t1)
        andi    v0, v0, VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_STAT_INIT_DONE
        bnez	v0,5f           # Skip init if done already
        nop

	jal	hal_vcoreiii_init_memctl
	nop

	jal	hal_vcoreiii_wait_memctl
	nop

3:
        jal	hal_vcoreiii_train_bytelane
        nop

        bnez    v0, 45f                 # If failure goto reset
        nop

        # Test DDR lanes working OK
4:
	li	t0,VTSS_DDR_TO		# DDR base addr
        move    v0, zero                # zero
        subu    v1, zero, 1             # -1
        
        sw      v0, 0(t0)               # mem[0] = 0
        sw      v1, 4(t0)               # mem[1] = -1

        # Test 5 reads offset 0 (= 0)
        li      t1, 5
41:
        lw      a0, 0(t0)
        bne     a0, v0, 45f
        subu    t1, t1, 1
        bnez    t1, 41b
	nop

        # Test 5 reads offset 4 (= -1)
        li      t1, 5
42:
        lw      a0, 4(t0)
        bne     a0, v1, 45f
        subu    t1, t1, 1
        bnez    t1, 42b
	nop

        # All well & tested
        b       5f
        nop
        
45:
        # Do a fifo reset and start over
#if defined(VTSS_ICPU_CFG_MEMCTRL_MEMPHY_CFG) && defined(VTSS_F_ICPU_CFG_MEMCTRL_MEMPHY_CFG_PHY_FIFO_RST)
        li      t1, VTSS_ICPU_CFG_MEMCTRL_MEMPHY_CFG
        lw      v0, 0(t1)               # Read ICPU_CFG::MEMPHY_CFG
        ori     v0, v0, VTSS_F_ICPU_CFG_MEMCTRL_MEMPHY_CFG_PHY_FIFO_RST # Set PHY_FIFO_RST
        sw      v0, 0(t1)               # Set ICPU_CFG::MEMPHY_CFG
        li      v1, ~VTSS_F_ICPU_CFG_MEMCTRL_MEMPHY_CFG_PHY_FIFO_RST
        and     v0, v0, v1              # Clear PHY_FIFO_RST
        sw      v0, 0(t1)               # Set ICPU_CFG::MEMPHY_CFG
#endif /* VTSS_ICPU_CFG_MEMCTRL_MEMPHY_CFG */

        b       3b
        nop
        
        # Done DDR setup and test
5:

#if defined(CYG_HAL_STARTUP_ROMRAM)

        # Determine if we are already in RAM
        la	t1, VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_GENERAL_CTRL
        lw      v0, 0(t1)               # Read ICPU_CFG::CPU_SYSTEM_CTRL::GENERAL_CTRL
        andi    v0, v0, VTSS_F_ICPU_CFG_CPU_SYSTEM_CTRL_GENERAL_CTRL_BOOT_MODE_ENA                
        beq	v0,zero,2f
        nop

	# Having got the RAM working, we must now relocate the entire
	# ROM into it and then continue execution from RAM.

	li	t0,VTSS_DDR_TO		# dest addr
	lar	t1,reset_vector		# source addr
	lar	t3,__ram_data_end	# end dest addr
1:	
	lw	v0,0(t1)		# get word
	sw	v0,0(t0)		# write word
	addiu	t1,t1,4
	addiu	t0,t0,4
	bne	t1,t3,1b
	nop

        la	t1, VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_GENERAL_CTRL
        li      v1, ~VTSS_F_ICPU_CFG_CPU_SYSTEM_CTRL_GENERAL_CTRL_BOOT_MODE_ENA
        lw      v0, 0(t1)               # Read ICPU_CFG::CPU_SYSTEM_CTRL::GENERAL_CTRL
        and     v1, v1, v0
        sw      v1, 0(t1)               # Store w. bootmode cleared
        lw      v0, 0(t1)               # Read again to activate

        nop                             # Skidmarks...
        nop
        nop

2:
	# We are now executing out of RAM!
#endif

	.endm

#define CYGPKG_HAL_MIPS_MEMC_DEFINED

#endif


#------------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_PLATFORM_INC
# end of platform.inc
