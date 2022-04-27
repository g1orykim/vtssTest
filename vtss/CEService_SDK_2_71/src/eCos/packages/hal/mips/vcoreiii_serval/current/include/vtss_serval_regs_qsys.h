#ifndef _VTSS_SERVAL_REGS_QSYS_H_
#define _VTSS_SERVAL_REGS_QSYS_H_

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

#define VTSS_QSYS_SYSTEM_PORT_MODE(ri)       VTSS_IOREG(VTSS_TO_QSYS,0x5680 + (ri))
#define  VTSS_F_QSYS_SYSTEM_PORT_MODE_DEQUEUE_DIS  VTSS_BIT(1)
#define  VTSS_F_QSYS_SYSTEM_PORT_MODE_DEQUEUE_LATE  VTSS_BIT(0)

#define VTSS_QSYS_SYSTEM_SWITCH_PORT_MODE(ri)  VTSS_IOREG(VTSS_TO_QSYS,0x568d + (ri))
#define  VTSS_F_QSYS_SYSTEM_SWITCH_PORT_MODE_PORT_ENA  VTSS_BIT(13)
#define  VTSS_F_QSYS_SYSTEM_SWITCH_PORT_MODE_INGRESS_DROP_MODE  VTSS_BIT(9)
#define  VTSS_F_QSYS_SYSTEM_SWITCH_PORT_MODE_TX_PFC_ENA(x)  VTSS_ENCODE_BITFIELD(x,1,8)
#define  VTSS_M_QSYS_SYSTEM_SWITCH_PORT_MODE_TX_PFC_ENA     VTSS_ENCODE_BITMASK(1,8)
#define  VTSS_X_QSYS_SYSTEM_SWITCH_PORT_MODE_TX_PFC_ENA(x)  VTSS_EXTRACT_BITFIELD(x,1,8)
#define  VTSS_F_QSYS_SYSTEM_SWITCH_PORT_MODE_TX_PFC_MODE  VTSS_BIT(0)

#define VTSS_QSYS_SYSTEM_STAT_CNT_CFG        VTSS_IOREG(VTSS_TO_QSYS,0x5699)
#define  VTSS_F_QSYS_SYSTEM_STAT_CNT_CFG_TX_GREEN_CNT_MODE  VTSS_BIT(5)
#define  VTSS_F_QSYS_SYSTEM_STAT_CNT_CFG_TX_YELLOW_CNT_MODE  VTSS_BIT(4)
#define  VTSS_F_QSYS_SYSTEM_STAT_CNT_CFG_DROP_GREEN_CNT_MODE  VTSS_BIT(3)
#define  VTSS_F_QSYS_SYSTEM_STAT_CNT_CFG_DROP_YELLOW_CNT_MODE  VTSS_BIT(2)
#define  VTSS_F_QSYS_SYSTEM_STAT_CNT_CFG_DROP_COUNT_EGRESS  VTSS_BIT(0)

#define VTSS_QSYS_SYSTEM_EEE_CFG(ri)         VTSS_IOREG(VTSS_TO_QSYS,0x569a + (ri))
#define  VTSS_F_QSYS_SYSTEM_EEE_CFG_EEE_FAST_QUEUES(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_QSYS_SYSTEM_EEE_CFG_EEE_FAST_QUEUES     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_QSYS_SYSTEM_EEE_CFG_EEE_FAST_QUEUES(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

#define VTSS_QSYS_SYSTEM_EEE_THRES           VTSS_IOREG(VTSS_TO_QSYS,0x56a5)
#define  VTSS_F_QSYS_SYSTEM_EEE_THRES_EEE_HIGH_BYTES(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_QSYS_SYSTEM_EEE_THRES_EEE_HIGH_BYTES     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_QSYS_SYSTEM_EEE_THRES_EEE_HIGH_BYTES(x)  VTSS_EXTRACT_BITFIELD(x,8,8)
#define  VTSS_F_QSYS_SYSTEM_EEE_THRES_EEE_HIGH_FRAMES(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_QSYS_SYSTEM_EEE_THRES_EEE_HIGH_FRAMES     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_QSYS_SYSTEM_EEE_THRES_EEE_HIGH_FRAMES(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

