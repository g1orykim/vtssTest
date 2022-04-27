/*

 Vitesse Clock API software.

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

#include "synce_custom_clock_api.h"
#include "main.h"
#include "vtss_os.h"
#include "critd_api.h"
#include "port_custom_api.h"
#include "misc_api.h"
#include <vtss_trace_lvl_api.h>
#include <cyg/io/spi.h>
#include <stdio.h>

#ifdef VTSS_SW_OPTION_ZL_3034X_API
#include "zl_3034x_synce_clock_api.h"
#endif

#define VTSS_TRACE_MODULE_ID VTSS_MODULE_ID_SYNCE

#define SPI_SET_ADDRESS      0x00
#define SPI_WRITE            0x40
#define SPI_WRITE_INCREMENT  0x60
#define SPI_READ             0x80
#define SPI_READ_INCREMENT   0xA0

#define SPI_FREE_RUN_REG        0
#define SPI_CK_PRIOR_REG        1
#define SPI_CKSEL_REG           3
#define SPI_DHOLD_REG           3
#define SPI_AUTOSEL_REG         4
#define SPI_N31_REG            43
#define SPI_N32_REG            46
#define SPI_CLK_ACTV_REG      128
#define SPI_LOS_INT_REG       129
#define SPI_LOSX_INT_REG      129
#define SPI_LOL_INT_REG       130
#define SPI_FOS_INT_REG       130
#define SPI_DIGHOLDVALID_REG  130

#define CLOCK_INTERRUPT_MASK_REG      23
#define CLOCK_INTERRUPT_PENDING_REG  131
#define CLOCK_INTERRUPT_STATUS_REG   129

#define CLOCK_LOS1_MASK              0x02
#define CLOCK_LOS2_MASK              0x04
#define CLOCK_FOS1_MASK              0x02
#define CLOCK_FOS2_MASK              0x04
#define CLOCK_LOSX_MASK              0x01
#define CLOCK_LOL_MASK               0x01

#if (VTSS_TRACE_ENABLED)
#define CRIT_ENTER() critd_enter(&crit, 1, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#define CRIT_EXIT()  critd_exit( &crit, 1, VTSS_TRACE_LVL_NOISE, __FILE__, __LINE__)
#else
#define CRIT_ENTER() critd_enter(&crit)
#define CRIT_EXIT()  critd_exit( &crit)
#endif /* VTSS_TRACE_ENABLED */



static cyg_handle_t     thread_handle_clock;
static cyg_thread       thread_block_clock;
static char             thread_stack_clock[THREAD_DEFAULT_STACK_SIZE];
static critd_t          crit;

#ifdef VTSS_ARCH_LUTON28
#include <cyg/io/vcoreii_spi_drv.h>
static cyg_vcoreii_spi_device_t dev;
#endif

#ifdef VTSS_ARCH_LUTON26
#include <cyg/io/spi_vcoreiii.h>
#define SYNCE_SPI_GPIO_CS 10
CYG_DEVS_SPI_MIPS_VCOREIII_DEVICE(dev, VTSS_SPI_CS_NONE, SYNCE_SPI_GPIO_CS);
#endif

#ifdef VTSS_ARCH_SERVAL
#include <cyg/io/spi_vcoreiii.h>
#define SYNCE_SPI_GPIO_CS 6
CYG_DEVS_SPI_MIPS_VCOREIII_DEVICE(dev, VTSS_SPI_CS_NONE, SYNCE_SPI_GPIO_CS);
#define SYNCE_SPI_T1_GPIO_CS 17
CYG_DEVS_SPI_MIPS_VCOREIII_DEVICE(dev_t1, VTSS_SPI_CS_NONE, SYNCE_SPI_T1_GPIO_CS); /* T1-E1-J1 device on Serval NDI design */
#endif

#ifdef VTSS_ARCH_JAGUAR_1
#include <cyg/io/spi_vcoreiii.h>
#define SYNCE_SPI_GPIO_CS 17
CYG_DEVS_SPI_MIPS_VCOREIII_DEVICE(dev, 2, VTSS_SPI_GPIO_NONE);
#define SYNCE_SPI_CPLD_GPIO_CS 18
CYG_DEVS_SPI_MIPS_VCOREIII_DEVICE(dev_cpld, 3, VTSS_SPI_GPIO_NONE); /* CPLD device on New Jr1 ref board design */
#endif

typedef struct {
    BOOL           active;
    u32            time;
    vtss_mtimer_t  timer;
} clock_holdoff_t;

static BOOL hold_over_on_free_run_clock = false;
static BOOL pcb104=FALSE;
static BOOL si5326=FALSE;
static BOOL zarlink=FALSE;
#ifdef VTSS_ARCH_SERVAL
static BOOL t1e1j1=FALSE;
#endif
static clock_frequency_t clock_freq[CLOCK_INPUT_MAX] = {CLOCK_FREQ_INVALID, CLOCK_FREQ_INVALID};
static clock_holdoff_t   clock_holdoff[CLOCK_INPUT_MAX];
static clock_selection_mode_t   clock_selection_mode;
uint clock_my_input_max;  /* actual number of clock sources may be less than the defined CLOCK_INPUT_MAX */
uint synce_my_prio_disabled; 

#define SYNCE_CUST_RC(expr) { vtss_rc _rc_ = (expr); if (_rc_ < VTSS_RC_OK) { \
T_I("Error code: %x", _rc_); }}

static i8 synce_clk_sel(void)
{
    // the I2C clock selector depends on the board
    int board_type = vtss_board_type();

    if (board_type == VTSS_BOARD_SERVAL_REF || board_type == VTSS_BOARD_SERVAL_PCB106_REF) {
        return 11;    // on serval ref board, use GPIO#11 as clock selector
    } else {
        return NO_I2C_MULTIPLEXER;
    }
}



static void spi_set_address(uchar address)
{
    cyg_uint8 tx_data[2], rx_data[2];

    tx_data[0] = SPI_SET_ADDRESS;
    tx_data[1] = address;
    cyg_spi_transfer((cyg_spi_device*)&dev, 0, 2, tx_data, rx_data);
}


static void spi_write(uchar address, uchar data)
{
    cyg_uint8 tx_data[2], rx_data[2];

    T_I("address %x, data %x", address, data);
    if (si5326) {
        spi_set_address(address);
    
        tx_data[0] = SPI_WRITE;
        tx_data[1] = data;
        cyg_spi_transfer((cyg_spi_device*)&dev, 0, 2, tx_data, rx_data);
    }
    if (zarlink) {
        address &= 0x7F;    /* Clear first bit to indicate write */
        tx_data[0] = address;
        tx_data[1] = data;
        cyg_spi_transfer((cyg_spi_device*)&dev, 0, 2, tx_data, rx_data);
    }
}


