/*

 Vitesse Switch Software.

==========================================================================

      Vitesse Switch Software

      Register Definitions

==========================================================================
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
 ==========================================================================
 #####DESCRIPTIONBEGIN####

 Author(s):    lpovlsen
 Contributors: 
 Date:         2011-10-18
 Purpose:      HAL register definitions
 Description:  This file contains register definitions used by the
               HAL.

 ####DESCRIPTIONEND####

*/
#ifndef _VTSS_JAGUAR_REGS_ANA_L2_H_
#define _VTSS_JAGUAR_REGS_ANA_L2_H_

#include "vtss_jaguar_regs_common.h"

/*********************************************************************** 
 *
 * Target: \a ANA_L2
 *
 * Layer 2 sub block of the Analyzer
 *
 ***********************************************************************/

/**
 * Register Group: \a ANA_L2:COMMON
 *
 * Common configurations for all ports
 */


/** 
 * \brief Common CCM configuration
 *
 * \details
 * Configures CCM handling.
 *
 * Register: \a ANA_L2:COMMON:CCM_CFG
 */
#define VTSS_ANA_L2_COMMON_CCM_CFG           VTSS_IOREG(VTSS_TO_ANA_L2,0x4400)

/** 
 * \brief
 * Configures the CPU queue for frames copied due to CCM CHG FLAG set in
 * CCM entry and enabled with: ANA_L2::CCM_CFG.CCM_CHG_COPY_ENA.
 * 
 * Frames are send to this CPU queue.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_L2_COMMON_CCM_CFG . CPU_CHG_COPY_OAM_QU
 */
#define  VTSS_F_ANA_L2_COMMON_CCM_CFG_CPU_CHG_COPY_OAM_QU(x)  VTSS_ENCODE_BITFIELD(x,13,3)
#define  VTSS_M_ANA_L2_COMMON_CCM_CFG_CPU_CHG_COPY_OAM_QU     VTSS_ENCODE_BITMASK(13,3)
#define  VTSS_X_ANA_L2_COMMON_CCM_CFG_CPU_CHG_COPY_OAM_QU(x)  VTSS_EXTRACT_BITFIELD(x,13,3)

/** 
 * \brief
 * Configures the CPU queue for frames without proper configured CCM entry
 * if enabled with: ANA_L2::CCM_CFG.CPU_WRONG_OAM_COPY_ENA.
 * 
 * Applicable frames are send to this CPU queue.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_L2_COMMON_CCM_CFG . CPU_WRONG_OAM_QU
 */
#define  VTSS_F_ANA_L2_COMMON_CCM_CFG_CPU_WRONG_OAM_QU(x)  VTSS_ENCODE_BITFIELD(x,10,3)
#define  VTSS_M_ANA_L2_COMMON_CCM_CFG_CPU_WRONG_OAM_QU     VTSS_ENCODE_BITMASK(10,3)
#define  VTSS_X_ANA_L2_COMMON_CCM_CFG_CPU_WRONG_OAM_QU(x)  VTSS_EXTRACT_BITFIELD(x,10,3)

/** 
 * \brief
 * Configures special CPU copy per CCM interval. This can be used as hit me
 * once CPU copy per configured CCM entry (which allows the next frame
 * received to be send to CPU). This can e.g. be used for periodic CPU
 * MEG_ID check.
 * 
 * Frames are send to the CPU queue configured in
 * ANA_L2::CCM_CFG.CPU_CHG_COPY_OAM_QU.
 *
 * \details 
 * x0x: no copy
 * x1x: Copy frames received at an active CCM entry with miss cnt > 0 and
 * CHG flag set
 * CHG flag cleared upon copy
 *
 * Field: ::VTSS_ANA_L2_COMMON_CCM_CFG . CCM_CHG_COPY_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_CCM_CFG_CCM_CHG_COPY_ENA(x)  VTSS_ENCODE_BITFIELD(x,7,3)
#define  VTSS_M_ANA_L2_COMMON_CCM_CFG_CCM_CHG_COPY_ENA     VTSS_ENCODE_BITMASK(7,3)
#define  VTSS_X_ANA_L2_COMMON_CCM_CFG_CCM_CHG_COPY_ENA(x)  VTSS_EXTRACT_BITFIELD(x,7,3)

/** 
 * \brief
 * Configures CPU copy operation for CCM frames with wrong MEPID per CCM
 * interval.
 * 
 * Frames are send to the CPU queue configured in
 * ANA_L2::CCM_CFG.CPU_WRONG_OAM_QU.
 *
 * \details 
 * x0x: no copy
 * x1x: Copy frames received with not matching MEPID to CPU
 *
 * Field: ::VTSS_ANA_L2_COMMON_CCM_CFG . CPU_COPY_MEPID_MISMATCH_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_CCM_CFG_CPU_COPY_MEPID_MISMATCH_ENA(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_ANA_L2_COMMON_CCM_CFG_CPU_COPY_MEPID_MISMATCH_ENA     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_ANA_L2_COMMON_CCM_CFG_CPU_COPY_MEPID_MISMATCH_ENA(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * Configures CPU copy operation of OAM CCM frames received after down per
 * CCM interval.
 * 
 * Frames are send to the CPU queue configured in
 * ANA_L2::CCM_CFG.CPU_CHG_COPY_OAM_QU.
 *
 * \details 
 * x0x: no copy
 * x1x: Copy frames received after down (CCM_MISS_CNT =3 and CCM_CHG flag
 * set)
 *
 * Field: ::VTSS_ANA_L2_COMMON_CCM_CFG . CPU_COPY_FRM_AFTER_DOWN_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_CCM_CFG_CPU_COPY_FRM_AFTER_DOWN_ENA(x)  VTSS_ENCODE_BITFIELD(x,1,3)
#define  VTSS_M_ANA_L2_COMMON_CCM_CFG_CPU_COPY_FRM_AFTER_DOWN_ENA     VTSS_ENCODE_BITMASK(1,3)
#define  VTSS_X_ANA_L2_COMMON_CCM_CFG_CPU_COPY_FRM_AFTER_DOWN_ENA(x)  VTSS_EXTRACT_BITFIELD(x,1,3)

/** 
 * \brief
 * Configures CPU copy operation for OAM CCM frames received without proper
 * configured CCM entity.
 * 
 * Frames are send to the CPU queue configured in
 * ANA_L2::CCM_CFG.CPU_WRONG_OAM_QU.
 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_CCM_CFG . CPU_WRONG_OAM_COPY_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_CCM_CFG_CPU_WRONG_OAM_COPY_ENA  VTSS_BIT(0)


/** 
 * \brief Host Mode configuration
 *
 * \details
 * Configures Host mode operation.
 *
 * Register: \a ANA_L2:COMMON:HM_CFG
 */
#define VTSS_ANA_L2_COMMON_HM_CFG            VTSS_IOREG(VTSS_TO_ANA_L2,0x4401)

/** 
 * \brief
 * Allows BDA forwarding to remote UPSID.
 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_HM_CFG . REMOTE_CONID_FWD_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_HM_CFG_REMOTE_CONID_FWD_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Configures MAC in MAC encapsulation towards Host port.
 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_HM_CFG . HM_CONID_FWD_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_HM_CFG_HM_CONID_FWD_ENA  VTSS_BIT(0)


/** 
 * \brief Forwarding configuration
 *
 * \details
 * Configure common forwarding options. 
 * 
 * Upon reception of a frame the (DMAC, EFID) is used to perform a lookup
 * in the MAC table. If an entry is found the destination is known and
 * forwarded according to the entry ADDE_TYPE and ADDR. Alternatively the
 * destination is considered unknown and might be applicable for flooding.
 *
 * Register: \a ANA_L2:COMMON:FWD_CFG
 */
#define VTSS_ANA_L2_COMMON_FWD_CFG           VTSS_IOREG(VTSS_TO_ANA_L2,0x4402)

/** 
 * \brief
 * Controls forwarding from entries with NXT_LRN_ALL set.

 *
 * \details 
 * 0: NXT_LRN_ALL not used for forwarding.
 * 1: Treat entries with NXT_LRN_ALL set as being unknown (triggers
 * flooding instead)
 *
 * Field: ::VTSS_ANA_L2_COMMON_FWD_CFG . BDA_UNKNOWN_WHEN_NXT_LRN_ALL_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_FWD_CFG_BDA_UNKNOWN_WHEN_NXT_LRN_ALL_ENA  VTSS_BIT(29)

/** 
 * \brief
 * Controls forwarding to C-domain of frames received from a B-port with
 * unknown multicast BDA
 *
 * \details 
 * 0: MAC in MAC encapsulated frames with unknown multicast BDA will not be
 * forwarded to C-domain.
 * 1: MAC in MAC encapsulated frames with unknown multicast BDA will
 * forwarded to C-domain.
 *
 * Field: ::VTSS_ANA_L2_COMMON_FWD_CFG . UNKNOWN_BMC_CBP_FWD_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_FWD_CFG_UNKNOWN_BMC_CBP_FWD_ENA  VTSS_BIT(28)

/** 
 * \brief
 * Controls forwarding to C-domain of frames received from a B-port with
 * unknown unicast BDA.
 *
 * \details 
 * 0: MAC in MAC encapsulated frames with unknown unicast BDA will not be
 * forwarded to C-domain.
 * 1: MAC in MAC encapsulated frames with unknown unicast BDA will
 * forwarded to C-domain.
 *
 * Field: ::VTSS_ANA_L2_COMMON_FWD_CFG . UNKNOWN_BUC_CBP_FWD_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_FWD_CFG_UNKNOWN_BUC_CBP_FWD_ENA  VTSS_BIT(27)

/** 
 * \brief
 * Configure use of (DMAC,EFID), (DIP_GRP, EFID) or (DIP_GRP, SIP) for IP
 * multicast
 * 
 * When set to use (DIP_GRP, EFID) for IPv4 multicast then DIP_GRP is
 * specified via LRN::MAC_ACCESS_CFG_1.MAC_ENTRY_MAC_LSB(27:0) and
 * LRN::MAC_ACCESS_CFG_0.MAC_ENTRY_MAC_MSB must be set to 0x0100
 * Note: these entries can be of LRN::MAC_ACCESS_CFG_2.MAC_ENTRY_TYPE: STD
 * or IPv4MC
 * 
 * When set to use (DIP_GRP, SIP) for IPv4 multicast then DIP_GRP is
 * specified via LRN::MAC_ACCESS_CFG_1.MAC_ENTRY_MAC_LSB(27:0)
 * SIP(3:0) is specified via LRN::MAC_ACCESS_CFG_1.MAC_ENTRY_MAC_LSB(31:28)
 * SIP(19:4) is specified via LRN::MAC_ACCESS_CFG_0.MAC_ENTRY_MAC_MSB)
 * SIP(31:20) is specified via LRN::MAC_ACCESS_CFG_0.MAC_ENTRY_FID
 * Note: these entries can only be of LRN::MAC_ACCESS_CFG_2.MAC_ENTRY_TYPE:
 * STD 
 *
 * \details 
 * 0: Use (DMAC, EFID)
 * 1: Use (DIP_GRP, EFID)
 * 2: Use (DIP_GRP, SIP)
 *
 * Field: ::VTSS_ANA_L2_COMMON_FWD_CFG . FWD_KEY_SEL
 */
#define  VTSS_F_ANA_L2_COMMON_FWD_CFG_FWD_KEY_SEL(x)  VTSS_ENCODE_BITFIELD(x,25,2)
#define  VTSS_M_ANA_L2_COMMON_FWD_CFG_FWD_KEY_SEL     VTSS_ENCODE_BITMASK(25,2)
#define  VTSS_X_ANA_L2_COMMON_FWD_CFG_FWD_KEY_SEL(x)  VTSS_EXTRACT_BITFIELD(x,25,2)

/** 
 * \brief
 * Enables use of MAC in MAC and dual domain functionality
 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_FWD_CFG . DUAL_DOMAIN_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_FWD_CFG_DUAL_DOMAIN_ENA  VTSS_BIT(24)

/** 
 * \brief
 * Configures MAC table split between CMAC table and BMAC table
 *
 * \details 
 * 0: No MAC tables
 * 1: 8K CMAC table
 * 2: 16K CMAC table
 * 3: 32K CMAC table
 * 4: No CMAC table and 32K (+4K direct addressable via conid) BMAC table
 * 5: Reserved
 * 6: 16K CMAC table and 16K (+4K direct addressable via conid) BMAC table
 * 7: 32K CMAC table and 4K BMAC table
 *
 * Field: ::VTSS_ANA_L2_COMMON_FWD_CFG . MAC_TBL_SPLIT_SEL
 */
#define  VTSS_F_ANA_L2_COMMON_FWD_CFG_MAC_TBL_SPLIT_SEL(x)  VTSS_ENCODE_BITFIELD(x,20,3)
#define  VTSS_M_ANA_L2_COMMON_FWD_CFG_MAC_TBL_SPLIT_SEL     VTSS_ENCODE_BITMASK(20,3)
#define  VTSS_X_ANA_L2_COMMON_FWD_CFG_MAC_TBL_SPLIT_SEL(x)  VTSS_EXTRACT_BITFIELD(x,20,3)