#define VTSS_QSYS_SYSTEM_IGR_NO_SHARING      VTSS_IOREG(VTSS_TO_QSYS,0x56a6)
#define  VTSS_F_QSYS_SYSTEM_IGR_NO_SHARING_IGR_NO_SHARING(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_QSYS_SYSTEM_IGR_NO_SHARING_IGR_NO_SHARING     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_QSYS_SYSTEM_IGR_NO_SHARING_IGR_NO_SHARING(x)  VTSS_EXTRACT_BITFIELD(x,0,12)

#define VTSS_QSYS_SYSTEM_EGR_NO_SHARING      VTSS_IOREG(VTSS_TO_QSYS,0x56a7)
#define  VTSS_F_QSYS_SYSTEM_EGR_NO_SHARING_EGR_NO_SHARING(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_QSYS_SYSTEM_EGR_NO_SHARING_EGR_NO_SHARING     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_QSYS_SYSTEM_EGR_NO_SHARING_EGR_NO_SHARING(x)  VTSS_EXTRACT_BITFIELD(x,0,12)

#define VTSS_QSYS_SYSTEM_SW_STATUS(ri)       VTSS_IOREG(VTSS_TO_QSYS,0x56a8 + (ri))
#define  VTSS_F_QSYS_SYSTEM_SW_STATUS_EQ_AVAIL(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_QSYS_SYSTEM_SW_STATUS_EQ_AVAIL     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_QSYS_SYSTEM_SW_STATUS_EQ_AVAIL(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

#define VTSS_QSYS_SYSTEM_EXT_CPU_CFG         VTSS_IOREG(VTSS_TO_QSYS,0x56b4)
#define  VTSS_F_QSYS_SYSTEM_EXT_CPU_CFG_EXT_CPU_PORT(x)  VTSS_ENCODE_BITFIELD(x,8,5)
#define  VTSS_M_QSYS_SYSTEM_EXT_CPU_CFG_EXT_CPU_PORT     VTSS_ENCODE_BITMASK(8,5)
#define  VTSS_X_QSYS_SYSTEM_EXT_CPU_CFG_EXT_CPU_PORT(x)  VTSS_EXTRACT_BITFIELD(x,8,5)
#define  VTSS_F_QSYS_SYSTEM_EXT_CPU_CFG_EXT_CPUQ_MSK(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_QSYS_SYSTEM_EXT_CPU_CFG_EXT_CPUQ_MSK     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_QSYS_SYSTEM_EXT_CPU_CFG_EXT_CPUQ_MSK(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

#define VTSS_QSYS_SYSTEM_CPU_GROUP_MAP       VTSS_IOREG(VTSS_TO_QSYS,0x56b6)
#define  VTSS_F_QSYS_SYSTEM_CPU_GROUP_MAP_CPU_GROUP_MAP(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_QSYS_SYSTEM_CPU_GROUP_MAP_CPU_GROUP_MAP     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_QSYS_SYSTEM_CPU_GROUP_MAP_CPU_GROUP_MAP(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

#define VTSS_QSYS_QMAP_QMAP(gi)              VTSS_IOREG_IX(VTSS_TO_QSYS,0x56b7,gi,1,0,0)
#define  VTSS_F_QSYS_QMAP_QMAP_SE_BASE(x)     VTSS_ENCODE_BITFIELD(x,5,8)
#define  VTSS_M_QSYS_QMAP_QMAP_SE_BASE        VTSS_ENCODE_BITMASK(5,8)
#define  VTSS_X_QSYS_QMAP_QMAP_SE_BASE(x)     VTSS_EXTRACT_BITFIELD(x,5,8)
#define  VTSS_F_QSYS_QMAP_QMAP_SE_IDX_SEL(x)  VTSS_ENCODE_BITFIELD(x,2,3)
#define  VTSS_M_QSYS_QMAP_QMAP_SE_IDX_SEL     VTSS_ENCODE_BITMASK(2,3)
#define  VTSS_X_QSYS_QMAP_QMAP_SE_IDX_SEL(x)  VTSS_EXTRACT_BITFIELD(x,2,3)
#define  VTSS_F_QSYS_QMAP_QMAP_SE_INP_SEL(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_QSYS_QMAP_QMAP_SE_INP_SEL     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_QSYS_QMAP_QMAP_SE_INP_SEL(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

