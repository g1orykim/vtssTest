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
#ifndef _VTSS_JAGUAR_REGS_VCAP_IS1_H_
#define _VTSS_JAGUAR_REGS_VCAP_IS1_H_


#include "vtss_jaguar_regs_common.h"

/*********************************************************************** 
 *
 * Target: \a VCAP_IS1
 *
 * Ingress QoS and Profile classification block.
 *
 ***********************************************************************/

/**
 * Register Group: \a VCAP_IS1:IS1_CONTROL
 *
 * Miscellaneous Configuration Registers for VCAP
 */


/** 
 * \brief Miscellaneous VCAP Configuration
 *
 * \details
 * Register: \a VCAP_IS1:IS1_CONTROL:ACL_CFG
 */
#define VTSS_VCAP_IS1_IS1_CONTROL_ACL_CFG    VTSS_IOREG(VTSS_TO_VCAP_IS1,0x0)

/** 
 * \brief
 * Enables or disables use of VCAP frame processing. The VCAP can still be
 * configured even though VCAP frame processing is disabled.
 *
 * \details 
 * 0:  VCAP frame processing is disabled.
 * 1:  VCAP frame processing is enabled.
 *
 * Field: ::VTSS_VCAP_IS1_IS1_CONTROL_ACL_CFG . ACL_ENA
 */
#define  VTSS_F_VCAP_IS1_IS1_CONTROL_ACL_CFG_ACL_ENA  VTSS_BIT(0)


/** 
 * \brief Entry and Action Control
 *
 * \details
 * Register: \a VCAP_IS1:IS1_CONTROL:ACE_UPDATE_CTRL
 */
#define VTSS_VCAP_IS1_IS1_CONTROL_ACE_UPDATE_CTRL  VTSS_IOREG(VTSS_TO_VCAP_IS1,0x1)

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
 * Field: ::VTSS_VCAP_IS1_IS1_CONTROL_ACE_UPDATE_CTRL . ACE_INDEX
 */
#define  VTSS_F_VCAP_IS1_IS1_CONTROL_ACE_UPDATE_CTRL_ACE_INDEX(x)  VTSS_ENCODE_BITFIELD(x,4,10)
#define  VTSS_M_VCAP_IS1_IS1_CONTROL_ACE_UPDATE_CTRL_ACE_INDEX     VTSS_ENCODE_BITMASK(4,10)
#define  VTSS_X_VCAP_IS1_IS1_CONTROL_ACE_UPDATE_CTRL_ACE_INDEX(x)  VTSS_EXTRACT_BITFIELD(x,4,10)

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
 * Field: ::VTSS_VCAP_IS1_IS1_CONTROL_ACE_UPDATE_CTRL . ACE_UPDATE_CMD
 */
#define  VTSS_F_VCAP_IS1_IS1_CONTROL_ACE_UPDATE_CTRL_ACE_UPDATE_CMD(x)  VTSS_ENCODE_BITFIELD(x,1,3)
#define  VTSS_M_VCAP_IS1_IS1_CONTROL_ACE_UPDATE_CTRL_ACE_UPDATE_CMD     VTSS_ENCODE_BITMASK(1,3)
#define  VTSS_X_VCAP_IS1_IS1_CONTROL_ACE_UPDATE_CTRL_ACE_UPDATE_CMD(x)  VTSS_EXTRACT_BITFIELD(x,1,3)

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
 * Field: ::VTSS_VCAP_IS1_IS1_CONTROL_ACE_UPDATE_CTRL . ACE_UPDATE_SHOT
 */
#define  VTSS_F_VCAP_IS1_IS1_CONTROL_ACE_UPDATE_CTRL_ACE_UPDATE_SHOT  VTSS_BIT(0)


/** 
 * \brief Advanced Move/Copy Operations
 *
 * \details
 * Register: \a VCAP_IS1:IS1_CONTROL:ACE_MV_CFG
 */
#define VTSS_VCAP_IS1_IS1_CONTROL_ACE_MV_CFG  VTSS_IOREG(VTSS_TO_VCAP_IS1,0x2)

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
 * Field: ::VTSS_VCAP_IS1_IS1_CONTROL_ACE_MV_CFG . ACE_MV_NUM_POS
 */
#define  VTSS_F_VCAP_IS1_IS1_CONTROL_ACE_MV_CFG_ACE_MV_NUM_POS(x)  VTSS_ENCODE_BITFIELD(x,16,10)
#define  VTSS_M_VCAP_IS1_IS1_CONTROL_ACE_MV_CFG_ACE_MV_NUM_POS     VTSS_ENCODE_BITMASK(16,10)
#define  VTSS_X_VCAP_IS1_IS1_CONTROL_ACE_MV_CFG_ACE_MV_NUM_POS(x)  VTSS_EXTRACT_BITFIELD(x,16,10)

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
 * Field: ::VTSS_VCAP_IS1_IS1_CONTROL_ACE_MV_CFG . ACE_MV_RST_HIT_STATUS_ENA
 */
#define  VTSS_F_VCAP_IS1_IS1_CONTROL_ACE_MV_CFG_ACE_MV_RST_HIT_STATUS_ENA  VTSS_BIT(15)

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
 * Field: ::VTSS_VCAP_IS1_IS1_CONTROL_ACE_MV_CFG . ACE_MV_SIZE
 */
#define  VTSS_F_VCAP_IS1_IS1_CONTROL_ACE_MV_CFG_ACE_MV_SIZE(x)  VTSS_ENCODE_BITFIELD(x,0,10)
#define  VTSS_M_VCAP_IS1_IS1_CONTROL_ACE_MV_CFG_ACE_MV_SIZE     VTSS_ENCODE_BITMASK(0,10)
#define  VTSS_X_VCAP_IS1_IS1_CONTROL_ACE_MV_CFG_ACE_MV_SIZE(x)  VTSS_EXTRACT_BITFIELD(x,0,10)


/** 
 * \brief Entry/Action Cache Status
 *
 * \details
 * Register: \a VCAP_IS1:IS1_CONTROL:ACE_STATUS
 */
#define VTSS_VCAP_IS1_IS1_CONTROL_ACE_STATUS  VTSS_IOREG(VTSS_TO_VCAP_IS1,0x3)

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
 * Field: ::VTSS_VCAP_IS1_IS1_CONTROL_ACE_STATUS . ACE_ACTION_TYPE
 */
#define  VTSS_F_VCAP_IS1_IS1_CONTROL_ACE_STATUS_ACE_ACTION_TYPE  VTSS_BIT(4)

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
 * Field: ::VTSS_VCAP_IS1_IS1_CONTROL_ACE_STATUS . ACE_ENTRY_TYPE
 */
#define  VTSS_F_VCAP_IS1_IS1_CONTROL_ACE_STATUS_ACE_ENTRY_TYPE  VTSS_BIT(0)


/** 
 * \brief Status Sticky Bits
 *
 * \details
 * Register: \a VCAP_IS1:IS1_CONTROL:ACE_STICKY
 */
#define VTSS_VCAP_IS1_IS1_CONTROL_ACE_STICKY  VTSS_IOREG(VTSS_TO_VCAP_IS1,0x4)

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
 * Field: ::VTSS_VCAP_IS1_IS1_CONTROL_ACE_STICKY . ACE_DEL_STICKY
 */
#define  VTSS_F_VCAP_IS1_IS1_CONTROL_ACE_STICKY_ACE_DEL_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a VCAP_IS1:VLAN_PAG_ENTRY
 *
 * Entry data for entry type VLAN_PAG.
 */


/** 
 * \brief Configuration of Advanced Classification
 *
 * \details
 * Register: \a VCAP_IS1:VLAN_PAG_ENTRY:ENTRY
 */
#define VTSS_VCAP_IS1_VLAN_PAG_ENTRY_ENTRY   VTSS_IOREG(VTSS_TO_VCAP_IS1,0x5)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ENTRY_ENTRY . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ENTRY_ENTRY_ISDX_NEQ0  VTSS_BIT(10)

/** 
 * \brief
 * Indicate frame received from host interface.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ENTRY_ENTRY . VPORT_VLD
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ENTRY_ENTRY_VPORT_VLD  VTSS_BIT(9)

/** 
 * \details 
 * Port PAG or UPSID, UPSPN
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ENTRY_ENTRY . PAG
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ENTRY_ENTRY_PAG(x)  VTSS_ENCODE_BITFIELD(x,1,8)
#define  VTSS_M_VCAP_IS1_VLAN_PAG_ENTRY_ENTRY_PAG     VTSS_ENCODE_BITMASK(1,8)
#define  VTSS_X_VCAP_IS1_VLAN_PAG_ENTRY_ENTRY_PAG(x)  VTSS_EXTRACT_BITFIELD(x,1,8)

/** 
 * \brief
 * Specify whether the entry is valid or not.
 * 
 * Used for indirect access to all classification entries.
 *
 * \details 
 * '0' : Disabled
 * '1' : Enabled
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ENTRY_ENTRY . VLD
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ENTRY_ENTRY_VLD  VTSS_BIT(0)


/** 
 * \brief Configuration of Advanced Classification
 *
 * \details
 * Register: \a VCAP_IS1:VLAN_PAG_ENTRY:IF_GRP
 */
#define VTSS_VCAP_IS1_VLAN_PAG_ENTRY_IF_GRP  VTSS_IOREG(VTSS_TO_VCAP_IS1,0x6)


/** 
 * \brief Configuration of Advanced Classification
 *
 * \details
 * Register: \a VCAP_IS1:VLAN_PAG_ENTRY:VLAN
 */
#define VTSS_VCAP_IS1_VLAN_PAG_ENTRY_VLAN    VTSS_IOREG(VTSS_TO_VCAP_IS1,0x7)

