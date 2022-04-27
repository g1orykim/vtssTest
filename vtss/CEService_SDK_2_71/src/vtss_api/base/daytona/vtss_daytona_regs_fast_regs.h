#ifndef _VTSS_DAYTONA_REGS_FAST_REGS_H_
#define _VTSS_DAYTONA_REGS_FAST_REGS_H_

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
 * Target: \a FAST_REGS
 *
 * Fast register block
 *
 ***********************************************************************/

/**
 * Register Group: \a FAST_REGS::CSR_REGS
 *
 * Control and status registers
 */


/** 
 * \brief Slow data register, MSB
 *
 * \details
 * Register: \a FAST_REGS::CSR_REGS::SLOWDATA_MSB
 */
#define VTSS_FAST_REGS_CSR_REGS_SLOWDATA_MSB  VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0x0)

/** 
 * \brief
 * MSB of a posted read
 *
 * \details 
 * Field: ::VTSS_FAST_REGS_CSR_REGS_SLOWDATA_MSB . SLOW_DATA_MSB
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_SLOWDATA_MSB_SLOW_DATA_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_FAST_REGS_CSR_REGS_SLOWDATA_MSB_SLOW_DATA_MSB     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_FAST_REGS_CSR_REGS_SLOWDATA_MSB_SLOW_DATA_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Slow data register, LSB
 *
 * \details
 * Register: \a FAST_REGS::CSR_REGS::SLOWDATA_LSB
 */
#define VTSS_FAST_REGS_CSR_REGS_SLOWDATA_LSB  VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0x1)

/** 
 * \brief
 * LSB of a posted read
 *
 * \details 
 * Field: ::VTSS_FAST_REGS_CSR_REGS_SLOWDATA_LSB . SLOW_DATA_LSB
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_SLOWDATA_LSB_SLOW_DATA_LSB(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_FAST_REGS_CSR_REGS_SLOWDATA_LSB_SLOW_DATA_LSB     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_FAST_REGS_CSR_REGS_SLOWDATA_LSB_SLOW_DATA_LSB(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Configuration and status register 1
 *
 * \details
 * Register: \a FAST_REGS::CSR_REGS::CFG_STATUS_1
 */
#define VTSS_FAST_REGS_CSR_REGS_CFG_STATUS_1  VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0x2)

/** 
 * \brief
 * Indicates the amount of time that PI_DONE_N is driven de-asserted after
 * PI_CS_N, PI_OE_N, or PI_RD_N are seen de-asserted. Time is in 6.5 ns
 * increments. Value of 0 is maximum drive, 52 ns.
 *
 * \details 
 * Field: ::VTSS_FAST_REGS_CSR_REGS_CFG_STATUS_1 . PI_DONE_DRIVE_DIS
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_CFG_STATUS_1_PI_DONE_DRIVE_DIS(x)  VTSS_ENCODE_BITFIELD(x,12,3)
#define  VTSS_M_FAST_REGS_CSR_REGS_CFG_STATUS_1_PI_DONE_DRIVE_DIS     VTSS_ENCODE_BITMASK(12,3)
#define  VTSS_X_FAST_REGS_CSR_REGS_CFG_STATUS_1_PI_DONE_DRIVE_DIS(x)  VTSS_EXTRACT_BITFIELD(x,12,3)

/** 
 * \brief
 * This bit is used to force the assertion of the PI_RDY_N signal. This is
 * used by interrupt service routines to restore the state of the results
 * from a posted write or read. It is a one-shot bit and is self clearing.
 *
 * \details 
 * 0: Normal operation
 * 1: Force the assertion of PI_RDY_N (self clearing)
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_CFG_STATUS_1 . FORCE_PI_RDY_N
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_CFG_STATUS_1_FORCE_PI_RDY_N(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_CFG_STATUS_1_FORCE_PI_RDY_N     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_CFG_STATUS_1_FORCE_PI_RDY_N(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Set the PI_INTR interrupt signal to be active low or high. If the signal
 * is active low, it is only driven when low and is tri-state when high.
 * When active high, it is always driven and never tri-stated.
 *
 * \details 
 * 0: INTR interrupt signal active high
 * 1: INTR interrupt signal active low
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_CFG_STATUS_1 . INTR_POLARITY
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_CFG_STATUS_1_INTR_POLARITY(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_CFG_STATUS_1_INTR_POLARITY     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_CFG_STATUS_1_INTR_POLARITY(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Configuration and status register 2
 *
 * \details
 * Register: \a FAST_REGS::CSR_REGS::CFG_STATUS_2
 */
#define VTSS_FAST_REGS_CSR_REGS_CFG_STATUS_2  VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0x3)

