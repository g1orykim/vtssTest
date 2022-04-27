#ifndef _VTSS_JAGUAR2_REGS_VCAP_CORE_H_
#define _VTSS_JAGUAR2_REGS_VCAP_CORE_H_

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
 * Target: \a VCAP_CORE
 *
 * \see vtss_target_VCAP_CORE_e
 *
 * Vitesse Content Aware Processor
 *
 ***********************************************************************/

/**
 * Register Group: \a VCAP_CORE:VCAP_CORE_CFG
 *
 * VCAP operations
 */


/** 
 * \brief Initiation of read/write/move/initialization operations
 *
 * \details
 * Register: \a VCAP_CORE:VCAP_CORE_CFG:VCAP_UPDATE_CTRL
 *
 * @param target A \a ::vtss_target_VCAP_CORE_e target
 */
#define VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL(target)  VTSS_IOREG(target,0x0)

/** 
 * \brief
 * Specifies the operation that will be performed when
 * VCAP_UPDATE_CTRL.UPDATE_SHOT is set. For all of the operations it is
 * possible to disable read/write of entries, actions, and/or counter by
 * setting VCAP_UPDATE_CTRL.UPDATE_ENTRY_DIS,
 * VCAP_UPDATE_CTRL.UPDATE_ACTION_DIS, and/or
 * VCAP_UPDATE_CTRL.UPDATE_CNT_DIS respectively. Writing/moving to
 * unimplemented addresses are ignored. Reading/moving from unimplemented
 * addresses returns never-match for entries, and zeros from
 * actions/counters.
 * 
 * For version 2 VCAPs: Active rules may only be written to empty
 * (initialized) entries; that is software must not overwrite one active
 * entry with a new entry. To empty one or more active rules; use the init
 * operation with VCAP_UPDATE_CTRL.UPDATE_ACTION_DIS,
 * VCAP_UPDATE_CTRL.UPDATE_CNT_DIS, and VCAP_UPDATE_CTRL.CLEAR_CACHE bits
 * set to '1'. Move operations automatically empty enties; so it is ok when
 * source and destination ranges overlap. Version 1 VCAPs does not restict
 * overwriting of active rules.
 * 
 * Write operation writes the contents of the cache to the VCAP address
 * specified by VCAP_UPDATE_CTRL.UPDATE_ADDR.
 * 
 * Read opeation reads from the VCAP address VCAP_UPDATE_CTRL.UPDATE_ADDR
 * into the cache.
 * 
 * Move up opeation moves one or more VCAP addresses from a high address to
 * a lower address, this is equivalent to decreasing priority of a rule.
 * The starting address is specified by VCAP_UPDATE_CTRL.UPDATE_ADDR, the
 * number of addresses (the range) that is moved is defined by
 * VCAP_MV_CFG.MV_SIZE, the distance to move is defined by VCAP_MV_CFG.
 * MV_NUM_POS. After moving an address the entry will be cleared (set to
 * never-match.) For version 2 VCAPs: When the counter-move is enabled then
 * counter-values are guaranteed to be consistent. When counter-move is
 * disabled then move operation is faster and destination counter-value
 * will be set to zero. For version 1 VCAPs; see
 * VCAP_UPDATE_CTRL.MV_TRAFFIC_IGN for more information.
 * 
 * Move down opeation moves one or more VCAP addresses from a low address
 * to a higer address, this is equivalent to increasing priority of a rule.
 * This operation is equivalent to "Move up" except for the direction that
 * it moves addresses, see "Move up" for more details.
 * 
 * Init operation writes the contents of the cache to one or more VCAP
 * addresses. The starting address is specified by
 * VCAP_UPDATE_CTRL.UPDATE_ADDR, the number of addresses (the range) that
 * is written is defined by VCAP_MV_CFG.MV_SIZE. Setting
 * VCAP_UPDATE_CTRL.CLEAR_CACHE at the same time as setting the operation
 * will clear the cache and cause the init operation to empty the range of
 * addresses.
 *
 * \details 
 * 000: Write from cache to VCAP
 * 001: Read from VCAP to cache
 * 010: Move entry and/or action up (decreasing addresses)
 * 011: Move entry and/or action down (increasing addresses)
 * 100: Initialize VCAP with the cache-value
 *
 * Field: ::VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL . UPDATE_CMD
 */
