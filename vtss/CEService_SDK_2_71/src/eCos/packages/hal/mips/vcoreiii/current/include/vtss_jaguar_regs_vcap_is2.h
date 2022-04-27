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
#ifndef _VTSS_JAGUAR_REGS_VCAP_IS2_H_
#define _VTSS_JAGUAR_REGS_VCAP_IS2_H_

#include "vtss_jaguar_regs_common.h"

/*********************************************************************** 
 *
 * Target: \a VCAP_IS2
 *
 * Ingress Security, OAM, and Control protocol classification block.
 *
 ***********************************************************************/

/**
 * Register Group: \a VCAP_IS2:IS2_CONTROL
 *
 * Miscellaneous Configuration Registers for VCAP
 */


/** 
 * \brief Miscellaneous VCAP Configuration
 *
 * \details
 * Register: \a VCAP_IS2:IS2_CONTROL:ACL_CFG
 */
#define VTSS_VCAP_IS2_IS2_CONTROL_ACL_CFG    VTSS_IOREG(VTSS_TO_VCAP_IS2,0x0)

/** 
 * \brief
 * Enables or disables use of VCAP frame processing. The VCAP can still be
 * configured even though VCAP frame processing is disabled.
 *
 * \details 
 * 0:  VCAP frame processing is disabled.
 * 1:  VCAP frame processing is enabled.
 *
 * Field: ::VTSS_VCAP_IS2_IS2_CONTROL_ACL_CFG . ACL_ENA
 */
#define  VTSS_F_VCAP_IS2_IS2_CONTROL_ACL_CFG_ACL_ENA  VTSS_BIT(0)


/** 
 * \brief Entry and Action Control
 *
 * \details
 * Register: \a VCAP_IS2:IS2_CONTROL:ACE_UPDATE_CTRL
 */
#define VTSS_VCAP_IS2_IS2_CONTROL_ACE_UPDATE_CTRL  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x24)

/** 
 * \brief
 * Specifies the entry type bit mask. It must normally be set to 0, but can
 * be used to create entries that cover multiple entry types.
 * Entry types are as follows:
 * 000 : MAC_ETYPE
 * 001 : MAC_LLC
 * 010 : MAC_SNAP
 * 011 : ARP
 * 100 : IP_TCP_UDP
 * 101 : IP_OTHER
 * 110 : IP6_STD
 * Example 1:
 * If ACE_TYPE_MASK is set 001 while copying an IP_TCP_UDP or IP_OTHER from
 * the cache to the TCAM/RAM, the entry can be used to match both
 * IP_TCP_UDP and IP_OTHER categorized frames.
 * Example 2:
 * If ACE_TYPE_MASK is set to 001 while copying a MAC_ETYPE or MAC_LLC from
 * the cache to the TCAM/RAM, the entry can be used to match both MAC_ETYPE
 * and MAC_LLC categorized frames.
 * Note: If ACE_TYPE_MASK is set to a value different from 0, all register
 * fields that are not common to all the covered entry types must be set to
 * don't-care. That is, if, in example 1, an IP_TCP_UDP entry was created,
 * both the ACE_L4_PORT and ACE_L4_MISC registers must be set to
 * don't-care, because they do not exist for an IP_OTHER entry.
 *
 * \details 
 * 000: An entry that is copied from the cache to the TCAM only allows for
 * one entry type.
 * 001: Bit 0 of the 3-bit entry type is treated as don't-care during TCAM
 * lookup.
 * 010: Bit 1 of the 3-bit entry type is treated as don't-care during TCAM
 * lookup.
 * 011: Bit 0 and 1 of the 3-bit entry type is treated as don't-care during
 * TCAM lookup.
 * 100: Bit 2 of the 3-bit entry type is treated as don't-care during TCAM
 * lookup.
 * 101: Bit 0 and 2 of the 3-bit entry type is treated as don't-care during
 * TCAM lookup.
 * 110: Bit 1 and 2 of the 3-bit entry type is treated as don't-care during
 * TCAM lookup.
 * 111: Bit 0, 1 and 2 of the 3-bit entry type is treated as don't-care
 * during TCAM lookup.
 *
 * Field: ::VTSS_VCAP_IS2_IS2_CONTROL_ACE_UPDATE_CTRL . ACE_ENTRY_TYPE_MASK
 */
#define  VTSS_F_VCAP_IS2_IS2_CONTROL_ACE_UPDATE_CTRL_ACE_ENTRY_TYPE_MASK(x)  VTSS_ENCODE_BITFIELD(x,20,4)
#define  VTSS_M_VCAP_IS2_IS2_CONTROL_ACE_UPDATE_CTRL_ACE_ENTRY_TYPE_MASK     VTSS_ENCODE_BITMASK(20,4)
#define  VTSS_X_VCAP_IS2_IS2_CONTROL_ACE_UPDATE_CTRL_ACE_ENTRY_TYPE_MASK(x)  VTSS_EXTRACT_BITFIELD(x,20,4)

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
 * Field: ::VTSS_VCAP_IS2_IS2_CONTROL_ACE_UPDATE_CTRL . ACE_INDEX
 */
#define  VTSS_F_VCAP_IS2_IS2_CONTROL_ACE_UPDATE_CTRL_ACE_INDEX(x)  VTSS_ENCODE_BITFIELD(x,4,10)
#define  VTSS_M_VCAP_IS2_IS2_CONTROL_ACE_UPDATE_CTRL_ACE_INDEX     VTSS_ENCODE_BITMASK(4,10)
#define  VTSS_X_VCAP_IS2_IS2_CONTROL_ACE_UPDATE_CTRL_ACE_INDEX(x)  VTSS_EXTRACT_BITFIELD(x,4,10)

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
 * Field: ::VTSS_VCAP_IS2_IS2_CONTROL_ACE_UPDATE_CTRL . ACE_UPDATE_CMD
 */
#define  VTSS_F_VCAP_IS2_IS2_CONTROL_ACE_UPDATE_CTRL_ACE_UPDATE_CMD(x)  VTSS_ENCODE_BITFIELD(x,1,3)
#define  VTSS_M_VCAP_IS2_IS2_CONTROL_ACE_UPDATE_CTRL_ACE_UPDATE_CMD     VTSS_ENCODE_BITMASK(1,3)
#define  VTSS_X_VCAP_IS2_IS2_CONTROL_ACE_UPDATE_CTRL_ACE_UPDATE_CMD(x)  VTSS_EXTRACT_BITFIELD(x,1,3)

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
 * Field: ::VTSS_VCAP_IS2_IS2_CONTROL_ACE_UPDATE_CTRL . ACE_UPDATE_SHOT
 */
#define  VTSS_F_VCAP_IS2_IS2_CONTROL_ACE_UPDATE_CTRL_ACE_UPDATE_SHOT  VTSS_BIT(0)


/** 
 * \brief Advanced Move/Copy Operations
 *
 * \details
 * Register: \a VCAP_IS2:IS2_CONTROL:ACE_MV_CFG
 */
#define VTSS_VCAP_IS2_IS2_CONTROL_ACE_MV_CFG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x25)

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
 * Field: ::VTSS_VCAP_IS2_IS2_CONTROL_ACE_MV_CFG . ACE_MV_NUM_POS
 */
#define  VTSS_F_VCAP_IS2_IS2_CONTROL_ACE_MV_CFG_ACE_MV_NUM_POS(x)  VTSS_ENCODE_BITFIELD(x,16,10)
#define  VTSS_M_VCAP_IS2_IS2_CONTROL_ACE_MV_CFG_ACE_MV_NUM_POS     VTSS_ENCODE_BITMASK(16,10)
#define  VTSS_X_VCAP_IS2_IS2_CONTROL_ACE_MV_CFG_ACE_MV_NUM_POS(x)  VTSS_EXTRACT_BITFIELD(x,16,10)

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
 * Field: ::VTSS_VCAP_IS2_IS2_CONTROL_ACE_MV_CFG . ACE_MV_RST_HIT_STATUS_ENA
 */
#define  VTSS_F_VCAP_IS2_IS2_CONTROL_ACE_MV_CFG_ACE_MV_RST_HIT_STATUS_ENA  VTSS_BIT(15)

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
 * Field: ::VTSS_VCAP_IS2_IS2_CONTROL_ACE_MV_CFG . ACE_MV_SIZE
 */
#define  VTSS_F_VCAP_IS2_IS2_CONTROL_ACE_MV_CFG_ACE_MV_SIZE(x)  VTSS_ENCODE_BITFIELD(x,0,10)
#define  VTSS_M_VCAP_IS2_IS2_CONTROL_ACE_MV_CFG_ACE_MV_SIZE     VTSS_ENCODE_BITMASK(0,10)
#define  VTSS_X_VCAP_IS2_IS2_CONTROL_ACE_MV_CFG_ACE_MV_SIZE(x)  VTSS_EXTRACT_BITFIELD(x,0,10)


/** 
 * \brief Entry/Action Cache Status
 *
 * \details
 * Register: \a VCAP_IS2:IS2_CONTROL:ACE_STATUS
 */
#define VTSS_VCAP_IS2_IS2_CONTROL_ACE_STATUS  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x26)

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
 * Field: ::VTSS_VCAP_IS2_IS2_CONTROL_ACE_STATUS . ACE_ENTRY_TYPE
 */
#define  VTSS_F_VCAP_IS2_IS2_CONTROL_ACE_STATUS_ACE_ENTRY_TYPE(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_VCAP_IS2_IS2_CONTROL_ACE_STATUS_ACE_ENTRY_TYPE     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_VCAP_IS2_IS2_CONTROL_ACE_STATUS_ACE_ENTRY_TYPE(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief Status Sticky Bits
 *
 * \details
 * Register: \a VCAP_IS2:IS2_CONTROL:ACE_STICKY
 */
#define VTSS_VCAP_IS2_IS2_CONTROL_ACE_STICKY  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x27)

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
 * Field: ::VTSS_VCAP_IS2_IS2_CONTROL_ACE_STICKY . ACE_DEL_STICKY
 */
#define  VTSS_F_VCAP_IS2_IS2_CONTROL_ACE_STICKY_ACE_DEL_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a VCAP_IS2:MAC_ETYPE_ENTRY
 *
 * Entry data for entry type MAC_ETYPE.
 */


/** 
 * \details
 * Register: \a VCAP_IS2:MAC_ETYPE_ENTRY:ACE_VLD
 */
#define VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_VLD  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x28)

/** 
 * \brief
 * This field sets the entry to valid, and it must not be set to
 * "don't-care".
 *
 * \details 
 * 0: The entry is not valid.
 * 1: The entry is valid.
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Miscellaneous Entry Fields
 *
 * \details
 * Register: \a VCAP_IS2:MAC_ETYPE_ENTRY:ACE_TYPE
 */
#define VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x29)

/** 
 * \details 
 * 0: Only untagged frames must be able to match this entry.
 * 1: Only tagged frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Is set when the frame was matched in the VCAP IS0 .
 *
 * \details 
 * 0: Match frames, which did not have match in VCAP IS0.
 * 1: Match frames, which did have match in VCAP IS0.

 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * 1. lookup: Frame hit an ingress router leg in ANA_L3.
 * 2. lookup: Frame is to be L3 forwarded (as decided in ANA_L3).

 *
 * \details 
 * 0: Only frames that are not subject to layer 3 routing must be able to
 * match this entry.
 * 1: Only frames that are subject to layer 3 routing must be able to match
 * this entry.
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE . L3_RT
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE_L3_RT  VTSS_BIT(22)

/** 
 * \brief
 * Match only frames with (SMAC, SIP) match in LPM table in ANA_L3.
 *
 * \details 
 * 0 : Only frames with no (SMAC, SIP) match in LPM table must match this
 * entry.
 * 1 : Only frames with (SMAC, SIP) match in LPM table must match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE . L3_SMAC_SIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE_L3_SMAC_SIP_MATCH  VTSS_BIT(21)

/** 
 * \brief
 * Match only frames with (DMAC, DIP) match in LPM table in ANA_L3.
 *
 * \details 
 * 0 : Only frames with no (DMAC, DIP) match in LPM table must match this
 * entry.
 * 1 : Only frames with (DMAC, DIP) match in LPM table must match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE . L3_DMAC_DIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE_L3_DMAC_DIP_MATCH  VTSS_BIT(23)

/** 
 * \brief
 * Control whether entry shall match frames, which are to be discarded.

 *
 * \details 
 * 0: Match only frames, which shall be discarded.
 * 1: Match only frames, which shall not be discarded.
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * For frames, which did not have a match in VCAP IS0:
 * This field is matched against the VLAN ID resulting from the VLAN
 * classification in the ANA_CL.
 * 
 * If frame is to be routed (as decided by ANA_L3) and
 * ANA_CL:PORT:ADV_CL_CFG.SEC_ROUTE_HANDLING_ENA is set, then 
 * ingress VID will be used in first VCAP IS2 lookup and egress VID will be
 * used in second VCAP IS2 lookup.
 * 
 * For frames, which did have a match in VCAP IS0:
 * This field is matched against the ISDX value resulting from VCAP IS0.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * This field is matched against the user priority (UPRIO) resulting from
 * the VLAN classification in the ANA_CL.

 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * This field is matched against the Canonical Form Indicator (CFI)
 * resulting from the VLAN classification in ANA_CL.
 * 
 * The CFI bit is also referred to as Drop Eligible Indicator (DEI).
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Control whether frames with broadcast outer DMAC (0xFFFFFFFFFFFF) shall
 * be able to match this entry.

 *
 * \details 
 * 0: Broadcast frames must not be able to match this entry.
 * 1: Broadcast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Control whether frames with multicast outer DMAC shall be able to match
 * this entry. 
 *
 * \details 
 * 0: Multicast frames must not be able to match this entry.
 * 1: Multicast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * This field defines if an entry applies to first, second, or both lookups
 * in VCAP IS2.
 *
 * \details 
 * 0: The entry is used only for first lookup.
 * 1: The entry is used only for second lookup.
 * 
 * To match both first and second lookup VCAP_IS2:*_MASK:FIRST.
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Ingress Port Mask
 *
 * \details
 * Register: \a VCAP_IS2:MAC_ETYPE_ENTRY:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x2a)


/** 
 * \brief Policy Association Group
 *
 * \details
 * Register: \a VCAP_IS2:MAC_ETYPE_ENTRY:ACE_PAG
 */
#define VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_PAG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x2b)

/** 
 * \brief
 * This field is matched against the Policy Association Group resulting
 * from the PAG classification in VCAP_S0 or VCAP_S1.
 *
 * \details 
 * 0: Only frames from ingress ports that map to PAG 0 must be able to
 * match this entry.
 * 1: Only frames from ingress ports that map to PAG 1 must be able to
 * match this entry.
 * ...
 * n: Only frames from ingress ports that map to PAG n must be able to
 * match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Source MAC Address Bit 47:32
 *
 * \details
 * Register: \a VCAP_IS2:MAC_ETYPE_ENTRY:ACE_L2_SMAC_HIGH
 */
#define VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_L2_SMAC_HIGH  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x2c)

/** 
 * \brief
 * This field is matched against bit 47:32 of the SMAC address.
 * 
 * For OAM entries, inner MAC (C-SA) is used if ISMACINMACOAM is set for
 * frame.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_L2_SMAC_HIGH . L2_SMAC_HIGH
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Source MAC Address Bit 31:0
 *
 * \details
 * Register: \a VCAP_IS2:MAC_ETYPE_ENTRY:ACE_L2_SMAC_LOW
 */
#define VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_L2_SMAC_LOW  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x2d)


/** 
 * \brief Destination MAC Address Bit 47:32
 *
 * \details
 * Register: \a VCAP_IS2:MAC_ETYPE_ENTRY:ACE_L2_DMAC_HIGH
 */
#define VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_L2_DMAC_HIGH  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x2e)

/** 
 * \brief
 * This field is matched against bit 47:32 of the DMAC address.
 * 
 * For OAM entries, inner MAC (C-DA) is used if ISMACINMACOAM is set for
 * frame.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_L2_DMAC_HIGH . L2_DMAC_HIGH
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_L2_DMAC_HIGH_L2_DMAC_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_L2_DMAC_HIGH_L2_DMAC_HIGH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_L2_DMAC_HIGH_L2_DMAC_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Destination MAC Address Bit 31:0
 *
 * \details
 * Register: \a VCAP_IS2:MAC_ETYPE_ENTRY:ACE_L2_DMAC_LOW
 */
#define VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_L2_DMAC_LOW  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x2f)


/** 
 * \brief Etype Field and Payload Bytes
 *
 * \details
 * Register: \a VCAP_IS2:MAC_ETYPE_ENTRY:ACE_L2_ETYPE
 */
#define VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_L2_ETYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x30)

/** 
 * \brief
 * This field is matched against the first two bytes following the Ethernet
 * Type field.
 * Up to two VLAN tags and/or one I-Tag may precede the Ethernet Type
 * field.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_L2_ETYPE . L2_PAYLOAD
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_L2_ETYPE_L2_PAYLOAD(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_L2_ETYPE_L2_PAYLOAD     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_L2_ETYPE_L2_PAYLOAD(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * This field is matched against the Ethernet Type field.
 * Up to two VLAN tags and/or one I-Tag may precede the Ethernet Type
 * field.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_L2_ETYPE . ETYPE
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_L2_ETYPE_ETYPE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_L2_ETYPE_ETYPE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_MAC_ETYPE_ENTRY_ACE_L2_ETYPE_ETYPE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VCAP_IS2:MAC_LLC_ENTRY
 *
 * Entry data for entry type MAC_LLC.
 */


/** 
 * \details
 * Register: \a VCAP_IS2:MAC_LLC_ENTRY:ACE_VLD
 */
#define VTSS_VCAP_IS2_MAC_LLC_ENTRY_ACE_VLD  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x38)

/** 
 * \brief
 * This field sets the entry to valid, and it must not be set to
 * "don't-care".
 *
 * \details 
 * 0: The entry is not valid.
 * 1: The entry is valid.
 *
 * Field: ::VTSS_VCAP_IS2_MAC_LLC_ENTRY_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_MAC_LLC_ENTRY_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Miscellaneous Entry Fields
 *
 * \details
 * Register: \a VCAP_IS2:MAC_LLC_ENTRY:ACE_TYPE
 */
#define VTSS_VCAP_IS2_MAC_LLC_ENTRY_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x39)

/** 
 * \details 
 * 0: Only untagged frames must be able to match this entry.
 * 1: Only tagged frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_MAC_LLC_ENTRY_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_MAC_LLC_ENTRY_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Is set when the frame was matched in the VCAP IS0 .
 *
 * \details 
 * 0: Match frames, which did not have match in VCAP IS0.
 * 1: Match frames, which did have match in VCAP IS0.

 *
 * Field: ::VTSS_VCAP_IS2_MAC_LLC_ENTRY_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_MAC_LLC_ENTRY_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * Control whether entry shall match frames, which are to be discarded.

 *
 * \details 
 * 0: Match only frames, which shall be discarded.
 * 1: Match only frames, which shall not be discarded.
 *
 * Field: ::VTSS_VCAP_IS2_MAC_LLC_ENTRY_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_MAC_LLC_ENTRY_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * For frames, which did not have a match in VCAP IS0:
 * This field is matched against the VLAN ID resulting from the VLAN
 * classification in the ANA_CL.
 * 
 * If frame is to be routed (as decided by ANA_L3) and
 * ANA_CL:PORT:ADV_CL_CFG.SEC_ROUTE_HANDLING_ENA is set, then 
 * ingress VID will be used in first VCAP IS2 lookup and egress VID will be
 * used in second VCAP IS2 lookup.
 * 
 * For frames, which did have a match in VCAP IS0:
 * This field is matched against the ISDX value resulting from VCAP IS0.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_MAC_LLC_ENTRY_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_MAC_LLC_ENTRY_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_MAC_LLC_ENTRY_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_MAC_LLC_ENTRY_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * This field is matched against the user priority (UPRIO) resulting from
 * the VLAN classification in the ANA_CL.

 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_MAC_LLC_ENTRY_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_MAC_LLC_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_MAC_LLC_ENTRY_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_MAC_LLC_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * This field is matched against the Canonical Form Indicator (CFI)
 * resulting from the VLAN classification in ANA_CL.
 * 
 * The CFI bit is also referred to as Drop Eligible Indicator (DEI).
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_MAC_LLC_ENTRY_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_MAC_LLC_ENTRY_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Control whether frames with broadcast outer DMAC (0xFFFFFFFFFFFF) shall
 * be able to match this entry.

 *
 * \details 
 * 0: Broadcast frames must not be able to match this entry.
 * 1: Broadcast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_MAC_LLC_ENTRY_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_MAC_LLC_ENTRY_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Control whether frames with multicast outer DMAC shall be able to match
 * this entry. 
 *
 * \details 
 * 0: Multicast frames must not be able to match this entry.
 * 1: Multicast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_MAC_LLC_ENTRY_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_MAC_LLC_ENTRY_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * This field defines if an entry applies to first, second, or both lookups
 * in VCAP IS2.
 *
 * \details 
 * 0: The entry is used only for first lookup.
 * 1: The entry is used only for second lookup.
 * 
 * To match both first and second lookup VCAP_IS2:*_MASK:FIRST.
 *
 * Field: ::VTSS_VCAP_IS2_MAC_LLC_ENTRY_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_MAC_LLC_ENTRY_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Ingress Port Mask
 *
 * \details
 * Register: \a VCAP_IS2:MAC_LLC_ENTRY:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_MAC_LLC_ENTRY_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x3a)


/** 
 * \brief Policy Association Group
 *
 * \details
 * Register: \a VCAP_IS2:MAC_LLC_ENTRY:ACE_PAG
 */
#define VTSS_VCAP_IS2_MAC_LLC_ENTRY_ACE_PAG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x3b)

/** 
 * \brief
 * This field is matched against the Policy Association Group resulting
 * from the PAG classification in VCAP_S0 or VCAP_S1.
 *
 * \details 
 * 0: Only frames from ingress ports that map to PAG 0 must be able to
 * match this entry.
 * 1: Only frames from ingress ports that map to PAG 1 must be able to
 * match this entry.
 * ...
 * n: Only frames from ingress ports that map to PAG n must be able to
 * match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_MAC_LLC_ENTRY_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_MAC_LLC_ENTRY_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_MAC_LLC_ENTRY_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_MAC_LLC_ENTRY_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Source MAC Address Bit 47:32
 *
 * \details
 * Register: \a VCAP_IS2:MAC_LLC_ENTRY:ACE_L2_SMAC_HIGH
 */
#define VTSS_VCAP_IS2_MAC_LLC_ENTRY_ACE_L2_SMAC_HIGH  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x3c)

/** 
 * \brief
 * This field is matched against bit 47:32 of the SMAC address.
 * 
 * For OAM entries, inner MAC (C-SA) is used if ISMACINMACOAM is set for
 * frame.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_MAC_LLC_ENTRY_ACE_L2_SMAC_HIGH . L2_SMAC_HIGH
 */
#define  VTSS_F_VCAP_IS2_MAC_LLC_ENTRY_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_MAC_LLC_ENTRY_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_MAC_LLC_ENTRY_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Source MAC Address Bit 31:0
 *
 * \details
 * Register: \a VCAP_IS2:MAC_LLC_ENTRY:ACE_L2_SMAC_LOW
 */
#define VTSS_VCAP_IS2_MAC_LLC_ENTRY_ACE_L2_SMAC_LOW  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x3d)


/** 
 * \brief Destination MAC Address Bit 47:32
 *
 * \details
 * Register: \a VCAP_IS2:MAC_LLC_ENTRY:ACE_L2_DMAC_HIGH
 */
#define VTSS_VCAP_IS2_MAC_LLC_ENTRY_ACE_L2_DMAC_HIGH  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x3e)

/** 
 * \brief
 * This field is matched against bit 47:32 of the DMAC address.
 * 
 * For OAM entries, inner MAC (C-DA) is used if ISMACINMACOAM is set for
 * frame.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_MAC_LLC_ENTRY_ACE_L2_DMAC_HIGH . L2_DMAC_HIGH
 */
#define  VTSS_F_VCAP_IS2_MAC_LLC_ENTRY_ACE_L2_DMAC_HIGH_L2_DMAC_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_MAC_LLC_ENTRY_ACE_L2_DMAC_HIGH_L2_DMAC_HIGH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_MAC_LLC_ENTRY_ACE_L2_DMAC_HIGH_L2_DMAC_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Destination MAC Address Bit 31:0
 *
 * \details
 * Register: \a VCAP_IS2:MAC_LLC_ENTRY:ACE_L2_DMAC_LOW
 */
#define VTSS_VCAP_IS2_MAC_LLC_ENTRY_ACE_L2_DMAC_LOW  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x3f)


/** 
 * \brief DSAP, SSAP and CTRL Fields Plus One Payload Byte
 *
 * \details
 * Register: \a VCAP_IS2:MAC_LLC_ENTRY:ACE_L2_LLC
 */
#define VTSS_VCAP_IS2_MAC_LLC_ENTRY_ACE_L2_LLC  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x40)

/**
 * Register Group: \a VCAP_IS2:MAC_SNAP_ENTRY
 *
 * Entry data for entry type MAC_SNAP.
 */


/** 
 * \details
 * Register: \a VCAP_IS2:MAC_SNAP_ENTRY:ACE_VLD
 */
#define VTSS_VCAP_IS2_MAC_SNAP_ENTRY_ACE_VLD  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x48)

/** 
 * \brief
 * This field sets the entry to valid, and it must not be set to
 * "don't-care".
 *
 * \details 
 * 0: The entry is not valid.
 * 1: The entry is valid.
 *
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_ENTRY_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_ENTRY_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Miscellaneous Entry Fields
 *
 * \details
 * Register: \a VCAP_IS2:MAC_SNAP_ENTRY:ACE_TYPE
 */
#define VTSS_VCAP_IS2_MAC_SNAP_ENTRY_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x49)

/** 
 * \details 
 * 0: Only untagged frames must be able to match this entry.
 * 1: Only tagged frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_ENTRY_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_ENTRY_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Is set when the frame was matched in the VCAP IS0 .
 *
 * \details 
 * 0: Match frames, which did not have match in VCAP IS0.
 * 1: Match frames, which did have match in VCAP IS0.

 *
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_ENTRY_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_ENTRY_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * Control whether entry shall match frames, which are to be discarded.

 *
 * \details 
 * 0: Match only frames, which shall be discarded.
 * 1: Match only frames, which shall not be discarded.
 *
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_ENTRY_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_ENTRY_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * For frames, which did not have a match in VCAP IS0:
 * This field is matched against the VLAN ID resulting from the VLAN
 * classification in the ANA_CL.
 * 
 * If frame is to be routed (as decided by ANA_L3) and
 * ANA_CL:PORT:ADV_CL_CFG.SEC_ROUTE_HANDLING_ENA is set, then 
 * ingress VID will be used in first VCAP IS2 lookup and egress VID will be
 * used in second VCAP IS2 lookup.
 * 
 * For frames, which did have a match in VCAP IS0:
 * This field is matched against the ISDX value resulting from VCAP IS0.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_ENTRY_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_ENTRY_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_MAC_SNAP_ENTRY_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_MAC_SNAP_ENTRY_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * This field is matched against the user priority (UPRIO) resulting from
 * the VLAN classification in the ANA_CL.

 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_ENTRY_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_MAC_SNAP_ENTRY_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_MAC_SNAP_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * This field is matched against the Canonical Form Indicator (CFI)
 * resulting from the VLAN classification in ANA_CL.
 * 
 * The CFI bit is also referred to as Drop Eligible Indicator (DEI).
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_ENTRY_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_ENTRY_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Control whether frames with broadcast outer DMAC (0xFFFFFFFFFFFF) shall
 * be able to match this entry.

 *
 * \details 
 * 0: Broadcast frames must not be able to match this entry.
 * 1: Broadcast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_ENTRY_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_ENTRY_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Control whether frames with multicast outer DMAC shall be able to match
 * this entry. 
 *
 * \details 
 * 0: Multicast frames must not be able to match this entry.
 * 1: Multicast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_ENTRY_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_ENTRY_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * This field defines if an entry applies to first, second, or both lookups
 * in VCAP IS2.
 *
 * \details 
 * 0: The entry is used only for first lookup.
 * 1: The entry is used only for second lookup.
 * 
 * To match both first and second lookup VCAP_IS2:*_MASK:FIRST.
 *
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_ENTRY_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_ENTRY_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Ingress Port Mask
 *
 * \details
 * Register: \a VCAP_IS2:MAC_SNAP_ENTRY:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_MAC_SNAP_ENTRY_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x4a)


/** 
 * \brief Policy Association Group
 *
 * \details
 * Register: \a VCAP_IS2:MAC_SNAP_ENTRY:ACE_PAG
 */
#define VTSS_VCAP_IS2_MAC_SNAP_ENTRY_ACE_PAG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x4b)

/** 
 * \brief
 * This field is matched against the Policy Association Group resulting
 * from the PAG classification in VCAP_S0 or VCAP_S1.
 *
 * \details 
 * 0: Only frames from ingress ports that map to PAG 0 must be able to
 * match this entry.
 * 1: Only frames from ingress ports that map to PAG 1 must be able to
 * match this entry.
 * ...
 * n: Only frames from ingress ports that map to PAG n must be able to
 * match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_ENTRY_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_ENTRY_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_MAC_SNAP_ENTRY_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_MAC_SNAP_ENTRY_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Source MAC Address Bit 47:32
 *
 * \details
 * Register: \a VCAP_IS2:MAC_SNAP_ENTRY:ACE_L2_SMAC_HIGH
 */
#define VTSS_VCAP_IS2_MAC_SNAP_ENTRY_ACE_L2_SMAC_HIGH  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x4c)

/** 
 * \brief
 * This field is matched against bit 47:32 of the SMAC address.
 * 
 * For OAM entries, inner MAC (C-SA) is used if ISMACINMACOAM is set for
 * frame.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_ENTRY_ACE_L2_SMAC_HIGH . L2_SMAC_HIGH
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_ENTRY_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_MAC_SNAP_ENTRY_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_MAC_SNAP_ENTRY_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Source MAC Address Bit 31:0
 *
 * \details
 * Register: \a VCAP_IS2:MAC_SNAP_ENTRY:ACE_L2_SMAC_LOW
 */
#define VTSS_VCAP_IS2_MAC_SNAP_ENTRY_ACE_L2_SMAC_LOW  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x4d)


/** 
 * \brief Destination MAC Address Bit 47:32
 *
 * \details
 * Register: \a VCAP_IS2:MAC_SNAP_ENTRY:ACE_L2_DMAC_HIGH
 */
#define VTSS_VCAP_IS2_MAC_SNAP_ENTRY_ACE_L2_DMAC_HIGH  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x4e)

/** 
 * \brief
 * This field is matched against bit 47:32 of the DMAC address.
 * 
 * For OAM entries, inner MAC (C-DA) is used if ISMACINMACOAM is set for
 * frame.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_ENTRY_ACE_L2_DMAC_HIGH . L2_DMAC_HIGH
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_ENTRY_ACE_L2_DMAC_HIGH_L2_DMAC_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_MAC_SNAP_ENTRY_ACE_L2_DMAC_HIGH_L2_DMAC_HIGH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_MAC_SNAP_ENTRY_ACE_L2_DMAC_HIGH_L2_DMAC_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Destination MAC Address Bit 31:0
 *
 * \details
 * Register: \a VCAP_IS2:MAC_SNAP_ENTRY:ACE_L2_DMAC_LOW
 */
#define VTSS_VCAP_IS2_MAC_SNAP_ENTRY_ACE_L2_DMAC_LOW  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x4f)


/** 
 * \brief ORG CODE Bit 15:0 and SNAP TYPE Field
 *
 * \details
 * Register: \a VCAP_IS2:MAC_SNAP_ENTRY:ACE_L2_SNAP_LOW
 */
#define VTSS_VCAP_IS2_MAC_SNAP_ENTRY_ACE_L2_SNAP_LOW  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x50)


/** 
 * \brief ORG CODE Bit 23:16
 *
 * \details
 * Register: \a VCAP_IS2:MAC_SNAP_ENTRY:ACE_L2_SNAP_HIGH
 */
#define VTSS_VCAP_IS2_MAC_SNAP_ENTRY_ACE_L2_SNAP_HIGH  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x51)

/** 
 * \brief
 * This field is matched against bit 23:16 of the 24 bit SNAP ORG CODE
 * field.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_ENTRY_ACE_L2_SNAP_HIGH . L2_SNAP_HIGH
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_ENTRY_ACE_L2_SNAP_HIGH_L2_SNAP_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_MAC_SNAP_ENTRY_ACE_L2_SNAP_HIGH_L2_SNAP_HIGH     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_MAC_SNAP_ENTRY_ACE_L2_SNAP_HIGH_L2_SNAP_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a VCAP_IS2:ARP_ENTRY
 *
 * Entry data for entry type ARP.
 */


/** 
 * \details
 * Register: \a VCAP_IS2:ARP_ENTRY:ACE_VLD
 */
#define VTSS_VCAP_IS2_ARP_ENTRY_ACE_VLD      VTSS_IOREG(VTSS_TO_VCAP_IS2,0x58)

/** 
 * \brief
 * This field sets the entry to valid, and it must not be set to
 * "don't-care".
 *
 * \details 
 * 0: The entry is not valid.
 * 1: The entry is valid.
 *
 * Field: ::VTSS_VCAP_IS2_ARP_ENTRY_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_ARP_ENTRY_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Miscellaneous Entry Fields
 *
 * \details
 * Register: \a VCAP_IS2:ARP_ENTRY:ACE_TYPE
 */
#define VTSS_VCAP_IS2_ARP_ENTRY_ACE_TYPE     VTSS_IOREG(VTSS_TO_VCAP_IS2,0x59)

/** 
 * \details 
 * 0: Only untagged frames must be able to match this entry.
 * 1: Only tagged frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_ARP_ENTRY_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_ARP_ENTRY_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Is set when the frame was matched in the VCAP IS0 .
 *
 * \details 
 * 0: Match frames, which did not have match in VCAP IS0.
 * 1: Match frames, which did have match in VCAP IS0.

 *
 * Field: ::VTSS_VCAP_IS2_ARP_ENTRY_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_ARP_ENTRY_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * Control whether entry shall match frames, which are to be discarded.

 *
 * \details 
 * 0: Match only frames, which shall be discarded.
 * 1: Match only frames, which shall not be discarded.
 *
 * Field: ::VTSS_VCAP_IS2_ARP_ENTRY_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_ARP_ENTRY_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * For frames, which did not have a match in VCAP IS0:
 * This field is matched against the VLAN ID resulting from the VLAN
 * classification in the ANA_CL.
 * 
 * If frame is to be routed (as decided by ANA_L3) and
 * ANA_CL:PORT:ADV_CL_CFG.SEC_ROUTE_HANDLING_ENA is set, then 
 * ingress VID will be used in first VCAP IS2 lookup and egress VID will be
 * used in second VCAP IS2 lookup.
 * 
 * For frames, which did have a match in VCAP IS0:
 * This field is matched against the ISDX value resulting from VCAP IS0.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_ARP_ENTRY_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_ARP_ENTRY_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_ARP_ENTRY_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_ARP_ENTRY_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * This field is matched against the user priority (UPRIO) resulting from
 * the VLAN classification in the ANA_CL.

 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_ARP_ENTRY_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_ARP_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_ARP_ENTRY_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_ARP_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * This field is matched against the Canonical Form Indicator (CFI)
 * resulting from the VLAN classification in ANA_CL.
 * 
 * The CFI bit is also referred to as Drop Eligible Indicator (DEI).
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_ARP_ENTRY_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_ARP_ENTRY_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Control whether frames with broadcast outer DMAC (0xFFFFFFFFFFFF) shall
 * be able to match this entry.

 *
 * \details 
 * 0: Broadcast frames must not be able to match this entry.
 * 1: Broadcast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_ARP_ENTRY_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_ARP_ENTRY_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Control whether frames with multicast outer DMAC shall be able to match
 * this entry. 
 *
 * \details 
 * 0: Multicast frames must not be able to match this entry.
 * 1: Multicast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_ARP_ENTRY_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_ARP_ENTRY_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * This field defines if an entry applies to first, second, or both lookups
 * in VCAP IS2.
 *
 * \details 
 * 0: The entry is used only for first lookup.
 * 1: The entry is used only for second lookup.
 * 
 * To match both first and second lookup VCAP_IS2:*_MASK:FIRST.
 *
 * Field: ::VTSS_VCAP_IS2_ARP_ENTRY_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_ARP_ENTRY_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Ingress Port Mask
 *
 * \details
 * Register: \a VCAP_IS2:ARP_ENTRY:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_ARP_ENTRY_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x5a)


/** 
 * \brief Policy Association Group
 *
 * \details
 * Register: \a VCAP_IS2:ARP_ENTRY:ACE_PAG
 */
#define VTSS_VCAP_IS2_ARP_ENTRY_ACE_PAG      VTSS_IOREG(VTSS_TO_VCAP_IS2,0x5b)

/** 
 * \brief
 * This field is matched against the Policy Association Group resulting
 * from the PAG classification in VCAP_S0 or VCAP_S1.
 *
 * \details 
 * 0: Only frames from ingress ports that map to PAG 0 must be able to
 * match this entry.
 * 1: Only frames from ingress ports that map to PAG 1 must be able to
 * match this entry.
 * ...
 * n: Only frames from ingress ports that map to PAG n must be able to
 * match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_ARP_ENTRY_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_ARP_ENTRY_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_ARP_ENTRY_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_ARP_ENTRY_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Source MAC Address Bit 47:32
 *
 * \details
 * Register: \a VCAP_IS2:ARP_ENTRY:ACE_L2_SMAC_HIGH
 */