/** 
 * \brief
 * Indicates that an indirect read is in process
 *
 * \details 
 * 0: An indirect read has completed or no read yet posted
 * 1: An indirect read has been initiated and has not yet completed
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_CFG_STATUS_2 . RD_IN_PROGRESS
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_CFG_STATUS_2_RD_IN_PROGRESS(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_CFG_STATUS_2_RD_IN_PROGRESS     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_CFG_STATUS_2_RD_IN_PROGRESS(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Indicates that an indirect write is in process
 *
 * \details 
 * 0: An indirect write has completed or no write yet posted
 * 1: An indirect write has been initiated and has not yet completed
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_CFG_STATUS_2 . WR_IN_PROGRESS
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_CFG_STATUS_2_WR_IN_PROGRESS(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_CFG_STATUS_2_WR_IN_PROGRESS     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_CFG_STATUS_2_WR_IN_PROGRESS(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Used to indicate when an indirect access has completed
 *
 * \details 
 * 0: Indirect access either not complete or none in process
 * 1: Indirect access has completed
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_CFG_STATUS_2 . PI_RDY
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_CFG_STATUS_2_PI_RDY(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_CFG_STATUS_2_PI_RDY     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_CFG_STATUS_2_PI_RDY(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Number of clock cycles to wait after one or more of PI_CS_N, PI_OE_N, or
 * PI_RD_N are seen de-asserted before PI_DATA tri-states. The unit is 13
 * ns, thereby forcing read data to be stable for up to 100 ns more than
 * the datasheet specifies. The field should be lowered to match what the
 * PI master (CPU) requires. 
 * Note that there will be up to one internal clock cycle (3 ns) of jitter
 * in the exact amount of hold added.
 * Equation: (PI_HOLD * 13 ns) + 11 ns + up to 3 ns for internal clock
 * alignment.
 *
 * \details 
 * 0 - 7: Number of 13 ns wait cycles added
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_CFG_STATUS_2 . PI_HOLD
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_CFG_STATUS_2_PI_HOLD(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_FAST_REGS_CSR_REGS_CFG_STATUS_2_PI_HOLD     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_FAST_REGS_CSR_REGS_CFG_STATUS_2_PI_HOLD(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * Request acknowledges replies for write accesses.
 * It is possible to have registers to give acknowledge replies upon write
 * accesses. The reply contains information abort if the register accepted
 * the write.
 * This register is only used for debugging purposes.
 *
 * \details 
 * 0: Registers don't reply on write accesses 
 * 1: All registers reply on write accesses
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_CFG_STATUS_2 . REQUEST_ACK
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_CFG_STATUS_2_REQUEST_ACK(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_CFG_STATUS_2_REQUEST_ACK     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_CFG_STATUS_2_REQUEST_ACK(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Number of clock cycles to wait after PI_CS_N is seen low before other
 * inputs are sampled. The unit is 13 ns, thereby allowing the input
 * signals to be unstable for up to 200 ns more than the datasheet
 * specifies. The field should be lowered to match what the PI master (CPU)
 * can deliver. 
 * Note: There will be up to one internal clock cycle (3 ns) of jitter in
 * the exact sampling point.
 *
 * \details 
 * 0 - 15: Number of 13 ns wait cycles to be added
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_CFG_STATUS_2 . PI_WAIT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_CFG_STATUS_2_PI_WAIT(x)  VTSS_ENCODE_BITFIELD(x,3,4)
#define  VTSS_M_FAST_REGS_CSR_REGS_CFG_STATUS_2_PI_WAIT     VTSS_ENCODE_BITMASK(3,4)
#define  VTSS_X_FAST_REGS_CSR_REGS_CFG_STATUS_2_PI_WAIT(x)  VTSS_EXTRACT_BITFIELD(x,3,4)


/** 
 * \brief Interrupt status register, channel 0
 *
 * \details
 * Register: \a FAST_REGS::CSR_REGS::INT_STATUS_CH_0
 */
#define VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_0  VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0x4)

/** 
 * \brief
 * Interrupt status for the UPI SERDES for channel 0
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_0 . UPI_0_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_UPI_0_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_UPI_0_INT_STAT     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_UPI_0_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Interrupt status for the eFEC block for channel 0
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_0 . EFEC_0_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_EFEC_0_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_EFEC_0_INT_STAT     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_EFEC_0_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Interrupt status for OTN block 2, channel 0
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_0 . OTN_2_0_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_OTN_2_0_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_OTN_2_0_INT_STAT     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_OTN_2_0_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Interrupt status for WIS block 2 for channel 0
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_0 . WIS_2_0_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_WIS_2_0_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_WIS_2_0_INT_STAT     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_WIS_2_0_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Interrupt status for 64/66 PCS block 2 for channel 0
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_0 . PCS_2_0_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_PCS_2_0_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_PCS_2_0_INT_STAT     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_PCS_2_0_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Interrupt status for GFP block 2 for channel 0
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_0 . GFP_2_0_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_GFP_2_0_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_GFP_2_0_INT_STAT     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_GFP_2_0_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Interrupt status for MAC block 2 for channel 0
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_0 . MAC_2_0_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_MAC_2_0_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_MAC_2_0_INT_STAT     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_MAC_2_0_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Interrupt status for the rate adaptation buffer for channel 0
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_0 . RAB_0_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_RAB_0_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_RAB_0_INT_STAT     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_RAB_0_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Interrupt status for MAC block 1 for channel 0
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_0 . MAC_1_0_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_MAC_1_0_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_MAC_1_0_INT_STAT     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_MAC_1_0_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Interrupt status for GFP block 2 for channel 0
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_0 . GFP_1_0_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_GFP_1_0_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_GFP_1_0_INT_STAT     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_GFP_1_0_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Interrupt status for 64/66 PCS block 1 for channel 0
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_0 . PCS_1_0_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_PCS_1_0_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_PCS_1_0_INT_STAT     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_PCS_1_0_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Interrupt status for WIS block 1 for channel 0
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_0 . WIS_1_0_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_WIS_1_0_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_WIS_1_0_INT_STAT     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_WIS_1_0_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Interrupt status for OTN block 1, channel 0
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_0 . OTN_1_0_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_OTN_1_0_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_OTN_1_0_INT_STAT     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_OTN_1_0_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Interrupt status for XAUI PCS for channel 0
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_0 . XAUI_0_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_XAUI_0_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_XAUI_0_INT_STAT     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_XAUI_0_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Interrupt status for SFI-4.2 for channel 0
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_0 . SFI_0_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_SFI_0_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_SFI_0_INT_STAT     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_SFI_0_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Interrupt status for TFI-5 port for channel 0
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_0 . TFI_0_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_TFI_0_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_TFI_0_INT_STAT     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_0_TFI_0_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Interrupt status register, channel 1
 *
 * \details
 * Register: \a FAST_REGS::CSR_REGS::INT_STATUS_CH_1
 */
#define VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_1  VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0x5)