#define VTSS_QSYS_SGRP_ISDX_SGRP(gi)         VTSS_IOREG_IX(VTSS_TO_QSYS,0x6000,gi,1,0,0)
#define  VTSS_F_QSYS_SGRP_ISDX_SGRP_ISDX_SGRP(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_QSYS_SGRP_ISDX_SGRP_ISDX_SGRP     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_QSYS_SGRP_ISDX_SGRP_ISDX_SGRP(x)  VTSS_EXTRACT_BITFIELD(x,0,7)

#define VTSS_QSYS_TIMED_FRAME_DB_TIMED_FRAME_ENTRY(gi)  VTSS_IOREG_IX(VTSS_TO_QSYS,0x6400,gi,1,0,0)
#define  VTSS_F_QSYS_TIMED_FRAME_DB_TIMED_FRAME_ENTRY_TFRM_VLD  VTSS_BIT(23)
#define  VTSS_F_QSYS_TIMED_FRAME_DB_TIMED_FRAME_ENTRY_TFRM_FP(x)  VTSS_ENCODE_BITFIELD(x,8,15)
#define  VTSS_M_QSYS_TIMED_FRAME_DB_TIMED_FRAME_ENTRY_TFRM_FP     VTSS_ENCODE_BITMASK(8,15)
#define  VTSS_X_QSYS_TIMED_FRAME_DB_TIMED_FRAME_ENTRY_TFRM_FP(x)  VTSS_EXTRACT_BITFIELD(x,8,15)
#define  VTSS_F_QSYS_TIMED_FRAME_DB_TIMED_FRAME_ENTRY_TFRM_PORTNO(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_QSYS_TIMED_FRAME_DB_TIMED_FRAME_ENTRY_TFRM_PORTNO     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_QSYS_TIMED_FRAME_DB_TIMED_FRAME_ENTRY_TFRM_PORTNO(x)  VTSS_EXTRACT_BITFIELD(x,4,4)
#define  VTSS_F_QSYS_TIMED_FRAME_DB_TIMED_FRAME_ENTRY_TFRM_TM_SEL(x)  VTSS_ENCODE_BITFIELD(x,1,3)
#define  VTSS_M_QSYS_TIMED_FRAME_DB_TIMED_FRAME_ENTRY_TFRM_TM_SEL     VTSS_ENCODE_BITMASK(1,3)
#define  VTSS_X_QSYS_TIMED_FRAME_DB_TIMED_FRAME_ENTRY_TFRM_TM_SEL(x)  VTSS_EXTRACT_BITFIELD(x,1,3)
#define  VTSS_F_QSYS_TIMED_FRAME_DB_TIMED_FRAME_ENTRY_TFRM_TM_T  VTSS_BIT(0)

#define VTSS_QSYS_TIMED_FRAME_CFG_TFRM_MISC  VTSS_IOREG(VTSS_TO_QSYS,0x56c4)
#define  VTSS_F_QSYS_TIMED_FRAME_CFG_TFRM_MISC_TIMED_CANCEL_SLOT(x)  VTSS_ENCODE_BITFIELD(x,9,10)
#define  VTSS_M_QSYS_TIMED_FRAME_CFG_TFRM_MISC_TIMED_CANCEL_SLOT     VTSS_ENCODE_BITMASK(9,10)
#define  VTSS_X_QSYS_TIMED_FRAME_CFG_TFRM_MISC_TIMED_CANCEL_SLOT(x)  VTSS_EXTRACT_BITFIELD(x,9,10)
#define  VTSS_F_QSYS_TIMED_FRAME_CFG_TFRM_MISC_TIMED_CANCEL_1SHOT  VTSS_BIT(8)

