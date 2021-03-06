#ifndef _VTSS_JAGUAR2_REGS_ANA_L3_H_
#define _VTSS_JAGUAR2_REGS_ANA_L3_H_

/*

 Vitesse Switch API software.

 Copyright (c) 2002-2013 Vitesse Semiconductor Corporation "Vitesse". All
 Rights Reserved.

 Unpublished rights reserved under the copyright laws of the United States of
 America, other countries and international treaties. Permission to use, copy,
 store and modify, the software and its source code is granted. Permission to
 integrate into other products, disclose, transmit and distribute the software
 in an absolute machine readable format (e.g. HEX file) is also granted.  The
 source code of the software may not be disclosed, transmitted or distributed
 without the written permission of Vitesse. The software and its source code
 may only be used in products utilizing the Vitesse switch products.

 This copyright notice must appear in any copy, modification, disclosure,
 transmission or distribution of the software. Vitesse retains all ownership,
 copyright, trade secret and proprietary rights in the software.

 THIS SOFTWARE HAS BEEN PROVIDED "AS IS," WITHOUT EXPRESS OR IMPLIED WARRANTY
 INCLUDING, WITHOUT LIMITATION, IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR USE AND NON-INFRINGEMENT.

*/

#include "vtss_jaguar2_regs_common.h"

/*********************************************************************** 
 *
 * Target: \a ANA_L3
 *
 * ANA_L3 provides the following functionality:
 * - VLAN handling
 * - MSTP handling
 * - IP routing
 *
 ***********************************************************************/

/**
 * Register Group: \a ANA_L3:COMMON
 *
 * Common configurations for all ports
 */


/** 
 * \brief Miscellanous control parameters
 *
 * \details
 * Register: \a ANA_L3:COMMON:MISC_CTRL
 */
#define VTSS_ANA_L3_COMMON_MISC_CTRL         VTSS_IOREG(VTSS_TO_ANA_L3,0xd508)

/** 
 * \brief
 * Enable update of AC for routed frames.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_COMMON_MISC_CTRL . AC_UPDATE_ENA
 */
#define  VTSS_F_ANA_L3_COMMON_MISC_CTRL_AC_UPDATE_ENA  VTSS_BIT(0)


/** 
 * \brief VLAN Control
 *
 * \details
 * Basic VLAN related configuration
 *
 * Register: \a ANA_L3:COMMON:VLAN_CTRL
 */
#define VTSS_ANA_L3_COMMON_VLAN_CTRL         VTSS_IOREG(VTSS_TO_ANA_L3,0xd509)

/** 
 * \brief
 * Enable/disable VLAN lookup. 
 * This field must be enabled to allow VLAN and MSTP filtering. For VLAN
 * unaware operation, this field can be disabled.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_COMMON_VLAN_CTRL . VLAN_ENA
 */
#define  VTSS_F_ANA_L3_COMMON_VLAN_CTRL_VLAN_ENA  VTSS_BIT(0)


/** 
 * \brief Unicast routing control configuration
 *
 * \details
 * Enable of L3 unicast traffic per port
 *
 * Register: \a ANA_L3:COMMON:L3_UC_ENA
 */
#define VTSS_ANA_L3_COMMON_L3_UC_ENA         VTSS_IOREG(VTSS_TO_ANA_L3,0xd50a)


/** 
 * \brief Unicast routing control configuration
 *
 * \details
 * Enable of L3 unicast traffic per port
 *
 * Register: \a ANA_L3:COMMON:L3_UC_ENA1
 */
#define VTSS_ANA_L3_COMMON_L3_UC_ENA1        VTSS_IOREG(VTSS_TO_ANA_L3,0xd50b)

/** 
 * \brief
 * Pls. refer to L3_UC_ENA.L3_UC_ENA description.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_COMMON_L3_UC_ENA1 . L3_UC_ENA1
 */
#define  VTSS_F_ANA_L3_COMMON_L3_UC_ENA1_L3_UC_ENA1(x)  VTSS_ENCODE_BITFIELD(x,0,21)
#define  VTSS_M_ANA_L3_COMMON_L3_UC_ENA1_L3_UC_ENA1     VTSS_ENCODE_BITMASK(0,21)
#define  VTSS_X_ANA_L3_COMMON_L3_UC_ENA1_L3_UC_ENA1(x)  VTSS_EXTRACT_BITFIELD(x,0,21)


/** 
 * \brief Multicast routing control configuration
 *
 * \details
 * IP multicast traffic enable per port
 *
 * Register: \a ANA_L3:COMMON:L3_MC_ENA
 */
#define VTSS_ANA_L3_COMMON_L3_MC_ENA         VTSS_IOREG(VTSS_TO_ANA_L3,0xd50c)


/** 
 * \brief Multicast routing control Configuration
 *
 * \details
 * IP multicast traffic enable per port
 *
 * Register: \a ANA_L3:COMMON:L3_MC_ENA1
 */
#define VTSS_ANA_L3_COMMON_L3_MC_ENA1        VTSS_IOREG(VTSS_TO_ANA_L3,0xd50d)

/** 
 * \brief
 * Pls. refer to L3_MC_ENA.L3_MC_ENA description.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_COMMON_L3_MC_ENA1 . L3_MC_ENA1
 */
#define  VTSS_F_ANA_L3_COMMON_L3_MC_ENA1_L3_MC_ENA1(x)  VTSS_ENCODE_BITFIELD(x,0,21)
#define  VTSS_M_ANA_L3_COMMON_L3_MC_ENA1_L3_MC_ENA1     VTSS_ENCODE_BITMASK(0,21)
#define  VTSS_X_ANA_L3_COMMON_L3_MC_ENA1_L3_MC_ENA1(x)  VTSS_EXTRACT_BITFIELD(x,0,21)


/** 
 * \brief Forwarding Control
 *
 * \details
 * Configuration of forwarding state per port
 *
 * Register: \a ANA_L3:COMMON:PORT_FWD_CTRL
 */
#define VTSS_ANA_L3_COMMON_PORT_FWD_CTRL     VTSS_IOREG(VTSS_TO_ANA_L3,0xd50e)


/** 
 * \brief Forwarding Control
 *
 * \details
 * Configuration of forwarding state per port
 *
 * Register: \a ANA_L3:COMMON:PORT_FWD_CTRL1
 */
#define VTSS_ANA_L3_COMMON_PORT_FWD_CTRL1    VTSS_IOREG(VTSS_TO_ANA_L3,0xd50f)

/** 
 * \brief
 * Pls. refer to PORT_FWD_CTRL.PORT_FWD_ENA description.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_COMMON_PORT_FWD_CTRL1 . PORT_FWD_ENA1
 */
#define  VTSS_F_ANA_L3_COMMON_PORT_FWD_CTRL1_PORT_FWD_ENA1(x)  VTSS_ENCODE_BITFIELD(x,0,21)
#define  VTSS_M_ANA_L3_COMMON_PORT_FWD_CTRL1_PORT_FWD_ENA1     VTSS_ENCODE_BITMASK(0,21)
#define  VTSS_X_ANA_L3_COMMON_PORT_FWD_CTRL1_PORT_FWD_ENA1(x)  VTSS_EXTRACT_BITFIELD(x,0,21)


/** 
 * \brief Learning Control
 *
 * \details
 * Configuration of learning state per port
 *
 * Register: \a ANA_L3:COMMON:PORT_LRN_CTRL
 */
#define VTSS_ANA_L3_COMMON_PORT_LRN_CTRL     VTSS_IOREG(VTSS_TO_ANA_L3,0xd510)


/** 
 * \brief Learning Control
 *
 * \details
 * Configuration of learning state per port
 *
 * Register: \a ANA_L3:COMMON:PORT_LRN_CTRL1
 */
#define VTSS_ANA_L3_COMMON_PORT_LRN_CTRL1    VTSS_IOREG(VTSS_TO_ANA_L3,0xd511)

/** 
 * \brief
 * Pls. refer to PORT_LRN_CTRL.PORT_LRN_ENA description.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_COMMON_PORT_LRN_CTRL1 . PORT_LRN_ENA1
 */
#define  VTSS_F_ANA_L3_COMMON_PORT_LRN_CTRL1_PORT_LRN_ENA1(x)  VTSS_ENCODE_BITFIELD(x,0,21)
#define  VTSS_M_ANA_L3_COMMON_PORT_LRN_CTRL1_PORT_LRN_ENA1     VTSS_ENCODE_BITMASK(0,21)
#define  VTSS_X_ANA_L3_COMMON_PORT_LRN_CTRL1_PORT_LRN_ENA1(x)  VTSS_EXTRACT_BITFIELD(x,0,21)


/** 
 * \brief VLAN Filter Control
 *
 * \details
 * Configuration of VLAN ingress filtering per port
 *
 * Register: \a ANA_L3:COMMON:VLAN_FILTER_CTRL
 */
#define VTSS_ANA_L3_COMMON_VLAN_FILTER_CTRL  VTSS_IOREG(VTSS_TO_ANA_L3,0xd512)


/** 
 * \brief VLAN Filter Control
 *
 * \details
 * Configuration of VLAN ingress filtering per port
 *
 * Register: \a ANA_L3:COMMON:VLAN_FILTER_CTRL1
 */
#define VTSS_ANA_L3_COMMON_VLAN_FILTER_CTRL1  VTSS_IOREG(VTSS_TO_ANA_L3,0xd513)

/** 
 * \brief
 * Pls. refer to VLAN_FILTER_CTRL.VLAN_IGR_FILTER_ENA description.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_COMMON_VLAN_FILTER_CTRL1 . VLAN_IGR_FILTER_ENA1
 */
#define  VTSS_F_ANA_L3_COMMON_VLAN_FILTER_CTRL1_VLAN_IGR_FILTER_ENA1(x)  VTSS_ENCODE_BITFIELD(x,0,21)
#define  VTSS_M_ANA_L3_COMMON_VLAN_FILTER_CTRL1_VLAN_IGR_FILTER_ENA1     VTSS_ENCODE_BITMASK(0,21)
#define  VTSS_X_ANA_L3_COMMON_VLAN_FILTER_CTRL1_VLAN_IGR_FILTER_ENA1(x)  VTSS_EXTRACT_BITFIELD(x,0,21)


/** 
 * \brief VLAN isolated port mask
 *
 * \details
 * Configuration of isolated port mask. See description of
 * VLAN:VLAN_CFG.VLAN_PRIVATE_ENA
 *
 * Register: \a ANA_L3:COMMON:VLAN_ISOLATED_CFG
 */
#define VTSS_ANA_L3_COMMON_VLAN_ISOLATED_CFG  VTSS_IOREG(VTSS_TO_ANA_L3,0xd514)


/** 
 * \brief VLAN isolated port mask
 *
 * \details
 * Configuration of isolated port mask. See description of
 * VLAN:VLAN_CFG.VLAN_PRIVATE_ENA
 *
 * Register: \a ANA_L3:COMMON:VLAN_ISOLATED_CFG1
 */
#define VTSS_ANA_L3_COMMON_VLAN_ISOLATED_CFG1  VTSS_IOREG(VTSS_TO_ANA_L3,0xd515)

/** 
 * \brief
 * Pls. refer to VLAN_ISOLATED_CFG.VLAN_ISOLATED_MASK description.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_COMMON_VLAN_ISOLATED_CFG1 . VLAN_ISOLATED_MASK1
 */
#define  VTSS_F_ANA_L3_COMMON_VLAN_ISOLATED_CFG1_VLAN_ISOLATED_MASK1(x)  VTSS_ENCODE_BITFIELD(x,0,21)
#define  VTSS_M_ANA_L3_COMMON_VLAN_ISOLATED_CFG1_VLAN_ISOLATED_MASK1     VTSS_ENCODE_BITMASK(0,21)
#define  VTSS_X_ANA_L3_COMMON_VLAN_ISOLATED_CFG1_VLAN_ISOLATED_MASK1(x)  VTSS_EXTRACT_BITFIELD(x,0,21)


/** 
 * \brief VLAN Community port mask
 *
 * \details
 * Configuration of Community port mask. See description of
 * VLAN:VLAN_CFG.VLAN_PRIVATE_ENA
 *
 * Register: \a ANA_L3:COMMON:VLAN_COMMUNITY_CFG
 */
#define VTSS_ANA_L3_COMMON_VLAN_COMMUNITY_CFG  VTSS_IOREG(VTSS_TO_ANA_L3,0xd516)


/** 
 * \brief VLAN Community port mask
 *
 * \details
 * Configuration of Community port mask. See description of
 * VLAN:VLAN_CFG.VLAN_PRIVATE_ENA
 *
 * Register: \a ANA_L3:COMMON:VLAN_COMMUNITY_CFG1
 */
#define VTSS_ANA_L3_COMMON_VLAN_COMMUNITY_CFG1  VTSS_IOREG(VTSS_TO_ANA_L3,0xd517)

/** 
 * \brief
 * Pls. refer to VLAN_COMMUNITY_CFG.VLAN_COMMUNITY_MASK description.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_COMMON_VLAN_COMMUNITY_CFG1 . VLAN_COMMUNITY_MASK1
 */
#define  VTSS_F_ANA_L3_COMMON_VLAN_COMMUNITY_CFG1_VLAN_COMMUNITY_MASK1(x)  VTSS_ENCODE_BITFIELD(x,0,21)
#define  VTSS_M_ANA_L3_COMMON_VLAN_COMMUNITY_CFG1_VLAN_COMMUNITY_MASK1     VTSS_ENCODE_BITMASK(0,21)
#define  VTSS_X_ANA_L3_COMMON_VLAN_COMMUNITY_CFG1_VLAN_COMMUNITY_MASK1(x)  VTSS_EXTRACT_BITFIELD(x,0,21)


/** 
 * \brief Routing Configuration
 *
 * \details
 * Configuration of routing checks.
 * Note that these checks only applies to frames matching a router leg.
 *
 * Register: \a ANA_L3:COMMON:ROUTING_CFG
 */
#define VTSS_ANA_L3_COMMON_ROUTING_CFG       VTSS_IOREG(VTSS_TO_ANA_L3,0xd518)

/** 
 * \brief
 * Change SMAC of routed frame to the SMAC of egress router leg.
 * 
 * Normally SMAC is changed in REW, but if routing and PTP is to be
 * supported concurrently , then RT_SMAC_UPDATE_ENA must be set.
 * 
 * When RT_SMAC_UPDATE_ENA is set and ingress mirroring is used, then
 * ingress mirrored frames will have SMAC and DMAC values corresponding to
 * the routed frame.
 * 
 * Related parameters:
 * ANA_ACL::VCAP_S2_MISC_CTRL.ACL_RT_SEL
 * ANA_ACL::VCAP_S2_MISC_CTRL.ACL_RT_UPDATE_CL_VID_ENA
 *
 * \details 
 * Field: ::VTSS_ANA_L3_COMMON_ROUTING_CFG . RT_SMAC_UPDATE_ENA
 */
#define  VTSS_F_ANA_L3_COMMON_ROUTING_CFG_RT_SMAC_UPDATE_ENA  VTSS_BIT(26)

/** 
 * \brief
 * Control which non-IP unicast frames, matching router leg's MAC address,
 * that shall be redirected to CPU.
 * 
 * CPU queue is configured in CPU_QU_CFG.CPU_RLEG_QU.
 *
 * \details 
 * 0: Redirect all frames.
 * 1: Redirect ARP frames (Ethertype 0x0806)
 * 2: Redirect RARP frames (Ethertype 0x8035)
 * 3: Redirect ARP and RARP frames.
 *
 * Field: ::VTSS_ANA_L3_COMMON_ROUTING_CFG . RLEG_NONIP_UC_REDIR_MODE
 */
#define  VTSS_F_ANA_L3_COMMON_ROUTING_CFG_RLEG_NONIP_UC_REDIR_MODE(x)  VTSS_ENCODE_BITFIELD(x,24,2)
#define  VTSS_M_ANA_L3_COMMON_ROUTING_CFG_RLEG_NONIP_UC_REDIR_MODE     VTSS_ENCODE_BITMASK(24,2)
#define  VTSS_X_ANA_L3_COMMON_ROUTING_CFG_RLEG_NONIP_UC_REDIR_MODE(x)  VTSS_EXTRACT_BITFIELD(x,24,2)

/** 
 * \brief
 * Control handling of IPv6 frames which are otherwise to be L3 forwarded
 * but 
 * have Payload Length > VMID:MAX_LEN.IP6_LEN_MAX  for egress router leg.
 * 
 * CPU queue is configured in CPU_QU_CFG.CPU_IP_LEN_QU
 *
 * \details 
 * 0: Drop frame
 * 1: Redirect frame to CPU
 *
 * Field: ::VTSS_ANA_L3_COMMON_ROUTING_CFG . IP6_LEN_REDIR
 */
#define  VTSS_F_ANA_L3_COMMON_ROUTING_CFG_IP6_LEN_REDIR  VTSS_BIT(22)

