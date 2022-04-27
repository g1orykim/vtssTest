#ifndef _VTSS_SERVAL_REGS_PCIE_H_
#define _VTSS_SERVAL_REGS_PCIE_H_

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

#define VTSS_PCIE_PCIE_CONF_SPACE(ri)        VTSS_IOREG(VTSS_TO_PCIE_EP,0x0 + (ri))

#define VTSS_PCIE_PCIE_ATU_REGION            VTSS_IOREG(VTSS_TO_PCIE_EP,0x240)
#define  VTSS_F_PCIE_PCIE_ATU_REGION_ATU_IDX  VTSS_BIT(0)

#define VTSS_PCIE_PCIE_ATU_CFG1              VTSS_IOREG(VTSS_TO_PCIE_EP,0x241)
#define  VTSS_F_PCIE_PCIE_ATU_CFG1_ATU_ATTR(x)  VTSS_ENCODE_BITFIELD(x,9,2)
#define  VTSS_M_PCIE_PCIE_ATU_CFG1_ATU_ATTR     VTSS_ENCODE_BITMASK(9,2)
#define  VTSS_X_PCIE_PCIE_ATU_CFG1_ATU_ATTR(x)  VTSS_EXTRACT_BITFIELD(x,9,2)
#define  VTSS_F_PCIE_PCIE_ATU_CFG1_ATU_TD     VTSS_BIT(8)
#define  VTSS_F_PCIE_PCIE_ATU_CFG1_ATU_TC(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_PCIE_PCIE_ATU_CFG1_ATU_TC     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_PCIE_PCIE_ATU_CFG1_ATU_TC(x)  VTSS_EXTRACT_BITFIELD(x,5,3)
#define  VTSS_F_PCIE_PCIE_ATU_CFG1_ATU_TYPE(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_PCIE_PCIE_ATU_CFG1_ATU_TYPE     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_PCIE_PCIE_ATU_CFG1_ATU_TYPE(x)  VTSS_EXTRACT_BITFIELD(x,0,5)

#define VTSS_PCIE_PCIE_ATU_CFG2              VTSS_IOREG(VTSS_TO_PCIE_EP,0x242)
#define  VTSS_F_PCIE_PCIE_ATU_CFG2_ATU_REGION_ENA  VTSS_BIT(31)
#define  VTSS_F_PCIE_PCIE_ATU_CFG2_ATU_MSG_CODE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_PCIE_PCIE_ATU_CFG2_ATU_MSG_CODE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_PCIE_PCIE_ATU_CFG2_ATU_MSG_CODE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

#define VTSS_PCIE_PCIE_ATU_BASE_ADDR_LOW     VTSS_IOREG(VTSS_TO_PCIE_EP,0x243)
#define  VTSS_F_PCIE_PCIE_ATU_BASE_ADDR_LOW_ATU_BASE_ADDR_LOW(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_PCIE_PCIE_ATU_BASE_ADDR_LOW_ATU_BASE_ADDR_LOW     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_PCIE_PCIE_ATU_BASE_ADDR_LOW_ATU_BASE_ADDR_LOW(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

#define VTSS_PCIE_PCIE_ATU_BASE_ADDR_HIGH    VTSS_IOREG(VTSS_TO_PCIE_EP,0x244)

#define VTSS_PCIE_PCIE_ATU_LIMIT_ADDR        VTSS_IOREG(VTSS_TO_PCIE_EP,0x245)
#define  VTSS_F_PCIE_PCIE_ATU_LIMIT_ADDR_ATU_LIMIT_ADDR(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_PCIE_PCIE_ATU_LIMIT_ADDR_ATU_LIMIT_ADDR     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_PCIE_PCIE_ATU_LIMIT_ADDR_ATU_LIMIT_ADDR(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

#define VTSS_PCIE_PCIE_ATU_TGT_ADDR_LOW      VTSS_IOREG(VTSS_TO_PCIE_EP,0x246)
#define  VTSS_F_PCIE_PCIE_ATU_TGT_ADDR_LOW_ATU_TGT_ADDR_LOW(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_PCIE_PCIE_ATU_TGT_ADDR_LOW_ATU_TGT_ADDR_LOW     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_PCIE_PCIE_ATU_TGT_ADDR_LOW_ATU_TGT_ADDR_LOW(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

#define VTSS_PCIE_PCIE_ATU_TGT_ADDR_HIGH     VTSS_IOREG(VTSS_TO_PCIE_EP,0x247)


#endif /* _VTSS_SERVAL_REGS_PCIE_H_ */