static void spi_write_masked(uchar address, uchar data, uchar mask)
{
    cyg_uint8 tx_data[2], rx_data[2];

    T_I("address %x, data %x, mask %x", address, data, mask);
    if (si5326) {
        spi_set_address(address);
    
        tx_data[0] = SPI_READ;
        tx_data[1] = 0;
        cyg_spi_transfer((cyg_spi_device*)&dev, 0, 2, tx_data, rx_data);
    
        tx_data[0] = SPI_WRITE;
        tx_data[1] = (rx_data[1] & ~mask) | (data & mask);
        cyg_spi_transfer((cyg_spi_device*)&dev, 0, 2, tx_data, rx_data);
    }
    if (zarlink) {
        address |= 0x80;    /* Set first bit to indicate read */
        tx_data[0] = address;
        tx_data[1] = 0;
        cyg_spi_transfer((cyg_spi_device*)&dev, 0, 2, tx_data, rx_data);

        address &= 0x7F;    /* Clear first bit to indicate write */
        tx_data[0] = address;
        tx_data[1] = (rx_data[1] & ~mask) | (data & mask);
        cyg_spi_transfer((cyg_spi_device*)&dev, 0, 2, tx_data, rx_data);
    }
}

static void spi_read(uchar address, uchar *data)
{
    cyg_uint8 tx_data[2], rx_data[2];

    *data = 0;

    if (si5326) {
        spi_set_address(address);
    
        tx_data[0] = SPI_READ;
        tx_data[1] = 0;
        cyg_spi_transfer((cyg_spi_device*)&dev, 0, 2, tx_data, rx_data);
        *data = rx_data[1];
    }
    if (zarlink) {
        address |= 0x80;    /* Set first bit to indicate read */
        tx_data[0] = address;
        tx_data[1] = 0;
        cyg_spi_transfer((cyg_spi_device*)&dev, 0, 2, tx_data, rx_data);
        *data = rx_data[1];
    }
}

/*
static void spi_write_increment(uchar data)
{
    cyg_uint8 tx_data[2], rx_data[2];

    tx_data[0] = SPI_WRITE_INCREMENT;
    tx_data[1] = data;
    cyg_spi_transfer((cyg_spi_device*)&dev, 0, 2, tx_data, rx_data);
}


static void spi_read_increment(uchar *data)
{
    cyg_uint8 tx_data[2], rx_data[2];

    tx_data[0] = SPI_READ_INCREMENT;
    tx_data[1] = 0;
    cyg_spi_transfer((cyg_spi_device*)&dev, 0, 2, tx_data, rx_data);

    *data = rx_data[1];
}
*/


#ifdef VTSS_ARCH_SERVAL
static void spi_t1_write(uchar address, uchar data)
{
    cyg_uint8 tx_data[3], rx_data[3];

    tx_data[0] = address >> 7;
    tx_data[1] = address << 1;
    tx_data[2] = data;

    cyg_spi_transfer((cyg_spi_device*)&dev_t1, 0, 3, tx_data, rx_data);
}

static void spi_t1_read(uchar address, uchar *data)
{
    cyg_uint8 tx_data[3], rx_data[3];

    tx_data[0] = (address >> 7) | 0x80;
    tx_data[1] = address << 1;
    tx_data[2] = 0;

    cyg_spi_transfer((cyg_spi_device*)&dev_t1, 0, 3, tx_data, rx_data);

    *data = rx_data[2];
}

static void spi_t1_write_masked(uchar address, uchar data, uchar mask)
{
    cyg_uint8 tx_data[3], rx_data[3];

    tx_data[0] = (address >> 7) | 0x80; /* Read command */
    tx_data[1] = address << 1;

    cyg_spi_transfer((cyg_spi_device*)&dev_t1, 0, 2, tx_data, rx_data);

    tx_data[0] &= 0x7F;                 /* Write command */
    tx_data[2] = (rx_data[1] & ~mask) | (data & mask);
    cyg_spi_transfer((cyg_spi_device*)&dev_t1, 0, 3, tx_data, rx_data);
}
#endif

#ifdef VTSS_ARCH_JAGUAR_1
static void spi_cpld_write(uchar address, uchar data)
{
    cyg_uint8 tx_data[2], rx_data[2];

    tx_data[0] = (address & 0x7F) | 0x80;
    tx_data[1] = data;

    cyg_spi_transfer((cyg_spi_device*)&dev_cpld, 0, 2, tx_data, rx_data);
}
static void spi_cpld_read(uchar address, uchar *data)
{
    cyg_uint8 tx_data[2], rx_data[2];

    tx_data[0] = (address & 0x7F);
    tx_data[1] = 0;

    cyg_spi_transfer((cyg_spi_device*)&dev_cpld, 0, 2, tx_data, rx_data);

    *data = rx_data[1];
}

#endif

/*
 * This function implements the automatic mode if holdoff is configured.
 * I.e. When LOS is detected in automatic mode the switchover to an other source is postponed until the holdoff timer expires
 * pseudo code:
 * if automatic mode {
 *   if (los && holdoff timer active) keep selected source.
 *   if (los && holdof timer expired) select the best clock without active LOS.
 *   if new selected is not configured for holdoff, then enter automatic mode in hw
 * }
 */
