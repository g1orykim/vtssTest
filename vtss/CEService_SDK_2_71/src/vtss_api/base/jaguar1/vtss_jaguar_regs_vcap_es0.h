#ifndef _VTSS_JAGUAR_REGS_VCAP_ES0_H_
#define _VTSS_JAGUAR_REGS_VCAP_ES0_H_

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
 * Target: \a VCAP_ES0
 *
 * Egress per Service or VLAN frame manipulations block.
 *
 ***********************************************************************/

/**
 * Register Group: \a VCAP_ES0:ES0_CONTROL
 *
 * Miscellaneous Configuration Registers for VCAP
 */


/** 
 * \brief Miscellaneous VCAP Configuration
 *
 * \details
 * Register: \a VCAP_ES0:ES0_CONTROL:ACL_CFG
 */
#define VTSS_VCAP_ES0_ES0_CONTROL_ACL_CFG    VTSS_IOREG(VTSS_TO_VCAP_ES0,0x0)

/** 
 * \brief
 * Enables or disables use of VCAP frame processing. The VCAP can still be
 * configured even though VCAP frame processing is disabled.
 *
 * \details 
 * 0:  VCAP frame processing is disabled.
 * 1:  VCAP frame processing is enabled.
 *
 * Field: ::VTSS_VCAP_ES0_ES0_CONTROL_ACL_CFG . ACL_ENA
 */
#define  VTSS_F_VCAP_ES0_ES0_CONTROL_ACL_CFG_ACL_ENA  VTSS_BIT(0)


/** 
 * \brief Entry and Action Control
 *
 * \details
 * Register: \a VCAP_ES0:ES0_CONTROL:ACE_UPDATE_CTRL
 */
#define VTSS_VCAP_ES0_ES0_CONTROL_ACE_UPDATE_CTRL  VTSS_IOREG(VTSS_TO_VCAP_ES0,0x1)

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
 * Field: ::VTSS_VCAP_ES0_ES0_CONTROL_ACE_UPDATE_CTRL . ACE_INDEX
 */
#define  VTSS_F_VCAP_ES0_ES0_CONTROL_ACE_UPDATE_CTRL_ACE_INDEX(x)  VTSS_ENCODE_BITFIELD(x,4,13)
#define  VTSS_M_VCAP_ES0_ES0_CONTROL_ACE_UPDATE_CTRL_ACE_INDEX     VTSS_ENCODE_BITMASK(4,13)
#define  VTSS_X_VCAP_ES0_ES0_CONTROL_ACE_UPDATE_CTRL_ACE_INDEX(x)  VTSS_EXTRACT_BITFIELD(x,4,13)

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
 * Field: ::VTSS_VCAP_ES0_ES0_CONTROL_ACE_UPDATE_CTRL . ACE_UPDATE_CMD
 */
#define  VTSS_F_VCAP_ES0_ES0_CONTROL_ACE_UPDATE_CTRL_ACE_UPDATE_CMD(x)  VTSS_ENCODE_BITFIELD(x,1,3)
#define  VTSS_M_VCAP_ES0_ES0_CONTROL_ACE_UPDATE_CTRL_ACE_UPDATE_CMD     VTSS_ENCODE_BITMASK(1,3)
#define  VTSS_X_VCAP_ES0_ES0_CONTROL_ACE_UPDATE_CTRL_ACE_UPDATE_CMD(x)  VTSS_EXTRACT_BITFIELD(x,1,3)

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
 * Field: ::VTSS_VCAP_ES0_ES0_CONTROL_ACE_UPDATE_CTRL . ACE_UPDATE_SHOT
 */
#define  VTSS_F_VCAP_ES0_ES0_CONTROL_ACE_UPDATE_CTRL_ACE_UPDATE_SHOT  VTSS_BIT(0)


/** 
 * \brief Advanced Move/Copy Operations
 *
 * \details
 * Register: \a VCAP_ES0:ES0_CONTROL:ACE_MV_CFG
 */
#define VTSS_VCAP_ES0_ES0_CONTROL_ACE_MV_CFG  VTSS_IOREG(VTSS_TO_VCAP_ES0,0x2)

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
 * Field: ::VTSS_VCAP_ES0_ES0_CONTROL_ACE_MV_CFG . ACE_MV_NUM_POS
 */
#define  VTSS_F_VCAP_ES0_ES0_CONTROL_ACE_MV_CFG_ACE_MV_NUM_POS(x)  VTSS_ENCODE_BITFIELD(x,16,13)
#define  VTSS_M_VCAP_ES0_ES0_CONTROL_ACE_MV_CFG_ACE_MV_NUM_POS     VTSS_ENCODE_BITMASK(16,13)
#define  VTSS_X_VCAP_ES0_ES0_CONTROL_ACE_MV_CFG_ACE_MV_NUM_POS(x)  VTSS_EXTRACT_BITFIELD(x,16,13)

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
 * Field: ::VTSS_VCAP_ES0_ES0_CONTROL_ACE_MV_CFG . ACE_MV_RST_HIT_STATUS_ENA
 */