/** 
 * \brief
 * Control handling of IPv4 frames which are otherwise to be L3 forwarded
 * but 
 * have Total Length > VMID:MAX_LEN.IP4_LEN_MAX for egress router leg.
 * 
 * CPU queue is configured in CPU_QU_CFG.CPU_IP_LEN_QU
 *
 * \details 
 * 0: Drop frame
 * 1: Redirect frame to CPU
 *
 * Field: ::VTSS_ANA_L3_COMMON_ROUTING_CFG . IP4_LEN_REDIR
 */
#define  VTSS_F_ANA_L3_COMMON_ROUTING_CFG_IP4_LEN_REDIR  VTSS_BIT(21)

/** 
 * \brief
 * Copy IPv6 frames with broadcast DMAC to CPU.
 * 
 * CPU queue is configured in CPU_QU_CFG.CPU_RLEG_QU.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_COMMON_ROUTING_CFG . IP6_L2_BC_COPY_ENA
 */
#define  VTSS_F_ANA_L3_COMMON_ROUTING_CFG_IP6_L2_BC_COPY_ENA  VTSS_BIT(20)

/** 
 * \brief
 * Copy IPv4 frames with broadcast DMAC to CPU.
 * 
 * CPU queue is configured in CPU_QU_CFG.CPU_RLEG_QU.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_COMMON_ROUTING_CFG . IP4_L2_BC_COPY_ENA
 */
#define  VTSS_F_ANA_L3_COMMON_ROUTING_CFG_IP4_L2_BC_COPY_ENA  VTSS_BIT(19)

/** 
 * \brief
 * Redirect IPv6 frames failing SIP RPF check to CPU.
 * 
 * CPU queue is configured in CPU_QU_CFG.CPU_SIP_RPF_QU.
 * 
 * Related parameters:
 * VMID:RLEG_CTRL.RLEG_IP6_SIP_RPF_MODE
 * COMMON:CPU_QU_CFG.CPU_SIP_RPF_QU

 *
 * \details 
 * Field: ::VTSS_ANA_L3_COMMON_ROUTING_CFG . RLEG_IP6_SIP_RPF_REDIR_ENA
 */
#define  VTSS_F_ANA_L3_COMMON_ROUTING_CFG_RLEG_IP6_SIP_RPF_REDIR_ENA  VTSS_BIT(18)

/** 
 * \brief
 * Redirect IPv4 frames failing SIP RPF check to CPU.
 * 
 * CPU queue is configured in CPU_QU_CFG.CPU_SIP_RPF_QU.
 * 
 * Related parameters:
 * VMID:RLEG_CTRL.RLEG_IP4_SIP_RPF_MODE
 * COMMON:CPU_QU_CFG.CPU_SIP_RPF_QU
 *
 * \details 
 * Field: ::VTSS_ANA_L3_COMMON_ROUTING_CFG . RLEG_IP4_SIP_RPF_REDIR_ENA
 */
#define  VTSS_F_ANA_L3_COMMON_ROUTING_CFG_RLEG_IP4_SIP_RPF_REDIR_ENA  VTSS_BIT(17)

/** 
 * \brief
 * Enable DIP checks for IPv6 packets to be routed. 
 * 
 * If enabled and the frame's DIP falls within the given range then the
 * frame is not routed.
 * 
 * Bit 0: ::/128 (Unspecified address)
 * Bit 1: ::1/128 (Loopback address)
 * 
 * Frames which are not routed due to DIP check can be redirected to CPU by
 * setting CPU_RLEG_IP_HDR_FAIL_REDIR_ENA=1.

 *
 * \details 
 * '0': 
 * Frame is allowed to be routed.
 * 
 * '1': 
 * Frame is not routed. 
 * Frame is redirected to CPU if CPU_RLEG_IP_HDR_FAIL_REDIR_ENA=1.
 *
 * Field: ::VTSS_ANA_L3_COMMON_ROUTING_CFG . IP6_DIP_ADDR_VIOLATION_REDIR_ENA
 */
#define  VTSS_F_ANA_L3_COMMON_ROUTING_CFG_IP6_DIP_ADDR_VIOLATION_REDIR_ENA(x)  VTSS_ENCODE_BITFIELD(x,15,2)
#define  VTSS_M_ANA_L3_COMMON_ROUTING_CFG_IP6_DIP_ADDR_VIOLATION_REDIR_ENA     VTSS_ENCODE_BITMASK(15,2)
#define  VTSS_X_ANA_L3_COMMON_ROUTING_CFG_IP6_DIP_ADDR_VIOLATION_REDIR_ENA(x)  VTSS_EXTRACT_BITFIELD(x,15,2)

/** 
 * \brief
 * Enable DIP checks for IPv4 packets matching a router leg.
 * 
 * If enabled and the frame's DIP falls within the given range then the
 * frame is not routed.
 * 
 * Bit 0: 0.0.0.0 - 0.255.255.255
 * Bit 1: 127.0.0.0 - 127.255.255.255 (Loopback network)
 * Bit 2: 240.0.0.0 - 255.255.255.254 (Experimental)
 * 
 * Frames which are not routed due to DIP check can be redirected to CPU by
 * setting CPU_RLEG_IP_HDR_FAIL_REDIR_ENA=1.
 *
 * \details 
 * '0': 
 * Frame is allowed to be routed.
 * 
 * '1': 
 * Frame is not routed. 
 * Frame is redirected to CPU if CPU_RLEG_IP_HDR_FAIL_REDIR_ENA=1.
 *
 * Field: ::VTSS_ANA_L3_COMMON_ROUTING_CFG . IP4_DIP_ADDR_VIOLATION_REDIR_ENA
 */
#define  VTSS_F_ANA_L3_COMMON_ROUTING_CFG_IP4_DIP_ADDR_VIOLATION_REDIR_ENA(x)  VTSS_ENCODE_BITFIELD(x,11,3)
#define  VTSS_M_ANA_L3_COMMON_ROUTING_CFG_IP4_DIP_ADDR_VIOLATION_REDIR_ENA     VTSS_ENCODE_BITMASK(11,3)
#define  VTSS_X_ANA_L3_COMMON_ROUTING_CFG_IP4_DIP_ADDR_VIOLATION_REDIR_ENA(x)  VTSS_EXTRACT_BITFIELD(x,11,3)

/** 
 * \brief
 * Enable SIP checks for IPv6 packets to be routed. 
 * 
 * If enabled and the frame's SIP falls within the given range then the
 * frame is not routed.
 * 
 * Bit 0: ::/128 (Unspecified address)
 * Bit 1: ::1/128 (Loopback address)
 * Bit 2: ff00::/8 (IPv6 multicast addresses)
 * 
 * Frames which are not routed due to SIP check can be redirected to CPU by
 * setting CPU_RLEG_IP_HDR_FAIL_REDIR_ENA=1.
 *
 * \details 
 * '0': 
 * Frame is allowed to be routed.
 * 
 * '1': 
 * Frame is not routed. 
 * Frame is redirected to CPU if CPU_RLEG_IP_HDR_FAIL_REDIR_ENA=1.
 *
 * Field: ::VTSS_ANA_L3_COMMON_ROUTING_CFG . IP6_SIP_ADDR_VIOLATION_REDIR_ENA
 */
#define  VTSS_F_ANA_L3_COMMON_ROUTING_CFG_IP6_SIP_ADDR_VIOLATION_REDIR_ENA(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_ANA_L3_COMMON_ROUTING_CFG_IP6_SIP_ADDR_VIOLATION_REDIR_ENA     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_ANA_L3_COMMON_ROUTING_CFG_IP6_SIP_ADDR_VIOLATION_REDIR_ENA(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * Enable SIP checks for IPv4 packets matching a router leg.
 * 
 * If enabled and the frame's SIP falls within the given range then the
 * frame is not routed.
 * 
 * Bit 0: 0.0.0.0 - 0.255.255.255 
 * Bit 1: 127.0.0.0 - 127.255.255.255 (Loopback network)
 * Bit 2: 224.0.0.0 - 255.255.255.255 (Multicast/experimental/broadcast)
 * 
 * Frames which are not routed due to SIP check can be redirected to CPU by
 * setting CPU_RLEG_IP_HDR_FAIL_REDIR_ENA=1.

 *
 * \details 
 * '0': 
 * Frame is allowed to be routed.
 * 
 * '1': 
 * Frame is not routed. 
 * Frame is redirected to CPU if CPU_RLEG_IP_HDR_FAIL_REDIR_ENA=1.
 *
 * Field: ::VTSS_ANA_L3_COMMON_ROUTING_CFG . IP4_SIP_ADDR_VIOLATION_REDIR_ENA
 */
#define  VTSS_F_ANA_L3_COMMON_ROUTING_CFG_IP4_SIP_ADDR_VIOLATION_REDIR_ENA(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_ANA_L3_COMMON_ROUTING_CFG_IP4_SIP_ADDR_VIOLATION_REDIR_ENA     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_ANA_L3_COMMON_ROUTING_CFG_IP4_SIP_ADDR_VIOLATION_REDIR_ENA(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * Control CPU redirection of IP error frames matching a router leg.
 * 
 * CPU queue is configured in CPU_QU_CFG.CPU_RLEG_IP_HDR_FAIL_QU.
 * 
 * The following errors are covered:
 * IPv4 header length error
 * IPv4 header checksum error
 * SIP violations, if enabled in IP4_SIP_ADDR_VIOLATION_REDIR_ENA /
 * IP6_SIP_ADDR_VIOLATION_REDIR_ENA.
 * DIP violations, if enabled in IP4_DIP_ADDR_VIOLATION_REDIR_ENA /
 * IP4_DIP_ADDR_VIOLATION_REDIR_ENA.
 * IP MC frames with unicast DMAC.
 * 
 * Related parameters:
 * VLAN_ARP_L3MC_STICKY:L3_ARP_IPMC_STICKY.RLEG_UC_HDR_ERR_REDIR_STICKY
 * VLAN_ARP_L3MC_STICKY:L3_ARP_IPMC_STICKY.RLEG_MC_HDR_ERR_REDIR_STICKY
 *
 * \details 
 * 0: Disable redirection (errored frames are discarded)
 * 1: Enable redirection to CPU queue
 *
 * Field: ::VTSS_ANA_L3_COMMON_ROUTING_CFG . CPU_RLEG_IP_HDR_FAIL_REDIR_ENA
 */
#define  VTSS_F_ANA_L3_COMMON_ROUTING_CFG_CPU_RLEG_IP_HDR_FAIL_REDIR_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Enable redirection to CPU of IPv6 frames with Hop-by-Hop options.
 * 
 * CPU queue is configured in CPU_QU_CFG.CPU_RLEG_IP_OPT_QU.
 * 
 * Packets with Hob-by-Hop options are not subject to routing.

 *
 * \details 
 * Field: ::VTSS_ANA_L3_COMMON_ROUTING_CFG . CPU_IP6_HOPBYHOP_REDIR_ENA
 */
#define  VTSS_F_ANA_L3_COMMON_ROUTING_CFG_CPU_IP6_HOPBYHOP_REDIR_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Enable redirection to CPU of IPv4 frames with IP4 options.
 * 
 * CPU queue is configured in CPU_QU_CFG.CPU_RLEG_IP_OPT_QU.
 * 
 * Packets with IP options are not subject to routing.
 * 
 * Related parameters:
 * CPU_QU_CFG.CPU_RLEG_IP_OPT_QU
 * VLAN_ARP_L3MC_STICKY:L3_ARP_IPMC_STICKY.RLEG_MC_IP_OPT_REDIR_STICKY
 *
 * \details 
 * Field: ::VTSS_ANA_L3_COMMON_ROUTING_CFG . CPU_IP4_OPTIONS_REDIR_ENA
 */
#define  VTSS_F_ANA_L3_COMMON_ROUTING_CFG_CPU_IP4_OPTIONS_REDIR_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Enable redirection to CPU of IPv6 UC packets, which match a router leg 
 * and have Hop Limit less than 2.
 * 
 * CPU queue is configured in CPU_QU_CFG.CPU_IP_TTL_FAIL_QU.
 *
 * \details 
 * 0: Disable redirection (errored frames are discarded)
 * 1: Enable redirection to CPU queue
 *
 * Field: ::VTSS_ANA_L3_COMMON_ROUTING_CFG . IP6_HC_REDIR_ENA
 */
#define  VTSS_F_ANA_L3_COMMON_ROUTING_CFG_IP6_HC_REDIR_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable redirection to CPU of IPv4 UC packets, which match a router leg 
 * and have TTL less than 2.
 * 
 * CPU queue is configured in CPU_QU_CFG.CPU_IP_TTL_FAIL_QU.
 *
 * \details 
 * 0: Disable redirection (errored frames are discarded)
 * 1: Enable redirection to CPU queue
 *
 * Field: ::VTSS_ANA_L3_COMMON_ROUTING_CFG . IP4_TTL_REDIR_ENA
 */
#define  VTSS_F_ANA_L3_COMMON_ROUTING_CFG_IP4_TTL_REDIR_ENA  VTSS_BIT(0)


/** 
 * \brief Router leg base MAC address
 *
 * \details
 * Configuration of router leg base MAC address.
 *
 * Register: \a ANA_L3:COMMON:RLEG_CFG_0
 */
#define VTSS_ANA_L3_COMMON_RLEG_CFG_0        VTSS_IOREG(VTSS_TO_ANA_L3,0xd519)

/** 
 * \brief
 * Router leg base MAC address, least significant bits.
 * 
 * In order to have different MAC addresses per router leg, the base
 * address may be incremented using VID or VMID, ref. RLEG_MAC_TYPE_SEL.
 * 
 * This must be configured consistently in REW::RLEG_CFG_0.RLEG_MAC_LSB.
 *
 * \details 
 * Bit 0: MAC address, bit 0
 * ...
 * Bit 23: MAC address, bit 23
 *
 * Field: ::VTSS_ANA_L3_COMMON_RLEG_CFG_0 . RLEG_MAC_LSB
 */
#define  VTSS_F_ANA_L3_COMMON_RLEG_CFG_0_RLEG_MAC_LSB(x)  VTSS_ENCODE_BITFIELD(x,8,24)
#define  VTSS_M_ANA_L3_COMMON_RLEG_CFG_0_RLEG_MAC_LSB     VTSS_ENCODE_BITMASK(8,24)
#define  VTSS_X_ANA_L3_COMMON_RLEG_CFG_0_RLEG_MAC_LSB(x)  VTSS_EXTRACT_BITFIELD(x,8,24)


/** 
 * \brief Router Leg base MAC address
 *
 * \details
 * Configuration of router leg base MAC address.
 *
 * Register: \a ANA_L3:COMMON:RLEG_CFG_1
 */
#define VTSS_ANA_L3_COMMON_RLEG_CFG_1        VTSS_IOREG(VTSS_TO_ANA_L3,0xd51a)

/** 
 * \brief
 * Configuration of router leg specific MAC address.
 * 
 * This must be configured consistently in
 * REW::RLEG_CFG_1.RLEG_MAC_TYPE_SEL.
 *
 * \details 
 * 0: VMID used to increment base MAC address:
 * RLEG_MAC = RLEG_MAC_MSB[23:0] . ((RLEG_MAC_LSB[23:0] + VMID[7:0]) mod
 * 2**24)
 * 
 * 1: VID used to increment base MAC address:
 * RLEG_MAC = RLEG_MAC_MSB[23:0] . ((RLEG_MAC_LSB[23:0] + VID[11:0]) mod
 * 2**24)
 * 
 * 2: Base MAC address used for all router legs
 * RLEG_MAC = RLEG_MAC_MSB[23:0] . RLEG_MAC_LSB[23:0] 
 * 
 * 3: Reserved
 * 
 * 3: Reserved.
 *
 * Field: ::VTSS_ANA_L3_COMMON_RLEG_CFG_1 . RLEG_MAC_TYPE_SEL
 */
#define  VTSS_F_ANA_L3_COMMON_RLEG_CFG_1_RLEG_MAC_TYPE_SEL(x)  VTSS_ENCODE_BITFIELD(x,24,2)
#define  VTSS_M_ANA_L3_COMMON_RLEG_CFG_1_RLEG_MAC_TYPE_SEL     VTSS_ENCODE_BITMASK(24,2)
#define  VTSS_X_ANA_L3_COMMON_RLEG_CFG_1_RLEG_MAC_TYPE_SEL(x)  VTSS_EXTRACT_BITFIELD(x,24,2)

/** 
 * \brief
 * Router leg base MAC address, least significant bits.
 * 
 * In order to have different MAC addresses per router leg, the base
 * address may be incremented using VID or VMID, ref. RLEG_MAC_TYPE_SEL.
 * 
 * This must be configured consistently in REW::RLEG_CFG_1.RLEG_MAC_MSB.
 *
 * \details 
 * Bit 0: MAC address, bit 24
 * ...
 * Bit 23: MAC address, bit 47
 *
 * Field: ::VTSS_ANA_L3_COMMON_RLEG_CFG_1 . RLEG_MAC_MSB
 */
