/*

 Vitesse Switch Software.

 #####ECOSGPLCOPYRIGHTBEGIN#####
 -------------------------------------------
 This file is part of eCos, the Embedded Configurable Operating System.
 Copyright (C) 1998-2012 Free Software Foundation, Inc.

 eCos is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free
 Software Foundation; either version 2 or (at your option) any later
 version.

 eCos is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.

 You should have received a copy of the GNU General Public License
 along with eCos; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 As a special exception, if other files instantiate templates or use
 macros or inline functions from this file, or you compile this file
 and link it with other works to produce a work based on this file,
 this file does not by itself cause the resulting work to be covered by
 the GNU General Public License. However the source code for this file
 must still be made available in accordance with section (3) of the GNU
 General Public License v2.

 This exception does not invalidate any other reasons why a work based
 on this file might be covered by the GNU General Public License.
 -------------------------------------------
 #####ECOSGPLCOPYRIGHTEND#####

*/
//==========================================================================
//
//      I2C driver for Vitesse Semiconductor Switch Chip
//
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):
// Contributors:  Flemming Jahn
// Date:          2010-08-04
// Description:   I2C driver for Vitesse Semiconductor Switch chip
//####DESCRIPTIONEND####
//==========================================================================

#include <stdio.h>
#include <stdlib.h>
#include <pkgconf/system.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#include <cyg/io/i2c.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/i2c_vcoreiii.h>

#if defined(CYG_HAL_VCOREIII_CHIPTYPE_LUTON26)
#define GPIO_SCL 5
#define GPIO_SDA 6
#elif defined(CYG_HAL_VCOREIII_CHIPTYPE_JAGUAR)
#define GPIO_SCL 14
#define GPIO_SDA 15
#elif defined(CYG_HAL_VCOREIII_CHIPTYPE_SERVAL)
#define GPIO_SCL 6
#define GPIO_SDA 7

#else
#warning Unknown Board flavor - I2C may not work.
#endif

#ifndef MAX
  #define MAX(x,y) ((x)>(y) ? (x) : (y))
#endif
#define CYG_I2C_MSEC2TICK(msec) MAX(1, msec / (CYGNUM_HAL_RTC_NUMERATOR / CYGNUM_HAL_RTC_DENOMINATOR / 1000000)) // returns at least 1 tick

#define VCOREIII_TX_FIFO_THRESHOLD  6

// Macro for accessing registers - Used for being able to see when we do registers accesses
#define VTSS_WR(address,data) ((address) = (data))
#define VTSS_RD(address) (address)

// Enable for printing out debugging
#if 0
#define DEBUG(_format_, ...) (void) diag_printf(_format_, ## __VA_ARGS__)
#else
  #define DEBUG(_format_, ...)
#endif

// Macro for printing warning messages
#if 0
#define WARNING(_format_, ...) (void) diag_printf(_format_, ## __VA_ARGS__)
#else
  #define WARNING(_format_, ...)
#endif

// Macro for printing error messages
#if 1
#define ERROR(_format_, ...) (void) diag_printf(_format_, ## __VA_ARGS__)
#else
  #define ERROR(_format_, ...)
#endif

#define I2C_MAX_RETRIES 3       /* Driver I2c retries */

// The interrupt flag bits.
#define XFER_STAT_DONE         VTSS_BIT(0) /* Rx/Tx xfer done */
#define XFER_STAT_ERROR        VTSS_BIT(1) /* Error on xfer */
#define XFER_STAT_ERROR_ABORT  VTSS_BIT(2) /* Error on abort */
#define XFER_STAT_ERROR_STOP   VTSS_BIT(3) /* Error on stop */
#define XFER_STAT_ERROR_UNK    VTSS_BIT(4) /* Error on unexpected */
#define XFER_STAT_ERROR_BUSY   VTSS_BIT(5) /* Controller busy */
#define XFER_STAT_ERROR_NULL   VTSS_BIT(6) /* Null Request */
#define XFER_STAT_ERROR_UNINIT VTSS_BIT(7) /* Unitialized */
#define XFER_STAT_ALL     0xff        /* All causes */

