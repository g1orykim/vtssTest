//=============================================================================
//
//      m25pxx_test.c
//
//      SPI flash driver tests for Numonyx M25Pxx devices and compatibles.
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
// Author(s):   Chris Holgate
// Date:        2008-12-22
// Purpose:     Numonyx M25Pxx SPI flash driver stress tests.
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/system.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/testcase.h>           // Test macros
#include <cyg/infra/cyg_ass.h>            // Assertion macros
#include <cyg/infra/diag.h>               // Diagnostic output

#include <cyg/hal/hal_arch.h>             // CYGNUM_HAL_STACK_SIZE_TYPICAL
#include <cyg/hal/hal_if.h>
#include <cyg/kernel/kapi.h>

#include <cyg/io/spi.h>
#include <cyg/io/flash.h>
#include <cyg/io/m25pxx.h>
#include <cyg/io/flash_dev.h>

#include <string.h>

#include <cyg/io/spi.h>                 // Common SPI API
#include <cyg/io/spi_vcoreiii.h>        // VCore-III SPI data structures
#include <cyg/io/m25pxx.h>              // M25P interface

extern struct cyg_flash_dev vcoreiii_m25pxx;

//-----------------------------------------------------------------------------
// Thread data structures.

static cyg_uint32 stack [CYGNUM_HAL_STACK_SIZE_TYPICAL / 4];
static cyg_thread thread_data;
static cyg_handle_t thread_handle;

//-----------------------------------------------------------------------------
// Data transfer buffers.

#define BUF_SIZE 1024

static cyg_uint8 wbuf [BUF_SIZE];
static cyg_uint8 rbuf [BUF_SIZE];

//-----------------------------------------------------------------------------
// Run checkerboard and inverse checkerboard writes over each sector in turn.

cyg_uint32 run_test_1
  (void)
{
    int status;
    cyg_uint32 i, j;
    cyg_uint32 errors = 0;
    cyg_flashaddr_t base_addr, err_addr;

    diag_printf ("Test 1 - write and verify checkerboard and inverse checkerboard.\n");
    
    // Iterate over all flash sectors.
    for (i = 0; i < vcoreiii_m25pxx.block_info->blocks; i++) {
        base_addr =  vcoreiii_m25pxx.start + (i * vcoreiii_m25pxx.block_info->block_size);

        // Erase block.
        status = cyg_flash_erase (base_addr, 1, &err_addr);
        if (status != FLASH_ERR_OK) {
            diag_printf ("Flash erase error : %s\n", cyg_flash_errmsg (status));
            errors ++;
        }

        // Set up buffer with checkerboard.
        for (j = 0; j < BUF_SIZE;) {
           wbuf [j++] = 0x55;
           wbuf [j++] = 0xAA;
        }

        // Write the checkerboard to the entire sector.
        for (j = 0; j < vcoreiii_m25pxx.block_info->block_size; j += BUF_SIZE) {
            status = cyg_flash_program (base_addr + j, wbuf, BUF_SIZE, &err_addr);
            if (status != FLASH_ERR_OK) {
                diag_printf ("Flash write error : %s\n", cyg_flash_errmsg (status));
                errors ++;
            }
        }

        // Read back the checkerboard and verify.
        for (j = 0; j < vcoreiii_m25pxx.block_info->block_size; j += BUF_SIZE) {
            status = cyg_flash_read (base_addr + j, rbuf, BUF_SIZE, &err_addr);
            if (status != FLASH_ERR_OK) {
                diag_printf ("Flash read error : %s\n", cyg_flash_errmsg (status));
                errors ++;
            }
            else if (memcmp (rbuf, wbuf, BUF_SIZE) != 0) {
                diag_printf ("Flash read data corruption (0x%08X - 0x%08X).\n", base_addr + j, base_addr + j + BUF_SIZE - 1);
                errors ++;
            }
        }

        // Erase block.
        status = cyg_flash_erase (base_addr, 1, &err_addr);
        if (status != FLASH_ERR_OK) {
            diag_printf ("Flash erase error : %s\n", cyg_flash_errmsg (status));
            errors ++;
        }

        // Set up buffer with inverse checkerboard.
        for (j = 0; j < BUF_SIZE;) {
           wbuf [j++] = 0xAA;
           wbuf [j++] = 0x55;
        }

        // Write the checkerboard to the entire sector.
        for (j = 0; j < vcoreiii_m25pxx.block_info->block_size; j += BUF_SIZE) {
            status = cyg_flash_program (base_addr + j, wbuf, BUF_SIZE, &err_addr);
            if (status != FLASH_ERR_OK) {
                diag_printf ("Flash write error : %s\n", cyg_flash_errmsg (status));
                errors ++;
            }
        }

        // Read back the checkerboard and verify.
        for (j = 0; j < vcoreiii_m25pxx.block_info->block_size; j += BUF_SIZE) {
            status = cyg_flash_read (base_addr + j, rbuf, BUF_SIZE, &err_addr);
            if (status != FLASH_ERR_OK) {
                diag_printf ("Flash read error : %s\n", cyg_flash_errmsg (status));
                errors ++;
            }
            else if (memcmp (rbuf, wbuf, BUF_SIZE) != 0) {
                diag_printf ("Flash read data corruption (0x%08X - 0x%08X).\n", base_addr + j, base_addr + j + BUF_SIZE - 1);
                errors ++;
            }
        }
    }
    return errors;
}

