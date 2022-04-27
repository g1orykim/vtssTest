#ifndef _VTSS_JAGUAR_REGS_LRN_H_
#define _VTSS_JAGUAR_REGS_LRN_H_

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

#include "vtss_jaguar_regs_common.h"

/*********************************************************************** 
 *
 * Target: \a LRN
 *
 * Learn block
 *
 ***********************************************************************/

/**
 * Register Group: \a LRN:COMMON
 *
 * Common configurations and status for all ports
 */


/** 
 * \brief MAC Table Control
 *
 * \details
 * Configures command and access parameters when accessing the MAC table.
 *
 * Register: \a LRN:COMMON:COMMON_ACCESS_CTRL
 */
#define VTSS_LRN_COMMON_COMMON_ACCESS_CTRL   VTSS_IOREG(VTSS_TO_LRN,0x0)

/** 
 * \brief
 * Configures the MAC table row index that holds the MAC_ENTRY that is
 * accessed when CPU_ACCESS_CMD command is READ or WRITE.
 * 
 * Configures the MAC table starting row index when CPU_ACCESS_CMD command
 * is FIND NEXT. If the Command succeeds in finding a matching entry this
 * field is updated with the row index for the (last) matching entry. 
 *
 * \details 
 * 0: Row 0
 * 1: Row 1
 * ...
 * n: Row n.
 *
 * Field: ::VTSS_LRN_COMMON_COMMON_ACCESS_CTRL . CPU_ACCESS_DIRECT_ROW
 */
#define  VTSS_F_LRN_COMMON_COMMON_ACCESS_CTRL_CPU_ACCESS_DIRECT_ROW(x)  VTSS_ENCODE_BITFIELD(x,14,14)
#define  VTSS_M_LRN_COMMON_COMMON_ACCESS_CTRL_CPU_ACCESS_DIRECT_ROW     VTSS_ENCODE_BITMASK(14,14)
#define  VTSS_X_LRN_COMMON_COMMON_ACCESS_CTRL_CPU_ACCESS_DIRECT_ROW(x)  VTSS_EXTRACT_BITFIELD(x,14,14)

/** 
 * \brief
 * Configures the MAC table bucket or column index that holds the MAC_ENTRY
 * that is accessed when CPU_ACCESS_CMD command is READ or WRITE.

 *
 * \details 
 * 0: Column (bucket) 0
 * 1: Column (bucket) 1
 * ...
 * n: Column (bucket) n.
 *
 * Field: ::VTSS_LRN_COMMON_COMMON_ACCESS_CTRL . CPU_ACCESS_DIRECT_COL
 */
#define  VTSS_F_LRN_COMMON_COMMON_ACCESS_CTRL_CPU_ACCESS_DIRECT_COL(x)  VTSS_ENCODE_BITFIELD(x,12,2)
#define  VTSS_M_LRN_COMMON_COMMON_ACCESS_CTRL_CPU_ACCESS_DIRECT_COL     VTSS_ENCODE_BITMASK(12,2)
#define  VTSS_X_LRN_COMMON_COMMON_ACCESS_CTRL_CPU_ACCESS_DIRECT_COL(x)  VTSS_EXTRACT_BITFIELD(x,12,2)

/** 
 * \brief
 * Configures which MAC table that is used when CPU_ACCESS_CMD command is
 * LEARN, UNLEARN, or LOOKUP.
 *
 * \details 
 * 0: Access CMAC table
 * 1: Access BMAC table
 *
 * Field: ::VTSS_LRN_COMMON_COMMON_ACCESS_CTRL . CPU_ACCESS_BMAC_TABLE
 */
#define  VTSS_F_LRN_COMMON_COMMON_ACCESS_CTRL_CPU_ACCESS_BMAC_TABLE  VTSS_BIT(8)

/** 
 * \brief
 * Configures the MAC table access command:
 * 
 * LEARN, UNLEARN, and LOOKUP Command accesses a MAC_ENTRY at the
 * HASH(VID/FID,MAC) index. MAC and FID/VID must to be written to the
 * respective bitgroups in registers MAC_ACCESS_CFG_[0|1|2].
 * 
 * READ and WRITE Command accesses a MAC_ENTRY at the index specified via
 * CPU_ACCESS_DIRECT_ROW,  CPU_ACCESS_DIRECT_COL and CPU_ACCESS_DIRECT_TYPE
 * fields.
 * 
 * SCAN Command access traverses the entire MAC table and handle all found
 * entries depending on configuration and filter settings. If any set of
 * filters is enabled then only entries matching the enabled filters are
 * affected / found. Refer to description of bitgroups found in
 * LRN::SCAN_NEXT_CFG-	Please note that it is possible to configure the
 * scan to FIND ALL or FIND first via
 * LRN::SCAN_NEXT_CFG.SCAN_NEXT_UNTIL_FOUND_ENA.
 * 
 * FIND SMALLEST Command traverses the entire table and returns the
 * MAC_ENTRY with the smallest FID/VID,MAC which is larger than a
 * configured starting FID/VID,MAC value (configured in
 * MAC_ACCESS_CFG_[0|1|2].). FID/VID take precedence over MAC.
 * 
 * CLEAR ALL Command traverses the entire table and resets all entries.
 *
 * \details 
 * 0: LEARN
 * 1: UNLEARN
 * 2: LOOKUP
 * 3: READ
 * 4: WRITE
 * 5: SCAN (AGE or FIND NEXT)
 * 6: FIND SMALLEST
 * 7: CLEAR ALL
 *
 * Field: ::VTSS_LRN_COMMON_COMMON_ACCESS_CTRL . CPU_ACCESS_CMD
 */
#define  VTSS_F_LRN_COMMON_COMMON_ACCESS_CTRL_CPU_ACCESS_CMD(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_LRN_COMMON_COMMON_ACCESS_CTRL_CPU_ACCESS_CMD     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_LRN_COMMON_COMMON_ACCESS_CTRL_CPU_ACCESS_CMD(x)  VTSS_EXTRACT_BITFIELD(x,4,4)

/** 
 * \brief
 * Perform the access command specified by CPU_ACCESS_CMD. Cleared when the
 * access completes.
 *
 * \details 
 * 0: Idle
 * 1: Initiate CPU ACCESS
 * 
 * The bit is cleared upon completion
 *
 * Field: ::VTSS_LRN_COMMON_COMMON_ACCESS_CTRL . MAC_TABLE_ACCESS_SHOT
 */
#define  VTSS_F_LRN_COMMON_COMMON_ACCESS_CTRL_MAC_TABLE_ACCESS_SHOT  VTSS_BIT(0)


/** 
 * \brief MAC entry Data 0
 *
 * \details
 * Configures MAC address and FID/VID when accessing the MAC table.
 *
 * Register: \a LRN:COMMON:MAC_ACCESS_CFG_0
 */
#define VTSS_LRN_COMMON_MAC_ACCESS_CFG_0     VTSS_IOREG(VTSS_TO_LRN,0x1)

/** 
 * \brief
 * Configures FID/VID for the MAC_ENTRY. For C-domain entries this values
 * is a FID for unicast entries and CL-VID for multicast entries.  For
 * B-Domain entires this field is always used to specify BVID.
 * 
 * Can also be used as FID/VID filter value during SCAN or FIND SMALLEST
 * Command.
 *
 * \details 
 * Field: ::VTSS_LRN_COMMON_MAC_ACCESS_CFG_0 . MAC_ENTRY_FID
 */
#define  VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_0_MAC_ENTRY_FID(x)  VTSS_ENCODE_BITFIELD(x,16,12)
#define  VTSS_M_LRN_COMMON_MAC_ACCESS_CFG_0_MAC_ENTRY_FID     VTSS_ENCODE_BITMASK(16,12)
#define  VTSS_X_LRN_COMMON_MAC_ACCESS_CFG_0_MAC_ENTRY_FID(x)  VTSS_EXTRACT_BITFIELD(x,16,12)

/** 
 * \brief
 * Configures 16 MSB bits of the MAC address for the MAC_ENTRY.
 * 
 * Used as starting MAC value during FIND SMALLEST Command.
 *
 * \details 
 * Upper 16 bits of MAC address.
 *
 * Field: ::VTSS_LRN_COMMON_MAC_ACCESS_CFG_0 . MAC_ENTRY_MAC_MSB
 */
#define  VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_0_MAC_ENTRY_MAC_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_LRN_COMMON_MAC_ACCESS_CFG_0_MAC_ENTRY_MAC_MSB     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_LRN_COMMON_MAC_ACCESS_CFG_0_MAC_ENTRY_MAC_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief MAC entry Data 1
 *
 * \details
 * Configures MAC address when accessing the MAC table.
 *
 * Register: \a LRN:COMMON:MAC_ACCESS_CFG_1
 */
#define VTSS_LRN_COMMON_MAC_ACCESS_CFG_1     VTSS_IOREG(VTSS_TO_LRN,0x2)


/** 
 * \brief MAC entry Data 2
 *
 * \details
 * Configures warious MAC_ENTRY parameters when accessing the MAC table
 *
 * Register: \a LRN:COMMON:MAC_ACCESS_CFG_2
 */
#define VTSS_LRN_COMMON_MAC_ACCESS_CFG_2     VTSS_IOREG(VTSS_TO_LRN,0x3)

