#ifndef _VTSS_JAGUAR2_REGS_ANA_L2_H_
#define _VTSS_JAGUAR2_REGS_ANA_L2_H_

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
#define VTSS_ANA_L2_COMMON_FWD_CFG           VTSS_IOREG(VTSS_TO_ANA_L2,0x228aa)

/** 
 * \brief
 * Configures MAC table Size
 *
 * \details 
 * "00": No MAC tables
 * "01": 8K CMAC table
 * "10": 16K CMAC table
 * "11": 32K CMAC table
 *
 * Field: ::VTSS_ANA_L2_COMMON_FWD_CFG . MAC_TBL_SPLIT_SEL
 */
#define  VTSS_F_ANA_L2_COMMON_FWD_CFG_MAC_TBL_SPLIT_SEL(x)  VTSS_ENCODE_BITFIELD(x,20,2)
#define  VTSS_M_ANA_L2_COMMON_FWD_CFG_MAC_TBL_SPLIT_SEL     VTSS_ENCODE_BITMASK(20,2)
#define  VTSS_X_ANA_L2_COMMON_FWD_CFG_MAC_TBL_SPLIT_SEL(x)  VTSS_EXTRACT_BITFIELD(x,20,2)

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
#define  VTSS_F_ANA_L2_COMMON_FWD_CFG_ISDX_DEFAULT_DLB_ENA  VTSS_BIT(18)

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
 * \brief Learn configuration
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
#define VTSS_ANA_L2_COMMON_LRN_CFG           VTSS_IOREG(VTSS_TO_ANA_L2,0x228ab)

/** 
 * \brief
 * Setting VSTAX_LRN_SKIP_ENA=1 causes the lrn_mode field in the VStaX
 * header to be set to 1 if the auto learning is disabled for the frame.
 * 
 * I.e. if auto learning in the local unit is not allowed for the frame,
 * other units in the stack will not do auto learning of the forwarded
 * frame either.
 * 
 * Auto learning can be disabled due to
 * ANA_L2::AUTO_LRN_CFG
 * ANA_L3:MSTP:MSTP_LRN_CFG
 * ANA_L3::PORT_LRN_CTRL
 * ANA_L3:VLAN:VLAN_CFG.VLAN_LRN_DIS
 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_LRN_CFG . VSTAX_LRN_SKIP_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_LRN_CFG_VSTAX_LRN_SKIP_ENA  VTSS_BIT(25)

/** 
 * \brief
 * Configures entries learned on a multicast index to not paticipate in
 * learning.
 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_LRN_CFG . IGNORE_MCIDX_PORTMOVE_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_LRN_CFG_IGNORE_MCIDX_PORTMOVE_ENA  VTSS_BIT(24)

/** 
 * \brief
 * Configures the CPU queue for frames copied to CPU due to FID LEARN limit
 * check. 
 * 
 * Frames are send to this CPU queue.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_L2_COMMON_LRN_CFG . CPU_FID_LIMIT_QU
 */
#define  VTSS_F_ANA_L2_COMMON_LRN_CFG_CPU_FID_LIMIT_QU(x)  VTSS_ENCODE_BITFIELD(x,20,3)
#define  VTSS_M_ANA_L2_COMMON_LRN_CFG_CPU_FID_LIMIT_QU     VTSS_ENCODE_BITMASK(20,3)
#define  VTSS_X_ANA_L2_COMMON_LRN_CFG_CPU_FID_LIMIT_QU(x)  VTSS_EXTRACT_BITFIELD(x,20,3)

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
#define  VTSS_F_ANA_L2_COMMON_LRN_CFG_CPU_COPY_LRN_LIMIT_EXCEED_ENA  VTSS_BIT(19)

/** 
 * \brief
 * Configures if aged entries are choosen before nonaged when randomly
 * selecting a non locket entry to replace
 *
 * \details 
 * 0: age flags are not considered when selecting an entry
 * 1: age flags are considered when selecting an entry.
 *
 * Field: ::VTSS_ANA_L2_COMMON_LRN_CFG . AGED_BEFORE_NONAGED
 */
#define  VTSS_F_ANA_L2_COMMON_LRN_CFG_AGED_BEFORE_NONAGED  VTSS_BIT(18)

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
 * Configures entries in CAM row to be moved back to MAC table when empty
 * entries exists
 *
 * \details 
 * 0: Known CAM entries are not moved back to MAC table
 * 1: Known CAM entries are moved to MAC table when empty entries exists on
 * a hashed row.
 *
 * Field: ::VTSS_ANA_L2_COMMON_LRN_CFG . LRN_MOVE_CAM_ENTRY_BACK
 */
#define  VTSS_F_ANA_L2_COMMON_LRN_CFG_LRN_MOVE_CAM_ENTRY_BACK  VTSS_BIT(15)

/** 
 * \brief
 * Configures auto learning use of entries in the CAM. 
 * 
 * See ANA_L2::AUTO_LRN_CFG
 *
 * \details 
 * 0: Auto learn is limited to CMAC table
 * 1: Auto learn may also use MAC CAM entries.
 *
 * Field: ::VTSS_ANA_L2_COMMON_LRN_CFG . AUTO_LRN_USE_MAC_CAM_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_LRN_CFG_AUTO_LRN_USE_MAC_CAM_ENA  VTSS_BIT(14)

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
 * Configures use of entries in the CAM row for CPU learning. 

 *
 * \details 
 * 0: CPU learn is limited to MAC table
 * 1: CPU learn may also use MAC CAM entries.
 *
 * Field: ::VTSS_ANA_L2_COMMON_LRN_CFG . CPU_LRN_USE_MAC_CAM_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_LRN_CFG_CPU_LRN_USE_MAC_CAM_ENA  VTSS_BIT(12)

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
#define VTSS_ANA_L2_COMMON_FILTER_OTHER_CTRL  VTSS_IOREG(VTSS_TO_ANA_L2,0x228ac)

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
#define VTSS_ANA_L2_COMMON_FILTER_LOCAL_CTRL  VTSS_IOREG(VTSS_TO_ANA_L2,0x228ad)


/** 
 * \brief Front Port scan filter configuration
 *
 * \details
 * Register: \a ANA_L2:COMMON:FILTER_LOCAL_CTRL1
 */