#define VTSS_QSYS_TIMED_FRAME_CFG_TFRM_PORT_DLY  VTSS_IOREG(VTSS_TO_QSYS,0x56c5)
#define  VTSS_F_QSYS_TIMED_FRAME_CFG_TFRM_PORT_DLY_TFRM_PORT_DLY_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,13)
#define  VTSS_M_QSYS_TIMED_FRAME_CFG_TFRM_PORT_DLY_TFRM_PORT_DLY_ENA     VTSS_ENCODE_BITMASK(0,13)
#define  VTSS_X_QSYS_TIMED_FRAME_CFG_TFRM_PORT_DLY_TFRM_PORT_DLY_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,13)

#define VTSS_QSYS_TIMED_FRAME_CFG_TFRM_TIMER_CFG_1  VTSS_IOREG(VTSS_TO_QSYS,0x56c6)

#define VTSS_QSYS_TIMED_FRAME_CFG_TFRM_TIMER_CFG_2  VTSS_IOREG(VTSS_TO_QSYS,0x56c7)

#define VTSS_QSYS_TIMED_FRAME_CFG_TFRM_TIMER_CFG_3  VTSS_IOREG(VTSS_TO_QSYS,0x56c8)

#define VTSS_QSYS_TIMED_FRAME_CFG_TFRM_TIMER_CFG_4  VTSS_IOREG(VTSS_TO_QSYS,0x56c9)

#define VTSS_QSYS_TIMED_FRAME_CFG_TFRM_TIMER_CFG_5  VTSS_IOREG(VTSS_TO_QSYS,0x56ca)

#define VTSS_QSYS_TIMED_FRAME_CFG_TFRM_TIMER_CFG_6  VTSS_IOREG(VTSS_TO_QSYS,0x56cb)

#define VTSS_QSYS_TIMED_FRAME_CFG_TFRM_TIMER_CFG_7  VTSS_IOREG(VTSS_TO_QSYS,0x56cc)

#define VTSS_QSYS_TIMED_FRAME_CFG_TFRM_TIMER_CFG_8  VTSS_IOREG(VTSS_TO_QSYS,0x56cd)

#define VTSS_QSYS_RES_QOS_ADV_RED_PROFILE(ri)  VTSS_IOREG(VTSS_TO_QSYS,0x56ce + (ri))
#define  VTSS_F_QSYS_RES_QOS_ADV_RED_PROFILE_WM_RED_LOW(x)  VTSS_ENCODE_BITFIELD(x,11,11)
#define  VTSS_M_QSYS_RES_QOS_ADV_RED_PROFILE_WM_RED_LOW     VTSS_ENCODE_BITMASK(11,11)
#define  VTSS_X_QSYS_RES_QOS_ADV_RED_PROFILE_WM_RED_LOW(x)  VTSS_EXTRACT_BITFIELD(x,11,11)
#define  VTSS_F_QSYS_RES_QOS_ADV_RED_PROFILE_WM_RED_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,11)
#define  VTSS_M_QSYS_RES_QOS_ADV_RED_PROFILE_WM_RED_HIGH     VTSS_ENCODE_BITMASK(0,11)
#define  VTSS_X_QSYS_RES_QOS_ADV_RED_PROFILE_WM_RED_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,11)

#define VTSS_QSYS_RES_QOS_ADV_RES_QOS_MODE   VTSS_IOREG(VTSS_TO_QSYS,0x56de)
#define  VTSS_F_QSYS_RES_QOS_ADV_RES_QOS_MODE_RES_QOS_RSRVD(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_QSYS_RES_QOS_ADV_RES_QOS_MODE_RES_QOS_RSRVD     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_QSYS_RES_QOS_ADV_RES_QOS_MODE_RES_QOS_RSRVD(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

#define VTSS_QSYS_RES_CTRL_RES_CFG(gi)       VTSS_IOREG_IX(VTSS_TO_QSYS,0x5800,gi,2,0,0)
#define  VTSS_F_QSYS_RES_CTRL_RES_CFG_WM_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_QSYS_RES_CTRL_RES_CFG_WM_HIGH     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_QSYS_RES_CTRL_RES_CFG_WM_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,12)