/** 
 * \brief
 * Configures the ADDRESS for the MAC_ENTRY. Encoding of this field is
 * determined by MAC_ENTRY_ADDR_TYPE.
 * 
 * Can be used as filter value during SCAN or FIND SMALLEST Command.
 * 
 * If MAC_ENTRY_TYPE=1 or MAC_ENTRY_TYPE=2 this field is ignored.

 *
 * \details 
 * MAC_ENTRY_ADDR_TYPE= UPSID_PN:
 *   MAC_ENTRY_ADDR(9:5) = UPSID
 *   MAC_ENTRY_ADDR(4:0) = UPSPN
 *   Specifies unicast forwarding to port: UPSPN in device: UPSID.
 * 
 * MAC_ENTRY_ADDR_TYPE = UPSID_CPU_OR_INT:
 *   MAC_ENTRY_ADDR(9:5) = UPSID
 *   MAC_ENTRY_ADDR(3:0) = CPU queue number when MAC_ENTRY_ADDR(11) = 0
 *   MAC_ENTRY_ADDR(3:0) = Internal port number when MAC_ENTRY_ADDR(11) = 1
 *	  0xe: Internal port number (intpn_router)
 *	  0xf: Local lookup at destination upsid (ntpn_dlookup)
 * 
 * MAC_ENTRY_ADDR_TYPE = GLAG
 *   MAC_ENTRY_ADDR = GLAGID 
 *   Specifies unicast forwarding to the global aggregated port group:
 * GLAGID.
 * 
 * MAC_ENTRY_ADDR_TYPE = MC_IDX:
 *   MAC_ENTRY_ADDR = MC_IDX
 *   Specifies forwarding according to ANA_AC::PGID:[32 + MC_IDX)
 * 
 * MAC_ENTRY_ADDR_TYPE = CONID_FWD:
 *   MAC_ENTRY_ADDR = CONID (connection id)
 *   Specifies MACinMAC forwarding via the BMAC address located at the BMAC
 * table index: CONID	(only used for C-domain entries)
 * 
 * MAC_ENTRY_ADDR_TYPE = PATHGRP_FWD:
 *   MAC_ENTRY_ADDR = PATHGRP (path group index)
 *   Specifies MACinMAC forwarding via a one of two protected CONIDs
 * configured in the 
 *   PATHGRP table.   A lookup in ANA_L2::PATHGRP:[PATHGRP] returns two
 * CONIDs together 
 *   with additional control signals that selects which of the two CONID to
 * use as BMAC table index.
 *   (only used for C-domain entries)
 *
 * Field: ::VTSS_LRN_COMMON_MAC_ACCESS_CFG_2 . MAC_ENTRY_ADDR
 */
#define  VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_ADDR(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_ADDR     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_ADDR(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Configures MAC_ENTRY dual domain forward.
 * 
 * For B-domain entries this is used to specify whether forwarding to via
 * Customer Backbone Ports (CBP) to C-domain is possible or not. Only used
 * for traffic received from a B-port.
 * 
 * For C-domain entries this is used to specify whether forwarding to both
 * C-domain and B-domain ports are possible or not. Used to allow multicast
 * forwarding to C-domain entries via the MAC_ENTRY forwarding and B-domain
 * forwarding according to ANA_L2::ISDX:FWD_TYPE. Only used for traffic
 * received from a C-port.

 *
 * \details 
 * 0: No dual domain forwarding allowed
 * 1: Dual domain forwarding allowed
 *
 * Field: ::VTSS_LRN_COMMON_MAC_ACCESS_CFG_2 . MAC_ENTRY_DUAL_DOMAIN_FWD
 */
#define  VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_DUAL_DOMAIN_FWD  VTSS_BIT(15)

/** 
 * \brief
 * Configures MAC_ENTRY address type. 
 * 
 * Can be used as filter value during SCAN or FIND SMALLEST Command.
 * 
 * If MAC_ENTRY_TYPE=1 or MAC_ENTRY_TYPE=2 this field is ignored.
 *
 * \details 
 * 0: UPSID_PN 
 * 1: UPSID_CPU_OR_INT
 * 2: GLAG
 * 3: MC_IDX
 * 4: CONID_FWD
 * 5: PATHGRP_FWD
 *
 * Field: ::VTSS_LRN_COMMON_MAC_ACCESS_CFG_2 . MAC_ENTRY_ADDR_TYPE
 */
#define  VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_ADDR_TYPE(x)  VTSS_ENCODE_BITFIELD(x,12,3)
#define  VTSS_M_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_ADDR_TYPE     VTSS_ENCODE_BITMASK(12,3)
#define  VTSS_X_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_ADDR_TYPE(x)  VTSS_EXTRACT_BITFIELD(x,12,3)

/** 
 * \brief
 * Configures MAC_ENTRY type. Controls whether front port destination mask
 * is embedded in MAC_ENTRY or forwarding is based on ADDR_TYPE.
 * 
 * IPv4 multicast entries will always assume upper 25 MSB bit is
 * 01-00-5E-0X-XX-XX
 * IPv6 multicast entries will always assume upper 16 MSB bit is
 * 33-33-XX-XX-XX-XX.
 *
 * \details 
 * 0: STD entry
 * 1: IPv4 MC entry with front port destination mask in MSB of MAC address
 * 2: IPv6 MC entry with front port destination mask in MSB of MAC address
 * 3: PATH_CCM_MEPID available in MAC_ENTRY_MAC_LSB(15:0) (only valid when
 * installed at an odd bucket position)
 *
 * Field: ::VTSS_LRN_COMMON_MAC_ACCESS_CFG_2 . MAC_ENTRY_TYPE
 */
#define  VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_TYPE(x)  VTSS_ENCODE_BITFIELD(x,10,2)
#define  VTSS_M_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_TYPE     VTSS_ENCODE_BITMASK(10,2)
#define  VTSS_X_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_TYPE(x)  VTSS_EXTRACT_BITFIELD(x,10,2)

/** 
 * \brief
 * Configures MAC_ENTRY SRC_KILL_FWD flag. Used to discard forwarding of
 * frames from this source.
 *
 * \details 
 * 0: Disable
 * 1: Enable (Frames with this entry MAC address as SMAC are killed)
 *
 * Field: ::VTSS_LRN_COMMON_MAC_ACCESS_CFG_2 . MAC_ENTRY_SRC_KILL_FWD
 */
#define  VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_SRC_KILL_FWD  VTSS_BIT(8)

/** 
 * \brief
 * Configures MAC_ENTRY NXT_LRN_ALL flag. 
 * 
 * For B-domain entries this allows extended MAC table ageing period. see
 * AUTOAGE_INC_AGE_AND_NXT_LRN_ALL_ENA. 
 * 
 * For C-domain entries this is used to ensure MAC entries are learned
 * consistend in a multi device configuration.
 * 
 * Cleared by HW when a frame is received for a (IFID, SMAC) matching this
 * entry.
 * 
 * Only available when MAC_ENTRY_CCM_TYPE_INTERVAL =0.
 *
 * \details 
 * 0:  Disabled 
 * 1:  Next learn all.
 *
 * Field: ::VTSS_LRN_COMMON_MAC_ACCESS_CFG_2 . MAC_ENTRY_NXT_LRN_ALL
 */
#define  VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_NXT_LRN_ALL  VTSS_BIT(7)

/** 
 * \brief
 * Configures MAC_ENTRY CPU copy flag. 
 * 
 * Used together with ANA_L2::FWD_CFG.CPU_DMAC_COPY_ENA to determine if a
 * CPU copy should be send when the entry found as forwarding entry.
 * 
 * Used together with ANA_L2::LRN_CFG.CPU_SMAC_COPY_ENA to determine if a
 * CPU copy should be send when the entry found as source entry.

 *
 * \details 
 * Field: ::VTSS_LRN_COMMON_MAC_ACCESS_CFG_2 . MAC_ENTRY_CPU_COPY
 */
#define  VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_CPU_COPY  VTSS_BIT(6)

/** 
 * \brief
 * Configures MAC_ENTRY VLAN ignore flag.  
 * 
 * Can be used to ignore VLAN mask or Source mask during forwarding based
 * on ANA_L2::FWD_CFG.FILTER_MODE_SEL. 
 *
 * \details 
 * 0: no effect
 * 1: VLAN mask or source mask is ignored.
 *
 * Field: ::VTSS_LRN_COMMON_MAC_ACCESS_CFG_2 . MAC_ENTRY_VLAN_IGNORE
 */
#define  VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_VLAN_IGNORE  VTSS_BIT(5)

/** 
 * \brief
 * MAC_ENTRY AGE flag used to identify if a MAC_ENTRY is active of not.
 * 
 * Cleared by HW when a entry is used as source entry. Used to ensure that
 * entires associated with inactive stations can be located and discarded. 
 * 
 * The MAC table can be aged by HW either a CPU Scan command or via
 * Automatic ageing (LRN::AUTOAGE_CFG). 
 * 
 * Only available when MAC_ENTRY_CCM_TYPE_INTERVAL =0.
 *
 * \details 
 * 0:  Not aged 
 * n:  Aged n times.
 *
 * Field: ::VTSS_LRN_COMMON_MAC_ACCESS_CFG_2 . MAC_ENTRY_AGE_FLAG
 */
#define  VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_AGE_FLAG(x)  VTSS_ENCODE_BITFIELD(x,3,2)
#define  VTSS_M_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_AGE_FLAG     VTSS_ENCODE_BITMASK(3,2)
#define  VTSS_X_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_AGE_FLAG(x)  VTSS_EXTRACT_BITFIELD(x,3,2)

/** 
 * \brief
 * Configures the MAC_ENTRY mirror flag. 
 * 
 * Used to configure MAC entries that are candidates for mirroring. See:
 * ANA_AC::MIRROR_PROBE.PROBE_MAC_MODE.
 *
 * \details 
 * Field: ::VTSS_LRN_COMMON_MAC_ACCESS_CFG_2 . MAC_ENTRY_MIRROR
 */
#define  VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_MIRROR  VTSS_BIT(2)

