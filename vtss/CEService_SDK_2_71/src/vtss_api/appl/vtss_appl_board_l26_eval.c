/*

 Vitesse API software.

 Copyright (c) 2002-2013 Vitesse Semiconductor Corporation "Vitesse". All
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

#if defined(BOARD_LUTON26_EVAL)

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
    reg_write(0, 2, (1<<12) | (2<<8) | (1<<3));
    reg_write(0, 3, 0);
    VTSS_MSLEEP(500);
    reg_write(0, 3, (1<<7) | (1<<1) | (1<<0));
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
    return VTSS_PORT_INTERFACE_SGMII;
}

/* ================================================================= *
 *  Board init.
 * ================================================================= */
// lu26_pre_reset(), called before system reset.
// Disables the SGPIO controller and waits the required amount of time for it to settle.
// The potential problem is that: When resetting the SIO controller (and GPIOs)
// when the chain is only partially shifted out is that; if the shift registers on
// the PCB latches the data during reset (i.e. GPIO latch / clk is tristating and
// then being pulled to an active value) then all sorts of bad things can happen
//(for example a customers could use serial GPIO controlling board reset).
static void lu26_pre_reset(void)
{
    vtss_gpio_no_t gpio;
    
    /* Disable SGPIO controller (GPIO 0-3 for SGPIO) */
    for (gpio = 0; gpio < 4; gpio++) {
        (void) vtss_gpio_mode_set(NULL, 0, gpio, VTSS_GPIO_IN);
    }
    VTSS_MSLEEP(10); /* Waits a period time */

    // Internal PHY
    (void) vtss_phy_pre_system_reset(NULL, 0);
    
    // External PHY (Atom12)
    (void) vtss_phy_pre_system_reset(NULL, 12);

    
    // Put in coma mode during reboot. If not in coma mode during reboot, the link can come up before tat system is initialized, and that can generate CRC errors if frames are transmitted as soon as there is linkup.
    (void) vtss_phy_coma_mode_enable(NULL, 0); // Shall use the same port as used in the vtss_phy_post_reset function 
}

/* Post ports reset */
static vtss_rc lu26_post_reset(void)
{
    return vtss_phy_post_reset(NULL, 0);
}

static int board_init(int argc, const char **argv, vtss_board_t *board)
{
    vtss_port_no_t  port_no;
    vtss_port_map_t *map;
    
    board_io_init();
    board->init.init_conf->reg_read = reg_read;
    board->init.init_conf->reg_write = reg_write;
    board->init.init_conf->mux_mode = VTSS_PORT_MUX_MODE_1;
    board->port_map = port_map;
    board->port_interface = port_interface;

    /* Setup port map and calculate port count */
    board->port_count = VTSS_PORTS;
    for (port_no = VTSS_PORT_NO_START; port_no < VTSS_PORT_NO_END; port_no++) {
        map = &port_map[port_no];
        if (port_no < 12) {
            /* Port 0-11: Internal PHYs */
            map->chip_port = port_no;
            map->miim_controller = VTSS_MIIM_CONTROLLER_0;
            map->miim_addr = port_no;
        } else if (port_no < 20) {
            /* Port 12-19: Octal module assumed */
            map->miim_controller = VTSS_MIIM_CONTROLLER_1;
            if (port_no == 12) {
                map->chip_port = 14;
                map->miim_addr = 9;
            } else if (port_no == 13) {
                map->chip_port = 15;
                map->miim_addr = 8;
            } else if (port_no == 14) {
                map->chip_port = 17;
                map->miim_addr = 11;
            } else if (port_no == 15) {
                map->chip_port = 18;
                map->miim_addr = 10;
            } else if (port_no == 16) {
                map->chip_port = 21;
                map->miim_addr = 13;
            } else if (port_no == 17) {
                map->chip_port = 20;
                map->miim_addr = 12;
            } else if (port_no == 18) {
                map->chip_port = 23;
                map->miim_addr = 15;
            } else { /* Port 19 */
                map->chip_port = 22;
                map->miim_addr = 14;
            } 
        } else {
            /* Port 20-25: Unused */
            map->chip_port = CHIP_PORT_UNUSED;
            map->miim_controller = VTSS_MIIM_CONTROLLER_NONE;
            map->miim_addr = -1;
            if (port_no < board->port_count)
                board->port_count = port_no;
        }
    }

    return 0;
}

void vtss_board_l26_eval_init(vtss_board_t *board)
{
    board->pre_reset  = lu26_pre_reset;
    board->post_reset = lu26_post_reset;
    board->descr = "Caracal-2";
    board->target = VTSS_TARGET_CARACAL_2;
    board->feature.port_control = 1;
    board->feature.layer2 = 1;
    board->feature.packet = 1;
    board->board_init = board_init;
}
#endif /* BOARD_LUTON26_EVAL */