/** 
 * \brief
 * Priority code point (known as user priority)
 *
 * \details 
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ENTRY_VLAN . PCP
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ENTRY_VLAN_PCP(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_VCAP_IS1_VLAN_PAG_ENTRY_VLAN_PCP     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_VCAP_IS1_VLAN_PAG_ENTRY_VLAN_PCP(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * Drop eligible from outer tag
 *
 * \details 
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ENTRY_VLAN . OUTER_DEI
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ENTRY_VLAN_OUTER_DEI  VTSS_BIT(3)

/** 
 * \brief
 * Drop eligible from inner tag (known as cfi)
 *
 * \details 
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ENTRY_VLAN . INNER_DEI
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ENTRY_VLAN_INNER_DEI  VTSS_BIT(2)

/** 
 * \brief
 * Specifies VLAN VID for the entry.
 * 
 * Used for indirect access to all classification entries.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ENTRY_VLAN . VID
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ENTRY_VLAN_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS1_VLAN_PAG_ENTRY_VLAN_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS1_VLAN_PAG_ENTRY_VLAN_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * Specifies whether frame is double tagged and VLAN aware at ingress.
 * 
 * Used for indirect access to all classification entries.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ENTRY_VLAN . VLAN_DBL_TAGGED
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ENTRY_VLAN_VLAN_DBL_TAGGED  VTSS_BIT(1)

/** 
 * \brief
 * Specifies whether frame is tagged and VLAN aware at ingress.
 * 
 * Used for indirect access to all classification entries.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ENTRY_VLAN . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ENTRY_VLAN_VLAN_TAGGED  VTSS_BIT(0)


/** 
 * \brief Configuration of Advanced Classification
 *
 * \details
 * Register: \a VCAP_IS1:VLAN_PAG_ENTRY:FLAGS
 */
#define VTSS_VCAP_IS1_VLAN_PAG_ENTRY_FLAGS   VTSS_IOREG(VTSS_TO_VCAP_IS1,0x8)

/** 
 * \brief
 * Specifies layer 2 EtherType for the entry.
 * 
 * For IPv4 frames this is the TCP / UDP DPORT
 * 
 * Used for indirect access to all entries.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ENTRY_FLAGS . ETYPE
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ENTRY_FLAGS_ETYPE(x)  VTSS_ENCODE_BITFIELD(x,8,16)
#define  VTSS_M_VCAP_IS1_VLAN_PAG_ENTRY_FLAGS_ETYPE     VTSS_ENCODE_BITMASK(8,16)
#define  VTSS_X_VCAP_IS1_VLAN_PAG_ENTRY_FLAGS_ETYPE(x)  VTSS_EXTRACT_BITFIELD(x,8,16)

/** 
 * \brief
 * IP Proto=6 (TCP)
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ENTRY_FLAGS . TCP
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ENTRY_FLAGS_TCP  VTSS_BIT(7)

/** 
 * \brief
 * IP Proto=6 or 17 (TCP or UDP)
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ENTRY_FLAGS . TCP_UDP
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ENTRY_FLAGS_TCP_UDP  VTSS_BIT(6)

/** 
 * \brief
 * Specifies Internet Protocol Version 4 frame flag for the entry.
 * 
 * Used for indirect access to all entries.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ENTRY_FLAGS . IP4
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ENTRY_FLAGS_IP4  VTSS_BIT(5)

/** 
 * \brief
 * Specifies Layer 2 IP multicast flag (DMAC(47:23=0x01005E0) for the
 * entry.
 * 
 * Used for indirect access to all entries.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ENTRY_FLAGS . IP_MC
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ENTRY_FLAGS_IP_MC  VTSS_BIT(4)

/** 
 * \brief
 * Specifies Internet Protocol frame flag for the entry.
 * 
 * Used for indirect access to all entries.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 * Field can be written by SW and HW.
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ENTRY_FLAGS . IP
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ENTRY_FLAGS_IP  VTSS_BIT(3)

/** 
 * \brief
 * Specifies EtherType flag for the entry.
 * 
 * Used for indirect access to all entries.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ENTRY_FLAGS . ETYPE_LEN
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ENTRY_FLAGS_ETYPE_LEN  VTSS_BIT(2)

/** 
 * \brief
 * Specifies Layer 2 multicast flag (DMAC(40)=one) for the entry.
 * 
 * Used for indirect access to all entries.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ENTRY_FLAGS . L2_MC
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ENTRY_FLAGS_L2_MC  VTSS_BIT(1)

/** 
 * \brief
 * Specifies Layer 2 broadcast flag (DMAC=all ones) for the entry.
 * 
 * Used for indirect access to all entries.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ENTRY_FLAGS . L2_BC
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ENTRY_FLAGS_L2_BC  VTSS_BIT(0)


/** 
 * \brief Configuration of Advanced Classification
 *
 * \details
 * Register: \a VCAP_IS1:VLAN_PAG_ENTRY:L2_MAC_ADDR_HIGH
 */
#define VTSS_VCAP_IS1_VLAN_PAG_ENTRY_L2_MAC_ADDR_HIGH  VTSS_IOREG(VTSS_TO_VCAP_IS1,0x9)

/** 
 * \brief
 * Specifies MAC ADDR OUI for the entry (MAC ADDR bit 47:32).
 * Either SMAC or DMAC based on the action result from S0.
 * Used for indirect access to all entries.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ENTRY_L2_MAC_ADDR_HIGH . L2_MAC_ADDR_HIGH
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ENTRY_L2_MAC_ADDR_HIGH_L2_MAC_ADDR_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_VCAP_IS1_VLAN_PAG_ENTRY_L2_MAC_ADDR_HIGH_L2_MAC_ADDR_HIGH     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_VCAP_IS1_VLAN_PAG_ENTRY_L2_MAC_ADDR_HIGH_L2_MAC_ADDR_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief Configuration of Advanced Classification
 *
 * \details
 * Register: \a VCAP_IS1:VLAN_PAG_ENTRY:L2_MAC_ADDR_LOW
 */
#define VTSS_VCAP_IS1_VLAN_PAG_ENTRY_L2_MAC_ADDR_LOW  VTSS_IOREG(VTSS_TO_VCAP_IS1,0xa)

/** 
 * \brief
 * Specifies LSB MAC ADDR for the entry.
 * Either SMAC or DMAC based on the action result from S0.
 * Only used for indirect access to User classification entries.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ENTRY_L2_MAC_ADDR_LOW . L2_MAC_ADDR_LOW
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ENTRY_L2_MAC_ADDR_LOW_L2_MAC_ADDR_LOW(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_VCAP_IS1_VLAN_PAG_ENTRY_L2_MAC_ADDR_LOW_L2_MAC_ADDR_LOW     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_VCAP_IS1_VLAN_PAG_ENTRY_L2_MAC_ADDR_LOW_L2_MAC_ADDR_LOW(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief Configuration of Advanced Classification
 *
 * \details
 * Register: \a VCAP_IS1:VLAN_PAG_ENTRY:L3_IP4_SIP
 */
#define VTSS_VCAP_IS1_VLAN_PAG_ENTRY_L3_IP4_SIP  VTSS_IOREG(VTSS_TO_VCAP_IS1,0xb)


/** 
 * \brief Configuration of Advanced Classification
 *
 * \details
 * Register: \a VCAP_IS1:VLAN_PAG_ENTRY:L3_MISC
 */
#define VTSS_VCAP_IS1_VLAN_PAG_ENTRY_L3_MISC  VTSS_IOREG(VTSS_TO_VCAP_IS1,0xc)

/** 
 * \brief
 * Layer 3 fragment flag.
 *
 * \details 
 * 0: IPv4 frames where the More Fragments (MF) bit is set or the Fragment
 * Offset (FRAG OFFSET) field is greater than 0 must not be able to match
 * this entry.
 * 1: Only IPv4 frames where the More Fragments (MF) bit is set or the
 * Fragment Offset (FRAG OFFSET) field is greater than 0 must be able to
 * match this entry.
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ENTRY_L3_MISC . L3_FRAGMENT
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ENTRY_L3_MISC_L3_FRAGMENT  VTSS_BIT(1)


/** 
 * \brief Miscellaneous TCP / UDP Fields
 *
 * \details
 * Register: \a VCAP_IS1:VLAN_PAG_ENTRY:L4_MISC
 */
#define VTSS_VCAP_IS1_VLAN_PAG_ENTRY_L4_MISC  VTSS_IOREG(VTSS_TO_VCAP_IS1,0xd)

/** 
 * \brief
 * TCP / UDP Source / Destination Port range match flags. Each bit in this
 * field is match against the result of a VCAP TCP / UDP SPORT / DPORT
 * range check.
 * If a port range check is disabled, the L4_RNG bit for this check should
 * be set to either 0 or "don't care".
 *
 * \details 
 * xxxxxxx0: Only TCP / UDP frames where the SPORT / DPORT is outside the
 * range specified in ANA_CL::ADV_RNG_VALUE_CFG[0] must be able to match
 * this entry, unless the port range check has been disabled.
 * xxxxxxx1: Only TCP / UDP frames where the SPORT / DPORT is within the
 * range specified in ANA_CL::ADV_RNG_VALUE_CFG[0] must be able to match
 * this entry.
 * xxxxxx0x: Only TCP / UDP frames where the SPORT / DPORT is outside the
 * range specified in ANA_CL::ADV_RNG_VALUE_CFG[1] must be able to match
 * this entry, unless the port range check has been disabled.
 * (...)
 * 1xxxxxxx: Only TCP / UDP frames where the SPORT / DPORT is within the
 * range specified in ANA_CL::ADV_RNG_VALUE_CFG[7] must be able to match
 * this entry.

 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ENTRY_L4_MISC . L4_RNG
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ENTRY_L4_MISC_L4_RNG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS1_VLAN_PAG_ENTRY_L4_MISC_L4_RNG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS1_VLAN_PAG_ENTRY_L4_MISC_L4_RNG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a VCAP_IS1:QOS_ENTRY
 *
 * Entry data for entry type QOS.
 */


/** 
 * \brief Configuration of Advanced Classification
 *
 * \details
 * Register: \a VCAP_IS1:QOS_ENTRY:ENTRY
 */
#define VTSS_VCAP_IS1_QOS_ENTRY_ENTRY        VTSS_IOREG(VTSS_TO_VCAP_IS1,0x15)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS1_QOS_ENTRY_ENTRY . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS1_QOS_ENTRY_ENTRY_ISDX_NEQ0  VTSS_BIT(10)

/** 
 * \brief
 * Indicate frame received from host interface.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS1_QOS_ENTRY_ENTRY . VPORT_VLD
 */
#define  VTSS_F_VCAP_IS1_QOS_ENTRY_ENTRY_VPORT_VLD  VTSS_BIT(9)

/** 
 * \details 
 * Port PAG or UPSID, UPSPN
 *
 * Field: ::VTSS_VCAP_IS1_QOS_ENTRY_ENTRY . PAG
 */
#define  VTSS_F_VCAP_IS1_QOS_ENTRY_ENTRY_PAG(x)  VTSS_ENCODE_BITFIELD(x,1,8)
#define  VTSS_M_VCAP_IS1_QOS_ENTRY_ENTRY_PAG     VTSS_ENCODE_BITMASK(1,8)
#define  VTSS_X_VCAP_IS1_QOS_ENTRY_ENTRY_PAG(x)  VTSS_EXTRACT_BITFIELD(x,1,8)

