#ifndef _VTSS_DAYTONA_REGS_DEVCPU_ORG_H_
#define _VTSS_DAYTONA_REGS_DEVCPU_ORG_H_

/*

 Vitesse Switch API software.

 Copyright (c) 2002-2010 Vitesse Semiconductor Corporation "Vitesse". All
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

#include "vtss_daytona_regs_common.h"

/*********************************************************************** 
 *
 * Target: \a DEVCPU_ORG
 *
 * Origin in CPU device
 *
 ***********************************************************************/

/**
 * Register Group: \a DEVCPU_ORG::ORG
 *
 * Origin registers
 */


/** 
 * \brief Unknown target module ID 1
 *
 * \details
 * Register: \a DEVCPU_ORG::ORG::ERR_ACCESS_DROP_1
 */
#define VTSS_DEVCPU_ORG_ORG_ERR_ACCESS_DROP_1  VTSS_IOREG(VTSS_TO_CFG_IF_ORG,0x0)

/** 
 * \brief
 * Interrupt mask for UTM_STICKY
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_DEVCPU_ORG_ORG_ERR_ACCESS_DROP_1 . UTM_INT_MASK
 */
#define  VTSS_F_DEVCPU_ORG_ORG_ERR_ACCESS_DROP_1_UTM_INT_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_DEVCPU_ORG_ORG_ERR_ACCESS_DROP_1_UTM_INT_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_DEVCPU_ORG_ORG_ERR_ACCESS_DROP_1_UTM_INT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Sticky bit that indicates at least one request for an unknown target
 * module has been done. Bit is cleared by writing a 1 to this position.
 *
 * \details 
 * 0: No errors occurred
 * 1: At least one request to an unknown target has been done
 *
 * Field: ::VTSS_DEVCPU_ORG_ORG_ERR_ACCESS_DROP_1 . UTM_STICKY
 */
#define  VTSS_F_DEVCPU_ORG_ORG_ERR_ACCESS_DROP_1_UTM_STICKY(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_DEVCPU_ORG_ORG_ERR_ACCESS_DROP_1_UTM_STICKY     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_DEVCPU_ORG_ORG_ERR_ACCESS_DROP_1_UTM_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Target module ID
 * When the UTM_STICKY bit is set, this field holds the ID of the last
 * target that was unknown
 *
 * \details 
 * 0x01: Module ID 1 
 * 0xFF: Module ID 255
 *
 * Field: ::VTSS_DEVCPU_ORG_ORG_ERR_ACCESS_DROP_1 . TGT_MODULE_UTM_STICKY
 */
#define  VTSS_F_DEVCPU_ORG_ORG_ERR_ACCESS_DROP_1_TGT_MODULE_UTM_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_DEVCPU_ORG_ORG_ERR_ACCESS_DROP_1_TGT_MODULE_UTM_STICKY     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_DEVCPU_ORG_ORG_ERR_ACCESS_DROP_1_TGT_MODULE_UTM_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Unknown target module ID 2
 *
 * \details
 * Register: \a DEVCPU_ORG::ORG::ERR_ACCESS_DROP_2
 */
#define VTSS_DEVCPU_ORG_ORG_ERR_ACCESS_DROP_2  VTSS_IOREG(VTSS_TO_CFG_IF_ORG,0x1)

/** 
 * \brief
 * Interrupt mask for NO_ACTION_STICKY
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_DEVCPU_ORG_ORG_ERR_ACCESS_DROP_2 . NO_ACTION_INT_MASK
 */
#define  VTSS_F_DEVCPU_ORG_ORG_ERR_ACCESS_DROP_2_NO_ACTION_INT_MASK(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_DEVCPU_ORG_ORG_ERR_ACCESS_DROP_2_NO_ACTION_INT_MASK     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_DEVCPU_ORG_ORG_ERR_ACCESS_DROP_2_NO_ACTION_INT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Sticky bit that indicates that at least one request was received by a
 * target, but the target didn't do anything with it ( e.g. access to a non
 * existing register). Bit is cleared by writing a 1 to this position.
 *
 * \details 
 * 0: No errors occurred
 * 1: At least one request was received with no action
 *
 * Field: ::VTSS_DEVCPU_ORG_ORG_ERR_ACCESS_DROP_2 . NO_ACTION_STICKY
 */
