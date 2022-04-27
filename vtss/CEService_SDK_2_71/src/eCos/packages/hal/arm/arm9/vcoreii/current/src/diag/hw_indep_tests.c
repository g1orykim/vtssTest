//==========================================================================
//
//      hw_indep_tests.c
//
//      HAL support for H/W independent Power-On-Self-Test for VCOREII
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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
// Author(s):    Rene Schipp von Branitz Nielsen
// Contributors:
// Date:         2009-04-16
// Purpose:      HAL board support
// Description:  Implementations of HAL POST.
//
//####DESCRIPTIONEND####

#include <redboot.h>
#include "vcoreii_diag.h"
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/vcoreii.h>

#define PORT_MIN  0
#define PORT_MAX 27

static inline volatile unsigned long *__mac_reg__(int port, unsigned int reg)
{
    return ((port) < 16 ? ((volatile unsigned long *)(VCOREII_SWC_REG(1, (port) -  0, (reg)))) : ((volatile unsigned long *)(VCOREII_SWC_REG(6, (port) - 16, (reg)))));
}

#define ANA_REG(reg)                (*((volatile unsigned long *)(VCOREII_SWC_REG(2, 0, (reg)))))
#define SYS_REG(reg)                (*((volatile unsigned long *)(VCOREII_SWC_REG(7, 0, (reg)))))
#define BIST_REG(reg)               (*((volatile unsigned long *)(VCOREII_SWC_REG(3, 2, (reg)))))
#define CPUQ_REG(qu, reg)           (*((volatile unsigned long *)(VCOREII_SWC_REG(4, 2 * (qu), (reg)))))
#define MAC_REG(port, reg)          (*__mac_reg__ (port, reg))
#define VAUI_LANE_REG(lane_id, reg) (*((volatile unsigned long *)(VCOREII_SWC_REG(3, 4, (reg) + 4 * (lane_id)))))

#if defined(CYGBLD_BUILD_POST_HW_DEP) && CYGBLD_BUILD_POST_HW_DEP
  // These functions must be implemented in hw_dep_tests.c.
  extern void vcoreii_diag_hw_dep_subtest_begin(vcoreii_diag_subtests_t subtest);
  extern void vcoreii_diag_hw_dep_subtest_progress(vcoreii_diag_subtests_t subtest);
  extern void vcoreii_diag_hw_dep_subtest_end(vcoreii_diag_subtests_t subtest);
#endif

