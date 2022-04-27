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

#include <cyg/hal/vtss_serval_regs.h>

#if defined(CYG_HAL_VCOREIII_DDRTYPE_H5TQ1G63BFA) /* Refboard */

/* Hynix H5TQ1G63BFA (1Gbit DDR3, x16) @ 3.20ns */
#define VC3_MPAR_bank_addr_cnt    3
#define VC3_MPAR_row_addr_cnt     13
#define VC3_MPAR_col_addr_cnt     10
#define VC3_MPAR_tREFI            2437
#define VC3_MPAR_tRAS_min         12
#define VC3_MPAR_CL               6
#define VC3_MPAR_tWTR             4
#define VC3_MPAR_tRC              16
#define VC3_MPAR_tFAW             16
#define VC3_MPAR_tRP              5
#define VC3_MPAR_tRRD             4
#define VC3_MPAR_tRCD             5
#define VC3_MPAR_tMRD             4
#define VC3_MPAR_tRFC             35
#define VC3_MPAR_CWL              5
#define VC3_MPAR_tXPR             38
#define VC3_MPAR_tMOD             12
#define VC3_MPAR_tDLLK            512
#define VC3_MPAR_tWR              5

#elif defined(CYG_HAL_VCOREIII_DDRTYPE_MT41J128M16HA) /* Validation board */

/* Micron MT41J128M16HA-15E:D (2Gbit DDR3, x16) @ 3.20ns */
#define VC3_MPAR_bank_addr_cnt    3
#define VC3_MPAR_row_addr_cnt     14
#define VC3_MPAR_col_addr_cnt     10
#define VC3_MPAR_tREFI            2437
#define VC3_MPAR_tRAS_min         12
#define VC3_MPAR_CL               5
#define VC3_MPAR_tWTR             4
#define VC3_MPAR_tRC              16
#define VC3_MPAR_tFAW             16
#define VC3_MPAR_tRP              5
#define VC3_MPAR_tRRD             4
#define VC3_MPAR_tRCD             5
#define VC3_MPAR_tMRD             4
#define VC3_MPAR_tRFC             50
#define VC3_MPAR_CWL              5
#define VC3_MPAR_tXPR             54
#define VC3_MPAR_tMOD             12
#define VC3_MPAR_tDLLK            512
#define VC3_MPAR_tWR              5

#else

#error Unknown DDR system configuration - please add!

#endif

// Memory configuration parameters

#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#endif

#ifdef CYGNUM_HAL_MIPS_VCOREIII_MEMORY_16BIT
 #define VC3_MPAR_16BIT       1
#else
 #define VC3_MPAR_16BIT       0
#endif

#ifdef CYGNUM_HAL_MIPS_VCOREIII_MEMORY_DDR3
 #define VC3_MPAR_DDR3_MODE    1 /* DDR3 */
 #define VC3_MPAR_BURST_LENGTH 8 /* Always 8 (1) for DDR3 */
 #ifdef CYGNUM_HAL_MIPS_VCOREIII_MEMORY_16BIT
  #define VC3_MPAR_BURST_SIZE   1 // Always 1 for DDR3/16bit
 #else
  #define VC3_MPAR_BURST_SIZE   0 /* else */
 #endif
#else
 #define VC3_MPAR_DDR3_MODE    0 /* DDR2 */
 #ifdef CYGNUM_HAL_MIPS_VCOREIII_MEMORY_16BIT
  #define VC3_MPAR_BURST_LENGTH 4 /* in DDR2 16-bit mode, use burstlen 4 */
 #else
  #define VC3_MPAR_BURST_LENGTH 8 /* For 8-bit IF we must run burst-8 */
 #endif
 #define VC3_MPAR_BURST_SIZE   0 /* Always 0 for DDR2 */
#endif

#define VC3_MPAR_RL VC3_MPAR_CL
#if !defined(CYGNUM_HAL_MIPS_VCOREIII_MEMORY_DDR3)
#define VC3_MPAR_WL (VC3_MPAR_RL-1)
#define VC3_MPAR_MD VC3_MPAR_tMRD
#define VC3_MPAR_ID VC3_MPAR__400_ns_dly
#define VC3_MPAR_SD VC3_MPAR_tXSRD
#define VC3_MPAR_OW (VC3_MPAR_WL-2)
#define VC3_MPAR_OR (VC3_MPAR_WL-3)
#define VC3_MPAR_RP (VC3_MPAR_bank_addr_cnt < 3 ? VC3_MPAR_tRP : VC3_MPAR_tRPA)
#define VC3_MPAR_FAW (VC3_MPAR_bank_addr_cnt < 3 ? 1 : VC3_MPAR_tFAW)
#define VC3_MPAR_BL (VC3_MPAR_BURST_LENGTH == 4 ? 2 : 4)
#define VTSS_MEMPARM_MR0 \
	(VC3_MPAR_BURST_LENGTH == 8 ? 3 : 2) | (VC3_MPAR_CL << 4) | ((VC3_MPAR_tWR-1) << 9)