#define  VTSS_F_VCAP_ES0_ES0_CONTROL_ACE_MV_CFG_ACE_MV_RST_HIT_STATUS_ENA  VTSS_BIT(15)

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
 * Field: ::VTSS_VCAP_ES0_ES0_CONTROL_ACE_MV_CFG . ACE_MV_SIZE
 */
#define  VTSS_F_VCAP_ES0_ES0_CONTROL_ACE_MV_CFG_ACE_MV_SIZE(x)  VTSS_ENCODE_BITFIELD(x,0,13)
#define  VTSS_M_VCAP_ES0_ES0_CONTROL_ACE_MV_CFG_ACE_MV_SIZE     VTSS_ENCODE_BITMASK(0,13)
#define  VTSS_X_VCAP_ES0_ES0_CONTROL_ACE_MV_CFG_ACE_MV_SIZE(x)  VTSS_EXTRACT_BITFIELD(x,0,13)


/** 
 * \brief Entry/Action Cache Status
 *
 * \details
 * Register: \a VCAP_ES0:ES0_CONTROL:ACE_STATUS
 */
#define VTSS_VCAP_ES0_ES0_CONTROL_ACE_STATUS  VTSS_IOREG(VTSS_TO_VCAP_ES0,0x3)

/** 
 * \brief
 * This field indicates the type of action that is currently in the action
 * cache.
 * The ACE_ACTION_TYPE field must be read before the action fields
 * associated with the given action type can be read from the cache.
 *
 * \details 
 * 0x0: The action cache holds an ingress action.
 * 0x1: The action cache holds an egress action.

 *
 * Field: ::VTSS_VCAP_ES0_ES0_CONTROL_ACE_STATUS . ACE_ACTION_TYPE
 */
#define  VTSS_F_VCAP_ES0_ES0_CONTROL_ACE_STATUS_ACE_ACTION_TYPE  VTSS_BIT(4)

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
 * Field: ::VTSS_VCAP_ES0_ES0_CONTROL_ACE_STATUS . ACE_ENTRY_TYPE
 */
#define  VTSS_F_VCAP_ES0_ES0_CONTROL_ACE_STATUS_ACE_ENTRY_TYPE  VTSS_BIT(0)


/** 
 * \brief Status Sticky Bits
 *
 * \details
 * Register: \a VCAP_ES0:ES0_CONTROL:ACE_STICKY
 */
#define VTSS_VCAP_ES0_ES0_CONTROL_ACE_STICKY  VTSS_IOREG(VTSS_TO_VCAP_ES0,0x4)

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
 * Field: ::VTSS_VCAP_ES0_ES0_CONTROL_ACE_STICKY . ACE_DEL_STICKY
 */
#define  VTSS_F_VCAP_ES0_ES0_CONTROL_ACE_STICKY_ACE_DEL_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a VCAP_ES0:ISDX_ENTRY
 *
 * Entry data for entry type ISDX.
 */


/** 
 * \details
 * Register: \a VCAP_ES0:ISDX_ENTRY:A
 */
#define VTSS_VCAP_ES0_ISDX_ENTRY_A           VTSS_IOREG(VTSS_TO_VCAP_ES0,0x5)

/** 
 * \brief
 * Set if ISDX is different from 0.
 *
 * \details 
 * Field: ::VTSS_VCAP_ES0_ISDX_ENTRY_A . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_ES0_ISDX_ENTRY_A_ISDX_NEQ0  VTSS_BIT(9)

/** 
 * \brief
 * Egress port number or virtual port.
 *
 * \details 
 * Field: ::VTSS_VCAP_ES0_ISDX_ENTRY_A . VEGR_PORT
 */
#define  VTSS_F_VCAP_ES0_ISDX_ENTRY_A_VEGR_PORT(x)  VTSS_ENCODE_BITFIELD(x,1,8)
#define  VTSS_M_VCAP_ES0_ISDX_ENTRY_A_VEGR_PORT     VTSS_ENCODE_BITMASK(1,8)
#define  VTSS_X_VCAP_ES0_ISDX_ENTRY_A_VEGR_PORT(x)  VTSS_EXTRACT_BITFIELD(x,1,8)

/** 
 * \brief
 * Set if entry is valid. Only valid entries can be 'hit'.
 *
 * \details 
 * '0' : Disabled
 * '1' : Enabled
 *
 * Field: ::VTSS_VCAP_ES0_ISDX_ENTRY_A . VLD
 */
#define  VTSS_F_VCAP_ES0_ISDX_ENTRY_A_VLD     VTSS_BIT(0)


/** 
 * \details
 * Register: \a VCAP_ES0:ISDX_ENTRY:ISDX1
 */
#define VTSS_VCAP_ES0_ISDX_ENTRY_ISDX1       VTSS_IOREG(VTSS_TO_VCAP_ES0,0x6)

/** 
 * \brief
 * Ingress Service Index
 *
 * \details 
 * Field: ::VTSS_VCAP_ES0_ISDX_ENTRY_ISDX1 . ISDX
 */
#define  VTSS_F_VCAP_ES0_ISDX_ENTRY_ISDX1_ISDX(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_VCAP_ES0_ISDX_ENTRY_ISDX1_ISDX     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_VCAP_ES0_ISDX_ENTRY_ISDX1_ISDX(x)  VTSS_EXTRACT_BITFIELD(x,0,12)