/** 
 * \brief
 * Configures the MAC_ENTRY locked flag. Used to install static MAC table
 * entries that can not be removed by learning and ageing.
 *
 * \details 
 * 0: Not locked
 * 1: Locked.
 *
 * Field: ::VTSS_LRN_COMMON_MAC_ACCESS_CFG_2 . MAC_ENTRY_LOCKED
 */
#define  VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_LOCKED  VTSS_BIT(1)

/** 
 * \brief
 * Configures the MAC_ENTRY valid flag. Used to indicate that this entry is
 * active. This bit can be cleared for non locked entries by auto learning
 * or via the CPU_ACCESS_CMD UNLEARN command.
 *
 * \details 
 * 0: Invalid
 * 1: Valid.
 *
 * Field: ::VTSS_LRN_COMMON_MAC_ACCESS_CFG_2 . MAC_ENTRY_VLD
 */
#define  VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_2_MAC_ENTRY_VLD  VTSS_BIT(0)


/** 
 * \brief MAC entry Data 3
 *
 * \details
 * Configures MAC entry destination used when accessing the MAC table
 * multicast entries.
 *
 * Register: \a LRN:COMMON:MAC_ACCESS_CFG_3
 */
#define VTSS_LRN_COMMON_MAC_ACCESS_CFG_3     VTSS_IOREG(VTSS_TO_LRN,0x4)


/** 
 * \brief MAC entry Data 4
 *
 * \details
 * Configures OAM CCM flag and Tunnel index when accessing the MAC table.
 *
 * Register: \a LRN:COMMON:MAC_ACCESS_CFG_4
 */
#define VTSS_LRN_COMMON_MAC_ACCESS_CFG_4     VTSS_IOREG(VTSS_TO_LRN,0x5)

/** 
 * \brief
 * Configures the MAC_ENTRY tunnel policer index.
 *
 * \details 
 * Field: ::VTSS_LRN_COMMON_MAC_ACCESS_CFG_4 . MAC_ENTRY_TDLB_IDX
 */
#define  VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_4_MAC_ENTRY_TDLB_IDX(x)  VTSS_ENCODE_BITFIELD(x,8,9)
#define  VTSS_M_LRN_COMMON_MAC_ACCESS_CFG_4_MAC_ENTRY_TDLB_IDX     VTSS_ENCODE_BITMASK(8,9)
#define  VTSS_X_LRN_COMMON_MAC_ACCESS_CFG_4_MAC_ENTRY_TDLB_IDX(x)  VTSS_EXTRACT_BITFIELD(x,8,9)

/** 
 * \brief
 * MAC_ENTRY CCM changed flag used by HW to identify whether Loss of
 * Continuity (LOC) has been declared from the host identified via this
 * MAC_ENTRY.
 * 
 * LOC state is cleared by HW when receiving a CCM message from the source
 * associated with this MAC_ENTRY. A CCM message is identified via VCAP_S2
 * with an associated action: CCM_TYPE. 
 * 
 * The MAC table can be configured to periodically increment CCM miss count
 * and decleare LOC via a HW configured by LRN::CCM_CTRL.  
 * 
 * This flag can also be used for "CPU hit me once operation" where the
 * next CCM message is send to CPU. See ANA_L2::CCM_CFG.CCM_CHG_COPY_ENA.
 * 
 * Only available when MAC_ENTRY_CCM_TYPE_INTERVAL != 0.
 *
 * \details 
 * 0: No LOC has been decleared.
 * 1: LOC has been decleared
 *
 * Field: ::VTSS_LRN_COMMON_MAC_ACCESS_CFG_4 . MAC_ENTRY_CCM_CHG
 */
#define  VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_4_MAC_ENTRY_CCM_CHG  VTSS_BIT(4)

/** 
 * \brief
 * MAC_ENTRY CCM miss count used by HW to identify the number of CCM
 * periods has passed without CCM mesages received from the host identified
 * via this MAC_ENTRY.
 * 
 * Cleared by HW when receiving a CCM message from the source associated
 * with this MAC_ENTRY. A CCM message is identified via VCAP_S2 with an
 * associated action: CCM_TYPE. 
 * 
 * The MAC table can be configured to periodically increment CCM miss count
 * via a HW configured by LRN::CCM_CTRL.  
 * 
 * Only available when MAC_ENTRY_CCM_TYPE_INTERVAL != 0.
 *
 * \details 
 * Field: ::VTSS_LRN_COMMON_MAC_ACCESS_CFG_4 . MAC_ENTRY_CCM_MISS_CNT
 */
#define  VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_4_MAC_ENTRY_CCM_MISS_CNT(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_LRN_COMMON_MAC_ACCESS_CFG_4_MAC_ENTRY_CCM_MISS_CNT     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_LRN_COMMON_MAC_ACCESS_CFG_4_MAC_ENTRY_CCM_MISS_CNT(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Configures the MAC_ENTRY CCM interval (only possible if entry is locked
 * via MAC_ENTRY_LOCKED).
 *
 * \details 
 * 0: No ccm
 * 1: ccm fault management
 * 2: performance management
 * 3: protection switching
 *
 * Field: ::VTSS_LRN_COMMON_MAC_ACCESS_CFG_4 . MAC_ENTRY_CCM_TYPE_INTERVAL
 */
#define  VTSS_F_LRN_COMMON_MAC_ACCESS_CFG_4_MAC_ENTRY_CCM_TYPE_INTERVAL(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_LRN_COMMON_MAC_ACCESS_CFG_4_MAC_ENTRY_CCM_TYPE_INTERVAL     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_LRN_COMMON_MAC_ACCESS_CFG_4_MAC_ENTRY_CCM_TYPE_INTERVAL(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Scan parameters
 *
 * \details
 * Configures various filters and associated actions when scanning the MAC
 * table via CPU Scan Command. 
 *
 * Register: \a LRN:COMMON:SCAN_NEXT_CFG
 */
#define VTSS_LRN_COMMON_SCAN_NEXT_CFG        VTSS_IOREG(VTSS_TO_LRN,0x6)

/** 
 * \brief
 * Controls update of MAC_ENTRY_CCM_MISS_CNT (or MAC_ENTRY_AGE_FLAG when
 * MAC_ENTRY_CCM_TYPE_INTERVAL equals 0) for found entries.
 * 
 * Field is ignored if SCAN_NEXT_INC_AGE_BITS_ENA is set
 *
 * \details 
 * 0: No change to MAC_ENTRY_CCM_MISS_CNT (or MAC_ENTRY_AGE_FLAG)
 * 1: Clear MAC_ENTRY_CCM_MISS_CNT (or MAC_ENTRY_AGE_FLAG)
 * 2: Set MAC_ENTRY_CCM_MISS_CNT (or MAC_ENTRY_AGE_FLAG)
 * 3: Incement MAC_ENTRY_CCM_MISS_CNT (or MAC_ENTRY_AGE_FLAG)
 * 4: Decrement MAC_ENTRY_CCM_MISS_CNT (or MAC_ENTRY_AGE_FLAG). 
 *
 * Field: ::VTSS_LRN_COMMON_SCAN_NEXT_CFG . SCAN_CCM_MISS_CNT_UPDATE_SEL
 */
#define  VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_SCAN_CCM_MISS_CNT_UPDATE_SEL(x)  VTSS_ENCODE_BITFIELD(x,28,3)
#define  VTSS_M_LRN_COMMON_SCAN_NEXT_CFG_SCAN_CCM_MISS_CNT_UPDATE_SEL     VTSS_ENCODE_BITMASK(28,3)
#define  VTSS_X_LRN_COMMON_SCAN_NEXT_CFG_SCAN_CCM_MISS_CNT_UPDATE_SEL(x)  VTSS_EXTRACT_BITFIELD(x,28,3)

/** 
 * \brief
 * Controls update of MAC_ENTRY_CCM_CHG (or MAC_ENTRY_NXT_LRN_ALL when
 * MAC_ENTRY_CCM_TYPE_INTERVAL equals 0) for found entries.
 *
 * \details 
 * 0: No change to MAC_ENTRY_CCM_CHG (or MAC_ENTRY_NXT_LRN_ALL)
 * 1: Clear MAC_ENTRY_CCM_CHG (or MAC_ENTRY_NXT_LRN_ALL)
 * 2: Set MAC_ENTRY_CCM_CHG (or MAC_ENTRY_NXT_LRN_ALL). 
 *
 * Field: ::VTSS_LRN_COMMON_SCAN_NEXT_CFG . SCAN_CCM_CHG_UPDATE_SEL
 */
#define  VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_SCAN_CCM_CHG_UPDATE_SEL(x)  VTSS_ENCODE_BITFIELD(x,26,2)
#define  VTSS_M_LRN_COMMON_SCAN_NEXT_CFG_SCAN_CCM_CHG_UPDATE_SEL     VTSS_ENCODE_BITMASK(26,2)
#define  VTSS_X_LRN_COMMON_SCAN_NEXT_CFG_SCAN_CCM_CHG_UPDATE_SEL(x)  VTSS_EXTRACT_BITFIELD(x,26,2)

/** 
 * \brief
 * Specify how CCM_MISS_CNT(or MAC_ENTRY_AGE when
 * MAC_ENTRY_CCM_TYPE_INTERVAL == 0) is matched during scan commands.
 *
 * \details 
 * 0: Ignore 
 * 1: Find entries with CCM_MISS_CNT equal to
 * MAC_ACCESS_CFG_4.MAC_ENTRY_CCM_MISS_CNT or larger
 * 2: Find entries with CCM_MISS_CNT equal to
 * MAC_ACCESS_CFG_4.MAC_ENTRY_CCM_MISS_CNT or smaller
 * 3: Find entries with CCM_MISS_CNT equal to
 * MAC_ACCESS_CFG_4.MAC_ENTRY_CCM_MISS_CNT.
 *
 * Field: ::VTSS_LRN_COMMON_SCAN_NEXT_CFG . CCM_MISS_CNT_FILTER_SEL
 */
