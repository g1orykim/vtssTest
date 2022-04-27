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
// Author(s):     Flemming Jahn
// Contributors:  
// Date:          2008-04-31
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
#include <cyg/io/i2c_vcoreii.h>

#ifndef MAX
  #define MAX(x,y) ((x)>(y) ? (x) : (y))
#endif
#define CYG_I2C_MSEC2TICK(msec) MAX(1, msec / (CYGNUM_HAL_RTC_NUMERATOR / CYGNUM_HAL_RTC_DENOMINATOR / 1000000)) // returns at least 1 tick

// Macro for accessing registers - Used for being able to see when we do registers accesses
#define VTSS_WR(address,data) ((address) = (data))
#define VTSS_RD(address) (address)

# define    I2C_BASE(_extra_)       ((_extra_)->i2c_base)
# define    I2C_ISRVEC(_extra_)     ((_extra_)->i2c_isrvec)
# define    I2C_ISRPRI(_extra_)     ((_extra_)->i2c_isrpri)
# define    I2C_FDR(_extra_)        ((_extra_)->i2c_fdr)



// Enable for printing out debuging 
#if 0
# define DEBUG(_format_, ...) diag_printf(_format_, ## __VA_ARGS__)
#else
# define DEBUG(_format_, ...)
#endif


// Macro for printing error messages
#if 1
# define ERROR(_format_, ...) diag_printf(_format_, ## __VA_ARGS__)
#else 
# define ERROR(_format_, ...)
#endif



// Since the hardware I2C controller only have a 8 bytes rx fifo, we use a software fifo to
// which the received bytes is moved to upon interrupt ( move from hardware fifo to sw fifo).
// This constant define the software fifo's depth
#define SW_RX_FIFO_MAX 30

// Software fifo definition
static cyg_uint8  sw_rx_fifo[SW_RX_FIFO_MAX];
static cyg_uint8  sw_rx_fifo_wr_ptr = 0;
static cyg_uint8  sw_rx_fifo_rd_ptr = 0;

static cyg_flag_t interrupt_wait_flag;
static cyg_uint8 interrupt_wait_cnt;
static cyg_drv_mutex_t i2c_lock ;
static cyg_bool system_rebooting = 0;

// Function for incrementing read or write pointer for the software fifo
static cyg_uint8 get_next_sw_fifo_ptr (cyg_uint8 sw_fifo_ptr) {
    if (sw_fifo_ptr < SW_RX_FIFO_MAX - 1) {
        sw_fifo_ptr++;
    } else {
        sw_fifo_ptr = 0;
    }
    return sw_fifo_ptr;
}

// Return the fill level for the software fifo
static cyg_uint8 get_sw_rx_fifo_level (void) {
    if (sw_rx_fifo_wr_ptr >= sw_rx_fifo_rd_ptr) {
        return sw_rx_fifo_wr_ptr - sw_rx_fifo_rd_ptr;
    } else {
        return SW_RX_FIFO_MAX - sw_rx_fifo_rd_ptr + sw_rx_fifo_wr_ptr;
    }
}

// Function that returns one byte from the software fifo ( the read data is removed from the fifo).
cyg_uint8 sw_rx_fifo_rd(void) {
    cyg_uint8 data_read = 0;
    if (get_sw_rx_fifo_level() == 0 ) {
        // This shall never happen - The calling function must check the fifo level before reading the sw fifo.
        ERROR("I2C Error : software rx fifo underrun, sw_rx_fifo_rd_ptr = %d, sw_rx_fifo_wr_ptr = %d\n",
              sw_rx_fifo_rd_ptr, sw_rx_fifo_wr_ptr);
    } else {
        // Read out and remove data from the fifo
        data_read = sw_rx_fifo[sw_rx_fifo_rd_ptr];
        sw_rx_fifo_rd_ptr = get_next_sw_fifo_ptr(sw_rx_fifo_rd_ptr);
    }
    return data_read;
}



