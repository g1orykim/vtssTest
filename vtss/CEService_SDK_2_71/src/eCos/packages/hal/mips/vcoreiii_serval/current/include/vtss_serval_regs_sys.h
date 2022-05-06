#ifndef _VTSS_SERVAL_REGS_SYS_H_
#define _VTSS_SERVAL_REGS_SYS_H_

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

#define VTSS_SYS_SYSTEM_RESET_CFG            VTSS_IOREG(VTSS_TO_SYS,0x146)
#define  VTSS_F_SYS_SYSTEM_RESET_CFG_CORE_ENA  VTSS_BIT(7)
#define  VTSS_F_SYS_SYSTEM_RESET_CFG_MEM_ENA  VTSS_BIT(6)
#define  VTSS_F_SYS_SYSTEM_RESET_CFG_MEM_INIT  VTSS_BIT(5)
#define  VTSS_F_SYS_SYSTEM_RESET_CFG_ENCAP_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_SYS_SYSTEM_RESET_CFG_ENCAP_CNT     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_SYS_SYSTEM_RESET_CFG_ENCAP_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,5)

#define VTSS_SYS_SYSTEM_VLAN_ETYPE_CFG       VTSS_IOREG(VTSS_TO_SYS,0x148)
#define  VTSS_F_SYS_SYSTEM_VLAN_ETYPE_CFG_VLAN_S_TAG_ETYPE_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_SYS_SYSTEM_VLAN_ETYPE_CFG_VLAN_S_TAG_ETYPE_VAL     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_SYS_SYSTEM_VLAN_ETYPE_CFG_VLAN_S_TAG_ETYPE_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_SYS_SYSTEM_PORT_MODE(ri)        VTSS_IOREG(VTSS_TO_SYS,0x149 + (ri))
#define  VTSS_F_SYS_SYSTEM_PORT_MODE_INCL_INJ_HDR(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_SYS_SYSTEM_PORT_MODE_INCL_INJ_HDR     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_SYS_SYSTEM_PORT_MODE_INCL_INJ_HDR(x)  VTSS_EXTRACT_BITFIELD(x,4,2)
#define  VTSS_F_SYS_SYSTEM_PORT_MODE_INCL_XTR_HDR(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_SYS_SYSTEM_PORT_MODE_INCL_XTR_HDR     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_SYS_SYSTEM_PORT_MODE_INCL_XTR_HDR(x)  VTSS_EXTRACT_BITFIELD(x,2,2)
#define  VTSS_F_SYS_SYSTEM_PORT_MODE_MPLS_ENA  VTSS_BIT(1)
#define  VTSS_F_SYS_SYSTEM_PORT_MODE_INJ_HDR_ERR  VTSS_BIT(0)

#define VTSS_SYS_SYSTEM_FRONT_PORT_MODE(ri)  VTSS_IOREG(VTSS_TO_SYS,0x156 + (ri))
#define  VTSS_F_SYS_SYSTEM_FRONT_PORT_MODE_HDX_MODE  VTSS_BIT(0)

#define VTSS_SYS_SYSTEM_FRM_AGING            VTSS_IOREG(VTSS_TO_SYS,0x161)
#define  VTSS_F_SYS_SYSTEM_FRM_AGING_AGE_TX_ENA  VTSS_BIT(20)
#define  VTSS_F_SYS_SYSTEM_FRM_AGING_MAX_AGE(x)  VTSS_ENCODE_BITFIELD(x,0,20)
#define  VTSS_M_SYS_SYSTEM_FRM_AGING_MAX_AGE     VTSS_ENCODE_BITMASK(0,20)
#define  VTSS_X_SYS_SYSTEM_FRM_AGING_MAX_AGE(x)  VTSS_EXTRACT_BITFIELD(x,0,20)