#define  VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_CCM_MISS_CNT_FILTER_SEL(x)  VTSS_ENCODE_BITFIELD(x,24,2)
#define  VTSS_M_LRN_COMMON_SCAN_NEXT_CFG_CCM_MISS_CNT_FILTER_SEL     VTSS_ENCODE_BITMASK(24,2)
#define  VTSS_X_LRN_COMMON_SCAN_NEXT_CFG_CCM_MISS_CNT_FILTER_SEL(x)  VTSS_EXTRACT_BITFIELD(x,24,2)

/** 
 * \brief
 * Controls update of MAC_ENTRY_ADDR_TYPE and MAC_ENTRY_ADDR for entries
 * matched during a SCAN command. 
 * (Will only have effect if SCAN_NEXT_UNTIL_FOUND_ENA is disabled).
 * 
 * Allows all MAC addresses learned on a specific port to be moved to
 * another port. Refer also to PORT_MOVE_NEW_ADDR and SCAN_ENTRY_ADDR_MASK
 *
 * \details 
 * 0: MAC_ENTRY_ADDR and MAC_ENTRY_ADDR_TYPE will be kept unaffected.  
 * 1: Entries matching the condition will have MAC_ENTRY_ADDR and
 * MAC_ENTRY_ADDR_TYPE updated.
 *
 * Field: ::VTSS_LRN_COMMON_SCAN_NEXT_CFG . SCAN_NEXT_MOVE_FOUND_ENA
 */
#define  VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_SCAN_NEXT_MOVE_FOUND_ENA  VTSS_BIT(22)

/** 
 * \brief
 * Controls which MAC tables are scanned.
 *
 * \details 
 * 0: Scan both MAC tables
 * 1: Scan only C-domain MAC table
 * 2: Scan only B-domain MAC table.
 *
 * Field: ::VTSS_LRN_COMMON_SCAN_NEXT_CFG . SCAN_DOMAIN_SEL
 */
#define  VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_SCAN_DOMAIN_SEL(x)  VTSS_ENCODE_BITFIELD(x,20,2)
#define  VTSS_M_LRN_COMMON_SCAN_NEXT_CFG_SCAN_DOMAIN_SEL     VTSS_ENCODE_BITMASK(20,2)
#define  VTSS_X_LRN_COMMON_SCAN_NEXT_CFG_SCAN_DOMAIN_SEL(x)  VTSS_EXTRACT_BITFIELD(x,20,2)

/** 
 * \brief
 * Gives the scan result per bucket of the last row scanned (bitgroup
 * contains 1 bit per bucket of an entire row).
 *
 * \details 
 * Per bucket:
 * 0: Bucket does not match filter condition
 * 1: Bucket does match filter condition
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_LRN_COMMON_SCAN_NEXT_CFG . SCAN_NEXT_STATUS
 */
#define  VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_SCAN_NEXT_STATUS(x)  VTSS_ENCODE_BITFIELD(x,16,4)
#define  VTSS_M_LRN_COMMON_SCAN_NEXT_CFG_SCAN_NEXT_STATUS     VTSS_ENCODE_BITMASK(16,4)
#define  VTSS_X_LRN_COMMON_SCAN_NEXT_CFG_SCAN_NEXT_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,16,4)

/** 
 * \brief
 * Allows scan of selective entry types.
 *
 * \details 
 * "0000": Scan only changed CCM entries (MAC_ENTRY_CCM_TYPE_INTERVAL
 * different from 0 and MAC_ENTRY_CCM_CHG set).
 * "xxx1": Scan NON CCM entries (MAC_ENTRY_CCM_TYPE_INTERVAL equals to 0)
 * "xx1x": Scan CCM entries with CCM equal to ccm fault management
 * (MAC_ENTRY_CCM_TYPE_INTERVAL equals to 1)
 * "x1xx": Scan CCM entries with CCM equal to performance management
 * (MAC_ENTRY_CCM_TYPE_INTERVAL equals to 2)
 * "1xxx": Scan CCM entries with CCM equal to protection switching
 * (MAC_ENTRY_CCM_TYPE_INTERVAL equals to 3)
 * "1111": Scan All entries
 *
 * Field: ::VTSS_LRN_COMMON_SCAN_NEXT_CFG . SCAN_TYPE_MASK
 */
#define  VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_SCAN_TYPE_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,4)
#define  VTSS_M_LRN_COMMON_SCAN_NEXT_CFG_SCAN_TYPE_MASK     VTSS_ENCODE_BITMASK(12,4)
#define  VTSS_X_LRN_COMMON_SCAN_NEXT_CFG_SCAN_TYPE_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,4)

/** 
 * \brief
 * Configures whether NXT_LRN_ALL values is don't care or entries with a
 * specific NXT_LRN_ALL/CCM_CHG value are found during FIND SMALLEST or
 * SCAN commands.
 *
 * \details 
 * 0: SCAN and FIND_SMALLEST commands don't care about
 * MAC_ENTRY_NXT_LRN_ALL.
 * 1: Enable filter. SCAN and FIND_SMALLEST commands will only search for
 * entries with NXT_LRN_ALL set according to MAC_ENTRY_NXT_LRN_ALL.

 *
 * Field: ::VTSS_LRN_COMMON_SCAN_NEXT_CFG . NXT_LRN_ALL_FILTER_ENA
 */
#define  VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_NXT_LRN_ALL_FILTER_ENA  VTSS_BIT(11)

/** 
 * \brief
 * Configures scanning of all port or only port not disabled in
 * ANA_L2::FILTER_OTHER_CTRL and ANA_L2::FILTER_LOCAL_CTRL.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_LRN_COMMON_SCAN_NEXT_CFG . SCAN_USE_PORT_FILTER_ENA
 */
#define  VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_SCAN_USE_PORT_FILTER_ENA  VTSS_BIT(10)

/** 
 * \brief
 * Controls whether scan removes entires matching the condition or not.
 * 
 * CPU Age scan:
 * if SCAN_NEXT_REMOVE_FOUND_ENA==1 and SCAN_NEXT_INC_AGE_BITS_ENA == 1 and
 * SCAN_NEXT_AGED_ONLY_ENA==0 then non aged entries (MAC_ENTRY_AGE_FLAG
 * less than max configured via ANA_L2::LRN_CFG.AGE_SIZE) will be aged
 * (MAC_ENTRY_AGE_FLAG incremented) and aged entries  (MAC_ENTRY_AGE_FLAG
 * set to MAX) will be removed.
 * 
 * CPU cleanup scan:
 * if SCAN_NEXT_REMOVE_FOUND_ENA==1 and SCAN_NEXT_INC_AGE_BITS_ENA ==0 then
 * all entries matching the enabled filtering conditions will be removed.
 * 
 * (Will only have effect if SCAN_NEXT_UNTIL_FOUND_ENA is disabled).

 *
 * \details 
 * 0: No entires will be removed.
 * 1: Entries matching the condition will be removed (or MAC_ENTRY_AGE_FLAG
 * will be incremented based on SCAN_NEXT_INC_AGE_BITS_ENA)
 *
 * Field: ::VTSS_LRN_COMMON_SCAN_NEXT_CFG . SCAN_NEXT_REMOVE_FOUND_ENA
 */
#define  VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_SCAN_NEXT_REMOVE_FOUND_ENA  VTSS_BIT(6)

/** 
 * \brief
 * Specify whether the scan stops at the next row which matches the
 * condition during scan commands.
 *
 * \details 
 * 0: SCAN command scans through the complete table. 
 * 1: Scan stops at the first row with one or more entries matching the
 * filter conditions.
 *
 * Field: ::VTSS_LRN_COMMON_SCAN_NEXT_CFG . SCAN_NEXT_UNTIL_FOUND_ENA
 */
#define  VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_SCAN_NEXT_UNTIL_FOUND_ENA  VTSS_BIT(5)

/** 
 * \brief
 * Configures whether the MAC_ENTRY_AGE_FLAG will be incremented during
 * scan commands or not.
 *
 * \details 
 * 0: Find next will keep entries intact. 
 * 1: Find next will increment MAC_ENTRY_AGE_FLAG for entries matching the
 * filter conditions. Refer also to bitgroup SCAN_NEXT_REMOVE_FOUND_ENA
 *
 * Field: ::VTSS_LRN_COMMON_SCAN_NEXT_CFG . SCAN_NEXT_INC_AGE_BITS_ENA
 */
#define  VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_SCAN_NEXT_INC_AGE_BITS_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Specify whether AGE flags (or MAC_ENTRY_CCM_MISS_CNT when
 * MAC_ENTRY_CCM_TYPE_INTERVAL different from 0) are ignored during scan
 * commands.
 *
 * \details 
 * 0: Ignore the MAC_ENTRY_AGE_FLAG duting scan.
 * 1: Find only entries with MAC_ENTRY_AGE_FLAG set.
 *
 * Field: ::VTSS_LRN_COMMON_SCAN_NEXT_CFG . SCAN_NEXT_AGED_ONLY_ENA
 */
#define  VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_SCAN_NEXT_AGED_ONLY_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Configures whether locked entries are ignored during a FIND SMALLEST or
 * SCAN commands or not.
 *
 * \details 
 * 0: Consider only unlocked (entries with MAC_ENTRY_LOCKED cleared)
 * entries during FIND SMALLEST or SCAN commands.
 * 1: Ignore MAC_ENTRY_LOCKED flag during FIND SMALLEST or SCAN commands.
 *
 * Field: ::VTSS_LRN_COMMON_SCAN_NEXT_CFG . SCAN_NEXT_IGNORE_LOCKED_ENA
 */