#define VTSS_MEMPARM_MR1 0x3c2 /* DLL-on, Full-OD, AL=0, RTT=120 ohm, nDQS-on, RDQS-off, out-en */
#define VTSS_MEMPARM_MR2 0
#define VTSS_MEMPARM_MR3 0
#else
#define VC3_MPAR_WL VC3_MPAR_CWL
#define VC3_MPAR_MD VC3_MPAR_tMOD
#define VC3_MPAR_ID VC3_MPAR_tXPR
#define VC3_MPAR_SD VC3_MPAR_tDLLK
#define VC3_MPAR_OW 2
#define VC3_MPAR_OR 2
#define VC3_MPAR_RP VC3_MPAR_tRP
#define VC3_MPAR_FAW VC3_MPAR_tFAW
#define VC3_MPAR_BL 4
#define VTSS_MEMPARM_MR0 ((VC3_MPAR_RL - 4) << 4) | ((VC3_MPAR_tWR - 4) << 9)
#define VTSS_MEMPARM_MR1 0x0040 /* ODT_RTT: 120 ohm */
#define VTSS_MEMPARM_MR2 ((VC3_MPAR_WL - 5) << 3) 
#define VTSS_MEMPARM_MR3 0
#endif  /* CYGNUM_HAL_MIPS_VCOREIII_MEMORY_DDR3 */

#define VTSS_MEMPARM_MEMCFG                                             \
    (VC3_MPAR_16BIT ? VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_CFG_DDR_WIDTH : 0) | \
    (VC3_MPAR_DDR3_MODE ? VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_CFG_DDR_MODE : 0) | \
    (VC3_MPAR_BURST_SIZE ? VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_CFG_BURST_SIZE : 0) | \
    (VC3_MPAR_BURST_LENGTH == 8 ? VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_CFG_BURST_LEN : 0) | \
    (VC3_MPAR_bank_addr_cnt == 3 ? VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_CFG_BANK_CNT : 0) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_CFG_MSB_ROW_ADDR(VC3_MPAR_row_addr_cnt-1) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_CFG_MSB_COL_ADDR(VC3_MPAR_col_addr_cnt-1)

#define VTSS_MEMPARM_PERIOD                                             \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_REF_PERIOD_MAX_PEND_REF(8) |        \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_REF_PERIOD_REF_PERIOD(VC3_MPAR_tREFI)

#define VTSS_MEMPARM_TIMING0                                            \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING0_RD_TO_WR_DLY(VC3_MPAR_RL+VC3_MPAR_BL+1-VC3_MPAR_WL) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING0_WR_CS_CHANGE_DLY(VC3_MPAR_BL-1) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING0_RD_CS_CHANGE_DLY(VC3_MPAR_BL) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING0_RAS_TO_PRECH_DLY(VC3_MPAR_tRAS_min-1) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING0_WR_TO_PRECH_DLY(VC3_MPAR_WL + \
                                                            VC3_MPAR_BL + \
                                                            VC3_MPAR_tWR - 1) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING0_RD_TO_PRECH_DLY(VC3_MPAR_BL-1) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING0_WR_DATA_XFR_DLY(VC3_MPAR_WL-1) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING0_RD_DATA_XFR_DLY(VC3_MPAR_RL-3)

#define VTSS_MEMPARM_TIMING1                                            \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING1_RAS_TO_RAS_SAME_BANK_DLY(VC3_MPAR_tRC-1) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING1_BANK8_FAW_DLY(VC3_MPAR_FAW-1) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING1_PRECH_TO_RAS_DLY(VC3_MPAR_RP-1) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING1_RAS_TO_RAS_DLY(VC3_MPAR_tRRD-1) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING1_RAS_TO_CAS_DLY(VC3_MPAR_tRCD-1) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING1_WR_TO_RD_DLY(VC3_MPAR_WL +  \
                                                         VC3_MPAR_BL +  \
                                                         VC3_MPAR_tWTR - 1)

#define VTSS_MEMPARM_TIMING2                                            \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING2_PRECH_ALL_DLY(VC3_MPAR_RP-1) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING2_MDSET_DLY(VC3_MPAR_MD-1) |  \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING2_REF_DLY(VC3_MPAR_tRFC-1) |  \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING2_INIT_DLY(VC3_MPAR_ID-1)

#define VTSS_MEMPARM_TIMING3 \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING3_WR_TO_RD_CS_CHANGE_DLY(VC3_MPAR_WL + VC3_MPAR_tWTR - 1) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING3_ODT_RD_DLY(VC3_MPAR_OR-1) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING3_ODT_WR_DLY(VC3_MPAR_OW-1) | \
    VTSS_F_ICPU_CFG_MEMCTRL_MEMCTRL_TIMING3_LOCAL_ODT_RD_DLY(VC3_MPAR_RL-1)

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
    extern cyg_i2c_bus            hal_vcore_i2c_bus;

#ifndef __ASSEMBLER__

externC void vcoreiii_io_writel(cyg_uint32 val, volatile void *addr);
externC cyg_uint32 vcoreiii_io_readl(volatile void *addr);
externC void vcoreiii_io_clr(cyg_uint32 mask, volatile void *addr);
externC void vcoreiii_io_set(cyg_uint32 mask, volatile void *addr);
externC void vcoreiii_io_mask_set(volatile void *addr, 
                                  cyg_uint32 clr_mask, 
                                  cyg_uint32 set_mask);

#endif  /* __ASSEMBLER__ */

//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_PLF_IO_H