/******************************************************************************/
// membist()
/******************************************************************************/
static int membist(bool show_progress, vcoreii_diag_err_info_t *err_info)
{
  const char *test_name = "Memory BIST";
  cyg_uint32 val;
  int i, iteration, was_dcache_on, was_icache_on, failing_ram = 0, result = 1;

#if defined(CYGBLD_BUILD_POST_HW_DEP) && CYGBLD_BUILD_POST_HW_DEP
  // Allow for H/W dependent initialization (like turning on a LED).
  vcoreii_diag_hw_dep_subtest_begin(VCOREII_DIAG_SUBTEST_MEMBIST);
#endif

  // We gotta disable caches before messing with the RAMs (making up the caches).
  HAL_DCACHE_IS_ENABLED(was_dcache_on);
  HAL_ICACHE_IS_ENABLED(was_icache_on);
  if(was_dcache_on) {
    HAL_DCACHE_SYNC();
    HAL_DCACHE_INVALIDATE_ALL();
    HAL_DCACHE_DISABLE();
  }
  if(was_icache_on) {
    HAL_ICACHE_INVALIDATE_ALL();
    HAL_ICACHE_DISABLE();
  }

  if(show_progress) {
    diag_printf("%s: Running...", test_name);
  }

  for(iteration = 0; iteration < 2; iteration++) {
    for(i = 0; i <= VCOREII_MEMORY_END; i++) {
      if(i == 7 || i == 8 || i == 37) {
        // Skip the two 8051 RAMs and the MMU tag RAM
        continue;
      }
      if(iteration == 0) {
        // Start BIST. Data written indicates: WRITE | COMMAND | NORMAL_BIST_EN | RAM_ID
        BIST_REG(0) = 0x01010100 + i;
        CYGACC_CALL_IF_DELAY_US((cyg_int32)1000);
      } else {
        // Read result. Data written indicates: READ | RESULT | RAM_ID
        BIST_REG(0) = 0x00020000 + i;
        CYGACC_CALL_IF_DELAY_US((cyg_int32)1000);
        val = BIST_REG(1);
        if(val & 0x1) {
          if((val & 0x2) == 0) {
            diag_printf("\nError: %s: BIST failed for RAM #%d\n", test_name, i);
            failing_ram = i;
            result = 0;
            goto do_exit;
          }
        } else {
          diag_printf("\nError: %s: BIST for RAM #%d not finalized within expected timeframe.\n", test_name, i);
          failing_ram = i;
          result = 0;
          goto do_exit;
        }
      }
    }
  }

  if(show_progress) {
    diag_printf(" Done\n");
  }

#if defined(CYGBLD_BUILD_POST_HW_DEP) && CYGBLD_BUILD_POST_HW_DEP
  // Allow for H/W dependent end of subtest (like turning off a LED).
  vcoreii_diag_hw_dep_subtest_end(VCOREII_DIAG_SUBTEST_MEMBIST);
#endif

do_exit:
  if(was_dcache_on) {
    HAL_DCACHE_ENABLE();
  }
  if(was_icache_on) {
    HAL_ICACHE_ENABLE();
  }

  // Reformat analyzer memories.
  ANA_REG(0xb0) = 5;
  ANA_REG(0xd0) = 3;
  CYGACC_CALL_IF_DELAY_US((cyg_int32)50);
 
  if(result == 0) {
    err_info->failing_test = VCOREII_DIAG_SUBTEST_MEMBIST;
    err_info->info1 = (void *)failing_ram;
  }

  return result;
}

/******************************************************************************/
// ddrtest_walking_one()
// Helper function for ddrtest(). Writes walking-one values to a given memory
// address. Temporarily disables dcache if on.
/******************************************************************************/
static int ddrtest_walking_one(const char *test_name, cyg_uint32 *ptr, vcoreii_diag_err_info_t *err_info)
{
  cyg_uint32 val, expect, i;
  int        dcache_on = 0;

#ifdef HAL_DCACHE_IS_ENABLED
  HAL_DCACHE_IS_ENABLED(dcache_on);
#endif

  if(dcache_on) {             /* Disable while testing */
    HAL_DCACHE_SYNC();
    HAL_DCACHE_DISABLE();
    HAL_DCACHE_SYNC();
    HAL_DCACHE_INVALIDATE_ALL();
  }

  for(i = 0; i < 32; i++) {
    ptr[0] = (1 << i);      /* The cell data */
    ptr[1] = ~0L;           /* Drive other Data pins */
    if((val = *ptr) != (expect = (1 << i))) {
      diag_printf("\n%s: Walking-one RAM failure at %p, expected 0x%08x but got 0x%08x\n", test_name, ptr, expect, val);
      err_info->failing_test = VCOREII_DIAG_SUBTEST_DDRTEST;
      err_info->info1        = ptr;
      err_info->info2        = (void *)expect;
      err_info->info3        = (void *)val;
      return 0;
    }
  }

  if(dcache_on) {
    HAL_DCACHE_ENABLE();    /* Re-enable */
  }

  return 1; // Success
}

/******************************************************************************/
// ddrtest()
// Walking ones test - temporarily disables cache (if on)
/******************************************************************************/
static int ddrtest(bool show_progress, vcoreii_diag_err_info_t *err_info)
{
  #define BOUNDARY_1MBYTE(ptr) ((((cyg_uint32)ptr) & (0x100000-1)) == 0)
  const char *test_name = "DDR SDRAM";
  volatile cyg_uint32 *ptr;
  cyg_uint32 val, expect;
  int dcache_on = 0;

#if defined(CYGBLD_BUILD_POST_HW_DEP) && CYGBLD_BUILD_POST_HW_DEP
  // Allow for H/W dependent initialization (like turning on a LED).
  vcoreii_diag_hw_dep_subtest_begin(VCOREII_DIAG_SUBTEST_DDRTEST);
#endif

  if(show_progress) {
    diag_printf("%s: Testing [%p-%p]...", test_name, mem_segments[0].start, mem_segments[0].end);
  }

  // Do an initial walking-one test.
  if(!ddrtest_walking_one(test_name, (cyg_uint32 *)mem_segments[0].start, err_info)) {
    return 0;
  }

  // Write sweep
  for(ptr = (cyg_uint32 *)mem_segments[0].start; ptr < (cyg_uint32 *)mem_segments[0].end; ptr++) {
    if(BOUNDARY_1MBYTE(ptr)) {
      // Do a walking-one test on every 1MByte boundary.
      if(!ddrtest_walking_one(test_name, (cyg_uint32 *)ptr, err_info)) {
        return 0;
      }
#if defined(CYGBLD_BUILD_POST_HW_DEP) && CYGBLD_BUILD_POST_HW_DEP
      // Allow for flashing of a LED.
      vcoreii_diag_hw_dep_subtest_progress(VCOREII_DIAG_SUBTEST_DDRTEST);
#endif
    }
    *ptr = ~(cyg_uint32)ptr;
  }

#ifdef HAL_DCACHE_IS_ENABLED
  HAL_DCACHE_IS_ENABLED(dcache_on);
#endif

  if(dcache_on) {
    // Flush dcache to RAM
    HAL_DCACHE_SYNC();
    // And mark all cache lines as empty.
    HAL_DCACHE_INVALIDATE_ALL();
  }

  /* Read sweep */
  for(ptr = (cyg_uint32 *)mem_segments[0].start; ptr < (cyg_uint32 *)mem_segments[0].end; ptr++) {
#if defined(CYGBLD_BUILD_POST_HW_DEP) && CYGBLD_BUILD_POST_HW_DEP
      if(BOUNDARY_1MBYTE(ptr)) {
        // Allow for flashing of a LED.
        vcoreii_diag_hw_dep_subtest_progress(VCOREII_DIAG_SUBTEST_DDRTEST);
      }
#endif
    if((val = *ptr) != (expect = ~(cyg_uint32)ptr)) {
      diag_printf("\nError: %s: RAM failure at %p, expected 0x%08x but got 0x%08x\n", test_name, ptr, expect, val);
      err_info->failing_test = VCOREII_DIAG_SUBTEST_DDRTEST;
      err_info->info1        = (void *)ptr;
      err_info->info2        = (void *)expect;
      err_info->info3        = (void *)val;
      return 0;
    }
  }

  if(show_progress) {
    diag_printf(" Done\n");
  }

#if defined(CYGBLD_BUILD_POST_HW_DEP) && CYGBLD_BUILD_POST_HW_DEP
  // Allow for H/W dependent end of subtest (like turning off a LED).
  vcoreii_diag_hw_dep_subtest_end(VCOREII_DIAG_SUBTEST_DDRTEST);
#endif

  return 1; // Success

  #undef BOUNDARY_1MBYTE
}

/******************************************************************************/
// loopback_tx_frame()
// This function transmits one test frame of 128 bytes length on port port.
/******************************************************************************/
static void loopback_tx_frame(int port)
{
  cyg_uint32 i;
  int        len = 128;

  MAC_REG(port, 0xC0) = len << 16;
  MAC_REG(port, 0xC0) = 0x00000000;
  // Generate this frame:
  // f0f0f0f0f0f0f0f0 a5a5a5a5a5a5a5a5 5a5a5a5a5a5a5a5a 0f0f0f0f0f0f0f0f
  // ...
  // DMAC=f0f0f0f0f0f0 SMAC=f0f0a5a5a5a5 TYPE=a5a5

  for (i = 2; i < len / 4 + 2; i++) {
    MAC_REG(port, 0xc0) = ((i / 2) % 4 * 0x55555555) ^ 0xf0f0f0f0;
  }
  // Allow for some time to stabilize queues
  CYGACC_CALL_IF_DELAY_US((cyg_int32)1000);
  MAC_REG(port, 0xc4) = 1;
}

/******************************************************************************/
// loopback_hijack_frame()
// Hijack frame(s) from port to CPU queue #0, this function is pretty specific.
// It expects a single frame and it will do strange things to the queue
// system before capturing the frames.
/******************************************************************************/
static int loopback_hijack_frame(int port) {
  cyg_uint32 dummy;

  // Reset CPU Rx Queue #0.
  dummy = CPUQ_REG(0, 0x1);
  
  // Configure destination queue
  ANA_REG(0xa1) = 0;
  
  // Hijack frame and check queue
  ANA_REG(0x80 + port) = 0x1 << 30;

  CYGACC_CALL_IF_DELAY_US((cyg_int32)1000);
  
  ANA_REG(0x80 + port) = 0x0;

  // Note that we only check if a frame is present in the queue. We don't spool it
  // out or anything. But since we reset the queues before using them this should
  // be alright.
  // Return 1 if frame present, 0 if not.
  return (SYS_REG(0x31) & (1 << 28)) ? 1 : 0;
}

/******************************************************************************/
// loopback_init()
// Configuration function for setting up the ports.
// Configure the port for 1Gbps, Loopback as close to the edge as possible.
/******************************************************************************/
static void loopback_init(int port)
{
  cyg_uint32 reg;

  // 1 Gbps, 5 IFG
  MAC_REG(port, 0x00) = 0x10060141;

  // PCS cfg
  MAC_REG(port, 0x18) = 0x3004001;

  // Configure watermarks
  MAC_REG(port, 0xdf) = 2 << 16;
  MAC_REG(port, 0xe0) = 0x12121212;
  MAC_REG(port, 0xe8) = (31 << 13) | (2 << 11) | (1 << 10) | (1 << 8) | 21;
  MAC_REG(port, 0xe9) = ( 1 << 13) | (1 << 11) | (1 << 10) | (2 << 8);
  MAC_REG(port, 0xea) = ( 3 << 13) | (1 << 11) | (1 << 10) | (2 << 8);
  MAC_REG(port, 0xeb) = ( 7 << 13) | (1 << 11) | (1 << 10) | (2 << 8);
  MAC_REG(port, 0xec) = 31;
  MAC_REG(port, 0xed) = 31;

  // Configure PHYs depending on device mode
  if(port < 24) {
    // SGMII Port
    // Configure the SGMII macro. The sub-modes that does not need the macro
    // can put this into reset again.
    // Configure macro for AC-coupled mode for the AC coupled validation board
    // we need to change to the CMU_TEST_CTRL reg
    MAC_REG(port, 0x1b) = 0x201;

    // Loopback-mode
    reg  = 0x04160054; // Default without cdr-disable, and with c-mode termination
    reg |= (1 << 8) | (1 << 28) | (1 << 26);   // rx-enable + tx-enable + output level = 1
    MAC_REG(port, 0x1a) = reg;                 // Include rx-reset + tx-reset
    CYGACC_CALL_IF_DELAY_US((cyg_int32)10);

    MAC_REG(port, 0x1a) = reg | 1 | (1 << 25); // Same config, just without the resets
    CYGACC_CALL_IF_DELAY_US((cyg_int32)1000);

    // Loop fairly close to the edge of the chip. Configure macro for internal loopback
    reg = MAC_REG(port, 0x1a);
    reg |= (1 << 9) | (1 << 27);               // Rx-loopback + Tx-loopback
    MAC_REG(port, 0x1a) = reg | 1 | (1 << 25); // Include Rx-reset + Tx-reset
    CYGACC_CALL_IF_DELAY_US((cyg_int32)10);
    MAC_REG(port, 0x1a) = reg;                 // Same config, just without the resets
    CYGACC_CALL_IF_DELAY_US((cyg_int32)1000);
  } else {
    // VAUI Port
    int lane_id = port - 24; // VAUI ports are 24 - 27

    // Write cfg register, de-emphasis = 3, drive level = 7
    VAUI_LANE_REG(lane_id, 0x7) = 0x37;

    // Set equipment loop
    VAUI_LANE_REG(lane_id, 0x5) = 0x4;

    reg = 0x000C0807;
    VAUI_LANE_REG(lane_id, 0x4) = reg | (1 << 31); // Write config with reset
    CYGACC_CALL_IF_DELAY_US((cyg_int32)10);
    VAUI_LANE_REG(lane_id, 0x4) = reg;  // Write config
    CYGACC_CALL_IF_DELAY_US((cyg_int32)1000);
  }

  // Clear all device counters
  MAC_REG(port, 0x52) = 0x0;
}

/******************************************************************************/
// loopback()
// Consists of two tests:
//   Transmission of a frame onto a port, and checking it loops back forever.
//   Transmission of a frame from any port to any port and checking that it loops
//   forever.
/******************************************************************************/
static int loopback(bool show_progress, vcoreii_diag_err_info_t *err_info)
{
  const char *test_name = "Loopback";
  cyg_uint32 mask, dummy;
  int        p, lane_id, result = 1;

#if defined(CYGBLD_BUILD_POST_HW_DEP) && CYGBLD_BUILD_POST_HW_DEP
  // Allow for H/W dependent initialization (like turning on a LED).
  vcoreii_diag_hw_dep_subtest_begin(VCOREII_DIAG_SUBTEST_LOOPBACK_PORT);
#endif

  if(show_progress) {
    diag_printf("%s: Initializing ports...", test_name);
  }

  for(p = PORT_MIN; p <= PORT_MAX; p++) {
    loopback_init(p);
  }

  if(show_progress) {
    diag_printf(" Done\n");
  }

  // The chip needs time to lock (stabilize) after this, in particular because the
  // loopback modes typically just report link (cannot really check anything)
  CYGACC_CALL_IF_DELAY_US((cyg_int32)100000);
  for(p = PORT_MIN; p <= PORT_MAX; p++) {
    // Enable MAC recieve
    MAC_REG(p, 0x0) = MAC_REG(p, 0x0) | (0x1 << 16);
  }

  mask = 0;
  for(p = PORT_MIN; p <= PORT_MAX; p++) {
    mask |= 1 << p;
  }

  ANA_REG(0x10) = mask; // Allow all ports in recvmask
  ANA_REG(0x0d) = 0x0;  // No learning

  for(p = PORT_MIN; p <= PORT_MAX; p++) {
    MAC_REG(p, 0x24) = 0x06; // Re-calculate CRC on all frames
  }

  if(show_progress) {
    diag_printf("%s: Looping a frame on each port...", test_name);
  }

  // Loop a frame on each port
  for(p = PORT_MIN; p <= PORT_MAX; p++) {
    ANA_REG(0x80 + p) = 1 << p;
    loopback_tx_frame(p);
    CYGACC_CALL_IF_DELAY_US((cyg_int32)5); // Sleep about 5us for a 1G link
    if(!loopback_hijack_frame(p)) {
      diag_printf("\nError: %s: Frame did not get through to CPU queue #0 for port #%d\n", test_name, p);
      err_info->failing_test = VCOREII_DIAG_SUBTEST_LOOPBACK_PORT;
      err_info->info1        = (void *)p;
      result = 0;
      goto do_exit;
    }
  }

  if(show_progress) {
    diag_printf(" Done\n");
  }

#if defined(CYGBLD_BUILD_POST_HW_DEP) && CYGBLD_BUILD_POST_HW_DEP
  // Allow for H/W dependent end of subtest (like turning off a LED).
  vcoreii_diag_hw_dep_subtest_end(VCOREII_DIAG_SUBTEST_LOOPBACK_PORT);
#endif

#if defined(CYGBLD_BUILD_POST_HW_DEP) && CYGBLD_BUILD_POST_HW_DEP
  // Allow for H/W dependent initialization (like turning on a LED).
  vcoreii_diag_hw_dep_subtest_begin(VCOREII_DIAG_SUBTEST_LOOPBACK_RIPPLE);
#endif

  if(show_progress) {
    diag_printf("%s: Looping frames from port-to-port...", test_name);
  }

  // Loop between ports
  for(p = PORT_MIN; p < PORT_MAX; p++) {
    ANA_REG(0x80 + p) = 1 << (p + 1);
  }
  ANA_REG(0x80 + PORT_MAX) = 1;
  loopback_tx_frame(0);
  CYGACC_CALL_IF_DELAY_US((cyg_int32)20000);
  // Hijack the frame from any port.
  if(!loopback_hijack_frame(PORT_MAX/2)) {
    diag_printf("\nError: %s: Frame was lost in port-to-port loop\n", test_name);
    err_info->failing_test = VCOREII_DIAG_SUBTEST_LOOPBACK_RIPPLE;
    err_info->info1        = (void *)p;
    result = 0;
    goto do_exit;
  }

  if(show_progress) {
    diag_printf(" Done\n");
  }

#if defined(CYGBLD_BUILD_POST_HW_DEP) && CYGBLD_BUILD_POST_HW_DEP
  // Allow for H/W dependent end of subtest (like turning off a LED).
  vcoreii_diag_hw_dep_subtest_end(VCOREII_DIAG_SUBTEST_LOOPBACK_RIPPLE);
#endif

do_exit:
  // Disable all ports.
  mask = (1 << 28) - 1;
  for(p = PORT_MIN; p <= PORT_MAX; p++) {
    MAC_REG(p, 0x1a) = 0x00562000; // Default value
    MAC_REG(p, 0x00) = 0x2004003c; // Default value
    ANA_REG(p+ 0x80) = mask & ~(1 << p);
  }
  for(lane_id = 0; lane_id < 4; lane_id++) {
    VAUI_LANE_REG(lane_id, 0x7) = 0x0;
    VAUI_LANE_REG(lane_id, 0x5) = 0;
    VAUI_LANE_REG(lane_id, 0x4) = 0x000C0801;
  }

  // Reset CPU Rx Queue #0.
  dummy = CPUQ_REG(0, 0x1);

  return result;
}

/******************************************************************************/
// vcoreii_diag_hw_indep_tests()
// Performs a range of tests that only depend on the chip - not the surrounding
// hardware.
/******************************************************************************/
void vcoreii_diag_hw_indep_tests(bool show_progress, vcoreii_diag_subtests_t tests_to_run, vcoreii_diag_err_info_t *err_info)
{
  if(tests_to_run & VCOREII_DIAG_SUBTEST_MEMBIST) {
    if(!membist(show_progress, err_info)) {
      return;
    }
  }
  if(tests_to_run & VCOREII_DIAG_SUBTEST_DDRTEST) {
    if(!ddrtest(show_progress, err_info)) {
      return;
    }
  }
  if((tests_to_run & VCOREII_DIAG_SUBTEST_LOOPBACK_PORT) || (tests_to_run & VCOREII_DIAG_SUBTEST_LOOPBACK_RIPPLE)) {
    if(!loopback(show_progress, err_info)) {
      return;
    }
  }
}