/**
 * Register Group: \a VCAP_ES0:VID_ENTRY
 *
 * Entry data for entry type VID.
 */


/** 
 * \details
 * Register: \a VCAP_ES0:VID_ENTRY:A
 */
#define VTSS_VCAP_ES0_VID_ENTRY_A            VTSS_IOREG(VTSS_TO_VCAP_ES0,0x7)

/** 
 * \brief
 * Set if ISDX is different from 0.
 *
 * \details 
 * Field: ::VTSS_VCAP_ES0_VID_ENTRY_A . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_ES0_VID_ENTRY_A_ISDX_NEQ0  VTSS_BIT(9)

/** 
 * \brief
 * Egress port number or virtual port.
 *
 * \details 
 * Field: ::VTSS_VCAP_ES0_VID_ENTRY_A . VEGR_PORT
 */
#define  VTSS_F_VCAP_ES0_VID_ENTRY_A_VEGR_PORT(x)  VTSS_ENCODE_BITFIELD(x,1,8)
#define  VTSS_M_VCAP_ES0_VID_ENTRY_A_VEGR_PORT     VTSS_ENCODE_BITMASK(1,8)
#define  VTSS_X_VCAP_ES0_VID_ENTRY_A_VEGR_PORT(x)  VTSS_EXTRACT_BITFIELD(x,1,8)

/** 
 * \brief
 * Set if entry is valid. Only valid entries can be 'hit'.
 *
 * \details 
 * '0' : Disabled
 * '1' : Enabled
 *
 * Field: ::VTSS_VCAP_ES0_VID_ENTRY_A . VLD
 */
#define  VTSS_F_VCAP_ES0_VID_ENTRY_A_VLD      VTSS_BIT(0)


/** 
 * \details
 * Register: \a VCAP_ES0:VID_ENTRY:VID1
 */
#define VTSS_VCAP_ES0_VID_ENTRY_VID1         VTSS_IOREG(VTSS_TO_VCAP_ES0,0x8)

/** 
 * \brief
 * Classified VID (Will be classified I-VID for service frames (ISDX != 0).
 * For non-service frames (ISDX = 0), this will be CL-BVID.
 *
 * \details 
 * Field: ::VTSS_VCAP_ES0_VID_ENTRY_VID1 . VID
 */
#define  VTSS_F_VCAP_ES0_VID_ENTRY_VID1_VID(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_VCAP_ES0_VID_ENTRY_VID1_VID     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_VCAP_ES0_VID_ENTRY_VID1_VID(x)  VTSS_EXTRACT_BITFIELD(x,0,12)

/**
 * Register Group: \a VCAP_ES0:ISDX_MASK
 *
 * Entry mask for entry type ISDX.
 */


/** 
 * \brief Mask register corresponding to ISDX_ENTRY:A
 *
 * \details
 * Mask register corresponding to ISDX_ENTRY:A
 *
 * Register: \a VCAP_ES0:ISDX_MASK:A
 */
#define VTSS_VCAP_ES0_ISDX_MASK_A            VTSS_IOREG(VTSS_TO_VCAP_ES0,0x9)

/** 
 * \brief
 * Mask for field ISDX_ENTRY:A.ISDX_NEQ0
 *
 * \details 
 * '0': Entry field ISDX_NEQ0 must match
 * '1': Entry field ISDX_NEQ0 is ignored
 *
 * Field: ::VTSS_VCAP_ES0_ISDX_MASK_A . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_ES0_ISDX_MASK_A_ISDX_NEQ0  VTSS_BIT(9)

/** 
 * \brief
 * Mask for field ISDX_ENTRY:A.VEGR_PORT
 *
 * \details 
 * '0': Entry field VEGR_PORT must match
 * '1': Entry field VEGR_PORT is ignored
 *
 * Field: ::VTSS_VCAP_ES0_ISDX_MASK_A . VEGR_PORT
 */
#define  VTSS_F_VCAP_ES0_ISDX_MASK_A_VEGR_PORT(x)  VTSS_ENCODE_BITFIELD(x,1,8)
#define  VTSS_M_VCAP_ES0_ISDX_MASK_A_VEGR_PORT     VTSS_ENCODE_BITMASK(1,8)
#define  VTSS_X_VCAP_ES0_ISDX_MASK_A_VEGR_PORT(x)  VTSS_EXTRACT_BITFIELD(x,1,8)

/** 
 * \brief
 * Mask has to be valid always.
 *
 * \details 
 * Must be set to '0'
 *
 * Field: ::VTSS_VCAP_ES0_ISDX_MASK_A . VLD
 */
#define  VTSS_F_VCAP_ES0_ISDX_MASK_A_VLD      VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to ISDX_ENTRY:ISDX1
 *
 * \details
 * Mask register corresponding to ISDX_ENTRY:ISDX1
 *
 * Register: \a VCAP_ES0:ISDX_MASK:ISDX1
 */
#define VTSS_VCAP_ES0_ISDX_MASK_ISDX1        VTSS_IOREG(VTSS_TO_VCAP_ES0,0xa)

