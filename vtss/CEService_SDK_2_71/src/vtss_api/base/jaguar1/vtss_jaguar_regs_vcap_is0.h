#ifndef _VTSS_JAGUAR_REGS_VCAP_IS0_H_
#define _VTSS_JAGUAR_REGS_VCAP_IS0_H_

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
 * Target: \a VCAP_IS0
 *
 * Ingress Service classification block.
 *
 ***********************************************************************/

/**
 * Register Group: \a VCAP_IS0:IS0_CONTROL
 *
 * Miscellaneous Configuration Registers for VCAP
 */


/** 
 * \brief Miscellaneous VCAP Configuration
 *
 * \details
 * Register: \a VCAP_IS0:IS0_CONTROL:ACL_CFG
 */
#define VTSS_VCAP_IS0_IS0_CONTROL_ACL_CFG    VTSS_IOREG(VTSS_TO_VCAP_IS0,0x0)

/** 
 * \brief
 * Enables or disables use of VCAP frame processing. The VCAP can still be
 * configured even though VCAP frame processing is disabled.
 *
 * \details 
 * 0:  VCAP frame processing is disabled.
 * 1:  VCAP frame processing is enabled.
 *
 * Field: ::VTSS_VCAP_IS0_IS0_CONTROL_ACL_CFG . ACL_ENA
 */
#define  VTSS_F_VCAP_IS0_IS0_CONTROL_ACL_CFG_ACL_ENA  VTSS_BIT(0)


/** 
 * \brief Entry and Action Control
 *
 * \details
 * Register: \a VCAP_IS0:IS0_CONTROL:ACE_UPDATE_CTRL
 */
#define VTSS_VCAP_IS0_IS0_CONTROL_ACE_UPDATE_CTRL  VTSS_IOREG(VTSS_TO_VCAP_IS0,0x1)

/** 
 * \brief
 * This field holds the entry/action address that must be used when copying
 * the cache to the TCAM/RAM. It is also used when copying an entry/action
 * from the TCAM/RAM to the cache.
 * When a MOVE operation is performed, this register holds the address base
 * of the block of entries/actions that must be moved.

 *
 * \details 
 * Encoding when copying the entry and action cache to the TCAM/RAM:
 * 0x0 : The data in the cache is placed at entry/action index 0.
 * 0x1 : The data in the cache is placed at entry/action index 1.
 * ...
 * Encoding for MOVE operations:
 * 0x0 : The entries and/or actions at address 0x0 through
 * 0x0+ACE_MV_SIZE-1 are moved up or down.
 * 0x1 : The entries and/or actions at address 0x1 through
 * 0x1+ACE_MV_SIZE-1 are moved up or down.
 * ...
 * n : The entries and/or actions at address n through n+ACE_MV_SIZE-1 are
 * moved up or down.
 *
 * Field: ::VTSS_VCAP_IS0_IS0_CONTROL_ACE_UPDATE_CTRL . ACE_INDEX
 */
#define  VTSS_F_VCAP_IS0_IS0_CONTROL_ACE_UPDATE_CTRL_ACE_INDEX(x)  VTSS_ENCODE_BITFIELD(x,4,13)
#define  VTSS_M_VCAP_IS0_IS0_CONTROL_ACE_UPDATE_CTRL_ACE_INDEX     VTSS_ENCODE_BITMASK(4,13)
#define  VTSS_X_VCAP_IS0_IS0_CONTROL_ACE_UPDATE_CTRL_ACE_INDEX(x)  VTSS_EXTRACT_BITFIELD(x,4,13)

/** 
 * \brief
 * This field specifies the basic operation that must be carried out when
 * the one-shot ACE_UPDATE_SHOT field is set. For example, a number of
 * entries and/or associated actions can be moved up or down in the
 * TCAM/RAM.
 *
 * \details 
 * 000: Copy entry and action cache to TCAM/RAM.
 * 001: Copy entry and action from TCAM/RAM to cache.
 * 010: Move entry and action at address ACE_INDEX through
 * ACE_INDEX+ACE_MV_SIZE-1 up ACE_MV_NUM_POS positions.
 * 011: Move entry and action at address ACE_INDEX through
 * ACE_INDEX+ACE_MV_SIZE-1 down ACE_MV_NUM_POS positions.
 * 100: Initialize all entries and normal actions with the value in the
 * cache. This can be used to, for example, reset all entries and actions.
 * Any default actions are always reset to the fixed reset values.
 *
 * Field: ::VTSS_VCAP_IS0_IS0_CONTROL_ACE_UPDATE_CTRL . ACE_UPDATE_CMD
 */
#define  VTSS_F_VCAP_IS0_IS0_CONTROL_ACE_UPDATE_CTRL_ACE_UPDATE_CMD(x)  VTSS_ENCODE_BITFIELD(x,1,3)
#define  VTSS_M_VCAP_IS0_IS0_CONTROL_ACE_UPDATE_CTRL_ACE_UPDATE_CMD     VTSS_ENCODE_BITMASK(1,3)
#define  VTSS_X_VCAP_IS0_IS0_CONTROL_ACE_UPDATE_CTRL_ACE_UPDATE_CMD(x)  VTSS_EXTRACT_BITFIELD(x,1,3)

/** 
 * \brief
 * The operation specified in ACE_UPDATE_CMD is initiated when setting this
 * field to 1. Once the bit is set to 1, it remains 1 until the operation
 * is completed.
 *
 * \details 
 * 0 : Idle
 * 1 : Perform operation specified in ACE_UPDATE_CMD
 *
 * Field: ::VTSS_VCAP_IS0_IS0_CONTROL_ACE_UPDATE_CTRL . ACE_UPDATE_SHOT
 */
#define  VTSS_F_VCAP_IS0_IS0_CONTROL_ACE_UPDATE_CTRL_ACE_UPDATE_SHOT  VTSS_BIT(0)


/** 
 * \brief Advanced Move/Copy Operations
 *
 * \details
 * Register: \a VCAP_IS0:IS0_CONTROL:ACE_MV_CFG
 */
#define VTSS_VCAP_IS0_IS0_CONTROL_ACE_MV_CFG  VTSS_IOREG(VTSS_TO_VCAP_IS0,0x2)

/** 
 * \brief
 * This field holds the number of positions the entries and/or actions must
 * be moved up or down during a MOVE operation.
 *
 * \details 
 * 0x0 : No entries or actions are moved.
 * 0x1 : Each entry and/or action that is to be moved is moved 1 position
 * up or down.
 * 0x2 : Each entry and/or action that is to be moved is moved 2 positions
 * up or down.
 * 0x3 : Each entry and/or action that is to be moved is moved 3 positions
 * up or down.
 * ...
 *
 * Field: ::VTSS_VCAP_IS0_IS0_CONTROL_ACE_MV_CFG . ACE_MV_NUM_POS
 */
#define  VTSS_F_VCAP_IS0_IS0_CONTROL_ACE_MV_CFG_ACE_MV_NUM_POS(x)  VTSS_ENCODE_BITFIELD(x,16,13)
#define  VTSS_M_VCAP_IS0_IS0_CONTROL_ACE_MV_CFG_ACE_MV_NUM_POS     VTSS_ENCODE_BITMASK(16,13)
#define  VTSS_X_VCAP_IS0_IS0_CONTROL_ACE_MV_CFG_ACE_MV_NUM_POS(x)  VTSS_EXTRACT_BITFIELD(x,16,13)

/** 
 * \brief
 * Enables or disables resetting of the action hit counter
 * (INGRESS::ACE_CNT.HIT_CNT / EGRESS::ACE_CNT.HIT_CNT) at the new action
 * position in the RAM when an action is moved one or more positions up or
 * down during a MOVE operation.
 *
 * \details 
 * 0:  Hit counter bit is not cleared.
 * 1:  Hit counter bit is cleared at the new action position.
 *
 * Field: ::VTSS_VCAP_IS0_IS0_CONTROL_ACE_MV_CFG . ACE_MV_RST_HIT_STATUS_ENA
 */
#define  VTSS_F_VCAP_IS0_IS0_CONTROL_ACE_MV_CFG_ACE_MV_RST_HIT_STATUS_ENA  VTSS_BIT(15)

