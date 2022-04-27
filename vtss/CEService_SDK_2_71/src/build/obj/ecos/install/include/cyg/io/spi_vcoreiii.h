#ifndef CYGONCE_DEVS_SPI_MIPS_VCOREIII_H
#define CYGONCE_DEVS_SPI_MIPS_VCOREIII_H
//=============================================================================
//
//      spi_vcoreiii.h
//
//      Header definitions for VCore-III SPI driver.
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
// Author(s):   Lars Povlsen
// Date:        2009-09-15
// Purpose:     VCore-III SPI driver definitions.
// Description: 
// Usage:       #include <cyg/io/spi_vcoreiii.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <pkgconf/io_spi.h>
#include <pkgconf/devs_spi_mips_vcoreiii.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/spi.h>

#define VTSS_SPI_CS_NONE   0xFF
#define VTSS_SPI_GPIO_NONE 0xFF

// _cs_ should be used to select the mux.
// _arg_ is the GPIO mask for generating the chip-select, it's up to the application code
// to make sure that the pin's alternate functions are correctly set-up
// and that the pin is output enabled and value is initialized to 1 (chip deselected)
// prior to calling the generic cyg_spi_xxx() functions.
#define CYG_DEVS_SPI_MIPS_VCOREIII_DEVICE_EXT(_name_, _cs_, _mask_, _value_)    \
    cyg_spi_mips_vcoreiii_device_t _name_ = {                                   \
        .spi_device  = { .spi_bus = (cyg_spi_bus*) &cyg_spi_mips_vcoreiii_bus },\
        .cs_num      = _cs_,                                                    \
        .mask        = _mask_,                                                  \
        .value       = _value_,                                                 \
    }
// At most one of _cs_ and _gpio_ should be different from one of the
// VTSS_SPI_xxx_NONE macros above.
// If using a GPIO for chip-select, it's up to the application code
// to make sure that the pin' salternate functions are correctly set-up
// and that the pin is output enabled and value is initialized to 1 (chip deselected)
/*lint --e{648} */
#define CYG_DEVS_SPI_MIPS_VCOREIII_DEVICE(_name_, _cs_, _gpio_)                 \
        CYG_DEVS_SPI_MIPS_VCOREIII_DEVICE_EXT(_name_, _cs_, (_gpio_) == VTSS_SPI_GPIO_NONE  ? 0 : VTSS_BIT(_gpio_), 0)  \

typedef struct {
    // ---- Upper layer data ----
    cyg_spi_bus   spi_bus;

    // ---- Lower layer data ----
} cyg_spi_mips_vcoreiii_bus_t;

typedef struct cyg_spi_mips_vcoreiii_device_s {
    // ---- Upper layer data ----
    cyg_spi_device spi_device; // Upper layer SPI device data.

    // ---- Device setup (user configurable) ----
    cyg_uint8  cs_num;          // Chip select number.
    cyg_uint32 mask;            // GPIO mask.
    cyg_uint32 value;           // GPIO mask.
    cyg_uint8  delay;           // The delay in usecs to be done before and after change of clock signal against SPI device
    cyg_bool   init_clk_high;   // Set to TRUE if the SCK should be high prior to each access and kept high at the end (a.k.a. CPOL == 1).

    // ---- Device state (private) ----
} cyg_spi_mips_vcoreiii_device_t;

externC cyg_spi_mips_vcoreiii_bus_t cyg_spi_mips_vcoreiii_bus;

//=============================================================================
#endif // CYGONCE_DEVS_SPI_MIPS_VCOREIII_H