/** 
 * \brief
 * Mask for field ISDX_ENTRY:ISDX1.ISDX
 *
 * \details 
 * '0': Entry field ISDX must match
 * '1': Entry field ISDX is ignored
 *
 * Field: ::VTSS_VCAP_ES0_ISDX_MASK_ISDX1 . ISDX
 */
#define  VTSS_F_VCAP_ES0_ISDX_MASK_ISDX1_ISDX(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_VCAP_ES0_ISDX_MASK_ISDX1_ISDX     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_VCAP_ES0_ISDX_MASK_ISDX1_ISDX(x)  VTSS_EXTRACT_BITFIELD(x,0,12)

/**
 * Register Group: \a VCAP_ES0:VID_MASK
 *
 * Entry mask for entry type VID.
 */


/** 
 * \brief Mask register corresponding to VID_ENTRY:A
 *
 * \details
 * Mask register corresponding to VID_ENTRY:A
 *
 * Register: \a VCAP_ES0:VID_MASK:A
 */
#define VTSS_VCAP_ES0_VID_MASK_A             VTSS_IOREG(VTSS_TO_VCAP_ES0,0xb)

/** 
 * \brief
 * Mask for field VID_ENTRY:A.ISDX_NEQ0
 *
 * \details 
 * '0': Entry field ISDX_NEQ0 must match
 * '1': Entry field ISDX_NEQ0 is ignored
 *
 * Field: ::VTSS_VCAP_ES0_VID_MASK_A . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_ES0_VID_MASK_A_ISDX_NEQ0  VTSS_BIT(9)

/** 
 * \brief
 * Mask for field VID_ENTRY:A.VEGR_PORT
 *
 * \details 
 * '0': Entry field VEGR_PORT must match
 * '1': Entry field VEGR_PORT is ignored
 *
 * Field: ::VTSS_VCAP_ES0_VID_MASK_A . VEGR_PORT
 */
#define  VTSS_F_VCAP_ES0_VID_MASK_A_VEGR_PORT(x)  VTSS_ENCODE_BITFIELD(x,1,8)
#define  VTSS_M_VCAP_ES0_VID_MASK_A_VEGR_PORT     VTSS_ENCODE_BITMASK(1,8)
#define  VTSS_X_VCAP_ES0_VID_MASK_A_VEGR_PORT(x)  VTSS_EXTRACT_BITFIELD(x,1,8)

/** 
 * \brief
 * Mask has to be valid always.
 *
 * \details 
 * Must be set to '0'
 *
 * Field: ::VTSS_VCAP_ES0_VID_MASK_A . VLD
 */
#define  VTSS_F_VCAP_ES0_VID_MASK_A_VLD       VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to VID_ENTRY:VID1
 *
 * \details
 * Mask register corresponding to VID_ENTRY:VID1
 *
 * Register: \a VCAP_ES0:VID_MASK:VID1
 */
#define VTSS_VCAP_ES0_VID_MASK_VID1          VTSS_IOREG(VTSS_TO_VCAP_ES0,0xc)

/** 
 * \brief
 * Mask for field VID_ENTRY:VID1.VID
 *
 * \details 
 * '0': Entry field VID must match
 * '1': Entry field VID is ignored
 *
 * Field: ::VTSS_VCAP_ES0_VID_MASK_VID1 . VID
 */
#define  VTSS_F_VCAP_ES0_VID_MASK_VID1_VID(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_VCAP_ES0_VID_MASK_VID1_VID     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_VCAP_ES0_VID_MASK_VID1_VID(x)  VTSS_EXTRACT_BITFIELD(x,0,12)

/**
 * Register Group: \a VCAP_ES0:MACINMAC_ACTION
 *
 * Not documented
 */


/** 
 * \details
 * Register: \a VCAP_ES0:MACINMAC_ACTION:A
 */
#define VTSS_VCAP_ES0_MACINMAC_ACTION_A      VTSS_IOREG(VTSS_TO_VCAP_ES0,0xd)

/** 
 * \details 
 * Field: ::VTSS_VCAP_ES0_MACINMAC_ACTION_A . VLD
 */
#define  VTSS_F_VCAP_ES0_MACINMAC_ACTION_A_VLD  VTSS_BIT(0)


/** 
 * \details
 * Register: \a VCAP_ES0:MACINMAC_ACTION:MACINMAC1
 */
#define VTSS_VCAP_ES0_MACINMAC_ACTION_MACINMAC1  VTSS_IOREG(VTSS_TO_VCAP_ES0,0xe)

/** 
 * \brief
 * Backbone Service Instance Identifier (ISID) to be inserted into the
 * I-TAG of a PBB encapsulated frame.
 *
 * \details 
 * Field: ::VTSS_VCAP_ES0_MACINMAC_ACTION_MACINMAC1 . ISID_VAL
 */
#define  VTSS_F_VCAP_ES0_MACINMAC_ACTION_MACINMAC1_ISID_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_VCAP_ES0_MACINMAC_ACTION_MACINMAC1_ISID_VAL     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_VCAP_ES0_MACINMAC_ACTION_MACINMAC1_ISID_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \details
 * Register: \a VCAP_ES0:MACINMAC_ACTION:MACINMAC2
 */
#define VTSS_VCAP_ES0_MACINMAC_ACTION_MACINMAC2  VTSS_IOREG(VTSS_TO_VCAP_ES0,0xf)