/** 
 * \brief
 * Specify whether the entry is valid or not.
 * 
 * Used for indirect access to all classification entries.
 *
 * \details 
 * '0' : Disabled
 * '1' : Enabled
 *
 * Field: ::VTSS_VCAP_IS1_QOS_ENTRY_ENTRY . VLD
 */
#define  VTSS_F_VCAP_IS1_QOS_ENTRY_ENTRY_VLD  VTSS_BIT(0)


/** 
 * \brief Configuration of Advanced Classification
 *
 * \details
 * Register: \a VCAP_IS1:QOS_ENTRY:IF_GRP
 */
#define VTSS_VCAP_IS1_QOS_ENTRY_IF_GRP       VTSS_IOREG(VTSS_TO_VCAP_IS1,0x16)


/** 
 * \brief Configuration of Advanced Classification
 *
 * \details
 * Register: \a VCAP_IS1:QOS_ENTRY:VLAN
 */
#define VTSS_VCAP_IS1_QOS_ENTRY_VLAN         VTSS_IOREG(VTSS_TO_VCAP_IS1,0x17)

/** 
 * \brief
 * Priority code point (known as user priority)
 *
 * \details 
 * Field: ::VTSS_VCAP_IS1_QOS_ENTRY_VLAN . PCP
 */
#define  VTSS_F_VCAP_IS1_QOS_ENTRY_VLAN_PCP(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_VCAP_IS1_QOS_ENTRY_VLAN_PCP     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_VCAP_IS1_QOS_ENTRY_VLAN_PCP(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * Drop eligible from outer tag
 *
 * \details 
 * Field: ::VTSS_VCAP_IS1_QOS_ENTRY_VLAN . OUTER_DEI
 */
#define  VTSS_F_VCAP_IS1_QOS_ENTRY_VLAN_OUTER_DEI  VTSS_BIT(3)

/** 
 * \brief
 * Drop eligible from inner tag (known as cfi)
 *
 * \details 
 * Field: ::VTSS_VCAP_IS1_QOS_ENTRY_VLAN . INNER_DEI
 */
#define  VTSS_F_VCAP_IS1_QOS_ENTRY_VLAN_INNER_DEI  VTSS_BIT(2)

/** 
 * \brief
 * Specifies VLAN VID for the entry.
 * 
 * Used for indirect access to all classification entries.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_QOS_ENTRY_VLAN . VID
 */
#define  VTSS_F_VCAP_IS1_QOS_ENTRY_VLAN_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS1_QOS_ENTRY_VLAN_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS1_QOS_ENTRY_VLAN_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * Specifies whether frame is double tagged and VLAN aware at ingress.
 * 
 * Used for indirect access to all classification entries.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_QOS_ENTRY_VLAN . VLAN_DBL_TAGGED
 */
#define  VTSS_F_VCAP_IS1_QOS_ENTRY_VLAN_VLAN_DBL_TAGGED  VTSS_BIT(1)

/** 
 * \brief
 * Specifies whether frame is tagged and VLAN aware at ingress.
 * 
 * Used for indirect access to all classification entries.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_QOS_ENTRY_VLAN . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS1_QOS_ENTRY_VLAN_VLAN_TAGGED  VTSS_BIT(0)


/** 
 * \brief Configuration of Advanced Classification
 *
 * \details
 * Register: \a VCAP_IS1:QOS_ENTRY:FLAGS
 */
#define VTSS_VCAP_IS1_QOS_ENTRY_FLAGS        VTSS_IOREG(VTSS_TO_VCAP_IS1,0x18)

/** 
 * \brief
 * Specifies layer 2 EtherType for the entry.
 * 
 * For IPv4 frames this is the TCP / UDP DPORT
 * 
 * Used for indirect access to all entries.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_QOS_ENTRY_FLAGS . ETYPE
 */
#define  VTSS_F_VCAP_IS1_QOS_ENTRY_FLAGS_ETYPE(x)  VTSS_ENCODE_BITFIELD(x,8,16)
#define  VTSS_M_VCAP_IS1_QOS_ENTRY_FLAGS_ETYPE     VTSS_ENCODE_BITMASK(8,16)
#define  VTSS_X_VCAP_IS1_QOS_ENTRY_FLAGS_ETYPE(x)  VTSS_EXTRACT_BITFIELD(x,8,16)

/** 
 * \brief
 * IP Proto=6 (TCP)
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_QOS_ENTRY_FLAGS . TCP
 */
#define  VTSS_F_VCAP_IS1_QOS_ENTRY_FLAGS_TCP  VTSS_BIT(7)

/** 
 * \brief
 * IP Proto=6 or 17 (TCP or UDP)
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_QOS_ENTRY_FLAGS . TCP_UDP
 */
#define  VTSS_F_VCAP_IS1_QOS_ENTRY_FLAGS_TCP_UDP  VTSS_BIT(6)

/** 
 * \brief
 * Specifies Internet Protocol Version 4 frame flag for the entry.
 * 
 * Used for indirect access to all entries.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_QOS_ENTRY_FLAGS . IP4
 */
#define  VTSS_F_VCAP_IS1_QOS_ENTRY_FLAGS_IP4  VTSS_BIT(5)

/** 
 * \brief
 * Specifies Layer 2 IP multicast flag (DMAC(47:23=0x01005E0) for the
 * entry.
 * 
 * Used for indirect access to all entries.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_QOS_ENTRY_FLAGS . IP_MC
 */
#define  VTSS_F_VCAP_IS1_QOS_ENTRY_FLAGS_IP_MC  VTSS_BIT(4)

/** 
 * \brief
 * Specifies Internet Protocol frame flag for the entry.
 * 
 * Used for indirect access to all entries.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 * Field can be written by SW and HW.
 *
 * Field: ::VTSS_VCAP_IS1_QOS_ENTRY_FLAGS . IP
 */
#define  VTSS_F_VCAP_IS1_QOS_ENTRY_FLAGS_IP   VTSS_BIT(3)

/** 
 * \brief
 * Specifies EtherType flag for the entry.
 * 
 * Used for indirect access to all entries.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_QOS_ENTRY_FLAGS . ETYPE_LEN
 */
#define  VTSS_F_VCAP_IS1_QOS_ENTRY_FLAGS_ETYPE_LEN  VTSS_BIT(2)

/** 
 * \brief
 * Specifies Layer 2 multicast flag (DMAC(40)=one) for the entry.
 * 
 * Used for indirect access to all entries.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_QOS_ENTRY_FLAGS . L2_MC
 */
#define  VTSS_F_VCAP_IS1_QOS_ENTRY_FLAGS_L2_MC  VTSS_BIT(1)

/** 
 * \brief
 * Specifies Layer 2 broadcast flag (DMAC=all ones) for the entry.
 * 
 * Used for indirect access to all entries.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_QOS_ENTRY_FLAGS . L2_BC
 */
#define  VTSS_F_VCAP_IS1_QOS_ENTRY_FLAGS_L2_BC  VTSS_BIT(0)


/** 
 * \brief Configuration of Advanced Classification
 *
 * \details
 * Register: \a VCAP_IS1:QOS_ENTRY:L2_MAC_ADDR_HIGH
 */
#define VTSS_VCAP_IS1_QOS_ENTRY_L2_MAC_ADDR_HIGH  VTSS_IOREG(VTSS_TO_VCAP_IS1,0x19)

/** 
 * \brief
 * Specifies MAC ADDR OUI for the entry (MAC ADDR bit 47:32).
 * Either SMAC or DMAC based on the action result from S0.
 * Used for indirect access to all entries.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS1_QOS_ENTRY_L2_MAC_ADDR_HIGH . L2_MAC_ADDR_HIGH
 */
#define  VTSS_F_VCAP_IS1_QOS_ENTRY_L2_MAC_ADDR_HIGH_L2_MAC_ADDR_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_VCAP_IS1_QOS_ENTRY_L2_MAC_ADDR_HIGH_L2_MAC_ADDR_HIGH     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_VCAP_IS1_QOS_ENTRY_L2_MAC_ADDR_HIGH_L2_MAC_ADDR_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief Configuration of Advanced Classification
 *
 * \details
 * Register: \a VCAP_IS1:QOS_ENTRY:L3_IP4_SIP
 */
#define VTSS_VCAP_IS1_QOS_ENTRY_L3_IP4_SIP   VTSS_IOREG(VTSS_TO_VCAP_IS1,0x1a)


/** 
 * \brief Configuration of Advanced Classification
 *
 * \details
 * Register: \a VCAP_IS1:QOS_ENTRY:L3_MISC
 */
#define VTSS_VCAP_IS1_QOS_ENTRY_L3_MISC      VTSS_IOREG(VTSS_TO_VCAP_IS1,0x1b)

/** 
 * \brief
 * Specifies layer 3 DSCP Pattern for the entry.
 * 
 * Only used for indirect access to QoS classification entries.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_QOS_ENTRY_L3_MISC . L3_DSCP
 */
#define  VTSS_F_VCAP_IS1_QOS_ENTRY_L3_MISC_L3_DSCP(x)  VTSS_ENCODE_BITFIELD(x,4,6)
#define  VTSS_M_VCAP_IS1_QOS_ENTRY_L3_MISC_L3_DSCP     VTSS_ENCODE_BITMASK(4,6)
#define  VTSS_X_VCAP_IS1_QOS_ENTRY_L3_MISC_L3_DSCP(x)  VTSS_EXTRACT_BITFIELD(x,4,6)

/** 
 * \brief
 * Layer 3 fragment flag.
 *
 * \details 
 * 0: IPv4 frames where the More Fragments (MF) bit is set or the Fragment
 * Offset (FRAG OFFSET) field is greater than 0 must not be able to match
 * this entry.
 * 1: Only IPv4 frames where the More Fragments (MF) bit is set or the
 * Fragment Offset (FRAG OFFSET) field is greater than 0 must be able to
 * match this entry.
 *
 * Field: ::VTSS_VCAP_IS1_QOS_ENTRY_L3_MISC . L3_FRAGMENT
 */
#define  VTSS_F_VCAP_IS1_QOS_ENTRY_L3_MISC_L3_FRAGMENT  VTSS_BIT(1)


/** 
 * \brief TCP / UDP Source and Destination Ports
 *
 * \details
 * Register: \a VCAP_IS1:QOS_ENTRY:L4_PORT
 */
#define VTSS_VCAP_IS1_QOS_ENTRY_L4_PORT      VTSS_IOREG(VTSS_TO_VCAP_IS1,0x1c)