#define VTSS_VCAP_IS2_ARP_ENTRY_ACE_L2_SMAC_HIGH  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x5c)

/** 
 * \brief
 * This field is matched against bit 47:32 of the SMAC address.
 * 
 * For OAM entries, inner MAC (C-SA) is used if ISMACINMACOAM is set for
 * frame.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_ARP_ENTRY_ACE_L2_SMAC_HIGH . L2_SMAC_HIGH
 */
#define  VTSS_F_VCAP_IS2_ARP_ENTRY_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_ARP_ENTRY_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_ARP_ENTRY_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Source MAC Address Bit 31:0
 *
 * \details
 * Register: \a VCAP_IS2:ARP_ENTRY:ACE_L2_SMAC_LOW
 */
#define VTSS_VCAP_IS2_ARP_ENTRY_ACE_L2_SMAC_LOW  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x5d)


/** 
 * \brief Miscellaneous Layer 3 Entry Fields
 *
 * \details
 * Register: \a VCAP_IS2:ARP_ENTRY:ACE_L3_MISC
 */
#define VTSS_VCAP_IS2_ARP_ENTRY_ACE_L3_MISC  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x5e)

/** 
 * \brief
 * Is set when DIP equals SIP.
 * This is supported for both IPv4 and IPv6 addresses.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_ARP_ENTRY_ACE_L3_MISC . DIP_EQ_SIP
 */
#define  VTSS_F_VCAP_IS2_ARP_ENTRY_ACE_L3_MISC_DIP_EQ_SIP  VTSS_BIT(20)


/** 
 * \brief ARP Specific Fields
 *
 * \details
 * Register: \a VCAP_IS2:ARP_ENTRY:ACE_L3_ARP
 */
#define VTSS_VCAP_IS2_ARP_ENTRY_ACE_L3_ARP   VTSS_IOREG(VTSS_TO_VCAP_IS2,0x5f)

/** 
 * \brief
 * ARP / RARP Hardware Address Space field (HRD) flag.
 *
 * \details 
 * 0: ARP / RARP frames where the Hardware Address Space field (HRD) is
 * equal to Ethernet (1) must not be able to match this entry.
 * 1: Only ARP / RARP frames where the Hardware Address Space field (HRD)
 * is equal to Ethernet (1) must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_ARP_ENTRY_ACE_L3_ARP . ARP_ADDR_SPACE_OK
 */
#define  VTSS_F_VCAP_IS2_ARP_ENTRY_ACE_L3_ARP_ARP_ADDR_SPACE_OK  VTSS_BIT(7)

/** 
 * \brief
 * ARP / RARP Protocol Address Space field (PRO) flag.
 *
 * \details 
 * 0: ARP / RARP frames where the Protocol Address Space field (PRO) is
 * equal to IP (0x800) must not be able to match this entry.
 * 1: Only ARP / RARP frames where the Protocol Address Space field (PRO)
 * is equal to IP (0x800) must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_ARP_ENTRY_ACE_L3_ARP . ARP_PROTO_SPACE_OK
 */
#define  VTSS_F_VCAP_IS2_ARP_ENTRY_ACE_L3_ARP_ARP_PROTO_SPACE_OK  VTSS_BIT(6)

/** 
 * \brief
 * ARP / RARP Hardware Address Length field (HLN) flag.
 *
 * \details 
 * 0: ARP / RARP frames where the Hardware Address Length field (HLN) is
 * equal to Ethernet (0x6) and the Protocol Address Length field is equal
 * to IPv4 (0x4) must not be able to match this entry.
 * 1: Only ARP / RARP frames where the Hardware Address Length field (HLN)
 * is equal to Ethernet (0x6) and the Protocol Address Length field is
 * equal to IPv4 (0x4) must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_ARP_ENTRY_ACE_L3_ARP . ARP_LEN_OK
 */
#define  VTSS_F_VCAP_IS2_ARP_ENTRY_ACE_L3_ARP_ARP_LEN_OK  VTSS_BIT(5)

/** 
 * \brief
 * RARP Target Hardware Address field (THA) flag. This field can be used to
 * capture RARP-Requests where a client is trying to get its own IP
 * address.
 *
 * \details 
 * 0: Only RARP frames where the Target Hardware Address field (THA) is not
 * equal to the SMAC address and ARP frames must be able to match this
 * field.
 * 1: Only RARP frames where the Target Hardware Address field (THA) is
 * equal to the SMAC address must be able to match this entry
 *
 * Field: ::VTSS_VCAP_IS2_ARP_ENTRY_ACE_L3_ARP . ARP_TARGET_MATCH
 */
#define  VTSS_F_VCAP_IS2_ARP_ENTRY_ACE_L3_ARP_ARP_TARGET_MATCH  VTSS_BIT(4)

/** 
 * \brief
 * ARP Sender Hardware Address field (SHA) flag. This field can be used to
 * check that an ARP packet contains correct ARP SHA. Some hosts do not
 * have access to the SMAC and therefore rely on ARP SHA being the correct
 * SMAC.
 *
 * \details 
 * 0: ARP frames where the Sender Hardware Address field (SHA) is not equal
 * to the SMAC address and RARP frames must be able to match this field.
 * 1: Only ARP frames where the Sender Hardware Address field (SHA) is
 * equal to the SMAC address must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_ARP_ENTRY_ACE_L3_ARP . ARP_SENDER_MATCH
 */
#define  VTSS_F_VCAP_IS2_ARP_ENTRY_ACE_L3_ARP_ARP_SENDER_MATCH  VTSS_BIT(3)

/** 
 * \brief
 * Unknown ARP / RARP Opcode field (OP) flag.
 *
 * \details 
 * 0: Only ARP / RARP frames where the ARP / RARP Opcode specifies a
 * request or reply must be able to match this entry.
 * 1: Only ARP / RARP frames where the ARP / RARP Opcode does not specify a
 * request nor a reply must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_ARP_ENTRY_ACE_L3_ARP . ARP_OPCODE_UNKNOWN
 */
#define  VTSS_F_VCAP_IS2_ARP_ENTRY_ACE_L3_ARP_ARP_OPCODE_UNKNOWN  VTSS_BIT(2)

/** 
 * \brief
 * ARP / RARP Opcode field (OP) flag.
 *
 * \details 
 * 0x0: Only ARP Requests (OP = 0x1) must be able to match this entry.
 * 0x1: Only ARP Replies (OP = 0x2) must be able to match this entry.
 * 0x2: Only RARP Requests (OP = 0x3) must be able to match this entry.
 * 0x3: Only RARP Replies (OP = 0x4) must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_ARP_ENTRY_ACE_L3_ARP . ARP_OPCODE
 */
#define  VTSS_F_VCAP_IS2_ARP_ENTRY_ACE_L3_ARP_ARP_OPCODE(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_VCAP_IS2_ARP_ENTRY_ACE_L3_ARP_ARP_OPCODE     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_VCAP_IS2_ARP_ENTRY_ACE_L3_ARP_ARP_OPCODE(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief IPv4 Destination Address
 *
 * \details
 * Register: \a VCAP_IS2:ARP_ENTRY:ACE_L3_IP4_DIP
 */
#define VTSS_VCAP_IS2_ARP_ENTRY_ACE_L3_IP4_DIP  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x60)


/** 
 * \brief IPv4 Source Address
 *
 * \details
 * Register: \a VCAP_IS2:ARP_ENTRY:ACE_L3_IP4_SIP
 */
#define VTSS_VCAP_IS2_ARP_ENTRY_ACE_L3_IP4_SIP  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x61)

/**
 * Register Group: \a VCAP_IS2:IP_TCP_UDP_ENTRY
 *
 * Entry data for entry type IP_TCP_UDP.
 */


/** 
 * \details
 * Register: \a VCAP_IS2:IP_TCP_UDP_ENTRY:ACE_VLD
 */
#define VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_VLD  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x68)

/** 
 * \brief
 * This field sets the entry to valid, and it must not be set to
 * "don't-care".
 *
 * \details 
 * 0: The entry is not valid.
 * 1: The entry is valid.
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Miscellaneous Entry Fields
 *
 * \details
 * Register: \a VCAP_IS2:IP_TCP_UDP_ENTRY:ACE_TYPE
 */
#define VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x69)

/** 
 * \details 
 * 0: Only untagged frames must be able to match this entry.
 * 1: Only tagged frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Is set when the frame was matched in the VCAP IS0 .
 *
 * \details 
 * 0: Match frames, which did not have match in VCAP IS0.
 * 1: Match frames, which did have match in VCAP IS0.

 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * 1. lookup: Frame hit an ingress router leg in ANA_L3.
 * 2. lookup: Frame is to be L3 forwarded (as decided in ANA_L3).

 *
 * \details 
 * 0: Only frames that are not subject to layer 3 routing must be able to
 * match this entry.
 * 1: Only frames that are subject to layer 3 routing must be able to match
 * this entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE . L3_RT
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE_L3_RT  VTSS_BIT(22)

/** 
 * \brief
 * Match only frames with (SMAC, SIP) match in LPM table in ANA_L3.
 *
 * \details 
 * 0 : Only frames with no (SMAC, SIP) match in LPM table must match this
 * entry.
 * 1 : Only frames with (SMAC, SIP) match in LPM table must match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE . L3_SMAC_SIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE_L3_SMAC_SIP_MATCH  VTSS_BIT(21)

/** 
 * \brief
 * Match only frames with (DMAC, DIP) match in LPM table in ANA_L3.
 *
 * \details 
 * 0 : Only frames with no (DMAC, DIP) match in LPM table must match this
 * entry.
 * 1 : Only frames with (DMAC, DIP) match in LPM table must match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE . L3_DMAC_DIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE_L3_DMAC_DIP_MATCH  VTSS_BIT(23)

/** 
 * \brief
 * Control whether entry shall match frames, which are to be discarded.

 *
 * \details 
 * 0: Match only frames, which shall be discarded.
 * 1: Match only frames, which shall not be discarded.
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * For frames, which did not have a match in VCAP IS0:
 * This field is matched against the VLAN ID resulting from the VLAN
 * classification in the ANA_CL.
 * 
 * If frame is to be routed (as decided by ANA_L3) and
 * ANA_CL:PORT:ADV_CL_CFG.SEC_ROUTE_HANDLING_ENA is set, then 
 * ingress VID will be used in first VCAP IS2 lookup and egress VID will be
 * used in second VCAP IS2 lookup.
 * 
 * For frames, which did have a match in VCAP IS0:
 * This field is matched against the ISDX value resulting from VCAP IS0.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * This field is matched against the user priority (UPRIO) resulting from
 * the VLAN classification in the ANA_CL.

 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * This field is matched against the Canonical Form Indicator (CFI)
 * resulting from the VLAN classification in ANA_CL.
 * 
 * The CFI bit is also referred to as Drop Eligible Indicator (DEI).
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Control whether frames with broadcast outer DMAC (0xFFFFFFFFFFFF) shall
 * be able to match this entry.

 *
 * \details 
 * 0: Broadcast frames must not be able to match this entry.
 * 1: Broadcast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Control whether frames with multicast outer DMAC shall be able to match
 * this entry. 
 *
 * \details 
 * 0: Multicast frames must not be able to match this entry.
 * 1: Multicast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * This field defines if an entry applies to first, second, or both lookups
 * in VCAP IS2.
 *
 * \details 
 * 0: The entry is used only for first lookup.
 * 1: The entry is used only for second lookup.
 * 
 * To match both first and second lookup VCAP_IS2:*_MASK:FIRST.
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Ingress Port Mask
 *
 * \details
 * Register: \a VCAP_IS2:IP_TCP_UDP_ENTRY:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x6a)


/** 
 * \brief Policy Association Group
 *
 * \details
 * Register: \a VCAP_IS2:IP_TCP_UDP_ENTRY:ACE_PAG
 */
#define VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_PAG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x6b)

/** 
 * \brief
 * This field is matched against the Policy Association Group resulting
 * from the PAG classification in VCAP_S0 or VCAP_S1.
 *
 * \details 
 * 0: Only frames from ingress ports that map to PAG 0 must be able to
 * match this entry.
 * 1: Only frames from ingress ports that map to PAG 1 must be able to
 * match this entry.
 * ...
 * n: Only frames from ingress ports that map to PAG n must be able to
 * match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Miscellaneous Layer 2 Entry Fields
 *
 * \details
 * Register: \a VCAP_IS2:IP_TCP_UDP_ENTRY:ACE_L2_MISC
 */
#define VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L2_MISC  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x6c)

/** 
 * \brief
 * Specifies Internet Protocol Version 4 frame flag for the entry.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L2_MISC . IP4
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L2_MISC_IP4  VTSS_BIT(5)


/** 
 * \brief Miscellaneous Layer 3 Entry Fields
 *
 * \details
 * Register: \a VCAP_IS2:IP_TCP_UDP_ENTRY:ACE_L3_MISC
 */
#define VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L3_MISC  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x6d)

/** 
 * \brief
 * Is set when DIP equals SIP.
 * This is supported for both IPv4 and IPv6 addresses.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L3_MISC . DIP_EQ_SIP
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L3_MISC_DIP_EQ_SIP  VTSS_BIT(20)

/** 
 * \brief
 * For IPv4 frames, this field is matched against the IP4 Protocol field.
 * For IPv6 frames, this field is matched against the IPv6 Next Header
 * field.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L3_MISC . L3_IP_PROTO
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L3_MISC_L3_IP_PROTO(x)  VTSS_ENCODE_BITFIELD(x,12,8)
#define  VTSS_M_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L3_MISC_L3_IP_PROTO     VTSS_ENCODE_BITMASK(12,8)
#define  VTSS_X_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L3_MISC_L3_IP_PROTO(x)  VTSS_EXTRACT_BITFIELD(x,12,8)

/** 
 * \brief
 * This field is matched against the IPv4/IPv6 Differentiated Services (DS)
 * field.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L3_MISC . L3_TOS
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L3_MISC_L3_TOS(x)  VTSS_ENCODE_BITFIELD(x,4,8)
#define  VTSS_M_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L3_MISC_L3_TOS     VTSS_ENCODE_BITMASK(4,8)
#define  VTSS_X_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L3_MISC_L3_TOS(x)  VTSS_EXTRACT_BITFIELD(x,4,8)

/** 
 * \brief
 * This bit matches IPv4 frames with the following properties:
 * a) The IPv4 header contains options, i.e. IPv4 header length is greater
 * than 5.
 * and/or
 * b) The IPv4 header's fragment offset is different from zero.
 *
 * \details 
 * 0: IPv4 frames with options must not be able to match this entry.
 * 1: Only IPv4 frames with options must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L3_MISC . L3_OPTIONS
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L3_MISC_L3_OPTIONS  VTSS_BIT(2)

/** 
 * \brief
 * IPv4 fragment flag.
 *
 * \details 
 * 0: IPv4 frames where the More Fragments (MF) bit is set or the Fragment
 * Offset (FRAG OFFSET) field is greater than 0 must not be able to match
 * this entry.
 * 1: Only IPv4 frames where the More Fragments (MF) bit is set or the
 * Fragment Offset (FRAG OFFSET) field is greater than 0 must be able to
 * match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L3_MISC . L3_FRAGMENT
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L3_MISC_L3_FRAGMENT  VTSS_BIT(1)

/** 
 * \brief
 * IPv4 Time-To-Live / IPv6 Hop Limit greater than zero.
 *
 * \details 
 * 0: IP frames with a Time-To-Live / Hop Limit field greater than zero
 * must not be able to match this entry.
 * 1: Only IP frames with a Time-To-Live / Hop Limit field greater than
 * zero must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L3_MISC . L3_TTL_GT0
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L3_MISC_L3_TTL_GT0  VTSS_BIT(0)


/** 
 * \brief IPv4 Destination Address
 *
 * \details
 * Register: \a VCAP_IS2:IP_TCP_UDP_ENTRY:ACE_L3_IP4_DIP
 */
#define VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L3_IP4_DIP  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x6e)


/** 
 * \brief IPv4 Source Address
 *
 * \details
 * Register: \a VCAP_IS2:IP_TCP_UDP_ENTRY:ACE_L3_IP4_SIP
 */
#define VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L3_IP4_SIP  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x6f)


/** 
 * \brief TCP / UDP Source and Destination Ports
 *
 * \details
 * Register: \a VCAP_IS2:IP_TCP_UDP_ENTRY:ACE_L4_PORT
 */
#define VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_PORT  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x70)

/** 
 * \brief
 * This field is matched against the IPv4/IPv6 TCP / UDP Source Port field
 * (SPORT). 
 * Masked out for IPv4 frames with Fragment Offset > 0 or IPv4 header
 * options.
 * 
 * Note that ACE_L4_MISC.L4_RNG can be used to match against a range of
 * SPORT/DPORT values.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_PORT . L4_SPORT
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_PORT_L4_SPORT(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_PORT_L4_SPORT     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_PORT_L4_SPORT(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * This field is matched against the IPv4/IPv6 TCP / UDP Destination Port
 * field (DPORT). 
 * Masked out for IPv4 frames with Fragment Offset > 0 or IPv4 header
 * options.
 * 
 * Note that ACE_L4_MISC.L4_RNG can be used to match against a range of
 * SPORT/DPORT values.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_PORT . L4_DPORT
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_PORT_L4_DPORT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_PORT_L4_DPORT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_PORT_L4_DPORT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Miscellaneous TCP / UDP fields
 *
 * \details
 * Works for both IPv4 and IPv6 frames.
 *
 * Register: \a VCAP_IS2:IP_TCP_UDP_ENTRY:ACE_L4_MISC
 */
#define VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_MISC  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x71)

/** 
 * \brief
 * SPORT equal to DPORT.
 * 
 * Masked out for frames with IPv4 frames with Fragment Offset > 0.
 *
 * \details 
 * 0: TCP / UDP frames where the SPORT is equal to the DPORT must not be
 * able to match this entry.
 * 1: Only TCP / UDP frames where the DPORT is equal to the SPORT must be
 * able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_MISC . SPORT_EQ_DPORT
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_MISC_SPORT_EQ_DPORT  VTSS_BIT(15)

/** 
 * \brief
 * TCP Sequence field equal to zero.
 * 
 * Masked out for frames with IPv4 frames with Fragment Offset > 0, as well
 * as for UDP frames.
 *
 * \details 
 * 0: Only frames where the TCP Sequence field is greater than or equal to
 * 1 must be able to match this entry.
 * 1: Only frames where the TCP Sequence field is equal to 0 must be able
 * to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_MISC . SEQUENCE_EQ0
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_MISC_SEQUENCE_EQ0  VTSS_BIT(14)

/** 
 * \brief
 * This field is matched against the TCP 'No more data from sender' field
 * (FIN).
 * 
 * Masked out for frames with IPv4 frames with Fragment Offset > 0, as well
 * as for UDP frames.

 *
 * \details 
 * 0: TCP frames where the FIN field is set must not be able to match this
 * entry.
 * 1: Only TCP frames where the FIN field is set must be able to match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_MISC . L4_FIN
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_MISC_L4_FIN  VTSS_BIT(13)

/** 
 * \brief
 * This field is matched against the TCP 'Synchronize sequence numbers'
 * field (SYN).
 * 
 * Masked out for frames with IPv4 frames with Fragment Offset > 0, as well
 * as for UDP frames.

 *
 * \details 
 * 0: TCP frames where the SYN field is set must not be able to match this
 * entry.
 * 1: Only TCP frames where the SYN field is set must be able to match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_MISC . L4_SYN
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_MISC_L4_SYN  VTSS_BIT(12)

/** 
 * \brief
 * This field is matched against the TCP 'Reset the connection' field
 * (RST).
 * 
 * Masked out for frames with IPv4 frames with Fragment Offset > 0, as well
 * as for UDP frames.
 *
 * \details 
 * 0: TCP frames where the RST field is set must not be able to match this
 * entry.
 * 1: Only TCP frames where the RST field is set must be able to match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_MISC . L4_RST
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_MISC_L4_RST  VTSS_BIT(11)

/** 
 * \brief
 * This field is matched against the TCP 'Push Function' field (PSH).
 * 
 * Masked out for frames with IPv4 frames with Fragment Offset > 0, as well
 * as for UDP frames.
 *
 * \details 
 * 0: TCP frames where the PSH field is set must not be able to match this
 * entry.
 * 1: Only TCP frames where the PSH field is set must be able to match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_MISC . L4_PSH
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_MISC_L4_PSH  VTSS_BIT(10)

/** 
 * \brief
 * This field is matched against the TCP 'Acknowledgment field significant'
 * field (ACK).
 * 
 * Masked out for frames with IPv4 frames with Fragment Offset > 0, as well
 * as for UDP frames.
 *
 * \details 
 * 0: TCP frames where the ACK field is set must not be able to match this
 * entry.
 * 1: Only TCP frames where the ACK field is set must be able to match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_MISC . L4_ACK
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_MISC_L4_ACK  VTSS_BIT(9)

/** 
 * \brief
 * This field is matched against the TCP 'Urgent Pointer field significant'
 * field (URG).
 * 
 * Masked out for frames with IPv4 frames with Fragment Offset > 0, as well
 * as for UDP frames.
 *
 * \details 
 * 0: TCP frames where the URG field is set must not be able to match this
 * entry.
 * 1: Only TCP frames where the URG field is set must be able to match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_MISC . L4_URG
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_MISC_L4_URG  VTSS_BIT(8)

/** 
 * \brief
 * TCP / UDP SPORT/DPORT range match flags. 
 * 
 * In 
 *   ANA_CL:COMMON:ADV_RNG_CTRL
 *   ANA_CL:COMMON:ADV_RNG_VALUE_CFG
 * a number of range checkers can be configured. These can be used to
 * identify a range of TCP/UDP SPORT/DPORT values. If such a range is
 * identified, a L4_RNG bit will be set and can be matched by VCAP_IS2.
 * 
 * If a L4_RNG bit is not used, then it must be set to "don't care".
 * 
 * L4_RNG is automatically masked out for frames with Fragments Offset > 0.
 * 
 * The range checkers in ANA_CL can also be used to match against other
 * fields - VID, DSCP and custom - and if so, L4_RNG can be used to match
 * against VID/DSCP/custom values. Though this is considered an unusual use
 * of L4_RNG.
 *
 * \details 
 * 0: Only entries with the range bit cleared shall be able to match this
 * entry. 
 * 1: Only entries with the range bit set shall be able to match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_MISC . L4_RNG
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_MISC_L4_RNG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_MISC_L4_RNG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_IP_TCP_UDP_ENTRY_ACE_L4_MISC_L4_RNG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a VCAP_IS2:IP_OTHER_ENTRY
 *
 * Entry data for entry type IP_OTHER.
 */


/** 
 * \details
 * Register: \a VCAP_IS2:IP_OTHER_ENTRY:ACE_VLD
 */
#define VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_VLD  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x78)

/** 
 * \brief
 * This field sets the entry to valid, and it must not be set to
 * "don't-care".
 *
 * \details 
 * 0: The entry is not valid.
 * 1: The entry is valid.
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_ENTRY_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Miscellaneous Entry Fields
 *
 * \details
 * Register: \a VCAP_IS2:IP_OTHER_ENTRY:ACE_TYPE
 */
#define VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x79)

/** 
 * \details 
 * 0: Only untagged frames must be able to match this entry.
 * 1: Only tagged frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Is set when the frame was matched in the VCAP IS0 .
 *
 * \details 
 * 0: Match frames, which did not have match in VCAP IS0.
 * 1: Match frames, which did have match in VCAP IS0.

 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * 1. lookup: Frame hit an ingress router leg in ANA_L3.
 * 2. lookup: Frame is to be L3 forwarded (as decided in ANA_L3).

 *
 * \details 
 * 0: Only frames that are not subject to layer 3 routing must be able to
 * match this entry.
 * 1: Only frames that are subject to layer 3 routing must be able to match
 * this entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE . L3_RT
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE_L3_RT  VTSS_BIT(22)

/** 
 * \brief
 * Match only frames with (SMAC, SIP) match in LPM table in ANA_L3.
 *
 * \details 
 * 0 : Only frames with no (SMAC, SIP) match in LPM table must match this
 * entry.
 * 1 : Only frames with (SMAC, SIP) match in LPM table must match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE . L3_SMAC_SIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE_L3_SMAC_SIP_MATCH  VTSS_BIT(21)

/** 
 * \brief
 * Match only frames with (DMAC, DIP) match in LPM table in ANA_L3.
 *
 * \details 
 * 0 : Only frames with no (DMAC, DIP) match in LPM table must match this
 * entry.
 * 1 : Only frames with (DMAC, DIP) match in LPM table must match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE . L3_DMAC_DIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE_L3_DMAC_DIP_MATCH  VTSS_BIT(23)

/** 
 * \brief
 * Control whether entry shall match frames, which are to be discarded.

 *
 * \details 
 * 0: Match only frames, which shall be discarded.
 * 1: Match only frames, which shall not be discarded.
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * For frames, which did not have a match in VCAP IS0:
 * This field is matched against the VLAN ID resulting from the VLAN
 * classification in the ANA_CL.
 * 
 * If frame is to be routed (as decided by ANA_L3) and
 * ANA_CL:PORT:ADV_CL_CFG.SEC_ROUTE_HANDLING_ENA is set, then 
 * ingress VID will be used in first VCAP IS2 lookup and egress VID will be
 * used in second VCAP IS2 lookup.
 * 
 * For frames, which did have a match in VCAP IS0:
 * This field is matched against the ISDX value resulting from VCAP IS0.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * This field is matched against the user priority (UPRIO) resulting from
 * the VLAN classification in the ANA_CL.

 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * This field is matched against the Canonical Form Indicator (CFI)
 * resulting from the VLAN classification in ANA_CL.
 * 
 * The CFI bit is also referred to as Drop Eligible Indicator (DEI).
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Control whether frames with broadcast outer DMAC (0xFFFFFFFFFFFF) shall
 * be able to match this entry.

 *
 * \details 
 * 0: Broadcast frames must not be able to match this entry.
 * 1: Broadcast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Control whether frames with multicast outer DMAC shall be able to match
 * this entry. 
 *
 * \details 
 * 0: Multicast frames must not be able to match this entry.
 * 1: Multicast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * This field defines if an entry applies to first, second, or both lookups
 * in VCAP IS2.
 *
 * \details 
 * 0: The entry is used only for first lookup.
 * 1: The entry is used only for second lookup.
 * 
 * To match both first and second lookup VCAP_IS2:*_MASK:FIRST.
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_ENTRY_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Ingress Port Mask
 *
 * \details
 * Register: \a VCAP_IS2:IP_OTHER_ENTRY:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x7a)


/** 
 * \brief Policy Association Group
 *
 * \details
 * Register: \a VCAP_IS2:IP_OTHER_ENTRY:ACE_PAG
 */
#define VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_PAG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x7b)

/** 
 * \brief
 * This field is matched against the Policy Association Group resulting
 * from the PAG classification in VCAP_S0 or VCAP_S1.
 *
 * \details 
 * 0: Only frames from ingress ports that map to PAG 0 must be able to
 * match this entry.
 * 1: Only frames from ingress ports that map to PAG 1 must be able to
 * match this entry.
 * ...
 * n: Only frames from ingress ports that map to PAG n must be able to
 * match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_ENTRY_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_IP_OTHER_ENTRY_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_IP_OTHER_ENTRY_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Miscellaneous Layer 2 Entry Fields
 *
 * \details
 * Register: \a VCAP_IS2:IP_OTHER_ENTRY:ACE_L2_MISC
 */
#define VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_L2_MISC  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x7c)

/** 
 * \brief
 * Specifies Internet Protocol Version 4 frame flag for the entry.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_L2_MISC . IP4
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_ENTRY_ACE_L2_MISC_IP4  VTSS_BIT(5)


/** 
 * \brief Miscellaneous Layer 3 Entry Fields
 *
 * \details
 * Register: \a VCAP_IS2:IP_OTHER_ENTRY:ACE_L3_MISC
 */
#define VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_L3_MISC  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x7d)

/** 
 * \brief
 * Is set when DIP equals SIP.
 * This is supported for both IPv4 and IPv6 addresses.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_L3_MISC . DIP_EQ_SIP
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_ENTRY_ACE_L3_MISC_DIP_EQ_SIP  VTSS_BIT(20)

/** 
 * \brief
 * For IPv4 frames, this field is matched against the IP4 Protocol field.
 * For IPv6 frames, this field is matched against the IPv6 Next Header
 * field.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_L3_MISC . L3_IP_PROTO
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_ENTRY_ACE_L3_MISC_L3_IP_PROTO(x)  VTSS_ENCODE_BITFIELD(x,12,8)
#define  VTSS_M_VCAP_IS2_IP_OTHER_ENTRY_ACE_L3_MISC_L3_IP_PROTO     VTSS_ENCODE_BITMASK(12,8)
#define  VTSS_X_VCAP_IS2_IP_OTHER_ENTRY_ACE_L3_MISC_L3_IP_PROTO(x)  VTSS_EXTRACT_BITFIELD(x,12,8)

/** 
 * \brief
 * This field is matched against the IPv4/IPv6 Differentiated Services (DS)
 * field.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_L3_MISC . L3_TOS
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_ENTRY_ACE_L3_MISC_L3_TOS(x)  VTSS_ENCODE_BITFIELD(x,4,8)
#define  VTSS_M_VCAP_IS2_IP_OTHER_ENTRY_ACE_L3_MISC_L3_TOS     VTSS_ENCODE_BITMASK(4,8)
#define  VTSS_X_VCAP_IS2_IP_OTHER_ENTRY_ACE_L3_MISC_L3_TOS(x)  VTSS_EXTRACT_BITFIELD(x,4,8)

/** 
 * \brief
 * This bit matches IPv4 frames with the following properties:
 * a) The IPv4 header contains options, i.e. IPv4 header length is greater
 * than 5.
 * and/or
 * b) The IPv4 header's fragment offset is different from zero.
 *
 * \details 
 * 0: IPv4 frames with options must not be able to match this entry.
 * 1: Only IPv4 frames with options must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_L3_MISC . L3_OPTIONS
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_ENTRY_ACE_L3_MISC_L3_OPTIONS  VTSS_BIT(2)

/** 
 * \brief
 * IPv4 fragment flag.
 *
 * \details 
 * 0: IPv4 frames where the More Fragments (MF) bit is set or the Fragment
 * Offset (FRAG OFFSET) field is greater than 0 must not be able to match
 * this entry.
 * 1: Only IPv4 frames where the More Fragments (MF) bit is set or the
 * Fragment Offset (FRAG OFFSET) field is greater than 0 must be able to
 * match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_L3_MISC . L3_FRAGMENT
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_ENTRY_ACE_L3_MISC_L3_FRAGMENT  VTSS_BIT(1)

/** 
 * \brief
 * IPv4 Time-To-Live / IPv6 Hop Limit greater than zero.
 *
 * \details 
 * 0: IP frames with a Time-To-Live / Hop Limit field greater than zero
 * must not be able to match this entry.
 * 1: Only IP frames with a Time-To-Live / Hop Limit field greater than
 * zero must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_L3_MISC . L3_TTL_GT0
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_ENTRY_ACE_L3_MISC_L3_TTL_GT0  VTSS_BIT(0)


/** 
 * \brief IPv4 Destination Address
 *
 * \details
 * Register: \a VCAP_IS2:IP_OTHER_ENTRY:ACE_L3_IP4_DIP
 */
#define VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_L3_IP4_DIP  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x7e)


/** 
 * \brief IPv4 Source Address
 *
 * \details
 * Register: \a VCAP_IS2:IP_OTHER_ENTRY:ACE_L3_IP4_SIP
 */
#define VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_L3_IP4_SIP  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x7f)


/** 
 * \brief Bit 31:0 of First 6 IPv4/IPv6 Payload Bytes
 *
 * \details
 * Register: \a VCAP_IS2:IP_OTHER_ENTRY:ACE_IP4_OTHER_0
 */
#define VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_IP4_OTHER_0  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x80)


/** 
 * \brief Bit 47:32 of First 6 IPv4/IPv6 Payload Bytes
 *
 * \details
 * Register: \a VCAP_IS2:IP_OTHER_ENTRY:ACE_IP4_OTHER_1
 */
#define VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_IP4_OTHER_1  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x81)

/** 
 * \brief
 * This field is matched against bit 47:32 of the first six IPv4/IPv6
 * payload bytes.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_ENTRY_ACE_IP4_OTHER_1 . IP4_PAYLOAD_HIGH
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_ENTRY_ACE_IP4_OTHER_1_IP4_PAYLOAD_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_IP_OTHER_ENTRY_ACE_IP4_OTHER_1_IP4_PAYLOAD_HIGH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_IP_OTHER_ENTRY_ACE_IP4_OTHER_1_IP4_PAYLOAD_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VCAP_IS2:IP6_STD_ENTRY
 *
 * Entry data for entry type IP6_STD.
 */


/** 
 * \details
 * Register: \a VCAP_IS2:IP6_STD_ENTRY:ACE_VLD
 */
#define VTSS_VCAP_IS2_IP6_STD_ENTRY_ACE_VLD  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x88)

/** 
 * \brief
 * This field sets the entry to valid, and it must not be set to
 * "don't-care".
 *
 * \details 
 * 0: The entry is not valid.
 * 1: The entry is valid.
 *
 * Field: ::VTSS_VCAP_IS2_IP6_STD_ENTRY_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_ENTRY_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Miscellaneous Entry Fields
 *
 * \details
 * Register: \a VCAP_IS2:IP6_STD_ENTRY:ACE_TYPE
 */
#define VTSS_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x89)

/** 
 * \details 
 * 0: Only untagged frames must be able to match this entry.
 * 1: Only tagged frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Is set when the frame was matched in the VCAP IS0 .
 *
 * \details 
 * 0: Match frames, which did not have match in VCAP IS0.
 * 1: Match frames, which did have match in VCAP IS0.

 *
 * Field: ::VTSS_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * 1. lookup: Frame hit an ingress router leg in ANA_L3.
 * 2. lookup: Frame is to be L3 forwarded (as decided in ANA_L3).

 *
 * \details 
 * 0: Only frames that are not subject to layer 3 routing must be able to
 * match this entry.
 * 1: Only frames that are subject to layer 3 routing must be able to match
 * this entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE . L3_RT
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE_L3_RT  VTSS_BIT(22)

/** 
 * \brief
 * Match only frames with (SMAC, SIP) match in LPM table in ANA_L3.
 *
 * \details 
 * 0 : Only frames with no (SMAC, SIP) match in LPM table must match this
 * entry.
 * 1 : Only frames with (SMAC, SIP) match in LPM table must match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE . L3_SMAC_SIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE_L3_SMAC_SIP_MATCH  VTSS_BIT(21)

/** 
 * \brief
 * Match only frames with (DMAC, DIP) match in LPM table in ANA_L3.
 *
 * \details 
 * 0 : Only frames with no (DMAC, DIP) match in LPM table must match this
 * entry.
 * 1 : Only frames with (DMAC, DIP) match in LPM table must match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE . L3_DMAC_DIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE_L3_DMAC_DIP_MATCH  VTSS_BIT(23)

/** 
 * \brief
 * Control whether entry shall match frames, which are to be discarded.

 *
 * \details 
 * 0: Match only frames, which shall be discarded.
 * 1: Match only frames, which shall not be discarded.
 *
 * Field: ::VTSS_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * For frames, which did not have a match in VCAP IS0:
 * This field is matched against the VLAN ID resulting from the VLAN
 * classification in the ANA_CL.
 * 
 * If frame is to be routed (as decided by ANA_L3) and
 * ANA_CL:PORT:ADV_CL_CFG.SEC_ROUTE_HANDLING_ENA is set, then 
 * ingress VID will be used in first VCAP IS2 lookup and egress VID will be
 * used in second VCAP IS2 lookup.
 * 
 * For frames, which did have a match in VCAP IS0:
 * This field is matched against the ISDX value resulting from VCAP IS0.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * This field is matched against the user priority (UPRIO) resulting from
 * the VLAN classification in the ANA_CL.

 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * This field is matched against the Canonical Form Indicator (CFI)
 * resulting from the VLAN classification in ANA_CL.
 * 
 * The CFI bit is also referred to as Drop Eligible Indicator (DEI).
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Control whether frames with broadcast outer DMAC (0xFFFFFFFFFFFF) shall
 * be able to match this entry.

 *
 * \details 
 * 0: Broadcast frames must not be able to match this entry.
 * 1: Broadcast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Control whether frames with multicast outer DMAC shall be able to match
 * this entry. 
 *
 * \details 
 * 0: Multicast frames must not be able to match this entry.
 * 1: Multicast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * This field defines if an entry applies to first, second, or both lookups
 * in VCAP IS2.
 *
 * \details 
 * 0: The entry is used only for first lookup.
 * 1: The entry is used only for second lookup.
 * 
 * To match both first and second lookup VCAP_IS2:*_MASK:FIRST.
 *
 * Field: ::VTSS_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_ENTRY_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Ingress Port Mask
 *
 * \details
 * Register: \a VCAP_IS2:IP6_STD_ENTRY:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_IP6_STD_ENTRY_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x8a)


/** 
 * \brief Policy Association Group
 *
 * \details
 * Register: \a VCAP_IS2:IP6_STD_ENTRY:ACE_PAG
 */
#define VTSS_VCAP_IS2_IP6_STD_ENTRY_ACE_PAG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x8b)

/** 
 * \brief
 * This field is matched against the Policy Association Group resulting
 * from the PAG classification in VCAP_S0 or VCAP_S1.
 *
 * \details 
 * 0: Only frames from ingress ports that map to PAG 0 must be able to
 * match this entry.
 * 1: Only frames from ingress ports that map to PAG 1 must be able to
 * match this entry.
 * ...
 * n: Only frames from ingress ports that map to PAG n must be able to
 * match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_IP6_STD_ENTRY_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_ENTRY_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_IP6_STD_ENTRY_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_IP6_STD_ENTRY_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Miscellaneous Layer 3 Entry Fields
 *
 * \details
 * Register: \a VCAP_IS2:IP6_STD_ENTRY:ACE_L3_MISC
 */
