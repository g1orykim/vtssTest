#ifndef CYGONCE_PLF_IO_H
#define CYGONCE_PLF_IO_H

//=============================================================================
//
//      plf_io.h
//
//      Platform specific IO support
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
// Author(s):    lpovlsen
// Contributors: 
// Date:         2009-10-12
// Purpose:      
// Description:  VCore-III platform IO support
// Usage:        #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/hal_misc.h>
#include <cyg/hal/hal_arch.h>

#ifdef __ASSEMBLER__
#define VTSS_IOREG(t,o)	VTSS_IOADDR(t,o)
#endif

//-----------------------------------------------------------------------------

#if defined(CYG_HAL_VCOREIII_CHIPTYPE_JAGUAR)

#include <cyg/hal/vtss_jaguar_regs.h>

#if 0                           /* Validation board */

/* Micron 2Gb MT47H128M16-3 16Meg x 16 x 8 banks, DDR-533@CL4 @ 4.80ns */
#define VC3_MPAR_bank_addr_cnt    3
#define VC3_MPAR_row_addr_cnt     14
#define VC3_MPAR_col_addr_cnt     10
#define VC3_MPAR_tREFI            1625
#define VC3_MPAR_tRAS_min         9
#define VC3_MPAR_CL               4
#define VC3_MPAR_tWTR             2
#define VC3_MPAR_tRC              12
#define VC3_MPAR_tFAW             11
#define VC3_MPAR_tRP              4
#define VC3_MPAR_tRRD             2
#define VC3_MPAR_tRCD             4
#define VC3_MPAR_tRPA             4
#define VC3_MPAR_tRP              4
#define VC3_MPAR_tMRD             2
#define VC3_MPAR_tRFC             42
#define VC3_MPAR__400_ns_dly      84
#define VC3_MPAR_tWR              4

#else

/* Micron 1Gb MT47H64M16-3 8Meg x 16 x 8 banks, DDR-533@CL4 @ 4.80ns */
#define VC3_MPAR_bank_addr_cnt    3
#define VC3_MPAR_row_addr_cnt     13
#define VC3_MPAR_col_addr_cnt     10
#define VC3_MPAR_tREFI            1625
#define VC3_MPAR_tRAS_min         9
#define VC3_MPAR_CL               4
#define VC3_MPAR_tWTR             2
#define VC3_MPAR_tRC              12
#define VC3_MPAR_tFAW             11
#define VC3_MPAR_tRP              4
#define VC3_MPAR_tRRD             3
#define VC3_MPAR_tRCD             4
#define VC3_MPAR_tRPA             4
#define VC3_MPAR_tRP              4
#define VC3_MPAR_tMRD             2
#define VC3_MPAR_tRFC             27
#define VC3_MPAR__400_ns_dly      84
#define VC3_MPAR_tWR              4

#endif

#elif defined(CYG_HAL_VCOREIII_CHIPTYPE_LUTON26)

#include <cyg/hal/vtss_luton26_regs.h>

/* Micron 1Gb MT47H128M8-3 16Meg x 8 x 8 banks, DDR-533@CL4 @ 4.80ns */
#define VC3_MPAR_bank_addr_cnt    3
#define VC3_MPAR_row_addr_cnt     14
#define VC3_MPAR_col_addr_cnt     10
#define VC3_MPAR_tREFI            1625
#define VC3_MPAR_tRAS_min         9
#define VC3_MPAR_CL               4
#define VC3_MPAR_tWTR             2
#define VC3_MPAR_tRC              12
#define VC3_MPAR_tFAW             8
#define VC3_MPAR_tRP              4
#define VC3_MPAR_tRRD             2
#define VC3_MPAR_tRCD             4
#define VC3_MPAR_tRPA             4
#define VC3_MPAR_tRP              4
#define VC3_MPAR_tMRD             2
#define VC3_MPAR_tRFC             27
#define VC3_MPAR__400_ns_dly      84
#define VC3_MPAR_tWR              4

#else

#error Undefined VCore-III chip type

#endif

// Memory configuration parameters

#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

#ifdef CYGNUM_HAL_MIPS_VCOREIII_MEMORY_16BIT
#define VC3_MPAR_BURST_LENGTH 4 // in DDR2 16-bit mode, use burstlen 4
#define VC3_MPAR_BURST_SIZE   0 // Could be 1 for DDR3
#else // 8-bit IF
#define VC3_MPAR_BURST_LENGTH 8 // For 8-bit IF we must run burst-8
#define VC3_MPAR_BURST_SIZE   0 // Always 0 for 8-bit if
#endif

#define VTSS_MEMPARM_MEMCFG                                             \
    (VC3_MPAR_BURST_SIZE ? VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_CFG_BURST_SIZE : 0) | \
    (VC3_MPAR_BURST_LENGTH == 8 ? VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_CFG_BURST_LEN : 0) | \
    (VC3_MPAR_bank_addr_cnt == 3 ? VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_CFG_BANK_CNT : 0) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_CFG_MSB_ROW_ADDR(VC3_MPAR_row_addr_cnt-1) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_CFG_MSB_COL_ADDR(VC3_MPAR_col_addr_cnt-1)

