/*

 Vitesse Switch API software.

 Copyright (c) 2002-2008 Vitesse Semiconductor Corporation "Vitesse". All
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


// Becuase we have the need for some addtional GPIO for the Luton28 PCB076 boards,
// a seriliazed GPIO has been added to the board.These seialized GPIOs are described in TNxxxx
// section 2.6.3. The ser_gpio.c contains 2 functions for acccessing the serialized GPIOs.



#ifndef _VTSS_SER_GPIO_H_
#define _VTSS_SER_GPIO_H__

#include "main.h"

// Function for reading the serialized GPIOs.
uint serialized_gpio_data_rd (void);

// Function for writting to the serialized GPIOs ( 16 bits )
void serialized_gpio_data_wr (uint ser_data_in,uint mask_vector);

#endif /* _VTSS_SER_GPIO_H__ */
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