#define VTSS_VCAP_IS2_IP6_STD_ENTRY_ACE_L3_MISC  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x8c)

/** 
 * \brief
 * For IPv4 frames, this field is matched against the IP4 Protocol field.
 * For IPv6 frames, this field is matched against the IPv6 Next Header
 * field.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_IP6_STD_ENTRY_ACE_L3_MISC . L3_IP_PROTO
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_ENTRY_ACE_L3_MISC_L3_IP_PROTO(x)  VTSS_ENCODE_BITFIELD(x,12,8)
#define  VTSS_M_VCAP_IS2_IP6_STD_ENTRY_ACE_L3_MISC_L3_IP_PROTO     VTSS_ENCODE_BITMASK(12,8)
#define  VTSS_X_VCAP_IS2_IP6_STD_ENTRY_ACE_L3_MISC_L3_IP_PROTO(x)  VTSS_EXTRACT_BITFIELD(x,12,8)


/** 
 * \brief IPv6 Source Address - Bit 31:0
 *
 * \details
 * Register: \a VCAP_IS2:IP6_STD_ENTRY:ACE_L3_IP6_SIP_0
 */
#define VTSS_VCAP_IS2_IP6_STD_ENTRY_ACE_L3_IP6_SIP_0  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x8d)


/** 
 * \brief IPv6 Source Address - Bit 63:32
 *
 * \details
 * Register: \a VCAP_IS2:IP6_STD_ENTRY:ACE_L3_IP6_SIP_1
 */
#define VTSS_VCAP_IS2_IP6_STD_ENTRY_ACE_L3_IP6_SIP_1  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x8e)


/** 
 * \brief IPv6 Source Address - Bit 95:64
 *
 * \details
 * Register: \a VCAP_IS2:IP6_STD_ENTRY:ACE_L3_IP6_SIP_2
 */
#define VTSS_VCAP_IS2_IP6_STD_ENTRY_ACE_L3_IP6_SIP_2  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x8f)


/** 
 * \brief IPv6 Source Address - Bit 127:96
 *
 * \details
 * Register: \a VCAP_IS2:IP6_STD_ENTRY:ACE_L3_IP6_SIP_3
 */
#define VTSS_VCAP_IS2_IP6_STD_ENTRY_ACE_L3_IP6_SIP_3  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x90)

/**
 * Register Group: \a VCAP_IS2:OAM_ENTRY
 *
 * Entry data for entry type OAM.
 */


/** 
 * \details
 * Register: \a VCAP_IS2:OAM_ENTRY:ACE_VLD
 */
#define VTSS_VCAP_IS2_OAM_ENTRY_ACE_VLD      VTSS_IOREG(VTSS_TO_VCAP_IS2,0x98)

/** 
 * \brief
 * This field sets the entry to valid, and it must not be set to
 * "don't-care".
 *
 * \details 
 * 0: The entry is not valid.
 * 1: The entry is valid.
 *
 * Field: ::VTSS_VCAP_IS2_OAM_ENTRY_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_OAM_ENTRY_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Miscellaneous Entry Fields
 *
 * \details
 * Register: \a VCAP_IS2:OAM_ENTRY:ACE_TYPE
 */
#define VTSS_VCAP_IS2_OAM_ENTRY_ACE_TYPE     VTSS_IOREG(VTSS_TO_VCAP_IS2,0x99)

/** 
 * \details 
 * 0: Only untagged frames must be able to match this entry.
 * 1: Only tagged frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_OAM_ENTRY_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_OAM_ENTRY_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Is set when the frame was matched in the VCAP IS0 .
 *
 * \details 
 * 0: Match frames, which did not have match in VCAP IS0.
 * 1: Match frames, which did have match in VCAP IS0.

 *
 * Field: ::VTSS_VCAP_IS2_OAM_ENTRY_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_OAM_ENTRY_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * Control whether entry shall match frames, which are to be discarded.

 *
 * \details 
 * 0: Match only frames, which shall be discarded.
 * 1: Match only frames, which shall not be discarded.
 *
 * Field: ::VTSS_VCAP_IS2_OAM_ENTRY_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_OAM_ENTRY_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * For frames, which did not have a match in VCAP IS0:
 * This field is matched against the VLAN ID resulting from the VLAN
 * classification in the ANA_CL.
 * 
 * If frame is to be routed (as decided by ANA_L3) and
 * ANA_CL:PORT:ADV_CL_CFG.SEC_ROUTE_HANDLING_ENA is set, then 
 * ingress VID will be used in first VCAP IS2 lookup and egress VID will be
 * used in second VCAP IS2 lookup.
 * 
 * For frames, which did have a match in VCAP IS0:
 * This field is matched against the ISDX value resulting from VCAP IS0.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_OAM_ENTRY_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_OAM_ENTRY_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_OAM_ENTRY_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_OAM_ENTRY_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * This field is matched against the user priority (UPRIO) resulting from
 * the VLAN classification in the ANA_CL.

 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_OAM_ENTRY_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_OAM_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_OAM_ENTRY_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_OAM_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * This field is matched against the Canonical Form Indicator (CFI)
 * resulting from the VLAN classification in ANA_CL.
 * 
 * The CFI bit is also referred to as Drop Eligible Indicator (DEI).
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_OAM_ENTRY_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_OAM_ENTRY_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * This field defines if an entry applies to first, second, or both lookups
 * in VCAP IS2.
 *
 * \details 
 * 0: The entry is used only for first lookup.
 * 1: The entry is used only for second lookup.
 * 
 * To match both first and second lookup VCAP_IS2:*_MASK:FIRST.
 *
 * Field: ::VTSS_VCAP_IS2_OAM_ENTRY_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_OAM_ENTRY_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Ingress Port Mask
 *
 * \details
 * Register: \a VCAP_IS2:OAM_ENTRY:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_OAM_ENTRY_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x9a)


/** 
 * \brief Policy Association Group
 *
 * \details
 * Register: \a VCAP_IS2:OAM_ENTRY:ACE_PAG
 */
#define VTSS_VCAP_IS2_OAM_ENTRY_ACE_PAG      VTSS_IOREG(VTSS_TO_VCAP_IS2,0x9b)

/** 
 * \brief
 * This field is matched against the Policy Association Group resulting
 * from the PAG classification in VCAP_S0 or VCAP_S1.
 *
 * \details 
 * 0: Only frames from ingress ports that map to PAG 0 must be able to
 * match this entry.
 * 1: Only frames from ingress ports that map to PAG 1 must be able to
 * match this entry.
 * ...
 * n: Only frames from ingress ports that map to PAG n must be able to
 * match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_OAM_ENTRY_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_OAM_ENTRY_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_OAM_ENTRY_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_OAM_ENTRY_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Source MAC Address Bit 47:32
 *
 * \details
 * Register: \a VCAP_IS2:OAM_ENTRY:ACE_L2_SMAC_HIGH
 */
#define VTSS_VCAP_IS2_OAM_ENTRY_ACE_L2_SMAC_HIGH  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x9c)

/** 
 * \brief
 * This field is matched against bit 47:32 of the SMAC address.
 * 
 * For OAM entries, inner MAC (C-SA) is used if ISMACINMACOAM is set for
 * frame.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_OAM_ENTRY_ACE_L2_SMAC_HIGH . L2_SMAC_HIGH
 */
#define  VTSS_F_VCAP_IS2_OAM_ENTRY_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_OAM_ENTRY_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_OAM_ENTRY_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Source MAC Address Bit 31:0
 *
 * \details
 * Register: \a VCAP_IS2:OAM_ENTRY:ACE_L2_SMAC_LOW
 */
#define VTSS_VCAP_IS2_OAM_ENTRY_ACE_L2_SMAC_LOW  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x9d)


/** 
 * \brief Destination MAC Address Bit 47:32
 *
 * \details
 * Register: \a VCAP_IS2:OAM_ENTRY:ACE_L2_DMAC_HIGH
 */
#define VTSS_VCAP_IS2_OAM_ENTRY_ACE_L2_DMAC_HIGH  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x9e)

/** 
 * \brief
 * This field is matched against bit 47:32 of the DMAC address.
 * 
 * For OAM entries, inner MAC (C-DA) is used if ISMACINMACOAM is set for
 * frame.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_OAM_ENTRY_ACE_L2_DMAC_HIGH . L2_DMAC_HIGH
 */
#define  VTSS_F_VCAP_IS2_OAM_ENTRY_ACE_L2_DMAC_HIGH_L2_DMAC_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_OAM_ENTRY_ACE_L2_DMAC_HIGH_L2_DMAC_HIGH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_OAM_ENTRY_ACE_L2_DMAC_HIGH_L2_DMAC_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Destination MAC Address Bit 31:0
 *
 * \details
 * Register: \a VCAP_IS2:OAM_ENTRY:ACE_L2_DMAC_LOW
 */
#define VTSS_VCAP_IS2_OAM_ENTRY_ACE_L2_DMAC_LOW  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x9f)


/** 
 * \brief OAM fields
 *
 * \details
 * Register: \a VCAP_IS2:OAM_ENTRY:ACE_OAM_0
 */
#define VTSS_VCAP_IS2_OAM_ENTRY_ACE_OAM_0    VTSS_IOREG(VTSS_TO_VCAP_IS2,0xa0)

/** 
 * \brief
 * Version field of OAM PDU (ref. ITU-T Y.1731).
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_OAM_ENTRY_ACE_OAM_0 . OAM_VER
 */
#define  VTSS_F_VCAP_IS2_OAM_ENTRY_ACE_OAM_0_OAM_VER(x)  VTSS_ENCODE_BITFIELD(x,8,5)
#define  VTSS_M_VCAP_IS2_OAM_ENTRY_ACE_OAM_0_OAM_VER     VTSS_ENCODE_BITMASK(8,5)
#define  VTSS_X_VCAP_IS2_OAM_ENTRY_ACE_OAM_0_OAM_VER(x)  VTSS_EXTRACT_BITFIELD(x,8,5)

/** 
 * \brief
 * Match for different values of MEL field of OAM PDU (ref. ITU-T Y.1731).
 * 
 * Together with the mask, the following kinds of rules may be created:
 * Exact match, e.g. MEL=2: 0b0000011
 * Below, e.g. MEL<=4: 0b000XXXX
 * Above, e.g. MEL>=5: 0bXX11111
 * Between, e.g. 3<= MEL<=5: 0b00XX111

 *
 * \details 
 * Bit 0: MEG Level 1
 * Bit 1: MEG Level 2
 * Bit 6: MEG Level 7
 * 
 * If no bits are set, CE matches only for MEG level 0.
 *
 * Field: ::VTSS_VCAP_IS2_OAM_ENTRY_ACE_OAM_0 . OAM_MEL_FLAGS
 */
#define  VTSS_F_VCAP_IS2_OAM_ENTRY_ACE_OAM_0_OAM_MEL_FLAGS(x)  VTSS_ENCODE_BITFIELD(x,1,7)
#define  VTSS_M_VCAP_IS2_OAM_ENTRY_ACE_OAM_0_OAM_MEL_FLAGS     VTSS_ENCODE_BITMASK(1,7)
#define  VTSS_X_VCAP_IS2_OAM_ENTRY_ACE_OAM_0_OAM_MEL_FLAGS(x)  VTSS_EXTRACT_BITFIELD(x,1,7)

/** 
 * \brief
 * OAM within MAC-in-MAC frames, ref. IEEE 802.1ah, "Provider Backbone
 * Bridges".
 *
 * \details 
 * 0: Only allow OAM frames that are NOT MAC-in-MAC encapsulated to match
 * this control entry.
 * 1: Only allow OAM frames that are MAC-in-MAC encapsulated to match this
 * control entry.
 *
 * Field: ::VTSS_VCAP_IS2_OAM_ENTRY_ACE_OAM_0 . ISMACINMACOAM
 */
#define  VTSS_F_VCAP_IS2_OAM_ENTRY_ACE_OAM_0_ISMACINMACOAM  VTSS_BIT(0)


/** 
 * \brief OAM fields
 *
 * \details
 * Register: \a VCAP_IS2:OAM_ENTRY:ACE_OAM_1
 */
#define VTSS_VCAP_IS2_OAM_ENTRY_ACE_OAM_1    VTSS_IOREG(VTSS_TO_VCAP_IS2,0xa1)

/** 
 * \brief
 * MEP ID field of OAM CCM PDU (ref. ITU-T Y.1731).
 * 
 * This field should be wildcarded for OpCodes different from 1 (CCM PDU).
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_OAM_ENTRY_ACE_OAM_1 . OAM_MEPID
 */
#define  VTSS_F_VCAP_IS2_OAM_ENTRY_ACE_OAM_1_OAM_MEPID(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_VCAP_IS2_OAM_ENTRY_ACE_OAM_1_OAM_MEPID     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_VCAP_IS2_OAM_ENTRY_ACE_OAM_1_OAM_MEPID(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Flags field of OAM PDU (ref. ITU-T Y.1731).
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_OAM_ENTRY_ACE_OAM_1 . OAM_FLAGS
 */
#define  VTSS_F_VCAP_IS2_OAM_ENTRY_ACE_OAM_1_OAM_FLAGS(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_VCAP_IS2_OAM_ENTRY_ACE_OAM_1_OAM_FLAGS     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_VCAP_IS2_OAM_ENTRY_ACE_OAM_1_OAM_FLAGS(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * OpCode field of OAM PDU (ref. ITU-T Y.1731).
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_OAM_ENTRY_ACE_OAM_1 . OAM_OPCODE
 */
#define  VTSS_F_VCAP_IS2_OAM_ENTRY_ACE_OAM_1_OAM_OPCODE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_OAM_ENTRY_ACE_OAM_1_OAM_OPCODE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_OAM_ENTRY_ACE_OAM_1_OAM_OPCODE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a VCAP_IS2:CUSTOM_0_ENTRY
 *
 * Entry data for entry type CUSTOM_0.
 */


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_0_ENTRY:ACE_VLD
 */
#define VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_VLD  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xa8)

/** 
 * \brief
 * This field sets the entry to valid, and it must not be set to
 * "don't-care".
 *
 * \details 
 * 0: The entry is not valid.
 * 1: The entry is valid.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_ENTRY_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Miscellaneous Entry Fields
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_0_ENTRY:ACE_TYPE
 */
#define VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xa9)

/** 
 * \details 
 * 0: Only untagged frames must be able to match this entry.
 * 1: Only tagged frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Is set when the frame was matched in the VCAP IS0 .
 *
 * \details 
 * 0: Match frames, which did not have match in VCAP IS0.
 * 1: Match frames, which did have match in VCAP IS0.

 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * 1. lookup: Frame hit an ingress router leg in ANA_L3.
 * 2. lookup: Frame is to be L3 forwarded (as decided in ANA_L3).

 *
 * \details 
 * 0: Only frames that are not subject to layer 3 routing must be able to
 * match this entry.
 * 1: Only frames that are subject to layer 3 routing must be able to match
 * this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE . L3_RT
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE_L3_RT  VTSS_BIT(22)

/** 
 * \brief
 * Match only frames with (SMAC, SIP) match in LPM table in ANA_L3.
 *
 * \details 
 * 0 : Only frames with no (SMAC, SIP) match in LPM table must match this
 * entry.
 * 1 : Only frames with (SMAC, SIP) match in LPM table must match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE . L3_SMAC_SIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE_L3_SMAC_SIP_MATCH  VTSS_BIT(21)

/** 
 * \brief
 * Match only frames with (DMAC, DIP) match in LPM table in ANA_L3.
 *
 * \details 
 * 0 : Only frames with no (DMAC, DIP) match in LPM table must match this
 * entry.
 * 1 : Only frames with (DMAC, DIP) match in LPM table must match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE . L3_DMAC_DIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE_L3_DMAC_DIP_MATCH  VTSS_BIT(23)

/** 
 * \brief
 * Control whether entry shall match frames, which are to be discarded.

 *
 * \details 
 * 0: Match only frames, which shall be discarded.
 * 1: Match only frames, which shall not be discarded.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * For frames, which did not have a match in VCAP IS0:
 * This field is matched against the VLAN ID resulting from the VLAN
 * classification in the ANA_CL.
 * 
 * If frame is to be routed (as decided by ANA_L3) and
 * ANA_CL:PORT:ADV_CL_CFG.SEC_ROUTE_HANDLING_ENA is set, then 
 * ingress VID will be used in first VCAP IS2 lookup and egress VID will be
 * used in second VCAP IS2 lookup.
 * 
 * For frames, which did have a match in VCAP IS0:
 * This field is matched against the ISDX value resulting from VCAP IS0.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * This field is matched against the user priority (UPRIO) resulting from
 * the VLAN classification in the ANA_CL.

 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * This field is matched against the Canonical Form Indicator (CFI)
 * resulting from the VLAN classification in ANA_CL.
 * 
 * The CFI bit is also referred to as Drop Eligible Indicator (DEI).
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Control whether frames with broadcast outer DMAC (0xFFFFFFFFFFFF) shall
 * be able to match this entry.

 *
 * \details 
 * 0: Broadcast frames must not be able to match this entry.
 * 1: Broadcast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Control whether frames with multicast outer DMAC shall be able to match
 * this entry. 
 *
 * \details 
 * 0: Multicast frames must not be able to match this entry.
 * 1: Multicast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * This field defines if an entry applies to first, second, or both lookups
 * in VCAP IS2.
 *
 * \details 
 * 0: The entry is used only for first lookup.
 * 1: The entry is used only for second lookup.
 * 
 * To match both first and second lookup VCAP_IS2:*_MASK:FIRST.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_ENTRY_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Ingress Port Mask
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_0_ENTRY:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xaa)


/** 
 * \brief Policy Association Group
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_0_ENTRY:ACE_PAG
 */
#define VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_PAG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xab)

/** 
 * \brief
 * This field is matched against the Policy Association Group resulting
 * from the PAG classification in VCAP_S0 or VCAP_S1.
 *
 * \details 
 * 0: Only frames from ingress ports that map to PAG 0 must be able to
 * match this entry.
 * 1: Only frames from ingress ports that map to PAG 1 must be able to
 * match this entry.
 * ...
 * n: Only frames from ingress ports that map to PAG n must be able to
 * match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_ENTRY_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_CUSTOM_0_ENTRY_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_CUSTOM_0_ENTRY_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Custom data - Bit 31:0
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_0
 */
#define VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_0  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xac)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_0.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_0 . CUSTOM_DATA_0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Custom data - Bit 63:32
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_1
 */
#define VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_1  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xad)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_0 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_1 . CUSTOM_DATA_1
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Custom data - Bit 95:64
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_2
 */
#define VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_2  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xae)

/** 
 * \brief
 *  Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_1
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_2 . CUSTOM_DATA_2
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Custom data - Bit 127:96
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_3
 */
#define VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_3  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xaf)

/** 
 * \brief
 *  Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_1 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_3 . CUSTOM_DATA_3
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_4
 */
#define VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_4  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xb0)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_4 . CUSTOM_DATA_4
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_5
 */
#define VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_5  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xb1)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_2 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_5 . CUSTOM_DATA_5
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_6
 */
#define VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_6  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xb2)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_3
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_6 . CUSTOM_DATA_6
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_7
 */
#define VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_7  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xb3)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_3 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_7 . CUSTOM_DATA_7
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_0_ENTRY_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VCAP_IS2:CUSTOM_1_ENTRY
 *
 * Entry data for entry type CUSTOM_1.
 */


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_1_ENTRY:ACE_VLD
 */
#define VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_VLD  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xb8)

/** 
 * \brief
 * This field sets the entry to valid, and it must not be set to
 * "don't-care".
 *
 * \details 
 * 0: The entry is not valid.
 * 1: The entry is valid.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_ENTRY_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Miscellaneous Entry Fields
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_1_ENTRY:ACE_TYPE
 */
#define VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xb9)

/** 
 * \details 
 * 0: Only untagged frames must be able to match this entry.
 * 1: Only tagged frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Is set when the frame was matched in the VCAP IS0 .
 *
 * \details 
 * 0: Match frames, which did not have match in VCAP IS0.
 * 1: Match frames, which did have match in VCAP IS0.

 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * 1. lookup: Frame hit an ingress router leg in ANA_L3.
 * 2. lookup: Frame is to be L3 forwarded (as decided in ANA_L3).

 *
 * \details 
 * 0: Only frames that are not subject to layer 3 routing must be able to
 * match this entry.
 * 1: Only frames that are subject to layer 3 routing must be able to match
 * this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE . L3_RT
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE_L3_RT  VTSS_BIT(22)

/** 
 * \brief
 * Match only frames with (SMAC, SIP) match in LPM table in ANA_L3.
 *
 * \details 
 * 0 : Only frames with no (SMAC, SIP) match in LPM table must match this
 * entry.
 * 1 : Only frames with (SMAC, SIP) match in LPM table must match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE . L3_SMAC_SIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE_L3_SMAC_SIP_MATCH  VTSS_BIT(21)

/** 
 * \brief
 * Match only frames with (DMAC, DIP) match in LPM table in ANA_L3.
 *
 * \details 
 * 0 : Only frames with no (DMAC, DIP) match in LPM table must match this
 * entry.
 * 1 : Only frames with (DMAC, DIP) match in LPM table must match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE . L3_DMAC_DIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE_L3_DMAC_DIP_MATCH  VTSS_BIT(23)

/** 
 * \brief
 * Control whether entry shall match frames, which are to be discarded.

 *
 * \details 
 * 0: Match only frames, which shall be discarded.
 * 1: Match only frames, which shall not be discarded.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * For frames, which did not have a match in VCAP IS0:
 * This field is matched against the VLAN ID resulting from the VLAN
 * classification in the ANA_CL.
 * 
 * If frame is to be routed (as decided by ANA_L3) and
 * ANA_CL:PORT:ADV_CL_CFG.SEC_ROUTE_HANDLING_ENA is set, then 
 * ingress VID will be used in first VCAP IS2 lookup and egress VID will be
 * used in second VCAP IS2 lookup.
 * 
 * For frames, which did have a match in VCAP IS0:
 * This field is matched against the ISDX value resulting from VCAP IS0.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * This field is matched against the user priority (UPRIO) resulting from
 * the VLAN classification in the ANA_CL.

 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * This field is matched against the Canonical Form Indicator (CFI)
 * resulting from the VLAN classification in ANA_CL.
 * 
 * The CFI bit is also referred to as Drop Eligible Indicator (DEI).
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Control whether frames with broadcast outer DMAC (0xFFFFFFFFFFFF) shall
 * be able to match this entry.

 *
 * \details 
 * 0: Broadcast frames must not be able to match this entry.
 * 1: Broadcast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Control whether frames with multicast outer DMAC shall be able to match
 * this entry. 
 *
 * \details 
 * 0: Multicast frames must not be able to match this entry.
 * 1: Multicast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * This field defines if an entry applies to first, second, or both lookups
 * in VCAP IS2.
 *
 * \details 
 * 0: The entry is used only for first lookup.
 * 1: The entry is used only for second lookup.
 * 
 * To match both first and second lookup VCAP_IS2:*_MASK:FIRST.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_ENTRY_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Ingress Port Mask
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_1_ENTRY:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xba)


/** 
 * \brief Policy Association Group
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_1_ENTRY:ACE_PAG
 */
#define VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_PAG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xbb)

/** 
 * \brief
 * This field is matched against the Policy Association Group resulting
 * from the PAG classification in VCAP_S0 or VCAP_S1.
 *
 * \details 
 * 0: Only frames from ingress ports that map to PAG 0 must be able to
 * match this entry.
 * 1: Only frames from ingress ports that map to PAG 1 must be able to
 * match this entry.
 * ...
 * n: Only frames from ingress ports that map to PAG n must be able to
 * match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_ENTRY_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_CUSTOM_1_ENTRY_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_CUSTOM_1_ENTRY_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Custom data - Bit 31:0
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_0
 */
#define VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_0  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xbc)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_0.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_0 . CUSTOM_DATA_0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Custom data - Bit 63:32
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_1
 */
#define VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_1  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xbd)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_0 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_1 . CUSTOM_DATA_1
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Custom data - Bit 95:64
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_2
 */
#define VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_2  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xbe)

/** 
 * \brief
 *  Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_1
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_2 . CUSTOM_DATA_2
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Custom data - Bit 127:96
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_3
 */
#define VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_3  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xbf)

/** 
 * \brief
 *  Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_1 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_3 . CUSTOM_DATA_3
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_4
 */
#define VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_4  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xc0)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_4 . CUSTOM_DATA_4
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_5
 */
#define VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_5  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xc1)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_2 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_5 . CUSTOM_DATA_5
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_6
 */
#define VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_6  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xc2)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_3
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_6 . CUSTOM_DATA_6
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_7
 */
#define VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_7  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xc3)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_3 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_7 . CUSTOM_DATA_7
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_1_ENTRY_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VCAP_IS2:CUSTOM_2_ENTRY
 *
 * Entry data for entry type CUSTOM_2.
 */


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_2_ENTRY:ACE_VLD
 */
#define VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_VLD  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xc8)

/** 
 * \brief
 * This field sets the entry to valid, and it must not be set to
 * "don't-care".
 *
 * \details 
 * 0: The entry is not valid.
 * 1: The entry is valid.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_ENTRY_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Miscellaneous Entry Fields
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_2_ENTRY:ACE_TYPE
 */
#define VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xc9)

/** 
 * \details 
 * 0: Only untagged frames must be able to match this entry.
 * 1: Only tagged frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Is set when the frame was matched in the VCAP IS0 .
 *
 * \details 
 * 0: Match frames, which did not have match in VCAP IS0.
 * 1: Match frames, which did have match in VCAP IS0.

 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * 1. lookup: Frame hit an ingress router leg in ANA_L3.
 * 2. lookup: Frame is to be L3 forwarded (as decided in ANA_L3).

 *
 * \details 
 * 0: Only frames that are not subject to layer 3 routing must be able to
 * match this entry.
 * 1: Only frames that are subject to layer 3 routing must be able to match
 * this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE . L3_RT
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE_L3_RT  VTSS_BIT(22)

/** 
 * \brief
 * Match only frames with (SMAC, SIP) match in LPM table in ANA_L3.
 *
 * \details 
 * 0 : Only frames with no (SMAC, SIP) match in LPM table must match this
 * entry.
 * 1 : Only frames with (SMAC, SIP) match in LPM table must match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE . L3_SMAC_SIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE_L3_SMAC_SIP_MATCH  VTSS_BIT(21)

/** 
 * \brief
 * Match only frames with (DMAC, DIP) match in LPM table in ANA_L3.
 *
 * \details 
 * 0 : Only frames with no (DMAC, DIP) match in LPM table must match this
 * entry.
 * 1 : Only frames with (DMAC, DIP) match in LPM table must match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE . L3_DMAC_DIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE_L3_DMAC_DIP_MATCH  VTSS_BIT(23)

/** 
 * \brief
 * Control whether entry shall match frames, which are to be discarded.

 *
 * \details 
 * 0: Match only frames, which shall be discarded.
 * 1: Match only frames, which shall not be discarded.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * For frames, which did not have a match in VCAP IS0:
 * This field is matched against the VLAN ID resulting from the VLAN
 * classification in the ANA_CL.
 * 
 * If frame is to be routed (as decided by ANA_L3) and
 * ANA_CL:PORT:ADV_CL_CFG.SEC_ROUTE_HANDLING_ENA is set, then 
 * ingress VID will be used in first VCAP IS2 lookup and egress VID will be
 * used in second VCAP IS2 lookup.
 * 
 * For frames, which did have a match in VCAP IS0:
 * This field is matched against the ISDX value resulting from VCAP IS0.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * This field is matched against the user priority (UPRIO) resulting from
 * the VLAN classification in the ANA_CL.

 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * This field is matched against the Canonical Form Indicator (CFI)
 * resulting from the VLAN classification in ANA_CL.
 * 
 * The CFI bit is also referred to as Drop Eligible Indicator (DEI).
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Control whether frames with broadcast outer DMAC (0xFFFFFFFFFFFF) shall
 * be able to match this entry.

 *
 * \details 
 * 0: Broadcast frames must not be able to match this entry.
 * 1: Broadcast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Control whether frames with multicast outer DMAC shall be able to match
 * this entry. 
 *
 * \details 
 * 0: Multicast frames must not be able to match this entry.
 * 1: Multicast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * This field defines if an entry applies to first, second, or both lookups
 * in VCAP IS2.
 *
 * \details 
 * 0: The entry is used only for first lookup.
 * 1: The entry is used only for second lookup.
 * 
 * To match both first and second lookup VCAP_IS2:*_MASK:FIRST.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_ENTRY_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Ingress Port Mask
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_2_ENTRY:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xca)


/** 
 * \brief Policy Association Group
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_2_ENTRY:ACE_PAG
 */
#define VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_PAG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xcb)

/** 
 * \brief
 * This field is matched against the Policy Association Group resulting
 * from the PAG classification in VCAP_S0 or VCAP_S1.
 *
 * \details 
 * 0: Only frames from ingress ports that map to PAG 0 must be able to
 * match this entry.
 * 1: Only frames from ingress ports that map to PAG 1 must be able to
 * match this entry.
 * ...
 * n: Only frames from ingress ports that map to PAG n must be able to
 * match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_ENTRY_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_CUSTOM_2_ENTRY_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_CUSTOM_2_ENTRY_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Custom data - Bit 31:0
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_0
 */
#define VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_0  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xcc)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_0.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_0 . CUSTOM_DATA_0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Custom data - Bit 63:32
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_1
 */
#define VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_1  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xcd)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_0 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_1 . CUSTOM_DATA_1
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Custom data - Bit 95:64
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_2
 */
#define VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_2  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xce)

/** 
 * \brief
 *  Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_1
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_2 . CUSTOM_DATA_2
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Custom data - Bit 127:96
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_3
 */
#define VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_3  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xcf)

/** 
 * \brief
 *  Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_1 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_3 . CUSTOM_DATA_3
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_4
 */
#define VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_4  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xd0)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_4 . CUSTOM_DATA_4
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_5
 */
#define VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_5  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xd1)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_2 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_5 . CUSTOM_DATA_5
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_6
 */
#define VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_6  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xd2)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_3
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_6 . CUSTOM_DATA_6
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_7
 */
#define VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_7  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xd3)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_3 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_7 . CUSTOM_DATA_7
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_2_ENTRY_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VCAP_IS2:CUSTOM_3_ENTRY
 *
 * Entry data for entry type CUSTOM_3.
 */


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_3_ENTRY:ACE_VLD
 */
#define VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_VLD  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xd8)

/** 
 * \brief
 * This field sets the entry to valid, and it must not be set to
 * "don't-care".
 *
 * \details 
 * 0: The entry is not valid.
 * 1: The entry is valid.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_ENTRY_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Miscellaneous Entry Fields
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_3_ENTRY:ACE_TYPE
 */
#define VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xd9)

/** 
 * \details 
 * 0: Only untagged frames must be able to match this entry.
 * 1: Only tagged frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Is set when the frame was matched in the VCAP IS0 .
 *
 * \details 
 * 0: Match frames, which did not have match in VCAP IS0.
 * 1: Match frames, which did have match in VCAP IS0.

 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * 1. lookup: Frame hit an ingress router leg in ANA_L3.
 * 2. lookup: Frame is to be L3 forwarded (as decided in ANA_L3).

 *
 * \details 
 * 0: Only frames that are not subject to layer 3 routing must be able to
 * match this entry.
 * 1: Only frames that are subject to layer 3 routing must be able to match
 * this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE . L3_RT
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE_L3_RT  VTSS_BIT(22)

/** 
 * \brief
 * Match only frames with (SMAC, SIP) match in LPM table in ANA_L3.
 *
 * \details 
 * 0 : Only frames with no (SMAC, SIP) match in LPM table must match this
 * entry.
 * 1 : Only frames with (SMAC, SIP) match in LPM table must match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE . L3_SMAC_SIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE_L3_SMAC_SIP_MATCH  VTSS_BIT(21)

/** 
 * \brief
 * Match only frames with (DMAC, DIP) match in LPM table in ANA_L3.
 *
 * \details 
 * 0 : Only frames with no (DMAC, DIP) match in LPM table must match this
 * entry.
 * 1 : Only frames with (DMAC, DIP) match in LPM table must match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE . L3_DMAC_DIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE_L3_DMAC_DIP_MATCH  VTSS_BIT(23)

/** 
 * \brief
 * Control whether entry shall match frames, which are to be discarded.

 *
 * \details 
 * 0: Match only frames, which shall be discarded.
 * 1: Match only frames, which shall not be discarded.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * For frames, which did not have a match in VCAP IS0:
 * This field is matched against the VLAN ID resulting from the VLAN
 * classification in the ANA_CL.
 * 
 * If frame is to be routed (as decided by ANA_L3) and
 * ANA_CL:PORT:ADV_CL_CFG.SEC_ROUTE_HANDLING_ENA is set, then 
 * ingress VID will be used in first VCAP IS2 lookup and egress VID will be
 * used in second VCAP IS2 lookup.
 * 
 * For frames, which did have a match in VCAP IS0:
 * This field is matched against the ISDX value resulting from VCAP IS0.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * This field is matched against the user priority (UPRIO) resulting from
 * the VLAN classification in the ANA_CL.

 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * This field is matched against the Canonical Form Indicator (CFI)
 * resulting from the VLAN classification in ANA_CL.
 * 
 * The CFI bit is also referred to as Drop Eligible Indicator (DEI).
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Control whether frames with broadcast outer DMAC (0xFFFFFFFFFFFF) shall
 * be able to match this entry.

 *
 * \details 
 * 0: Broadcast frames must not be able to match this entry.
 * 1: Broadcast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Control whether frames with multicast outer DMAC shall be able to match
 * this entry. 
 *
 * \details 
 * 0: Multicast frames must not be able to match this entry.
 * 1: Multicast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * This field defines if an entry applies to first, second, or both lookups
 * in VCAP IS2.
 *
 * \details 
 * 0: The entry is used only for first lookup.
 * 1: The entry is used only for second lookup.
 * 
 * To match both first and second lookup VCAP_IS2:*_MASK:FIRST.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_ENTRY_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Ingress Port Mask
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_3_ENTRY:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xda)


/** 
 * \brief Policy Association Group
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_3_ENTRY:ACE_PAG
 */
#define VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_PAG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xdb)

/** 
 * \brief
 * This field is matched against the Policy Association Group resulting
 * from the PAG classification in VCAP_S0 or VCAP_S1.
 *
 * \details 
 * 0: Only frames from ingress ports that map to PAG 0 must be able to
 * match this entry.
 * 1: Only frames from ingress ports that map to PAG 1 must be able to
 * match this entry.
 * ...
 * n: Only frames from ingress ports that map to PAG n must be able to
 * match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_ENTRY_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_CUSTOM_3_ENTRY_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_CUSTOM_3_ENTRY_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Custom data - Bit 31:0
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_0
 */
#define VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_0  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xdc)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_0.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_0 . CUSTOM_DATA_0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Custom data - Bit 63:32
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_1
 */
#define VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_1  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xdd)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_0 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_1 . CUSTOM_DATA_1
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Custom data - Bit 95:64
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_2
 */
#define VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_2  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xde)

/** 
 * \brief
 *  Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_1
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_2 . CUSTOM_DATA_2
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Custom data - Bit 127:96
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_3
 */
#define VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_3  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xdf)

/** 
 * \brief
 *  Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_1 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_3 . CUSTOM_DATA_3
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_4
 */
#define VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_4  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xe0)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_4 . CUSTOM_DATA_4
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_5
 */
#define VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_5  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xe1)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_2 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_5 . CUSTOM_DATA_5
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_6
 */
#define VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_6  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xe2)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_3
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_6 . CUSTOM_DATA_6
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_7
 */
#define VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_7  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xe3)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_3 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_7 . CUSTOM_DATA_7
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_3_ENTRY_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VCAP_IS2:CUSTOM_4_ENTRY
 *
 * Entry data for entry type CUSTOM_4.
 */


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_4_ENTRY:ACE_VLD
 */
#define VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_VLD  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xe8)

/** 
 * \brief
 * This field sets the entry to valid, and it must not be set to
 * "don't-care".
 *
 * \details 
 * 0: The entry is not valid.
 * 1: The entry is valid.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_ENTRY_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Miscellaneous Entry Fields
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_4_ENTRY:ACE_TYPE
 */
#define VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xe9)