/** 
 * \brief
 * This field holds the number of entries and/or actions that must be moved
 * up or down during a MOVE operation.
 *
 * \details 
 * 0x0: No entry or action is moved.
 * 0x1: The entry and/or action at address ACE_UPDATE_CTRL.ACE_INDEX is
 * moved up or down.
 * 0x2: The entries and/or actions at address ACE_UPDATE_CTRL.ACE_INDEX
 * through ACE_UPDATE_CTRL.ACE_INDEX+1 are moved up or down.
 * 0x3: The entries and/or actions at address ACE_UPDATE_CTRL.ACE_INDEX
 * through ACE_UPDATE_CTRL.ACE_INDEX+2 are moved up or down.
 * ...
 *
 * Field: ::VTSS_VCAP_IS0_IS0_CONTROL_ACE_MV_CFG . ACE_MV_SIZE
 */
#define  VTSS_F_VCAP_IS0_IS0_CONTROL_ACE_MV_CFG_ACE_MV_SIZE(x)  VTSS_ENCODE_BITFIELD(x,0,13)
#define  VTSS_M_VCAP_IS0_IS0_CONTROL_ACE_MV_CFG_ACE_MV_SIZE     VTSS_ENCODE_BITMASK(0,13)
#define  VTSS_X_VCAP_IS0_IS0_CONTROL_ACE_MV_CFG_ACE_MV_SIZE(x)  VTSS_EXTRACT_BITFIELD(x,0,13)


/** 
 * \brief Entry/Action Cache Status
 *
 * \details
 * Register: \a VCAP_IS0:IS0_CONTROL:ACE_STATUS
 */
#define VTSS_VCAP_IS0_IS0_CONTROL_ACE_STATUS  VTSS_IOREG(VTSS_TO_VCAP_IS0,0x3)

/** 
 * \brief
 * This field indicates the type of entry that is currently in the entry
 * cache.
 * The ACE_ENTRY_TYPE field must be read before the entry fields associated
 * with the given entry type can be read from the cache.
 *
 * \details 
 * The ACE entry types are assigned an increasing number in the order
 * described in a later chapter.
 *
 * Field: ::VTSS_VCAP_IS0_IS0_CONTROL_ACE_STATUS . ACE_ENTRY_TYPE
 */
#define  VTSS_F_VCAP_IS0_IS0_CONTROL_ACE_STATUS_ACE_ENTRY_TYPE(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_VCAP_IS0_IS0_CONTROL_ACE_STATUS_ACE_ENTRY_TYPE     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_VCAP_IS0_IS0_CONTROL_ACE_STATUS_ACE_ENTRY_TYPE(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Status Sticky Bits
 *
 * \details
 * Register: \a VCAP_IS0:IS0_CONTROL:ACE_STICKY
 */
#define VTSS_VCAP_IS0_IS0_CONTROL_ACE_STICKY  VTSS_IOREG(VTSS_TO_VCAP_IS0,0x4)

/** 
 * \brief
 * This sticky bit is set if a move operation caused one or more entries
 * and associated actions to be moved outside the available address space
 * and thereby deleted.
 *
 * \details 
 * 0 : No entry and associated action was deleted due to a MOVE operation.
 * 1 : One or more entries and associated actions were deleted as a result
 * of a move operation.
 *
 * Field: ::VTSS_VCAP_IS0_IS0_CONTROL_ACE_STICKY . ACE_DEL_STICKY
 */
#define  VTSS_F_VCAP_IS0_IS0_CONTROL_ACE_STICKY_ACE_DEL_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a VCAP_IS0:ISID_ENTRY
 *
 * Entry data for entry type ISID.
 */


/** 
 * \details
 * Register: \a VCAP_IS0:ISID_ENTRY:A
 */
#define VTSS_VCAP_IS0_ISID_ENTRY_A           VTSS_IOREG(VTSS_TO_VCAP_IS0,0x5)

/** 
 * \brief
 * Ingress logical port, possibly virtual port.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_ISID_ENTRY_A . VIGR_PORT
 */
#define  VTSS_F_VCAP_IS0_ISID_ENTRY_A_VIGR_PORT(x)  VTSS_ENCODE_BITFIELD(x,1,8)
#define  VTSS_M_VCAP_IS0_ISID_ENTRY_A_VIGR_PORT     VTSS_ENCODE_BITMASK(1,8)
#define  VTSS_X_VCAP_IS0_ISID_ENTRY_A_VIGR_PORT(x)  VTSS_EXTRACT_BITFIELD(x,1,8)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS0_ISID_ENTRY_A . VLD
 */
#define  VTSS_F_VCAP_IS0_ISID_ENTRY_A_VLD     VTSS_BIT(0)


/** 
 * \details
 * Register: \a VCAP_IS0:ISID_ENTRY:ISID0
 */
#define VTSS_VCAP_IS0_ISID_ENTRY_ISID0       VTSS_IOREG(VTSS_TO_VCAP_IS0,0x6)

/** 
 * \brief
 * Qualified by BUNDLING_TAGGED.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_ISID_ENTRY_ISID0 . BUND_DEI
 */
#define  VTSS_F_VCAP_IS0_ISID_ENTRY_ISID0_BUND_DEI  VTSS_BIT(17)

/** 
 * \brief
 * Qualified by BUNDLING_TAGGED.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_ISID_ENTRY_ISID0 . BUND_PCP
 */
#define  VTSS_F_VCAP_IS0_ISID_ENTRY_ISID0_BUND_PCP(x)  VTSS_ENCODE_BITFIELD(x,14,3)
#define  VTSS_M_VCAP_IS0_ISID_ENTRY_ISID0_BUND_PCP     VTSS_ENCODE_BITMASK(14,3)
#define  VTSS_X_VCAP_IS0_ISID_ENTRY_ISID0_BUND_PCP(x)  VTSS_EXTRACT_BITFIELD(x,14,3)

/** 
 * \brief
 * Qualified by BUNDLING_TAGGED.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_ISID_ENTRY_ISID0 . BUND_VID
 */
#define  VTSS_F_VCAP_IS0_ISID_ENTRY_ISID0_BUND_VID(x)  VTSS_ENCODE_BITFIELD(x,2,12)
#define  VTSS_M_VCAP_IS0_ISID_ENTRY_ISID0_BUND_VID     VTSS_ENCODE_BITMASK(2,12)
#define  VTSS_X_VCAP_IS0_ISID_ENTRY_ISID0_BUND_VID(x)  VTSS_EXTRACT_BITFIELD(x,2,12)

/** 
 * \brief
 * Qualified by BUNDLING_TAGGED. Type length field of the bundling TAG.
 *
 * \details 
 * '0': Customer TAG
 * '1': Sevice TAG
 *
 * Field: ::VTSS_VCAP_IS0_ISID_ENTRY_ISID0 . BUND_TPI
 */
#define  VTSS_F_VCAP_IS0_ISID_ENTRY_ISID0_BUND_TPI  VTSS_BIT(1)

/** 
 * \brief
 * Qualifier for presense of bundling TAG in I-TAGGED frames.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_ISID_ENTRY_ISID0 . BUNDLING_TAGGED
 */
#define  VTSS_F_VCAP_IS0_ISID_ENTRY_ISID0_BUNDLING_TAGGED  VTSS_BIT(0)


/** 
 * \details
 * Register: \a VCAP_IS0:ISID_ENTRY:ISID1
 */
#define VTSS_VCAP_IS0_ISID_ENTRY_ISID1       VTSS_IOREG(VTSS_TO_VCAP_IS0,0x7)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS0_ISID_ENTRY_ISID1 . ITAG_RES1
 */
#define  VTSS_F_VCAP_IS0_ISID_ENTRY_ISID1_ITAG_RES1  VTSS_BIT(5)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS0_ISID_ENTRY_ISID1 . ITAG_NCA
 */
#define  VTSS_F_VCAP_IS0_ISID_ENTRY_ISID1_ITAG_NCA  VTSS_BIT(4)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS0_ISID_ENTRY_ISID1 . ITAG_DEI
 */