static void control_selector(void)
{
    u8 pval = 0, lval = 0, freerun = 0;
    BOOL found, los = FALSE;
    u32 i, best_prio, best_clock, prio = 0;

    T_I("clock_selection_mode %d", clock_selection_mode);
    if ((clock_selection_mode == CLOCK_SELECTION_MODE_AUTOMATIC_NONREVERTIVE) || (clock_selection_mode == CLOCK_SELECTION_MODE_AUTOMATIC_REVERTIVE)) {
        best_prio = clock_my_input_max;
        best_clock = 0;
        found = FALSE;

        if (si5326)  {
            spi_read(SPI_CK_PRIOR_REG, &pval);
            spi_read(SPI_LOS_INT_REG, &lval);
            spi_read(SPI_FREE_RUN_REG, &freerun);
        }
        for(i=0; i<clock_my_input_max; ++i) {
            if (si5326)  {
                prio = (i == ((pval & 0x0F) >> 2)) ? 1 : 0;
                los = (i) ? (lval & 0x04) : (lval & 0x02);
            }
#ifdef VTSS_SW_OPTION_ZL_3034X_API
            if (zarlink) {
                SYNCE_CUST_RC(zl_3034x_clock_priority_get(i,&prio));
                SYNCE_CUST_RC(zl_3034x_clock_los_get(i,&los));
            }
#endif
            los = los && !clock_holdoff[i].active;   /* LOS is not active if Hold Off timer is still running */
            if ((prio < best_prio) && (!los)) { /* check for better priority on clock without active LOS */
                found = TRUE;
                best_clock = i;
                best_prio = prio;
            }
        }

        T_I("found %d, best_clock %d, best_prio %d", found, best_clock, best_prio);
        if (found) { /* Best clock found */
            if (si5326)  {
                if (!(freerun & 0x40) && (clock_holdoff[best_clock].time)) { /* Not in free run and hold off configured on this best clock - manual select */
                    spi_write_masked(SPI_CKSEL_REG, best_clock<<6, 0xC0);
                    spi_write_masked(SPI_AUTOSEL_REG, 0x00, 0xC0);
                }                    
                else {  /* Free run or no hold off configured on this best clock - automatic mode */
                    if (clock_selection_mode == CLOCK_SELECTION_MODE_AUTOMATIC_NONREVERTIVE)    spi_write_masked(SPI_AUTOSEL_REG, 0x40, 0xC0);
                    if (clock_selection_mode == CLOCK_SELECTION_MODE_AUTOMATIC_REVERTIVE)       spi_write_masked(SPI_AUTOSEL_REG, 0x80, 0xC0);
                }
            }
#ifdef VTSS_SW_OPTION_ZL_3034X_API
            if (zarlink) {
                clock_selector_state_t  selector_state;
                uint clock_input;
                SYNCE_CUST_RC(zl_3034x_clock_selector_state_get(&selector_state, &clock_input));
                T_I("selector state %d, clock_input %d", selector_state, clock_input);
                if ((selector_state != CLOCK_SELECTOR_STATE_FREERUN) && (clock_holdoff[best_clock].time)) { /* Not in free run and hold off configured on this best clock - manual select */
                    SYNCE_CUST_RC(zl_3034x_clock_selection_mode_set(CLOCK_SELECTION_MODE_MANUEL, best_clock));
                    T_I("Set selection mode to manuel, best_clock %d", best_clock);
                } else {
                    SYNCE_CUST_RC(zl_3034x_clock_selection_mode_set(clock_selection_mode, best_clock));
                    T_I("Set selection mode to %d, best_clock %d", clock_selection_mode, best_clock);
                }
            }
#endif
        }
    }
}


/*
 * This function sets up the frequency plan for the two clock inputs, then the clock is in locked mode.
 */
static void locked_mode(const uint                 clock_input,
                        const clock_frequency_t    frequency)
{
    u8 freerun;
    const u8 frq_125 [] = {0x00, 0x0c, 0x34};
    const u8 frq_10  [] = {0x00, 0x00, 0xf9};
    const u8 *n3counter;

    T_D("Set source %d to expect %s", clock_input, frequency == CLOCK_FREQ_125MHZ ? "125 MHz" : "10 MHz");
    spi_read(SPI_FREE_RUN_REG, &freerun);

    if (freerun & 0x40) return;

    spi_write_masked(SPI_DHOLD_REG, 0x20, 0x20);   /* Forced hold over */

    if (!pcb104) {
        if ((vtss_board_type() == VTSS_BOARD_JAG_CU24_REF) || (vtss_board_type() == VTSS_BOARD_JAG_SFP24_REF) || (vtss_board_type() == VTSS_BOARD_JAG_PCB107_REF))
        {
            spi_write_masked(40, 0x40, 0xE0);   /* N2_HS: 6 */
    
            spi_write_masked(40, 0, 0x0F);   /* N2_LS: 420 */
            spi_write(41, 0x01);             /* N2_LS: 420 */
            spi_write(42, 0xA3);             /* N2_LS: 420 */
    
            spi_write(43, 0);   /* N31: 63 */
            spi_write(44, 0);   /* N31: 63 */
            spi_write(45, 62);  /* N31: 63 */
    
            spi_write(46, 0);   /* N32: 63 */
            spi_write(47, 0);   /* N32: 63 */
            spi_write(48, 62);  /* N32: 63 */
        }
        else
        {
            spi_write(SPI_N32_REG+0, 0x00);             /* configure clock port 2 to 125 mHz */
            spi_write(SPI_N32_REG+1, 0x03);
            spi_write(SPI_N32_REG+2, 0xC7);
        }
    }
    else {
        /* clock1 125Mhz and clock 2 125Mhz */
        if (frequency == CLOCK_FREQ_125MHZ) {
            n3counter = frq_125;
        } else {
            n3counter = frq_10;
        }
        if (clock_input == 0) {
            spi_write(43, n3counter[0]);   /* N31 */
            spi_write(44, n3counter[1]);
            spi_write(45, n3counter[2]);
            /* if 125 MHz then CLKIN_0 is selected, else SMA1 is selected as input source */
        } else {        
            spi_write(46, n3counter[0]);   /* N32 */
            spi_write(47, n3counter[1]);
            spi_write(48, n3counter[2]);
        }
    }

    spi_write_masked(SPI_DHOLD_REG, 0x00, 0x20);        /* Get out of forced hold over */
}


/*
 * This function sets up the frequency plan for the two clock inputs, then the clock is in free run mode.
 */
static void free_run_mode(void)
{
    u8 freerun;

    spi_read(SPI_FREE_RUN_REG, &freerun);
    if (!(freerun & 0x40)) {
        spi_write_masked(SPI_DHOLD_REG, 0x20, 0x20);   /* Forced hold over */

        if (!pcb104) {
            if ((vtss_board_type() == VTSS_BOARD_JAG_CU24_REF) || (vtss_board_type() == VTSS_BOARD_JAG_SFP24_REF) || (vtss_board_type() == VTSS_BOARD_JAG_PCB107_REF)) {
                /* clock1 125Mhz and clock 2 114,285Mhz */
                spi_write_masked(40, 0xC0, 0xE0);   /* N2_HS: 10 */
                
                spi_write_masked(40, 0, 0x0F);   /* N2_LS: 280 */
                spi_write(41, 0x01);             /* N2_LS: 280 */
                spi_write(42, 0x17);             /* N2_LS: 280 */
        
                spi_write(43, 0);   /* N31: 70 */
                spi_write(44, 0);   /* N31: 70 */
                spi_write(45, 69);  /* N31: 70 */
                
                spi_write(46, 0);   /* N32: 64 */
                spi_write(47, 0);   /* N32: 64 */
                spi_write(48, 63);  /* N32: 64 */
            }
            else {
                spi_write(SPI_N32_REG+0, 0x00);                     /* XA/XB on CKIN_2 */
                spi_write(SPI_N32_REG+1, 0x03);
                spi_write(SPI_N32_REG+2, 0x74);
            }
        }
        else {
            /* clock1 125Mhz and clock 2 38,88Mhz */
            spi_write(43, 0x0);   /* N31 */
            spi_write(44, 0x0C);
            spi_write(45, 0x34);
            
            spi_write(46, 0x0);   /* N32 */
            spi_write(47, 0x03);
            spi_write(48, 0xCB);
        }
        spi_write_masked(SPI_FREE_RUN_REG, 0x40, 0x40);     /* Free run */
    }

    spi_write_masked(SPI_CKSEL_REG, 0x40, 0xC0);        /* CKIN_2 selected */
    spi_write_masked(SPI_AUTOSEL_REG, 0x00, 0xC0);      /* Manuel select */

    spi_write_masked(SPI_DHOLD_REG, 0x00, 0x20);     /* Get out of forced hold over */
}


