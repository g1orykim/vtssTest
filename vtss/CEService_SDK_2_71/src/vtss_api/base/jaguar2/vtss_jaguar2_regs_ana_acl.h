#ifndef _VTSS_JAGUAR2_REGS_ANA_ACL_H_
#define _VTSS_JAGUAR2_REGS_ANA_ACL_H_

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
 * Target: \a ANA_ACL
 *
 * Access Control List sub block of the Analyzer
 *
 ***********************************************************************/

/**
 * Register Group: \a ANA_ACL:PORT
 *
 * VCAP_IS2 configuration per port
 */


/** 
 * \brief VCAP_IS2 key selection
 *
 * \details
 * Key selection for VCAP_IS2 lookups. Replicated per lookup.
 *
 * Register: \a ANA_ACL:PORT:VCAP_S2_KEY_SEL
 *
 * @param gi Replicator: x_FFL_CHIP_NUM_INB_PORTS (??), 0-56
 * @param ri Register: VCAP_S2_KEY_SEL (??), 0-1
 */
#define VTSS_ANA_ACL_PORT_VCAP_S2_KEY_SEL(gi,ri)  VTSS_IOREG_IX(VTSS_TO_ANA_ACL,0x1000,gi,2,ri,0)

/** 
 * \brief
 * Applies to IPv4 multicast frames.
 *
 * \details 
 * 0: Match against MAC_ETYPE entries.
 * 1: Match against IP4_TCP_UDP for IPv4 TCP/UDP frames and against
 * IP4_OTHER entries for other IPv4 frames.
 * 2: Match against IP6_OTHER entries.
 * 3: Match against IP4_VID entries.

 *
 * Field: ::VTSS_ANA_ACL_PORT_VCAP_S2_KEY_SEL . IP4_MC_KEY_SEL
 */
#define  VTSS_F_ANA_ACL_PORT_VCAP_S2_KEY_SEL_IP4_MC_KEY_SEL(x)  VTSS_ENCODE_BITFIELD(x,6,2)
#define  VTSS_M_ANA_ACL_PORT_VCAP_S2_KEY_SEL_IP4_MC_KEY_SEL     VTSS_ENCODE_BITMASK(6,2)
#define  VTSS_X_ANA_ACL_PORT_VCAP_S2_KEY_SEL_IP4_MC_KEY_SEL(x)  VTSS_EXTRACT_BITFIELD(x,6,2)

/** 
 * \brief
 * Applies to IPv4 unicast frames.
 *
 * \details 
 * 0: Match against MAC_ETYPE entries.
 * 1: Match against IP4_TCP_UDP for IPv4 TCP/UDP frames and against
 * IP4_OTHER entries for other IPv4 frames.
 * 2: Match against IP6_OTHER entries.

 *
 * Field: ::VTSS_ANA_ACL_PORT_VCAP_S2_KEY_SEL . IP4_UC_KEY_SEL
 */
#define  VTSS_F_ANA_ACL_PORT_VCAP_S2_KEY_SEL_IP4_UC_KEY_SEL(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_ANA_ACL_PORT_VCAP_S2_KEY_SEL_IP4_UC_KEY_SEL     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_ANA_ACL_PORT_VCAP_S2_KEY_SEL_IP4_UC_KEY_SEL(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * Applies to IPv6 multicast frames.
 *
 * \details 
 * 0: Match against MAC_ETYPE entries.
 * 1: Match against IP6_OTHER entries.
 * 2: Match against IP6_VID entries.

 *
 * Field: ::VTSS_ANA_ACL_PORT_VCAP_S2_KEY_SEL . IP6_MC_KEY_SEL
 */