// Function for moving data from the hardware fifo to the software fifo.
void sw_rx_fifo_wr(void) {
    sw_rx_fifo[sw_rx_fifo_wr_ptr] = (cyg_uint8)VTSS_RD(VTSS_TWI_DATA_CMD);
    if (get_sw_rx_fifo_level() < (SW_RX_FIFO_MAX -2) ) {
        // Move data from hw-fifo to sw-fifo
        sw_rx_fifo_wr_ptr = get_next_sw_fifo_ptr(sw_rx_fifo_wr_ptr);
    } else {
        ERROR("I2C - software fifo overrun - level = %d, wr_ptr = %d, rd_ptr = %d \n",
              get_sw_rx_fifo_level(),sw_rx_fifo_wr_ptr,sw_rx_fifo_rd_ptr);
    }
}



// ----------------------------------------------------------------------------
// Interrupt handling and polling
//


static cyg_uint32
vcoreii_i2c_isr(cyg_vector_t vec, cyg_addrword_t data)
{
    int dummy; // Used to avoid warning during compile
    cyg_drv_interrupt_mask(vec);            // Block this interrupt until unmask
    cyg_drv_interrupt_acknowledge(vec);     // Tell eCos to allow further interrupt processing

    int int_stat_value = VTSS_RD(VTSS_TWI_INTR_STAT); // Read the I2C interrupt vector 


    // Interrupt due to rx fifo contain data ?
    if ( (int_stat_value & VTSS_F_M_RX_FULL) == VTSS_F_M_RX_FULL) {
        sw_rx_fifo_wr(); // move data from hw fifo to sw fifo
        dummy = VTSS_RD(VTSS_F_R_RX_FULL); // Clear the interrupt
    } else {
        DEBUG ("Could not move to sw fifo, int_stat_value = %d\n", int_stat_value);
        cyg_drv_interrupt_unmask(vec);   // Re-unmasked the interrupt
        return (CYG_ISR_HANDLED);
    }
    return (CYG_ISR_HANDLED|CYG_ISR_CALL_DSR); // Call the DSR
}

static void
vcoreii_i2c_dsr(cyg_vector_t vec, cyg_ucount32 count, cyg_addrword_t data)
{
    interrupt_wait_cnt --;
    if (interrupt_wait_cnt == 0 ) {
        DEBUG("DSR \n");
        cyg_flag_setbits(&interrupt_wait_flag, 1);
    }

    cyg_drv_interrupt_unmask(vec);   // Re-unmasked the interrupt
}




// Function for waiting until the TX buffer is empty . This is needed for making sure that 
// multiple I2C transactions aren't concatenated into one transaction at the I2C bus.
//
// If the buffer isn't emptied within "timeout" ms the function returns 0 else 1
static int wait_for_tx_buffer_empty(cyg_int8 timeout) {

    DEBUG("TX buffer empty Timeout, timeout = %d\n", timeout);

    // Check if TX fifo is empty. If not wait until it is.
    while (VTSS_RD(VTSS_TWI_TXFLR) != 0 && timeout > 0) {
        // Wait 1 ms
        cyg_thread_delay(CYG_I2C_MSEC2TICK(1));
        timeout--; // Make sure we don't run forever
    }

    // Fifo was not emptied within 500 ms.
    if (timeout == 0) {
        ERROR("TX buffer empty Timeout\n");
        return 0;
    }
    
    return 1;
}


// If the buffer is still full after "timeout" ms the function returns 0 else 1
static int wait_for_tx_buf_not_full(cyg_uint8 timeout) {

    DEBUG("wait_for_tx_buf_not_full, timeout = %d\n", timeout);
   
    // Check if TX fifo is empty. If not wait until it is.
    while (VTSS_RD(VTSS_TWI_TXFLR) > 6 && timeout > 0) {
        // Wait 1 ms
        cyg_thread_delay(CYG_I2C_MSEC2TICK(1));
        timeout--; // Make sure we don't run forever
    }

    // Fifo was not emptied within 500 ms.
    if (timeout == 0) {
        ERROR("I2C Error: TX buffer not full Timeout\n");
        return 0;
    }
   
    return 1;
}