#define VTSS_ANA_L2_COMMON_FILTER_LOCAL_CTRL1  VTSS_IOREG(VTSS_TO_ANA_L2,0x228ae)

/** 
 * \brief
 * Pls. refer to FILTER_LOCAL_CTRL.FILTER_FRONTPORT_ENA description.
 *
 * \details 
 * x0x: Handle entries associated with the front port during automatic
 * ageing and CPU scan
 * x1x: Ignore entries associated with the front port during automatic
 * ageing and CPU scan

 *
 * Field: ::VTSS_ANA_L2_COMMON_FILTER_LOCAL_CTRL1 . FILTER_FRONTPORT_ENA1
 */
#define  VTSS_F_ANA_L2_COMMON_FILTER_LOCAL_CTRL1_FILTER_FRONTPORT_ENA1(x)  VTSS_ENCODE_BITFIELD(x,0,21)
#define  VTSS_M_ANA_L2_COMMON_FILTER_LOCAL_CTRL1_FILTER_FRONTPORT_ENA1     VTSS_ENCODE_BITMASK(0,21)
#define  VTSS_X_ANA_L2_COMMON_FILTER_LOCAL_CTRL1_FILTER_FRONTPORT_ENA1(x)  VTSS_EXTRACT_BITFIELD(x,0,21)


/** 
 * \brief Auto learn configuration
 *
 * \details
 * Configures automatic learning per port 
 *
 * Register: \a ANA_L2:COMMON:AUTO_LRN_CFG
 */
#define VTSS_ANA_L2_COMMON_AUTO_LRN_CFG      VTSS_IOREG(VTSS_TO_ANA_L2,0x228af)


/** 
 * \brief Auto learn configuration
 *
 * \details
 * Configures automatic learning per port 
 *
 * Register: \a ANA_L2:COMMON:AUTO_LRN_CFG1
 */
#define VTSS_ANA_L2_COMMON_AUTO_LRN_CFG1     VTSS_IOREG(VTSS_TO_ANA_L2,0x228b0)

/** 
 * \brief
 * Pls. refer to AUTO_LRN_CFG.AUTO_LRN_ENA description.
 *
 * \details 
 * x0x: Disable automatic learning for the port
 * x1x: Enable automatic learning of incoming learn frames received on this
 * port
 *
 * Field: ::VTSS_ANA_L2_COMMON_AUTO_LRN_CFG1 . AUTO_LRN_ENA1
 */
#define  VTSS_F_ANA_L2_COMMON_AUTO_LRN_CFG1_AUTO_LRN_ENA1(x)  VTSS_ENCODE_BITFIELD(x,0,21)
#define  VTSS_M_ANA_L2_COMMON_AUTO_LRN_CFG1_AUTO_LRN_ENA1     VTSS_ENCODE_BITMASK(0,21)
#define  VTSS_X_ANA_L2_COMMON_AUTO_LRN_CFG1_AUTO_LRN_ENA1(x)  VTSS_EXTRACT_BITFIELD(x,0,21)


/** 
 * \brief Secure learn configuration
 *
 * \details
 * Configures secure forwarding per port 
 *
 * Register: \a ANA_L2:COMMON:LRN_SECUR_CFG
 */
#define VTSS_ANA_L2_COMMON_LRN_SECUR_CFG     VTSS_IOREG(VTSS_TO_ANA_L2,0x228b1)


/** 
 * \brief Secure learn configuration
 *
 * \details
 * Configures secure forwarding per port 
 *
 * Register: \a ANA_L2:COMMON:LRN_SECUR_CFG1
 */
#define VTSS_ANA_L2_COMMON_LRN_SECUR_CFG1    VTSS_IOREG(VTSS_TO_ANA_L2,0x228b2)

/** 
 * \brief
 * Pls. refer to LRN_SECUR_CFG.DROP_UNKNOWN_SRC_ENA description.
 *
 * \details 
 * x0x: Disable drop of incoming learn frames for the port
 * x1x: Enable drop of incoming learn frames received on this port
 *
 * Field: ::VTSS_ANA_L2_COMMON_LRN_SECUR_CFG1 . DROP_UNKNOWN_SRC_ENA1
 */
#define  VTSS_F_ANA_L2_COMMON_LRN_SECUR_CFG1_DROP_UNKNOWN_SRC_ENA1(x)  VTSS_ENCODE_BITFIELD(x,0,21)
#define  VTSS_M_ANA_L2_COMMON_LRN_SECUR_CFG1_DROP_UNKNOWN_SRC_ENA1     VTSS_ENCODE_BITMASK(0,21)
#define  VTSS_X_ANA_L2_COMMON_LRN_SECUR_CFG1_DROP_UNKNOWN_SRC_ENA1(x)  VTSS_EXTRACT_BITFIELD(x,0,21)


/** 
 * \brief Secure learn configuration for locked entries
 *
 * \details
 * Configures secure forwarding for static (LOCKED) entries per port 
 *
 * Register: \a ANA_L2:COMMON:LRN_SECUR_LOCKED_CFG
 */
#define VTSS_ANA_L2_COMMON_LRN_SECUR_LOCKED_CFG  VTSS_IOREG(VTSS_TO_ANA_L2,0x228b3)


/** 
 * \brief Secure learn configuration for locked entries
 *
 * \details
 * Configures secure forwarding for static (LOCKED) entries per port 
 *
 * Register: \a ANA_L2:COMMON:LRN_SECUR_LOCKED_CFG1
 */
#define VTSS_ANA_L2_COMMON_LRN_SECUR_LOCKED_CFG1  VTSS_IOREG(VTSS_TO_ANA_L2,0x228b4)

/** 
 * \brief
 * Pls. refer to LRN_SECUR_LOCKED_CFG.DROP_MOVED_LOCKED_ENA description.
 *
 * \details 
 * x0x: Disable
 * x1x: Enable drop of incoming frames triggering a port move for a locked
 * entry in the MAC table received on this port
 *
 * Field: ::VTSS_ANA_L2_COMMON_LRN_SECUR_LOCKED_CFG1 . DROP_MOVED_LOCKED_ENA1
 */