#define  VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_CMD(x)  VTSS_ENCODE_BITFIELD(x,22,3)
#define  VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_CMD     VTSS_ENCODE_BITMASK(22,3)
#define  VTSS_X_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_CMD(x)  VTSS_EXTRACT_BITFIELD(x,22,3)

/** 
 * \brief
 * Set to disable update of entries for VCAP operations: For
 * read-operations entry-cache will remain unchanged. For write/move/init
 * operations the VCAP-entry will remain unchanged.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL . UPDATE_ENTRY_DIS
 */
#define  VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_ENTRY_DIS  VTSS_BIT(21)

/** 
 * \brief
 * Set to disable update of actions for VCAP operations: For
 * read-operations action-cache will remain unchanged. For write/move/init
 * operations the VCAP-action will remain unchanged.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL . UPDATE_ACTION_DIS
 */
#define  VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_ACTION_DIS  VTSS_BIT(20)

/** 
 * \brief
 * Set to disable update of counter for VCAP operations: For
 * read-operations counter-cache will remain unchanged. For write/init
 * operations the VCAP-counter will remain unchanged. For move operations
 * the destination VCAP-counters will be set to zeros.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL . UPDATE_CNT_DIS
 */
#define  VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_CNT_DIS  VTSS_BIT(19)

/** 
 * \brief
 * The address to access for VCAP operations. For VCAPs that has defaults;
 * the default actions are placed after the normal actions, each default
 * action takes up one complete full-word.
 *
 * \details 
 * CLM0 defaults at 0x8000-0x8710
 * CLM1 defaults at 0x8720-0x8E30
 * CLM2 defaults at 0x8E40-0x9550
 * S2 defaults at 0x9560-0x98F0
 * ES0 defaults at 0x1000-0x1034
 *
 * Field: ::VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL . UPDATE_ADDR
 */
#define  VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_ADDR(x)  VTSS_ENCODE_BITFIELD(x,3,16)
#define  VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_ADDR     VTSS_ENCODE_BITMASK(3,16)
#define  VTSS_X_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_ADDR(x)  VTSS_EXTRACT_BITFIELD(x,3,16)

/** 
 * \brief
 * Set to initiate the opeation specified in VCAP_UPDATE_CTRL.UPDATE_CMD.
 * This bit is automatically cleared by hardware when the operation is
 * finished.
 * Software must not change write fields in the VCAP target while this
 * field is set (while operation is active.)
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL . UPDATE_SHOT
 */
#define  VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_SHOT  VTSS_BIT(2)

/** 
 * \brief
 * Set to clear the cache. This field is cleared immediately by hardware
 * (at the same time as clearing the cache).
 * For version 1 VCAPs the entry/mask is set to match-0 encoding. Action
 * and counter will be set to zeros.
 * For version 2 VCAPs the entry/mask is set to match-off endcoding. Action
 * and counter will be set to zeros.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL . CLEAR_CACHE
 */
#define  VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_CLEAR_CACHE  VTSS_BIT(1)

/** 
 * \brief
 * For version 1 VCAPs: Set to ignore interrupting traffic during move
 * operations, this will increase speed of the move operations but
 * counter-events may be lost for the VCAP addresses that are moved. When
 * this field is cleared, then interrupting traffic will cause a restart of
 * the move operation (to ensure consistent counter values) and becasue of
 * this, move operations on a heavily loaded device may take a long time to
 * finish.
 * This field is not used for version 2 VCAPs, and value is don't care.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL . MV_TRAFFIC_IGN
 */
#define  VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_MV_TRAFFIC_IGN  VTSS_BIT(0)


/** 
 * \brief Configuration of move/initialization
 *
 * \details
 * Register: \a VCAP_CORE:VCAP_CORE_CFG:VCAP_MV_CFG
 *
 * @param target A \a ::vtss_target_VCAP_CORE_e target
 */
#define VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_MV_CFG(target)  VTSS_IOREG(target,0x1)

/** 
 * \brief
 * Specifies the distance to move moved during move operations. I.e. if
 * this field is set to 4 for a move-down operation, then source address n
 * is moved to destination address n+5.
 *
 * \details 
 * 0: Distance is one position
 * 1: Distance is two positions
 * n: Distance is n+1 positions
 *
 * Field: ::VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_MV_CFG . MV_NUM_POS
 */