/** 
 * \brief
 * Port Tag VID (BVID) will be set to ES0.VID_A_VAL - unless
 * PORT.TAG_CTRL.VID_SEL = "00"
 *
 * \details 
 * Field: ::VTSS_VCAP_ES0_MACINMAC_ACTION_MACINMAC2 . BVID_ENA
 */
#define  VTSS_F_VCAP_ES0_MACINMAC_ACTION_MACINMAC2_BVID_ENA  VTSS_BIT(11)

/** 
 * \brief
 * Index into B-SMAC address table.
 * 
 * In case of CL-IVID=0xFFF and ITAG_KEEP_ENA=1, this field has a special
 * value.
 * 
 * When the above is true, the following bits are used to select MiM fields
 * to be overwritten:
 * 
 * ITAG_BSA_IDX[0] = 1; Modify frame ISID; ISID = ES0.MIM_HIT.ISID
 * ITAG_BSA_IDX[1] = 1; Modify frame BDA; BDA = IFH.BDA
 * ITAG_BSA_IDX[2] = 1; Modify frame BSA; BSA= BSA source table[0]
 *
 * \details 
 * Field: ::VTSS_VCAP_ES0_MACINMAC_ACTION_MACINMAC2 . ITAG_BSA_IDX
 */
#define  VTSS_F_VCAP_ES0_MACINMAC_ACTION_MACINMAC2_ITAG_BSA_IDX(x)  VTSS_ENCODE_BITFIELD(x,6,5)
#define  VTSS_M_VCAP_ES0_MACINMAC_ACTION_MACINMAC2_ITAG_BSA_IDX     VTSS_ENCODE_BITMASK(6,5)
#define  VTSS_X_VCAP_ES0_MACINMAC_ACTION_MACINMAC2_ITAG_BSA_IDX(x)  VTSS_EXTRACT_BITFIELD(x,6,5)

/** 
 * \brief
 * If frame is MiM encapsulated, do not modify the received MiM
 * encapsulation.
 * This option can be overwritten in case of CL-IVID=0xFFF.
 * 
 * In case of CL-IVID=0xFFF, the value of ITAG_BSA_IDX can  be used to
 * modify certain fields in the MiM encapsulation.
 *
 * \details 
 * Field: ::VTSS_VCAP_ES0_MACINMAC_ACTION_MACINMAC2 . ITAG_KEEP_ENA
 */
#define  VTSS_F_VCAP_ES0_MACINMAC_ACTION_MACINMAC2_ITAG_KEEP_ENA  VTSS_BIT(5)

/** 
 * \brief
 * Selection of source to be used for Bundling Service TAG tag.
 *
 * \details 
 * 0 : 0x8100
 * 1 : 0x88A8
 * 2 : Set to port default TPI (PORT_TAG_DEFAULT.PORT_TPI_ETYPE)
 * 3 : if IFH.VSTAX.TAG.TAG_TYPE = 0 then 0x8100 else 0x88A8 (or custom)
 *
 * Field: ::VTSS_VCAP_ES0_MACINMAC_ACTION_MACINMAC2 . STAG_TPI_SEL
 */
#define  VTSS_F_VCAP_ES0_MACINMAC_ACTION_MACINMAC2_STAG_TPI_SEL(x)  VTSS_ENCODE_BITFIELD(x,3,2)
#define  VTSS_M_VCAP_ES0_MACINMAC_ACTION_MACINMAC2_STAG_TPI_SEL     VTSS_ENCODE_BITMASK(3,2)
#define  VTSS_X_VCAP_ES0_MACINMAC_ACTION_MACINMAC2_STAG_TPI_SEL(x)  VTSS_EXTRACT_BITFIELD(x,3,2)

/** 
 * \brief
 * Selection of source to be used for Bundling Service TAG DEI and PCP
 *
 * \details 
 * 0 : Classified
 * 1 : Fixed to ES0
 * 2 : Fixed to port
 * 3 : Mapped
 *
 * Field: ::VTSS_VCAP_ES0_MACINMAC_ACTION_MACINMAC2 . STAG_QOS_SRC_SEL
 */
#define  VTSS_F_VCAP_ES0_MACINMAC_ACTION_MACINMAC2_STAG_QOS_SRC_SEL(x)  VTSS_ENCODE_BITFIELD(x,1,2)
#define  VTSS_M_VCAP_ES0_MACINMAC_ACTION_MACINMAC2_STAG_QOS_SRC_SEL     VTSS_ENCODE_BITMASK(1,2)
#define  VTSS_X_VCAP_ES0_MACINMAC_ACTION_MACINMAC2_STAG_QOS_SRC_SEL(x)  VTSS_EXTRACT_BITFIELD(x,1,2)

/** 
 * \brief
 * Add Bundling Service TAG (only possible if pop_cnt>0). Note: bundling
 * Service TAG VID will always be set to CL-IVID.
 *
 * \details 
 * Field: ::VTSS_VCAP_ES0_MACINMAC_ACTION_MACINMAC2 . STAG_BUNDLING_ENA
 */
#define  VTSS_F_VCAP_ES0_MACINMAC_ACTION_MACINMAC2_STAG_BUNDLING_ENA  VTSS_BIT(0)