#define  VTSS_F_ANA_L3_COMMON_RLEG_CFG_1_RLEG_MAC_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_ANA_L3_COMMON_RLEG_CFG_1_RLEG_MAC_MSB     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_ANA_L3_COMMON_RLEG_CFG_1_RLEG_MAC_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief CPU Queue
 *
 * \details
 * Configuration of CPU queues relevant for routing
 *
 * Register: \a ANA_L3:COMMON:CPU_QU_CFG
 */
#define VTSS_ANA_L3_COMMON_CPU_QU_CFG        VTSS_IOREG(VTSS_TO_ANA_L3,0xd51b)

/** 
 * \brief
 * CPU queue number for non-IP unicast frames matching an ingress router
 * leg, e.g. ARP PDUs.
 * CPU queue for IP frames with L2 broadcast DMAC, received by router leg.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_L3_COMMON_CPU_QU_CFG . CPU_RLEG_QU
 */
#define  VTSS_F_ANA_L3_COMMON_CPU_QU_CFG_CPU_RLEG_QU(x)  VTSS_ENCODE_BITFIELD(x,28,3)
#define  VTSS_M_ANA_L3_COMMON_CPU_QU_CFG_CPU_RLEG_QU     VTSS_ENCODE_BITMASK(28,3)
#define  VTSS_X_ANA_L3_COMMON_CPU_QU_CFG_CPU_RLEG_QU(x)  VTSS_EXTRACT_BITFIELD(x,28,3)

/** 
 * \brief
 * CPU queue number for IPv4 frames with options and IPv6 frames with
 * Hop-by-Hop option. 
 * 
 * Redirection of such frames is enabled using
 * ROUTING_CFG.CPU_IP4_OPTIONS_REDIR_ENA
 * ROUTING_CFG.CPU_IP6_HOPBYHOP_REDIR_ENA
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_L3_COMMON_CPU_QU_CFG . CPU_RLEG_IP_OPT_QU
 */
#define  VTSS_F_ANA_L3_COMMON_CPU_QU_CFG_CPU_RLEG_IP_OPT_QU(x)  VTSS_ENCODE_BITFIELD(x,24,3)
#define  VTSS_M_ANA_L3_COMMON_CPU_QU_CFG_CPU_RLEG_IP_OPT_QU     VTSS_ENCODE_BITMASK(24,3)
#define  VTSS_X_ANA_L3_COMMON_CPU_QU_CFG_CPU_RLEG_IP_OPT_QU(x)  VTSS_EXTRACT_BITFIELD(x,24,3)

/** 
 * \brief
 * CPU queue number for IPv4 frames with IP header errors. 
 * 
 * Redirection of such frames is enabled using
 * ROUTING_CFG.CPU_RLEG_IP_HDR_FAIL_REDIR_ENA
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_L3_COMMON_CPU_QU_CFG . CPU_RLEG_IP_HDR_FAIL_QU
 */
#define  VTSS_F_ANA_L3_COMMON_CPU_QU_CFG_CPU_RLEG_IP_HDR_FAIL_QU(x)  VTSS_ENCODE_BITFIELD(x,20,3)
#define  VTSS_M_ANA_L3_COMMON_CPU_QU_CFG_CPU_RLEG_IP_HDR_FAIL_QU     VTSS_ENCODE_BITMASK(20,3)
#define  VTSS_X_ANA_L3_COMMON_CPU_QU_CFG_CPU_RLEG_IP_HDR_FAIL_QU(x)  VTSS_EXTRACT_BITFIELD(x,20,3)

/** 
 * \brief
 * CPU queue for frames failing SIP RPF check.
 * 
 * Related parameters:
 * COMMON:ROUTING_CFG.RLEG_IP4_SIP_RPF_REDIR_ENA
 * COMMON:ROUTING_CFG.RLEG_IP6_SIP_RPF_REDIR_ENA
 * VMID:RLEG_CTRL.RLEG_IP4_SIP_RPF_MODE
 * VMID:RLEG_CTRL.RLEG_IP6_SIP_RPF_MODE
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_L3_COMMON_CPU_QU_CFG . CPU_SIP_RPF_QU
 */
#define  VTSS_F_ANA_L3_COMMON_CPU_QU_CFG_CPU_SIP_RPF_QU(x)  VTSS_ENCODE_BITFIELD(x,16,3)
#define  VTSS_M_ANA_L3_COMMON_CPU_QU_CFG_CPU_SIP_RPF_QU     VTSS_ENCODE_BITMASK(16,3)
#define  VTSS_X_ANA_L3_COMMON_CPU_QU_CFG_CPU_SIP_RPF_QU(x)  VTSS_EXTRACT_BITFIELD(x,16,3)

/** 
 * \brief
 * CPU queue for IPv4/IPv6 frames failing MTU check.
 * 
 * Related parameters:
 * COMMON:ROUTING_CFG.IP4_LEN_REDIR
 * COMMON:ROUTING_CFG.IP6_LEN_REDIR
 * VMID:MAX_LEN.IP4_MAX_LEN
 * VMID:MAX_LEN.IP6_MAX_LEN

 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_L3_COMMON_CPU_QU_CFG . CPU_IP_LEN_QU
 */
#define  VTSS_F_ANA_L3_COMMON_CPU_QU_CFG_CPU_IP_LEN_QU(x)  VTSS_ENCODE_BITFIELD(x,12,3)
#define  VTSS_M_ANA_L3_COMMON_CPU_QU_CFG_CPU_IP_LEN_QU     VTSS_ENCODE_BITMASK(12,3)
#define  VTSS_X_ANA_L3_COMMON_CPU_QU_CFG_CPU_IP_LEN_QU(x)  VTSS_EXTRACT_BITFIELD(x,12,3)

/** 
 * \brief
 * CPU queue for failed IPv4/IPv6 multicast lookup or failed RPF check.
 * 
 * Related enable bits:
 * L3MC:L3MC_CTRL.RPF_CHK_ENA

 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_L3_COMMON_CPU_QU_CFG . CPU_MC_FAIL_QU
 */
#define  VTSS_F_ANA_L3_COMMON_CPU_QU_CFG_CPU_MC_FAIL_QU(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_ANA_L3_COMMON_CPU_QU_CFG_CPU_MC_FAIL_QU     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_ANA_L3_COMMON_CPU_QU_CFG_CPU_MC_FAIL_QU(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * CPU queue for failed IPv4/IPv6 unicast LPM lookup, invalid ARP entry
 * (ARP_ENA=0) or failed ICMP redirect check.
 * 
 * Related enable bits:
 * VMID.RLEG_CTRL.RLEG_IP4_ICMP_REDIR_ENA
 * VMID.RLEG_CTRL.RLEG_IP6_ICMP_REDIR_ENA
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_L3_COMMON_CPU_QU_CFG . CPU_UC_FAIL_QU
 */
#define  VTSS_F_ANA_L3_COMMON_CPU_QU_CFG_CPU_UC_FAIL_QU(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_ANA_L3_COMMON_CPU_QU_CFG_CPU_UC_FAIL_QU     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_ANA_L3_COMMON_CPU_QU_CFG_CPU_UC_FAIL_QU(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * CPU queue for IP unicast frames with a TTL/HL <2 and with successful ARP
 * lookup yielding a non-zero DMAC.
 * CPU queue for IP multicast frames with a TTL/HL <2 and with successful
 * VCAP lookup.
 * 
 * Related enable bits:
 * COMMON:ROUTING_CFG.IP4_TTL_REDIR_ENA
 * COMMON:ROUTING_CFG.IP6_HC_REDIR_ENA
 * L3MC:L3MC_CTRL.IPMC_TTL_COPY_ENA
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_L3_COMMON_CPU_QU_CFG . CPU_IP_TTL_FAIL_QU
 */
#define  VTSS_F_ANA_L3_COMMON_CPU_QU_CFG_CPU_IP_TTL_FAIL_QU(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_ANA_L3_COMMON_CPU_QU_CFG_CPU_IP_TTL_FAIL_QU     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_ANA_L3_COMMON_CPU_QU_CFG_CPU_IP_TTL_FAIL_QU(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief IPv4 Virtual Router Leg Configuration
 *
 * \details
 * Configuration of VRRP MAC address for IPv4.
 * 
 * Use of VRRP for IPv4 is enabled per router leg in 
 * VMID:RLEG_CTRL.RLEG_IP4_VRID_ENA
 *
 * Register: \a ANA_L3:COMMON:VRRP_IP4_CFG_0
 */
#define VTSS_ANA_L3_COMMON_VRRP_IP4_CFG_0    VTSS_IOREG(VTSS_TO_ANA_L3,0xd51c)

/** 
 * \brief
 * Mid part of IPv4 Virtual Router Redundancy Protocol MAC address.  

 *
 * \details 
 * IPv4 VRRP MAC address Bit 31 downto 8.
 *
 * Field: ::VTSS_ANA_L3_COMMON_VRRP_IP4_CFG_0 . VRRP_IP4_BASE_MAC_MID
 */
#define  VTSS_F_ANA_L3_COMMON_VRRP_IP4_CFG_0_VRRP_IP4_BASE_MAC_MID(x)  VTSS_ENCODE_BITFIELD(x,8,24)
#define  VTSS_M_ANA_L3_COMMON_VRRP_IP4_CFG_0_VRRP_IP4_BASE_MAC_MID     VTSS_ENCODE_BITMASK(8,24)
#define  VTSS_X_ANA_L3_COMMON_VRRP_IP4_CFG_0_VRRP_IP4_BASE_MAC_MID(x)  VTSS_EXTRACT_BITFIELD(x,8,24)


/** 
 * \brief IPv4 Virtual Router Leg Configuration
 *
 * \details
 * Configuration of VRRP MAC address for IPv4.
 * 
 * Use of VRRP for IPv4 is enabled per router leg in 
 * VMID:RLEG_CTRL.RLEG_IP4_VRID_ENA
 *
 * Register: \a ANA_L3:COMMON:VRRP_IP4_CFG_1
 */
#define VTSS_ANA_L3_COMMON_VRRP_IP4_CFG_1    VTSS_IOREG(VTSS_TO_ANA_L3,0xd51d)

/** 
 * \brief
 * Upper part of IPv4 Virtual Router Redundancy Protocol MAC address.  
 *
 * \details 
 * IPv4 VRRP MAC address Bit 47 downto 32.
 *
 * Field: ::VTSS_ANA_L3_COMMON_VRRP_IP4_CFG_1 . VRRP_IP4_BASE_MAC_HIGH
 */
#define  VTSS_F_ANA_L3_COMMON_VRRP_IP4_CFG_1_VRRP_IP4_BASE_MAC_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_ANA_L3_COMMON_VRRP_IP4_CFG_1_VRRP_IP4_BASE_MAC_HIGH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_ANA_L3_COMMON_VRRP_IP4_CFG_1_VRRP_IP4_BASE_MAC_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief IPv6 Virtual Router Leg Configuration
 *
 * \details
 * Configuration of VRRP MAC address for IPv6.
 * 
 * Use of VRRP for IPv6 is enabled per router leg in 
 * VMID:RLEG_CTRL.RLEG_IP6_VRID_ENA
 *
 * Register: \a ANA_L3:COMMON:VRRP_IP6_CFG_0
 */
#define VTSS_ANA_L3_COMMON_VRRP_IP6_CFG_0    VTSS_IOREG(VTSS_TO_ANA_L3,0xd51e)

/** 
 * \brief
 * Mid part of IPv6 Virtual Router Redundancy Protocol MAC address.  
 *
 * \details 
 * IPv6 VRRP MAC address Bit 31 downto 8.
 *
 * Field: ::VTSS_ANA_L3_COMMON_VRRP_IP6_CFG_0 . VRRP_IP6_BASE_MAC_MID
 */
#define  VTSS_F_ANA_L3_COMMON_VRRP_IP6_CFG_0_VRRP_IP6_BASE_MAC_MID(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_ANA_L3_COMMON_VRRP_IP6_CFG_0_VRRP_IP6_BASE_MAC_MID     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_ANA_L3_COMMON_VRRP_IP6_CFG_0_VRRP_IP6_BASE_MAC_MID(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief IPv6 Virtual Router Leg Configuration
 *
 * \details
 * Configuration of VRRP MAC address for IPv6.
 * 
 * Use of VRRP for IPv6 is enabled per router leg in 
 * VMID:RLEG_CTRL.RLEG_IP6_VRID_ENA
 *
 * Register: \a ANA_L3:COMMON:VRRP_IP6_CFG_1
 */
#define VTSS_ANA_L3_COMMON_VRRP_IP6_CFG_1    VTSS_IOREG(VTSS_TO_ANA_L3,0xd51f)

/** 
 * \brief
 * Upper part of IPv6 Virtual Router Redundancy Protocol MAC address.  
 *
 * \details 
 * IPv6 VRRP MAC address Bit 47 downto 32.
 *
 * Field: ::VTSS_ANA_L3_COMMON_VRRP_IP6_CFG_1 . VRRP_IP6_BASE_MAC_HIGH
 */
#define  VTSS_F_ANA_L3_COMMON_VRRP_IP6_CFG_1_VRRP_IP6_BASE_MAC_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_ANA_L3_COMMON_VRRP_IP6_CFG_1_VRRP_IP6_BASE_MAC_HIGH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_ANA_L3_COMMON_VRRP_IP6_CFG_1_VRRP_IP6_BASE_MAC_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Enable (SIP,SMAC) / (SIP,VMID) check
 *
 * \details
 * Bit per port that enables (SIP, SMAC) and/or (SIP, VMID) check.
 *
 * Register: \a ANA_L3:COMMON:SIP_SECURE_ENA
 */
#define VTSS_ANA_L3_COMMON_SIP_SECURE_ENA    VTSS_IOREG(VTSS_TO_ANA_L3,0xd520)


/** 
 * \brief Enable (SIP,SMAC) / (SIP,VMID) check
 *
 * \details
 * Bit per port that enables (SIP, SMAC) and/or (SIP, VMID) check.
 *
 * Register: \a ANA_L3:COMMON:SIP_SECURE_ENA1
 */
#define VTSS_ANA_L3_COMMON_SIP_SECURE_ENA1   VTSS_IOREG(VTSS_TO_ANA_L3,0xd521)

/** 
 * \brief
 * Pls. refer to SIP_SECURE_ENA.SIP_CMP_ENA description.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_COMMON_SIP_SECURE_ENA1 . SIP_CMP_ENA1
 */
#define  VTSS_F_ANA_L3_COMMON_SIP_SECURE_ENA1_SIP_CMP_ENA1(x)  VTSS_ENCODE_BITFIELD(x,0,21)
#define  VTSS_M_ANA_L3_COMMON_SIP_SECURE_ENA1_SIP_CMP_ENA1     VTSS_ENCODE_BITMASK(0,21)
#define  VTSS_X_ANA_L3_COMMON_SIP_SECURE_ENA1_SIP_CMP_ENA1(x)  VTSS_EXTRACT_BITFIELD(x,0,21)


/** 
 * \brief Enable (DIP,SMAC) / (DIP,VMID) check
 *
 * \details
 * Bit per port that enables (DIP, DMAC) and/or (DIP, VMID) check.
 *
 * Register: \a ANA_L3:COMMON:DIP_SECURE_ENA
 */
#define VTSS_ANA_L3_COMMON_DIP_SECURE_ENA    VTSS_IOREG(VTSS_TO_ANA_L3,0xd522)


/** 
 * \brief Enable (DIP,SMAC) / (DIP,VMID) check
 *
 * \details
 * Bit per port that enables (DIP, DMAC) and/or (DIP, VMID) check.
 *
 * Register: \a ANA_L3:COMMON:DIP_SECURE_ENA1
 */
#define VTSS_ANA_L3_COMMON_DIP_SECURE_ENA1   VTSS_IOREG(VTSS_TO_ANA_L3,0xd523)

/** 
 * \brief
 * Pls. refer to DIP_SECURE_ENA.DIP_CMP_ENA description.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_COMMON_DIP_SECURE_ENA1 . DIP_CMP_ENA1
 */
#define  VTSS_F_ANA_L3_COMMON_DIP_SECURE_ENA1_DIP_CMP_ENA1(x)  VTSS_ENCODE_BITFIELD(x,0,21)
#define  VTSS_M_ANA_L3_COMMON_DIP_SECURE_ENA1_DIP_CMP_ENA1     VTSS_ENCODE_BITMASK(0,21)
#define  VTSS_X_ANA_L3_COMMON_DIP_SECURE_ENA1_DIP_CMP_ENA1(x)  VTSS_EXTRACT_BITFIELD(x,0,21)


/** 
 * \brief Enable SIP RPF check
 *
 * \details
 * Bit per port that enables SIP RPF check.
 *
 * Register: \a ANA_L3:COMMON:SIP_RPF_ENA
 */
