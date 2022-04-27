#ifndef CYGONCE_HAL_PLATFORM_SETUP_H
#define CYGONCE_HAL_PLATFORM_SETUP_H

//==========================================================================
//
//      hal_platform_setup.h
//
//      Platform specific support for HAL (assembly code)
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
// Purpose:      ARM9/VCOREII platform specific support routines
// Description: 
// Usage:        #include <cyg/hal/hal_platform_setup.h>
//     Only used by "vectors.S"         
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/system.h>             // System-wide configuration info
#include CYGBLD_HAL_VARIANT_H           // Variant specific configuration
#include CYGBLD_HAL_PLATFORM_H          // Platform specific configuration
#include CYGHWR_MEMORY_LAYOUT_H
#include <cyg/hal/hal_mmu.h>            // MMU definitions
#define VTSS_REGISTER(t,o) VTSS_IOADDR(t,o)
#include <cyg/hal/vcoreii.h>           // Platform specific hardware definitions

#if defined(CYG_HAL_STARTUP_ROMRAM) || defined(CYG_HAL_STARTUP_ROM)
#define PLATFORM_SETUP1 _platform_setup1
#define CYGHWR_HAL_ARM_HAS_MMU
#define CYGSEM_HAL_ROM_RESET_USES_JUMP

#if CYGNUM_HAL_ARM_VCOREII_AHB_CLOCK == 104
#define PLL_DIV_CODE		0x9
#define DDR_TRC			5
#define DDR_TRCAR		7
#define DDR_TWR_TRP_TRCD	1
#define DDR_TRASMIN		4
#define DDR_CAS_EXT		0
#define DDR_CAS			1
#define DDR_READPIPE		1
#define DDR_TREF		812
#elif CYGNUM_HAL_ARM_VCOREII_AHB_CLOCK == 125
#define PLL_DIV_CODE		0x5
#define DDR_TRC			6
#define DDR_TRCAR		8
#define DDR_TWR_TRP_TRCD	1
#define DDR_TRASMIN		4
#define DDR_CAS_EXT		0
#define DDR_CAS			1
#define DDR_READPIPE		1
#define DDR_TREF		974
#elif CYGNUM_HAL_ARM_VCOREII_AHB_CLOCK == 156
#define PLL_DIV_CODE		0x1
#define DDR_TRC			8
#define DDR_TRCAR		10
#define DDR_TWR_TRP_TRCD	2
#define DDR_TRASMIN		6
#define DDR_CAS_EXT		VTSS_F_EXTENDED_CAS_LATENCY
#define DDR_CAS			1
#define DDR_READPIPE		1
#define DDR_TREF		1218
#elif CYGNUM_HAL_ARM_VCOREII_AHB_CLOCK == 178
#define PLL_DIV_CODE		0xc
#define DDR_TRC			9
#define DDR_TRCAR		12
#define DDR_TWR_TRP_TRCD	2
#define DDR_TRASMIN		7
#define DDR_CAS_EXT		0
#define DDR_CAS			2
#define DDR_READPIPE		2
#define DDR_TREF		1392
#elif CYGNUM_HAL_ARM_VCOREII_AHB_CLOCK == 208
#define PLL_DIV_CODE		0x8
#define DDR_TRC			11
#define DDR_TRCAR		14
#define DDR_TWR_TRP_TRCD	3
#define DDR_TRASMIN		8
#define DDR_CAS_EXT		0
#define DDR_CAS			2
#define DDR_READPIPE		2
#define DDR_TREF		1624
#else
#error "Country to be explored"
#endif

#if CYGNUM_HAL_ARM_VCOREII_DDR_SIZE == 0x2000000
#define COL_ADDR_WD             8
#elif CYGNUM_HAL_ARM_VCOREII_DDR_SIZE == 0x4000000
#define COL_ADDR_WD             9
#else
#error "Specified DDR Ram volume"
#endif

#define SCONVAL (\
	VTSS_F_S_DATA_WIDTH(1) /* Default */ |\
	VTSS_F_S_COL_ADDR_WIDTH(COL_ADDR_WD) |\
	VTSS_F_S_ROW_ADDR_WIDTH(12) |\
	VTSS_F_S_BANK_ADDR_WIDTH(1) /* Default - 2 banks */)