// Function for restarting the controller. This is need in case of errors or when the TAR register needs to change.
void vcoreiii_restart_controller(const cyg_i2c_device *dev) {

    cyg_vcore_i2c_extra* extra = 
        (cyg_vcore_i2c_extra*)dev->i2c_bus->i2c_extra;
  
    wait_for_tx_buffer_empty(extra->i2c_wait);
    
    // Check that the hardware fifos are empty ( They are flushed when the controller is disabled )
    if (VTSS_RD(VTSS_TWI_RXFLR) != 0 || VTSS_RD(VTSS_TWI_TXFLR) != 0) {
        ERROR ("I2C - Hardware fifos are being flushed \n");
    }

    // disable the I2C controller because TAR register can not be changed when the controller is enabled.
    VTSS_WR(VTSS_TWI_CTRL,0x0);
    
    //Set Target address
    int tar_reg_value = 0; 
    tar_reg_value |= dev->i2c_address;
    VTSS_WR(VTSS_TWI_TAR,tar_reg_value);
    
    // Enable the I2C controller 
    VTSS_WR(VTSS_TWI_CTRL,VTSS_F_ENABLE);
}

// Due to that the tar register only must be changed when the controller is disabled 
// we have a special function for setting the tar register.
void vcoreii_set_tar_register (const cyg_i2c_device *dev, cyg_bool send_start) {
    // Check if the TAR register needs to be changed.
    if (dev->i2c_address != (VTSS_RD(VTSS_TWI_TAR) & 0x3FF)) {
        vcoreiii_restart_controller(dev);
    }
}

// Function for transmitting the data - Will return for "timeout" mSec.
// returns 0 upon timeout failure.
static int tx_i2c_data(const cyg_uint32 tx_data, cyg_uint8 timeout) 
{
    // Check if TX fifo is full. If so, wait until there is room for one more byte
    if (wait_for_tx_buf_not_full(timeout) == 0) {
        return 0;
    }

    VTSS_WR(VTSS_TWI_DATA_CMD, tx_data);
    return 1;
}



// ----------------------------------------------------------------------------
// The functions needed for all I2C devices.