#define VTSS_SYS_SYSTEM_STAT_CFG             VTSS_IOREG(VTSS_TO_SYS,0x162)
#define  VTSS_F_SYS_SYSTEM_STAT_CFG_STAT_CLEAR_SHOT(x)  VTSS_ENCODE_BITFIELD(x,10,5)
#define  VTSS_M_SYS_SYSTEM_STAT_CFG_STAT_CLEAR_SHOT     VTSS_ENCODE_BITMASK(10,5)
#define  VTSS_X_SYS_SYSTEM_STAT_CFG_STAT_CLEAR_SHOT(x)  VTSS_EXTRACT_BITFIELD(x,10,5)
#define  VTSS_F_SYS_SYSTEM_STAT_CFG_STAT_VIEW(x)  VTSS_ENCODE_BITFIELD(x,0,10)
#define  VTSS_M_SYS_SYSTEM_STAT_CFG_STAT_VIEW     VTSS_ENCODE_BITMASK(0,10)
#define  VTSS_X_SYS_SYSTEM_STAT_CFG_STAT_VIEW(x)  VTSS_EXTRACT_BITFIELD(x,0,10)

#define VTSS_SYS_SYSTEM_MPLS_QOS_MAP_CFG(ri)  VTSS_IOREG(VTSS_TO_SYS,0x16f + (ri))
#define  VTSS_F_SYS_SYSTEM_MPLS_QOS_MAP_CFG_MPLS_TC_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_SYS_SYSTEM_MPLS_QOS_MAP_CFG_MPLS_TC_VAL     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_SYS_SYSTEM_MPLS_QOS_MAP_CFG_MPLS_TC_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,24)

#define VTSS_SYS_SYSTEM_IO_PATH_DELAY(ri)    VTSS_IOREG(VTSS_TO_SYS,0x17f + (ri))
#define  VTSS_F_SYS_SYSTEM_IO_PATH_DELAY_RX_PATH_DELAY(x)  VTSS_ENCODE_BITFIELD(x,12,12)
#define  VTSS_M_SYS_SYSTEM_IO_PATH_DELAY_RX_PATH_DELAY     VTSS_ENCODE_BITMASK(12,12)
#define  VTSS_X_SYS_SYSTEM_IO_PATH_DELAY_RX_PATH_DELAY(x)  VTSS_EXTRACT_BITFIELD(x,12,12)
#define  VTSS_F_SYS_SYSTEM_IO_PATH_DELAY_TX_PATH_DELAY(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_SYS_SYSTEM_IO_PATH_DELAY_TX_PATH_DELAY     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_SYS_SYSTEM_IO_PATH_DELAY_TX_PATH_DELAY(x)  VTSS_EXTRACT_BITFIELD(x,0,12)

#define VTSS_SYS_SYSTEM_MISC_CFG             VTSS_IOREG(VTSS_TO_SYS,0x18b)

#define VTSS_SYS_ENCAPSULATIONS_ENCAP_CTRL   VTSS_IOREG(VTSS_TO_SYS,0x18c)
#define  VTSS_F_SYS_ENCAPSULATIONS_ENCAP_CTRL_ENCAP_ID(x)  VTSS_ENCODE_BITFIELD(x,1,10)
#define  VTSS_M_SYS_ENCAPSULATIONS_ENCAP_CTRL_ENCAP_ID     VTSS_ENCODE_BITMASK(1,10)
#define  VTSS_X_SYS_ENCAPSULATIONS_ENCAP_CTRL_ENCAP_ID(x)  VTSS_EXTRACT_BITFIELD(x,1,10)
#define  VTSS_F_SYS_ENCAPSULATIONS_ENCAP_CTRL_ENCAP_WR  VTSS_BIT(0)

#define VTSS_SYS_ENCAPSULATIONS_ENCAP_DATA(ri)  VTSS_IOREG(VTSS_TO_SYS,0x18d + (ri))

