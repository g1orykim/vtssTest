//==========================================================================
//
//      threadload.h
//
//      Interface for the threadload code.
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 2002 Free Software Foundation, Inc.                        
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
// Contributors: Rene Schipp von Branitz Nielsen
// Date:         2011-12-16
// Purpose:      
// Description:  
//      API for the threadload measurement code
//        
//####DESCRIPTIONEND####
//
//==========================================================================

#ifndef _SERVICES_THREADLOAD_THREADLOAD_H
#define _SERVICES_THREADLOAD_THREADLOAD_H

#include <pkgconf/threadload.h>

#ifdef __cplusplus
externC 
{
#endif  

/* Start keeping an eye on the thread load */
void cyg_threadload_start(void);

/* Stop keeping an eye on the thread load */
void cyg_threadload_stop(void);

// Returns CPU load in percentage * 100 per thread over the last one and
// 10 seconds.
// The arrays are indexed by thread id and must be allocated by caller.
// The function always clears the arrays, but returns false if the
// measuring is not started with cyg_threadload_start(), true otherwise.
// Either of the arrays arguments may be NULL, indicating that the caller
// will not need these measurements.
// Index == 0 is for DSR load.
// Index > 0 are for thread load (indexed by thread ID).
cyg_bool cyg_threadload_get(cyg_uint16 load_1sec[CYGNUM_THREADLOAD_MAX_ID], cyg_uint16 load_10sec[CYGNUM_THREADLOAD_MAX_ID]);

#ifdef __cplusplus
}
#endif

#endif /* _SERVICES_THREADLOAD_THREADLOAD_H */