/* Non-zero => error */
static int inline check_empty(void)
{
    cyg_uint32 reg;
    int i = 20;
    static cyg_uint32 seq;
    seq++;
    while((reg = VTSS_RD(VTSS_TWI_TWI_ENABLE_STATUS)) & VTSS_F_TWI_TWI_ENABLE_STATUS_BUSY) {
        if(i-- == 0) {
            ERROR("I2C controller seems stuck: seq %d, %0x, TAR = 0x%lx\n", seq, reg, VTSS_RD(VTSS_TWI_TWI_TAR));
            return 1;           /* return driver error */
        }
        HAL_DELAY_US(20);       /* Unfortunately our only option...! */
    }
    if ((reg = VTSS_RD(VTSS_TWI_TWI_RXFLR)) != 0) { ERROR("RX buffer non-empty :0x%X \n", reg); return 1; }
    if ((reg = VTSS_RD(VTSS_TWI_TWI_TXFLR)) != 0)  { ERROR("TX buffer non-empty :0x%X \n", reg); return 1;}
    if ((reg = VTSS_RD(VTSS_TWI_TWI_INTR_MASK)) != 0) {ERROR ("Interrupt mask non-zero :0x%X \n", reg); return 1;}
    if ((reg = VTSS_RD(VTSS_TWI_TWI_CTRL))  != 0) { ERROR("CTRL non-zero :0x%X \n", reg); return 1;}
    return 0;
}


/* NB: This function explicitly does not disable interrupts!
 * ---------------------------------------------------------
 * Either we are called when starting up - and no TWI irq conditions are enabled,
 * - OR we are on DSR context, and the main TWI irq is masked all-together.
 * So there!
 */
static void stuff_fifo(cyg_vcore_i2c_req *req)
{
    while(VTSS_RD(VTSS_TWI_TWI_STAT) & VTSS_F_TWI_TWI_STAT_TFNF) {
        if(req->transmit_count) {
            VTSS_WR(VTSS_TWI_TWI_DATA_CMD, *req->transmit_pointer++);
            req->transmit_count--;
        } else if(req->read_unissued) {
            VTSS_WR(VTSS_TWI_TWI_DATA_CMD, VTSS_F_TWI_TWI_DATA_CMD_CMD); /* Issue read command */
            req->read_unissued--;
        } else {
            break;
        }
    }
}

// ----------------------------------------------------------------------------
// Interrupt handling and polling
//

static cyg_uint32 vcoreiii_i2c_isr(cyg_vector_t vec, cyg_addrword_t data)
{
    cyg_drv_interrupt_mask(vec); // Block this interrupt until unmask
    cyg_drv_interrupt_acknowledge(vec);     // Tell eCos to allow further interrupt processing
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR); // Call the DSR
}

static void irq_signal_done(cyg_vcore_i2c_extra *extra, cyg_flag_value_t cond)
{
    VTSS_WR(VTSS_TWI_TWI_INTR_MASK, 0); /* Disable ints */
    cyg_flag_setbits(&extra->wait_flag, cond);
}