#define  VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_MV_CFG_MV_NUM_POS(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_MV_CFG_MV_NUM_POS     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_VCAP_CORE_VCAP_CORE_CFG_VCAP_MV_CFG_MV_NUM_POS(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Specifies the number of addresses to moved/initialize during  move/init
 * operations.
 *
 * \details 
 * 0: Address VCAP_UPDATE_CTRL.UPDATE_ADDR is moved/initialized
 * 1: Addresses VCAP_UPDATE_CTRL.UPDATE_ADDR and
 * VCAP_UPDATE_CTRL.UPDATE_ADDR+1 are moved/initialized
 * n: Addresses VCAP_UPDATE_CTRL.UPDATE_ADDR through
 * VCAP_UPDATE_CTRL.UPDATE_ADDR+n are moved/initialized
 *
 * Field: ::VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_MV_CFG . MV_SIZE
 */
#define  VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_MV_CFG_MV_SIZE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_MV_CFG_MV_SIZE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VCAP_CORE_VCAP_CORE_CFG_VCAP_MV_CFG_MV_SIZE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VCAP_CORE:VCAP_CORE_CACHE
 *
 * VCAP cache
 */


/** 
 * \brief Entry data cache
 *
 * \details
 * Register: \a VCAP_CORE:VCAP_CORE_CACHE:VCAP_ENTRY_DAT
 *
 * @param target A \a ::vtss_target_VCAP_CORE_e target
 * @param ri Register: VCAP_ENTRY_DAT (??), 0-63
 */
#define VTSS_VCAP_CORE_VCAP_CORE_CACHE_VCAP_ENTRY_DAT(target,ri)  VTSS_IOREG(target,0x2 + (ri))


/** 
 * \brief Entry mask cache
 *
 * \details
 * Register: \a VCAP_CORE:VCAP_CORE_CACHE:VCAP_MASK_DAT
 *
 * @param target A \a ::vtss_target_VCAP_CORE_e target
 * @param ri Register: VCAP_MASK_DAT (??), 0-63
 */
#define VTSS_VCAP_CORE_VCAP_CORE_CACHE_VCAP_MASK_DAT(target,ri)  VTSS_IOREG(target,0x42 + (ri))


/** 
 * \brief Action cache
 *
 * \details
 * Register: \a VCAP_CORE:VCAP_CORE_CACHE:VCAP_ACTION_DAT
 *
 * @param target A \a ::vtss_target_VCAP_CORE_e target
 * @param ri Register: VCAP_ACTION_DAT (??), 0-63
 */
#define VTSS_VCAP_CORE_VCAP_CORE_CACHE_VCAP_ACTION_DAT(target,ri)  VTSS_IOREG(target,0x82 + (ri))


/** 
 * \brief Counter cache
 *
 * \details
 * Register: \a VCAP_CORE:VCAP_CORE_CACHE:VCAP_CNT_DAT
 *
 * @param target A \a ::vtss_target_VCAP_CORE_e target
 * @param ri Register: VCAP_CNT_DAT (??), 0-31
 */
#define VTSS_VCAP_CORE_VCAP_CORE_CACHE_VCAP_CNT_DAT(target,ri)  VTSS_IOREG(target,0xc2 + (ri))


/** 
 * \brief Entry type-group
 *
 * \details
 * Register: \a VCAP_CORE:VCAP_CORE_CACHE:VCAP_TG_DAT
 *
 * @param target A \a ::vtss_target_VCAP_CORE_e target
 */
#define VTSS_VCAP_CORE_VCAP_CORE_CACHE_VCAP_TG_DAT(target)  VTSS_IOREG(target,0xe2)

/**
 * Register Group: \a VCAP_CORE:VCAP_CORE_MAP
 *
 * Mapping of cores to interfaces
 */


/** 
 * \brief Core index
 *
 * \details
 * Register: \a VCAP_CORE:VCAP_CORE_MAP:VCAP_CORE_IDX
 *
 * @param target A \a ::vtss_target_VCAP_CORE_e target
 */