/** 
 * \details 
 * 0: Only untagged frames must be able to match this entry.
 * 1: Only tagged frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Is set when the frame was matched in the VCAP IS0 .
 *
 * \details 
 * 0: Match frames, which did not have match in VCAP IS0.
 * 1: Match frames, which did have match in VCAP IS0.

 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * 1. lookup: Frame hit an ingress router leg in ANA_L3.
 * 2. lookup: Frame is to be L3 forwarded (as decided in ANA_L3).

 *
 * \details 
 * 0: Only frames that are not subject to layer 3 routing must be able to
 * match this entry.
 * 1: Only frames that are subject to layer 3 routing must be able to match
 * this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE . L3_RT
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE_L3_RT  VTSS_BIT(22)

/** 
 * \brief
 * Match only frames with (SMAC, SIP) match in LPM table in ANA_L3.
 *
 * \details 
 * 0 : Only frames with no (SMAC, SIP) match in LPM table must match this
 * entry.
 * 1 : Only frames with (SMAC, SIP) match in LPM table must match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE . L3_SMAC_SIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE_L3_SMAC_SIP_MATCH  VTSS_BIT(21)

/** 
 * \brief
 * Match only frames with (DMAC, DIP) match in LPM table in ANA_L3.
 *
 * \details 
 * 0 : Only frames with no (DMAC, DIP) match in LPM table must match this
 * entry.
 * 1 : Only frames with (DMAC, DIP) match in LPM table must match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE . L3_DMAC_DIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE_L3_DMAC_DIP_MATCH  VTSS_BIT(23)

/** 
 * \brief
 * Control whether entry shall match frames, which are to be discarded.

 *
 * \details 
 * 0: Match only frames, which shall be discarded.
 * 1: Match only frames, which shall not be discarded.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * For frames, which did not have a match in VCAP IS0:
 * This field is matched against the VLAN ID resulting from the VLAN
 * classification in the ANA_CL.
 * 
 * If frame is to be routed (as decided by ANA_L3) and
 * ANA_CL:PORT:ADV_CL_CFG.SEC_ROUTE_HANDLING_ENA is set, then 
 * ingress VID will be used in first VCAP IS2 lookup and egress VID will be
 * used in second VCAP IS2 lookup.
 * 
 * For frames, which did have a match in VCAP IS0:
 * This field is matched against the ISDX value resulting from VCAP IS0.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * This field is matched against the user priority (UPRIO) resulting from
 * the VLAN classification in the ANA_CL.

 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * This field is matched against the Canonical Form Indicator (CFI)
 * resulting from the VLAN classification in ANA_CL.
 * 
 * The CFI bit is also referred to as Drop Eligible Indicator (DEI).
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Control whether frames with broadcast outer DMAC (0xFFFFFFFFFFFF) shall
 * be able to match this entry.

 *
 * \details 
 * 0: Broadcast frames must not be able to match this entry.
 * 1: Broadcast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Control whether frames with multicast outer DMAC shall be able to match
 * this entry. 
 *
 * \details 
 * 0: Multicast frames must not be able to match this entry.
 * 1: Multicast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * This field defines if an entry applies to first, second, or both lookups
 * in VCAP IS2.
 *
 * \details 
 * 0: The entry is used only for first lookup.
 * 1: The entry is used only for second lookup.
 * 
 * To match both first and second lookup VCAP_IS2:*_MASK:FIRST.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_ENTRY_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Ingress Port Mask
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_4_ENTRY:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xea)


/** 
 * \brief Policy Association Group
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_4_ENTRY:ACE_PAG
 */
#define VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_PAG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xeb)

/** 
 * \brief
 * This field is matched against the Policy Association Group resulting
 * from the PAG classification in VCAP_S0 or VCAP_S1.
 *
 * \details 
 * 0: Only frames from ingress ports that map to PAG 0 must be able to
 * match this entry.
 * 1: Only frames from ingress ports that map to PAG 1 must be able to
 * match this entry.
 * ...
 * n: Only frames from ingress ports that map to PAG n must be able to
 * match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_ENTRY_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_CUSTOM_4_ENTRY_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_CUSTOM_4_ENTRY_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Custom data - Bit 31:0
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_0
 */
#define VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_0  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xec)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_0.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_0 . CUSTOM_DATA_0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Custom data - Bit 63:32
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_1
 */
#define VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_1  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xed)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_0 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_1 . CUSTOM_DATA_1
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Custom data - Bit 95:64
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_2
 */
#define VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_2  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xee)

/** 
 * \brief
 *  Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_1
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_2 . CUSTOM_DATA_2
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Custom data - Bit 127:96
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_3
 */
#define VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_3  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xef)

/** 
 * \brief
 *  Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_1 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_3 . CUSTOM_DATA_3
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_4
 */
#define VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_4  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xf0)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_4 . CUSTOM_DATA_4
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_5
 */
#define VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_5  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xf1)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_2 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_5 . CUSTOM_DATA_5
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_6
 */
#define VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_6  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xf2)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_3
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_6 . CUSTOM_DATA_6
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_7
 */
#define VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_7  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xf3)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_3 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_7 . CUSTOM_DATA_7
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_4_ENTRY_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VCAP_IS2:CUSTOM_5_ENTRY
 *
 * Entry data for entry type CUSTOM_5.
 */


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_5_ENTRY:ACE_VLD
 */
#define VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_VLD  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xf8)

/** 
 * \brief
 * This field sets the entry to valid, and it must not be set to
 * "don't-care".
 *
 * \details 
 * 0: The entry is not valid.
 * 1: The entry is valid.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_ENTRY_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Miscellaneous Entry Fields
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_5_ENTRY:ACE_TYPE
 */
#define VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xf9)

/** 
 * \details 
 * 0: Only untagged frames must be able to match this entry.
 * 1: Only tagged frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Is set when the frame was matched in the VCAP IS0 .
 *
 * \details 
 * 0: Match frames, which did not have match in VCAP IS0.
 * 1: Match frames, which did have match in VCAP IS0.

 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * 1. lookup: Frame hit an ingress router leg in ANA_L3.
 * 2. lookup: Frame is to be L3 forwarded (as decided in ANA_L3).

 *
 * \details 
 * 0: Only frames that are not subject to layer 3 routing must be able to
 * match this entry.
 * 1: Only frames that are subject to layer 3 routing must be able to match
 * this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE . L3_RT
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE_L3_RT  VTSS_BIT(22)

/** 
 * \brief
 * Match only frames with (SMAC, SIP) match in LPM table in ANA_L3.
 *
 * \details 
 * 0 : Only frames with no (SMAC, SIP) match in LPM table must match this
 * entry.
 * 1 : Only frames with (SMAC, SIP) match in LPM table must match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE . L3_SMAC_SIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE_L3_SMAC_SIP_MATCH  VTSS_BIT(21)

/** 
 * \brief
 * Match only frames with (DMAC, DIP) match in LPM table in ANA_L3.
 *
 * \details 
 * 0 : Only frames with no (DMAC, DIP) match in LPM table must match this
 * entry.
 * 1 : Only frames with (DMAC, DIP) match in LPM table must match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE . L3_DMAC_DIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE_L3_DMAC_DIP_MATCH  VTSS_BIT(23)

/** 
 * \brief
 * Control whether entry shall match frames, which are to be discarded.

 *
 * \details 
 * 0: Match only frames, which shall be discarded.
 * 1: Match only frames, which shall not be discarded.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * For frames, which did not have a match in VCAP IS0:
 * This field is matched against the VLAN ID resulting from the VLAN
 * classification in the ANA_CL.
 * 
 * If frame is to be routed (as decided by ANA_L3) and
 * ANA_CL:PORT:ADV_CL_CFG.SEC_ROUTE_HANDLING_ENA is set, then 
 * ingress VID will be used in first VCAP IS2 lookup and egress VID will be
 * used in second VCAP IS2 lookup.
 * 
 * For frames, which did have a match in VCAP IS0:
 * This field is matched against the ISDX value resulting from VCAP IS0.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * This field is matched against the user priority (UPRIO) resulting from
 * the VLAN classification in the ANA_CL.

 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * This field is matched against the Canonical Form Indicator (CFI)
 * resulting from the VLAN classification in ANA_CL.
 * 
 * The CFI bit is also referred to as Drop Eligible Indicator (DEI).
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Control whether frames with broadcast outer DMAC (0xFFFFFFFFFFFF) shall
 * be able to match this entry.

 *
 * \details 
 * 0: Broadcast frames must not be able to match this entry.
 * 1: Broadcast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Control whether frames with multicast outer DMAC shall be able to match
 * this entry. 
 *
 * \details 
 * 0: Multicast frames must not be able to match this entry.
 * 1: Multicast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * This field defines if an entry applies to first, second, or both lookups
 * in VCAP IS2.
 *
 * \details 
 * 0: The entry is used only for first lookup.
 * 1: The entry is used only for second lookup.
 * 
 * To match both first and second lookup VCAP_IS2:*_MASK:FIRST.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_ENTRY_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Ingress Port Mask
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_5_ENTRY:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xfa)


/** 
 * \brief Policy Association Group
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_5_ENTRY:ACE_PAG
 */
#define VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_PAG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xfb)

/** 
 * \brief
 * This field is matched against the Policy Association Group resulting
 * from the PAG classification in VCAP_S0 or VCAP_S1.
 *
 * \details 
 * 0: Only frames from ingress ports that map to PAG 0 must be able to
 * match this entry.
 * 1: Only frames from ingress ports that map to PAG 1 must be able to
 * match this entry.
 * ...
 * n: Only frames from ingress ports that map to PAG n must be able to
 * match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_ENTRY_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_CUSTOM_5_ENTRY_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_CUSTOM_5_ENTRY_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Custom data - Bit 31:0
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_0
 */
#define VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_0  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xfc)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_0.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_0 . CUSTOM_DATA_0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Custom data - Bit 63:32
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_1
 */
#define VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_1  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xfd)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_0 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_1 . CUSTOM_DATA_1
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Custom data - Bit 95:64
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_2
 */
#define VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_2  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xfe)

/** 
 * \brief
 *  Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_1
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_2 . CUSTOM_DATA_2
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Custom data - Bit 127:96
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_3
 */
#define VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_3  VTSS_IOREG(VTSS_TO_VCAP_IS2,0xff)

/** 
 * \brief
 *  Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_1 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_3 . CUSTOM_DATA_3
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_4
 */
#define VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_4  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x100)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_4 . CUSTOM_DATA_4
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_5
 */
#define VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_5  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x101)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_2 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_5 . CUSTOM_DATA_5
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_6
 */
#define VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_6  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x102)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_3
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_6 . CUSTOM_DATA_6
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_7
 */
#define VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_7  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x103)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_3 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_7 . CUSTOM_DATA_7
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_5_ENTRY_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VCAP_IS2:CUSTOM_6_ENTRY
 *
 * Entry data for entry type CUSTOM_6.
 */


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_6_ENTRY:ACE_VLD
 */
#define VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_VLD  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x108)

/** 
 * \brief
 * This field sets the entry to valid, and it must not be set to
 * "don't-care".
 *
 * \details 
 * 0: The entry is not valid.
 * 1: The entry is valid.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_ENTRY_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Miscellaneous Entry Fields
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_6_ENTRY:ACE_TYPE
 */
#define VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x109)

/** 
 * \details 
 * 0: Only untagged frames must be able to match this entry.
 * 1: Only tagged frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Is set when the frame was matched in the VCAP IS0 .
 *
 * \details 
 * 0: Match frames, which did not have match in VCAP IS0.
 * 1: Match frames, which did have match in VCAP IS0.

 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * 1. lookup: Frame hit an ingress router leg in ANA_L3.
 * 2. lookup: Frame is to be L3 forwarded (as decided in ANA_L3).

 *
 * \details 
 * 0: Only frames that are not subject to layer 3 routing must be able to
 * match this entry.
 * 1: Only frames that are subject to layer 3 routing must be able to match
 * this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE . L3_RT
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE_L3_RT  VTSS_BIT(22)

/** 
 * \brief
 * Match only frames with (SMAC, SIP) match in LPM table in ANA_L3.
 *
 * \details 
 * 0 : Only frames with no (SMAC, SIP) match in LPM table must match this
 * entry.
 * 1 : Only frames with (SMAC, SIP) match in LPM table must match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE . L3_SMAC_SIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE_L3_SMAC_SIP_MATCH  VTSS_BIT(21)

/** 
 * \brief
 * Match only frames with (DMAC, DIP) match in LPM table in ANA_L3.
 *
 * \details 
 * 0 : Only frames with no (DMAC, DIP) match in LPM table must match this
 * entry.
 * 1 : Only frames with (DMAC, DIP) match in LPM table must match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE . L3_DMAC_DIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE_L3_DMAC_DIP_MATCH  VTSS_BIT(23)

/** 
 * \brief
 * Control whether entry shall match frames, which are to be discarded.

 *
 * \details 
 * 0: Match only frames, which shall be discarded.
 * 1: Match only frames, which shall not be discarded.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * For frames, which did not have a match in VCAP IS0:
 * This field is matched against the VLAN ID resulting from the VLAN
 * classification in the ANA_CL.
 * 
 * If frame is to be routed (as decided by ANA_L3) and
 * ANA_CL:PORT:ADV_CL_CFG.SEC_ROUTE_HANDLING_ENA is set, then 
 * ingress VID will be used in first VCAP IS2 lookup and egress VID will be
 * used in second VCAP IS2 lookup.
 * 
 * For frames, which did have a match in VCAP IS0:
 * This field is matched against the ISDX value resulting from VCAP IS0.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * This field is matched against the user priority (UPRIO) resulting from
 * the VLAN classification in the ANA_CL.

 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * This field is matched against the Canonical Form Indicator (CFI)
 * resulting from the VLAN classification in ANA_CL.
 * 
 * The CFI bit is also referred to as Drop Eligible Indicator (DEI).
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Control whether frames with broadcast outer DMAC (0xFFFFFFFFFFFF) shall
 * be able to match this entry.

 *
 * \details 
 * 0: Broadcast frames must not be able to match this entry.
 * 1: Broadcast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Control whether frames with multicast outer DMAC shall be able to match
 * this entry. 
 *
 * \details 
 * 0: Multicast frames must not be able to match this entry.
 * 1: Multicast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * This field defines if an entry applies to first, second, or both lookups
 * in VCAP IS2.
 *
 * \details 
 * 0: The entry is used only for first lookup.
 * 1: The entry is used only for second lookup.
 * 
 * To match both first and second lookup VCAP_IS2:*_MASK:FIRST.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_ENTRY_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Ingress Port Mask
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_6_ENTRY:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x10a)


/** 
 * \brief Policy Association Group
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_6_ENTRY:ACE_PAG
 */
#define VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_PAG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x10b)

/** 
 * \brief
 * This field is matched against the Policy Association Group resulting
 * from the PAG classification in VCAP_S0 or VCAP_S1.
 *
 * \details 
 * 0: Only frames from ingress ports that map to PAG 0 must be able to
 * match this entry.
 * 1: Only frames from ingress ports that map to PAG 1 must be able to
 * match this entry.
 * ...
 * n: Only frames from ingress ports that map to PAG n must be able to
 * match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_ENTRY_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_CUSTOM_6_ENTRY_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_CUSTOM_6_ENTRY_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Custom data - Bit 31:0
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_0
 */
#define VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_0  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x10c)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_0.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_0 . CUSTOM_DATA_0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Custom data - Bit 63:32
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_1
 */
#define VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_1  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x10d)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_0 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_1 . CUSTOM_DATA_1
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Custom data - Bit 95:64
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_2
 */
#define VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_2  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x10e)

/** 
 * \brief
 *  Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_1
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_2 . CUSTOM_DATA_2
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Custom data - Bit 127:96
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_3
 */
#define VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_3  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x10f)

/** 
 * \brief
 *  Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_1 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_3 . CUSTOM_DATA_3
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_4
 */
#define VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_4  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x110)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_4 . CUSTOM_DATA_4
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_5
 */
#define VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_5  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x111)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_2 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_5 . CUSTOM_DATA_5
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_6
 */
#define VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_6  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x112)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_3
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_6 . CUSTOM_DATA_6
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_7
 */
#define VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_7  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x113)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_3 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_7 . CUSTOM_DATA_7
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_6_ENTRY_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VCAP_IS2:CUSTOM_7_ENTRY
 *
 * Entry data for entry type CUSTOM_7.
 */


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_7_ENTRY:ACE_VLD
 */
#define VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_VLD  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x118)

/** 
 * \brief
 * This field sets the entry to valid, and it must not be set to
 * "don't-care".
 *
 * \details 
 * 0: The entry is not valid.
 * 1: The entry is valid.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_ENTRY_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Miscellaneous Entry Fields
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_7_ENTRY:ACE_TYPE
 */
#define VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x119)

/** 
 * \details 
 * 0: Only untagged frames must be able to match this entry.
 * 1: Only tagged frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Is set when the frame was matched in the VCAP IS0 .
 *
 * \details 
 * 0: Match frames, which did not have match in VCAP IS0.
 * 1: Match frames, which did have match in VCAP IS0.

 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * 1. lookup: Frame hit an ingress router leg in ANA_L3.
 * 2. lookup: Frame is to be L3 forwarded (as decided in ANA_L3).

 *
 * \details 
 * 0: Only frames that are not subject to layer 3 routing must be able to
 * match this entry.
 * 1: Only frames that are subject to layer 3 routing must be able to match
 * this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE . L3_RT
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE_L3_RT  VTSS_BIT(22)

/** 
 * \brief
 * Match only frames with (SMAC, SIP) match in LPM table in ANA_L3.
 *
 * \details 
 * 0 : Only frames with no (SMAC, SIP) match in LPM table must match this
 * entry.
 * 1 : Only frames with (SMAC, SIP) match in LPM table must match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE . L3_SMAC_SIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE_L3_SMAC_SIP_MATCH  VTSS_BIT(21)

/** 
 * \brief
 * Match only frames with (DMAC, DIP) match in LPM table in ANA_L3.
 *
 * \details 
 * 0 : Only frames with no (DMAC, DIP) match in LPM table must match this
 * entry.
 * 1 : Only frames with (DMAC, DIP) match in LPM table must match this
 * entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE . L3_DMAC_DIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE_L3_DMAC_DIP_MATCH  VTSS_BIT(23)

/** 
 * \brief
 * Control whether entry shall match frames, which are to be discarded.

 *
 * \details 
 * 0: Match only frames, which shall be discarded.
 * 1: Match only frames, which shall not be discarded.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * For frames, which did not have a match in VCAP IS0:
 * This field is matched against the VLAN ID resulting from the VLAN
 * classification in the ANA_CL.
 * 
 * If frame is to be routed (as decided by ANA_L3) and
 * ANA_CL:PORT:ADV_CL_CFG.SEC_ROUTE_HANDLING_ENA is set, then 
 * ingress VID will be used in first VCAP IS2 lookup and egress VID will be
 * used in second VCAP IS2 lookup.
 * 
 * For frames, which did have a match in VCAP IS0:
 * This field is matched against the ISDX value resulting from VCAP IS0.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * This field is matched against the user priority (UPRIO) resulting from
 * the VLAN classification in the ANA_CL.

 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * This field is matched against the Canonical Form Indicator (CFI)
 * resulting from the VLAN classification in ANA_CL.
 * 
 * The CFI bit is also referred to as Drop Eligible Indicator (DEI).
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Control whether frames with broadcast outer DMAC (0xFFFFFFFFFFFF) shall
 * be able to match this entry.

 *
 * \details 
 * 0: Broadcast frames must not be able to match this entry.
 * 1: Broadcast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Control whether frames with multicast outer DMAC shall be able to match
 * this entry. 
 *
 * \details 
 * 0: Multicast frames must not be able to match this entry.
 * 1: Multicast frames must be able to match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * This field defines if an entry applies to first, second, or both lookups
 * in VCAP IS2.
 *
 * \details 
 * 0: The entry is used only for first lookup.
 * 1: The entry is used only for second lookup.
 * 
 * To match both first and second lookup VCAP_IS2:*_MASK:FIRST.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_ENTRY_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Ingress Port Mask
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_7_ENTRY:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x11a)


/** 
 * \brief Policy Association Group
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_7_ENTRY:ACE_PAG
 */
#define VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_PAG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x11b)

/** 
 * \brief
 * This field is matched against the Policy Association Group resulting
 * from the PAG classification in VCAP_S0 or VCAP_S1.
 *
 * \details 
 * 0: Only frames from ingress ports that map to PAG 0 must be able to
 * match this entry.
 * 1: Only frames from ingress ports that map to PAG 1 must be able to
 * match this entry.
 * ...
 * n: Only frames from ingress ports that map to PAG n must be able to
 * match this entry.
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_ENTRY_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_CUSTOM_7_ENTRY_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_CUSTOM_7_ENTRY_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Custom data - Bit 31:0
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_0
 */
#define VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_0  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x11c)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_0.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_0 . CUSTOM_DATA_0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Custom data - Bit 63:32
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_1
 */
#define VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_1  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x11d)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_0 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_1 . CUSTOM_DATA_1
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Custom data - Bit 95:64
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_2
 */
#define VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_2  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x11e)

/** 
 * \brief
 *  Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_1
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_2 . CUSTOM_DATA_2
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Custom data - Bit 127:96
 *
 * \details
 * Register: \a VCAP_IS2:CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_3
 */
#define VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_3  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x11f)

/** 
 * \brief
 *  Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_1 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_3 . CUSTOM_DATA_3
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_4
 */
#define VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_4  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x120)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_4 . CUSTOM_DATA_4
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_5
 */
#define VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_5  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x121)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_2 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_5 . CUSTOM_DATA_5
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_6
 */
#define VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_6  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x122)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_3
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_6 . CUSTOM_DATA_6
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VCAP_IS2:CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_7
 */
#define VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_7  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x123)

/** 
 * \brief
 * Match against frame data starting at byte position
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_3 + 2
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_7 . CUSTOM_DATA_7
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_7_ENTRY_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VCAP_IS2:MAC_ETYPE_MASK
 *
 * Entry mask for entry type MAC_ETYPE.
 */


/** 
 * \brief Mask register corresponding to MAC_ETYPE_ENTRY:ACE_VLD
 *
 * \details
 * Mask register corresponding to MAC_ETYPE_ENTRY:ACE_VLD
 *
 * Register: \a VCAP_IS2:MAC_ETYPE_MASK:ACE_VLD
 */
#define VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_VLD  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x128)

/** 
 * \brief
 * Mask has to be valid always.
 *
 * \details 
 * Must be set to '0'
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_MASK_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to MAC_ETYPE_ENTRY:ACE_TYPE
 *
 * \details
 * Mask register corresponding to MAC_ETYPE_ENTRY:ACE_TYPE
 *
 * Register: \a VCAP_IS2:MAC_ETYPE_MASK:ACE_TYPE
 */
#define VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x129)

/** 
 * \brief
 * Mask for field MAC_ETYPE_ENTRY:ACE_TYPE.VLAN_TAGGED
 *
 * \details 
 * '0': Entry field VLAN_TAGGED must match
 * '1': Entry field VLAN_TAGGED is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Mask for field MAC_ETYPE_ENTRY:ACE_TYPE.ISDX_NEQ0
 *
 * \details 
 * '0': Entry field ISDX_NEQ0 must match
 * '1': Entry field ISDX_NEQ0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * Mask for field MAC_ETYPE_ENTRY:ACE_TYPE.L3_RT
 *
 * \details 
 * '0': Entry field L3_RT must match
 * '1': Entry field L3_RT is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE . L3_RT
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE_L3_RT  VTSS_BIT(22)

/** 
 * \brief
 * Mask for field MAC_ETYPE_ENTRY:ACE_TYPE.L3_SMAC_SIP_MATCH
 *
 * \details 
 * '0': Entry field L3_SMAC_SIP_MATCH must match
 * '1': Entry field L3_SMAC_SIP_MATCH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE . L3_SMAC_SIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE_L3_SMAC_SIP_MATCH  VTSS_BIT(21)

/** 
 * \brief
 * Mask for field MAC_ETYPE_ENTRY:ACE_TYPE.L3_DMAC_DIP_MATCH
 *
 * \details 
 * '0': Entry field L3_DMAC_DIP_MATCH must match
 * '1': Entry field L3_DMAC_DIP_MATCH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE . L3_DMAC_DIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE_L3_DMAC_DIP_MATCH  VTSS_BIT(23)

/** 
 * \brief
 * Mask for field MAC_ETYPE_ENTRY:ACE_TYPE.L2_FWD
 *
 * \details 
 * '0': Entry field L2_FWD must match
 * '1': Entry field L2_FWD is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * Mask for field MAC_ETYPE_ENTRY:ACE_TYPE.VID
 *
 * \details 
 * '0': Entry field VID must match
 * '1': Entry field VID is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * Mask for field MAC_ETYPE_ENTRY:ACE_TYPE.UPRIO
 *
 * \details 
 * '0': Entry field UPRIO must match
 * '1': Entry field UPRIO is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * Mask for field MAC_ETYPE_ENTRY:ACE_TYPE.CFI
 *
 * \details 
 * '0': Entry field CFI must match
 * '1': Entry field CFI is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Mask for field MAC_ETYPE_ENTRY:ACE_TYPE.L2_BC
 *
 * \details 
 * '0': Entry field L2_BC must match
 * '1': Entry field L2_BC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Mask for field MAC_ETYPE_ENTRY:ACE_TYPE.L2_MC
 *
 * \details 
 * '0': Entry field L2_MC must match
 * '1': Entry field L2_MC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * Mask for field MAC_ETYPE_ENTRY:ACE_TYPE.FIRST
 *
 * \details 
 * '0': Entry field FIRST must match
 * '1': Entry field FIRST is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_MASK_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Mask register corresponding to MAC_ETYPE_ENTRY:ACE_IGR_PORT_MASK
 *
 * \details
 * Mask register corresponding to MAC_ETYPE_ENTRY:ACE_IGR_PORT_MASK
 *
 * Register: \a VCAP_IS2:MAC_ETYPE_MASK:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x12a)


/** 
 * \brief Mask register corresponding to MAC_ETYPE_ENTRY:ACE_PAG
 *
 * \details
 * Mask register corresponding to MAC_ETYPE_ENTRY:ACE_PAG
 *
 * Register: \a VCAP_IS2:MAC_ETYPE_MASK:ACE_PAG
 */
#define VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_PAG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x12b)

/** 
 * \brief
 * Mask for field MAC_ETYPE_ENTRY:ACE_PAG.PAG
 *
 * \details 
 * '0': Entry field PAG must match
 * '1': Entry field PAG is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_MASK_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_MAC_ETYPE_MASK_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_MAC_ETYPE_MASK_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Mask register corresponding to MAC_ETYPE_ENTRY:ACE_L2_SMAC_HIGH
 *
 * \details
 * Mask register corresponding to MAC_ETYPE_ENTRY:ACE_L2_SMAC_HIGH
 *
 * Register: \a VCAP_IS2:MAC_ETYPE_MASK:ACE_L2_SMAC_HIGH
 */
#define VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_L2_SMAC_HIGH  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x12c)

/** 
 * \brief
 * Mask for field MAC_ETYPE_ENTRY:ACE_L2_SMAC_HIGH.L2_SMAC_HIGH
 *
 * \details 
 * '0': Entry field L2_SMAC_HIGH must match
 * '1': Entry field L2_SMAC_HIGH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_L2_SMAC_HIGH . L2_SMAC_HIGH
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_MASK_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_MAC_ETYPE_MASK_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_MAC_ETYPE_MASK_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to MAC_ETYPE_ENTRY:ACE_L2_SMAC_LOW
 *
 * \details
 * Mask register corresponding to MAC_ETYPE_ENTRY:ACE_L2_SMAC_LOW
 *
 * Register: \a VCAP_IS2:MAC_ETYPE_MASK:ACE_L2_SMAC_LOW
 */
#define VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_L2_SMAC_LOW  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x12d)


/** 
 * \brief Mask register corresponding to MAC_ETYPE_ENTRY:ACE_L2_DMAC_HIGH
 *
 * \details
 * Mask register corresponding to MAC_ETYPE_ENTRY:ACE_L2_DMAC_HIGH
 *
 * Register: \a VCAP_IS2:MAC_ETYPE_MASK:ACE_L2_DMAC_HIGH
 */
#define VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_L2_DMAC_HIGH  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x12e)

/** 
 * \brief
 * Mask for field MAC_ETYPE_ENTRY:ACE_L2_DMAC_HIGH.L2_DMAC_HIGH
 *
 * \details 
 * '0': Entry field L2_DMAC_HIGH must match
 * '1': Entry field L2_DMAC_HIGH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_L2_DMAC_HIGH . L2_DMAC_HIGH
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_MASK_ACE_L2_DMAC_HIGH_L2_DMAC_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_MAC_ETYPE_MASK_ACE_L2_DMAC_HIGH_L2_DMAC_HIGH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_MAC_ETYPE_MASK_ACE_L2_DMAC_HIGH_L2_DMAC_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to MAC_ETYPE_ENTRY:ACE_L2_DMAC_LOW
 *
 * \details
 * Mask register corresponding to MAC_ETYPE_ENTRY:ACE_L2_DMAC_LOW
 *
 * Register: \a VCAP_IS2:MAC_ETYPE_MASK:ACE_L2_DMAC_LOW
 */
#define VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_L2_DMAC_LOW  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x12f)


/** 
 * \brief Mask register corresponding to MAC_ETYPE_ENTRY:ACE_L2_ETYPE
 *
 * \details
 * Mask register corresponding to MAC_ETYPE_ENTRY:ACE_L2_ETYPE
 *
 * Register: \a VCAP_IS2:MAC_ETYPE_MASK:ACE_L2_ETYPE
 */
#define VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_L2_ETYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x130)

/** 
 * \brief
 * Mask for field MAC_ETYPE_ENTRY:ACE_L2_ETYPE.L2_PAYLOAD
 *
 * \details 
 * '0': Entry field L2_PAYLOAD must match
 * '1': Entry field L2_PAYLOAD is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_L2_ETYPE . L2_PAYLOAD
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_MASK_ACE_L2_ETYPE_L2_PAYLOAD(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_VCAP_IS2_MAC_ETYPE_MASK_ACE_L2_ETYPE_L2_PAYLOAD     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_VCAP_IS2_MAC_ETYPE_MASK_ACE_L2_ETYPE_L2_PAYLOAD(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Mask for field MAC_ETYPE_ENTRY:ACE_L2_ETYPE.ETYPE
 *
 * \details 
 * '0': Entry field ETYPE must match
 * '1': Entry field ETYPE is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_ETYPE_MASK_ACE_L2_ETYPE . ETYPE
 */
#define  VTSS_F_VCAP_IS2_MAC_ETYPE_MASK_ACE_L2_ETYPE_ETYPE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_MAC_ETYPE_MASK_ACE_L2_ETYPE_ETYPE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_MAC_ETYPE_MASK_ACE_L2_ETYPE_ETYPE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VCAP_IS2:MAC_LLC_MASK
 *
 * Entry mask for entry type MAC_LLC.
 */


/** 
 * \brief Mask register corresponding to MAC_LLC_ENTRY:ACE_VLD
 *
 * \details
 * Mask register corresponding to MAC_LLC_ENTRY:ACE_VLD
 *
 * Register: \a VCAP_IS2:MAC_LLC_MASK:ACE_VLD
 */
#define VTSS_VCAP_IS2_MAC_LLC_MASK_ACE_VLD   VTSS_IOREG(VTSS_TO_VCAP_IS2,0x138)

/** 
 * \brief
 * Mask has to be valid always.
 *
 * \details 
 * Must be set to '0'
 *
 * Field: ::VTSS_VCAP_IS2_MAC_LLC_MASK_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_MAC_LLC_MASK_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to MAC_LLC_ENTRY:ACE_TYPE
 *
 * \details
 * Mask register corresponding to MAC_LLC_ENTRY:ACE_TYPE
 *
 * Register: \a VCAP_IS2:MAC_LLC_MASK:ACE_TYPE
 */
#define VTSS_VCAP_IS2_MAC_LLC_MASK_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x139)

/** 
 * \brief
 * Mask for field MAC_LLC_ENTRY:ACE_TYPE.VLAN_TAGGED
 *
 * \details 
 * '0': Entry field VLAN_TAGGED must match
 * '1': Entry field VLAN_TAGGED is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_LLC_MASK_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_MAC_LLC_MASK_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Mask for field MAC_LLC_ENTRY:ACE_TYPE.ISDX_NEQ0
 *
 * \details 
 * '0': Entry field ISDX_NEQ0 must match
 * '1': Entry field ISDX_NEQ0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_LLC_MASK_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_MAC_LLC_MASK_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * Mask for field MAC_LLC_ENTRY:ACE_TYPE.L2_FWD
 *
 * \details 
 * '0': Entry field L2_FWD must match
 * '1': Entry field L2_FWD is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_LLC_MASK_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_MAC_LLC_MASK_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * Mask for field MAC_LLC_ENTRY:ACE_TYPE.VID
 *
 * \details 
 * '0': Entry field VID must match
 * '1': Entry field VID is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_LLC_MASK_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_MAC_LLC_MASK_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_MAC_LLC_MASK_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_MAC_LLC_MASK_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * Mask for field MAC_LLC_ENTRY:ACE_TYPE.UPRIO
 *
 * \details 
 * '0': Entry field UPRIO must match
 * '1': Entry field UPRIO is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_LLC_MASK_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_MAC_LLC_MASK_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_MAC_LLC_MASK_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_MAC_LLC_MASK_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * Mask for field MAC_LLC_ENTRY:ACE_TYPE.CFI
 *
 * \details 
 * '0': Entry field CFI must match
 * '1': Entry field CFI is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_LLC_MASK_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_MAC_LLC_MASK_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Mask for field MAC_LLC_ENTRY:ACE_TYPE.L2_BC
 *
 * \details 
 * '0': Entry field L2_BC must match
 * '1': Entry field L2_BC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_LLC_MASK_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_MAC_LLC_MASK_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Mask for field MAC_LLC_ENTRY:ACE_TYPE.L2_MC
 *
 * \details 
 * '0': Entry field L2_MC must match
 * '1': Entry field L2_MC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_LLC_MASK_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_MAC_LLC_MASK_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * Mask for field MAC_LLC_ENTRY:ACE_TYPE.FIRST
 *
 * \details 
 * '0': Entry field FIRST must match
 * '1': Entry field FIRST is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_LLC_MASK_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_MAC_LLC_MASK_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Mask register corresponding to MAC_LLC_ENTRY:ACE_IGR_PORT_MASK
 *
 * \details
 * Mask register corresponding to MAC_LLC_ENTRY:ACE_IGR_PORT_MASK
 *
 * Register: \a VCAP_IS2:MAC_LLC_MASK:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_MAC_LLC_MASK_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x13a)


/** 
 * \brief Mask register corresponding to MAC_LLC_ENTRY:ACE_PAG
 *
 * \details
 * Mask register corresponding to MAC_LLC_ENTRY:ACE_PAG
 *
 * Register: \a VCAP_IS2:MAC_LLC_MASK:ACE_PAG
 */
#define VTSS_VCAP_IS2_MAC_LLC_MASK_ACE_PAG   VTSS_IOREG(VTSS_TO_VCAP_IS2,0x13b)

/** 
 * \brief
 * Mask for field MAC_LLC_ENTRY:ACE_PAG.PAG
 *
 * \details 
 * '0': Entry field PAG must match
 * '1': Entry field PAG is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_LLC_MASK_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_MAC_LLC_MASK_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_MAC_LLC_MASK_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_MAC_LLC_MASK_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Mask register corresponding to MAC_LLC_ENTRY:ACE_L2_SMAC_HIGH
 *
 * \details
 * Mask register corresponding to MAC_LLC_ENTRY:ACE_L2_SMAC_HIGH
 *
 * Register: \a VCAP_IS2:MAC_LLC_MASK:ACE_L2_SMAC_HIGH
 */
#define VTSS_VCAP_IS2_MAC_LLC_MASK_ACE_L2_SMAC_HIGH  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x13c)

/** 
 * \brief
 * Mask for field MAC_LLC_ENTRY:ACE_L2_SMAC_HIGH.L2_SMAC_HIGH
 *
 * \details 
 * '0': Entry field L2_SMAC_HIGH must match
 * '1': Entry field L2_SMAC_HIGH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_LLC_MASK_ACE_L2_SMAC_HIGH . L2_SMAC_HIGH
 */
#define  VTSS_F_VCAP_IS2_MAC_LLC_MASK_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_MAC_LLC_MASK_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_MAC_LLC_MASK_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to MAC_LLC_ENTRY:ACE_L2_SMAC_LOW
 *
 * \details
 * Mask register corresponding to MAC_LLC_ENTRY:ACE_L2_SMAC_LOW
 *
 * Register: \a VCAP_IS2:MAC_LLC_MASK:ACE_L2_SMAC_LOW
 */
#define VTSS_VCAP_IS2_MAC_LLC_MASK_ACE_L2_SMAC_LOW  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x13d)


/** 
 * \brief Mask register corresponding to MAC_LLC_ENTRY:ACE_L2_DMAC_HIGH
 *
 * \details
 * Mask register corresponding to MAC_LLC_ENTRY:ACE_L2_DMAC_HIGH
 *
 * Register: \a VCAP_IS2:MAC_LLC_MASK:ACE_L2_DMAC_HIGH
 */
#define VTSS_VCAP_IS2_MAC_LLC_MASK_ACE_L2_DMAC_HIGH  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x13e)

/** 
 * \brief
 * Mask for field MAC_LLC_ENTRY:ACE_L2_DMAC_HIGH.L2_DMAC_HIGH
 *
 * \details 
 * '0': Entry field L2_DMAC_HIGH must match
 * '1': Entry field L2_DMAC_HIGH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_LLC_MASK_ACE_L2_DMAC_HIGH . L2_DMAC_HIGH
 */
#define  VTSS_F_VCAP_IS2_MAC_LLC_MASK_ACE_L2_DMAC_HIGH_L2_DMAC_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_MAC_LLC_MASK_ACE_L2_DMAC_HIGH_L2_DMAC_HIGH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_MAC_LLC_MASK_ACE_L2_DMAC_HIGH_L2_DMAC_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to MAC_LLC_ENTRY:ACE_L2_DMAC_LOW
 *
 * \details
 * Mask register corresponding to MAC_LLC_ENTRY:ACE_L2_DMAC_LOW
 *
 * Register: \a VCAP_IS2:MAC_LLC_MASK:ACE_L2_DMAC_LOW
 */
#define VTSS_VCAP_IS2_MAC_LLC_MASK_ACE_L2_DMAC_LOW  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x13f)


/** 
 * \brief Mask register corresponding to MAC_LLC_ENTRY:ACE_L2_LLC
 *
 * \details
 * Mask register corresponding to MAC_LLC_ENTRY:ACE_L2_LLC
 *
 * Register: \a VCAP_IS2:MAC_LLC_MASK:ACE_L2_LLC
 */
