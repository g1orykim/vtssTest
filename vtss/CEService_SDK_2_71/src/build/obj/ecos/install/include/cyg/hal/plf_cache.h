#ifndef CYGONCE_PLF_CACHE_H
#define CYGONCE_PLF_CACHE_H

//=============================================================================
//
//      plf_cache.h
//
//      HAL cache control API
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
// Author(s):   nickg
// Contributors:nickg
// Date:        2001-03-20
// Purpose:     Cache control API
// Description: The macros defined here provide the HAL APIs for handling
//              cache control operations.
// Usage:
//              #include <cyg/hal/plf_cache.h>
//              ...
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/infra/cyg_type.h>

#include <cyg/hal/hal_arch.h>

// Data cache
#define HAL_DCACHE_SIZE                 32768   // Size of data cache in bytes
#define HAL_DCACHE_LINE_SIZE            32      // Size of a data cache line
#define HAL_DCACHE_WAYS                 4       // Associativity of the cache

// Instruction cache
#define HAL_ICACHE_SIZE                 32768   // Size of data cache in bytes
#define HAL_ICACHE_LINE_SIZE            32      // Size of a data cache line
#define HAL_ICACHE_WAYS                 4       // Associativity of the cache

// Cache operational mode, these defines has been added to get arround a flaw
// in the existing code. The HAL_CACHE_CONFIG_K0_VAL (on a MIPS 24K) is encoded
// as follows:
//   0 = cache-enabled, write-through, no write-allocate.
//   3 = cache-enabled, write-back, write-allocate.
// If using write-through "MM" (bit 18) has to be disabled too.
#define HAL_CACHE_CONFIG_K0_MSK 7
#define HAL_CACHE_CONFIG_MM     (1 << 18)
#define HAL_CACHE_CONFIG_SET_K0(config0)                                \
    CYG_MACRO_START                                                     \
    config0 &= ~HAL_CACHE_CONFIG_K0_MSK;                                \
    if(VTSS_X_DEVCPU_GCB_CHIP_REGS_CHIP_ID_REV_ID(VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_ID) > 0) { \
        config0 |= HAL_CACHE_CONFIG_MM;                                 \
        config0 |= 3;           /* 3 = cache-enabled, write-back */     \
    } else {                                                            \
        config0 &= ~HAL_CACHE_CONFIG_MM;                                \
        /* 0 = cache-enabled, write-through */                          \
    }                                                                   \
    CYG_MACRO_END

//=============================================================================

// This macro has been modified to work with MIPS 24K. The corresponding variant versions
//   access the wrong coprocessor 0 regs. We still use variant defines here though.
#define HAL_DCACHE_INVALIDATE_ALL_DEFINED
#define HAL_DCACHE_INVALIDATE_ALL()                                                     \
    CYG_MACRO_START                                                                     \
    register volatile CYG_BYTE *addr;                                                   \
    asm volatile (" mtc0 $0, $28, 2;"                                   \
                  " nop;"                                               \
                  " nop;"                                               \
                  " nop;");                                             \
    for (addr = (CYG_BYTE *)CYGARC_KSEG_CACHED_BASE;                                    \
         addr < (CYG_BYTE *)(CYGARC_KSEG_CACHED_BASE + HAL_DCACHE_SIZE);                \
         addr += HAL_DCACHE_LINE_SIZE )                                                 \
    {                                                                                   \
        asm volatile (" cache %0, 0(%1)"                                                \
                      :                                                                 \
                      : "I" (HAL_CACHE_OP(HAL_WHICH_DCACHE, HAL_INDEX_STORE_TAG)),      \
                        "r"(addr));                                                     \
    }                                                                                   \
    CYG_MACRO_END