#define  VTSS_F_ANA_L2_COMMON_LRN_SECUR_LOCKED_CFG1_DROP_MOVED_LOCKED_ENA1(x)  VTSS_ENCODE_BITFIELD(x,0,21)
#define  VTSS_M_ANA_L2_COMMON_LRN_SECUR_LOCKED_CFG1_DROP_MOVED_LOCKED_ENA1     VTSS_ENCODE_BITMASK(0,21)
#define  VTSS_X_ANA_L2_COMMON_LRN_SECUR_LOCKED_CFG1_DROP_MOVED_LOCKED_ENA1(x)  VTSS_EXTRACT_BITFIELD(x,0,21)


/** 
 * \brief Learn copy configuration
 *
 * \details
 * Configures CPU copy of learn frames per port
 *
 * Register: \a ANA_L2:COMMON:LRN_COPY_CFG
 */
#define VTSS_ANA_L2_COMMON_LRN_COPY_CFG      VTSS_IOREG(VTSS_TO_ANA_L2,0x228b5)


/** 
 * \brief Learn copy configuration
 *
 * \details
 * Configures CPU copy of learn frames per port
 *
 * Register: \a ANA_L2:COMMON:LRN_COPY_CFG1
 */
#define VTSS_ANA_L2_COMMON_LRN_COPY_CFG1     VTSS_IOREG(VTSS_TO_ANA_L2,0x228b6)

/** 
 * \brief
 * Pls. refer to LRN_COPY_CFG.CPU_LRN_COPY_ENA description.
 *
 * \details 
 * x0x: Disable
 * x1x: incoming learn frames received on this port are copied to the CPU.
 *
 * Field: ::VTSS_ANA_L2_COMMON_LRN_COPY_CFG1 . CPU_LRN_COPY_ENA1
 */
#define  VTSS_F_ANA_L2_COMMON_LRN_COPY_CFG1_CPU_LRN_COPY_ENA1(x)  VTSS_ENCODE_BITFIELD(x,0,21)
#define  VTSS_M_ANA_L2_COMMON_LRN_COPY_CFG1_CPU_LRN_COPY_ENA1     VTSS_ENCODE_BITMASK(0,21)
#define  VTSS_X_ANA_L2_COMMON_LRN_COPY_CFG1_CPU_LRN_COPY_ENA1(x)  VTSS_EXTRACT_BITFIELD(x,0,21)


/** 
 * \brief Service DLB Port default configuration
 *
 * \details
 * Configures per port dual leaky bucket indexes
 *
 * Register: \a ANA_L2:COMMON:PORT_DLB_CFG
 *
 * @param ri Replicator: x_FFL_CHIP_NUM_ETH_PORTS (??), 0-52
 */
#define VTSS_ANA_L2_COMMON_PORT_DLB_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_L2,0x228b7 + (ri))

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
 * \brief Configures if additional VID/FID are used when doing scan
 *
 * \details
 * This is to allow faster ring protection update
 *
 * Register: \a ANA_L2:COMMON:SCAN_FID_CTRL
 */
#define VTSS_ANA_L2_COMMON_SCAN_FID_CTRL     VTSS_IOREG(VTSS_TO_ANA_L2,0x228ec)

/** 
 * \brief
 * Controls use of additional FIDs used when doing scan.
 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_SCAN_FID_CTRL . SCAN_FID_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_SCAN_FID_CTRL_SCAN_FID_ENA  VTSS_BIT(0)


/** 
 * \brief Configures a number of additional VID/FID to be used when doing scan
 *
 * \details
 * This is to allow faster ring protection update
 *
 * Register: \a ANA_L2:COMMON:SCAN_FID_CFG
 *
 * @param ri Replicator: x_FFL_ANA_NUM_SCAN_FID (??), 0-15
 */
#define VTSS_ANA_L2_COMMON_SCAN_FID_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_L2,0x228ed + (ri))

/** 
 * \brief
 * Configures additional FIDs used when doing scan.
 *
 * \details 
 * 0: Disable
 * >0: FID is matched when doing scan.
 *
 * Field: ::VTSS_ANA_L2_COMMON_SCAN_FID_CFG . SCAN_FID_VAL
 */
#define  VTSS_F_ANA_L2_COMMON_SCAN_FID_CFG_SCAN_FID_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,13)
#define  VTSS_M_ANA_L2_COMMON_SCAN_FID_CFG_SCAN_FID_VAL     VTSS_ENCODE_BITMASK(0,13)
#define  VTSS_X_ANA_L2_COMMON_SCAN_FID_CFG_SCAN_FID_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,13)


/** 
 * \brief Sticky Portmove status
 *
 * \details
 * Identifies ports with moved stations
 *
 * Register: \a ANA_L2:COMMON:MOVELOG_STICKY
 */
#define VTSS_ANA_L2_COMMON_MOVELOG_STICKY    VTSS_IOREG(VTSS_TO_ANA_L2,0x228fd)


/** 
 * \brief Sticky Portmove status
 *
 * \details
 * Identifies ports with moved stations
 *
 * Register: \a ANA_L2:COMMON:MOVELOG_STICKY1
 */
#define VTSS_ANA_L2_COMMON_MOVELOG_STICKY1   VTSS_IOREG(VTSS_TO_ANA_L2,0x228fe)

/** 
 * \brief
 * Pls. refer to MOVELOG_STICKY.PORTMOVE_LOG_STICKY description.
 *
 * \details 
 * 'XX...XXX': If port X is set, at least one station has moved to logical
 * port X.
 *
 * Field: ::VTSS_ANA_L2_COMMON_MOVELOG_STICKY1 . PORTMOVE_LOG_STICKY1
 */
#define  VTSS_F_ANA_L2_COMMON_MOVELOG_STICKY1_PORTMOVE_LOG_STICKY1(x)  VTSS_ENCODE_BITFIELD(x,0,21)
#define  VTSS_M_ANA_L2_COMMON_MOVELOG_STICKY1_PORTMOVE_LOG_STICKY1     VTSS_ENCODE_BITMASK(0,21)
#define  VTSS_X_ANA_L2_COMMON_MOVELOG_STICKY1_PORTMOVE_LOG_STICKY1(x)  VTSS_EXTRACT_BITFIELD(x,0,21)


/** 
 * \brief VSTAX configuration
 *
 * \details
 * Configures stacking learning and forwarding operation
 *
 * Register: \a ANA_L2:COMMON:VSTAX_CTRL
 */