#define VTSS_ANA_L3_COMMON_SIP_RPF_ENA       VTSS_IOREG(VTSS_TO_ANA_L3,0xd524)


/** 
 * \brief Enable SIP RPF check
 *
 * \details
 * Bit per port that enables SIP RPF check.
 *
 * Register: \a ANA_L3:COMMON:SIP_RPF_ENA1
 */
#define VTSS_ANA_L3_COMMON_SIP_RPF_ENA1      VTSS_IOREG(VTSS_TO_ANA_L3,0xd525)

/** 
 * \brief
 * Enable SIP RPF check per ingress port.
 * 
 * For more information, refer to 
 * VMID:RLEG_CTRL.RLEG_IP4_SIP_RPF_MODE
 * VMID:RLEG_CTRL.RLEG_IP6_SIP_RPF_MODE
 *
 * \details 
 * Field: ::VTSS_ANA_L3_COMMON_SIP_RPF_ENA1 . SIP_RPF_ENA1
 */
#define  VTSS_F_ANA_L3_COMMON_SIP_RPF_ENA1_SIP_RPF_ENA1(x)  VTSS_ENCODE_BITFIELD(x,0,21)
#define  VTSS_M_ANA_L3_COMMON_SIP_RPF_ENA1_SIP_RPF_ENA1     VTSS_ENCODE_BITMASK(0,21)
#define  VTSS_X_ANA_L3_COMMON_SIP_RPF_ENA1_SIP_RPF_ENA1(x)  VTSS_EXTRACT_BITFIELD(x,0,21)


/** 
 * \brief Service Control
 *
 * \details
 * Miscellaneous service configuration.
 *
 * Register: \a ANA_L3:COMMON:SERVICE_CFG
 */
#define VTSS_ANA_L3_COMMON_SERVICE_CFG       VTSS_IOREG(VTSS_TO_ANA_L3,0xd526)

/** 
 * \brief
 * Force Egress FID=VLAN.VLAN_CFG.VLAN_FID when ISDX > 0 and frame has
 * multicast DMAC.
 * 
 * See also SERVICE_CFG.VSI_FORCE_MC_EFID_ENA.

 *
 * \details 
 * 0: Normal EFID behaviour for multicast  DMAC.
 * 1: Force EFID=VLAN.VLAN_CFG.VLAN_FIDfor multicast DMAC when ISDX > 0.
 *
 * Field: ::VTSS_ANA_L3_COMMON_SERVICE_CFG . VSI_FORCE_MC_EFID_ENA
 */
#define  VTSS_F_ANA_L3_COMMON_SERVICE_CFG_VSI_FORCE_MC_EFID_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Force Egress FID=VLAN.VLAN_CFG.VLAN_FID when
 * ANA_CL:IPT:VSI_CFG.VSI_ENA==1 and frame has multicast DMAC.
 * 
 * See also SERVICE_CFG.VSI_FORCE_MC_EFID_ENA.

 *
 * \details 
 * 0: Normal EFID behaviour for multicast  DMAC.
 * 1: Force EFID=VLAN.VLAN_CFG.VLAN_FID when VSI_ENA==1 and frame has
 * multicast DMAC.
 *
 * Field: ::VTSS_ANA_L3_COMMON_SERVICE_CFG . ISDX_FORCE_MC_EFID_ENA
 */
#define  VTSS_F_ANA_L3_COMMON_SERVICE_CFG_ISDX_FORCE_MC_EFID_ENA  VTSS_BIT(0)

/**
 * Register Group: \a ANA_L3:TUPE
 *
 * Table UPdata Engine (TUPE)
 */


/** 
 * \brief Miscellaneous TUPE parameters
 *
 * \details
 * Register: \a ANA_L3:TUPE:TUPE_MISC
 */
#define VTSS_ANA_L3_TUPE_TUPE_MISC           VTSS_IOREG(VTSS_TO_ANA_L3,0xd527)

/** 
 * \brief
 * Start TUPE.
 * 
 * Write 1 to start TUPE.
 * Set to 0 by TUPE when done.
 * 
 * Note:
 * While TUPE is running (i.e. TUPE_START=1) CPU must not write to VLAN
 * table.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_TUPE_TUPE_MISC . TUPE_START
 */
#define  VTSS_F_ANA_L3_TUPE_TUPE_MISC_TUPE_START  VTSS_BIT(0)

/** 
 * \brief
 * Enable use of TUPE_CTRL_VAL and TUPE_CTRL_VAL_MASK.
 * 
 * For further information refer to
 * COMMON:TUPE_CTRL_VAL.TUPE_CTRL_VAL
 *
 * \details 
 * Field: ::VTSS_ANA_L3_TUPE_TUPE_MISC . TUPE_CTRL_VAL_ENA
 */
#define  VTSS_F_ANA_L3_TUPE_TUPE_MISC_TUPE_CTRL_VAL_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable use of TUPE_CTRL_BIT_MASK.
 * 
 * For further information refer to
 * COMMON:TUPE_CTRL_BIT_MASK.TUPE_CTRL_BIT_MASK
 *
 * \details 
 * Field: ::VTSS_ANA_L3_TUPE_TUPE_MISC . TUPE_CTRL_BIT_ENA
 */
#define  VTSS_F_ANA_L3_TUPE_TUPE_MISC_TUPE_CTRL_BIT_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Enable use of TUPE_PORT_MASK_A.
 * 
 * For further information refer to
 * COMMON:TUPE_PORT_MASK_A.TUPE_PORT_MASK_A
 *
 * \details 
 * Field: ::VTSS_ANA_L3_TUPE_TUPE_MISC . TUPE_PORT_MASK_A_ENA
 */
#define  VTSS_F_ANA_L3_TUPE_TUPE_MISC_TUPE_PORT_MASK_A_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Enable use of TUPE_PORT_MASK_B
 * 
 * For further information refer to
 * COMMON:TUPE_PORT_MASK_B.TUPE_PORT_MASK_B
 *
 * \details 
 * Field: ::VTSS_ANA_L3_TUPE_TUPE_MISC . TUPE_PORT_MASK_B_ENA
 */
#define  VTSS_F_ANA_L3_TUPE_TUPE_MISC_TUPE_PORT_MASK_B_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Enable combined use of TUPE_CTRL_BIT_MASK and TUPE_PORT_MASK_A.
 * 
 * For further information refer to
 * COMMON:TUPE_CTRL_BIT_MASK.TUPE_CTRL_BIT_MASK
 *
 * \details 
 * Field: ::VTSS_ANA_L3_TUPE_TUPE_MISC . TUPE_COMB_MASK_ENA
 */
#define  VTSS_F_ANA_L3_TUPE_TUPE_MISC_TUPE_COMB_MASK_ENA  VTSS_BIT(5)


/** 
 * \details
 * Register: \a ANA_L3:TUPE:TUPE_ADDR
 */
#define VTSS_ANA_L3_TUPE_TUPE_ADDR           VTSS_IOREG(VTSS_TO_ANA_L3,0xd528)

/** 
 * \brief
 * First address in VLAN table for TUPE to process.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_TUPE_TUPE_ADDR . TUPE_START_ADDR
 */
#define  VTSS_F_ANA_L3_TUPE_TUPE_ADDR_TUPE_START_ADDR(x)  VTSS_ENCODE_BITFIELD(x,0,13)
#define  VTSS_M_ANA_L3_TUPE_TUPE_ADDR_TUPE_START_ADDR     VTSS_ENCODE_BITMASK(0,13)
#define  VTSS_X_ANA_L3_TUPE_TUPE_ADDR_TUPE_START_ADDR(x)  VTSS_EXTRACT_BITFIELD(x,0,13)

/** 
 * \brief
 * Last address in VLAN table for TUPE to process.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_TUPE_TUPE_ADDR . TUPE_END_ADDR
 */
#define  VTSS_F_ANA_L3_TUPE_TUPE_ADDR_TUPE_END_ADDR(x)  VTSS_ENCODE_BITFIELD(x,16,13)
#define  VTSS_M_ANA_L3_TUPE_TUPE_ADDR_TUPE_END_ADDR     VTSS_ENCODE_BITMASK(16,13)
#define  VTSS_X_ANA_L3_TUPE_TUPE_ADDR_TUPE_END_ADDR(x)  VTSS_EXTRACT_BITFIELD(x,16,13)


/** 
 * \brief TUPE command: Port mask bits to clear
 *
 * \details
 * Register: \a ANA_L3:TUPE:TUPE_CMD_PORT_MASK_CLR
 */
#define VTSS_ANA_L3_TUPE_TUPE_CMD_PORT_MASK_CLR  VTSS_IOREG(VTSS_TO_ANA_L3,0xd529)


/** 
 * \brief TUPE command: Port mask bits to clear
 *
 * \details
 * Register: \a ANA_L3:TUPE:TUPE_CMD_PORT_MASK_CLR1
 */
#define VTSS_ANA_L3_TUPE_TUPE_CMD_PORT_MASK_CLR1  VTSS_IOREG(VTSS_TO_ANA_L3,0xd52a)

/** 
 * \brief
 * See TUPE_CMD_PORT_MASK_CLR.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_TUPE_TUPE_CMD_PORT_MASK_CLR1 . TUPE_CMD_PORT_MASK_CLR1
 */
#define  VTSS_F_ANA_L3_TUPE_TUPE_CMD_PORT_MASK_CLR1_TUPE_CMD_PORT_MASK_CLR1(x)  VTSS_ENCODE_BITFIELD(x,0,21)
#define  VTSS_M_ANA_L3_TUPE_TUPE_CMD_PORT_MASK_CLR1_TUPE_CMD_PORT_MASK_CLR1     VTSS_ENCODE_BITMASK(0,21)
#define  VTSS_X_ANA_L3_TUPE_TUPE_CMD_PORT_MASK_CLR1_TUPE_CMD_PORT_MASK_CLR1(x)  VTSS_EXTRACT_BITFIELD(x,0,21)


/** 
 * \brief TUPE command: Port mask bits to set
 *
 * \details
 * Register: \a ANA_L3:TUPE:TUPE_CMD_PORT_MASK_SET
 */
#define VTSS_ANA_L3_TUPE_TUPE_CMD_PORT_MASK_SET  VTSS_IOREG(VTSS_TO_ANA_L3,0xd52b)


/** 
 * \brief TUPE command: Port mask bits to set
 *
 * \details
 * Register: \a ANA_L3:TUPE:TUPE_CMD_PORT_MASK_SET1
 */
#define VTSS_ANA_L3_TUPE_TUPE_CMD_PORT_MASK_SET1  VTSS_IOREG(VTSS_TO_ANA_L3,0xd52c)

/** 
 * \brief
 * See TUPE_CMD_PORT_MASK_SET.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_TUPE_TUPE_CMD_PORT_MASK_SET1 . TUPE_CMD_PORT_MASK_SET1
 */
#define  VTSS_F_ANA_L3_TUPE_TUPE_CMD_PORT_MASK_SET1_TUPE_CMD_PORT_MASK_SET1(x)  VTSS_ENCODE_BITFIELD(x,0,21)
#define  VTSS_M_ANA_L3_TUPE_TUPE_CMD_PORT_MASK_SET1_TUPE_CMD_PORT_MASK_SET1     VTSS_ENCODE_BITMASK(0,21)
#define  VTSS_X_ANA_L3_TUPE_TUPE_CMD_PORT_MASK_SET1_TUPE_CMD_PORT_MASK_SET1(x)  VTSS_EXTRACT_BITFIELD(x,0,21)


/** 
 * \brief TUPE parameter controlling which VLAN table entries to update
 *
 * \details
 * Register: \a ANA_L3:TUPE:TUPE_CTRL_VAL
 */
#define VTSS_ANA_L3_TUPE_TUPE_CTRL_VAL       VTSS_IOREG(VTSS_TO_ANA_L3,0xd52d)

/** 
 * \brief
 * If enabled by TUPE_CTRL_VAL_ENA, then the following condition must be
 * met for TUPE to update VLAN table entry:
 * 
 * (VLAN:TUPE_CTRL & TUPE_CTRL_VAL_MASK) == TUPE_CTRL_VAL
 *
 * \details 
 * Field: ::VTSS_ANA_L3_TUPE_TUPE_CTRL_VAL . TUPE_CTRL_VAL
 */
#define  VTSS_F_ANA_L3_TUPE_TUPE_CTRL_VAL_TUPE_CTRL_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_ANA_L3_TUPE_TUPE_CTRL_VAL_TUPE_CTRL_VAL     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_ANA_L3_TUPE_TUPE_CTRL_VAL_TUPE_CTRL_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief TUPE parameter controlling which VLAN table entries to update
 *
 * \details
 * Register: \a ANA_L3:TUPE:TUPE_CTRL_VAL_MASK
 */
#define VTSS_ANA_L3_TUPE_TUPE_CTRL_VAL_MASK  VTSS_IOREG(VTSS_TO_ANA_L3,0xd52e)

/** 
 * \brief
 * See TUPE_CTRL_VAL.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_TUPE_TUPE_CTRL_VAL_MASK . TUPE_CTRL_VAL_MASK
 */
#define  VTSS_F_ANA_L3_TUPE_TUPE_CTRL_VAL_MASK_TUPE_CTRL_VAL_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_ANA_L3_TUPE_TUPE_CTRL_VAL_MASK_TUPE_CTRL_VAL_MASK     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_ANA_L3_TUPE_TUPE_CTRL_VAL_MASK_TUPE_CTRL_VAL_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief TUPE parameter controlling which VLAN table entries to update
 *
 * \details
 * Register: \a ANA_L3:TUPE:TUPE_CTRL_BIT_MASK
 */
#define VTSS_ANA_L3_TUPE_TUPE_CTRL_BIT_MASK  VTSS_IOREG(VTSS_TO_ANA_L3,0xd52f)

/** 
 * \brief
 * If enabled by TUPE_CTRL_BIT_ENA, then the following condition must be
 * met for TUPE to update VLAN table entry:
 * 
 * (VLAN:TUPE_CTRL & TUPE_CTRL_BIT_MASK) != 0
 * 
 * 
 * If enabled by TUPE_COMB_MASK_ENA, then the following condition must be
 * met for TUPE to update VLAN table entry:
 * 
 * ((VLAN:TUPE_CTRL & TUPE_CTRL_BIT_MASK) != 0) ||
 * (VLAN:VLAN_MASK_CFG.VLAN_PORT_MASK & TUPE_PORT_MASK_A) != 0
 *
 * \details 
 * Field: ::VTSS_ANA_L3_TUPE_TUPE_CTRL_BIT_MASK . TUPE_CTRL_BIT_MASK
 */
#define  VTSS_F_ANA_L3_TUPE_TUPE_CTRL_BIT_MASK_TUPE_CTRL_BIT_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_ANA_L3_TUPE_TUPE_CTRL_BIT_MASK_TUPE_CTRL_BIT_MASK     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_ANA_L3_TUPE_TUPE_CTRL_BIT_MASK_TUPE_CTRL_BIT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief TUPE parameter controlling which VLAN table entries to update
 *
 * \details
 * Register: \a ANA_L3:TUPE:TUPE_PORT_MASK_A
 */
#define VTSS_ANA_L3_TUPE_TUPE_PORT_MASK_A    VTSS_IOREG(VTSS_TO_ANA_L3,0xd530)


/** 
 * \brief TUPE parameter controlling which VLAN table entries to update
 *
 * \details
 * Register: \a ANA_L3:TUPE:TUPE_PORT_MASK_A1
 */
#define VTSS_ANA_L3_TUPE_TUPE_PORT_MASK_A1   VTSS_IOREG(VTSS_TO_ANA_L3,0xd531)

/** 
 * \brief
 * See TUPE_PORT_MASK_A.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_TUPE_TUPE_PORT_MASK_A1 . TUPE_PORT_MASK_A1
 */
#define  VTSS_F_ANA_L3_TUPE_TUPE_PORT_MASK_A1_TUPE_PORT_MASK_A1(x)  VTSS_ENCODE_BITFIELD(x,0,21)
#define  VTSS_M_ANA_L3_TUPE_TUPE_PORT_MASK_A1_TUPE_PORT_MASK_A1     VTSS_ENCODE_BITMASK(0,21)
#define  VTSS_X_ANA_L3_TUPE_TUPE_PORT_MASK_A1_TUPE_PORT_MASK_A1(x)  VTSS_EXTRACT_BITFIELD(x,0,21)


/** 
 * \brief TUPE parameter controlling which VLAN table entries to update
 *
 * \details
 * Register: \a ANA_L3:TUPE:TUPE_PORT_MASK_B
 */
#define VTSS_ANA_L3_TUPE_TUPE_PORT_MASK_B    VTSS_IOREG(VTSS_TO_ANA_L3,0xd532)


/** 
 * \brief TUPE parameter controlling which VLAN table entries to update
 *
 * \details
 * Register: \a ANA_L3:TUPE:TUPE_PORT_MASK_B1
 */