#define  VTSS_F_ANA_ACL_PORT_VCAP_S2_KEY_SEL_IP6_MC_KEY_SEL(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_ANA_ACL_PORT_VCAP_S2_KEY_SEL_IP6_MC_KEY_SEL     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_ANA_ACL_PORT_VCAP_S2_KEY_SEL_IP6_MC_KEY_SEL(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Applies to IPv6 unicast frames.
 *
 * \details 
 * 0: Match against MAC_ETYPE entries.
 * 1: Match against IP6_OTHER entries.

 *
 * Field: ::VTSS_ANA_ACL_PORT_VCAP_S2_KEY_SEL . IP6_UC_KEY_SEL
 */
#define  VTSS_F_ANA_ACL_PORT_VCAP_S2_KEY_SEL_IP6_UC_KEY_SEL  VTSS_BIT(1)

/** 
 * \brief
 * Applies to ARP/RARP frames.
 *
 * \details 
 * 0: Match against MAC_ETYPE entries.
 * 1: Match against ARP entries.

 *
 * Field: ::VTSS_ANA_ACL_PORT_VCAP_S2_KEY_SEL . ARP_KEY_SEL
 */
#define  VTSS_F_ANA_ACL_PORT_VCAP_S2_KEY_SEL_ARP_KEY_SEL  VTSS_BIT(0)

/**
 * Register Group: \a ANA_ACL:VCAP_S2
 *
 * Common configurations used by VCAP_IS2
 */


/** 
 * \brief VCAP S2 configuration
 *
 * \details
 * Configuration of advanced classification per port.
 * 
 * For the 2-bit fields of this register the following applies:
 * Bit[0]: Relates to first lookup in VCAP_IS2
 * Bit[1]: Relates to second lookup in VCAP_IS2
 *
 * Register: \a ANA_ACL:VCAP_S2:VCAP_S2_CFG
 *
 * @param ri Replicator: x_FFL_CHIP_NUM_INB_PORTS (??), 0-56
 */
#define VTSS_ANA_ACL_VCAP_S2_VCAP_S2_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_ACL,0x1072 + (ri))