/** 
 * \brief
 * Interrupt status for the UPI SERDES for channel 1
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_1 . UPI_1_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_UPI_1_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_UPI_1_INT_STAT     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_UPI_1_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Interrupt status for the eFEC block for channel 1
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_1 . EFEC_1_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_EFEC_1_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_EFEC_1_INT_STAT     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_EFEC_1_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Interrupt status for OTN block 2, channel 1
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_1 . OTN_2_1_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_OTN_2_1_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_OTN_2_1_INT_STAT     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_OTN_2_1_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Interrupt status for WIS block 2 for channel 1
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_1 . WIS_2_1_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_WIS_2_1_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_WIS_2_1_INT_STAT     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_WIS_2_1_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Interrupt status for 64/66 block 2 for channel 1
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_1 . PCS_2_1_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_PCS_2_1_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_PCS_2_1_INT_STAT     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_PCS_2_1_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Interrupt status for GFP block 2 for channel 1
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_1 . GFP_2_1_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_GFP_2_1_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_GFP_2_1_INT_STAT     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_GFP_2_1_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Interrupt status for MAC block 2 for channel 1
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_1 . MAC_2_1_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_MAC_2_1_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_MAC_2_1_INT_STAT     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_MAC_2_1_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Interrupt status for the rate adaptation buffer for channel 1
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_1 . RAB_1_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_RAB_1_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_RAB_1_INT_STAT     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_RAB_1_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Interrupt status for MAC block 1 for channel 1
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_1 . MAC_1_1_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_MAC_1_1_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_MAC_1_1_INT_STAT     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_MAC_1_1_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Interrupt status for GFP block 1 for channel 1
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_1 . GFP_1_1_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_GFP_1_1_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_GFP_1_1_INT_STAT     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_GFP_1_1_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Interrupt status for 64/66 PCS block 1 for channel 1
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_1 . PCS_1_1_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_PCS_1_1_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_PCS_1_1_INT_STAT     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_PCS_1_1_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Interrupt status for WIS block 1 for channel 1
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_1 . WIS_1_1_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_WIS_1_1_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_WIS_1_1_INT_STAT     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_WIS_1_1_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Interrupt status for OTN block 1, channel 1
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_1 . OTN_1_1_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_OTN_1_1_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_OTN_1_1_INT_STAT     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_OTN_1_1_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Interrupt status for XAUI PCS for channel 1
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_1 . XAUI_1_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_XAUI_1_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_XAUI_1_INT_STAT     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_XAUI_1_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Interrupt status for SFI-4.2 for channel 1
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_1 . SFI_1_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_SFI_1_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_SFI_1_INT_STAT     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_SFI_1_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Interrupt status for TFI-5 port for channel 1
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_CH_1 . TFI_1_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_TFI_1_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_TFI_1_INT_STAT     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_CH_1_TFI_1_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Miscellaneous interrupt status register
 *
 * \details
 * Register: \a FAST_REGS::CSR_REGS::INT_STATUS_MISC
 */
#define VTSS_FAST_REGS_CSR_REGS_INT_STATUS_MISC  VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0x6)

/** 
 * \brief
 * Interrupt status for the origin block in the internal CSR ring
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_MISC . CPU_IF_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_MISC_CPU_IF_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_MISC_CPU_IF_INT_STAT     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_MISC_CPU_IF_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Interrupt status for GPIO pins
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_MISC . GPIO_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_MISC_GPIO_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_MISC_GPIO_INT_STAT     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_MISC_GPIO_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Interrupt status for the client XFI SERDES for channel 1
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_MISC . CXFI_1_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_MISC_CXFI_1_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_MISC_CXFI_1_INT_STAT     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_MISC_CXFI_1_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Interrupt status for the client XFI SERDES for channel 0
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_MISC . CXFI_0_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_MISC_CXFI_0_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_MISC_CXFI_0_INT_STAT     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_MISC_CXFI_0_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Interrupt status for the line XFI SERDES for channel 1
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_MISC . LXFI_1_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_MISC_LXFI_1_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_MISC_LXFI_1_INT_STAT     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_MISC_LXFI_1_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Interrupt status for the line XFI SERDES for channel 0
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_MISC . LXFI_0_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_MISC_LXFI_0_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_MISC_LXFI_0_INT_STAT     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_MISC_LXFI_0_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Interrupt from the EFEC7 block for channel 1
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_MISC . EFEC7_1_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_MISC_EFEC7_1_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_MISC_EFEC7_1_INT_STAT     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_MISC_EFEC7_1_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Interrupt from the EFEC7 block for channel 0
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_MISC . EFEC7_0_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_MISC_EFEC7_0_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_MISC_EFEC7_0_INT_STAT     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_MISC_EFEC7_0_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Interrupt from the 1588 block for channel 1
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_MISC . IP1588_1_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_MISC_IP1588_1_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_MISC_IP1588_1_INT_STAT     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_MISC_IP1588_1_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Interrupt from the 1588 block for channel 0
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_MISC . IP1588_0_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_MISC_IP1588_0_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_MISC_IP1588_0_INT_STAT     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_MISC_IP1588_0_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Interrupt status for the PMTICK counters
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_MISC . PMTICK_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_MISC_PMTICK_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_MISC_PMTICK_INT_STAT     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_MISC_PMTICK_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Interrupt status for I2C interfaces
 *
 * \details 
 * 0: No interrupt pending
 * 1: Interrupt pending
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_STATUS_MISC . I2C_INT_STAT
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_STATUS_MISC_I2C_INT_STAT(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_STATUS_MISC_I2C_INT_STAT     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_STATUS_MISC_I2C_INT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Interrupt enable register, channel 0
 *
 * \details
 * Register: \a FAST_REGS::CSR_REGS::INT_ENABLE_CH_0
 */
#define VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0  VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0x7)

