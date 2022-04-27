#ifndef _VTSS_JAGUAR2_REGS_DSM_H_
#define _VTSS_JAGUAR2_REGS_DSM_H_

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

#define VTSS_DSM_RAM_CTRL_RAM_INIT           VTSS_IOREG(VTSS_TO_DSM,0x2)
#define  VTSS_F_DSM_RAM_CTRL_RAM_INIT_RAM_INIT  VTSS_BIT(1)
#define  VTSS_F_DSM_RAM_CTRL_RAM_INIT_RAM_ENA  VTSS_BIT(0)

#define VTSS_DSM_COREMEM_CM_ADDR             VTSS_IOREG(VTSS_TO_DSM,0x0)
#define  VTSS_F_DSM_COREMEM_CM_ADDR_CM_ID(x)  VTSS_ENCODE_BITFIELD(x,22,8)
#define  VTSS_M_DSM_COREMEM_CM_ADDR_CM_ID     VTSS_ENCODE_BITMASK(22,8)
#define  VTSS_X_DSM_COREMEM_CM_ADDR_CM_ID(x)  VTSS_EXTRACT_BITFIELD(x,22,8)
#define  VTSS_F_DSM_COREMEM_CM_ADDR_CM_ADDR(x)  VTSS_ENCODE_BITFIELD(x,0,22)
#define  VTSS_M_DSM_COREMEM_CM_ADDR_CM_ADDR     VTSS_ENCODE_BITMASK(0,22)
#define  VTSS_X_DSM_COREMEM_CM_ADDR_CM_ADDR(x)  VTSS_EXTRACT_BITFIELD(x,0,22)

#define VTSS_DSM_COREMEM_CM_DATA             VTSS_IOREG(VTSS_TO_DSM,0x1)

#define VTSS_DSM_CFG_BUF_CFG(ri)             VTSS_IOREG(VTSS_TO_DSM,0x3 + (ri))
#define  VTSS_F_DSM_CFG_BUF_CFG_CSC_STAT_DIS  VTSS_BIT(1)
#define  VTSS_F_DSM_CFG_BUF_CFG_AGING_ENA     VTSS_BIT(0)

#define VTSS_DSM_CFG_RATE_CTRL(ri)           VTSS_IOREG(VTSS_TO_DSM,0x3a + (ri))
#define  VTSS_F_DSM_CFG_RATE_CTRL_FRM_GAP_COMP(x)  VTSS_ENCODE_BITFIELD(x,24,8)
#define  VTSS_M_DSM_CFG_RATE_CTRL_FRM_GAP_COMP     VTSS_ENCODE_BITMASK(24,8)
#define  VTSS_X_DSM_CFG_RATE_CTRL_FRM_GAP_COMP(x)  VTSS_EXTRACT_BITFIELD(x,24,8)

#define VTSS_DSM_CFG_IPG_SHRINK_CFG(ri)      VTSS_IOREG(VTSS_TO_DSM,0x71 + (ri))
#define  VTSS_F_DSM_CFG_IPG_SHRINK_CFG_IPG_PREAM_SHRINK_ENA  VTSS_BIT(1)
#define  VTSS_F_DSM_CFG_IPG_SHRINK_CFG_IPG_SHRINK_ENA  VTSS_BIT(0)

#define VTSS_DSM_CFG_CLR_BUF(ri)             VTSS_IOREG(VTSS_TO_DSM,0xa8 + (ri))

#define VTSS_DSM_CFG_SCH_STOP_WM_CFG(ri)     VTSS_IOREG(VTSS_TO_DSM,0xaa + (ri))
#define  VTSS_F_DSM_CFG_SCH_STOP_WM_CFG_SCH_STOP_WM(x)  VTSS_ENCODE_BITFIELD(x,0,9)
#define  VTSS_M_DSM_CFG_SCH_STOP_WM_CFG_SCH_STOP_WM     VTSS_ENCODE_BITMASK(0,9)
#define  VTSS_X_DSM_CFG_SCH_STOP_WM_CFG_SCH_STOP_WM(x)  VTSS_EXTRACT_BITFIELD(x,0,9)

#define VTSS_DSM_CFG_TX_START_WM_CFG(ri)     VTSS_IOREG(VTSS_TO_DSM,0xe1 + (ri))
#define  VTSS_F_DSM_CFG_TX_START_WM_CFG_TX_START_WM(x)  VTSS_ENCODE_BITFIELD(x,0,9)
#define  VTSS_M_DSM_CFG_TX_START_WM_CFG_TX_START_WM     VTSS_ENCODE_BITMASK(0,9)
#define  VTSS_X_DSM_CFG_TX_START_WM_CFG_TX_START_WM(x)  VTSS_EXTRACT_BITFIELD(x,0,9)