/** 
 * \brief
 * For frames to be routed, enable/disable the use of IRLEG VID and ERLEG
 * VID as VID value in VCAP_ IS2 lookup (ref. VCAP_IS2:*_ENTRY:VID).
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_CFG . SEC_ROUTE_HANDLING_ENA
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_ROUTE_HANDLING_ENA  VTSS_BIT(26)

/** 
 * \brief
 * For OAM frames (with up to three VLAN tags), enable matching against
 * control entries of type OAM in VCAP_IS2. Otherwise, OAM frames are
 * matched against control entries of type MAC_ETYPE.
 * 
 * OAM frames are identified by the following EtherType values:
 * 0x8902 - ITU-T Y.1731
 * 0x8809 - Link Level OAM 
 * 0x88EE - MEF-16 (E-LMI)
 * 
 * Bit[0]: Relates to first lookup in VCAP_IS2
 * Bit[1]: Relates to second lookup in VCAP_IS2

 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_CFG . SEC_TYPE_OAM_ENA
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_OAM_ENA(x)  VTSS_ENCODE_BITFIELD(x,24,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_OAM_ENA     VTSS_ENCODE_BITMASK(24,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_OAM_ENA(x)  VTSS_EXTRACT_BITFIELD(x,24,2)

/** 
 * \brief
 * For IPv6 frames, enable matching against control entries of types
 * IP4_TCP_UDP and IP4_OTHER in VCAP_IS2.
 * 
 * The SIP and DIP fields of IP4_TCP_UDP and IP4_OTHER control entries are
 * used to match against bits 63:0 of IPv6 SIP.
 * 
 * Bit[0]: Relates to first lookup in VCAP_IS2
 * Bit[1]: Relates to second lookup in VCAP_IS2

 *
 * \details 
 * 0: Disable the classification.
 * 1: Enable the classification.
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_CFG . SEC_TYPE_IP6_TCPUDP_OTHER_ENA
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_TCPUDP_OTHER_ENA(x)  VTSS_ENCODE_BITFIELD(x,22,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_TCPUDP_OTHER_ENA     VTSS_ENCODE_BITMASK(22,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_TCPUDP_OTHER_ENA(x)  VTSS_EXTRACT_BITFIELD(x,22,2)

/** 
 * \brief
 * For IPv6 multicast frames, enable matching against control entries of
 * type IP6_VID in VCAP_IS2. Otherwise, IPv6 multicast frames are handled
 * as either IPv6 TCP/UDP frames or IPv6 Other frames, see
 * SEC_TYPE_IP6_TCPUDP_ENA and SEC_TYPE_IP6_OTHER_ENA.
 * 
 * Bit[0]: Relates to first lookup in VCAP_IS2
 * Bit[1]: Relates to second lookup in VCAP_IS2

 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_CFG . SEC_TYPE_IP6_VID_ENA
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_VID_ENA(x)  VTSS_ENCODE_BITFIELD(x,20,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_VID_ENA     VTSS_ENCODE_BITMASK(20,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_VID_ENA(x)  VTSS_EXTRACT_BITFIELD(x,20,2)

/** 
 * \brief
 * For IPv6 frames, enable matching against control entries of type IP6_STD
 * in VCAP_IS2. Otherwise, IPv6 frames are handled as IPv4 frames, see
 * SEC_TYPE_IP6_TCPUDP_OTHER_ENA.
 * 
 * Bit[0]: Relates to first lookup in VCAP_IS2
 * Bit[1]: Relates to second lookup in VCAP_IS2

 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_CFG . SEC_TYPE_IP6_STD_ENA
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_STD_ENA(x)  VTSS_ENCODE_BITFIELD(x,18,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_STD_ENA     VTSS_ENCODE_BITMASK(18,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_STD_ENA(x)  VTSS_EXTRACT_BITFIELD(x,18,2)

/** 
 * \brief
 * For IPv6 TCP/UDP frames, enable matching against control entries of type
 * IP6_TCP_UDP in VCAP_IS2. Otherwise, IPv6 TCP/UDP frames are handled as
 * IPv6 standard frames, SEC_TYPE_IP6_STD_ENA.
 * 
 * Bit[0]: Relates to first lookup in VCAP_IS2
 * Bit[1]: Relates to second lookup in VCAP_IS2

 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_CFG . SEC_TYPE_IP6_TCPUDP_ENA
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_TCPUDP_ENA(x)  VTSS_ENCODE_BITFIELD(x,16,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_TCPUDP_ENA     VTSS_ENCODE_BITMASK(16,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_TCPUDP_ENA(x)  VTSS_EXTRACT_BITFIELD(x,16,2)

/** 
 * \brief
 * For IPv6 frames, enable matching against control entries of type
 * IP6_OTHER in VCAP_IS2. Otherwise, IPv6 Other frames are handled as IPv6
 * standard frames, SEC_TYPE_IP6_STD_ENA.
 * 
 * Bit[0]: Relates to first lookup in VCAP_IS2
 * Bit[1]: Relates to second lookup in VCAP_IS2

 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_CFG . SEC_TYPE_IP6_OTHER_ENA
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_OTHER_ENA(x)  VTSS_ENCODE_BITFIELD(x,14,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_OTHER_ENA     VTSS_ENCODE_BITMASK(14,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP6_OTHER_ENA(x)  VTSS_EXTRACT_BITFIELD(x,14,2)

/** 
 * \brief
 * For IPv4 multicast frames, enable matching against control entries of
 * type IP4_VID in VCAP_IS2. Otherwise, IPv4 multicast frames are handled
 * as either IPv4 TCP/UDP frames or IPv4 Other frames, see
 * SEC_TYPE_IP4_TCPUDP_ENA and SEC_TYPE_IP4_OTHER_ENA.
 * 
 * Bit[0]: Relates to first lookup in VCAP_IS2
 * Bit[1]: Relates to second lookup in VCAP_IS2

 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_CFG . SEC_TYPE_IP4_VID_ENA
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP4_VID_ENA(x)  VTSS_ENCODE_BITFIELD(x,12,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP4_VID_ENA     VTSS_ENCODE_BITMASK(12,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP4_VID_ENA(x)  VTSS_EXTRACT_BITFIELD(x,12,2)

/** 
 * \brief
 * For IPv4 TCP/UDP frames, enable matching against control entries of type
 * IP_TCP_UDP in VCAP_IS2. Otherwise, IPv4 TCP/UDP frames are handled as
 * IPv4 Other frames, see SEC_TYPE_IP4_OTHER_ENA.
 * 
 * Bit[0]: Relates to first lookup in VCAP_IS2
 * Bit[1]: Relates to second lookup in VCAP_IS2

 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_CFG . SEC_TYPE_IP4_TCPUDP_ENA
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP4_TCPUDP_ENA(x)  VTSS_ENCODE_BITFIELD(x,10,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP4_TCPUDP_ENA     VTSS_ENCODE_BITMASK(10,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP4_TCPUDP_ENA(x)  VTSS_EXTRACT_BITFIELD(x,10,2)

/** 
 * \brief
 * For IPv4 frames, enable matching against control entries of type
 * IP4_OTHER in VCAP_IS2. Otherwise, IPv4 frames are matched against
 * control entries of type MAC_ETYPE.
 * 
 * Bit[0]: Relates to first lookup in VCAP_IS2
 * Bit[1]: Relates to second lookup in VCAP_IS2

 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_CFG . SEC_TYPE_IP4_OTHER_ENA
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP4_OTHER_ENA(x)  VTSS_ENCODE_BITFIELD(x,8,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP4_OTHER_ENA     VTSS_ENCODE_BITMASK(8,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_IP4_OTHER_ENA(x)  VTSS_EXTRACT_BITFIELD(x,8,2)

/** 
 * \brief
 * For ARP frames (EtherType 0x0806), enable matching against control
 * entries of type ARP in VCAP_IS2. Otherwise, ARP frames are matched
 * against control entries of type MAC_ETYPE.
 * 
 * Bit[0]: Relates to first lookup in VCAP_IS2
 * Bit[1]: Relates to second lookup in VCAP_IS2

 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_CFG . SEC_TYPE_ARP_ENA
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_ARP_ENA(x)  VTSS_ENCODE_BITFIELD(x,6,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_ARP_ENA     VTSS_ENCODE_BITMASK(6,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_ARP_ENA(x)  VTSS_EXTRACT_BITFIELD(x,6,2)

/** 
 * \brief
 * For SNAP frames, enable matching against control entries of type
 * MAC_SNAP in VCAP_IS2. Otherwise SNAP frames frames are handled as LLC
 * frames, see SEC_TYPE_MAC_LLC_ENA.
 * 
 * SNAP frames are identified by
 * * EtherType < 0x0600
 * * DSAP = 0xAA
 * * SSAP = 0xAA
 * * CTRL = 0x03
 * 
 * Bit[0]: Relates to first lookup in VCAP_IS2
 * Bit[1]: Relates to second lookup in VCAP_IS2

 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_CFG . SEC_TYPE_MAC_SNAP_ENA
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_MAC_SNAP_ENA(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_MAC_SNAP_ENA     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_MAC_SNAP_ENA(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * For LLC frames, enable matching against control entries of type MAC_LLC
 * in VCAP_IS2. Otherwise, LLC frames are matched against control entries
 * of type MAC_ETYPE.
 * 
 * LLC frames are identified as frames with EtherType < 0x0600 that are not
 * SNAP frames. Note that SNAP frames can be handled as LLC frames by
 * disabling SEC_TYPE_MAC_SNAP_ENA.
 * 
 * Bit[0]: Relates to first lookup in VCAP_IS2
 * Bit[1]: Relates to second lookup in VCAP_IS2

 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_CFG . SEC_TYPE_MAC_LLC_ENA
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_MAC_LLC_ENA(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_MAC_LLC_ENA     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_TYPE_MAC_LLC_ENA(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Enable/disable VCAP IS2 lookup.
 * 
 * Bit[0]: Relates to first lookup in VCAP_IS2
 * Bit[1]: Relates to second lookup in VCAP_IS2

 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_CFG . SEC_ENA
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_ENA     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_CFG_SEC_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Configuration of various features
 *
 * \details
 * Register: \a ANA_ACL:VCAP_S2:VCAP_S2_MISC_CTRL
 */
