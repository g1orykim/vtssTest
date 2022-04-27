/*

 Vitesse Switch SyncE software.

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

#ifdef VTSS_ARCH_JAGUAR_1
#include "synce_trace.h"
#include "pcb107_cpld.h"
#include <cyg/io/spi.h>
#include <cyg/io/spi_vcoreiii.h>

/****************************************************************************/
/*  Global variables                                                                                                                      */
/****************************************************************************/

/* CPLD register defines */
#define PCB107_CPLD_ID      0x01      // Cpld ID
#define PCB107_CPLD_REV     0x02      // Cpld Revision
#define PCB107_CPLD_MUX0    0x06      // MUX 0
#define PCB107_CPLD_MUX1    0x07      // MUX 1
#define PCB107_CPLD_MUX2    0x08      // MUX 2
#define PCB107_CPLD_MUX3    0x09      // MUX 3

#define PCB107_CPLD_MUX_MAX 0x04      // Number of muxes
#define PCB107_CPLD_MUX_INPUT_MAX 20  // number of mux inputs

#define SYNCE_SPI_CPLD_GPIO_CS 18
CYG_DEVS_SPI_MIPS_VCOREIII_DEVICE_EXT(dev_pcb_107_cpld, 3, 0x2000C0, 0x80); /* MUX selector pins: 21 = 0,7 = 1,6 = 0 */



static void pcb107_cpld_write(uchar address, uchar data)
{
    cyg_uint8 tx_data[2], rx_data[2];

    tx_data[0] = (address & 0x7F) | 0x80;
    tx_data[1] = data;

    cyg_spi_transfer((cyg_spi_device*)&dev_pcb_107_cpld, 0, 2, tx_data, rx_data);
}

static void pcb107_cpld_read(uchar address, uchar *data)
{
    cyg_uint8 tx_data[2], rx_data[2];

    tx_data[0] = (address & 0x7F);
    tx_data[1] = 0;

    cyg_spi_transfer((cyg_spi_device*)&dev_pcb_107_cpld, 0, 2, tx_data, rx_data);

    *data = rx_data[1];
}

static uchar cpld_id;
static uchar cpld_rev;

void pcb107_cpld_init(void)
{
    if (vtss_gpio_mode_set(NULL, 0, SYNCE_SPI_CPLD_GPIO_CS, VTSS_GPIO_ALT_0) != VTSS_RC_OK)    T_D("error returned");
    if (vtss_gpio_direction_set(NULL, 0, 6, TRUE) != VTSS_RC_OK)    T_D("error returned");
    if (vtss_gpio_direction_set(NULL, 0, 7, TRUE) != VTSS_RC_OK)    T_D("error returned");
    if (vtss_gpio_direction_set(NULL, 0, 21, TRUE) != VTSS_RC_OK)    T_D("error returned");
    dev_pcb_107_cpld.delay = 1;
    pcb107_cpld_read(PCB107_CPLD_ID, &cpld_id);
    pcb107_cpld_read(PCB107_CPLD_REV, &cpld_rev);
    T_IG(TRACE_GRP_API, "PCB107 CPLD id %d, revision %d", cpld_id, cpld_rev);
    
    
}

static u32 curr_mux [PCB107_CPLD_MUX_MAX] = {0xff,0xff,0xff,0xff};

void pcb107_cpld_mux_set(u32 mux, u32 input)
{
    uchar mux_reg;
    if (mux >= PCB107_CPLD_MUX_MAX || input >= PCB107_CPLD_MUX_INPUT_MAX) {
        T_EG(TRACE_GRP_API, "Invalid mux %d or input %d", mux, input);
    } else {
        if (input != curr_mux[mux]) {
            mux_reg = (cpld_rev > 1) ? PCB107_CPLD_MUX0 + mux : PCB107_CPLD_MUX3 - mux;
            T_WG(TRACE_GRP_API, "Setting mux %d to input %d", mux, input);
            pcb107_cpld_write(mux_reg, input);
            curr_mux[mux] = input;
        }
    }
}
#endif


/****************************************************************************/
/*                                                                                                                                                    */
/*  End of file.                                                                                                                              */
/*                                                                                                                                                   */
/****************************************************************************/