/** 
 * \brief
 * This field is matched against the TCP / UDP Source Port field (SPORT).
 *
 * \details 
 * Field: ::VTSS_VCAP_IS1_QOS_ENTRY_L4_PORT . L4_SPORT
 */
#define  VTSS_F_VCAP_IS1_QOS_ENTRY_L4_PORT_L4_SPORT(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_VCAP_IS1_QOS_ENTRY_L4_PORT_L4_SPORT     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_VCAP_IS1_QOS_ENTRY_L4_PORT_L4_SPORT(x)  VTSS_EXTRACT_BITFIELD(x,16,16)


/** 
 * \brief Miscellaneous TCP / UDP Fields
 *
 * \details
 * Register: \a VCAP_IS1:QOS_ENTRY:L4_MISC
 */
#define VTSS_VCAP_IS1_QOS_ENTRY_L4_MISC      VTSS_IOREG(VTSS_TO_VCAP_IS1,0x1d)

/** 
 * \brief
 * TCP / UDP Source / Destination Port range match flags. Each bit in this
 * field is match against the result of a VCAP TCP / UDP SPORT / DPORT
 * range check.
 * If a port range check is disabled, the L4_RNG bit for this check should
 * be set to either 0 or "don't care".
 *
 * \details 
 * xxxxxxx0: Only TCP / UDP frames where the SPORT / DPORT is outside the
 * range specified in ANA_CL::ADV_RNG_VALUE_CFG[0] must be able to match
 * this entry, unless the port range check has been disabled.
 * xxxxxxx1: Only TCP / UDP frames where the SPORT / DPORT is within the
 * range specified in ANA_CL::ADV_RNG_VALUE_CFG[0] must be able to match
 * this entry.
 * xxxxxx0x: Only TCP / UDP frames where the SPORT / DPORT is outside the
 * range specified in ANA_CL::ADV_RNG_VALUE_CFG[1] must be able to match
 * this entry, unless the port range check has been disabled.
 * (...)
 * 1xxxxxxx: Only TCP / UDP frames where the SPORT / DPORT is within the
 * range specified in ANA_CL::ADV_RNG_VALUE_CFG[7] must be able to match
 * this entry.

 *
 * Field: ::VTSS_VCAP_IS1_QOS_ENTRY_L4_MISC . L4_RNG
 */
#define  VTSS_F_VCAP_IS1_QOS_ENTRY_L4_MISC_L4_RNG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS1_QOS_ENTRY_L4_MISC_L4_RNG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS1_QOS_ENTRY_L4_MISC_L4_RNG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a VCAP_IS1:VLAN_PAG_MASK
 *
 * Entry mask for entry type VLAN_PAG.
 */


/** 
 * \brief Mask register corresponding to VLAN_PAG_ENTRY:ENTRY
 *
 * \details
 * Mask register corresponding to VLAN_PAG_ENTRY:ENTRY
 *
 * Register: \a VCAP_IS1:VLAN_PAG_MASK:ENTRY
 */
#define VTSS_VCAP_IS1_VLAN_PAG_MASK_ENTRY    VTSS_IOREG(VTSS_TO_VCAP_IS1,0x25)

/** 
 * \brief
 * Mask for field VLAN_PAG_ENTRY:ENTRY.ISDX_NEQ0
 *
 * \details 
 * '0': Entry field ISDX_NEQ0 must match
 * '1': Entry field ISDX_NEQ0 is ignored
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_MASK_ENTRY . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_MASK_ENTRY_ISDX_NEQ0  VTSS_BIT(10)

/** 
 * \brief
 * Mask has to be valid always.
 *
 * \details 
 * Must be set to '0'
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_MASK_ENTRY . VPORT_VLD
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_MASK_ENTRY_VPORT_VLD  VTSS_BIT(9)

/** 
 * \brief
 * Mask for field VLAN_PAG_ENTRY:ENTRY.PAG
 *
 * \details 
 * '0': Entry field PAG must match
 * '1': Entry field PAG is ignored
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_MASK_ENTRY . PAG
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_MASK_ENTRY_PAG(x)  VTSS_ENCODE_BITFIELD(x,1,8)
#define  VTSS_M_VCAP_IS1_VLAN_PAG_MASK_ENTRY_PAG     VTSS_ENCODE_BITMASK(1,8)
#define  VTSS_X_VCAP_IS1_VLAN_PAG_MASK_ENTRY_PAG(x)  VTSS_EXTRACT_BITFIELD(x,1,8)

/** 
 * \brief
 * Mask has to be valid always.
 *
 * \details 
 * Must be set to '0'
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_MASK_ENTRY . VLD
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_MASK_ENTRY_VLD  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to VLAN_PAG_ENTRY:IF_GRP
 *
 * \details
 * Mask register corresponding to VLAN_PAG_ENTRY:IF_GRP
 *
 * Register: \a VCAP_IS1:VLAN_PAG_MASK:IF_GRP
 */
#define VTSS_VCAP_IS1_VLAN_PAG_MASK_IF_GRP   VTSS_IOREG(VTSS_TO_VCAP_IS1,0x26)


/** 
 * \brief Mask register corresponding to VLAN_PAG_ENTRY:VLAN
 *
 * \details
 * Mask register corresponding to VLAN_PAG_ENTRY:VLAN
 *
 * Register: \a VCAP_IS1:VLAN_PAG_MASK:VLAN
 */
#define VTSS_VCAP_IS1_VLAN_PAG_MASK_VLAN     VTSS_IOREG(VTSS_TO_VCAP_IS1,0x27)

/** 
 * \brief
 * Mask for field VLAN_PAG_ENTRY:VLAN.PCP
 *
 * \details 
 * '0': Entry field PCP must match
 * '1': Entry field PCP is ignored
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_MASK_VLAN . PCP
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_MASK_VLAN_PCP(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_VCAP_IS1_VLAN_PAG_MASK_VLAN_PCP     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_VCAP_IS1_VLAN_PAG_MASK_VLAN_PCP(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * Mask for field VLAN_PAG_ENTRY:VLAN.OUTER_DEI
 *
 * \details 
 * '0': Entry field OUTER_DEI must match
 * '1': Entry field OUTER_DEI is ignored
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_MASK_VLAN . OUTER_DEI
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_MASK_VLAN_OUTER_DEI  VTSS_BIT(3)

/** 
 * \brief
 * Mask for field VLAN_PAG_ENTRY:VLAN.INNER_DEI
 *
 * \details 
 * '0': Entry field INNER_DEI must match
 * '1': Entry field INNER_DEI is ignored
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_MASK_VLAN . INNER_DEI
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_MASK_VLAN_INNER_DEI  VTSS_BIT(2)

/** 
 * \brief
 * Mask for field VLAN_PAG_ENTRY:VLAN.VID
 *
 * \details 
 * '0': Entry field VID must match
 * '1': Entry field VID is ignored
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_MASK_VLAN . VID
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_MASK_VLAN_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS1_VLAN_PAG_MASK_VLAN_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS1_VLAN_PAG_MASK_VLAN_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * Mask for field VLAN_PAG_ENTRY:VLAN.VLAN_DBL_TAGGED
 *
 * \details 
 * '0': Entry field VLAN_DBL_TAGGED must match
 * '1': Entry field VLAN_DBL_TAGGED is ignored
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_MASK_VLAN . VLAN_DBL_TAGGED
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_MASK_VLAN_VLAN_DBL_TAGGED  VTSS_BIT(1)

/** 
 * \brief
 * Mask for field VLAN_PAG_ENTRY:VLAN.VLAN_TAGGED
 *
 * \details 
 * '0': Entry field VLAN_TAGGED must match
 * '1': Entry field VLAN_TAGGED is ignored
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_MASK_VLAN . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_MASK_VLAN_VLAN_TAGGED  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to VLAN_PAG_ENTRY:FLAGS
 *
 * \details
 * Mask register corresponding to VLAN_PAG_ENTRY:FLAGS
 *
 * Register: \a VCAP_IS1:VLAN_PAG_MASK:FLAGS
 */
#define VTSS_VCAP_IS1_VLAN_PAG_MASK_FLAGS    VTSS_IOREG(VTSS_TO_VCAP_IS1,0x28)

/** 
 * \brief
 * Mask for field VLAN_PAG_ENTRY:FLAGS.ETYPE
 *
 * \details 
 * '0': Entry field ETYPE must match
 * '1': Entry field ETYPE is ignored
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_MASK_FLAGS . ETYPE
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_MASK_FLAGS_ETYPE(x)  VTSS_ENCODE_BITFIELD(x,8,16)
#define  VTSS_M_VCAP_IS1_VLAN_PAG_MASK_FLAGS_ETYPE     VTSS_ENCODE_BITMASK(8,16)
#define  VTSS_X_VCAP_IS1_VLAN_PAG_MASK_FLAGS_ETYPE(x)  VTSS_EXTRACT_BITFIELD(x,8,16)

/** 
 * \brief
 * Mask for field VLAN_PAG_ENTRY:FLAGS.TCP
 *
 * \details 
 * '0': Entry field TCP must match
 * '1': Entry field TCP is ignored
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_MASK_FLAGS . TCP
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_MASK_FLAGS_TCP  VTSS_BIT(7)

/** 
 * \brief
 * Mask for field VLAN_PAG_ENTRY:FLAGS.TCP_UDP
 *
 * \details 
 * '0': Entry field TCP_UDP must match
 * '1': Entry field TCP_UDP is ignored
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_MASK_FLAGS . TCP_UDP
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_MASK_FLAGS_TCP_UDP  VTSS_BIT(6)

/** 
 * \brief
 * Mask for field VLAN_PAG_ENTRY:FLAGS.IP4
 *
 * \details 
 * '0': Entry field IP4 must match
 * '1': Entry field IP4 is ignored
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_MASK_FLAGS . IP4
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_MASK_FLAGS_IP4  VTSS_BIT(5)

/** 
 * \brief
 * Mask for field VLAN_PAG_ENTRY:FLAGS.IP_MC
 *
 * \details 
 * '0': Entry field IP_MC must match
 * '1': Entry field IP_MC is ignored
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_MASK_FLAGS . IP_MC
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_MASK_FLAGS_IP_MC  VTSS_BIT(4)

/** 
 * \brief
 * Mask for field VLAN_PAG_ENTRY:FLAGS.IP
 *
 * \details 
 * '0': Entry field IP must match
 * '1': Entry field IP is ignored
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_MASK_FLAGS . IP
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_MASK_FLAGS_IP  VTSS_BIT(3)

/** 
 * \brief
 * Mask for field VLAN_PAG_ENTRY:FLAGS.ETYPE_LEN
 *
 * \details 
 * '0': Entry field ETYPE_LEN must match
 * '1': Entry field ETYPE_LEN is ignored
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_MASK_FLAGS . ETYPE_LEN
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_MASK_FLAGS_ETYPE_LEN  VTSS_BIT(2)

/** 
 * \brief
 * Mask for field VLAN_PAG_ENTRY:FLAGS.L2_MC
 *
 * \details 
 * '0': Entry field L2_MC must match
 * '1': Entry field L2_MC is ignored
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_MASK_FLAGS . L2_MC
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_MASK_FLAGS_L2_MC  VTSS_BIT(1)

/** 
 * \brief
 * Mask for field VLAN_PAG_ENTRY:FLAGS.L2_BC
 *
 * \details 
 * '0': Entry field L2_BC must match
 * '1': Entry field L2_BC is ignored
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_MASK_FLAGS . L2_BC
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_MASK_FLAGS_L2_BC  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to VLAN_PAG_ENTRY:L2_MAC_ADDR_HIGH
 *
 * \details
 * Mask register corresponding to VLAN_PAG_ENTRY:L2_MAC_ADDR_HIGH
 *
 * Register: \a VCAP_IS1:VLAN_PAG_MASK:L2_MAC_ADDR_HIGH
 */