#define VTSS_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL  VTSS_IOREG(VTSS_TO_ANA_ACL,0x10ab)

/** 
 * \brief
 * Enables pipeline actions to be updated to ANA_VLAN if discarded by
 * ANA_L3.
 *
 * \details 
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL . VLAN_PIPELINE_ACT_ENA
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_VLAN_PIPELINE_ACT_ENA  VTSS_BIT(13)

/** 
 * \brief
 * Controls how to update routing statistics events for ingress ACL
 * actions.
 *
 * \details 
 * 0:
 * UC: If frame is dropped by ACL rule, then clear ivmid_ip_uc_received (as
 * if frame never reached the router)
 * MC: If frame is dropped by ACL rule, then leave ivmid_ip_mc_received
 * unchanged (as if frame may reach the router)
 * 1: If frame is dropped by ACL rule, then clear
 * ivmid_ip_uc_received/ivmid_ip_mc_received (as if frame never reached the
 * router, but was dropped in the L2 switch)
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL . ACL_RT_IGR_RLEG_STAT_MODE
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_ACL_RT_IGR_RLEG_STAT_MODE  VTSS_BIT(12)

/** 
 * \brief
 *  Controls how to update routing statistics events for egress ACL
 * actions.
 *
 * \details 
 * 0: If routed frame is dropped by ACL rule, then clear
 * ivmid_ip_uc_received/ivmid_ip_mc_received (as if frame never left the
 * router)
 * 1: If routed frame is dropped by ACL rule, then leave
 * ivmid_ip_uc_received/ivmid_ip_mc_received set (as if frame did leave the
 * router and was dropped in the L2 switch)
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL . ACL_RT_EGR_RLEG_STAT_MODE
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_ACL_RT_EGR_RLEG_STAT_MODE  VTSS_BIT(11)

/** 
 * \brief
 * If set, force use of VID key type in ES0 when routing in ACL block.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL . ACL_RT_FORCE_ES0_VID_ENA
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_ACL_RT_FORCE_ES0_VID_ENA  VTSS_BIT(10)

/** 
 * \brief
 * If set, Classified VID is set to egress VID.
 * 
 * Related parameters:
 * ANA_L3::ROUTING_CFG.RT_SMAC_UPDATE_ENA
 * ANA_ACL::VCAP_S2_MISC_CTRL.ACL_RT_SEL
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL . ACL_RT_UPDATE_CL_VID_ENA
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_ACL_RT_UPDATE_CL_VID_ENA  VTSS_BIT(9)

/** 
 * \brief
 * If set, IFH.GEN_IDX is set to egress RLEG (to be used by special ES0 key
 * mode).
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL . ACL_RT_UPDATE_GEN_IDX_ERLEG_ENA
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_ACL_RT_UPDATE_GEN_IDX_ERLEG_ENA  VTSS_BIT(8)

/** 
 * \brief
 * If set, IFH.GEN_IDX is set to egress VID (to be used by special ES0 key
 * mode).
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL . ACL_RT_UPDATE_GEN_IDX_EVID_ENA
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_ACL_RT_UPDATE_GEN_IDX_EVID_ENA  VTSS_BIT(7)

/** 
 * \brief
 * Controls routing in ACL block.
 * 
 * When ANA_ACL performs routing, ANA_ACL will
 * 1) Change DMAC to next-hop MAC address (as determined by ANA_L3).
 * 2) Set IFH.FWD.DST_MODE=ENCAP to prevent REW from doing routing related
 * frame editting.
 * 
 * Decrement of TTL/Hop Limit is still performed by REW. 
 * 
 * When performing routing in ANA_ACL, editting of SMAC must be performed
 * by ANA_L3. 
 * 
 * Related parameters:
 * ANA_L3::ROUTING_CFG.RT_SMAC_UPDATE_ENA
 * ANA_ACL::VCAP_S2_MISC_CTRL.ACL_RT_UPDATE_CL_VID_ENA
 *
 * \details 
 * 0: Disable (no routing in ACL block)
 * 1: Allow routing independently of ACL action
 * 2: Allow routing if ACL action allows routing (see IS2 action field
 * ACL_RT_MODE).
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL . ACL_RT_SEL
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_ACL_RT_SEL(x)  VTSS_ENCODE_BITFIELD(x,5,2)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_ACL_RT_SEL     VTSS_ENCODE_BITMASK(5,2)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_ACL_RT_SEL(x)  VTSS_EXTRACT_BITFIELD(x,5,2)

/** 
 * \brief
 * Force VCAP lookup to use IGR_PORT_MASK_SEL=3 for looped frames instead
 * of IGR_PORT_MASK_SEL=1.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL . LBK_IGR_MASK_SEL3_ENA
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_LBK_IGR_MASK_SEL3_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Enable VCAP key field IGR_PORT_MASK_SEL=2 for masqueraded frames.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL . MASQ_IGR_MASK_ENA
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_MASQ_IGR_MASK_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Enable VCAP key field IGR_PORT_MASK_SEL=3 for frames received with VStaX
 * header.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL . FP_VS2_IGR_MASK_ENA
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_FP_VS2_IGR_MASK_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Enable VCAP key field IGR_PORT_MASK_SEL=3 for frames from VD0 or VD1.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL . VD_IGR_MASK_ENA
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_VD_IGR_MASK_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable VCAP key field IGR_PORT_MASK_SEL=3 for CPU injected frames.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL . CPU_IGR_MASK_ENA
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_MISC_CTRL_CPU_IGR_MASK_ENA  VTSS_BIT(0)


/** 
 * \brief Configuration of TCP range generation
 *
 * \details
 * Register: \a ANA_ACL:VCAP_S2:VCAP_S2_RNG_CTRL
 *
 * @param ri Replicator: x_FFL_ANA_NUM_TCP_RANGES (??), 0-7
 */