#define VTSS_ANA_L2_COMMON_VSTAX_CTRL        VTSS_IOREG(VTSS_TO_ANA_L2,0x228ff)

/** 
 * \brief
 * Enable setting nxt_lrn_all for frames route modified in Analyzer that
 * ought to trigger lrn_all.
 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_VSTAX_CTRL . VSTAX2_NXT_LRN_ALL_RT_ACL_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_VSTAX_CTRL_VSTAX2_NXT_LRN_ALL_RT_ACL_ENA  VTSS_BIT(25)

/** 
 * \brief
 * Enable copy to local CPU for frames route modified in Analyzer that
 * ought to trigger lrn_all.
 * 
 * Copies are send to the CPU queue specified by
 * ANA_L2::VSTAX_CTRL.CPU_VSTAX_QU. 
 *
 * \details 
 * Field: ::VTSS_ANA_L2_COMMON_VSTAX_CTRL . VSTAX2_LCPU_CP_ACL_RT_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_VSTAX_CTRL_VSTAX2_LCPU_CP_ACL_RT_ENA  VTSS_BIT(24)

/** 
 * \brief
 * Configures own UPSID to be used for stacking.
 * 
 * This must be configured consistently across the following registers:
 * ANA_CL::UPSID_CFG.UPSID_NUM
 * ANA_AC::COMMON_VSTAX_CFG.OWN_UPSID
 * ANA_L2::VSTAX_CTRL.OWN_UPSID
 * REW::COMMON_CTRL.OWN_UPSID
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
 * \brief ANA_L2 Interrupt events
 *
 * \details
 * ANA_L2 Interrupt events if interrupt enable is set in ANA_L2::INTR_ENA
 * and the corresponding event is triggered in ANA_L2::INTR_IDENT
 *
 * Register: \a ANA_L2:COMMON:INTR
 */
#define VTSS_ANA_L2_COMMON_INTR              VTSS_IOREG(VTSS_TO_ANA_L2,0x22900)

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
#define  VTSS_F_ANA_L2_COMMON_INTR_LRN_ACCESS_COMPLETE_INTR  VTSS_BIT(3)

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
#define  VTSS_F_ANA_L2_COMMON_INTR_VCAP_S2_INTR  VTSS_BIT(2)

/** 
 * \brief
 * Set if Port learn limit triggered interrupt.
 * 
 * To enable interrupt of this events set the corresponding *_INTR_ENA
 *
 * \details 
 * 0: No event has occured
 * 1: VCAP_S2 triggered inq occured
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR . PORT_LRN_LIMIT_INTR
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_PORT_LRN_LIMIT_INTR  VTSS_BIT(1)

/** 
 * \brief
 * Set if FID learn limit triggered interrupt.
 * 
 * To enable interrupt of this events set the corresponding *_INTR_ENA
 *
 * \details 
 * 0: No event has occured
 * 1: VCAP_S2 triggered inq occured
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR . FID_LIMIT_INTR
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_FID_LIMIT_INTR  VTSS_BIT(0)


/** 
 * \brief ANA_L2 interrupt enable mask
 *
 * \details
 * Register: \a ANA_L2:COMMON:INTR_ENA
 */
#define VTSS_ANA_L2_COMMON_INTR_ENA          VTSS_IOREG(VTSS_TO_ANA_L2,0x22901)

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
#define  VTSS_F_ANA_L2_COMMON_INTR_ENA_LRN_ACCESS_COMPLETE_INTR_ENA  VTSS_BIT(3)

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
#define  VTSS_F_ANA_L2_COMMON_INTR_ENA_VCAP_S2_INTR_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Configures VCAP_S2_INTR interrupts
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_ENA . PORT_LRN_LIMIT_INTR_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_ENA_PORT_LRN_LIMIT_INTR_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Configures FID_LIMIT_INTR interrupts
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_ENA . FID_LIMIT_INTR_ENA
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_ENA_FID_LIMIT_INTR_ENA  VTSS_BIT(0)


/** 
 * \brief ANA_L2 interrupt status
 *
 * \details
 * Register: \a ANA_L2:COMMON:INTR_IDENT
 */
#define VTSS_ANA_L2_COMMON_INTR_IDENT        VTSS_IOREG(VTSS_TO_ANA_L2,0x22902)

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
#define  VTSS_F_ANA_L2_COMMON_INTR_IDENT_LRN_ACCESS_COMPLETE_INTR_IDENT  VTSS_BIT(3)

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
#define  VTSS_F_ANA_L2_COMMON_INTR_IDENT_VCAP_S2_INTR_IDENT  VTSS_BIT(2)

/** 
 * \brief
 * Status of PORT_LRN_LIMIT_INTR interrupts
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt to CPU
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_IDENT . PORT_LRN_LIMIT_INTR_IDENT
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_IDENT_PORT_LRN_LIMIT_INTR_IDENT  VTSS_BIT(1)

/** 
 * \brief
 * Status of FID_LIMIT_INTR interrupts
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt to CPU
 *
 * Field: ::VTSS_ANA_L2_COMMON_INTR_IDENT . FID_LIMIT_INTR_IDENT
 */
#define  VTSS_F_ANA_L2_COMMON_INTR_IDENT_FID_LIMIT_INTR_IDENT  VTSS_BIT(0)

/**
 * Register Group: \a ANA_L2:LRN_LIMIT
 *
 * Learn limits per FID
 */


/** 
 * \brief Controls automatic learn limits
 *
 * \details
 * Per VSI configuration of autolearn limits
 *
 * Register: \a ANA_L2:LRN_LIMIT:FID_LIMIT_STATUS
 *
 * @param gi Replicator: x_FFL_ANA_NUM_FIDS (??), 0-5119
 */
#define VTSS_ANA_L2_LRN_LIMIT_FID_LIMIT_STATUS(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_L2,0x20000,gi,2,0,0)

/** 
 * \brief
 * Set if specified MAX learn cnt limit is exceeded and max learn cnt was
 * enabled. Write '1' to clear this field.
 *
 * \details 
 * 0: Learn cnt not exceeded.
 * 1: Learning operation has failed due to FID max learn cnt exceeded.
 * Write '1' to clear this field.
 *
 * Field: ::VTSS_ANA_L2_LRN_LIMIT_FID_LIMIT_STATUS . FID_LRN_LIMIT_EXCEEDED_STICKY
 */