#define VTSS_VCAP_IS1_VLAN_PAG_MASK_L2_MAC_ADDR_HIGH  VTSS_IOREG(VTSS_TO_VCAP_IS1,0x29)

/** 
 * \brief
 * Mask for field VLAN_PAG_ENTRY:L2_MAC_ADDR_HIGH.L2_MAC_ADDR_HIGH
 *
 * \details 
 * '0': Entry field L2_MAC_ADDR_HIGH must match
 * '1': Entry field L2_MAC_ADDR_HIGH is ignored
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_MASK_L2_MAC_ADDR_HIGH . L2_MAC_ADDR_HIGH
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_MASK_L2_MAC_ADDR_HIGH_L2_MAC_ADDR_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_VCAP_IS1_VLAN_PAG_MASK_L2_MAC_ADDR_HIGH_L2_MAC_ADDR_HIGH     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_VCAP_IS1_VLAN_PAG_MASK_L2_MAC_ADDR_HIGH_L2_MAC_ADDR_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief Mask register corresponding to VLAN_PAG_ENTRY:L2_MAC_ADDR_LOW
 *
 * \details
 * Mask register corresponding to VLAN_PAG_ENTRY:L2_MAC_ADDR_LOW
 *
 * Register: \a VCAP_IS1:VLAN_PAG_MASK:L2_MAC_ADDR_LOW
 */
#define VTSS_VCAP_IS1_VLAN_PAG_MASK_L2_MAC_ADDR_LOW  VTSS_IOREG(VTSS_TO_VCAP_IS1,0x2a)

/** 
 * \brief
 * Mask for field VLAN_PAG_ENTRY:L2_MAC_ADDR_LOW.L2_MAC_ADDR_LOW
 *
 * \details 
 * '0': Entry field L2_MAC_ADDR_LOW must match
 * '1': Entry field L2_MAC_ADDR_LOW is ignored
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_MASK_L2_MAC_ADDR_LOW . L2_MAC_ADDR_LOW
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_MASK_L2_MAC_ADDR_LOW_L2_MAC_ADDR_LOW(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_VCAP_IS1_VLAN_PAG_MASK_L2_MAC_ADDR_LOW_L2_MAC_ADDR_LOW     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_VCAP_IS1_VLAN_PAG_MASK_L2_MAC_ADDR_LOW_L2_MAC_ADDR_LOW(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief Mask register corresponding to VLAN_PAG_ENTRY:L3_IP4_SIP
 *
 * \details
 * Mask register corresponding to VLAN_PAG_ENTRY:L3_IP4_SIP
 *
 * Register: \a VCAP_IS1:VLAN_PAG_MASK:L3_IP4_SIP
 */
#define VTSS_VCAP_IS1_VLAN_PAG_MASK_L3_IP4_SIP  VTSS_IOREG(VTSS_TO_VCAP_IS1,0x2b)


/** 
 * \brief Mask register corresponding to VLAN_PAG_ENTRY:L3_MISC
 *
 * \details
 * Mask register corresponding to VLAN_PAG_ENTRY:L3_MISC
 *
 * Register: \a VCAP_IS1:VLAN_PAG_MASK:L3_MISC
 */
#define VTSS_VCAP_IS1_VLAN_PAG_MASK_L3_MISC  VTSS_IOREG(VTSS_TO_VCAP_IS1,0x2c)

/** 
 * \brief
 * Mask for field VLAN_PAG_ENTRY:L3_MISC.L3_FRAGMENT
 *
 * \details 
 * '0': Entry field L3_FRAGMENT must match
 * '1': Entry field L3_FRAGMENT is ignored
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_MASK_L3_MISC . L3_FRAGMENT
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_MASK_L3_MISC_L3_FRAGMENT  VTSS_BIT(1)


/** 
 * \brief Mask register corresponding to VLAN_PAG_ENTRY:L4_MISC
 *
 * \details
 * Mask register corresponding to VLAN_PAG_ENTRY:L4_MISC
 *
 * Register: \a VCAP_IS1:VLAN_PAG_MASK:L4_MISC
 */
#define VTSS_VCAP_IS1_VLAN_PAG_MASK_L4_MISC  VTSS_IOREG(VTSS_TO_VCAP_IS1,0x2d)

/** 
 * \brief
 * Mask for field VLAN_PAG_ENTRY:L4_MISC.L4_RNG
 *
 * \details 
 * '0': Entry field L4_RNG must match
 * '1': Entry field L4_RNG is ignored
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_MASK_L4_MISC . L4_RNG
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_MASK_L4_MISC_L4_RNG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS1_VLAN_PAG_MASK_L4_MISC_L4_RNG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS1_VLAN_PAG_MASK_L4_MISC_L4_RNG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a VCAP_IS1:QOS_MASK
 *
 * Entry mask for entry type QOS.
 */


/** 
 * \brief Mask register corresponding to QOS_ENTRY:ENTRY
 *
 * \details
 * Mask register corresponding to QOS_ENTRY:ENTRY
 *
 * Register: \a VCAP_IS1:QOS_MASK:ENTRY
 */
#define VTSS_VCAP_IS1_QOS_MASK_ENTRY         VTSS_IOREG(VTSS_TO_VCAP_IS1,0x35)

/** 
 * \brief
 * Mask for field QOS_ENTRY:ENTRY.ISDX_NEQ0
 *
 * \details 
 * '0': Entry field ISDX_NEQ0 must match
 * '1': Entry field ISDX_NEQ0 is ignored
 *
 * Field: ::VTSS_VCAP_IS1_QOS_MASK_ENTRY . ISDX_NEQ0
 */
#define  VTSS_F_VCAP_IS1_QOS_MASK_ENTRY_ISDX_NEQ0  VTSS_BIT(10)

/** 
 * \brief
 * Mask has to be valid always.
 *
 * \details 
 * Must be set to '0'
 *
 * Field: ::VTSS_VCAP_IS1_QOS_MASK_ENTRY . VPORT_VLD
 */
#define  VTSS_F_VCAP_IS1_QOS_MASK_ENTRY_VPORT_VLD  VTSS_BIT(9)

/** 
 * \brief
 * Mask for field QOS_ENTRY:ENTRY.PAG
 *
 * \details 
 * '0': Entry field PAG must match
 * '1': Entry field PAG is ignored
 *
 * Field: ::VTSS_VCAP_IS1_QOS_MASK_ENTRY . PAG
 */
#define  VTSS_F_VCAP_IS1_QOS_MASK_ENTRY_PAG(x)  VTSS_ENCODE_BITFIELD(x,1,8)
#define  VTSS_M_VCAP_IS1_QOS_MASK_ENTRY_PAG     VTSS_ENCODE_BITMASK(1,8)
#define  VTSS_X_VCAP_IS1_QOS_MASK_ENTRY_PAG(x)  VTSS_EXTRACT_BITFIELD(x,1,8)

/** 
 * \brief
 * Mask has to be valid always.
 *
 * \details 
 * Must be set to '0'
 *
 * Field: ::VTSS_VCAP_IS1_QOS_MASK_ENTRY . VLD
 */
#define  VTSS_F_VCAP_IS1_QOS_MASK_ENTRY_VLD   VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to QOS_ENTRY:IF_GRP
 *
 * \details
 * Mask register corresponding to QOS_ENTRY:IF_GRP
 *
 * Register: \a VCAP_IS1:QOS_MASK:IF_GRP
 */
#define VTSS_VCAP_IS1_QOS_MASK_IF_GRP        VTSS_IOREG(VTSS_TO_VCAP_IS1,0x36)


/** 
 * \brief Mask register corresponding to QOS_ENTRY:VLAN
 *
 * \details
 * Mask register corresponding to QOS_ENTRY:VLAN
 *
 * Register: \a VCAP_IS1:QOS_MASK:VLAN
 */
#define VTSS_VCAP_IS1_QOS_MASK_VLAN          VTSS_IOREG(VTSS_TO_VCAP_IS1,0x37)

/** 
 * \brief
 * Mask for field QOS_ENTRY:VLAN.PCP
 *
 * \details 
 * '0': Entry field PCP must match
 * '1': Entry field PCP is ignored
 *
 * Field: ::VTSS_VCAP_IS1_QOS_MASK_VLAN . PCP
 */
#define  VTSS_F_VCAP_IS1_QOS_MASK_VLAN_PCP(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_VCAP_IS1_QOS_MASK_VLAN_PCP     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_VCAP_IS1_QOS_MASK_VLAN_PCP(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * Mask for field QOS_ENTRY:VLAN.OUTER_DEI
 *
 * \details 
 * '0': Entry field OUTER_DEI must match
 * '1': Entry field OUTER_DEI is ignored
 *
 * Field: ::VTSS_VCAP_IS1_QOS_MASK_VLAN . OUTER_DEI
 */
#define  VTSS_F_VCAP_IS1_QOS_MASK_VLAN_OUTER_DEI  VTSS_BIT(3)

/** 
 * \brief
 * Mask for field QOS_ENTRY:VLAN.INNER_DEI
 *
 * \details 
 * '0': Entry field INNER_DEI must match
 * '1': Entry field INNER_DEI is ignored
 *
 * Field: ::VTSS_VCAP_IS1_QOS_MASK_VLAN . INNER_DEI
 */
#define  VTSS_F_VCAP_IS1_QOS_MASK_VLAN_INNER_DEI  VTSS_BIT(2)