#define VTSS_SYS_COREMEM_CM_ADDR             VTSS_IOREG(VTSS_TO_SYS,0x144)
#define  VTSS_F_SYS_COREMEM_CM_ADDR_CM_ADDR(x)  VTSS_ENCODE_BITFIELD(x,0,22)
#define  VTSS_M_SYS_COREMEM_CM_ADDR_CM_ADDR     VTSS_ENCODE_BITMASK(0,22)
#define  VTSS_X_SYS_COREMEM_CM_ADDR_CM_ADDR(x)  VTSS_EXTRACT_BITFIELD(x,0,22)

#define VTSS_SYS_COREMEM_CM_DATA             VTSS_IOREG(VTSS_TO_SYS,0x145)

#define VTSS_SYS_PAUSE_CFG_PAUSE_CFG(ri)     VTSS_IOREG(VTSS_TO_SYS,0x197 + (ri))
#define  VTSS_F_SYS_PAUSE_CFG_PAUSE_CFG_PAUSE_START(x)  VTSS_ENCODE_BITFIELD(x,13,12)
#define  VTSS_M_SYS_PAUSE_CFG_PAUSE_CFG_PAUSE_START     VTSS_ENCODE_BITMASK(13,12)
#define  VTSS_X_SYS_PAUSE_CFG_PAUSE_CFG_PAUSE_START(x)  VTSS_EXTRACT_BITFIELD(x,13,12)
#define  VTSS_F_SYS_PAUSE_CFG_PAUSE_CFG_PAUSE_STOP(x)  VTSS_ENCODE_BITFIELD(x,1,12)
#define  VTSS_M_SYS_PAUSE_CFG_PAUSE_CFG_PAUSE_STOP     VTSS_ENCODE_BITMASK(1,12)
#define  VTSS_X_SYS_PAUSE_CFG_PAUSE_CFG_PAUSE_STOP(x)  VTSS_EXTRACT_BITFIELD(x,1,12)
#define  VTSS_F_SYS_PAUSE_CFG_PAUSE_CFG_PAUSE_ENA  VTSS_BIT(0)

#define VTSS_SYS_PAUSE_CFG_PAUSE_TOT_CFG     VTSS_IOREG(VTSS_TO_SYS,0x1a3)
#define  VTSS_F_SYS_PAUSE_CFG_PAUSE_TOT_CFG_PAUSE_TOT_START(x)  VTSS_ENCODE_BITFIELD(x,12,12)
#define  VTSS_M_SYS_PAUSE_CFG_PAUSE_TOT_CFG_PAUSE_TOT_START     VTSS_ENCODE_BITMASK(12,12)
#define  VTSS_X_SYS_PAUSE_CFG_PAUSE_TOT_CFG_PAUSE_TOT_START(x)  VTSS_EXTRACT_BITFIELD(x,12,12)
#define  VTSS_F_SYS_PAUSE_CFG_PAUSE_TOT_CFG_PAUSE_TOT_STOP(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_SYS_PAUSE_CFG_PAUSE_TOT_CFG_PAUSE_TOT_STOP     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_SYS_PAUSE_CFG_PAUSE_TOT_CFG_PAUSE_TOT_STOP(x)  VTSS_EXTRACT_BITFIELD(x,0,12)

#define VTSS_SYS_PAUSE_CFG_ATOP(ri)          VTSS_IOREG(VTSS_TO_SYS,0x1a4 + (ri))
#define  VTSS_F_SYS_PAUSE_CFG_ATOP_ATOP(x)    VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_SYS_PAUSE_CFG_ATOP_ATOP       VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_SYS_PAUSE_CFG_ATOP_ATOP(x)    VTSS_EXTRACT_BITFIELD(x,0,12)

#define VTSS_SYS_PAUSE_CFG_ATOP_TOT_CFG      VTSS_IOREG(VTSS_TO_SYS,0x1b0)
#define  VTSS_F_SYS_PAUSE_CFG_ATOP_TOT_CFG_ATOP_TOT(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_SYS_PAUSE_CFG_ATOP_TOT_CFG_ATOP_TOT     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_SYS_PAUSE_CFG_ATOP_TOT_CFG_ATOP_TOT(x)  VTSS_EXTRACT_BITFIELD(x,0,12)