#define  VTSS_F_ANA_L2_LRN_LIMIT_FID_LIMIT_STATUS_FID_LRN_LIMIT_EXCEEDED_STICKY  VTSS_BIT(16)

/** 
 * \brief
 * Contains the number of MAC table entries currently learned associated
 * with a given FID.
 *
 * \details 
 * 0 : no entries
 *
 * Field: ::VTSS_ANA_L2_LRN_LIMIT_FID_LIMIT_STATUS . FID_LRN_CNT
 */
#define  VTSS_F_ANA_L2_LRN_LIMIT_FID_LIMIT_STATUS_FID_LRN_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_ANA_L2_LRN_LIMIT_FID_LIMIT_STATUS_FID_LRN_CNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_ANA_L2_LRN_LIMIT_FID_LIMIT_STATUS_FID_LRN_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Controls automatic learn limits per FID
 *
 * \details
 * Per FID configuration of MAC table learn limits
 *
 * Register: \a ANA_L2:LRN_LIMIT:FID_LIMIT_CTRL
 *
 * @param gi Replicator: x_FFL_ANA_NUM_FIDS (??), 0-5119
 */
#define VTSS_ANA_L2_LRN_LIMIT_FID_LIMIT_CTRL(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_L2,0x20000,gi,2,0,1)

/** 
 * \brief
 * Allow setting FID_LIMIT_INTR when exceeding limit on learning (happens
 * when MAC address are supposed to be installed in the MAC table.
 *
 * \details 
 * 0: Disable 
 * 1: allow FID_LIMIT_INTR to be set upon trying to learn a MAC address
 * that causes learn limit to be exeeded
 *
 * Field: ::VTSS_ANA_L2_LRN_LIMIT_FID_LIMIT_CTRL . FID_LIMIT_EXCEED_IRQ_ENA
 */
#define  VTSS_F_ANA_L2_LRN_LIMIT_FID_LIMIT_CTRL_FID_LIMIT_EXCEED_IRQ_ENA  VTSS_BIT(17)

/** 
 * \brief
 * Action for traffic when learn limit is exceeded.
 *
 * \details 
 * 00: Normal forward
 * 01: Enable redirection to CPU queue
 * 10: Enable copy to CPU queue
 * 11: Discard the frame
 *
 * Field: ::VTSS_ANA_L2_LRN_LIMIT_FID_LIMIT_CTRL . FID_LIMIT_EXCEED_SEL
 */
#define  VTSS_F_ANA_L2_LRN_LIMIT_FID_LIMIT_CTRL_FID_LIMIT_EXCEED_SEL(x)  VTSS_ENCODE_BITFIELD(x,15,2)
#define  VTSS_M_ANA_L2_LRN_LIMIT_FID_LIMIT_CTRL_FID_LIMIT_EXCEED_SEL     VTSS_ENCODE_BITMASK(15,2)
#define  VTSS_X_ANA_L2_LRN_LIMIT_FID_LIMIT_CTRL_FID_LIMIT_EXCEED_SEL(x)  VTSS_EXTRACT_BITFIELD(x,15,2)

/** 
 * \brief
 * Configures the number of MAC table entries that can be used for a given
 * FID (via Automatic learning and CPU based learning with LOCK bit cleared
 * and not multicast).
 *
 * \details 
 * 0: Disable i.e. no learn limit for the FID
 * 1: Only learning of one MAC address allowed for this FID
 * ...
 * n: Learning of n MAC address allowed for this FID
 *
 * Field: ::VTSS_ANA_L2_LRN_LIMIT_FID_LIMIT_CTRL . FID_LRN_CNT_LIMIT
 */
#define  VTSS_F_ANA_L2_LRN_LIMIT_FID_LIMIT_CTRL_FID_LRN_CNT_LIMIT(x)  VTSS_ENCODE_BITFIELD(x,0,15)
#define  VTSS_M_ANA_L2_LRN_LIMIT_FID_LIMIT_CTRL_FID_LRN_CNT_LIMIT     VTSS_ENCODE_BITMASK(0,15)
#define  VTSS_X_ANA_L2_LRN_LIMIT_FID_LIMIT_CTRL_FID_LRN_CNT_LIMIT(x)  VTSS_EXTRACT_BITFIELD(x,0,15)

/**
 * Register Group: \a ANA_L2:PORT_LIMIT
 *
 * Learn limits per PORT and GLAG
 */


/** 
 * \brief Controls automatic learn limits
 *
 * \details
 * Per port configuration of autolearn limits
 *
 * Register: \a ANA_L2:PORT_LIMIT:PORT_LIMIT_STATUS
 *
 * @param gi Replicator: x_FFL_ANA_NUM_PORT_LIMITS (??), 0-84
 */
#define VTSS_ANA_L2_PORT_LIMIT_PORT_LIMIT_STATUS(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_L2,0x22800,gi,2,0,0)

/** 
 * \brief
 * Set if specified MAX learn cnt limit is exceeded and max learn cnt was
 * enabled. Write '1' to clear this field.
 *
 * \details 
 * 0: Learn cnt not exceeded.
 * 1: Learning operation has failed due to PORT max learn cnt exceeded.
 * Write '1' to clear this field.
 *
 * Field: ::VTSS_ANA_L2_PORT_LIMIT_PORT_LIMIT_STATUS . PORT_LRN_LIMIT_EXCEEDED_STICKY
 */
#define  VTSS_F_ANA_L2_PORT_LIMIT_PORT_LIMIT_STATUS_PORT_LRN_LIMIT_EXCEEDED_STICKY  VTSS_BIT(16)

/** 
 * \brief
 * Contains the number of MAC table entries currently learned associated
 * with a given logical PORT or GLAG.
 *
 * \details 
 * 0 : no entries
 *
 * Field: ::VTSS_ANA_L2_PORT_LIMIT_PORT_LIMIT_STATUS . PORT_LRN_CNT
 */