/*
 * This thread waits for the source clock frequencies to be set.
 * Then if (the clock is in free_run mode ) set clock into holdover mode
 */
static BOOL clock_thread_stop = FALSE;
static void clock_thread(cyg_addrword_t data)
{
    uchar hold_valid, free_run, active;

    CRIT_ENTER();
    hold_over_on_free_run_clock = TRUE;                 /* we go into hold over on free run clock until new hold over valid */
    CRIT_EXIT();

    for (;;)
    {
        VTSS_OS_MSLEEP(500);

        CRIT_ENTER();

        if (clock_thread_stop) {
            clock_thread_stop = FALSE;
            break;
        }

        if ((clock_freq[0] != CLOCK_FREQ_INVALID) || (clock_freq[1] != CLOCK_FREQ_INVALID)) {
            spi_read(SPI_DIGHOLDVALID_REG, &hold_valid);
            spi_read(SPI_FREE_RUN_REG, &free_run);
            spi_read(SPI_CLK_ACTV_REG, &active);
    
            if (free_run & 0x40)
            {
            /* clock controller in free run - check for digital hold valid */
                if (hold_valid & 0x40)
                {
                /* now we can disable Free Run Mode as we are sure digital hold is ready  */
                    spi_write_masked(SPI_DHOLD_REG, 0x20, 0x20);        /* Forced hold over */
                    spi_write_masked(SPI_FREE_RUN_REG, 0x20, 0x60);     /* Disable Free Run Mode and select CKOUT always on */
    
                    locked_mode(0, clock_freq[0]);
                    locked_mode(1, clock_freq[1]);
                }
            }
            else
            {
                if (active & 0x03)
                {
                    if (hold_valid & 0x40)
                    {
                        hold_over_on_free_run_clock = FALSE;    /* when new hold over valid appears we can go into real holdover on an external clock */
                        break;   /* done when NOT hold over on free run clock */
                    }
                }
            }
        }
        CRIT_EXIT();
    }
    CRIT_EXIT(); /* This is when this infinite loop is breaked */
}



vtss_rc clock_init(BOOL  cold_init)
{
    vtss_rc rc=VTSS_OK;
    u8      partnum[2];

    /*lint --e{454} */
    /* Locking mechanism is initialized but not released - this is to protect any call to API until clock_startup has been done */
    critd_init(&crit, "Clock Crit", VTSS_MODULE_ID_SYNCE, VTSS_MODULE_ID_SYNCE, CRITD_TYPE_MUTEX);

#ifdef VTSS_ARCH_LUTON28
    cyg_vcoreii_spi_bus_init(); /* actually this should not be done here, it should be part of system initialisation */
    cyg_vcoreii_spi_device_init(&dev);
#endif

#if defined(VTSS_ARCH_LUTON26) || defined(VTSS_ARCH_SERVAL)
    rc += vtss_gpio_mode_set(NULL, 0, SYNCE_SPI_GPIO_CS, VTSS_GPIO_OUT);
    rc += vtss_gpio_write(NULL, 0, SYNCE_SPI_GPIO_CS, TRUE);
    dev.delay = 1;
#endif
#ifdef VTSS_ARCH_SERVAL
    rc += vtss_gpio_mode_set(NULL, 0, SYNCE_SPI_T1_GPIO_CS, VTSS_GPIO_OUT);
    rc += vtss_gpio_write(NULL, 0, SYNCE_SPI_T1_GPIO_CS, TRUE);
    dev_t1.delay = 1;
#endif
#ifdef VTSS_ARCH_JAGUAR_1
    rc += vtss_gpio_mode_set(NULL, 0, SYNCE_SPI_GPIO_CS, VTSS_GPIO_ALT_0);
    dev.delay = 1;
#endif
    dev.init_clk_high = FALSE;

    memset(clock_holdoff, 0, sizeof(clock_holdoff));

    zarlink = TRUE;               /* Try if a Zarlink controller is on SPI */
    spi_read(0, &partnum[0]);     /* Read chip id to detect if this is a Zarlink */
    zarlink = (((partnum[0] & 0x1F) == 0x0C) || ((partnum[0] & 0x1F) == 0x0D));

#ifdef VTSS_ARCH_SERVAL
    spi_t1_read(16, &partnum[0]);  /* Try if a T1-E1-J1 controller is on SPI -- Read chip id to detect if this */
    t1e1j1 = ((partnum[0] & 0xF0) == 0x10);
#endif

    cyg_thread_create(THREAD_DEFAULT_PRIO,
                      clock_thread,
                      0,
                      "Clock API",
                      thread_stack_clock,
                      sizeof(thread_stack_clock),
                      &thread_handle_clock,
                      &thread_block_clock);

    if (zarlink) {
        clock_my_input_max = CLOCK_INPUT_MAX;
        synce_my_prio_disabled = 0x0f;

        /* do ZL30343 initialization */
#ifdef VTSS_SW_OPTION_ZL_3034X_API
        rc = zl_3034x_clock_init(cold_init);
#endif
        return(rc);
    } else {
    
        clock_my_input_max = CLOCK_INPUT_MAX-1;
        synce_my_prio_disabled = clock_my_input_max;
        si5326 = TRUE;
        spi_read(134, &partnum[0]);   /* Try if Silabs 5326 is on SPI */
        spi_read(135, &partnum[1]);   /* Read part number to detect if this is a Silabs 5326 */
        si5326 = ((partnum[0] == 0x01) && (((partnum[1] & 0xF0)== 0xA0) || ((partnum[1] & 0xF0)== 0xC0)));
        if (cold_init && si5326) {
        /* This is to support the SiLabs SynceE module as the enable of 156,25 on CLKOUT2must be done as erly as possible - before we know what board it is */
        /* If this is a Vitesse SyncE board this SPI operations will go no-where as CPLD SPI selector is not configured yet */
            if ((vtss_board_type() == VTSS_BOARD_JAG_CU24_REF) || (vtss_board_type() == VTSS_BOARD_JAG_SFP24_REF) || (vtss_board_type() == VTSS_BOARD_JAG_PCB107_REF))
            {
                /* On Jaguar platform clock port 2 has to be used for 156,25 MHz to 10G PHY */
                /* This config is for free-run (114,285) on CLKIN2 and 125 on CLKIN1 + 125 on CLKOUT1 and 156,25 on CLKOUT2 */
                spi_write_masked(25, 0x00, 0xE0);   /* N1_HS: 4 */
                
                spi_write(31, 0);   /* NC1_LS: 10 */
                spi_write(32, 0);   /* NC1_LS: 10 */
                spi_write(33, 9);   /* NC1_LS: 10 */
                
                spi_write(34, 0);   /* NC2_LS: 8 */
                spi_write(35, 0);   /* NC2_LS: 8 */
                spi_write(36, 7);   /* NC2_LS: 8 */
            
                /* Enable CKOUT2 output buffer */
                spi_write_masked(10, 0x00, 0x08);
                /* Enable CKOUT2 (CMOS) */
                spi_write_masked(6, 0x10, 0x38);
            }
            else {
                /* 10 MHz clockoutput on port 2 is wanted and possible */
                /* Enable CKOUT2 output buffer */
                spi_write_masked(10, 0x00, 0x08);
                /* Enable CKOUT2 (CMOS) */
                spi_write_masked(6, 0x10, 0x38);
                /* set NC2_LS to 50 (= NC1_LS *125MHz/10MHz), */
                /* NC1_LS = 4 */
                spi_write(36, 49); /* a binary value of 49 means divide by 50 */
            }
        }
    }
    return(rc);
}

