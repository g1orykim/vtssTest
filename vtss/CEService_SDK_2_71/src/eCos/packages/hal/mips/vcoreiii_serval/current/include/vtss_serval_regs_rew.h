#ifndef _VTSS_SERVAL_REGS_REW_H_
#define _VTSS_SERVAL_REGS_REW_H_

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

#define VTSS_REW_PORT_PORT_VLAN_CFG(gi)      VTSS_IOREG_IX(VTSS_TO_REW,0x0,gi,32,0,0)
#define  VTSS_F_REW_PORT_PORT_VLAN_CFG_PORT_TPID(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_REW_PORT_PORT_VLAN_CFG_PORT_TPID     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_REW_PORT_PORT_VLAN_CFG_PORT_TPID(x)  VTSS_EXTRACT_BITFIELD(x,16,16)
#define  VTSS_F_REW_PORT_PORT_VLAN_CFG_PORT_DEI  VTSS_BIT(15)
#define  VTSS_F_REW_PORT_PORT_VLAN_CFG_PORT_PCP(x)  VTSS_ENCODE_BITFIELD(x,12,3)
#define  VTSS_M_REW_PORT_PORT_VLAN_CFG_PORT_PCP     VTSS_ENCODE_BITMASK(12,3)
#define  VTSS_X_REW_PORT_PORT_VLAN_CFG_PORT_PCP(x)  VTSS_EXTRACT_BITFIELD(x,12,3)
#define  VTSS_F_REW_PORT_PORT_VLAN_CFG_PORT_VID(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_REW_PORT_PORT_VLAN_CFG_PORT_VID     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_REW_PORT_PORT_VLAN_CFG_PORT_VID(x)  VTSS_EXTRACT_BITFIELD(x,0,12)

#define VTSS_REW_PORT_TAG_CFG(gi)            VTSS_IOREG_IX(VTSS_TO_REW,0x0,gi,32,0,1)
#define  VTSS_F_REW_PORT_TAG_CFG_TAG_CFG(x)   VTSS_ENCODE_BITFIELD(x,7,2)
#define  VTSS_M_REW_PORT_TAG_CFG_TAG_CFG      VTSS_ENCODE_BITMASK(7,2)
#define  VTSS_X_REW_PORT_TAG_CFG_TAG_CFG(x)   VTSS_EXTRACT_BITFIELD(x,7,2)
#define  VTSS_F_REW_PORT_TAG_CFG_TAG_TPID_CFG(x)  VTSS_ENCODE_BITFIELD(x,5,2)
#define  VTSS_M_REW_PORT_TAG_CFG_TAG_TPID_CFG     VTSS_ENCODE_BITMASK(5,2)
#define  VTSS_X_REW_PORT_TAG_CFG_TAG_TPID_CFG(x)  VTSS_EXTRACT_BITFIELD(x,5,2)
#define  VTSS_F_REW_PORT_TAG_CFG_TAG_VID_CFG  VTSS_BIT(4)
#define  VTSS_F_REW_PORT_TAG_CFG_TAG_PCP_CFG(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_REW_PORT_TAG_CFG_TAG_PCP_CFG     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_REW_PORT_TAG_CFG_TAG_PCP_CFG(x)  VTSS_EXTRACT_BITFIELD(x,2,2)
#define  VTSS_F_REW_PORT_TAG_CFG_TAG_DEI_CFG(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_REW_PORT_TAG_CFG_TAG_DEI_CFG     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_REW_PORT_TAG_CFG_TAG_DEI_CFG(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