#define VTSS_SYS_PAUSE_CFG_MAC_FC_CFG(ri)    VTSS_IOREG(VTSS_TO_SYS,0x1b1 + (ri))
#define  VTSS_F_SYS_PAUSE_CFG_MAC_FC_CFG_FC_LINK_SPEED(x)  VTSS_ENCODE_BITFIELD(x,26,2)
#define  VTSS_M_SYS_PAUSE_CFG_MAC_FC_CFG_FC_LINK_SPEED     VTSS_ENCODE_BITMASK(26,2)
#define  VTSS_X_SYS_PAUSE_CFG_MAC_FC_CFG_FC_LINK_SPEED(x)  VTSS_EXTRACT_BITFIELD(x,26,2)
#define  VTSS_F_SYS_PAUSE_CFG_MAC_FC_CFG_FC_LATENCY_CFG(x)  VTSS_ENCODE_BITFIELD(x,20,6)
#define  VTSS_M_SYS_PAUSE_CFG_MAC_FC_CFG_FC_LATENCY_CFG     VTSS_ENCODE_BITMASK(20,6)
#define  VTSS_X_SYS_PAUSE_CFG_MAC_FC_CFG_FC_LATENCY_CFG(x)  VTSS_EXTRACT_BITFIELD(x,20,6)
#define  VTSS_F_SYS_PAUSE_CFG_MAC_FC_CFG_ZERO_PAUSE_ENA  VTSS_BIT(18)
#define  VTSS_F_SYS_PAUSE_CFG_MAC_FC_CFG_TX_FC_ENA  VTSS_BIT(17)
#define  VTSS_F_SYS_PAUSE_CFG_MAC_FC_CFG_RX_FC_ENA  VTSS_BIT(16)
#define  VTSS_F_SYS_PAUSE_CFG_MAC_FC_CFG_PAUSE_VAL_CFG(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_SYS_PAUSE_CFG_MAC_FC_CFG_PAUSE_VAL_CFG     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_SYS_PAUSE_CFG_MAC_FC_CFG_PAUSE_VAL_CFG(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_SYS_MMGT_MMGT                   VTSS_IOREG(VTSS_TO_SYS,0x1bc)
#define  VTSS_F_SYS_MMGT_MMGT_FREECNT(x)      VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_SYS_MMGT_MMGT_FREECNT         VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_SYS_MMGT_MMGT_FREECNT(x)      VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_SYS_MMGT_MMGT_FAST              VTSS_IOREG(VTSS_TO_SYS,0x1bd)

#define VTSS_SYS_STAT_CNT(gi)                VTSS_IOREG_IX(VTSS_TO_SYS,0x0,gi,1,0,0)