#define  VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_SCAN_NEXT_IGNORE_LOCKED_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Specifies whether FID values is dont care or will only affect entries
 * with a specific FID value during FIND SMALLEST or SCAN commands (refer
 * also to MAC_ENTRY_FID and ADDR_FILTER_ENA / MAC_ENTRY_ADDR).
 *
 * \details 
 * 0: Disable. SCAN and FIND_SMALLEST commands will treat FID values as
 * dont care.
 * 1: Enable FID/VID filter. SCAN and FIND_SMALLEST commands will only
 * search for entries with FID/VID value equal to
 * LRN::MAC_ACCESS_CFG_0.MAC_ENTRY_FID).
 *
 * Field: ::VTSS_LRN_COMMON_SCAN_NEXT_CFG . FID_FILTER_ENA
 */
#define  VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_FID_FILTER_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Specifies whether ADDR values is dont care or will only affect entries
 * with a specific ADDR value during FIND SMALLEST or SCAN commands (refer
 * also to MAC_ENTRY_ADDR, MAC_ENTRY_ADDR_TYPE and FID_FILTER_ENA /
 * MAC_ENTRY_ADDR).
 *
 * \details 
 * 0: Disable. SCAN and FIND_SMALLEST commands will treat MAC_ENTRY_ADDR
 * and MAC_ENTRY_ADDR_TYPE values as don't care..
 * 1: Enable MAC_ENTRY_ADDR and MAC_ENTRY_ADDR_TYPE filter. SCAN and
 * FIND_SMALLEST commands will only search for entries with Address value
 * corresponding to MAC_ENTRY_ADDR and MAC_ENTRY_ADDR_TYPE taking
 * LRN::SCAN_NEXT_CFG_1.SCAN_ENTRY_ADDR_MASK into account).
 *
 * Field: ::VTSS_LRN_COMMON_SCAN_NEXT_CFG . ADDR_FILTER_ENA
 */
#define  VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_ADDR_FILTER_ENA  VTSS_BIT(0)


/** 
 * \brief Age Filter
 *
 * \details
 * Register: \a LRN:COMMON:SCAN_NEXT_CFG_1
 */
#define VTSS_LRN_COMMON_SCAN_NEXT_CFG_1      VTSS_IOREG(VTSS_TO_LRN,0x7)

/** 
 * \brief
 * NEW ADDRESS when doing port move scan. This specifies the MAC_ENTRY_ADDR
 * with don't care positions (i.e. only care positions are replaced).
 *
 * \details 
 * Field: ::VTSS_LRN_COMMON_SCAN_NEXT_CFG_1 . PORT_MOVE_NEW_ADDR
 */
#define  VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_1_PORT_MOVE_NEW_ADDR(x)  VTSS_ENCODE_BITFIELD(x,15,17)
#define  VTSS_M_LRN_COMMON_SCAN_NEXT_CFG_1_PORT_MOVE_NEW_ADDR     VTSS_ENCODE_BITMASK(15,17)
#define  VTSS_X_LRN_COMMON_SCAN_NEXT_CFG_1_PORT_MOVE_NEW_ADDR(x)  VTSS_EXTRACT_BITFIELD(x,15,17)

/** 
 * \brief
 * ADDRESS mask when doing port scan. This correspond to specify
 * MAC_ENTRY_ADDR with dont care positions (MSB-1 controls two additional
 * type bits).
 * 
 * To scan for all conid entries set MAC_ENTRY_ADDR_TYPE = CONID_FWD and
 * SCAN_ENTRY_ADDR_MASK(14)=1 and remaining SCAN_ENTRY_ADDR_MASK bits to
 * ALL_ZEROS
 * To scan for all conid entries with a specific conid set
 * MAC_ENTRY_ADDR_TYPE = CONID_FWD and SCAN_ENTRY_ADDR_MASK = ALL_ONES
 * To scan for all upsid_pn entries set MAC_ENTRY_ADDR_TYPE= UPSID_PN and
 * SCAN_ENTRY_ADDR_MASK(14:13)="11" and remaining SCAN_ENTRY_ADDR_MASK bits
 * to ALL_ZEROS
 * To scan for all upsid_pn entries with a specific UPSID_UPSPN (ignoreing
 * Dual domain indication) set MAC_ENTRY_ADDR_TYPE= UPSID_PN and
 * SCAN_ENTRY_ADDR_MASK(14:13)="11" and SCAN_ENTRY_ADDR_MASK(12)='0' and
 * SCAN_ENTRY_ADDR_MASK(11:0) bits to ALL_ONES

 *
 * \details 
 * 0: The corresponding bit is treated as dont care (ignore bit value)
 * 1: The corresponding bit is treated as care
 *
 * Field: ::VTSS_LRN_COMMON_SCAN_NEXT_CFG_1 . SCAN_ENTRY_ADDR_MASK
 */
#define  VTSS_F_LRN_COMMON_SCAN_NEXT_CFG_1_SCAN_ENTRY_ADDR_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,15)
#define  VTSS_M_LRN_COMMON_SCAN_NEXT_CFG_1_SCAN_ENTRY_ADDR_MASK     VTSS_ENCODE_BITMASK(0,15)
#define  VTSS_X_LRN_COMMON_SCAN_NEXT_CFG_1_SCAN_ENTRY_ADDR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,15)


/** 
 * \brief Configures last scan row
 *
 * \details
 * Configures an ending row for scan access. 
 *
 * Register: \a LRN:COMMON:SCAN_LAST_ROW_CFG
 */
#define VTSS_LRN_COMMON_SCAN_LAST_ROW_CFG    VTSS_IOREG(VTSS_TO_LRN,0x8)

/** 
 * \brief
 * Configures the last row to be part of scan. ignored if set to all ones.
 *
 * \details 
 * 0: Scan completes at row 0
 * ...
 * n: Scan completes at row n
 *
 * Field: ::VTSS_LRN_COMMON_SCAN_LAST_ROW_CFG . SCAN_LAST_ROW
 */
#define  VTSS_F_LRN_COMMON_SCAN_LAST_ROW_CFG_SCAN_LAST_ROW(x)  VTSS_ENCODE_BITFIELD(x,0,14)
#define  VTSS_M_LRN_COMMON_SCAN_LAST_ROW_CFG_SCAN_LAST_ROW     VTSS_ENCODE_BITMASK(0,14)
#define  VTSS_X_LRN_COMMON_SCAN_LAST_ROW_CFG_SCAN_LAST_ROW(x)  VTSS_EXTRACT_BITFIELD(x,0,14)


/** 
 * \brief Age Filter
 *
 * \details
 * Register: \a LRN:COMMON:SCAN_NEXT_CNT
 */
#define VTSS_LRN_COMMON_SCAN_NEXT_CNT        VTSS_IOREG(VTSS_TO_LRN,0x9)

/** 
 * \brief
 * Configures SCAN increment to use nxt_lrn_all as additional age bit where
 * frames are flooded.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_LRN_COMMON_SCAN_NEXT_CNT . SCAN_INC_AGE_AND_NXT_LRN_ALL_ENA
 */
#define  VTSS_F_LRN_COMMON_SCAN_NEXT_CNT_SCAN_INC_AGE_AND_NXT_LRN_ALL_ENA  VTSS_BIT(31)

/** 
 * \brief
 * Specifies the CCM SCAN entry types. This is cleared per bit once SCAN of
 * the given type start.
 *
 * \details 
 * xx1: SCAN PORT entries
 * x1x: SCAN ISDX entries
 * 1xx: SCAN PATH entries
 *
 * Field: ::VTSS_LRN_COMMON_SCAN_NEXT_CNT . SCAN_NEXT_CCM_TYPE
 */
#define  VTSS_F_LRN_COMMON_SCAN_NEXT_CNT_SCAN_NEXT_CCM_TYPE(x)  VTSS_ENCODE_BITFIELD(x,28,3)
#define  VTSS_M_LRN_COMMON_SCAN_NEXT_CNT_SCAN_NEXT_CCM_TYPE     VTSS_ENCODE_BITMASK(28,3)
#define  VTSS_X_LRN_COMMON_SCAN_NEXT_CNT_SCAN_NEXT_CCM_TYPE(x)  VTSS_EXTRACT_BITFIELD(x,28,3)

/** 
 * \brief
 * Reports the current CCM SCAN entry type.
 *
 * \details 
 * 1: SCAN PORT entries
 * 2: SCAN ISDX entries
 * 3: SCAN PATH entries
 *
 * Field: ::VTSS_LRN_COMMON_SCAN_NEXT_CNT . SCAN_NEXT_CCM_TYPE_STATUS
 */
#define  VTSS_F_LRN_COMMON_SCAN_NEXT_CNT_SCAN_NEXT_CCM_TYPE_STATUS(x)  VTSS_ENCODE_BITFIELD(x,24,2)
#define  VTSS_M_LRN_COMMON_SCAN_NEXT_CNT_SCAN_NEXT_CCM_TYPE_STATUS     VTSS_ENCODE_BITMASK(24,2)
#define  VTSS_X_LRN_COMMON_SCAN_NEXT_CNT_SCAN_NEXT_CCM_TYPE_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,24,2)

/** 
 * \brief
 * Counter with number of found scan entries.
 *
 * \details 
 * Field: ::VTSS_LRN_COMMON_SCAN_NEXT_CNT . SCAN_NEXT_CNT
 */
#define  VTSS_F_LRN_COMMON_SCAN_NEXT_CNT_SCAN_NEXT_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_LRN_COMMON_SCAN_NEXT_CNT_SCAN_NEXT_CNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_LRN_COMMON_SCAN_NEXT_CNT_SCAN_NEXT_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Automated age scan configuration
 *
 * \details
 * Configures automated age scan of MAC table
 *
 * Register: \a LRN:COMMON:AUTOAGE_CFG
 */