#define VTSS_REW_PORT_PORT_CFG(gi)           VTSS_IOREG_IX(VTSS_TO_REW,0x0,gi,32,0,2)
#define  VTSS_F_REW_PORT_PORT_CFG_ES0_ENA     VTSS_BIT(5)
#define  VTSS_F_REW_PORT_PORT_CFG_FCS_UPDATE_NONCPU_CFG(x)  VTSS_ENCODE_BITFIELD(x,3,2)
#define  VTSS_M_REW_PORT_PORT_CFG_FCS_UPDATE_NONCPU_CFG     VTSS_ENCODE_BITMASK(3,2)
#define  VTSS_X_REW_PORT_PORT_CFG_FCS_UPDATE_NONCPU_CFG(x)  VTSS_EXTRACT_BITFIELD(x,3,2)
#define  VTSS_F_REW_PORT_PORT_CFG_FCS_UPDATE_CPU_ENA  VTSS_BIT(2)
#define  VTSS_F_REW_PORT_PORT_CFG_FLUSH_ENA   VTSS_BIT(1)
#define  VTSS_F_REW_PORT_PORT_CFG_AGE_DIS     VTSS_BIT(0)

#define VTSS_REW_PORT_DSCP_CFG(gi)           VTSS_IOREG_IX(VTSS_TO_REW,0x0,gi,32,0,3)
#define  VTSS_F_REW_PORT_DSCP_CFG_DSCP_REWR_CFG(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_REW_PORT_DSCP_CFG_DSCP_REWR_CFG     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_REW_PORT_DSCP_CFG_DSCP_REWR_CFG(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

#define VTSS_REW_PORT_PCP_DEI_QOS_MAP_CFG(gi,ri)  VTSS_IOREG_IX(VTSS_TO_REW,0x0,gi,32,ri,4)
#define  VTSS_F_REW_PORT_PCP_DEI_QOS_MAP_CFG_DEI_QOS_VAL  VTSS_BIT(3)
#define  VTSS_F_REW_PORT_PCP_DEI_QOS_MAP_CFG_PCP_QOS_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_REW_PORT_PCP_DEI_QOS_MAP_CFG_PCP_QOS_VAL     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_REW_PORT_PCP_DEI_QOS_MAP_CFG_PCP_QOS_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,3)

#define VTSS_REW_PORT_PTP_CFG(gi)            VTSS_IOREG_IX(VTSS_TO_REW,0x0,gi,32,0,20)
#define  VTSS_F_REW_PORT_PTP_CFG_PTP_BACKPLANE_MODE  VTSS_BIT(7)
#define  VTSS_F_REW_PORT_PTP_CFG_PTP_1STEP_DIS  VTSS_BIT(2)
#define  VTSS_F_REW_PORT_PTP_CFG_PTP_2STEP_DIS  VTSS_BIT(1)

#define VTSS_REW_PORT_PTP_DLY1_CFG(gi)       VTSS_IOREG_IX(VTSS_TO_REW,0x0,gi,32,0,21)

#define VTSS_REW_COMMON_DSCP_REMAP_DP1_CFG(ri)  VTSS_IOREG(VTSS_TO_REW,0x1a4 + (ri))
#define  VTSS_F_REW_COMMON_DSCP_REMAP_DP1_CFG_DSCP_REMAP_DP1_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_REW_COMMON_DSCP_REMAP_DP1_CFG_DSCP_REMAP_DP1_VAL     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_REW_COMMON_DSCP_REMAP_DP1_CFG_DSCP_REMAP_DP1_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

#define VTSS_REW_COMMON_DSCP_REMAP_CFG(ri)   VTSS_IOREG(VTSS_TO_REW,0x1e4 + (ri))
#define  VTSS_F_REW_COMMON_DSCP_REMAP_CFG_DSCP_REMAP_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_REW_COMMON_DSCP_REMAP_CFG_DSCP_REMAP_VAL     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_REW_COMMON_DSCP_REMAP_CFG_DSCP_REMAP_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

#define VTSS_REW_COMMON_STAT_CFG             VTSS_IOREG(VTSS_TO_REW,0x224)
#define  VTSS_F_REW_COMMON_STAT_CFG_STAT_MODE(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_REW_COMMON_STAT_CFG_STAT_MODE     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_REW_COMMON_STAT_CFG_STAT_MODE(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

#define VTSS_REW_PPT_PPT(ri)                 VTSS_IOREG(VTSS_TO_REW,0x1a0 + (ri))


#endif /* _VTSS_SERVAL_REGS_REW_H_ */