#define VTSS_VCAP_CORE_VCAP_CORE_MAP_VCAP_CORE_IDX(target)  VTSS_IOREG(target,0xe3)

/** 
 * \brief
 * Set to a value in the range 0 though (VCAP_CORE::CORE_CNT-1) to access
 * the mapping of that core (via VCAP_CORE::VCAP_CORE_MAP.)
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_VCAP_CORE_MAP_VCAP_CORE_IDX . CORE_IDX
 */
#define  VTSS_F_VCAP_CORE_VCAP_CORE_MAP_VCAP_CORE_IDX_CORE_IDX(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_VCAP_CORE_VCAP_CORE_MAP_VCAP_CORE_IDX_CORE_IDX     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_VCAP_CORE_VCAP_CORE_MAP_VCAP_CORE_IDX_CORE_IDX(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief Mapping of core
 *
 * \details
 * Register: \a VCAP_CORE:VCAP_CORE_MAP:VCAP_CORE_MAP
 *
 * @param target A \a ::vtss_target_VCAP_CORE_e target
 */
#define VTSS_VCAP_CORE_VCAP_CORE_MAP_VCAP_CORE_MAP(target)  VTSS_IOREG(target,0xe4)

/** 
 * \brief
 * Configure ownership of core n (defined by VCAP_CORE::VCAP_CORE_IDX),
 * ownership is a number in the range 0 though (VCAP_CORE::IF_CNT-1).
 * When a core is mapped to a specific interface; lookups from that
 * interface will be applied to the core. VCAP priority is still observed,
 * a match in two cores will only cause the most significant rule to be
 * "hit" (highest address.)
 * By default all cores are in power-save mode (owner = 0), reading and
 * writing is possible during power-save.
 *
 * \details 
 * Power-save: 0
 * CLM0: 1
 * CLM1: 2
 * CLM2: 3
 * S2: 4
 * LPM: 5
 *
 * Field: ::VTSS_VCAP_CORE_VCAP_CORE_MAP_VCAP_CORE_MAP . CORE_MAP
 */
#define  VTSS_F_VCAP_CORE_VCAP_CORE_MAP_VCAP_CORE_MAP_CORE_MAP(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_VCAP_CORE_VCAP_CORE_MAP_VCAP_CORE_MAP_CORE_MAP     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_VCAP_CORE_VCAP_CORE_MAP_VCAP_CORE_MAP_CORE_MAP(x)  VTSS_EXTRACT_BITFIELD(x,0,3)

/**
 * Register Group: \a VCAP_CORE:VCAP_CORE_STICKY
 *
 * Sticky bits
 */


/** 
 * \brief Debug information
 *
 * \details
 * Register: \a VCAP_CORE:VCAP_CORE_STICKY:VCAP_STICKY
 *
 * @param target A \a ::vtss_target_VCAP_CORE_e target
 */
#define VTSS_VCAP_CORE_VCAP_CORE_STICKY_VCAP_STICKY(target)  VTSS_IOREG(target,0xe5)

/** 
 * \brief
 * A move operation has resulted in deleting of one or more rules.
 * This field applies only to version 1 VCAPs, for version 2 VCAPs it is
 * not implemented and reading it will return zero.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_VCAP_CORE_STICKY_VCAP_STICKY . VCAP_ROW_DELETED_STICKY
 */
#define  VTSS_F_VCAP_CORE_VCAP_CORE_STICKY_VCAP_STICKY_VCAP_ROW_DELETED_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a VCAP_CORE:VCAP_CONST
 *
 * VCAP constants
 */


/** 
 * \brief VCAP version
 *
 * \details
 * Register: \a VCAP_CORE:VCAP_CONST:VCAP_VER
 *
 * @param target A \a ::vtss_target_VCAP_CORE_e target
 */
#define VTSS_VCAP_CORE_VCAP_CONST_VCAP_VER(target)  VTSS_IOREG(target,0xe6)


/** 
 * \brief Entry width
 *
 * \details
 * Register: \a VCAP_CORE:VCAP_CONST:ENTRY_WIDTH
 *
 * @param target A \a ::vtss_target_VCAP_CORE_e target
 */
#define VTSS_VCAP_CORE_VCAP_CONST_ENTRY_WIDTH(target)  VTSS_IOREG(target,0xe7)


