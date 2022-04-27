#ifndef _VTSS_JAGUAR2_REGS_ANA_ACL_H_
#define _VTSS_JAGUAR2_REGS_ANA_ACL_H_

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

#define VTSS_ANA_ACL_PORT_VCAP_S2_KEY_SEL(gi,ri)  VTSS_IOREG_IX(VTSS_TO_ANA_ACL,0x1000,gi,2,ri,0)
#define  VTSS_F_ANA_ACL_PORT_VCAP_S2_KEY_SEL_IP4_MC_KEY_SEL(x)  VTSS_ENCODE_BITFIELD(x,6,2)
#define  VTSS_M_ANA_ACL_PORT_VCAP_S2_KEY_SEL_IP4_MC_KEY_SEL     VTSS_ENCODE_BITMASK(6,2)
#define  VTSS_X_ANA_ACL_PORT_VCAP_S2_KEY_SEL_IP4_MC_KEY_SEL(x)  VTSS_EXTRACT_BITFIELD(x,6,2)
#define  VTSS_F_ANA_ACL_PORT_VCAP_S2_KEY_SEL_IP4_UC_KEY_SEL(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_ANA_ACL_PORT_VCAP_S2_KEY_SEL_IP4_UC_KEY_SEL     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_ANA_ACL_PORT_VCAP_S2_KEY_SEL_IP4_UC_KEY_SEL(x)  VTSS_EXTRACT_BITFIELD(x,4,2)
#define  VTSS_F_ANA_ACL_PORT_VCAP_S2_KEY_SEL_IP6_MC_KEY_SEL(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_ANA_ACL_PORT_VCAP_S2_KEY_SEL_IP6_MC_KEY_SEL     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_ANA_ACL_PORT_VCAP_S2_KEY_SEL_IP6_MC_KEY_SEL(x)  VTSS_EXTRACT_BITFIELD(x,2,2)
#define  VTSS_F_ANA_ACL_PORT_VCAP_S2_KEY_SEL_IP6_UC_KEY_SEL  VTSS_BIT(1)
#define  VTSS_F_ANA_ACL_PORT_VCAP_S2_KEY_SEL_ARP_KEY_SEL  VTSS_BIT(0)