/** 
 * \brief
 * Interrupt enable for the UPI SERDES for channel 0
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0 . UPI_0_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_UPI_0_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_UPI_0_INT_ENABLE     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_UPI_0_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Interrupt enable for the eFEC block in channel 0
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0 . EFEC_0_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_EFEC_0_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_EFEC_0_INT_ENABLE     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_EFEC_0_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Interrupt enable for OTN block 2 in channel 0
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0 . OTN_2_0_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_OTN_2_0_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_OTN_2_0_INT_ENABLE     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_OTN_2_0_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Interrupt enable for WIS block 2 for channel 0
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0 . WIS_2_0_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_WIS_2_0_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_WIS_2_0_INT_ENABLE     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_WIS_2_0_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Interrupt enable for 64/66 PCS block 2 for channel 0
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0 . PCS_2_0_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_PCS_2_0_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_PCS_2_0_INT_ENABLE     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_PCS_2_0_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Interrupt enable for GFP block 2 in channel 0
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0 . GFP_2_0_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_GFP_2_0_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_GFP_2_0_INT_ENABLE     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_GFP_2_0_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Interrupt enable for MAC block 2 for channel 0
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0 . MAC_2_0_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_MAC_2_0_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_MAC_2_0_INT_ENABLE     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_MAC_2_0_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Interrupt enable for the rate adaptation buffer for channel 0
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0 . RAB_0_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_RAB_0_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_RAB_0_INT_ENABLE     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_RAB_0_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Interrupt enable for MAC block 1 for channel 0
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0 . MAC_1_0_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_MAC_1_0_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_MAC_1_0_INT_ENABLE     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_MAC_1_0_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Interrupt enable for GFP block 1 in channel 0
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0 . GFP_1_0_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_GFP_1_0_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_GFP_1_0_INT_ENABLE     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_GFP_1_0_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Interrupt enable for 64/66 PCS block 1 for channel 0
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0 . PCS_1_0_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_PCS_1_0_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_PCS_1_0_INT_ENABLE     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_PCS_1_0_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Interrupt enable for WIS block 1 for channel 0
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0 . WIS_1_0_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_WIS_1_0_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_WIS_1_0_INT_ENABLE     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_WIS_1_0_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Interrupt enable for OTN block 1 in channel 0
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0 . OTN_1_0_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_OTN_1_0_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_OTN_1_0_INT_ENABLE     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_OTN_1_0_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Interrupt enable for the XAUI PCS on channel 0
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0 . XAUI_0_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_XAUI_0_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_XAUI_0_INT_ENABLE     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_XAUI_0_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Interrupt enable for the SFI-4.2 port on channel 0
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0 . SFI_0_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_SFI_0_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_SFI_0_INT_ENABLE     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_SFI_0_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Interrupt enable for the TFI-5 port on channel 0
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0 . TFI_0_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_TFI_0_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_TFI_0_INT_ENABLE     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_0_TFI_0_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Interrupt enable register, channel 1
 *
 * \details
 * Register: \a FAST_REGS::CSR_REGS::INT_ENABLE_CH_1
 */
#define VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1  VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0x8)

/** 
 * \brief
 * Interrupt enable for the UPI SERDES for channel 0
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1 . UPI_1_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_UPI_1_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_UPI_1_INT_ENABLE     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_UPI_1_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Interrupt enable for the eFEC block in channel 1
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1 . EFEC_1_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_EFEC_1_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_EFEC_1_INT_ENABLE     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_EFEC_1_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Interrupt enable for OTN block 2 in channel 1
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1 . OTN_2_1_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_OTN_2_1_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_OTN_2_1_INT_ENABLE     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_OTN_2_1_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Interrupt enable for WIS block 2 for channel 2
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1 . WIS_2_1_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_WIS_2_1_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_WIS_2_1_INT_ENABLE     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_WIS_2_1_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Interrupt enable for 64/66 PCS block 2 for channel 1
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1 . PCS_2_1_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_PCS_2_1_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_PCS_2_1_INT_ENABLE     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_PCS_2_1_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Interrupt enable for GFP block 2 in channel 1
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1 . GFP_2_1_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_GFP_2_1_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_GFP_2_1_INT_ENABLE     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_GFP_2_1_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Interrupt enable for the MAC block 2 for channel 1
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1 . MAC_2_1_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_MAC_2_1_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_MAC_2_1_INT_ENABLE     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_MAC_2_1_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Interrupt enable for the rate adaptation buffer for channel 1
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1 . RAB_1_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_RAB_1_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_RAB_1_INT_ENABLE     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_RAB_1_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Interrupt enable for MAC block 1 for channel 1
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1 . MAC_1_1_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_MAC_1_1_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_MAC_1_1_INT_ENABLE     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_MAC_1_1_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Interrupt enable for GFP block 1 in channel 1
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1 . GFP_1_1_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_GFP_1_1_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_GFP_1_1_INT_ENABLE     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_GFP_1_1_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Interrupt enable for 64/66 PCS 1 for channel 2
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1 . PCS_1_1_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_PCS_1_1_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_PCS_1_1_INT_ENABLE     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_PCS_1_1_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Interrupt enable for WIS block 1 for channel 1
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1 . WIS_1_1_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_WIS_1_1_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_WIS_1_1_INT_ENABLE     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_WIS_1_1_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Interrupt enable for OTN block 1 in channel 1
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1 . OTN_1_1_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_OTN_1_1_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_OTN_1_1_INT_ENABLE     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_OTN_1_1_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Interrupt enable for the XAUI PCS on channel 1
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1 . XAUI_1_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_XAUI_1_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_XAUI_1_INT_ENABLE     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_XAUI_1_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Interrupt enable for the SFI-4.2 port on channel 1
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1 . SFI_1_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_SFI_1_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_SFI_1_INT_ENABLE     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_SFI_1_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Interrupt enable for the TFI-5 port on channel 1
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1 . TFI_1_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_TFI_1_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_TFI_1_INT_ENABLE     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_CH_1_TFI_1_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Miscellaneous interrupt enable register
 *
 * \details
 * Register: \a FAST_REGS::CSR_REGS::INT_ENABLE_MISC
 */