#define VTSS_MEMPARM_PERIOD                                             \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_REF_PERIOD_MAX_PEND_REF(1) |        \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_REF_PERIOD_REF_PERIOD(VC3_MPAR_tREFI)

#define VTSS_MEMPARM_TIMING0                                            \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING0_RAS_TO_PRECH_DLY(VC3_MPAR_tRAS_min-1) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING0_WR_TO_PRECH_DLY(VC3_MPAR_CL + \
                                                            (VC3_MPAR_BURST_LENGTH == 8 ? 2 : 0) + \
                                                            VC3_MPAR_tWR) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING0_RD_TO_PRECH_DLY((VC3_MPAR_BURST_LENGTH == 8 ? 3 : 1)) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING0_WR_DATA_XFR_DLY(VC3_MPAR_CL-3) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING0_RD_DATA_XFR_DLY(VC3_MPAR_CL-3)

#define VTSS_MEMPARM_TIMING1                                            \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING1_RAS_TO_RAS_SAME_BANK_DLY(VC3_MPAR_tRC-1) | \
VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING1_BANK8_FAW_DLY(MAX(0,VC3_MPAR_tFAW-1)) | \
VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING1_PRECH_TO_RAS_DLY(VC3_MPAR_tRP-1) | \
VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING1_RAS_TO_RAS_DLY(VC3_MPAR_tRRD-1) | \
VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING1_RAS_TO_CAS_DLY(VC3_MPAR_tRCD-1) | \
VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING1_WR_TO_RD_DLY(VC3_MPAR_CL + \
                                                     (VC3_MPAR_BURST_LENGTH == 8 ? 2 : 0) + \
                                                     MAX(2,VC3_MPAR_tWTR))
#if (VC3_MPAR_tRPA > 0)
#define VTSS_MEMPARM_TIMING2                                            \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING2_PRECH_ALL_DLY(VC3_MPAR_tRPA-1) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING2_MDSET_DLY(VC3_MPAR_tMRD-1) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING2_REF_DLY(VC3_MPAR_tRFC-1) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING2_FOUR_HUNDRED_NS_DLY(VC3_MPAR__400_ns_dly)
#else
#define VTSS_MEMPARM_TIMING2                                            \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING2_PRECH_ALL_DLY(VC3_MPAR_tRP-1) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING2_MDSET_DLY(VC3_MPAR_tMRD-1) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING2_REF_DLY(VC3_MPAR_tRFC-1) |    \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING2_FOUR_HUNDRED_NS_DLY(VC3_MPAR__400_ns_dly)
#endif

#define VTSS_MEMPARM_TIMING3 \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING3_WR_TO_RD_CS_CHANGE_DLY(MAX(3,VC3_MPAR_CL-1)) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING3_ODT_WR_DLY(VC3_MPAR_CL-1) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING3_LOCAL_ODT_RD_DLY(VC3_MPAR_CL-1)

#define VTSS_MEMPARM_MR0 \
	/* Assumes DDR2 operation */ \
	(VC3_MPAR_BURST_LENGTH == 8 ? 3 : 2) | (VC3_MPAR_CL << 4) | ((VC3_MPAR_tWR-1) << 9)

// Additional (target) offsets

#define VTSS_DDR_TO     0x20000000  /* DDR RAM base offset */
#define VTSS_MEMCTL1_TO 0x40000000  /* SPI/PI base offset */
#define VTSS_MEMCTL2_TO 0x50000000  /* SPI/PI base offset */
#define VTSS_FLASH_TO   VTSS_MEMCTL1_TO /* Flash base offset */

#define VTSS_IO_PI_REGION(cs)    (VTSS_MEMCTL2_TO + (0x4000000*(cs)))

// ----------------------------------------------------------------------------
// exported I2C devices on OEM board
//
#define HAL_I2C_EXPORTED_DEVICES                                \
    extern cyg_i2c_device         i2c_si570_vcxo;               \
    extern cyg_i2c_bus            hal_vcore_i2c_bus;

#ifndef __ASSEMBLER__

externC void vcoreiii_io_writel(cyg_uint32 val, volatile void *addr);
externC cyg_uint32 vcoreiii_io_readl(volatile void *addr);
externC void vcoreiii_io_clr(cyg_uint32 mask, volatile void *addr);
externC void vcoreiii_io_set(cyg_uint32 mask, volatile void *addr);
externC void vcoreiii_io_mask_set(volatile void *addr, 
                                  cyg_uint32 clr_mask, 
                                  cyg_uint32 set_mask);

#if defined(CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR)
externC void vcoreiii_io_sec_writel(cyg_uint32 val, volatile void *addr);
externC cyg_uint32 vcoreiii_io_sec_readl(volatile void *addr);
externC void vcoreiii_io_clr_sec(volatile void *addr, cyg_uint32 mask);
externC void vcoreiii_io_set_sec(volatile void *addr, cyg_uint32 mask);
externC void vcoreiii_io_mask_set_sec(volatile void *addr, 
                                      cyg_uint32 clr_mask, 
                                      cyg_uint32 set_mask);
#endif  /* CYG_HAL_MIPS_VCOREIII_DUAL_JAGUAR */

#endif  /* __ASSEMBLER__ */

//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_PLF_IO_H