//-----------------------------------------------------------------------------
// Write and verify counting sequence over all device.

cyg_uint32 run_test_2
  (void)
{
    int status;
    cyg_uint32 i, j;
    cyg_uint32 errors = 0;
    cyg_flashaddr_t base_addr, err_addr;

    diag_printf ("Test 2 - write and verify counting sequence.\n");
    
    // Erase all flash sectors.
    for (i = 0; i < vcoreiii_m25pxx.block_info->blocks; i++) {
        base_addr =  vcoreiii_m25pxx.start + (i * vcoreiii_m25pxx.block_info->block_size);

        // Erase block.
        status = cyg_flash_erase (base_addr, 1, &err_addr);
        if (status != FLASH_ERR_OK) {
            diag_printf ("Flash erase error : %s\n", cyg_flash_errmsg (status));
            errors ++;
        }
    }

    // Write counting sequence.
    base_addr =  vcoreiii_m25pxx.start;
    for (i = 0; i < (vcoreiii_m25pxx.end - vcoreiii_m25pxx.start) / 4;) {
        for (j = 0; j < BUF_SIZE;) {
            wbuf [j++] = (cyg_uint8) ((i >> 0) & 0xFF);
            wbuf [j++] = (cyg_uint8) ((i >> 8) & 0xFF);
            wbuf [j++] = (cyg_uint8) ((i >> 16) & 0xFF);
            wbuf [j++] = (cyg_uint8) ((i >> 24) & 0xFF);            
            i++;
        }
        status = cyg_flash_program (base_addr, wbuf, BUF_SIZE, &err_addr);
        if (status != FLASH_ERR_OK) {
            diag_printf ("Flash write error : %s\n", cyg_flash_errmsg (status));
            errors ++;
        }
        base_addr += BUF_SIZE;
    }

    // Verify counting sequence.
    base_addr = vcoreiii_m25pxx.start;
    for (i = 0; i < (vcoreiii_m25pxx.end - vcoreiii_m25pxx.start) / 4;) {
        for (j = 0; j < BUF_SIZE;) {
            wbuf [j++] = (cyg_uint8) ((i >> 0) & 0xFF);
            wbuf [j++] = (cyg_uint8) ((i >> 8) & 0xFF);
            wbuf [j++] = (cyg_uint8) ((i >> 16) & 0xFF);
            wbuf [j++] = (cyg_uint8) ((i >> 24) & 0xFF);            
            i++;
        }
        status = cyg_flash_read (base_addr, rbuf, BUF_SIZE, &err_addr);
        if (status != FLASH_ERR_OK) {
            diag_printf ("Flash read error : %s\n", cyg_flash_errmsg (status));
            errors ++;
        }
        else if (memcmp (rbuf, wbuf, BUF_SIZE) != 0) {
            diag_printf ("Flash read data corruption (0x%08X - 0x%08X).\n", base_addr, base_addr + BUF_SIZE - 1);
            errors ++;
        }
        base_addr += BUF_SIZE;
    }
    return errors;
}

//-----------------------------------------------------------------------------
// Perform non-aligned buffer read/write tests, spanning sector boundaries.