/*lint -sem(clock_startup,   thread_protected) ... We're protected */
vtss_rc clock_startup(BOOL  cold_init,
                      BOOL  pcb104_synce)
{
    u8      partnum[2];

    if (!zarlink) {

        /* Before this is called the clock_init must be called and any HW configuration must be done to assure SPI is connected to Si5326 */
        pcb104 = pcb104_synce;
    
        si5326 = TRUE;
        spi_read(134, &partnum[0]);   /* Read part number to detect if this is a Silabs 5326 */
        spi_read(135, &partnum[1]);   /* Read part number to detect if this is a Silabs 5326 */
        si5326 = ((partnum[0] == 0x01) && (((partnum[1] & 0xF0)== 0xA0) || ((partnum[1] & 0xF0)== 0xC0)));
        T_I("si5326 part number 0x%x, 0x%x", partnum[0], partnum[1]);
        if (!si5326) {
            T_W("Unknown Silabs CLOCK MULTIPLIER part mumber 0x%x, 0x%x", partnum[0], partnum[1]);
        }
        if (cold_init && si5326) {
            /* interrupt to output pin */
            spi_write_masked(20, 0x01, 0x01);
            /* various pins are disabled */
            spi_write(21, 0x00);
    
            if (!pcb104) {
            }
            else {
                /* FOS reference selector to XA/XB */
                spi_write_masked(7, 0x00, 0x07);
                /* FOS Threshold to +/- 200ppm */
                spi_write_masked(19, 0x60, 0x60);
                /* FOS measured clock rate divider */
//                spi_write_masked(55, 0x1B, 0x3F);
                /* Disable FOS monitoring as it will not work anyway */
//                spi_write_masked(139, 0x00, 0x03);
            }
        }
    } else {    /* Do zl30343 startup */
#ifdef VTSS_SW_OPTION_ZL_3034X_API
        (void)zl_3034x_clock_startup(cold_init);
#endif
    }

    /*lint -e(455) */
    CRIT_EXIT();

    return(VTSS_OK);
}



vtss_rc clock_selection_mode_set(const clock_selection_mode_t   mode,
                                 const uint                     clock_input)
{
    u8 freerun, hold_valid;
    vtss_rc rc = VTSS_OK;
    if (clock_input >= clock_my_input_max)  return(rc);

    CRIT_ENTER();
    clock_selection_mode = mode;

    if (si5326)  {
        spi_read(SPI_FREE_RUN_REG, &freerun);
        freerun &= 0x40;
        clock_thread_stop = FALSE;

        switch (mode)
        {
            case CLOCK_SELECTION_MODE_MANUEL:
                if (freerun)    cyg_thread_resume(thread_handle_clock);         /* Get out of free run */
                spi_write_masked(SPI_DHOLD_REG, 0x00, 0x20);                    /* Get out of forced hold over */
                spi_write_masked(SPI_CKSEL_REG, clock_input<<6, 0xC0);
                spi_write_masked(SPI_AUTOSEL_REG, 0x00, 0xC0);
                break;
            case CLOCK_SELECTION_MODE_AUTOMATIC_NONREVERTIVE:
                if (freerun)    cyg_thread_resume(thread_handle_clock);         /* Get out of free run */
                spi_write_masked(SPI_DHOLD_REG, 0x00, 0x20);                    /* Get out of forced hold over */
                control_selector();
                break;
            case CLOCK_SELECTION_MODE_AUTOMATIC_REVERTIVE:
                if (freerun)    cyg_thread_resume(thread_handle_clock);         /* Get out of free run */
                spi_write_masked(SPI_DHOLD_REG, 0x00, 0x20);                    /* Get out of forced hold over */
                control_selector();
                break;
            case CLOCK_SELECTION_MODE_FORCED_HOLDOVER:
                if (freerun)                 break;
                spi_read(SPI_DIGHOLDVALID_REG, &hold_valid);
                if (!(hold_valid & 0x40))    break;
                spi_write_masked(SPI_DHOLD_REG, 0x20, 0x20);
                break;
            case CLOCK_SELECTION_MODE_FORCED_FREE_RUN:
                spi_write_masked(SPI_DHOLD_REG, 0x00, 0x20);                    /* Get out of forced hold over */
                clock_thread_stop = TRUE;
                free_run_mode();
                break;
            default: break;
        }
    }
    if (zarlink)  {
#ifdef VTSS_SW_OPTION_ZL_3034X_API
        rc = zl_3034x_clock_selection_mode_set(mode, clock_input);
        switch (mode)
        {
            case CLOCK_SELECTION_MODE_MANUEL:
                break;
            case CLOCK_SELECTION_MODE_AUTOMATIC_NONREVERTIVE:
                control_selector();
                break;
            case CLOCK_SELECTION_MODE_AUTOMATIC_REVERTIVE:
                control_selector();
                break;
            case CLOCK_SELECTION_MODE_FORCED_HOLDOVER:
                break;
            case CLOCK_SELECTION_MODE_FORCED_FREE_RUN:
                break;
            default: break;
        }
#endif
    }
    CRIT_EXIT();

    return(rc);
}



