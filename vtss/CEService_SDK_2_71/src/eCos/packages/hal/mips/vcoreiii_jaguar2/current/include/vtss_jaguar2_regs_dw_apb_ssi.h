#ifndef _VTSS_JAGUAR2_REGS_DW_APB_SSI_H_
#define _VTSS_JAGUAR2_REGS_DW_APB_SSI_H_

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

#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0  VTSS_IOREG(VTSS_TO_SSI,0x0)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_CFS(x)  VTSS_ENCODE_BITFIELD(x,12,4)
#define  VTSS_M_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_CFS     VTSS_ENCODE_BITMASK(12,4)
#define  VTSS_X_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_CFS(x)  VTSS_EXTRACT_BITFIELD(x,12,4)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_SRL  VTSS_BIT(11)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_TMOD(x)  VTSS_ENCODE_BITFIELD(x,8,2)
#define  VTSS_M_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_TMOD     VTSS_ENCODE_BITMASK(8,2)
#define  VTSS_X_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_TMOD(x)  VTSS_EXTRACT_BITFIELD(x,8,2)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_SCPOL  VTSS_BIT(7)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_SCPH  VTSS_BIT(6)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_FRF(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_FRF     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_FRF(x)  VTSS_EXTRACT_BITFIELD(x,4,2)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_DFS(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_DFS     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_DFS(x)  VTSS_EXTRACT_BITFIELD(x,0,4)

#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR1  VTSS_IOREG(VTSS_TO_SSI,0x1)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR1_NDF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR1_NDF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR1_NDF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_SSIENR  VTSS_IOREG(VTSS_TO_SSI,0x2)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_SSIENR_SSI_EN  VTSS_BIT(0)

#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_MWCR  VTSS_IOREG(VTSS_TO_SSI,0x3)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_MWCR_MHS  VTSS_BIT(2)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_MWCR_MDD  VTSS_BIT(1)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_MWCR_MWMOD  VTSS_BIT(0)

#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_SER  VTSS_IOREG(VTSS_TO_SSI,0x4)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_SER_SER(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DW_APB_SSI_ssi_memory_map_ssi_address_block_SER_SER     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DW_APB_SSI_ssi_memory_map_ssi_address_block_SER_SER(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_BAUDR  VTSS_IOREG(VTSS_TO_SSI,0x5)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_BAUDR_SCKDV(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DW_APB_SSI_ssi_memory_map_ssi_address_block_BAUDR_SCKDV     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DW_APB_SSI_ssi_memory_map_ssi_address_block_BAUDR_SCKDV(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_TXFTLR  VTSS_IOREG(VTSS_TO_SSI,0x6)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_TXFTLR_TFT(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_DW_APB_SSI_ssi_memory_map_ssi_address_block_TXFTLR_TFT     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_DW_APB_SSI_ssi_memory_map_ssi_address_block_TXFTLR_TFT(x)  VTSS_EXTRACT_BITFIELD(x,0,3)

#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXFTLR  VTSS_IOREG(VTSS_TO_SSI,0x7)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXFTLR_RFT(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXFTLR_RFT     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXFTLR_RFT(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_TXFLR  VTSS_IOREG(VTSS_TO_SSI,0x8)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_TXFLR_TXTFL(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_DW_APB_SSI_ssi_memory_map_ssi_address_block_TXFLR_TXTFL     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_DW_APB_SSI_ssi_memory_map_ssi_address_block_TXFLR_TXTFL(x)  VTSS_EXTRACT_BITFIELD(x,0,3)

#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXFLR  VTSS_IOREG(VTSS_TO_SSI,0x9)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXFLR_RXTFL(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXFLR_RXTFL     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXFLR_RXTFL(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_IMR  VTSS_IOREG(VTSS_TO_SSI,0xb)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_IMR_MSTIM  VTSS_BIT(5)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_IMR_RXFIM  VTSS_BIT(4)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_IMR_RXOIM  VTSS_BIT(3)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_IMR_RXUIM  VTSS_BIT(2)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_IMR_TXOIM  VTSS_BIT(1)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_IMR_TXEIM  VTSS_BIT(0)

#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_ISR  VTSS_IOREG(VTSS_TO_SSI,0xc)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_ISR_MSTIS  VTSS_BIT(5)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_ISR_RXFIS  VTSS_BIT(4)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_ISR_RXOIS  VTSS_BIT(3)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_ISR_RXUIS  VTSS_BIT(2)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_ISR_TXOIS  VTSS_BIT(1)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_ISR_TXEIS  VTSS_BIT(0)

#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_RISR  VTSS_IOREG(VTSS_TO_SSI,0xd)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_RISR_MSTIR  VTSS_BIT(5)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_RISR_RXFIR  VTSS_BIT(4)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_RISR_RXOIR  VTSS_BIT(3)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_RISR_RXUIR  VTSS_BIT(2)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_RISR_TXOIR  VTSS_BIT(1)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_RISR_TXEIR  VTSS_BIT(0)

#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_TXOICR  VTSS_IOREG(VTSS_TO_SSI,0xe)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_TXOICR_TXOICR  VTSS_BIT(0)

#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXOICR  VTSS_IOREG(VTSS_TO_SSI,0xf)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXOICR_RXOICR  VTSS_BIT(0)

#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXUICR  VTSS_IOREG(VTSS_TO_SSI,0x10)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXUICR_RXUICR  VTSS_BIT(0)

#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_MSTICR  VTSS_IOREG(VTSS_TO_SSI,0x11)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_MSTICR_MSTICR  VTSS_BIT(0)

#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_ICR  VTSS_IOREG(VTSS_TO_SSI,0x12)
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_ICR_ICR  VTSS_BIT(0)

#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_DR(ri)  VTSS_IOREG(VTSS_TO_SSI,0x18 + (ri))
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_DR_dr(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DW_APB_SSI_ssi_memory_map_ssi_address_block_DR_dr     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DW_APB_SSI_ssi_memory_map_ssi_address_block_DR_dr(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


#endif /* _VTSS_JAGUAR2_REGS_DW_APB_SSI_H_ */
