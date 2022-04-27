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

#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/drv_api.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>

#include <cyg/io/spi.h>
#include <cyg/io/spi_vcoreiii.h>
#include <pkgconf/devs_spi_mips_vcoreiii.h>

#include <string.h>

#define DELAY() \
    asm volatile("nop; nop; nop; nop; nop; nop; nop; nop;"); \
    asm volatile("nop; nop; nop; nop; nop; nop; nop; nop;"); \
    asm volatile("nop; nop; nop; nop; nop; nop; nop; nop;");

static void spi_transaction_begin(cyg_spi_device *device)
{
    cyg_spi_mips_vcoreiii_device_t *vcoreiii_device = (cyg_spi_mips_vcoreiii_device_t *)device;
    cyg_uint32 value;

    // Start driving SI_nEN[dev] = '1' to make sure any previous H/W-assisted access is terminated correctly.
    value = VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_PIN_CTRL_MODE; /* SW Bitbang */

    if (vcoreiii_device->cs_num != VTSS_SPI_CS_NONE) {
        value |= VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_SPI_CS_OE(VTSS_BIT(vcoreiii_device->cs_num)); /* CS_OE = 1, but don't chip-select */
    }

    // For GPIO chip-selects, upper layers must already have output-enabled the GPIO pin with code similar to the following:
    // VTSS_DEVCPU_GCB_GPIO_GPIO_OE |= vcoreiii_device->mask;

    VTSS_ICPU_CFG_SPI_MST_SW_MODE = value;

    hal_delay_us(1);

    // Now make sure the clock is driven to the correct polarity before chip-selecting.
    value |= VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_SPI_SCK_OE; /* SCK_OE */
    if (vcoreiii_device->init_clk_high) {
       value |= VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_SPI_SCK;
    }
    VTSS_ICPU_CFG_SPI_MST_SW_MODE = value;

    // Short delay
    DELAY();

    /* Now enable CS */
    if (vcoreiii_device->cs_num != VTSS_SPI_CS_NONE) {
        VTSS_ICPU_CFG_SPI_MST_SW_MODE = value | VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_SPI_CS(VTSS_BIT(vcoreiii_device->cs_num)); /* CS_OE */
    }
    if (vcoreiii_device->mask != VTSS_SPI_GPIO_NONE) {
        VTSS_DEVCPU_GCB_GPIO_GPIO_OUT = (VTSS_DEVCPU_GCB_GPIO_GPIO_OUT & ~vcoreiii_device->mask) | (vcoreiii_device->mask & vcoreiii_device->value);
    }
}

static void spi_transaction_end(cyg_spi_device *device)
{
    cyg_spi_mips_vcoreiii_device_t *vcoreiii_device = (cyg_spi_mips_vcoreiii_device_t *)device;
    cyg_uint32 clk_value = VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_SPI_SCK;
    cyg_uint32 cs_value, value;

    // Might be that the transaction already ended in spi_transaction_transfer().
    // Check to see if we're still in bit-banging mode.
    if ((VTSS_ICPU_CFG_SPI_MST_SW_MODE & VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_PIN_CTRL_MODE) == 0) {
        // No longer under S/W control.
        return;
    }

    if (vcoreiii_device->cs_num != VTSS_SPI_CS_NONE) {
        cs_value = VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_SPI_CS_OE(VTSS_BIT(vcoreiii_device->cs_num)) | /* CS_OE */
                   VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_SPI_CS(VTSS_BIT(vcoreiii_device->cs_num));     /* CS */
    } else {
        cs_value = 0;
    }

    // We ended up (in spi_transaction_transfer()) with CLK driven high.
    if (!vcoreiii_device->init_clk_high) {
        // Drive it low as dictated by higher layer.
        clk_value = 0;
        VTSS_ICPU_CFG_SPI_MST_SW_MODE =
            cs_value                                         | /* Possible CS_OE and CS */
            VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_SPI_SCK_OE    | /* SCK_OE */
            VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_SPI_SDO_OE    | /* SDO OE */
            VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_PIN_CTRL_MODE;  /* SW Bitbang */
        DELAY();
    }

    // Keep driving the CLK to its current value while actively deselecting CS.
    value =
        clk_value                                         | /* SCK = 0 or 1 */
        VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_SPI_SCK_OE     | /* SCK_OE = 1 */
        VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_PIN_CTRL_MODE;   /* SW Bitbang */

    if (vcoreiii_device->cs_num != VTSS_SPI_CS_NONE) {
        cs_value = VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_SPI_CS_OE(VTSS_BIT(vcoreiii_device->cs_num)); /* CS_OE = 1, CS = 0 => nCS = 1 */
        value |= cs_value;
    }
    if (vcoreiii_device->mask != VTSS_SPI_GPIO_NONE) {
        VTSS_DEVCPU_GCB_GPIO_GPIO_OUT = (VTSS_DEVCPU_GCB_GPIO_GPIO_OUT & ~vcoreiii_device->mask) | (vcoreiii_device->mask & ~vcoreiii_device->value);
    }

    VTSS_ICPU_CFG_SPI_MST_SW_MODE = value;
    DELAY();

    // Stop driving the clock, but keep on driving CS with nCS == 1
    VTSS_ICPU_CFG_SPI_MST_SW_MODE =
        cs_value |                                        /* CS_OE = 1, nCS = 1 */
        VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_PIN_CTRL_MODE; /* SW Bitbang */

    hal_delay_us(1);

    // Drop everything
    VTSS_ICPU_CFG_SPI_MST_SW_MODE = 0;
}