#define  VTSS_F_VCAP_IS0_ISID_ENTRY_ISID1_ITAG_DEI  VTSS_BIT(3)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS0_ISID_ENTRY_ISID1 . ITAG_PCP
 */
#define  VTSS_F_VCAP_IS0_ISID_ENTRY_ISID1_ITAG_PCP(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_VCAP_IS0_ISID_ENTRY_ISID1_ITAG_PCP     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_VCAP_IS0_ISID_ENTRY_ISID1_ITAG_PCP(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \details
 * Register: \a VCAP_IS0:ISID_ENTRY:ISID2
 */
#define VTSS_VCAP_IS0_ISID_ENTRY_ISID2       VTSS_IOREG(VTSS_TO_VCAP_IS0,0x8)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS0_ISID_ENTRY_ISID2 . ITAG_ISID
 */
#define  VTSS_F_VCAP_IS0_ISID_ENTRY_ISID2_ITAG_ISID(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_VCAP_IS0_ISID_ENTRY_ISID2_ITAG_ISID     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_VCAP_IS0_ISID_ENTRY_ISID2_ITAG_ISID(x)  VTSS_EXTRACT_BITFIELD(x,0,24)

/**
 * Register Group: \a VCAP_IS0:DBL_VID_ENTRY
 *
 * Entry data for entry type DBL_VID.
 */


/** 
 * \details
 * Register: \a VCAP_IS0:DBL_VID_ENTRY:A
 */
#define VTSS_VCAP_IS0_DBL_VID_ENTRY_A        VTSS_IOREG(VTSS_TO_VCAP_IS0,0x9)

/** 
 * \brief
 * Ingress logical port, possibly virtual port.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_DBL_VID_ENTRY_A . VIGR_PORT
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_ENTRY_A_VIGR_PORT(x)  VTSS_ENCODE_BITFIELD(x,1,8)
#define  VTSS_M_VCAP_IS0_DBL_VID_ENTRY_A_VIGR_PORT     VTSS_ENCODE_BITMASK(1,8)
#define  VTSS_X_VCAP_IS0_DBL_VID_ENTRY_A_VIGR_PORT(x)  VTSS_EXTRACT_BITFIELD(x,1,8)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS0_DBL_VID_ENTRY_A . VLD
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_ENTRY_A_VLD  VTSS_BIT(0)


/** 
 * \details
 * Register: \a VCAP_IS0:DBL_VID_ENTRY:DBL_VID0
 */
#define VTSS_VCAP_IS0_DBL_VID_ENTRY_DBL_VID0  VTSS_IOREG(VTSS_TO_VCAP_IS0,0xa)

/** 
 * \brief
 * Qualified by VLAN_TAGGED.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_DBL_VID_ENTRY_DBL_VID0 . OUTER_DEI
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_ENTRY_DBL_VID0_OUTER_DEI  VTSS_BIT(11)

/** 
 * \brief
 * Qualified by VLAN_TAGGED.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_DBL_VID_ENTRY_DBL_VID0 . OUTER_PCP
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_ENTRY_DBL_VID0_OUTER_PCP(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_VCAP_IS0_DBL_VID_ENTRY_DBL_VID0_OUTER_PCP     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_VCAP_IS0_DBL_VID_ENTRY_DBL_VID0_OUTER_PCP(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * Qualified by VLAN_DBL_TAGGED.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_DBL_VID_ENTRY_DBL_VID0 . INNER_DEI
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_ENTRY_DBL_VID0_INNER_DEI  VTSS_BIT(7)

/** 
 * \brief
 * Qualified by VLAN_DBL_TAGGED.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_DBL_VID_ENTRY_DBL_VID0 . INNER_PCP
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_ENTRY_DBL_VID0_INNER_PCP(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_VCAP_IS0_DBL_VID_ENTRY_DBL_VID0_INNER_PCP     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_VCAP_IS0_DBL_VID_ENTRY_DBL_VID0_INNER_PCP(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * Qualified by VLAN_TAGGED. Type length field of the outer TAG.
 *
 * \details 
 * '0': Customer TAG
 * '1': Sevice TAG
 *
 * Field: ::VTSS_VCAP_IS0_DBL_VID_ENTRY_DBL_VID0 . OUTER_TPI
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_ENTRY_DBL_VID0_OUTER_TPI  VTSS_BIT(3)

/** 
 * \brief
 * Qualified by VLAN_DBL_TAGGED. Type length field of the inner TAG.
 *
 * \details 
 * '0': Customer TAG
 * '1': Sevice TAG
 *
 * Field: ::VTSS_VCAP_IS0_DBL_VID_ENTRY_DBL_VID0 . INNER_TPI
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_ENTRY_DBL_VID0_INNER_TPI  VTSS_BIT(2)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS0_DBL_VID_ENTRY_DBL_VID0 . VLAN_DBL_TAGGED
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_ENTRY_DBL_VID0_VLAN_DBL_TAGGED  VTSS_BIT(1)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS0_DBL_VID_ENTRY_DBL_VID0 . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_ENTRY_DBL_VID0_VLAN_TAGGED  VTSS_BIT(0)


/** 
 * \details
 * Register: \a VCAP_IS0:DBL_VID_ENTRY:DBL_VID1
 */
#define VTSS_VCAP_IS0_DBL_VID_ENTRY_DBL_VID1  VTSS_IOREG(VTSS_TO_VCAP_IS0,0xb)

/** 
 * \brief
 * Qualified by VLAN_TAGGED.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_DBL_VID_ENTRY_DBL_VID1 . OUTER_VID
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_ENTRY_DBL_VID1_OUTER_VID(x)  VTSS_ENCODE_BITFIELD(x,12,12)
#define  VTSS_M_VCAP_IS0_DBL_VID_ENTRY_DBL_VID1_OUTER_VID     VTSS_ENCODE_BITMASK(12,12)
#define  VTSS_X_VCAP_IS0_DBL_VID_ENTRY_DBL_VID1_OUTER_VID(x)  VTSS_EXTRACT_BITFIELD(x,12,12)

/** 
 * \brief
 * Qualified by VLAN_DBL_TAGGED.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_DBL_VID_ENTRY_DBL_VID1 . INNER_VID
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_ENTRY_DBL_VID1_INNER_VID(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_VCAP_IS0_DBL_VID_ENTRY_DBL_VID1_INNER_VID     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_VCAP_IS0_DBL_VID_ENTRY_DBL_VID1_INNER_VID(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \details
 * Register: \a VCAP_IS0:DBL_VID_ENTRY:DBL_VID2
 */
#define VTSS_VCAP_IS0_DBL_VID_ENTRY_DBL_VID2  VTSS_IOREG(VTSS_TO_VCAP_IS0,0xc)

/** 
 * \brief
 * DSCP value from IP TOS field. Either directly from frame or remapped in
 * basic classification.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_DBL_VID_ENTRY_DBL_VID2 . L3_DSCP
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_ENTRY_DBL_VID2_L3_DSCP(x)  VTSS_ENCODE_BITFIELD(x,2,6)
#define  VTSS_M_VCAP_IS0_DBL_VID_ENTRY_DBL_VID2_L3_DSCP     VTSS_ENCODE_BITMASK(2,6)
#define  VTSS_X_VCAP_IS0_DBL_VID_ENTRY_DBL_VID2_L3_DSCP(x)  VTSS_EXTRACT_BITFIELD(x,2,6)

/** 
 * \details 
 * 0: NON-IP
 * 1: IPv4
 * 2: IPv6
 *
 * Field: ::VTSS_VCAP_IS0_DBL_VID_ENTRY_DBL_VID2 . PROT
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_ENTRY_DBL_VID2_PROT(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_VCAP_IS0_DBL_VID_ENTRY_DBL_VID2_PROT     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_VCAP_IS0_DBL_VID_ENTRY_DBL_VID2_PROT(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

/**
 * Register Group: \a VCAP_IS0:MPLS_ENTRY
 *
 * Entry data for entry type MPLS.
 */


/** 
 * \details
 * Register: \a VCAP_IS0:MPLS_ENTRY:A
 */