#define VTSS_ANA_L3_TUPE_TUPE_PORT_MASK_B1   VTSS_IOREG(VTSS_TO_ANA_L3,0xd533)

/** 
 * \brief
 * See TUPE_PORT_MASK_B.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_TUPE_TUPE_PORT_MASK_B1 . TUPE_PORT_MASK_B1
 */
#define  VTSS_F_ANA_L3_TUPE_TUPE_PORT_MASK_B1_TUPE_PORT_MASK_B1(x)  VTSS_ENCODE_BITFIELD(x,0,21)
#define  VTSS_M_ANA_L3_TUPE_TUPE_PORT_MASK_B1_TUPE_PORT_MASK_B1     VTSS_ENCODE_BITMASK(0,21)
#define  VTSS_X_ANA_L3_TUPE_TUPE_PORT_MASK_B1_TUPE_PORT_MASK_B1(x)  VTSS_EXTRACT_BITFIELD(x,0,21)

/**
 * Register Group: \a ANA_L3:VLAN
 *
 * VLAN table configuration
 */


/** 
 * \brief Router Leg Identification / Mapped VLAN ID
 *
 * \details
 * Register: \a ANA_L3:VLAN:VMID_CFG
 *
 * @param gi Replicator: x_ANA_L3_VLAN_TABLE_LEN (??), 0-5119
 */
#define VTSS_ANA_L3_VLAN_VMID_CFG(gi)        VTSS_IOREG_IX(VTSS_TO_ANA_L3,0x0,gi,8,0,0)

/** 
 * \brief
 * Routing:
 * VMID, identifying VLAN's router leg.
 * 
 * Security check: 
 * "Mapped VLAN ID".
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_VMID_CFG . VMID
 */
#define  VTSS_F_ANA_L3_VLAN_VMID_CFG_VMID(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_ANA_L3_VLAN_VMID_CFG_VMID     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_ANA_L3_VLAN_VMID_CFG_VMID(x)  VTSS_EXTRACT_BITFIELD(x,0,7)


/** 
 * \brief VLAN Configuration
 *
 * \details
 * Various configuration of VLAN handles
 *
 * Register: \a ANA_L3:VLAN:VLAN_CFG
 *
 * @param gi Replicator: x_ANA_L3_VLAN_TABLE_LEN (??), 0-5119
 */
#define VTSS_ANA_L3_VLAN_VLAN_CFG(gi)        VTSS_IOREG_IX(VTSS_TO_ANA_L3,0x0,gi,8,0,1)

/** 
 * \brief
 * Pointer to STP instance associated with VLAN.
 * The value must not exceed the size of the MSTP table, ref. MSTP.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_VLAN_CFG . VLAN_MSTP_PTR
 */
#define  VTSS_F_ANA_L3_VLAN_VLAN_CFG_VLAN_MSTP_PTR(x)  VTSS_ENCODE_BITFIELD(x,24,7)
#define  VTSS_M_ANA_L3_VLAN_VLAN_CFG_VLAN_MSTP_PTR     VTSS_ENCODE_BITMASK(24,7)
#define  VTSS_X_ANA_L3_VLAN_VLAN_CFG_VLAN_MSTP_PTR(x)  VTSS_EXTRACT_BITFIELD(x,24,7)

/** 
 * \brief
 * FID to be used for learning and forwarding.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_VLAN_CFG . VLAN_FID
 */
#define  VTSS_F_ANA_L3_VLAN_VLAN_CFG_VLAN_FID(x)  VTSS_ENCODE_BITFIELD(x,8,13)
#define  VTSS_M_ANA_L3_VLAN_VLAN_CFG_VLAN_FID     VTSS_ENCODE_BITMASK(8,13)
#define  VTSS_X_ANA_L3_VLAN_VLAN_CFG_VLAN_FID(x)  VTSS_EXTRACT_BITFIELD(x,8,13)

/** 
 * \brief
 * Enable secure forwarding on a per VLAN basis.
 * 
 * When secure forwarding is enabled, only frames with known SMAC will be
 * forwarded.
 *
 * \details 
 * 0: Forwarding is allowed regardless of SMAC being known or unknown.
 * 1: Forwarding is only allowed for frames with known SMAC.
 *
 * Field: ::VTSS_ANA_L3_VLAN_VLAN_CFG . VLAN_SEC_FWD_ENA
 */
#define  VTSS_F_ANA_L3_VLAN_VLAN_CFG_VLAN_SEC_FWD_ENA  VTSS_BIT(5)

/** 
 * \brief
 * Disable flooding of frames with unknown DMAC on a per VLAN basis.
 * 
 * Note that when VLAN_FLOOD_DIS=1, then frames with broadcast or multicast
 * DMAC are only forwarded if installed in MAC table.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_VLAN_CFG . VLAN_FLOOD_DIS
 */
#define  VTSS_F_ANA_L3_VLAN_VLAN_CFG_VLAN_FLOOD_DIS  VTSS_BIT(4)

/** 
 * \brief
 * Disable learning of SMAC of frames received on this VLAN.
 * 
 * Related parameters:
 * VLAN_ARP_L3MC_STICKY:VLAN_STICKY.VLAN_LRN_DENY_STICKY
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_VLAN_CFG . VLAN_LRN_DIS
 */
#define  VTSS_F_ANA_L3_VLAN_VLAN_CFG_VLAN_LRN_DIS  VTSS_BIT(3)

/** 
 * \brief
 * Enable router leg in VLAN.
 * If enabled, the ID of the router leg is configured in VMID_CFG.VMID.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_VLAN_CFG . VLAN_RLEG_ENA
 */
#define  VTSS_F_ANA_L3_VLAN_VLAN_CFG_VLAN_RLEG_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Enable / disable this VLAN as a Private VLAN (PVLAN). 
 * 
 * Ports within a PVLAN are categorized into three different port types:
 * Promiscuous ports: 
 * A promiscuous port can communicate with all ports in the PVLAN,
 * including the isolated and community ports.
 * 
 * Isolated ports: 
 * An isolated port has complete Layer 2 separation from the other ports
 * within the same PVLAN, but not from the promiscuous ports. PVLANs block
 * all traffic to isolated ports except traffic from promiscuous ports.
 * Traffic from isolated port is forwarded only to promiscuous ports.
 * 
 * Community ports: 
 * Community ports communicate among themselves and with the PVLAN's
 * promiscuous ports. Community ports cannot communicate with isolated
 * ports.
 * 
 * 
 * Related parameters:
 * COMMON:VLAN_ISOLATED_CFG
 * COMMON:VLAN_COMMUNITY_CFG

 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_VLAN_CFG . VLAN_PRIVATE_ENA
 */
#define  VTSS_F_ANA_L3_VLAN_VLAN_CFG_VLAN_PRIVATE_ENA  VTSS_BIT(1)

/** 
 * \brief
 * VLAN mirror enable flag. If this field is set, frames classified to this
 * ingress VLAN are mirrored.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_VLAN_CFG . VLAN_MIRROR_ENA
 */
#define  VTSS_F_ANA_L3_VLAN_VLAN_CFG_VLAN_MIRROR_ENA  VTSS_BIT(0)


/** 
 * \brief TUPE Control
 *
 * \details
 * Control value for Table UPdate Engine (TUPE).
 * 
 * See ANA_L3:TUPE.
 *
 * Register: \a ANA_L3:VLAN:TUPE_CTRL
 *
 * @param gi Replicator: x_ANA_L3_VLAN_TABLE_LEN (??), 0-5119
 */
#define VTSS_ANA_L3_VLAN_TUPE_CTRL(gi)       VTSS_IOREG_IX(VTSS_TO_ANA_L3,0x0,gi,8,0,2)

/** 
 * \brief
 * Control value for Table UPdate Engine (TUPE).
 * 
 * Note that unused bits in VLAN_PORT_MASK may also be used to control
 * which VLAN table entries TUPE shall update.
 * 
 * See ANA_L3:TUPE.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_TUPE_CTRL . TUPE_CTRL
 */
#define  VTSS_F_ANA_L3_VLAN_TUPE_CTRL_TUPE_CTRL(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_ANA_L3_VLAN_TUPE_CTRL_TUPE_CTRL     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_ANA_L3_VLAN_TUPE_CTRL_TUPE_CTRL(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief VLAN Port Mask Configuration
 *
 * \details
 * Configuration of VLAN portmask.
 *
 * Register: \a ANA_L3:VLAN:VLAN_MASK_CFG
 *
 * @param gi Replicator: x_ANA_L3_VLAN_TABLE_LEN (??), 0-5119
 */
#define VTSS_ANA_L3_VLAN_VLAN_MASK_CFG(gi)   VTSS_IOREG_IX(VTSS_TO_ANA_L3,0x0,gi,8,0,3)


/** 
 * \brief VLAN Port Mask Configuration
 *
 * \details
 * Configuration of VLAN portmask.
 *
 * Register: \a ANA_L3:VLAN:VLAN_MASK_CFG1
 *
 * @param gi Replicator: x_ANA_L3_VLAN_TABLE_LEN (??), 0-5119
 */
#define VTSS_ANA_L3_VLAN_VLAN_MASK_CFG1(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_L3,0x0,gi,8,0,4)

/** 
 * \brief
 * Pls. refer to VLAN_MASK_CFG.VLAN_PORT_MASK description.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_VLAN_MASK_CFG1 . VLAN_PORT_MASK1
 */
#define  VTSS_F_ANA_L3_VLAN_VLAN_MASK_CFG1_VLAN_PORT_MASK1(x)  VTSS_ENCODE_BITFIELD(x,0,21)
#define  VTSS_M_ANA_L3_VLAN_VLAN_MASK_CFG1_VLAN_PORT_MASK1     VTSS_ENCODE_BITMASK(0,21)
#define  VTSS_X_ANA_L3_VLAN_VLAN_MASK_CFG1_VLAN_PORT_MASK1(x)  VTSS_EXTRACT_BITFIELD(x,0,21)

/**
 * Register Group: \a ANA_L3:MSTP
 *
 * MSTP table configuration
 */


/** 
 * \brief MSTP Forwarding Control
 *
 * \details
 * Configuration of forwarding state per MSTP
 *
 * Register: \a ANA_L3:MSTP:MSTP_FWD_CFG
 *
 * @param gi Replicator: x_ANA_L3_NUM_MSTP (??), 0-65
 */
#define VTSS_ANA_L3_MSTP_MSTP_FWD_CFG(gi)    VTSS_IOREG_IX(VTSS_TO_ANA_L3,0xd400,gi,4,0,0)


/** 
 * \brief MSTP Forwarding Control
 *
 * \details
 * Configuration of forwarding state per MSTP
 *
 * Register: \a ANA_L3:MSTP:MSTP_FWD_CFG1
 *
 * @param gi Replicator: x_ANA_L3_NUM_MSTP (??), 0-65
 */
#define VTSS_ANA_L3_MSTP_MSTP_FWD_CFG1(gi)   VTSS_IOREG_IX(VTSS_TO_ANA_L3,0xd400,gi,4,0,1)

/** 
 * \brief
 * Pls. refer to MSTP_FWD_CFG.MSTP_FWD_MASK description.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_MSTP_MSTP_FWD_CFG1 . MSTP_FWD_MASK1
 */
#define  VTSS_F_ANA_L3_MSTP_MSTP_FWD_CFG1_MSTP_FWD_MASK1(x)  VTSS_ENCODE_BITFIELD(x,0,21)
#define  VTSS_M_ANA_L3_MSTP_MSTP_FWD_CFG1_MSTP_FWD_MASK1     VTSS_ENCODE_BITMASK(0,21)
#define  VTSS_X_ANA_L3_MSTP_MSTP_FWD_CFG1_MSTP_FWD_MASK1(x)  VTSS_EXTRACT_BITFIELD(x,0,21)


/** 
 * \brief MSTP Learning Control
 *
 * \details
 * Configuration of learning state per MSTP.
 *
 * Register: \a ANA_L3:MSTP:MSTP_LRN_CFG
 *
 * @param gi Replicator: x_ANA_L3_NUM_MSTP (??), 0-65
 */
#define VTSS_ANA_L3_MSTP_MSTP_LRN_CFG(gi)    VTSS_IOREG_IX(VTSS_TO_ANA_L3,0xd400,gi,4,0,2)


/** 
 * \brief MSTP Learning Control
 *
 * \details
 * Register: \a ANA_L3:MSTP:MSTP_LRN_CFG1
 *
 * @param gi Replicator: x_ANA_L3_NUM_MSTP (??), 0-65
 */
#define VTSS_ANA_L3_MSTP_MSTP_LRN_CFG1(gi)   VTSS_IOREG_IX(VTSS_TO_ANA_L3,0xd400,gi,4,0,3)

/** 
 * \brief
 * Pls. refer to MSTP_LRN_CFG.MSTP_LRN_MASK description.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_MSTP_MSTP_LRN_CFG1 . MSTP_LRN_MASK1
 */
#define  VTSS_F_ANA_L3_MSTP_MSTP_LRN_CFG1_MSTP_LRN_MASK1(x)  VTSS_ENCODE_BITFIELD(x,0,21)
#define  VTSS_M_ANA_L3_MSTP_MSTP_LRN_CFG1_MSTP_LRN_MASK1     VTSS_ENCODE_BITMASK(0,21)
#define  VTSS_X_ANA_L3_MSTP_MSTP_LRN_CFG1_MSTP_LRN_MASK1(x)  VTSS_EXTRACT_BITFIELD(x,0,21)

/**
 * Register Group: \a ANA_L3:VMID
 *
 * Router Leg Table
 */


/** 
 * \brief Router leg control
 *
 * \details
 * Register: \a ANA_L3:VMID:RLEG_CTRL
 *
 * @param gi Replicator: x_ANA_NUM_VMID (??), 0-127
 */
#define VTSS_ANA_L3_VMID_RLEG_CTRL(gi)       VTSS_IOREG_IX(VTSS_TO_ANA_L3,0xd000,gi,8,0,0)

/** 
 * \brief
 * VID for egress router leg.
 * 
 * This must be configured consistently in REW:VMID:RLEG_CTRL.RLEG_EVID.

 *
 * \details 
 * Field: ::VTSS_ANA_L3_VMID_RLEG_CTRL . RLEG_EVID
 */
#define  VTSS_F_ANA_L3_VMID_RLEG_CTRL_RLEG_EVID(x)  VTSS_ENCODE_BITFIELD(x,16,12)
#define  VTSS_M_ANA_L3_VMID_RLEG_CTRL_RLEG_EVID     VTSS_ENCODE_BITMASK(16,12)
#define  VTSS_X_ANA_L3_VMID_RLEG_CTRL_RLEG_EVID(x)  VTSS_EXTRACT_BITFIELD(x,16,12)

/** 
 * \brief
 * SIP RPF mode for IPv6.
 * If SIP RPF check fails, then frame is not allowed to be L3 forwarded.
 * 
 * Disabled:
 * No SIP RPF check.
 * 
 * RGID Mode:
 * Verify that SIP belongs to a known subnet and verify that SIP's RGID is
 * enabled in ingress router leg's RGID_MASK.
 * 
 * Rleg Mode:
 * Verify that SIP belongs to a known subnet, and that the frame is
 * received via the router leg through which that subnet is reached.
 * If SIP is reached through an ECMP path then no SIP RPF check is
 * performed.
 * 
 * Combined Mode:
 * Apply Rleg Mode for non ECMP paths and RGID Mode for ECMP paths.
 * 
 * Related parameters:
 * COMMON:SIP_RPF_ENA
 * COMMON:ROUTING_CFG.RLEG_IP6_SIP_RPF_REDIR_ENA
 * COMMON:CPU_QU_CFG.CPU_SIP_RPF_QU
 * VLAN_ARP_L3MC_STICKY:L3_ARP_IPMC_STICKY.IP6_SIP_RPF_FILTER_STICKY
 * VMID:SIP_RPF.RLEG_RGID_MASK

 *
 * \details 
 * 0: Disabled
 * 1: RGID Mode
 * 2: Rleg Mode
 * 3: Combined Mode
 *
 * Field: ::VTSS_ANA_L3_VMID_RLEG_CTRL . RLEG_IP6_SIP_RPF_MODE
 */
#define  VTSS_F_ANA_L3_VMID_RLEG_CTRL_RLEG_IP6_SIP_RPF_MODE(x)  VTSS_ENCODE_BITFIELD(x,10,2)
#define  VTSS_M_ANA_L3_VMID_RLEG_CTRL_RLEG_IP6_SIP_RPF_MODE     VTSS_ENCODE_BITMASK(10,2)
#define  VTSS_X_ANA_L3_VMID_RLEG_CTRL_RLEG_IP6_SIP_RPF_MODE(x)  VTSS_EXTRACT_BITFIELD(x,10,2)