/** 
 * \brief
 * Mask for field QOS_ENTRY:VLAN.VID
 *
 * \details 
 * '0': Entry field VID must match
 * '1': Entry field VID is ignored
 *
 * Field: ::VTSS_VCAP_IS1_QOS_MASK_VLAN . VID
 */
#define  VTSS_F_VCAP_IS1_QOS_MASK_VLAN_VID(x)  VTSS_ENCODE_BITFIELD(x,8,12)
#define  VTSS_M_VCAP_IS1_QOS_MASK_VLAN_VID     VTSS_ENCODE_BITMASK(8,12)
#define  VTSS_X_VCAP_IS1_QOS_MASK_VLAN_VID(x)  VTSS_EXTRACT_BITFIELD(x,8,12)

/** 
 * \brief
 * Mask for field QOS_ENTRY:VLAN.VLAN_DBL_TAGGED
 *
 * \details 
 * '0': Entry field VLAN_DBL_TAGGED must match
 * '1': Entry field VLAN_DBL_TAGGED is ignored
 *
 * Field: ::VTSS_VCAP_IS1_QOS_MASK_VLAN . VLAN_DBL_TAGGED
 */
#define  VTSS_F_VCAP_IS1_QOS_MASK_VLAN_VLAN_DBL_TAGGED  VTSS_BIT(1)

/** 
 * \brief
 * Mask for field QOS_ENTRY:VLAN.VLAN_TAGGED
 *
 * \details 
 * '0': Entry field VLAN_TAGGED must match
 * '1': Entry field VLAN_TAGGED is ignored
 *
 * Field: ::VTSS_VCAP_IS1_QOS_MASK_VLAN . VLAN_TAGGED
 */
#define  VTSS_F_VCAP_IS1_QOS_MASK_VLAN_VLAN_TAGGED  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to QOS_ENTRY:FLAGS
 *
 * \details
 * Mask register corresponding to QOS_ENTRY:FLAGS
 *
 * Register: \a VCAP_IS1:QOS_MASK:FLAGS
 */
#define VTSS_VCAP_IS1_QOS_MASK_FLAGS         VTSS_IOREG(VTSS_TO_VCAP_IS1,0x38)

/** 
 * \brief
 * Mask for field QOS_ENTRY:FLAGS.ETYPE
 *
 * \details 
 * '0': Entry field ETYPE must match
 * '1': Entry field ETYPE is ignored
 *
 * Field: ::VTSS_VCAP_IS1_QOS_MASK_FLAGS . ETYPE
 */
#define  VTSS_F_VCAP_IS1_QOS_MASK_FLAGS_ETYPE(x)  VTSS_ENCODE_BITFIELD(x,8,16)
#define  VTSS_M_VCAP_IS1_QOS_MASK_FLAGS_ETYPE     VTSS_ENCODE_BITMASK(8,16)
#define  VTSS_X_VCAP_IS1_QOS_MASK_FLAGS_ETYPE(x)  VTSS_EXTRACT_BITFIELD(x,8,16)

/** 
 * \brief
 * Mask for field QOS_ENTRY:FLAGS.TCP
 *
 * \details 
 * '0': Entry field TCP must match
 * '1': Entry field TCP is ignored
 *
 * Field: ::VTSS_VCAP_IS1_QOS_MASK_FLAGS . TCP
 */
#define  VTSS_F_VCAP_IS1_QOS_MASK_FLAGS_TCP   VTSS_BIT(7)

/** 
 * \brief
 * Mask for field QOS_ENTRY:FLAGS.TCP_UDP
 *
 * \details 
 * '0': Entry field TCP_UDP must match
 * '1': Entry field TCP_UDP is ignored
 *
 * Field: ::VTSS_VCAP_IS1_QOS_MASK_FLAGS . TCP_UDP
 */
#define  VTSS_F_VCAP_IS1_QOS_MASK_FLAGS_TCP_UDP  VTSS_BIT(6)

/** 
 * \brief
 * Mask for field QOS_ENTRY:FLAGS.IP4
 *
 * \details 
 * '0': Entry field IP4 must match
 * '1': Entry field IP4 is ignored
 *
 * Field: ::VTSS_VCAP_IS1_QOS_MASK_FLAGS . IP4
 */
#define  VTSS_F_VCAP_IS1_QOS_MASK_FLAGS_IP4   VTSS_BIT(5)

/** 
 * \brief
 * Mask for field QOS_ENTRY:FLAGS.IP_MC
 *
 * \details 
 * '0': Entry field IP_MC must match
 * '1': Entry field IP_MC is ignored
 *
 * Field: ::VTSS_VCAP_IS1_QOS_MASK_FLAGS . IP_MC
 */
#define  VTSS_F_VCAP_IS1_QOS_MASK_FLAGS_IP_MC  VTSS_BIT(4)

/** 
 * \brief
 * Mask for field QOS_ENTRY:FLAGS.IP
 *
 * \details 
 * '0': Entry field IP must match
 * '1': Entry field IP is ignored
 *
 * Field: ::VTSS_VCAP_IS1_QOS_MASK_FLAGS . IP
 */
#define  VTSS_F_VCAP_IS1_QOS_MASK_FLAGS_IP    VTSS_BIT(3)

/** 
 * \brief
 * Mask for field QOS_ENTRY:FLAGS.ETYPE_LEN
 *
 * \details 
 * '0': Entry field ETYPE_LEN must match
 * '1': Entry field ETYPE_LEN is ignored
 *
 * Field: ::VTSS_VCAP_IS1_QOS_MASK_FLAGS . ETYPE_LEN
 */
#define  VTSS_F_VCAP_IS1_QOS_MASK_FLAGS_ETYPE_LEN  VTSS_BIT(2)

/** 
 * \brief
 * Mask for field QOS_ENTRY:FLAGS.L2_MC
 *
 * \details 
 * '0': Entry field L2_MC must match
 * '1': Entry field L2_MC is ignored
 *
 * Field: ::VTSS_VCAP_IS1_QOS_MASK_FLAGS . L2_MC
 */
#define  VTSS_F_VCAP_IS1_QOS_MASK_FLAGS_L2_MC  VTSS_BIT(1)

/** 
 * \brief
 * Mask for field QOS_ENTRY:FLAGS.L2_BC
 *
 * \details 
 * '0': Entry field L2_BC must match
 * '1': Entry field L2_BC is ignored
 *
 * Field: ::VTSS_VCAP_IS1_QOS_MASK_FLAGS . L2_BC
 */
#define  VTSS_F_VCAP_IS1_QOS_MASK_FLAGS_L2_BC  VTSS_BIT(0)


/** 
 * \brief Mask register corresponding to QOS_ENTRY:L2_MAC_ADDR_HIGH
 *
 * \details
 * Mask register corresponding to QOS_ENTRY:L2_MAC_ADDR_HIGH
 *
 * Register: \a VCAP_IS1:QOS_MASK:L2_MAC_ADDR_HIGH
 */
#define VTSS_VCAP_IS1_QOS_MASK_L2_MAC_ADDR_HIGH  VTSS_IOREG(VTSS_TO_VCAP_IS1,0x39)

/** 
 * \brief
 * Mask for field QOS_ENTRY:L2_MAC_ADDR_HIGH.L2_MAC_ADDR_HIGH
 *
 * \details 
 * '0': Entry field L2_MAC_ADDR_HIGH must match
 * '1': Entry field L2_MAC_ADDR_HIGH is ignored
 *
 * Field: ::VTSS_VCAP_IS1_QOS_MASK_L2_MAC_ADDR_HIGH . L2_MAC_ADDR_HIGH
 */
#define  VTSS_F_VCAP_IS1_QOS_MASK_L2_MAC_ADDR_HIGH_L2_MAC_ADDR_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_VCAP_IS1_QOS_MASK_L2_MAC_ADDR_HIGH_L2_MAC_ADDR_HIGH     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_VCAP_IS1_QOS_MASK_L2_MAC_ADDR_HIGH_L2_MAC_ADDR_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief Mask register corresponding to QOS_ENTRY:L3_IP4_SIP
 *
 * \details
 * Mask register corresponding to QOS_ENTRY:L3_IP4_SIP
 *
 * Register: \a VCAP_IS1:QOS_MASK:L3_IP4_SIP
 */
#define VTSS_VCAP_IS1_QOS_MASK_L3_IP4_SIP    VTSS_IOREG(VTSS_TO_VCAP_IS1,0x3a)


/** 
 * \brief Mask register corresponding to QOS_ENTRY:L3_MISC
 *
 * \details
 * Mask register corresponding to QOS_ENTRY:L3_MISC
 *
 * Register: \a VCAP_IS1:QOS_MASK:L3_MISC
 */
#define VTSS_VCAP_IS1_QOS_MASK_L3_MISC       VTSS_IOREG(VTSS_TO_VCAP_IS1,0x3b)

/** 
 * \brief
 * Mask for field QOS_ENTRY:L3_MISC.L3_DSCP
 *
 * \details 
 * '0': Entry field L3_DSCP must match
 * '1': Entry field L3_DSCP is ignored
 *
 * Field: ::VTSS_VCAP_IS1_QOS_MASK_L3_MISC . L3_DSCP
 */
#define  VTSS_F_VCAP_IS1_QOS_MASK_L3_MISC_L3_DSCP(x)  VTSS_ENCODE_BITFIELD(x,4,6)
#define  VTSS_M_VCAP_IS1_QOS_MASK_L3_MISC_L3_DSCP     VTSS_ENCODE_BITMASK(4,6)
#define  VTSS_X_VCAP_IS1_QOS_MASK_L3_MISC_L3_DSCP(x)  VTSS_EXTRACT_BITFIELD(x,4,6)

/** 
 * \brief
 * Mask for field QOS_ENTRY:L3_MISC.L3_FRAGMENT
 *
 * \details 
 * '0': Entry field L3_FRAGMENT must match
 * '1': Entry field L3_FRAGMENT is ignored
 *
 * Field: ::VTSS_VCAP_IS1_QOS_MASK_L3_MISC . L3_FRAGMENT
 */
#define  VTSS_F_VCAP_IS1_QOS_MASK_L3_MISC_L3_FRAGMENT  VTSS_BIT(1)


/** 
 * \brief Mask register corresponding to QOS_ENTRY:L4_PORT
 *
 * \details
 * Mask register corresponding to QOS_ENTRY:L4_PORT
 *
 * Register: \a VCAP_IS1:QOS_MASK:L4_PORT
 */
#define VTSS_VCAP_IS1_QOS_MASK_L4_PORT       VTSS_IOREG(VTSS_TO_VCAP_IS1,0x3c)

