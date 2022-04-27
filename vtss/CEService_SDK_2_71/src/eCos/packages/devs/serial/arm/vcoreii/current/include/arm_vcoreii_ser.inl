//==========================================================================
//
//      io/serial/arm/arm_vcoreii_ser.inl
//
//      VCore-II Serial I/O definitions
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    lpovlsen
// Contributors: gthomas, jlarmour
// Date:         2006-06-01
// Purpose:      VCore-II Serial I/O module (interrupt driven version)
// Description: 
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/hal_intr.h>
#include <cyg/hal/vcoreii.h>

//-----------------------------------------------------------------------------
// Baud rate specification

#define BAUD_DIVISOR(_x_) (VCOREII_AHB_CLOCK_FREQ/16/_x_)

static unsigned short select_baud[] = {
    0,    // Unused
    0,    // 50
    0,    // 75
    0,    // 110
    0,    // 134.5
    0,    // 150
    0,    // 200
    0,    // 300
    0,    // 600
    BAUD_DIVISOR(1200),
    0,    // 1800
    BAUD_DIVISOR(2400),
    0,    // 3600
    BAUD_DIVISOR(4800),
    0 ,   // 7200
    BAUD_DIVISOR(9600),
    BAUD_DIVISOR(14400),
    BAUD_DIVISOR(19200),
    BAUD_DIVISOR(38400),
    BAUD_DIVISOR(57600),
    BAUD_DIVISOR(115200),
    BAUD_DIVISOR(230400),
};

#ifdef CYGPKG_IO_SERIAL_ARM_VCOREII_SERIAL1
static pc_serial_info vcoreii_serial_info1 = {VTSS_TB_UART, CYGNUM_HAL_INT_UART};
#if CYGNUM_IO_SERIAL_ARM_VCOREII_SERIAL1_BUFSIZE > 0
static unsigned char vcoreii_serial_out_buf1[CYGNUM_IO_SERIAL_ARM_VCOREII_SERIAL1_BUFSIZE];
static unsigned char vcoreii_serial_in_buf1[CYGNUM_IO_SERIAL_ARM_VCOREII_SERIAL1_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(vcoreii_serial_channel1,
                                       pc_serial_funs, 
                                       vcoreii_serial_info1,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_VCOREII_SERIAL1_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &vcoreii_serial_out_buf1[0], sizeof(vcoreii_serial_out_buf1),
                                       &vcoreii_serial_in_buf1[0], sizeof(vcoreii_serial_in_buf1)
    );
#else
static SERIAL_CHANNEL(vcoreii_serial_channel1,
                      pc_serial_funs, 
                      vcoreii_serial_info1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_VCOREII_SERIAL1_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif

DEVTAB_ENTRY(vcoreii_serial_io1, 
             CYGDAT_IO_SERIAL_ARM_VCOREII_SERIAL1_NAME,
             0,                     // Does not depend on a lower level interface
             &cyg_io_serial_devio, 
             pc_serial_init, 
             pc_serial_lookup,     // Serial driver may need initializing
             &vcoreii_serial_channel1
    );

// In designs using Synopsys's DesignWare APB UART v.3.04a or earlies, or
// v.3.05a or later with UART_16550_COMPATIBLE set to 'No', the UART's
// busy-functionality may cause the UART to hang when programming new
// divisors while receiving a character. The following override of the
// SER_16X5X_WRITE_LCR macro fixes this problem.
#define REG_usr SER_REG(0x1F) /* UART status register */
#define SER_16X5X_WRITE_LCR(_base_, _val_)             \
  do {                                                 \
    while(1) {                                         \
      unsigned char _lcr_rd, _usr;                     \
      HAL_WRITE_UINT8(_base_ + REG_lcr, _val_);        \
      HAL_READ_UINT8(_base_ + REG_lcr, _lcr_rd);       \
      if((_lcr_rd & LCR_DL) == ((_val_) & LCR_DL))     \
        break;                                         \
      /* Read the USR to clear any busy interrupts */  \
      HAL_READ_UINT8(_base_ + REG_usr, _usr);          \
    }                                                  \
  } while(0);

#define ISR_BUSY 0x07 /* Busy mask for the Synopsys designware UART */
#define SER_16X5X_READ_ISR(_base_, _val_)                      \
  do {                                                         \
    cyg_uint8 _usr;                                            \
    /* First get the ISR value into _val_                   */ \
    HAL_READ_UINT8(_base_ + REG_isr, (_val_));                 \
                                                               \
    if((_val_) & ISR_BUSY) {                                   \
      /* This must be checked before this macro returns,    */ \
      /* because the LSBit is set in the ISR_BUSY mask.     */ \
      /* Read uart status register to clear this interrupt, */ \
      /* which may occur if writing to the LCR register     */ \
      /* while the UART is busy.                            */ \
      HAL_READ_UINT8(_base_ + REG_usr, _usr);                  \
      HAL_READ_UINT8(_base_ + REG_isr, (_val_));               \
    }                                                          \
  } while(0);

#endif //  CYGPKG_IO_SERIAL_ARM_VCOREII_SERIAL1

// EOF arm_vcoreii_ser.inl