#define VTSS_ANA_ACL_VCAP_S2_VCAP_S2_RNG_CTRL(ri)  VTSS_IOREG(VTSS_TO_ANA_ACL,0x10ac + (ri))

/** 
 * \brief
 * Selected field matched against the range
 *
 * \details 
 * 0: Idle (No match)
 * 1: TCP / UDP dport value is matched against range
 * 2: TCP / UDP sport value is matched against range
 * 3: TCP / UDP dport or sport values are matched against range
 * 4: Classified VIDvalue is matched against range
 * 5: Classified DSCP value is matched against range
 * 6: Selected value from frame is matched against range, see
 * ANA_ACL::VCAP_S2_RNG_OFFSET_CFG for details.
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_RNG_CTRL . RNG_TYPE_SEL
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_RNG_CTRL_RNG_TYPE_SEL(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_RNG_CTRL_RNG_TYPE_SEL     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_RNG_CTRL_RNG_TYPE_SEL(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Configuration of  matcher range generation
 *
 * \details
 * Register: \a ANA_ACL:VCAP_S2:VCAP_S2_RNG_VALUE_CFG
 *
 * @param ri Replicator: x_FFL_ANA_NUM_TCP_RANGES (??), 0-7
 */
#define VTSS_ANA_ACL_VCAP_S2_VCAP_S2_RNG_VALUE_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_ACL,0x10b4 + (ri))

