#ifndef _VTSS_SERVAL_REGS_DEVCPU_ORG_H_
#define _VTSS_SERVAL_REGS_DEVCPU_ORG_H_

/*
 *
 * VCore-III Register Definitions
 *
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

#include "vtss_serval_regs_common.h"

#define VTSS_DEVCPU_ORG_ORG_IF_CTRL          VTSS_IOREG(VTSS_TO_DEVCPU_ORG,0x0)
#define  VTSS_F_DEVCPU_ORG_ORG_IF_CTRL_IF_CTRL(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_DEVCPU_ORG_ORG_IF_CTRL_IF_CTRL     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_DEVCPU_ORG_ORG_IF_CTRL_IF_CTRL(x)  VTSS_EXTRACT_BITFIELD(x,0,4)

#define VTSS_DEVCPU_ORG_ORG_IF_CFGSTAT       VTSS_IOREG(VTSS_TO_DEVCPU_ORG,0x1)
#define  VTSS_F_DEVCPU_ORG_ORG_IF_CFGSTAT_IF_STAT  VTSS_BIT(16)
#define  VTSS_F_DEVCPU_ORG_ORG_IF_CFGSTAT_IF_CFG(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_DEVCPU_ORG_ORG_IF_CFGSTAT_IF_CFG     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_DEVCPU_ORG_ORG_IF_CFGSTAT_IF_CFG(x)  VTSS_EXTRACT_BITFIELD(x,0,4)

#define VTSS_DEVCPU_ORG_ORG_ORG_CFG          VTSS_IOREG(VTSS_TO_DEVCPU_ORG,0x2)
#define  VTSS_F_DEVCPU_ORG_ORG_ORG_CFG_FAST_WR  VTSS_BIT(0)

#define VTSS_DEVCPU_ORG_ORG_ERR_CNTS         VTSS_IOREG(VTSS_TO_DEVCPU_ORG,0x3)
#define  VTSS_F_DEVCPU_ORG_ORG_ERR_CNTS_ERR_TGT_BUSY(x)  VTSS_ENCODE_BITFIELD(x,16,4)
#define  VTSS_M_DEVCPU_ORG_ORG_ERR_CNTS_ERR_TGT_BUSY     VTSS_ENCODE_BITMASK(16,4)
#define  VTSS_X_DEVCPU_ORG_ORG_ERR_CNTS_ERR_TGT_BUSY(x)  VTSS_EXTRACT_BITFIELD(x,16,4)
#define  VTSS_F_DEVCPU_ORG_ORG_ERR_CNTS_ERR_WD_DROP_ORG(x)  VTSS_ENCODE_BITFIELD(x,12,4)
#define  VTSS_M_DEVCPU_ORG_ORG_ERR_CNTS_ERR_WD_DROP_ORG     VTSS_ENCODE_BITMASK(12,4)
#define  VTSS_X_DEVCPU_ORG_ORG_ERR_CNTS_ERR_WD_DROP_ORG(x)  VTSS_EXTRACT_BITFIELD(x,12,4)
#define  VTSS_F_DEVCPU_ORG_ORG_ERR_CNTS_ERR_WD_DROP(x)  VTSS_ENCODE_BITFIELD(x,8,4)
#define  VTSS_M_DEVCPU_ORG_ORG_ERR_CNTS_ERR_WD_DROP     VTSS_ENCODE_BITMASK(8,4)
#define  VTSS_X_DEVCPU_ORG_ORG_ERR_CNTS_ERR_WD_DROP(x)  VTSS_EXTRACT_BITFIELD(x,8,4)
#define  VTSS_F_DEVCPU_ORG_ORG_ERR_CNTS_ERR_NO_ACTION(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_DEVCPU_ORG_ORG_ERR_CNTS_ERR_NO_ACTION     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_DEVCPU_ORG_ORG_ERR_CNTS_ERR_NO_ACTION(x)  VTSS_EXTRACT_BITFIELD(x,4,4)
#define  VTSS_F_DEVCPU_ORG_ORG_ERR_CNTS_ERR_UTM(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_DEVCPU_ORG_ORG_ERR_CNTS_ERR_UTM     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_DEVCPU_ORG_ORG_ERR_CNTS_ERR_UTM(x)  VTSS_EXTRACT_BITFIELD(x,0,4)

#define VTSS_DEVCPU_ORG_ORG_TIMEOUT_CFG      VTSS_IOREG(VTSS_TO_DEVCPU_ORG,0x4)
#define  VTSS_F_DEVCPU_ORG_ORG_TIMEOUT_CFG_TIMEOUT_CFG(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_DEVCPU_ORG_ORG_TIMEOUT_CFG_TIMEOUT_CFG     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_DEVCPU_ORG_ORG_TIMEOUT_CFG_TIMEOUT_CFG(x)  VTSS_EXTRACT_BITFIELD(x,0,12)

#define VTSS_DEVCPU_ORG_ORG_GPR              VTSS_IOREG(VTSS_TO_DEVCPU_ORG,0x5)

#define VTSS_DEVCPU_ORG_ORG_MAILBOX_SET      VTSS_IOREG(VTSS_TO_DEVCPU_ORG,0x6)

#define VTSS_DEVCPU_ORG_ORG_MAILBOX_CLR      VTSS_IOREG(VTSS_TO_DEVCPU_ORG,0x7)

#define VTSS_DEVCPU_ORG_ORG_MAILBOX          VTSS_IOREG(VTSS_TO_DEVCPU_ORG,0x8)

#define VTSS_DEVCPU_ORG_ORG_SEMA_CFG         VTSS_IOREG(VTSS_TO_DEVCPU_ORG,0x9)
#define  VTSS_F_DEVCPU_ORG_ORG_SEMA_CFG_SEMA_INTR_POL(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_DEVCPU_ORG_ORG_SEMA_CFG_SEMA_INTR_POL     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_DEVCPU_ORG_ORG_SEMA_CFG_SEMA_INTR_POL(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

#define VTSS_DEVCPU_ORG_ORG_SEMA0            VTSS_IOREG(VTSS_TO_DEVCPU_ORG,0xa)
#define  VTSS_F_DEVCPU_ORG_ORG_SEMA0_SEMA0    VTSS_BIT(0)

#define VTSS_DEVCPU_ORG_ORG_SEMA0_OWNER      VTSS_IOREG(VTSS_TO_DEVCPU_ORG,0xb)

#define VTSS_DEVCPU_ORG_ORG_SEMA1            VTSS_IOREG(VTSS_TO_DEVCPU_ORG,0xc)
#define  VTSS_F_DEVCPU_ORG_ORG_SEMA1_SEMA1    VTSS_BIT(0)

#define VTSS_DEVCPU_ORG_ORG_SEMA1_OWNER      VTSS_IOREG(VTSS_TO_DEVCPU_ORG,0xd)


#endif /* _VTSS_SERVAL_REGS_DEVCPU_ORG_H_ */