#define  VTSS_F_ANA_L2_PORT_LIMIT_PORT_LIMIT_STATUS_PORT_LRN_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_ANA_L2_PORT_LIMIT_PORT_LIMIT_STATUS_PORT_LRN_CNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_ANA_L2_PORT_LIMIT_PORT_LIMIT_STATUS_PORT_LRN_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Controls automatic learn limits per logical port or GLAG
 *
 * \details
 * Per Port configuration of MAC table learn limits
 *
 * Register: \a ANA_L2:PORT_LIMIT:PORT_LIMIT_CTRL
 *
 * @param gi Replicator: x_FFL_ANA_NUM_PORT_LIMITS (??), 0-84
 */
#define VTSS_ANA_L2_PORT_LIMIT_PORT_LIMIT_CTRL(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_L2,0x22800,gi,2,0,1)

/** 
 * \brief
 * Allow setting PORT_LIMIT_INTR when exceeding limit on learning (happens
 * when MAC address are supposed to be installed in the MAC table.
 *
 * \details 
 * 0: Disable 
 * 1: allow PORT_LIMIT_INTR to be set upon trying to learn a MAC address
 * that causes learn limit to be exeeded
 *
 * Field: ::VTSS_ANA_L2_PORT_LIMIT_PORT_LIMIT_CTRL . PORT_LIMIT_EXCEED_IRQ_ENA
 */
#define  VTSS_F_ANA_L2_PORT_LIMIT_PORT_LIMIT_CTRL_PORT_LIMIT_EXCEED_IRQ_ENA  VTSS_BIT(17)

/** 
 * \brief
 * Action for traffic when learn limit is exceeded.
 *
 * \details 
 * 00: Normal forward
 * 01: Enable redirection to CPU queue
 * 10: Enable copy to CPU queue
 * 11: Discard the frame
 *
 * Field: ::VTSS_ANA_L2_PORT_LIMIT_PORT_LIMIT_CTRL . PORT_LIMIT_EXCEED_SEL
 */
#define  VTSS_F_ANA_L2_PORT_LIMIT_PORT_LIMIT_CTRL_PORT_LIMIT_EXCEED_SEL(x)  VTSS_ENCODE_BITFIELD(x,15,2)
#define  VTSS_M_ANA_L2_PORT_LIMIT_PORT_LIMIT_CTRL_PORT_LIMIT_EXCEED_SEL     VTSS_ENCODE_BITMASK(15,2)
#define  VTSS_X_ANA_L2_PORT_LIMIT_PORT_LIMIT_CTRL_PORT_LIMIT_EXCEED_SEL(x)  VTSS_EXTRACT_BITFIELD(x,15,2)

/** 
 * \brief
 * Configures the number of MAC table entries that can be used for a given
 * PORT (via Automatic learning and CPU based learning with LOCK bit
 * cleared and not multicast).
 *
 * \details 
 * 0: Disable i.e. no learn limit for the PORT
 * 1: Only learning of one MAC address allowed for this logical port
 * ...
 * n: Learning of n MAC address allowed for this port
 *
 * Field: ::VTSS_ANA_L2_PORT_LIMIT_PORT_LIMIT_CTRL . PORT_LRN_CNT_LIMIT
 */
#define  VTSS_F_ANA_L2_PORT_LIMIT_PORT_LIMIT_CTRL_PORT_LRN_CNT_LIMIT(x)  VTSS_ENCODE_BITFIELD(x,0,15)
#define  VTSS_M_ANA_L2_PORT_LIMIT_PORT_LIMIT_CTRL_PORT_LRN_CNT_LIMIT     VTSS_ENCODE_BITMASK(0,15)
#define  VTSS_X_ANA_L2_PORT_LIMIT_PORT_LIMIT_CTRL_PORT_LRN_CNT_LIMIT(x)  VTSS_EXTRACT_BITFIELD(x,0,15)

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
 * @param gi Replicator: x_FFL_CHIP_NUM_SDX (??), 0-4095
 */
#define VTSS_ANA_L2_ISDX_PORT_MASK_CFG(gi)   VTSS_IOREG_IX(VTSS_TO_ANA_L2,0x0,gi,32,0,0)


/** 
 * \brief Ingress service port mask configuration
 *
 * \details
 * Configures port mask to be used in relation to service
 *
 * Register: \a ANA_L2:ISDX:PORT_MASK_CFG1
 *
 * @param gi Replicator: x_FFL_CHIP_NUM_SDX (??), 0-4095
 */
#define VTSS_ANA_L2_ISDX_PORT_MASK_CFG1(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_L2,0x0,gi,32,0,1)

/** 
 * \brief
 * Pls. refer to PORT_MASK_CFG.PORT_MASK description.
 *
 * \details 
 * 'XX...XXX': Where X is '0' or '1', representing a port mask.
 *
 * Field: ::VTSS_ANA_L2_ISDX_PORT_MASK_CFG1 . PORT_MASK1
 */
#define  VTSS_F_ANA_L2_ISDX_PORT_MASK_CFG1_PORT_MASK1(x)  VTSS_ENCODE_BITFIELD(x,0,21)
#define  VTSS_M_ANA_L2_ISDX_PORT_MASK_CFG1_PORT_MASK1     VTSS_ENCODE_BITMASK(0,21)
#define  VTSS_X_ANA_L2_ISDX_PORT_MASK_CFG1_PORT_MASK1(x)  VTSS_EXTRACT_BITFIELD(x,0,21)


/** 
 * \brief Ingress service forwarding configuration
 *
 * \details
 * Configures service based forwarding.
 *
 * Register: \a ANA_L2:ISDX:SERVICE_CTRL
 *
 * @param gi Replicator: x_FFL_CHIP_NUM_SDX (??), 0-4095
 */
#define VTSS_ANA_L2_ISDX_SERVICE_CTRL(gi)    VTSS_IOREG_IX(VTSS_TO_ANA_L2,0x0,gi,32,0,2)

/** 
 * \brief
 * Configures replacment of VLAN PORTMASK with ISDX::PORT_MASK_CFG
 *
 * \details 
 * Field: ::VTSS_ANA_L2_ISDX_SERVICE_CTRL . PORT_MASK_REPLACE_ENA
 */
#define  VTSS_F_ANA_L2_ISDX_SERVICE_CTRL_PORT_MASK_REPLACE_ENA  VTSS_BIT(25)

/** 
 * \brief
 * Aggregation code value.
 *
 * \details 
 * If AGGR_REPLACE_ENA is set then the aggregation code is replaced with
 * this value.
 * If AGGR_REPLACE_ENA is cleared and AGGR_VAL != 0 then no aggregation is
 * applied.
 *
 * Field: ::VTSS_ANA_L2_ISDX_SERVICE_CTRL . AGGR_VAL
 */
