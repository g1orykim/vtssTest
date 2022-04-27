//==========================================================================
//
//      vcoreii_diag.c
//
//      Entry point for POST for VCOREII
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
// Purpose:      Entry Point for POST
// Description:
//
//####DESCRIPTIONEND####

#include "vcoreii_diag.h"

extern void vcoreii_diag_hw_indep_tests(bool show_progress, vcoreii_diag_subtests_t tests_to_run, vcoreii_diag_err_info_t *err_info);

#if defined(CYGBLD_BUILD_POST_HW_DEP) && CYGBLD_BUILD_POST_HW_DEP
  extern void vcoreii_diag_hw_dep_begin(bool show_progress);
  extern void vcoreii_diag_hw_dep_tests(bool show_progress, vcoreii_diag_subtests_t tests_to_run, vcoreii_diag_err_info_t *err_info);
  extern void vcoreii_diag_hw_dep_end  (bool show_progress, vcoreii_diag_err_info_t *err_info);
#endif

/******************************************************************************/
// do_diag()
// This is the entry point to the H/W dep & indep tests.
// It takes one optional argument, -p, which - when specified - enables
// display of progress on the console.
/******************************************************************************/
static void do_diag(int arg, char *argv[])
{
  vcoreii_diag_err_info_t err_info;
  vcoreii_diag_subtests_t tests_to_run = VCOREII_DIAG_SUBTEST_NONE;
  bool                    quiet, run_all, run_membist, run_ramtest, run_loopback_test;
#if defined(CYGBLD_BUILD_POST_HW_DEP) && CYGBLD_BUILD_POST_HW_DEP
  bool                    run_hw_dependent_tests;
  struct option_info      opts[6];
#else
  struct option_info      opts[5];
#endif

  // Args:   option flag  takes_arg type                     arg                         arg_set  name
  init_opts(&opts[0], 'q', false, OPTION_ARG_TYPE_FLG, (void **)&quiet,                  NULL, "Quiet operation");
  init_opts(&opts[1], 'a', false, OPTION_ARG_TYPE_FLG, (void **)&run_all,                NULL, "Run all tests");
  init_opts(&opts[2], 'm', false, OPTION_ARG_TYPE_FLG, (void **)&run_membist,            NULL, "Run memory BIST");
  init_opts(&opts[3], 'd', false, OPTION_ARG_TYPE_FLG, (void **)&run_ramtest,            NULL, "Run DDR SDRAM test");
  init_opts(&opts[4], 'l', false, OPTION_ARG_TYPE_FLG, (void **)&run_loopback_test,      NULL, "Run loopback test");
#if defined(CYGBLD_BUILD_POST_HW_DEP) && CYGBLD_BUILD_POST_HW_DEP
  init_opts(&opts[5], 'h', false, OPTION_ARG_TYPE_FLG, (void **)&run_hw_dependent_tests, NULL, "Run hardware dependent tests");
#endif

  // Args:     cnt vals idx_of_1st opts num_opts               def_arg def_arg_type def_descr
  if(!scan_opts(argc, argv, 1, opts, sizeof(opts)/sizeof(opts[0]), 0, 0, "")) {
    return;
  }

  // scan_opts() automatically resets all values to 0 (false).
  if(run_all) {
    tests_to_run = (vcoreii_diag_subtests_t) -1; // Set all bits.
  } else {
    if(run_membist) {
      tests_to_run |= VCOREII_DIAG_SUBTEST_MEMBIST;
    }
    if(run_ramtest) {
      tests_to_run |= VCOREII_DIAG_SUBTEST_DDRTEST;
    }
    if(run_loopback_test) {
      tests_to_run |= VCOREII_DIAG_SUBTEST_LOOPBACK_PORT | VCOREII_DIAG_SUBTEST_LOOPBACK_RIPPLE;
    }
#if defined(CYGBLD_BUILD_POST_HW_DEP) && CYGBLD_BUILD_POST_HW_DEP
    if(run_hw_dependent_tests) {
      tests_to_run |= VCOREII_DIAG_SUBTEST_HW_DEPENDENT_TESTS;
    }
#endif
  }

// RBNTBD: As long as we haven't found a fix for transmitting frames on the cable
// when doing the loopback test, always disable it.
  tests_to_run &= ~VCOREII_DIAG_SUBTEST_LOOPBACK_PORT;
  tests_to_run &= ~VCOREII_DIAG_SUBTEST_LOOPBACK_RIPPLE;

  if(tests_to_run == VCOREII_DIAG_SUBTEST_NONE) {
    diag_printf("Error: At least one of the test options must be specified\n");
    return;
  }

  // So far no error.
  err_info.failing_test = VCOREII_DIAG_SUBTEST_NONE;

#if defined(CYGBLD_BUILD_POST_HW_DEP) && CYGBLD_BUILD_POST_HW_DEP
  // Allow a H/W specific pre-step, which e.g. could flash the LEDs
  // to signal that we've just booted.
  vcoreii_diag_hw_dep_begin(!quiet);
#endif

  // Do H/W independent tests and place result in err_info.
  // The tests themselves may print info to the console, but must
  // fill in the err_info structure in order for a possible H/W
  // dependent vcoreii_diag_hw_dep_end() function to take proper action.
  vcoreii_diag_hw_indep_tests(!quiet, tests_to_run, &err_info);

#if defined(CYGBLD_BUILD_POST_HW_DEP) && CYGBLD_BUILD_POST_HW_DEP
  // If enabled and H/W independent tests succeeded, also do H/W dependent tests,
  // and place result in err_info.
  // Again, the tests themselves may print info to the console, but must
  // fill in the err_info structure in order for the H/W dependent vcoreii_diag_hw_dep_end()
  // function to take proper action.
  if(err_info.failing_test == VCOREII_DIAG_SUBTEST_NONE) {
    vcoreii_diag_hw_dep_tests(!quiet, tests_to_run, &err_info);
  }

  // Call the H/W specific post-step, which e.g. could flash the LEDs
  // in a special way to signal the results of the H/W independent and H/W
  // dependent tests.
  vcoreii_diag_hw_dep_end(!quiet, &err_info);
#else
  // If an error occurred, halt. The error is already shown on the console.
  if(err_info.failing_test != VCOREII_DIAG_SUBTEST_NONE) {
    diag_printf("\n***HALTING***\n");
    while(1);
    /* ENOTREACHED */
  }
#endif
}