/** 
 * \brief Number of full-word entries
 *
 * \details
 * Register: \a VCAP_CORE:VCAP_CONST:ENTRY_CNT
 *
 * @param target A \a ::vtss_target_VCAP_CORE_e target
 */
#define VTSS_VCAP_CORE_VCAP_CONST_ENTRY_CNT(target)  VTSS_IOREG(target,0xe8)


/** 
 * \brief Number of subwords per full-word entry
 *
 * \details
 * Register: \a VCAP_CORE:VCAP_CONST:ENTRY_SWCNT
 *
 * @param target A \a ::vtss_target_VCAP_CORE_e target
 */
#define VTSS_VCAP_CORE_VCAP_CONST_ENTRY_SWCNT(target)  VTSS_IOREG(target,0xe9)


/** 
 * \brief Entry type-group width
 *
 * \details
 * Register: \a VCAP_CORE:VCAP_CONST:ENTRY_TG_WIDTH
 *
 * @param target A \a ::vtss_target_VCAP_CORE_e target
 */
#define VTSS_VCAP_CORE_VCAP_CONST_ENTRY_TG_WIDTH(target)  VTSS_IOREG(target,0xea)


/** 
 * \brief Number of defaults
 *
 * \details
 * Register: \a VCAP_CORE:VCAP_CONST:ACTION_DEF_CNT
 *
 * @param target A \a ::vtss_target_VCAP_CORE_e target
 */
#define VTSS_VCAP_CORE_VCAP_CONST_ACTION_DEF_CNT(target)  VTSS_IOREG(target,0xeb)


/** 
 * \brief Action width
 *
 * \details
 * Register: \a VCAP_CORE:VCAP_CONST:ACTION_WIDTH
 *
 * @param target A \a ::vtss_target_VCAP_CORE_e target
 */
#define VTSS_VCAP_CORE_VCAP_CONST_ACTION_WIDTH(target)  VTSS_IOREG(target,0xec)


/** 
 * \brief Counter width
 *
 * \details
 * Register: \a VCAP_CORE:VCAP_CONST:CNT_WIDTH
 *
 * @param target A \a ::vtss_target_VCAP_CORE_e target
 */
#define VTSS_VCAP_CORE_VCAP_CONST_CNT_WIDTH(target)  VTSS_IOREG(target,0xed)


/** 
 * \brief Number of cores
 *
 * \details
 * Register: \a VCAP_CORE:VCAP_CONST:CORE_CNT
 *
 * @param target A \a ::vtss_target_VCAP_CORE_e target
 */
#define VTSS_VCAP_CORE_VCAP_CONST_CORE_CNT(target)  VTSS_IOREG(target,0xee)


/** 
 * \brief Number of VCAP interfaces
 *
 * \details
 * Register: \a VCAP_CORE:VCAP_CONST:IF_CNT
 *
 * @param target A \a ::vtss_target_VCAP_CORE_e target
 */
#define VTSS_VCAP_CORE_VCAP_CONST_IF_CNT(target)  VTSS_IOREG(target,0xef)

/**
 * Register Group: \a VCAP_CORE:TCAM_BIST
 *
 * Build in test for TCAM
 */


/** 
 * \brief Control of the TCAM
 *
 * \details
 * Register: \a VCAP_CORE:TCAM_BIST:TCAM_CTRL
 *
 * @param target A \a ::vtss_target_VCAP_CORE_e target
 */
#define VTSS_VCAP_CORE_TCAM_BIST_TCAM_CTRL(target)  VTSS_IOREG(target,0xf0)

/** 
 * \brief
 * Set this field to start manual BIST of the TCAM. This field will be
 * cleared once BIST is complete. The BIST procedure requires that the TCAM
 * is initialized before start, setting TCAM_INIT at the same time as
 * setting this field will first initalize the TCAM and then run BIST.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_TCAM_BIST_TCAM_CTRL . TCAM_BIST
 */
#define  VTSS_F_VCAP_CORE_TCAM_BIST_TCAM_CTRL_TCAM_BIST  VTSS_BIT(1)

/** 
 * \brief
 * Set this field to start manual initialization of the TCAM. This field is
 * cleared once initialization is complete.
 * The TCAM has random contents after reset and must be initialized prior
 * to usage.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_TCAM_BIST_TCAM_CTRL . TCAM_INIT
 */