/** 
 * \brief
 * Configures use of port based Dual leaky bucket index when no service has
 * been selected (ISDX == 0).
 * 
 * Per port index configured via  ANA_L2::PORT_DLB_CFG.DLB_IDX.

 *
 * \details 
 * 0: Disable. ANA_L2:ISDX[0]:DLB_CFG.DLB_IDX used for all ISDX_VAL values.
 * 1: Enable. ANA_L2::PORT_DLB_CFG[port_num].DLB_IDX used when ISDX_VAL=0
 *
 * Field: ::VTSS_ANA_L2_COMMON_FWD_CFG . ISDX_DEFAULT_DLB_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_FWD_CFG_ISDX_DEFAULT_DLB_ENA  VTSS_BIT(17)

/** 
 * \brief
 * Enables service forwarding based on lookup of ISDX in ISDX table.
 * 
 * See ANA_L2::ISDX
 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_FWD_CFG . ISDX_LOOKUP_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_FWD_CFG_ISDX_LOOKUP_ENA  VTSS_BIT(16)

/** 
 * \brief
 * Configures the CPU queue for forwarded frames that have the CPU_COPY
 * flag set when enabled with:	ANA_L2::FWD_CFG.CPU_DMAC_COPY_ENA.
 * 
 * Applicable frames are send to this CPU queue.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_L2_COMMON_FWD_CFG . CPU_DMAC_QU
 */
#define  VTSS_F_ANA_L2_COMMON_FWD_CFG_CPU_DMAC_QU(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_ANA_L2_COMMON_FWD_CFG_CPU_DMAC_QU     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_ANA_L2_COMMON_FWD_CFG_CPU_DMAC_QU(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * Configures loopback for frames with DMAC = SMAC and bit 41 in the MAC
 * address set (= local administrated).
 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_FWD_CFG . LOOPBACK_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_FWD_CFG_LOOPBACK_ENA  VTSS_BIT(7)

/** 
 * \brief
 * Configures CPU copy of frames where (DMAC, EFID) is known in the MAC
 * table and the entry has the CPU_COPY bit set.
 * 
 * Frames are send to the CPU queue configured in
 * ANA_L2::FWD_CFG.CPU_DMAC_QU.
 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_FWD_CFG . CPU_DMAC_COPY_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_FWD_CFG_CPU_DMAC_COPY_ENA  VTSS_BIT(6)

/** 
 * \brief
 * Configures use of LRN.MAC_ACCESS_CFG_2.MAC_ENTRY_VLAN_IGNORE FLAG from
 * the MAC table entries as VLAN_IGNORE or as SRC_MASK_IGNORE.
 * 
 * This also configures the use of ANA_L2::FWD_CFG.FLOOD_IGNORE_VLAN_ENA
 * when frames are flooded.
 *
 * \details 
 * 0: Use as VLAN ignore which means that the VLAN mask from
 * ANA_L3.VLAN_MASK:CFG.VLAN_PORT_MASK is not applied when forwarding via
 * entry with VLAN_IGNORE set.
 * 1: Use as Source mask ignore which means that the SOURCE mask from
 * ANA_AC::SRC_CFG.PORT_MASK is not applied when forwarding (frames can be
 * send back to the sending port)
 *
 * Field: ::VTSS_ANA_L2_COMMON_FWD_CFG . FILTER_MODE_SEL
 */
#define  VTSS_F_ANA_L2_COMMON_FWD_CFG_FILTER_MODE_SEL  VTSS_BIT(4)

/** 
 * \brief
 * Configures mirroring operation for flooded frames. See
 * ANA_AC::MIRROR_PROBE.PROBE_MAC_MODE
 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_FWD_CFG . FLOOD_MIRROR_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_FWD_CFG_FLOOD_MIRROR_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Configures VLAN_IGNORE operation for flooded frames. 
 * 
 * See also ANA_L2::FWD_CFG.FILTER_MODE_SEL.
 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_FWD_CFG . FLOOD_IGNORE_VLAN_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_FWD_CFG_FLOOD_IGNORE_VLAN_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Configures CPU copy operation for flooded frames.
 * 
 * Frames are send to the CPU queue configured in
 * ANA_L2::FWD_CFG.CPU_DMAC_QU.
 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_FWD_CFG . FLOOD_CPU_COPY_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_FWD_CFG_FLOOD_CPU_COPY_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Configures use of MAC table for forwarding.
 *
 * \details 
 * 0: Disable all MAC fwd related lookups. All frames are flooded.
 * 1: Enable all MAC fwd related lookups
 *
 * Field: ::VTSS_ANA_L2_COMMON_FWD_CFG . FWD_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_FWD_CFG_FWD_ENA  VTSS_BIT(0)


/** 
 * \brief Table initializations
 *
 * \details
 * Controls initialization of the tables to an known state.
 *
 * Register: \a ANA_L2:COMMON:TABLE_CLR_CFG
 */
#define VTSS_ANA_L2_COMMON_TABLE_CLR_CFG     VTSS_IOREG(VTSS_TO_ANA_L2,0x4403)

/** 
 * \brief
 * Initialize ANA_L2::MAC table table.	The bit is cleared by HW when Init
 * is complete
 *
 * \details 
 * 0: Idle (no clear)
 * 1: Initialize table
 *
 * Field: ::VTSS_ANA_L2_COMMON_TABLE_CLR_CFG . MAC_TBL_INIT_SHOT
 */
#define  VTSS_F_ANA_L2_COMMON_TABLE_CLR_CFG_MAC_TBL_INIT_SHOT  VTSS_BIT(2)

/** 
 * \brief
 * Initialize ANA_L2::ISDX table.  The bit is cleared by HW when Init is
 * complete
 *
 * \details 
 * 0: Idle (no clear)
 * 1: Initialize table
 *
 * Field: ::VTSS_ANA_L2_COMMON_TABLE_CLR_CFG . ISDX_INIT_SHOT
 */
#define  VTSS_F_ANA_L2_COMMON_TABLE_CLR_CFG_ISDX_INIT_SHOT  VTSS_BIT(1)

/** 
 * \brief
 * Initialize ANA_L2::PATHGRP table. The bit is cleared by HW when Init is
 * complete
 *
 * \details 
 * 0: Idle (no clear)
 * 1: Initialize table
 *
 * Field: ::VTSS_ANA_L2_COMMON_TABLE_CLR_CFG . PATHGRP_INIT_SHOT
 */
#define  VTSS_F_ANA_L2_COMMON_TABLE_CLR_CFG_PATHGRP_INIT_SHOT  VTSS_BIT(0)


/** 
 * \brief Learn configuration (C-domain)
 *
 * \details
 * Configures learn handling. Upon reception of a frame the (SMAC, IFID) is
 * used to perform a lookup in the MAC table. If an entry is found the
 * station sending the frame is considered known and the SOURCE entry is
 * retrieved from the MAC table - alternatively the station is considered
 * unknown.
 * 
 * A comparison of the received port information and the port information
 * known is performed for known stations to detection port move.
 * 
 * A frame with changed or unknown station information is considered a
 * learn frame.
 *
 * Register: \a ANA_L2:COMMON:LRN_CFG
 */
#define VTSS_ANA_L2_COMMON_LRN_CFG           VTSS_IOREG(VTSS_TO_ANA_L2,0x4404)

/** 
 * \brief
 * Configures copy of frames from unknown stations (unknown MAC table
 * source entry) when port learn limit is exceeded (see
 * ANA_L2::LRN_CNT_CTRL.AUTO_LRN_CNT_MAX) .
 * 
 * Frames are send to the CPU queue configured in
 * ANA_L2::LRN_CFG.CPU_LRN_QU.

 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_LRN_CFG . CPU_COPY_LRN_LIMIT_EXCEED_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_LRN_CFG_CPU_COPY_LRN_LIMIT_EXCEED_ENA  VTSS_BIT(18)

/** 
 * \brief
 * Configures the AGE_FLAG size
 *
 * \details 
 * 0: one bit ageing periods used
 * 1: two bit ageing periods used.
 *
 * Field: ::VTSS_ANA_L2_COMMON_LRN_CFG . AGE_SIZE
 */
#define  VTSS_F_ANA_L2_COMMON_LRN_CFG_AGE_SIZE  VTSS_BIT(17)

/** 
 * \brief
 * Configures use Basic or advanced VStaX2 learning mode.
 *
 * \details 
 * 0: Entries are learned on UPSID and UPSPN ports (VSTAX Advanced
 * forwarding mode)
 * 1: Entries are learned on stacking ports (VSTAX basic forwarding mode).
 *
 * Field: ::VTSS_ANA_L2_COMMON_LRN_CFG . VSTAX_BASIC_LRN_MODE_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_LRN_CFG_VSTAX_BASIC_LRN_MODE_ENA  VTSS_BIT(16)

/** 
 * \brief
 * Configures how already known MAC table entries are replace of in case of
 * hash depletion when receiving traffic from an unknown station.
 *
 * \details 
 * 0: Non locked entries are replaced
 * 1: Known entries are never replaced.
 *
 * Field: ::VTSS_ANA_L2_COMMON_LRN_CFG . AUTO_LRN_REPLACE_DIS
 */
#define  VTSS_F_ANA_L2_COMMON_LRN_CFG_AUTO_LRN_REPLACE_DIS  VTSS_BIT(13)

/** 
 * \brief
 * Configures the CPU queue for source entries with the CPU_COPY flag set.
 * Frames enabled with:  ANA_L2::LRN_CFG.CPU_SMAC_COPY_ENA are send to this
 * CPU queue.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_L2_COMMON_LRN_CFG . CPU_SMAC_QU
 */
#define  VTSS_F_ANA_L2_COMMON_LRN_CFG_CPU_SMAC_QU(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_ANA_L2_COMMON_LRN_CFG_CPU_SMAC_QU     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_ANA_L2_COMMON_LRN_CFG_CPU_SMAC_QU(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * Configures the CPU queue for CPU based learning when enabled via
 * ANA_L2::LRN_COPY_CFG. 
 * 
 * Frames are send to this CPU queue.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_L2_COMMON_LRN_CFG . CPU_LRN_QU
 */
#define  VTSS_F_ANA_L2_COMMON_LRN_CFG_CPU_LRN_QU(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_ANA_L2_COMMON_LRN_CFG_CPU_LRN_QU     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_ANA_L2_COMMON_LRN_CFG_CPU_LRN_QU(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * Configures clearing of the age bit in the source entry
 * (LRN::MAC_ACCESS_CFG_2.MAC_ENTRY_AGE_FLAG) when receiving traffic from
 * known stations which has the  LOCKED bit
 * (LRN::MAC_ACCESS_CFG_2.MAC_ENTRY_LOCKED) in the source entry set.

 *
 * \details 
 * 0: LOCKED entries are not touched.
 * 1: The AGE flag for LOCKED entries will be cleared.
 *
 * Field: ::VTSS_ANA_L2_COMMON_LRN_CFG . AGE_LOCKED_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_LRN_CFG_AGE_LOCKED_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Configures copy of frames from known stations with changed port
 * (LRN::MAC_ACCESS_CFG_2.MAC_ENTRY_ADDR_TYPE and
 * LRN::MAC_ACCESS_CFG_2.MAC_ENTRY_ADDR) in the source entry and the LOCKED
 * bit (LRN::MAC_ACCESS_CFG_2.MAC_ENTRY_LOCKED) set.
 * 
 * Copies are send to the CPU queue specified by
 * ANA_L2::LRN_CFG.CPU_LRN_QU.
 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_LRN_CFG . LOCKED_PORTMOVE_COPY_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_LRN_CFG_LOCKED_PORTMOVE_COPY_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Configures detection of frames from known stations with changed port
 * (LRN::MAC_ACCESS_CFG_2.MAC_ENTRY_ADDR_TYPE and
 * LRN::MAC_ACCESS_CFG_2.MAC_ENTRY_ADDR) in the source entry and the LOCKED
 * bit (LRN::MAC_ACCESS_CFG_2.MAC_ENTRY_LOCKED) set.
 * 
 * When enabled, frames from static (LOCKED) entries with changed port
 * triggers ANA_L2::MOVELOG_STICKY ..
 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_LRN_CFG . LOCKED_PORTMOVE_DETECT_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_LRN_CFG_LOCKED_PORTMOVE_DETECT_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Configures CPU copy of frames from known stations with the CPU_COPY bit
 * set in the SOURCE entry. 
 * 
 * Copies are send to the CPU queue specified by
 * ANA_L2::LRN_CFG.CPU_SMAC_QU.
 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_LRN_CFG . CPU_SMAC_COPY_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_LRN_CFG_CPU_SMAC_COPY_ENA  VTSS_BIT(0)


/** 
 * \brief Remote scan filter configuration
 *
 * \details
 * Register: \a ANA_L2:COMMON:FILTER_OTHER_CTRL
 */
#define VTSS_ANA_L2_COMMON_FILTER_OTHER_CTRL  VTSS_IOREG(VTSS_TO_ANA_L2,0x4405)

/** 
 * \brief
 * Configures additional filtering of Remote entries (i.e. entries with
 * UPSID != ANA_L2::VSTAX_CTRL.OWN_UPSID) for automatic ageing and CPU
 * scan.
 * 
 * See LRN::SCAN_NEXT_CFG.SCAN_USE_PORT_FILTER_ENA and 
 * LRN::AUTOAGE_CFG.AUTOAGE_USE_PORT_FILTER_ENA.

 *
 * \details 
 * 0: handle remote entries during automatic ageing and CPU scan
 * 1: ignore remote entries during Automatic ageing and CPU scan

 *
 * Field: ::VTSS_ANA_L2_COMMON_FILTER_OTHER_CTRL . FILTER_REMOTE_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_FILTER_OTHER_CTRL_FILTER_REMOTE_ENA  VTSS_BIT(0)


/** 
 * \brief Front Port scan filter configuration
 *
 * \details
 * Register: \a ANA_L2:COMMON:FILTER_LOCAL_CTRL
 */