/** 
 * \details
 * Register: \a VCAP_ES0:MACINMAC_ACTION:B
 */
#define VTSS_VCAP_ES0_MACINMAC_ACTION_B      VTSS_IOREG(VTSS_TO_VCAP_ES0,0x10)

/** 
 * \brief
 * VID value. Can be used as a source for Egress TAG VID.
 *
 * \details 
 * Field: ::VTSS_VCAP_ES0_MACINMAC_ACTION_B . VID_A_VAL
 */
#define  VTSS_F_VCAP_ES0_MACINMAC_ACTION_B_VID_A_VAL(x)  VTSS_ENCODE_BITFIELD(x,18,12)
#define  VTSS_M_VCAP_ES0_MACINMAC_ACTION_B_VID_A_VAL     VTSS_ENCODE_BITMASK(18,12)
#define  VTSS_X_VCAP_ES0_MACINMAC_ACTION_B_VID_A_VAL(x)  VTSS_EXTRACT_BITFIELD(x,18,12)

/** 
 * \brief
 * Egress Service Index (Select Egress Statistics Counter)
 *
 * \details 
 * Field: ::VTSS_VCAP_ES0_MACINMAC_ACTION_B . ESDX_VAL
 */
#define  VTSS_F_VCAP_ES0_MACINMAC_ACTION_B_ESDX_VAL(x)  VTSS_ENCODE_BITFIELD(x,6,12)
#define  VTSS_M_VCAP_ES0_MACINMAC_ACTION_B_ESDX_VAL     VTSS_ENCODE_BITMASK(6,12)
#define  VTSS_X_VCAP_ES0_MACINMAC_ACTION_B_ESDX_VAL(x)  VTSS_EXTRACT_BITFIELD(x,6,12)

/** 
 * \brief
 * DEI value. Can be used as a source for Egress Tagging.
 *
 * \details 
 * Field: ::VTSS_VCAP_ES0_MACINMAC_ACTION_B . DEI_VAL
 */
#define  VTSS_F_VCAP_ES0_MACINMAC_ACTION_B_DEI_VAL  VTSS_BIT(5)

/** 
 * \brief
 * PCP value. Can be used as a source for Egress Tagging.
 *
 * \details 
 * Field: ::VTSS_VCAP_ES0_MACINMAC_ACTION_B . PCP_VAL
 */
#define  VTSS_F_VCAP_ES0_MACINMAC_ACTION_B_PCP_VAL(x)  VTSS_ENCODE_BITFIELD(x,2,3)
#define  VTSS_M_VCAP_ES0_MACINMAC_ACTION_B_PCP_VAL     VTSS_ENCODE_BITMASK(2,3)
#define  VTSS_X_VCAP_ES0_MACINMAC_ACTION_B_PCP_VAL(x)  VTSS_EXTRACT_BITFIELD(x,2,3)

/** 
 * \brief
 * Selection of source to be used for DEI and PCP in ES0 tag / ITAG
 * (dependent on action type).
 *
 * \details 
 * 0 : Classified PCP/DEI values
 * 1 : Fixed to ES0
 * 2 : Fixed to port default (PORT_TCI_PRIO/PORT_TCI_CFI)
 * 3 : Use Mapped versions of classified QOS class and DP level
 *
 * Field: ::VTSS_VCAP_ES0_MACINMAC_ACTION_B . QOS_SRC_SEL
 */
#define  VTSS_F_VCAP_ES0_MACINMAC_ACTION_B_QOS_SRC_SEL(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_VCAP_ES0_MACINMAC_ACTION_B_QOS_SRC_SEL     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_VCAP_ES0_MACINMAC_ACTION_B_QOS_SRC_SEL(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Sticky bit status register
 *
 * \details
 * Register: \a VCAP_ES0:MACINMAC_ACTION:C
 */
#define VTSS_VCAP_ES0_MACINMAC_ACTION_C      VTSS_IOREG(VTSS_TO_VCAP_ES0,0x11)

/** 
 * \brief
 * Advanced classification activity bit.
 * 
 * 
 * Used for indirect access to all classification entries.
 *
 * \details 
 * '0' : no activity
 * '1' : rule has been triggered
 *
 * Field: ::VTSS_VCAP_ES0_MACINMAC_ACTION_C . HIT_STICKY
 */
#define  VTSS_F_VCAP_ES0_MACINMAC_ACTION_C_HIT_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a VCAP_ES0:TAG_ACTION
 *
 * Not documented
 */


/** 
 * \details
 * Register: \a VCAP_ES0:TAG_ACTION:A
 */
#define VTSS_VCAP_ES0_TAG_ACTION_A           VTSS_IOREG(VTSS_TO_VCAP_ES0,0x15)

/** 
 * \details 
 * Field: ::VTSS_VCAP_ES0_TAG_ACTION_A . VLD
 */
#define  VTSS_F_VCAP_ES0_TAG_ACTION_A_VLD     VTSS_BIT(0)


/** 
 * \details
 * Register: \a VCAP_ES0:TAG_ACTION:TAG1
 */
#define VTSS_VCAP_ES0_TAG_ACTION_TAG1        VTSS_IOREG(VTSS_TO_VCAP_ES0,0x16)

