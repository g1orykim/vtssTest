##=============================================================================
##
##	vcoreiii_misc.S
##
##	Misc utility functions for diagnostics
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

#include <pkgconf/system.h>	
#include <pkgconf/hal.h>

#ifdef CYGPKG_KERNEL
# include <pkgconf/kernel.h>	
#endif
			
#include <cyg/hal/arch.inc>	
#include <cyg/hal/hal_arch.h>	

FUNC_START(vcoreiii_zero_mem)
#ifdef CYGHWR_HAL_MIPS_64BIT
#define STORE_OP        sd
#define BLOCK_SHIFT     6
#else
#define STORE_OP        sw
#define BLOCK_SHIFT     5
#endif
        andi    a2,a0,mips_regsize-1    # is bss aligned?
        bne     a2,zero,1f              # skip word copy
        nop
        
        # loop with 8 stores per loop
        subu            a3,a1,a0                # get length
        srl             a3,a3,BLOCK_SHIFT       # get number of blocks
        sll             a3,a3,BLOCK_SHIFT       # get length of blocks
        addu            a3,a0,a3                # get end addr of blocks
2:      STORE_OP        zero,(mips_regsize*0)(a0)
        STORE_OP        zero,(mips_regsize*1)(a0)
        STORE_OP        zero,(mips_regsize*2)(a0)
        STORE_OP        zero,(mips_regsize*3)(a0)
        STORE_OP        zero,(mips_regsize*4)(a0)
        STORE_OP        zero,(mips_regsize*5)(a0)
        STORE_OP        zero,(mips_regsize*6)(a0)
        STORE_OP        zero,(mips_regsize*7)(a0)
        addu            a0,a0,mips_regsize*8    # next addr
        bne             a3,a0,2b                # to next store
        nop

        # If length is a multiple of block size then we
        # are done and need to skip the byte loop
        beq		a1,a0,3f
        nop
	
        # finish 1 byte at a time
1:      sb      zero,0(a0)              # zero memory
        addiu   a0,a0,1                 # next addr
        bne     a0,a1,1b                # to next store
        nop
3:      jr      ra
        nop
FUNC_END(vcoreiii_zero_mem)

        