#define VTSS_ANA_L2_COMMON_FILTER_LOCAL_CTRL  VTSS_IOREG(VTSS_TO_ANA_L2,0x4406)


/** 
 * \brief Learn options for B-domain
 *
 * \details
 * Configures learn handling in BMAC table. Upon reception of a frame the
 * (SMAC, BVID) is used to perform a lookup in the MAC table. If an entry
 * is found the station sending the frame is considered known and the
 * SOURCE entry is retrieved from the MAC table - alternatively the station
 * is considered unknown.
 * 
 * A comparison of the received port information and the port information
 * known is performed for known stations to detection port move.
 *
 * Register: \a ANA_L2:COMMON:BLRN_CFG
 */
#define VTSS_ANA_L2_COMMON_BLRN_CFG          VTSS_IOREG(VTSS_TO_ANA_L2,0x4407)

/** 
 * \brief
 * Configures the CPU queue for PBB_TE frames from wrong or unknown source
 * station information when enabled via ANA_L2::PBB_TE_LRN_CHK_ENA. 
 * 
 * Applicable frames are send to this CPU queue.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_L2_COMMON_BLRN_CFG . PBB_TE_LRN_CHK_QU
 */
#define  VTSS_F_ANA_L2_COMMON_BLRN_CFG_PBB_TE_LRN_CHK_QU(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_ANA_L2_COMMON_BLRN_CFG_PBB_TE_LRN_CHK_QU     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_ANA_L2_COMMON_BLRN_CFG_PBB_TE_LRN_CHK_QU(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * Configures the CPU queue for frames to be learned via CPU when enabled
 * via ANA_L2::BLRN_COPY_CFG. 
 * 
 * Applicable frames are send to this CPU queue.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_L2_COMMON_BLRN_CFG . CPU_BLRN_QU
 */
#define  VTSS_F_ANA_L2_COMMON_BLRN_CFG_CPU_BLRN_QU(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_ANA_L2_COMMON_BLRN_CFG_CPU_BLRN_QU     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_ANA_L2_COMMON_BLRN_CFG_CPU_BLRN_QU(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * Configures copy of frames in PBB-TE VLANS from unknown or known source
 * stations with changed port (LRN::MAC_ACCESS_CFG_2.MAC_ENTRY_ADDR_TYPE
 * and LRN::MAC_ACCESS_CFG_2.MAC_ENTRY_ADDR) in the source entry and the
 * LOCKED bit (LRN::MAC_ACCESS_CFG_2.MAC_ENTRY_LOCKED) set.
 * 
 * PBB-TE VLANs must be configured with ANA_L3::BVLAN_CFG.BVLAN_LRN_DIS = 1
 * and ANA_L3::BVLAN_CFG.BVLAN_SEC_FWD_ENA =1
 * 
 * Copies are send to the CPU queue specified by
 * ANA_L2::BLRN_CFG.PBB_TE_LRN_CHK_QU.
 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_BLRN_CFG . PBB_TE_LRN_CHK_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_BLRN_CFG_PBB_TE_LRN_CHK_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Configures copy of frames from known stations with changed port
 * (LRN::MAC_ACCESS_CFG_2.MAC_ENTRY_ADDR_TYPE and
 * LRN::MAC_ACCESS_CFG_2.MAC_ENTRY_ADDR) in the source entry and the LOCKED
 * bit (LRN::MAC_ACCESS_CFG_2.MAC_ENTRY_LOCKED) set.
 * 
 * Copies are send to the CPU queue specified by
 * ANA_L2::BLRN_CFG.CPU_BLRN_QU.
 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_BLRN_CFG . BLOCKED_PORTMOVE_COPY_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_BLRN_CFG_BLOCKED_PORTMOVE_COPY_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Configures detection of frames from known stations with changed port
 * (LRN::MAC_ACCESS_CFG_2.MAC_ENTRY_ADDR_TYPE and
 * LRN::MAC_ACCESS_CFG_2.MAC_ENTRY_ADDR) in the source entry and the LOCKED
 * bit (LRN::MAC_ACCESS_CFG_2.MAC_ENTRY_LOCKED) set.
 * 
 * When enabled, frames from static (LOCKED) entries with changed port
 * triggers ANA_L2::MOVELOG_STICKY .
 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_BLRN_CFG . BLOCKED_PORTMOVE_DETECT_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_BLRN_CFG_BLOCKED_PORTMOVE_DETECT_ENA  VTSS_BIT(0)


/** 
 * \brief Auto learn configuration
 *
 * \details
 * Configures automatic learning per port 
 *
 * Register: \a ANA_L2:COMMON:AUTO_LRN_CFG
 */
#define VTSS_ANA_L2_COMMON_AUTO_LRN_CFG      VTSS_IOREG(VTSS_TO_ANA_L2,0x4408)


/** 
 * \brief Secure learn configuration
 *
 * \details
 * Configures secure forwarding per port 
 *
 * Register: \a ANA_L2:COMMON:LRN_SECUR_CFG
 */
#define VTSS_ANA_L2_COMMON_LRN_SECUR_CFG     VTSS_IOREG(VTSS_TO_ANA_L2,0x4409)


/** 
 * \brief Secure learn configuration for locked entries
 *
 * \details
 * Configures secure forwarding for static (LOCKED) entries per port 
 *
 * Register: \a ANA_L2:COMMON:LRN_SECUR_LOCKED_CFG
 */
#define VTSS_ANA_L2_COMMON_LRN_SECUR_LOCKED_CFG  VTSS_IOREG(VTSS_TO_ANA_L2,0x440a)


/** 
 * \brief Learn copy configuration
 *
 * \details
 * Configures CPU copy of learn frames per port
 *
 * Register: \a ANA_L2:COMMON:LRN_COPY_CFG
 */
#define VTSS_ANA_L2_COMMON_LRN_COPY_CFG      VTSS_IOREG(VTSS_TO_ANA_L2,0x440b)


/** 
 * \brief Auto learn configuration for BMAC table
 *
 * \details
 * Configures automatic B-domain learning for per port
 *
 * Register: \a ANA_L2:COMMON:AUTO_BLRN_CFG
 */
#define VTSS_ANA_L2_COMMON_AUTO_BLRN_CFG     VTSS_IOREG(VTSS_TO_ANA_L2,0x440c)


/** 
 * \brief Secure learn configuration for BMAC table
 *
 * \details
 * Configures secure B-domain forwarding per port 
 *
 * Register: \a ANA_L2:COMMON:BLRN_SECUR_CFG
 */
#define VTSS_ANA_L2_COMMON_BLRN_SECUR_CFG    VTSS_IOREG(VTSS_TO_ANA_L2,0x440d)


/** 
 * \brief Learn copy configuration for BMAC table
 *
 * \details
 * Configures CPU copy of B-domain learn frames per port
 *
 * Register: \a ANA_L2:COMMON:BLRN_COPY_CFG
 */
#define VTSS_ANA_L2_COMMON_BLRN_COPY_CFG     VTSS_IOREG(VTSS_TO_ANA_L2,0x440e)


/** 
 * \brief Service DLB Port default configuration
 *
 * \details
 * Configures per port dual leaky bucket indexes
 *
 * Register: \a ANA_L2:COMMON:PORT_DLB_CFG
 *
 * @param ri Replicator: x_ANA_NUM_PORTS (??), 0-31
 */
#define VTSS_ANA_L2_COMMON_PORT_DLB_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_L2,0x440f + (ri))

/** 
 * \brief
 * Specifies the default port Dual leaky bucket index to be used for frames
 * with ISDX=0 if ANA_L2::FWD_CFG.ISDX_DEFAULT_DLB_ENA is enabled.

 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_PORT_DLB_CFG . PORT_DLB_IDX
 */
#define  VTSS_F_ANA_L2_COMMON_PORT_DLB_CFG_PORT_DLB_IDX(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_ANA_L2_COMMON_PORT_DLB_CFG_PORT_DLB_IDX     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_ANA_L2_COMMON_PORT_DLB_CFG_PORT_DLB_IDX(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \brief Port CCM configuration
 *
 * \details
 * Configures per port CCM entries
 *
 * Register: \a ANA_L2:COMMON:PORT_CCM_CTRL
 *
 * @param ri Replicator: x_ANA_NUM_PORTS (??), 0-31
 */
#define VTSS_ANA_L2_COMMON_PORT_CCM_CTRL(ri)  VTSS_IOREG(VTSS_TO_ANA_L2,0x442f + (ri))

/** 
 * \brief
 * Configures the expected CCM MEPID. 
 *
 * \details 
 * 0: No MEPID check
 * other: Expected MEPID
 *
 * Field: ::VTSS_ANA_L2_COMMON_PORT_CCM_CTRL . CCM_MEPID
 */
#define  VTSS_F_ANA_L2_COMMON_PORT_CCM_CTRL_CCM_MEPID(x)  VTSS_ENCODE_BITFIELD(x,8,16)
#define  VTSS_M_ANA_L2_COMMON_PORT_CCM_CTRL_CCM_MEPID     VTSS_ENCODE_BITMASK(8,16)
#define  VTSS_X_ANA_L2_COMMON_PORT_CCM_CTRL_CCM_MEPID(x)  VTSS_EXTRACT_BITFIELD(x,8,16)

/** 
 * \brief
 * Indicate whether Loss of Continuity (LOC) has been declared for the port
 * CCM. 
 * 
 * This flag can also be used for "CPU hit me once operation" where the
 * next CCM message is send to CPU. See ANA_L2::CCM_CFG.CCM_CHG_COPY_ENA.
 * 
 * Clear this bit to receive new down interrupts for this port MEP monitor.
 *
 * \details 
 * 0: No LOC has been decleared.
 * 1: LOC has been decleared (but might have been cleared again if
 * CCM_MISS_CNT is less than three)
 *
 * Field: ::VTSS_ANA_L2_COMMON_PORT_CCM_CTRL . CCM_CHG
 */
#define  VTSS_F_ANA_L2_COMMON_PORT_CCM_CTRL_CCM_CHG  VTSS_BIT(4)

/** 
 * \brief
 * Specifies the number of CCM periods has passed without CCM mesages
 * received from the peer MEP.
 * 
 * Cleared by HW when receiving a CCM message from the peer associated with
 * this port MEP monitor. A CCM message is identified via VCAP_S2 with an
 * associated action: CCM_TYPE set to PORT. 
 * 
 * Can be configured to periodically increment CCM miss count via HW
 * configured by LRN::CCM_CTRL.
 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_PORT_CCM_CTRL . CCM_MISS_CNT
 */
#define  VTSS_F_ANA_L2_COMMON_PORT_CCM_CTRL_CCM_MISS_CNT(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_ANA_L2_COMMON_PORT_CCM_CTRL_CCM_MISS_CNT     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_ANA_L2_COMMON_PORT_CCM_CTRL_CCM_MISS_CNT(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Configures CCM monitor interval.
 *
 * \details 
 * 0: not ccm
 * 1: ccm fault management
 * 2: performance management
 * 3: protection switching
 *
 * Field: ::VTSS_ANA_L2_COMMON_PORT_CCM_CTRL . CCM_TYPE_INTERVAL
 */
#define  VTSS_F_ANA_L2_COMMON_PORT_CCM_CTRL_CCM_TYPE_INTERVAL(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_ANA_L2_COMMON_PORT_CCM_CTRL_CCM_TYPE_INTERVAL     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_ANA_L2_COMMON_PORT_CCM_CTRL_CCM_TYPE_INTERVAL(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Sticky Portmove status
 *
 * \details
 * Identifies ports with moved stations
 *
 * Register: \a ANA_L2:COMMON:MOVELOG_STICKY
 */
#define VTSS_ANA_L2_COMMON_MOVELOG_STICKY    VTSS_IOREG(VTSS_TO_ANA_L2,0x444f)


/** 
 * \brief VSTAX configuration
 *
 * \details
 * Configures stacking learning and forwarding operation
 *
 * Register: \a ANA_L2:COMMON:VSTAX_CTRL
 */
#define VTSS_ANA_L2_COMMON_VSTAX_CTRL        VTSS_IOREG(VTSS_TO_ANA_L2,0x4450)

/** 
 * \brief
 * Configures own upsid.
 * 
 * This must be configured consistently across the following registers:
 * ANA_AC::COMMON_VSTAX_CFG.OWN_UPSID
 * ANA_L2::VSTAX_CTRL.OWN_UPSID
 * ASM::SP_UPSID_CFG.OWN_UPSID
 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_VSTAX_CTRL . OWN_UPSID
 */
#define  VTSS_F_ANA_L2_COMMON_VSTAX_CTRL_OWN_UPSID(x)  VTSS_ENCODE_BITFIELD(x,16,5)
#define  VTSS_M_ANA_L2_COMMON_VSTAX_CTRL_OWN_UPSID     VTSS_ENCODE_BITMASK(16,5)
#define  VTSS_X_ANA_L2_COMMON_VSTAX_CTRL_OWN_UPSID(x)  VTSS_EXTRACT_BITFIELD(x,16,5)