#define  VTSS_F_ANA_L2_ISDX_SERVICE_CTRL_AGGR_VAL(x)  VTSS_ENCODE_BITFIELD(x,21,4)
#define  VTSS_M_ANA_L2_ISDX_SERVICE_CTRL_AGGR_VAL     VTSS_ENCODE_BITMASK(21,4)
#define  VTSS_X_ANA_L2_ISDX_SERVICE_CTRL_AGGR_VAL(x)  VTSS_EXTRACT_BITFIELD(x,21,4)

/** 
 * \brief
 * Avoids applying source mask
 *
 * \details 
 * 0: Source port mask is applied.
 * 1: Source mask is ignored.
 *
 * Field: ::VTSS_ANA_L2_ISDX_SERVICE_CTRL . SRC_MASK_DIS
 */
#define  VTSS_F_ANA_L2_ISDX_SERVICE_CTRL_SRC_MASK_DIS  VTSS_BIT(20)

/** 
 * \brief
 * Overrule aggregation code from ANA_CL. 
 *
 * \details 
 * 0: Use aggregation code calculated in classifier. See AGGR_VAL for mode
 * to disable aggregation.
 * 1: Use SERVICE_CTRL.AGGR_VAL as aggregation code.
 *
 * Field: ::VTSS_ANA_L2_ISDX_SERVICE_CTRL . AGGR_REPLACE_ENA
 */
#define  VTSS_F_ANA_L2_ISDX_SERVICE_CTRL_AGGR_REPLACE_ENA  VTSS_BIT(19)

/** 
 * \brief
 * Select ES0 key type.
 *
 * \details 
 * 0: Use VID key in ES0.
 * 1: Use ISDX key in ES0.
 *
 * Field: ::VTSS_ANA_L2_ISDX_SERVICE_CTRL . ES0_ISDX_KEY_ENA
 */
#define  VTSS_F_ANA_L2_ISDX_SERVICE_CTRL_ES0_ISDX_KEY_ENA  VTSS_BIT(18)

/** 
 * \brief
 * Enable forwarding based on CDA
 *
 * \details 
 * Field: ::VTSS_ANA_L2_ISDX_SERVICE_CTRL . CDA_FWD_ENA
 */
#define  VTSS_F_ANA_L2_ISDX_SERVICE_CTRL_CDA_FWD_ENA  VTSS_BIT(17)

/** 
 * \brief
 * Enable service based forwarding
 *
 * \details 
 * Field: ::VTSS_ANA_L2_ISDX_SERVICE_CTRL . ISDX_BASED_FWD_ENA
 */
#define  VTSS_F_ANA_L2_ISDX_SERVICE_CTRL_ISDX_BASED_FWD_ENA  VTSS_BIT(16)

/** 
 * \brief
 * Enable service based learning
 *
 * \details 
 * Field: ::VTSS_ANA_L2_ISDX_SERVICE_CTRL . ISDX_BASED_SRC_ENA
 */
#define  VTSS_F_ANA_L2_ISDX_SERVICE_CTRL_ISDX_BASED_SRC_ENA  VTSS_BIT(15)

/** 
 * \brief
 * Forwarding address type if enabled via ISDX_BASED_FWD_ENA
 *
 * \details 
 * 0: UPSID_PN 
 * 3: MC_IDX
 * 7: NO_ADDR
 * other: Reserved
 *
 * Field: ::VTSS_ANA_L2_ISDX_SERVICE_CTRL . FWD_TYPE
 */
#define  VTSS_F_ANA_L2_ISDX_SERVICE_CTRL_FWD_TYPE(x)  VTSS_ENCODE_BITFIELD(x,12,3)
#define  VTSS_M_ANA_L2_ISDX_SERVICE_CTRL_FWD_TYPE     VTSS_ENCODE_BITMASK(12,3)
#define  VTSS_X_ANA_L2_ISDX_SERVICE_CTRL_FWD_TYPE(x)  VTSS_EXTRACT_BITFIELD(x,12,3)

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
 * Field: ::VTSS_ANA_L2_ISDX_SERVICE_CTRL . FWD_ADDR
 */