/** 
 * \brief
 * Mask for field QOS_ENTRY:L4_PORT.L4_SPORT
 *
 * \details 
 * '0': Entry field L4_SPORT must match
 * '1': Entry field L4_SPORT is ignored
 *
 * Field: ::VTSS_VCAP_IS1_QOS_MASK_L4_PORT . L4_SPORT
 */
#define  VTSS_F_VCAP_IS1_QOS_MASK_L4_PORT_L4_SPORT(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_VCAP_IS1_QOS_MASK_L4_PORT_L4_SPORT     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_VCAP_IS1_QOS_MASK_L4_PORT_L4_SPORT(x)  VTSS_EXTRACT_BITFIELD(x,16,16)


/** 
 * \brief Mask register corresponding to QOS_ENTRY:L4_MISC
 *
 * \details
 * Mask register corresponding to QOS_ENTRY:L4_MISC
 *
 * Register: \a VCAP_IS1:QOS_MASK:L4_MISC
 */
#define VTSS_VCAP_IS1_QOS_MASK_L4_MISC       VTSS_IOREG(VTSS_TO_VCAP_IS1,0x3d)

/** 
 * \brief
 * Mask for field QOS_ENTRY:L4_MISC.L4_RNG
 *
 * \details 
 * '0': Entry field L4_RNG must match
 * '1': Entry field L4_RNG is ignored
 *
 * Field: ::VTSS_VCAP_IS1_QOS_MASK_L4_MISC . L4_RNG
 */
#define  VTSS_F_VCAP_IS1_QOS_MASK_L4_MISC_L4_RNG(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS1_QOS_MASK_L4_MISC_L4_RNG     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS1_QOS_MASK_L4_MISC_L4_RNG(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a VCAP_IS1:VLAN_PAG_ACTION
 *
 * ACL Stage 1 action registers
 */


/** 
 * \brief Advanced Classification result
 *
 * \details
 * Register: \a VCAP_IS1:VLAN_PAG_ACTION:PAG
 */
#define VTSS_VCAP_IS1_VLAN_PAG_ACTION_PAG    VTSS_IOREG(VTSS_TO_VCAP_IS1,0x45)

/** 
 * \brief
 * New PAG value to be used in VCAP_IS2 lookup. Only bits enabled in
 * PAG_OVERRIDE_MASK are changed.
 * 
 * If PAG value is not changed in VCAP_IS1, then VCAP_IS2 will use the PAG
 * value from VCAP_IS0.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ACTION_PAG . PAG_VAL
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ACTION_PAG_PAG_VAL(x)  VTSS_ENCODE_BITFIELD(x,16,8)
#define  VTSS_M_VCAP_IS1_VLAN_PAG_ACTION_PAG_PAG_VAL     VTSS_ENCODE_BITMASK(16,8)
#define  VTSS_X_VCAP_IS1_VLAN_PAG_ACTION_PAG_PAG_VAL(x)  VTSS_EXTRACT_BITFIELD(x,16,8)

/** 
 * \brief
 * Enable / Disable overiding of bits in PAG with value in PAG_VAL. 
 * See description of PAG_VAL.
 *
 * \details 
 * '0' : Disable
 * '1' : Enable
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ACTION_PAG . PAG_OVERRIDE_MASK
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ACTION_PAG_PAG_OVERRIDE_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VCAP_IS1_VLAN_PAG_ACTION_PAG_PAG_OVERRIDE_MASK     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VCAP_IS1_VLAN_PAG_ACTION_PAG_PAG_OVERRIDE_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Advanced Classification result
 *
 * \details
 * Register: \a VCAP_IS1:VLAN_PAG_ACTION:MISC
 */
#define VTSS_VCAP_IS1_VLAN_PAG_ACTION_MISC   VTSS_IOREG(VTSS_TO_VCAP_IS1,0x46)

/** 
 * \brief
 * See CUSTOM_ACE_TYPE_ENA.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ACTION_MISC . CUSTOM_ACE_TYPE_VAL
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ACTION_MISC_CUSTOM_ACE_TYPE_VAL(x)  VTSS_ENCODE_BITFIELD(x,28,3)
#define  VTSS_M_VCAP_IS1_VLAN_PAG_ACTION_MISC_CUSTOM_ACE_TYPE_VAL     VTSS_ENCODE_BITMASK(28,3)
#define  VTSS_X_VCAP_IS1_VLAN_PAG_ACTION_MISC_CUSTOM_ACE_TYPE_VAL(x)  VTSS_EXTRACT_BITFIELD(x,28,3)

/** 
 * \brief
 * Enable use of CUSTOM CE type in VCAP_IS2.
 * 
 * The actual CUSTOM CE type number (CUSTOM_0 - CUSTOM_7) is selected by
 * CUSTOM_ACE_TYPE_VAL.
 * 
 * The byte position used for the CUSTOM match is specified in 
 * VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS.CUSTOM_POS_VAL_0 - 3.
 * 
 * In order for VCAP_IS1 to enable CUSTOM CE type, this must also be
 * enabled in ANA_CL:PORT:ADV_CL_CFG.SEC_TYPE_CUSTOM_ENA.

 *
 * \details 
 * '0' : Disable
 * '1' : Enable
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ACTION_MISC . CUSTOM_ACE_TYPE_ENA
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ACTION_MISC_CUSTOM_ACE_TYPE_ENA  VTSS_BIT(24)

/** 
 * \brief
 * PCP value for advanced user classification.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ACTION_MISC . PCP_VAL
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ACTION_MISC_PCP_VAL(x)  VTSS_ENCODE_BITFIELD(x,16,3)
#define  VTSS_M_VCAP_IS1_VLAN_PAG_ACTION_MISC_PCP_VAL     VTSS_ENCODE_BITMASK(16,3)
#define  VTSS_X_VCAP_IS1_VLAN_PAG_ACTION_MISC_PCP_VAL(x)  VTSS_EXTRACT_BITFIELD(x,16,3)

/** 
 * \brief
 * Advanced VLAN Classification result. 
 * Can either be used for replace of the basic classified VID
 * (VID_OVERRIDE_ENA) or for VID offset addition (VID_REPLACE_ENA).
 * 
 * Used for indirect access to User classification entries.
 *
 * \details 
 * Field can be written by SW and HW.
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ACTION_MISC . VID_ADD_VAL
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ACTION_MISC_VID_ADD_VAL(x)  VTSS_ENCODE_BITFIELD(x,4,12)
#define  VTSS_M_VCAP_IS1_VLAN_PAG_ACTION_MISC_VID_ADD_VAL     VTSS_ENCODE_BITMASK(4,12)
#define  VTSS_X_VCAP_IS1_VLAN_PAG_ACTION_MISC_VID_ADD_VAL(x)  VTSS_EXTRACT_BITFIELD(x,4,12)

/** 
 * \brief
 * Tag Protocol Identifier value for advanced user classification.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ACTION_MISC . PCP_DEI_ENA
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ACTION_MISC_PCP_DEI_ENA  VTSS_BIT(3)

/** 
 * \brief
 * DEI value for advanced user classification.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ACTION_MISC . DEI_VAL
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ACTION_MISC_DEI_VAL  VTSS_BIT(2)

/** 
 * \brief
 * Enable / Disable addition of the VID value to the Basic VLAN
 * classification result.
 * 
 * Used for indirect access to User classification entries.
 *
 * \details 
 * '0' : Disable
 * '1' : Enable
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ACTION_MISC . VID_REPLACE_ENA
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ACTION_MISC_VID_REPLACE_ENA  VTSS_BIT(1)


/** 
 * \brief Block position
 *
 * \details
 * Block position
 *
 * Register: \a VCAP_IS1:VLAN_PAG_ACTION:CUSTOM_POS
 */
#define VTSS_VCAP_IS1_VLAN_PAG_ACTION_CUSTOM_POS  VTSS_IOREG(VTSS_TO_VCAP_IS1,0x47)

/** 
 * \brief
 * Byte position for CUSTOM_DATA_6 and CUSTOM_DATA_7 in VCAP_S2.
 * 
 * See also
 * VCAP_IS1:VLAN_PAG_ACTION:MISC.CUSTOM_ACE_TYPE_ENA
 * VCAP_IS2:CUSTOM_*_ENTRY:ACE_CUSTOM_DATA_6 - 7.
 *
 * \details 
 * Byte position, starting from DMAC.
 * I.e. left-most byte of DMAC is byte 0.
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ACTION_CUSTOM_POS . CUSTOM_POS_VAL_3
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ACTION_CUSTOM_POS_CUSTOM_POS_VAL_3(x)  VTSS_ENCODE_BITFIELD(x,24,7)
#define  VTSS_M_VCAP_IS1_VLAN_PAG_ACTION_CUSTOM_POS_CUSTOM_POS_VAL_3     VTSS_ENCODE_BITMASK(24,7)
#define  VTSS_X_VCAP_IS1_VLAN_PAG_ACTION_CUSTOM_POS_CUSTOM_POS_VAL_3(x)  VTSS_EXTRACT_BITFIELD(x,24,7)

/** 
 * \brief
 * Byte position for CUSTOM_DATA_4 and CUSTOM_DATA_5 in VCAP_S2.
 * 
 * See also
 * VCAP_IS1:VLAN_PAG_ACTION:MISC.CUSTOM_ACE_TYPE_ENA
 * VCAP_IS2:CUSTOM_*_ENTRY:ACE_CUSTOM_DATA_4 - 5.
 *
 * \details 
 * Byte position, starting from DMAC.
 * I.e. left-most byte of DMAC is byte 0.
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ACTION_CUSTOM_POS . CUSTOM_POS_VAL_2
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ACTION_CUSTOM_POS_CUSTOM_POS_VAL_2(x)  VTSS_ENCODE_BITFIELD(x,16,7)
#define  VTSS_M_VCAP_IS1_VLAN_PAG_ACTION_CUSTOM_POS_CUSTOM_POS_VAL_2     VTSS_ENCODE_BITMASK(16,7)
#define  VTSS_X_VCAP_IS1_VLAN_PAG_ACTION_CUSTOM_POS_CUSTOM_POS_VAL_2(x)  VTSS_EXTRACT_BITFIELD(x,16,7)

/** 
 * \brief
 * Byte position for CUSTOM_DATA_2 and CUSTOM_DATA_3 in VCAP_S2.
 * 
 * See also
 * VCAP_IS1:VLAN_PAG_ACTION:MISC.CUSTOM_ACE_TYPE_ENA
 * VCAP_IS2:CUSTOM_*_ENTRY:ACE_CUSTOM_DATA_2 - 3.
 *
 * \details 
 * Byte position, starting from DMAC.
 * I.e. left-most byte of DMAC is byte 0.
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ACTION_CUSTOM_POS . CUSTOM_POS_VAL_1
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ACTION_CUSTOM_POS_CUSTOM_POS_VAL_1(x)  VTSS_ENCODE_BITFIELD(x,8,7)
#define  VTSS_M_VCAP_IS1_VLAN_PAG_ACTION_CUSTOM_POS_CUSTOM_POS_VAL_1     VTSS_ENCODE_BITMASK(8,7)
#define  VTSS_X_VCAP_IS1_VLAN_PAG_ACTION_CUSTOM_POS_CUSTOM_POS_VAL_1(x)  VTSS_EXTRACT_BITFIELD(x,8,7)

/** 
 * \brief
 * Byte position for CUSTOM_DATA_0 and CUSTOM_DATA_1 in VCAP_S2.
 * 
 * See also
 * VCAP_IS1:VLAN_PAG_ACTION:MISC.CUSTOM_ACE_TYPE_ENA
 * VCAP_IS2:CUSTOM_*_ENTRY:ACE_CUSTOM_DATA_0 - 1.
 *
 * \details 
 * Byte position, starting from DMAC.
 * I.e. left-most byte of DMAC is byte 0.
 *
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ACTION_CUSTOM_POS . CUSTOM_POS_VAL_0
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ACTION_CUSTOM_POS_CUSTOM_POS_VAL_0(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_VCAP_IS1_VLAN_PAG_ACTION_CUSTOM_POS_CUSTOM_POS_VAL_0     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_VCAP_IS1_VLAN_PAG_ACTION_CUSTOM_POS_CUSTOM_POS_VAL_0(x)  VTSS_EXTRACT_BITFIELD(x,0,7)


/** 
 * \details
 * Register: \a VCAP_IS1:VLAN_PAG_ACTION:ISDX
 */