/** 
 * \brief
 * VID value. Can be used as a source for Service TAG VID. (See
 * TAG_VID_SEL)
 *
 * \details 
 * Field: ::VTSS_VCAP_ES0_TAG_ACTION_TAG1 . VID_B_VAL
 */
#define  VTSS_F_VCAP_ES0_TAG_ACTION_TAG1_VID_B_VAL(x)  VTSS_ENCODE_BITFIELD(x,6,12)
#define  VTSS_M_VCAP_ES0_TAG_ACTION_TAG1_VID_B_VAL     VTSS_ENCODE_BITMASK(6,12)
#define  VTSS_X_VCAP_ES0_TAG_ACTION_TAG1_VID_B_VAL(x)  VTSS_EXTRACT_BITFIELD(x,6,12)

/** 
 * \brief
 * Select VID source for Service tag.
 *
 * \details 
 * 0 : CL-VID (If B-port use CL-BVID otherwise use CL-IVID)
 * 1 : VID_A_VAL
 * 2 : VID_B_VAL
 * 3 : Use Default Port VID (PORT_TCI_VID)
 *
 * Field: ::VTSS_VCAP_ES0_TAG_ACTION_TAG1 . TAG_VID_SEL
 */
#define  VTSS_F_VCAP_ES0_TAG_ACTION_TAG1_TAG_VID_SEL(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_VCAP_ES0_TAG_ACTION_TAG1_TAG_VID_SEL     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_VCAP_ES0_TAG_ACTION_TAG1_TAG_VID_SEL(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * Select TPI for Service tag.
 *
 * \details 
 * 0 : 0x8100
 * 1 : 0x88A8
 * 2 : Set to port default (PORT_TPI_ETYPE)
 * 3 : If ingress ETYPE is C-TAG use 0x8100 otherwise use port default
 * (PORT_TPI_ETYPE)
 *
 * Field: ::VTSS_VCAP_ES0_TAG_ACTION_TAG1 . TAG_TPI_SEL
 */
#define  VTSS_F_VCAP_ES0_TAG_ACTION_TAG1_TAG_TPI_SEL(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_VCAP_ES0_TAG_ACTION_TAG1_TAG_TPI_SEL     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_VCAP_ES0_TAG_ACTION_TAG1_TAG_TPI_SEL(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Control Service tagging.
 *
 * \details 
 * 0 : No ES0 tagging.
 * 1 : Push only ES0 tag. (Supress port tag configuration.)
 * 2 : Push ES0 tag as inner tag, independent of port tag. (Port tag will
 * be applied as configured by port configuration)
 * 3 : Always push two tags, with ES0 tag as inner tag. (Supress port tag
 * configuration.)
 *
 * Field: ::VTSS_VCAP_ES0_TAG_ACTION_TAG1 . TAG_CTRL
 */
#define  VTSS_F_VCAP_ES0_TAG_ACTION_TAG1_TAG_CTRL(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_VCAP_ES0_TAG_ACTION_TAG1_TAG_CTRL     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_VCAP_ES0_TAG_ACTION_TAG1_TAG_CTRL(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \details
 * Register: \a VCAP_ES0:TAG_ACTION:B
 */
#define VTSS_VCAP_ES0_TAG_ACTION_B           VTSS_IOREG(VTSS_TO_VCAP_ES0,0x17)

/** 
 * \brief
 * VID value. Can be used as a source for Egress TAG VID.
 *
 * \details 
 * Field: ::VTSS_VCAP_ES0_TAG_ACTION_B . VID_A_VAL
 */
#define  VTSS_F_VCAP_ES0_TAG_ACTION_B_VID_A_VAL(x)  VTSS_ENCODE_BITFIELD(x,18,12)
#define  VTSS_M_VCAP_ES0_TAG_ACTION_B_VID_A_VAL     VTSS_ENCODE_BITMASK(18,12)
#define  VTSS_X_VCAP_ES0_TAG_ACTION_B_VID_A_VAL(x)  VTSS_EXTRACT_BITFIELD(x,18,12)

/** 
 * \brief
 * Egress Service Index (Select Egress Statistics Counter)
 *
 * \details 
 * Field: ::VTSS_VCAP_ES0_TAG_ACTION_B . ESDX_VAL
 */
#define  VTSS_F_VCAP_ES0_TAG_ACTION_B_ESDX_VAL(x)  VTSS_ENCODE_BITFIELD(x,6,12)
#define  VTSS_M_VCAP_ES0_TAG_ACTION_B_ESDX_VAL     VTSS_ENCODE_BITMASK(6,12)
#define  VTSS_X_VCAP_ES0_TAG_ACTION_B_ESDX_VAL(x)  VTSS_EXTRACT_BITFIELD(x,6,12)

/** 
 * \brief
 * DEI value. Can be used as a source for Egress Tagging.
 *
 * \details 
 * Field: ::VTSS_VCAP_ES0_TAG_ACTION_B . DEI_VAL
 */
#define  VTSS_F_VCAP_ES0_TAG_ACTION_B_DEI_VAL  VTSS_BIT(5)