#define VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_MISC  VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0x9)

/** 
 * \brief
 * Interrupt enable for CPU_IF_INT_STAT
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_MISC . CPU_IF_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_CPU_IF_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_CPU_IF_INT_ENABLE     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_CPU_IF_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Interrupt enable for GPIO pins
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_MISC . GPIO_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_GPIO_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_GPIO_INT_ENABLE     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_GPIO_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Interrupt enable for the client XFI SERDES for channel 1
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_MISC . CXFI_1_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_CXFI_1_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_CXFI_1_INT_ENABLE     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_CXFI_1_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Interrupt enable for the client XFI SERDES for channel 0
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_MISC . CXFI_0_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_CXFI_0_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_CXFI_0_INT_ENABLE     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_CXFI_0_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Interrupt enable for the line XFI SERDES for channel 1
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_MISC . LXFI_1_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_LXFI_1_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_LXFI_1_INT_ENABLE     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_LXFI_1_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Interrupt enable for the line XFI SERDES for channel 0
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_MISC . LXFI_0_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_LXFI_0_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_LXFI_0_INT_ENABLE     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_LXFI_0_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Interrupt enable for the EFEC7 block for channel 1
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_MISC . EFEC7_1_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_EFEC7_1_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_EFEC7_1_INT_ENABLE     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_EFEC7_1_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Interrupt enable for the EFEC7 block for channel 0
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_MISC . EFEC7_0_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_EFEC7_0_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_EFEC7_0_INT_ENABLE     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_EFEC7_0_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Interrupt enable for the 1588 block for channel 1
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_MISC . IP1588_1_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_IP1588_1_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_IP1588_1_INT_ENABLE     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_IP1588_1_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Interrupt enable for the 1588 block for channel 0
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_MISC . IP1588_0_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_IP1588_0_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_IP1588_0_INT_ENABLE     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_IP1588_0_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Interrupt enable for the PMTICK counters
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_MISC . PMTICK_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_PMTICK_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_PMTICK_INT_ENABLE     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_PMTICK_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Interrupt enable for I2C interfaces
 *
 * \details 
 * 0: Interrupt not enabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_INT_ENABLE_MISC . I2C_INT_ENABLE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_I2C_INT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_I2C_INT_ENABLE     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_INT_ENABLE_MISC_I2C_INT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief I2C prescale register, channel 0
 *
 * \details
 * Register: \a FAST_REGS::CSR_REGS::I2C_PRESCALE_0
 */
#define VTSS_FAST_REGS_CSR_REGS_I2C_PRESCALE_0  VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0xa)

/** 
 * \brief
 * Prescale value
 *
 * \details 
 * Prescale value = (sys_clk/(5 * SCL frequency)) -1
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_PRESCALE_0 . I2C_PRESCALE_0
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_PRESCALE_0_I2C_PRESCALE_0(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_PRESCALE_0_I2C_PRESCALE_0     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_PRESCALE_0_I2C_PRESCALE_0(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief I2C control register, channel 0
 *
 * \details
 * Register: \a FAST_REGS::CSR_REGS::I2C_CTL_REG_0
 */
#define VTSS_FAST_REGS_CSR_REGS_I2C_CTL_REG_0  VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0xb)

/** 
 * \brief
 * Enable two-wire serial channel 0
 *
 * \details 
 * 0: Disable the channel 0 two-wire serial interface
 * 1: Enable the channel 0 two-wire serial interface
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_CTL_REG_0 . I2C_EN_0
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_CTL_REG_0_I2C_EN_0(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_CTL_REG_0_I2C_EN_0     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_CTL_REG_0_I2C_EN_0(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief I2C transmit register, channel 0
 *
 * \details
 * Register: \a FAST_REGS::CSR_REGS::I2C_TX_REG_0
 */
#define VTSS_FAST_REGS_CSR_REGS_I2C_TX_REG_0  VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0xc)

/** 
 * \brief
 * Next byte to be transmitted on the I2C interface, upper 7 bits of data,
 * or slave address
 *
 * \details 
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_TX_REG_0 . I2C_TX_DATA_ADR_0
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_TX_REG_0_I2C_TX_DATA_ADR_0(x)  VTSS_ENCODE_BITFIELD(x,1,7)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_TX_REG_0_I2C_TX_DATA_ADR_0     VTSS_ENCODE_BITMASK(1,7)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_TX_REG_0_I2C_TX_DATA_ADR_0(x)  VTSS_EXTRACT_BITFIELD(x,1,7)

/** 
 * \brief
 * LSB of data byte for data transfers or read/write control for address
 * phase 
 *
 * \details 
 * 1: Read
 * 0: Write
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_TX_REG_0 . I2C_TX_DATA_CTL_0
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_TX_REG_0_I2C_TX_DATA_CTL_0(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_TX_REG_0_I2C_TX_DATA_CTL_0     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_TX_REG_0_I2C_TX_DATA_CTL_0(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief I2C receive data register, channel 0
 *
 * \details
 * Register: \a FAST_REGS::CSR_REGS::I2C_RX_REG_0
 */
#define VTSS_FAST_REGS_CSR_REGS_I2C_RX_REG_0  VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0xd)

/** 
 * \brief
 * Last byte received from the I2C interface
 *
 * \details 
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_RX_REG_0 . I2C_RX_REG_0
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_RX_REG_0_I2C_RX_REG_0(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_RX_REG_0_I2C_RX_REG_0     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_RX_REG_0_I2C_RX_REG_0(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief I2C command register, channel 0
 *
 * \details
 * Register: \a FAST_REGS::CSR_REGS::I2C_CMD_REG_0
 */
#define VTSS_FAST_REGS_CSR_REGS_I2C_CMD_REG_0  VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0xe)