#define VTSS_VCAP_IS0_MPLS_ENTRY_A           VTSS_IOREG(VTSS_TO_VCAP_IS0,0xd)

/** 
 * \brief
 * Ingress logical port, possibly virtual port.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_MPLS_ENTRY_A . VIGR_PORT
 */
#define  VTSS_F_VCAP_IS0_MPLS_ENTRY_A_VIGR_PORT(x)  VTSS_ENCODE_BITFIELD(x,1,8)
#define  VTSS_M_VCAP_IS0_MPLS_ENTRY_A_VIGR_PORT     VTSS_ENCODE_BITMASK(1,8)
#define  VTSS_X_VCAP_IS0_MPLS_ENTRY_A_VIGR_PORT(x)  VTSS_EXTRACT_BITFIELD(x,1,8)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS0_MPLS_ENTRY_A . VLD
 */
#define  VTSS_F_VCAP_IS0_MPLS_ENTRY_A_VLD     VTSS_BIT(0)


/** 
 * \details
 * Register: \a VCAP_IS0:MPLS_ENTRY:MPLS0
 */
#define VTSS_VCAP_IS0_MPLS_ENTRY_MPLS0       VTSS_IOREG(VTSS_TO_VCAP_IS0,0xe)

/** 
 * \brief
 * Inner MPLs label will be set to 0xFFFFFF if not present.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_MPLS_ENTRY_MPLS0 . INNER_MPLS
 */
#define  VTSS_F_VCAP_IS0_MPLS_ENTRY_MPLS0_INNER_MPLS(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_VCAP_IS0_MPLS_ENTRY_MPLS0_INNER_MPLS     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_VCAP_IS0_MPLS_ENTRY_MPLS0_INNER_MPLS(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \details
 * Register: \a VCAP_IS0:MPLS_ENTRY:MPLS1
 */
#define VTSS_VCAP_IS0_MPLS_ENTRY_MPLS1       VTSS_IOREG(VTSS_TO_VCAP_IS0,0xf)

/** 
 * \brief
 * Outer MPLS label.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_MPLS_ENTRY_MPLS1 . OUTER_MPLS
 */
#define  VTSS_F_VCAP_IS0_MPLS_ENTRY_MPLS1_OUTER_MPLS(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_VCAP_IS0_MPLS_ENTRY_MPLS1_OUTER_MPLS     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_VCAP_IS0_MPLS_ENTRY_MPLS1_OUTER_MPLS(x)  VTSS_EXTRACT_BITFIELD(x,0,24)

/**
 * Register Group: \a VCAP_IS0:MAC_ADDR_ENTRY
 *
 * Entry data for entry type MAC_ADDR.
 */


/** 
 * \details
 * Register: \a VCAP_IS0:MAC_ADDR_ENTRY:A
 */
#define VTSS_VCAP_IS0_MAC_ADDR_ENTRY_A       VTSS_IOREG(VTSS_TO_VCAP_IS0,0x11)

/** 
 * \brief
 * Ingress logical port, possibly virtual port.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_MAC_ADDR_ENTRY_A . VIGR_PORT
 */
#define  VTSS_F_VCAP_IS0_MAC_ADDR_ENTRY_A_VIGR_PORT(x)  VTSS_ENCODE_BITFIELD(x,1,8)
#define  VTSS_M_VCAP_IS0_MAC_ADDR_ENTRY_A_VIGR_PORT     VTSS_ENCODE_BITMASK(1,8)
#define  VTSS_X_VCAP_IS0_MAC_ADDR_ENTRY_A_VIGR_PORT(x)  VTSS_EXTRACT_BITFIELD(x,1,8)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS0_MAC_ADDR_ENTRY_A . VLD
 */
#define  VTSS_F_VCAP_IS0_MAC_ADDR_ENTRY_A_VLD  VTSS_BIT(0)


/** 
 * \details
 * Register: \a VCAP_IS0:MAC_ADDR_ENTRY:MAC_ADDR0
 */
#define VTSS_VCAP_IS0_MAC_ADDR_ENTRY_MAC_ADDR0  VTSS_IOREG(VTSS_TO_VCAP_IS0,0x12)

/** 
 * \brief
 * This can either be SMAC or DMAC based on a port configuration.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_MAC_ADDR_ENTRY_MAC_ADDR0 . L2_SMAC_HIGH
 */
#define  VTSS_F_VCAP_IS0_MAC_ADDR_ENTRY_MAC_ADDR0_L2_SMAC_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_VCAP_IS0_MAC_ADDR_ENTRY_MAC_ADDR0_L2_SMAC_HIGH     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_VCAP_IS0_MAC_ADDR_ENTRY_MAC_ADDR0_L2_SMAC_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \details
 * Register: \a VCAP_IS0:MAC_ADDR_ENTRY:MAC_ADDR1
 */
#define VTSS_VCAP_IS0_MAC_ADDR_ENTRY_MAC_ADDR1  VTSS_IOREG(VTSS_TO_VCAP_IS0,0x13)

/** 
 * \brief
 * This can either be SMAC or DMAC based on a port configuration.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_MAC_ADDR_ENTRY_MAC_ADDR1 . L2_SMAC_LOW
 */
#define  VTSS_F_VCAP_IS0_MAC_ADDR_ENTRY_MAC_ADDR1_L2_SMAC_LOW(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_VCAP_IS0_MAC_ADDR_ENTRY_MAC_ADDR1_L2_SMAC_LOW     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_VCAP_IS0_MAC_ADDR_ENTRY_MAC_ADDR1_L2_SMAC_LOW(x)  VTSS_EXTRACT_BITFIELD(x,0,24)

/**
 * Register Group: \a VCAP_IS0:ISID_MASK
 *
 * Entry mask for entry type ISID.
 */


/** 
 * \brief Mask register corresponding to ISID_ENTRY:A
 *
 * \details
 * Mask register corresponding to ISID_ENTRY:A
 *
 * Register: \a VCAP_IS0:ISID_MASK:A
 */
#define VTSS_VCAP_IS0_ISID_MASK_A            VTSS_IOREG(VTSS_TO_VCAP_IS0,0x15)

/** 
 * \brief
 * Mask for field ISID_ENTRY:A.VIGR_PORT
 *
 * \details 
 * '0': Entry field VIGR_PORT must match
 * '1': Entry field VIGR_PORT is ignored
 *
 * Field: ::VTSS_VCAP_IS0_ISID_MASK_A . VIGR_PORT
 */
#define  VTSS_F_VCAP_IS0_ISID_MASK_A_VIGR_PORT(x)  VTSS_ENCODE_BITFIELD(x,1,8)
#define  VTSS_M_VCAP_IS0_ISID_MASK_A_VIGR_PORT     VTSS_ENCODE_BITMASK(1,8)
#define  VTSS_X_VCAP_IS0_ISID_MASK_A_VIGR_PORT(x)  VTSS_EXTRACT_BITFIELD(x,1,8)

/** 
 * \brief
 * Mask has to be valid always.
 *
 * \details 
 * Must be set to '0'
 *
 * Field: ::VTSS_VCAP_IS0_ISID_MASK_A . VLD
 */
#define  VTSS_F_VCAP_IS0_ISID_MASK_A_VLD      VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to ISID_ENTRY:ISID0
 *
 * \details
 * Mask register corresponding to ISID_ENTRY:ISID0
 *
 * Register: \a VCAP_IS0:ISID_MASK:ISID0
 */
#define VTSS_VCAP_IS0_ISID_MASK_ISID0        VTSS_IOREG(VTSS_TO_VCAP_IS0,0x16)

/** 
 * \brief
 * Mask for field ISID_ENTRY:ISID0.BUND_DEI
 *
 * \details 
 * '0': Entry field BUND_DEI must match
 * '1': Entry field BUND_DEI is ignored
 *
 * Field: ::VTSS_VCAP_IS0_ISID_MASK_ISID0 . BUND_DEI
 */
#define  VTSS_F_VCAP_IS0_ISID_MASK_ISID0_BUND_DEI  VTSS_BIT(17)