#define VTSS_QSYS_RES_CTRL_RES_STAT(gi)      VTSS_IOREG_IX(VTSS_TO_QSYS,0x5800,gi,2,0,1)
#define  VTSS_F_QSYS_RES_CTRL_RES_STAT_INUSE(x)  VTSS_ENCODE_BITFIELD(x,15,15)
#define  VTSS_M_QSYS_RES_CTRL_RES_STAT_INUSE     VTSS_ENCODE_BITMASK(15,15)
#define  VTSS_X_QSYS_RES_CTRL_RES_STAT_INUSE(x)  VTSS_EXTRACT_BITFIELD(x,15,15)
#define  VTSS_F_QSYS_RES_CTRL_RES_STAT_MAXUSE(x)  VTSS_ENCODE_BITFIELD(x,0,15)
#define  VTSS_M_QSYS_RES_CTRL_RES_STAT_MAXUSE     VTSS_ENCODE_BITMASK(0,15)
#define  VTSS_X_QSYS_RES_CTRL_RES_STAT_MAXUSE(x)  VTSS_EXTRACT_BITFIELD(x,0,15)

#define VTSS_QSYS_DROP_CFG_EGR_DROP_MODE     VTSS_IOREG(VTSS_TO_QSYS,0x56df)
#define  VTSS_F_QSYS_DROP_CFG_EGR_DROP_MODE_EGRESS_DROP_MODE(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_QSYS_DROP_CFG_EGR_DROP_MODE_EGRESS_DROP_MODE     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_QSYS_DROP_CFG_EGR_DROP_MODE_EGRESS_DROP_MODE(x)  VTSS_EXTRACT_BITFIELD(x,0,12)

#define VTSS_QSYS_MMGT_EQ_CTRL               VTSS_IOREG(VTSS_TO_QSYS,0x56e0)
#define  VTSS_F_QSYS_MMGT_EQ_CTRL_FP_FREE_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_QSYS_MMGT_EQ_CTRL_FP_FREE_CNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_QSYS_MMGT_EQ_CTRL_FP_FREE_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_QSYS_HSCH_CIR_CFG(gi)           VTSS_IOREG_IX(VTSS_TO_QSYS,0x0,gi,32,0,0)
#define  VTSS_F_QSYS_HSCH_CIR_CFG_CIR_RATE(x)  VTSS_ENCODE_BITFIELD(x,6,15)
#define  VTSS_M_QSYS_HSCH_CIR_CFG_CIR_RATE     VTSS_ENCODE_BITMASK(6,15)
#define  VTSS_X_QSYS_HSCH_CIR_CFG_CIR_RATE(x)  VTSS_EXTRACT_BITFIELD(x,6,15)
#define  VTSS_F_QSYS_HSCH_CIR_CFG_CIR_BURST(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_QSYS_HSCH_CIR_CFG_CIR_BURST     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_QSYS_HSCH_CIR_CFG_CIR_BURST(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

#define VTSS_QSYS_HSCH_EIR_CFG(gi)           VTSS_IOREG_IX(VTSS_TO_QSYS,0x0,gi,32,0,1)
#define  VTSS_F_QSYS_HSCH_EIR_CFG_EIR_RATE(x)  VTSS_ENCODE_BITFIELD(x,7,15)
#define  VTSS_M_QSYS_HSCH_EIR_CFG_EIR_RATE     VTSS_ENCODE_BITMASK(7,15)
#define  VTSS_X_QSYS_HSCH_EIR_CFG_EIR_RATE(x)  VTSS_EXTRACT_BITFIELD(x,7,15)
#define  VTSS_F_QSYS_HSCH_EIR_CFG_EIR_BURST(x)  VTSS_ENCODE_BITFIELD(x,1,6)
#define  VTSS_M_QSYS_HSCH_EIR_CFG_EIR_BURST     VTSS_ENCODE_BITMASK(1,6)
#define  VTSS_X_QSYS_HSCH_EIR_CFG_EIR_BURST(x)  VTSS_EXTRACT_BITFIELD(x,1,6)
#define  VTSS_F_QSYS_HSCH_EIR_CFG_EIR_MARK_ENA  VTSS_BIT(0)