#define VTSS_LRN_COMMON_AUTOAGE_CFG          VTSS_IOREG(VTSS_TO_LRN,0xa)

/** 
 * \brief
 * Configures automated ageing of all port or only port not disabled in
 * ANA_L2::FILTER_OTHER_CTRL and ANA_L2::FILTER_LOCAL_CTRL.
 *
 * \details 
 * 0: Disable use additional port filter
 * 1: Enable use of additional port filter
 *
 * Field: ::VTSS_LRN_COMMON_AUTOAGE_CFG . AUTOAGE_USE_PORT_FILTER_ENA
 */
#define  VTSS_F_LRN_COMMON_AUTOAGE_CFG_AUTOAGE_USE_PORT_FILTER_ENA  VTSS_BIT(31)

/** 
 * \brief
 * Configures prolonged (x2) life for B-MAC table entries when operating in
 * with two MAC tables to ensure all C-domain entries are aged before
 * associeted B-domain entries are aged. 
 *
 * \details 
 * 0: Disable 
 * 1: enable prolonged age period for b-domain entries (B-MAC entries are
 * kept even though they are no longe used for forwarding)
 *
 * Field: ::VTSS_LRN_COMMON_AUTOAGE_CFG . AUTOAGE_INC_AGE_AND_NXT_LRN_ALL_ENA
 */
#define  VTSS_F_LRN_COMMON_AUTOAGE_CFG_AUTOAGE_INC_AGE_AND_NXT_LRN_ALL_ENA  VTSS_BIT(28)

/** 
 * \brief
 * Sets the unit time. Setting UNIT_SIZE to zero effectively disables
 * ageing.
 *
 * \details 
 * 0: Disable
 * 1: 100 us (fastest possible scan is bestcase 150 us so increase
 * AUTOAGE_PERIOD_VAL to 1)
 * 2: 10 ms
 * 3: 1 s
 *
 * Field: ::VTSS_LRN_COMMON_AUTOAGE_CFG . AUTOAGE_UNIT_SIZE
 */
#define  VTSS_F_LRN_COMMON_AUTOAGE_CFG_AUTOAGE_UNIT_SIZE(x)  VTSS_ENCODE_BITFIELD(x,24,2)
#define  VTSS_M_LRN_COMMON_AUTOAGE_CFG_AUTOAGE_UNIT_SIZE     VTSS_ENCODE_BITMASK(24,2)
#define  VTSS_X_LRN_COMMON_AUTOAGE_CFG_AUTOAGE_UNIT_SIZE(x)  VTSS_EXTRACT_BITFIELD(x,24,2)

/** 
 * \brief
 * Time in units between automatic aging of an entry.
 * If otherwise subject to aging, an inactive entry is removed after
 * between AGE_PERIOD_VAL and 2*AGE_PERIOD_VAL.
 *
 * \details 
 * n: Age period = (n+1) * UNIT_SIZE
 *
 * Field: ::VTSS_LRN_COMMON_AUTOAGE_CFG . AUTOAGE_PERIOD_VAL
 */
#define  VTSS_F_LRN_COMMON_AUTOAGE_CFG_AUTOAGE_PERIOD_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,22)
#define  VTSS_M_LRN_COMMON_AUTOAGE_CFG_AUTOAGE_PERIOD_VAL     VTSS_ENCODE_BITMASK(0,22)
#define  VTSS_X_LRN_COMMON_AUTOAGE_CFG_AUTOAGE_PERIOD_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,22)


/** 
 * \brief Automated age scan configuration and status
 *
 * \details
 * Configures automated age scan of MAC table
 *
 * Register: \a LRN:COMMON:AUTOAGE_CFG_1
 */
#define VTSS_LRN_COMMON_AUTOAGE_CFG_1        VTSS_IOREG(VTSS_TO_LRN,0xb)

/** 
 * \brief
 * Current autoage row. Changed on every autoage command. Indicate the MAC
 * table row to be autoaged aged next.
 * 
 * Incremented by hardware during auto ageing.
 *
 * \details 
 * 0: Row 0
 * 1: Row 1
 * ...
 * n: Row n.
 *
 * Field: ::VTSS_LRN_COMMON_AUTOAGE_CFG_1 . AUTOAGE_NEXT_ROW
 */
#define  VTSS_F_LRN_COMMON_AUTOAGE_CFG_1_AUTOAGE_NEXT_ROW(x)  VTSS_ENCODE_BITFIELD(x,16,14)
#define  VTSS_M_LRN_COMMON_AUTOAGE_CFG_1_AUTOAGE_NEXT_ROW     VTSS_ENCODE_BITMASK(16,14)
#define  VTSS_X_LRN_COMMON_AUTOAGE_CFG_1_AUTOAGE_NEXT_ROW(x)  VTSS_EXTRACT_BITFIELD(x,16,14)

/** 
 * \brief
 * Triggers an instant hardware autoage scan (once current scan completes
 * -which can be ensured using AUTOAGE_FORCE_HW_SCAN_STOP_ENA).
 * 
 * The bit is cleared by HW when a new automated scan begins.
 *
 * \details 
 * 0: No force
 * 1: Force start of autoage scan
 *
 * Field: ::VTSS_LRN_COMMON_AUTOAGE_CFG_1 . AUTOAGE_FORCE_HW_SCAN_SHOT
 */
#define  VTSS_F_LRN_COMMON_AUTOAGE_CFG_1_AUTOAGE_FORCE_HW_SCAN_SHOT  VTSS_BIT(2)

/** 
 * \brief
 * Triggers forced stop of current autoage scan
 * 
 * The bit is cleared by HW when scan stop is obtained
 *
 * \details 
 * 0: No force
 * 1: Force autoage SCAN stop
 *
 * Field: ::VTSS_LRN_COMMON_AUTOAGE_CFG_1 . AUTOAGE_FORCE_HW_SCAN_STOP_SHOT
 */
#define  VTSS_F_LRN_COMMON_AUTOAGE_CFG_1_AUTOAGE_FORCE_HW_SCAN_STOP_SHOT  VTSS_BIT(1)

/** 
 * \brief
 * Configures automated age scan stop after current automated scan
 * completes.

 *
 * \details 
 * 0: No force
 * 1: SCAN stop after current scan completes and stays stopped.
 *
 * Field: ::VTSS_LRN_COMMON_AUTOAGE_CFG_1 . AUTOAGE_FORCE_IDLE_ENA
 */
#define  VTSS_F_LRN_COMMON_AUTOAGE_CFG_1_AUTOAGE_FORCE_IDLE_ENA  VTSS_BIT(0)


/** 
 * \brief Auto Learn Options
 *
 * \details
 * Register: \a LRN:COMMON:AUTO_LRN_CFG
 */
#define VTSS_LRN_COMMON_AUTO_LRN_CFG         VTSS_IOREG(VTSS_TO_LRN,0xc)

/** 
 * \brief
 * The SRC_KILL_FWD flag for auto-learned entries will be set to the value
 * configured in this field.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_LRN_COMMON_AUTO_LRN_CFG . AUTO_LRN_SRC_KILL_FWD
 */
#define  VTSS_F_LRN_COMMON_AUTO_LRN_CFG_AUTO_LRN_SRC_KILL_FWD  VTSS_BIT(4)

/** 
 * \brief
 * The VLAN_IGNORE flag for auto-learned entries will be set to the value
 * configured in this field.
 *
 * \details 
 * 0: VLAN mask is used
 * 1: VLAN mask is ignored.
 *
 * Field: ::VTSS_LRN_COMMON_AUTO_LRN_CFG . AUTO_LRN_IGNORE_VLAN
 */
#define  VTSS_F_LRN_COMMON_AUTO_LRN_CFG_AUTO_LRN_IGNORE_VLAN  VTSS_BIT(3)

/** 
 * \brief
 * The CPU_COPY flag for auto-learned entries will be set to the value
 * configured in this field.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_LRN_COMMON_AUTO_LRN_CFG . AUTO_LRN_CPU_COPY
 */
#define  VTSS_F_LRN_COMMON_AUTO_LRN_CFG_AUTO_LRN_CPU_COPY  VTSS_BIT(2)

/** 
 * \brief
 * The MIRROR flag for auto-learned entries will be set to the value
 * configured in this field.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_LRN_COMMON_AUTO_LRN_CFG . AUTO_LRN_MIRROR
 */
#define  VTSS_F_LRN_COMMON_AUTO_LRN_CFG_AUTO_LRN_MIRROR  VTSS_BIT(1)


/** 
 * \brief CCM scan configuration
 *
 * \details
 * Configures automated CCM scan
 *
 * Register: \a LRN:COMMON:CCM_CTRL
 */
#define VTSS_LRN_COMMON_CCM_CTRL             VTSS_IOREG(VTSS_TO_LRN,0xd)

/** 
 * \brief
 * Specifies an instant CCM miss count scan for the configured interval
 * (once current scan completes).
 * 
 * This can be combined CCM_FORCE_HW_SCAN_STOP_ENA to ensure the scan is
 * immidiately carried out.
 * 
 * The bit is cleared by HW when increment is complete
 *
 * \details 
 * 0: No force
 * "xx1": Force one CCM miss count increment for fault management entries
 * "x1x": Force one CCM miss count increment for performance management
 * entries
 * "1xx": Force one CCM miss count increment for protection switching
 * entries

 *
 * Field: ::VTSS_LRN_COMMON_CCM_CTRL . CCM_FORCE_HW_SCAN_ENA
 */