vtss_rc clock_selector_state_get(clock_selector_state_t  *const selector_state,
                                 uint                    *const clock_input)
{
    uchar clk_actv, free_run, lol;

    *selector_state = CLOCK_SELECTOR_STATE_FREERUN;
    *clock_input = 0;
    vtss_rc rc = VTSS_OK;

    CRIT_ENTER();
    if (si5326)  {
        spi_read(SPI_CLK_ACTV_REG, &clk_actv);
        spi_read(SPI_FREE_RUN_REG, &free_run);
        spi_read(SPI_LOL_INT_REG, &lol);

        lol &= 0x01;
        free_run &= 0x40;

        *selector_state = CLOCK_SELECTOR_STATE_LOCKED;

        if (free_run && (clk_actv & 0x02))    *selector_state = CLOCK_SELECTOR_STATE_FREERUN;    /* FREE_RUN is set and clock 2 is active - then it is free run  */
        else
        if (lol) {
            if (hold_over_on_free_run_clock)    *selector_state = CLOCK_SELECTOR_STATE_FREERUN;    /* Hold over - but on the free run frequency  */
            else                                *selector_state = CLOCK_SELECTOR_STATE_HOLDOVER;   /* Loss Of Lock is active and NOT free run - then it is hold over  */
        }
        else
        if (clk_actv & 0x01) *clock_input = 0;
        else
        if (clk_actv & 0x02) *clock_input = 1;
        else
        if (hold_over_on_free_run_clock)    *selector_state = CLOCK_SELECTOR_STATE_FREERUN;    /* Hold over - but on the free run frequency  */
        else                                *selector_state = CLOCK_SELECTOR_STATE_HOLDOVER;
    }
    if (zarlink)  {
#ifdef VTSS_SW_OPTION_ZL_3034X_API
        rc = zl_3034x_clock_selector_state_get(selector_state, clock_input);
#endif
    }
    CRIT_EXIT();

    return(rc);
}


static u8 clock_priority = 0xFF;

vtss_rc clock_priority_set(const uint   clock_input,
                           const uint   priority)
{
    u8 other_clk, reg;
    vtss_rc rc = VTSS_OK;

    if (clock_input >= clock_my_input_max)  return(rc);

    CRIT_ENTER();
    if (si5326)  {
        other_clk = (clock_input) ? 0 : 1;  /* A clock input must be assigned to a quality or it can not be selected - this might cause spurious LOL during re-configuration of priority */
        reg = (priority) ? ((clock_input<<2) | other_clk) : ((other_clk<<2) | clock_input);
        spi_write_masked(SPI_CK_PRIOR_REG, reg, 0x0F);

        if (reg != clock_priority) {
            clock_priority = reg;
            control_selector();
        }
    }
    if (zarlink)  {
#ifdef VTSS_SW_OPTION_ZL_3034X_API
        rc = zl_3034x_clock_priority_set(clock_input, priority);
#endif
    }
    CRIT_EXIT();

    return(rc);
}



vtss_rc   clock_priority_get(const uint   clock_input,
                             uint         *const priority)
{
    u8 prio;
    vtss_rc rc = VTSS_OK;

    *priority = 0;

    if (clock_input >= clock_my_input_max)  return(VTSS_OK);

    CRIT_ENTER();
    if (si5326)  {
        spi_read(SPI_CK_PRIOR_REG, &prio);
        *priority = (clock_input == ((prio & 0x0F) >> 2)) ? 1 : 0;
    }
    if (zarlink)  {
#ifdef VTSS_SW_OPTION_ZL_3034X_API
        rc = zl_3034x_clock_priority_get(clock_input, priority);
#endif
    }
    CRIT_EXIT();

    return(rc);
}

vtss_rc   clock_holdoff_time_set(const uint   clock_input,
                                 const uint   ho_time)
{
    vtss_rc rc = VTSS_OK;
    if (clock_input >= clock_my_input_max)  return(VTSS_OK);
    CRIT_ENTER();
    clock_holdoff[clock_input].time = ho_time;

    control_selector();
    CRIT_EXIT();
    T_I("clock_input %d, time %d", clock_input, ho_time);
    return(rc);
}



vtss_rc   clock_holdoff_event(const uint   clock_input)
{
    u8 val;
    BOOL state = FALSE;
    vtss_rc rc = VTSS_OK;

    if (clock_input >= clock_my_input_max)  return(rc);

    CRIT_ENTER();
    if (si5326)  {
        spi_read(SPI_LOS_INT_REG, &val);
        state = (clock_input) ? (val & 0x04) : (val & 0x02);
    }
    if (zarlink)  {
#ifdef VTSS_SW_OPTION_ZL_3034X_API
        rc = zl_3034x_clock_los_get(clock_input, &state);
#endif
    }
    if (state && clock_holdoff[clock_input].time) {
        clock_holdoff[clock_input].active = TRUE;
        VTSS_MTIMER_START(&clock_holdoff[clock_input].timer, clock_holdoff[clock_input].time);
    }

    control_selector();
    CRIT_EXIT();
    T_I("clock_input %d, state %d", clock_input, state);

    return(rc);
}


vtss_rc   clock_holdoff_run(BOOL *const active)
{
    u8 val;
    u32 i;
    BOOL state = FALSE;
    vtss_rc rc = VTSS_OK;


    CRIT_ENTER();
    *active = FALSE;

    for (i=0; i<clock_my_input_max; ++i) { /* Run through all clocks */
        if (clock_holdoff[i].time && clock_holdoff[i].active) {   /* This clock has active Hold Off - run it */
            if (si5326)  {
                spi_read(SPI_LOS_INT_REG, &val);
                state = (i) ? (val & 0x04) : (val & 0x02);
            }
            if (zarlink)  {
#ifdef VTSS_SW_OPTION_ZL_3034X_API
                rc = zl_3034x_clock_los_get(i, &state);
#endif
            }
            if (state) { /* LOCS is still active */
                if (VTSS_MTIMER_TIMEOUT(&clock_holdoff[i].timer)) {
                    clock_holdoff[i].active = FALSE;
                    control_selector(); /* Hold Off time out - set the selector */
                }
                else
                    *active = TRUE;  /* No time out - keep running */
            }
            else
                clock_holdoff[i].active = FALSE;
        }
    }
    CRIT_EXIT();
    T_I("active %d", *active);

    return(rc);
}



vtss_rc   clock_holdoff_active_get(const uint   clock_input,
                                   BOOL         *const active)
{
    *active = FALSE;

    CRIT_ENTER();
    *active = clock_holdoff[clock_input].active;
    CRIT_EXIT();
    T_I("clock_input %d, active %d", clock_input, *active);

    return(VTSS_OK);
}