/** 
 * \brief
 * SIP RPF mode for IPv4.
 * If SIP RPF check fails, then frame is not allowed to be L3 forwarded.
 * 
 * Disabled:
 * No SIP RPF check.
 * 
 * RGID Mode:
 * Verify that SIP belongs to a known subnet and verify that SIP's RGID is
 * enabled in ingress router leg's RLEG_RGID_MASK.
 * 
 * Rleg Mode:
 * Verify that SIP belongs to a known subnet, and that the frame is
 * received via the router leg through which that subnet is reached.
 * If SIP is reached through an ECMP path then no SIP RPF check is
 * performed.
 * 
 * Combined Mode:
 * Apply Rleg Mode for non ECMP paths and RGID Mode for ECMP paths.
 * 
 * Related parameters:
 * COMMON:SIP_RPF_ENA
 * COMMON:ROUTING_CFG.RLEG_IP4_SIP_RPF_REDIR_ENA
 * COMMON:CPU_QU_CFG.CPU_SIP_RPF_QU
 * VLAN_ARP_L3MC_STICKY:L3_ARP_IPMC_STICKY.IP4_SIP_RPF_FILTER_STICKY
 * VMID:SIP_RPF.RLEG_RGID_MASK
 *
 * \details 
 * 0: Disabled
 * 1: RGID Mode
 * 2: Rleg Mode
 * 3: Combined Mode
 *
 * Field: ::VTSS_ANA_L3_VMID_RLEG_CTRL . RLEG_IP4_SIP_RPF_MODE
 */
#define  VTSS_F_ANA_L3_VMID_RLEG_CTRL_RLEG_IP4_SIP_RPF_MODE(x)  VTSS_ENCODE_BITFIELD(x,8,2)
#define  VTSS_M_ANA_L3_VMID_RLEG_CTRL_RLEG_IP4_SIP_RPF_MODE     VTSS_ENCODE_BITMASK(8,2)
#define  VTSS_X_ANA_L3_VMID_RLEG_CTRL_RLEG_IP4_SIP_RPF_MODE(x)  VTSS_EXTRACT_BITFIELD(x,8,2)

/** 
 * \brief
 * Enable IPv6 unicast routing.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VMID_RLEG_CTRL . RLEG_IP6_UC_ENA
 */
#define  VTSS_F_ANA_L3_VMID_RLEG_CTRL_RLEG_IP6_UC_ENA  VTSS_BIT(7)

/** 
 * \brief
 * Enable IPv4 unicast routing.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VMID_RLEG_CTRL . RLEG_IP4_UC_ENA
 */
#define  VTSS_F_ANA_L3_VMID_RLEG_CTRL_RLEG_IP4_UC_ENA  VTSS_BIT(6)

/** 
 * \brief
 * Enable IPv6 multicast routing.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VMID_RLEG_CTRL . RLEG_IP6_MC_ENA
 */
#define  VTSS_F_ANA_L3_VMID_RLEG_CTRL_RLEG_IP6_MC_ENA  VTSS_BIT(5)

/** 
 * \brief
 * Enable IPv4 multicast routing.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VMID_RLEG_CTRL . RLEG_IP4_MC_ENA
 */
#define  VTSS_F_ANA_L3_VMID_RLEG_CTRL_RLEG_IP4_MC_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Enable redirection to CPU of IPv6 packets with IVMID==EVMID (i.e.
 * packets that are to be routed back to the router leg, they were received
 * on).
 * 
 * CPU queue is configured in COMMON:CPU_QU_CFG.CPU_UC_FAIL_QU.
 * 
 * Related parameters:
 * COMMON:CPU_QU_CFG.CPU_UC_FAIL_QU
 * VLAN_ARP_L3MC_STICKY:L3_ARP_IPMC_STICKY.UC_ICMP_REDIR_STICKY
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VMID_RLEG_CTRL . RLEG_IP6_ICMP_REDIR_ENA
 */
#define  VTSS_F_ANA_L3_VMID_RLEG_CTRL_RLEG_IP6_ICMP_REDIR_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Enable redirection to CPU of IPv4 packets with IVMID==EVMID (i.e.
 * packets that are to be routed back to the router leg, they were received
 * on).
 * 
 * CPU queue is configured in COMMON:CPU_QU_CFG.CPU_UC_FAIL_QU.
 * 
 * Related parameters:
 * COMMON:CPU_QU_CFG.CPU_UC_FAIL_QU
 * VLAN_ARP_L3MC_STICKY:L3_ARP_IPMC_STICKY.UC_ICMP_REDIR_STICKY
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VMID_RLEG_CTRL . RLEG_IP4_ICMP_REDIR_ENA
 */
#define  VTSS_F_ANA_L3_VMID_RLEG_CTRL_RLEG_IP4_ICMP_REDIR_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Enable IPv6 virtual router leg. 
 * 
 * If enabled, the router leg can also be addressed using a MAC address
 * constructed using the following fields:
 * Bits 47-32: COMMON:VRRP_IP6_BASE_MAC_HIGH 
 * Bits 31-8: COMMON:VRRP_IP6_BASE_MAC_MID 
 * Bits 7-0: VMID.VRRP_CFG.RLEG_IP6_VRID
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VMID_RLEG_CTRL . RLEG_IP6_VRID_ENA
 */
#define  VTSS_F_ANA_L3_VMID_RLEG_CTRL_RLEG_IP6_VRID_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable IPv4 virtual router leg. 
 * 
 * If enabled, the router leg can also be addressed using a MAC address
 * constructed using the following fields:
 * Bits 47-32: COMMON:VRRP_IP4_BASE_MAC_HIGH 
 * Bits 31-8: COMMON:VRRP_IP4_BASE_MAC_MID 
 * Bits 7-0: VMID.VRRP_CFG.RLEG_IP4_VRID
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VMID_RLEG_CTRL . RLEG_IP4_VRID_ENA
 */
#define  VTSS_F_ANA_L3_VMID_RLEG_CTRL_RLEG_IP4_VRID_ENA  VTSS_BIT(0)


/** 
 * \brief Virtual router leg configuration
 *
 * \details
 * Configuration of Virtual Router Interface MAC address
 *
 * Register: \a ANA_L3:VMID:VRRP_CFG
 *
 * @param gi Replicator: x_ANA_NUM_VMID (??), 0-127
 * @param ri Replicator: x_ANA_L3_NUM_VRID (??), 0-1
 */
#define VTSS_ANA_L3_VMID_VRRP_CFG(gi,ri)     VTSS_IOREG_IX(VTSS_TO_ANA_L3,0xd000,gi,8,ri,1)

/** 
 * \brief
 * Router leg's VRID for IPv4.
 * The configured VRID is used as part of the VRRP router MAC address.
 * 
 * Only applicable if VRRP is enabled for router leg.
 * See RLEG_CTRL.RLEG_IP4_VRID_ENA for further details.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VMID_VRRP_CFG . RLEG_IP4_VRID
 */
#define  VTSS_F_ANA_L3_VMID_VRRP_CFG_RLEG_IP4_VRID(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_ANA_L3_VMID_VRRP_CFG_RLEG_IP4_VRID     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_ANA_L3_VMID_VRRP_CFG_RLEG_IP4_VRID(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/** 
 * \brief
 * Router leg's VRID for IPv6.
 * The configured VRID is used as part of the VRRP router MAC address.
 * 
 * Only applicable if VRRP is enabled for router leg.
 * See RLEG_CTRL.RLEG_IP6_VRID_ENA for further details.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VMID_VRRP_CFG . RLEG_IP6_VRID
 */
#define  VTSS_F_ANA_L3_VMID_VRRP_CFG_RLEG_IP6_VRID(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_ANA_L3_VMID_VRRP_CFG_RLEG_IP6_VRID     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_ANA_L3_VMID_VRRP_CFG_RLEG_IP6_VRID(x)  VTSS_EXTRACT_BITFIELD(x,8,8)


/** 
 * \brief IP multicast router leg configuration
 *
 * \details
 * Register: \a ANA_L3:VMID:VMID_MC
 *
 * @param gi Replicator: x_ANA_NUM_VMID (??), 0-127
 */
#define VTSS_ANA_L3_VMID_VMID_MC(gi)         VTSS_IOREG_IX(VTSS_TO_ANA_L3,0xd000,gi,8,0,3)

/** 
 * \brief
 * Enable checking of DMAC for IPv6 multicast packets.
 * 
 * I.e. verify that 
 *   DMAC[47:32] == 0x3333
 * 
 * If the check fails, the packet will not be routed.

 *
 * \details 
 * Field: ::VTSS_ANA_L3_VMID_VMID_MC . RLEG_IP6_MC_DMAC_CHK_ENA
 */
#define  VTSS_F_ANA_L3_VMID_VMID_MC_RLEG_IP6_MC_DMAC_CHK_ENA  VTSS_BIT(17)

/** 
 * \brief
 * Enable checking of DMAC for IPv4 multicast packets.
 * 
 * I.e. verify that 
 *   DMAC[47:24] == 0x01005e
 *   DMAC[23] == 0
 * 
 * If the check fails, the packet will not be routed.

 *
 * \details 
 * Field: ::VTSS_ANA_L3_VMID_VMID_MC . RLEG_IP4_MC_DMAC_CHK_ENA
 */
#define  VTSS_F_ANA_L3_VMID_VMID_MC_RLEG_IP4_MC_DMAC_CHK_ENA  VTSS_BIT(16)

/** 
 * \brief
 * IPv4 multicast TTL limit.
 * Packets with TTL below the configured limit are not routed.
 * 
 * TTL is checked against RLEG_IP4_MC_TTL before transmission on egress
 * router leg.
 * 
 * Note: Regardless of the value configured for this parameter, IPv4 MC
 * packets with TTL<2 are not routed.
 *
 * \details 
 * 0: Router leg based MC TTL check disabled.
 * 1-2: N/A - such packets are not routed anyway.
 * 3: If packet's TTL is < 3 then packet is not routed.
 * 4: If packet's TTL is < 4 then packet is not routed.
 * ...
 *
 * Field: ::VTSS_ANA_L3_VMID_VMID_MC . RLEG_IP4_MC_TTL
 */
#define  VTSS_F_ANA_L3_VMID_VMID_MC_RLEG_IP4_MC_TTL(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_ANA_L3_VMID_VMID_MC_RLEG_IP4_MC_TTL     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_ANA_L3_VMID_VMID_MC_RLEG_IP4_MC_TTL(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/** 
 * \brief
 * IPv6 multicast HL limit.
 * Packets with HL below the configured limit are not routed.
 * 
 * HL is checked against RLEG_IP6_MC_TTL before transmission on egress
 * router leg.
 * 
 * Note: Regardless of the value configured for this parameter, IPv6 MC
 * packets with HC<2 are not routed.
 *
 * \details 
 * 0: Router leg based MC HL check disabled
 * 1-2: N/A - such packets are not routed anyway.
 * 3: If packet's HC is < 3 then packet is not routed.
 * 4: If packet's HC is < 4 then packet is not routed.
 * ...
 *
 * Field: ::VTSS_ANA_L3_VMID_VMID_MC . RLEG_IP6_MC_TTL
 */
#define  VTSS_F_ANA_L3_VMID_VMID_MC_RLEG_IP6_MC_TTL(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_ANA_L3_VMID_VMID_MC_RLEG_IP6_MC_TTL     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_ANA_L3_VMID_VMID_MC_RLEG_IP6_MC_TTL(x)  VTSS_EXTRACT_BITFIELD(x,8,8)


/** 
 * \details
 * Register: \a ANA_L3:VMID:SIP_RPF
 *
 * @param gi Replicator: x_ANA_NUM_VMID (??), 0-127
 */
#define VTSS_ANA_L3_VMID_SIP_RPF(gi)         VTSS_IOREG_IX(VTSS_TO_ANA_L3,0xd000,gi,8,0,4)

/** 
 * \brief
 * Route Group IDs (RGIDs) accepted for this router leg when SIP_RPF_MODE
 * is set to "RGID Mode".
 * 
 * See description of RLEG_IP4_SIP_RPF_MODE / RLEG_IP6_SIP_RPF_MODE.

 *
 * \details 
 * Bit 0: Accept routes with RGID=0 
 * Bit 1: Accept routes with RGID=1
 * ...

 *
 * Field: ::VTSS_ANA_L3_VMID_SIP_RPF . RLEG_RGID_MASK
 */
#define  VTSS_F_ANA_L3_VMID_SIP_RPF_RLEG_RGID_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_ANA_L3_VMID_SIP_RPF_RLEG_RGID_MASK     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_ANA_L3_VMID_SIP_RPF_RLEG_RGID_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \details
 * Register: \a ANA_L3:VMID:MAX_LEN
 *
 * @param gi Replicator: x_ANA_NUM_VMID (??), 0-127
 */
#define VTSS_ANA_L3_VMID_MAX_LEN(gi)         VTSS_IOREG_IX(VTSS_TO_ANA_L3,0xd000,gi,8,0,5)

/** 
 * \brief
 * Max "Payload Length" (ref. RFC2460) of IPv6 frames using this egress
 * router leg.
 * 
 * Related parameters:
 * COMMON:ROUTING_CFG.IP6_LEN_REDIR
 * COMMON:CPU_QU_CFG.CPU_IP_LEN_QU
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VMID_MAX_LEN . IP6_MAX_LEN
 */
#define  VTSS_F_ANA_L3_VMID_MAX_LEN_IP6_MAX_LEN(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_ANA_L3_VMID_MAX_LEN_IP6_MAX_LEN     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_ANA_L3_VMID_MAX_LEN_IP6_MAX_LEN(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Max "Total Length" (ref. RFC791) of IPv4 frames using this egress router
 * leg.
 * 
 * Related parameters:
 * COMMON:ROUTING_CFG.IP4_LEN_REDIR
 * COMMON:CPU_QU_CFG.CPU_IP_LEN_QU

 *
 * \details 
 * Field: ::VTSS_ANA_L3_VMID_MAX_LEN . IP4_MAX_LEN
 */
#define  VTSS_F_ANA_L3_VMID_MAX_LEN_IP4_MAX_LEN(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_ANA_L3_VMID_MAX_LEN_IP4_MAX_LEN     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_ANA_L3_VMID_MAX_LEN_IP4_MAX_LEN(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a ANA_L3:ARP
 *
 * ARP table configuration
 */


/** 
 * \brief ARP Data
 *
 * \details
 * Configuration registers for ARP table
 *
 * Register: \a ANA_L3:ARP:ARP_CFG_0
 *
 * @param gi Replicator: x_ANA_NUM_ARP (??), 0-2047
 */
#define VTSS_ANA_L3_ARP_ARP_CFG_0(gi)        VTSS_IOREG_IX(VTSS_TO_ANA_L3,0xc000,gi,2,0,0)

/** 
 * \brief
 * 16 most significant bits of MAC address.
 * Used for ARP entry and/or (SMAC,SIP)/(DMAC,DIP) check.
 * 
 * Least significant bits are configured in ARP_CFG_0.MAC_LSB.
 * 
 * If MAC address for ARP entry is all-zeros, then the frame is redirected
 * to CPU. 
 * CPU queue used for such frames is configured in ZERO_DMAC_CPU_QU.
 *
 * \details 
 * Bit 0: MAC address bit 32
 * ...
 * Bit 15: MAC address bit 47
 *
 * Field: ::VTSS_ANA_L3_ARP_ARP_CFG_0 . MAC_MSB
 */
#define  VTSS_F_ANA_L3_ARP_ARP_CFG_0_MAC_MSB(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_ANA_L3_ARP_ARP_CFG_0_MAC_MSB     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_ANA_L3_ARP_ARP_CFG_0_MAC_MSB(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Routing lookup:
 * Egress router leg (EVMID). 
 * 
 * Security lookup:
 * Expected Ingress mapped VLAN ID for security lookup when
 * SECUR_MATCH_VMID_ENA is enabled.
 * 
 * SIP RPF:
 * Expected ingress VMID of SIP_RPF_ENA==1 and
 * VMID:RLEG_CTRL:RLEG_IPx_SIP_RPF_MODE==Rleg Mode:

 *
 * \details 
 * Field: ::VTSS_ANA_L3_ARP_ARP_CFG_0 . ARP_VMID
 */
#define  VTSS_F_ANA_L3_ARP_ARP_CFG_0_ARP_VMID(x)  VTSS_ENCODE_BITFIELD(x,8,7)
#define  VTSS_M_ANA_L3_ARP_ARP_CFG_0_ARP_VMID     VTSS_ENCODE_BITMASK(8,7)
#define  VTSS_X_ANA_L3_ARP_ARP_CFG_0_ARP_VMID(x)  VTSS_EXTRACT_BITFIELD(x,8,7)

/** 
 * \brief
 * CPU queue used for CPU redirect if MAC address in ARP entry is
 * all-zeros.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_ARP_ARP_CFG_0 . ZERO_DMAC_CPU_QU
 */