static void spi_transaction_transfer(cyg_spi_device  *device,
                                     cyg_bool         polled,
                                     cyg_uint32       count,
                                     const cyg_uint8 *tx_data,
                                     cyg_uint8       *rx_data,
                                     cyg_bool         drop_cs)
{
    cyg_spi_mips_vcoreiii_device_t *vcoreiii_device = (cyg_spi_mips_vcoreiii_device_t *)device;
    cyg_uint32 i, svalue;

    // Check to see if we're currently in bit-banging mode. If not, gracefully start.
    if ((VTSS_ICPU_CFG_SPI_MST_SW_MODE & VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_PIN_CTRL_MODE) == 0) {
        // Someone forgot to call spi_transaction_begin(). Do it for them.
        spi_transaction_begin(device);
    }

    svalue =
        VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_SPI_SCK_OE    | /* SCK_OE */
        VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_SPI_SDO_OE    | /* SDO OE */
        VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_PIN_CTRL_MODE;  /* SW Bitbang */

    if (vcoreiii_device->cs_num != VTSS_SPI_CS_NONE) {
        svalue |= VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_SPI_CS_OE(VTSS_BIT(vcoreiii_device->cs_num)) | /* CS_OE */
                  VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_SPI_CS(VTSS_BIT(vcoreiii_device->cs_num));     /* CS */
    }

    for (i = 0; i < count; i++) {
        cyg_uint32 rx = 0, mask = 0x80, value;
        while (mask) {
            // Initial condition: CLK is low.
            value = svalue;
            if (tx_data && tx_data[i] & mask) {
                value |= VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_SPI_SDO;
            }

            // Drive data while taking CLK low. The device we're accessing will
            // sample on the following rising edge and will output data on this edge for us
            // to be sampled at the end of this loop.
            VTSS_ICPU_CFG_SPI_MST_SW_MODE = value & ~VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_SPI_SCK;

            // Wait for t_setup. All devices do have a setup-time, so we always insert some
            // delay here. Some devices have a very long setup-time, which can be adjusted by
            // the user through vcoreiii_device->delay.
            DELAY();
            hal_delay_us(vcoreiii_device->delay); // Further setup-time. Will return quickly if called with 0.

            // Drive the clock high.
            VTSS_ICPU_CFG_SPI_MST_SW_MODE = value | VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_SPI_SCK;

            // Wait for t_hold. See comment about t_setup above.
            DELAY();
            hal_delay_us(vcoreiii_device->delay); // Further hold-time. Will return quickly if called with 0.

            // We sample as close to the next falling edge as possible
            value = VTSS_ICPU_CFG_SPI_MST_SW_MODE;
            if (value & VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_SPI_SDI) {
                rx |= mask;
            }
            mask >>= 1;
        }
        if (rx_data) {
            rx_data[i] = (cyg_uint8)rx;
        }
    }

    // Drop CS?
    if (drop_cs) {
       spi_transaction_end(device);
    }
}

static void spi_transaction_tick(cyg_spi_device *device,
                                 cyg_bool        polled,
                                 cyg_uint32      count)
{
}

static int spi_get_config(cyg_spi_device *device,
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

cyg_spi_mips_vcoreiii_bus_t cyg_spi_mips_vcoreiii_bus = {
    .spi_bus.spi_transaction_begin    = spi_transaction_begin,
    .spi_bus.spi_transaction_transfer = spi_transaction_transfer,
    .spi_bus.spi_transaction_tick     = spi_transaction_tick,
    .spi_bus.spi_transaction_end      = spi_transaction_end,
    .spi_bus.spi_get_config           = spi_get_config,
    .spi_bus.spi_set_config           = spi_set_config,
};