#define VTSS_QSYS_HSCH_SE_CFG(gi)            VTSS_IOREG_IX(VTSS_TO_QSYS,0x0,gi,32,0,2)
#define  VTSS_F_QSYS_HSCH_SE_CFG_SE_DWRR_CNT(x)  VTSS_ENCODE_BITFIELD(x,6,4)
#define  VTSS_M_QSYS_HSCH_SE_CFG_SE_DWRR_CNT     VTSS_ENCODE_BITMASK(6,4)
#define  VTSS_X_QSYS_HSCH_SE_CFG_SE_DWRR_CNT(x)  VTSS_EXTRACT_BITFIELD(x,6,4)
#define  VTSS_F_QSYS_HSCH_SE_CFG_SE_RR_ENA    VTSS_BIT(5)
#define  VTSS_F_QSYS_HSCH_SE_CFG_SE_AVB_ENA   VTSS_BIT(4)
#define  VTSS_F_QSYS_HSCH_SE_CFG_SE_FRM_MODE(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_QSYS_HSCH_SE_CFG_SE_FRM_MODE     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_QSYS_HSCH_SE_CFG_SE_FRM_MODE(x)  VTSS_EXTRACT_BITFIELD(x,2,2)
#define  VTSS_F_QSYS_HSCH_SE_CFG_SE_EXC_ENA   VTSS_BIT(1)

#define VTSS_QSYS_HSCH_SE_DWRR_CFG(gi,ri)    VTSS_IOREG_IX(VTSS_TO_QSYS,0x0,gi,32,ri,3)
#define  VTSS_F_QSYS_HSCH_SE_DWRR_CFG_DWRR_COST(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_QSYS_HSCH_SE_DWRR_CFG_DWRR_COST     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_QSYS_HSCH_SE_DWRR_CFG_DWRR_COST(x)  VTSS_EXTRACT_BITFIELD(x,0,5)

#define VTSS_QSYS_HSCH_SE_CONNECT(gi)        VTSS_IOREG_IX(VTSS_TO_QSYS,0x0,gi,32,0,15)
#define  VTSS_F_QSYS_HSCH_SE_CONNECT_SE_OUTP_IDX(x)  VTSS_ENCODE_BITFIELD(x,17,8)
#define  VTSS_M_QSYS_HSCH_SE_CONNECT_SE_OUTP_IDX     VTSS_ENCODE_BITMASK(17,8)
#define  VTSS_X_QSYS_HSCH_SE_CONNECT_SE_OUTP_IDX(x)  VTSS_EXTRACT_BITFIELD(x,17,8)
#define  VTSS_F_QSYS_HSCH_SE_CONNECT_SE_INP_IDX(x)  VTSS_ENCODE_BITFIELD(x,9,8)
#define  VTSS_M_QSYS_HSCH_SE_CONNECT_SE_INP_IDX     VTSS_ENCODE_BITMASK(9,8)
#define  VTSS_X_QSYS_HSCH_SE_CONNECT_SE_INP_IDX(x)  VTSS_EXTRACT_BITFIELD(x,9,8)
#define  VTSS_F_QSYS_HSCH_SE_CONNECT_SE_OUTP_CON(x)  VTSS_ENCODE_BITFIELD(x,5,4)
#define  VTSS_M_QSYS_HSCH_SE_CONNECT_SE_OUTP_CON     VTSS_ENCODE_BITMASK(5,4)
#define  VTSS_X_QSYS_HSCH_SE_CONNECT_SE_OUTP_CON(x)  VTSS_EXTRACT_BITFIELD(x,5,4)
#define  VTSS_F_QSYS_HSCH_SE_CONNECT_SE_INP_CNT(x)  VTSS_ENCODE_BITFIELD(x,1,4)
#define  VTSS_M_QSYS_HSCH_SE_CONNECT_SE_INP_CNT     VTSS_ENCODE_BITMASK(1,4)
#define  VTSS_X_QSYS_HSCH_SE_CONNECT_SE_INP_CNT(x)  VTSS_EXTRACT_BITFIELD(x,1,4)
#define  VTSS_F_QSYS_HSCH_SE_CONNECT_SE_TERMINAL  VTSS_BIT(0)