#define  VTSS_F_DEVCPU_ORG_ORG_ERR_ACCESS_DROP_2_NO_ACTION_STICKY(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_DEVCPU_ORG_ORG_ERR_ACCESS_DROP_2_NO_ACTION_STICKY     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_DEVCPU_ORG_ORG_ERR_ACCESS_DROP_2_NO_ACTION_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Target module ID
 * When the NO_ACTION_STICKY bit is set, this field holds the ID of the
 * last target that received a request that didn't resolve in an action
 *
 * \details 
 * 0x01: Module ID 1 
 * 0xFF: Module ID 255
 *
 * Field: ::VTSS_DEVCPU_ORG_ORG_ERR_ACCESS_DROP_2 . TGT_MODULE_NO_ACTION_STICKY
 */
#define  VTSS_F_DEVCPU_ORG_ORG_ERR_ACCESS_DROP_2_TGT_MODULE_NO_ACTION_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_DEVCPU_ORG_ORG_ERR_ACCESS_DROP_2_TGT_MODULE_NO_ACTION_STICKY     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_DEVCPU_ORG_ORG_ERR_ACCESS_DROP_2_TGT_MODULE_NO_ACTION_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Target module busy, MSB
 *
 * \details
 * Write all ones to this register to clear it
 *
 * Register: \a DEVCPU_ORG::ORG::ERR_TGT_MSB
 */
#define VTSS_DEVCPU_ORG_ORG_ERR_TGT_MSB      VTSS_IOREG(VTSS_TO_CFG_IF_ORG,0x3)

/** 
 * \brief
 * Resets the origin block
 *
 * \details 
 * 0: No reset
 * 1: Reset (auto cleared)
 *
 * Field: ::VTSS_DEVCPU_ORG_ORG_ERR_TGT_MSB . ORG_RST
 */
#define  VTSS_F_DEVCPU_ORG_ORG_ERR_TGT_MSB_ORG_RST(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_DEVCPU_ORG_ORG_ERR_TGT_MSB_ORG_RST     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_DEVCPU_ORG_ORG_ERR_TGT_MSB_ORG_RST(x)  VTSS_EXTRACT_BITFIELD(x,9,1)


/** 
 * \brief Target module busy, LSB
 *
 * \details
 * Write all ones to this register to clear it.
 *
 * Register: \a DEVCPU_ORG::ORG::ERR_TGT_LSB
 */
#define VTSS_DEVCPU_ORG_ORG_ERR_TGT_LSB      VTSS_IOREG(VTSS_TO_CFG_IF_ORG,0x4)

/** 
 * \brief
 * Sticky bit that indicates at least one request was not processed because
 * the target dropped the request due to watchdog time out. Bit is cleared
 * by writing a 1 to this position.
 *
 * \details 
 * 0: No error has occurred 
 * 1: A least one request was dropped due to watchdog time out
 *
 * Field: ::VTSS_DEVCPU_ORG_ORG_ERR_TGT_LSB . WD_STICKY
 */
#define  VTSS_F_DEVCPU_ORG_ORG_ERR_TGT_LSB_WD_STICKY(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_DEVCPU_ORG_ORG_ERR_TGT_LSB_WD_STICKY     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_DEVCPU_ORG_ORG_ERR_TGT_LSB_WD_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Target module ID
 * When the WD_STICKY bit is set, this field holds the ID of the last
 * target that was unable to process a request due to watchdog time out
 *
 * \details 
 * 0x01: Module ID 1 
 * 0xFF: Module ID 255
 *
 * Field: ::VTSS_DEVCPU_ORG_ORG_ERR_TGT_LSB . TGT_MODULE_WD_DROP
 */
#define  VTSS_F_DEVCPU_ORG_ORG_ERR_TGT_LSB_TGT_MODULE_WD_DROP(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_DEVCPU_ORG_ORG_ERR_TGT_LSB_TGT_MODULE_WD_DROP     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_DEVCPU_ORG_ORG_ERR_TGT_LSB_TGT_MODULE_WD_DROP(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Target error interrupt mask registers
 *
 * \details
 * Register: \a DEVCPU_ORG::ORG::ERR_TGT_INT_MASK
 */