static void vcoreiii_i2c_dsr(cyg_vector_t vec, cyg_ucount32 count, cyg_addrword_t data)
{
    cyg_vcore_i2c_extra *extra = (cyg_vcore_i2c_extra *) data;
    cyg_vcore_i2c_req   *i2c_req = &extra->req;
    cyg_uint32 status;

    while((status = VTSS_RD(VTSS_TWI_TWI_INTR_STAT)) && i2c_req) { // figure out what interrupt we got
        i2c_req->last_stat = status; /* For status reporting */

        /* Transmit abort? */
        if(status & VTSS_F_TWI_TWI_INTR_STAT_TX_ABRT) {
            i2c_req->abort_stat = VTSS_RD(VTSS_TWI_TWI_TX_ABRT_SOURCE);
            WARNING("TX abrt src 0x%x\n", i2c_req->abort_stat);
            irq_signal_done(extra, XFER_STAT_ERROR|XFER_STAT_ERROR_ABORT);
            break;
        }

        /* Check RX - TX - Other (failure) interrupts */
        if (status & VTSS_F_TWI_TWI_INTR_MASK_M_RX_FULL) {
            status &= ~VTSS_F_TWI_TWI_INTR_MASK_M_RX_FULL;
            if (i2c_req->read_pointer == 0) {
                ERROR ("Got data, but not expecting any?\n");
                VTSS_WR(VTSS_TWI_TWI_INTR_MASK, 0); /* Disable ints */
            } else {
                // Drain Rx FIFO
                while(VTSS_RD(VTSS_TWI_TWI_STAT) & VTSS_F_TWI_TWI_STAT_RFNE) { /* While data in fifo */
                    cyg_uint8 rd_data = VTSS_RD(VTSS_TWI_TWI_DATA_CMD);
                    if(i2c_req->read_pending) {
                        *i2c_req->read_pointer++ = rd_data;
                        i2c_req->read_pending--;
                    } else {
                        ERROR("i2c: Read buffer overrun, skipping data byte 0x%02x\n", rd_data);
                    }
                }
            }
        }

        if (status & VTSS_F_TWI_TWI_INTR_MASK_M_TX_EMPTY) {
            status &= ~VTSS_F_TWI_TWI_INTR_MASK_M_TX_EMPTY;            
            if(i2c_req->transmit_count || i2c_req->read_unissued) {
                DEBUG("Stuffing more data - tx %d rx %d left \n", i2c_req->transmit_count, i2c_req->read_unissued);
                stuff_fifo(i2c_req);
            } else {
                /* Done tx, no more IRQ's here */
                VTSS_WR(VTSS_TWI_TWI_INTR_MASK, 
                        VTSS_RD(VTSS_TWI_TWI_INTR_MASK) & ~VTSS_F_TWI_TWI_INTR_MASK_M_TX_EMPTY);
            }
        }

        /* Stop detected? */
        if (status & VTSS_F_TWI_TWI_INTR_STAT_STOP_DET) {
            cyg_bool error = false;
            if(i2c_req->transmit_count != 0) {
                error = true;
                WARNING("Stop on %d byte transfer(s) remaining\n", i2c_req->transmit_count);
            } else if(i2c_req->read_pending != 0) {
                error = true;
                WARNING("Stop on %d byte read(s) remaining\n", i2c_req->read_pending);
            }
            /* Signal done */
            irq_signal_done(extra, error ? XFER_STAT_ERROR|XFER_STAT_ERROR_STOP : XFER_STAT_DONE);
            break;
        }

        if(status) {
            ERROR ("i2c: Unexpected IRQ (TWI_INTR_STAT = 0x%0x)\n", status);
            irq_signal_done(extra, XFER_STAT_ERROR|XFER_STAT_ERROR_UNK);
        }

    }
    
    if(!i2c_req) {
        VTSS_WR(VTSS_TWI_TWI_INTR_MASK, 0); /* Disable interrupts */
        ERROR("I2C interrupt while no pending request - status 0x%0x\n", status);
    }

    cyg_drv_interrupt_unmask(vec);   // Re-unmasked the interrupt
}

// Function for restarting the controller. This is needed when the TAR
// register needs to change.
// In : dev - The i2c dev.
static void vcoreiii_restart_controller(const cyg_i2c_device* dev)
{
    // Disable the I2C controller because TAR register cannot be changed when the controller is enabled.
    VTSS_WR(VTSS_TWI_TWI_CTRL, 0x0);

    // Set Target address
    VTSS_WR(VTSS_TWI_TWI_TAR, dev->i2c_address);
    
    // Enable the I2C controller
    VTSS_WR(VTSS_TWI_TWI_CTRL, VTSS_F_TWI_TWI_CTRL_ENABLE);
}

static void vcoreiii_disable_controller(void)
{
    /* Disable ints */
    VTSS_WR(VTSS_TWI_TWI_INTR_MASK, 0);
    /* Disable the I2C controller */
    VTSS_WR(VTSS_TWI_TWI_CTRL, 0x0);
}


void cyg_vcoreiii_i2c_set_gpio_alt(cyg_bool_t set, int i2c_clk_sel) {
    
    if (set) {
        // If i2c clock multiplexing is supported (i2c_clk_sel >= 0) then set the i2c multiplexer
        if (i2c_clk_sel >= 0) {
            vcoreiii_gpio_set_alternate(i2c_clk_sel, 3); // Enable TWI overlaid multiplexer mode
            vcoreiii_gpio_set_output_level(i2c_clk_sel, 1); // Enable the TWI clk output
            DEBUG("i2c_clk_sel:%d,\n", i2c_clk_sel);    
        }

        // The I2C is an overlaid function on the GPIOx and GPIOy. Enable overlaying function 1
        vcoreiii_gpio_set_alternate(GPIO_SCL, 1); // Set SCL alternative mode 1 (TWI mode)
        vcoreiii_gpio_set_alternate(GPIO_SDA, 1); // Set SDA alternative mode 1 (TWI mode)
    } else {
        // If i2c clock multiplexing is supported (i2c_clk_sel >= 0) then disable the i2c multiplexer
        if (i2c_clk_sel >= 0) {
            vcoreiii_gpio_set_output_level(i2c_clk_sel, 0); // Disable the TWI clk output
            vcoreiii_gpio_set_alternate(i2c_clk_sel, 3); // Disable TWI overlaid multiplexer mode (Set to GPIO mode)
        }

        vcoreiii_gpio_set_alternate(GPIO_SCL, 0); // Set SCL alternative mode 0 (GPIO mode)
        vcoreiii_gpio_set_alternate(GPIO_SDA, 0);     // Set SDA alternative mode 0 (GPIO mode) 
    }
}