/** 
 * \brief
 * Mask for field ISID_ENTRY:ISID0.BUND_PCP
 *
 * \details 
 * '0': Entry field BUND_PCP must match
 * '1': Entry field BUND_PCP is ignored
 *
 * Field: ::VTSS_VCAP_IS0_ISID_MASK_ISID0 . BUND_PCP
 */
#define  VTSS_F_VCAP_IS0_ISID_MASK_ISID0_BUND_PCP(x)  VTSS_ENCODE_BITFIELD(x,14,3)
#define  VTSS_M_VCAP_IS0_ISID_MASK_ISID0_BUND_PCP     VTSS_ENCODE_BITMASK(14,3)
#define  VTSS_X_VCAP_IS0_ISID_MASK_ISID0_BUND_PCP(x)  VTSS_EXTRACT_BITFIELD(x,14,3)

/** 
 * \brief
 * Mask for field ISID_ENTRY:ISID0.BUND_VID
 *
 * \details 
 * '0': Entry field BUND_VID must match
 * '1': Entry field BUND_VID is ignored
 *
 * Field: ::VTSS_VCAP_IS0_ISID_MASK_ISID0 . BUND_VID
 */
#define  VTSS_F_VCAP_IS0_ISID_MASK_ISID0_BUND_VID(x)  VTSS_ENCODE_BITFIELD(x,2,12)
#define  VTSS_M_VCAP_IS0_ISID_MASK_ISID0_BUND_VID     VTSS_ENCODE_BITMASK(2,12)
#define  VTSS_X_VCAP_IS0_ISID_MASK_ISID0_BUND_VID(x)  VTSS_EXTRACT_BITFIELD(x,2,12)

/** 
 * \brief
 * Mask for field ISID_ENTRY:ISID0.BUND_TPI
 *
 * \details 
 * '0': Entry field BUND_TPI must match
 * '1': Entry field BUND_TPI is ignored
 *
 * Field: ::VTSS_VCAP_IS0_ISID_MASK_ISID0 . BUND_TPI
 */
#define  VTSS_F_VCAP_IS0_ISID_MASK_ISID0_BUND_TPI  VTSS_BIT(1)

/** 
 * \brief
 * Mask for field ISID_ENTRY:ISID0.BUNDLING_TAGGED
 *
 * \details 
 * '0': Entry field BUNDLING_TAGGED must match
 * '1': Entry field BUNDLING_TAGGED is ignored
 *
 * Field: ::VTSS_VCAP_IS0_ISID_MASK_ISID0 . BUNDLING_TAGGED
 */
#define  VTSS_F_VCAP_IS0_ISID_MASK_ISID0_BUNDLING_TAGGED  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to ISID_ENTRY:ISID1
 *
 * \details
 * Mask register corresponding to ISID_ENTRY:ISID1
 *
 * Register: \a VCAP_IS0:ISID_MASK:ISID1
 */
#define VTSS_VCAP_IS0_ISID_MASK_ISID1        VTSS_IOREG(VTSS_TO_VCAP_IS0,0x17)

/** 
 * \brief
 * Mask for field ISID_ENTRY:ISID1.ITAG_RES1
 *
 * \details 
 * '0': Entry field ITAG_RES1 must match
 * '1': Entry field ITAG_RES1 is ignored
 *
 * Field: ::VTSS_VCAP_IS0_ISID_MASK_ISID1 . ITAG_RES1
 */
#define  VTSS_F_VCAP_IS0_ISID_MASK_ISID1_ITAG_RES1  VTSS_BIT(5)

/** 
 * \brief
 * Mask for field ISID_ENTRY:ISID1.ITAG_NCA
 *
 * \details 
 * '0': Entry field ITAG_NCA must match
 * '1': Entry field ITAG_NCA is ignored
 *
 * Field: ::VTSS_VCAP_IS0_ISID_MASK_ISID1 . ITAG_NCA
 */
#define  VTSS_F_VCAP_IS0_ISID_MASK_ISID1_ITAG_NCA  VTSS_BIT(4)

/** 
 * \brief
 * Mask for field ISID_ENTRY:ISID1.ITAG_DEI
 *
 * \details 
 * '0': Entry field ITAG_DEI must match
 * '1': Entry field ITAG_DEI is ignored
 *
 * Field: ::VTSS_VCAP_IS0_ISID_MASK_ISID1 . ITAG_DEI
 */
#define  VTSS_F_VCAP_IS0_ISID_MASK_ISID1_ITAG_DEI  VTSS_BIT(3)

/** 
 * \brief
 * Mask for field ISID_ENTRY:ISID1.ITAG_PCP
 *
 * \details 
 * '0': Entry field ITAG_PCP must match
 * '1': Entry field ITAG_PCP is ignored
 *
 * Field: ::VTSS_VCAP_IS0_ISID_MASK_ISID1 . ITAG_PCP
 */
#define  VTSS_F_VCAP_IS0_ISID_MASK_ISID1_ITAG_PCP(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_VCAP_IS0_ISID_MASK_ISID1_ITAG_PCP     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_VCAP_IS0_ISID_MASK_ISID1_ITAG_PCP(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Mask register corresponding to ISID_ENTRY:ISID2
 *
 * \details
 * Mask register corresponding to ISID_ENTRY:ISID2
 *
 * Register: \a VCAP_IS0:ISID_MASK:ISID2
 */
#define VTSS_VCAP_IS0_ISID_MASK_ISID2        VTSS_IOREG(VTSS_TO_VCAP_IS0,0x18)

/** 
 * \brief
 * Mask for field ISID_ENTRY:ISID2.ITAG_ISID
 *
 * \details 
 * '0': Entry field ITAG_ISID must match
 * '1': Entry field ITAG_ISID is ignored
 *
 * Field: ::VTSS_VCAP_IS0_ISID_MASK_ISID2 . ITAG_ISID
 */
#define  VTSS_F_VCAP_IS0_ISID_MASK_ISID2_ITAG_ISID(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_VCAP_IS0_ISID_MASK_ISID2_ITAG_ISID     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_VCAP_IS0_ISID_MASK_ISID2_ITAG_ISID(x)  VTSS_EXTRACT_BITFIELD(x,0,24)

/**
 * Register Group: \a VCAP_IS0:DBL_VID_MASK
 *
 * Entry mask for entry type DBL_VID.
 */


/** 
 * \brief Mask register corresponding to DBL_VID_ENTRY:A
 *
 * \details
 * Mask register corresponding to DBL_VID_ENTRY:A
 *
 * Register: \a VCAP_IS0:DBL_VID_MASK:A
 */
#define VTSS_VCAP_IS0_DBL_VID_MASK_A         VTSS_IOREG(VTSS_TO_VCAP_IS0,0x19)

/** 
 * \brief
 * Mask for field DBL_VID_ENTRY:A.VIGR_PORT
 *
 * \details 
 * '0': Entry field VIGR_PORT must match
 * '1': Entry field VIGR_PORT is ignored
 *
 * Field: ::VTSS_VCAP_IS0_DBL_VID_MASK_A . VIGR_PORT
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_MASK_A_VIGR_PORT(x)  VTSS_ENCODE_BITFIELD(x,1,8)
#define  VTSS_M_VCAP_IS0_DBL_VID_MASK_A_VIGR_PORT     VTSS_ENCODE_BITMASK(1,8)
#define  VTSS_X_VCAP_IS0_DBL_VID_MASK_A_VIGR_PORT(x)  VTSS_EXTRACT_BITFIELD(x,1,8)

/** 
 * \brief
 * Mask has to be valid always.
 *
 * \details 
 * Must be set to '0'
 *
 * Field: ::VTSS_VCAP_IS0_DBL_VID_MASK_A . VLD
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_MASK_A_VLD   VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to DBL_VID_ENTRY:DBL_VID0
 *
 * \details
 * Mask register corresponding to DBL_VID_ENTRY:DBL_VID0
 *
 * Register: \a VCAP_IS0:DBL_VID_MASK:DBL_VID0
 */
#define VTSS_VCAP_IS0_DBL_VID_MASK_DBL_VID0  VTSS_IOREG(VTSS_TO_VCAP_IS0,0x1a)