/** 
 * \brief
 * Upper range value
 *
 * \details 
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_RNG_VALUE_CFG . RNG_MAX_VALUE
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_RNG_VALUE_CFG_RNG_MAX_VALUE(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_RNG_VALUE_CFG_RNG_MAX_VALUE     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_RNG_VALUE_CFG_RNG_MAX_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Lower range value
 *
 * \details 
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_RNG_VALUE_CFG . RNG_MIN_VALUE
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_RNG_VALUE_CFG_RNG_MIN_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_RNG_VALUE_CFG_RNG_MIN_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_RNG_VALUE_CFG_RNG_MIN_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Configuration of selected range generation
 *
 * \details
 * Register: \a ANA_ACL:VCAP_S2:VCAP_S2_RNG_OFFSET_CFG
 */
#define VTSS_ANA_ACL_VCAP_S2_VCAP_S2_RNG_OFFSET_CFG  VTSS_IOREG(VTSS_TO_ANA_ACL,0x10bc)

/** 
 * \brief
 * 16-bit offset position of selectable range matcher input counting from
 * the EtherType (up to three VLAN tags skipped).
 *
 * \details 
 * 0: EtherType
 * 1: frame byte 0 and 1 after EtherType
 * ...
 * n: frame byte 2n-2 and 2n-1 after EtherType
 *
 * Field: ::VTSS_ANA_ACL_VCAP_S2_VCAP_S2_RNG_OFFSET_CFG . RNG_OFFSET_POS
 */