#define STMG0RVAL (\
	VTSS_F_EXTENDED_T_XSR(0xD) /* Default - 208 */ |\
	DDR_CAS_EXT |\
	VTSS_F_T_RC(DDR_TRC) |\
	VTSS_F_T_XSR(0) /* Default - 208 */ |\
	VTSS_F_T_RCAR(DDR_TRCAR) |\
	VTSS_F_T_WR(DDR_TWR_TRP_TRCD) |\
	VTSS_F_T_RP(DDR_TWR_TRP_TRCD) |\
	VTSS_F_T_RCD(DDR_TWR_TRP_TRCD) |\
	VTSS_F_T_RAS_MIN(DDR_TRASMIN) |\
	VTSS_F_CAS_LATENCY(DDR_CAS) /* Default - 3 clocks */ )

#define STMG1RVAL (\
	VTSS_F_T_WTR(2) /* DDR400 */ |\
	VTSS_F_NUM_INIT_REF(1) /* Don't care */)

#define SCTLVAL (\
	VTSS_F_NUM_OPEN_BANKS(3) /* Default - 4 banks */ |\
	VTSS_F_READ_PIPE(DDR_READPIPE) |\
	VTSS_F_PRECHARGE_ALGORITHM /* Default - delayed precharge */)

#define SREFVAL (DDR_TREF)

	.macro SET_LED_MACRO x
	.endm

// This macro is *only* usable in pristine reset state. Uses timer0 destructively.
        .macro MS_SLEEP reg1, reg2, msec

        /* Start from 0 */
        mov	\reg2, #0
	ldr	\reg1, =VTSS_TIMERS_TIMER_RELOAD_VALUE_0
        str	\reg2, [\reg1]

        /* Start timer */
        mov	\reg2, #(VTSS_F_TIMER_ENA|VTSS_F_FORCE_RELOAD)
	ldr	\reg1, =VTSS_TIMERS_TIMER_CTRL_0
        str	\reg2, [\reg1]

        /* Poll */
	ldr	\reg1, =VTSS_TIMERS_TIMER_VALUE_0
1045:
        ldr	\reg2, [\reg1]
        cmp	\reg2, #(\msec * 10) /* We're running 10KHz default */
	blt	1045b

        /* Disable timer */
        mov	\reg2, #0
	ldr	\reg1, =VTSS_TIMERS_TIMER_CTRL_0
        str	\reg2, [\reg1]

	.endm

// This macro represents the initial startup code for the platform        
        .macro  _platform_setup1
        SET_LED_MACRO 0xff
        // Disable and clear caches
        mrc  p15,0,r0,c1,c0,0
        bic  r0,r0,#0x1000              // disable ICache
        bic  r0,r0,#0x000f              // disable DCache, write buffer,
                                        // MMU and alignment faults
        mcr  p15,0,r0,c1,c0,0
        nop
        nop
        mov  r0,#0
        mcr  p15,0,r0,c7,c6,0           // clear data cache

	SET_LED_MACRO 1

	/* Reset memories */
        ldr	r1, =VCOREII_MEMINIT_MEMINIT
        ldr	r2, =0x1010100
        ldr	r3, =VCOREII_MEMORY_START
memres:
	add	r4, r3, r2
	str	r4, [r1]

        MS_SLEEP r5, r6, 1      /* Sleep 1 msec */

        add	r3, r3, #1
	cmp	r3, #VCOREII_MEMORY_END
	ble	memres

        MS_SLEEP r5, r6, 30     /* Sleep 30 msec */

	/* Reset DLL + memctl (to be safe) */
	ldr	r1, =VTSS_CPU_SYSTEM_CTRL_RESET
        mov	r2, #(VTSS_F_MEMCTRL_DLL_RST_FORCE | VTSS_F_MEMCTRL_RST_FORCE )
	str	r2, [r1]

	/* Change to 178MHz clk - DDR can run at 200MHz. Try 208MHz ? */
	ldr	r0, =VTSS_CPU_SYSTEM_CTRL_CLOCK
	mov	r1, #PLL_DIV_CODE
	str	r1, [r0]

	/* Take DDR DLL out of reset */
	ldr	r1, =VTSS_CPU_SYSTEM_CTRL_RESET
	ldr	r2, [r1]
	bic	r2, r2, #VTSS_F_MEMCTRL_DLL_RST_FORCE
	str	r2, [r1]

	/* wait for ddr DLL lock */
	ldr	r1, =VTSS_MEMCTRL_DDR_DLL_STATUS
