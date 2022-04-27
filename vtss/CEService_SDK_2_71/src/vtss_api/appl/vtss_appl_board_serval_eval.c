/*

 Vitesse API software.

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
 
 $Id$
 $Revision$

*/

#include "vtss_api.h"
#include "vtss_appl.h"

#if defined(BOARD_SERVAL_EVAL)

/* ================================================================= *
 *  Register access
 * ================================================================= */

#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/vitgenio.h>

static int fd;

static vtss_rc board_rd_wr(const u32 addr, u32 *const value, int write)
{
    struct vitgenio_cpld_spi_readwrite spi_buf;
    char                               *buf = &spi_buf.buffer[0];
    u32                                val;

    /* Address is 22 bits: (tgt_id << 14) + reg_addr */
    val = (write ? *value : 0xffffffff);
    buf[0] = ((write << 7) | ((addr >> 16) & 0x3f));
    buf[1] = ((addr >> 8) & 0xff);
    buf[2] = ((addr >> 0) & 0xff);
    buf[3] = ((val >> 24) & 0xff);
    buf[4] = ((val >> 16) & 0xff);
    buf[5] = ((val >> 8) & 0xff);
    buf[6] = ((val >> 0) & 0xff);
    spi_buf.length = 7;
    if (ioctl(fd, VITGENIO_CPLD_SPI_READWRITE, &spi_buf) < 0) {
        T_E("SPI_READWRITE failed");
        return VTSS_RC_ERROR;
    }
    if (!write) {
        *value = ((buf[3] << 24) | (buf[4] << 16) | (buf[5] << 8) | (buf[6] << 0));
    }
    T_N("%s: addr 0x%08x (0x%02x:0x%04x), value: 0x%08x", 
        write ? "WR" : "RD", addr, (addr >> 14) & 0xff, addr & 0x3fff, *value);

    return VTSS_RC_OK;
}

static vtss_rc reg_read(const vtss_chip_no_t chip_no,
                        const u32            addr,
                        u32                  *const value)
{
    return board_rd_wr(addr, value, 0);
}

static vtss_rc reg_write(const vtss_chip_no_t chip_no,
                         const u32            addr,
                         const u32            value)
{
    u32 val = value;
    return board_rd_wr(addr, &val, 1);
}

static void board_io_init(void)
{
    struct vitgenio_cpld_spi_setup setup = {
        /* char ss_select; Which of the CPLD_GPIOs is used for Slave Select */
        VITGENIO_SPI_SS_CPLD_GPIO0,
        VITGENIO_SPI_SS_ACTIVE_LOW, /* char ss_activelow; Slave Select (Chip Select) active low: true, active high: false */
        VITGENIO_SPI_CPOL_0, /* char sck_activelow; CPOL=0: false, CPOL=1: true */
        VITGENIO_SPI_CPHA_0, /* char sck_phase_early; CPHA=0: false, CPHA=1: true */
        VITGENIO_SPI_MSBIT_FIRST, /* char bitorder_msbfirst; */
        0, /* char reserved1; currently unused, only here for alignment purposes */
        0, /* char reserved2; currently unused, only here for alignment purposes */
        0, /* char reserved3; currently unused, only here for alignment purposes */
        200 /* unsigned int ndelay; minimum delay in nanoseconds, two of these delays are used per clock cycle */
    };
    u32 value;

    if ((fd = open("/dev/vitgenio", 0)) < 0) {
        T_E("open /dev/vitgenio failed");
        return;
    }
    
    /* SI */
    ioctl(fd, VITGENIO_ENABLE_CPLD_SPI);
    ioctl(fd, VITGENIO_CPLD_SPI_SETUP, &setup);

    /* CPLD1 */
    setup.ss_select = VITGENIO_SPI_SS_CPLD_GPIO2;
    ioctl(fd, VITGENIO_ENABLE_CPLD_SPI);
    ioctl(fd, VITGENIO_CPLD_SPI_SETUP, &setup);

    /* CPLD0 */
    setup.ss_select = VITGENIO_SPI_SS_CPLD_GPIO1;
    ioctl(fd, VITGENIO_ENABLE_CPLD_SPI);
    ioctl(fd, VITGENIO_CPLD_SPI_SETUP, &setup);

    /* Read CPLD version */
    reg_read(0, 1, &value);
    T_D("CPLD register 1: 0x%08x", value);

    /* Board configuration and reset using CPLD register 2 and 3 */
    reg_write(0, 2, (15<<8) | (5<<4) | (1<<3));
    reg_write(0, 3, 0);
    VTSS_MSLEEP(500);
    reg_write(0, 3, (1<<1) | (1<<0));
    VTSS_MSLEEP(500);
    
    /* SI */
    setup.ss_select = VITGENIO_SPI_SS_CPLD_GPIO0;
    ioctl(fd, VITGENIO_ENABLE_CPLD_SPI);
    ioctl(fd, VITGENIO_CPLD_SPI_SETUP, &setup);
}

/* Board port map */
static vtss_port_map_t port_map[VTSS_PORT_ARRAY_SIZE];

static vtss_port_interface_t port_interface(vtss_port_no_t port_no)
{
    return (port_no < 8 ? VTSS_PORT_INTERFACE_SGMII : VTSS_PORT_INTERFACE_SERDES);
}

/* ================================================================= *
 *  Board init.
 * ================================================================= */

static int board_init(int argc, const char **argv, vtss_board_t *board)
{
    vtss_port_no_t  port_no;
    vtss_port_map_t *map;
    
    board_io_init();
    board->init.init_conf->reg_read = reg_read;
    board->init.init_conf->reg_write = reg_write;
    board->port_map = port_map;
    board->port_interface = port_interface;

    /* Setup port map and calculate port count */
    board->port_count = VTSS_PORTS;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        map = &port_map[port_no];

        if (port_no < 8) {
            /* Octal PHY module assumed */
            map->chip_port = (8 - port_no - 1);
            map->miim_controller = VTSS_MIIM_CONTROLLER_0;
            map->miim_addr = (8 + port_no);
        } else {
            map->chip_port = port_no;
            map->miim_controller = VTSS_MIIM_CONTROLLER_NONE;
        }
    }

    return 0;
}

static void board_init_post(vtss_board_t *board) 
{
    /* Enable GPIO_11 and GPIO_12 as MIIM controllers */
    vtss_gpio_mode_set(NULL, 0, 11, VTSS_GPIO_ALT_1);
    vtss_gpio_mode_set(NULL, 0, 12, VTSS_GPIO_ALT_1);
}

void vtss_board_serval_eval_init(vtss_board_t *board)
{
    board->descr = "Serval";
    board->target = VTSS_TARGET_SERVAL;
    board->feature.port_control = 1;
    board->feature.layer2 = 1;
    board->feature.packet = 1;
    board->board_init = board_init;
    board->board_init_post = board_init_post;
}
#endif /* BOARD_SERVAL_EVAL */