/** 
 * \brief
 * Mask for field DBL_VID_ENTRY:DBL_VID0.OUTER_DEI
 *
 * \details 
 * '0': Entry field OUTER_DEI must match
 * '1': Entry field OUTER_DEI is ignored
 *
 * Field: ::VTSS_VCAP_IS0_DBL_VID_MASK_DBL_VID0 . OUTER_DEI
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_MASK_DBL_VID0_OUTER_DEI  VTSS_BIT(11)

/** 
 * \brief
 * Mask for field DBL_VID_ENTRY:DBL_VID0.OUTER_PCP
 *
 * \details 
 * '0': Entry field OUTER_PCP must match
 * '1': Entry field OUTER_PCP is ignored
 *
 * Field: ::VTSS_VCAP_IS0_DBL_VID_MASK_DBL_VID0 . OUTER_PCP
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_MASK_DBL_VID0_OUTER_PCP(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_VCAP_IS0_DBL_VID_MASK_DBL_VID0_OUTER_PCP     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_VCAP_IS0_DBL_VID_MASK_DBL_VID0_OUTER_PCP(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * Mask for field DBL_VID_ENTRY:DBL_VID0.INNER_DEI
 *
 * \details 
 * '0': Entry field INNER_DEI must match
 * '1': Entry field INNER_DEI is ignored
 *
 * Field: ::VTSS_VCAP_IS0_DBL_VID_MASK_DBL_VID0 . INNER_DEI
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_MASK_DBL_VID0_INNER_DEI  VTSS_BIT(7)

/** 
 * \brief
 * Mask for field DBL_VID_ENTRY:DBL_VID0.INNER_PCP
 *
 * \details 
 * '0': Entry field INNER_PCP must match
 * '1': Entry field INNER_PCP is ignored
 *
 * Field: ::VTSS_VCAP_IS0_DBL_VID_MASK_DBL_VID0 . INNER_PCP
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_MASK_DBL_VID0_INNER_PCP(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_VCAP_IS0_DBL_VID_MASK_DBL_VID0_INNER_PCP     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_VCAP_IS0_DBL_VID_MASK_DBL_VID0_INNER_PCP(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * Mask for field DBL_VID_ENTRY:DBL_VID0.OUTER_TPI
 *
 * \details 
 * '0': Entry field OUTER_TPI must match
 * '1': Entry field OUTER_TPI is ignored
 *
 * Field: ::VTSS_VCAP_IS0_DBL_VID_MASK_DBL_VID0 . OUTER_TPI
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_MASK_DBL_VID0_OUTER_TPI  VTSS_BIT(3)

/** 
 * \brief
 * Mask for field DBL_VID_ENTRY:DBL_VID0.INNER_TPI
 *
 * \details 
 * '0': Entry field INNER_TPI must match
 * '1': Entry field INNER_TPI is ignored
 *
 * Field: ::VTSS_VCAP_IS0_DBL_VID_MASK_DBL_VID0 . INNER_TPI
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_MASK_DBL_VID0_INNER_TPI  VTSS_BIT(2)

/** 
 * \brief
 * Mask for field DBL_VID_ENTRY:DBL_VID0.VLAN_DBL_TAGGED
 *
 * \details 
 * '0': Entry field VLAN_DBL_TAGGED must match
 * '1': Entry field VLAN_DBL_TAGGED is ignored
 *
 * Field: ::VTSS_VCAP_IS0_DBL_VID_MASK_DBL_VID0 . VLAN_DBL_TAGGED
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_MASK_DBL_VID0_VLAN_DBL_TAGGED  VTSS_BIT(1)

/** 
 * \brief
 * Mask for field DBL_VID_ENTRY:DBL_VID0.VLAN_TAGGED
 *
 * \details 
 * '0': Entry field VLAN_TAGGED must match
 * '1': Entry field VLAN_TAGGED is ignored
 *
 * Field: ::VTSS_VCAP_IS0_DBL_VID_MASK_DBL_VID0 . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_MASK_DBL_VID0_VLAN_TAGGED  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to DBL_VID_ENTRY:DBL_VID1
 *
 * \details
 * Mask register corresponding to DBL_VID_ENTRY:DBL_VID1
 *
 * Register: \a VCAP_IS0:DBL_VID_MASK:DBL_VID1
 */
#define VTSS_VCAP_IS0_DBL_VID_MASK_DBL_VID1  VTSS_IOREG(VTSS_TO_VCAP_IS0,0x1b)

/** 
 * \brief
 * Mask for field DBL_VID_ENTRY:DBL_VID1.OUTER_VID
 *
 * \details 
 * '0': Entry field OUTER_VID must match
 * '1': Entry field OUTER_VID is ignored
 *
 * Field: ::VTSS_VCAP_IS0_DBL_VID_MASK_DBL_VID1 . OUTER_VID
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_MASK_DBL_VID1_OUTER_VID(x)  VTSS_ENCODE_BITFIELD(x,12,12)
#define  VTSS_M_VCAP_IS0_DBL_VID_MASK_DBL_VID1_OUTER_VID     VTSS_ENCODE_BITMASK(12,12)
#define  VTSS_X_VCAP_IS0_DBL_VID_MASK_DBL_VID1_OUTER_VID(x)  VTSS_EXTRACT_BITFIELD(x,12,12)

/** 
 * \brief
 * Mask for field DBL_VID_ENTRY:DBL_VID1.INNER_VID
 *
 * \details 
 * '0': Entry field INNER_VID must match
 * '1': Entry field INNER_VID is ignored
 *
 * Field: ::VTSS_VCAP_IS0_DBL_VID_MASK_DBL_VID1 . INNER_VID
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_MASK_DBL_VID1_INNER_VID(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_VCAP_IS0_DBL_VID_MASK_DBL_VID1_INNER_VID     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_VCAP_IS0_DBL_VID_MASK_DBL_VID1_INNER_VID(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \brief Mask register corresponding to DBL_VID_ENTRY:DBL_VID2
 *
 * \details
 * Mask register corresponding to DBL_VID_ENTRY:DBL_VID2
 *
 * Register: \a VCAP_IS0:DBL_VID_MASK:DBL_VID2
 */
#define VTSS_VCAP_IS0_DBL_VID_MASK_DBL_VID2  VTSS_IOREG(VTSS_TO_VCAP_IS0,0x1c)

/** 
 * \brief
 * Mask for field DBL_VID_ENTRY:DBL_VID2.L3_DSCP
 *
 * \details 
 * '0': Entry field L3_DSCP must match
 * '1': Entry field L3_DSCP is ignored
 *
 * Field: ::VTSS_VCAP_IS0_DBL_VID_MASK_DBL_VID2 . L3_DSCP
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_MASK_DBL_VID2_L3_DSCP(x)  VTSS_ENCODE_BITFIELD(x,2,6)
#define  VTSS_M_VCAP_IS0_DBL_VID_MASK_DBL_VID2_L3_DSCP     VTSS_ENCODE_BITMASK(2,6)
#define  VTSS_X_VCAP_IS0_DBL_VID_MASK_DBL_VID2_L3_DSCP(x)  VTSS_EXTRACT_BITFIELD(x,2,6)

/** 
 * \brief
 * Mask for field DBL_VID_ENTRY:DBL_VID2.PROT
 *
 * \details 
 * '0': Entry field PROT must match
 * '1': Entry field PROT is ignored
 *
 * Field: ::VTSS_VCAP_IS0_DBL_VID_MASK_DBL_VID2 . PROT
 */
#define  VTSS_F_VCAP_IS0_DBL_VID_MASK_DBL_VID2_PROT(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_VCAP_IS0_DBL_VID_MASK_DBL_VID2_PROT     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_VCAP_IS0_DBL_VID_MASK_DBL_VID2_PROT(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

/**
 * Register Group: \a VCAP_IS0:MPLS_MASK
 *
 * Entry mask for entry type MPLS.
 */


/** 
 * \brief Mask register corresponding to MPLS_ENTRY:A
 *
 * \details
 * Mask register corresponding to MPLS_ENTRY:A
 *
 * Register: \a VCAP_IS0:MPLS_MASK:A
 */
#define VTSS_VCAP_IS0_MPLS_MASK_A            VTSS_IOREG(VTSS_TO_VCAP_IS0,0x1d)