vtss_rc clock_frequency_set(const uint                 clock_input,
                            const clock_frequency_t    frequency)
{
    if (clock_input >= clock_my_input_max)        return(VTSS_OK);
    if (clock_freq[clock_input] == frequency)  return(VTSS_OK);

    CRIT_ENTER();
    clock_freq[clock_input] = frequency;

    switch (frequency) {
        case CLOCK_FREQ_125MHZ:
        case CLOCK_FREQ_10MHZ:
            if (si5326) {
                locked_mode(clock_input, frequency);
            }
            break;
        default: break;
    }
    CRIT_EXIT();

    return(VTSS_OK);
}



vtss_rc   clock_locs_state_get(const uint   clock_input,
                               BOOL         *const state)
{
    uchar val;
    vtss_rc rc = VTSS_OK;
    *state = FALSE;
    CRIT_ENTER();
    
    if (si5326)  {
        spi_read(SPI_LOS_INT_REG, &val);
        T_I("clock_input %d, SPI_LOS_INT_REG %x", clock_input, val);
        *state = (clock_input) ? (val & 0x04) : (val & 0x02);
    }
    if (zarlink)  {
#ifdef VTSS_SW_OPTION_ZL_3034X_API
        rc = zl_3034x_clock_los_get(clock_input, state);
#endif
    }
    CRIT_EXIT();

    return(rc);
}

vtss_rc   clock_fos_state_get(const uint   clock_input,
                              BOOL         *const state)
{
    uchar val;

    *state = FALSE;

    if (!si5326)  return(VTSS_OK); /* not used in zarlink */

    CRIT_ENTER();
    spi_read(SPI_FOS_INT_REG, &val);

    *state = (clock_input) ? (val & 0x04) : (val & 0x02);
    CRIT_EXIT();

    return(VTSS_OK);
}

vtss_rc   clock_losx_state_get(BOOL         *const state)
{
    uchar val;
    vtss_rc rc = VTSS_OK;
    *state = FALSE;
    CRIT_ENTER();
    if (si5326)  {
        spi_read(SPI_LOSX_INT_REG, &val);

        *state = val & 0x01;
    }
    if (zarlink)  {
#ifdef VTSS_SW_OPTION_ZL_3034X_API
        rc = zl_3034x_clock_losx_state_get(state);
#endif
    }
    CRIT_EXIT();

    return(rc);
}

vtss_rc   clock_lol_state_get(BOOL         *const state)
{
    uchar val;
    vtss_rc rc = VTSS_OK;
    *state = FALSE;
    CRIT_ENTER();
    if (si5326)  {
        spi_read(SPI_LOL_INT_REG, &val);

        *state = val & 0x01;
    }
    if (zarlink)  {
#ifdef VTSS_SW_OPTION_ZL_3034X_API
        rc = zl_3034x_clock_lol_state_get(state);
#endif
    }
    CRIT_EXIT();
    return(rc);
}



vtss_rc   clock_dhold_state_get(BOOL         *const state)
{
    uchar val;
    vtss_rc rc = VTSS_OK;
    *state = FALSE;
    CRIT_ENTER();
    if (si5326)  {
        spi_read(SPI_DIGHOLDVALID_REG, &val);

        *state = val & 0x40;
    }
    if (zarlink)  {
#ifdef VTSS_SW_OPTION_ZL_3034X_API
        rc = zl_3034x_clock_dhold_state_get(state);
#endif
    }
    CRIT_EXIT();

    return(rc);
}

void clock_event_poll(BOOL interrupt,  clock_event_type_t *ev_mask)
{
    u8       status1, mask1, status2, mask2;
    u8       pending1, pending2;
    *ev_mask = 0;

    CRIT_ENTER();

    if (si5326)  {
        pending1 = pending2 = 0;
        
        /* Read interrupt pending and mask register 1 in Clock controller */
        spi_read(CLOCK_INTERRUPT_PENDING_REG, &pending1);
        spi_read(CLOCK_INTERRUPT_PENDING_REG+1, &pending2);
        spi_read(CLOCK_INTERRUPT_MASK_REG, &mask1);
        spi_read(CLOCK_INTERRUPT_MASK_REG+1, &mask2);
        pending2 = pending2>>1; /* for some obscure reason pending2 is rotated to the left in register */
        T_I("pending1: %x, mask1 %x, pending2: %x, mask2 %x", pending1, mask1, pending2, mask2);

        if (!interrupt)
        {
        /* No interrupt polling for falling edge */
            /* During timeout we are polling for falling edge of a source as it is not able to generate interrupt */
            spi_read(CLOCK_INTERRUPT_STATUS_REG, &status1);
            spi_read(CLOCK_INTERRUPT_STATUS_REG+1, &status2);
            status2 &= 0x07;  /* This is to clear bit's used for other things */
            /* This is a bit tricky. Pending is actually a latched version of status. So if pending and status not the same it's a falling edge of status */
            /* The XOR of pending and status is treated as active pending and all hooked to the source is signalled */
            pending1 ^= status1;
            pending2 ^= status2;
            /* clear pending in Clock controller */
            spi_write_masked(CLOCK_INTERRUPT_PENDING_REG, 0, pending1);
            spi_write_masked(CLOCK_INTERRUPT_PENDING_REG+1, 0, pending2<<1);
        }
        else
        {
        /* interrupt */
            /* only handle interrupt active on sources still enabled */
            pending1 &= ~mask1; /* remember that interrupt is enabled when bit is '0' */
            pending2 &= ~mask2; /* remember that interrupt is enabled when bit is '0' */
        }

        *ev_mask = 0;
        if (pending1)
        {
            /* mask has to be cleared on pending sources */
            spi_write_masked(CLOCK_INTERRUPT_MASK_REG, pending1, pending1);
            if (pending1 & CLOCK_LOSX_MASK)
            /* Change in LOSX */
                *ev_mask |= CLOCK_LOSX_EV;
            if (pending1 & CLOCK_LOS1_MASK)
            /* Change in LOS1 */
                *ev_mask |= CLOCK_LOCS1_EV;
            if (pending1 & CLOCK_LOS2_MASK)
            /* Change in LOS2 */
                *ev_mask |= CLOCK_LOCS2_EV;
        }
        if (pending2)
        {
            /* mask has to be cleared on pending sources */
            spi_write_masked(CLOCK_INTERRUPT_MASK_REG+1, pending2, pending2);
            if (pending2 & CLOCK_LOL_MASK)
            /* Change in LOL */
                *ev_mask |= CLOCK_LOL_EV;
            if (pending2 & CLOCK_FOS1_MASK)
            /* Change in FOS1 */
                *ev_mask |= CLOCK_FOS1_EV;
            if (pending2 & CLOCK_FOS2_MASK)
            /* Change in FOS2 */
                *ev_mask |= CLOCK_FOS2_EV;
        }
        T_I("interrupt: %d, ev_mask %x", interrupt, *ev_mask);
    }
    if (zarlink)  {
#ifdef VTSS_SW_OPTION_ZL_3034X_API
        zl_3034x_clock_event_poll(interrupt, ev_mask);
#endif
    }
    CRIT_EXIT();
}