/** 
 * \brief
 * Configures CPU queue for VSTAX2 triggered events (Configurable via
 * ANA_L2::VSTAX_CTRL).
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_L2_COMMON_VSTAX_CTRL . CPU_VSTAX_QU
 */
#define  VTSS_F_ANA_L2_COMMON_VSTAX_CTRL_CPU_VSTAX_QU(x)  VTSS_ENCODE_BITFIELD(x,12,3)
#define  VTSS_M_ANA_L2_COMMON_VSTAX_CTRL_CPU_VSTAX_QU     VTSS_ENCODE_BITMASK(12,3)
#define  VTSS_X_ANA_L2_COMMON_VSTAX_CTRL_CPU_VSTAX_QU(x)  VTSS_EXTRACT_BITFIELD(x,12,3)

/** 
 * \brief
 * Enable copy to local CPU for frames received with different destination
 * in VStaX2 header than specified in local MAC table or if DMAC unknown is
 * received for a frame known in MAC table.
 * 
 * Copies are send to the CPU queue specified by
 * ANA_L2::VSTAX_CTRL.CPU_VSTAX_QU.
 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_VSTAX_CTRL . VSTAX2_LCPU_CP_UNEXP_DST_PORT_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_VSTAX_CTRL_VSTAX2_LCPU_CP_UNEXP_DST_PORT_ENA  VTSS_BIT(11)

/** 
 * \brief
 * Enable copy to local CPU for frames received on a front port with SMAC
 * entry nxt_lrn_all set.
 * 
 * Copies are send to the CPU queue specified by
 * ANA_L2::VSTAX_CTRL.CPU_VSTAX_QU.
 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_VSTAX_CTRL . VSTAX2_LCPU_CP_NXT_LRN_ALL_FRONT_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_VSTAX_CTRL_VSTAX2_LCPU_CP_NXT_LRN_ALL_FRONT_ENA  VTSS_BIT(10)

/** 
 * \brief
 * Enable copy to local CPU for frames received on a stack port with SMAC
 * address which causes a refresh of the age bit. 
 * 
 * Copies are send to the CPU queue specified by
 * ANA_L2::VSTAX_CTRL.CPU_VSTAX_QU.
 * 
 * Note: No copy will be generated if learning is disabled due to VLAN
 * learn state etc.

 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_VSTAX_CTRL . VSTAX2_LCPU_CP_REFRESH_STACK_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_VSTAX_CTRL_VSTAX2_LCPU_CP_REFRESH_STACK_ENA  VTSS_BIT(9)

/** 
 * \brief
 * Enable copy to local CPU for frames received on a front port with SMAC
 * address which causes a refresh of the age bit. 
 * 
 * Copies are send to the CPU queue specified by
 * ANA_L2::VSTAX_CTRL.CPU_VSTAX_QU.
 * 
 * Note: No copy will be generated if learning is disabled due to VLAN
 * learn state etc.

 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_VSTAX_CTRL . VSTAX2_LCPU_CP_REFRESH_FRONT_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_VSTAX_CTRL_VSTAX2_LCPU_CP_REFRESH_FRONT_ENA  VTSS_BIT(8)

/** 
 * \brief
 * Enable copy to local CPU for frames received on a stack port with SMAC
 * address previous learned on another port. 
 * 
 * Copies are send to the CPU queue specified by
 * ANA_L2::VSTAX_CTRL.CPU_VSTAX_QU.
 * 
 * Note: No copy will be generated if learning is disabled due to VLAN
 * learn state etc.

 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_VSTAX_CTRL . VSTAX2_LCPU_CP_MOVE_STACK_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_VSTAX_CTRL_VSTAX2_LCPU_CP_MOVE_STACK_ENA  VTSS_BIT(7)

/** 
 * \brief
 * Enable copy to local CPU for frames received on a stack port with new
 * SMAC address. 
 * 
 * Copies are send to the CPU queue specified by
 * ANA_L2::VSTAX_CTRL.CPU_VSTAX_QU.
 * 
 * Note: No copy will be generated if learning is disabled due to VLAN
 * learn state etc.

 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_VSTAX_CTRL . VSTAX2_LCPU_CP_NEW_STACK_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_VSTAX_CTRL_VSTAX2_LCPU_CP_NEW_STACK_ENA  VTSS_BIT(6)

/** 
 * \brief
 * Enable copy to local CPU for frames received on a front port with SMAC
 * address previous learned on another port. 
 * 
 * Copies are send to the CPU queue specified by
 * ANA_L2::VSTAX_CTRL.CPU_VSTAX_QU.
 * 
 * Note: No copy will be generated if learning is disabled due to VLAN
 * learn state etc.

 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_VSTAX_CTRL . VSTAX2_LCPU_CP_MOVE_FRONT_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_VSTAX_CTRL_VSTAX2_LCPU_CP_MOVE_FRONT_ENA  VTSS_BIT(5)

/** 
 * \brief
 * Enable copy to local CPU for frames with new SMAC address received on a
 * front port. 
 * 
 * Copies are send to the CPU queue specified by
 * ANA_L2::VSTAX_CTRL.CPU_VSTAX_QU.
 * 
 * Note: No copy will be generated if learning is disabled due to VLAN
 * learn state etc.

 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_VSTAX_CTRL . VSTAX2_LCPU_CP_NEW_FRONT_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_VSTAX_CTRL_VSTAX2_LCPU_CP_NEW_FRONT_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Enable lrn-all generation triggered by unexpected flooding, where MAC
 * address is known on local port. 
 * 
 * Note: No copy will be generated if learning is disabled due to VLAN
 * learn state etc.

 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_VSTAX_CTRL . VSTAX2_LRN_ALL_UNEXP_FLOOD_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_VSTAX_CTRL_VSTAX2_LRN_ALL_UNEXP_FLOOD_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Enable lrn-all generation triggered by MAC entry refresh for MAC
 * addresses learned on local port. 
 * 
 * Note: No copy will be generated if learning is disabled due to VLAN
 * learn state etc.

 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_VSTAX_CTRL . VSTAX2_LRN_ALL_REFRESH_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_VSTAX_CTRL_VSTAX2_LRN_ALL_REFRESH_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Enable lrn-all generation triggered by MAC address moved to local port. 
 * 
 * Note: No copy will be generated if learning is disabled due to VLAN
 * learn state etc.

 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_VSTAX_CTRL . VSTAX2_LRN_ALL_MOVE_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_VSTAX_CTRL_VSTAX2_LRN_ALL_MOVE_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable lrn-all generation triggered by learn of new MAC address. 
 * 
 * Note: No copy will be generated if learning is disabled due to VLAN
 * learn state etc.

 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_VSTAX_CTRL . VSTAX2_LRN_ALL_NEW_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_VSTAX_CTRL_VSTAX2_LRN_ALL_NEW_ENA  VTSS_BIT(0)


/** 
 * \brief Automatic learn limit configuration
 *
 * \details
 * Per port configuration of autolearn limits
 *
 * Register: \a ANA_L2:COMMON:LRN_CNT_CTRL
 *
 * @param ri Replicator: x_ANA_NUM_PORTS (??), 0-31
 */
#define VTSS_ANA_L2_COMMON_LRN_CNT_CTRL(ri)  VTSS_IOREG(VTSS_TO_ANA_L2,0x4451 + (ri))

/** 
 * \brief
 * Set if specified lrn rate is exceeded and LEARN max cnt was enabled.
 * Write '1' to clear this field.
 *
 * \details 
 * 0: No cnt exceeded.
 * 1: An autonomous learning operation has failed due to cnt exceeded.
 * Write '1' to clear this field.
 *
 * Field: ::VTSS_ANA_L2_COMMON_LRN_CNT_CTRL . LRN_CNT_MAX_STICKY
 */
#define  VTSS_F_ANA_L2_COMMON_LRN_CNT_CTRL_LRN_CNT_MAX_STICKY  VTSS_BIT(31)

/** 
 * \brief
 * Configures the number of MAC table entries that can be used for a given
 * logical port (via Automatic learning and CPU based learning with LOCK
 * bit cleared).
 *
 * \details 
 * 0: Learn limit disabled for port
 * 1: Learn of one MAC address allowed for port
 * 2: Learn of two MAC address allowed for port
 * 3: Learn of four MAC address allowed for port
 * ...
 * n: Learn of n MAC address allowed for port
 *
 * Field: ::VTSS_ANA_L2_COMMON_LRN_CNT_CTRL . AUTO_LRN_CNT_MAX
 */
#define  VTSS_F_ANA_L2_COMMON_LRN_CNT_CTRL_AUTO_LRN_CNT_MAX(x)  VTSS_ENCODE_BITFIELD(x,16,15)
#define  VTSS_M_ANA_L2_COMMON_LRN_CNT_CTRL_AUTO_LRN_CNT_MAX     VTSS_ENCODE_BITMASK(16,15)
#define  VTSS_X_ANA_L2_COMMON_LRN_CNT_CTRL_AUTO_LRN_CNT_MAX(x)  VTSS_EXTRACT_BITFIELD(x,16,15)

/** 
 * \brief
 * Configures the number of MAC table entries current learned for a given
 * logical port.
 *
 * \details 
 * 0 : no entries
 *
 * Field: ::VTSS_ANA_L2_COMMON_LRN_CNT_CTRL . AUTO_LRN_CNT
 */
#define  VTSS_F_ANA_L2_COMMON_LRN_CNT_CTRL_AUTO_LRN_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_ANA_L2_COMMON_LRN_CNT_CTRL_AUTO_LRN_CNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_ANA_L2_COMMON_LRN_CNT_CTRL_AUTO_LRN_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief ANA_L2 Interrupt events
 *
 * \details
 * ANA_L2 Interrupt events if interrupt enable is set in ANA_L2::INTR_ENA
 * and the corresponding event is triggered in ANA_L2::INTR_IDENT
 *
 * Register: \a ANA_L2:COMMON:INTR
 */
#define VTSS_ANA_L2_COMMON_INTR              VTSS_IOREG(VTSS_TO_ANA_L2,0x4471)