#define  VTSS_F_ANA_L3_ARP_ARP_CFG_0_ZERO_DMAC_CPU_QU(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_ANA_L3_ARP_ARP_CFG_0_ZERO_DMAC_CPU_QU     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_ANA_L3_ARP_ARP_CFG_0_ZERO_DMAC_CPU_QU(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * Enable use for SIP RPF check.
 * 
 * Ref.
 * VMID:RLEG_CTRL.RLEG_IP4_SIP_RPF_MODE
 * VMID:RLEG_CTRL.RLEG_IP6_SIP_RPF_MODE
 *
 * \details 
 * Field: ::VTSS_ANA_L3_ARP_ARP_CFG_0 . SIP_RPF_ENA
 */
#define  VTSS_F_ANA_L3_ARP_ARP_CFG_0_SIP_RPF_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Enable Security VMID check. 
 * 
 * SIP check: 
 * If SECUR_MATCH_VMID_ENA=1 and the IVMID is not equal to the VMID in the
 * ARP table (ARP_VMID) when looking up SIP, then the L3_SMAC_SIP_MATCH
 * input to ANA_ACL will be deasserted.
 * 
 * DIP check: 
 * If SECUR_MATCH_VMID_ENA=1 and the IVMID is not equal to the VMID in the
 * ARP table (ARP_VMID) when looking up DIP, then the L3_SMAC_DIP_MATCH
 * input to ANA_ACL will be deasserted.
 * DIP check is only performed for L2 forwarded IP unicast frames.
 * 
 * Related parameters:
 * COMMON:SIP_SECURE_ENA
 * COMMON:DIP_SECURE_ENA
 *
 * \details 
 * Field: ::VTSS_ANA_L3_ARP_ARP_CFG_0 . SECUR_MATCH_VMID_ENA
 */
#define  VTSS_F_ANA_L3_ARP_ARP_CFG_0_SECUR_MATCH_VMID_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Enable Security MAC check. 
 * 
 * SIP check: 
 * If SECUR_MATCH_MAC_ENA=1 and the frame's SMAC is not equal to the MAC in
 * the ARP table (ARP_MAC) when looking up SIP then the L3_SMAC_SIP_MATCH
 * input to ANA_ACL will be deasserted.
 * 
 * DIP check: 
 * If SECUR_MATCH_MAC_ENA=1 and the frame's DMAC is not equal to the MAC in
 * the ARP table (ARP_MAC) when looking up DIP then the L3_SMAC_DIP_MATCH
 * input to ANA_ACL will be deasserted.
 * DIP check is only performed for L2 forwarded IP unicast frames.
 * 
 * Related parameters:
 * COMMON:SIP_SECURE_ENA
 * COMMON:DIP_SECURE_ENA
 *
 * \details 
 * Field: ::VTSS_ANA_L3_ARP_ARP_CFG_0 . SECUR_MATCH_MAC_ENA
 */
#define  VTSS_F_ANA_L3_ARP_ARP_CFG_0_SECUR_MATCH_MAC_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable entry for address resolution usage.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_ARP_ARP_CFG_0 . ARP_ENA
 */
#define  VTSS_F_ANA_L3_ARP_ARP_CFG_0_ARP_ENA  VTSS_BIT(0)


/** 
 * \brief ARP Data 1
 *
 * \details
 * Configuration registers for ARP table
 *
 * Register: \a ANA_L3:ARP:ARP_CFG_1
 *
 * @param gi Replicator: x_ANA_NUM_ARP (??), 0-2047
 */
#define VTSS_ANA_L3_ARP_ARP_CFG_1(gi)        VTSS_IOREG_IX(VTSS_TO_ANA_L3,0xc000,gi,2,0,1)

/**
 * Register Group: \a ANA_L3:L3MC
 *
 * L3 Multicast Table
 */


/** 
 * \brief Multicast router leg mask
 *
 * \details
 * Register: \a ANA_L3:L3MC:EVMID_MASK_CFG
 *
 * @param gi Replicator: x_ANA_NUM_IPMC_MASKS (??), 0-1023
 * @param ri Replicator: x_ANA_NUM_VMID_DIV32_CEIL (??), 0-3
 */
#define VTSS_ANA_L3_L3MC_EVMID_MASK_CFG(gi,ri)  VTSS_IOREG_IX(VTSS_TO_ANA_L3,0xa000,gi,8,ri,0)


/** 
 * \brief L3 multicast control register
 *
 * \details
 * L3 multicast control register
 *
 * Register: \a ANA_L3:L3MC:L3MC_CTRL
 *
 * @param gi Replicator: x_ANA_NUM_IPMC_MASKS (??), 0-1023
 */
#define VTSS_ANA_L3_L3MC_L3MC_CTRL(gi)       VTSS_IOREG_IX(VTSS_TO_ANA_L3,0xa000,gi,8,0,4)

/** 
 * \brief
 * Expected IVMID if RPF check is enabled, ref. RPF_CHK_ENA.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3MC_L3MC_CTRL . RPF_VMID
 */
#define  VTSS_F_ANA_L3_L3MC_L3MC_CTRL_RPF_VMID(x)  VTSS_ENCODE_BITFIELD(x,8,7)
#define  VTSS_M_ANA_L3_L3MC_L3MC_CTRL_RPF_VMID     VTSS_ENCODE_BITMASK(8,7)
#define  VTSS_X_ANA_L3_L3MC_L3MC_CTRL_RPF_VMID(x)  VTSS_EXTRACT_BITFIELD(x,8,7)

/** 
 * \brief
 * CPU queue used for frames redirected due to CPU_REDIR_MODE.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3MC_L3MC_CTRL . CPU_QU
 */
#define  VTSS_F_ANA_L3_L3MC_L3MC_CTRL_CPU_QU(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_ANA_L3_L3MC_L3MC_CTRL_CPU_QU     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_ANA_L3_L3MC_L3MC_CTRL_CPU_QU(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * 00: No CPU redirection/copy.
 * 01: Copy CPU
 * 10: Copy to CPU, skip L3 forwarding but preserve L2 forwarding.
 * 11: Redirect to CPU, skip L2 and L3 forwarding.
 * 
 * CPU queue used is configured in CPU_QU.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3MC_L3MC_CTRL . CPU_REDIR_MODE
 */
#define  VTSS_F_ANA_L3_L3MC_L3MC_CTRL_CPU_REDIR_MODE(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_ANA_L3_L3MC_L3MC_CTRL_CPU_REDIR_MODE     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_ANA_L3_L3MC_L3MC_CTRL_CPU_REDIR_MODE(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Enable reverse path forwarding check, i.e. only allow routing of frames
 * received via a specific router leg (RPF_VMID), i.e. IVMID=RPF_VMID.
 * 
 * Related parameters:
 * VLAN_ARP_L3MC_STICKY:L3_ARP_IPMC_STICKY.MC_RPF_FILTER_STICKY
 * 
 * Note that this check is a different check than SIP RPF check, ref. 
 * VMID:RLEG_CTRL.RLEG_IP4_SIP_RPF_MODE
 * VMID:RLEG_CTRL.RLEG_IP6_SIP_RPF_MODE
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3MC_L3MC_CTRL . RPF_CHK_ENA
 */
#define  VTSS_F_ANA_L3_L3MC_L3MC_CTRL_RPF_CHK_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable CPU copy of frames, which are otherwise candidates for routing,
 * but have TTL/HL<2.
 * Such frames are not L3 forwarded, but may still be subject to L2
 * forwarding.
 * 
 * CPU queue used is configured in COMMON:CPU_QU_CFG.CPU_IP_TTL_FAIL_QU.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3MC_L3MC_CTRL . IPMC_TTL_COPY_ENA
 */
#define  VTSS_F_ANA_L3_L3MC_L3MC_CTRL_IPMC_TTL_COPY_ENA  VTSS_BIT(0)

/**
 * Register Group: \a ANA_L3:LPM_REMAP_STICKY
 *
 * Diagnostic information
 */


/** 
 * \brief Router Leg Diagnostic
 *
 * \details
 * Diagnostic bits related to longest prefix matching.
 *
 * Register: \a ANA_L3:LPM_REMAP_STICKY:L3_LPM_REMAP_STICKY
 */
#define VTSS_ANA_L3_LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY  VTSS_IOREG(VTSS_TO_ANA_L3,0xd534)

/** 
 * \brief
 * Set if an IPv6 SIP security lookup has found a match in LPM lookup.
 * Note that this bit is not set for DIP security LPM lookups
 *
 * \details 
 * Field: ::VTSS_ANA_L3_LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY . SECUR_IP6_LPM_FOUND_STICKY
 */
#define  VTSS_F_ANA_L3_LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY_SECUR_IP6_LPM_FOUND_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * Set if an IPv4 SIP security lookup has found a match in LPM lookup.
 * Note that this bit is not set for DIP security LPM lookups
 *
 * \details 
 * Field: ::VTSS_ANA_L3_LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY . SECUR_IP4_LPM_FOUND_STICKY
 */
#define  VTSS_F_ANA_L3_LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY_SECUR_IP4_LPM_FOUND_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Set if routing lookup found an IPv6 unicast longest prefix match.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY . LPM_IP6UC_FOUND_STICKY
 */
#define  VTSS_F_ANA_L3_LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY_LPM_IP6UC_FOUND_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * Set if routing lookup found an IPv4 unicast longest prefix match.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY . LPM_IP4UC_FOUND_STICKY
 */
#define  VTSS_F_ANA_L3_LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY_LPM_IP4UC_FOUND_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Set if an IPv6 multicast longest prefix match has been found and the
 * returned L3MC_IDX is less than the number of L3MC table entries. 
 *
 * \details 
 * Field: ::VTSS_ANA_L3_LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY . LPM_IP6MC_FOUND_STICKY
 */
#define  VTSS_F_ANA_L3_LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY_LPM_IP6MC_FOUND_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Set if an IPv4 multicast longest prefix match has been found and the
 * returned L3MC_IDX is less than the number of L3MC table entries. 
 *
 * \details 
 * Field: ::VTSS_ANA_L3_LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY . LPM_IP4MC_FOUND_STICKY
 */
#define  VTSS_F_ANA_L3_LPM_REMAP_STICKY_L3_LPM_REMAP_STICKY_LPM_IP4MC_FOUND_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a ANA_L3:VLAN_ARP_L3MC_STICKY
 *
 * Diagnostic information
 */


/** 
 * \brief VLAN Diagnostic
 *
 * \details
 * Register: \a ANA_L3:VLAN_ARP_L3MC_STICKY:VLAN_STICKY
 */
#define VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_VLAN_STICKY  VTSS_IOREG(VTSS_TO_ANA_L3,0xd535)

/** 
 * \brief
 * Set if learning has been denied for a frame due to 
 * COMMON:PORT_LRN_CTRL.PORT_LRN_ENA = 0
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_VLAN_STICKY . PORT_LRN_DENY_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_VLAN_STICKY_PORT_LRN_DENY_STICKY  VTSS_BIT(16)

/** 
 * \brief
 * Set if a frame has been denied forwarding due to 
 * COMMON:PORT_FWD_CTRL.PORT_FWD_ENA = 0
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_VLAN_STICKY . PORT_FWD_DENY_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_VLAN_STICKY_PORT_FWD_DENY_STICKY  VTSS_BIT(15)

/** 
 * \brief
 * Set if MSTP_FWD_MASK has allowed forwarding of a frame.
 * 
 * Related parameters:
 * MSTP:MSTP_FWD_CFG.MSTP_FWD_MASK
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_VLAN_STICKY . MSTP_FWD_ALLOWED_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_VLAN_STICKY_MSTP_FWD_ALLOWED_STICKY  VTSS_BIT(6)

/** 
 * \brief
 * Set if a frame has been filtered due to MSTP_FWD_MASK.
 * 
 * Related parameters:
 * MSTP:MSTP_FWD_CFG.MSTP_FWD_MASK
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_VLAN_STICKY . MSTP_DISCARD_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_VLAN_STICKY_MSTP_DISCARD_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * Set if VLAN_LRN_DIS has denied learning for a frame.
 * 
 * Related parameters:
 * VLAN:VLAN_CFG.VLAN_LRN_DIS

 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_VLAN_STICKY . VLAN_LRN_DENY_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_VLAN_STICKY_VLAN_LRN_DENY_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Set if MSTP_LRN_MASK has denied learning for a frame.
 * 
 * Related parameters:
 * MSTP:MSTP_LRN_CFG.MSTP_LRN_MASK
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_VLAN_STICKY . MSTP_LRN_DENY_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_VLAN_STICKY_MSTP_LRN_DENY_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * Set if MSTP_LRN_MASK has allowed learning for a frame.
 * 
 * Related parameters:
 * MSTP:MSTP_LRN_CFG.MSTP_LRN_MASK
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_VLAN_STICKY . MSTP_LRN_ALLOWED_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_VLAN_STICKY_MSTP_LRN_ALLOWED_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Set if a frame has been classified to a VLAN with an empty port mask.
 * 
 * Related parameters:
 * VLAN:VLAN_MASK_CFG.VLAN_PORT_MASK
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_VLAN_STICKY . VLAN_LOOKUP_INVLD_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_VLAN_STICKY_VLAN_LOOKUP_INVLD_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Set if a frame has been filtered due to VLAN ingress filtering.
 * 
 * Related parameters:
 * COMMON:VLAN_FILTER_CTRL.VLAN_IGR_FILTER_ENA
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_VLAN_STICKY . VLAN_IGR_FILTER_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_VLAN_STICKY_VLAN_IGR_FILTER_STICKY  VTSS_BIT(0)


/** 
 * \brief ARP and L3MC table related diagnostic registers
 *
 * \details
 * ARP and L3MC table related diagnostic registers.
 *
 * Register: \a ANA_L3:VLAN_ARP_L3MC_STICKY:L3_ARP_IPMC_STICKY
 */
#define VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY  VTSS_IOREG(VTSS_TO_ANA_L3,0xd536)