#define VTSS_ANA_ACL_VCAP_S2_VCAP_S2_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_ACL,0x1072 + (ri))
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_ROUTE_HANDLING_ENA  VTSS_BIT(26)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_OAM_ENA(x)  VTSS_ENCODE_BITFIELD(x,24,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_OAM_ENA     VTSS_ENCODE_BITMASK(24,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_OAM_ENA(x)  VTSS_EXTRACT_BITFIELD(x,24,2)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_TCPUDP_OTHER_ENA(x)  VTSS_ENCODE_BITFIELD(x,22,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_TCPUDP_OTHER_ENA     VTSS_ENCODE_BITMASK(22,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_TCPUDP_OTHER_ENA(x)  VTSS_EXTRACT_BITFIELD(x,22,2)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_VID_ENA(x)  VTSS_ENCODE_BITFIELD(x,20,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_VID_ENA     VTSS_ENCODE_BITMASK(20,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_VID_ENA(x)  VTSS_EXTRACT_BITFIELD(x,20,2)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_STD_ENA(x)  VTSS_ENCODE_BITFIELD(x,18,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_STD_ENA     VTSS_ENCODE_BITMASK(18,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_STD_ENA(x)  VTSS_EXTRACT_BITFIELD(x,18,2)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_TCPUDP_ENA(x)  VTSS_ENCODE_BITFIELD(x,16,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_TCPUDP_ENA     VTSS_ENCODE_BITMASK(16,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_TCPUDP_ENA(x)  VTSS_EXTRACT_BITFIELD(x,16,2)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_OTHER_ENA(x)  VTSS_ENCODE_BITFIELD(x,14,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_OTHER_ENA     VTSS_ENCODE_BITMASK(14,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_OTHER_ENA(x)  VTSS_EXTRACT_BITFIELD(x,14,2)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP4_VID_ENA(x)  VTSS_ENCODE_BITFIELD(x,12,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP4_VID_ENA     VTSS_ENCODE_BITMASK(12,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP4_VID_ENA(x)  VTSS_EXTRACT_BITFIELD(x,12,2)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP4_TCPUDP_ENA(x)  VTSS_ENCODE_BITFIELD(x,10,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP4_TCPUDP_ENA     VTSS_ENCODE_BITMASK(10,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP4_TCPUDP_ENA(x)  VTSS_EXTRACT_BITFIELD(x,10,2)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP4_OTHER_ENA(x)  VTSS_ENCODE_BITFIELD(x,8,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP4_OTHER_ENA     VTSS_ENCODE_BITMASK(8,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP4_OTHER_ENA(x)  VTSS_EXTRACT_BITFIELD(x,8,2)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_ARP_ENA(x)  VTSS_ENCODE_BITFIELD(x,6,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_ARP_ENA     VTSS_ENCODE_BITMASK(6,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_ARP_ENA(x)  VTSS_EXTRACT_BITFIELD(x,6,2)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_MAC_SNAP_ENA(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_MAC_SNAP_ENA     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_MAC_SNAP_ENA(x)  VTSS_EXTRACT_BITFIELD(x,4,2)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_MAC_LLC_ENA(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_MAC_LLC_ENA     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_MAC_LLC_ENA(x)  VTSS_EXTRACT_BITFIELD(x,2,2)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_ENA     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

#define VTSS_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL  VTSS_IOREG(VTSS_TO_ANA_ACL,0x10ab)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_VLAN_PIPELINE_ACT_ENA  VTSS_BIT(13)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_ACL_RT_IGR_RLEG_STAT_MODE  VTSS_BIT(12)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_ACL_RT_EGR_RLEG_STAT_MODE  VTSS_BIT(11)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_ACL_RT_FORCE_ES0_VID_ENA  VTSS_BIT(10)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_ACL_RT_UPDATE_CL_VID_ENA  VTSS_BIT(9)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_ACL_RT_UPDATE_GEN_IDX_ERLEG_ENA  VTSS_BIT(8)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_ACL_RT_UPDATE_GEN_IDX_EVID_ENA  VTSS_BIT(7)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_ACL_RT_SEL(x)  VTSS_ENCODE_BITFIELD(x,5,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_ACL_RT_SEL     VTSS_ENCODE_BITMASK(5,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_ACL_RT_SEL(x)  VTSS_EXTRACT_BITFIELD(x,5,2)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_LBK_IGR_MASK_SEL3_ENA  VTSS_BIT(4)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_MASQ_IGR_MASK_ENA  VTSS_BIT(3)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_FP_VS2_IGR_MASK_ENA  VTSS_BIT(2)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_VD_IGR_MASK_ENA  VTSS_BIT(1)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_CPU_IGR_MASK_ENA  VTSS_BIT(0)

#define VTSS_ANA_ACL_VCAP_S2_VCAP_S2_RNG_CTRL(ri)  VTSS_IOREG(VTSS_TO_ANA_ACL,0x10ac + (ri))
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_RNG_CTRL_RNG_TYPE_SEL(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_RNG_CTRL_RNG_TYPE_SEL     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_RNG_CTRL_RNG_TYPE_SEL(x)  VTSS_EXTRACT_BITFIELD(x,0,3)

#define VTSS_ANA_ACL_VCAP_S2_VCAP_S2_RNG_VALUE_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_ACL,0x10b4 + (ri))
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_RNG_VALUE_CFG_RNG_MAX_VALUE(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_RNG_VALUE_CFG_RNG_MAX_VALUE     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_RNG_VALUE_CFG_RNG_MAX_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,16,16)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_RNG_VALUE_CFG_RNG_MIN_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_RNG_VALUE_CFG_RNG_MIN_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_RNG_VALUE_CFG_RNG_MIN_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

#define VTSS_ANA_ACL_VCAP_S2_VCAP_S2_RNG_OFFSET_CFG  VTSS_IOREG(VTSS_TO_ANA_ACL,0x10bc)
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_RNG_OFFSET_CFG_RNG_OFFSET_POS(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_RNG_OFFSET_CFG_RNG_OFFSET_POS     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_RNG_OFFSET_CFG_RNG_OFFSET_POS(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

#define VTSS_ANA_ACL_STICKY_SEC_LOOKUP_STICKY(ri)  VTSS_IOREG(VTSS_TO_ANA_ACL,0x10bd + (ri))
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_CUSTOM2_STICKY  VTSS_BIT(13)
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_CUSTOM1_STICKY  VTSS_BIT(12)
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_OAM_STICKY  VTSS_BIT(11)
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_IP6_VID_STICKY  VTSS_BIT(10)
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_IP6_STD_STICKY  VTSS_BIT(9)
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_IP6_TCPUDP_STICKY  VTSS_BIT(8)
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_IP6_OTHER_STICKY  VTSS_BIT(7)
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_IP4_VID_STICKY  VTSS_BIT(6)
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_IP4_TCPUDP_STICKY  VTSS_BIT(5)
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_IP4_OTHER_STICKY  VTSS_BIT(4)
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_ARP_STICKY  VTSS_BIT(3)
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_MAC_SNAP_STICKY  VTSS_BIT(2)
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_MAC_LLC_STICKY  VTSS_BIT(1)
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_MAC_ETYPE_STICKY  VTSS_BIT(0)

#define VTSS_ANA_ACL_CNT_TBL_CNT(gi)         VTSS_IOREG_IX(VTSS_TO_ANA_ACL,0x0,gi,1,0,0)


#endif /* _VTSS_JAGUAR2_REGS_ANA_ACL_H_ */