/** 
 * \brief
 * PCP value. Can be used as a source for Egress Tagging.
 *
 * \details 
 * Field: ::VTSS_VCAP_ES0_TAG_ACTION_B . PCP_VAL
 */
#define  VTSS_F_VCAP_ES0_TAG_ACTION_B_PCP_VAL(x)  VTSS_ENCODE_BITFIELD(x,2,3)
#define  VTSS_M_VCAP_ES0_TAG_ACTION_B_PCP_VAL     VTSS_ENCODE_BITMASK(2,3)
#define  VTSS_X_VCAP_ES0_TAG_ACTION_B_PCP_VAL(x)  VTSS_EXTRACT_BITFIELD(x,2,3)

/** 
 * \brief
 * Selection of source to be used for DEI and PCP in ES0 tag / ITAG
 * (dependent on action type).
 *
 * \details 
 * 0 : Classified PCP/DEI values
 * 1 : Fixed to ES0
 * 2 : Fixed to port default (PORT_TCI_PRIO/PORT_TCI_CFI)
 * 3 : Use Mapped versions of classified QOS class and DP level
 *
 * Field: ::VTSS_VCAP_ES0_TAG_ACTION_B . QOS_SRC_SEL
 */
#define  VTSS_F_VCAP_ES0_TAG_ACTION_B_QOS_SRC_SEL(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_VCAP_ES0_TAG_ACTION_B_QOS_SRC_SEL     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_VCAP_ES0_TAG_ACTION_B_QOS_SRC_SEL(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Sticky bit status register
 *
 * \details
 * Register: \a VCAP_ES0:TAG_ACTION:C
 */
#define VTSS_VCAP_ES0_TAG_ACTION_C           VTSS_IOREG(VTSS_TO_VCAP_ES0,0x18)

/** 
 * \brief
 * Advanced classification activity bit.
 * 
 * 
 * Used for indirect access to all classification entries.
 *
 * \details 
 * '0' : no activity
 * '1' : rule has been triggered
 *
 * Field: ::VTSS_VCAP_ES0_TAG_ACTION_C . HIT_STICKY
 */
#define  VTSS_F_VCAP_ES0_TAG_ACTION_C_HIT_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a VCAP_ES0:TCAM_BIST
 *
 * Build in test for TCAM
 */


/** 
 * \brief Control of the TCAM
 *
 * \details
 * Register: \a VCAP_ES0:TCAM_BIST:TCAM_CTRL
 */
#define VTSS_VCAP_ES0_TCAM_BIST_TCAM_CTRL    VTSS_IOREG(VTSS_TO_VCAP_ES0,0x1d)

/** 
 * \brief
 * Set this field to start manual BIST of the TCAM. This field will be
 * cleared once BIST is complete. The BIST procedure requires that the TCAM
 * is initialized before start, setting TCAM_INIT at the same time as
 * setting this field will first initalize the TCAM and then run BIST.
 *
 * \details 
 * Field: ::VTSS_VCAP_ES0_TCAM_BIST_TCAM_CTRL . TCAM_BIST
 */
#define  VTSS_F_VCAP_ES0_TCAM_BIST_TCAM_CTRL_TCAM_BIST  VTSS_BIT(1)

/** 
 * \brief
 * Set this field to start manual initialization of the TCAM. This field is
 * cleared once initialization is complete.
 * The TCAM has random contents after reset and must be initialized prior
 * to usage.
 *
 * \details 
 * Field: ::VTSS_VCAP_ES0_TCAM_BIST_TCAM_CTRL . TCAM_INIT
 */
#define  VTSS_F_VCAP_ES0_TCAM_BIST_TCAM_CTRL_TCAM_INIT  VTSS_BIT(0)


/** 
 * \brief Status for the TCAM
 *
 * \details
 * Register: \a VCAP_ES0:TCAM_BIST:TCAM_STAT
 */
#define VTSS_VCAP_ES0_TCAM_BIST_TCAM_STAT    VTSS_IOREG(VTSS_TO_VCAP_ES0,0x20)

/** 
 * \brief
 * Set if BIST failed.
 *
 * \details 
 * Field: ::VTSS_VCAP_ES0_TCAM_BIST_TCAM_STAT . BIST_ERR
 */
#define  VTSS_F_VCAP_ES0_TCAM_BIST_TCAM_STAT_BIST_ERR  VTSS_BIT(2)

/** 
 * \brief
 * Set when the BIST is still running. When checking the BIST result this
 * field must be cleared.
 *
 * \details 
 * Field: ::VTSS_VCAP_ES0_TCAM_BIST_TCAM_STAT . BIST_BUSY
 */
#define  VTSS_F_VCAP_ES0_TCAM_BIST_TCAM_STAT_BIST_BUSY  VTSS_BIT(1)

/** 
 * \brief
 * Indicates the current operational state of the TCAM.
 *
 * \details 
 * '0': Busy with initialization.
 * '1': Ready to be used.
 *
 * Field: ::VTSS_VCAP_ES0_TCAM_BIST_TCAM_STAT . TCAM_RDY
 */
#define  VTSS_F_VCAP_ES0_TCAM_BIST_TCAM_STAT_TCAM_RDY  VTSS_BIT(0)


#endif /* _VTSS_JAGUAR_REGS_VCAP_ES0_H_ */