#define VTSS_QSYS_HSCH_SE_DLB_SENSE(gi)      VTSS_IOREG_IX(VTSS_TO_QSYS,0x0,gi,32,0,16)
#define  VTSS_F_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_PRIO(x)  VTSS_ENCODE_BITFIELD(x,11,3)
#define  VTSS_M_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_PRIO     VTSS_ENCODE_BITMASK(11,3)
#define  VTSS_X_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_PRIO(x)  VTSS_EXTRACT_BITFIELD(x,11,3)
#define  VTSS_F_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_SPORT(x)  VTSS_ENCODE_BITFIELD(x,7,4)
#define  VTSS_M_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_SPORT     VTSS_ENCODE_BITMASK(7,4)
#define  VTSS_X_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_SPORT(x)  VTSS_EXTRACT_BITFIELD(x,7,4)
#define  VTSS_F_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_DPORT(x)  VTSS_ENCODE_BITFIELD(x,3,4)
#define  VTSS_M_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_DPORT     VTSS_ENCODE_BITMASK(3,4)
#define  VTSS_X_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_DPORT(x)  VTSS_EXTRACT_BITFIELD(x,3,4)
#define  VTSS_F_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_PRIO_ENA  VTSS_BIT(2)
#define  VTSS_F_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_SPORT_ENA  VTSS_BIT(1)
#define  VTSS_F_QSYS_HSCH_SE_DLB_SENSE_SE_DLB_DPORT_ENA  VTSS_BIT(0)

#define VTSS_QSYS_HSCH_CIR_STATE(gi)         VTSS_IOREG_IX(VTSS_TO_QSYS,0x0,gi,32,0,17)
#define  VTSS_F_QSYS_HSCH_CIR_STATE_CIR_LVL(x)  VTSS_ENCODE_BITFIELD(x,4,22)
#define  VTSS_M_QSYS_HSCH_CIR_STATE_CIR_LVL     VTSS_ENCODE_BITMASK(4,22)
#define  VTSS_X_QSYS_HSCH_CIR_STATE_CIR_LVL(x)  VTSS_EXTRACT_BITFIELD(x,4,22)

#define VTSS_QSYS_HSCH_EIR_STATE(gi)         VTSS_IOREG_IX(VTSS_TO_QSYS,0x0,gi,32,0,18)
#define  VTSS_F_QSYS_HSCH_EIR_STATE_EIR_LVL(x)  VTSS_ENCODE_BITFIELD(x,0,22)
#define  VTSS_M_QSYS_HSCH_EIR_STATE_EIR_LVL     VTSS_ENCODE_BITMASK(0,22)
#define  VTSS_X_QSYS_HSCH_EIR_STATE_EIR_LVL(x)  VTSS_EXTRACT_BITFIELD(x,0,22)

#define VTSS_QSYS_HSCH_MISC_HSCH_MISC_CFG    VTSS_IOREG(VTSS_TO_QSYS,0x56e2)
#define  VTSS_F_QSYS_HSCH_MISC_HSCH_MISC_CFG_SE_CONNECT_VLD  VTSS_BIT(7)
#define  VTSS_F_QSYS_HSCH_MISC_HSCH_MISC_CFG_FRM_ADJ(x)  VTSS_ENCODE_BITFIELD(x,2,5)
#define  VTSS_M_QSYS_HSCH_MISC_HSCH_MISC_CFG_FRM_ADJ     VTSS_ENCODE_BITMASK(2,5)
#define  VTSS_X_QSYS_HSCH_MISC_HSCH_MISC_CFG_FRM_ADJ(x)  VTSS_EXTRACT_BITFIELD(x,2,5)
#define  VTSS_F_QSYS_HSCH_MISC_HSCH_MISC_CFG_QSHP_EXC_ENA  VTSS_BIT(0)


#endif /* _VTSS_SERVAL_REGS_QSYS_H_ */
