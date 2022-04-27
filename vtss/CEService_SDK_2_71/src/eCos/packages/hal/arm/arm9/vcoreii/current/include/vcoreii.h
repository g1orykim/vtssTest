#ifndef _VTSS_H_VCOREII_
#define _VTSS_H_VCOREII_

//=============================================================================
//
//      vcoreii.h
//
//      Platform specific support (register layout, etc)
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
// Author(s):    Lars Povlsen
// Contributors:
// Date:         2006-03-22
// Purpose:      VCore-II platform specific support routines
// Description: 
// Usage:        #include <cyg/hal/vcoreii.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <cyg/hal/vcoreii_amba_top.h>

#define VCOREII_ALIAS_PHYS_BASE 0x00000000
#define VCOREII_SDRAM_PHYS_BASE 0x40000000
#define VCOREII_FLASH_PHYS_BASE 0x80000000
#define VCOREII_SWC_PHYS_BASE	0xa0000000

/* Aligning with VCOREIII */
#define VTSS_FLASH_TO		VCOREII_FLASH_PHYS_BASE

#define VCOREII_SWC_REG(b, s, r) (VCOREII_SWC_PHYS_BASE + ((((b) << 12) + ((s) << 8) + (r)) << 2))

#define VCOREII_MEMINIT_MEMINIT	VCOREII_SWC_REG(3, 2, 0)
#define VCOREII_MEMINIT_MEMRES	VCOREII_SWC_REG(3, 2, 1)
#define VCOREII_SYSTEM_GLORESET (*((volatile unsigned long*)(VCOREII_SWC_REG(7, 0, 20))))
#define VCOREII_F_SYSTEM_GLORESET_STROBE       VTSS_BIT(4)
#define VCOREII_F_SYSTEM_GLORESET_ICPU_LOCK    VTSS_BIT(3)
#define VCOREII_F_SYSTEM_GLORESET_MEM_LOCK     VTSS_BIT(2)
#define VCOREII_F_SYSTEM_GLORESET_MASTER_RESET VTSS_BIT(0)
#define VCOREII_SYSTEM_MBOX_VAL (*((volatile unsigned long*)(VCOREII_SWC_REG(7, 0, 0x15))))
#define VCOREII_SYSTEM_MBOX_SET (*((volatile unsigned long*)(VCOREII_SWC_REG(7, 0, 0x16))))
#define VCOREII_SYSTEM_MBOX_CLR (*((volatile unsigned long*)(VCOREII_SWC_REG(7, 0, 0x17))))

/* VCOREII/ARM memory blocks - inclusive */
#define VCOREII_MEMORY_START	37
#define VCOREII_MEMORY_END	50

#define VCOREII_BLOCK_SYS	7
#define VCOREII_SYS_GPIO	0x33
#define VCOREII_SYS_GPIO_DATA	0x34

#if defined(CYGNUM_HAL_ARM_VCOREII_AHB_CLOCK)
#if (CYGNUM_HAL_ARM_VCOREII_AHB_CLOCK == 104)
#define VCOREII_AHB_CLOCK_FREQ	104170000
#define VCOREII_AHB_CLOCK_CODE	0x9
#elif (CYGNUM_HAL_ARM_VCOREII_AHB_CLOCK == 125)
#define VCOREII_AHB_CLOCK_FREQ 125000000
#define VCOREII_AHB_CLOCK_CODE	0x5
#elif (CYGNUM_HAL_ARM_VCOREII_AHB_CLOCK == 156)
#define VCOREII_AHB_CLOCK_FREQ 156250000
#define VCOREII_AHB_CLOCK_CODE	0x1
#elif (CYGNUM_HAL_ARM_VCOREII_AHB_CLOCK == 178)
#define VCOREII_AHB_CLOCK_FREQ 178570000
#define VCOREII_AHB_CLOCK_CODE	0xc
#elif (CYGNUM_HAL_ARM_VCOREII_AHB_CLOCK == 208)
#define VCOREII_AHB_CLOCK_FREQ 208333333
#define VCOREII_AHB_CLOCK_CODE	0x8
#else
#error "Invalid AHB clock setting, valid values are: 104MHz, 125MHz, 156MHz, 178MHz, 208MHz"
#endif
#endif

#endif /* _VTSS_H_VCOREII_ */