// This macro has been modified to work with MIPS 24K. The corresponding variant versions
//   access the wrong coprocessor 0 regs. We still use variant defines here though.
#define HAL_ICACHE_INVALIDATE_ALL_DEFINED
#define HAL_ICACHE_INVALIDATE_ALL()                                                     \
    CYG_MACRO_START                                                                     \
    register volatile CYG_BYTE *addr;                                                   \
    asm volatile (" mtc0 $0, $28, 0;"                                   \
                  " nop;"                                               \
                  " nop;"                                               \
                  " nop;");                                             \
    for (addr = (CYG_BYTE *)CYGARC_KSEG_CACHED_BASE;                                    \
         addr < (CYG_BYTE *)(CYGARC_KSEG_CACHED_BASE + HAL_ICACHE_SIZE);                \
         addr += HAL_ICACHE_LINE_SIZE )                                                 \
    {                                                                                   \
        asm volatile (" cache %0, 0(%1)"                                                \
                      :                                                                 \
                      : "I" (HAL_CACHE_OP(HAL_WHICH_ICACHE, HAL_INDEX_STORE_TAG)),      \
                        "r"(addr));                                                     \
    }                                                                                   \
    CYG_MACRO_END

#define HAL_DCACHE_SYNC_DEFINED
#define HAL_DCACHE_SYNC()                                               \
    CYG_MACRO_START                                                     \
    register volatile CYG_BYTE *addr;                                   \
    for (addr = (CYG_BYTE *)CYGARC_KSEG_CACHED_BASE;                    \
         addr < (CYG_BYTE *)(CYGARC_KSEG_CACHED_BASE + HAL_ICACHE_SIZE); \
         addr += HAL_ICACHE_LINE_SIZE )                                 \
    {                                                                   \
        asm volatile (" cache %0, 0(%1)"                                \
                      :                                                 \
                      : "I" (HAL_CACHE_OP(HAL_WHICH_DCACHE, HAL_INDEX_INVALIDATE)), \
                        "r"(addr));                                     \
    }                                                                   \
    CYG_MACRO_END

// Write dirty cache lines to memory and invalidate the cache entries
// for the given address range.
#define HAL_DCACHE_FLUSH_DEFINED
#define HAL_DCACHE_FLUSH(_base_, _asize_)                               \
    CYG_MACRO_START                                                     \
    register CYG_ADDRESS addr, enda;                                    \
    asm volatile (" mtc0 $0, $28, 2;"                                   \
                  " nop;"                                               \
                  " nop;"                                               \
                  " nop;");                                             \
    for (addr = (CYG_ADDRESS)(_base_) & (~(HAL_DCACHE_LINE_SIZE - 1)),  \
         enda = (CYG_ADDRESS)(_base_) + (_asize_);                      \
         addr < enda;                                                   \
         addr += HAL_DCACHE_LINE_SIZE)                                  \
        asm volatile (" cache %0, 0(%1)"                                \
                      :                                                 \
                      : "I" (HAL_CACHE_OP(HAL_WHICH_DCACHE, HAL_DCACHE_HIT_INVALIDATE)), \
                        "r"(addr));                                     \
    CYG_MACRO_END

// Write dirty cache lines to memory for the given address range.
#define HAL_DCACHE_STORE_DEFINED
#define HAL_DCACHE_STORE(_base_ , _asize_)                              \
    CYG_MACRO_START                                                     \
    register CYG_ADDRESS addr, enda;                                    \
    for (addr = (CYG_ADDRESS)(_base_) & (~(HAL_DCACHE_LINE_SIZE - 1)),  \
         enda = (CYG_ADDRESS)(_base_) + (_asize_);                      \
         addr < enda;                                                   \
         addr += HAL_DCACHE_LINE_SIZE)                                  \
        asm volatile (" cache %0, 0(%1)"                                \
                      :                                                 \
                      : "I" (HAL_CACHE_OP(HAL_WHICH_DCACHE, HAL_DCACHE_HIT_WRITEBACK)), \
                        "r"(addr));                                     \
    CYG_MACRO_END

//-----------------------------------------------------------------------------
#endif // ifndef CYGONCE_PLF_CACHE_H
// End of plf_cache.h