/** 
 * \brief
 * Mask for field MPLS_ENTRY:A.VIGR_PORT
 *
 * \details 
 * '0': Entry field VIGR_PORT must match
 * '1': Entry field VIGR_PORT is ignored
 *
 * Field: ::VTSS_VCAP_IS0_MPLS_MASK_A . VIGR_PORT
 */
#define  VTSS_F_VCAP_IS0_MPLS_MASK_A_VIGR_PORT(x)  VTSS_ENCODE_BITFIELD(x,1,8)
#define  VTSS_M_VCAP_IS0_MPLS_MASK_A_VIGR_PORT     VTSS_ENCODE_BITMASK(1,8)
#define  VTSS_X_VCAP_IS0_MPLS_MASK_A_VIGR_PORT(x)  VTSS_EXTRACT_BITFIELD(x,1,8)

/** 
 * \brief
 * Mask has to be valid always.
 *
 * \details 
 * Must be set to '0'
 *
 * Field: ::VTSS_VCAP_IS0_MPLS_MASK_A . VLD
 */
#define  VTSS_F_VCAP_IS0_MPLS_MASK_A_VLD      VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to MPLS_ENTRY:MPLS0
 *
 * \details
 * Mask register corresponding to MPLS_ENTRY:MPLS0
 *
 * Register: \a VCAP_IS0:MPLS_MASK:MPLS0
 */
#define VTSS_VCAP_IS0_MPLS_MASK_MPLS0        VTSS_IOREG(VTSS_TO_VCAP_IS0,0x1e)

/** 
 * \brief
 * Mask for field MPLS_ENTRY:MPLS0.INNER_MPLS
 *
 * \details 
 * '0': Entry field INNER_MPLS must match
 * '1': Entry field INNER_MPLS is ignored
 *
 * Field: ::VTSS_VCAP_IS0_MPLS_MASK_MPLS0 . INNER_MPLS
 */
#define  VTSS_F_VCAP_IS0_MPLS_MASK_MPLS0_INNER_MPLS(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_VCAP_IS0_MPLS_MASK_MPLS0_INNER_MPLS     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_VCAP_IS0_MPLS_MASK_MPLS0_INNER_MPLS(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief Mask register corresponding to MPLS_ENTRY:MPLS1
 *
 * \details
 * Mask register corresponding to MPLS_ENTRY:MPLS1
 *
 * Register: \a VCAP_IS0:MPLS_MASK:MPLS1
 */
#define VTSS_VCAP_IS0_MPLS_MASK_MPLS1        VTSS_IOREG(VTSS_TO_VCAP_IS0,0x1f)

/** 
 * \brief
 * Mask for field MPLS_ENTRY:MPLS1.OUTER_MPLS
 *
 * \details 
 * '0': Entry field OUTER_MPLS must match
 * '1': Entry field OUTER_MPLS is ignored
 *
 * Field: ::VTSS_VCAP_IS0_MPLS_MASK_MPLS1 . OUTER_MPLS
 */
#define  VTSS_F_VCAP_IS0_MPLS_MASK_MPLS1_OUTER_MPLS(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_VCAP_IS0_MPLS_MASK_MPLS1_OUTER_MPLS     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_VCAP_IS0_MPLS_MASK_MPLS1_OUTER_MPLS(x)  VTSS_EXTRACT_BITFIELD(x,0,24)

/**
 * Register Group: \a VCAP_IS0:MAC_ADDR_MASK
 *
 * Entry mask for entry type MAC_ADDR.
 */


/** 
 * \brief Mask register corresponding to MAC_ADDR_ENTRY:A
 *
 * \details
 * Mask register corresponding to MAC_ADDR_ENTRY:A
 *
 * Register: \a VCAP_IS0:MAC_ADDR_MASK:A
 */
#define VTSS_VCAP_IS0_MAC_ADDR_MASK_A        VTSS_IOREG(VTSS_TO_VCAP_IS0,0x21)

/** 
 * \brief
 * Mask for field MAC_ADDR_ENTRY:A.VIGR_PORT
 *
 * \details 
 * '0': Entry field VIGR_PORT must match
 * '1': Entry field VIGR_PORT is ignored
 *
 * Field: ::VTSS_VCAP_IS0_MAC_ADDR_MASK_A . VIGR_PORT
 */
#define  VTSS_F_VCAP_IS0_MAC_ADDR_MASK_A_VIGR_PORT(x)  VTSS_ENCODE_BITFIELD(x,1,8)
#define  VTSS_M_VCAP_IS0_MAC_ADDR_MASK_A_VIGR_PORT     VTSS_ENCODE_BITMASK(1,8)
#define  VTSS_X_VCAP_IS0_MAC_ADDR_MASK_A_VIGR_PORT(x)  VTSS_EXTRACT_BITFIELD(x,1,8)

/** 
 * \brief
 * Mask has to be valid always.
 *
 * \details 
 * Must be set to '0'
 *
 * Field: ::VTSS_VCAP_IS0_MAC_ADDR_MASK_A . VLD
 */
#define  VTSS_F_VCAP_IS0_MAC_ADDR_MASK_A_VLD  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to MAC_ADDR_ENTRY:MAC_ADDR0
 *
 * \details
 * Mask register corresponding to MAC_ADDR_ENTRY:MAC_ADDR0
 *
 * Register: \a VCAP_IS0:MAC_ADDR_MASK:MAC_ADDR0
 */
#define VTSS_VCAP_IS0_MAC_ADDR_MASK_MAC_ADDR0  VTSS_IOREG(VTSS_TO_VCAP_IS0,0x22)

/** 
 * \brief
 * Mask for field MAC_ADDR_ENTRY:MAC_ADDR0.L2_SMAC_HIGH
 *
 * \details 
 * '0': Entry field L2_SMAC_HIGH must match
 * '1': Entry field L2_SMAC_HIGH is ignored
 *
 * Field: ::VTSS_VCAP_IS0_MAC_ADDR_MASK_MAC_ADDR0 . L2_SMAC_HIGH
 */
#define  VTSS_F_VCAP_IS0_MAC_ADDR_MASK_MAC_ADDR0_L2_SMAC_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_VCAP_IS0_MAC_ADDR_MASK_MAC_ADDR0_L2_SMAC_HIGH     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_VCAP_IS0_MAC_ADDR_MASK_MAC_ADDR0_L2_SMAC_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief Mask register corresponding to MAC_ADDR_ENTRY:MAC_ADDR1
 *
 * \details
 * Mask register corresponding to MAC_ADDR_ENTRY:MAC_ADDR1
 *
 * Register: \a VCAP_IS0:MAC_ADDR_MASK:MAC_ADDR1
 */
#define VTSS_VCAP_IS0_MAC_ADDR_MASK_MAC_ADDR1  VTSS_IOREG(VTSS_TO_VCAP_IS0,0x23)

/** 
 * \brief
 * Mask for field MAC_ADDR_ENTRY:MAC_ADDR1.L2_SMAC_LOW
 *
 * \details 
 * '0': Entry field L2_SMAC_LOW must match
 * '1': Entry field L2_SMAC_LOW is ignored
 *
 * Field: ::VTSS_VCAP_IS0_MAC_ADDR_MASK_MAC_ADDR1 . L2_SMAC_LOW
 */
#define  VTSS_F_VCAP_IS0_MAC_ADDR_MASK_MAC_ADDR1_L2_SMAC_LOW(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_VCAP_IS0_MAC_ADDR_MASK_MAC_ADDR1_L2_SMAC_LOW     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_VCAP_IS0_MAC_ADDR_MASK_MAC_ADDR1_L2_SMAC_LOW(x)  VTSS_EXTRACT_BITFIELD(x,0,24)

/**
 * Register Group: \a VCAP_IS0:BASETYPE_ACTION
 *
 * ACL VLAN table action registers
 */


/** 
 * \details
 * Register: \a VCAP_IS0:BASETYPE_ACTION:A
 */
#define VTSS_VCAP_IS0_BASETYPE_ACTION_A      VTSS_IOREG(VTSS_TO_VCAP_IS0,0x25)