#define VTSS_DSM_CFG_DEV_TX_STOP_WM_CFG(ri)  VTSS_IOREG(VTSS_TO_DSM,0x118 + (ri))
#define  VTSS_F_DSM_CFG_DEV_TX_STOP_WM_CFG_FAST_STARTUP_ENA  VTSS_BIT(7)
#define  VTSS_F_DSM_CFG_DEV_TX_STOP_WM_CFG_DEV10G_SHADOW_ENA  VTSS_BIT(6)
#define  VTSS_F_DSM_CFG_DEV_TX_STOP_WM_CFG_DEV_TX_STOP_WM(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_DSM_CFG_DEV_TX_STOP_WM_CFG_DEV_TX_STOP_WM     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_DSM_CFG_DEV_TX_STOP_WM_CFG_DEV_TX_STOP_WM(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

#define VTSS_DSM_CFG_RX_PAUSE_CFG(ri)        VTSS_IOREG(VTSS_TO_DSM,0x14f + (ri))
#define  VTSS_F_DSM_CFG_RX_PAUSE_CFG_RX_PAUSE_EN  VTSS_BIT(1)
#define  VTSS_F_DSM_CFG_RX_PAUSE_CFG_FC_OBEY_LOCAL  VTSS_BIT(0)

#define VTSS_DSM_CFG_ETH_FC_CFG(ri)          VTSS_IOREG(VTSS_TO_DSM,0x186 + (ri))
#define  VTSS_F_DSM_CFG_ETH_FC_CFG_FC_ANA_ENA  VTSS_BIT(1)
#define  VTSS_F_DSM_CFG_ETH_FC_CFG_FC_QS_ENA  VTSS_BIT(0)

#define VTSS_DSM_CFG_ETH_PFC_CFG(ri)         VTSS_IOREG(VTSS_TO_DSM,0x1bd + (ri))
#define  VTSS_F_DSM_CFG_ETH_PFC_CFG_PFC_MIN_UPDATE_TIME(x)  VTSS_ENCODE_BITFIELD(x,2,15)
#define  VTSS_M_DSM_CFG_ETH_PFC_CFG_PFC_MIN_UPDATE_TIME     VTSS_ENCODE_BITMASK(2,15)
#define  VTSS_X_DSM_CFG_ETH_PFC_CFG_PFC_MIN_UPDATE_TIME(x)  VTSS_EXTRACT_BITFIELD(x,2,15)
#define  VTSS_F_DSM_CFG_ETH_PFC_CFG_PFC_XOFF_MIN_UPDATE_ENA  VTSS_BIT(1)
#define  VTSS_F_DSM_CFG_ETH_PFC_CFG_PFC_ENA   VTSS_BIT(0)

#define VTSS_DSM_CFG_MAC_CFG(ri)             VTSS_IOREG(VTSS_TO_DSM,0x1f4 + (ri))
#define  VTSS_F_DSM_CFG_MAC_CFG_TX_PAUSE_VAL(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_DSM_CFG_MAC_CFG_TX_PAUSE_VAL     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_DSM_CFG_MAC_CFG_TX_PAUSE_VAL(x)  VTSS_EXTRACT_BITFIELD(x,16,16)
#define  VTSS_F_DSM_CFG_MAC_CFG_HDX_BACKPRESSURE  VTSS_BIT(2)
#define  VTSS_F_DSM_CFG_MAC_CFG_SEND_PAUSE_FRM_TWICE  VTSS_BIT(1)
#define  VTSS_F_DSM_CFG_MAC_CFG_TX_PAUSE_XON_XOFF  VTSS_BIT(0)

#define VTSS_DSM_CFG_MAC_ADDR_BASE_HIGH_CFG(ri)  VTSS_IOREG(VTSS_TO_DSM,0x22b + (ri))
#define  VTSS_F_DSM_CFG_MAC_ADDR_BASE_HIGH_CFG_MAC_ADDR_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_DSM_CFG_MAC_ADDR_BASE_HIGH_CFG_MAC_ADDR_HIGH     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_DSM_CFG_MAC_ADDR_BASE_HIGH_CFG_MAC_ADDR_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,24)

#define VTSS_DSM_CFG_MAC_ADDR_BASE_LOW_CFG(ri)  VTSS_IOREG(VTSS_TO_DSM,0x260 + (ri))
#define  VTSS_F_DSM_CFG_MAC_ADDR_BASE_LOW_CFG_MAC_ADDR_LOW(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_DSM_CFG_MAC_ADDR_BASE_LOW_CFG_MAC_ADDR_LOW     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_DSM_CFG_MAC_ADDR_BASE_LOW_CFG_MAC_ADDR_LOW(x)  VTSS_EXTRACT_BITFIELD(x,0,24)

#define VTSS_DSM_CFG_DBG_CTRL                VTSS_IOREG(VTSS_TO_DSM,0x295)
#define  VTSS_F_DSM_CFG_DBG_CTRL_DBG_EVENT_CTRL(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_DSM_CFG_DBG_CTRL_DBG_EVENT_CTRL     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_DSM_CFG_DBG_CTRL_DBG_EVENT_CTRL(x)  VTSS_EXTRACT_BITFIELD(x,0,3)