#define  VTSS_F_ANA_ACL_VCAP_S2_VCAP_S2_RNG_OFFSET_CFG_RNG_OFFSET_POS(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_ANA_ACL_VCAP_S2_VCAP_S2_RNG_OFFSET_CFG_RNG_OFFSET_POS     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_ANA_ACL_VCAP_S2_VCAP_S2_RNG_OFFSET_CFG_RNG_OFFSET_POS(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

/**
 * Register Group: \a ANA_ACL:STICKY
 *
 * Sticky diagnostic status
 */


/** 
 * \brief Sticky bits register for events generated by advanced VCAP classification
 *
 * \details
 * Register: \a ANA_ACL:STICKY:SEC_LOOKUP_STICKY
 *
 * @param ri Register: SEC_LOOKUP_STICKY (??), 0-1
 */
#define VTSS_ANA_ACL_STICKY_SEC_LOOKUP_STICKY(ri)  VTSS_IOREG(VTSS_TO_ANA_ACL,0x10bd + (ri))

/** 
 * \brief
 * This sticky bit signals Custom2 lookup was performed.
 *
 * \details 
 * Field: ::VTSS_ANA_ACL_STICKY_SEC_LOOKUP_STICKY . SEC_TYPE_CUSTOM2_STICKY
 */
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_CUSTOM2_STICKY  VTSS_BIT(13)

/** 
 * \brief
 * This sticky bit signals Custom1 lookup was performed.
 *
 * \details 
 * Field: ::VTSS_ANA_ACL_STICKY_SEC_LOOKUP_STICKY . SEC_TYPE_CUSTOM1_STICKY
 */
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_CUSTOM1_STICKY  VTSS_BIT(12)

/** 
 * \brief
 * This sticky bit signals OAM lookup was performed.
 *
 * \details 
 * Field: ::VTSS_ANA_ACL_STICKY_SEC_LOOKUP_STICKY . SEC_TYPE_OAM_STICKY
 */
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_OAM_STICKY  VTSS_BIT(11)

/** 
 * \brief
 * This sticky bit signals IP6_VID lookup was performed.
 *
 * \details 
 * Field: ::VTSS_ANA_ACL_STICKY_SEC_LOOKUP_STICKY . SEC_TYPE_IP6_VID_STICKY
 */
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_IP6_VID_STICKY  VTSS_BIT(10)