#define VTSS_VCAP_IS2_MAC_LLC_MASK_ACE_L2_LLC  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x140)

/**
 * Register Group: \a VCAP_IS2:MAC_SNAP_MASK
 *
 * Entry mask for entry type MAC_SNAP.
 */


/** 
 * \brief Mask register corresponding to MAC_SNAP_ENTRY:ACE_VLD
 *
 * \details
 * Mask register corresponding to MAC_SNAP_ENTRY:ACE_VLD
 *
 * Register: \a VCAP_IS2:MAC_SNAP_MASK:ACE_VLD
 */
#define VTSS_VCAP_IS2_MAC_SNAP_MASK_ACE_VLD  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x148)

/** 
 * \brief
 * Mask has to be valid always.
 *
 * \details 
 * Must be set to '0'
 *
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_MASK_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_MASK_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to MAC_SNAP_ENTRY:ACE_TYPE
 *
 * \details
 * Mask register corresponding to MAC_SNAP_ENTRY:ACE_TYPE
 *
 * Register: \a VCAP_IS2:MAC_SNAP_MASK:ACE_TYPE
 */
#define VTSS_VCAP_IS2_MAC_SNAP_MASK_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x149)

/** 
 * \brief
 * Mask for field MAC_SNAP_ENTRY:ACE_TYPE.VLAN_TAGGED
 *
 * \details 
 * '0': Entry field VLAN_TAGGED must match
 * '1': Entry field VLAN_TAGGED is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_MASK_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_MASK_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Mask for field MAC_SNAP_ENTRY:ACE_TYPE.ISDX_NEQ0
 *
 * \details 
 * '0': Entry field ISDX_NEQ0 must match
 * '1': Entry field ISDX_NEQ0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_MASK_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_MASK_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * Mask for field MAC_SNAP_ENTRY:ACE_TYPE.L2_FWD
 *
 * \details 
 * '0': Entry field L2_FWD must match
 * '1': Entry field L2_FWD is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_MASK_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_MASK_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * Mask for field MAC_SNAP_ENTRY:ACE_TYPE.VID
 *
 * \details 
 * '0': Entry field VID must match
 * '1': Entry field VID is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_MASK_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_MASK_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_MAC_SNAP_MASK_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_MAC_SNAP_MASK_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * Mask for field MAC_SNAP_ENTRY:ACE_TYPE.UPRIO
 *
 * \details 
 * '0': Entry field UPRIO must match
 * '1': Entry field UPRIO is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_MASK_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_MASK_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_MAC_SNAP_MASK_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_MAC_SNAP_MASK_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * Mask for field MAC_SNAP_ENTRY:ACE_TYPE.CFI
 *
 * \details 
 * '0': Entry field CFI must match
 * '1': Entry field CFI is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_MASK_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_MASK_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Mask for field MAC_SNAP_ENTRY:ACE_TYPE.L2_BC
 *
 * \details 
 * '0': Entry field L2_BC must match
 * '1': Entry field L2_BC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_MASK_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_MASK_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Mask for field MAC_SNAP_ENTRY:ACE_TYPE.L2_MC
 *
 * \details 
 * '0': Entry field L2_MC must match
 * '1': Entry field L2_MC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_MASK_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_MASK_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * Mask for field MAC_SNAP_ENTRY:ACE_TYPE.FIRST
 *
 * \details 
 * '0': Entry field FIRST must match
 * '1': Entry field FIRST is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_MASK_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_MASK_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Mask register corresponding to MAC_SNAP_ENTRY:ACE_IGR_PORT_MASK
 *
 * \details
 * Mask register corresponding to MAC_SNAP_ENTRY:ACE_IGR_PORT_MASK
 *
 * Register: \a VCAP_IS2:MAC_SNAP_MASK:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_MAC_SNAP_MASK_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x14a)


/** 
 * \brief Mask register corresponding to MAC_SNAP_ENTRY:ACE_PAG
 *
 * \details
 * Mask register corresponding to MAC_SNAP_ENTRY:ACE_PAG
 *
 * Register: \a VCAP_IS2:MAC_SNAP_MASK:ACE_PAG
 */
#define VTSS_VCAP_IS2_MAC_SNAP_MASK_ACE_PAG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x14b)

/** 
 * \brief
 * Mask for field MAC_SNAP_ENTRY:ACE_PAG.PAG
 *
 * \details 
 * '0': Entry field PAG must match
 * '1': Entry field PAG is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_MASK_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_MASK_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_MAC_SNAP_MASK_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_MAC_SNAP_MASK_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Mask register corresponding to MAC_SNAP_ENTRY:ACE_L2_SMAC_HIGH
 *
 * \details
 * Mask register corresponding to MAC_SNAP_ENTRY:ACE_L2_SMAC_HIGH
 *
 * Register: \a VCAP_IS2:MAC_SNAP_MASK:ACE_L2_SMAC_HIGH
 */
#define VTSS_VCAP_IS2_MAC_SNAP_MASK_ACE_L2_SMAC_HIGH  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x14c)

/** 
 * \brief
 * Mask for field MAC_SNAP_ENTRY:ACE_L2_SMAC_HIGH.L2_SMAC_HIGH
 *
 * \details 
 * '0': Entry field L2_SMAC_HIGH must match
 * '1': Entry field L2_SMAC_HIGH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_MASK_ACE_L2_SMAC_HIGH . L2_SMAC_HIGH
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_MASK_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_MAC_SNAP_MASK_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_MAC_SNAP_MASK_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to MAC_SNAP_ENTRY:ACE_L2_SMAC_LOW
 *
 * \details
 * Mask register corresponding to MAC_SNAP_ENTRY:ACE_L2_SMAC_LOW
 *
 * Register: \a VCAP_IS2:MAC_SNAP_MASK:ACE_L2_SMAC_LOW
 */
#define VTSS_VCAP_IS2_MAC_SNAP_MASK_ACE_L2_SMAC_LOW  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x14d)


/** 
 * \brief Mask register corresponding to MAC_SNAP_ENTRY:ACE_L2_DMAC_HIGH
 *
 * \details
 * Mask register corresponding to MAC_SNAP_ENTRY:ACE_L2_DMAC_HIGH
 *
 * Register: \a VCAP_IS2:MAC_SNAP_MASK:ACE_L2_DMAC_HIGH
 */
#define VTSS_VCAP_IS2_MAC_SNAP_MASK_ACE_L2_DMAC_HIGH  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x14e)

/** 
 * \brief
 * Mask for field MAC_SNAP_ENTRY:ACE_L2_DMAC_HIGH.L2_DMAC_HIGH
 *
 * \details 
 * '0': Entry field L2_DMAC_HIGH must match
 * '1': Entry field L2_DMAC_HIGH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_MASK_ACE_L2_DMAC_HIGH . L2_DMAC_HIGH
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_MASK_ACE_L2_DMAC_HIGH_L2_DMAC_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_MAC_SNAP_MASK_ACE_L2_DMAC_HIGH_L2_DMAC_HIGH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_MAC_SNAP_MASK_ACE_L2_DMAC_HIGH_L2_DMAC_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to MAC_SNAP_ENTRY:ACE_L2_DMAC_LOW
 *
 * \details
 * Mask register corresponding to MAC_SNAP_ENTRY:ACE_L2_DMAC_LOW
 *
 * Register: \a VCAP_IS2:MAC_SNAP_MASK:ACE_L2_DMAC_LOW
 */
#define VTSS_VCAP_IS2_MAC_SNAP_MASK_ACE_L2_DMAC_LOW  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x14f)


/** 
 * \brief Mask register corresponding to MAC_SNAP_ENTRY:ACE_L2_SNAP_LOW
 *
 * \details
 * Mask register corresponding to MAC_SNAP_ENTRY:ACE_L2_SNAP_LOW
 *
 * Register: \a VCAP_IS2:MAC_SNAP_MASK:ACE_L2_SNAP_LOW
 */
#define VTSS_VCAP_IS2_MAC_SNAP_MASK_ACE_L2_SNAP_LOW  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x150)


/** 
 * \brief Mask register corresponding to MAC_SNAP_ENTRY:ACE_L2_SNAP_HIGH
 *
 * \details
 * Mask register corresponding to MAC_SNAP_ENTRY:ACE_L2_SNAP_HIGH
 *
 * Register: \a VCAP_IS2:MAC_SNAP_MASK:ACE_L2_SNAP_HIGH
 */
#define VTSS_VCAP_IS2_MAC_SNAP_MASK_ACE_L2_SNAP_HIGH  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x151)

/** 
 * \brief
 * Mask for field MAC_SNAP_ENTRY:ACE_L2_SNAP_HIGH.L2_SNAP_HIGH
 *
 * \details 
 * '0': Entry field L2_SNAP_HIGH must match
 * '1': Entry field L2_SNAP_HIGH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_MAC_SNAP_MASK_ACE_L2_SNAP_HIGH . L2_SNAP_HIGH
 */
#define  VTSS_F_VCAP_IS2_MAC_SNAP_MASK_ACE_L2_SNAP_HIGH_L2_SNAP_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_MAC_SNAP_MASK_ACE_L2_SNAP_HIGH_L2_SNAP_HIGH     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_MAC_SNAP_MASK_ACE_L2_SNAP_HIGH_L2_SNAP_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a VCAP_IS2:ARP_MASK
 *
 * Entry mask for entry type ARP.
 */


/** 
 * \brief Mask register corresponding to ARP_ENTRY:ACE_VLD
 *
 * \details
 * Mask register corresponding to ARP_ENTRY:ACE_VLD
 *
 * Register: \a VCAP_IS2:ARP_MASK:ACE_VLD
 */
#define VTSS_VCAP_IS2_ARP_MASK_ACE_VLD       VTSS_IOREG(VTSS_TO_VCAP_IS2,0x158)

/** 
 * \brief
 * Mask has to be valid always.
 *
 * \details 
 * Must be set to '0'
 *
 * Field: ::VTSS_VCAP_IS2_ARP_MASK_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_ARP_MASK_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to ARP_ENTRY:ACE_TYPE
 *
 * \details
 * Mask register corresponding to ARP_ENTRY:ACE_TYPE
 *
 * Register: \a VCAP_IS2:ARP_MASK:ACE_TYPE
 */
#define VTSS_VCAP_IS2_ARP_MASK_ACE_TYPE      VTSS_IOREG(VTSS_TO_VCAP_IS2,0x159)

/** 
 * \brief
 * Mask for field ARP_ENTRY:ACE_TYPE.VLAN_TAGGED
 *
 * \details 
 * '0': Entry field VLAN_TAGGED must match
 * '1': Entry field VLAN_TAGGED is ignored
 *
 * Field: ::VTSS_VCAP_IS2_ARP_MASK_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_ARP_MASK_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Mask for field ARP_ENTRY:ACE_TYPE.ISDX_NEQ0
 *
 * \details 
 * '0': Entry field ISDX_NEQ0 must match
 * '1': Entry field ISDX_NEQ0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_ARP_MASK_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_ARP_MASK_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * Mask for field ARP_ENTRY:ACE_TYPE.L2_FWD
 *
 * \details 
 * '0': Entry field L2_FWD must match
 * '1': Entry field L2_FWD is ignored
 *
 * Field: ::VTSS_VCAP_IS2_ARP_MASK_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_ARP_MASK_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * Mask for field ARP_ENTRY:ACE_TYPE.VID
 *
 * \details 
 * '0': Entry field VID must match
 * '1': Entry field VID is ignored
 *
 * Field: ::VTSS_VCAP_IS2_ARP_MASK_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_ARP_MASK_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_ARP_MASK_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_ARP_MASK_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * Mask for field ARP_ENTRY:ACE_TYPE.UPRIO
 *
 * \details 
 * '0': Entry field UPRIO must match
 * '1': Entry field UPRIO is ignored
 *
 * Field: ::VTSS_VCAP_IS2_ARP_MASK_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_ARP_MASK_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_ARP_MASK_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_ARP_MASK_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * Mask for field ARP_ENTRY:ACE_TYPE.CFI
 *
 * \details 
 * '0': Entry field CFI must match
 * '1': Entry field CFI is ignored
 *
 * Field: ::VTSS_VCAP_IS2_ARP_MASK_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_ARP_MASK_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Mask for field ARP_ENTRY:ACE_TYPE.L2_BC
 *
 * \details 
 * '0': Entry field L2_BC must match
 * '1': Entry field L2_BC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_ARP_MASK_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_ARP_MASK_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Mask for field ARP_ENTRY:ACE_TYPE.L2_MC
 *
 * \details 
 * '0': Entry field L2_MC must match
 * '1': Entry field L2_MC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_ARP_MASK_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_ARP_MASK_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * Mask for field ARP_ENTRY:ACE_TYPE.FIRST
 *
 * \details 
 * '0': Entry field FIRST must match
 * '1': Entry field FIRST is ignored
 *
 * Field: ::VTSS_VCAP_IS2_ARP_MASK_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_ARP_MASK_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Mask register corresponding to ARP_ENTRY:ACE_IGR_PORT_MASK
 *
 * \details
 * Mask register corresponding to ARP_ENTRY:ACE_IGR_PORT_MASK
 *
 * Register: \a VCAP_IS2:ARP_MASK:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_ARP_MASK_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x15a)


/** 
 * \brief Mask register corresponding to ARP_ENTRY:ACE_PAG
 *
 * \details
 * Mask register corresponding to ARP_ENTRY:ACE_PAG
 *
 * Register: \a VCAP_IS2:ARP_MASK:ACE_PAG
 */
#define VTSS_VCAP_IS2_ARP_MASK_ACE_PAG       VTSS_IOREG(VTSS_TO_VCAP_IS2,0x15b)

/** 
 * \brief
 * Mask for field ARP_ENTRY:ACE_PAG.PAG
 *
 * \details 
 * '0': Entry field PAG must match
 * '1': Entry field PAG is ignored
 *
 * Field: ::VTSS_VCAP_IS2_ARP_MASK_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_ARP_MASK_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_ARP_MASK_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_ARP_MASK_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Mask register corresponding to ARP_ENTRY:ACE_L2_SMAC_HIGH
 *
 * \details
 * Mask register corresponding to ARP_ENTRY:ACE_L2_SMAC_HIGH
 *
 * Register: \a VCAP_IS2:ARP_MASK:ACE_L2_SMAC_HIGH
 */
#define VTSS_VCAP_IS2_ARP_MASK_ACE_L2_SMAC_HIGH  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x15c)

/** 
 * \brief
 * Mask for field ARP_ENTRY:ACE_L2_SMAC_HIGH.L2_SMAC_HIGH
 *
 * \details 
 * '0': Entry field L2_SMAC_HIGH must match
 * '1': Entry field L2_SMAC_HIGH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_ARP_MASK_ACE_L2_SMAC_HIGH . L2_SMAC_HIGH
 */
#define  VTSS_F_VCAP_IS2_ARP_MASK_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_ARP_MASK_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_ARP_MASK_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to ARP_ENTRY:ACE_L2_SMAC_LOW
 *
 * \details
 * Mask register corresponding to ARP_ENTRY:ACE_L2_SMAC_LOW
 *
 * Register: \a VCAP_IS2:ARP_MASK:ACE_L2_SMAC_LOW
 */
#define VTSS_VCAP_IS2_ARP_MASK_ACE_L2_SMAC_LOW  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x15d)


/** 
 * \brief Mask register corresponding to ARP_ENTRY:ACE_L3_MISC
 *
 * \details
 * Mask register corresponding to ARP_ENTRY:ACE_L3_MISC
 *
 * Register: \a VCAP_IS2:ARP_MASK:ACE_L3_MISC
 */
#define VTSS_VCAP_IS2_ARP_MASK_ACE_L3_MISC   VTSS_IOREG(VTSS_TO_VCAP_IS2,0x15e)

/** 
 * \brief
 * Mask for field ARP_ENTRY:ACE_L3_MISC.DIP_EQ_SIP
 *
 * \details 
 * '0': Entry field DIP_EQ_SIP must match
 * '1': Entry field DIP_EQ_SIP is ignored
 *
 * Field: ::VTSS_VCAP_IS2_ARP_MASK_ACE_L3_MISC . DIP_EQ_SIP
 */
#define  VTSS_F_VCAP_IS2_ARP_MASK_ACE_L3_MISC_DIP_EQ_SIP  VTSS_BIT(20)


/** 
 * \brief Mask register corresponding to ARP_ENTRY:ACE_L3_ARP
 *
 * \details
 * Mask register corresponding to ARP_ENTRY:ACE_L3_ARP
 *
 * Register: \a VCAP_IS2:ARP_MASK:ACE_L3_ARP
 */
#define VTSS_VCAP_IS2_ARP_MASK_ACE_L3_ARP    VTSS_IOREG(VTSS_TO_VCAP_IS2,0x15f)

/** 
 * \brief
 * Mask for field ARP_ENTRY:ACE_L3_ARP.ARP_ADDR_SPACE_OK
 *
 * \details 
 * '0': Entry field ARP_ADDR_SPACE_OK must match
 * '1': Entry field ARP_ADDR_SPACE_OK is ignored
 *
 * Field: ::VTSS_VCAP_IS2_ARP_MASK_ACE_L3_ARP . ARP_ADDR_SPACE_OK
 */
#define  VTSS_F_VCAP_IS2_ARP_MASK_ACE_L3_ARP_ARP_ADDR_SPACE_OK  VTSS_BIT(7)

/** 
 * \brief
 * Mask for field ARP_ENTRY:ACE_L3_ARP.ARP_PROTO_SPACE_OK
 *
 * \details 
 * '0': Entry field ARP_PROTO_SPACE_OK must match
 * '1': Entry field ARP_PROTO_SPACE_OK is ignored
 *
 * Field: ::VTSS_VCAP_IS2_ARP_MASK_ACE_L3_ARP . ARP_PROTO_SPACE_OK
 */
#define  VTSS_F_VCAP_IS2_ARP_MASK_ACE_L3_ARP_ARP_PROTO_SPACE_OK  VTSS_BIT(6)

/** 
 * \brief
 * Mask for field ARP_ENTRY:ACE_L3_ARP.ARP_LEN_OK
 *
 * \details 
 * '0': Entry field ARP_LEN_OK must match
 * '1': Entry field ARP_LEN_OK is ignored
 *
 * Field: ::VTSS_VCAP_IS2_ARP_MASK_ACE_L3_ARP . ARP_LEN_OK
 */
#define  VTSS_F_VCAP_IS2_ARP_MASK_ACE_L3_ARP_ARP_LEN_OK  VTSS_BIT(5)

/** 
 * \brief
 * Mask for field ARP_ENTRY:ACE_L3_ARP.ARP_TARGET_MATCH
 *
 * \details 
 * '0': Entry field ARP_TARGET_MATCH must match
 * '1': Entry field ARP_TARGET_MATCH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_ARP_MASK_ACE_L3_ARP . ARP_TARGET_MATCH
 */
#define  VTSS_F_VCAP_IS2_ARP_MASK_ACE_L3_ARP_ARP_TARGET_MATCH  VTSS_BIT(4)

/** 
 * \brief
 * Mask for field ARP_ENTRY:ACE_L3_ARP.ARP_SENDER_MATCH
 *
 * \details 
 * '0': Entry field ARP_SENDER_MATCH must match
 * '1': Entry field ARP_SENDER_MATCH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_ARP_MASK_ACE_L3_ARP . ARP_SENDER_MATCH
 */
#define  VTSS_F_VCAP_IS2_ARP_MASK_ACE_L3_ARP_ARP_SENDER_MATCH  VTSS_BIT(3)

/** 
 * \brief
 * Mask for field ARP_ENTRY:ACE_L3_ARP.ARP_OPCODE_UNKNOWN
 *
 * \details 
 * '0': Entry field ARP_OPCODE_UNKNOWN must match
 * '1': Entry field ARP_OPCODE_UNKNOWN is ignored
 *
 * Field: ::VTSS_VCAP_IS2_ARP_MASK_ACE_L3_ARP . ARP_OPCODE_UNKNOWN
 */
#define  VTSS_F_VCAP_IS2_ARP_MASK_ACE_L3_ARP_ARP_OPCODE_UNKNOWN  VTSS_BIT(2)

/** 
 * \brief
 * Mask for field ARP_ENTRY:ACE_L3_ARP.ARP_OPCODE
 *
 * \details 
 * '0': Entry field ARP_OPCODE must match
 * '1': Entry field ARP_OPCODE is ignored
 *
 * Field: ::VTSS_VCAP_IS2_ARP_MASK_ACE_L3_ARP . ARP_OPCODE
 */
#define  VTSS_F_VCAP_IS2_ARP_MASK_ACE_L3_ARP_ARP_OPCODE(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_VCAP_IS2_ARP_MASK_ACE_L3_ARP_ARP_OPCODE     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_VCAP_IS2_ARP_MASK_ACE_L3_ARP_ARP_OPCODE(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Mask register corresponding to ARP_ENTRY:ACE_L3_IP4_DIP
 *
 * \details
 * Mask register corresponding to ARP_ENTRY:ACE_L3_IP4_DIP
 *
 * Register: \a VCAP_IS2:ARP_MASK:ACE_L3_IP4_DIP
 */
#define VTSS_VCAP_IS2_ARP_MASK_ACE_L3_IP4_DIP  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x160)


/** 
 * \brief Mask register corresponding to ARP_ENTRY:ACE_L3_IP4_SIP
 *
 * \details
 * Mask register corresponding to ARP_ENTRY:ACE_L3_IP4_SIP
 *
 * Register: \a VCAP_IS2:ARP_MASK:ACE_L3_IP4_SIP
 */
#define VTSS_VCAP_IS2_ARP_MASK_ACE_L3_IP4_SIP  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x161)

/**
 * Register Group: \a VCAP_IS2:IP_TCP_UDP_MASK
 *
 * Entry mask for entry type IP_TCP_UDP.
 */


/** 
 * \brief Mask register corresponding to IP_TCP_UDP_ENTRY:ACE_VLD
 *
 * \details
 * Mask register corresponding to IP_TCP_UDP_ENTRY:ACE_VLD
 *
 * Register: \a VCAP_IS2:IP_TCP_UDP_MASK:ACE_VLD
 */
#define VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_VLD  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x168)

/** 
 * \brief
 * Mask has to be valid always.
 *
 * \details 
 * Must be set to '0'
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to IP_TCP_UDP_ENTRY:ACE_TYPE
 *
 * \details
 * Mask register corresponding to IP_TCP_UDP_ENTRY:ACE_TYPE
 *
 * Register: \a VCAP_IS2:IP_TCP_UDP_MASK:ACE_TYPE
 */
#define VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x169)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_TYPE.VLAN_TAGGED
 *
 * \details 
 * '0': Entry field VLAN_TAGGED must match
 * '1': Entry field VLAN_TAGGED is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_TYPE.ISDX_NEQ0
 *
 * \details 
 * '0': Entry field ISDX_NEQ0 must match
 * '1': Entry field ISDX_NEQ0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_TYPE.L3_RT
 *
 * \details 
 * '0': Entry field L3_RT must match
 * '1': Entry field L3_RT is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE . L3_RT
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE_L3_RT  VTSS_BIT(22)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_TYPE.L3_SMAC_SIP_MATCH
 *
 * \details 
 * '0': Entry field L3_SMAC_SIP_MATCH must match
 * '1': Entry field L3_SMAC_SIP_MATCH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE . L3_SMAC_SIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE_L3_SMAC_SIP_MATCH  VTSS_BIT(21)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_TYPE.L3_DMAC_DIP_MATCH
 *
 * \details 
 * '0': Entry field L3_DMAC_DIP_MATCH must match
 * '1': Entry field L3_DMAC_DIP_MATCH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE . L3_DMAC_DIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE_L3_DMAC_DIP_MATCH  VTSS_BIT(23)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_TYPE.L2_FWD
 *
 * \details 
 * '0': Entry field L2_FWD must match
 * '1': Entry field L2_FWD is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_TYPE.VID
 *
 * \details 
 * '0': Entry field VID must match
 * '1': Entry field VID is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_TYPE.UPRIO
 *
 * \details 
 * '0': Entry field UPRIO must match
 * '1': Entry field UPRIO is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_TYPE.CFI
 *
 * \details 
 * '0': Entry field CFI must match
 * '1': Entry field CFI is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_TYPE.L2_BC
 *
 * \details 
 * '0': Entry field L2_BC must match
 * '1': Entry field L2_BC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_TYPE.L2_MC
 *
 * \details 
 * '0': Entry field L2_MC must match
 * '1': Entry field L2_MC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_TYPE.FIRST
 *
 * \details 
 * '0': Entry field FIRST must match
 * '1': Entry field FIRST is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Mask register corresponding to IP_TCP_UDP_ENTRY:ACE_IGR_PORT_MASK
 *
 * \details
 * Mask register corresponding to IP_TCP_UDP_ENTRY:ACE_IGR_PORT_MASK
 *
 * Register: \a VCAP_IS2:IP_TCP_UDP_MASK:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x16a)


/** 
 * \brief Mask register corresponding to IP_TCP_UDP_ENTRY:ACE_PAG
 *
 * \details
 * Mask register corresponding to IP_TCP_UDP_ENTRY:ACE_PAG
 *
 * Register: \a VCAP_IS2:IP_TCP_UDP_MASK:ACE_PAG
 */
#define VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_PAG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x16b)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_PAG.PAG
 *
 * \details 
 * '0': Entry field PAG must match
 * '1': Entry field PAG is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_IP_TCP_UDP_MASK_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_IP_TCP_UDP_MASK_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Mask register corresponding to IP_TCP_UDP_ENTRY:ACE_L2_MISC
 *
 * \details
 * Mask register corresponding to IP_TCP_UDP_ENTRY:ACE_L2_MISC
 *
 * Register: \a VCAP_IS2:IP_TCP_UDP_MASK:ACE_L2_MISC
 */
#define VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L2_MISC  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x16c)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_L2_MISC.IP4
 *
 * \details 
 * '0': Entry field IP4 must match
 * '1': Entry field IP4 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L2_MISC . IP4
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L2_MISC_IP4  VTSS_BIT(5)


/** 
 * \brief Mask register corresponding to IP_TCP_UDP_ENTRY:ACE_L3_MISC
 *
 * \details
 * Mask register corresponding to IP_TCP_UDP_ENTRY:ACE_L3_MISC
 *
 * Register: \a VCAP_IS2:IP_TCP_UDP_MASK:ACE_L3_MISC
 */
#define VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L3_MISC  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x16d)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_L3_MISC.DIP_EQ_SIP
 *
 * \details 
 * '0': Entry field DIP_EQ_SIP must match
 * '1': Entry field DIP_EQ_SIP is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L3_MISC . DIP_EQ_SIP
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L3_MISC_DIP_EQ_SIP  VTSS_BIT(20)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_L3_MISC.L3_IP_PROTO
 *
 * \details 
 * '0': Entry field L3_IP_PROTO must match
 * '1': Entry field L3_IP_PROTO is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L3_MISC . L3_IP_PROTO
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L3_MISC_L3_IP_PROTO(x)  VTSS_ENCODE_BITFIELD(x,12,8)
#define  VTSS_M_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L3_MISC_L3_IP_PROTO     VTSS_ENCODE_BITMASK(12,8)
#define  VTSS_X_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L3_MISC_L3_IP_PROTO(x)  VTSS_EXTRACT_BITFIELD(x,12,8)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_L3_MISC.L3_TOS
 *
 * \details 
 * '0': Entry field L3_TOS must match
 * '1': Entry field L3_TOS is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L3_MISC . L3_TOS
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L3_MISC_L3_TOS(x)  VTSS_ENCODE_BITFIELD(x,4,8)
#define  VTSS_M_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L3_MISC_L3_TOS     VTSS_ENCODE_BITMASK(4,8)
#define  VTSS_X_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L3_MISC_L3_TOS(x)  VTSS_EXTRACT_BITFIELD(x,4,8)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_L3_MISC.L3_OPTIONS
 *
 * \details 
 * '0': Entry field L3_OPTIONS must match
 * '1': Entry field L3_OPTIONS is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L3_MISC . L3_OPTIONS
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L3_MISC_L3_OPTIONS  VTSS_BIT(2)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_L3_MISC.L3_FRAGMENT
 *
 * \details 
 * '0': Entry field L3_FRAGMENT must match
 * '1': Entry field L3_FRAGMENT is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L3_MISC . L3_FRAGMENT
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L3_MISC_L3_FRAGMENT  VTSS_BIT(1)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_L3_MISC.L3_TTL_GT0
 *
 * \details 
 * '0': Entry field L3_TTL_GT0 must match
 * '1': Entry field L3_TTL_GT0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L3_MISC . L3_TTL_GT0
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L3_MISC_L3_TTL_GT0  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to IP_TCP_UDP_ENTRY:ACE_L3_IP4_DIP
 *
 * \details
 * Mask register corresponding to IP_TCP_UDP_ENTRY:ACE_L3_IP4_DIP
 *
 * Register: \a VCAP_IS2:IP_TCP_UDP_MASK:ACE_L3_IP4_DIP
 */
#define VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L3_IP4_DIP  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x16e)


/** 
 * \brief Mask register corresponding to IP_TCP_UDP_ENTRY:ACE_L3_IP4_SIP
 *
 * \details
 * Mask register corresponding to IP_TCP_UDP_ENTRY:ACE_L3_IP4_SIP
 *
 * Register: \a VCAP_IS2:IP_TCP_UDP_MASK:ACE_L3_IP4_SIP
 */
#define VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L3_IP4_SIP  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x16f)


/** 
 * \brief Mask register corresponding to IP_TCP_UDP_ENTRY:ACE_L4_PORT
 *
 * \details
 * Mask register corresponding to IP_TCP_UDP_ENTRY:ACE_L4_PORT
 *
 * Register: \a VCAP_IS2:IP_TCP_UDP_MASK:ACE_L4_PORT
 */