#define  VTSS_F_VCAP_CORE_TCAM_BIST_TCAM_CTRL_TCAM_INIT  VTSS_BIT(0)


/** 
 * \brief Configuration of the TCAM
 *
 * \details
 * Register: \a VCAP_CORE:TCAM_BIST:TCAM_CFG
 *
 * @param target A \a ::vtss_target_VCAP_CORE_e target
 */
#define VTSS_VCAP_CORE_TCAM_BIST_TCAM_CFG(target)  VTSS_IOREG(target,0xf1)

/** 
 * \brief
 * Configures the BIST engine to stop testing if an error is detected. This
 * is a debug feature that leaves the contents of the TCAM at the point of
 * the failure, thusly allowing debug of the failing pattern via normal R/W
 * to the TCAM.
 * Please note that because the bist is pipelilned, the contents of the
 * TCAM may have been modified a few times since the actual error.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_TCAM_BIST_TCAM_CFG . TCAM_BIST_SOE_ENA
 */
#define  VTSS_F_VCAP_CORE_TCAM_BIST_TCAM_CFG_TCAM_BIST_SOE_ENA  VTSS_BIT(8)

/** 
 * \brief
 * Disable clock-gating on previous stage hit in the TCAM, clock-gating on
 * previous stage hit is implemented for conserving power when a hit is
 * found in an early pipeline stage.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_TCAM_BIST_TCAM_CFG . TCAM_HCG_DIS
 */
#define  VTSS_F_VCAP_CORE_TCAM_BIST_TCAM_CFG_TCAM_HCG_DIS  VTSS_BIT(7)

/** 
 * \brief
 * Disable clock-gating in the TCAM, clock-gating is implemented for
 * conserving power while the TCAMs are IDLE.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_TCAM_BIST_TCAM_CFG . TCAM_CG_DIS
 */
#define  VTSS_F_VCAP_CORE_TCAM_BIST_TCAM_CFG_TCAM_CG_DIS  VTSS_BIT(6)

/** 
 * \brief
 * Bias setting for the TCAM keeper circuit, configure as described in
 * datasheet.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_TCAM_BIST_TCAM_CFG . TCAM_BIAS
 */
#define  VTSS_F_VCAP_CORE_TCAM_BIST_TCAM_CFG_TCAM_BIAS(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_VCAP_CORE_TCAM_BIST_TCAM_CFG_TCAM_BIAS     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_VCAP_CORE_TCAM_BIST_TCAM_CFG_TCAM_BIAS(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief Status for the TCAM
 *
 * \details
 * Register: \a VCAP_CORE:TCAM_BIST:TCAM_STAT
 *
 * @param target A \a ::vtss_target_VCAP_CORE_e target
 */
#define VTSS_VCAP_CORE_TCAM_BIST_TCAM_STAT(target)  VTSS_IOREG(target,0xf2)

/** 
 * \brief
 * Set if BIST sub-procedure Read-Tree failed.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_TCAM_BIST_TCAM_STAT . BIST_RT_ERR
 */
#define  VTSS_F_VCAP_CORE_TCAM_BIST_TCAM_STAT_BIST_RT_ERR  VTSS_BIT(15)

/** 
 * \brief
 * Set if BIST sub-procedure Priority-Encode failed.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_TCAM_BIST_TCAM_STAT . BIST_PENC_ERR
 */
#define  VTSS_F_VCAP_CORE_TCAM_BIST_TCAM_STAT_BIST_PENC_ERR  VTSS_BIT(14)

/** 
 * \brief
 * Set if BIST sub-procedure Compare failed.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_TCAM_BIST_TCAM_STAT . BIST_COMP_ERR
 */
#define  VTSS_F_VCAP_CORE_TCAM_BIST_TCAM_STAT_BIST_COMP_ERR  VTSS_BIT(13)

/** 
 * \brief
 * Set if BIST sub-procedure Address Decode failed.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_TCAM_BIST_TCAM_STAT . BIST_ADDR_ERR
 */
#define  VTSS_F_VCAP_CORE_TCAM_BIST_TCAM_STAT_BIST_ADDR_ERR  VTSS_BIT(12)