#define  VTSS_F_ANA_L2_ISDX_SERVICE_CTRL_FWD_ADDR(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_ANA_L2_ISDX_SERVICE_CTRL_FWD_ADDR     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_ANA_L2_ISDX_SERVICE_CTRL_FWD_ADDR(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \brief Ingress service dual leaky bucket policer, base address
 *
 * \details
 * Specifies service DLB policer index.
 * 
 * If ANA_L2::FWD_CFG.ISDX_DEFAULT_DLB_ENA is set some of the indeces are
 * used for port default DLB (see ANA_L2::PORT_DLB_CFG.DLB_IDX).
 * 
 * The ingress service DLB policer is selected as DLB_IDX +
 * DLB_COS_OFFSET[frame's classified  COS ID].
 *
 * Register: \a ANA_L2:ISDX:DLB_CFG
 *
 * @param gi Replicator: x_FFL_CHIP_NUM_SDX (??), 0-4095
 */
#define VTSS_ANA_L2_ISDX_DLB_CFG(gi)         VTSS_IOREG_IX(VTSS_TO_ANA_L2,0x0,gi,32,0,3)

/** 
 * \brief
 * Dual leaky bucket base address.
 *
 * \details 
 * Field: ::VTSS_ANA_L2_ISDX_DLB_CFG . DLB_IDX
 */
#define  VTSS_F_ANA_L2_ISDX_DLB_CFG_DLB_IDX(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_ANA_L2_ISDX_DLB_CFG_DLB_IDX     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_ANA_L2_ISDX_DLB_CFG_DLB_IDX(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \brief Ingress service dual leaky bucket policer, offset per COS ID
 *
 * \details
 * Register: \a ANA_L2:ISDX:DLB_COS_CFG
 *
 * @param gi Replicator: x_FFL_CHIP_NUM_SDX (??), 0-4095
 * @param ri Register: DLB_COS_CFG (??), 0-7
 */
#define VTSS_ANA_L2_ISDX_DLB_COS_CFG(gi,ri)  VTSS_IOREG_IX(VTSS_TO_ANA_L2,0x0,gi,32,ri,4)

/** 
 * \brief
 * Dual leaky bucket offset per COS ID.
 *
 * \details 
 * Field: ::VTSS_ANA_L2_ISDX_DLB_COS_CFG . DLB_COS_OFFSET
 */
#define  VTSS_F_ANA_L2_ISDX_DLB_COS_CFG_DLB_COS_OFFSET(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_ANA_L2_ISDX_DLB_COS_CFG_DLB_COS_OFFSET     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_ANA_L2_ISDX_DLB_COS_CFG_DLB_COS_OFFSET(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Ingress service counter set, base address
 *
 * \details
 * The ingress service counter set is selected as ISDX_BASE_ADDR +
 * ISDX_COS_OFFSET[frame's classified  COS ID].
 *
 * Register: \a ANA_L2:ISDX:ISDX_BASE_CFG
 *
 * @param gi Replicator: x_FFL_CHIP_NUM_SDX (??), 0-4095
 */
#define VTSS_ANA_L2_ISDX_ISDX_BASE_CFG(gi)   VTSS_IOREG_IX(VTSS_TO_ANA_L2,0x0,gi,32,0,12)

/** 
 * \brief
 * Ingress service counter set base address.
 *
 * \details 
 * Field: ::VTSS_ANA_L2_ISDX_ISDX_BASE_CFG . ISDX_BASE_ADDR
 */
#define  VTSS_F_ANA_L2_ISDX_ISDX_BASE_CFG_ISDX_BASE_ADDR(x)  VTSS_ENCODE_BITFIELD(x,0,13)
#define  VTSS_M_ANA_L2_ISDX_ISDX_BASE_CFG_ISDX_BASE_ADDR     VTSS_ENCODE_BITMASK(0,13)
#define  VTSS_X_ANA_L2_ISDX_ISDX_BASE_CFG_ISDX_BASE_ADDR(x)  VTSS_EXTRACT_BITFIELD(x,0,13)


/** 
 * \brief Ingress service counter set, offset per COS ID
 *
 * \details
 * Register: \a ANA_L2:ISDX:ISDX_COS_CFG
 *
 * @param gi Replicator: x_FFL_CHIP_NUM_SDX (??), 0-4095
 * @param ri Register: ISDX_COS_CFG (??), 0-7
 */
#define VTSS_ANA_L2_ISDX_ISDX_COS_CFG(gi,ri)  VTSS_IOREG_IX(VTSS_TO_ANA_L2,0x0,gi,32,ri,13)

/** 
 * \brief
 * Ingress service counter set offset per COS ID.
 *
 * \details 
 * Field: ::VTSS_ANA_L2_ISDX_ISDX_COS_CFG . ISDX_COS_OFFSET
 */
#define  VTSS_F_ANA_L2_ISDX_ISDX_COS_CFG_ISDX_COS_OFFSET(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_ANA_L2_ISDX_ISDX_COS_CFG_ISDX_COS_OFFSET     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_ANA_L2_ISDX_ISDX_COS_CFG_ISDX_COS_OFFSET(x)  VTSS_EXTRACT_BITFIELD(x,0,3)

/**
 * Register Group: \a ANA_L2:STICKY
 *
 * Sticky diagnostic status
 */


/** 
 * \brief Sticky diagnostic status
 *
 * \details
 * Register: \a ANA_L2:STICKY:STICKY
 */
#define VTSS_ANA_L2_STICKY_STICKY            VTSS_IOREG(VTSS_TO_ANA_L2,0x22903)

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
#define  VTSS_F_ANA_L2_STICKY_STICKY_AUTO_LRN_RATE_EXCEED_STICKY  VTSS_BIT(15)

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
#define  VTSS_F_ANA_L2_STICKY_STICKY_FWD_FLOOD_KILL_STICKY  VTSS_BIT(14)

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
#define  VTSS_F_ANA_L2_STICKY_STICKY_VLAN_IGNORE_STICKY  VTSS_BIT(13)

/** 
 * \brief
 * Set if forwarding is performed without SRC contribution either because
 * of a known Destination entry with VLAN ignore set
 * (LRN.MAC_ACCESS_CFG_2.MAC_ENTRY_VLAN_IGNORE FLAG) set or for an unknown
 * destination with flood VLAN ignore set
 * (ANA_L2::FWD_CFG.FLOOD_IGNORE_VLAN_ENA) and filter_mode_sel set to SRC
 * ignore.
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
 * Field: ::VTSS_ANA_L2_STICKY_STICKY . SRC_IGNORE_STICKY
 */
#define  VTSS_F_ANA_L2_STICKY_STICKY_SRC_IGNORE_STICKY  VTSS_BIT(12)

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
 * \brief Sticky diagnostic counter mask
 *
 * \details
 * Register: \a ANA_L2:STICKY_MASK:STICKY_MASK
 *
 * @param gi Replicator: x_FFL_ANA_NUM_CONCURRENT_CNTS (??), 0-3
 */
#define VTSS_ANA_L2_STICKY_MASK_STICKY_MASK(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_L2,0x22904,gi,1,0,0)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . AUTO_LRN_RATE_EXCEED_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_AUTO_LRN_RATE_EXCEED_STICKY_MASK  VTSS_BIT(15)

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
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_FWD_FLOOD_KILL_STICKY_MASK  VTSS_BIT(14)

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
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_VLAN_IGNORE_STICKY_MASK  VTSS_BIT(13)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . SRC_IGNORE_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_SRC_IGNORE_STICKY_MASK  VTSS_BIT(12)

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
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . FWD_FLOOD_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_FWD_FLOOD_STICKY_MASK  VTSS_BIT(4)

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
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_FWD_ENTRY_FOUND_STICKY_MASK  VTSS_BIT(3)

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
 * Field: ::VTSS_ANA_L2_STICKY_MASK_STICKY_MASK . LEARN_KNOWN_STICKY_MASK
 */
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_LEARN_KNOWN_STICKY_MASK  VTSS_BIT(1)

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
#define  VTSS_F_ANA_L2_STICKY_MASK_STICKY_MASK_LEARN_UNKNOWN_STICKY_MASK  VTSS_BIT(0)


#endif /* _VTSS_JAGUAR2_REGS_ANA_L2_H_ */