#define VTSS_DEVCPU_ORG_ORG_ERR_TGT_INT_MASK  VTSS_IOREG(VTSS_TO_CFG_IF_ORG,0x5)

/** 
 * \brief
 * Interrupt mask for WD_STICKY
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_DEVCPU_ORG_ORG_ERR_TGT_INT_MASK . WD_STICKY_INT_MASK
 */
#define  VTSS_F_DEVCPU_ORG_ORG_ERR_TGT_INT_MASK_WD_STICKY_INT_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_DEVCPU_ORG_ORG_ERR_TGT_INT_MASK_WD_STICKY_INT_MASK     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_DEVCPU_ORG_ORG_ERR_TGT_INT_MASK_WD_STICKY_INT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Error counters 1
 *
 * \details
 * Register: \a DEVCPU_ORG::ORG::ERR_CNTS_1
 */
#define VTSS_DEVCPU_ORG_ORG_ERR_CNTS_1       VTSS_IOREG(VTSS_TO_CFG_IF_ORG,0x6)

/** 
 * \brief
 * Busy counter
 * Counts the number of requests that were not processed by the target
 * module, because it was busy. This may be because the target module was
 * waiting for access to/from its host.
 * This counter wraps around.
 *
 * \details 
 * 0x00: No requests not processed 
 * 0x01: 1 request not processed 
 * . . . 
 * 0xFF: 255 requests not processed
 *
 * Field: ::VTSS_DEVCPU_ORG_ORG_ERR_CNTS_1 . BUSY_CNT
 */
#define  VTSS_F_DEVCPU_ORG_ORG_ERR_CNTS_1_BUSY_CNT(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_DEVCPU_ORG_ORG_ERR_CNTS_1_BUSY_CNT     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_DEVCPU_ORG_ORG_ERR_CNTS_1_BUSY_CNT(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Watchdog counter
 * Counts the number of requests that were not processed by the target
 * module, because the targets watchdog timed out. This should only happen
 * if an attempt is made to access an invalid register location in a valid
 * target.
 * This counter wraps around.
 *
 * \details 
 * 0x00: No requests not processed 
 * 0x01: 1 request not processed 
 * . . . 
 * 0xFF: 255 requests not processed
 *
 * Field: ::VTSS_DEVCPU_ORG_ORG_ERR_CNTS_1 . WD_DROP_CNT
 */
#define  VTSS_F_DEVCPU_ORG_ORG_ERR_CNTS_1_WD_DROP_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_DEVCPU_ORG_ORG_ERR_CNTS_1_WD_DROP_CNT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_DEVCPU_ORG_ORG_ERR_CNTS_1_WD_DROP_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Error counters 2
 *
 * \details
 * Register: \a DEVCPU_ORG::ORG::ERR_CNTS_2
 */
#define VTSS_DEVCPU_ORG_ORG_ERR_CNTS_2       VTSS_IOREG(VTSS_TO_CFG_IF_ORG,0x7)

/** 
 * \brief
 * No action counter
 * Counts the number of requests that were not processed by the target
 * module, because the target did not know what to do ( e.g. access to a
 * non existing register ).
 * This counter wraps around and can be written by SW.
 *
 * \details 
 * 0x00: No requests not processed 
 * 0x01: 1 request not processed 
 * . . . 
 * 0xFF: 255 requests not processed
 *
 * Field: ::VTSS_DEVCPU_ORG_ORG_ERR_CNTS_2 . NO_ACTION_CNT
 */
#define  VTSS_F_DEVCPU_ORG_ORG_ERR_CNTS_2_NO_ACTION_CNT(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_DEVCPU_ORG_ORG_ERR_CNTS_2_NO_ACTION_CNT     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_DEVCPU_ORG_ORG_ERR_CNTS_2_NO_ACTION_CNT(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Unknown target counter
 * Counts the number of requests that were not processed by the target
 * module, because the target was no found.
 * This counter wraps around and can be written by SW.
 *
 * \details 
 * 0x00: No requests not processed 
 * 0x01: 1 request not processed 
 * . . . 
 * 0xFF: 255 requests not processed
 *
 * Field: ::VTSS_DEVCPU_ORG_ORG_ERR_CNTS_2 . UTM_CNT
 */