void clock_event_enable(clock_event_type_t ev_mask)
{
    u8  mask, pending1, pending2;

    CRIT_ENTER();
    if (si5326)  {
        spi_read(CLOCK_INTERRUPT_PENDING_REG, &pending1);
        spi_read(CLOCK_INTERRUPT_PENDING_REG+1, &pending2);
        pending2 = pending2>>1; /* for some obscure reason pending2 is rotated to the left in register */

        if (ev_mask & CLOCK_LOCS1_EV) {
            mask = CLOCK_LOS1_MASK;
            spi_write_masked(CLOCK_INTERRUPT_MASK_REG, 0, mask&~pending1);
        }
        if (ev_mask & CLOCK_LOCS2_EV) {
            mask = CLOCK_LOS2_MASK;
            spi_write_masked(CLOCK_INTERRUPT_MASK_REG, 0, mask&~pending1);
        }
        if (ev_mask & CLOCK_FOS1_EV) {
            mask = CLOCK_FOS1_MASK;
            spi_write_masked(CLOCK_INTERRUPT_MASK_REG+1, 0, mask&~pending2);
        }
        if (ev_mask & CLOCK_FOS2_MASK) {
            mask = CLOCK_FOS2_EV;
            spi_write_masked(CLOCK_INTERRUPT_MASK_REG+1, 0, mask&~pending2);
        }
        if (ev_mask & CLOCK_LOSX_MASK) {
            mask = CLOCK_LOSX_EV;
            spi_write_masked(CLOCK_INTERRUPT_MASK_REG, 0, mask&~pending1);
        }
        if (ev_mask & CLOCK_LOL_EV) {
            mask = CLOCK_LOL_MASK;
            spi_write_masked(CLOCK_INTERRUPT_MASK_REG+1, 0, mask&~pending2);
        }
    }
    if (zarlink)  {
#ifdef VTSS_SW_OPTION_ZL_3034X_API
        zl_3034x_clock_event_enable(ev_mask);
#endif
    }
    CRIT_EXIT();
}


vtss_rc clock_station_clk_out_freq_set(const u32 freq_khz)
{    
    vtss_rc rc = VTSS_OK;
    u8      tx_buf[2];
    CRIT_ENTER();
    if (si5326)  {
        
        tx_buf[0] = 15; /* Set CPLD Clock input selector: CKIN1 connected to CLKIN_0 */
        if (freq_khz == 10000) {
            tx_buf[1] = 0x02;
        } else  if (freq_khz == 2048) {
            tx_buf[1] = 0x01;
        } else  if (freq_khz == 0) {
            tx_buf[1] = 0x00;
        } else {
            rc = VTSS_RC_ERROR;
        }
        if (rc == VTSS_OK) {
            if (vtss_i2c_wr(NULL, 0x75, tx_buf, 2, 50, synce_clk_sel()) != VTSS_RC_OK)    T_D("Clock freq out set failed  %u", 0);
            T_I("Set output frequencey");
        }
    }
    if (zarlink)  {
#ifdef VTSS_SW_OPTION_ZL_3034X_API
        rc = zl_3034x_station_clk_out_freq_set(freq_khz);
#endif
    }
    CRIT_EXIT();
    return(rc);
}

vtss_rc clock_station_clk_in_freq_set(const u32 freq_khz)
{    
    vtss_rc rc = VTSS_OK;
    CRIT_ENTER();
    if (si5326 && (freq_khz != 0 && freq_khz != 10000))  {
        T_D("not supported yet in silabs designs");
        rc = VTSS_RC_ERROR;
    }
    if (zarlink)  {
#ifdef VTSS_SW_OPTION_ZL_3034X_API
        rc = zl_3034x_station_clk_in_freq_set(freq_khz);
#endif
    }
    CRIT_EXIT();
    return(rc);
}

vtss_rc clock_station_clock_type_get(uint *const clock_type)
{
    vtss_rc rc = VTSS_OK;
    CRIT_ENTER();
    *clock_type = 2;
    if (si5326 && pcb104) *clock_type = 1;
    if (zarlink)  {
        *clock_type = 0;
    }
    T_D("clock_type %d", *clock_type);
    CRIT_EXIT();
    return(rc);
}

vtss_rc clock_eec_option_set(const clock_eec_option_t clock_eec_option)
{
    vtss_rc rc = VTSS_OK;
    CRIT_ENTER();
    if (si5326)  {
        T_I("not supported yet in silabs designs");
    }
    if (zarlink)  {
#ifdef VTSS_SW_OPTION_ZL_3034X_API
        rc = zl_3034x_eec_option_set(clock_eec_option);
#endif
    }
    CRIT_EXIT();
    return(rc);
}


vtss_rc clock_read(const uint     reg,
                   uint    *const value)
{
    uchar val;

    CRIT_ENTER();
#ifdef VTSS_ARCH_SERVAL
    if (t1e1j1 && (reg & 0xFF00))
        spi_t1_read((reg & 0xFF), &val);
    else
#endif
#ifdef VTSS_ARCH_JAGUAR_1
    if (reg & 0xFF00)
        spi_cpld_read((reg & 0xFF), &val);
    else
#endif
    spi_read(reg, &val);
    *value = val;
    CRIT_EXIT();

    return(VTSS_OK);
}



vtss_rc clock_write(const uint    reg,
                    const uint    value)
{
    CRIT_ENTER();
#ifdef VTSS_ARCH_SERVAL
    if (t1e1j1 && (reg & 0xFF00))
        spi_t1_write((reg & 0xFF), value);
    else
#endif
#ifdef VTSS_ARCH_JAGUAR_1
    if (reg & 0xFF00)
        spi_cpld_write((reg & 0xFF), value);
    else
#endif
    spi_write(reg, value);
    CRIT_EXIT();

    return(VTSS_OK);
}



vtss_rc clock_writemasked(const uint     reg,
                          const uint     value,
                          const uint     mask)
{
    CRIT_ENTER();
#ifdef VTSS_ARCH_SERVAL
    if (t1e1j1 && (reg & 0xFF00))
        spi_t1_write_masked((reg & 0xFF), value, mask);
    else
#endif
    spi_write_masked(reg, value, mask);
    CRIT_EXIT();

    return(VTSS_OK);
}