#define VTSS_SYS_PTP_PTP_STATUS              VTSS_IOREG(VTSS_TO_SYS,0x1c3)
#define  VTSS_F_SYS_PTP_PTP_STATUS_PTP_TXSTAMP_OAM  VTSS_BIT(29)
#define  VTSS_F_SYS_PTP_PTP_STATUS_PTP_OVFL   VTSS_BIT(28)
#define  VTSS_F_SYS_PTP_PTP_STATUS_PTP_MESS_VLD  VTSS_BIT(27)
#define  VTSS_F_SYS_PTP_PTP_STATUS_PTP_MESS_ID(x)  VTSS_ENCODE_BITFIELD(x,21,6)
#define  VTSS_M_SYS_PTP_PTP_STATUS_PTP_MESS_ID     VTSS_ENCODE_BITMASK(21,6)
#define  VTSS_X_SYS_PTP_PTP_STATUS_PTP_MESS_ID(x)  VTSS_EXTRACT_BITFIELD(x,21,6)
#define  VTSS_F_SYS_PTP_PTP_STATUS_PTP_MESS_TXPORT(x)  VTSS_ENCODE_BITFIELD(x,16,5)
#define  VTSS_M_SYS_PTP_PTP_STATUS_PTP_MESS_TXPORT     VTSS_ENCODE_BITMASK(16,5)
#define  VTSS_X_SYS_PTP_PTP_STATUS_PTP_MESS_TXPORT(x)  VTSS_EXTRACT_BITFIELD(x,16,5)
#define  VTSS_F_SYS_PTP_PTP_STATUS_PTP_MESS_SEQ_ID(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_SYS_PTP_PTP_STATUS_PTP_MESS_SEQ_ID     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_SYS_PTP_PTP_STATUS_PTP_MESS_SEQ_ID(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_SYS_PTP_PTP_TXSTAMP             VTSS_IOREG(VTSS_TO_SYS,0x1c4)
#define  VTSS_F_SYS_PTP_PTP_TXSTAMP_PTP_TXSTAMP(x)  VTSS_ENCODE_BITFIELD(x,0,30)
#define  VTSS_M_SYS_PTP_PTP_TXSTAMP_PTP_TXSTAMP     VTSS_ENCODE_BITMASK(0,30)
#define  VTSS_X_SYS_PTP_PTP_TXSTAMP_PTP_TXSTAMP(x)  VTSS_EXTRACT_BITFIELD(x,0,30)
#define  VTSS_F_SYS_PTP_PTP_TXSTAMP_PTP_TXSTAMP_SEC  VTSS_BIT(31)

#define VTSS_SYS_PTP_PTP_NXT                 VTSS_IOREG(VTSS_TO_SYS,0x1c5)
#define  VTSS_F_SYS_PTP_PTP_NXT_PTP_NXT       VTSS_BIT(0)

#define VTSS_SYS_PTP_PTP_CFG                 VTSS_IOREG(VTSS_TO_SYS,0x1c6)
#define  VTSS_F_SYS_PTP_PTP_CFG_PTP_STAMP_WID(x)  VTSS_ENCODE_BITFIELD(x,2,6)
#define  VTSS_M_SYS_PTP_PTP_CFG_PTP_STAMP_WID     VTSS_ENCODE_BITMASK(2,6)
#define  VTSS_X_SYS_PTP_PTP_CFG_PTP_STAMP_WID(x)  VTSS_EXTRACT_BITFIELD(x,2,6)
#define  VTSS_F_SYS_PTP_PTP_CFG_PTP_CF_ROLL_MODE(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_SYS_PTP_PTP_CFG_PTP_CF_ROLL_MODE     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_SYS_PTP_PTP_CFG_PTP_CF_ROLL_MODE(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

#define VTSS_SYS_PTP_TOD_PTP_TOD_MSB         VTSS_IOREG(VTSS_TO_SYS,0x140)
#define  VTSS_F_SYS_PTP_TOD_PTP_TOD_MSB_PTP_TOD_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_SYS_PTP_TOD_PTP_TOD_MSB_PTP_TOD_MSB     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_SYS_PTP_TOD_PTP_TOD_MSB_PTP_TOD_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_SYS_PTP_TOD_PTP_TOD_LSB         VTSS_IOREG(VTSS_TO_SYS,0x141)

#define VTSS_SYS_PTP_TOD_PTP_TOD_NSEC        VTSS_IOREG(VTSS_TO_SYS,0x142)
#define  VTSS_F_SYS_PTP_TOD_PTP_TOD_NSEC_PTP_TOD_NSEC(x)  VTSS_ENCODE_BITFIELD(x,0,30)
#define  VTSS_M_SYS_PTP_TOD_PTP_TOD_NSEC_PTP_TOD_NSEC     VTSS_ENCODE_BITMASK(0,30)
#define  VTSS_X_SYS_PTP_TOD_PTP_TOD_NSEC_PTP_TOD_NSEC(x)  VTSS_EXTRACT_BITFIELD(x,0,30)


#endif /* _VTSS_SERVAL_REGS_SYS_H_ */