/** 
 * \brief
 * Enable use of DMAC, DIP instead of SMAC, SIP in VCAP S1.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_BASETYPE_ACTION_A . S1_DMAC_ENA
 */
#define  VTSS_F_VCAP_IS0_BASETYPE_ACTION_A_S1_DMAC_ENA  VTSS_BIT(13)

/** 
 * \brief
 * This field disables L3 based forwarding.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_BASETYPE_ACTION_A . L3_MC_LOOKUP_DIS
 */
#define  VTSS_F_VCAP_IS0_BASETYPE_ACTION_A_L3_MC_LOOKUP_DIS  VTSS_BIT(12)

/** 
 * \brief
 * This field disables OAM detection in VCAP S2.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_BASETYPE_ACTION_A . OAM_DETECT_DIS
 */
#define  VTSS_F_VCAP_IS0_BASETYPE_ACTION_A_OAM_DETECT_DIS  VTSS_BIT(11)

/** 
 * \brief
 * This field is only used for DLB_VID entries.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_BASETYPE_ACTION_A . SECOND_LOOKUP_DIS
 */
#define  VTSS_F_VCAP_IS0_BASETYPE_ACTION_A_SECOND_LOOKUP_DIS  VTSS_BIT(10)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS0_BASETYPE_ACTION_A . MAC_POP_CNT
 */
#define  VTSS_F_VCAP_IS0_BASETYPE_ACTION_A_MAC_POP_CNT  VTSS_BIT(9)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS0_BASETYPE_ACTION_A . VLAN_POP_CNT
 */
#define  VTSS_F_VCAP_IS0_BASETYPE_ACTION_A_VLAN_POP_CNT(x)  VTSS_ENCODE_BITFIELD(x,7,2)
#define  VTSS_M_VCAP_IS0_BASETYPE_ACTION_A_VLAN_POP_CNT     VTSS_ENCODE_BITMASK(7,2)
#define  VTSS_X_VCAP_IS0_BASETYPE_ACTION_A_VLAN_POP_CNT(x)  VTSS_EXTRACT_BITFIELD(x,7,2)

/** 
 * \brief
 * If set, VID_VAL issued as BVID.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_BASETYPE_ACTION_A . BVID_ENA
 */
#define  VTSS_F_VCAP_IS0_BASETYPE_ACTION_A_BVID_ENA  VTSS_BIT(6)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS0_BASETYPE_ACTION_A . VID_ENA
 */
#define  VTSS_F_VCAP_IS0_BASETYPE_ACTION_A_VID_ENA  VTSS_BIT(5)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS0_BASETYPE_ACTION_A . DEI_VAL
 */
#define  VTSS_F_VCAP_IS0_BASETYPE_ACTION_A_DEI_VAL  VTSS_BIT(4)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS0_BASETYPE_ACTION_A . PCP_VAL
 */
#define  VTSS_F_VCAP_IS0_BASETYPE_ACTION_A_PCP_VAL(x)  VTSS_ENCODE_BITFIELD(x,1,3)
#define  VTSS_M_VCAP_IS0_BASETYPE_ACTION_A_PCP_VAL     VTSS_ENCODE_BITMASK(1,3)
#define  VTSS_X_VCAP_IS0_BASETYPE_ACTION_A_PCP_VAL(x)  VTSS_EXTRACT_BITFIELD(x,1,3)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS0_BASETYPE_ACTION_A . PCP_DEI_ENA
 */
#define  VTSS_F_VCAP_IS0_BASETYPE_ACTION_A_PCP_DEI_ENA  VTSS_BIT(0)


/** 
 * \details
 * Register: \a VCAP_IS0:BASETYPE_ACTION:B
 */
#define VTSS_VCAP_IS0_BASETYPE_ACTION_B      VTSS_IOREG(VTSS_TO_VCAP_IS0,0x26)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS0_BASETYPE_ACTION_B . ISDX_VAL
 */
#define  VTSS_F_VCAP_IS0_BASETYPE_ACTION_B_ISDX_VAL(x)  VTSS_ENCODE_BITFIELD(x,20,12)
#define  VTSS_M_VCAP_IS0_BASETYPE_ACTION_B_ISDX_VAL     VTSS_ENCODE_BITMASK(20,12)
#define  VTSS_X_VCAP_IS0_BASETYPE_ACTION_B_ISDX_VAL(x)  VTSS_EXTRACT_BITFIELD(x,20,12)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS0_BASETYPE_ACTION_B . VID_VAL
 */
#define  VTSS_F_VCAP_IS0_BASETYPE_ACTION_B_VID_VAL(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS0_BASETYPE_ACTION_B_VID_VAL     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS0_BASETYPE_ACTION_B_VID_VAL(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS0_BASETYPE_ACTION_B . PAG_VAL
 */
#define  VTSS_F_VCAP_IS0_BASETYPE_ACTION_B_PAG_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS0_BASETYPE_ACTION_B_PAG_VAL     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS0_BASETYPE_ACTION_B_PAG_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \details
 * Register: \a VCAP_IS0:BASETYPE_ACTION:C
 */
#define VTSS_VCAP_IS0_BASETYPE_ACTION_C      VTSS_IOREG(VTSS_TO_VCAP_IS0,0x27)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS0_BASETYPE_ACTION_C . HIT_STICKY
 */
#define  VTSS_F_VCAP_IS0_BASETYPE_ACTION_C_HIT_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a VCAP_IS0:TCAM_BIST
 *
 * Build in test for TCAM
 */


/** 
 * \brief Control of the TCAM
 *
 * \details
 * Register: \a VCAP_IS0:TCAM_BIST:TCAM_CTRL
 */
#define VTSS_VCAP_IS0_TCAM_BIST_TCAM_CTRL    VTSS_IOREG(VTSS_TO_VCAP_IS0,0x29)

/** 
 * \brief
 * Set this field to start manual BIST of the TCAM. This field will be
 * cleared once BIST is complete. The BIST procedure requires that the TCAM
 * is initialized before start, setting TCAM_INIT at the same time as
 * setting this field will first initalize the TCAM and then run BIST.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_TCAM_BIST_TCAM_CTRL . TCAM_BIST
 */
#define  VTSS_F_VCAP_IS0_TCAM_BIST_TCAM_CTRL_TCAM_BIST  VTSS_BIT(1)

/** 
 * \brief
 * Set this field to start manual initialization of the TCAM. This field is
 * cleared once initialization is complete.
 * The TCAM has random contents after reset and must be initialized prior
 * to usage.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_TCAM_BIST_TCAM_CTRL . TCAM_INIT
 */
#define  VTSS_F_VCAP_IS0_TCAM_BIST_TCAM_CTRL_TCAM_INIT  VTSS_BIT(0)


/** 
 * \brief Status for the TCAM
 *
 * \details
 * Register: \a VCAP_IS0:TCAM_BIST:TCAM_STAT
 */
#define VTSS_VCAP_IS0_TCAM_BIST_TCAM_STAT    VTSS_IOREG(VTSS_TO_VCAP_IS0,0x2c)

/** 
 * \brief
 * Set if BIST failed.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_TCAM_BIST_TCAM_STAT . BIST_ERR
 */
#define  VTSS_F_VCAP_IS0_TCAM_BIST_TCAM_STAT_BIST_ERR  VTSS_BIT(2)

/** 
 * \brief
 * Set when the BIST is still running. When checking the BIST result this
 * field must be cleared.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS0_TCAM_BIST_TCAM_STAT . BIST_BUSY
 */
#define  VTSS_F_VCAP_IS0_TCAM_BIST_TCAM_STAT_BIST_BUSY  VTSS_BIT(1)

/** 
 * \brief
 * Indicates the current operational state of the TCAM.
 *
 * \details 
 * '0': Busy with initialization.
 * '1': Ready to be used.
 *
 * Field: ::VTSS_VCAP_IS0_TCAM_BIST_TCAM_STAT . TCAM_RDY
 */
#define  VTSS_F_VCAP_IS0_TCAM_BIST_TCAM_STAT_TCAM_RDY  VTSS_BIT(0)


#endif /* _VTSS_JAGUAR_REGS_VCAP_IS0_H_ */