/** 
 * \brief
 * Generate a start to initiate a bus cycle or a repeated start condition
 * to hold the bus after a transaction so that another transaction can be
 * initiated.
 * Automatically cleared after the start cycle has completed.
 *
 * \details 
 * 1: Generate a START or repeated START cycle
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_CMD_REG_0 . I2C_STA_0
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_CMD_REG_0_I2C_STA_0(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_CMD_REG_0_I2C_STA_0     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_CMD_REG_0_I2C_STA_0(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Generate a stop cycle to terminate the current transaction.
 * Automatically cleared after the current byte is sent.
 *
 * \details 
 * 1: Generate a STOP cycle
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_CMD_REG_0 . I2C_STO_0
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_CMD_REG_0_I2C_STO_0(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_CMD_REG_0_I2C_STO_0     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_CMD_REG_0_I2C_STO_0(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Read command
 * Automatically cleared after the current byte is sent.
 *
 * \details 
 * 1: Read a byte from the slave
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_CMD_REG_0 . I2C_RD_0
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_CMD_REG_0_I2C_RD_0(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_CMD_REG_0_I2C_RD_0     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_CMD_REG_0_I2C_RD_0(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Write command
 * Automatically cleared after the current byte is sent.
 *
 * \details 
 * 1: Write a byte to the slave
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_CMD_REG_0 . I2C_WR_0
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_CMD_REG_0_I2C_WR_0(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_CMD_REG_0_I2C_WR_0     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_CMD_REG_0_I2C_WR_0(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * ACK/NACK for read operations
 * Set to a 1 if the master wants to perform another read (ACK). Set to 0
 * if this is the final read of the sequence (NACK).
 * Automatically cleared after the current byte is sent.
 *
 * \details 
 * 0: NACK
 * 1: ACK
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_CMD_REG_0 . I2C_ACK_0
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_CMD_REG_0_I2C_ACK_0(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_CMD_REG_0_I2C_ACK_0     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_CMD_REG_0_I2C_ACK_0(x)  VTSS_EXTRACT_BITFIELD(x,3,1)


/** 
 * \brief I2C status register, channel 0
 *
 * \details
 * Register: \a FAST_REGS::CSR_REGS::I2C_STATUS_REG_0
 */
#define VTSS_FAST_REGS_CSR_REGS_I2C_STATUS_REG_0  VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0xf)

/** 
 * \brief
 * Received an acknowledge from the slave device on a write
 *
 * \details 
 * 1: Acknowledge received
 * 0: No acknowledge received
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_STATUS_REG_0 . I2C_TXACK_STICKY_0
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_STATUS_REG_0_I2C_TXACK_STICKY_0(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_STATUS_REG_0_I2C_TXACK_STICKY_0     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_STATUS_REG_0_I2C_TXACK_STICKY_0(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * I2C bus arbitration lost. 
 * This condition occurs when the I2C master wants to drive SDA high but
 * the pin remains low.
 *
 * \details 
 * 1: I2C bus arbitration lost
 * 0: Normal operation
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_STATUS_REG_0 . I2C_AL_STICKY_0
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_STATUS_REG_0_I2C_AL_STICKY_0(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_STATUS_REG_0_I2C_AL_STICKY_0     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_STATUS_REG_0_I2C_AL_STICKY_0(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Indicates whether the previously completed write was acknowledged or
 * not. Cleared at the start of every write.
 *
 * \details 
 * 1: Write was acknowledged
 * 0: Write no acknowledged
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_STATUS_REG_0 . I2C_TXACK_0
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_STATUS_REG_0_I2C_TXACK_0(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_STATUS_REG_0_I2C_TXACK_0     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_STATUS_REG_0_I2C_TXACK_0(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * I2C bus cycle/transfer in progress, read, write, START or STOP.
 *
 * \details 
 * 1: Cycle/transfer in progress
 * 0: Cycle/transfer complete
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_STATUS_REG_0 . I2C_TIP_0
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_STATUS_REG_0_I2C_TIP_0(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_STATUS_REG_0_I2C_TIP_0     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_STATUS_REG_0_I2C_TIP_0(x)  VTSS_EXTRACT_BITFIELD(x,1,1)


/** 
 * \brief I2C interrupt MASK register, channel 0
 *
 * \details
 * Register: \a FAST_REGS::CSR_REGS::I2C_INT_MASK_REG_0
 */
#define VTSS_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_0  VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0x10)

/** 
 * \brief
 * Interrupt mask for I2C_TXACK_STICKY_0
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_0 . I2C_TXACK_MASK_0
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_0_I2C_TXACK_MASK_0(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_0_I2C_TXACK_MASK_0     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_0_I2C_TXACK_MASK_0(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Interrupt mask for I2C_AL_STICKY_0
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_0 . I2C_AL_MASK_0
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_0_I2C_AL_MASK_0(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_0_I2C_AL_MASK_0     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_0_I2C_AL_MASK_0(x)  VTSS_EXTRACT_BITFIELD(x,5,1)


/** 
 * \brief I2C prescale register, channel 1
 *
 * \details
 * Register: \a FAST_REGS::CSR_REGS::I2C_PRESCALE_1
 */
#define VTSS_FAST_REGS_CSR_REGS_I2C_PRESCALE_1  VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0x11)

/** 
 * \brief
 * Prescale value
 *
 * \details 
 * Prescale value = ((REFCLK frequency * 2)/(5 * SCL frequency)) -1
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_PRESCALE_1 . I2C_PRESCALE_1
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_PRESCALE_1_I2C_PRESCALE_1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_PRESCALE_1_I2C_PRESCALE_1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_PRESCALE_1_I2C_PRESCALE_1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief I2C control register, channel 1
 *
 * \details
 * Register: \a FAST_REGS::CSR_REGS::I2C_CTL_REG_1
 */
#define VTSS_FAST_REGS_CSR_REGS_I2C_CTL_REG_1  VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0x12)