void
cyg_vcoreii_i2c_init(struct cyg_i2c_bus* bus)
{
    static cyg_drv_cond_t  i2c_wait;
    static cyg_interrupt   i2c_interrupt_data;
    static cyg_handle_t    i2c_interrupt_handle;   // For initializing the interrupt

    int register_value; 


    // Initialise interrupt
    static int interrupt_intialised = 0;
    if (! interrupt_intialised) {
        cyg_drv_mutex_init(&i2c_lock);
        cyg_drv_cond_init(&i2c_wait, &i2c_lock);

        cyg_drv_interrupt_create(CYGNUM_HAL_INT_I2C,  // Interrupt Vector 
                                 0,   // Interrupt Priority 
                                 (cyg_addrword_t) NULL,
                                 &vcoreii_i2c_isr,
                                 &vcoreii_i2c_dsr,
                                 &i2c_interrupt_handle,
                                 &i2c_interrupt_data);
    
        cyg_drv_interrupt_attach(i2c_interrupt_handle); 

        // Interrupts can now be safely unmasked
        cyg_drv_interrupt_unmask(CYGNUM_HAL_INT_I2C);

        cyg_flag_init(&interrupt_wait_flag);

        interrupt_intialised = 1;
    }
  

    while (!cyg_drv_mutex_lock(&i2c_lock)); // Mutex for avoiding mulitple accesses to the hardware


    //
    // The bus frequency is set by the platform.
    //
    // Set clock speed for standard speed
    int clk_freq;
    clk_freq = VCOREII_AHB_CLOCK_FREQ;

    DEBUG("clk_freq = %u\n" , clk_freq);

    // Setting high clock flak to 5 us ( for standard clk (100 kHz))
    register_value = (5*clk_freq/1000000) - 8; // Datasheet Section 6.13.9.5
    VTSS_WR(VTSS_TWI_SS_SCL_HCNT,register_value);
    DEBUG("VTSS_TWI_SS_SCL_HCNTq = %u\n" ,register_value);

    // Setting low clock flak to 5 us ( for standard clk ( 100 kHz))
    register_value = (5 * clk_freq/1000000) - 1; // Datasheet Section 6.13.9.6
    VTSS_WR(VTSS_TWI_SS_SCL_LCNT,register_value);
    DEBUG("VTSS_TWI_SS_SCL_LCNT = %u\n" ,register_value);
    
    // Setting high clock flak to 1.1 us ( for fast clock (400 kHz)) ( Asym. because VTSS_TWI_FS_SCL_LCNT mustn't be below 1.3 us).
    register_value = (1.1 * clk_freq/1000000) - 8; // Datasheet Section 6.13.9.7
    VTSS_WR(VTSS_TWI_FS_SCL_HCNT,register_value);
    DEBUG("VTSS_TWI_FS_SCL_HCNT = %u\n" ,register_value);

    // Setting low clock flak to 1.4 us ( for fast clock (400 kHz)) ( Asym. because VTSS_TWI_FS_SCL_LCNT mustn't be below 1.3 us).
    register_value = (1.4 * clk_freq/1000000) - 1 ;// Datasheet Section 6.13.9.8
    VTSS_WR(VTSS_TWI_FS_SCL_LCNT,register_value);
    DEBUG("VTSS_TWI_FS_SCL_LCNT = %u\n" ,register_value);
  
    // The I2C is an overlaid function on the GPIO4 and GPIO5. Enable the overlaying by setting bits 4 & 5 to zero..
    HAL_READ_UINT32(VCOREII_SWC_REG(VCOREII_BLOCK_SYS, 0, VCOREII_SYS_GPIO),register_value);
    register_value &= ~VTSS_BIT(4); // Clear bit 4
    register_value &= ~VTSS_BIT(5); // Clear bit 5
    HAL_WRITE_UINT32(VCOREII_SWC_REG(VCOREII_BLOCK_SYS, 0, VCOREII_SYS_GPIO), register_value);


    // Set I2C clock frequency to 100 kbps.
    register_value = 0;
    register_value |= VTSS_BIT(0); // Master enable
    register_value |= VTSS_BIT(1); // Set bit 1
    register_value &= ~VTSS_BIT(2); // Clear bit 2
    register_value &= ~VTSS_BIT(3); // 7 bit mode
    register_value &= ~VTSS_BIT(4); // 7 bit mode
    register_value |= VTSS_BIT(5); // Restart enable
    register_value |= VTSS_BIT(6); // Slave disable

    VTSS_WR(VTSS_TWI_CFG, register_value);
    DEBUG("VTSS_TWI_CFG  = %u\n" ,register_value);

    // Set clock speed for to normal speed       
    register_value = (0.25 * clk_freq/1000000); // Datasheet section 6.13.9.30
    VTSS_WR(VTSS_TWI_SDA_SETUP,register_value);
    DEBUG("VTSS_TWI_SDA_SETUP  = %u\n" ,register_value);

    // Enable I2C controller
    VTSS_WR(VTSS_TWI_CTRL,VTSS_F_ENABLE);

    // Enable interrupt for when the hardware rx fifo contains data
    VTSS_WR(VTSS_TWI_INTR_MASK,0x04);
    VTSS_WR(VTSS_TWI_RX_TL,0x00); // 0x00 means one byte of data in the fifo


    cyg_drv_mutex_unlock(&i2c_lock); 
}

cyg_uint32
cyg_vcoreii_i2c_tx(const cyg_i2c_device* dev, cyg_bool send_start, const cyg_uint8* tx_data, cyg_uint32 count, cyg_bool send_stop)
{
    cyg_vcore_i2c_extra* extra = 
        (cyg_vcore_i2c_extra*)dev->i2c_bus->i2c_extra;


    DEBUG("TX LOCK 0x%X extra->rebooting = %d system_rebooting = %d\n", dev->i2c_address, extra->rebooting, system_rebooting);

    // Because the controller doesn't like to be reset while a i2c access is in progress we will stop of further 
    // accesses is the system is going to reboot. The application layer must remember to signal this 
    // by doing a dummy write when rebooting. Any I2C address can be used. 
    if (extra->rebooting) {
        system_rebooting = 1;
        DEBUG("Setting system_rebooting \n"); 
    }
    // If rebooting is in progress then simply skip the write
    if (system_rebooting) {
        return count;
    }


    DEBUG("TX LOCK 0x%X, data = 0x%X \n",dev->i2c_address,tx_data);
    while (!cyg_drv_mutex_lock(&i2c_lock)); // Mutex for avoiding mulitple accesses to the hardware
    cyg_uint32 bytes_transmitted = 0;
    
    // Setup Target address register
    vcoreii_set_tar_register(dev,send_start);
 
    //  Do the write
    int tx_byte;
    DEBUG("tx_data = ");
    if (wait_for_tx_buffer_empty(extra->i2c_wait)) {
        cyg_drv_isr_lock();  // Make sure that we a re not interrupted in order to have the tx data split.  
        for (tx_byte = count; tx_byte > 0; tx_byte--) {
            DEBUG("0x%X ",*tx_data);
            if (tx_i2c_data(*tx_data, extra->i2c_wait)  == 0) {
                // tx buffer remained full - timeout
                break;  
            } 
          
            tx_data++; // Point to next data
            bytes_transmitted++; // Ok - one more byte transmitted.
        }   
        cyg_drv_isr_unlock(); // Allow interrupt again.
    }
    DEBUG(" \n");
    DEBUG("TX UNLOCK 0x%X \n",dev->i2c_address);
    cyg_drv_mutex_unlock(&i2c_lock);
    return bytes_transmitted;
}



