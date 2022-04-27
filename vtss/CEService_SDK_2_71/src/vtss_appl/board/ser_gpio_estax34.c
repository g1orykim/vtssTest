/*

 Vitesse Switch API software.

 Copyright (c) 2002-2010 Vitesse Semiconductor Corporation "Vitesse". All
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


#include <cyg/hal/hal_io.h>
#include "ser_gpio.h"

// #include "poe.h" // Can be used for debuging by sharing the trance system
uint last_ser_data;


// Function for writting the serialized GPIOs. See section 2.6.3 in RBM0027
// This function remembers the last data writing, so it is possible for multiple functions
// to use the function. Each function calling must set the bits it want to change in the mask vector.
void serialized_gpio_data_wr (uint ser_data_in,uint mask_vector) {
    
    // Do only change bits that are masked in via the mask vector.
    uint new_data = ser_data_in & mask_vector;
    uint old_data = last_ser_data & (~mask_vector);
    last_ser_data = new_data | old_data;
    
    
   // The following piece of code must be protected being access by mulitple function, because it's a read-modify-write operation.
    // Therefore, we protect it by a scheduler lock.
    cyg_scheduler_lock();
    uint register_value;
    HAL_READ_UINT32(VCOREII_SWC_REG(VCOREII_BLOCK_SYS, 0, VCOREII_SYS_GPIO),register_value);
    register_value |= VTSS_BIT(3); // Set bit 3 ( data )
    register_value |= VTSS_BIT(2); // Set bit 2 ( clk )
    register_value |= VTSS_BIT(7); // Set bit 3 ( latch )
    HAL_WRITE_UINT32(VCOREII_SWC_REG(VCOREII_BLOCK_SYS, 0, VCOREII_SYS_GPIO), register_value);
//    T_D("Init of GPIO CTRL = 0x%X",register_value); 
    
    HAL_READ_UINT32(VCOREII_SWC_REG(VCOREII_BLOCK_SYS, 0, VCOREII_SYS_GPIO_DATA),register_value);
    register_value |= VTSS_BIT(19); // Set bit data as output
    register_value |= VTSS_BIT(18); // Set bit clk as output
    register_value |= VTSS_BIT(23); // Set bit lacth as output
    
    int bit_index ;
    register_value &= ~VTSS_BIT(7); // Set lacth to 0
    for (bit_index = 15 ; bit_index >= 0 ; bit_index--) {
        register_value &= ~VTSS_BIT(2); // Set clk to 0
        if ((last_ser_data >> bit_index) & 0x1) {
            register_value |= VTSS_BIT(3); // Set data to 1
        } else {
            register_value &= ~VTSS_BIT(3); // Set data to 0
        }
        
        HAL_WRITE_UINT32(VCOREII_SWC_REG(VCOREII_BLOCK_SYS, 0, VCOREII_SYS_GPIO_DATA), register_value);
        
//        T_D("Write data bit %d,  DATA = 0x%X",bit_index,register_value); 
        
        register_value |= VTSS_BIT(2); // Set clock to 1
        HAL_WRITE_UINT32(VCOREII_SWC_REG(VCOREII_BLOCK_SYS, 0, VCOREII_SYS_GPIO_DATA), register_value);
//        T_D("Toggle Clk - 0x%X",register_value); 
    }
    register_value &= ~VTSS_BIT(2); // Set clk to 0
    register_value |= VTSS_BIT(7); // Set latch to 1
    HAL_WRITE_UINT32(VCOREII_SWC_REG(VCOREII_BLOCK_SYS, 0, VCOREII_SYS_GPIO_DATA), register_value);
    register_value &= ~VTSS_BIT(7); // Set latch to 0
    HAL_WRITE_UINT32(VCOREII_SWC_REG(VCOREII_BLOCK_SYS, 0, VCOREII_SYS_GPIO_DATA), register_value);
    register_value |= VTSS_BIT(7); // Set latch to 1
    HAL_WRITE_UINT32(VCOREII_SWC_REG(VCOREII_BLOCK_SYS, 0, VCOREII_SYS_GPIO_DATA), register_value);
    cyg_scheduler_unlock();   
}


// Function for reading the serialized GPIOs. See section 2.6.3 in TNxxx
uint serialized_gpio_data_rd (void) {
    uint ser_data_in = 0;
    int register_value_out = 0;
    int register_value_in;
    char bit_val;
    int bit_index;

    serialized_gpio_data_wr(0,0); // For making a read, we have to do a write. This write will simply write the last writing value again.

    // The following piece of code must be protected being access by mulitple function, because it's a read-modify-write operation.
    // Therefore, we protect it by a scheduler lock.
    cyg_scheduler_lock();
     
    HAL_READ_UINT32(VCOREII_SWC_REG(VCOREII_BLOCK_SYS, 0, VCOREII_SYS_GPIO_DATA),register_value_out);
    register_value_out &= ~VTSS_BIT(19); // Set bit data as input
    register_value_out |= VTSS_BIT(18); // Set bit clk as output
    register_value_out |= VTSS_BIT(23); // Set bit lacth as output
    register_value_out &= ~VTSS_BIT(2); // Set clk to 0
    register_value_out |= VTSS_BIT(7); // Set latch to 1
    HAL_WRITE_UINT32(VCOREII_SWC_REG(VCOREII_BLOCK_SYS, 0, VCOREII_SYS_GPIO_DATA), register_value_out);

    for (bit_index = 0 ; bit_index < 26000 ; bit_index++) {
    }
    
    for (bit_index = 0 ; bit_index < 16 ; bit_index++) {
        HAL_READ_UINT32(VCOREII_SWC_REG(VCOREII_BLOCK_SYS, 0, VCOREII_SYS_GPIO_DATA),register_value_in);
        bit_val = (register_value_in >> 3) & 0x1 ; // See TNxxx, section 2.6.3
        ser_data_in += bit_val << (15 - bit_index);  
//        T_D("ser data = 0x%X, register_value_in = 0x%X, bit_val = %d,bit_index = %d",ser_data_in,register_value_in,bit_val,bit_index); 
        
        register_value_out |= VTSS_BIT(2); // Set clock to 1
        HAL_WRITE_UINT32(VCOREII_SWC_REG(VCOREII_BLOCK_SYS, 0, VCOREII_SYS_GPIO_DATA), register_value_out);
        register_value_out &= ~VTSS_BIT(2); // Set clk to 0
        HAL_WRITE_UINT32(VCOREII_SWC_REG(VCOREII_BLOCK_SYS, 0, VCOREII_SYS_GPIO_DATA), register_value_out);
    }
    cyg_scheduler_unlock();
    return ser_data_in;

}
        
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
