/*

 Vitesse Switch API software.

 Copyright (c) 2002-2011 Vitesse Semiconductor Corporation "Vitesse". All
 Rights Reserved.

 Unpublished rights reserved under the copyright laws of the United States of
 America, other countries and international treaties. Permission to use, copy,
 store and modify, the software and its source code is granted. Permission to
 integrate into other products, disclose, transmit and distribute the software
 in an absolute machine readable format (e.g. HEX file) is also granted.  The
 source code of the software may not be disclosed, transmitted or distributed
 without the written permission of Vitesse. The software and its source code
 may only be used in products utilizing the Vitesse switch products.

 This copyright notice must appear in any copy, modification, disclosure,
 transmission or distribution of the software. Vitesse retains all ownership,
 copyright, trade secret and proprietary rights in the software.

 THIS SOFTWARE HAS BEEN PROVIDED "AS IS," WITHOUT EXPRESS OR IMPLIED WARRANTY
 INCLUDING, WITHOUT LIMITATION, IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR USE AND NON-INFRINGEMENT.

*/
/*lint -esym(766, vtss_silabs_clk_api.h) */
//#if defined(VTSS_CHIP_JAGUAR_1) || defined(VTSS_CHIP_JAGUAR_1_DUAL)
#include "vtss_silabs_clk_api.h"
//#endif /* VTSS_CHIP_JAGUAR_1 || VTSS_CHIP_JAGUAR_1_DUAL */
#if defined(VTSS_FEATURE_PHY_TIMESTAMP)
#if defined(VTSS_PHY_TS_SILABS_CLK_DLL)
#include "main.h"
#include "vtss_types.h"
#include <cyg/io/spi_vcoreiii.h>

CYG_DEVS_SPI_MIPS_VCOREIII_DEVICE(spi_dev, 3, VTSS_SPI_GPIO_NONE);

#define SPI_SET_ADDRESS      0x00
#define SPI_WRITE            0x40
#define SPI_READ             0x80

static cyg_uint8 tx_data[2], rx_data[2];

static void spi_set_address(u8 address)
{
    tx_data[0] = SPI_SET_ADDRESS;
    tx_data[1] = address;
    cyg_spi_transfer((cyg_spi_device*)&spi_dev, 0, 2, tx_data, rx_data);
}


static void spi_write(u8 address, u8 data)
{
    spi_set_address(address);

    tx_data[0] = SPI_WRITE;
    tx_data[1] = data;
    cyg_spi_transfer((cyg_spi_device*)&spi_dev, 0, 2, tx_data, rx_data);
}

vtss_rc si5326_dll_init(void)
{
    u32 reg;

    reg = VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(0);
    VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(0) = reg | VTSS_BIT(18);
    spi_dev.delay = 1;

    spi_write(0, 0x14);
    spi_write(1, 0xE4);
    spi_write(2, 0xA2);
    spi_write(3, 0x15);
    spi_write(4, 0x92);
    spi_write(5, 0xED);
    spi_write(6, 0x3F);
    spi_write(7, 0x2A);
    spi_write(8, 0x00);
    spi_write(9, 0xC0);
    spi_write(10, 0x00);
    spi_write(11, 0x42);
    spi_write(16, 0x00);
    spi_write(17, 0x80);
    spi_write(18, 0x00);
    spi_write(19, 0x2C);
    spi_write(20, 0x3E);
    spi_write(21, 0xFF);
    spi_write(22, 0xDF);
    spi_write(23, 0x1F);
    spi_write(24, 0x3F);
    spi_write(25, 0x20);
    spi_write(31, 0x00);
    spi_write(32, 0x00);
    spi_write(33, 0x03);
    spi_write(34, 0x00);
    spi_write(35, 0x00);
    spi_write(36, 0x03);
    spi_write(40, 0x00);
    spi_write(41, 0x02);
    spi_write(42, 0x77);
    spi_write(43, 0x00);
    spi_write(44, 0x00);
    spi_write(45, 0x4E);
    spi_write(46, 0x00);
    spi_write(47, 0x00);
    spi_write(48, 0x4E);
    spi_write(55, 0x00);
    spi_write(131, 0x1F);
    spi_write(132, 0x02);
    spi_write(138, 0x0F);
    spi_write(139, 0xFF);
    spi_write(142, 0x00);
    spi_write(143, 0x00);
    spi_write(136, 0x40);

    return VTSS_RC_OK;
}
#endif /* VTSS_PHY_TS_SILABS_CLK_DLL */
#endif /* VTSS_FEATURE_PHY_TIMESTAMP */