/** 
 * \brief
 * Set if LRN CPU access triggered interrupt.
 * 
 * To enable interrupt of this events set the corresponding *_INTR_ENA
 *
 * \details 
 * 0: No event has occured
 * 1: LRN_ACCESS_COMPLETE triggered inq occured
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR . LRN_ACCESS_COMPLETE_INTR
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_LRN_ACCESS_COMPLETE_INTR  VTSS_BIT(25)

/** 
 * \brief
 * Set if VCAP_S2 triggered interrupt.
 * 
 * To enable interrupt of this events set the corresponding *_INTR_ENA
 *
 * \details 
 * 0: No event has occured
 * 1: VCAP_S2 triggered inq occured
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR . VCAP_S2_INTR
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_VCAP_S2_INTR  VTSS_BIT(24)

/** 
 * \brief
 * Set if CCM isdx fault management down has cleared due to received CCM
 * messages.
 * 
 * To enable interrupt of this events set the corresponding *_INTR_ENA
 *
 * \details 
 * 0: No event has occured
 * 1: ISDX CCM fault management up event occured
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR . CCM_ISDX_FAULT_MNGM_UP_INTR
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_CCM_ISDX_FAULT_MNGM_UP_INTR  VTSS_BIT(22)

/** 
 * \brief
 * Set if CCM isdx performance management down has cleared due to received
 * CCM messages.
 * 
 * To enable interrupt of this events set the corresponding *_INTR_ENA
 *
 * \details 
 * 0: No event has occured
 * 1: ISDX CCM performance management up event occured
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR . CCM_ISDX_PERF_MNGM_UP_INTR
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_CCM_ISDX_PERF_MNGM_UP_INTR  VTSS_BIT(21)

/** 
 * \brief
 * Set if CCM isdx protection switching down has cleared due to received
 * CCM messages.
 * 
 * To enable interrupt of this events set the corresponding *_INTR_ENA
 *
 * \details 
 * 0: No event has occured
 * 1: ISDX CCM protection switching up event occured
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR . CCM_ISDX_PROT_SWITCH_UP_INTR
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_CCM_ISDX_PROT_SWITCH_UP_INTR  VTSS_BIT(20)

/** 
 * \brief
 * Set if CCM path fault management down has cleared due to received CCM
 * messages.
 * 
 * To enable interrupt of this events set the corresponding *_INTR_ENA
 *
 * \details 
 * 0: No event has occured
 * 1: PATH CCM fault management up event occured
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR . CCM_PATH_FAULT_MNGM_UP_INTR
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_CCM_PATH_FAULT_MNGM_UP_INTR  VTSS_BIT(18)

/** 
 * \brief
 * Set if CCM path performance management down has cleared due to received
 * CCM messages.
 * 
 * To enable interrupt of this events set the corresponding *_INTR_ENA
 *
 * \details 
 * 0: No event has occured
 * 1: PATH CCM performance management up event occured
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR . CCM_PATH_PERF_MNGM_UP_INTR
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_CCM_PATH_PERF_MNGM_UP_INTR  VTSS_BIT(17)

/** 
 * \brief
 * Set if CCM path protection switching down has cleared due to received
 * CCM messages.
 * 
 * To enable interrupt of this events set the corresponding *_INTR_ENA
 *
 * \details 
 * 0: No event has occured
 * 1: PATH CCM protection switching up event occured
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR . CCM_PATH_PROT_SWITCH_UP_INTR
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_CCM_PATH_PROT_SWITCH_UP_INTR  VTSS_BIT(16)

/** 
 * \brief
 * Set if CCM port fault management down has cleared due to received CCM
 * messages.
 * 
 * To enable interrupt of this events set the corresponding *_INTR_ENA
 *
 * \details 
 * 0: No event has occured
 * 1: PORT CCM fault management up event occured
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR . CCM_PORT_FAULT_MNGM_UP_INTR
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_CCM_PORT_FAULT_MNGM_UP_INTR  VTSS_BIT(14)

/** 
 * \brief
 * Set if CCM port performance management down has cleared due to received
 * CCM messages.
 * 
 * To enable interrupt of this events set the corresponding *_INTR_ENA
 *
 * \details 
 * 0: No event has occured
 * 1: PORT CCM performance management up event occured
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR . CCM_PORT_PERF_MNGM_UP_INTR
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_CCM_PORT_PERF_MNGM_UP_INTR  VTSS_BIT(13)

/** 
 * \brief
 * Set if CCM port protection switching down has cleared due to received
 * CCM messages.
 * 
 * To enable interrupt of this events set the corresponding *_INTR_ENA
 *
 * \details 
 * 0: No event has occured
 * 1: PORT CCM protection switching up event occured
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR . CCM_PORT_PROT_SWITCH_UP_INTR
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_CCM_PORT_PROT_SWITCH_UP_INTR  VTSS_BIT(12)

/** 
 * \brief
 * Set if three CCM isdx fault management intervals has passed without
 * received CCM messages.
 * 
 * To enable interrupt of this events set the corresponding *_INTR_ENA
 *
 * \details 
 * 0: No event has occured
 * 1: ISDX CCM fault management down event occured
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR . CCM_PORT_FAULT_MNGM_DOWN_INTR
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_CCM_PORT_FAULT_MNGM_DOWN_INTR  VTSS_BIT(10)

/** 
 * \brief
 * Set if three CCM isdx performance management intervals has passed
 * without received CCM messages.
 * 
 * To enable interrupt of this events set the corresponding *_INTR_ENA
 *
 * \details 
 * 0: No event has occured
 * 1: ISDX CCM performance management down event occured
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR . CCM_PORT_PERF_MNGM_DOWN_INTR
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_CCM_PORT_PERF_MNGM_DOWN_INTR  VTSS_BIT(9)

/** 
 * \brief
 * Set if three CCM isdx protection switching intervals has passed without
 * received CCM messages.
 * 
 * To enable interrupt of this events set the corresponding *_INTR_ENA
 *
 * \details 
 * 0: No event has occured
 * 1: ISDX CCM protection switching down event occured
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR . CCM_PORT_PROT_SWITCH_DOWN_INTR
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_CCM_PORT_PROT_SWITCH_DOWN_INTR  VTSS_BIT(8)

/** 
 * \brief
 * Set if three CCM path fault management intervals has passed without
 * received CCM messages.
 * 
 * To enable interrupt of this events set the corresponding *_INTR_ENA
 *
 * \details 
 * 0: No event has occured
 * 1: PATH CCM fault management down event occured
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR . CCM_PATH_FAULT_MNGM_DOWN_INTR
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_CCM_PATH_FAULT_MNGM_DOWN_INTR  VTSS_BIT(6)

/** 
 * \brief
 * Set if three CCM path performance management intervals has passed
 * without received CCM messages.
 * 
 * To enable interrupt of this events set the corresponding *_INTR_ENA
 *
 * \details 
 * 0: No event has occured
 * 1: PATH CCM performance management down event occured
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR . CCM_PATH_PERF_MNGM_DOWN_INTR
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_CCM_PATH_PERF_MNGM_DOWN_INTR  VTSS_BIT(5)

/** 
 * \brief
 * Set if three CCM path protection switching intervals has passed without
 * received CCM messages.
 * 
 * To enable interrupt of this events set the corresponding *_INTR_ENA
 *
 * \details 
 * 0: No event has occured
 * 1: PATH CCM protection switching down event occured
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR . CCM_PATH_PROT_SWITCH_DOWN_INTR
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_CCM_PATH_PROT_SWITCH_DOWN_INTR  VTSS_BIT(4)

/** 
 * \brief
 * Set if three CCM port fault management intervals has passed without
 * received CCM messages.
 * 
 * To enable interrupt of this events set the corresponding *_INTR_ENA
 *
 * \details 
 * 0: No event has occured
 * 1: Port CCM fault management down event occured
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR . CCM_ISDX_FAULT_MNGM_DOWN_INTR
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_CCM_ISDX_FAULT_MNGM_DOWN_INTR  VTSS_BIT(2)

/** 
 * \brief
 * Set if three CCM port performance management intervals has passed
 * without received CCM messages.
 * 
 * To enable interrupt of this events set the corresponding *_INTR_ENA
 *
 * \details 
 * 0: No event has occured
 * 1: PORT CCM performance management down event occured
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR . CCM_ISDX_PERF_MNGM_DOWN_INTR
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_CCM_ISDX_PERF_MNGM_DOWN_INTR  VTSS_BIT(1)

/** 
 * \brief
 * Set if three CCM protection switching intervals has passed without
 * received CCM messages.
 * 
 * To enable interrupt of this events set the corresponding *_INTR_ENA
 *
 * \details 
 * 0: No event has occured
 * 1: PORT CCM protection switching down event occured
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR . CCM_ISDX_PROT_SWITCH_DOWN_INTR
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_CCM_ISDX_PROT_SWITCH_DOWN_INTR  VTSS_BIT(0)


/** 
 * \brief ANA_L2 interrupt enable mask
 *
 * \details
 * Register: \a ANA_L2:COMMON:INTR_ENA
 */
#define VTSS_ANA_L2_COMMON_INTR_ENA          VTSS_IOREG(VTSS_TO_ANA_L2,0x4472)

/** 
 * \brief
 * Configures LRN_ACCESS_COMPLETE_INTR interrupts
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_ENA . LRN_ACCESS_COMPLETE_INTR_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_ENA_LRN_ACCESS_COMPLETE_INTR_ENA  VTSS_BIT(25)

/** 
 * \brief
 * Configures VCAP_S2_INTR interrupts
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_ENA . VCAP_S2_INTR_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_ENA_VCAP_S2_INTR_ENA  VTSS_BIT(24)

/** 
 * \brief
 * Configures CCM_ISDX_FAULT_MNGM_UP_INTR interrupts
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_ENA . CCM_ISDX_FAULT_MNGM_UP_INTR_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_ENA_CCM_ISDX_FAULT_MNGM_UP_INTR_ENA  VTSS_BIT(22)

/** 
 * \brief
 * Configures CCM_ISDX_PERF_MNGM_UP_INTR interrupts
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_ENA . CCM_ISDX_PERF_MNGM_UP_INTR_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_ENA_CCM_ISDX_PERF_MNGM_UP_INTR_ENA  VTSS_BIT(21)

/** 
 * \brief
 * Configures CCM_ISDX_PROT_SWITCH_UP_INTR interrupts
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_ENA . CCM_ISDX_PROT_SWITCH_UP_INTR_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_ENA_CCM_ISDX_PROT_SWITCH_UP_INTR_ENA  VTSS_BIT(20)

/** 
 * \brief
 * Configures CCM_PATH_FAULT_MNGM_UP_INTR interrupts
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_ENA . CCM_PATH_FAULT_MNGM_UP_INTR_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_ENA_CCM_PATH_FAULT_MNGM_UP_INTR_ENA  VTSS_BIT(18)

/** 
 * \brief
 * Configures CCM_PATH_PERF_MNGM_UP_INTR interrupts
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_ENA . CCM_PATH_PERF_MNGM_UP_INTR_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_ENA_CCM_PATH_PERF_MNGM_UP_INTR_ENA  VTSS_BIT(17)

/** 
 * \brief
 * Configures CCM_PATH_PROT_SWITCH_UP_INTR interrupts
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_ENA . CCM_PATH_PROT_SWITCH_UP_INTR_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_ENA_CCM_PATH_PROT_SWITCH_UP_INTR_ENA  VTSS_BIT(16)

/** 
 * \brief
 * Configures CCM_PORT_FAULT_MNGM_UP_INTR interrupts
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_ENA . CCM_PORT_FAULT_MNGM_UP_INTR_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_ENA_CCM_PORT_FAULT_MNGM_UP_INTR_ENA  VTSS_BIT(14)

/** 
 * \brief
 * Configures CCM_PORT_PERF_MNGM_UP_INTR interrupts
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_ENA . CCM_PORT_PERF_MNGM_UP_INTR_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_ENA_CCM_PORT_PERF_MNGM_UP_INTR_ENA  VTSS_BIT(13)

/** 
 * \brief
 * Configures CCM_PORT_PROT_SWITCH_UP_INTR interrupts
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_ENA . CCM_PORT_PROT_SWITCH_UP_INTR_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_ENA_CCM_PORT_PROT_SWITCH_UP_INTR_ENA  VTSS_BIT(12)

/** 
 * \brief
 * Configures CCM_ISDX_FAULT_MNGM_DOWN_INTR interrupts
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_ENA . CCM_PORT_FAULT_MNGM_DOWN_INTR_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_ENA_CCM_PORT_FAULT_MNGM_DOWN_INTR_ENA  VTSS_BIT(10)

/** 
 * \brief
 * Configures CCM_ISDX_PERF_MNGM_DOWN_INTR interrupts
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_ENA . CCM_PORT_PERF_MNGM_DOWN_INTR_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_ENA_CCM_PORT_PERF_MNGM_DOWN_INTR_ENA  VTSS_BIT(9)

/** 
 * \brief
 * Configures CCM_ISDX_PROT_SWITCH_DOWN_INTR interrupts
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_ENA . CCM_PORT_PROT_SWITCH_DOWN_INTR_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_ENA_CCM_PORT_PROT_SWITCH_DOWN_INTR_ENA  VTSS_BIT(8)

/** 
 * \brief
 * Configures CCM_PATH_FAULT_MNGM_DOWN_INTR interrupts
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_ENA . CCM_PATH_FAULT_MNGM_DOWN_INTR_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_ENA_CCM_PATH_FAULT_MNGM_DOWN_INTR_ENA  VTSS_BIT(6)

/** 
 * \brief
 * Configures CCM_PATH_PERF_MNGM_DOWN_INTR interrupts
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_ENA . CCM_PATH_PERF_MNGM_DOWN_INTR_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_ENA_CCM_PATH_PERF_MNGM_DOWN_INTR_ENA  VTSS_BIT(5)

/** 
 * \brief
 * Configures CCM_PATH_PROT_SWITCH_DOWN_INTR interrupts
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_ENA . CCM_PATH_PROT_SWITCH_DOWN_INTR_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_ENA_CCM_PATH_PROT_SWITCH_DOWN_INTR_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Configures CCM_PORT_FAULT_MNGM_DOWN_INTR interrupts
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_ENA . CCM_ISDX_FAULT_MNGM_DOWN_INTR_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_ENA_CCM_ISDX_FAULT_MNGM_DOWN_INTR_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Configures CCM_PORT_PERF_MNGM_DOWN_INTR interrupts
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_ENA . CCM_ISDX_PERF_MNGM_DOWN_INTR_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_ENA_CCM_ISDX_PERF_MNGM_DOWN_INTR_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Configures CCM_PORT_PROT_SWITCH_DOWN_INTR interrupts
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_ENA . CCM_ISDX_PROT_SWITCH_DOWN_INTR_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_ENA_CCM_ISDX_PROT_SWITCH_DOWN_INTR_ENA  VTSS_BIT(0)


/** 
 * \brief ANA_L2 interrupt status
 *
 * \details
 * Register: \a ANA_L2:COMMON:INTR_IDENT
 */
#define VTSS_ANA_L2_COMMON_INTR_IDENT        VTSS_IOREG(VTSS_TO_ANA_L2,0x4473)