#define VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_PORT  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x170)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_L4_PORT.L4_SPORT
 *
 * \details 
 * '0': Entry field L4_SPORT must match
 * '1': Entry field L4_SPORT is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_PORT . L4_SPORT
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_PORT_L4_SPORT(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_PORT_L4_SPORT     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_PORT_L4_SPORT(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_L4_PORT.L4_DPORT
 *
 * \details 
 * '0': Entry field L4_DPORT must match
 * '1': Entry field L4_DPORT is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_PORT . L4_DPORT
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_PORT_L4_DPORT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_PORT_L4_DPORT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_PORT_L4_DPORT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to IP_TCP_UDP_ENTRY:ACE_L4_MISC
 *
 * \details
 * Mask register corresponding to IP_TCP_UDP_ENTRY:ACE_L4_MISC
 *
 * Register: \a VCAP_IS2:IP_TCP_UDP_MASK:ACE_L4_MISC
 */
#define VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_MISC  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x171)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_L4_MISC.SPORT_EQ_DPORT
 *
 * \details 
 * '0': Entry field SPORT_EQ_DPORT must match
 * '1': Entry field SPORT_EQ_DPORT is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_MISC . SPORT_EQ_DPORT
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_MISC_SPORT_EQ_DPORT  VTSS_BIT(15)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_L4_MISC.SEQUENCE_EQ0
 *
 * \details 
 * '0': Entry field SEQUENCE_EQ0 must match
 * '1': Entry field SEQUENCE_EQ0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_MISC . SEQUENCE_EQ0
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_MISC_SEQUENCE_EQ0  VTSS_BIT(14)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_L4_MISC.L4_FIN
 *
 * \details 
 * '0': Entry field L4_FIN must match
 * '1': Entry field L4_FIN is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_MISC . L4_FIN
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_MISC_L4_FIN  VTSS_BIT(13)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_L4_MISC.L4_SYN
 *
 * \details 
 * '0': Entry field L4_SYN must match
 * '1': Entry field L4_SYN is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_MISC . L4_SYN
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_MISC_L4_SYN  VTSS_BIT(12)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_L4_MISC.L4_RST
 *
 * \details 
 * '0': Entry field L4_RST must match
 * '1': Entry field L4_RST is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_MISC . L4_RST
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_MISC_L4_RST  VTSS_BIT(11)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_L4_MISC.L4_PSH
 *
 * \details 
 * '0': Entry field L4_PSH must match
 * '1': Entry field L4_PSH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_MISC . L4_PSH
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_MISC_L4_PSH  VTSS_BIT(10)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_L4_MISC.L4_ACK
 *
 * \details 
 * '0': Entry field L4_ACK must match
 * '1': Entry field L4_ACK is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_MISC . L4_ACK
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_MISC_L4_ACK  VTSS_BIT(9)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_L4_MISC.L4_URG
 *
 * \details 
 * '0': Entry field L4_URG must match
 * '1': Entry field L4_URG is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_MISC . L4_URG
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_MISC_L4_URG  VTSS_BIT(8)

/** 
 * \brief
 * Mask for field IP_TCP_UDP_ENTRY:ACE_L4_MISC.L4_RNG
 *
 * \details 
 * '0': Entry field L4_RNG must match
 * '1': Entry field L4_RNG is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_MISC . L4_RNG
 */
#define  VTSS_F_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_MISC_L4_RNG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_MISC_L4_RNG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_IP_TCP_UDP_MASK_ACE_L4_MISC_L4_RNG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a VCAP_IS2:IP_OTHER_MASK
 *
 * Entry mask for entry type IP_OTHER.
 */


/** 
 * \brief Mask register corresponding to IP_OTHER_ENTRY:ACE_VLD
 *
 * \details
 * Mask register corresponding to IP_OTHER_ENTRY:ACE_VLD
 *
 * Register: \a VCAP_IS2:IP_OTHER_MASK:ACE_VLD
 */
#define VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_VLD  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x178)

/** 
 * \brief
 * Mask has to be valid always.
 *
 * \details 
 * Must be set to '0'
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_MASK_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to IP_OTHER_ENTRY:ACE_TYPE
 *
 * \details
 * Mask register corresponding to IP_OTHER_ENTRY:ACE_TYPE
 *
 * Register: \a VCAP_IS2:IP_OTHER_MASK:ACE_TYPE
 */
#define VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x179)

/** 
 * \brief
 * Mask for field IP_OTHER_ENTRY:ACE_TYPE.VLAN_TAGGED
 *
 * \details 
 * '0': Entry field VLAN_TAGGED must match
 * '1': Entry field VLAN_TAGGED is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Mask for field IP_OTHER_ENTRY:ACE_TYPE.ISDX_NEQ0
 *
 * \details 
 * '0': Entry field ISDX_NEQ0 must match
 * '1': Entry field ISDX_NEQ0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * Mask for field IP_OTHER_ENTRY:ACE_TYPE.L3_RT
 *
 * \details 
 * '0': Entry field L3_RT must match
 * '1': Entry field L3_RT is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE . L3_RT
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE_L3_RT  VTSS_BIT(22)

/** 
 * \brief
 * Mask for field IP_OTHER_ENTRY:ACE_TYPE.L3_SMAC_SIP_MATCH
 *
 * \details 
 * '0': Entry field L3_SMAC_SIP_MATCH must match
 * '1': Entry field L3_SMAC_SIP_MATCH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE . L3_SMAC_SIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE_L3_SMAC_SIP_MATCH  VTSS_BIT(21)

/** 
 * \brief
 * Mask for field IP_OTHER_ENTRY:ACE_TYPE.L3_DMAC_DIP_MATCH
 *
 * \details 
 * '0': Entry field L3_DMAC_DIP_MATCH must match
 * '1': Entry field L3_DMAC_DIP_MATCH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE . L3_DMAC_DIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE_L3_DMAC_DIP_MATCH  VTSS_BIT(23)

/** 
 * \brief
 * Mask for field IP_OTHER_ENTRY:ACE_TYPE.L2_FWD
 *
 * \details 
 * '0': Entry field L2_FWD must match
 * '1': Entry field L2_FWD is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * Mask for field IP_OTHER_ENTRY:ACE_TYPE.VID
 *
 * \details 
 * '0': Entry field VID must match
 * '1': Entry field VID is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * Mask for field IP_OTHER_ENTRY:ACE_TYPE.UPRIO
 *
 * \details 
 * '0': Entry field UPRIO must match
 * '1': Entry field UPRIO is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * Mask for field IP_OTHER_ENTRY:ACE_TYPE.CFI
 *
 * \details 
 * '0': Entry field CFI must match
 * '1': Entry field CFI is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Mask for field IP_OTHER_ENTRY:ACE_TYPE.L2_BC
 *
 * \details 
 * '0': Entry field L2_BC must match
 * '1': Entry field L2_BC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Mask for field IP_OTHER_ENTRY:ACE_TYPE.L2_MC
 *
 * \details 
 * '0': Entry field L2_MC must match
 * '1': Entry field L2_MC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * Mask for field IP_OTHER_ENTRY:ACE_TYPE.FIRST
 *
 * \details 
 * '0': Entry field FIRST must match
 * '1': Entry field FIRST is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_MASK_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Mask register corresponding to IP_OTHER_ENTRY:ACE_IGR_PORT_MASK
 *
 * \details
 * Mask register corresponding to IP_OTHER_ENTRY:ACE_IGR_PORT_MASK
 *
 * Register: \a VCAP_IS2:IP_OTHER_MASK:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x17a)


/** 
 * \brief Mask register corresponding to IP_OTHER_ENTRY:ACE_PAG
 *
 * \details
 * Mask register corresponding to IP_OTHER_ENTRY:ACE_PAG
 *
 * Register: \a VCAP_IS2:IP_OTHER_MASK:ACE_PAG
 */
#define VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_PAG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x17b)

/** 
 * \brief
 * Mask for field IP_OTHER_ENTRY:ACE_PAG.PAG
 *
 * \details 
 * '0': Entry field PAG must match
 * '1': Entry field PAG is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_MASK_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_IP_OTHER_MASK_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_IP_OTHER_MASK_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Mask register corresponding to IP_OTHER_ENTRY:ACE_L2_MISC
 *
 * \details
 * Mask register corresponding to IP_OTHER_ENTRY:ACE_L2_MISC
 *
 * Register: \a VCAP_IS2:IP_OTHER_MASK:ACE_L2_MISC
 */
#define VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_L2_MISC  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x17c)

/** 
 * \brief
 * Mask for field IP_OTHER_ENTRY:ACE_L2_MISC.IP4
 *
 * \details 
 * '0': Entry field IP4 must match
 * '1': Entry field IP4 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_L2_MISC . IP4
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_MASK_ACE_L2_MISC_IP4  VTSS_BIT(5)


/** 
 * \brief Mask register corresponding to IP_OTHER_ENTRY:ACE_L3_MISC
 *
 * \details
 * Mask register corresponding to IP_OTHER_ENTRY:ACE_L3_MISC
 *
 * Register: \a VCAP_IS2:IP_OTHER_MASK:ACE_L3_MISC
 */
#define VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_L3_MISC  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x17d)

/** 
 * \brief
 * Mask for field IP_OTHER_ENTRY:ACE_L3_MISC.DIP_EQ_SIP
 *
 * \details 
 * '0': Entry field DIP_EQ_SIP must match
 * '1': Entry field DIP_EQ_SIP is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_L3_MISC . DIP_EQ_SIP
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_MASK_ACE_L3_MISC_DIP_EQ_SIP  VTSS_BIT(20)

/** 
 * \brief
 * Mask for field IP_OTHER_ENTRY:ACE_L3_MISC.L3_IP_PROTO
 *
 * \details 
 * '0': Entry field L3_IP_PROTO must match
 * '1': Entry field L3_IP_PROTO is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_L3_MISC . L3_IP_PROTO
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_MASK_ACE_L3_MISC_L3_IP_PROTO(x)  VTSS_ENCODE_BITFIELD(x,12,8)
#define  VTSS_M_VCAP_IS2_IP_OTHER_MASK_ACE_L3_MISC_L3_IP_PROTO     VTSS_ENCODE_BITMASK(12,8)
#define  VTSS_X_VCAP_IS2_IP_OTHER_MASK_ACE_L3_MISC_L3_IP_PROTO(x)  VTSS_EXTRACT_BITFIELD(x,12,8)

/** 
 * \brief
 * Mask for field IP_OTHER_ENTRY:ACE_L3_MISC.L3_TOS
 *
 * \details 
 * '0': Entry field L3_TOS must match
 * '1': Entry field L3_TOS is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_L3_MISC . L3_TOS
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_MASK_ACE_L3_MISC_L3_TOS(x)  VTSS_ENCODE_BITFIELD(x,4,8)
#define  VTSS_M_VCAP_IS2_IP_OTHER_MASK_ACE_L3_MISC_L3_TOS     VTSS_ENCODE_BITMASK(4,8)
#define  VTSS_X_VCAP_IS2_IP_OTHER_MASK_ACE_L3_MISC_L3_TOS(x)  VTSS_EXTRACT_BITFIELD(x,4,8)

/** 
 * \brief
 * Mask for field IP_OTHER_ENTRY:ACE_L3_MISC.L3_OPTIONS
 *
 * \details 
 * '0': Entry field L3_OPTIONS must match
 * '1': Entry field L3_OPTIONS is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_L3_MISC . L3_OPTIONS
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_MASK_ACE_L3_MISC_L3_OPTIONS  VTSS_BIT(2)

/** 
 * \brief
 * Mask for field IP_OTHER_ENTRY:ACE_L3_MISC.L3_FRAGMENT
 *
 * \details 
 * '0': Entry field L3_FRAGMENT must match
 * '1': Entry field L3_FRAGMENT is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_L3_MISC . L3_FRAGMENT
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_MASK_ACE_L3_MISC_L3_FRAGMENT  VTSS_BIT(1)

/** 
 * \brief
 * Mask for field IP_OTHER_ENTRY:ACE_L3_MISC.L3_TTL_GT0
 *
 * \details 
 * '0': Entry field L3_TTL_GT0 must match
 * '1': Entry field L3_TTL_GT0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_L3_MISC . L3_TTL_GT0
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_MASK_ACE_L3_MISC_L3_TTL_GT0  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to IP_OTHER_ENTRY:ACE_L3_IP4_DIP
 *
 * \details
 * Mask register corresponding to IP_OTHER_ENTRY:ACE_L3_IP4_DIP
 *
 * Register: \a VCAP_IS2:IP_OTHER_MASK:ACE_L3_IP4_DIP
 */
#define VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_L3_IP4_DIP  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x17e)


/** 
 * \brief Mask register corresponding to IP_OTHER_ENTRY:ACE_L3_IP4_SIP
 *
 * \details
 * Mask register corresponding to IP_OTHER_ENTRY:ACE_L3_IP4_SIP
 *
 * Register: \a VCAP_IS2:IP_OTHER_MASK:ACE_L3_IP4_SIP
 */
#define VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_L3_IP4_SIP  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x17f)


/** 
 * \brief Mask register corresponding to IP_OTHER_ENTRY:ACE_IP4_OTHER_0
 *
 * \details
 * Mask register corresponding to IP_OTHER_ENTRY:ACE_IP4_OTHER_0
 *
 * Register: \a VCAP_IS2:IP_OTHER_MASK:ACE_IP4_OTHER_0
 */
#define VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_IP4_OTHER_0  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x180)


/** 
 * \brief Mask register corresponding to IP_OTHER_ENTRY:ACE_IP4_OTHER_1
 *
 * \details
 * Mask register corresponding to IP_OTHER_ENTRY:ACE_IP4_OTHER_1
 *
 * Register: \a VCAP_IS2:IP_OTHER_MASK:ACE_IP4_OTHER_1
 */
#define VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_IP4_OTHER_1  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x181)

/** 
 * \brief
 * Mask for field IP_OTHER_ENTRY:ACE_IP4_OTHER_1.IP4_PAYLOAD_HIGH
 *
 * \details 
 * '0': Entry field IP4_PAYLOAD_HIGH must match
 * '1': Entry field IP4_PAYLOAD_HIGH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP_OTHER_MASK_ACE_IP4_OTHER_1 . IP4_PAYLOAD_HIGH
 */
#define  VTSS_F_VCAP_IS2_IP_OTHER_MASK_ACE_IP4_OTHER_1_IP4_PAYLOAD_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_IP_OTHER_MASK_ACE_IP4_OTHER_1_IP4_PAYLOAD_HIGH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_IP_OTHER_MASK_ACE_IP4_OTHER_1_IP4_PAYLOAD_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VCAP_IS2:IP6_STD_MASK
 *
 * Entry mask for entry type IP6_STD.
 */


/** 
 * \brief Mask register corresponding to IP6_STD_ENTRY:ACE_VLD
 *
 * \details
 * Mask register corresponding to IP6_STD_ENTRY:ACE_VLD
 *
 * Register: \a VCAP_IS2:IP6_STD_MASK:ACE_VLD
 */
#define VTSS_VCAP_IS2_IP6_STD_MASK_ACE_VLD   VTSS_IOREG(VTSS_TO_VCAP_IS2,0x188)

/** 
 * \brief
 * Mask has to be valid always.
 *
 * \details 
 * Must be set to '0'
 *
 * Field: ::VTSS_VCAP_IS2_IP6_STD_MASK_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_MASK_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to IP6_STD_ENTRY:ACE_TYPE
 *
 * \details
 * Mask register corresponding to IP6_STD_ENTRY:ACE_TYPE
 *
 * Register: \a VCAP_IS2:IP6_STD_MASK:ACE_TYPE
 */
#define VTSS_VCAP_IS2_IP6_STD_MASK_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x189)

/** 
 * \brief
 * Mask for field IP6_STD_ENTRY:ACE_TYPE.VLAN_TAGGED
 *
 * \details 
 * '0': Entry field VLAN_TAGGED must match
 * '1': Entry field VLAN_TAGGED is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP6_STD_MASK_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_MASK_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Mask for field IP6_STD_ENTRY:ACE_TYPE.ISDX_NEQ0
 *
 * \details 
 * '0': Entry field ISDX_NEQ0 must match
 * '1': Entry field ISDX_NEQ0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP6_STD_MASK_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_MASK_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * Mask for field IP6_STD_ENTRY:ACE_TYPE.L3_RT
 *
 * \details 
 * '0': Entry field L3_RT must match
 * '1': Entry field L3_RT is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP6_STD_MASK_ACE_TYPE . L3_RT
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_MASK_ACE_TYPE_L3_RT  VTSS_BIT(22)

/** 
 * \brief
 * Mask for field IP6_STD_ENTRY:ACE_TYPE.L3_SMAC_SIP_MATCH
 *
 * \details 
 * '0': Entry field L3_SMAC_SIP_MATCH must match
 * '1': Entry field L3_SMAC_SIP_MATCH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP6_STD_MASK_ACE_TYPE . L3_SMAC_SIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_MASK_ACE_TYPE_L3_SMAC_SIP_MATCH  VTSS_BIT(21)

/** 
 * \brief
 * Mask for field IP6_STD_ENTRY:ACE_TYPE.L3_DMAC_DIP_MATCH
 *
 * \details 
 * '0': Entry field L3_DMAC_DIP_MATCH must match
 * '1': Entry field L3_DMAC_DIP_MATCH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP6_STD_MASK_ACE_TYPE . L3_DMAC_DIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_MASK_ACE_TYPE_L3_DMAC_DIP_MATCH  VTSS_BIT(23)

/** 
 * \brief
 * Mask for field IP6_STD_ENTRY:ACE_TYPE.L2_FWD
 *
 * \details 
 * '0': Entry field L2_FWD must match
 * '1': Entry field L2_FWD is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP6_STD_MASK_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_MASK_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * Mask for field IP6_STD_ENTRY:ACE_TYPE.VID
 *
 * \details 
 * '0': Entry field VID must match
 * '1': Entry field VID is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP6_STD_MASK_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_MASK_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_IP6_STD_MASK_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_IP6_STD_MASK_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * Mask for field IP6_STD_ENTRY:ACE_TYPE.UPRIO
 *
 * \details 
 * '0': Entry field UPRIO must match
 * '1': Entry field UPRIO is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP6_STD_MASK_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_MASK_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_IP6_STD_MASK_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_IP6_STD_MASK_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * Mask for field IP6_STD_ENTRY:ACE_TYPE.CFI
 *
 * \details 
 * '0': Entry field CFI must match
 * '1': Entry field CFI is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP6_STD_MASK_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_MASK_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Mask for field IP6_STD_ENTRY:ACE_TYPE.L2_BC
 *
 * \details 
 * '0': Entry field L2_BC must match
 * '1': Entry field L2_BC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP6_STD_MASK_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_MASK_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Mask for field IP6_STD_ENTRY:ACE_TYPE.L2_MC
 *
 * \details 
 * '0': Entry field L2_MC must match
 * '1': Entry field L2_MC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP6_STD_MASK_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_MASK_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * Mask for field IP6_STD_ENTRY:ACE_TYPE.FIRST
 *
 * \details 
 * '0': Entry field FIRST must match
 * '1': Entry field FIRST is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP6_STD_MASK_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_MASK_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Mask register corresponding to IP6_STD_ENTRY:ACE_IGR_PORT_MASK
 *
 * \details
 * Mask register corresponding to IP6_STD_ENTRY:ACE_IGR_PORT_MASK
 *
 * Register: \a VCAP_IS2:IP6_STD_MASK:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_IP6_STD_MASK_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x18a)


/** 
 * \brief Mask register corresponding to IP6_STD_ENTRY:ACE_PAG
 *
 * \details
 * Mask register corresponding to IP6_STD_ENTRY:ACE_PAG
 *
 * Register: \a VCAP_IS2:IP6_STD_MASK:ACE_PAG
 */
#define VTSS_VCAP_IS2_IP6_STD_MASK_ACE_PAG   VTSS_IOREG(VTSS_TO_VCAP_IS2,0x18b)

/** 
 * \brief
 * Mask for field IP6_STD_ENTRY:ACE_PAG.PAG
 *
 * \details 
 * '0': Entry field PAG must match
 * '1': Entry field PAG is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP6_STD_MASK_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_MASK_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_IP6_STD_MASK_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_IP6_STD_MASK_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Mask register corresponding to IP6_STD_ENTRY:ACE_L3_MISC
 *
 * \details
 * Mask register corresponding to IP6_STD_ENTRY:ACE_L3_MISC
 *
 * Register: \a VCAP_IS2:IP6_STD_MASK:ACE_L3_MISC
 */
#define VTSS_VCAP_IS2_IP6_STD_MASK_ACE_L3_MISC  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x18c)

/** 
 * \brief
 * Mask for field IP6_STD_ENTRY:ACE_L3_MISC.L3_IP_PROTO
 *
 * \details 
 * '0': Entry field L3_IP_PROTO must match
 * '1': Entry field L3_IP_PROTO is ignored
 *
 * Field: ::VTSS_VCAP_IS2_IP6_STD_MASK_ACE_L3_MISC . L3_IP_PROTO
 */
#define  VTSS_F_VCAP_IS2_IP6_STD_MASK_ACE_L3_MISC_L3_IP_PROTO(x)  VTSS_ENCODE_BITFIELD(x,12,8)
#define  VTSS_M_VCAP_IS2_IP6_STD_MASK_ACE_L3_MISC_L3_IP_PROTO     VTSS_ENCODE_BITMASK(12,8)
#define  VTSS_X_VCAP_IS2_IP6_STD_MASK_ACE_L3_MISC_L3_IP_PROTO(x)  VTSS_EXTRACT_BITFIELD(x,12,8)


/** 
 * \brief Mask register corresponding to IP6_STD_ENTRY:ACE_L3_IP6_SIP_0
 *
 * \details
 * Mask register corresponding to IP6_STD_ENTRY:ACE_L3_IP6_SIP_0
 *
 * Register: \a VCAP_IS2:IP6_STD_MASK:ACE_L3_IP6_SIP_0
 */
#define VTSS_VCAP_IS2_IP6_STD_MASK_ACE_L3_IP6_SIP_0  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x18d)


/** 
 * \brief Mask register corresponding to IP6_STD_ENTRY:ACE_L3_IP6_SIP_1
 *
 * \details
 * Mask register corresponding to IP6_STD_ENTRY:ACE_L3_IP6_SIP_1
 *
 * Register: \a VCAP_IS2:IP6_STD_MASK:ACE_L3_IP6_SIP_1
 */
#define VTSS_VCAP_IS2_IP6_STD_MASK_ACE_L3_IP6_SIP_1  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x18e)


/** 
 * \brief Mask register corresponding to IP6_STD_ENTRY:ACE_L3_IP6_SIP_2
 *
 * \details
 * Mask register corresponding to IP6_STD_ENTRY:ACE_L3_IP6_SIP_2
 *
 * Register: \a VCAP_IS2:IP6_STD_MASK:ACE_L3_IP6_SIP_2
 */
#define VTSS_VCAP_IS2_IP6_STD_MASK_ACE_L3_IP6_SIP_2  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x18f)


/** 
 * \brief Mask register corresponding to IP6_STD_ENTRY:ACE_L3_IP6_SIP_3
 *
 * \details
 * Mask register corresponding to IP6_STD_ENTRY:ACE_L3_IP6_SIP_3
 *
 * Register: \a VCAP_IS2:IP6_STD_MASK:ACE_L3_IP6_SIP_3
 */
#define VTSS_VCAP_IS2_IP6_STD_MASK_ACE_L3_IP6_SIP_3  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x190)

/**
 * Register Group: \a VCAP_IS2:OAM_MASK
 *
 * Entry mask for entry type OAM.
 */


/** 
 * \brief Mask register corresponding to OAM_ENTRY:ACE_VLD
 *
 * \details
 * Mask register corresponding to OAM_ENTRY:ACE_VLD
 *
 * Register: \a VCAP_IS2:OAM_MASK:ACE_VLD
 */
#define VTSS_VCAP_IS2_OAM_MASK_ACE_VLD       VTSS_IOREG(VTSS_TO_VCAP_IS2,0x198)

/** 
 * \brief
 * Mask has to be valid always.
 *
 * \details 
 * Must be set to '0'
 *
 * Field: ::VTSS_VCAP_IS2_OAM_MASK_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_OAM_MASK_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to OAM_ENTRY:ACE_TYPE
 *
 * \details
 * Mask register corresponding to OAM_ENTRY:ACE_TYPE
 *
 * Register: \a VCAP_IS2:OAM_MASK:ACE_TYPE
 */
#define VTSS_VCAP_IS2_OAM_MASK_ACE_TYPE      VTSS_IOREG(VTSS_TO_VCAP_IS2,0x199)

/** 
 * \brief
 * Mask for field OAM_ENTRY:ACE_TYPE.VLAN_TAGGED
 *
 * \details 
 * '0': Entry field VLAN_TAGGED must match
 * '1': Entry field VLAN_TAGGED is ignored
 *
 * Field: ::VTSS_VCAP_IS2_OAM_MASK_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_OAM_MASK_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Mask for field OAM_ENTRY:ACE_TYPE.ISDX_NEQ0
 *
 * \details 
 * '0': Entry field ISDX_NEQ0 must match
 * '1': Entry field ISDX_NEQ0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_OAM_MASK_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_OAM_MASK_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * Mask for field OAM_ENTRY:ACE_TYPE.L2_FWD
 *
 * \details 
 * '0': Entry field L2_FWD must match
 * '1': Entry field L2_FWD is ignored
 *
 * Field: ::VTSS_VCAP_IS2_OAM_MASK_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_OAM_MASK_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * Mask for field OAM_ENTRY:ACE_TYPE.VID
 *
 * \details 
 * '0': Entry field VID must match
 * '1': Entry field VID is ignored
 *
 * Field: ::VTSS_VCAP_IS2_OAM_MASK_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_OAM_MASK_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_OAM_MASK_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_OAM_MASK_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * Mask for field OAM_ENTRY:ACE_TYPE.UPRIO
 *
 * \details 
 * '0': Entry field UPRIO must match
 * '1': Entry field UPRIO is ignored
 *
 * Field: ::VTSS_VCAP_IS2_OAM_MASK_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_OAM_MASK_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_OAM_MASK_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_OAM_MASK_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * Mask for field OAM_ENTRY:ACE_TYPE.CFI
 *
 * \details 
 * '0': Entry field CFI must match
 * '1': Entry field CFI is ignored
 *
 * Field: ::VTSS_VCAP_IS2_OAM_MASK_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_OAM_MASK_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Mask for field OAM_ENTRY:ACE_TYPE.FIRST
 *
 * \details 
 * '0': Entry field FIRST must match
 * '1': Entry field FIRST is ignored
 *
 * Field: ::VTSS_VCAP_IS2_OAM_MASK_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_OAM_MASK_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Mask register corresponding to OAM_ENTRY:ACE_IGR_PORT_MASK
 *
 * \details
 * Mask register corresponding to OAM_ENTRY:ACE_IGR_PORT_MASK
 *
 * Register: \a VCAP_IS2:OAM_MASK:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_OAM_MASK_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x19a)


/** 
 * \brief Mask register corresponding to OAM_ENTRY:ACE_PAG
 *
 * \details
 * Mask register corresponding to OAM_ENTRY:ACE_PAG
 *
 * Register: \a VCAP_IS2:OAM_MASK:ACE_PAG
 */
#define VTSS_VCAP_IS2_OAM_MASK_ACE_PAG       VTSS_IOREG(VTSS_TO_VCAP_IS2,0x19b)

/** 
 * \brief
 * Mask for field OAM_ENTRY:ACE_PAG.PAG
 *
 * \details 
 * '0': Entry field PAG must match
 * '1': Entry field PAG is ignored
 *
 * Field: ::VTSS_VCAP_IS2_OAM_MASK_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_OAM_MASK_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_OAM_MASK_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_OAM_MASK_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Mask register corresponding to OAM_ENTRY:ACE_L2_SMAC_HIGH
 *
 * \details
 * Mask register corresponding to OAM_ENTRY:ACE_L2_SMAC_HIGH
 *
 * Register: \a VCAP_IS2:OAM_MASK:ACE_L2_SMAC_HIGH
 */
#define VTSS_VCAP_IS2_OAM_MASK_ACE_L2_SMAC_HIGH  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x19c)

/** 
 * \brief
 * Mask for field OAM_ENTRY:ACE_L2_SMAC_HIGH.L2_SMAC_HIGH
 *
 * \details 
 * '0': Entry field L2_SMAC_HIGH must match
 * '1': Entry field L2_SMAC_HIGH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_OAM_MASK_ACE_L2_SMAC_HIGH . L2_SMAC_HIGH
 */
#define  VTSS_F_VCAP_IS2_OAM_MASK_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_OAM_MASK_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_OAM_MASK_ACE_L2_SMAC_HIGH_L2_SMAC_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to OAM_ENTRY:ACE_L2_SMAC_LOW
 *
 * \details
 * Mask register corresponding to OAM_ENTRY:ACE_L2_SMAC_LOW
 *
 * Register: \a VCAP_IS2:OAM_MASK:ACE_L2_SMAC_LOW
 */
#define VTSS_VCAP_IS2_OAM_MASK_ACE_L2_SMAC_LOW  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x19d)


/** 
 * \brief Mask register corresponding to OAM_ENTRY:ACE_L2_DMAC_HIGH
 *
 * \details
 * Mask register corresponding to OAM_ENTRY:ACE_L2_DMAC_HIGH
 *
 * Register: \a VCAP_IS2:OAM_MASK:ACE_L2_DMAC_HIGH
 */
#define VTSS_VCAP_IS2_OAM_MASK_ACE_L2_DMAC_HIGH  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x19e)

/** 
 * \brief
 * Mask for field OAM_ENTRY:ACE_L2_DMAC_HIGH.L2_DMAC_HIGH
 *
 * \details 
 * '0': Entry field L2_DMAC_HIGH must match
 * '1': Entry field L2_DMAC_HIGH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_OAM_MASK_ACE_L2_DMAC_HIGH . L2_DMAC_HIGH
 */
#define  VTSS_F_VCAP_IS2_OAM_MASK_ACE_L2_DMAC_HIGH_L2_DMAC_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_OAM_MASK_ACE_L2_DMAC_HIGH_L2_DMAC_HIGH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_OAM_MASK_ACE_L2_DMAC_HIGH_L2_DMAC_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to OAM_ENTRY:ACE_L2_DMAC_LOW
 *
 * \details
 * Mask register corresponding to OAM_ENTRY:ACE_L2_DMAC_LOW
 *
 * Register: \a VCAP_IS2:OAM_MASK:ACE_L2_DMAC_LOW
 */
#define VTSS_VCAP_IS2_OAM_MASK_ACE_L2_DMAC_LOW  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x19f)


/** 
 * \brief Mask register corresponding to OAM_ENTRY:ACE_OAM_0
 *
 * \details
 * Mask register corresponding to OAM_ENTRY:ACE_OAM_0
 *
 * Register: \a VCAP_IS2:OAM_MASK:ACE_OAM_0
 */
#define VTSS_VCAP_IS2_OAM_MASK_ACE_OAM_0     VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1a0)

/** 
 * \brief
 * Mask for field OAM_ENTRY:ACE_OAM_0.OAM_VER
 *
 * \details 
 * '0': Entry field OAM_VER must match
 * '1': Entry field OAM_VER is ignored
 *
 * Field: ::VTSS_VCAP_IS2_OAM_MASK_ACE_OAM_0 . OAM_VER
 */
#define  VTSS_F_VCAP_IS2_OAM_MASK_ACE_OAM_0_OAM_VER(x)  VTSS_ENCODE_BITFIELD(x,8,5)
#define  VTSS_M_VCAP_IS2_OAM_MASK_ACE_OAM_0_OAM_VER     VTSS_ENCODE_BITMASK(8,5)
#define  VTSS_X_VCAP_IS2_OAM_MASK_ACE_OAM_0_OAM_VER(x)  VTSS_EXTRACT_BITFIELD(x,8,5)

/** 
 * \brief
 * Mask for field OAM_ENTRY:ACE_OAM_0.OAM_MEL_FLAGS
 *
 * \details 
 * '0': Entry field OAM_MEL_FLAGS must match
 * '1': Entry field OAM_MEL_FLAGS is ignored
 *
 * Field: ::VTSS_VCAP_IS2_OAM_MASK_ACE_OAM_0 . OAM_MEL_FLAGS
 */
#define  VTSS_F_VCAP_IS2_OAM_MASK_ACE_OAM_0_OAM_MEL_FLAGS(x)  VTSS_ENCODE_BITFIELD(x,1,7)
#define  VTSS_M_VCAP_IS2_OAM_MASK_ACE_OAM_0_OAM_MEL_FLAGS     VTSS_ENCODE_BITMASK(1,7)
#define  VTSS_X_VCAP_IS2_OAM_MASK_ACE_OAM_0_OAM_MEL_FLAGS(x)  VTSS_EXTRACT_BITFIELD(x,1,7)

/** 
 * \brief
 * Mask for field OAM_ENTRY:ACE_OAM_0.ISMACINMACOAM
 *
 * \details 
 * '0': Entry field ISMACINMACOAM must match
 * '1': Entry field ISMACINMACOAM is ignored
 *
 * Field: ::VTSS_VCAP_IS2_OAM_MASK_ACE_OAM_0 . ISMACINMACOAM
 */
#define  VTSS_F_VCAP_IS2_OAM_MASK_ACE_OAM_0_ISMACINMACOAM  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to OAM_ENTRY:ACE_OAM_1
 *
 * \details
 * Mask register corresponding to OAM_ENTRY:ACE_OAM_1
 *
 * Register: \a VCAP_IS2:OAM_MASK:ACE_OAM_1
 */
#define VTSS_VCAP_IS2_OAM_MASK_ACE_OAM_1     VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1a1)

/** 
 * \brief
 * Mask for field OAM_ENTRY:ACE_OAM_1.OAM_MEPID
 *
 * \details 
 * '0': Entry field OAM_MEPID must match
 * '1': Entry field OAM_MEPID is ignored
 *
 * Field: ::VTSS_VCAP_IS2_OAM_MASK_ACE_OAM_1 . OAM_MEPID
 */
#define  VTSS_F_VCAP_IS2_OAM_MASK_ACE_OAM_1_OAM_MEPID(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_VCAP_IS2_OAM_MASK_ACE_OAM_1_OAM_MEPID     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_VCAP_IS2_OAM_MASK_ACE_OAM_1_OAM_MEPID(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Mask for field OAM_ENTRY:ACE_OAM_1.OAM_FLAGS
 *
 * \details 
 * '0': Entry field OAM_FLAGS must match
 * '1': Entry field OAM_FLAGS is ignored
 *
 * Field: ::VTSS_VCAP_IS2_OAM_MASK_ACE_OAM_1 . OAM_FLAGS
 */
#define  VTSS_F_VCAP_IS2_OAM_MASK_ACE_OAM_1_OAM_FLAGS(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_VCAP_IS2_OAM_MASK_ACE_OAM_1_OAM_FLAGS     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_VCAP_IS2_OAM_MASK_ACE_OAM_1_OAM_FLAGS(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Mask for field OAM_ENTRY:ACE_OAM_1.OAM_OPCODE
 *
 * \details 
 * '0': Entry field OAM_OPCODE must match
 * '1': Entry field OAM_OPCODE is ignored
 *
 * Field: ::VTSS_VCAP_IS2_OAM_MASK_ACE_OAM_1 . OAM_OPCODE
 */
#define  VTSS_F_VCAP_IS2_OAM_MASK_ACE_OAM_1_OAM_OPCODE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_OAM_MASK_ACE_OAM_1_OAM_OPCODE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_OAM_MASK_ACE_OAM_1_OAM_OPCODE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a VCAP_IS2:CUSTOM_0_MASK
 *
 * Entry mask for entry type CUSTOM_0.
 */


/** 
 * \brief Mask register corresponding to CUSTOM_0_ENTRY:ACE_VLD
 *
 * \details
 * Mask register corresponding to CUSTOM_0_ENTRY:ACE_VLD
 *
 * Register: \a VCAP_IS2:CUSTOM_0_MASK:ACE_VLD
 */
#define VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_VLD  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1a8)

/** 
 * \brief
 * Mask has to be valid always.
 *
 * \details 
 * Must be set to '0'
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_MASK_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to CUSTOM_0_ENTRY:ACE_TYPE
 *
 * \details
 * Mask register corresponding to CUSTOM_0_ENTRY:ACE_TYPE
 *
 * Register: \a VCAP_IS2:CUSTOM_0_MASK:ACE_TYPE
 */
#define VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1a9)

/** 
 * \brief
 * Mask for field CUSTOM_0_ENTRY:ACE_TYPE.VLAN_TAGGED
 *
 * \details 
 * '0': Entry field VLAN_TAGGED must match
 * '1': Entry field VLAN_TAGGED is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Mask for field CUSTOM_0_ENTRY:ACE_TYPE.ISDX_NEQ0
 *
 * \details 
 * '0': Entry field ISDX_NEQ0 must match
 * '1': Entry field ISDX_NEQ0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * Mask for field CUSTOM_0_ENTRY:ACE_TYPE.L3_RT
 *
 * \details 
 * '0': Entry field L3_RT must match
 * '1': Entry field L3_RT is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE . L3_RT
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE_L3_RT  VTSS_BIT(22)

/** 
 * \brief
 * Mask for field CUSTOM_0_ENTRY:ACE_TYPE.L3_SMAC_SIP_MATCH
 *
 * \details 
 * '0': Entry field L3_SMAC_SIP_MATCH must match
 * '1': Entry field L3_SMAC_SIP_MATCH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE . L3_SMAC_SIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE_L3_SMAC_SIP_MATCH  VTSS_BIT(21)

/** 
 * \brief
 * Mask for field CUSTOM_0_ENTRY:ACE_TYPE.L3_DMAC_DIP_MATCH
 *
 * \details 
 * '0': Entry field L3_DMAC_DIP_MATCH must match
 * '1': Entry field L3_DMAC_DIP_MATCH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE . L3_DMAC_DIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE_L3_DMAC_DIP_MATCH  VTSS_BIT(23)

/** 
 * \brief
 * Mask for field CUSTOM_0_ENTRY:ACE_TYPE.L2_FWD
 *
 * \details 
 * '0': Entry field L2_FWD must match
 * '1': Entry field L2_FWD is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * Mask for field CUSTOM_0_ENTRY:ACE_TYPE.VID
 *
 * \details 
 * '0': Entry field VID must match
 * '1': Entry field VID is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * Mask for field CUSTOM_0_ENTRY:ACE_TYPE.UPRIO
 *
 * \details 
 * '0': Entry field UPRIO must match
 * '1': Entry field UPRIO is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * Mask for field CUSTOM_0_ENTRY:ACE_TYPE.CFI
 *
 * \details 
 * '0': Entry field CFI must match
 * '1': Entry field CFI is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Mask for field CUSTOM_0_ENTRY:ACE_TYPE.L2_BC
 *
 * \details 
 * '0': Entry field L2_BC must match
 * '1': Entry field L2_BC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Mask for field CUSTOM_0_ENTRY:ACE_TYPE.L2_MC
 *
 * \details 
 * '0': Entry field L2_MC must match
 * '1': Entry field L2_MC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * Mask for field CUSTOM_0_ENTRY:ACE_TYPE.FIRST
 *
 * \details 
 * '0': Entry field FIRST must match
 * '1': Entry field FIRST is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_MASK_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Mask register corresponding to CUSTOM_0_ENTRY:ACE_IGR_PORT_MASK
 *
 * \details
 * Mask register corresponding to CUSTOM_0_ENTRY:ACE_IGR_PORT_MASK
 *
 * Register: \a VCAP_IS2:CUSTOM_0_MASK:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1aa)


/** 
 * \brief Mask register corresponding to CUSTOM_0_ENTRY:ACE_PAG
 *
 * \details
 * Mask register corresponding to CUSTOM_0_ENTRY:ACE_PAG
 *
 * Register: \a VCAP_IS2:CUSTOM_0_MASK:ACE_PAG
 */
#define VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_PAG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1ab)

/** 
 * \brief
 * Mask for field CUSTOM_0_ENTRY:ACE_PAG.PAG
 *
 * \details 
 * '0': Entry field PAG must match
 * '1': Entry field PAG is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_MASK_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_CUSTOM_0_MASK_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_CUSTOM_0_MASK_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Mask register corresponding to CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_0
 *
 * \details
 * Mask register corresponding to CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_0
 *
 * Register: \a VCAP_IS2:CUSTOM_0_MASK:ACE_CUSTOM_DATA_0
 */
#define VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_0  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1ac)

/** 
 * \brief
 * Mask for field CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_0.CUSTOM_DATA_0
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_0 must match
 * '1': Entry field CUSTOM_DATA_0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_0 . CUSTOM_DATA_0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_1
 *
 * \details
 * Mask register corresponding to CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_1
 *
 * Register: \a VCAP_IS2:CUSTOM_0_MASK:ACE_CUSTOM_DATA_1
 */
#define VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_1  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1ad)

/** 
 * \brief
 * Mask for field CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_1.CUSTOM_DATA_1
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_1 must match
 * '1': Entry field CUSTOM_DATA_1 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_1 . CUSTOM_DATA_1
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_2
 *
 * \details
 * Mask register corresponding to CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_2
 *
 * Register: \a VCAP_IS2:CUSTOM_0_MASK:ACE_CUSTOM_DATA_2
 */
#define VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_2  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1ae)

/** 
 * \brief
 * Mask for field CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_2.CUSTOM_DATA_2
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_2 must match
 * '1': Entry field CUSTOM_DATA_2 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_2 . CUSTOM_DATA_2
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_3
 *
 * \details
 * Mask register corresponding to CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_3
 *
 * Register: \a VCAP_IS2:CUSTOM_0_MASK:ACE_CUSTOM_DATA_3
 */
#define VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_3  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1af)

/** 
 * \brief
 * Mask for field CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_3.CUSTOM_DATA_3
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_3 must match
 * '1': Entry field CUSTOM_DATA_3 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_3 . CUSTOM_DATA_3
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_4
 *
 * \details
 * Mask register corresponding to CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_4
 *
 * Register: \a VCAP_IS2:CUSTOM_0_MASK:ACE_CUSTOM_DATA_4
 */
#define VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_4  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1b0)

/** 
 * \brief
 * Mask for field CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_4.CUSTOM_DATA_4
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_4 must match
 * '1': Entry field CUSTOM_DATA_4 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_4 . CUSTOM_DATA_4
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_5
 *
 * \details
 * Mask register corresponding to CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_5
 *
 * Register: \a VCAP_IS2:CUSTOM_0_MASK:ACE_CUSTOM_DATA_5
 */
#define VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_5  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1b1)

/** 
 * \brief
 * Mask for field CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_5.CUSTOM_DATA_5
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_5 must match
 * '1': Entry field CUSTOM_DATA_5 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_5 . CUSTOM_DATA_5
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_6
 *
 * \details
 * Mask register corresponding to CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_6
 *
 * Register: \a VCAP_IS2:CUSTOM_0_MASK:ACE_CUSTOM_DATA_6
 */
#define VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_6  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1b2)

/** 
 * \brief
 * Mask for field CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_6.CUSTOM_DATA_6
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_6 must match
 * '1': Entry field CUSTOM_DATA_6 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_6 . CUSTOM_DATA_6
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_7
 *
 * \details
 * Mask register corresponding to CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_7
 *
 * Register: \a VCAP_IS2:CUSTOM_0_MASK:ACE_CUSTOM_DATA_7
 */
#define VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_7  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1b3)

/** 
 * \brief
 * Mask for field CUSTOM_0_ENTRY:ACE_CUSTOM_DATA_7.CUSTOM_DATA_7
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_7 must match
 * '1': Entry field CUSTOM_DATA_7 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_7 . CUSTOM_DATA_7
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_0_MASK_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VCAP_IS2:CUSTOM_1_MASK
 *
 * Entry mask for entry type CUSTOM_1.
 */


/** 
 * \brief Mask register corresponding to CUSTOM_1_ENTRY:ACE_VLD
 *
 * \details
 * Mask register corresponding to CUSTOM_1_ENTRY:ACE_VLD
 *
 * Register: \a VCAP_IS2:CUSTOM_1_MASK:ACE_VLD
 */
#define VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_VLD  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1b8)

/** 
 * \brief
 * Mask has to be valid always.
 *
 * \details 
 * Must be set to '0'
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_MASK_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to CUSTOM_1_ENTRY:ACE_TYPE
 *
 * \details
 * Mask register corresponding to CUSTOM_1_ENTRY:ACE_TYPE
 *
 * Register: \a VCAP_IS2:CUSTOM_1_MASK:ACE_TYPE
 */
#define VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1b9)

/** 
 * \brief
 * Mask for field CUSTOM_1_ENTRY:ACE_TYPE.VLAN_TAGGED
 *
 * \details 
 * '0': Entry field VLAN_TAGGED must match
 * '1': Entry field VLAN_TAGGED is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Mask for field CUSTOM_1_ENTRY:ACE_TYPE.ISDX_NEQ0
 *
 * \details 
 * '0': Entry field ISDX_NEQ0 must match
 * '1': Entry field ISDX_NEQ0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * Mask for field CUSTOM_1_ENTRY:ACE_TYPE.L3_RT
 *
 * \details 
 * '0': Entry field L3_RT must match
 * '1': Entry field L3_RT is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE . L3_RT
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE_L3_RT  VTSS_BIT(22)

/** 
 * \brief
 * Mask for field CUSTOM_1_ENTRY:ACE_TYPE.L3_SMAC_SIP_MATCH
 *
 * \details 
 * '0': Entry field L3_SMAC_SIP_MATCH must match
 * '1': Entry field L3_SMAC_SIP_MATCH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE . L3_SMAC_SIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE_L3_SMAC_SIP_MATCH  VTSS_BIT(21)

/** 
 * \brief
 * Mask for field CUSTOM_1_ENTRY:ACE_TYPE.L3_DMAC_DIP_MATCH
 *
 * \details 
 * '0': Entry field L3_DMAC_DIP_MATCH must match
 * '1': Entry field L3_DMAC_DIP_MATCH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE . L3_DMAC_DIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE_L3_DMAC_DIP_MATCH  VTSS_BIT(23)

/** 
 * \brief
 * Mask for field CUSTOM_1_ENTRY:ACE_TYPE.L2_FWD
 *
 * \details 
 * '0': Entry field L2_FWD must match
 * '1': Entry field L2_FWD is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * Mask for field CUSTOM_1_ENTRY:ACE_TYPE.VID
 *
 * \details 
 * '0': Entry field VID must match
 * '1': Entry field VID is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * Mask for field CUSTOM_1_ENTRY:ACE_TYPE.UPRIO
 *
 * \details 
 * '0': Entry field UPRIO must match
 * '1': Entry field UPRIO is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * Mask for field CUSTOM_1_ENTRY:ACE_TYPE.CFI
 *
 * \details 
 * '0': Entry field CFI must match
 * '1': Entry field CFI is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Mask for field CUSTOM_1_ENTRY:ACE_TYPE.L2_BC
 *
 * \details 
 * '0': Entry field L2_BC must match
 * '1': Entry field L2_BC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Mask for field CUSTOM_1_ENTRY:ACE_TYPE.L2_MC
 *
 * \details 
 * '0': Entry field L2_MC must match
 * '1': Entry field L2_MC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * Mask for field CUSTOM_1_ENTRY:ACE_TYPE.FIRST
 *
 * \details 
 * '0': Entry field FIRST must match
 * '1': Entry field FIRST is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_MASK_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Mask register corresponding to CUSTOM_1_ENTRY:ACE_IGR_PORT_MASK
 *
 * \details
 * Mask register corresponding to CUSTOM_1_ENTRY:ACE_IGR_PORT_MASK
 *
 * Register: \a VCAP_IS2:CUSTOM_1_MASK:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1ba)


/** 
 * \brief Mask register corresponding to CUSTOM_1_ENTRY:ACE_PAG
 *
 * \details
 * Mask register corresponding to CUSTOM_1_ENTRY:ACE_PAG
 *
 * Register: \a VCAP_IS2:CUSTOM_1_MASK:ACE_PAG
 */
#define VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_PAG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1bb)

/** 
 * \brief
 * Mask for field CUSTOM_1_ENTRY:ACE_PAG.PAG
 *
 * \details 
 * '0': Entry field PAG must match
 * '1': Entry field PAG is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_MASK_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_CUSTOM_1_MASK_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_CUSTOM_1_MASK_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Mask register corresponding to CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_0
 *
 * \details
 * Mask register corresponding to CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_0
 *
 * Register: \a VCAP_IS2:CUSTOM_1_MASK:ACE_CUSTOM_DATA_0
 */
#define VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_0  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1bc)

/** 
 * \brief
 * Mask for field CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_0.CUSTOM_DATA_0
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_0 must match
 * '1': Entry field CUSTOM_DATA_0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_0 . CUSTOM_DATA_0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_1
 *
 * \details
 * Mask register corresponding to CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_1
 *
 * Register: \a VCAP_IS2:CUSTOM_1_MASK:ACE_CUSTOM_DATA_1
 */
#define VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_1  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1bd)

/** 
 * \brief
 * Mask for field CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_1.CUSTOM_DATA_1
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_1 must match
 * '1': Entry field CUSTOM_DATA_1 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_1 . CUSTOM_DATA_1
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_2
 *
 * \details
 * Mask register corresponding to CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_2
 *
 * Register: \a VCAP_IS2:CUSTOM_1_MASK:ACE_CUSTOM_DATA_2
 */
#define VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_2  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1be)

/** 
 * \brief
 * Mask for field CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_2.CUSTOM_DATA_2
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_2 must match
 * '1': Entry field CUSTOM_DATA_2 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_2 . CUSTOM_DATA_2
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_3
 *
 * \details
 * Mask register corresponding to CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_3
 *
 * Register: \a VCAP_IS2:CUSTOM_1_MASK:ACE_CUSTOM_DATA_3
 */
#define VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_3  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1bf)

/** 
 * \brief
 * Mask for field CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_3.CUSTOM_DATA_3
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_3 must match
 * '1': Entry field CUSTOM_DATA_3 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_3 . CUSTOM_DATA_3
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_4
 *
 * \details
 * Mask register corresponding to CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_4
 *
 * Register: \a VCAP_IS2:CUSTOM_1_MASK:ACE_CUSTOM_DATA_4
 */
#define VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_4  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1c0)

/** 
 * \brief
 * Mask for field CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_4.CUSTOM_DATA_4
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_4 must match
 * '1': Entry field CUSTOM_DATA_4 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_4 . CUSTOM_DATA_4
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_5
 *
 * \details
 * Mask register corresponding to CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_5
 *
 * Register: \a VCAP_IS2:CUSTOM_1_MASK:ACE_CUSTOM_DATA_5
 */
#define VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_5  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1c1)

/** 
 * \brief
 * Mask for field CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_5.CUSTOM_DATA_5
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_5 must match
 * '1': Entry field CUSTOM_DATA_5 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_5 . CUSTOM_DATA_5
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_6
 *
 * \details
 * Mask register corresponding to CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_6
 *
 * Register: \a VCAP_IS2:CUSTOM_1_MASK:ACE_CUSTOM_DATA_6
 */
#define VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_6  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1c2)

/** 
 * \brief
 * Mask for field CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_6.CUSTOM_DATA_6
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_6 must match
 * '1': Entry field CUSTOM_DATA_6 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_6 . CUSTOM_DATA_6
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_7
 *
 * \details
 * Mask register corresponding to CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_7
 *
 * Register: \a VCAP_IS2:CUSTOM_1_MASK:ACE_CUSTOM_DATA_7
 */
#define VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_7  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1c3)

/** 
 * \brief
 * Mask for field CUSTOM_1_ENTRY:ACE_CUSTOM_DATA_7.CUSTOM_DATA_7
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_7 must match
 * '1': Entry field CUSTOM_DATA_7 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_7 . CUSTOM_DATA_7
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_1_MASK_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VCAP_IS2:CUSTOM_2_MASK
 *
 * Entry mask for entry type CUSTOM_2.
 */


/** 
 * \brief Mask register corresponding to CUSTOM_2_ENTRY:ACE_VLD
 *
 * \details
 * Mask register corresponding to CUSTOM_2_ENTRY:ACE_VLD
 *
 * Register: \a VCAP_IS2:CUSTOM_2_MASK:ACE_VLD
 */
#define VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_VLD  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1c8)

/** 
 * \brief
 * Mask has to be valid always.
 *
 * \details 
 * Must be set to '0'
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_MASK_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to CUSTOM_2_ENTRY:ACE_TYPE
 *
 * \details
 * Mask register corresponding to CUSTOM_2_ENTRY:ACE_TYPE
 *
 * Register: \a VCAP_IS2:CUSTOM_2_MASK:ACE_TYPE
 */
#define VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1c9)

/** 
 * \brief
 * Mask for field CUSTOM_2_ENTRY:ACE_TYPE.VLAN_TAGGED
 *
 * \details 
 * '0': Entry field VLAN_TAGGED must match
 * '1': Entry field VLAN_TAGGED is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Mask for field CUSTOM_2_ENTRY:ACE_TYPE.ISDX_NEQ0
 *
 * \details 
 * '0': Entry field ISDX_NEQ0 must match
 * '1': Entry field ISDX_NEQ0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * Mask for field CUSTOM_2_ENTRY:ACE_TYPE.L3_RT
 *
 * \details 
 * '0': Entry field L3_RT must match
 * '1': Entry field L3_RT is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE . L3_RT
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE_L3_RT  VTSS_BIT(22)

/** 
 * \brief
 * Mask for field CUSTOM_2_ENTRY:ACE_TYPE.L3_SMAC_SIP_MATCH
 *
 * \details 
 * '0': Entry field L3_SMAC_SIP_MATCH must match
 * '1': Entry field L3_SMAC_SIP_MATCH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE . L3_SMAC_SIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE_L3_SMAC_SIP_MATCH  VTSS_BIT(21)

/** 
 * \brief
 * Mask for field CUSTOM_2_ENTRY:ACE_TYPE.L3_DMAC_DIP_MATCH
 *
 * \details 
 * '0': Entry field L3_DMAC_DIP_MATCH must match
 * '1': Entry field L3_DMAC_DIP_MATCH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE . L3_DMAC_DIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE_L3_DMAC_DIP_MATCH  VTSS_BIT(23)

/** 
 * \brief
 * Mask for field CUSTOM_2_ENTRY:ACE_TYPE.L2_FWD
 *
 * \details 
 * '0': Entry field L2_FWD must match
 * '1': Entry field L2_FWD is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * Mask for field CUSTOM_2_ENTRY:ACE_TYPE.VID
 *
 * \details 
 * '0': Entry field VID must match
 * '1': Entry field VID is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * Mask for field CUSTOM_2_ENTRY:ACE_TYPE.UPRIO
 *
 * \details 
 * '0': Entry field UPRIO must match
 * '1': Entry field UPRIO is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * Mask for field CUSTOM_2_ENTRY:ACE_TYPE.CFI
 *
 * \details 
 * '0': Entry field CFI must match
 * '1': Entry field CFI is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Mask for field CUSTOM_2_ENTRY:ACE_TYPE.L2_BC
 *
 * \details 
 * '0': Entry field L2_BC must match
 * '1': Entry field L2_BC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Mask for field CUSTOM_2_ENTRY:ACE_TYPE.L2_MC
 *
 * \details 
 * '0': Entry field L2_MC must match
 * '1': Entry field L2_MC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * Mask for field CUSTOM_2_ENTRY:ACE_TYPE.FIRST
 *
 * \details 
 * '0': Entry field FIRST must match
 * '1': Entry field FIRST is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_MASK_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Mask register corresponding to CUSTOM_2_ENTRY:ACE_IGR_PORT_MASK
 *
 * \details
 * Mask register corresponding to CUSTOM_2_ENTRY:ACE_IGR_PORT_MASK
 *
 * Register: \a VCAP_IS2:CUSTOM_2_MASK:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1ca)


/** 
 * \brief Mask register corresponding to CUSTOM_2_ENTRY:ACE_PAG
 *
 * \details
 * Mask register corresponding to CUSTOM_2_ENTRY:ACE_PAG
 *
 * Register: \a VCAP_IS2:CUSTOM_2_MASK:ACE_PAG
 */
#define VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_PAG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1cb)

/** 
 * \brief
 * Mask for field CUSTOM_2_ENTRY:ACE_PAG.PAG
 *
 * \details 
 * '0': Entry field PAG must match
 * '1': Entry field PAG is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_MASK_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_CUSTOM_2_MASK_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_CUSTOM_2_MASK_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Mask register corresponding to CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_0
 *
 * \details
 * Mask register corresponding to CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_0
 *
 * Register: \a VCAP_IS2:CUSTOM_2_MASK:ACE_CUSTOM_DATA_0
 */
#define VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_0  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1cc)

/** 
 * \brief
 * Mask for field CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_0.CUSTOM_DATA_0
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_0 must match
 * '1': Entry field CUSTOM_DATA_0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_0 . CUSTOM_DATA_0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_1
 *
 * \details
 * Mask register corresponding to CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_1
 *
 * Register: \a VCAP_IS2:CUSTOM_2_MASK:ACE_CUSTOM_DATA_1
 */
#define VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_1  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1cd)

/** 
 * \brief
 * Mask for field CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_1.CUSTOM_DATA_1
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_1 must match
 * '1': Entry field CUSTOM_DATA_1 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_1 . CUSTOM_DATA_1
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_2
 *
 * \details
 * Mask register corresponding to CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_2
 *
 * Register: \a VCAP_IS2:CUSTOM_2_MASK:ACE_CUSTOM_DATA_2
 */
#define VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_2  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1ce)

/** 
 * \brief
 * Mask for field CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_2.CUSTOM_DATA_2
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_2 must match
 * '1': Entry field CUSTOM_DATA_2 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_2 . CUSTOM_DATA_2
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_3
 *
 * \details
 * Mask register corresponding to CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_3
 *
 * Register: \a VCAP_IS2:CUSTOM_2_MASK:ACE_CUSTOM_DATA_3
 */
#define VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_3  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1cf)

/** 
 * \brief
 * Mask for field CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_3.CUSTOM_DATA_3
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_3 must match
 * '1': Entry field CUSTOM_DATA_3 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_3 . CUSTOM_DATA_3
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_4
 *
 * \details
 * Mask register corresponding to CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_4
 *
 * Register: \a VCAP_IS2:CUSTOM_2_MASK:ACE_CUSTOM_DATA_4
 */
#define VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_4  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1d0)

/** 
 * \brief
 * Mask for field CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_4.CUSTOM_DATA_4
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_4 must match
 * '1': Entry field CUSTOM_DATA_4 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_4 . CUSTOM_DATA_4
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_5
 *
 * \details
 * Mask register corresponding to CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_5
 *
 * Register: \a VCAP_IS2:CUSTOM_2_MASK:ACE_CUSTOM_DATA_5
 */
#define VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_5  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1d1)

/** 
 * \brief
 * Mask for field CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_5.CUSTOM_DATA_5
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_5 must match
 * '1': Entry field CUSTOM_DATA_5 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_5 . CUSTOM_DATA_5
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_6
 *
 * \details
 * Mask register corresponding to CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_6
 *
 * Register: \a VCAP_IS2:CUSTOM_2_MASK:ACE_CUSTOM_DATA_6
 */
#define VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_6  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1d2)

/** 
 * \brief
 * Mask for field CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_6.CUSTOM_DATA_6
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_6 must match
 * '1': Entry field CUSTOM_DATA_6 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_6 . CUSTOM_DATA_6
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_7
 *
 * \details
 * Mask register corresponding to CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_7
 *
 * Register: \a VCAP_IS2:CUSTOM_2_MASK:ACE_CUSTOM_DATA_7
 */
#define VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_7  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1d3)

/** 
 * \brief
 * Mask for field CUSTOM_2_ENTRY:ACE_CUSTOM_DATA_7.CUSTOM_DATA_7
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_7 must match
 * '1': Entry field CUSTOM_DATA_7 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_7 . CUSTOM_DATA_7
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_2_MASK_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VCAP_IS2:CUSTOM_3_MASK
 *
 * Entry mask for entry type CUSTOM_3.
 */


/** 
 * \brief Mask register corresponding to CUSTOM_3_ENTRY:ACE_VLD
 *
 * \details
 * Mask register corresponding to CUSTOM_3_ENTRY:ACE_VLD
 *
 * Register: \a VCAP_IS2:CUSTOM_3_MASK:ACE_VLD
 */
#define VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_VLD  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1d8)

/** 
 * \brief
 * Mask has to be valid always.
 *
 * \details 
 * Must be set to '0'
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_MASK_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to CUSTOM_3_ENTRY:ACE_TYPE
 *
 * \details
 * Mask register corresponding to CUSTOM_3_ENTRY:ACE_TYPE
 *
 * Register: \a VCAP_IS2:CUSTOM_3_MASK:ACE_TYPE
 */
#define VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1d9)

/** 
 * \brief
 * Mask for field CUSTOM_3_ENTRY:ACE_TYPE.VLAN_TAGGED
 *
 * \details 
 * '0': Entry field VLAN_TAGGED must match
 * '1': Entry field VLAN_TAGGED is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Mask for field CUSTOM_3_ENTRY:ACE_TYPE.ISDX_NEQ0
 *
 * \details 
 * '0': Entry field ISDX_NEQ0 must match
 * '1': Entry field ISDX_NEQ0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * Mask for field CUSTOM_3_ENTRY:ACE_TYPE.L3_RT
 *
 * \details 
 * '0': Entry field L3_RT must match
 * '1': Entry field L3_RT is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE . L3_RT
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE_L3_RT  VTSS_BIT(22)

/** 
 * \brief
 * Mask for field CUSTOM_3_ENTRY:ACE_TYPE.L3_SMAC_SIP_MATCH
 *
 * \details 
 * '0': Entry field L3_SMAC_SIP_MATCH must match
 * '1': Entry field L3_SMAC_SIP_MATCH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE . L3_SMAC_SIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE_L3_SMAC_SIP_MATCH  VTSS_BIT(21)

/** 
 * \brief
 * Mask for field CUSTOM_3_ENTRY:ACE_TYPE.L3_DMAC_DIP_MATCH
 *
 * \details 
 * '0': Entry field L3_DMAC_DIP_MATCH must match
 * '1': Entry field L3_DMAC_DIP_MATCH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE . L3_DMAC_DIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE_L3_DMAC_DIP_MATCH  VTSS_BIT(23)

/** 
 * \brief
 * Mask for field CUSTOM_3_ENTRY:ACE_TYPE.L2_FWD
 *
 * \details 
 * '0': Entry field L2_FWD must match
 * '1': Entry field L2_FWD is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * Mask for field CUSTOM_3_ENTRY:ACE_TYPE.VID
 *
 * \details 
 * '0': Entry field VID must match
 * '1': Entry field VID is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * Mask for field CUSTOM_3_ENTRY:ACE_TYPE.UPRIO
 *
 * \details 
 * '0': Entry field UPRIO must match
 * '1': Entry field UPRIO is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * Mask for field CUSTOM_3_ENTRY:ACE_TYPE.CFI
 *
 * \details 
 * '0': Entry field CFI must match
 * '1': Entry field CFI is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Mask for field CUSTOM_3_ENTRY:ACE_TYPE.L2_BC
 *
 * \details 
 * '0': Entry field L2_BC must match
 * '1': Entry field L2_BC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Mask for field CUSTOM_3_ENTRY:ACE_TYPE.L2_MC
 *
 * \details 
 * '0': Entry field L2_MC must match
 * '1': Entry field L2_MC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * Mask for field CUSTOM_3_ENTRY:ACE_TYPE.FIRST
 *
 * \details 
 * '0': Entry field FIRST must match
 * '1': Entry field FIRST is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_MASK_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Mask register corresponding to CUSTOM_3_ENTRY:ACE_IGR_PORT_MASK
 *
 * \details
 * Mask register corresponding to CUSTOM_3_ENTRY:ACE_IGR_PORT_MASK
 *
 * Register: \a VCAP_IS2:CUSTOM_3_MASK:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1da)


/** 
 * \brief Mask register corresponding to CUSTOM_3_ENTRY:ACE_PAG
 *
 * \details
 * Mask register corresponding to CUSTOM_3_ENTRY:ACE_PAG
 *
 * Register: \a VCAP_IS2:CUSTOM_3_MASK:ACE_PAG
 */
#define VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_PAG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1db)

/** 
 * \brief
 * Mask for field CUSTOM_3_ENTRY:ACE_PAG.PAG
 *
 * \details 
 * '0': Entry field PAG must match
 * '1': Entry field PAG is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_MASK_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_CUSTOM_3_MASK_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_CUSTOM_3_MASK_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Mask register corresponding to CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_0
 *
 * \details
 * Mask register corresponding to CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_0
 *
 * Register: \a VCAP_IS2:CUSTOM_3_MASK:ACE_CUSTOM_DATA_0
 */
#define VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_0  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1dc)

/** 
 * \brief
 * Mask for field CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_0.CUSTOM_DATA_0
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_0 must match
 * '1': Entry field CUSTOM_DATA_0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_0 . CUSTOM_DATA_0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_1
 *
 * \details
 * Mask register corresponding to CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_1
 *
 * Register: \a VCAP_IS2:CUSTOM_3_MASK:ACE_CUSTOM_DATA_1
 */
#define VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_1  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1dd)

/** 
 * \brief
 * Mask for field CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_1.CUSTOM_DATA_1
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_1 must match
 * '1': Entry field CUSTOM_DATA_1 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_1 . CUSTOM_DATA_1
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_2
 *
 * \details
 * Mask register corresponding to CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_2
 *
 * Register: \a VCAP_IS2:CUSTOM_3_MASK:ACE_CUSTOM_DATA_2
 */
#define VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_2  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1de)

/** 
 * \brief
 * Mask for field CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_2.CUSTOM_DATA_2
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_2 must match
 * '1': Entry field CUSTOM_DATA_2 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_2 . CUSTOM_DATA_2
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_3
 *
 * \details
 * Mask register corresponding to CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_3
 *
 * Register: \a VCAP_IS2:CUSTOM_3_MASK:ACE_CUSTOM_DATA_3
 */
#define VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_3  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1df)

/** 
 * \brief
 * Mask for field CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_3.CUSTOM_DATA_3
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_3 must match
 * '1': Entry field CUSTOM_DATA_3 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_3 . CUSTOM_DATA_3
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_4
 *
 * \details
 * Mask register corresponding to CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_4
 *
 * Register: \a VCAP_IS2:CUSTOM_3_MASK:ACE_CUSTOM_DATA_4
 */
#define VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_4  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1e0)

/** 
 * \brief
 * Mask for field CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_4.CUSTOM_DATA_4
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_4 must match
 * '1': Entry field CUSTOM_DATA_4 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_4 . CUSTOM_DATA_4
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_5
 *
 * \details
 * Mask register corresponding to CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_5
 *
 * Register: \a VCAP_IS2:CUSTOM_3_MASK:ACE_CUSTOM_DATA_5
 */
#define VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_5  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1e1)

/** 
 * \brief
 * Mask for field CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_5.CUSTOM_DATA_5
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_5 must match
 * '1': Entry field CUSTOM_DATA_5 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_5 . CUSTOM_DATA_5
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_6
 *
 * \details
 * Mask register corresponding to CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_6
 *
 * Register: \a VCAP_IS2:CUSTOM_3_MASK:ACE_CUSTOM_DATA_6
 */
#define VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_6  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1e2)

/** 
 * \brief
 * Mask for field CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_6.CUSTOM_DATA_6
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_6 must match
 * '1': Entry field CUSTOM_DATA_6 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_6 . CUSTOM_DATA_6
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_7
 *
 * \details
 * Mask register corresponding to CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_7
 *
 * Register: \a VCAP_IS2:CUSTOM_3_MASK:ACE_CUSTOM_DATA_7
 */
#define VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_7  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1e3)

/** 
 * \brief
 * Mask for field CUSTOM_3_ENTRY:ACE_CUSTOM_DATA_7.CUSTOM_DATA_7
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_7 must match
 * '1': Entry field CUSTOM_DATA_7 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_7 . CUSTOM_DATA_7
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_3_MASK_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VCAP_IS2:CUSTOM_4_MASK
 *
 * Entry mask for entry type CUSTOM_4.
 */


/** 
 * \brief Mask register corresponding to CUSTOM_4_ENTRY:ACE_VLD
 *
 * \details
 * Mask register corresponding to CUSTOM_4_ENTRY:ACE_VLD
 *
 * Register: \a VCAP_IS2:CUSTOM_4_MASK:ACE_VLD
 */
#define VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_VLD  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1e8)

/** 
 * \brief
 * Mask has to be valid always.
 *
 * \details 
 * Must be set to '0'
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_MASK_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to CUSTOM_4_ENTRY:ACE_TYPE
 *
 * \details
 * Mask register corresponding to CUSTOM_4_ENTRY:ACE_TYPE
 *
 * Register: \a VCAP_IS2:CUSTOM_4_MASK:ACE_TYPE
 */
#define VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1e9)

/** 
 * \brief
 * Mask for field CUSTOM_4_ENTRY:ACE_TYPE.VLAN_TAGGED
 *
 * \details 
 * '0': Entry field VLAN_TAGGED must match
 * '1': Entry field VLAN_TAGGED is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Mask for field CUSTOM_4_ENTRY:ACE_TYPE.ISDX_NEQ0
 *
 * \details 
 * '0': Entry field ISDX_NEQ0 must match
 * '1': Entry field ISDX_NEQ0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * Mask for field CUSTOM_4_ENTRY:ACE_TYPE.L3_RT
 *
 * \details 
 * '0': Entry field L3_RT must match
 * '1': Entry field L3_RT is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE . L3_RT
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE_L3_RT  VTSS_BIT(22)

/** 
 * \brief
 * Mask for field CUSTOM_4_ENTRY:ACE_TYPE.L3_SMAC_SIP_MATCH
 *
 * \details 
 * '0': Entry field L3_SMAC_SIP_MATCH must match
 * '1': Entry field L3_SMAC_SIP_MATCH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE . L3_SMAC_SIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE_L3_SMAC_SIP_MATCH  VTSS_BIT(21)

/** 
 * \brief
 * Mask for field CUSTOM_4_ENTRY:ACE_TYPE.L3_DMAC_DIP_MATCH
 *
 * \details 
 * '0': Entry field L3_DMAC_DIP_MATCH must match
 * '1': Entry field L3_DMAC_DIP_MATCH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE . L3_DMAC_DIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE_L3_DMAC_DIP_MATCH  VTSS_BIT(23)

/** 
 * \brief
 * Mask for field CUSTOM_4_ENTRY:ACE_TYPE.L2_FWD
 *
 * \details 
 * '0': Entry field L2_FWD must match
 * '1': Entry field L2_FWD is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * Mask for field CUSTOM_4_ENTRY:ACE_TYPE.VID
 *
 * \details 
 * '0': Entry field VID must match
 * '1': Entry field VID is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * Mask for field CUSTOM_4_ENTRY:ACE_TYPE.UPRIO
 *
 * \details 
 * '0': Entry field UPRIO must match
 * '1': Entry field UPRIO is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * Mask for field CUSTOM_4_ENTRY:ACE_TYPE.CFI
 *
 * \details 
 * '0': Entry field CFI must match
 * '1': Entry field CFI is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Mask for field CUSTOM_4_ENTRY:ACE_TYPE.L2_BC
 *
 * \details 
 * '0': Entry field L2_BC must match
 * '1': Entry field L2_BC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Mask for field CUSTOM_4_ENTRY:ACE_TYPE.L2_MC
 *
 * \details 
 * '0': Entry field L2_MC must match
 * '1': Entry field L2_MC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * Mask for field CUSTOM_4_ENTRY:ACE_TYPE.FIRST
 *
 * \details 
 * '0': Entry field FIRST must match
 * '1': Entry field FIRST is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_MASK_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Mask register corresponding to CUSTOM_4_ENTRY:ACE_IGR_PORT_MASK
 *
 * \details
 * Mask register corresponding to CUSTOM_4_ENTRY:ACE_IGR_PORT_MASK
 *
 * Register: \a VCAP_IS2:CUSTOM_4_MASK:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1ea)


/** 
 * \brief Mask register corresponding to CUSTOM_4_ENTRY:ACE_PAG
 *
 * \details
 * Mask register corresponding to CUSTOM_4_ENTRY:ACE_PAG
 *
 * Register: \a VCAP_IS2:CUSTOM_4_MASK:ACE_PAG
 */
#define VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_PAG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1eb)

/** 
 * \brief
 * Mask for field CUSTOM_4_ENTRY:ACE_PAG.PAG
 *
 * \details 
 * '0': Entry field PAG must match
 * '1': Entry field PAG is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_MASK_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_CUSTOM_4_MASK_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_CUSTOM_4_MASK_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Mask register corresponding to CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_0
 *
 * \details
 * Mask register corresponding to CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_0
 *
 * Register: \a VCAP_IS2:CUSTOM_4_MASK:ACE_CUSTOM_DATA_0
 */
#define VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_0  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1ec)

/** 
 * \brief
 * Mask for field CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_0.CUSTOM_DATA_0
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_0 must match
 * '1': Entry field CUSTOM_DATA_0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_0 . CUSTOM_DATA_0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_1
 *
 * \details
 * Mask register corresponding to CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_1
 *
 * Register: \a VCAP_IS2:CUSTOM_4_MASK:ACE_CUSTOM_DATA_1
 */
#define VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_1  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1ed)

/** 
 * \brief
 * Mask for field CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_1.CUSTOM_DATA_1
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_1 must match
 * '1': Entry field CUSTOM_DATA_1 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_1 . CUSTOM_DATA_1
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_2
 *
 * \details
 * Mask register corresponding to CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_2
 *
 * Register: \a VCAP_IS2:CUSTOM_4_MASK:ACE_CUSTOM_DATA_2
 */
#define VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_2  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1ee)

/** 
 * \brief
 * Mask for field CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_2.CUSTOM_DATA_2
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_2 must match
 * '1': Entry field CUSTOM_DATA_2 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_2 . CUSTOM_DATA_2
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_3
 *
 * \details
 * Mask register corresponding to CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_3
 *
 * Register: \a VCAP_IS2:CUSTOM_4_MASK:ACE_CUSTOM_DATA_3
 */
#define VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_3  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1ef)

/** 
 * \brief
 * Mask for field CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_3.CUSTOM_DATA_3
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_3 must match
 * '1': Entry field CUSTOM_DATA_3 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_3 . CUSTOM_DATA_3
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_4
 *
 * \details
 * Mask register corresponding to CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_4
 *
 * Register: \a VCAP_IS2:CUSTOM_4_MASK:ACE_CUSTOM_DATA_4
 */
#define VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_4  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1f0)

/** 
 * \brief
 * Mask for field CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_4.CUSTOM_DATA_4
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_4 must match
 * '1': Entry field CUSTOM_DATA_4 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_4 . CUSTOM_DATA_4
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_5
 *
 * \details
 * Mask register corresponding to CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_5
 *
 * Register: \a VCAP_IS2:CUSTOM_4_MASK:ACE_CUSTOM_DATA_5
 */
#define VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_5  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1f1)

/** 
 * \brief
 * Mask for field CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_5.CUSTOM_DATA_5
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_5 must match
 * '1': Entry field CUSTOM_DATA_5 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_5 . CUSTOM_DATA_5
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_6
 *
 * \details
 * Mask register corresponding to CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_6
 *
 * Register: \a VCAP_IS2:CUSTOM_4_MASK:ACE_CUSTOM_DATA_6
 */
#define VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_6  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1f2)

/** 
 * \brief
 * Mask for field CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_6.CUSTOM_DATA_6
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_6 must match
 * '1': Entry field CUSTOM_DATA_6 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_6 . CUSTOM_DATA_6
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_7
 *
 * \details
 * Mask register corresponding to CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_7
 *
 * Register: \a VCAP_IS2:CUSTOM_4_MASK:ACE_CUSTOM_DATA_7
 */
#define VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_7  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1f3)

/** 
 * \brief
 * Mask for field CUSTOM_4_ENTRY:ACE_CUSTOM_DATA_7.CUSTOM_DATA_7
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_7 must match
 * '1': Entry field CUSTOM_DATA_7 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_7 . CUSTOM_DATA_7
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_4_MASK_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VCAP_IS2:CUSTOM_5_MASK
 *
 * Entry mask for entry type CUSTOM_5.
 */


/** 
 * \brief Mask register corresponding to CUSTOM_5_ENTRY:ACE_VLD
 *
 * \details
 * Mask register corresponding to CUSTOM_5_ENTRY:ACE_VLD
 *
 * Register: \a VCAP_IS2:CUSTOM_5_MASK:ACE_VLD
 */
#define VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_VLD  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1f8)

/** 
 * \brief
 * Mask has to be valid always.
 *
 * \details 
 * Must be set to '0'
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_MASK_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to CUSTOM_5_ENTRY:ACE_TYPE
 *
 * \details
 * Mask register corresponding to CUSTOM_5_ENTRY:ACE_TYPE
 *
 * Register: \a VCAP_IS2:CUSTOM_5_MASK:ACE_TYPE
 */
#define VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1f9)

/** 
 * \brief
 * Mask for field CUSTOM_5_ENTRY:ACE_TYPE.VLAN_TAGGED
 *
 * \details 
 * '0': Entry field VLAN_TAGGED must match
 * '1': Entry field VLAN_TAGGED is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Mask for field CUSTOM_5_ENTRY:ACE_TYPE.ISDX_NEQ0
 *
 * \details 
 * '0': Entry field ISDX_NEQ0 must match
 * '1': Entry field ISDX_NEQ0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * Mask for field CUSTOM_5_ENTRY:ACE_TYPE.L3_RT
 *
 * \details 
 * '0': Entry field L3_RT must match
 * '1': Entry field L3_RT is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE . L3_RT
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE_L3_RT  VTSS_BIT(22)

/** 
 * \brief
 * Mask for field CUSTOM_5_ENTRY:ACE_TYPE.L3_SMAC_SIP_MATCH
 *
 * \details 
 * '0': Entry field L3_SMAC_SIP_MATCH must match
 * '1': Entry field L3_SMAC_SIP_MATCH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE . L3_SMAC_SIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE_L3_SMAC_SIP_MATCH  VTSS_BIT(21)

/** 
 * \brief
 * Mask for field CUSTOM_5_ENTRY:ACE_TYPE.L3_DMAC_DIP_MATCH
 *
 * \details 
 * '0': Entry field L3_DMAC_DIP_MATCH must match
 * '1': Entry field L3_DMAC_DIP_MATCH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE . L3_DMAC_DIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE_L3_DMAC_DIP_MATCH  VTSS_BIT(23)

/** 
 * \brief
 * Mask for field CUSTOM_5_ENTRY:ACE_TYPE.L2_FWD
 *
 * \details 
 * '0': Entry field L2_FWD must match
 * '1': Entry field L2_FWD is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * Mask for field CUSTOM_5_ENTRY:ACE_TYPE.VID
 *
 * \details 
 * '0': Entry field VID must match
 * '1': Entry field VID is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * Mask for field CUSTOM_5_ENTRY:ACE_TYPE.UPRIO
 *
 * \details 
 * '0': Entry field UPRIO must match
 * '1': Entry field UPRIO is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * Mask for field CUSTOM_5_ENTRY:ACE_TYPE.CFI
 *
 * \details 
 * '0': Entry field CFI must match
 * '1': Entry field CFI is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Mask for field CUSTOM_5_ENTRY:ACE_TYPE.L2_BC
 *
 * \details 
 * '0': Entry field L2_BC must match
 * '1': Entry field L2_BC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Mask for field CUSTOM_5_ENTRY:ACE_TYPE.L2_MC
 *
 * \details 
 * '0': Entry field L2_MC must match
 * '1': Entry field L2_MC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * Mask for field CUSTOM_5_ENTRY:ACE_TYPE.FIRST
 *
 * \details 
 * '0': Entry field FIRST must match
 * '1': Entry field FIRST is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_MASK_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Mask register corresponding to CUSTOM_5_ENTRY:ACE_IGR_PORT_MASK
 *
 * \details
 * Mask register corresponding to CUSTOM_5_ENTRY:ACE_IGR_PORT_MASK
 *
 * Register: \a VCAP_IS2:CUSTOM_5_MASK:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1fa)


/** 
 * \brief Mask register corresponding to CUSTOM_5_ENTRY:ACE_PAG
 *
 * \details
 * Mask register corresponding to CUSTOM_5_ENTRY:ACE_PAG
 *
 * Register: \a VCAP_IS2:CUSTOM_5_MASK:ACE_PAG
 */
#define VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_PAG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1fb)

/** 
 * \brief
 * Mask for field CUSTOM_5_ENTRY:ACE_PAG.PAG
 *
 * \details 
 * '0': Entry field PAG must match
 * '1': Entry field PAG is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_MASK_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_CUSTOM_5_MASK_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_CUSTOM_5_MASK_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Mask register corresponding to CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_0
 *
 * \details
 * Mask register corresponding to CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_0
 *
 * Register: \a VCAP_IS2:CUSTOM_5_MASK:ACE_CUSTOM_DATA_0
 */
#define VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_0  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1fc)

/** 
 * \brief
 * Mask for field CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_0.CUSTOM_DATA_0
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_0 must match
 * '1': Entry field CUSTOM_DATA_0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_0 . CUSTOM_DATA_0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_1
 *
 * \details
 * Mask register corresponding to CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_1
 *
 * Register: \a VCAP_IS2:CUSTOM_5_MASK:ACE_CUSTOM_DATA_1
 */
#define VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_1  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1fd)

/** 
 * \brief
 * Mask for field CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_1.CUSTOM_DATA_1
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_1 must match
 * '1': Entry field CUSTOM_DATA_1 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_1 . CUSTOM_DATA_1
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_2
 *
 * \details
 * Mask register corresponding to CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_2
 *
 * Register: \a VCAP_IS2:CUSTOM_5_MASK:ACE_CUSTOM_DATA_2
 */
#define VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_2  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1fe)

/** 
 * \brief
 * Mask for field CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_2.CUSTOM_DATA_2
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_2 must match
 * '1': Entry field CUSTOM_DATA_2 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_2 . CUSTOM_DATA_2
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_3
 *
 * \details
 * Mask register corresponding to CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_3
 *
 * Register: \a VCAP_IS2:CUSTOM_5_MASK:ACE_CUSTOM_DATA_3
 */
#define VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_3  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x1ff)

/** 
 * \brief
 * Mask for field CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_3.CUSTOM_DATA_3
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_3 must match
 * '1': Entry field CUSTOM_DATA_3 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_3 . CUSTOM_DATA_3
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_4
 *
 * \details
 * Mask register corresponding to CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_4
 *
 * Register: \a VCAP_IS2:CUSTOM_5_MASK:ACE_CUSTOM_DATA_4
 */
#define VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_4  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x200)

/** 
 * \brief
 * Mask for field CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_4.CUSTOM_DATA_4
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_4 must match
 * '1': Entry field CUSTOM_DATA_4 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_4 . CUSTOM_DATA_4
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_5
 *
 * \details
 * Mask register corresponding to CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_5
 *
 * Register: \a VCAP_IS2:CUSTOM_5_MASK:ACE_CUSTOM_DATA_5
 */
#define VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_5  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x201)

/** 
 * \brief
 * Mask for field CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_5.CUSTOM_DATA_5
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_5 must match
 * '1': Entry field CUSTOM_DATA_5 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_5 . CUSTOM_DATA_5
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_6
 *
 * \details
 * Mask register corresponding to CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_6
 *
 * Register: \a VCAP_IS2:CUSTOM_5_MASK:ACE_CUSTOM_DATA_6
 */
#define VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_6  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x202)

/** 
 * \brief
 * Mask for field CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_6.CUSTOM_DATA_6
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_6 must match
 * '1': Entry field CUSTOM_DATA_6 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_6 . CUSTOM_DATA_6
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_7
 *
 * \details
 * Mask register corresponding to CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_7
 *
 * Register: \a VCAP_IS2:CUSTOM_5_MASK:ACE_CUSTOM_DATA_7
 */
#define VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_7  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x203)

/** 
 * \brief
 * Mask for field CUSTOM_5_ENTRY:ACE_CUSTOM_DATA_7.CUSTOM_DATA_7
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_7 must match
 * '1': Entry field CUSTOM_DATA_7 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_7 . CUSTOM_DATA_7
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_5_MASK_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VCAP_IS2:CUSTOM_6_MASK
 *
 * Entry mask for entry type CUSTOM_6.
 */


/** 
 * \brief Mask register corresponding to CUSTOM_6_ENTRY:ACE_VLD
 *
 * \details
 * Mask register corresponding to CUSTOM_6_ENTRY:ACE_VLD
 *
 * Register: \a VCAP_IS2:CUSTOM_6_MASK:ACE_VLD
 */
#define VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_VLD  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x208)

/** 
 * \brief
 * Mask has to be valid always.
 *
 * \details 
 * Must be set to '0'
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_MASK_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to CUSTOM_6_ENTRY:ACE_TYPE
 *
 * \details
 * Mask register corresponding to CUSTOM_6_ENTRY:ACE_TYPE
 *
 * Register: \a VCAP_IS2:CUSTOM_6_MASK:ACE_TYPE
 */
#define VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x209)

/** 
 * \brief
 * Mask for field CUSTOM_6_ENTRY:ACE_TYPE.VLAN_TAGGED
 *
 * \details 
 * '0': Entry field VLAN_TAGGED must match
 * '1': Entry field VLAN_TAGGED is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Mask for field CUSTOM_6_ENTRY:ACE_TYPE.ISDX_NEQ0
 *
 * \details 
 * '0': Entry field ISDX_NEQ0 must match
 * '1': Entry field ISDX_NEQ0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * Mask for field CUSTOM_6_ENTRY:ACE_TYPE.L3_RT
 *
 * \details 
 * '0': Entry field L3_RT must match
 * '1': Entry field L3_RT is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE . L3_RT
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE_L3_RT  VTSS_BIT(22)

/** 
 * \brief
 * Mask for field CUSTOM_6_ENTRY:ACE_TYPE.L3_SMAC_SIP_MATCH
 *
 * \details 
 * '0': Entry field L3_SMAC_SIP_MATCH must match
 * '1': Entry field L3_SMAC_SIP_MATCH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE . L3_SMAC_SIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE_L3_SMAC_SIP_MATCH  VTSS_BIT(21)

/** 
 * \brief
 * Mask for field CUSTOM_6_ENTRY:ACE_TYPE.L3_DMAC_DIP_MATCH
 *
 * \details 
 * '0': Entry field L3_DMAC_DIP_MATCH must match
 * '1': Entry field L3_DMAC_DIP_MATCH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE . L3_DMAC_DIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE_L3_DMAC_DIP_MATCH  VTSS_BIT(23)

/** 
 * \brief
 * Mask for field CUSTOM_6_ENTRY:ACE_TYPE.L2_FWD
 *
 * \details 
 * '0': Entry field L2_FWD must match
 * '1': Entry field L2_FWD is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * Mask for field CUSTOM_6_ENTRY:ACE_TYPE.VID
 *
 * \details 
 * '0': Entry field VID must match
 * '1': Entry field VID is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * Mask for field CUSTOM_6_ENTRY:ACE_TYPE.UPRIO
 *
 * \details 
 * '0': Entry field UPRIO must match
 * '1': Entry field UPRIO is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * Mask for field CUSTOM_6_ENTRY:ACE_TYPE.CFI
 *
 * \details 
 * '0': Entry field CFI must match
 * '1': Entry field CFI is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Mask for field CUSTOM_6_ENTRY:ACE_TYPE.L2_BC
 *
 * \details 
 * '0': Entry field L2_BC must match
 * '1': Entry field L2_BC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Mask for field CUSTOM_6_ENTRY:ACE_TYPE.L2_MC
 *
 * \details 
 * '0': Entry field L2_MC must match
 * '1': Entry field L2_MC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * Mask for field CUSTOM_6_ENTRY:ACE_TYPE.FIRST
 *
 * \details 
 * '0': Entry field FIRST must match
 * '1': Entry field FIRST is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_MASK_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Mask register corresponding to CUSTOM_6_ENTRY:ACE_IGR_PORT_MASK
 *
 * \details
 * Mask register corresponding to CUSTOM_6_ENTRY:ACE_IGR_PORT_MASK
 *
 * Register: \a VCAP_IS2:CUSTOM_6_MASK:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x20a)


/** 
 * \brief Mask register corresponding to CUSTOM_6_ENTRY:ACE_PAG
 *
 * \details
 * Mask register corresponding to CUSTOM_6_ENTRY:ACE_PAG
 *
 * Register: \a VCAP_IS2:CUSTOM_6_MASK:ACE_PAG
 */
#define VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_PAG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x20b)

/** 
 * \brief
 * Mask for field CUSTOM_6_ENTRY:ACE_PAG.PAG
 *
 * \details 
 * '0': Entry field PAG must match
 * '1': Entry field PAG is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_MASK_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_CUSTOM_6_MASK_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_CUSTOM_6_MASK_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Mask register corresponding to CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_0
 *
 * \details
 * Mask register corresponding to CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_0
 *
 * Register: \a VCAP_IS2:CUSTOM_6_MASK:ACE_CUSTOM_DATA_0
 */
#define VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_0  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x20c)

/** 
 * \brief
 * Mask for field CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_0.CUSTOM_DATA_0
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_0 must match
 * '1': Entry field CUSTOM_DATA_0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_0 . CUSTOM_DATA_0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_1
 *
 * \details
 * Mask register corresponding to CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_1
 *
 * Register: \a VCAP_IS2:CUSTOM_6_MASK:ACE_CUSTOM_DATA_1
 */
#define VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_1  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x20d)

/** 
 * \brief
 * Mask for field CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_1.CUSTOM_DATA_1
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_1 must match
 * '1': Entry field CUSTOM_DATA_1 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_1 . CUSTOM_DATA_1
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_2
 *
 * \details
 * Mask register corresponding to CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_2
 *
 * Register: \a VCAP_IS2:CUSTOM_6_MASK:ACE_CUSTOM_DATA_2
 */
#define VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_2  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x20e)

/** 
 * \brief
 * Mask for field CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_2.CUSTOM_DATA_2
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_2 must match
 * '1': Entry field CUSTOM_DATA_2 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_2 . CUSTOM_DATA_2
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_3
 *
 * \details
 * Mask register corresponding to CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_3
 *
 * Register: \a VCAP_IS2:CUSTOM_6_MASK:ACE_CUSTOM_DATA_3
 */
#define VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_3  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x20f)

/** 
 * \brief
 * Mask for field CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_3.CUSTOM_DATA_3
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_3 must match
 * '1': Entry field CUSTOM_DATA_3 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_3 . CUSTOM_DATA_3
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_4
 *
 * \details
 * Mask register corresponding to CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_4
 *
 * Register: \a VCAP_IS2:CUSTOM_6_MASK:ACE_CUSTOM_DATA_4
 */
#define VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_4  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x210)

/** 
 * \brief
 * Mask for field CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_4.CUSTOM_DATA_4
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_4 must match
 * '1': Entry field CUSTOM_DATA_4 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_4 . CUSTOM_DATA_4
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_5
 *
 * \details
 * Mask register corresponding to CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_5
 *
 * Register: \a VCAP_IS2:CUSTOM_6_MASK:ACE_CUSTOM_DATA_5
 */
#define VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_5  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x211)

/** 
 * \brief
 * Mask for field CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_5.CUSTOM_DATA_5
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_5 must match
 * '1': Entry field CUSTOM_DATA_5 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_5 . CUSTOM_DATA_5
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_6
 *
 * \details
 * Mask register corresponding to CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_6
 *
 * Register: \a VCAP_IS2:CUSTOM_6_MASK:ACE_CUSTOM_DATA_6
 */
#define VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_6  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x212)

/** 
 * \brief
 * Mask for field CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_6.CUSTOM_DATA_6
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_6 must match
 * '1': Entry field CUSTOM_DATA_6 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_6 . CUSTOM_DATA_6
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_7
 *
 * \details
 * Mask register corresponding to CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_7
 *
 * Register: \a VCAP_IS2:CUSTOM_6_MASK:ACE_CUSTOM_DATA_7
 */
#define VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_7  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x213)

/** 
 * \brief
 * Mask for field CUSTOM_6_ENTRY:ACE_CUSTOM_DATA_7.CUSTOM_DATA_7
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_7 must match
 * '1': Entry field CUSTOM_DATA_7 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_7 . CUSTOM_DATA_7
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_6_MASK_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VCAP_IS2:CUSTOM_7_MASK
 *
 * Entry mask for entry type CUSTOM_7.
 */


/** 
 * \brief Mask register corresponding to CUSTOM_7_ENTRY:ACE_VLD
 *
 * \details
 * Mask register corresponding to CUSTOM_7_ENTRY:ACE_VLD
 *
 * Register: \a VCAP_IS2:CUSTOM_7_MASK:ACE_VLD
 */
#define VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_VLD  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x218)

/** 
 * \brief
 * Mask has to be valid always.
 *
 * \details 
 * Must be set to '0'
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_VLD . VLD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_MASK_ACE_VLD_VLD  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to CUSTOM_7_ENTRY:ACE_TYPE
 *
 * \details
 * Mask register corresponding to CUSTOM_7_ENTRY:ACE_TYPE
 *
 * Register: \a VCAP_IS2:CUSTOM_7_MASK:ACE_TYPE
 */
#define VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x219)

/** 
 * \brief
 * Mask for field CUSTOM_7_ENTRY:ACE_TYPE.VLAN_TAGGED
 *
 * \details 
 * '0': Entry field VLAN_TAGGED must match
 * '1': Entry field VLAN_TAGGED is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE_VLAN_TAGGED  VTSS_BIT(31)

/** 
 * \brief
 * Mask for field CUSTOM_7_ENTRY:ACE_TYPE.ISDX_NEQ0
 *
 * \details 
 * '0': Entry field ISDX_NEQ0 must match
 * '1': Entry field ISDX_NEQ0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE_ISDX_NEQ0  VTSS_BIT(30)

/** 
 * \brief
 * Mask for field CUSTOM_7_ENTRY:ACE_TYPE.L3_RT
 *
 * \details 
 * '0': Entry field L3_RT must match
 * '1': Entry field L3_RT is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE . L3_RT
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE_L3_RT  VTSS_BIT(22)

/** 
 * \brief
 * Mask for field CUSTOM_7_ENTRY:ACE_TYPE.L3_SMAC_SIP_MATCH
 *
 * \details 
 * '0': Entry field L3_SMAC_SIP_MATCH must match
 * '1': Entry field L3_SMAC_SIP_MATCH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE . L3_SMAC_SIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE_L3_SMAC_SIP_MATCH  VTSS_BIT(21)

/** 
 * \brief
 * Mask for field CUSTOM_7_ENTRY:ACE_TYPE.L3_DMAC_DIP_MATCH
 *
 * \details 
 * '0': Entry field L3_DMAC_DIP_MATCH must match
 * '1': Entry field L3_DMAC_DIP_MATCH is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE . L3_DMAC_DIP_MATCH
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE_L3_DMAC_DIP_MATCH  VTSS_BIT(23)

/** 
 * \brief
 * Mask for field CUSTOM_7_ENTRY:ACE_TYPE.L2_FWD
 *
 * \details 
 * '0': Entry field L2_FWD must match
 * '1': Entry field L2_FWD is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE . L2_FWD
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE_L2_FWD  VTSS_BIT(20)

/** 
 * \brief
 * Mask for field CUSTOM_7_ENTRY:ACE_TYPE.VID
 *
 * \details 
 * '0': Entry field VID must match
 * '1': Entry field VID is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE . VID
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * Mask for field CUSTOM_7_ENTRY:ACE_TYPE.UPRIO
 *
 * \details 
 * '0': Entry field UPRIO must match
 * '1': Entry field UPRIO is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE . UPRIO
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE_UPRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * Mask for field CUSTOM_7_ENTRY:ACE_TYPE.CFI
 *
 * \details 
 * '0': Entry field CFI must match
 * '1': Entry field CFI is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE . CFI
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE_CFI  VTSS_BIT(4)

/** 
 * \brief
 * Mask for field CUSTOM_7_ENTRY:ACE_TYPE.L2_BC
 *
 * \details 
 * '0': Entry field L2_BC must match
 * '1': Entry field L2_BC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE . L2_BC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE_L2_BC  VTSS_BIT(3)

/** 
 * \brief
 * Mask for field CUSTOM_7_ENTRY:ACE_TYPE.L2_MC
 *
 * \details 
 * '0': Entry field L2_MC must match
 * '1': Entry field L2_MC is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE . L2_MC
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE_L2_MC  VTSS_BIT(2)

/** 
 * \brief
 * Mask for field CUSTOM_7_ENTRY:ACE_TYPE.FIRST
 *
 * \details 
 * '0': Entry field FIRST must match
 * '1': Entry field FIRST is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE . FIRST
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_MASK_ACE_TYPE_FIRST  VTSS_BIT(1)


/** 
 * \brief Mask register corresponding to CUSTOM_7_ENTRY:ACE_IGR_PORT_MASK
 *
 * \details
 * Mask register corresponding to CUSTOM_7_ENTRY:ACE_IGR_PORT_MASK
 *
 * Register: \a VCAP_IS2:CUSTOM_7_MASK:ACE_IGR_PORT_MASK
 */
#define VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_IGR_PORT_MASK  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x21a)


/** 
 * \brief Mask register corresponding to CUSTOM_7_ENTRY:ACE_PAG
 *
 * \details
 * Mask register corresponding to CUSTOM_7_ENTRY:ACE_PAG
 *
 * Register: \a VCAP_IS2:CUSTOM_7_MASK:ACE_PAG
 */
#define VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_PAG  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x21b)

/** 
 * \brief
 * Mask for field CUSTOM_7_ENTRY:ACE_PAG.PAG
 *
 * \details 
 * '0': Entry field PAG must match
 * '1': Entry field PAG is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_PAG . PAG
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_MASK_ACE_PAG_PAG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS2_CUSTOM_7_MASK_ACE_PAG_PAG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS2_CUSTOM_7_MASK_ACE_PAG_PAG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Mask register corresponding to CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_0
 *
 * \details
 * Mask register corresponding to CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_0
 *
 * Register: \a VCAP_IS2:CUSTOM_7_MASK:ACE_CUSTOM_DATA_0
 */
#define VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_0  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x21c)

/** 
 * \brief
 * Mask for field CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_0.CUSTOM_DATA_0
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_0 must match
 * '1': Entry field CUSTOM_DATA_0 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_0 . CUSTOM_DATA_0
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_0_CUSTOM_DATA_0(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_1
 *
 * \details
 * Mask register corresponding to CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_1
 *
 * Register: \a VCAP_IS2:CUSTOM_7_MASK:ACE_CUSTOM_DATA_1
 */
#define VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_1  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x21d)

/** 
 * \brief
 * Mask for field CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_1.CUSTOM_DATA_1
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_1 must match
 * '1': Entry field CUSTOM_DATA_1 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_1 . CUSTOM_DATA_1
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_1_CUSTOM_DATA_1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_2
 *
 * \details
 * Mask register corresponding to CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_2
 *
 * Register: \a VCAP_IS2:CUSTOM_7_MASK:ACE_CUSTOM_DATA_2
 */
#define VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_2  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x21e)

/** 
 * \brief
 * Mask for field CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_2.CUSTOM_DATA_2
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_2 must match
 * '1': Entry field CUSTOM_DATA_2 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_2 . CUSTOM_DATA_2
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_2_CUSTOM_DATA_2(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_3
 *
 * \details
 * Mask register corresponding to CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_3
 *
 * Register: \a VCAP_IS2:CUSTOM_7_MASK:ACE_CUSTOM_DATA_3
 */
#define VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_3  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x21f)

/** 
 * \brief
 * Mask for field CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_3.CUSTOM_DATA_3
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_3 must match
 * '1': Entry field CUSTOM_DATA_3 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_3 . CUSTOM_DATA_3
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_3_CUSTOM_DATA_3(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_4
 *
 * \details
 * Mask register corresponding to CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_4
 *
 * Register: \a VCAP_IS2:CUSTOM_7_MASK:ACE_CUSTOM_DATA_4
 */
#define VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_4  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x220)

/** 
 * \brief
 * Mask for field CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_4.CUSTOM_DATA_4
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_4 must match
 * '1': Entry field CUSTOM_DATA_4 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_4 . CUSTOM_DATA_4
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_4_CUSTOM_DATA_4(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_5
 *
 * \details
 * Mask register corresponding to CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_5
 *
 * Register: \a VCAP_IS2:CUSTOM_7_MASK:ACE_CUSTOM_DATA_5
 */
#define VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_5  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x221)

/** 
 * \brief
 * Mask for field CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_5.CUSTOM_DATA_5
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_5 must match
 * '1': Entry field CUSTOM_DATA_5 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_5 . CUSTOM_DATA_5
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_5_CUSTOM_DATA_5(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_6
 *
 * \details
 * Mask register corresponding to CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_6
 *
 * Register: \a VCAP_IS2:CUSTOM_7_MASK:ACE_CUSTOM_DATA_6
 */
#define VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_6  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x222)

/** 
 * \brief
 * Mask for field CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_6.CUSTOM_DATA_6
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_6 must match
 * '1': Entry field CUSTOM_DATA_6 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_6 . CUSTOM_DATA_6
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_6_CUSTOM_DATA_6(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask register corresponding to CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_7
 *
 * \details
 * Mask register corresponding to CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_7
 *
 * Register: \a VCAP_IS2:CUSTOM_7_MASK:ACE_CUSTOM_DATA_7
 */
#define VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_7  VTSS_IOREG(VTSS_TO_VCAP_IS2,0x223)

/** 
 * \brief
 * Mask for field CUSTOM_7_ENTRY:ACE_CUSTOM_DATA_7.CUSTOM_DATA_7
 *
 * \details 
 * '0': Entry field CUSTOM_DATA_7 must match
 * '1': Entry field CUSTOM_DATA_7 is ignored
 *
 * Field: ::VTSS_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_7 . CUSTOM_DATA_7
 */
#define  VTSS_F_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_IS2_CUSTOM_7_MASK_ACE_CUSTOM_DATA_7_CUSTOM_DATA_7(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VCAP_IS2:BASETYPE_ACTION
 *
 * VCAP IS2 Action Configuration
 */


/** 
 * \details
 * Register: \a VCAP_IS2:BASETYPE_ACTION:A
 */
#define VTSS_VCAP_IS2_BASETYPE_ACTION_A      VTSS_IOREG(VTSS_TO_VCAP_IS2,0x228)

/** 
 * \brief
 * Select latch of timestamp+frame ID latch in Tx port device.
 * 
 * When a latch is selected, the transmission time as well as an ID of the
 * transmitted frame is stored in the latch in the Tx port device. When the
 * CPU later processes the copy of the frame, it can then read the
 * transmission timestamp of the frame and use the ID to verify that the
 * timestamp does belong to the frame being processed.
 * 
 * Three sets of latches are available in each port device to better
 * support multiple PTP flows per physical port.
 * 
 * This latch registers are:
 * DEV1G::DEV_PTP_TX_TSTAMP and DEV1G::DEV_PTP_TX_ID
 * DEV10G::DEV_PTP_TX_TSTAMP and DEV10G::DEV_PTP_TX_ID
 *
 * \details 
 * 0: No latch
 * 1: Latch register 0
 * 2: Latch register 1
 * 3: Latch register 2
 *
 * Field: ::VTSS_VCAP_IS2_BASETYPE_ACTION_A . OAM_TX_TIMESTAMP_SEL
 */
#define  VTSS_F_VCAP_IS2_BASETYPE_ACTION_A_OAM_TX_TIMESTAMP_SEL(x)  VTSS_ENCODE_BITFIELD(x,24,2)
#define  VTSS_M_VCAP_IS2_BASETYPE_ACTION_A_OAM_TX_TIMESTAMP_SEL     VTSS_ENCODE_BITMASK(24,2)
#define  VTSS_X_VCAP_IS2_BASETYPE_ACTION_A_OAM_TX_TIMESTAMP_SEL(x)  VTSS_EXTRACT_BITFIELD(x,24,2)

/** 
 * \brief
 * Enable Rx timestamp in frame sent to CPU.
 * 
 * The Rx timestamp can be written into the frame in different ways. This
 * must be configured in these registers:
 * ANA_AC:PS_COMMON:MISC_CTRL.OAM_RX_TSTAMP_IN_FCS_ENA or 
 * ANA_AC:PS_COMMON:MISC_CTRL.OAM_RX_TSTAMP_FRM_EXTEND_ENA
 *
 * \details 
 * 0: No stamping
 * 1: Stamp Rx timestamp into frame.
 *
 * Field: ::VTSS_VCAP_IS2_BASETYPE_ACTION_A . OAM_RX_TIMESTAMP_ENA
 */
#define  VTSS_F_VCAP_IS2_BASETYPE_ACTION_A_OAM_RX_TIMESTAMP_ENA  VTSS_BIT(23)

/** 
 * \brief
 * Controls loopback of frames, i.e. forwarding back to the port, where the
 * frame was received.
 * Frames are unmodified when looped back.

 *
 * \details 
 * 0: Disable loopback
 * 1: Enable loopback
 *
 * Field: ::VTSS_VCAP_IS2_BASETYPE_ACTION_A . LOOPBACK_SEL
 */
#define  VTSS_F_VCAP_IS2_BASETYPE_ACTION_A_LOOPBACK_SEL  VTSS_BIT(22)

/** 
 * \brief
 * Selects OAM CCM checking in ANA_L2.
 * 
 * The corresponding CCM entry must be configured in:
 * Link CCM: ANA_L2::PORT_CCM_CTRL
 * Path CCM: LRN::MAC_ACCESS_CFG_4
 * Service CCM: ANA_L2::CCM_CTRL
 *
 * \details 
 * 0: No CCM checking
 * 1: Link CCM checking (based on port id)
 * 2: Path CCM checking (based on conid)
 * 3: Service CCM checking (based on ISDX_VAL)
 *
 * Field: ::VTSS_VCAP_IS2_BASETYPE_ACTION_A . OAM_CCM_TYPE
 */
#define  VTSS_F_VCAP_IS2_BASETYPE_ACTION_A_OAM_CCM_TYPE(x)  VTSS_ENCODE_BITFIELD(x,20,2)
#define  VTSS_M_VCAP_IS2_BASETYPE_ACTION_A_OAM_CCM_TYPE     VTSS_ENCODE_BITMASK(20,2)
#define  VTSS_X_VCAP_IS2_BASETYPE_ACTION_A_OAM_CCM_TYPE(x)  VTSS_EXTRACT_BITFIELD(x,20,2)

/** 
 * \brief
 * Mirror frame as if it had hit a mirror probe.
 * If a given frame hits multiple mirror probes, then the highest probe
 * number "wins".
 * 
 * Mirror probes are configured in ANA_AC:MIRROR_PROBE.
 * 
 * The mirror port, to which frames of a mirror probe are forwarded, is
 * configured in ARB:CFG_STATUS:MIRROR_CFG.
 *
 * \details 
 * 0: No mirroring
 * 1: Mirror probe 0
 * 2: Mirror probe 1
 * 3: Mirror probe 2
 *
 * Field: ::VTSS_VCAP_IS2_BASETYPE_ACTION_A . MIRROR_PROBE
 */
#define  VTSS_F_VCAP_IS2_BASETYPE_ACTION_A_MIRROR_PROBE(x)  VTSS_ENCODE_BITFIELD(x,18,2)
#define  VTSS_M_VCAP_IS2_BASETYPE_ACTION_A_MIRROR_PROBE     VTSS_ENCODE_BITMASK(18,2)
#define  VTSS_X_VCAP_IS2_BASETYPE_ACTION_A_MIRROR_PROBE(x)  VTSS_EXTRACT_BITFIELD(x,18,2)

/** 
 * \brief
 * Offset added to the DLB policer index from the ISDX table. 
 * See ANA_L2:ISDX:DLB_CFG.DLB_IDX.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_BASETYPE_ACTION_A . DLB_OFFSET
 */
#define  VTSS_F_VCAP_IS2_BASETYPE_ACTION_A_DLB_OFFSET(x)  VTSS_ENCODE_BITFIELD(x,15,3)
#define  VTSS_M_VCAP_IS2_BASETYPE_ACTION_A_DLB_OFFSET     VTSS_ENCODE_BITMASK(15,3)
#define  VTSS_X_VCAP_IS2_BASETYPE_ACTION_A_DLB_OFFSET(x)  VTSS_EXTRACT_BITFIELD(x,15,3)

/** 
 * \brief
 * Configure which policer to use when policing frames hitting this action.
 * This field is applicable only when policing is enabled (see POLICE_ENA).
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_BASETYPE_ACTION_A . POLICE_IDX
 */
#define  VTSS_F_VCAP_IS2_BASETYPE_ACTION_A_POLICE_IDX(x)  VTSS_ENCODE_BITFIELD(x,10,5)
#define  VTSS_M_VCAP_IS2_BASETYPE_ACTION_A_POLICE_IDX     VTSS_ENCODE_BITMASK(10,5)
#define  VTSS_X_VCAP_IS2_BASETYPE_ACTION_A_POLICE_IDX(x)  VTSS_EXTRACT_BITFIELD(x,10,5)

/** 
 * \brief
 * Set to enable policing of frames hitting this action. 
 * When policing is enabled frames are policed by the policer specified in
 * POLICE_IDX.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_BASETYPE_ACTION_A . POLICE_ENA
 */
#define  VTSS_F_VCAP_IS2_BASETYPE_ACTION_A_POLICE_ENA  VTSS_BIT(9)

/** 
 * \brief
 * Set to enable/disable routing of frames hitting this action.

 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_VCAP_IS2_BASETYPE_ACTION_A . RT_ENA
 */
#define  VTSS_F_VCAP_IS2_BASETYPE_ACTION_A_RT_ENA  VTSS_BIT(8)

/** 
 * \brief
 * Set to enable/disable learning of frames hitting this action.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_VCAP_IS2_BASETYPE_ACTION_A . LRN_ENA
 */
#define  VTSS_F_VCAP_IS2_BASETYPE_ACTION_A_LRN_ENA  VTSS_BIT(7)

/** 
 * \brief
 * Set to enable/disable forwarding of frames hitting this action.
 * 
 * Note that - unlike other VCAP_IS2 actions - if first lookup in VCAP_IS2
 * results in dropping frame, then this cannot be changed by the second
 * lookup.
 *
 * \details 
 * 0: Drop frame
 * 1: Allow forwarding of frame.
 *
 * Field: ::VTSS_VCAP_IS2_BASETYPE_ACTION_A . FW_ENA
 */
#define  VTSS_F_VCAP_IS2_BASETYPE_ACTION_A_FW_ENA  VTSS_BIT(6)

/** 
 * \brief
 * If set an interrrupt is triggered when this rule is hit.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_BASETYPE_ACTION_A . IRQ_TRIGGER
 */
#define  VTSS_F_VCAP_IS2_BASETYPE_ACTION_A_IRQ_TRIGGER  VTSS_BIT(5)

/** 
 * \brief
 * This field determines the CPU extraction queue that is used when a frame
 * is copied to the CPU due to a HIT_ME_ONCE or CPU_COPY_ENA action.
 *
 * \details 
 * 0x0: Frames copied to the CPU are passed to CPU extraction queue 0.
 * 0x1: Frames copied to the CPU are passed to CPU extraction queue 1.
 * 0x2: Frames copied to the CPU are passed to CPU extraction queue 2.
 * ...
 *
 * Field: ::VTSS_VCAP_IS2_BASETYPE_ACTION_A . CPU_QU_NUM
 */
#define  VTSS_F_VCAP_IS2_BASETYPE_ACTION_A_CPU_QU_NUM(x)  VTSS_ENCODE_BITFIELD(x,2,3)
#define  VTSS_M_VCAP_IS2_BASETYPE_ACTION_A_CPU_QU_NUM     VTSS_ENCODE_BITMASK(2,3)
#define  VTSS_X_VCAP_IS2_BASETYPE_ACTION_A_CPU_QU_NUM(x)  VTSS_EXTRACT_BITFIELD(x,2,3)

/** 
 * \brief
 * This field can be used to cause frames that hit this action to be copied
 * to the CPU extraction queue specified in CPU_QU_NUM.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_BASETYPE_ACTION_A . CPU_COPY_ENA
 */
#define  VTSS_F_VCAP_IS2_BASETYPE_ACTION_A_CPU_COPY_ENA  VTSS_BIT(1)

/** 
 * \brief
 * This field can be used make the first frame that hits this action to be
 * copied to the CPU extraction queue specified in CPU_QU_NUM. The bit is
 * then cleared, and any frames that later hit this action are not copied
 * to the CPU.

 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_BASETYPE_ACTION_A . HIT_ME_ONCE
 */
#define  VTSS_F_VCAP_IS2_BASETYPE_ACTION_A_HIT_ME_ONCE  VTSS_BIT(0)


/** 
 * \brief Redirection Action Configuration
 *
 * \details
 * Register: \a VCAP_IS2:BASETYPE_ACTION:B
 */
#define VTSS_VCAP_IS2_BASETYPE_ACTION_B      VTSS_IOREG(VTSS_TO_VCAP_IS2,0x229)

/** 
 * \brief
 * If REDIR_ENA is set, then this field determines the destination port
 * mask that must be used when redirecting a frame.
 * 
 * If REDIR_ENA is 0, then non-zero values of this field can be used to
 * activate various advanced actions.
 * This is described in the encoding below.
 *
 * \details 
 * REDIR_ENA=1:
 * Redirect to ANA_AC:PGID[REDIR_PGID].PGID_CFG_0.PORT_MASK.
 * 
 * ---------
 * ADVANCED ACTIONS: REDIR_ENA=0 and REDIR_PGID(10)=0:
 * 
 * REDIR_PGID(1:0): #Handle ISDX
 *   1: isdx = isdx + 1  # incr isdx 
 *   2: isdx = 0	 # Clear isdx 
 * 
 * REDIR_PGID(2): #Ignore source mask
 *   0: No change
 *   1: Ignore Source mask
 * 
 * REDIR_PGID(3): Force use of source independent multicast (using ACL S2
 * redir bit) even though MAC table is setup to use IPv4 source specific
 * multicasting
 * 
 * ---------
 * ADVANCED ACTIONS: REDIR_ENA=0 and REDIR_PGID(10)=1:
 * 
 * REDIR_PGID(1:0): #CVLAN MASK handle
 *   0: vlan_mask = vlan_mask AND PORT_MASK # reduce 
 *   1: vlan_mask = vlan_mask  # keep 
 *   2: vlan_mask = PORT_MASK  # replace 
 *   3: vlan_mask = 0	       # remove all ports 
 * 
 * REDIR_PGID(3:2): #BVLAN MASK handle
 *   0: bvlan_mask = bvlan_mask AND PORT_MASK # reduce 
 *   1: bvlan_mask = vlan_mask	# keep 
 *   2: bvlan_mask = PORT_MASK	# replace 
 *   3: bvlan_mask = 0		# remove all ports 
 * 
 * REDIR_PGID(5:4): #Change port type
 *   0: No change
 *   1: igr_port_type = Force to B-PORT 
 *   2: igr_port_type = Force to C-PORT 
 * 
 * REDIR_PGID(6): #Change BVID
 *   0: No change
 *   1: Force BVID to 0xfff
 * 
 * REDIR_PGID(7): #BVID LRN disable
 *   0: No change
 *   1: Force bvlan_lrn_ena to 0
 * 
 * REDIR_PGID(8): #Remove itag avail
 *   0: No change
 *   1: Clear itag_avail # Do not allow inner MAC operation
 * 
 * REDIR_PGID(9): #Send additional copy to virtual device
 *   0: No change
 *   1: Additional virtual device copy

 *
 * Field: ::VTSS_VCAP_IS2_BASETYPE_ACTION_B . REDIR_PGID
 */
#define  VTSS_F_VCAP_IS2_BASETYPE_ACTION_B_REDIR_PGID(x)  VTSS_ENCODE_BITFIELD(x,4,11)
#define  VTSS_M_VCAP_IS2_BASETYPE_ACTION_B_REDIR_PGID     VTSS_ENCODE_BITMASK(4,11)
#define  VTSS_X_VCAP_IS2_BASETYPE_ACTION_B_REDIR_PGID(x)  VTSS_EXTRACT_BITFIELD(x,4,11)

/** 
 * \brief
 * This field can be used to enable redirection to the ports specified in
 * ANA_AC:PGID[REDIR_PGID].PGID_CFG_0.PORT_MASK.
 * Frames can be redirected even though normal forwarding is disabled
 * (FWD_ENA=0).
 * 
 * If REDIR_ENA is set to 0 to disable this functionality, then REDIR_PGID
 * must also be set to 0.
 * 
 * Various advanced actions can be achieved by setting REDIR_ENA=0 and
 * REDIR_PGID!=0.
 * This is described in the encoding of REDIR_PGID.
 *
 * \details 
 * 0: Frames that hit this action are not redirected.
 * 1: Frames that hit this action are redirected according to the PGID
 * configuration.
 *
 * Field: ::VTSS_VCAP_IS2_BASETYPE_ACTION_B . REDIR_ENA
 */
#define  VTSS_F_VCAP_IS2_BASETYPE_ACTION_B_REDIR_ENA  VTSS_BIT(0)


/** 
 * \details
 * Register: \a VCAP_IS2:BASETYPE_ACTION:C
 */
#define VTSS_VCAP_IS2_BASETYPE_ACTION_C      VTSS_IOREG(VTSS_TO_VCAP_IS2,0x22a)


/** 
 * \brief Action Hit Counter
 *
 * \details
 * Register: \a VCAP_IS2:BASETYPE_ACTION:D
 */
#define VTSS_VCAP_IS2_BASETYPE_ACTION_D      VTSS_IOREG(VTSS_TO_VCAP_IS2,0x22b)

/**
 * Register Group: \a VCAP_IS2:TCAM_BIST
 *
 * Build in test for TCAM
 */


/** 
 * \brief Control of the TCAM
 *
 * \details
 * Register: \a VCAP_IS2:TCAM_BIST:TCAM_CTRL
 */
#define VTSS_VCAP_IS2_TCAM_BIST_TCAM_CTRL    VTSS_IOREG(VTSS_TO_VCAP_IS2,0x22c)

/** 
 * \brief
 * Set this field to start manual BIST of the TCAM. This field will be
 * cleared once BIST is complete. The BIST procedure requires that the TCAM
 * is initialized before start, setting TCAM_INIT at the same time as
 * setting this field will first initalize the TCAM and then run BIST.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_TCAM_BIST_TCAM_CTRL . TCAM_BIST
 */
#define  VTSS_F_VCAP_IS2_TCAM_BIST_TCAM_CTRL_TCAM_BIST  VTSS_BIT(1)

/** 
 * \brief
 * Set this field to start manual initialization of the TCAM. This field is
 * cleared once initialization is complete.
 * The TCAM has random contents after reset and must be initialized prior
 * to usage.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_TCAM_BIST_TCAM_CTRL . TCAM_INIT
 */
#define  VTSS_F_VCAP_IS2_TCAM_BIST_TCAM_CTRL_TCAM_INIT  VTSS_BIT(0)


/** 
 * \brief Status for the TCAM
 *
 * \details
 * Register: \a VCAP_IS2:TCAM_BIST:TCAM_STAT
 */
#define VTSS_VCAP_IS2_TCAM_BIST_TCAM_STAT    VTSS_IOREG(VTSS_TO_VCAP_IS2,0x22f)

/** 
 * \brief
 * Set if BIST failed.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_TCAM_BIST_TCAM_STAT . BIST_ERR
 */
#define  VTSS_F_VCAP_IS2_TCAM_BIST_TCAM_STAT_BIST_ERR  VTSS_BIT(2)

/** 
 * \brief
 * Set when the BIST is still running. When checking the BIST result this
 * field must be cleared.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS2_TCAM_BIST_TCAM_STAT . BIST_BUSY
 */
#define  VTSS_F_VCAP_IS2_TCAM_BIST_TCAM_STAT_BIST_BUSY  VTSS_BIT(1)

/** 
 * \brief
 * Indicates the current operational state of the TCAM.
 *
 * \details 
 * '0': Busy with initialization.
 * '1': Ready to be used.
 *
 * Field: ::VTSS_VCAP_IS2_TCAM_BIST_TCAM_STAT . TCAM_RDY
 */
#define  VTSS_F_VCAP_IS2_TCAM_BIST_TCAM_STAT_TCAM_RDY  VTSS_BIT(0)


#endif /* _VTSS_JAGUAR_REGS_VCAP_IS2_H_ */