cyg_uint32 run_test_3
  (void)
{
    int status;
    cyg_uint32 i, count;
    cyg_uint32 errors = 0;
    cyg_flashaddr_t base_addr, err_addr;

    diag_printf ("Test 3 - test non-aligned writes and reads.\n");
    
    // Fill the write buffer with a basic counting sequence.
    count = 0;
    for (i = 0; i < BUF_SIZE;) {
        wbuf [i++] = (cyg_uint8) ((count >> 0) & 0xFF);
        wbuf [i++] = (cyg_uint8) ((count >> 8) & 0xFF);
        count++;
    }

    // Assuming 256 byte pages gives 256 possible alignments.
    base_addr = vcoreiii_m25pxx.start + vcoreiii_m25pxx.block_info->block_size;
    for (i = 1; i <= 256; i++) {

        // Erase sectors either side of sector boundary.
        status = cyg_flash_erase (base_addr - 1, 1, &err_addr);
        if (status != FLASH_ERR_OK) {
            diag_printf ("Flash erase error : %s\n", cyg_flash_errmsg (status));
            errors ++;
        }
        status = cyg_flash_erase (base_addr, 1, &err_addr);
        if (status != FLASH_ERR_OK) {
            diag_printf ("Flash erase error : %s\n", cyg_flash_errmsg (status));
            errors ++;
        }

        // Write data spanning sector boundary.        
        status = cyg_flash_program (base_addr - i, wbuf, BUF_SIZE, &err_addr);
        if (status != FLASH_ERR_OK) {
            diag_printf ("Flash write error : %s\n", cyg_flash_errmsg (status));
            errors ++;
        }

        // Verify data spanning sector boundary.
        status = cyg_flash_read (base_addr - i, rbuf, BUF_SIZE, &err_addr);
        if (status != FLASH_ERR_OK) {
            diag_printf ("Flash read error : %s\n", cyg_flash_errmsg (status));
            errors ++;
        }
        else if (memcmp (rbuf, wbuf, BUF_SIZE) != 0) {
            diag_printf ("Flash read data corruption (0x%08X - 0x%08X).\n", base_addr - i, base_addr - i + BUF_SIZE - 1);
            errors ++;
        }

        // Use next sector boundary for next test.
        base_addr += vcoreiii_m25pxx.block_info->block_size;
        if (base_addr >= vcoreiii_m25pxx.end)
            base_addr = vcoreiii_m25pxx.start + vcoreiii_m25pxx.block_info->block_size;
    }
    return errors;
}

//-----------------------------------------------------------------------------
// Run all M25Pxx SPI interface loopback tests.

void run_tests 
    (void)
{
    cyg_uint32 errors = 0;

    // Check that the device is intialised.
    if (vcoreiii_m25pxx.start == vcoreiii_m25pxx.end) {
        diag_printf ("M25Pxx device not initialised.\n");
        errors ++;
        goto out;
    }

#if 1
    // Run the tests.
    errors += run_test_1 (); 
    errors += run_test_2 (); 
    errors += run_test_3 ();
#endif

out:
    VTSS_ICPU_CFG_CPU_SYSTEM_CTRL_GPR(0) = errors ? 0xDEADBEEF: 0xCAFEBABE;
    diag_printf ("----\nTests complete - detected %d errors in total.\n", errors);
    if (errors == 0)
        CYG_TEST_PASS_FINISH ("M25Pxx driver tests ran OK.");
    else 
        CYG_TEST_FAIL_FINISH ("M25Pxx driver test FAILED.");
}

//-----------------------------------------------------------------------------
// User startup - tests are run in their own thread.

void cyg_user_start
    (void)
{
    diag_printf ("----\nChecking for M25Pxx compatible devices.\n");
    cyg_flash_set_global_printf((cyg_flash_printf *)&diag_printf);
    if((cyg_flash_init(NULL) != CYG_FLASH_ERR_OK) ||
       (vcoreiii_m25pxx.block_info->blocks == 0))
        diag_printf("Flash init failed\n");
    CYG_TEST_INIT();
    cyg_thread_create(
        10,                                   // Arbitrary priority
        (cyg_thread_entry_t*) run_tests,      // Thread entry point
        0,                                    // 
        "test_thread",                        // Thread name
        (cyg_uint8*) stack,                   // Stack
        CYGNUM_HAL_STACK_SIZE_TYPICAL,        // Stack size
        &thread_handle,                       // Thread handle
        &thread_data                          // Thread data structure
    );
    cyg_thread_resume(thread_handle);
    cyg_scheduler_start();
}

//=============================================================================