/** 
 * \brief
 * Status of LRN_ACCESS_COMPLETE_INTR interrupts
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt to CPU
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_IDENT . LRN_ACCESS_COMPLETE_INTR_IDENT
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_IDENT_LRN_ACCESS_COMPLETE_INTR_IDENT  VTSS_BIT(25)

/** 
 * \brief
 * Status of VCAP_S2_INTR interrupts
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt to CPU
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_IDENT . VCAP_S2_INTR_IDENT
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_IDENT_VCAP_S2_INTR_IDENT  VTSS_BIT(24)

/** 
 * \brief
 * Status of CCM_ISDX_FAULT_MNGM_UP_INTR interrupts
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt to CPU
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_IDENT . CCM_ISDX_FAULT_MNGM_UP_INTR_IDENT
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_IDENT_CCM_ISDX_FAULT_MNGM_UP_INTR_IDENT  VTSS_BIT(22)

/** 
 * \brief
 * Status of CCM_ISDX_PERF_MNGM_UP_INTR interrupts
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt to CPU
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_IDENT . CCM_ISDX_PERF_MNGM_UP_INTR_IDENT
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_IDENT_CCM_ISDX_PERF_MNGM_UP_INTR_IDENT  VTSS_BIT(21)

/** 
 * \brief
 * Status of CCM_ISDX_PROT_SWITCH_UP_INTR interrupts
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt to CPU
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_IDENT . CCM_ISDX_PROT_SWITCH_UP_INTR_IDENT
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_IDENT_CCM_ISDX_PROT_SWITCH_UP_INTR_IDENT  VTSS_BIT(20)

/** 
 * \brief
 * Status of CCM_PATH_FAULT_MNGM_UP_INTR interrupts
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt to CPU
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_IDENT . CCM_PATH_FAULT_MNGM_UP_INTR_IDENT
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_IDENT_CCM_PATH_FAULT_MNGM_UP_INTR_IDENT  VTSS_BIT(18)

/** 
 * \brief
 * Status of CCM_PATH_PERF_MNGM_UP_INTR interrupts
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt to CPU
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_IDENT . CCM_PATH_PERF_MNGM_UP_INTR_IDENT
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_IDENT_CCM_PATH_PERF_MNGM_UP_INTR_IDENT  VTSS_BIT(17)

/** 
 * \brief
 * Status of CCM_PATH_PROT_SWITCH_UP_INTR interrupts
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt to CPU
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_IDENT . CCM_PATH_PROT_SWITCH_UP_INTR_IDENT
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_IDENT_CCM_PATH_PROT_SWITCH_UP_INTR_IDENT  VTSS_BIT(16)

/** 
 * \brief
 * Status of CCM_PORT_FAULT_MNGM_UP_INTR interrupts
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt to CPU
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_IDENT . CCM_PORT_FAULT_MNGM_UP_INTR_IDENT
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_IDENT_CCM_PORT_FAULT_MNGM_UP_INTR_IDENT  VTSS_BIT(14)

/** 
 * \brief
 * Status of CCM_PORT_PERF_MNGM_UP_INTR interrupts
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt to CPU
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_IDENT . CCM_PORT_PERF_MNGM_UP_INTR_IDENT
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_IDENT_CCM_PORT_PERF_MNGM_UP_INTR_IDENT  VTSS_BIT(13)

/** 
 * \brief
 * Status of CCM_PORT_PROT_SWITCH_UP_INTR interrupts
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt to CPU
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_IDENT . CCM_PORT_PROT_SWITCH_UP_INTR_IDENT
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_IDENT_CCM_PORT_PROT_SWITCH_UP_INTR_IDENT  VTSS_BIT(12)

/** 
 * \brief
 * Status of CCM_ISDX_FAULT_MNGM_DOWN_INTR interrupts
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt to CPU
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_IDENT . CCM_PORT_FAULT_MNGM_DOWN_INTR_IDENT
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_IDENT_CCM_PORT_FAULT_MNGM_DOWN_INTR_IDENT  VTSS_BIT(10)

/** 
 * \brief
 * Status of CCM_ISDX_PERF_MNGM_DOWN_INTR interrupts
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt to CPU
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_IDENT . CCM_PORT_PERF_MNGM_DOWN_INTR_IDENT
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_IDENT_CCM_PORT_PERF_MNGM_DOWN_INTR_IDENT  VTSS_BIT(9)

/** 
 * \brief
 * Status of CCM_ISDX_PROT_SWITCH_DOWN_INTR interrupts
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt to CPU
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_IDENT . CCM_PORT_PROT_SWITCH_DOWN_INTR_IDENT
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_IDENT_CCM_PORT_PROT_SWITCH_DOWN_INTR_IDENT  VTSS_BIT(8)

/** 
 * \brief
 * Status of CCM_PATH_FAULT_MNGM_DOWN_INTR interrupts
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt to CPU
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_IDENT . CCM_PATH_FAULT_MNGM_DOWN_INTR_IDENT
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_IDENT_CCM_PATH_FAULT_MNGM_DOWN_INTR_IDENT  VTSS_BIT(6)

/** 
 * \brief
 * Status of CCM_PATH_PERF_MNGM_DOWN_INTR interrupts
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt to CPU
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_IDENT . CCM_PATH_PERF_MNGM_DOWN_INTR_IDENT
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_IDENT_CCM_PATH_PERF_MNGM_DOWN_INTR_IDENT  VTSS_BIT(5)

/** 
 * \brief
 * Status of CCM_PATH_PROT_SWITCH_DOWN_INTR interrupts
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt to CPU
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_IDENT . CCM_PATH_PROT_SWITCH_DOWN_INTR_IDENT
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_IDENT_CCM_PATH_PROT_SWITCH_DOWN_INTR_IDENT  VTSS_BIT(4)

/** 
 * \brief
 * Status of CCM_PORT_FAULT_MNGM_DOWN_INTR interrupts
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt to CPU
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_IDENT . CCM_ISDX_FAULT_MNGM_DOWN_INTR_IDENT
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_IDENT_CCM_ISDX_FAULT_MNGM_DOWN_INTR_IDENT  VTSS_BIT(2)

/** 
 * \brief
 * Status of CCM_PORT_PERF_MNGM_DOWN_INTR interrupts
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt to CPU
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_IDENT . CCM_ISDX_PERF_MNGM_DOWN_INTR_IDENT
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_IDENT_CCM_ISDX_PERF_MNGM_DOWN_INTR_IDENT  VTSS_BIT(1)

/** 
 * \brief
 * Status of CCM_PORT_PROT_SWITCH_DOWN_INTR interrupts
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt to CPU
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_IDENT . CCM_ISDX_PROT_SWITCH_DOWN_INTR_IDENT
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_IDENT_CCM_ISDX_PROT_SWITCH_DOWN_INTR_IDENT  VTSS_BIT(0)


/** 
 * \brief Memory integrity status registers
 *
 * \details
 * Register: \a ANA_L2:COMMON:RAM_INTGR_ERR_IDENT
 */
#define VTSS_ANA_L2_COMMON_RAM_INTGR_ERR_IDENT  VTSS_IOREG(VTSS_TO_ANA_L2,0x4474)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_L2_COMMON_RAM_INTGR_ERR_IDENT . PATHGRP_RAM_INTGR_ERR
 */
#define  VTSS_F_ANA_L2_COMMON_RAM_INTGR_ERR_IDENT_PATHGRP_RAM_INTGR_ERR  VTSS_BIT(3)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_L2_COMMON_RAM_INTGR_ERR_IDENT . ISDX_RAM_INTGR_ERR
 */
#define  VTSS_F_ANA_L2_COMMON_RAM_INTGR_ERR_IDENT_ISDX_RAM_INTGR_ERR  VTSS_BIT(2)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_L2_COMMON_RAM_INTGR_ERR_IDENT . MAC_RAM_INTGR_ERR
 */
#define  VTSS_F_ANA_L2_COMMON_RAM_INTGR_ERR_IDENT_MAC_RAM_INTGR_ERR  VTSS_BIT(1)

/**
 * Register Group: \a ANA_L2:PATHGRP
 *
 * Pathgroup table configuration
 */


/** 
 * \brief Path group connections. index 0 is primary
 *
 * \details
 * Configures protected path groups with two conid's
 *
 * Register: \a ANA_L2:PATHGRP:CONID_CFG
 *
 * @param gi Replicator: x_ANA_NUM_PATHGRP (??), 0-511
 * @param ri Register: CONID_CFG (??), 0-1
 */
#define VTSS_ANA_L2_PATHGRP_CONID_CFG(gi,ri)  VTSS_IOREG_IX(VTSS_TO_ANA_L2,0x4000,gi,2,ri,0)

/** 
 * \brief
 * Configures connection ID for the pathgroup.
 * 
 * Specifies the index into BMAC table for (BDA,BVID) for PBB-TE to be used
 * if no CDA lookup is specified (CDA_FWD_ENA==0)
 *
 * \details 
 * Field: ::VTSS_ANA_L2_PATHGRP_CONID_CFG . CONID
 */
#define  VTSS_F_ANA_L2_PATHGRP_CONID_CFG_CONID(x)  VTSS_ENCODE_BITFIELD(x,8,16)
#define  VTSS_M_ANA_L2_PATHGRP_CONID_CFG_CONID     VTSS_ENCODE_BITMASK(8,16)
#define  VTSS_X_ANA_L2_PATHGRP_CONID_CFG_CONID(x)  VTSS_EXTRACT_BITFIELD(x,8,16)

/** 
 * \brief
 * Specifies whether the DLB index is incremented when this CONID is used.
 *
 * \details 
 * Field: ::VTSS_ANA_L2_PATHGRP_CONID_CFG . DLB_INC_ENA
 */
#define  VTSS_F_ANA_L2_PATHGRP_CONID_CFG_DLB_INC_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Specifies whether the ISDX index is incremented when this CONID is used.

 *
 * \details 
 * Field: ::VTSS_ANA_L2_PATHGRP_CONID_CFG . ISDX_INC_ENA
 */
#define  VTSS_F_ANA_L2_PATHGRP_CONID_CFG_ISDX_INC_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Specifies whether the connection can to be used for reception
 *
 * \details 
 * 0: Disable CONID (traffic from this CONID will be discarded).
 * 1: Enable CONID
 *
 * Field: ::VTSS_ANA_L2_PATHGRP_CONID_CFG . CONID_RX_ENA
 */
#define  VTSS_F_ANA_L2_PATHGRP_CONID_CFG_CONID_RX_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Specifies whether the connection can to be used for transmission
 * 
 * Traffic to be forwarded to Pathgroup with both CONID is disabled for
 * transmission will be discarded.
 *
 * \details 
 * 0: Disable CONID
 * 1: Enable CONID
 *
 * Field: ::VTSS_ANA_L2_PATHGRP_CONID_CFG . CONID_TX_ENA
 */
#define  VTSS_F_ANA_L2_PATHGRP_CONID_CFG_CONID_TX_ENA  VTSS_BIT(0)

/**
 * Register Group: \a ANA_L2:ISDX
 *
 * Ingress service table configuration
 */


/** 
 * \brief Ingress service port mask configuration
 *
 * \details
 * Configures port mask to be used in relation to service
 *
 * Register: \a ANA_L2:ISDX:PORT_MASK_CFG
 *
 * @param gi Replicator: x_ANA_NUM_ISDX (??), 0-4095
 */
#define VTSS_ANA_L2_ISDX_PORT_MASK_CFG(gi)   VTSS_IOREG_IX(VTSS_TO_ANA_L2,0x0,gi,4,0,0)


/** 
 * \brief Ingress service forwarding configuration
 *
 * \details
 * Configures service based forwarding.
 *
 * Register: \a ANA_L2:ISDX:SERVICE_CTRL
 *
 * @param gi Replicator: x_ANA_NUM_ISDX (??), 0-4095
 */
#define VTSS_ANA_L2_ISDX_SERVICE_CTRL(gi)    VTSS_IOREG_IX(VTSS_TO_ANA_L2,0x0,gi,4,0,1)

/** 
 * \brief
 * Configures how to forward the service. The encoding of this field is
 * specified by FWD_TYPE
 *
 * \details 
 * FWD_TYPE= UPSID_PN:
 *   FWD_ADDR(9:5) = UPSID
 *   FWD_ADDR(4:0) = UPSPN
 * Specifies static unicast forwarding to lport
 * 
 * FWD_TYPE = MC_IDX:
 * Specifies static multicast forwarding to the ports indexed by MC_IDX
 * into ANA_AC::PGID
 * 
 * FWD_TYPE = CONID_FWD:
 *   FWD_ADDR specifies a connection id (CONID) if ADDR less than
 * FFL_ANA_NUM_MAC_ADDR
 * Specifies the index into BMAC table for the Default (BDA,BVID) for PBB
 * to be used if CDA is unknown
 * Specifies the index into BMAC table for (BDA,BVID) for PBB-TE to be used
 * if no CDA lookup is specified (CDA_FWD_ENA==0)
 *   ADDR specifies a BVID if ADDR greater than or equal to
 * FFL_ANA_NUM_MAC_ADDR
 * Specifies the BVID = ADDR(11 downto 0) to be used for an ITAGGED
 * interface (only valid if ISDX_BASED_FWD=0 and FWD_TYPE = CONID_FWD and
 * FWD_ADDR >= FFL_ANA_NUM_MAC_ADDR)
 * 
 * FWD_TYPE = PATHGRP_FWD:
 *   FWD_ADDR specifies a path group index(PATHGRP)
 * Specifies the index into BMAC table for (BDA,BVID) for PBB-TE to be used
 * if CDA lookup returns the flag Pathgrp_fwd
 *
 * Field: ::VTSS_ANA_L2_ISDX_SERVICE_CTRL . FWD_ADDR
 */
#define  VTSS_F_ANA_L2_ISDX_SERVICE_CTRL_FWD_ADDR(x)  VTSS_ENCODE_BITFIELD(x,8,16)
#define  VTSS_M_ANA_L2_ISDX_SERVICE_CTRL_FWD_ADDR     VTSS_ENCODE_BITMASK(8,16)
#define  VTSS_X_ANA_L2_ISDX_SERVICE_CTRL_FWD_ADDR(x)  VTSS_EXTRACT_BITFIELD(x,8,16)

/** 
 * \brief
 * Selects between using ISDX and classified VID in REW ES0 key
 *
 * \details 
 * '0': disable (use VID key in ES0)
 * '1': enable (use ISDX key in ES0)
 *
 * Field: ::VTSS_ANA_L2_ISDX_SERVICE_CTRL . ES0_ISDX_KEY_ENA
 */
#define  VTSS_F_ANA_L2_ISDX_SERVICE_CTRL_ES0_ISDX_KEY_ENA  VTSS_BIT(7)