/** 
 * \brief
 * Enable two-wire serial channel 1
 *
 * \details 
 * 0: Disable the channel 1 two-wire serial interface
 * 1: Enable the channel 1 two-wire serial interface
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_CTL_REG_1 . I2C_EN_1
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_CTL_REG_1_I2C_EN_1(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_CTL_REG_1_I2C_EN_1     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_CTL_REG_1_I2C_EN_1(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief I2C transmit register, channel 1
 *
 * \details
 * Register: \a FAST_REGS::CSR_REGS::I2C_TX_REG_1
 */
#define VTSS_FAST_REGS_CSR_REGS_I2C_TX_REG_1  VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0x13)

/** 
 * \brief
 * Next byte to be transmitted on the I2C interface, upper 7 bits of data,
 * or slave address
 *
 * \details 
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_TX_REG_1 . I2C_TX_DATA_ADR_1
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_TX_REG_1_I2C_TX_DATA_ADR_1(x)  VTSS_ENCODE_BITFIELD(x,1,7)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_TX_REG_1_I2C_TX_DATA_ADR_1     VTSS_ENCODE_BITMASK(1,7)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_TX_REG_1_I2C_TX_DATA_ADR_1(x)  VTSS_EXTRACT_BITFIELD(x,1,7)

/** 
 * \brief
 * LSB of data byte for data transfers or read/write control for address
 * phase 
 *
 * \details 
 * 1: Read
 * 0: Write
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_TX_REG_1 . I2C_TX_DATA_CTL_1
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_TX_REG_1_I2C_TX_DATA_CTL_1(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_TX_REG_1_I2C_TX_DATA_CTL_1     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_TX_REG_1_I2C_TX_DATA_CTL_1(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief I2C receive data register, channel 1
 *
 * \details
 * Register: \a FAST_REGS::CSR_REGS::I2C_RX_REG_1
 */
#define VTSS_FAST_REGS_CSR_REGS_I2C_RX_REG_1  VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0x14)

/** 
 * \brief
 * Last byte received from the I2C interface
 *
 * \details 
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_RX_REG_1 . I2C_RX_REG_1
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_RX_REG_1_I2C_RX_REG_1(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_RX_REG_1_I2C_RX_REG_1     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_RX_REG_1_I2C_RX_REG_1(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief I2C command register, channel 0
 *
 * \details
 * Register: \a FAST_REGS::CSR_REGS::I2C_CMD_REG_1
 */
#define VTSS_FAST_REGS_CSR_REGS_I2C_CMD_REG_1  VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0x15)

/** 
 * \brief
 * Generate a start to initiate a bus cycle or a repeated start condition
 * to hold the bus after a transaction so that another transaction can be
 * initiated.
 * Automatically cleared after the start cycle has completed.
 *
 * \details 
 * 1: Generate a START or repeated START cycle
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_CMD_REG_1 . I2C_STA_1
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_CMD_REG_1_I2C_STA_1(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_CMD_REG_1_I2C_STA_1     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_CMD_REG_1_I2C_STA_1(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Generate a stop cycle to terminate the current transaction.
 * Automatically cleared after the current byte is sent.
 *
 * \details 
 * 1: Generate a STOP cycle
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_CMD_REG_1 . I2C_STO_1
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_CMD_REG_1_I2C_STO_1(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_CMD_REG_1_I2C_STO_1     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_CMD_REG_1_I2C_STO_1(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Read command
 * Automatically cleared after the current byte is sent.
 *
 * \details 
 * 1: Read a byte from the slave
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_CMD_REG_1 . I2C_RD_1
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_CMD_REG_1_I2C_RD_1(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_CMD_REG_1_I2C_RD_1     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_CMD_REG_1_I2C_RD_1(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Write command
 * Automatically cleared after the current byte is sent.
 *
 * \details 
 * 1: Write a byte to the slave
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_CMD_REG_1 . I2C_WR_1
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_CMD_REG_1_I2C_WR_1(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_CMD_REG_1_I2C_WR_1     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_CMD_REG_1_I2C_WR_1(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * ACK/NACK for read operations
 * Set to a 1 if the master wants to perform another read (ACK). Set to 0
 * if this is the final read of the sequence (NACK).
 * Automatically cleared after the current byte is sent.
 *
 * \details 
 * 0: NACK
 * 1: ACK
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_CMD_REG_1 . I2C_ACK_1
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_CMD_REG_1_I2C_ACK_1(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_CMD_REG_1_I2C_ACK_1     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_CMD_REG_1_I2C_ACK_1(x)  VTSS_EXTRACT_BITFIELD(x,3,1)


/** 
 * \brief I2C status register, channel 1
 *
 * \details
 * Register: \a FAST_REGS::CSR_REGS::I2C_STATUS_REG_1
 */
#define VTSS_FAST_REGS_CSR_REGS_I2C_STATUS_REG_1  VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0x16)

/** 
 * \brief
 * Received an acknowledge from the slave device on a write
 *
 * \details 
 * 1: Acknowledge received
 * 0: No acknowledge received
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_STATUS_REG_1 . I2C_TXACK_STICKY_1
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_STATUS_REG_1_I2C_TXACK_STICKY_1(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_STATUS_REG_1_I2C_TXACK_STICKY_1     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_STATUS_REG_1_I2C_TXACK_STICKY_1(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * I2C bus arbitration lost
 * This condition occurs when the I2C master wants to drive SDA high but
 * the pin remains low.
 *
 * \details 
 * 1: I2C bus arbitration lost
 * 0: Normal operation
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_STATUS_REG_1 . I2C_AL_STICKY_1
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_STATUS_REG_1_I2C_AL_STICKY_1(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_STATUS_REG_1_I2C_AL_STICKY_1     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_STATUS_REG_1_I2C_AL_STICKY_1(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Indicates whether the previously completed write was acknowledged or
 * not. Cleared at the start of every write.
 *
 * \details 
 * 1: Write was acknowledged
 * 0: Write not acknowledged
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_STATUS_REG_1 . I2C_TXACK_1
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_STATUS_REG_1_I2C_TXACK_1(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_STATUS_REG_1_I2C_TXACK_1     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_STATUS_REG_1_I2C_TXACK_1(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * I2C bus cycle/transfer in progress, read, write, START or STOP.
 *
 * \details 
 * 1: Cycle/transfer in progress
 * 0: Cycle/transfer complete
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_STATUS_REG_1 . I2C_TIP_1
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_STATUS_REG_1_I2C_TIP_1(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_STATUS_REG_1_I2C_TIP_1     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_STATUS_REG_1_I2C_TIP_1(x)  VTSS_EXTRACT_BITFIELD(x,1,1)


/** 
 * \brief I2C interrupt register, channel 1
 *
 * \details
 * Register: \a FAST_REGS::CSR_REGS::I2C_INT_MASK_REG_1
 */
