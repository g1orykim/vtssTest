// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2009 Free Software Foundation, Inc.
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

/* Code i this file is inheritet from eCos\packages\devs\spi\arm\at91\current\src\spi_at91.c */
/* This code is a simplified version - if you need to expand to a more generic and interrupt driven version, look into that file */

#include <pkgconf/hal.h>
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/vcoreii.h>
#include <cyg/hal/hal_platform_ints.h>
#include <cyg/io/spi.h>
#include <cyg/io/vcoreii_spi_drv.h>


static void spi_transaction_begin(cyg_spi_device *dev);

static void spi_transaction_transfer(cyg_spi_device  *dev,
                                     cyg_bool         polled,
                                     cyg_uint32       count,
                                     const cyg_uint8 *tx_data,
                                     cyg_uint8       *rx_data,
                                     cyg_bool         drop_cs);

static void spi_transaction_tick(cyg_spi_device *dev,
                                 cyg_bool        polled,
                                 cyg_uint32      count);

static void spi_transaction_end(cyg_spi_device* dev);

static int spi_get_config(cyg_spi_device *dev, 
                          cyg_uint32      key, 
                          void           *buf,
                          cyg_uint32     *len);

static int spi_set_config(cyg_spi_device *dev, 
                          cyg_uint32      key, 
                          const void     *buf, 
                          cyg_uint32     *len);


#define SPI_BUS_DI     0x10
#define SPI_BUS_MASTER 0x08
#define SPI_BUS_CLK    0x04
#define SPI_BUS_NEN    0x02
#define SPI_BUS_DO     0x01


typedef struct
{
    // ---- Upper layer data ----

    cyg_spi_bus   spi_bus;                  // Upper layer SPI bus data

    // ---- Lower layer data ----
     
} spi_bus_t;


spi_bus_t spi_bus0 =
{
    .spi_bus.spi_transaction_begin    = spi_transaction_begin,
    .spi_bus.spi_transaction_transfer = spi_transaction_transfer,
    .spi_bus.spi_transaction_tick     = spi_transaction_tick,
    .spi_bus.spi_transaction_end      = spi_transaction_end,
    .spi_bus.spi_get_config           = spi_get_config,
    .spi_bus.spi_set_config           = spi_set_config,
};



void cyg_vcoreii_spi_bus_init(void)
{
    /* Enable master SPI mode and reset device */
    HAL_WRITE_UINT32(VCOREII_SWC_REG(7, 0, 0x35), 0xF);
}



void cyg_vcoreii_spi_device_init(cyg_vcoreii_spi_device_t  *dev)
{
    cyg_uint32 value;

    dev->spi_device.spi_bus = (cyg_spi_bus *)&spi_bus0;

    /* Reset device by low/high of enable */
    HAL_READ_UINT32(VCOREII_SWC_REG(7, 0, 0x35), value);
    HAL_WRITE_UINT32(VCOREII_SWC_REG(7, 0, 0x35), value & ~SPI_BUS_NEN);
    HAL_WRITE_UINT32(VCOREII_SWC_REG(7, 0, 0x35), value | SPI_BUS_NEN);
}



static void spi_transaction_begin(cyg_spi_device *dev)
{
    cyg_uint32 value;

    /* enable device */
    HAL_READ_UINT32(VCOREII_SWC_REG(7, 0, 0x35), value);
    HAL_WRITE_UINT32(VCOREII_SWC_REG(7, 0, 0x35), value & ~SPI_BUS_NEN);
}



static void spi_transaction_transfer(cyg_spi_device  *dev, 
                                     cyg_bool         polled,  
                                     cyg_uint32       count, 
                                     const cyg_uint8 *tx_data, 
                                     cyg_uint8       *rx_data, 
                                     cyg_bool         drop_cs) 
{
    cyg_uint32  value, mask;
    cyg_uint16 i, j;
    
    /* get current register value */
    HAL_READ_UINT32(VCOREII_SWC_REG(7, 0, 0x35), value);

    for (i=0; i<count; ++i)
    {
        for (j=0, mask=0x80; j<8; ++j, mask>>=1)
        {
            /* set DO (data out) to the the polarity indicated in tx_data */
            value = (tx_data[i] & mask) ? value | SPI_BUS_DO : value & ~SPI_BUS_DO;
            HAL_WRITE_UINT32(VCOREII_SWC_REG(7, 0, 0x35), value);
            
            /* give clock pulse to SPI bus */
            HAL_WRITE_UINT32(VCOREII_SWC_REG(7, 0, 0x35), value & ~SPI_BUS_CLK);
            HAL_WRITE_UINT32(VCOREII_SWC_REG(7, 0, 0x35), value | SPI_BUS_CLK);
            
            HAL_READ_UINT32(VCOREII_SWC_REG(7, 0, 0x35), value);
            rx_data[i] = (value & SPI_BUS_DI) ? rx_data[i] | mask : rx_data[i] & ~mask;

        }
    }
}



static void spi_transaction_tick(cyg_spi_device *dev, 
                                 cyg_bool        polled,  
                                 cyg_uint32      count)
{
}



static void spi_transaction_end(cyg_spi_device* dev)
{
    cyg_uint32 value;

    /* disable device */
    HAL_READ_UINT32(VCOREII_SWC_REG(7, 0, 0x35), value);
    HAL_WRITE_UINT32(VCOREII_SWC_REG(7, 0, 0x35), value | SPI_BUS_NEN);
}



static int spi_get_config(cyg_spi_device *dev, 
                          cyg_uint32      key, 
                          void           *buf,
                          cyg_uint32     *len)
{
    return(0);
}



static int spi_set_config(cyg_spi_device *dev, 
                          cyg_uint32      key, 
                          const void     *buf, 
                          cyg_uint32     *len)
{
    return(0);
}