#define  VTSS_F_LRN_COMMON_CCM_CTRL_CCM_FORCE_HW_SCAN_ENA(x)  VTSS_ENCODE_BITFIELD(x,25,3)
#define  VTSS_M_LRN_COMMON_CCM_CTRL_CCM_FORCE_HW_SCAN_ENA     VTSS_ENCODE_BITMASK(25,3)
#define  VTSS_X_LRN_COMMON_CCM_CTRL_CCM_FORCE_HW_SCAN_ENA(x)  VTSS_EXTRACT_BITFIELD(x,25,3)

/** 
 * \brief
 * Specifies instant stop of CCM scan for the configured interval.
 * 
 * The bit is cleared by HW when scan stop is complete
 *
 * \details 
 * 0: No force
 * "xx1": Force stop of fault management scan
 * "x1x": Force stop of performance management scan
 * "1xx": Force stop of protection switching scan

 *
 * Field: ::VTSS_LRN_COMMON_CCM_CTRL . CCM_FORCE_HW_SCAN_STOP_ENA
 */
#define  VTSS_F_LRN_COMMON_CCM_CTRL_CCM_FORCE_HW_SCAN_STOP_ENA  VTSS_BIT(24)

/** 
 * \brief
 * Specifies the number of clk cycle before another scan entry can be
 * attempted.
 *
 * \details 
 * Field: ::VTSS_LRN_COMMON_CCM_CTRL . CCM_SPACE_BETWEEN_ENTRY_SCAN
 */
#define  VTSS_F_LRN_COMMON_CCM_CTRL_CCM_SPACE_BETWEEN_ENTRY_SCAN(x)  VTSS_ENCODE_BITFIELD(x,16,8)
#define  VTSS_M_LRN_COMMON_CCM_CTRL_CCM_SPACE_BETWEEN_ENTRY_SCAN     VTSS_ENCODE_BITMASK(16,8)
#define  VTSS_X_LRN_COMMON_CCM_CTRL_CCM_SPACE_BETWEEN_ENTRY_SCAN(x)  VTSS_EXTRACT_BITFIELD(x,16,8)

/** 
 * \brief
 * Specifies the number of us for a CCM base unit used when configuring
 * each of the scan intervals (CCM_TYPE_CFG.CCM_SCAN_INTERVAL).
 *
 * \details 
 * 0: Disable automatic HW scan
 * 1: 1 us
 * ...
 * n: n us
 *
 * Field: ::VTSS_LRN_COMMON_CCM_CTRL . CCM_BASE_TICK_CNT
 */
#define  VTSS_F_LRN_COMMON_CCM_CTRL_CCM_BASE_TICK_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_LRN_COMMON_CCM_CTRL_CCM_BASE_TICK_CNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_LRN_COMMON_CCM_CTRL_CCM_BASE_TICK_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Configuration of CCM types
 *
 * \details
 * Specifies the time for the three CCM scan intervals: 
 *   0:fault management
 *   1:performance management
 *   2:protection switching
 *
 * Register: \a LRN:COMMON:CCM_TYPE_CFG
 *
 * @param ri Register: CCM_TYPE_CFG (??), 0-2
 */
#define VTSS_LRN_COMMON_CCM_TYPE_CFG(ri)     VTSS_IOREG(VTSS_TO_LRN,0xe + (ri))

/** 
 * \brief
 * Specifies per CCM type the interval between CCM miss count increment .
 * 
 * Numbers in Base time units (configured through:
 * CCM_CTRL.CCM_BASE_TICK_CNT)
 *
 * \details 
 * 0: CCM miss count is not incremented by hardware for the CCM_TYPE
 * 1: CCM miss count is incremented every 1 unit time 
 * ...
 * n: CCM miss count is incremented every n unit time 
 *
 * Field: ::VTSS_LRN_COMMON_CCM_TYPE_CFG . CCM_SCAN_INTERVAL
 */
#define  VTSS_F_LRN_COMMON_CCM_TYPE_CFG_CCM_SCAN_INTERVAL(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_LRN_COMMON_CCM_TYPE_CFG_CCM_SCAN_INTERVAL     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_LRN_COMMON_CCM_TYPE_CFG_CCM_SCAN_INTERVAL(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief CCM SCAN row status
 *
 * \details
 * Status of current CCM scan
 *
 * Register: \a LRN:COMMON:CCM_STATUS
 */
#define VTSS_LRN_COMMON_CCM_STATUS           VTSS_IOREG(VTSS_TO_LRN,0x11)

/** 
 * \brief
 * High when CCM_SCAN is ongoing.
 *
 * \details 
 * 0: No scan active
 * x1x: CCM scan of the interval is ongoing.
 *
 * Field: ::VTSS_LRN_COMMON_CCM_STATUS . CCM_SCAN_ONGOING_STATUS
 */
#define  VTSS_F_LRN_COMMON_CCM_STATUS_CCM_SCAN_ONGOING_STATUS(x)  VTSS_ENCODE_BITFIELD(x,17,3)
#define  VTSS_M_LRN_COMMON_CCM_STATUS_CCM_SCAN_ONGOING_STATUS     VTSS_ENCODE_BITMASK(17,3)
#define  VTSS_X_LRN_COMMON_CCM_STATUS_CCM_SCAN_ONGOING_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,17,3)

/** 
 * \brief
 * Current CCM scan type.
 * Changed by hardware during CCM SCAN.
 *
 * \details 
 * xx1: SCAN PORT entries
 * x1x: SCAN ISDX entries
 * 1xx: SCAN PATH entries
 *
 * Field: ::VTSS_LRN_COMMON_CCM_STATUS . CCM_NEXT_TYPE
 */
#define  VTSS_F_LRN_COMMON_CCM_STATUS_CCM_NEXT_TYPE(x)  VTSS_ENCODE_BITFIELD(x,14,3)
#define  VTSS_M_LRN_COMMON_CCM_STATUS_CCM_NEXT_TYPE     VTSS_ENCODE_BITMASK(14,3)
#define  VTSS_X_LRN_COMMON_CCM_STATUS_CCM_NEXT_TYPE(x)  VTSS_EXTRACT_BITFIELD(x,14,3)

/** 
 * \brief
 * Current CCM row. Changed on every ccm command. Used to indicate the
 * index to be CCM scanned next.
 * 
 * Incremented by hardware during CCM SCAN.
 *
 * \details 
 * 0: Row 0
 * 1: Row 1
 * ...
 * n: Row n.
 *
 * Field: ::VTSS_LRN_COMMON_CCM_STATUS . CCM_NEXT_ROW
 */
#define  VTSS_F_LRN_COMMON_CCM_STATUS_CCM_NEXT_ROW(x)  VTSS_ENCODE_BITFIELD(x,0,14)
#define  VTSS_M_LRN_COMMON_CCM_STATUS_CCM_NEXT_ROW     VTSS_ENCODE_BITMASK(0,14)
#define  VTSS_X_LRN_COMMON_CCM_STATUS_CCM_NEXT_ROW(x)  VTSS_EXTRACT_BITFIELD(x,0,14)


/** 
 * \brief Sticky diagnostic status
 *
 * \details
 * Register: \a LRN:COMMON:EVENT_STICKY
 */
#define VTSS_LRN_COMMON_EVENT_STICKY         VTSS_IOREG(VTSS_TO_LRN,0x12)

