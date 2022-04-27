/*

 Vitesse Interrupt module software.

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

#include "vtss_api.h"
#include "cyg/io/i2c_vcoreii.h" // For CYG_I2C_VCOREII_DEVICE


/****************************************************************************/
/*  Global variables                                                        */
/****************************************************************************/

//
// Get board temperature (External I2C temperature chip at I2C address 0x4C)
//
int board_misc_get_chip_temp(void) {
    u8 chip_temp;
    CYG_I2C_VCOREII_DEVICE(temperature_device,0x4C);

    if (cyg_i2c_rx(&temperature_device, &chip_temp,1) == 0) {
        // Failing temperature reading
        return 255;
    } else {
        return chip_temp;
    }
}





/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