/** 
 * \brief
 * Selects between the two paths when FWD_TYPE is PATHGRP_FWD and both
 * paths are active.
 * 
 * For transmission:
 * If frames are to be transmitted via a pathgrp then if both paths are
 * active (CONID_CFG(0).CONID_TX_ENA and CONID_CFG(1).CONID_TX_ENA set),
 * then the selection will be done based on this configuration. 
 * If either of the two paths are inactive (CONID_CFG(0).CONID_TX_ENA or
 * CONID_CFG(1).CONID_TX_ENA cleared) then the oposite will be used.
 * 
 *	   
 * For reception:
 * If either of the two paths are inactive (CONID_CFG(0).CONID_RX_ENA or
 * CONID_CFG(1).CONID_RX_ENA cleared) frames from this path will be
 * discarded.
 * If frames are to be received via a pathgrp where both paths are active
 * (CONID_CFG(0).CONID_RX_ENA and CONID_CFG(1).CONID_RX_ENA set),
 * then frames received on the non prefered path will be discarded based on
 * this configuration unless the prefered path is set to either 
 * in which case frames will not be discarded no matter of which path it
 * was received on.
 *
 * \details 
 * "00": conid 0
 * "01": conid 1
 * "10": either (transmission based on aggregation code)
 *
 * Field: ::VTSS_ANA_L2_ISDX_SERVICE_CTRL . PREF_PATH_SEL
 */
#define  VTSS_F_ANA_L2_ISDX_SERVICE_CTRL_PREF_PATH_SEL(x)  VTSS_ENCODE_BITFIELD(x,5,2)
#define  VTSS_M_ANA_L2_ISDX_SERVICE_CTRL_PREF_PATH_SEL     VTSS_ENCODE_BITMASK(5,2)
#define  VTSS_X_ANA_L2_ISDX_SERVICE_CTRL_PREF_PATH_SEL(x)  VTSS_EXTRACT_BITFIELD(x,5,2)

/** 
 * \brief
 * Enable forwarding based on CDA
 *
 * \details 
 * Field: ::VTSS_ANA_L2_ISDX_SERVICE_CTRL . CDA_FWD_ENA
 */
#define  VTSS_F_ANA_L2_ISDX_SERVICE_CTRL_CDA_FWD_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Enable service based forwarding
 *
 * \details 
 * Field: ::VTSS_ANA_L2_ISDX_SERVICE_CTRL . ISDX_BASED_FWD_ENA
 */
#define  VTSS_F_ANA_L2_ISDX_SERVICE_CTRL_ISDX_BASED_FWD_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Forwarding address type if enabled via ISDX_BASED_FWD_ENA
 *
 * \details 
 * 0: UPSID_PN 
 * 3: MC_IDX
 * 4: CONID_FWD
 * 5: PATHGRP_FWD
 * 7: NO_ADDR
 * other: Reserved
 *
 * Field: ::VTSS_ANA_L2_ISDX_SERVICE_CTRL . FWD_TYPE
 */
#define  VTSS_F_ANA_L2_ISDX_SERVICE_CTRL_FWD_TYPE(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_ANA_L2_ISDX_SERVICE_CTRL_FWD_TYPE     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_ANA_L2_ISDX_SERVICE_CTRL_FWD_TYPE(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Ingress service CCM control
 *
 * \details
 * Configures ISDX CCM monitors
 *
 * Register: \a ANA_L2:ISDX:CCM_CTRL
 *
 * @param gi Replicator: x_ANA_NUM_ISDX (??), 0-4095
 */
#define VTSS_ANA_L2_ISDX_CCM_CTRL(gi)        VTSS_IOREG_IX(VTSS_TO_ANA_L2,0x0,gi,4,0,2)

/** 
 * \brief
 * Indicate expected peer MEPID for this service CCM MEP monitor. 
 *
 * \details 
 * 0: No MEPID check
 * other: Expected MEPID
 *
 * Field: ::VTSS_ANA_L2_ISDX_CCM_CTRL . CCM_MEPID
 */
#define  VTSS_F_ANA_L2_ISDX_CCM_CTRL_CCM_MEPID(x)  VTSS_ENCODE_BITFIELD(x,5,16)
#define  VTSS_M_ANA_L2_ISDX_CCM_CTRL_CCM_MEPID     VTSS_ENCODE_BITMASK(5,16)
#define  VTSS_X_ANA_L2_ISDX_CCM_CTRL_CCM_MEPID(x)  VTSS_EXTRACT_BITFIELD(x,5,16)

/** 
 * \brief
 * Indicate whether Loss of Continuity (LOC) has been declared for this
 * service CCM MEP monitor. 
 * 
 * This flag can also be used for "CPU hit me once operation" where the
 * next CCM message is send to CPU. See ANA_L2::CCM_CFG.CCM_CHG_COPY_ENA.
 * 
 * Clear this bit to receive new down interrupts for this MEP monitor
 *
 * \details 
 * 0: No LOC has been decleared.
 * 1: LOC has been decleared (but might have been cleared again if
 * CCM_MISS_CNT is less than three)
 *
 * Field: ::VTSS_ANA_L2_ISDX_CCM_CTRL . CCM_CHG
 */
#define  VTSS_F_ANA_L2_ISDX_CCM_CTRL_CCM_CHG  VTSS_BIT(4)

/** 
 * \brief
 * Specifies the number of CCM periods has passed without CCM mesages
 * received from the peer MEP.
 * 
 * Cleared by HW when receiving a CCM message from the peer associated with
 * this service MEP monitor. A CCM message is identified via VCAP_S2 with
 * an associated action: CCM_TYPE set to SERVICE. 
 * 
 * Can be configured to periodically increment CCM miss count via HW
 * configured by LRN::CCM_CTRL.
 *
 * \details 
 * Field: ::VTSS_ANA_L2_ISDX_CCM_CTRL . CCM_MISS_CNT
 */
#define  VTSS_F_ANA_L2_ISDX_CCM_CTRL_CCM_MISS_CNT(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_ANA_L2_ISDX_CCM_CTRL_CCM_MISS_CNT     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_ANA_L2_ISDX_CCM_CTRL_CCM_MISS_CNT(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Selects ccm interval.
 *
 * \details 
 * 0: not ccm
 * 1: ccm fault management
 * 2: performance management
 * 3: protection switching
 *
 * Field: ::VTSS_ANA_L2_ISDX_CCM_CTRL . CCM_TYPE_INTERVAL
 */
#define  VTSS_F_ANA_L2_ISDX_CCM_CTRL_CCM_TYPE_INTERVAL(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_ANA_L2_ISDX_CCM_CTRL_CCM_TYPE_INTERVAL     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_ANA_L2_ISDX_CCM_CTRL_CCM_TYPE_INTERVAL(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Ingress service dual leaky bucket configuration
 *
 * \details
 * Specifies Service DLB policer index.
 * 
 * If ANA_L2::FWD_CFG.ISDX_DEFAULT_DLB_ENA is set some of the index are
 * used for port default DLB (See ANA_L2::PORT_DLB_CFG.DLB_IDX)
 *
 * Register: \a ANA_L2:ISDX:DLB_CFG
 *
 * @param gi Replicator: x_ANA_NUM_ISDX (??), 0-4095
 */
#define VTSS_ANA_L2_ISDX_DLB_CFG(gi)         VTSS_IOREG_IX(VTSS_TO_ANA_L2,0x0,gi,4,0,3)

/** 
 * \brief
 * Dual leaky bucket index
 *
 * \details 
 * Field: ::VTSS_ANA_L2_ISDX_DLB_CFG . DLB_IDX
 */
#define  VTSS_F_ANA_L2_ISDX_DLB_CFG_DLB_IDX(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_ANA_L2_ISDX_DLB_CFG_DLB_IDX     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_ANA_L2_ISDX_DLB_CFG_DLB_IDX(x)  VTSS_EXTRACT_BITFIELD(x,0,12)

/**
 * Register Group: \a ANA_L2:STICKY
 *
 * Sticky diagnostic status
 */


/** 
 * \brief CCM diagnostic
 *
 * \details
 * Register: \a ANA_L2:STICKY:CCM_STICKY
 */
#define VTSS_ANA_L2_STICKY_CCM_STICKY        VTSS_IOREG(VTSS_TO_ANA_L2,0x4475)

/** 
 * \brief
 * Set if a CCM message has been received with wrong MEPID.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No event has occured
 * 1: CCM miss counter has been cleared
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_CCM_STICKY . CCM_PATH_MEPID_ERR_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_CCM_STICKY_CCM_PATH_MEPID_ERR_STICKY  VTSS_BIT(22)

/** 
 * \brief
 * Set if a port OAM CCM frame has been received without being configured
 * in PORT_CCM_CTRL.CCM_TYPE_INTERVAL.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No event has occured
 * 1: PATH CCM received to a not configured conid
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_CCM_STICKY . CCM_PATH_NOT_CONFIG_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_CCM_STICKY_CCM_PATH_NOT_CONFIG_STICKY  VTSS_BIT(20)

/** 
 * \brief
 * Set if a PATH OAM CCM frame has been received.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No event has occured
 * 1: PATH CCM received
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_CCM_STICKY . CCM_PATH_RECEIVED_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_CCM_STICKY_CCM_PATH_RECEIVED_STICKY  VTSS_BIT(19)

/** 
 * \brief
 * Set if a CCM message has been received with wrong MEPID.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No event has occured
 * 1: CCM miss counter has been cleared
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_CCM_STICKY . CCM_ISDX_MEPID_ERR_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_CCM_STICKY_CCM_ISDX_MEPID_ERR_STICKY  VTSS_BIT(14)

/** 
 * \brief
 * Set if a ISDX OAM CCM frame has been received without being configured
 * in PORT_CCM_CTRL.CCM_TYPE_INTERVAL.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No event has occured
 * 1: ISDX CCM received to a not configured port
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_CCM_STICKY . CCM_ISDX_NOT_CONFIG_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_CCM_STICKY_CCM_ISDX_NOT_CONFIG_STICKY  VTSS_BIT(12)

/** 
 * \brief
 * Set if a ISDX OAM CCM frame has been received.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No event has occured
 * 1: ISDX CCM received
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_CCM_STICKY . CCM_ISDX_RECEIVED_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_CCM_STICKY_CCM_ISDX_RECEIVED_STICKY  VTSS_BIT(11)

/** 
 * \brief
 * Set if a CCM message has been received with wrong MEPID.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No event has occured
 * 1: CCM miss counter has been cleared
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_CCM_STICKY . CCM_PORT_MEPID_ERR_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_CCM_STICKY_CCM_PORT_MEPID_ERR_STICKY  VTSS_BIT(6)

/** 
 * \brief
 * Set if a port OAM CCM frame has been received without being configured
 * in PORT_CCM_CTRL.CCM_TYPE_INTERVAL.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No event has occured
 * 1: Port CCM received to a not configured port
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_CCM_STICKY . CCM_PORT_NOT_CONFIG_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_CCM_STICKY_CCM_PORT_NOT_CONFIG_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Set if a port OAM CCM frame has been received.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No event has occured
 * 1: Port CCM received
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_CCM_STICKY . CCM_PORT_RECEIVED_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_CCM_STICKY_CCM_PORT_RECEIVED_STICKY  VTSS_BIT(3)


/** 
 * \brief Sticky diagnostic status
 *
 * \details
 * Register: \a ANA_L2:STICKY:STICKY
 */
#define VTSS_ANA_L2_STICKY_STICKY            VTSS_IOREG(VTSS_TO_ANA_L2,0x4476)