// ----------------------------------------------------------------------------
// The functions needed for all I2C devices.

void cyg_vcoreiii_i2c_init(struct cyg_i2c_bus *bus)
{
    cyg_vcore_i2c_extra *extra = bus->i2c_extra;
    static cyg_interrupt   i2c_interrupt_data;
    static cyg_handle_t    i2c_interrupt_handle;   // For initializing the interrupt
    int register_value;

    DEBUG("extra->i2c_flags = %u\n", extra->i2c_flags);
    // Initialize interrupt and mutex
    if (!(extra->i2c_flags & I2C_FLAG_INIT)) {

        //
        // The bus frequency is set by the platform.
        //
        // Set clock speed for standard speed
        int clk_freq = CYGNUM_HAL_MIPS_VCOREIII_AHB_CLOCK;
        
        DEBUG("clk_freq = %u\n", clk_freq);
    
        // Setting high clock flank to 5 us (for standard clk (100 kHz))
        register_value = (5 * clk_freq / 1000000) - 8; // Datasheet Section 6.13.9.5
        VTSS_WR(VTSS_TWI_TWI_SS_SCL_HCNT, register_value);
        DEBUG("VTSS_TWI_TWI_SS_SCL_HCNT = %u\n", register_value);

        // Setting low clock flank to 5 us (for standard clk (100 kHz))
        register_value = (5 * clk_freq / 1000000) - 1; // Datasheet Section 6.13.9.6
        VTSS_WR(VTSS_TWI_TWI_SS_SCL_LCNT,register_value);
        DEBUG("VTSS_TWI_TWI_SS_SCL_LCNT = %u\n", register_value);
        
        // Setting TWI_DELAY to 1000ns
        register_value = VTSS_F_ICPU_CFG_TWI_DELAY_TWI_CONFIG_TWI_CNT_RELOAD((unsigned int)(1 * clk_freq / 1000000) - 1) |
            VTSS_F_ICPU_CFG_TWI_DELAY_TWI_CONFIG_TWI_DELAY_ENABLE; // Datasheet Section 6.8.3
        
        VTSS_WR(VTSS_ICPU_CFG_TWI_DELAY_TWI_CONFIG, register_value);
        DEBUG("VTSS_ICPU_CFG_TWI_DELAY_TWI_CONFIG = %u\n", register_value);
        
        // Setting high clock flak to 1.1 us (for fast clock (400 kHz)) (Asym. because VTSS_TWI_FS_SCL_LCNT mustn't be below 1.3 us).
        register_value = (1.1 * clk_freq / 1000000) - 8; // Datasheet Section 6.13.9.7
        VTSS_WR(VTSS_TWI_TWI_FS_SCL_HCNT, register_value);
        DEBUG("VTSS_TWI_TWI_FS_SCL_HCNT = %u\n", register_value);
        
        // Setting low clock flak to 1.4 us ( for fast clock (400 kHz)) ( Asym. because VTSS_TWI_FS_SCL_LCNT mustn't be below 1.3 us).
        register_value = (1.4 * clk_freq / 1000000) - 1 ;// Datasheet Section 6.13.9.8
        VTSS_WR(VTSS_TWI_TWI_FS_SCL_LCNT,register_value);
        DEBUG("VTSS_TWI_TWI_FS_SCL_LCNT = %u\n", register_value);
        
        // Set I2C clock frequency to 100 kbps.
        register_value = 0;
        register_value |=  VTSS_BIT(0); // Master enable
        register_value |=  VTSS_BIT(1); // Set bit 1
        register_value &= ~VTSS_BIT(2); // Clear bit 2
        register_value &= ~VTSS_BIT(3); // 7 bit mode
        register_value &= ~VTSS_BIT(4); // 7 bit mode
        register_value |=  VTSS_BIT(5); // Restart enable
        register_value |=  VTSS_BIT(6); // Slave disable
        
        VTSS_WR(VTSS_TWI_TWI_CFG, register_value);
        DEBUG("VTSS_TWI_TWI_CFG  = %u\n", register_value);
        
        // Set clock speed for to normal speed
        register_value = (0.25 * clk_freq / 1000000); // Datasheet section 6.13.9.30
        VTSS_WR(VTSS_TWI_TWI_SDA_SETUP, register_value);
        DEBUG("VTSS_TWI_TWI_SDA_SETUP  = %u\n", register_value);
                
        // Generate RX int on first byte
        VTSS_WR(VTSS_TWI_TWI_RX_TL, 0x00);
        // Generate TX int when less than 3/4 full
        VTSS_WR(VTSS_TWI_TWI_TX_TL, VCOREIII_TX_FIFO_THRESHOLD);

        vcoreiii_disable_controller();
        
        /* eCos IRQ hookup */
        cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_TWI,  // Interrupt Vector
                                 0,   // Interrupt Priority
                                 (cyg_addrword_t) extra,
                                 vcoreiii_i2c_isr,
                                 vcoreiii_i2c_dsr,
                                 &i2c_interrupt_handle,
                                 &i2c_interrupt_data);

        cyg_drv_interrupt_attach(i2c_interrupt_handle);

        // Interrupts can now be safely unmasked
        cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_TWI);

        /* Initialize state */
        memset(&extra->req, 0, sizeof(extra->req));
        cyg_flag_init(&extra->wait_flag);
        
        extra->i2c_flags = I2C_FLAG_INIT;
    }

    cyg_vcoreiii_i2c_set_gpio_alt(true, extra->i2c_clk_sel);
}

static void i2c_restart (int i2c_clk_sel)
{
    int i, delay = 4;              /* I2C standard */
    
    /* Make sure SCL starts high */
    VTSS_DEVCPU_GCB_GPIO_GPIO_OUT_SET = VTSS_BIT(GPIO_SCL);

    /* Hijack - use as GPIO */
    vcoreiii_gpio_set_input(GPIO_SCL, 0);
    vcoreiii_gpio_set_input(GPIO_SDA, 1);

    for(i = 0; i < 8; i++) {
        VTSS_DEVCPU_GCB_GPIO_GPIO_OUT_SET = VTSS_BIT(GPIO_SCL); /* SCL High */
        HAL_DELAY_US(delay);
        if(VTSS_DEVCPU_GCB_GPIO_GPIO_IN & VTSS_BIT(GPIO_SDA)) {
            WARNING("SDA went high at %d\n", i);
            /* Simulate Stop condition - Drive SDA Low */
            VTSS_DEVCPU_GCB_GPIO_GPIO_OUT_CLR = VTSS_BIT(GPIO_SDA); /* SDA Low */
            vcoreiii_gpio_set_input(GPIO_SDA, 0); 
            HAL_DELAY_US(delay); 
            VTSS_DEVCPU_GCB_GPIO_GPIO_OUT_SET = VTSS_BIT(GPIO_SCL); /* SCL High */
            HAL_DELAY_US(delay);
            VTSS_DEVCPU_GCB_GPIO_GPIO_OUT_SET = VTSS_BIT(GPIO_SDA); /* SDA High */
            HAL_DELAY_US(delay);
            break;
        }
        VTSS_DEVCPU_GCB_GPIO_GPIO_OUT_CLR = VTSS_BIT(GPIO_SCL); /* SCL Low */
        HAL_DELAY_US(delay);
    }        
    
    /* Release Hijack - use as GPIO */
    vcoreiii_gpio_set_input(GPIO_SCL, 1); 
    vcoreiii_gpio_set_input(GPIO_SDA, 1);

    cyg_vcoreiii_i2c_set_gpio_alt(true, i2c_clk_sel);
}

static cyg_flag_value_t ll_i2c_xfer(const cyg_i2c_device *dev)
{
    cyg_vcore_i2c_extra *extra = dev->i2c_bus->i2c_extra;
    cyg_vcore_i2c_req *req = &extra->req;
    cyg_flag_value_t flag_value = XFER_STAT_ERROR|XFER_STAT_ERROR_NULL; /* If test below fails */

    if(req->transmit_count || req->read_pending) {
        cyg_uint16 wait_tmo = extra->i2c_wait;

        /* Assure buffers are empty */
        if(check_empty())
            return XFER_STAT_ERROR|XFER_STAT_ERROR_BUSY;

        cyg_drv_dsr_lock();

        // Initialize the xfer - Setup Target address register
        vcoreiii_restart_controller(dev);

        stuff_fifo(req);        /* Put initial data in */
        VTSS_WR(VTSS_TWI_TWI_INTR_MASK, 
                VTSS_F_TWI_TWI_INTR_MASK_M_RX_FULL|
                VTSS_F_TWI_TWI_INTR_MASK_M_TX_ABRT|
                VTSS_F_TWI_TWI_INTR_MASK_M_TX_EMPTY|
                VTSS_F_TWI_TWI_INTR_STAT_RX_DONE|
                VTSS_F_TWI_TWI_INTR_STAT_RX_OVER|
                VTSS_F_TWI_TWI_INTR_STAT_STOP_DET|
                VTSS_F_TWI_TWI_INTR_STAT_TX_OVER);

        cyg_drv_dsr_unlock();

        /* Wait for transfer completion */
        flag_value = cyg_flag_timed_wait(&extra->wait_flag, 
                                         XFER_STAT_ALL,
                                         CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR, 
                                         cyg_current_time() + CYG_I2C_MSEC2TICK(wait_tmo));

        /* Stop controller */
        vcoreiii_disable_controller();

        // Timeout?
        if (flag_value == 0) {
            WARNING("I2C Timeout(addr:%02X) - tx left %d, rx unissued %d pending %d\n", 
                    dev->i2c_address,
                    req->transmit_count, req->read_unissued, req->read_pending);
            i2c_restart(extra->i2c_clk_sel);
        } else if (flag_value & XFER_STAT_ERROR)   {
            // Error
            if(flag_value & XFER_STAT_ERROR_ABORT) {
                WARNING("Transfer incomplete(addr:%02X) - abort signaled - status 0x%x\n", 
                        dev->i2c_address, req->abort_stat);
                if(req->abort_stat & 
                   (VTSS_F_TWI_TWI_TX_ABRT_SOURCE_ABRT_TXDATA_NOACK|VTSS_F_TWI_TWI_TX_ABRT_SOURCE_ARB_LOST)) {
                    i2c_restart(extra->i2c_clk_sel);
                }
            }
            if(flag_value & XFER_STAT_ERROR_STOP) {
                WARNING("Transfer incomplete(addr:%02X) - stop detected, tx left %d, rx unissued %d pending %d\n\n", 
                      dev->i2c_address, req->transmit_count, req->read_unissued, req->read_pending);
            }
            if(flag_value & XFER_STAT_ERROR_UNK) {
                ERROR("Transfer incomplete(addr:%02X) - unknown condition 0x%0x\n", dev->i2c_address, req->last_stat);
            }
        }
    } else {
        flag_value = XFER_STAT_ERROR|XFER_STAT_ERROR_UNK;
    }
    
    return flag_value;
}

static cyg_bool_t cyg_vcoreiii_i2c_xfer(const cyg_i2c_device *dev)
{
    cyg_vcore_i2c_extra *extra = dev->i2c_bus->i2c_extra;
    cyg_vcore_i2c_req *busreq = &extra->req;
    cyg_flag_value_t ret = XFER_STAT_ERROR|XFER_STAT_ERROR_UNINIT; /* Uninitailized or no I2C */
    if((extra->i2c_flags & I2C_FLAG_INIT) && dev->i2c_address) {
        cyg_vcore_i2c_req req_copy;
        int i = 0;
        req_copy = *busreq;     /* Save original request */
        while(((ret = ll_i2c_xfer(dev)) & XFER_STAT_DONE) != XFER_STAT_DONE && ++i < I2C_MAX_RETRIES) {
            /* Re-init req, retry */
            WARNING("I2C Error(addr:%02X): Retry request #%d, CC = 0x%x, TAR = 0x%02lx\n", 
                    dev->i2c_address, i, ret, VTSS_RD(VTSS_TWI_TWI_TAR));
            *busreq = req_copy;
        }
        if((ret & XFER_STAT_DONE) != XFER_STAT_DONE) {
            WARNING("I2C Error(addr:%02X): Failed request after %d retries - CC 0x%x, last_stat 0x%x, abort stat 0x%x\n", 
                    dev->i2c_address, I2C_MAX_RETRIES, ret, busreq->last_stat, busreq->abort_stat);
        }
    }

    /* Set Pins back to GPIO mode */    
    cyg_vcoreiii_i2c_set_gpio_alt(false, extra->i2c_clk_sel);

    return !!(ret & XFER_STAT_DONE);
}

static void request_init(cyg_vcore_i2c_req *req)
{
    memset(req, 0, sizeof(*req));
}

static cyg_bool_t reqeust_add_txdata(cyg_vcore_i2c_req *req, 
                                     const cyg_uint8 *tx_data, 
                                     cyg_uint32 count)
{
    if(req->transmit_pointer == NULL) {
        req->transmit_pointer = tx_data; /* Tx buffer */
        req->transmit_count = count; /* Transmit data */
        return true;
    }
    return false;               /* Has tx buffer already */
}

static cyg_bool_t reqeust_add_rxdata(cyg_vcore_i2c_req *req,
                                     cyg_uint8 *rx_buffer, 
                                     cyg_uint32 count)
{
    if(req->read_pointer == NULL) {
        req->read_pointer = rx_buffer; /* Rx buffer */
        req->read_pending = req->read_unissued = count;
        return true;
    }
    return false;               /* Has rx buffer already */
}

cyg_uint32 cyg_vcoreiii_i2c_tx(const cyg_i2c_device *dev,
                               cyg_bool send_start,
                               const cyg_uint8 *tx_data,
                               cyg_uint32 count,
                               cyg_bool send_stop)
{
    cyg_vcore_i2c_extra *extra = dev->i2c_bus->i2c_extra;

    DEBUG("TX %u bytes from %p - address 0x%x (flags %x)\n", count, tx_data, dev->i2c_address, extra->i2c_flags);

    /* Because the controller doesn't like to be reset while a i2c
     * access is in progress we will stop of further accesses is the
     * system is going to reboot. The application layer must remember
     * to signal this by doing a dummy write when rebooting. Any I2C
     * address can be used.
     */
    if (extra->i2c_flags & I2C_FLAG_REBOOT) {
        DEBUG("system rebooting return count\n"); 
    } else {

        // Select i2c clk output
        cyg_vcoreiii_i2c_set_gpio_alt(true, extra->i2c_clk_sel);
 
        if(send_start)
            request_init(&extra->req);
        if(!reqeust_add_txdata(&extra->req, tx_data, count)) {
            ERROR("Error adding tx size %d, have %d\n", count, extra->req.transmit_count);
            return 0;
        }

        if(send_stop) {
            if(!cyg_vcoreiii_i2c_xfer(dev))
                return 0;
        }
        /* ELSE - have tx data in buffer */
    }

    return count;
}

cyg_uint32 cyg_vcoreiii_i2c_rx(const cyg_i2c_device* dev,
                               cyg_bool send_start,
                               cyg_uint8* rx_data,
                               cyg_uint32 count,
                               cyg_bool send_nack,
                               cyg_bool send_stop)
{
    cyg_vcore_i2c_extra *extra = dev->i2c_bus->i2c_extra;

    DEBUG("RX %u bytes to %p - address 0x%x\n", count, rx_data, dev->i2c_address);


    // If rebooting is in progress then simply skip the read
    if (extra->i2c_flags & I2C_FLAG_REBOOT) {
        DEBUG("system rebooting return zero\n"); 
        count = 0;
    } else {

        // Select i2c clk output
        cyg_vcoreiii_i2c_set_gpio_alt(true, extra->i2c_clk_sel);

        if(send_start)
            request_init(&extra->req);
        if(!reqeust_add_rxdata(&extra->req, rx_data, count)) {
            ERROR("Error adding rx size %d, have %d\n", count, extra->req.read_unissued);
            return 0;
        }

        if(send_stop) {
            if(!cyg_vcoreiii_i2c_xfer(dev))
                return 0;
        } else {
            ERROR("RX segment without stop?\n");
            return 0;
        }
    }

    return count;
}

void cyg_vcoreiii_i2c_stop(const cyg_i2c_device *dev)
{
}

//---------------------------------------------------------------------------
// EOF i2c_vcoreiii.c