cyg_uint32
cyg_vcoreii_i2c_rx(const cyg_i2c_device* dev, cyg_bool send_start, cyg_uint8* rx_data, cyg_uint32 count, cyg_bool send_nack, cyg_bool send_stop)
{
    cyg_vcore_i2c_extra* extra = 
        (cyg_vcore_i2c_extra*)dev->i2c_bus->i2c_extra;



    // If rebooting is in progress then simply skip the read
    if (system_rebooting) {
        return count;
    }


    DEBUG("RX LOCK 0x%X \n",dev->i2c_address);
    while (!cyg_drv_mutex_lock(&i2c_lock)); // Mutex for avoiding mulitple accesses to the hardware
    cyg_uint32 bytes_recieved = 0;

    if (send_start) {
        // Initialize the read
        // Setup Target address register
        vcoreii_set_tar_register(dev,send_start);
    }
    
    int rx_byte;
    DEBUG("count = %d ", count);
    cyg_flag_maskbits(&interrupt_wait_flag, 0xFFFFFFFF); // Clear the interrupt flag in order to determine if an interrupt has happen.
    interrupt_wait_cnt = count;

    for (rx_byte = count; rx_byte > 0; rx_byte--) {
        if (wait_for_tx_buf_not_full(extra->i2c_wait)) {
            if (tx_i2c_data(VTSS_F_CMD, extra->i2c_wait) == 0) {
                ERROR("I2C - Couldn't transmit read request \n");
                goto rx_error;
            } 
        } else {
            ERROR("I2C Error: wait_for_tx_buf_not_full \n");
            goto rx_error;
        }
    } 
    
    // Wait until the last byte has been received
    cyg_flag_value_t flag_value = 0;
    flag_value = 0xFFFFFFFF;
    flag_value = cyg_flag_timed_wait(&interrupt_wait_flag, 
                                     flag_value, 
                                     CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR, 
                                     cyg_current_time() + CYG_I2C_MSEC2TICK(extra->i2c_wait));
    
    // Didn't received the bytes ?
    if (flag_value == 0)   {
        ERROR("No i2c interrupt \n");
        goto rx_error;
    }

    // Read out the received data
    for (rx_byte = count; rx_byte > 0; rx_byte--) {
        if (get_sw_rx_fifo_level() == 0) {
            ERROR ("I2C Error: No data in software fifo \n");
        }

        *rx_data = sw_rx_fifo_rd();
        DEBUG("0x%X ", *rx_data);
        rx_data++;
        bytes_recieved++;
    }
    DEBUG(" \n");

    DEBUG("level = %d, wr_ptr = %d, rd_ptr = %d \n",
          get_sw_rx_fifo_level(),sw_rx_fifo_wr_ptr,sw_rx_fifo_rd_ptr);
    

    DEBUG("RX UNLOCK 0x%X \n",dev->i2c_address);
    cyg_drv_mutex_unlock(&i2c_lock);
    return bytes_recieved;

rx_error:
    // Something went wrong. Start at a known state.
    sw_rx_fifo_rd_ptr = 0 ;
    sw_rx_fifo_wr_ptr = 0 ;
    vcoreiii_restart_controller(dev);
    DEBUG("RX ERROR UNLOCK 0x%X \n",dev->i2c_address);
    cyg_drv_mutex_unlock(&i2c_lock);
    return 0;

}


void
cyg_vcoreii_i2c_stop(const cyg_i2c_device* dev)
{
}

//---------------------------------------------------------------------------
// EOF i2c_vcoreii.c
