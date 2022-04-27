#ifndef _VTSS_H_VCOREII_FDMA_
#define _VTSS_H_VCOREII_FDMA_

//=============================================================================
//
//      vcoreii_fdma.h
//
//      Platform specific Frame DMA support (register layout, etc)
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
// Author(s):    Rene Schipp von Branitz Nielsen
// Contributors:
// Date:         2006-03-22
// Purpose:      VCore-II Frame DMA platform specific register layout
// Description: 
// Usage:
//
//####DESCRIPTIONEND####
//
//=============================================================================

/* Channel & queue allocation */
#define FDMA_CH_TX_SWITCHED     0
#define FDMA_CH_RX_IP           1
#define FDMA_CH_RX_BPDU         2
#define FDMA_CH_RX_STACK        3
#define FDMA_CH_RX_SPROUT       4
#define FDMA_CH_TX_PORTSPEC     5
#define FDMA_CH_TX_STACK_L      6
#define FDMA_CH_TX_STACK_R      7

#if 1
  // For now we need to use only two queues,
  // because CPJ hasn't yet found time to
  // configure the remaining two.
  #define FDMA_Q_EXTR_IP        1
  #define FDMA_Q_EXTR_BPDU      0
  #define FDMA_Q_EXTR_STACK     0
  #define FDMA_Q_EXTR_SPROUT    0
#else
  #define FDMA_Q_EXTR_IP        0
  #define FDMA_Q_EXTR_BPDU      1
  #define FDMA_Q_EXTR_STACK     2
  #define FDMA_Q_EXTR_SPROUT    3
#endif

#define VTSS_STACK_L_PM_NUMBER  24
#define VTSS_STACK_R_PM_NUMBER  26

#endif  /* _VTSS_H_VCOREII_FDMA_ */
