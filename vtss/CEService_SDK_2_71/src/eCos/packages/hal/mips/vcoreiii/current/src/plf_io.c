//==========================================================================
//
//      plf_io.c
//
//      HAL platform I/O functions
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
// Purpose:      HAL I/O functions
// Description:  This file contains I/O functions provided by the
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

cyg_uint32 vcoreiii_io_readl(volatile void *addr)
{
    return *((volatile cyg_uint32*)addr);
}

void vcoreiii_io_writel(cyg_uint32 val, volatile void *addr)
{
    *((volatile cyg_uint32*)addr) = val;
}

void vcoreiii_io_clr(cyg_uint32 mask, volatile void *addr)
{
    (*((volatile cyg_uint32*)addr)) &= ~mask;
}

void vcoreiii_io_set(cyg_uint32 mask, volatile void *addr)
{
    (*((volatile cyg_uint32*)addr)) |= mask;
}

void vcoreiii_io_mask_set(volatile void *addr, 
                          cyg_uint32 clr_mask, 
                          cyg_uint32 set_mask)
{
    cyg_uint32 val = (*((volatile cyg_uint32*)addr));
    val &= ~clr_mask;
    val |= set_mask;
    (*((volatile cyg_uint32*)addr)) = val;
}

#if defined(CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR)

/* Secondary IO support */
static inline volatile void *primary2secondary_addr(volatile void *addr)
{
    volatile void *slvaddr = 
        (volatile void *) 
        (((cyg_uint32)addr) - 
         ((cyg_uint32)VTSS_IO_ORIGIN1_OFFSET) + 
         ((cyg_uint32)VTSS_IO_PI_REGION(CYGNUM_HAL_MIPS_VCOREIII_DUAL_JAGUAR_SECONDARY_CS)));
    return slvaddr;
}

static void sec_direct_writel(cyg_uint32 val, volatile void *addr)
{
    vcoreiii_io_writel(val, primary2secondary_addr(addr));
}

static cyg_uint32 sec_direct_readl(volatile void *addr)
{
    cyg_uint32 r = vcoreiii_io_readl(primary2secondary_addr(addr));
    return r;
}

static int sec_region1_accessible(cyg_uint32 addr)
{
    return
        (addr > (cyg_uint32) VTSS_IO_ORIGIN1_OFFSET) &&
        (addr < (((cyg_uint32)VTSS_IO_ORIGIN1_OFFSET) + VTSS_IO_ORIGIN1_SIZE));
}

static cyg_uint32 sec_region2_offset(volatile cyg_uint32 *addr)
{
    return 0x70000000 + (((cyg_uint32)addr) - ((cyg_uint32)VTSS_IO_ORIGIN2_OFFSET));
}

static cyg_uint32 sec_indirect_readl(volatile cyg_uint32 *addr)
{
    cyg_uint32 val, ctl;
    sec_direct_writel(sec_region2_offset(addr), &VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_ADDR);
    val = sec_direct_readl(&VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_DATA);
    do {
        ctl = sec_direct_readl(&VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL);
    } while(ctl & VTSS_F_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL_VA_BUSY);
    if(ctl & VTSS_F_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL_VA_ERR) {
        diag_printf("Secondary Read error on address %p, ctl = 0x%08x\n", addr, ctl);
    }
    val = sec_direct_readl(&VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_DATA_INERT);
    return val;
}

static void sec_indirect_writel(cyg_uint32 val, volatile cyg_uint32 *addr)
{
    cyg_uint32 ctl;
    sec_direct_writel(sec_region2_offset(addr), &VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_ADDR);
    sec_direct_writel(val, &VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_DATA);
    do {
        ctl = sec_direct_readl(&VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL);
    } while(ctl & VTSS_F_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL_VA_BUSY);
    if(ctl & VTSS_F_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL_VA_ERR) {
        diag_printf("Write error on address %p, ctl = 0x%08x\n", addr, ctl);
    }
}

void vcoreiii_io_sec_writel(cyg_uint32 val, volatile void *addr)
{
    if(sec_region1_accessible((cyg_uint32)addr))
        sec_direct_writel(val, addr);
    else
        sec_indirect_writel(val, addr);
}

cyg_uint32 vcoreiii_io_sec_readl(volatile void *addr)
{
    if(sec_region1_accessible((cyg_uint32)addr))
        return sec_direct_readl(addr);
    return sec_indirect_readl(addr);
}

void vcoreiii_io_clr_sec(volatile void *addr, cyg_uint32 mask)
{
    vcoreiii_io_sec_writel(vcoreiii_io_sec_readl(addr) & ~mask, addr);
}

void vcoreiii_io_set_sec(volatile void *addr, cyg_uint32 mask)
{
    vcoreiii_io_sec_writel(vcoreiii_io_sec_readl(addr) | mask, addr);
}

void vcoreiii_io_mask_set_sec(volatile void *addr, cyg_uint32 clr_mask, cyg_uint32 set_mask)
{
    cyg_uint32 val = vcoreiii_io_sec_readl(addr);
    val &= ~clr_mask;
    val |= set_mask;
    vcoreiii_io_sec_writel(val, addr);
}
#endif  /* CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR */

