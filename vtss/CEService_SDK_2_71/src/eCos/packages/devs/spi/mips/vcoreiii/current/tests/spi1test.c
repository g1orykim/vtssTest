//=============================================================================
//
//      spi1test.c
//
//      SPI simple test (for timing simulation).
//
//=============================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 2008, 2009 Free Software Foundation, Inc.
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
// Author(s):       Lars Povlsen
// Contributors(s): Chris Holgate
// Date:            2009-09-15
// Purpose:         VCore-III SPI test
// Description:     Standalone SPI tx test.
// Usage:           Compile as a standalone application.
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/testcase.h>         // Test macros
#include <cyg/infra/cyg_ass.h>          // Assertion macros
#include <cyg/infra/diag.h>             // Diagnostic output

#include <cyg/hal/hal_arch.h>           // CYGNUM_HAL_STACK_SIZE_TYPICAL
#include <cyg/kernel/kapi.h>

#include <cyg/io/spi.h>                 // Common SPI API
#include <cyg/io/spi_vcoreiii.h>        // VCore-III data structures

#include <string.h>

//---------------------------------------------------------------------------
// Thread data structures.

cyg_uint8 stack [CYGNUM_HAL_STACK_SIZE_TYPICAL];
cyg_thread thread_data;
cyg_handle_t thread_handle;

//---------------------------------------------------------------------------
// SPI loopback device driver data structures.

CYG_DEVS_SPI_MIPS_VCOREIII_DEVICE(spi1_dev, 1, VTSS_SPI_GPIO_NONE);

//---------------------------------------------------------------------------

const char tx_data[] = "Testing, testing, 12, 123.";
char rx_data [sizeof(tx_data)];

//---------------------------------------------------------------------------
// Run single transmit transaction using simple transfer API call.

void run_test_1 (cyg_bool polled)
{
    diag_printf ("Test 1 : Simple transfer test (polled = %d).\n", polled ? 1 : 0);
    cyg_spi_transfer (&spi1_dev.spi_device, polled, sizeof (tx_data), 
                      (const cyg_uint8*) &tx_data[0], (cyg_uint8*) &rx_data[0]);
    diag_printf ("Test 1 : Done\n");
}

//---------------------------------------------------------------------------
// Run all SPI interface tests.

void run_tests (void)
{
    run_test_1 (true); 

    VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_GPR(0) = 0xCAFEBABE;

    CYG_TEST_PASS_FINISH ("SPI tests ran OK");
}

//---------------------------------------------------------------------------
// User startup - tests are run in their own thread.

void cyg_user_start(void)
{
    CYG_TEST_INIT();
    cyg_thread_create(
        10,                                   // Arbitrary priority
        (cyg_thread_entry_t*) run_tests,      // Thread entry point
        0,                                    // 
        "test_thread",                        // Thread name
        &stack[0],                            // Stack
        CYGNUM_HAL_STACK_SIZE_TYPICAL,        // Stack size
        &thread_handle,                       // Thread handle
        &thread_data                          // Thread data structure
    );
    cyg_thread_resume(thread_handle);
    cyg_scheduler_start();
}

//=============================================================================