#define  VTSS_F_DEVCPU_ORG_ORG_ERR_CNTS_2_UTM_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_DEVCPU_ORG_ORG_ERR_CNTS_2_UTM_CNT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_DEVCPU_ORG_ORG_ERR_CNTS_2_UTM_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Last reply 0
 *
 * \details
 * Register: \a DEVCPU_ORG::ORG::LAST_REPLY_0
 */
#define VTSS_DEVCPU_ORG_ORG_LAST_REPLY_0     VTSS_IOREG(VTSS_TO_CFG_IF_ORG,0x8)

/** 
 * \brief
 * Last received status from an indirect access
 *
 * \details 
 * 000: REQUEST_OK  
 * 001: TGT_BUSY  
 * 010: UTM   
 * 011: NO_ACTION 
 * 100: WD_DROP
 *
 * Field: ::VTSS_DEVCPU_ORG_ORG_LAST_REPLY_0 . LAST_STATUS
 */
#define  VTSS_F_DEVCPU_ORG_ORG_LAST_REPLY_0_LAST_STATUS(x)  VTSS_ENCODE_BITFIELD(x,2,3)
#define  VTSS_M_DEVCPU_ORG_ORG_LAST_REPLY_0_LAST_STATUS     VTSS_ENCODE_BITMASK(2,3)
#define  VTSS_X_DEVCPU_ORG_ORG_LAST_REPLY_0_LAST_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,2,3)

/** 
 * \brief
 * Last received read/write_reply from an indirect access
 *
 * \details 
 * 1: Last received response was from a read command
 * 0: Last received response was from a write command
 *
 * Field: ::VTSS_DEVCPU_ORG_ORG_LAST_REPLY_0 . LAST_RD
 */
#define  VTSS_F_DEVCPU_ORG_ORG_LAST_REPLY_0_LAST_RD(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_DEVCPU_ORG_ORG_LAST_REPLY_0_LAST_RD     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_DEVCPU_ORG_ORG_LAST_REPLY_0_LAST_RD(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * This bit is set when the remaining fields within this register are
 * stable and valid
 *
 * \details 
 * 0: No replies have been received since this bit was cleared
 * 1: At least one reply has been received, and the last_* registers are
 * valid
 * Bit is cleared by writing a 1 to this position
 *
 * Field: ::VTSS_DEVCPU_ORG_ORG_LAST_REPLY_0 . LAST_REG_GRP_VLD
 */
#define  VTSS_F_DEVCPU_ORG_ORG_LAST_REPLY_0_LAST_REG_GRP_VLD(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_DEVCPU_ORG_ORG_LAST_REPLY_0_LAST_REG_GRP_VLD     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_DEVCPU_ORG_ORG_LAST_REPLY_0_LAST_REG_GRP_VLD(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Last reply address, MSB
 *
 * \details
 * Register: \a DEVCPU_ORG::ORG::LAST_REPLY_1_MSB
 */
#define VTSS_DEVCPU_ORG_ORG_LAST_REPLY_1_MSB  VTSS_IOREG(VTSS_TO_CFG_IF_ORG,0x9)

/** 
 * \brief
 * Last received target module ID from an indirect access
 *
 * \details 
 * Field: ::VTSS_DEVCPU_ORG_ORG_LAST_REPLY_1_MSB . LAST_TGT_ID
 */
#define  VTSS_F_DEVCPU_ORG_ORG_LAST_REPLY_1_MSB_LAST_TGT_ID(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_DEVCPU_ORG_ORG_LAST_REPLY_1_MSB_LAST_TGT_ID     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_DEVCPU_ORG_ORG_LAST_REPLY_1_MSB_LAST_TGT_ID(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Last received target address from an indirect access, MSB portion
 *
 * \details 
 * Field: ::VTSS_DEVCPU_ORG_ORG_LAST_REPLY_1_MSB . LAST_TGT_ADDR_MSB
 */
#define  VTSS_F_DEVCPU_ORG_ORG_LAST_REPLY_1_MSB_LAST_TGT_ADDR_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_DEVCPU_ORG_ORG_LAST_REPLY_1_MSB_LAST_TGT_ADDR_MSB     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_DEVCPU_ORG_ORG_LAST_REPLY_1_MSB_LAST_TGT_ADDR_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Last reply address, LSB
 *
 * \details
 * Register: \a DEVCPU_ORG::ORG::LAST_REPLY_1_LSB
 */