/** 
 * \brief
 * SIP LPM lookup has resulted in match with unsupported action type.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . WRONG_SIP_LPM_ACTION_TYPE_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_WRONG_SIP_LPM_ACTION_TYPE_STICKY  VTSS_BIT(29)

/** 
 * \brief
 * DIP or (DIP,SIP) LPM lookup has resulted in match with unsupported
 * action type.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . WRONG_DIP_LPM_ACTION_TYPE_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_WRONG_DIP_LPM_ACTION_TYPE_STICKY  VTSS_BIT(28)

/** 
 * \brief
 * Set if IPv6 SIP RPF check results in a frame not being L3 forwarded.
 * 
 * Related parameters:
 * VMID:RLEG_CTRL.RLEG_IP6_SIP_RPF_MODE
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . IP6_SIP_RPF_FILTER_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_IP6_SIP_RPF_FILTER_STICKY  VTSS_BIT(27)

/** 
 * \brief
 * Set if IPv4 SIP RPF check results in a frame not being L3 forwarded.
 * 
 * Related parameters:
 * VMID:RLEG_CTRL.RLEG_IP4_SIP_RPF_MODE
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . IP4_SIP_RPF_FILTER_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_IP4_SIP_RPF_FILTER_STICKY  VTSS_BIT(26)

/** 
 * \brief
 * Set if IPv4/IPv6 multicast forwarding fails because no there is no
 * router leg  in L3MC:EVMID_MASK_CFG.EVMID_MASK to forward to.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . NO_MC_VMID_AVAIL_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_NO_MC_VMID_AVAIL_STICKY  VTSS_BIT(25)

/** 
 * \brief
 * Set if a frame has exceeded IP4_MAX_LEN/IP6_MAX_LEN for an egress router
 * leg.
 * 
 * Ref. 
 * VMID:MAX_LEN.IP4_MAX_LEN
 * VMID:MAX_LEN.IP6_MAX_LEN
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . IP_MAX_LEN_EXCEEDED_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_IP_MAX_LEN_EXCEEDED_STICKY  VTSS_BIT(24)

/** 
 * \brief
 * Set if the IPv4/IPv6 DIP or (DIP,SIP) lookup failed due to 
 * - No match in LPM table 
 * - Unsupported action type in LPM table
 * - ARP table lookup returns an invalid entry.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . ENTRY_NOT_FOUND_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_ENTRY_NOT_FOUND_STICKY  VTSS_BIT(23)

/** 
 * \brief
 * Set if an IP MC frame copy from VD0 does not get L3 forwarded (i.e. gets
 * dropped).
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . MC_LOOPED_CP_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_MC_LOOPED_CP_STICKY  VTSS_BIT(22)

/** 
 * \brief
 * Set if neither L2 nor L3 forwarding is performed for an IP multicast
 * frame.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . NO_MC_FWD_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_NO_MC_FWD_STICKY  VTSS_BIT(21)

/** 
 * \brief
 * Set when IP multicast L3 forwarding has been completed for a frame.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . L3_MC_FWD_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_L3_MC_FWD_STICKY  VTSS_BIT(20)

/** 
 * \brief
 * Set if an IP MC frame has failed RPF check.
 * 
 * Related parameters:
 * L3MC:L3MC_CTRL.RPF_CHK_ENA
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . MC_RPF_FILTER_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_MC_RPF_FILTER_STICKY  VTSS_BIT(19)

/** 
 * \brief
 * Set when L2 forwarding of IPv4 multicast frame has completed.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . L2_MC_FWD_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_L2_MC_FWD_STICKY  VTSS_BIT(18)

/** 
 * \brief
 * Set if a frame has been redirected to CPU due to ICMP Redirect check. 
 * 
 * Related parameters:
 * VMID.RLEG_CTRL.RLEG_IP4_ICMP_REDIR_ENA
 * VMID.RLEG_CTRL.RLEG_IP6_ICMP_REDIR_ENA
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . UC_ICMP_REDIR_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_UC_ICMP_REDIR_STICKY  VTSS_BIT(17)

/** 
 * \brief
 * Set if a frame has been redirected to CPU due to a zero DMAC address in
 * the ARP entry.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . UC_ZERO_DMAC_FOUND_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_UC_ZERO_DMAC_FOUND_STICKY  VTSS_BIT(16)

/** 
 * \brief
 * Set in case of an IP unicast frame being filtered due to TTL.
 * I.e. a frame with TTL<2 which hits a valid ARP table entry with non-zero
 * MAC address.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . UC_TTL_FILTERING_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_UC_TTL_FILTERING_STICKY  VTSS_BIT(15)

/** 
 * \brief
 * Set if IP unicast routing lookup results in a valid ARP entry allowing
 * the frame to be routed. 
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . UC_ENTRY_FOUND_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_UC_ENTRY_FOUND_STICKY  VTSS_BIT(14)

/** 
 * \brief
 * Set if ingress router leg match has been found for an IP multicast frame
 * with a TTL less than 2. 
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . RLEG_MC_TTL_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_RLEG_MC_TTL_STICKY  VTSS_BIT(13)

/** 
 * \brief
 * Set for successful IPv4 SIP security lookup, i.e. matching for
 * (DMAC,SIP) and/or (VMID,SIP).
 * 
 * Related parameters:
 * COMMON:SIP_SECURE_ENA.SIP_CMP_ENA
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . SECUR_IP4_SIP_MATCH_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_SECUR_IP4_SIP_MATCH_STICKY  VTSS_BIT(12)

/** 
 * \brief
 * Set for successful IPv6 DIP security lookup, i.e. matching for
 * (DMAC,DIP) and/or (VMID,DIP).
 * 
 * Related parameters:
 * COMMON:DIP_SECURE_ENA.DIP_CMP_ENA

 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . SECUR_IP6_DIP_MATCH_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_SECUR_IP6_DIP_MATCH_STICKY  VTSS_BIT(11)

/** 
 * \brief
 * Set for successful IPv4 DIP security lookup, i.e. matching for
 * (DMAC,DIP) and/or (VMID,DIP).
 * 
 * Related parameters:
 * COMMON:DIP_SECURE_ENA.DIP_CMP_ENA

 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . SECUR_IP4_DIP_MATCH_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_SECUR_IP4_DIP_MATCH_STICKY  VTSS_BIT(10)

/** 
 * \brief
 * Set for successful IPv6 SIP security lookup, i.e. matching for
 * (DMAC,SIP) and/or (VMID,SIP).
 * 
 * Related parameters:
 * COMMON:SIP_SECURE_ENA.SIP_CMP_ENA
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . SECUR_IP6_SIP_MATCH_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_SECUR_IP6_SIP_MATCH_STICKY  VTSS_BIT(9)

/** 
 * \brief
 * Set if security lookup fails for DIP, i.e. mismatch for (DMAC,DIP) /
 * (VMID,DIP).
 * 
 * Related parameters:
 * COMMON:DIP_SECURE_ENA.DIP_CMP_ENA
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . SECUR_DIP_FAIL_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_SECUR_DIP_FAIL_STICKY  VTSS_BIT(8)

/** 
 * \brief
 * Set if security lookup fails for SIP, i.e. mismatch for (DMAC,DIP) /
 * (VMID,DIP).
 * 
 * Related parameters:
 * COMMON:SIP_SECURE_ENA.SIP_CMP_ENA
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . SECUR_SIP_FAIL_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_SECUR_SIP_FAIL_STICKY  VTSS_BIT(7)

/** 
 * \brief
 * Set if an ingress router leg match has been found for an IP multicast
 * frame. 
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . RLEG_MC_HIT_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_RLEG_MC_HIT_STICKY  VTSS_BIT(6)

/** 
 * \brief
 * Set if an ingress router leg match has been found for an IP unicast
 * frame. 
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . RLEG_UC_HIT_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_RLEG_UC_HIT_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * Set if ingress router leg match has been found for an IPv4/IPv6
 * multicast frame with options/hop-by-hop options. 
 * 
 * Such frames may optionally be redirected to CPU, ref.
 * COMMON:ROUTING_CFG.CPU_IP4_OPTIONS_REDIR_ENA
 * COMMON:ROUTING_CFG.CPU_IP6_HOPBYHOP_REDIR_ENA
 * 
 * A similar sticky bit is available for IP unicast frames:
 * RLEG_UC_IP_OPT_REDIR_STICKY
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . RLEG_MC_IP_OPT_REDIR_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_RLEG_MC_IP_OPT_REDIR_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Set if ingress router leg match has been found for an IPv4/IPv6 unicast
 * frame with options/hop-by-hop options. 
 * 
 * Such frames may optionally be redirected to CPU, ref.
 * COMMON:ROUTING_CFG.CPU_IP4_OPTIONS_REDIR_ENA
 * COMMON:ROUTING_CFG.CPU_IP6_HOPBYHOP_REDIR_ENA
 * 
 * A similar sticky bit is available for IP multicast frames:
 * RLEG_MC_IP_OPT_REDIR_STICKY
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . RLEG_UC_IP_OPT_REDIR_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_RLEG_UC_IP_OPT_REDIR_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * Set if an ingress router leg match has been found for an IP multicast
 * frame with header errors.
 * 
 * Refer to 
 * COMMON:ROUTING_CFG.CPU_RLEG_IP_HDR_FAIL_REDIR_ENA
 * for list of covered errors.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . RLEG_MC_HDR_ERR_REDIR_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_RLEG_MC_HDR_ERR_REDIR_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Set if an ingress router leg match has been found for an IP unicast
 * frame with header errors.
 * 
 * Refer to 
 * COMMON:ROUTING_CFG.CPU_RLEG_IP_HDR_FAIL_REDIR_ENA
 * for list of covered errors.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . RLEG_UC_HDR_ERR_REDIR_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_RLEG_UC_HDR_ERR_REDIR_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Set if ingress router leg match has been found for a non-IP frame. 
 * 
 * Such frames are redirected to 
 * COMMON:CPU_QU_CFG.CPU_RLEG_QU
 *
 * \details 
 * Field: ::VTSS_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY . RLEG_NONIP_UC_REDIR_STICKY
 */
#define  VTSS_F_ANA_L3_VLAN_ARP_L3MC_STICKY_L3_ARP_IPMC_STICKY_RLEG_NONIP_UC_REDIR_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a ANA_L3:L3_STICKY_MASK
 *
 * Counter configuration for diagnostic information
 */


/** 
 * \brief Enable per concurrent counter for each sticky bit
 *
 * \details
 * Each ANA_L3 sticky bit can be counted in each of the concurrent
 * counters.
 *
 * Register: \a ANA_L3:L3_STICKY_MASK:RLEG_STICKY_MASK
 *
 * @param gi Replicator: x_ANA_NUM_CONCURRENT_CNT (??), 0-3
 */
#define VTSS_ANA_L3_L3_STICKY_MASK_RLEG_STICKY_MASK(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_L3,0xd537,gi,4,0,0)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_RLEG_STICKY_MASK . RLEG_UC_IP_OPT_REDIR_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_RLEG_STICKY_MASK_RLEG_UC_IP_OPT_REDIR_STICKY_MASK  VTSS_BIT(7)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_RLEG_STICKY_MASK . RLEG_UC_HIT_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_RLEG_STICKY_MASK_RLEG_UC_HIT_STICKY_MASK  VTSS_BIT(6)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_RLEG_STICKY_MASK . RLEG_UC_HDR_ERR_REDIR_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_RLEG_STICKY_MASK_RLEG_UC_HDR_ERR_REDIR_STICKY_MASK  VTSS_BIT(5)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_RLEG_STICKY_MASK . RLEG_MC_TTL_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_RLEG_STICKY_MASK_RLEG_MC_TTL_STICKY_MASK  VTSS_BIT(4)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_RLEG_STICKY_MASK . RLEG_MC_IP_OPT_REDIR_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_RLEG_STICKY_MASK_RLEG_MC_IP_OPT_REDIR_STICKY_MASK  VTSS_BIT(3)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_RLEG_STICKY_MASK . RLEG_MC_HIT_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_RLEG_STICKY_MASK_RLEG_MC_HIT_STICKY_MASK  VTSS_BIT(2)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_RLEG_STICKY_MASK . RLEG_MC_HDR_ERR_REDIR_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_RLEG_STICKY_MASK_RLEG_MC_HDR_ERR_REDIR_STICKY_MASK  VTSS_BIT(1)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_RLEG_STICKY_MASK . RLEG_NONIP_UC_REDIR_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_RLEG_STICKY_MASK_RLEG_NONIP_UC_REDIR_STICKY_MASK  VTSS_BIT(0)


/** 
 * \brief Enable per concurrent counter for each sticky bit
 *
 * \details
 * Each ANA_L3 sticky bit can be counted in each of the concurrent
 * counters.
 *
 * Register: \a ANA_L3:L3_STICKY_MASK:ROUT_STICKY_MASK
 *
 * @param gi Replicator: x_ANA_NUM_CONCURRENT_CNT (??), 0-3
 */
#define VTSS_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_L3,0xd537,gi,4,0,1)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK . WRONG_SIP_LPM_ACTION_TYPE_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK_WRONG_SIP_LPM_ACTION_TYPE_STICKY_MASK  VTSS_BIT(19)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK . WRONG_DIP_LPM_ACTION_TYPE_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK_WRONG_DIP_LPM_ACTION_TYPE_STICKY_MASK  VTSS_BIT(18)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK . IP6_SIP_RPF_FILTER_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK_IP6_SIP_RPF_FILTER_STICKY_MASK  VTSS_BIT(17)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK . IP4_SIP_RPF_FILTER_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK_IP4_SIP_RPF_FILTER_STICKY_MASK  VTSS_BIT(16)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK . NO_MC_VMID_AVAIL_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK_NO_MC_VMID_AVAIL_STICKY_MASK  VTSS_BIT(15)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK . ENTRY_NOT_FOUND_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK_ENTRY_NOT_FOUND_STICKY_MASK  VTSS_BIT(14)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK . MC_LOOPED_CP_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK_MC_LOOPED_CP_STICKY_MASK  VTSS_BIT(13)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK . UC_ZERO_DMAC_FOUND_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK_UC_ZERO_DMAC_FOUND_STICKY_MASK  VTSS_BIT(12)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK . UC_TTL_FILTERING_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK_UC_TTL_FILTERING_STICKY_MASK  VTSS_BIT(11)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK . UC_ICMP_REDIR_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK_UC_ICMP_REDIR_STICKY_MASK  VTSS_BIT(10)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK . UC_ENTRY_FOUND_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK_UC_ENTRY_FOUND_STICKY_MASK  VTSS_BIT(9)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK . NO_MC_FWD_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK_NO_MC_FWD_STICKY_MASK  VTSS_BIT(8)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK . L3_MC_FWD_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK_L3_MC_FWD_STICKY_MASK  VTSS_BIT(7)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK . MC_RPF_FILTER_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK_MC_RPF_FILTER_STICKY_MASK  VTSS_BIT(6)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK . L2_MC_FWD_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK_L2_MC_FWD_STICKY_MASK  VTSS_BIT(5)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK . IP_MAX_LEN_EXCEEDED_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK_IP_MAX_LEN_EXCEEDED_STICKY_MASK  VTSS_BIT(4)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK . LPM_IP6UC_FOUND_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK_LPM_IP6UC_FOUND_STICKY_MASK  VTSS_BIT(3)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK . LPM_IP4UC_FOUND_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK_LPM_IP4UC_FOUND_STICKY_MASK  VTSS_BIT(2)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK . LPM_IP6MC_FOUND_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK_LPM_IP6MC_FOUND_STICKY_MASK  VTSS_BIT(1)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK . LPM_IP4MC_FOUND_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_ROUT_STICKY_MASK_LPM_IP4MC_FOUND_STICKY_MASK  VTSS_BIT(0)


/** 
 * \brief Enable per concurrent counter for each sticky bit
 *
 * \details
 * Each ANA_L3 sticky bit can be counted in each of the concurrent
 * counters.
 *
 * Register: \a ANA_L3:L3_STICKY_MASK:SECUR_STICKY_MASK
 *
 * @param gi Replicator: x_ANA_NUM_CONCURRENT_CNT (??), 0-3
 */
#define VTSS_ANA_L3_L3_STICKY_MASK_SECUR_STICKY_MASK(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_L3,0xd537,gi,4,0,2)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_SECUR_STICKY_MASK . SECUR_IP6_DIP_MATCH_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_SECUR_STICKY_MASK_SECUR_IP6_DIP_MATCH_STICKY_MASK  VTSS_BIT(7)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_SECUR_STICKY_MASK . SECUR_IP4_SIP_MATCH_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_SECUR_STICKY_MASK_SECUR_IP4_SIP_MATCH_STICKY_MASK  VTSS_BIT(6)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_SECUR_STICKY_MASK . SECUR_SIP_FAIL_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_SECUR_STICKY_MASK_SECUR_SIP_FAIL_STICKY_MASK  VTSS_BIT(5)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_SECUR_STICKY_MASK . SECUR_IP6_SIP_MATCH_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_SECUR_STICKY_MASK_SECUR_IP6_SIP_MATCH_STICKY_MASK  VTSS_BIT(4)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_SECUR_STICKY_MASK . SECUR_IP6_LPM_FOUND_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_SECUR_STICKY_MASK_SECUR_IP6_LPM_FOUND_STICKY_MASK  VTSS_BIT(3)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_SECUR_STICKY_MASK . SECUR_IP4_DIP_MATCH_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_SECUR_STICKY_MASK_SECUR_IP4_DIP_MATCH_STICKY_MASK  VTSS_BIT(2)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_SECUR_STICKY_MASK . SECUR_IP4_LPM_FOUND_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_SECUR_STICKY_MASK_SECUR_IP4_LPM_FOUND_STICKY_MASK  VTSS_BIT(1)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_SECUR_STICKY_MASK . SECUR_DIP_FAIL_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_SECUR_STICKY_MASK_SECUR_DIP_FAIL_STICKY_MASK  VTSS_BIT(0)


/** 
 * \brief Enable per concurrent counter for each sticky bit
 *
 * \details
 * Each ANA_L3 sticky bit can be counted in each of the concurrent
 * counters.
 *
 * Register: \a ANA_L3:L3_STICKY_MASK:VLAN_MSTP_STICKY_MASK
 *
 * @param gi Replicator: x_ANA_NUM_CONCURRENT_CNT (??), 0-3
 */
#define VTSS_ANA_L3_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_L3,0xd537,gi,4,0,3)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK . PORT_LRN_DENY_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK_PORT_LRN_DENY_STICKY_MASK  VTSS_BIT(16)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK . PORT_FWD_DENY_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK_PORT_FWD_DENY_STICKY_MASK  VTSS_BIT(15)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK . MSTP_FWD_ALLOWED_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK_MSTP_FWD_ALLOWED_STICKY_MASK  VTSS_BIT(6)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK . MSTP_DISCARD_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK_MSTP_DISCARD_STICKY_MASK  VTSS_BIT(5)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK . VLAN_LRN_DENY_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK_VLAN_LRN_DENY_STICKY_MASK  VTSS_BIT(4)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK . MSTP_LRN_DENY_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK_MSTP_LRN_DENY_STICKY_MASK  VTSS_BIT(3)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK . MSTP_LRN_ALLOWED_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK_MSTP_LRN_ALLOWED_STICKY_MASK  VTSS_BIT(2)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK . VLAN_LOOKUP_INVLD_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK_VLAN_LOOKUP_INVLD_STICKY_MASK  VTSS_BIT(1)

/** 
 * \brief
 * Enable counting of sticky event per concurrent counter.
 *
 * \details 
 * Field: ::VTSS_ANA_L3_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK . VLAN_IGR_FILTER_STICKY_MASK
 */
#define  VTSS_F_ANA_L3_L3_STICKY_MASK_VLAN_MSTP_STICKY_MASK_VLAN_IGR_FILTER_STICKY_MASK  VTSS_BIT(0)


#endif /* _VTSS_JAGUAR2_REGS_ANA_L3_H_ */