/******************************************************************************/
// Create a new RedBoot command called 'diag'.
/******************************************************************************/

// The C-compiler doesn't support #ifdefs within macro calls, so unfortunately
// we have to do almost the same twice.
#if defined(CYGBLD_BUILD_POST_HW_DEP) && CYGBLD_BUILD_POST_HW_DEP
RedBoot_cmd("diag",
            "Run Power-On-Self-Test\n"
            "  -q: Quiet operation\n"
            "  -a: Run all tests\n"
            "  -m: Run memory BIST\n"
            "  -d: Run DDR SDRAM test\n"
// RBNTBD: As long as we haven't found a fix for transmitting frames on the cable
// when doing the loopback test, always disable it.
//            "  -l: Run loopback test\n"
            "  -h: Run hardware dependent tests",
            "[-q] [-a] [-m] [-d] [-l] [-h]",
            do_diag);
#else
RedBoot_cmd("diag",
            "Run Power-On-Self-Test\n"
            "  -q: Quiet operation\n"
            "  -a: Run all tests\n"
            "  -m: Run memory BIST\n"
            "  -d: Run DDR SDRAM test\n"
// RBNTBD: As long as we haven't found a fix for transmitting frames on the cable
// when doing the loopback test, always disable it.
//            "  -l: Run loopback test"
            ,
            "[-q] [-a] [-m] [-d] [-l]",
            do_diag);
#endif