#define VTSS_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_1  VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0x17)

/** 
 * \brief
 * Interrupt mask for I2C_TXACK_STICKY_1
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_1 . I2C_TXACK_MASK_1
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_1_I2C_TXACK_MASK_1(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_1_I2C_TXACK_MASK_1     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_1_I2C_TXACK_MASK_1(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Interrupt mask for I2C_AL_STICKY_1
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_1 . I2C_AL_MASK_1
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_1_I2C_AL_MASK_1(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_1_I2C_AL_MASK_1     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_I2C_INT_MASK_REG_1_I2C_AL_MASK_1(x)  VTSS_EXTRACT_BITFIELD(x,5,1)


/** 
 * \brief Revision ID
 *
 * \details
 * Duplicate of register in GCB block
 *
 * Register: \a FAST_REGS::CSR_REGS::REV_ID
 */
#define VTSS_FAST_REGS_CSR_REGS_REV_ID       VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0x18)

/** 
 * \brief
 * Revision ID
 * Device revision starting from 1
 *
 * \details 
 * Field: ::VTSS_FAST_REGS_CSR_REGS_REV_ID . REV_ID
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_REV_ID_REV_ID(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_FAST_REGS_CSR_REGS_REV_ID_REV_ID     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_FAST_REGS_CSR_REGS_REV_ID_REV_ID(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief Part ID
 *
 * \details
 * Duplicate of register in GCB block
 *
 * Register: \a FAST_REGS::CSR_REGS::PART_ID
 */
#define VTSS_FAST_REGS_CSR_REGS_PART_ID      VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0x19)

/** 
 * \brief
 * Part ID
 *
 * \details 
 * Field: ::VTSS_FAST_REGS_CSR_REGS_PART_ID . PART_ID
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_PART_ID_PART_ID(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_FAST_REGS_CSR_REGS_PART_ID_PART_ID     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_FAST_REGS_CSR_REGS_PART_ID_PART_ID(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Manufacturers ID
 *
 * \details
 * Duplicate of register in GCB block
 *
 * Register: \a FAST_REGS::CSR_REGS::MFG_ID
 */
#define VTSS_FAST_REGS_CSR_REGS_MFG_ID       VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0x1a)

/** 
 * \brief
 * Manufacturers ID
 *
 * \details 
 * Field: ::VTSS_FAST_REGS_CSR_REGS_MFG_ID . MFG_ID
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_MFG_ID_MFG_ID(x)  VTSS_ENCODE_BITFIELD(x,0,11)
#define  VTSS_M_FAST_REGS_CSR_REGS_MFG_ID_MFG_ID     VTSS_ENCODE_BITMASK(0,11)
#define  VTSS_X_FAST_REGS_CSR_REGS_MFG_ID_MFG_ID(x)  VTSS_EXTRACT_BITFIELD(x,0,11)


/** 
 * \brief Value of one
 *
 * \details
 * Register: \a FAST_REGS::CSR_REGS::ONE
 */
#define VTSS_FAST_REGS_CSR_REGS_ONE          VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0x1b)

/** 
 * \brief
 * Returns 1
 *
 * \details 
 * Field: ::VTSS_FAST_REGS_CSR_REGS_ONE . ONE
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_ONE_ONE(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_ONE_ONE     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_ONE_ONE(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Variant select
 *
 * \details
 * Duplicate of register in GCB block
 *
 * Register: \a FAST_REGS::CSR_REGS::VAR_SEL
 */
#define VTSS_FAST_REGS_CSR_REGS_VAR_SEL      VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0x1c)

/** 
 * \brief
 * Variant select
 * Indicates the variant of the device
 *
 * \details 
 * Field: ::VTSS_FAST_REGS_CSR_REGS_VAR_SEL . VAR_SEL
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_VAR_SEL_VAR_SEL(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_FAST_REGS_CSR_REGS_VAR_SEL_VAR_SEL     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_FAST_REGS_CSR_REGS_VAR_SEL_VAR_SEL(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Die select
 *
 * \details
 * Duplicate of register in GCB block
 *
 * Register: \a FAST_REGS::CSR_REGS::DIE_SEL
 */
#define VTSS_FAST_REGS_CSR_REGS_DIE_SEL      VTSS_IOREG(VTSS_TO_CFG_IF_FAST_REGS,0x1d)

/** 
 * \brief
 * Indicates which of two dies this chip is
 *
 * \details 
 * 0: Die 0, channels 0 & 1
 * 1: Die 1, channels 2 & 3
 *
 * Field: ::VTSS_FAST_REGS_CSR_REGS_DIE_SEL . DIE_SEL
 */
#define  VTSS_F_FAST_REGS_CSR_REGS_DIE_SEL_DIE_SEL(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_FAST_REGS_CSR_REGS_DIE_SEL_DIE_SEL     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_FAST_REGS_CSR_REGS_DIE_SEL_DIE_SEL(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


#endif /* _VTSS_DAYTONA_REGS_FAST_REGS_H_ */
