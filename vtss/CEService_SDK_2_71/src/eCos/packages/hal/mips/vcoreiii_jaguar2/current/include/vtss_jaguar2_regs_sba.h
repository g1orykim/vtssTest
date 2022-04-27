#ifndef _VTSS_JAGUAR2_REGS_SBA_H_
#define _VTSS_JAGUAR2_REGS_SBA_H_

/*
 *
 * VCore-III Register Definitions
 *
 * Copyright (C) 2012 Vitesse Semiconductor Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "vtss_jaguar2_regs_common.h"

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

#define VTSS_SBA_SBA_RESERVED1(ri)           VTSS_IOREG(VTSS_TO_SBA,0x3 + (ri))

#define VTSS_SBA_SBA_EBT_COUNT               VTSS_IOREG(VTSS_TO_SBA,0xf)
#define  VTSS_F_SBA_SBA_EBT_COUNT_EBT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,10)
#define  VTSS_M_SBA_SBA_EBT_COUNT_EBT_COUNT     VTSS_ENCODE_BITMASK(0,10)
#define  VTSS_X_SBA_SBA_EBT_COUNT_EBT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,10)

#define VTSS_SBA_SBA_EBT_EN                  VTSS_IOREG(VTSS_TO_SBA,0x10)
#define  VTSS_F_SBA_SBA_EBT_EN_EBT_EN         VTSS_BIT(0)

#define VTSS_SBA_SBA_EBT                     VTSS_IOREG(VTSS_TO_SBA,0x11)
#define  VTSS_F_SBA_SBA_EBT_EBT               VTSS_BIT(0)

#define VTSS_SBA_SBA_DFT_MST                 VTSS_IOREG(VTSS_TO_SBA,0x12)
#define  VTSS_F_SBA_SBA_DFT_MST_DFT_MST(x)    VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_SBA_SBA_DFT_MST_DFT_MST       VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_SBA_SBA_DFT_MST_DFT_MST(x)    VTSS_EXTRACT_BITFIELD(x,0,4)

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


#endif /* _VTSS_JAGUAR2_REGS_SBA_H_ */