/** 
 * \brief
 * High when AUTOAGE is ongoing.
 *
 * \details 
 * 0: No event has occured
 * 1: AUTOAGE scan ongoing.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . AUTOAGE_SCAN_ONGOING_STATUS
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_AUTOAGE_SCAN_ONGOING_STATUS  VTSS_BIT(31)

/** 
 * \brief
 * Set when AUTOAGE scan completes.
 *
 * \details 
 * 0: No event has occured
 * 1: AUTOAGE scan completes
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . AUTOAGE_SCAN_COMPLETED_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_AUTOAGE_SCAN_COMPLETED_STICKY  VTSS_BIT(30)

/** 
 * \brief
 * Set when Autoage scan starts.
 *
 * \details 
 * 0: No event has occured
 * 1: AUTOAGE scan started
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . AUTOAGE_SCAN_STARTED_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_AUTOAGE_SCAN_STARTED_STICKY  VTSS_BIT(29)

/** 
 * \brief
 * Set when CCM_SCAN completes.
 *
 * \details 
 * 0: No event has occured
 * 1: Leak scan completes
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . CCM_SCAN_COMPLETED_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_CCM_SCAN_COMPLETED_STICKY  VTSS_BIT(27)

/** 
 * \brief
 * Set when CCM_SCAN starts.
 *
 * \details 
 * 0: No event has occured
 * 1: Leak scan started
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . CCM_SCAN_STARTED_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_CCM_SCAN_STARTED_STICKY  VTSS_BIT(26)

/** 
 * \brief
 * Set when a CCM scan could not start because a scan is already ongoing -
 * CCM_BASE_TICK_CNT must be increased.
 *
 * \details 
 * 0: No event has occured
 * 1: Scan could not start in time
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . CCM_SCAN_START_DELAYED_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_CCM_SCAN_START_DELAYED_STICKY  VTSS_BIT(25)

/** 
 * \brief
 * A sticky bit must be set when a learn request arrives to an already full
 * queue.
 *
 * \details 
 * 0: No overflow
 * 1:  Overflow
 * Write '1' to clear bit.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . LRN_QUEUE_OVERFLOW_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_LRN_QUEUE_OVERFLOW_STICKY  VTSS_BIT(24)

/** 
 * \brief
 * A non empty learn queue must be signalled.
 *
 * \details 
 * 0:  Empty
 * 1:  Non empty
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . LRN_QUEUE_DATA_AVAIL_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_LRN_QUEUE_DATA_AVAIL_STICKY  VTSS_BIT(23)

/** 
 * \brief
 * Set if a MAC address entry has been aged due to an autoage operation.
 *
 * \details 
 * 0: No entries have been aged due to autoage.
 * 1: At least 1 entry has been aged due to autoage.
 * Write '1' to clear this field.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . AUTOAGE_AGED_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_AUTOAGE_AGED_STICKY  VTSS_BIT(22)

/** 
 * \brief
 * Set if a MAC address entry has been removed due to an autoage operation.
 *
 * \details 
 * 0: No entries have been removed due to autoage.
 * 1: At least 1 entry has been removed due to autoage.
 * Write '1' to clear this field.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . AUTOAGE_REMOVE_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_AUTOAGE_REMOVE_STICKY  VTSS_BIT(21)

/** 
 * \brief
 * Set if a row matching the scan condition has been found. (Check after
 * MAC_TABLE_ACCESS_SHOT went to 0).
 *
 * \details 
 * 0: No entry has been found.
 * 1: An entry has been found.
 * Write '1' to clear this field.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . ROW_WITH_SCAN_ENTRY_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_ROW_WITH_SCAN_ENTRY_STICKY  VTSS_BIT(20)

/** 
 * \brief
 * Set if a MAC address entry has been removed due to a scan operation.
 *
 * \details 
 * 0: No entries have been removed due to scan.
 * 1: At least 1 entry has been removed due to scan.
 * Write '1' to clear this field.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . SCAN_REMOVED_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_SCAN_REMOVED_STICKY  VTSS_BIT(19)

/** 
 * \brief
 * Set if an autonomous learning operation has failed due to hash table
 * depletion (i.e. all entries in a row were locked). Write '1' to clear
 * this field.
 *
 * \details 
 * 0: No autonomous learning operations have failed.
 * 1: An autonomous learning operation has failed.
 * Write '1' to clear this field.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . AUTO_LRN_FAILED_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_AUTO_LRN_FAILED_STICKY  VTSS_BIT(17)

/** 
 * \brief
 * Set if a MAC address entry has been inserted into a MAC address table
 * column (bucket). Write '1' to clear this field.
 *
 * \details 
 * 0: No entries have been inserted into empty buckets
 * 1: An entry has been inserted into an empty bucket.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . AUTO_LRN_INSERT_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_AUTO_LRN_INSERT_STICKY  VTSS_BIT(15)

/** 
 * \brief
 * Set if MAC entry has been updated. Write '1' to clear this field.
 *
 * \details 
 * 0: No entries have been refreshed.
 * 1: An entry has been refreshed.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . AUTO_LRN_REFRESH_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_AUTO_LRN_REFRESH_STICKY  VTSS_BIT(14)

/** 
 * \brief
 * Set if a randomly selected non-locked and valid MAC address entry can
 * not be overwritten by a new (VID, SMAC). Write '1' to clear this field.
 *
 * \details 
 * 0: No entries have been replaced.
 * 1: An entry has not been replaced due to configuration.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . AUTO_LRN_REPLACE_FAILED_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_AUTO_LRN_REPLACE_FAILED_STICKY  VTSS_BIT(13)

/** 
 * \brief
 * Set if a randomly selected non-locked and valid MAC address entry has
 * been overwritten by a new (VID, SMAC). Write '1' to clear this field.
 *
 * \details 
 * 0: No entries have been replaced.
 * 1: An entry has been replaced.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . AUTO_LRN_REPLACE_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_AUTO_LRN_REPLACE_STICKY  VTSS_BIT(12)

/** 
 * \brief
 * Set when a direct CPU MAC READ operation has occured. Write '1' to clear
 * this field.
 *
 * \details 
 * 0:  No update
 * 1: CPU read direct operation has occured.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . CPU_READ_DIRECT_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_CPU_READ_DIRECT_STICKY  VTSS_BIT(10)

/** 
 * \brief
 * Set when a direct CPU MAC WRITE operation has occured. Write '1' to
 * clear this field.
 *
 * \details 
 * 0:  No update
 * 1: CPU write direct operation has occured.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . CPU_WRITE_DIRECT_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_CPU_WRITE_DIRECT_STICKY  VTSS_BIT(9)

/** 
 * \brief
 * Set if a CPU MAC LOOKUP operation has failed due to not found an entry.
 * Write '1' to clear this field.
 *
 * \details 
 * 0:  No fail
 * 1: CPU lookup operation failed.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . CPU_LOOKUP_FAILED_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_CPU_LOOKUP_FAILED_STICKY  VTSS_BIT(8)

/** 
 * \brief
 * Set if a CPU MAC LOOKUP operation has succeeded. Write '1' to clear this
 * field.
 *
 * \details 
 * 0:  No successful lookup
 * 1: Successful lookup.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . CPU_LOOKUP_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_CPU_LOOKUP_STICKY  VTSS_BIT(7)

/** 
 * \brief
 * Set if a CPU MAC UNLEARN operation has failed because the respective
 * (valid) entry was not found. Write '1' to clear this field.
 *
 * \details 
 * 0:  No UNLEARN operation failed
 * 1: CPU UNLEARN operation failed.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . CPU_UNLEARN_FAILED_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_CPU_UNLEARN_FAILED_STICKY  VTSS_BIT(6)

/** 
 * \brief
 * Set if a CPU MAC UNLEARN operation has succeeded. Write '1' to clear
 * this field.
 *
 * \details 
 * 0:  No CPU MAC UNLEARN operation took place
 * 1:  CPU UNLEARN operations has succeeded.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . CPU_UNLEARN_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_CPU_UNLEARN_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * Set if CPU learn replace can not be performed due to configuration.
 * Write '1' to clear this field.
 *
 * \details 
 * 0: No learn based replacement failed.
 * 1: The replacement of 1 or more entries failed.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . CPU_LRN_REPLACE_FAILED_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_CPU_LRN_REPLACE_FAILED_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Set if a valid MAC entry has been updated (refreshed), i.e. a write
 * access to the same bucket with the same MAC and VID has occured. Write
 * '1' to clear this field.
 *
 * \details 
 * 0: No entries have been refreshed.
 * 1: An entry has been refreshed.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . CPU_LRN_REFRESH_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_CPU_LRN_REFRESH_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * Set if a valid MAC entry has been overwritten, i.e. a write access to
 * the same bucket with a changed MAC and/or VID has occured. Write '1' to
 * clear this field.
 *
 * \details 
 * 0: No entries have been replaced.
 * 1: An entry has been replaced.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . CPU_LRN_REPLACE_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_CPU_LRN_REPLACE_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Set if a MAC entry has been inserted into an empty MAC table row,column
 * (=bucket). Write '1' to clear this field.
 *
 * \details 
 * 0: No entries have been inserted into an empty bucket
 * 1: An entry has been inserted into an empty bucket.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . CPU_LRN_INSERT_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_CPU_LRN_INSERT_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Set if an cpu learning operation has failed due to hash table depletion
 * (i.e. all entries in a row were locked). Write '1' to clear this field.
 *
 * \details 
 * 0: No CPU LEARN operations has failed.
 * 1: A CPU LEARN operation has failed.
 *
 * Field: ::VTSS_LRN_COMMON_EVENT_STICKY . CPU_LRN_FAILED_STICKY
 */
#define  VTSS_F_LRN_COMMON_EVENT_STICKY_CPU_LRN_FAILED_STICKY  VTSS_BIT(0)


/** 
 * \brief Learn Diagnostic
 *
 * \details
 * Register: \a LRN:COMMON:LATEST_POS_STATUS
 */
#define VTSS_LRN_COMMON_LATEST_POS_STATUS    VTSS_IOREG(VTSS_TO_LRN,0x13)

/** 
 * \brief
 * This field holds the current (i.e. latest accessed) MAC table row number
 * after a CPU_ACCESS_CMD has been finished.
 *
 * \details 
 * 0: Row 0
 * 1: Row 1
 * ...
 * n: Row n
 *
 * Field: ::VTSS_LRN_COMMON_LATEST_POS_STATUS . LATEST_ROW
 */
#define  VTSS_F_LRN_COMMON_LATEST_POS_STATUS_LATEST_ROW(x)  VTSS_ENCODE_BITFIELD(x,4,14)
#define  VTSS_M_LRN_COMMON_LATEST_POS_STATUS_LATEST_ROW     VTSS_ENCODE_BITMASK(4,14)
#define  VTSS_X_LRN_COMMON_LATEST_POS_STATUS_LATEST_ROW(x)  VTSS_EXTRACT_BITFIELD(x,4,14)

/** 
 * \brief
 * This field holds the current (i.e. latest accessed) column number after
 * a CPU_ACCESS_CMD has been finished.
 *
 * \details 
 * 0: Column (bucket) 0
 * 1: Column (bucket) 1
 * ...
 * n: Column (bucket) n
 *
 * Field: ::VTSS_LRN_COMMON_LATEST_POS_STATUS . LATEST_COL
 */
#define  VTSS_F_LRN_COMMON_LATEST_POS_STATUS_LATEST_COL(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_LRN_COMMON_LATEST_POS_STATUS_LATEST_COL     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_LRN_COMMON_LATEST_POS_STATUS_LATEST_COL(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Memory integrity status registers
 *
 * \details
 * Register: \a LRN:COMMON:RAM_INTGR_ERR_IDENT
 */
#define VTSS_LRN_COMMON_RAM_INTGR_ERR_IDENT  VTSS_IOREG(VTSS_TO_LRN,0x14)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_LRN_COMMON_RAM_INTGR_ERR_IDENT . IGR_LRN_RAM_INTGR_ERR
 */
#define  VTSS_F_LRN_COMMON_RAM_INTGR_ERR_IDENT_IGR_LRN_RAM_INTGR_ERR  VTSS_BIT(0)


#endif /* _VTSS_JAGUAR_REGS_LRN_H_ */