#define VTSS_DEVCPU_ORG_ORG_LAST_REPLY_1_LSB  VTSS_IOREG(VTSS_TO_CFG_IF_ORG,0xa)

/** 
 * \brief
 * Last received target address from an indirect access, LSB portion
 *
 * \details 
 * Field: ::VTSS_DEVCPU_ORG_ORG_LAST_REPLY_1_LSB . LAST_TGT_ADDR_LSB
 */
#define  VTSS_F_DEVCPU_ORG_ORG_LAST_REPLY_1_LSB_LAST_TGT_ADDR_LSB(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DEVCPU_ORG_ORG_LAST_REPLY_1_LSB_LAST_TGT_ADDR_LSB     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DEVCPU_ORG_ORG_LAST_REPLY_1_LSB_LAST_TGT_ADDR_LSB(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Last reply data, MSB 
 *
 * \details
 * Note that software must not rely on data returned from this register,
 * because there's no valid bit associated with it. And because the read of
 * O_LAST_ADDR_REPLY and this register cannot be performed in one atomic
 * operation, there is no guarantee, that the two register accesses relate
 * to the same reply.
 *
 * Register: \a DEVCPU_ORG::ORG::LAST_REPLY_2_MSB
 */
#define VTSS_DEVCPU_ORG_ORG_LAST_REPLY_2_MSB  VTSS_IOREG(VTSS_TO_CFG_IF_ORG,0xb)

/** 
 * \brief
 * Last received target data from an indirect access, MSB portion
 *
 * \details 
 * Field: ::VTSS_DEVCPU_ORG_ORG_LAST_REPLY_2_MSB . LAST_TGT_DATA_MSB
 */
#define  VTSS_F_DEVCPU_ORG_ORG_LAST_REPLY_2_MSB_LAST_TGT_DATA_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DEVCPU_ORG_ORG_LAST_REPLY_2_MSB_LAST_TGT_DATA_MSB     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DEVCPU_ORG_ORG_LAST_REPLY_2_MSB_LAST_TGT_DATA_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Last reply data, LSB
 *
 * \details
 * Register: \a DEVCPU_ORG::ORG::LAST_REPLY_2_LSB
 */
#define VTSS_DEVCPU_ORG_ORG_LAST_REPLY_2_LSB  VTSS_IOREG(VTSS_TO_CFG_IF_ORG,0xc)

/** 
 * \brief
 * Last received target data from an indirect access, LSB portion
 *
 * \details 
 * Field: ::VTSS_DEVCPU_ORG_ORG_LAST_REPLY_2_LSB . LAST_TGT_DATA_LSB
 */
#define  VTSS_F_DEVCPU_ORG_ORG_LAST_REPLY_2_LSB_LAST_TGT_DATA_LSB(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DEVCPU_ORG_ORG_LAST_REPLY_2_LSB_LAST_TGT_DATA_LSB     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DEVCPU_ORG_ORG_LAST_REPLY_2_LSB_LAST_TGT_DATA_LSB(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Configuration and status register
 *
 * \details
 * Register: \a DEVCPU_ORG::ORG::CFG_STATUS
 */
#define VTSS_DEVCPU_ORG_ORG_CFG_STATUS       VTSS_IOREG(VTSS_TO_CFG_IF_ORG,0xd)

/** 
 * \brief
 * This bit field should always be written to 1 for proper operation
 *
 * \details 
 * 0: Reserved, do not use
 * 1: Normal operation
 *
 * Field: ::VTSS_DEVCPU_ORG_ORG_CFG_STATUS . REQUEST_STATUS
 */
#define  VTSS_F_DEVCPU_ORG_ORG_CFG_STATUS_REQUEST_STATUS(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_DEVCPU_ORG_ORG_CFG_STATUS_REQUEST_STATUS     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_DEVCPU_ORG_ORG_CFG_STATUS_REQUEST_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


#endif /* _VTSS_DAYTONA_REGS_DEVCPU_ORG_H_ */
