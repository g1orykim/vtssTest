#ifndef _VCOREII_DIAG_H_
#define _VCOREII_DIAG_H_

//==========================================================================
//
//      vcoreii_diag.h
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

#include <redboot.h>

/******************************************************************************/
// Subtests used to identify any running test as well as the test in which
// a possibly error occurred.
// Note that the enumeration is a bitmask.
/******************************************************************************/
typedef enum {
  VCOREII_DIAG_SUBTEST_NONE               = 0x00, /**< No error.                                                                                          */
  VCOREII_DIAG_SUBTEST_MEMBIST            = 0x01, /**< BIST error. info1 contains the number of the last failing RAM.                                     */
  VCOREII_DIAG_SUBTEST_DDRTEST            = 0x02, /**< DDR SDRAM error. info1 contains failing address, info2 the expected value, info3 the actual value. */
  VCOREII_DIAG_SUBTEST_LOOPBACK_PORT      = 0x04, /**< Port loopback failed. info1 contains failing port.                                                 */
  VCOREII_DIAG_SUBTEST_LOOPBACK_RIPPLE    = 0x08, /**< Ripple loopback failed.                                                                            */
#if defined(CYGBLD_BUILD_POST_HW_DEP) && CYGBLD_BUILD_POST_HW_DEP
  VCOREII_DIAG_SUBTEST_HW_DEPENDENT_TESTS = 0x10, /**< Other tests.                                                                                       */
  /* Add more as needed for the H/W specific part. */
#endif
} vcoreii_diag_subtests_t;

/******************************************************************************/
// The structure passed to H/W dep & indep tests and whose results are used
// by the H/W dep signalling function (vcoreii_diag_hw_dep_end()).
/******************************************************************************/
typedef struct {
  vcoreii_diag_subtests_t failing_test; /**< Error that occurred */
  void                    *info1;       /**< Generic variable that holds additional information about the error. */
  void                    *info2;       /**< Generic variable that holds additional information about the error. */
  void                    *info3;       /**< Generic variable that holds additioanl information about the error. */
} vcoreii_diag_err_info_t;

#endif /* _VCOREII_DIAG_H_ */