memddr_loop1:
	ldr	r2, [r1]
	ands	r2,r2,#VTSS_F_DLL_LOCK
	beq	memddr_loop1

	/* Take DDR memctl out of reset.  */
	ldr	r1, =VTSS_CPU_SYSTEM_CTRL_RESET
	mov	r2, #0
	str	r2, [r1]
	      
	/* wait for DDR controller done */
	ldr	r1, =VTSS_MEMCTRL_SCTLR
memddr_loop2:
	ldr	r2, [r1]
	ands	r2,r2,#VTSS_F_INITIALIZE
	bne	memddr_loop2

	/* configure DDR */
	ldr	r1, =VTSS_MEMCTRL_SCONR
        ldr	r2, =SCONVAL
	str	r2, [r1]
	ldr	r1, =VTSS_MEMCTRL_STMG0R
        ldr	r2, =STMG0RVAL
	str	r2, [r1]
	ldr	r1, =VTSS_MEMCTRL_STMG1R
        ldr	r2, =STMG1RVAL
	str	r2, [r1]
	ldr	r1, =VTSS_MEMCTRL_SCTLR
        ldr	r2, =SCTLVAL
	str	r2, [r1]
	ldr	r1, =VTSS_MEMCTRL_SREFR
        ldr	r2, =SREFVAL
	str	r2, [r1]
                  
        /* Re-Initialize memory controller with new settings */
	ldr	r1, =VTSS_MEMCTRL_SCTLR
	ldr	r2, [r1]
	orr	r2,r2,#VTSS_F_INITIALIZE
	str	r2, [r1]
memddr_loop3:
	ldr	r2, [r1]
	ands	r2,r2,#VTSS_F_INITIALIZE
	bne	memddr_loop3

        /* Memory controller now up - wait 200 clocks to stabilize */
        mov	r0, #200
stabilize:
	subs	r0, r0, #1
	bne	stabilize

relocate:
#if defined(CYG_HAL_STARTUP_ROMRAM)
        ldr     r0,=__rom_vectors_lma   // Relocate FLASH/ROM to SDRAM
        ldr     r1,=VCOREII_SDRAM_PHYS_BASE
        ldr     r2,=__ram_data_end
        add	r2, r2, r1	// RAM phys end
20:
	ldmia	r0!, {r3-r10}
	stmia	r1!, {r3-r10}
	cmp	r1, r2
        blt     20b
#endif /* defined(CYG_HAL_STARTUP_ROMRAM) */

        SET_LED_MACRO 3

	/* take system out of boot-mode */
        ldr	r1, =(0xC0000000 + 0x2 * 4)
	mov	r2, #0
	str	r2, [r1]
	ldr	r2, [r1]

        // Set up a stack [for calling C code]
        ldr     r1,=__startup_stack
        ldr     r2,=VCOREII_ALIAS_PHYS_BASE
        orr     sp,r1,r2

        // Create MMU tables
        bl      hal_mmu_init

	SET_LED_MACRO 4

        // Enable MMU
       	ldr	r1,=MMU_Control_Init|MMU_Control_M
	mcr	MMU_CP,0,r1,MMU_Control,c0
        
        SET_LED_MACRO 4
        .endm
        
#else // defined(CYG_HAL_STARTUP_RAM)
#define PLATFORM_SETUP1
#endif

//-----------------------------------------------------------------------------
// end of hal_platform_setup.h
#endif // CYGONCE_HAL_PLATFORM_SETUP_H