/** 
 * \brief
 * This sticky bit signals IP6_STD lookup was performed.
 *
 * \details 
 * Field: ::VTSS_ANA_ACL_STICKY_SEC_LOOKUP_STICKY . SEC_TYPE_IP6_STD_STICKY
 */
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_IP6_STD_STICKY  VTSS_BIT(9)

/** 
 * \brief
 * This sticky bit signals IP6_TCPUDP lookup was performed.
 *
 * \details 
 * Field: ::VTSS_ANA_ACL_STICKY_SEC_LOOKUP_STICKY . SEC_TYPE_IP6_TCPUDP_STICKY
 */
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_IP6_TCPUDP_STICKY  VTSS_BIT(8)

/** 
 * \brief
 * This sticky bit signals IP6_OTHER lookup was performed.
 *
 * \details 
 * Field: ::VTSS_ANA_ACL_STICKY_SEC_LOOKUP_STICKY . SEC_TYPE_IP6_OTHER_STICKY
 */
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_IP6_OTHER_STICKY  VTSS_BIT(7)

/** 
 * \brief
 * This sticky bit signals IP4_VID lookup was performed.
 *
 * \details 
 * Field: ::VTSS_ANA_ACL_STICKY_SEC_LOOKUP_STICKY . SEC_TYPE_IP4_VID_STICKY
 */
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_IP4_VID_STICKY  VTSS_BIT(6)

/** 
 * \brief
 * This sticky bit signals IP4_TCPUDP lookup was performed.
 *
 * \details 
 * Field: ::VTSS_ANA_ACL_STICKY_SEC_LOOKUP_STICKY . SEC_TYPE_IP4_TCPUDP_STICKY
 */
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_IP4_TCPUDP_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * This sticky bit signals IP4_OTHER lookup was performed.
 *
 * \details 
 * Field: ::VTSS_ANA_ACL_STICKY_SEC_LOOKUP_STICKY . SEC_TYPE_IP4_OTHER_STICKY
 */
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_IP4_OTHER_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * This sticky bit signals ARP lookup was performed.
 *
 * \details 
 * Field: ::VTSS_ANA_ACL_STICKY_SEC_LOOKUP_STICKY . SEC_TYPE_ARP_STICKY
 */
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_ARP_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * This sticky bit signals MAC_SNAP lookup was performed.
 *
 * \details 
 * Field: ::VTSS_ANA_ACL_STICKY_SEC_LOOKUP_STICKY . SEC_TYPE_MAC_SNAP_STICKY
 */
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_MAC_SNAP_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * This sticky bit signals MAC_LLC lookup was performed.
 *
 * \details 
 * Field: ::VTSS_ANA_ACL_STICKY_SEC_LOOKUP_STICKY . SEC_TYPE_MAC_LLC_STICKY
 */
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_MAC_LLC_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * This sticky bit signals MAC_ETYPE lookup was performed.
 *
 * \details 
 * Field: ::VTSS_ANA_ACL_STICKY_SEC_LOOKUP_STICKY . SEC_TYPE_MAC_ETYPE_STICKY
 */
#define  VTSS_F_ANA_ACL_STICKY_SEC_LOOKUP_STICKY_SEC_TYPE_MAC_ETYPE_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a ANA_ACL:CNT_TBL
 *
 * ACL counter table
 */


/** 
 * \brief CAL counter
 *
 * \details
 * Register: \a ANA_ACL:CNT_TBL:CNT
 *
 * @param gi Replicator: x_FFL_ANA_NUM_ACL_CNT (??), 0-4095
 */
#define VTSS_ANA_ACL_CNT_TBL_CNT(gi)         VTSS_IOREG_IX(VTSS_TO_ANA_ACL,0x0,gi,1,0,0)


#endif /* _VTSS_JAGUAR2_REGS_ANA_ACL_H_ */