/** 
 * \brief
 * Set if an autonomous learning operation has failed due to specified lrn
 * rate is exceeded and LEARN max cnt was enabled. Write '1' to clear this
 * field.
 *
 * \details 
 * 0: No cnt exceeded.
 * 1: An autonomous learning operation has failed due to cnt exceeded.
 * Write '1' to clear this field.
 *
 * Field: ::VTSS_ANA_L2_STICKY_STICKY . AUTO_LRN_RATE_EXCEED_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_STICKY_AUTO_LRN_RATE_EXCEED_STICKY  VTSS_BIT(29)

/** 
 * \brief
 * Set if a frame from a B-port from unknown or known source stations with
 * changed port has been received.
 * 
 * Use ANA_L2::BLRN_CFG.PBB_TE_LRN_CHK_ENA to get these frames to a CPU
 * queue.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No PBB-TE source address check fail has occured
 * 1: A PBB-TE source address check fail has occured.
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_STICKY . PBB_TE_SRC_CHK_FAIL_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_STICKY_PBB_TE_SRC_CHK_FAIL_STICKY  VTSS_BIT(28)

/** 
 * \brief
 * Set if a frame from a B-port to CBP has been dropped due to CDA indicate
 * B-domain forward.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No frames have been dropped
 * 1: A frame has been dropped.
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_STICKY . CBP_SRC_FILTER_DROP_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_STICKY_CBP_SRC_FILTER_DROP_STICKY  VTSS_BIT(27)

/** 
 * \brief
 * Set if a frame is received from a B-port as MAC-in-MAC encapsulated with
 * a known BDA with disabled forwarding via Customer Bridge Port
 * (BDA_ENTRY.dual_domain_fwd cleared).
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No frames have been dropped
 * 1: A frame has been dropped.
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_STICKY . KNOWN_CBP_FWD_DROP_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_STICKY_KNOWN_CBP_FWD_DROP_STICKY  VTSS_BIT(26)

/** 
 * \brief
 * Set if a frame destined to C-domain has been dropped due to unknown MC
 * BDA. See ANA_L2::FWD_CFG.UNKNOWN_BMC_CBP_FWD_ENA.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No frames have been dropped
 * 1: A frame has been dropped.
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_STICKY . UNKNOWN_BMC_CBP_FWD_DROP_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_STICKY_UNKNOWN_BMC_CBP_FWD_DROP_STICKY  VTSS_BIT(25)

/** 
 * \brief
 * Set if a frame destined to C-domain has been dropped due to unknown UC
 * BDA. See ANA_L2::FWD_CFG.UNKNOWN_BUC_CBP_FWD_ENA.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No frames have been dropped
 * 1: A frame has been dropped.
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_STICKY . UNKNOWN_BUC_CBP_FWD_DROP_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_STICKY_UNKNOWN_BUC_CBP_FWD_DROP_STICKY  VTSS_BIT(24)

/** 
 * \brief
 * Set if a frame has been alloed due to ANA_L2::PATHGRP.CONID_RX_ENA.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No frames have pathgrp allowed
 * 1: A frame has been correct received on a pathgrp.
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_STICKY . PATHGRP_RX_ALLOW_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_STICKY_PATHGRP_RX_ALLOW_STICKY  VTSS_BIT(23)

/** 
 * \brief
 * Set if a frame has been dropped due to conid not matching expected
 * ANA_L2::PATHGRP.CONID.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No frames have been dropped
 * 1: A frame has been dropped.
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_STICKY . PATHGRP_UNKNOWN_DROP_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_STICKY_PATHGRP_UNKNOWN_DROP_STICKY  VTSS_BIT(22)

/** 
 * \brief
 * Set if a frame has been dropped due to ANA_L2::PATHGRP.CONID_RX_ENA.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No frames have been dropped
 * 1: A frame has been dropped.
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_STICKY . PATHGRP_RX_DROP_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_STICKY_PATHGRP_RX_DROP_STICKY  VTSS_BIT(21)

/** 
 * \brief
 * Set if flood has been removed due to indication from VLAN table.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No event has occured
 * 1: An integrity error has occured
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_STICKY . FWD_FLOOD_KILL_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_STICKY_FWD_FLOOD_KILL_STICKY  VTSS_BIT(20)

/** 
 * \brief
 * Set when an invalid address type is found in the MAC table.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No event has occured
 * 1: An integrity error has occured
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_STICKY . INVALID_ADDR_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_STICKY_INVALID_ADDR_STICKY  VTSS_BIT(19)

/** 
 * \brief
 * Set if forwarding is performed without VLAN contribution either because
 * of a known Destination entry with VLAN ignore set
 * (LRN.MAC_ACCESS_CFG_2.MAC_ENTRY_VLAN_IGNORE FLAG) set or for an unknown
 * destination with flood VLAN ignore set
 * (ANA_L2::FWD_CFG.FLOOD_IGNORE_VLAN_ENA).
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No event has occured
 * 1: VLAN ignore as occured
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_STICKY . VLAN_IGNORE_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_STICKY_VLAN_IGNORE_STICKY  VTSS_BIT(12)

/** 
 * \brief
 * Indication of a frame received with an UPSID != own UPSID
 * (ANA_L2::VSTAX_CTRL.OWN_UPSID) which was previously learned on own front
 * local port.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No event has occured
 * 1: A Local to remote port move has been detected
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_STICKY . LOCAL_TO_REMOTE_PORTMOVE_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_STICKY_LOCAL_TO_REMOTE_PORTMOVE_STICKY  VTSS_BIT(11)

/** 
 * \brief
 * Indication of a frame received on a local front port, which was
 * previously learned on a remote UPSID front port.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No event has occured
 * 1: A remote port to local port move has been detected
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_STICKY . REMOTE_TO_LOCAL_PORTMOVE_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_STICKY_REMOTE_TO_LOCAL_PORTMOVE_STICKY  VTSS_BIT(10)

/** 
 * \brief
 * Indication of a frame received on a remote front port, which was
 * previously learned on another remote UPSID front port.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No event has occured
 * 1: A remote port to remote port move has been detected
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_STICKY . REMOTE_TO_REMOTE_PORTMOVE_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_STICKY_REMOTE_TO_REMOTE_PORTMOVE_STICKY  VTSS_BIT(9)

/** 
 * \brief
 * Indication of a frame received on a GLAG which was previously learned
 * with a different GLAG.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No event has occured
 * 1: A GLAG to GLAG port move has been detected
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_STICKY . GLOBAL_TO_GLOBAL_PORTMOVE_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_STICKY_GLOBAL_TO_GLOBAL_PORTMOVE_STICKY  VTSS_BIT(8)

/** 
 * \brief
 * Indication of a frame received at a front local port which was
 * previously learned with a GLAG .
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No event has occured
 * 1: A GLAG to local port move has been detected
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_STICKY . GLOBAL_TO_LOCAL_PORTMOVE_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_STICKY_GLOBAL_TO_LOCAL_PORTMOVE_STICKY  VTSS_BIT(7)

/** 
 * \brief
 * Indication of a frame received with GLAG which was previously learned at
 * a front local port.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No event has occured
 * 1: A Local to GLAG port move has been detected
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_STICKY . LOCAL_TO_GLOBAL_PORTMOVE_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_STICKY_LOCAL_TO_GLOBAL_PORTMOVE_STICKY  VTSS_BIT(6)

/** 
 * \brief
 * Indication of a frame received on a local front port, which was
 * previously learned at a different local front port.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No event has occured
 * 1: A Local to local port move has been detected
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_STICKY . LOCAL_TO_LOCAL_PORTMOVE_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_STICKY_LOCAL_TO_LOCAL_PORTMOVE_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * Set if forwarding is based on flood.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0:  No flood event
 * 1: Flood event
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_STICKY . FWD_FLOOD_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_STICKY_FWD_FLOOD_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Set if a valid (EFID, DMAC) entry was found to be used for forwarding.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No (EFID, DMAC) entries have been found
 * 1: An (EFID, DMAC) entry has been found.
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_STICKY . FWD_ENTRY_FOUND_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_STICKY_FWD_ENTRY_FOUND_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * Set if a frame has been dropped due to
 * ANA_L2::LRN_SECURE_CFG.DROP_UNKNOWN_SRC_ENA.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No frames have been dropped
 * 1: A frame has been dropped.
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_STICKY_STICKY . DROP_UNKNOWN_SRC_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_STICKY_DROP_UNKNOWN_SRC_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Set if a valid (IFID, SMAC) entry was found.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No known source entries (IFID, SMAC) has been found
 * 1: At least one known entry (IFID, SMAC) has been found.
 * 
 * Write 1 to clear this field.
 *
 * Field: ::VTSS_ANA_L2_STICKY_STICKY . LEARN_KNOWN_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_STICKY_LEARN_KNOWN_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Set if no valid (IFID, SMAC) entry was found.
 * 
 * To enable the event as one of four counter events to the PORT STAT block
 * set the corresponding *_STICKY_MASK
 *
 * \details 
 * 0: No unknown sources (IFID, SMAC) has been found
 * 1: At least one unknown source IFID, SMAC) has been found.
 * 
 * Write 1 to clear this field.
 *
 * Field: ::VTSS_ANA_L2_STICKY_STICKY . LEARN_UNKNOWN_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_STICKY_LEARN_UNKNOWN_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a ANA_L2:STICKY_MASK
 *
 * Sticky diagnostic global port counter event configuration
 */


/** 
 * \brief CCM Diagnostic counter mask
 *
 * \details
 * Register: \a ANA_L2:STICKY_MASK:CCM_STICKY_MASK
 *
 * @param gi Replicator: x_ANA_NUM_CONCURRENT_CNT (??), 0-3
 */
#define VTSS_ANA_L2_STICKY_MASK_CCM_STICKY_MASK(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_L2,0x4477,gi,2,0,0)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_CCM_STICKY_MASK . CCM_PATH_MEPID_ERR_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_CCM_STICKY_MASK_CCM_PATH_MEPID_ERR_STICKY_MASK  VTSS_BIT(14)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_CCM_STICKY_MASK . CCM_PATH_NOT_CONFIG_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_CCM_STICKY_MASK_CCM_PATH_NOT_CONFIG_STICKY_MASK  VTSS_BIT(12)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_CCM_STICKY_MASK . CCM_PATH_RECEIVED_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_CCM_STICKY_MASK_CCM_PATH_RECEIVED_STICKY_MASK  VTSS_BIT(11)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_CCM_STICKY_MASK . CCM_ISDX_MEPID_ERR_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_CCM_STICKY_MASK_CCM_ISDX_MEPID_ERR_STICKY_MASK  VTSS_BIT(9)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_CCM_STICKY_MASK . CCM_ISDX_NOT_CONFIG_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_CCM_STICKY_MASK_CCM_ISDX_NOT_CONFIG_STICKY_MASK  VTSS_BIT(7)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_CCM_STICKY_MASK . CCM_ISDX_RECEIVED_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_CCM_STICKY_MASK_CCM_ISDX_RECEIVED_STICKY_MASK  VTSS_BIT(6)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_CCM_STICKY_MASK . CCM_PORT_MEPID_ERR_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_CCM_STICKY_MASK_CCM_PORT_MEPID_ERR_STICKY_MASK  VTSS_BIT(4)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_CCM_STICKY_MASK . CCM_PORT_NOT_CONFIG_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_CCM_STICKY_MASK_CCM_PORT_NOT_CONFIG_STICKY_MASK  VTSS_BIT(2)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_CCM_STICKY_MASK . CCM_PORT_RECEIVED_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_CCM_STICKY_MASK_CCM_PORT_RECEIVED_STICKY_MASK  VTSS_BIT(1)


/** 
 * \brief Sticky diagnostic counter mask
 *
 * \details
 * Register: \a ANA_L2:STICKY_MASK:STICKY_MASK
 *
 * @param gi Replicator: x_ANA_NUM_CONCURRENT_CNT (??), 0-3
 */
#define VTSS_ANA_L2_STICKY_MASK_STICKY_MASK(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_L2,0x4477,gi,2,0,1)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . PBB_TE_SRC_CHK_FAIL_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_PBB_TE_SRC_CHK_FAIL_STICKY_MASK  VTSS_BIT(28)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . CBP_SRC_FILTER_DROP_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_CBP_SRC_FILTER_DROP_STICKY_MASK  VTSS_BIT(27)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . KNOWN_CBP_FWD_DROP_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_KNOWN_CBP_FWD_DROP_STICKY_MASK  VTSS_BIT(26)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . UNKNOWN_BMC_CBP_FWD_DROP_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_UNKNOWN_BMC_CBP_FWD_DROP_STICKY_MASK  VTSS_BIT(25)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . UNKNOWN_BUC_CBP_FWD_DROP_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_UNKNOWN_BUC_CBP_FWD_DROP_STICKY_MASK  VTSS_BIT(24)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . PATHGRP_RX_ALLOW_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_PATHGRP_RX_ALLOW_STICKY_MASK  VTSS_BIT(23)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . PATHGRP_UNKNOWN_DROP_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_PATHGRP_UNKNOWN_DROP_STICKY_MASK  VTSS_BIT(22)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . PATHGRP_RX_DROP_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_PATHGRP_RX_DROP_STICKY_MASK  VTSS_BIT(21)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . FWD_FLOOD_KILL_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_FWD_FLOOD_KILL_STICKY_MASK  VTSS_BIT(20)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . INVALID_ADDR_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_INVALID_ADDR_STICKY_MASK  VTSS_BIT(19)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . VLAN_IGNORE_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_VLAN_IGNORE_STICKY_MASK  VTSS_BIT(12)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . LOCAL_TO_REMOTE_PORTMOVE_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_LOCAL_TO_REMOTE_PORTMOVE_STICKY_MASK  VTSS_BIT(11)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . REMOTE_TO_LOCAL_PORTMOVE_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_REMOTE_TO_LOCAL_PORTMOVE_STICKY_MASK  VTSS_BIT(10)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . REMOTE_TO_REMOTE_PORTMOVE_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_REMOTE_TO_REMOTE_PORTMOVE_STICKY_MASK  VTSS_BIT(9)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . GLOBAL_TO_GLOBAL_PORTMOVE_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_GLOBAL_TO_GLOBAL_PORTMOVE_STICKY_MASK  VTSS_BIT(8)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . GLOBAL_TO_LOCAL_PORTMOVE_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_GLOBAL_TO_LOCAL_PORTMOVE_STICKY_MASK  VTSS_BIT(7)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . LOCAL_TO_GLOBAL_PORTMOVE_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_LOCAL_TO_GLOBAL_PORTMOVE_STICKY_MASK  VTSS_BIT(6)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . LOCAL_TO_LOCAL_PORTMOVE_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_LOCAL_TO_LOCAL_PORTMOVE_STICKY_MASK  VTSS_BIT(5)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . FWD_ENTRY_FOUND_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_FWD_ENTRY_FOUND_STICKY_MASK  VTSS_BIT(4)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . FWD_FLOOD_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_FWD_FLOOD_STICKY_MASK  VTSS_BIT(3)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . DROP_UNKNOWN_SRC_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_DROP_UNKNOWN_SRC_STICKY_MASK  VTSS_BIT(2)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . LEARN_UNKNOWN_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_LEARN_UNKNOWN_STICKY_MASK  VTSS_BIT(1)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . LEARN_KNOWN_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_LEARN_KNOWN_STICKY_MASK  VTSS_BIT(0)


#endif /* _VTSS_JAGUAR_REGS_ANA_L2_H_ */