#define VTSS_DSM_STATUS_AGED_FRMS(ri)        VTSS_IOREG(VTSS_TO_DSM,0x296 + (ri))

#define VTSS_DSM_STATUS_CELL_BUS_STICKY      VTSS_IOREG(VTSS_TO_DSM,0x2cd)
#define  VTSS_F_DSM_STATUS_CELL_BUS_STICKY_CELL_BUS_MISSING_SOF_STICKY  VTSS_BIT(1)
#define  VTSS_F_DSM_STATUS_CELL_BUS_STICKY_CELL_BUS_MISSING_EOF_STICKY  VTSS_BIT(0)

#define VTSS_DSM_STATUS_BUF_OFLW_STICKY(ri)  VTSS_IOREG(VTSS_TO_DSM,0x2ce + (ri))

#define VTSS_DSM_STATUS_BUF_UFLW_STICKY(ri)  VTSS_IOREG(VTSS_TO_DSM,0x2d0 + (ri))

#define VTSS_DSM_STATUS_PRE_CNT_OFLW_STICKY  VTSS_IOREG(VTSS_TO_DSM,0x2d2)
#define  VTSS_F_DSM_STATUS_PRE_CNT_OFLW_STICKY_PRE_CNT_OFLW_STICKY  VTSS_BIT(8)

#define VTSS_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE(ri)  VTSS_IOREG(VTSS_TO_DSM,0x2d3 + (ri))
#define  VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_IPG_SCALE_VAL(x)  VTSS_ENCODE_BITFIELD(x,16,4)
#define  VTSS_M_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_IPG_SCALE_VAL     VTSS_ENCODE_BITMASK(16,4)
#define  VTSS_X_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_IPG_SCALE_VAL(x)  VTSS_EXTRACT_BITFIELD(x,16,4)
#define  VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_PAYLOAD_CFG  VTSS_BIT(9)
#define  VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_PAYLOAD_PREAM_CFG  VTSS_BIT(8)
#define  VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_TX_RATE_LIMIT_ACCUM_MODE_ENA  VTSS_BIT(4)
#define  VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_TX_RATE_IPG_PPM_ADAPT_ENA  VTSS_BIT(3)
#define  VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_TX_RATE_LIMIT_FRAME_RATE_ENA  VTSS_BIT(2)
#define  VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_TX_RATE_LIMIT_PAYLOAD_RATE_ENA  VTSS_BIT(1)
#define  VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_TX_RATE_LIMIT_FRAME_OVERHEAD_ENA  VTSS_BIT(0)

#define VTSS_DSM_RATE_LIMIT_CFG_TX_IPG_STRETCH_RATIO_CFG(ri)  VTSS_IOREG(VTSS_TO_DSM,0x30a + (ri))
#define  VTSS_F_DSM_RATE_LIMIT_CFG_TX_IPG_STRETCH_RATIO_CFG_TX_FINE_IPG_STRETCH_RATIO(x)  VTSS_ENCODE_BITFIELD(x,0,19)
#define  VTSS_M_DSM_RATE_LIMIT_CFG_TX_IPG_STRETCH_RATIO_CFG_TX_FINE_IPG_STRETCH_RATIO     VTSS_ENCODE_BITMASK(0,19)
#define  VTSS_X_DSM_RATE_LIMIT_CFG_TX_IPG_STRETCH_RATIO_CFG_TX_FINE_IPG_STRETCH_RATIO(x)  VTSS_EXTRACT_BITFIELD(x,0,19)

#define VTSS_DSM_RATE_LIMIT_CFG_TX_FRAME_RATE_START_CFG(ri)  VTSS_IOREG(VTSS_TO_DSM,0x341 + (ri))
#define  VTSS_F_DSM_RATE_LIMIT_CFG_TX_FRAME_RATE_START_CFG_TX_FRAME_RATE_START(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DSM_RATE_LIMIT_CFG_TX_FRAME_RATE_START_CFG_TX_FRAME_RATE_START     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DSM_RATE_LIMIT_CFG_TX_FRAME_RATE_START_CFG_TX_FRAME_RATE_START(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_HDR_CFG  VTSS_IOREG(VTSS_TO_DSM,0x378)
#define  VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_HDR_CFG_TX_RATE_LIMIT_HDR_SIZE(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_HDR_CFG_TX_RATE_LIMIT_HDR_SIZE     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_HDR_CFG_TX_RATE_LIMIT_HDR_SIZE(x)  VTSS_EXTRACT_BITFIELD(x,0,5)

#define VTSS_DSM_RATE_LIMIT_STATUS_TX_RATE_LIMIT_STICKY(ri)  VTSS_IOREG(VTSS_TO_DSM,0x379 + (ri))


#endif /* _VTSS_JAGUAR2_REGS_DSM_H_ */
