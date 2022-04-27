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
//#####DESCRIPTIONBEGIN####
//
// Author(s):     
// Contributors:  Flemming Jahn
// Date:          2010-08-04
// Description:   I2C driver for Vitesse VCore-III
//####DESCRIPTIONEND####
//==========================================================================

#ifndef _I2C_VCOREIII_H_
#define _I2C_VCOREIII_H_

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/i2c.h>

//
// Internal VCOREIII functions
//

externC void        cyg_vcoreiii_i2c_init(struct cyg_i2c_bus*);

// These function MUST be called within a thread.
externC cyg_uint32  cyg_vcoreiii_i2c_tx(const cyg_i2c_device*, cyg_bool, const cyg_uint8*, cyg_uint32, cyg_bool);
externC cyg_uint32  cyg_vcoreiii_i2c_rx(const cyg_i2c_device*, cyg_bool, cyg_uint8*, cyg_uint32, cyg_bool, cyg_bool);
externC void        cyg_vcoreiii_i2c_stop(const cyg_i2c_device*);

/*
 * Bus flags
 */
#define I2C_FLAG_INIT   VTSS_BIT(1) /* Init done */
#define I2C_FLAG_REBOOT VTSS_BIT(0) /* Reboot in progress */

typedef struct {
    const cyg_uint8 *transmit_pointer;
    cyg_uint32 transmit_count;

    cyg_uint8 *read_pointer;
    cyg_uint32 read_unissued;
    cyg_uint32 read_pending;

    cyg_uint32 last_stat;
    cyg_uint32 abort_stat;
} cyg_vcore_i2c_req;

typedef struct {
    cyg_uint8            i2c_wait; /* Maximum time to wait for the i2c device to respond */
    cyg_int8             i2c_clk_sel; /* The i2c clk selector (GPIO pin) (Set to -1 to default pin) */
    cyg_uint32           i2c_flags;
    cyg_flag_t           wait_flag;
    cyg_vcore_i2c_req    req;      /* Current bus request state */
} cyg_vcore_i2c_extra;

//
// Functions/macros for the host usage.
//

// Macro for initialising an I2C device. 
#define CYG_I2C_VCORE_DEVICE(_name_,_address_)                          \
    CYG_I2C_DEVICE(_name_,                                              \
                   &hal_vcore_i2c_bus,                                  \
                   _address_,                                           \
                   0x0,                                                 \
                   CYG_I2C_DEFAULT_DELAY)


/* Uniform interface */
#define CYG_I2C_VCOREIII_DEVICE CYG_I2C_VCORE_DEVICE

#endif /* _I2C_VCOREIII_H_ */