#define VTSS_VCAP_IS1_VLAN_PAG_ACTION_ISDX   VTSS_IOREG(VTSS_TO_VCAP_IS1,0x48)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ACTION_ISDX . ISDX_ADD_VAL
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ACTION_ISDX_ISDX_ADD_VAL(x)  VTSS_ENCODE_BITFIELD(x,1,12)
#define  VTSS_M_VCAP_IS1_VLAN_PAG_ACTION_ISDX_ISDX_ADD_VAL     VTSS_ENCODE_BITMASK(1,12)
#define  VTSS_X_VCAP_IS1_VLAN_PAG_ACTION_ISDX_ISDX_ADD_VAL(x)  VTSS_EXTRACT_BITFIELD(x,1,12)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ACTION_ISDX . ISDX_REPLACE_ENA
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ACTION_ISDX_ISDX_REPLACE_ENA  VTSS_BIT(0)


/** 
 * \brief Sticky bit status register
 *
 * \details
 * Register: \a VCAP_IS1:VLAN_PAG_ACTION:STICKY
 */
#define VTSS_VCAP_IS1_VLAN_PAG_ACTION_STICKY  VTSS_IOREG(VTSS_TO_VCAP_IS1,0x49)

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
 * Field: ::VTSS_VCAP_IS1_VLAN_PAG_ACTION_STICKY . HIT_STICKY
 */
#define  VTSS_F_VCAP_IS1_VLAN_PAG_ACTION_STICKY_HIT_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a VCAP_IS1:QOS_ACTION
 *
 * ACL Stage 1 action registers
 */


/** 
 * \brief Custom ACE selection
 *
 * \details
 * Register: \a VCAP_IS1:QOS_ACTION:DSCP
 */
#define VTSS_VCAP_IS1_QOS_ACTION_DSCP        VTSS_IOREG(VTSS_TO_VCAP_IS1,0x4d)

/** 
 * \brief
 * DSCP value for advanced QOS classification.
 * 
 * Used for indirect access to QoS classification entries.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_QOS_ACTION_DSCP . DSCP_VAL
 */
#define  VTSS_F_VCAP_IS1_QOS_ACTION_DSCP_DSCP_VAL(x)  VTSS_ENCODE_BITFIELD(x,8,6)
#define  VTSS_M_VCAP_IS1_QOS_ACTION_DSCP_DSCP_VAL     VTSS_ENCODE_BITMASK(8,6)
#define  VTSS_X_VCAP_IS1_QOS_ACTION_DSCP_DSCP_VAL(x)  VTSS_EXTRACT_BITFIELD(x,8,6)

/** 
 * \brief
 * Enable / Disable Advanced DSCP classification (overriding decision from
 * Basic classification).
 * 
 * Used for indirect access to QoS classification entries.
 *
 * \details 
 * '0' : Disable
 * '1' : Enable
 *
 * Field: ::VTSS_VCAP_IS1_QOS_ACTION_DSCP . DSCP_ENA
 */
#define  VTSS_F_VCAP_IS1_QOS_ACTION_DSCP_DSCP_ENA  VTSS_BIT(0)


/** 
 * \brief Advanced Classification result
 *
 * \details
 * Register: \a VCAP_IS1:QOS_ACTION:QOS
 */
#define VTSS_VCAP_IS1_QOS_ACTION_QOS         VTSS_IOREG(VTSS_TO_VCAP_IS1,0x4e)

/** 
 * \brief
 * Advanced QOS Classification result.
 * 
 * Used for indirect access to QoS classification entries.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 *
 * Field: ::VTSS_VCAP_IS1_QOS_ACTION_QOS . QOS_VAL
 */
#define  VTSS_F_VCAP_IS1_QOS_ACTION_QOS_QOS_VAL(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_VCAP_IS1_QOS_ACTION_QOS_QOS_VAL     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_VCAP_IS1_QOS_ACTION_QOS_QOS_VAL(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * Enable / Disable Advanced QOS classification (overriding decision from
 * Basic classification).
 * 
 * Used for indirect access to QoS classification entries.
 *
 * \details 
 * '0' : Disable
 * '1' : Enable
 *
 * Field: ::VTSS_VCAP_IS1_QOS_ACTION_QOS . QOS_ENA
 */
#define  VTSS_F_VCAP_IS1_QOS_ACTION_QOS_QOS_ENA  VTSS_BIT(0)


/** 
 * \details
 * Register: \a VCAP_IS1:QOS_ACTION:DP
 */
#define VTSS_VCAP_IS1_QOS_ACTION_DP          VTSS_IOREG(VTSS_TO_VCAP_IS1,0x4f)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS1_QOS_ACTION_DP . DP_VAL
 */
#define  VTSS_F_VCAP_IS1_QOS_ACTION_DP_DP_VAL(x)  VTSS_ENCODE_BITFIELD(x,1,2)
#define  VTSS_M_VCAP_IS1_QOS_ACTION_DP_DP_VAL     VTSS_ENCODE_BITMASK(1,2)
#define  VTSS_X_VCAP_IS1_QOS_ACTION_DP_DP_VAL(x)  VTSS_EXTRACT_BITFIELD(x,1,2)

/** 
 * \details 
 * Field: ::VTSS_VCAP_IS1_QOS_ACTION_DP . DP_ENA
 */
#define  VTSS_F_VCAP_IS1_QOS_ACTION_DP_DP_ENA  VTSS_BIT(0)


/** 
 * \brief Sticky bit status register
 *
 * \details
 * Register: \a VCAP_IS1:QOS_ACTION:STICKY
 */
#define VTSS_VCAP_IS1_QOS_ACTION_STICKY      VTSS_IOREG(VTSS_TO_VCAP_IS1,0x50)

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
 * Field: ::VTSS_VCAP_IS1_QOS_ACTION_STICKY . HIT_STICKY
 */
#define  VTSS_F_VCAP_IS1_QOS_ACTION_STICKY_HIT_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a VCAP_IS1:TCAM_BIST
 *
 * Build in test for TCAM
 */


/** 
 * \brief Control of the TCAM
 *
 * \details
 * Register: \a VCAP_IS1:TCAM_BIST:TCAM_CTRL
 */
#define VTSS_VCAP_IS1_TCAM_BIST_TCAM_CTRL    VTSS_IOREG(VTSS_TO_VCAP_IS1,0x55)

/** 
 * \brief
 * Set this field to start manual BIST of the TCAM. This field will be
 * cleared once BIST is complete. The BIST procedure requires that the TCAM
 * is initialized before start, setting TCAM_INIT at the same time as
 * setting this field will first initalize the TCAM and then run BIST.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS1_TCAM_BIST_TCAM_CTRL . TCAM_BIST
 */
#define  VTSS_F_VCAP_IS1_TCAM_BIST_TCAM_CTRL_TCAM_BIST  VTSS_BIT(1)

/** 
 * \brief
 * Set this field to start manual initialization of the TCAM. This field is
 * cleared once initialization is complete.
 * The TCAM has random contents after reset and must be initialized prior
 * to usage.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS1_TCAM_BIST_TCAM_CTRL . TCAM_INIT
 */
#define  VTSS_F_VCAP_IS1_TCAM_BIST_TCAM_CTRL_TCAM_INIT  VTSS_BIT(0)


/** 
 * \brief Status for the TCAM
 *
 * \details
 * Register: \a VCAP_IS1:TCAM_BIST:TCAM_STAT
 */
#define VTSS_VCAP_IS1_TCAM_BIST_TCAM_STAT    VTSS_IOREG(VTSS_TO_VCAP_IS1,0x58)

/** 
 * \brief
 * Set if BIST failed.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS1_TCAM_BIST_TCAM_STAT . BIST_ERR
 */
#define  VTSS_F_VCAP_IS1_TCAM_BIST_TCAM_STAT_BIST_ERR  VTSS_BIT(2)

/** 
 * \brief
 * Set when the BIST is still running. When checking the BIST result this
 * field must be cleared.
 *
 * \details 
 * Field: ::VTSS_VCAP_IS1_TCAM_BIST_TCAM_STAT . BIST_BUSY
 */
#define  VTSS_F_VCAP_IS1_TCAM_BIST_TCAM_STAT_BIST_BUSY  VTSS_BIT(1)

/** 
 * \brief
 * Indicates the current operational state of the TCAM.
 *
 * \details 
 * '0': Busy with initialization.
 * '1': Ready to be used.
 *
 * Field: ::VTSS_VCAP_IS1_TCAM_BIST_TCAM_STAT . TCAM_RDY
 */
#define  VTSS_F_VCAP_IS1_TCAM_BIST_TCAM_STAT_TCAM_RDY  VTSS_BIT(0)


#endif /* _VTSS_JAGUAR_REGS_VCAP_IS1_H_ */