/** 
 * \brief
 * Set if BIST sub-procedure Bit-Line-1 (extended) failed.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_TCAM_BIST_TCAM_STAT . BIST_BL1E_ERR
 */
#define  VTSS_F_VCAP_CORE_TCAM_BIST_TCAM_STAT_BIST_BL1E_ERR  VTSS_BIT(11)

/** 
 * \brief
 * Set if BIST sub-procedure Bit-Line-1 failed.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_TCAM_BIST_TCAM_STAT . BIST_BL1_ERR
 */
#define  VTSS_F_VCAP_CORE_TCAM_BIST_TCAM_STAT_BIST_BL1_ERR  VTSS_BIT(10)

/** 
 * \brief
 * Set if BIST sub-procedure Bit-Line-0 (extended) failed.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_TCAM_BIST_TCAM_STAT . BIST_BL0E_ERR
 */
#define  VTSS_F_VCAP_CORE_TCAM_BIST_TCAM_STAT_BIST_BL0E_ERR  VTSS_BIT(9)

/** 
 * \brief
 * Set if BIST sub-procedure Bit-Line-0 failed.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_TCAM_BIST_TCAM_STAT . BIST_BL0_ERR
 */
#define  VTSS_F_VCAP_CORE_TCAM_BIST_TCAM_STAT_BIST_BL0_ERR  VTSS_BIT(8)

/** 
 * \brief
 * Set if BIST sub-procedure Pattern-Horizontal-1 failed.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_TCAM_BIST_TCAM_STAT . BIST_PH1_ERR
 */
#define  VTSS_F_VCAP_CORE_TCAM_BIST_TCAM_STAT_BIST_PH1_ERR  VTSS_BIT(7)

/** 
 * \brief
 * Set if BIST sub-procedure Pattern-Horizontal-0 failed.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_TCAM_BIST_TCAM_STAT . BIST_PH0_ERR
 */
#define  VTSS_F_VCAP_CORE_TCAM_BIST_TCAM_STAT_BIST_PH0_ERR  VTSS_BIT(6)

/** 
 * \brief
 * Set if BIST sub-procedure Pattern-Vertical-1 failed.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_TCAM_BIST_TCAM_STAT . BIST_PV1_ERR
 */
#define  VTSS_F_VCAP_CORE_TCAM_BIST_TCAM_STAT_BIST_PV1_ERR  VTSS_BIT(5)

/** 
 * \brief
 * Set if BIST sub-procedure Pattern-Vertical-0 failed.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_TCAM_BIST_TCAM_STAT . BIST_PV0_ERR
 */
#define  VTSS_F_VCAP_CORE_TCAM_BIST_TCAM_STAT_BIST_PV0_ERR  VTSS_BIT(4)

/** 
 * \brief
 * Set when the BIST has been run. When checking the BIST result this field
 * must be set.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_TCAM_BIST_TCAM_STAT . BIST_RUN
 */
#define  VTSS_F_VCAP_CORE_TCAM_BIST_TCAM_STAT_BIST_RUN  VTSS_BIT(3)

/** 
 * \brief
 * Set if BIST failed.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_TCAM_BIST_TCAM_STAT . BIST_ERR
 */
#define  VTSS_F_VCAP_CORE_TCAM_BIST_TCAM_STAT_BIST_ERR  VTSS_BIT(2)

/** 
 * \brief
 * Set when the BIST is still running. When checking the BIST result this
 * field must be cleared.
 *
 * \details 
 * Field: ::VTSS_VCAP_CORE_TCAM_BIST_TCAM_STAT . BIST_BUSY
 */
#define  VTSS_F_VCAP_CORE_TCAM_BIST_TCAM_STAT_BIST_BUSY  VTSS_BIT(1)

/** 
 * \brief
 * Indicates the current operational state of the TCAM.
 *
 * \details 
 * '0': Busy with initialization.
 * '1': Ready to be used.
 *
 * Field: ::VTSS_VCAP_CORE_TCAM_BIST_TCAM_STAT . TCAM_RDY
 */
#define  VTSS_F_VCAP_CORE_TCAM_BIST_TCAM_STAT_TCAM_RDY  VTSS_BIT(0)


#endif /* _VTSS_JAGUAR2_REGS_VCAP_CORE_H_ */
