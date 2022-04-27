#ifndef _VTSS_SERVAL_REGS_SBA_H_
#define _VTSS_SERVAL_REGS_SBA_H_

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

#define VTSS_SBA_SBA_PL_CPU                  VTSS_IOREG(VTSS_TO_SBA,0x0)
#define  VTSS_F_SBA_SBA_PL_CPU_PL1(x)         VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_SBA_SBA_PL_CPU_PL1            VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_SBA_SBA_PL_CPU_PL1(x)         VTSS_EXTRACT_BITFIELD(x,0,4)

#define VTSS_SBA_SBA_PL_PCIE                 VTSS_IOREG(VTSS_TO_SBA,0x1)
#define  VTSS_F_SBA_SBA_PL_PCIE_PL2(x)        VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_SBA_SBA_PL_PCIE_PL2           VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_SBA_SBA_PL_PCIE_PL2(x)        VTSS_EXTRACT_BITFIELD(x,0,4)

#define VTSS_SBA_SBA_PL_CSR                  VTSS_IOREG(VTSS_TO_SBA,0x2)
#define  VTSS_F_SBA_SBA_PL_CSR_PL3(x)         VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_SBA_SBA_PL_CSR_PL3            VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_SBA_SBA_PL_CSR_PL3(x)         VTSS_EXTRACT_BITFIELD(x,0,4)

#define VTSS_SBA_SBA_WT_EN                   VTSS_IOREG(VTSS_TO_SBA,0x13)
#define  VTSS_F_SBA_SBA_WT_EN_WT_EN           VTSS_BIT(0)

#define VTSS_SBA_SBA_WT_TCL                  VTSS_IOREG(VTSS_TO_SBA,0x14)
#define  VTSS_F_SBA_SBA_WT_TCL_WT_TCL(x)      VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_SBA_SBA_WT_TCL_WT_TCL         VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_SBA_SBA_WT_TCL_WT_TCL(x)      VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_SBA_SBA_WT_CPU                  VTSS_IOREG(VTSS_TO_SBA,0x15)
#define  VTSS_F_SBA_SBA_WT_CPU_WT_CL1(x)      VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_SBA_SBA_WT_CPU_WT_CL1         VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_SBA_SBA_WT_CPU_WT_CL1(x)      VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_SBA_SBA_WT_PCIE                 VTSS_IOREG(VTSS_TO_SBA,0x16)
#define  VTSS_F_SBA_SBA_WT_PCIE_WT_CL2(x)     VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_SBA_SBA_WT_PCIE_WT_CL2        VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_SBA_SBA_WT_PCIE_WT_CL2(x)     VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_SBA_SBA_WT_CSR                  VTSS_IOREG(VTSS_TO_SBA,0x17)
#define  VTSS_F_SBA_SBA_WT_CSR_WT_CL3(x)      VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_SBA_SBA_WT_CSR_WT_CL3         VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_SBA_SBA_WT_CSR_WT_CL3(x)      VTSS_EXTRACT_BITFIELD(x,0,16)


#endif /* _VTSS_SERVAL_REGS_SBA_H_ */
