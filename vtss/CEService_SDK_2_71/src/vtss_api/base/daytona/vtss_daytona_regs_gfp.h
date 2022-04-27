#ifndef _VTSS_DAYTONA_REGS_GFP_H_
#define _VTSS_DAYTONA_REGS_GFP_H_

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
 * Target: \a GFP
 *
 * \see vtss_target_GFP_e
 *
 * GFP configuration and status registers
 *
 ***********************************************************************/

/**
 * Register Group: \a GFP::GFP_BASE
 *
 * GFP base registers
 */


/** 
 * \brief GFP control register
 *
 * \details
 * Register: \a GFP::GFP_BASE::GFP_CONTROL
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFP_BASE_GFP_CONTROL(target)  VTSS_IOREG(target,0x0)

/** 
 * \brief
 * Client data mode select.
 *
 * \details 
 * 00: 8 Gigabit Fibre Channel
 * 01: Reserved
 * 10: 10 Gigabit Ethernet Packet (G.sup43-6.2)
 * 11: 10 Gigabit Ethernet PP/OS (G.sup43-7.3)	   
 *
 * Field: ::VTSS_GFP_GFP_BASE_GFP_CONTROL . CLIENT_MODE_SEL
 */
#define  VTSS_F_GFP_GFP_BASE_GFP_CONTROL_CLIENT_MODE_SEL(x)  VTSS_ENCODE_BITFIELD(x,8,2)
#define  VTSS_M_GFP_GFP_BASE_GFP_CONTROL_CLIENT_MODE_SEL     VTSS_ENCODE_BITMASK(8,2)
#define  VTSS_X_GFP_GFP_BASE_GFP_CONTROL_CLIENT_MODE_SEL(x)  VTSS_EXTRACT_BITFIELD(x,8,2)

/** 
 * \brief
 * Force client signal fail loss of sync.
 * Allows SW control of CSF LSYNC input to the GFP mapper when
 * CSF_LSYNC_SEL = 0. Has no effect when CSF_LSYNC_SEL = 2 or 3.       
 *
 * \details 
 * 0: Force LSYNC condition inactive
 * 1: Force LSYNC condition active 
 *
 * Field: ::VTSS_GFP_GFP_BASE_GFP_CONTROL . FORCE_CSF_LSYNC
 */
#define  VTSS_F_GFP_GFP_BASE_GFP_CONTROL_FORCE_CSF_LSYNC(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_GFP_GFP_BASE_GFP_CONTROL_FORCE_CSF_LSYNC     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_GFP_GFP_BASE_GFP_CONTROL_FORCE_CSF_LSYNC(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Client signal fail loss of sync select.
 * Always set CSF_LSYNC_SEL to 0 for 8 Gigabit Fibre Channel mode because
 * both the XAUI PCS and 10GBASE-R PCS are not used.	    
 *
 * \details 
 * 0: Use SW LSYNC from FORCE_CSF_LSYNC
 * 1: Reserved
 * 2: Use HW LSYNC from XAUI PCS
 * 3: Use HW LSYNC from 10GBASE-R PCS	   
 *
 * Field: ::VTSS_GFP_GFP_BASE_GFP_CONTROL . CSF_LSYNC_SEL
 */
#define  VTSS_F_GFP_GFP_BASE_GFP_CONTROL_CSF_LSYNC_SEL(x)  VTSS_ENCODE_BITFIELD(x,5,2)
#define  VTSS_M_GFP_GFP_BASE_GFP_CONTROL_CSF_LSYNC_SEL     VTSS_ENCODE_BITMASK(5,2)
#define  VTSS_X_GFP_GFP_BASE_GFP_CONTROL_CSF_LSYNC_SEL(x)  VTSS_EXTRACT_BITFIELD(x,5,2)

/** 
 * \brief
 * Force client signal fail loss of signal.
 * Allows SW control of CSF LOS input to the GFP Mapper when CSF_LOS_SEL =
 * 1. Has no effect when CSF_LOS_SEL = 0.  
 *
 * \details 
 * 0: Force LOS condition inactive
 * 1: Force LOS condition active
 *
 * Field: ::VTSS_GFP_GFP_BASE_GFP_CONTROL . FORCE_CSF_LOS
 */
#define  VTSS_F_GFP_GFP_BASE_GFP_CONTROL_FORCE_CSF_LOS(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_GFP_GFP_BASE_GFP_CONTROL_FORCE_CSF_LOS     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_GFP_GFP_BASE_GFP_CONTROL_FORCE_CSF_LOS(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Client signal fail loss of signal select.
 *
 * \details 
 * 0: Use SW LOS from FORCE_CSF_LOS
 * 1: Use HW LOS from SERDES
 *
 * Field: ::VTSS_GFP_GFP_BASE_GFP_CONTROL . CSF_LOS_SEL
 */
#define  VTSS_F_GFP_GFP_BASE_GFP_CONTROL_CSF_LOS_SEL(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_GFP_GFP_BASE_GFP_CONTROL_CSF_LOS_SEL     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_GFP_GFP_BASE_GFP_CONTROL_CSF_LOS_SEL(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * GFP mapper software reset.
 *
 * \details 
 * 0: Block operates normally
 * 1: All logic (other than CSR target) is held in reset, clocks are not
 * disabled
 *
 * Field: ::VTSS_GFP_GFP_BASE_GFP_CONTROL . SW_RST
 */
#define  VTSS_F_GFP_GFP_BASE_GFP_CONTROL_SW_RST(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_GFP_GFP_BASE_GFP_CONTROL_SW_RST     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_GFP_GFP_BASE_GFP_CONTROL_SW_RST(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * GFP mapper clock enable.
 *
 * \details 
 * 0: All clocks for this module (with the exception of the csr_clk) are
 * disabled and GFP logic (other than CSR target) is held in reset
 * 1: All clocks for this module are enabled
 *
 * Field: ::VTSS_GFP_GFP_BASE_GFP_CONTROL . CLK_ENA
 */
#define  VTSS_F_GFP_GFP_BASE_GFP_CONTROL_CLK_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_GFP_GFP_BASE_GFP_CONTROL_CLK_ENA     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_GFP_GFP_BASE_GFP_CONTROL_CLK_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief GFP status register
 *
 * \details
 * A sticky bit is cleared by writing a 1 to its position. Bit(s) read as 1
 * indicate that the event(s) occurred.
 *
 * Register: \a GFP::GFP_BASE::GFP_STATUS
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFP_BASE_GFP_STATUS(target)  VTSS_IOREG(target,0x1)

/** 
 * \brief
 * GFP UPI interrupt pending.
 * This bit is set when there is an un-masked interrupt pending in the
 * GFP_UPI_INTERRUPT_STICKY register
 *
 * \details 
 * Field: ::VTSS_GFP_GFP_BASE_GFP_STATUS . GFP_UPI_INTERRUPT_PENDING
 */
#define  VTSS_F_GFP_GFP_BASE_GFP_STATUS_GFP_UPI_INTERRUPT_PENDING(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_GFP_GFP_BASE_GFP_STATUS_GFP_UPI_INTERRUPT_PENDING     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_GFP_GFP_BASE_GFP_STATUS_GFP_UPI_INTERRUPT_PENDING(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Rx GFP client interrupt pending.
 * This bit is set when there is an un-masked interrupt pending in the
 * RX_GFP_CLIENT_INTERRUPT_STICKY register
 *
 * \details 
 * Field: ::VTSS_GFP_GFP_BASE_GFP_STATUS . RX_GFP_CLIENT_INTERRUPT_PENDING
 */
#define  VTSS_F_GFP_GFP_BASE_GFP_STATUS_RX_GFP_CLIENT_INTERRUPT_PENDING(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_GFP_GFP_BASE_GFP_STATUS_RX_GFP_CLIENT_INTERRUPT_PENDING     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_GFP_GFP_BASE_GFP_STATUS_RX_GFP_CLIENT_INTERRUPT_PENDING(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Rx GFP interrupt pending.
 * This bit is set when there is an un-masked interrupt pending in the
 * RX_GFP_INTERRUPT_STICKY register
 *
 * \details 
 * Field: ::VTSS_GFP_GFP_BASE_GFP_STATUS . RX_GFP_INTERRUPT_PENDING
 */
#define  VTSS_F_GFP_GFP_BASE_GFP_STATUS_RX_GFP_INTERRUPT_PENDING(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_GFP_GFP_BASE_GFP_STATUS_RX_GFP_INTERRUPT_PENDING     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_GFP_GFP_BASE_GFP_STATUS_RX_GFP_INTERRUPT_PENDING(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Rx 8B10B interrupt pending.
 * This bit is set when there is an un-masked interrupt pending in the
 * RX_8B10B_INTERRUPT_STICKY register
 *
 * \details 
 * Field: ::VTSS_GFP_GFP_BASE_GFP_STATUS . RX_8B10B_INTERRUPT_PENDING
 */
#define  VTSS_F_GFP_GFP_BASE_GFP_STATUS_RX_8B10B_INTERRUPT_PENDING(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_GFP_GFP_BASE_GFP_STATUS_RX_8B10B_INTERRUPT_PENDING     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_GFP_GFP_BASE_GFP_STATUS_RX_8B10B_INTERRUPT_PENDING(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Rx PP/OS interrupt pending.
 * This bit is set when there is an un-masked interrupt pending in the
 * RX_PPOS_INTERRUPT_STICKY register
 *
 * \details 
 * Field: ::VTSS_GFP_GFP_BASE_GFP_STATUS . RX_PPOS_INTERRUPT_PENDING
 */
#define  VTSS_F_GFP_GFP_BASE_GFP_STATUS_RX_PPOS_INTERRUPT_PENDING(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_GFP_GFP_BASE_GFP_STATUS_RX_PPOS_INTERRUPT_PENDING     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_GFP_GFP_BASE_GFP_STATUS_RX_PPOS_INTERRUPT_PENDING(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Rx interrupt pending.
 * This bit is set when there is an un-masked interrupt pending in the
 * RX_INTERRUPT_STICKY register
 *
 * \details 
 * Field: ::VTSS_GFP_GFP_BASE_GFP_STATUS . RX_INTERRUPT_PENDING
 */
#define  VTSS_F_GFP_GFP_BASE_GFP_STATUS_RX_INTERRUPT_PENDING(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_GFP_GFP_BASE_GFP_STATUS_RX_INTERRUPT_PENDING     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_GFP_GFP_BASE_GFP_STATUS_RX_INTERRUPT_PENDING(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Tx 8B10B interrupt pending.
 * This bit is set when there is an un-masked interrupt pending in the
 * TX_8B10B_INTERRUPT_STICKY register
 *
 * \details 
 * Field: ::VTSS_GFP_GFP_BASE_GFP_STATUS . TX_8B10B_INTERRUPT_PENDING
 */
#define  VTSS_F_GFP_GFP_BASE_GFP_STATUS_TX_8B10B_INTERRUPT_PENDING(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_GFP_GFP_BASE_GFP_STATUS_TX_8B10B_INTERRUPT_PENDING     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_GFP_GFP_BASE_GFP_STATUS_TX_8B10B_INTERRUPT_PENDING(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Tx PP/OS interrupt pending.
 * This bit is set when there is an un-masked interrupt pending in the
 * TX_PPOS_INTERRUPT_STICKY register
 *
 * \details 
 * Field: ::VTSS_GFP_GFP_BASE_GFP_STATUS . TX_PPOS_INTERRUPT_PENDING
 */
#define  VTSS_F_GFP_GFP_BASE_GFP_STATUS_TX_PPOS_INTERRUPT_PENDING(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_GFP_GFP_BASE_GFP_STATUS_TX_PPOS_INTERRUPT_PENDING     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_GFP_GFP_BASE_GFP_STATUS_TX_PPOS_INTERRUPT_PENDING(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Tx interrupt pending.
 * This bit is set when there is an un-masked interrupt pending in the
 * TX_INTERRUPT_STICKY register
 *
 * \details 
 * Field: ::VTSS_GFP_GFP_BASE_GFP_STATUS . TX_INTERRUPT_PENDING
 */
#define  VTSS_F_GFP_GFP_BASE_GFP_STATUS_TX_INTERRUPT_PENDING(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_GFP_GFP_BASE_GFP_STATUS_TX_INTERRUPT_PENDING     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_GFP_GFP_BASE_GFP_STATUS_TX_INTERRUPT_PENDING(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief GFP client data UPI register
 *
 * \details
 * Register: \a GFP::GFP_BASE::GFP_CLIENT_DATA_UPI
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFP_BASE_GFP_CLIENT_DATA_UPI(target)  VTSS_IOREG(target,0x2)

/** 
 * \brief
 * Mismatched client data frame UPI.
 * This value is captured from the UPI field when a client data frame is
 * received with the following conditions:
 * UPI does not match the CDF_UPI_VALUE
 * CLIENT_MODE_SEL is set to 10
 * CDF_UPI_MISMATCH_IRQ_STICKY is 0	   
 *
 * \details 
 * Field: ::VTSS_GFP_GFP_BASE_GFP_CLIENT_DATA_UPI . MISMATCHED_CDF_UPI
 */
#define  VTSS_F_GFP_GFP_BASE_GFP_CLIENT_DATA_UPI_MISMATCHED_CDF_UPI(x)  VTSS_ENCODE_BITFIELD(x,16,8)
#define  VTSS_M_GFP_GFP_BASE_GFP_CLIENT_DATA_UPI_MISMATCHED_CDF_UPI     VTSS_ENCODE_BITMASK(16,8)
#define  VTSS_X_GFP_GFP_BASE_GFP_CLIENT_DATA_UPI_MISMATCHED_CDF_UPI(x)  VTSS_EXTRACT_BITFIELD(x,16,8)

/** 
 * \brief
 * Client data frame UPI mismatch discard.
 * This bit enables discarding of received client data frames when the UPI
 * does not match the CDF_UPI_VALUE and the CLIENT_MODE_SEL is set to 10
 *
 * \details 
 * 0: Do not discard client data frames with mismatched UPI
 * 1: Discard client data frames with mismatched UPI	   
 *
 * Field: ::VTSS_GFP_GFP_BASE_GFP_CLIENT_DATA_UPI . CDF_UPI_MISMATCH_DISCARD
 */
#define  VTSS_F_GFP_GFP_BASE_GFP_CLIENT_DATA_UPI_CDF_UPI_MISMATCH_DISCARD(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_GFP_GFP_BASE_GFP_CLIENT_DATA_UPI_CDF_UPI_MISMATCH_DISCARD     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_GFP_GFP_BASE_GFP_CLIENT_DATA_UPI_CDF_UPI_MISMATCH_DISCARD(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Client data frame UPI select.
 * This bit selects the source of the UPI field for transmitted client data
 * frames when the CLIENT_MODE_SEL is set to 10   
 *
 * \details 
 * 0: Use the UPI sent with the frame from the upstream block
 * 1: Use the UPI from the CDF_UPI_VALUE field
 *
 * Field: ::VTSS_GFP_GFP_BASE_GFP_CLIENT_DATA_UPI . CDF_UPI_SEL
 */
#define  VTSS_F_GFP_GFP_BASE_GFP_CLIENT_DATA_UPI_CDF_UPI_SEL(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_GFP_GFP_BASE_GFP_CLIENT_DATA_UPI_CDF_UPI_SEL     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_GFP_GFP_BASE_GFP_CLIENT_DATA_UPI_CDF_UPI_SEL(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Client data frame UPI value.
 * This value is inserted into the UPI field of transmitted client data
 * frames when the CLIENT_MODE_SEL is set to 10 and the CDF_UPI_SEL is set
 * to 1. This value is also compared with the UPI field of received client
 * data frames to detect a UPI mismatch.
 *
 * \details 
 * Field: ::VTSS_GFP_GFP_BASE_GFP_CLIENT_DATA_UPI . CDF_UPI_VALUE
 */
#define  VTSS_F_GFP_GFP_BASE_GFP_CLIENT_DATA_UPI_CDF_UPI_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_GFP_GFP_BASE_GFP_CLIENT_DATA_UPI_CDF_UPI_VALUE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_GFP_GFP_BASE_GFP_CLIENT_DATA_UPI_CDF_UPI_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief GFP UPI interrupt register
 *
 * \details
 * Register: \a GFP::GFP_BASE::GFP_UPI_INTERRUPT_STICKY
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFP_BASE_GFP_UPI_INTERRUPT_STICKY(target)  VTSS_IOREG(target,0x3)

/** 
 * \brief
 * Client data frame UPI mismatch IRQ sticky.
 * This IRQ bit is set when a client data frame is received and the UPI
 * does not match the CDF_UPI_VALUE and the CLIENT_MODE_SEL is set to 10.
 * This is a sticky bit meaning that once set, it will remain set until a
 * value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFP_BASE_GFP_UPI_INTERRUPT_STICKY . CDF_UPI_MISMATCH_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFP_BASE_GFP_UPI_INTERRUPT_STICKY_CDF_UPI_MISMATCH_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_GFP_GFP_BASE_GFP_UPI_INTERRUPT_STICKY_CDF_UPI_MISMATCH_IRQ_STICKY     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_GFP_GFP_BASE_GFP_UPI_INTERRUPT_STICKY_CDF_UPI_MISMATCH_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief GFP UPI interrupt mask register
 *
 * \details
 * Register: \a GFP::GFP_BASE::GFP_UPI_INTERRUPT_MASK
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFP_BASE_GFP_UPI_INTERRUPT_MASK(target)  VTSS_IOREG(target,0x4)

/** 
 * \brief
 * Client data frame UPI mismatch IRQ mask.
 * This IRQ mask bit is set to cause the assertion of the GFP_CPU_INT
 * output when a client data frame UPI mismatch interrupt occurs. If this
 * IRQ mask bit is cleared, a client data frame UPI mismatch interrupt
 * condition will not cause the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFP_BASE_GFP_UPI_INTERRUPT_MASK . CDF_UPI_MISMATCH_IRQ_MASK
 */
#define  VTSS_F_GFP_GFP_BASE_GFP_UPI_INTERRUPT_MASK_CDF_UPI_MISMATCH_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_GFP_GFP_BASE_GFP_UPI_INTERRUPT_MASK_CDF_UPI_MISMATCH_IRQ_MASK     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_GFP_GFP_BASE_GFP_UPI_INTERRUPT_MASK_CDF_UPI_MISMATCH_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/**
 * Register Group: \a GFP::GFPM_TX_ISR
 *
 * GFP mapper Tx interrupt status registers
 */


/** 
 * \brief GFPM Tx interrupt status register 
 *
 * \details
 * Register: \a GFP::GFPM_TX_ISR::TX_INTERRUPT_STATUS
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_ISR_TX_INTERRUPT_STATUS(target)  VTSS_IOREG(target,0x20)

/** 
 * \brief
 * FIFO write thresh high status.
 * The FIFO write thresh high status bit is set when the difference between
 * the transmit FIFO read and write pointers exceeds the programmed write
 * high threshold value. 
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_ISR_TX_INTERRUPT_STATUS . FIFO_WRITE_THRESH_HIGH_STATUS
 */
#define  VTSS_F_GFP_GFPM_TX_ISR_TX_INTERRUPT_STATUS_FIFO_WRITE_THRESH_HIGH_STATUS(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_GFP_GFPM_TX_ISR_TX_INTERRUPT_STATUS_FIFO_WRITE_THRESH_HIGH_STATUS     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_GFP_GFPM_TX_ISR_TX_INTERRUPT_STATUS_FIFO_WRITE_THRESH_HIGH_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * FIFO write thresh low status.
 * The FIFO write thresh low status bit is set when the difference between
 * the transmit FIFO read and write pointers falls below the programmed
 * write low threshold value.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_ISR_TX_INTERRUPT_STATUS . FIFO_WRITE_THRESH_LOW_STATUS
 */
#define  VTSS_F_GFP_GFPM_TX_ISR_TX_INTERRUPT_STATUS_FIFO_WRITE_THRESH_LOW_STATUS(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_GFP_GFPM_TX_ISR_TX_INTERRUPT_STATUS_FIFO_WRITE_THRESH_LOW_STATUS     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_GFP_GFPM_TX_ISR_TX_INTERRUPT_STATUS_FIFO_WRITE_THRESH_LOW_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * FIFO read thresh high status.
 * The FIFO read thresh high status bit is set when the difference between
 * the transmit FIFO read and write pointers exceeds the programmed read
 * high threshold value.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_ISR_TX_INTERRUPT_STATUS . FIFO_READ_THRESH_HIGH_STATUS
 */
#define  VTSS_F_GFP_GFPM_TX_ISR_TX_INTERRUPT_STATUS_FIFO_READ_THRESH_HIGH_STATUS(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_TX_ISR_TX_INTERRUPT_STATUS_FIFO_READ_THRESH_HIGH_STATUS     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_TX_ISR_TX_INTERRUPT_STATUS_FIFO_READ_THRESH_HIGH_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * FIFO read thresh low status.
 * The FIFO read thresh low status bit is set when the difference between
 * the transmit FIFO read and write pointers falls below the programmed
 * read low threshold value.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_ISR_TX_INTERRUPT_STATUS . FIFO_READ_THRESH_LOW_STATUS
 */
#define  VTSS_F_GFP_GFPM_TX_ISR_TX_INTERRUPT_STATUS_FIFO_READ_THRESH_LOW_STATUS(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_GFP_GFPM_TX_ISR_TX_INTERRUPT_STATUS_FIFO_READ_THRESH_LOW_STATUS     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_GFP_GFPM_TX_ISR_TX_INTERRUPT_STATUS_FIFO_READ_THRESH_LOW_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,12,1)


/** 
 * \brief GFPM Tx interrupt register 
 *
 * \details
 * Register: \a GFP::GFPM_TX_ISR::TX_INTERRUPT_STICKY
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_ISR_TX_INTERRUPT_STICKY(target)  VTSS_IOREG(target,0x21)

/** 
 * \brief
 * FIFO write thresh high IRQ sticky.
 * The FIFO write thresh high IRQ bit is set when the associated
 * FIFO_WRITE_THRESH_HIGH_STATUS bit changes state. This is a sticky bit
 * meaning that once set, it will remain set until a value of 1 is written
 * to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_ISR_TX_INTERRUPT_STICKY . FIFO_WRITE_THRESH_HIGH_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_TX_ISR_TX_INTERRUPT_STICKY_FIFO_WRITE_THRESH_HIGH_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_GFP_GFPM_TX_ISR_TX_INTERRUPT_STICKY_FIFO_WRITE_THRESH_HIGH_IRQ_STICKY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_GFP_GFPM_TX_ISR_TX_INTERRUPT_STICKY_FIFO_WRITE_THRESH_HIGH_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * FIFO write thresh low IRQ sticky.
 * The FIFO write thresh low IRQ bit is set when the associated
 * FIFO_WRITE_THRESH_LOW_STATUS bit changes state. This is a sticky bit
 * meaning that once set, it will remain set until a value of 1 is written
 * to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_ISR_TX_INTERRUPT_STICKY . FIFO_WRITE_THRESH_LOW_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_TX_ISR_TX_INTERRUPT_STICKY_FIFO_WRITE_THRESH_LOW_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_GFP_GFPM_TX_ISR_TX_INTERRUPT_STICKY_FIFO_WRITE_THRESH_LOW_IRQ_STICKY     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_GFP_GFPM_TX_ISR_TX_INTERRUPT_STICKY_FIFO_WRITE_THRESH_LOW_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * FIFO read thresh high IRQ sticky.
 * The FIFO read thresh high IRQ bit is set when the associated
 * FIFO_READ_THRESH_HIGH_STATUS bit changes state. This is a sticky bit
 * meaning that once set, it will remain set until a value of 1 is written
 * to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_ISR_TX_INTERRUPT_STICKY . FIFO_READ_THRESH_HIGH_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_TX_ISR_TX_INTERRUPT_STICKY_FIFO_READ_THRESH_HIGH_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_TX_ISR_TX_INTERRUPT_STICKY_FIFO_READ_THRESH_HIGH_IRQ_STICKY     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_TX_ISR_TX_INTERRUPT_STICKY_FIFO_READ_THRESH_HIGH_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * FIFO read thresh low IRQ sticky.
 * The FIFO read thresh low IRQ bit is set when the associated
 * FIFO_READ_THRESH_LOW_STATUS bit changes state. This is a sticky bit
 * meaning that once set, it will remain set until a value of 1 is written
 * to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_ISR_TX_INTERRUPT_STICKY . FIFO_READ_THRESH_LOW_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_TX_ISR_TX_INTERRUPT_STICKY_FIFO_READ_THRESH_LOW_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_GFP_GFPM_TX_ISR_TX_INTERRUPT_STICKY_FIFO_READ_THRESH_LOW_IRQ_STICKY     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_GFP_GFPM_TX_ISR_TX_INTERRUPT_STICKY_FIFO_READ_THRESH_LOW_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * FIFO overflow IRQ sticky.
 * The FIFO overflow IRQ bit is set when a transmit FIFO overflow condition
 * is detected. This is a sticky bit meaning that once set, it will remain
 * set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_ISR_TX_INTERRUPT_STICKY . FIFO_OVERFLOW_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_TX_ISR_TX_INTERRUPT_STICKY_FIFO_OVERFLOW_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_GFP_GFPM_TX_ISR_TX_INTERRUPT_STICKY_FIFO_OVERFLOW_IRQ_STICKY     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_GFP_GFPM_TX_ISR_TX_INTERRUPT_STICKY_FIFO_OVERFLOW_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * FIFO underflow IRQ sticky.
 * The FIFO underflow IRQ bit is set when a transmit FIFO underflow
 * condition is detected. This is a sticky bit meaning that once set, it
 * will remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_ISR_TX_INTERRUPT_STICKY . FIFO_UNDERFLOW_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_TX_ISR_TX_INTERRUPT_STICKY_FIFO_UNDERFLOW_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_GFP_GFPM_TX_ISR_TX_INTERRUPT_STICKY_FIFO_UNDERFLOW_IRQ_STICKY     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_GFP_GFPM_TX_ISR_TX_INTERRUPT_STICKY_FIFO_UNDERFLOW_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,10,1)


/** 
 * \brief GFPM Tx interrupt mask register 
 *
 * \details
 * Register: \a GFP::GFPM_TX_ISR::TX_INTERRUPT_MASK
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_ISR_TX_INTERRUPT_MASK(target)  VTSS_IOREG(target,0x22)

/** 
 * \brief
 * FIFO write thresh high IRQ mask.
 * The FIFO write thresh high IRQ mask bit is set to cause the assertion of
 * the GFP_CPU_INT output when a FIFO write threshold high interrupt
 * occurs. If the FIFO write thresh high IRQ mask bit is cleared, a FIFO
 * write threshold high interrupt condition will not cause the assertion of
 * the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_ISR_TX_INTERRUPT_MASK . FIFO_WRITE_THRESH_HIGH_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_TX_ISR_TX_INTERRUPT_MASK_FIFO_WRITE_THRESH_HIGH_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_GFP_GFPM_TX_ISR_TX_INTERRUPT_MASK_FIFO_WRITE_THRESH_HIGH_IRQ_MASK     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_GFP_GFPM_TX_ISR_TX_INTERRUPT_MASK_FIFO_WRITE_THRESH_HIGH_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * FIFO write thresh low IRQ mask.
 * The FIFO write thresh low IRQ mask bit is set to cause the assertion of
 * the GFP_CPU_INT output when a FIFO write threshold low interrupt occurs.
 * If the FIFO write thresh low IRQ mask bit is cleared, a FIFO write
 * threshold low interrupt condition will not cause the assertion of the
 * GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_ISR_TX_INTERRUPT_MASK . FIFO_WRITE_THRESH_LOW_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_TX_ISR_TX_INTERRUPT_MASK_FIFO_WRITE_THRESH_LOW_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_GFP_GFPM_TX_ISR_TX_INTERRUPT_MASK_FIFO_WRITE_THRESH_LOW_IRQ_MASK     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_GFP_GFPM_TX_ISR_TX_INTERRUPT_MASK_FIFO_WRITE_THRESH_LOW_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * FIFO read thresh high IRQ mask.
 * The FIFO read thresh high IRQ mask bit is set to cause the assertion of
 * the GFP_CPU_INT output when a FIFO read threshold high interrupt occurs.
 * If the FIFO read thresh high IRQ mask bit is cleared, a FIFO read
 * threshold high interrupt condition will not cause the assertion of the
 * GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_ISR_TX_INTERRUPT_MASK . FIFO_READ_THRESH_HIGH_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_TX_ISR_TX_INTERRUPT_MASK_FIFO_READ_THRESH_HIGH_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_TX_ISR_TX_INTERRUPT_MASK_FIFO_READ_THRESH_HIGH_IRQ_MASK     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_TX_ISR_TX_INTERRUPT_MASK_FIFO_READ_THRESH_HIGH_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * FIFO read thresh low IRQ mask.
 * The FIFO read thresh low IRQ mask bit is set to cause the assertion of
 * the GFP_CPU_INT output when a FIFO read threshold low interrupt occurs.
 * If the FIFO read thresh low IRQ mask bit is cleared, a FIFO read
 * threshold low interrupt condition will not cause the assertion of the
 * GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_ISR_TX_INTERRUPT_MASK . FIFO_READ_THRESH_LOW_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_TX_ISR_TX_INTERRUPT_MASK_FIFO_READ_THRESH_LOW_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_GFP_GFPM_TX_ISR_TX_INTERRUPT_MASK_FIFO_READ_THRESH_LOW_IRQ_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_GFP_GFPM_TX_ISR_TX_INTERRUPT_MASK_FIFO_READ_THRESH_LOW_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * FIFO overflow IRQ mask.
 * The FIFO overflow IRQ mask bit is set to cause the assertion of the
 * GFP_CPU_INT output when a FIFO overflow interrupt occurs. If the FIFO
 * overflow IRQ mask bit is cleared, a FIFO overflow interrupt condition
 * will not cause the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_ISR_TX_INTERRUPT_MASK . FIFO_OVERFLOW_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_TX_ISR_TX_INTERRUPT_MASK_FIFO_OVERFLOW_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_GFP_GFPM_TX_ISR_TX_INTERRUPT_MASK_FIFO_OVERFLOW_IRQ_MASK     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_GFP_GFPM_TX_ISR_TX_INTERRUPT_MASK_FIFO_OVERFLOW_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * FIFO underflow IRQ mask.
 * The FIFO underflow IRQ mask bit is set to cause the assertion of the
 * GFP_CPU_INT output when a FIFO underflow interrupt occurs. If the FIFO
 * underflow IRQ mask bit is cleared, a FIFO underflow interrupt condition
 * will not cause the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_ISR_TX_INTERRUPT_MASK . FIFO_UNDERFLOW_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_TX_ISR_TX_INTERRUPT_MASK_FIFO_UNDERFLOW_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_GFP_GFPM_TX_ISR_TX_INTERRUPT_MASK_FIFO_UNDERFLOW_IRQ_MASK     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_GFP_GFPM_TX_ISR_TX_INTERRUPT_MASK_FIFO_UNDERFLOW_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/**
 * Register Group: \a GFP::GFPM_TX_PPOS_ISR
 *
 * GFP mapper Tx PP/OS interrupt status registers
 */


/** 
 * \brief GFPM Tx PP/OS interrupt register 
 *
 * \details
 * Register: \a GFP::GFPM_TX_PPOS_ISR::TX_PPOS_INTERRUPT_STICKY
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY(target)  VTSS_IOREG(target,0x41)

/** 
 * \brief
 * Error character detect IRQ sticky.
 * The error character detect IRQ bit is set when an incoming error
 * character is detected. This is a sticky bit meaning that once set, it
 * will remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY . ERROR_CHARACTER_DETECT_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY_ERROR_CHARACTER_DETECT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY_ERROR_CHARACTER_DETECT_IRQ_STICKY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY_ERROR_CHARACTER_DETECT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Minimum packet violation IRQ sticky.
 * The minimum packet violation IRQ bit is set when an incoming minimum
 * packet violation is detected. This is a sticky bit meaning that once
 * set, it will remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY . MIN_PKT_VIOLATION_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY_MIN_PKT_VIOLATION_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY_MIN_PKT_VIOLATION_IRQ_STICKY     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY_MIN_PKT_VIOLATION_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Maximum packet violation IRQ sticky.
 * The maximum packet violation IRQ bit is set when an incoming maximum
 * packet violation is detected. This is a sticky bit meaning that once
 * set, it will remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY . MAX_PKT_VIOLATION_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY_MAX_PKT_VIOLATION_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY_MAX_PKT_VIOLATION_IRQ_STICKY     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY_MAX_PKT_VIOLATION_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Invalid control byte lane IRQ sticky.
 * The invalid control byte lane IRQ bit is set when an ordered set or
 * start of packet is detected on an invalid byte lane. This is a sticky
 * bit meaning that once set, it will remain set until a value of 1 is
 * written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY . INVALID_CONTROL_BYTE_LANE_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY_INVALID_CONTROL_BYTE_LANE_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY_INVALID_CONTROL_BYTE_LANE_IRQ_STICKY     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY_INVALID_CONTROL_BYTE_LANE_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Invalid control detect IRQ sticky.
 * The invalid control byte lane IRQ bit is set when a control character
 * other than TERMINATE is detected after data packet reception has
 * started. This is a sticky bit meaning that once set, it will remain set
 * until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY . INVALID_CONTROL_DETECT_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY_INVALID_CONTROL_DETECT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY_INVALID_CONTROL_DETECT_IRQ_STICKY     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY_INVALID_CONTROL_DETECT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Packet length mismatch IRQ sticky.
 * The packet length mismatch IRQ bit is set when the external packet
 * length does not match the calculated incoming data packet length. For
 * more information when the packet length is valid, see Tx PP/OS global
 * control register external packet length bit description. This is a
 * sticky bit meaning that once set, it will remain set until a value of 1
 * is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY . PKT_LENGTH_MISMATCH_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY_PKT_LENGTH_MISMATCH_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY_PKT_LENGTH_MISMATCH_IRQ_STICKY     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY_PKT_LENGTH_MISMATCH_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Minimum IFG violation IRQ sticky.
 * The minimum IFG violation IRQ bit is set when the incoming gap between
 * incoming ethernet frames is less than 5. This is a sticky bit meaning
 * that once set, it will remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY . MIN_IFG_VIOLATION_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY_MIN_IFG_VIOLATION_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY_MIN_IFG_VIOLATION_IRQ_STICKY     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_STICKY_MIN_IFG_VIOLATION_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,9,1)


/** 
 * \brief GFPM Tx PP/OS interrupt mask register   
 *
 * \details
 * Register: \a GFP::GFPM_TX_PPOS_ISR::TX_PPOS_INTERRUPT_MASK
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK(target)  VTSS_IOREG(target,0x42)

/** 
 * \brief
 * Error character detect IRQ mask.
 * The error character detect IRQ mask bit is set to cause the assertion of
 * the GFP_CPU_INT output when an error character detect interrupt
 * condition exists. If the error character detect irq mask bit is cleared,
 * an error character detect interrupt condition will not cause the
 * assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK . ERROR_CHARACTER_DETECT_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK_ERROR_CHARACTER_DETECT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK_ERROR_CHARACTER_DETECT_IRQ_MASK     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK_ERROR_CHARACTER_DETECT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Minimum packet violation IRQ mask.
 * The minimum packet violation IRQ mask bit is set to cause the assertion
 * of the GFP_CPU_INT output when a minimum packet violation interrupt
 * condition exists. If the minimum packet violation IRQ mask bit is
 * cleared, a minimum packet violation interrupt condition will not cause
 * the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK . MIN_PKT_VIOLATION_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK_MIN_PKT_VIOLATION_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK_MIN_PKT_VIOLATION_IRQ_MASK     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK_MIN_PKT_VIOLATION_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Maximum packet violation IRQ mask.
 * The maximum packet violation IRQ mask bit is set to cause the assertion
 * of the GFP_CPU_INT output when a maximum packet violation interrupt
 * condition exists. If the maximum packet violation IRQ mask bit is
 * cleared, a maximum packet violation interrupt condition will not cause
 * the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK . MAX_PKT_VIOLATION_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK_MAX_PKT_VIOLATION_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK_MAX_PKT_VIOLATION_IRQ_MASK     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK_MAX_PKT_VIOLATION_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Invalid control byte lane IRQ mask.
 * The invalid control byte lane IRQ mask bit is set to cause the assertion
 * of the GFP_CPU_INT output when an invalid control byte lane interrupt
 * condition exists. If the invalid control byte lane IRQ mask bit is
 * cleared, an invalid control byte lane interrupt condition will not cause
 * the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK . INVALID_CONTROL_BYTE_LANE_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK_INVALID_CONTROL_BYTE_LANE_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK_INVALID_CONTROL_BYTE_LANE_IRQ_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK_INVALID_CONTROL_BYTE_LANE_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Invalid control detect IRQ mask.
 * The invalid control detect IRQ mask bit is set to cause the assertion of
 * the GFP_CPU_INT output when an invalid control detect interrupt
 * condition exists. If the invalid control detect IRQ mask bit is cleared,
 * an invalid control detect interrupt condition will not cause the
 * assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK . INVALID_CONTROL_DETECT_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK_INVALID_CONTROL_DETECT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK_INVALID_CONTROL_DETECT_IRQ_MASK     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK_INVALID_CONTROL_DETECT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Packet length mismatch IRQ mask.
 * The packet length mismatch IRQ mask bit is set to cause the assertion of
 * the GFP_CPU_INT output when a packet length mismatch interrupt condition
 * exists. If the packet length mismatch IRQ mask bit is cleared, packet
 * length mismatch interrupt condition will not cause the assertion of the
 * GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK . PKT_LENGTH_MISMATCH_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK_PKT_LENGTH_MISMATCH_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK_PKT_LENGTH_MISMATCH_IRQ_MASK     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK_PKT_LENGTH_MISMATCH_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Minimum IFG violation IRQ mask.
 * The minimum IFG violation IRQ mask bit is set to cause the assertion of
 * the GFP_CPU_INT output when a minimum IFG violation interrupt condition
 * exists. If the minimum IFG violation IRQ mask bit is cleared, a minimum
 * IFG violation interrupt condition will not cause the assertion of the
 * GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK . MIN_IFG_VIOLATION_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK_MIN_IFG_VIOLATION_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK_MIN_IFG_VIOLATION_IRQ_MASK     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_GFP_GFPM_TX_PPOS_ISR_TX_PPOS_INTERRUPT_MASK_MIN_IFG_VIOLATION_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/**
 * Register Group: \a GFP::GFPM_TX_8B10B_ISR
 *
 * GFP mapper Tx 8B/10B interrupt status registers
 */


/** 
 * \brief GFPM Tx 8B/10B interrupt status register   
 *
 * \details
 * Register: \a GFP::GFPM_TX_8B10B_ISR::TX_8B10B_INTERRUPT_STATUS
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STATUS(target)  VTSS_IOREG(target,0x60)

/** 
 * \brief
 * Not operational status.
 * The not operational status bit is set when the 8B/10B decoder is
 * disabled or /RESET IN = 0 or the sw reset bit of the Tx 8B/10B global
 * control register is asserted.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STATUS . NOT_OPERATIONAL_STATUS
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STATUS_NOT_OPERATIONAL_STATUS(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STATUS_NOT_OPERATIONAL_STATUS     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STATUS_NOT_OPERATIONAL_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * LOS status.
 * The LOS status bit is set when TX LOS IN = 1 or the force LOS bit of the
 * Tx 8B/10B global control register is asserted.    
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STATUS . LOS_STATUS
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STATUS_LOS_STATUS(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STATUS_LOS_STATUS     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STATUS_LOS_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * LSYNC status.
 * The LSYNC status bit is set when the internal 8B/10B decoder
 * synchronization state machine is in the loss of synchronization state.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STATUS . LSYNC_STATUS
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STATUS_LSYNC_STATUS(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STATUS_LSYNC_STATUS     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STATUS_LSYNC_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,13,1)


/** 
 * \brief GFPM Tx 8B/10B interrupt register   
 *
 * \details
 * Register: \a GFP::GFPM_TX_8B10B_ISR::TX_8B10B_INTERRUPT_STICKY
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY(target)  VTSS_IOREG(target,0x61)

/** 
 * \brief
 * Not operational IRQ sticky.
 * The not operational IRQ bit is set when the associated
 * NOT_OPERATIONAL_STATUS bit changes state. This is a sticky bit meaning
 * that once set, it will remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY . NOT_OPERATIONAL_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_NOT_OPERATIONAL_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_NOT_OPERATIONAL_IRQ_STICKY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_NOT_OPERATIONAL_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * LOS IRQ sticky.
 * The LOS IRQ bit is set when TX LOS IN = 1 or the force LOS bit of the Tx
 * 8B/10B global control register is asserted. This is a sticky bit meaning
 * that once set, it will remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY . LOS_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_LOS_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_LOS_IRQ_STICKY     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_LOS_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * LSYNC IRQ sticky.
 * The LSYNC IRQ bit is set when the associated LSYNC_STATUS bit changes
 * state. This is a sticky bit meaning that once set, it will remain set
 * until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY . LSYNC_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_LSYNC_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_LSYNC_IRQ_STICKY     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_LSYNC_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Sync changed IRQ sticky.
 * The sync changed IRQ bit is set when the incoming alignment changes
 * (different than previous position) in the synchronization state machine.
 * This is a sticky bit meaning that once set, it will remain set until a
 * value of 1 is written to it.      
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY . SYNC_CHANGED_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_SYNC_CHANGED_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_SYNC_CHANGED_IRQ_STICKY     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_SYNC_CHANGED_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Invalid character IRQ sticky.
 * The invalid character IRQ bit is set when an incoming invalid character
 * is detected while the synchronization state machine is in the SYNC
 * state. This is a sticky bit meaning that once set, it will remain set
 * until a value of 1 is written to it.    
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY . INVALID_CHARACTER_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_INVALID_CHARACTER_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_INVALID_CHARACTER_IRQ_STICKY     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_INVALID_CHARACTER_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Special character misalign IRQ sticky.
 * The special character misalign IRQ bit is set when an incoming valid
 * special character is detected in the wrong byte lane (byte lanes 4-6 or
 * 0-3 where byte lane 7 is most significant) while the synchronization
 * state machine is in the SYNC state. This is a sticky bit meaning that
 * once set, it will remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY . SPECIAL_CHARACTER_MISALIGN_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_SPECIAL_CHARACTER_MISALIGN_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_SPECIAL_CHARACTER_MISALIGN_IRQ_STICKY     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_SPECIAL_CHARACTER_MISALIGN_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Disparity error IRQ sticky.
 * The disparity error IRQ bit is set when an incoming disparity error is
 * detected while the synchronization state machine is in the SYNC state.
 * This is a sticky bit meaning that once set, it will remain set until a
 * value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY . DISPARITY_ERROR_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_DISPARITY_ERROR_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_DISPARITY_ERROR_IRQ_STICKY     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_DISPARITY_ERROR_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,9,1)


/** 
 * \brief GFPM Tx 8B/10B interrupt mask register   
 *
 * \details
 * Register: \a GFP::GFPM_TX_8B10B_ISR::TX_8B10B_INTERRUPT_MASK
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK(target)  VTSS_IOREG(target,0x62)

/** 
 * \brief
 * Not operational IRQ mask.
 * The not operational IRQ mask bit is set to cause the assertion of the
 * GFP_CPU_INT output when a not operational interrupt condition exists. If
 * the not operational IRQ mask bit is cleared, a not operational interrupt
 * condition will not cause the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK . NOT_OPERATIONAL_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK_NOT_OPERATIONAL_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK_NOT_OPERATIONAL_IRQ_MASK     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK_NOT_OPERATIONAL_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * LOS IRQ mask.
 * The LOS IRQ mask bit is set to cause the assertion of the GFP_CPU_INT
 * output when an LOS interrupt condition exists. If the LOS IRQ mask bit
 * is cleared, an LOS interrupt condition will not cause the assertion of
 * the GFP_CPU_INT output. 
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK . LOS_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK_LOS_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK_LOS_IRQ_MASK     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK_LOS_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * LSYNC IRQ mask.
 * The LSYNC IRQ mask bit is set to cause the assertion of the GFP_CPU_INT
 * output when an LSYNC interrupt condition exists. If the LSYNC IRQ mask
 * bit is cleared, a loss of synchronization interrupt condition will not
 * cause the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK . LSYNC_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK_LSYNC_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK_LSYNC_IRQ_MASK     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK_LSYNC_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Sync changed IRQ mask.
 * The sync changed IRQ mask bit is set to cause the assertion of the
 * GFP_CPU_INT output when a sync changed interrupt condition exists. If
 * the sync changed IRQ mask bit is cleared, a sync changed interrupt
 * condition will not cause the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK . SYNC_CHANGED_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK_SYNC_CHANGED_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK_SYNC_CHANGED_IRQ_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK_SYNC_CHANGED_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Invalid character IRQ mask.
 * The invalid character IRQ mask bit is set to cause the assertion of the
 * GFP_CPU_INT output when an invalid character interrupt condition exists.
 * If the invalid character IRQ mask bit is cleared, an invalid character
 * interrupt condition will not cause the assertion of the GFP_CPU_INT
 * output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK . INVALID_CHARACTER_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK_INVALID_CHARACTER_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK_INVALID_CHARACTER_IRQ_MASK     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK_INVALID_CHARACTER_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Special character misalign IRQ mask.
 * The special character misalign IRQ mask bit is set to cause the
 * assertion of the GFP_CPU_INT output when a special character misalign
 * interrupt condition exists. If the special character misalign IRQ mask
 * bit is cleared, a special character misalign interrupt condition will
 * not cause the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK . SPECIAL_CHARACTER_MISALIGN_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK_SPECIAL_CHARACTER_MISALIGN_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK_SPECIAL_CHARACTER_MISALIGN_IRQ_MASK     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK_SPECIAL_CHARACTER_MISALIGN_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Disparity error IRQ mask.
 * The disparity error IRQ mask bit is set to cause the assertion of the
 * GFP_CPU_INT output when a disparity error interrupt condition exists. If
 * the disparity error IRQ mask bit is cleared, a disparity error interrupt
 * condition will not cause the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK . DISPARITY_ERROR_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK_DISPARITY_ERROR_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK_DISPARITY_ERROR_IRQ_MASK     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK_DISPARITY_ERROR_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/**
 * Register Group: \a GFP::GFPM_RX_ISR
 *
 * GFP mapper Rx interrupt status registers
 */


/** 
 * \brief GFPM Rx interrupt status register 
 *
 * \details
 * Register: \a GFP::GFPM_RX_ISR::RX_INTERRUPT_STATUS
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_ISR_RX_INTERRUPT_STATUS(target)  VTSS_IOREG(target,0x80)

/** 
 * \brief
 * FIFO write thresh high status.
 * The FIFO write thresh high status bit is set when the difference between
 * the transmit FIFO read and write pointers exceeds the programmed write
 * high threshold value.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_ISR_RX_INTERRUPT_STATUS . FIFO_WRITE_THRESH_HIGH_STATUS
 */
#define  VTSS_F_GFP_GFPM_RX_ISR_RX_INTERRUPT_STATUS_FIFO_WRITE_THRESH_HIGH_STATUS(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_GFP_GFPM_RX_ISR_RX_INTERRUPT_STATUS_FIFO_WRITE_THRESH_HIGH_STATUS     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_GFP_GFPM_RX_ISR_RX_INTERRUPT_STATUS_FIFO_WRITE_THRESH_HIGH_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * FIFO write thresh low status.
 * The FIFO write thresh low status bit is set when the difference between
 * the transmit FIFO read and write pointers falls below the programmed
 * write low threshold value.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_ISR_RX_INTERRUPT_STATUS . FIFO_WRITE_THRESH_LOW_STATUS
 */
#define  VTSS_F_GFP_GFPM_RX_ISR_RX_INTERRUPT_STATUS_FIFO_WRITE_THRESH_LOW_STATUS(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_GFP_GFPM_RX_ISR_RX_INTERRUPT_STATUS_FIFO_WRITE_THRESH_LOW_STATUS     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_GFP_GFPM_RX_ISR_RX_INTERRUPT_STATUS_FIFO_WRITE_THRESH_LOW_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * FIFO read thresh high status.
 * The FIFO read thresh high status bit is set when the difference between
 * the transmit FIFO read and write pointers exceeds the programmed read
 * high threshold value.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_ISR_RX_INTERRUPT_STATUS . FIFO_READ_THRESH_HIGH_STATUS
 */
#define  VTSS_F_GFP_GFPM_RX_ISR_RX_INTERRUPT_STATUS_FIFO_READ_THRESH_HIGH_STATUS(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_RX_ISR_RX_INTERRUPT_STATUS_FIFO_READ_THRESH_HIGH_STATUS     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_RX_ISR_RX_INTERRUPT_STATUS_FIFO_READ_THRESH_HIGH_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * FIFO read thresh low status.
 * The FIFO read thresh low status bit is set when the difference between
 * the transmit FIFO read and write pointers falls below the programmed
 * read low threshold value.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_ISR_RX_INTERRUPT_STATUS . FIFO_READ_THRESH_LOW_STATUS
 */
#define  VTSS_F_GFP_GFPM_RX_ISR_RX_INTERRUPT_STATUS_FIFO_READ_THRESH_LOW_STATUS(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_GFP_GFPM_RX_ISR_RX_INTERRUPT_STATUS_FIFO_READ_THRESH_LOW_STATUS     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_GFP_GFPM_RX_ISR_RX_INTERRUPT_STATUS_FIFO_READ_THRESH_LOW_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,12,1)


/** 
 * \brief GFPM Rx interrupt register 
 *
 * \details
 * Register: \a GFP::GFPM_RX_ISR::RX_INTERRUPT_STICKY
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_ISR_RX_INTERRUPT_STICKY(target)  VTSS_IOREG(target,0x81)

/** 
 * \brief
 * FIFO write thresh high IRQ sticky.
 * The FIFO write thresh high IRQ bit is set when the associated
 * FIFO_WRITE_THRESH_HIGH_STATUS bit changes state. This is a sticky bit
 * meaning that once set, it will remain set until a value of 1 is written
 * to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_ISR_RX_INTERRUPT_STICKY . FIFO_WRITE_THRESH_HIGH_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_ISR_RX_INTERRUPT_STICKY_FIFO_WRITE_THRESH_HIGH_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_GFP_GFPM_RX_ISR_RX_INTERRUPT_STICKY_FIFO_WRITE_THRESH_HIGH_IRQ_STICKY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_GFP_GFPM_RX_ISR_RX_INTERRUPT_STICKY_FIFO_WRITE_THRESH_HIGH_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * FIFO write thresh low IRQ sticky.
 * The FIFO write thresh low IRQ bit is set when the associated
 * FIFO_WRITE_THRESH_LOW_STATUS bit changes state. This is a sticky bit
 * meaning that once set, it will remain set until a value of 1 is written
 * to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_ISR_RX_INTERRUPT_STICKY . FIFO_WRITE_THRESH_LOW_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_ISR_RX_INTERRUPT_STICKY_FIFO_WRITE_THRESH_LOW_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_GFP_GFPM_RX_ISR_RX_INTERRUPT_STICKY_FIFO_WRITE_THRESH_LOW_IRQ_STICKY     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_GFP_GFPM_RX_ISR_RX_INTERRUPT_STICKY_FIFO_WRITE_THRESH_LOW_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * FIFO read thresh high IRQ sticky.
 * The FIFO read thresh high IRQ bit is set when the associated
 * FIFO_READ_THRESH_HIGH_STATUS bit changes state. This is a sticky bit
 * meaning that once set, it will remain set until a value of 1 is written
 * to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_ISR_RX_INTERRUPT_STICKY . FIFO_READ_THRESH_HIGH_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_ISR_RX_INTERRUPT_STICKY_FIFO_READ_THRESH_HIGH_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_RX_ISR_RX_INTERRUPT_STICKY_FIFO_READ_THRESH_HIGH_IRQ_STICKY     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_RX_ISR_RX_INTERRUPT_STICKY_FIFO_READ_THRESH_HIGH_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * FIFO read thresh low IRQ sticky.
 * The FIFO read thresh low IRQ bit is set when the associated
 * FIFO_READ_THRESH_LOW_STATUS bit changes state. This is a sticky bit
 * meaning that once set, it will remain set until a value of 1 is written
 * to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_ISR_RX_INTERRUPT_STICKY . FIFO_READ_THRESH_LOW_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_ISR_RX_INTERRUPT_STICKY_FIFO_READ_THRESH_LOW_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_GFP_GFPM_RX_ISR_RX_INTERRUPT_STICKY_FIFO_READ_THRESH_LOW_IRQ_STICKY     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_GFP_GFPM_RX_ISR_RX_INTERRUPT_STICKY_FIFO_READ_THRESH_LOW_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * FIFO overflow IRQ sticky.
 * The FIFO overflow IRQ bit is set when a transmit FIFO overflow condition
 * is detected. This is a sticky bit meaning that once set, it will remain
 * set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_ISR_RX_INTERRUPT_STICKY . FIFO_OVERFLOW_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_ISR_RX_INTERRUPT_STICKY_FIFO_OVERFLOW_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_GFP_GFPM_RX_ISR_RX_INTERRUPT_STICKY_FIFO_OVERFLOW_IRQ_STICKY     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_GFP_GFPM_RX_ISR_RX_INTERRUPT_STICKY_FIFO_OVERFLOW_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * FIFO underflow IRQ sticky.
 * The FIFO underflow IRQ bit is set when a transmit FIFO underflow
 * condition is detected. This is a sticky bit meaning that once set, it
 * will remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_ISR_RX_INTERRUPT_STICKY . FIFO_UNDERFLOW_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_ISR_RX_INTERRUPT_STICKY_FIFO_UNDERFLOW_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_GFP_GFPM_RX_ISR_RX_INTERRUPT_STICKY_FIFO_UNDERFLOW_IRQ_STICKY     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_GFP_GFPM_RX_ISR_RX_INTERRUPT_STICKY_FIFO_UNDERFLOW_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,10,1)


/** 
 * \brief GFPM Rx interrupt mask register 
 *
 * \details
 * Register: \a GFP::GFPM_RX_ISR::RX_INTERRUPT_MASK
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_ISR_RX_INTERRUPT_MASK(target)  VTSS_IOREG(target,0x82)

/** 
 * \brief
 * FIFO write thresh high IRQ mask.
 * The FIFO write thresh high IRQ mask bit is set to cause the assertion of
 * the GFP_CPU_INT output when a FIFO write threshold high interrupt
 * occurs. If the FIFO write thresh high IRQ mask bit is cleared, a FIFO
 * write threshold high interrupt condition will not cause the assertion of
 * the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_ISR_RX_INTERRUPT_MASK . FIFO_WRITE_THRESH_HIGH_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_ISR_RX_INTERRUPT_MASK_FIFO_WRITE_THRESH_HIGH_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_GFP_GFPM_RX_ISR_RX_INTERRUPT_MASK_FIFO_WRITE_THRESH_HIGH_IRQ_MASK     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_GFP_GFPM_RX_ISR_RX_INTERRUPT_MASK_FIFO_WRITE_THRESH_HIGH_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * FIFO write thresh low IRQ mask.
 * The FIFO write thresh low IRQ mask bit is set to cause the assertion of
 * the GFP_CPU_INT output when a FIFO write threshold low interrupt occurs.
 * If the FIFO write thresh low IRQ mask bit is cleared, a FIFO write
 * threshold low interrupt condition will not cause the assertion of the
 * GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_ISR_RX_INTERRUPT_MASK . FIFO_WRITE_THRESH_LOW_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_ISR_RX_INTERRUPT_MASK_FIFO_WRITE_THRESH_LOW_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_GFP_GFPM_RX_ISR_RX_INTERRUPT_MASK_FIFO_WRITE_THRESH_LOW_IRQ_MASK     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_GFP_GFPM_RX_ISR_RX_INTERRUPT_MASK_FIFO_WRITE_THRESH_LOW_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * FIFO read thresh high IRQ mask.
 * The FIFO read thresh high IRQ mask bit is set to cause the assertion of
 * the GFP_CPU_INT output when a FIFO read threshold high interrupt occurs.
 * If the FIFO read thresh high IRQ mask bit is cleared, a FIFO read
 * threshold high interrupt condition will not cause the assertion of the
 * GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_ISR_RX_INTERRUPT_MASK . FIFO_READ_THRESH_HIGH_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_ISR_RX_INTERRUPT_MASK_FIFO_READ_THRESH_HIGH_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_RX_ISR_RX_INTERRUPT_MASK_FIFO_READ_THRESH_HIGH_IRQ_MASK     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_RX_ISR_RX_INTERRUPT_MASK_FIFO_READ_THRESH_HIGH_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * FIFO read thresh low IRQ mask.
 * The FIFO read thresh low IRQ mask bit is set to cause the assertion of
 * the GFP_CPU_INT output when a FIFO read threshold low interrupt occurs.
 * If the FIFO read thresh low IRQ mask bit is cleared, a FIFO read
 * threshold low interrupt condition will not cause the assertion of the
 * GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_ISR_RX_INTERRUPT_MASK . FIFO_READ_THRESH_LOW_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_ISR_RX_INTERRUPT_MASK_FIFO_READ_THRESH_LOW_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_GFP_GFPM_RX_ISR_RX_INTERRUPT_MASK_FIFO_READ_THRESH_LOW_IRQ_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_GFP_GFPM_RX_ISR_RX_INTERRUPT_MASK_FIFO_READ_THRESH_LOW_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * FIFO overflow IRQ mask.
 * The FIFO overflow IRQ mask bit is set to cause the assertion of the
 * GFP_CPU_INT output when a FIFO overflow interrupt occurs. If the FIFO
 * overflow IRQ mask bit is cleared, a FIFO overflow interrupt condition
 * will not cause the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_ISR_RX_INTERRUPT_MASK . FIFO_OVERFLOW_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_ISR_RX_INTERRUPT_MASK_FIFO_OVERFLOW_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_GFP_GFPM_RX_ISR_RX_INTERRUPT_MASK_FIFO_OVERFLOW_IRQ_MASK     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_GFP_GFPM_RX_ISR_RX_INTERRUPT_MASK_FIFO_OVERFLOW_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * FIFO underflow IRQ mask.
 * The FIFO underflow IRQ mask bit is set to cause the assertion of the
 * GFP_CPU_INT output when a FIFO underflow interrupt occurs. If the FIFO
 * underflow IRQ mask bit is cleared, a FIFO underflow interrupt condition
 * will not cause the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_ISR_RX_INTERRUPT_MASK . FIFO_UNDERFLOW_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_ISR_RX_INTERRUPT_MASK_FIFO_UNDERFLOW_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_GFP_GFPM_RX_ISR_RX_INTERRUPT_MASK_FIFO_UNDERFLOW_IRQ_MASK     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_GFP_GFPM_RX_ISR_RX_INTERRUPT_MASK_FIFO_UNDERFLOW_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/**
 * Register Group: \a GFP::GFPM_RX_PPOS_ISR
 *
 * GFP mapper Rx PP/OS interrupt status registers
 */


/** 
 * \brief GFPM Rx PP/OS interrupt status register
 *
 * \details
 * Register: \a GFP::GFPM_RX_PPOS_ISR::RX_PPOS_INTERRUPT_STATUS
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STATUS(target)  VTSS_IOREG(target,0xa0)

/** 
 * \brief
 * SSF detect status.
 * The SSF detect status bit is set when an incoming SSF condition is
 * detected.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STATUS . SSF_DETECT_STATUS
 */
#define  VTSS_F_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STATUS_SSF_DETECT_STATUS(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STATUS_SSF_DETECT_STATUS     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STATUS_SSF_DETECT_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,15,1)


/** 
 * \brief GFPM Rx PP/OS interrupt register
 *
 * \details
 * Register: \a GFP::GFPM_RX_PPOS_ISR::RX_PPOS_INTERRUPT_STICKY
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY(target)  VTSS_IOREG(target,0xa1)

/** 
 * \brief
 * SSF detect IRQ sticky.
 * The SSF detect IRQ bit is set when an incoming SSF condition is
 * detected. This is a sticky bit meaning that once set, it will remain set
 * until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY . SSF_DETECT_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_SSF_DETECT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_SSF_DETECT_IRQ_STICKY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_SSF_DETECT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Packet invalid IRQ sticky.
 * The packet invalid IRQ bit is set when an incoming invalid packet
 * (signaling from GFP) is detected. This is a sticky bit meaning that once
 * set, it will remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY . PKT_INVALID_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_PKT_INVALID_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_PKT_INVALID_IRQ_STICKY     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_PKT_INVALID_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * UPI violation IRQ sticky.
 * The UPI violation IRQ bit is set when an incoming UPI from the GFP-F
 * does not match the data UPI value or the ordered set UPI value contained
 * in the Rs PP/OS UPI register. 
 * NOTE: If an incoming UPI violation is detected, the incoming frame is
 * dropped. This is a sticky bit meaning that once set, it will remain set
 * until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY . UPI_VIOLATION_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_UPI_VIOLATION_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_UPI_VIOLATION_IRQ_STICKY     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_UPI_VIOLATION_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * FIFO error IRQ sticky.
 * The FIFO error IRQ bit is set when FIFO error (underflow or overflow) is
 * detected. This is a sticky bit meaning that once set, it will remain set
 * until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY . FIFO_ERROR_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_FIFO_ERROR_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_FIFO_ERROR_IRQ_STICKY     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_FIFO_ERROR_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Minimum packet violation IRQ sticky.
 * The minimum packet violation IRQ bit is set when an incoming minimum
 * packet violation is detected. This is a sticky bit meaning that once
 * set, it will remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY . MIN_PKT_VIOLATION_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_MIN_PKT_VIOLATION_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_MIN_PKT_VIOLATION_IRQ_STICKY     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_MIN_PKT_VIOLATION_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Maximum packet violation IRQ sticky.
 * The maximum packet violation IRQ bit is set when an incoming maximum
 * packet violation is detected. This is a sticky bit meaning that once
 * set, it will remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY . MAX_PKT_VIOLATION_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_MAX_PKT_VIOLATION_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_MAX_PKT_VIOLATION_IRQ_STICKY     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_MAX_PKT_VIOLATION_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Test data packet detect IRQ sticky.
 * The test data packet detect IRQ bit is set when an incoming test data
 * packet is detected. This is a sticky bit meaning that once set, it will
 * remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY . TEST_DATA_PKT_DETECT_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_TEST_DATA_PKT_DETECT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_TEST_DATA_PKT_DETECT_IRQ_STICKY     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_TEST_DATA_PKT_DETECT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * OS O-code invalid IRQ sticky.
 * The OS O-code invalid IRQ bit is set when an incoming ordered set O-Code
 * value is invalid (valid values are 0000 and 1111). This is a sticky bit
 * meaning that once set, it will remain set until a value of 1 is written
 * to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY . OS_OCODE_INVALID_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_OS_OCODE_INVALID_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_OS_OCODE_INVALID_IRQ_STICKY     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_OS_OCODE_INVALID_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Packet length mismatch IRQ.
 * The packet length mismatch IRQ bit is set when an incoming packet length
 * mismatch is detected. This is a sticky bit meaning that once set, it
 * will remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY . PKT_LENGTH_MISMATCH_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_PKT_LENGTH_MISMATCH_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_PKT_LENGTH_MISMATCH_IRQ_STICKY     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_PKT_LENGTH_MISMATCH_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,7,1)


/** 
 * \brief GFPM Rx PP/OS interrupt mask register
 *
 * \details
 * Register: \a GFP::GFPM_RX_PPOS_ISR::RX_PPOS_INTERRUPT_MASK
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK(target)  VTSS_IOREG(target,0xa2)

/** 
 * \brief
 * SSF detect IRQ mask.
 * The SSF detect IRQ mask bit is set to cause the assertion of the
 * GFP_CPU_INT output when an SSF interrupt condition exists. If the SSF
 * detect IRQ mask bit is cleared, an SSF detect interrupt condition will
 * not cause the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK . SSF_DETECT_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_SSF_DETECT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_SSF_DETECT_IRQ_MASK     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_SSF_DETECT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Packet invalid IRQ mask.
 * The packet invalid IRQ mask bit is set to cause the assertion of the
 * GFP_CPU_INT output when a packet invalid interrupt condition exists. If
 * the packet invalid IRQ mask bit is cleared, a packet invalid interrupt
 * condition will not cause the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK . PKT_INVALID_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_PKT_INVALID_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_PKT_INVALID_IRQ_MASK     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_PKT_INVALID_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * UPI violation IRQ mask.
 * The UPI violation IRQ mask bit is set to cause the assertion of the
 * GFP_CPU_INT output when a UPI violation interrupt condition exists. If
 * the UPI violation IRQ mask bit is cleared, a UPI violation interrupt
 * condition will not cause the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK . UPI_VIOLATION_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_UPI_VIOLATION_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_UPI_VIOLATION_IRQ_MASK     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_UPI_VIOLATION_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * FIFO error IRQ mask.
 * The FIFO error IRQ mask bit is set to cause the assertion of the
 * GFP_CPU_INT output when a FIFO error interrupt condition exists. If the
 * FIFO error IRQ mask bit is cleared, a FIFO error interrupt condition
 * will not cause the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK . FIFO_ERROR_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_FIFO_ERROR_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_FIFO_ERROR_IRQ_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_FIFO_ERROR_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Minimum packet violation IRQ mask.
 * The minimum packet violation IRQ mask bit is set to cause the assertion
 * of the GFP_CPU_INT output when a minimum packet violation interrupt
 * condition exists. If the minimum packet violation IRQ mask bit is
 * cleared, a minimum packet violation interrupt condition will not cause
 * the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK . MIN_PKT_VIOLATION_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_MIN_PKT_VIOLATION_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_MIN_PKT_VIOLATION_IRQ_MASK     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_MIN_PKT_VIOLATION_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Maximum packet violation IRQ mask.
 * The maximum packet violation IRQ mask bit is set to cause the assertion
 * of the GFP_CPU_INT output when a maximum packet violation interrupt
 * condition exists. If the maximum packet violation IRQ mask bit is
 * cleared, a maximum packet violation interrupt condition will not cause
 * the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK . MAX_PKT_VIOLATION_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_MAX_PKT_VIOLATION_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_MAX_PKT_VIOLATION_IRQ_MASK     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_MAX_PKT_VIOLATION_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Test data packet detect IRQ mask.
 * The test data packet IRQ mask bit is set to cause the assertion of the
 * GFP_CPU_INT output when a test data packet detect interrupt condition
 * exists. If the test data packet detect IRQ mask bit is cleared, a test
 * data packet detect interrupt condition will not cause the assertion of
 * the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK . TEST_DATA_PKT_DETECT_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_TEST_DATA_PKT_DETECT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_TEST_DATA_PKT_DETECT_IRQ_MASK     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_TEST_DATA_PKT_DETECT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * OS O-code invalid IRQ mask.
 * The OS O-code invalid IRQ mask bit is set to cause the assertion of the
 * GFP_CPU_INT output when an ordered set invalid O-code interrupt
 * condition exists. If the OS O-code invalid IRQ mask bit is cleared, an
 * invalid ordered set O-code condition will not cause the assertion of the
 * GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK . OS_OCODE_INVALID_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_OS_OCODE_INVALID_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_OS_OCODE_INVALID_IRQ_MASK     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_OS_OCODE_INVALID_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Packet length mismatch IRQ mask.
 * The packet length mismatch IRQ mask bit is set to cause the assertion of
 * the GFP_CPU_INT output when a packet length mismatch interrupt condition
 * exists. If the packet length mismatch IRQ mask bit is cleared, a packet
 * length mismatch condition will not cause the assertion of the
 * GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK . PKT_LENGTH_MISMATCH_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_PKT_LENGTH_MISMATCH_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_PKT_LENGTH_MISMATCH_IRQ_MASK     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_PKT_LENGTH_MISMATCH_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/**
 * Register Group: \a GFP::GFPM_RX_8B10B_ISR
 *
 * GFP mapper Rx 8B/10B interrupt status registers
 */


/** 
 * \brief GFPM Rx 8B/10B interrupt status register
 *
 * \details
 * Register: \a GFP::GFPM_RX_8B10B_ISR::RX_8B10B_INTERRUPT_STATUS
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STATUS(target)  VTSS_IOREG(target,0xc0)

/** 
 * \brief
 * FIFO error thresh status.
 * The FIFO error status bit is set when the Rx FIFO is at the programmed
 * low error threshold level. This condition will cause error characters
 * K30.7 to be generated by the 8B/10B encoder.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STATUS . FIFO_ERROR_THRESH_STATUS
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STATUS_FIFO_ERROR_THRESH_STATUS(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STATUS_FIFO_ERROR_THRESH_STATUS     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STATUS_FIFO_ERROR_THRESH_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,7,1)


/** 
 * \brief GFPM Rx 8B/10B interrupt register
 *
 * \details
 * Register: \a GFP::GFPM_RX_8B10B_ISR::RX_8B10B_INTERRUPT_STICKY
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY(target)  VTSS_IOREG(target,0xc1)

/** 
 * \brief
 * Idle misalign detect IRQ sticky.
 * The idle misalign detect IRQ bit is set when the associated
 * IDLE_MISALIGN_DETECT_STATUS bit changes state. This is a sticky bit
 * meaning that once set, it will remain set until a value of 1 is written
 * to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY . IDLE_MISALIGN_DETECT_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_IDLE_MISALIGN_DETECT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_IDLE_MISALIGN_DETECT_IRQ_STICKY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_IDLE_MISALIGN_DETECT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Insert rate adapt IRQ sticky.
 * The insert rate adapt IRQ bit is set when an 8B/10B encoder performs
 * rate adaptation through the insertion of idles or primitive sequences
 * (see G.7041 specification section 8.4.1.2.1). This is a sticky bit
 * meaning that once set, it will remain set until a value of 1 is written
 * to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY . INSERT_RATE_ADAPT_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_INSERT_RATE_ADAPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_INSERT_RATE_ADAPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_INSERT_RATE_ADAPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Remove rate adapt IRQ sticky.
 * The remove rate adapt IRQ bit is set when an 8B/10B encoder performs
 * rate adaptation through the removal of idles or primitive sequences (see
 * G.7041 specification section 8.4.1.2.1). This is a sticky bit meaning
 * that once set, it will remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY . REMOVE_RATE_ADAPT_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_REMOVE_RATE_ADAPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_REMOVE_RATE_ADAPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_REMOVE_RATE_ADAPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * 10B_ERR detect IRQ sticky.
 * The 10B_ERR detect IRQ bit is set when an incoming 8B_ERR character is
 * detected. This is a sticky bit meaning that once set, it will remain set
 * until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY . RX_10B_ERR_DETECT_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_RX_10B_ERR_DETECT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_RX_10B_ERR_DETECT_IRQ_STICKY     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_RX_10B_ERR_DETECT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * 10B_ERR removal IRQ sticky.
 * The 10B_ERR removal IRQ bit is set when an incoming 8B_ERR character is
 * removed (see G.7041 specification section 8.4.1.2.1). This is a sticky
 * bit meaning that once set, it will remain set until a value of 1 is
 * written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY . RX_10B_ERR_REMOVAL_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_RX_10B_ERR_REMOVAL_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_RX_10B_ERR_REMOVAL_IRQ_STICKY     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_RX_10B_ERR_REMOVAL_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Disparity continuity error IRQ sticky.
 * The disparity continuity error IRQ bit is set when the encoder running
 * disparity is reset to negative when the current disparity is positive.
 * This is a sticky bit meaning that once set, it will remain set until a
 * value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY . DISPARITY_CONTINUITY_ERROR_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_DISPARITY_CONTINUITY_ERROR_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_DISPARITY_CONTINUITY_ERROR_IRQ_STICKY     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_DISPARITY_CONTINUITY_ERROR_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Invalid K character IRQ sticky.
 * The invalid K character IRQ bit is set when an incoming invalid K
 * character is detected. This is a sticky bit meaning that once set, it
 * will remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY . INVALID_K_CHARACTER_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_INVALID_K_CHARACTER_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_INVALID_K_CHARACTER_IRQ_STICKY     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_INVALID_K_CHARACTER_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * UPI mismatch IRQ sticky.
 * The UPI mismatch IRQ bit is set when an incoming UPI value not equal to
 * the transparent fibre channel UPI code of 0x'03' is detected. This is a
 * sticky bit meaning that once set, it will remain set until a value of 1
 * is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY . UPI_MISMATCH_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_UPI_MISMATCH_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_UPI_MISMATCH_IRQ_STICKY     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_UPI_MISMATCH_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * FIFO error thresh IRQ sticky.
 * The FIFO error IRQ bit is set when the associated
 * FIFO_ERROR_THRESH_STATUS bit changes state. This is a sticky bit meaning
 * that once set, it will remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY . FIFO_ERROR_THRESH_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_FIFO_ERROR_THRESH_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_FIFO_ERROR_THRESH_IRQ_STICKY     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_FIFO_ERROR_THRESH_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,7,1)


/** 
 * \brief GFPM Rx 8B/10B interrupt mask register
 *
 * \details
 * Register: \a GFP::GFPM_RX_8B10B_ISR::RX_8B10B_INTERRUPT_MASK
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK(target)  VTSS_IOREG(target,0xc2)

/** 
 * \brief
 * Idle misalign detect IRQ mask.
 * The idle misalign detect IRQ mask bit is set to cause the assertion of
 * the GFP_CPU_INT output when an idle misalign detect interrupt condition
 * exists. If the idle misalign detect IRQ mask bit is cleared, an idle
 * misalign detect interrupt condition will not cause the assertion of the
 * GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK . IDLE_MISALIGN_DETECT_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_IDLE_MISALIGN_DETECT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_IDLE_MISALIGN_DETECT_IRQ_MASK     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_IDLE_MISALIGN_DETECT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Insert rate adapt IRQ mask.
 * The insert rate adapt IRQ mask bit is set to cause the assertion of the
 * GFP_CPU_INT output when an insert rate adapt interrupt condition exists.
 * If the insert rate adapt IRQ mask bit is cleared, an insert rate adapt
 * interrupt condition will not cause the assertion of the GFP_CPU_INT
 * output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK . INSERT_RATE_ADAPT_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_INSERT_RATE_ADAPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_INSERT_RATE_ADAPT_IRQ_MASK     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_INSERT_RATE_ADAPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Remove rate adapt IRQ mask.
 * The remove rate adapt IRQ mask bit is set to cause the assertion of the
 * GFP_CPU_INT output when a remove rate adapt interrupt condition exists.
 * If the remove rate adapt IRQ mask bit is cleared, a remove rate adapt
 * interrupt condition will not cause the assertion of the GFP_CPU_INT
 * output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK . REMOVE_RATE_ADAPT_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_REMOVE_RATE_ADAPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_REMOVE_RATE_ADAPT_IRQ_MASK     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_REMOVE_RATE_ADAPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * 10B_ERR detect IRQ mask.
 * The 10B_ERR detect IRQ mask bit is set to cause the assertion of the
 * GFP_CPU_INT output when a 10B_ERR detect interrupt condition exists. If
 * the 10B_ERR detect IRQ mask bit is cleared, a 10B_ERR detect interrupt
 * condition will not cause the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK . RX_10B_ERR_DETECT_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_RX_10B_ERR_DETECT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_RX_10B_ERR_DETECT_IRQ_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_RX_10B_ERR_DETECT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * 10B_ERR removal IRQ mask.
 * The 10B_ERR removal IRQ mask bit is set to cause the assertion of the
 * GFP_CPU_INT output when a 10B_ERR removal interrupt condition exists. If
 * the 10B_ERR removal IRQ mask bit is cleared, a 10B_ERR removal interrupt
 * condition will not cause the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK . RX_10B_ERR_REMOVAL_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_RX_10B_ERR_REMOVAL_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_RX_10B_ERR_REMOVAL_IRQ_MASK     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_RX_10B_ERR_REMOVAL_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Disparity continuity error IRQ mask.
 * The disparity continuity error IRQ mask bit is set to cause the
 * assertion of the GFP_CPU_INT output when a disparity continuity error
 * interrupt condition exists. If the disparity continuity error IRQ mask
 * bit is cleared, a disparity continuity error interrupt condition will
 * not cause the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK . DISPARITY_CONTINUITY_ERROR_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_DISPARITY_CONTINUITY_ERROR_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_DISPARITY_CONTINUITY_ERROR_IRQ_MASK     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_DISPARITY_CONTINUITY_ERROR_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Invalid K character IRQ mask.
 * The invalid K character IRQ mask bit is set to cause the assertion of
 * the GFP_CPU_INT output when an invalid K character interrupt condition
 * exists. If the invalid K character IRQ mask bit is cleared, an invalid K
 * character interrupt condition will not cause the assertion of the
 * GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK . INVALID_K_CHARACTER_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_INVALID_K_CHARACTER_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_INVALID_K_CHARACTER_IRQ_MASK     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_INVALID_K_CHARACTER_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * UPI mismatch IRQ mask.
 * The UPI mismatch IRQ mask bit is set to cause the assertion of the
 * GFP_CPU_INT output when a UPI mismatch interrupt condition exists. If
 * the UPI mismatch IRQ mask bit is cleared, a UPI mismatch interrupt
 * condition will not cause the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK . UPI_MISMATCH_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_UPI_MISMATCH_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_UPI_MISMATCH_IRQ_MASK     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_UPI_MISMATCH_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * FIFO error thresh IRQ mask.
 * The FIFO error thresh IRQ mask bit is set to cause the assertion of the
 * GFP_CPU_INT output when a FIFO error threshold interrupt condition
 * exists. If the FIFO error thresh IRQ mask bit is cleared, a FIFO error
 * threshold interrupt condition will not cause the assertion of the
 * GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK . FIFO_ERROR_THRESH_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_FIFO_ERROR_THRESH_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_FIFO_ERROR_THRESH_IRQ_MASK     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_FIFO_ERROR_THRESH_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/**
 * Register Group: \a GFP::GFPM_RX_GFP_ISR
 *
 * GFP mapper Rx GFP interrupt status registers
 */


/** 
 * \brief GFPM Rx GFP interrupt status register
 *
 * \details
 * Register: \a GFP::GFPM_RX_GFP_ISR::RX_GFP_INTERRUPT_STATUS
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STATUS(target)  VTSS_IOREG(target,0xe0)

/** 
 * \brief
 * SSF status.
 * The SSF status bit is set when the frame delineator is in the HUNT or
 * PRESYNC state.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STATUS . SSF_STATUS
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STATUS_SSF_STATUS(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STATUS_SSF_STATUS     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STATUS_SSF_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,14,1)


/** 
 * \brief GFPM Rx GFP interrupt register
 *
 * \details
 * Register: \a GFP::GFPM_RX_GFP_ISR::RX_GFP_INTERRUPT_STICKY
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY(target)  VTSS_IOREG(target,0xe1)

/** 
 * \brief
 * PFI disabled IRQ sticky.
 * The PFI disabled IRQ bit is set when the frame delineator is in the SYNC
 * state and the PFI field of the payload header for valid incoming frame
 * is cleared. This is a sticky bit meaning that once set, it will remain
 * set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY . PFI_DISABLED_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_PFI_DISABLED_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_PFI_DISABLED_IRQ_STICKY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_PFI_DISABLED_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * SSF IRQ sticky.
 * The SSF IRQ bit is set when the associated SSF_STATUS bit changes state.
 * This is a sticky bit meaning that once set, it will remain set until a
 * value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY . SSF_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_SSF_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_SSF_IRQ_STICKY     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_SSF_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Core header corrected IRQ sticky.
 * The core header corrected IRQ bit is set when the frame delineator is in
 * the SYNC state and a single core header bit error is corrected. This is
 * a sticky bit meaning that once set, it will remain set until a value of
 * 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY . CORE_HDR_COR_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_CORE_HDR_COR_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_CORE_HDR_COR_IRQ_STICKY     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_CORE_HDR_COR_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Payload header corrected IRQ sticky.
 * The payload header corrected IRQ bit is set when the frame delineator is
 * in the SYNC state and a single payload header bit error is corrected.
 * This is a sticky bit meaning that once set, it will remain set until a
 * value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY . PYLD_HDR_COR_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_PYLD_HDR_COR_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_PYLD_HDR_COR_IRQ_STICKY     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_PYLD_HDR_COR_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Payload header uncorrected IRQ sticky.
 * The payload header uncorrected IRQ bit is set when the frame delineator
 * is in the SYNC state and multiple payload header bit errors occur. This
 * is a sticky bit meaning that once set, it will remain set until a value
 * of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY . PYLD_HDR_UNCOR_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_PYLD_HDR_UNCOR_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_PYLD_HDR_UNCOR_IRQ_STICKY     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_PYLD_HDR_UNCOR_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * PTI IRQ sticky.
 * The PTI IRQ bit is set when the frame delineator is in the SYNC state
 * and the extracted PTI bits do not equal 000 (client data frame) or 100
 * (client management frame). This is a sticky bit meaning that once set,
 * it will remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY . PTI_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_PTI_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_PTI_IRQ_STICKY     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_PTI_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * FCS IRQ sticky.
 * The FCS IRQ bit is set when the frame delineator is in the SYNC state
 * and the extracted FCS value does not equal the FCS value calculated by
 * the GFP receive processor. This is a sticky bit meaning that once set,
 * it will remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY . FCS_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_FCS_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_FCS_IRQ_STICKY     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_FCS_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * EXI unsupported IRQ sticky.
 * The EXI unsupported IRQ bit is set when the frame delineator is in the
 * SYNC state and the extracted EXI value is not equal to 000 (null
 * extension header). This is a sticky bit meaning that once set, it will
 * remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY . EXI_UNSUPPORTED_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_EXI_UNSUPPORTED_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_EXI_UNSUPPORTED_IRQ_STICKY     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_EXI_UNSUPPORTED_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Test frame detect IRQ sticky.
 * The test frame detect IRQ bit is set when an incoming frame with a
 * client data proprietary UPI field value of 111000 is received, the
 * incoming PLI value matches the expect test PLI value bits of the Rx GFP
 * test frame expect register, and all incoming payload bytes match the
 * expect test payload value bits of the Rx GFP test frame expect register.
 * This is a sticky bit meaning that once set, it will remain set until a
 * value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY . TEST_FRM_DETECT_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_TEST_FRM_DETECT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_TEST_FRM_DETECT_IRQ_STICKY     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_TEST_FRM_DETECT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * CRC-16 corrected IRQ sticky.
 * The CRC-16 corrected IRQ bit is set when a transparent mode CRC-16 error
 * is corrected. This is a sticky bit meaning that once set, it will remain
 * set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY . CRC16_COR_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_CRC16_COR_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_CRC16_COR_IRQ_STICKY     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_CRC16_COR_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * CRC-16 uncorrected IRQ sticky.
 * The CRC-16 uncorrected IRQ bit is set when incoming CRC-16 uncorrectable
 * errors are detected. This is a sticky bit meaning that once set, it will
 * remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY . CRC16_UNCOR_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_CRC16_UNCOR_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_CRC16_UNCOR_IRQ_STICKY     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_CRC16_UNCOR_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,5,1)


/** 
 * \brief GFPM Rx GFP interrupt mask register
 *
 * \details
 * Register: \a GFP::GFPM_RX_GFP_ISR::RX_GFP_INTERRUPT_MASK
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK(target)  VTSS_IOREG(target,0xe2)

/** 
 * \brief
 * PFI disabled IRQ mask.
 * The PFI disabled IRQ mask bit is set to cause the assertion of the
 * GFP_CPU_INT output when a PFI disabled interrupt condition exists. If
 * the PFI disabled IRQ mask bit is cleared, a PFI disabled interrupt
 * condition will not cause the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK . PFI_DISABLED_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_PFI_DISABLED_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_PFI_DISABLED_IRQ_MASK     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_PFI_DISABLED_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * SSF IRQ mask.
 * The SSF IRQ mask bit is set to cause the assertion of the GFP_CPU_INT
 * output when an SSF interrupt condition exists. If the SSF IRQ mask bit
 * is cleared, an SSF interrupt condition will not cause the assertion of
 * the GFP_CPU_INT output. 
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK . SSF_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_SSF_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_SSF_IRQ_MASK     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_SSF_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Core header corrected IRQ mask.
 * The core header corrected IRQ mask bit is set to cause the assertion of
 * the GFP_CPU_INT output when a core header corrected interrupt condition
 * exists. If the core header corrected IRQ mask bit is cleared, a core
 * header corrected interrupt condition will not cause the assertion of the
 * GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK . CORE_HDR_COR_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_CORE_HDR_COR_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_CORE_HDR_COR_IRQ_MASK     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_CORE_HDR_COR_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Payload header corrected IRQ mask.
 * The payload header corrected IRQ mask bit is set to cause the assertion
 * of the GFP_CPU_INT output when a payload header corrected interrupt
 * condition exists. If the payload header corrected IRQ mask bit is
 * cleared, a payload header corrected interrupt condition will not cause
 * the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK . PYLD_HDR_COR_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_PYLD_HDR_COR_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_PYLD_HDR_COR_IRQ_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_PYLD_HDR_COR_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Payload header uncorrected IRQ mask.
 * The payload header uncorrected IRQ mask bit is set to cause the
 * assertion of the GFP_CPU_INT output when a payload header uncorrected
 * interrupt condition exists. If the payload header uncorrected IRQ mask
 * bit is cleared, a payload header uncorrected interrupt condition will
 * not cause the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK . PYLD_HDR_UNCOR_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_PYLD_HDR_UNCOR_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_PYLD_HDR_UNCOR_IRQ_MASK     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_PYLD_HDR_UNCOR_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * PTI IRQ mask.
 * The PTI IRQ mask bit is set to cause the assertion of the GFP_CPU_INT
 * output when a PTI interrupt condition exists. If the PTI IRQ mask bit is
 * cleared, a PTI interrupt condition will not cause the assertion of the
 * GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK . PTI_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_PTI_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_PTI_IRQ_MASK     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_PTI_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * FCS IRQ mask.
 * The FCS IRQ mask bit is set to cause the assertion of the GFP_CPU_INT
 * output when an FCS interrupt condition exists. If the FCS IRQ mask bit
 * is cleared, an FCS interrupt condition will not cause the assertion of
 * the GFP_CPU_INT output. 
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK . FCS_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_FCS_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_FCS_IRQ_MASK     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_FCS_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * EXI unsupported IRQ mask.
 * The EXI IRQ mask bit is set to cause the assertion of the GFP_CPU_INT
 * output when an EXI interrupt condition exists. If the EXI IRQ mask bit
 * is cleared, an EXI interrupt condition will not cause the assertion of
 * the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK . EXI_UNSUPPORTED_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_EXI_UNSUPPORTED_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_EXI_UNSUPPORTED_IRQ_MASK     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_EXI_UNSUPPORTED_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Test frame detect IRQ mask.
 * The test frame detect IRQ mask bit is set to cause the assertion of the
 * GFP_CPU_INT output when test frame detect interrupt condition exists. If
 * the test frame detect IRQ mask bit is cleared, a test frame detect
 * interrupt condition will not cause the assertion of the GFP_CPU_INT
 * output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK . TEST_FRM_DETECT_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_TEST_FRM_DETECT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_TEST_FRM_DETECT_IRQ_MASK     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_TEST_FRM_DETECT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * CRC-16 corrected IRQ mask.
 * The CRC-16 corrected IRQ mask bit is set to cause the assertion of the
 * GFP_CPU_INT output when a CRC-16 corrected interrupt condition exists.
 * If the CRC-16 corrected IRQ mask bit is cleared, a CRC-16 corrected
 * interrupt condition will not cause the assertion of the GFP_CPU_INT
 * output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK . CRC16_COR_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_CRC16_COR_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_CRC16_COR_IRQ_MASK     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_CRC16_COR_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * CRC-16 uncorrected IRQ mask.
 * The CRC-16 uncorrected IRQ mask bit is set to cause the assertion of the
 * GFP_CPU_INT output when a CRC-16 uncorrected interrupt condition exists.
 * If the CRC-16 uncorrected IRQ mask bit is cleared, a CRC-16 uncorrected
 * interrupt condition will not cause the assertion of the GFP_CPU_INT
 * output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK . CRC16_UNCOR_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_CRC16_UNCOR_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_CRC16_UNCOR_IRQ_MASK     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_CRC16_UNCOR_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,5,1)


/** 
 * \brief GFPM Rx GFP client interrupt register
 *
 * \details
 * Register: \a GFP::GFPM_RX_GFP_ISR::RX_GFP_CLIENT_INTERRUPT_STICKY
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY(target)  VTSS_IOREG(target,0xe4)

/** 
 * \brief
 * Client frame mapped Ethernet detect IRQ sticky.
 * The client frame mapped Ethernet detect IRQ bit is set when the frame
 * delineator is in the SYNC state and a client frame mapped ethernet frame
 * is detected. This is a sticky bit meaning that once set, it will remain
 * set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY . CLIENT_FRAME_MAPPED_ETHERNET_DETECT_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_FRAME_MAPPED_ETHERNET_DETECT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_FRAME_MAPPED_ETHERNET_DETECT_IRQ_STICKY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_FRAME_MAPPED_ETHERNET_DETECT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Client frame mapped PPP detect IRQ sticky.
 * The client frame mapped PPP detect IRQ bit is set when the frame
 * delineator is in the SYNC state and a client frame mapped PPP frame is
 * detected. This is a sticky bit meaning that once set, it will remain set
 * until a value of 1 is written to it.       
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY . CLIENT_FRAME_MAPPED_PPP_DETECT_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_FRAME_MAPPED_PPP_DETECT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_FRAME_MAPPED_PPP_DETECT_IRQ_STICKY     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_FRAME_MAPPED_PPP_DETECT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Client frame mapped MPLS unicast detect IRQ sticky.
 * The client frame mapped MPLS unicast detect IRQ bit is set when the
 * frame delineator is in the SYNC state and a client frame mapped MPLS
 * unicast frame is detected. This is a sticky bit meaning that once set,
 * it will remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY . CLIENT_FRAME_MAPPED_MPLS_UC_DETECT_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_FRAME_MAPPED_MPLS_UC_DETECT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_FRAME_MAPPED_MPLS_UC_DETECT_IRQ_STICKY     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_FRAME_MAPPED_MPLS_UC_DETECT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Client transparent fibre channel detect IRQ sticky.
 * The client transparent fibre channel detect IRQ bit is set when the
 * frame delineator is in the SYNC state and a client transparent fibre
 * channel frame is detected. This is a sticky bit meaning that once set,
 * it will remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY . CLIENT_TRANSP_FIBRE_CHANNEL_DETECT_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_TRANSP_FIBRE_CHANNEL_DETECT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_TRANSP_FIBRE_CHANNEL_DETECT_IRQ_STICKY     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_TRANSP_FIBRE_CHANNEL_DETECT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Client frame mapped MPLS multicast detect IRQ sticky.
 * The client frame mapped MPLS multicast detect IRQ bit is set when the
 * frame delineator is in the SYNC state and a client frame mapped MPLS
 * multicast frame is detected. This is a sticky bit meaning that once set,
 * it will remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY . CLIENT_FRAME_MAPPED_MPLS_MC_DETECT_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_FRAME_MAPPED_MPLS_MC_DETECT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_FRAME_MAPPED_MPLS_MC_DETECT_IRQ_STICKY     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_FRAME_MAPPED_MPLS_MC_DETECT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * 64B/66B Ethernet data detect IRQ sticky.
 * The 64B/66B Ethernet data detect IRQ bit is set when 64B/66B ethernet
 * data is detected. This is a sticky bit meaning that once set, it will
 * remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY . RX_64B66B_ETHERNET_DATA_DETECT_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_RX_64B66B_ETHERNET_DATA_DETECT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_RX_64B66B_ETHERNET_DATA_DETECT_IRQ_STICKY     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_RX_64B66B_ETHERNET_DATA_DETECT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Client data proprietary detect IRQ sticky.
 * The client data proprietary detect IRQ bit is set when the frame
 * delineator is in the SYNC state and a client data proprietary frame is
 * detected. This is a sticky bit meaning that once set, it will remain set
 * until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY . CLIENT_DATA_PROPRIETARY_DETECT_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_DATA_PROPRIETARY_DETECT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_DATA_PROPRIETARY_DETECT_IRQ_STICKY     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_DATA_PROPRIETARY_DETECT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Client data unsupported detect IRQ sticky.
 * The client data unsupported detect IRQ bit is set when the frame
 * delineator is in the SYNC state, the extracted PTI bits are 000 and the
 * value of UPI extracted is NOT equal to the following values:
 * 00000001 (frame mapped Ethernet)
 * 00000010 (frame mapped PPP)
 * 00000011 (transparent fibre channel)
 * 00000110 (transparent gigabit Ethernet)
 * 00001000 (frame mapped MAPOS)
 * 1111000-11111110 (client proprietary) 
 * This is a sticky bit meaning that once set, it will remain set until a
 * value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY . CLIENT_DATA_UNSUPPORTED_DETECT_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_DATA_UNSUPPORTED_DETECT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_DATA_UNSUPPORTED_DETECT_IRQ_STICKY     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_DATA_UNSUPPORTED_DETECT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Client management LOS detect IRQ sticky.
 * The client management LOS detect IRQ bit is set when the frame
 * delineator is in the SYNC state and a client management frame with an
 * LOS UPI code (00000001) is detected. This is a sticky bit meaning that
 * once set, it will remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY . CLIENT_MGMT_LOS_DETECT_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_LOS_DETECT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_LOS_DETECT_IRQ_STICKY     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_LOS_DETECT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Client management LSYNC detect IRQ sticky.
 * The client management LSYNC detect IRQ bit is set when the frame
 * delineator is in the SYNC state and a client management frame with an
 * LSYNC UPI code (00000010) is detected. This is a sticky bit meaning that
 * once set, it will remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY . CLIENT_MGMT_LSYNC_DETECT_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_LSYNC_DETECT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_LSYNC_DETECT_IRQ_STICKY     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_LSYNC_DETECT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Client management proprietary detect IRQ sticky.
 * The client management proprietary detect IRQ bit is set when the frame
 * delineator is in the SYNC state and a client management proprietary
 * frame is detected. This is a sticky bit meaning that once set, it will
 * remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY . CLIENT_MGMT_PROPRIETARY_DETECT_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_PROPRIETARY_DETECT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_PROPRIETARY_DETECT_IRQ_STICKY     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_PROPRIETARY_DETECT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Client management unsupported detect IRQ sticky.
 * The client management unsupported IRQ bit is set when the frame
 * delineator is in the SYNC state, the extracted PTI bits are 100, and the
 * value of UPI extracted is not equal to either 00000001 (LOS) or 00000010
 * (LSYNC) or 1110000-11111110 management proprietary). This is a sticky
 * bit meaning that once set, it will remain set until a value of 1 is
 * written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY . CLIENT_MGMT_UNSUPPORTED_DETECT_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_UNSUPPORTED_DETECT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_UNSUPPORTED_DETECT_IRQ_STICKY     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_UNSUPPORTED_DETECT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Client management accept IRQ sticky.
 * The client management accept IRQ bit is set when an incoming client
 * management accept condition is detected. This is a sticky bit meaning
 * that once set, it will remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY . CLIENT_MGMT_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Client management accept overflow IRQ sticky.
 * The client management accept overflow IRQ bit is set when an incoming
 * client management accept overflow condition is detected. This is a
 * sticky bit meaning that once set, it will remain set until a value of 1
 * is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY . CLIENT_MGMT_ACCEPT_OVFL_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_ACCEPT_OVFL_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_ACCEPT_OVFL_IRQ_STICKY     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_ACCEPT_OVFL_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Client management inconsistent IRQ sticky.
 * The client management accept IRQ bit is set when an incoming client
 * management inconsistent condition is detected. This is a sticky bit
 * meaning that once set, it will remain set until a value of 1 is written
 * to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY . CLIENT_MGMT_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * 64B/66B Ethernet OS detect IRQ sticky.
 * The 64B/66B Ethernet OS detect IRQ bit is set when 64B/66B Ethernet
 * ordered set information is detected. This is a sticky bit meaning that
 * once set, it will remain set until a value of 1 is written to it.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY . RX_64B66B_ETHERNET_OS_DETECT_IRQ_STICKY
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_RX_64B66B_ETHERNET_OS_DETECT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_RX_64B66B_ETHERNET_OS_DETECT_IRQ_STICKY     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_RX_64B66B_ETHERNET_OS_DETECT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief GFPM Rx GFP client interrupt mask register
 *
 * \details
 * Register: \a GFP::GFPM_RX_GFP_ISR::RX_GFP_CLIENT_INTERRUPT_MASK
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK(target)  VTSS_IOREG(target,0xe5)

/** 
 * \brief
 * Client frame mapped Ethernet detect IRQ mask.
 * The client frame mapped Ethernet detect IRQ mask bit is set to cause the
 * assertion of the GFP_CPU_INT output when a client frame mapped Ethernet
 * detect interrupt condition exists. If the client frame mapped Ethernet
 * detect IRQ mask bit is cleared, a client frame mapped Ethernet detect
 * interrupt condition will not cause the assertion of the GFP_CPU_INT
 * output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK . CLIENT_FRAME_MAPPED_ETHERNET_DETECT_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_FRAME_MAPPED_ETHERNET_DETECT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_FRAME_MAPPED_ETHERNET_DETECT_IRQ_MASK     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_FRAME_MAPPED_ETHERNET_DETECT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Client frame mapped PPP detect IRQ mask.
 * The client frame mapped PPP detect IRQ mask bit is set to cause the
 * assertion of the GFP_CPU_INT output when a client frame mapped PPP
 * detect interrupt condition exists. If the client frame mapped PPP detect
 * IRQ mask bit is cleared, a client frame mapped PPP detect interrupt
 * condition will not cause the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK . CLIENT_FRAME_MAPPED_PPP_DETECT_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_FRAME_MAPPED_PPP_DETECT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_FRAME_MAPPED_PPP_DETECT_IRQ_MASK     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_FRAME_MAPPED_PPP_DETECT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Client frame mapped MPLS unicast detect IRQ mask.
 * The client frame mapped MPLS unicast detect IRQ mask bit is set to cause
 * the assertion of the GFP_CPU_INT output when a client frame mapped MPLS
 * unicast detect interrupt condition exists. If the client frame mapped
 * MPLS unicast detect IRQ mask bit is cleared, a client frame mapped MPLS
 * unicast detect interrupt condition will not cause the assertion of the
 * GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK . CLIENT_FRAME_MAPPED_MPLS_UC_DETECT_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_FRAME_MAPPED_MPLS_UC_DETECT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_FRAME_MAPPED_MPLS_UC_DETECT_IRQ_MASK     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_FRAME_MAPPED_MPLS_UC_DETECT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Client transparent fibre channel detect IRQ mask.
 * The client transparent fibre channel detect IRQ mask bit is set to cause
 * the assertion of the GFP_CPU_INT output when a client transparent fibre
 * channel detect interrupt condition exists. If the client transparent
 * fibre channel detect IRQ mask bit is cleared, a client transparent fibre
 * channel detect interrupt condition will not cause the assertion of the
 * GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK . CLIENT_TRANSP_FIBRE_CHANNEL_DETECT_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_TRANSP_FIBRE_CHANNEL_DETECT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_TRANSP_FIBRE_CHANNEL_DETECT_IRQ_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_TRANSP_FIBRE_CHANNEL_DETECT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Client frame mapped MPLS multicast detect IRQ mask.
 * The client frame mapped MPLS multicast detect IRQ mask bit is set to
 * cause the assertion of the GFP_CPU_INT output when a client frame mapped
 * MPLS multicast detect interrupt condition exists. If the client frame
 * mapped MPLS multicast detect IRQ mask bit is cleared, a client frame
 * mapped MPLS multicast detect interrupt condition will not cause the
 * assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK . CLIENT_FRAME_MAPPED_MPLS_MC_DETECT_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_FRAME_MAPPED_MPLS_MC_DETECT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_FRAME_MAPPED_MPLS_MC_DETECT_IRQ_MASK     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_FRAME_MAPPED_MPLS_MC_DETECT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * 64B/66B Ethernet data detect IRQ mask.
 * The 64B/66B Ethernet data detect IRQ mask bit is set to cause the
 * assertion of the GFP_CPU_INT output when a 64B/66B Ethernet data detect
 * interrupt condition exists. If the 64B/66B Ethernet data detect IRQ mask
 * bit is cleared, a 64B/66B Ethernet data detect interrupt condition will
 * not cause the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK . RX_64B66B_ETHERNET_DATA_DETECT_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_RX_64B66B_ETHERNET_DATA_DETECT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_RX_64B66B_ETHERNET_DATA_DETECT_IRQ_MASK     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_RX_64B66B_ETHERNET_DATA_DETECT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Client data proprietary detect IRQ mask.
 * The client data proprietary detect IRQ mask bit is set to cause the
 * assertion of the GFP_CPU_INT output when a client proprietary detect
 * interrupt condition exists. If the client data proprietary detect IRQ
 * mask bit is cleared, a client data proprietary detect interrupt
 * condition will not cause the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK . CLIENT_DATA_PROPRIETARY_DETECT_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_DATA_PROPRIETARY_DETECT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_DATA_PROPRIETARY_DETECT_IRQ_MASK     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_DATA_PROPRIETARY_DETECT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Client data unsupported detect IRQ mask.
 * The client data unsupported IRQ mask bit is set to cause the assertion
 * of the GFP_CPU_INT output when a client data frame unsupported interrupt
 * condition exists. If the client data unsupported detect IRQ mask bit is
 * cleared, a client data unsupported interrupt condition will not cause
 * the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK . CLIENT_DATA_UNSUPPORTED_DETECT_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_DATA_UNSUPPORTED_DETECT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_DATA_UNSUPPORTED_DETECT_IRQ_MASK     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_DATA_UNSUPPORTED_DETECT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Client management LOS detect IRQ mask.
 * The client management LOS IRQ mask bit is set to cause the assertion of
 * the GFP_CPU_INT output when a client management LOS interrupt condition
 * exists. If the client management LOS detect IRQ mask bit is cleared, a
 * client management LOS interrupt condition will not cause the assertion
 * of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK . CLIENT_MGMT_LOS_DETECT_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_MGMT_LOS_DETECT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_MGMT_LOS_DETECT_IRQ_MASK     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_MGMT_LOS_DETECT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Client management LSYNC detect IRQ mask.
 * The client management LSYNC IRQ mask bit is set to cause the assertion
 * of the GFP_CPU_INT output when a client management LSYNC interrupt
 * condition exists. If the client management LOS detect IRQ mask bit is
 * cleared, a client management LSYNC interrupt condition will not cause
 * the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK . CLIENT_MGMT_LSYNC_DETECT_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_MGMT_LSYNC_DETECT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_MGMT_LSYNC_DETECT_IRQ_MASK     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_MGMT_LSYNC_DETECT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Client management proprietary detect IRQ mask.
 * The client management proprietary detect IRQ mask bit is set to cause
 * the assertion of the GFP_CPU_INT output when a client management detect
 * interrupt condition exists. If the client management proprietary detect
 * IRQ mask bit is cleared, a client management proprietary detect
 * interrupt condition will not cause the assertion of the GFP_CPU_INT
 * output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK . CLIENT_MGMT_PROPRIETARY_DETECT_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_MGMT_PROPRIETARY_DETECT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_MGMT_PROPRIETARY_DETECT_IRQ_MASK     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_MGMT_PROPRIETARY_DETECT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Client management unsupported detect IRQ mask.
 * The client management unsupported IRQ mask bit is set to cause the
 * assertion of the GFP_CPU_INT output when a client management frame
 * unsupported interrupt condition exists. If the client management
 * unsupported detect IRQ mask bit is cleared, a client management
 * unsupported interrupt condition will not cause the assertion of the
 * GFP_CPU_INT output.	  
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK . CLIENT_MGMT_UNSUPPORTED_DETECT_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_MGMT_UNSUPPORTED_DETECT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_MGMT_UNSUPPORTED_DETECT_IRQ_MASK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_MGMT_UNSUPPORTED_DETECT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Client management accept IRQ mask.
 * The client management accept IRQ mask bit is set to cause the assertion
 * of the GFP_CPU_INT output when a client management frame accept
 * interrupt condition exists. If the client management accept IRQ mask bit
 * is cleared, a client management accept interrupt condition will not
 * cause the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK . CLIENT_MGMT_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_MGMT_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_MGMT_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_MGMT_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Client management accept overflow IRQ mask.
 * The client management accept overflow IRQ mask bit is set to cause the
 * assertion of the GFP_CPU_INT output when a client management frame
 * accept overflow interrupt condition exists. If the client management
 * accept overflow IRQ mask bit is cleared, a client management accept
 * overflow interrupt condition will not cause the assertion of the
 * GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK . CLIENT_MGMT_ACCEPT_OVFL_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_MGMT_ACCEPT_OVFL_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_MGMT_ACCEPT_OVFL_IRQ_MASK     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_MGMT_ACCEPT_OVFL_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Client management inconsistent IRQ mask.
 * The client management inconsistent IRQ mask bit is set to cause the
 * assertion of the GFP_CPU_INT output when a client management frame
 * inconsistent interrupt condition exists. If the client management
 * inconsistent IRQ mask bit is cleared, a client management inconsistent
 * interrupt condition will not cause the assertion of the GFP_CPU_INT
 * output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK . CLIENT_MGMT_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_MGMT_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_MGMT_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_MGMT_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * 64B/66B Ethernet OS detect IRQ mask.
 * The 64B/66B Ethernet OS detect IRQ mask bit is set to cause the
 * assertion of the GFP_CPU_INT output when a 64B/66B Ethernet OS detect
 * interrupt condition exists. If the 64B/66B Ethernet OS detect IRQ mask
 * bit is cleared, a 64B/66B Ethernet OS detect interrupt condition will
 * not cause the assertion of the GFP_CPU_INT output.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK . RX_64B66B_ETHERNET_OS_DETECT_IRQ_MASK
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_RX_64B66B_ETHERNET_OS_DETECT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_RX_64B66B_ETHERNET_OS_DETECT_IRQ_MASK     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_RX_64B66B_ETHERNET_OS_DETECT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/**
 * Register Group: \a GFP::GFPM_VERSION
 *
 * GFP mapper version registers
 */

/**
 * Register Group: \a GFP::GFPM_TX_GLOBAL
 *
 * GFP mapper Tx global registers
 */


/** 
 * \brief GFPM Tx global control register
 *
 * \details
 * Register: \a GFP::GFPM_TX_GLOBAL::TX_GLOBAL_CONTROL
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL(target)  VTSS_IOREG(target,0x401)

/** 
 * \brief
 * Mapper mode.
 * The mapper mode bits are used to configure the GFP mapper mode of
 * operation.
 *
 * \details 
 * 00: 8 Gigabit Fibre Channel
 * 01: Reserved
 * 10: 10 Gigabit Ethernet Packet (G.Sup43.6.2)
 * 11: 10 Gigabit Ethernet PP/OS (G.Sup43.7.3)	 
 *
 * Field: ::VTSS_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL . MAPPER_MODE
 */
#define  VTSS_F_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_MAPPER_MODE(x)  VTSS_ENCODE_BITFIELD(x,14,2)
#define  VTSS_M_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_MAPPER_MODE     VTSS_ENCODE_BITMASK(14,2)
#define  VTSS_X_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_MAPPER_MODE(x)  VTSS_EXTRACT_BITFIELD(x,14,2)

/** 
 * \brief
 * FIFO reset.
 * If the FIFO reset bit is set, the core will reset the transmit processor
 * FIFO read pointer. When the FIFO is reset, the read pointer (line side
 * of FIFO) address is reset to a value that is a distance (set through
 * design package) typically equal to half the depth of the FIFO away from
 * the write pointer. This bit is auto cleared when set.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL . FIFO_RESET
 */
#define  VTSS_F_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_FIFO_RESET(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_FIFO_RESET     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_FIFO_RESET(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * SW reset.
 * If the SW reset bit is set, the GFP Tx processor will perform a
 * synchronous reset operation. This state is equivalent to an external
 * hardware reset of the device.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL . SW_RESET
 */
#define  VTSS_F_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_SW_RESET(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_SW_RESET     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_SW_RESET(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Tx facility loopback.
 * If the Tx facility loopback bit is set, incoming transmit system side
 * data and associated signaling is looped back to the appropriate receive
 * system side outputs.
 * NOTE: This bit is valid for all modes of operation except 10 Gigabit
 * Ethernet packet mode.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL . TX_FACILITY_LOOPBACK
 */
#define  VTSS_F_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_TX_FACILITY_LOOPBACK(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_TX_FACILITY_LOOPBACK     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_TX_FACILITY_LOOPBACK(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Tx terminal loopback.
 * If the Tx terminal loopback bit is set, incoming transmit data is looped
 * back to the receive system side interface after GFP processing.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL . TX_TERMINAL_LOOPBACK
 */
#define  VTSS_F_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_TX_TERMINAL_LOOPBACK(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_TX_TERMINAL_LOOPBACK     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_TX_TERMINAL_LOOPBACK(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief GFPM Tx FIFO write high threshold register
 *
 * \details
 * Register: \a GFP::GFPM_TX_GLOBAL::TX_FIFO_WRITE_HIGH_THRESHOLD
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_GLOBAL_TX_FIFO_WRITE_HIGH_THRESHOLD(target)  VTSS_IOREG(target,0x402)

/** 
 * \brief
 * FIFO write high threshold value.
 * If the difference between the Tx FIFO write address (system side) and
 * the Tx FIFO read address (line side) is more than the value programmed
 * into the FIFO write high threshold value bits, a FIFO write high
 * threshold interrupt will be asserted.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GLOBAL_TX_FIFO_WRITE_HIGH_THRESHOLD . FIFO_WRITE_HIGH_THRESHOLD_VALUE
 */
#define  VTSS_F_GFP_GFPM_TX_GLOBAL_TX_FIFO_WRITE_HIGH_THRESHOLD_FIFO_WRITE_HIGH_THRESHOLD_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_TX_GLOBAL_TX_FIFO_WRITE_HIGH_THRESHOLD_FIFO_WRITE_HIGH_THRESHOLD_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_TX_GLOBAL_TX_FIFO_WRITE_HIGH_THRESHOLD_FIFO_WRITE_HIGH_THRESHOLD_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Tx FIFO write low threshold register
 *
 * \details
 * Register: \a GFP::GFPM_TX_GLOBAL::TX_FIFO_WRITE_LOW_THRESHOLD
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_GLOBAL_TX_FIFO_WRITE_LOW_THRESHOLD(target)  VTSS_IOREG(target,0x403)

/** 
 * \brief
 * FIFO write low threshold value.
 * If the difference between the Tx FIFO write address (system side) and
 * the Tx FIFO read address (line side) is less than the value programmed
 * into the FIFO write low threshold value bits, a FIFO write low threshold
 * interrupt will be asserted.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GLOBAL_TX_FIFO_WRITE_LOW_THRESHOLD . FIFO_WRITE_LOW_THRESHOLD_VALUE
 */
#define  VTSS_F_GFP_GFPM_TX_GLOBAL_TX_FIFO_WRITE_LOW_THRESHOLD_FIFO_WRITE_LOW_THRESHOLD_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_TX_GLOBAL_TX_FIFO_WRITE_LOW_THRESHOLD_FIFO_WRITE_LOW_THRESHOLD_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_TX_GLOBAL_TX_FIFO_WRITE_LOW_THRESHOLD_FIFO_WRITE_LOW_THRESHOLD_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Tx FIFO read high threshold register
 *
 * \details
 * Register: \a GFP::GFPM_TX_GLOBAL::TX_FIFO_READ_HIGH_THRESHOLD
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_GLOBAL_TX_FIFO_READ_HIGH_THRESHOLD(target)  VTSS_IOREG(target,0x404)

/** 
 * \brief
 * FIFO read high threshold value.
 * If the difference between the Tx FIFO write address (system side) and
 * the Tx FIFO read address (line side) is more than the value programmed
 * into the FIFO read high threshold value bits, a FIFO read high threshold
 * interrupt will be asserted.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GLOBAL_TX_FIFO_READ_HIGH_THRESHOLD . FIFO_READ_HIGH_THRESHOLD_VALUE
 */
#define  VTSS_F_GFP_GFPM_TX_GLOBAL_TX_FIFO_READ_HIGH_THRESHOLD_FIFO_READ_HIGH_THRESHOLD_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_TX_GLOBAL_TX_FIFO_READ_HIGH_THRESHOLD_FIFO_READ_HIGH_THRESHOLD_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_TX_GLOBAL_TX_FIFO_READ_HIGH_THRESHOLD_FIFO_READ_HIGH_THRESHOLD_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Tx FIFO read low threshold register
 *
 * \details
 * Register: \a GFP::GFPM_TX_GLOBAL::TX_FIFO_READ_LOW_THRESHOLD
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_GLOBAL_TX_FIFO_READ_LOW_THRESHOLD(target)  VTSS_IOREG(target,0x405)

/** 
 * \brief
 * FIFO read low threshold value.
 * If the difference between the Tx FIFO write address (system side) and
 * the Tx FIFO read address (line side) is less than the value programmed
 * into the FIFO read low threshold value bits, a FIFO read low threshold
 * interrupt will be asserted.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GLOBAL_TX_FIFO_READ_LOW_THRESHOLD . FIFO_READ_LOW_THRESHOLD_VALUE
 */
#define  VTSS_F_GFP_GFPM_TX_GLOBAL_TX_FIFO_READ_LOW_THRESHOLD_FIFO_READ_LOW_THRESHOLD_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_TX_GLOBAL_TX_FIFO_READ_LOW_THRESHOLD_FIFO_READ_LOW_THRESHOLD_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_TX_GLOBAL_TX_FIFO_READ_LOW_THRESHOLD_FIFO_READ_LOW_THRESHOLD_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a GFP::GFPM_TX_10GE_PPOS
 *
 * GFP mapper Tx 10GE PP/OS registers
 */


/** 
 * \brief GFPM Tx PP/OS global control register
 *
 * \details
 * Register: \a GFP::GFPM_TX_10GE_PPOS::TX_PPOS_GLOBAL_CONTROL
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL(target)  VTSS_IOREG(target,0x420)

/** 
 * \brief
 * Test mode.
 * If the test mode bit is set, test mode is enabled and incoming data is
 * dropped (not counted as errors or valid) and replaced by either data
 * packets or ordered sets as specified by the test generate select bits.
 * This bit is cleared for normal operation.	    
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL . TEST_MODE
 */
#define  VTSS_F_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_TEST_MODE(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_TEST_MODE     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_TEST_MODE(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Include preamble.
 * If the include preamble bit is set, the incoming ethernet frame is
 * encoded for GFP encapsulation using the preamble and frame start
 * delimiter.
 * NOTE: The incoming /S/ character will be replaced by a value of 0x'55'
 * prior to GFP-F encapsulation as specified in G.SUP43 section 7.3. If the
 * include preamble bit is cleared, the incoming preamble and frame start
 * delimiter fields will not be included for GFP-F encapsulation.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL . INCLUDE_PREAMBLE
 */
#define  VTSS_F_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_INCLUDE_PREAMBLE(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_INCLUDE_PREAMBLE     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_INCLUDE_PREAMBLE(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Test generate select.
 * The test generate select bits are used to indicate the type of data
 * generated when the test mode bit is set. If the test mode bit is
 * cleared, the test generate select bits are ignored.
 *
 * \details 
 * 00 or 11: 116 byte 'AA' test data packet (IFG=16)
 * 01: Continuous 4 byte sequence ordered set
 * 10: Continuous 4 byte signal ordered set
 *
 * Field: ::VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL . TEST_GENERATE_SELECT
 */
#define  VTSS_F_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_TEST_GENERATE_SELECT(x)  VTSS_ENCODE_BITFIELD(x,11,2)
#define  VTSS_M_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_TEST_GENERATE_SELECT     VTSS_ENCODE_BITMASK(11,2)
#define  VTSS_X_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_TEST_GENERATE_SELECT(x)  VTSS_EXTRACT_BITFIELD(x,11,2)

/** 
 * \brief
 * Block OS encode.
 * If the block OS encode bit is set, incoming signal or sequence ordered
 * sets will be blocked and not encoded for GFP-F encapsulation. If the
 * block OS encode bit is cleared, incoming signal or sequence ordered sets
 * will be encoded for GFP-F encapsulation.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL . BLOCK_OS_ENCODE
 */
#define  VTSS_F_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_BLOCK_OS_ENCODE(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_BLOCK_OS_ENCODE     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_BLOCK_OS_ENCODE(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Err sync mode.
 * If the err sync mode bit is set, all internal counters will be captured
 * on the rising edge of the incoming ERR SYNC IN internal input
 * (accessible as PMTICK) and transferred to their respective shadow memory
 * locations for user access (user read operations). In the err sync mode
 * of operation, latch and clear functionality for all counters is still
 * valid if the user wants the updated count between error sync intervals. 
 * If the err sync mode bit is cleared, counter latch and clear mode is
 * enabled, and internal counters are captured and made available for the
 * user to read only when the designated counter latch and clear bit is
 * asserted. Internal counters are always cleared when counts are captured
 * and transferred through either err sync or latch and clear modes of
 * operation. 
 * NOTE: the minimum pulse width for the ERR SYNC IN (PMTICK) signal is two
 * register interface clocks.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL . ERR_SYNC_MODE
 */
#define  VTSS_F_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_ERR_SYNC_MODE(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_ERR_SYNC_MODE     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_ERR_SYNC_MODE(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Normal data packet latch and clear.
 * If the normal data packet latch and clear bit is set, the core will
 * latch the current normal data packet count value and transfer it to the
 * normal data packet counter value bits of the Tx PP/OS normal data packet
 * counter. The Tx PP/OS normal data packet counter should not be read
 * after asserting the sync block error latch and clear bit until this bit
 * is auto cleared.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL . NORMAL_DATA_PKT_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_NORMAL_DATA_PKT_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_NORMAL_DATA_PKT_LAT_CLR     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_NORMAL_DATA_PKT_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Sequence OS latch and clear.
 * If the sequence OS latch and clear bit is set, the core will latch the
 * current sequence ordered set count value and transfer it to the sequence
 * ordered set counter value bits of the Tx PP/OS sequence ordered set
 * counter. The Tx PP/OS sequence ordered set counter should not be read
 * after asserting the sequence OS latch and clear bit until this bit is
 * auto cleared.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL . SEQUENCE_OS_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_SEQUENCE_OS_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_SEQUENCE_OS_LAT_CLR     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_SEQUENCE_OS_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Signal OS latch and clear.
 * If the signal OS latch and clear bit is set, the core will latch the
 * current signal ordered set count value and transfer it to the signal
 * ordered set counter value bits of the Tx PP/OS signal ordered set
 * counter. The Tx PP/OS signal ordered set counter should not be read
 * after asserting the signal OS latch and clear bit until this bit is auto
 * cleared.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL . SIGNAL_OS_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_SIGNAL_OS_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_SIGNAL_OS_LAT_CLR     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_SIGNAL_OS_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Error latch and clear.
 * If the error latch and clear bit is set, the core will latch the error
 * count value and transfer it to the error counter value bits of the Tx
 * PP/OS error counter. The Tx PP/OS error counter should not be read after
 * asserting the error latch and clear bit until this bit is auto cleared.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL . ERROR_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_ERROR_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_ERROR_LAT_CLR     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_GLOBAL_CONTROL_ERROR_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,3,1)


/** 
 * \brief GFPM Tx PP/OS UPI register
 *
 * \details
 * Register: \a GFP::GFPM_TX_10GE_PPOS::TX_PPOS_UPI
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPI(target)  VTSS_IOREG(target,0x421)

/** 
 * \brief
 * Data UPI value.
 * The data UPI value bits are used to indicate the UPI value used for
 * GFP-F encapsulation of incoming ethernet data frames. These bits have a
 * default value of 0001 0011 as recommended by specification G.SUP43
 * section 7.3.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPI . DATA_UPI_VALUE
 */
#define  VTSS_F_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPI_DATA_UPI_VALUE(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPI_DATA_UPI_VALUE     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPI_DATA_UPI_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Ordered set UPI value.
 * The ordered set UPI value bits are used to indicate the UPI value used
 * for GFP-F encapsulation of incoming ethernet ordered set frames. These
 * bits have a default value of 0001 0100 as recommended by specification
 * G.SUP43 section 7.3. 
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPI . ORDERED_SET_UPI_VALUE
 */
#define  VTSS_F_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPI_ORDERED_SET_UPI_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPI_ORDERED_SET_UPI_VALUE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPI_ORDERED_SET_UPI_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief GFPM Tx PP/OS upper normal data packet counter
 *
 * \details
 * Register: \a GFP::GFPM_TX_10GE_PPOS::TX_PPOS_UPPER_NORMAL_DATA_PKT_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPPER_NORMAL_DATA_PKT_CNT(target)  VTSS_IOREG(target,0x422)

/** 
 * \brief
 * Upper normal data packet counter value.
 * The upper normal data packet counter value bits are used to accumulate
 * the number incoming valid 10GE normal data packets received. A normal
 * data packet is counted when a valid start character is received followed
 * by a valid terminate character without error detection. For more
 * information, see error conditions described in Tx PP/OS error counter.
 * The upper normal data packet counter value bits contain the upper 16
 * bits of the 32 bit Tx PP/OS normal data packet counter. This is a
 * saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPPER_NORMAL_DATA_PKT_CNT . UPPER_NORMAL_DATA_PKT_CNT_VALUE
 */
#define  VTSS_F_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPPER_NORMAL_DATA_PKT_CNT_UPPER_NORMAL_DATA_PKT_CNT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPPER_NORMAL_DATA_PKT_CNT_UPPER_NORMAL_DATA_PKT_CNT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPPER_NORMAL_DATA_PKT_CNT_UPPER_NORMAL_DATA_PKT_CNT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Tx PP/OS lower normal data packet counter
 *
 * \details
 * Register: \a GFP::GFPM_TX_10GE_PPOS::TX_PPOS_LOWER_NORMAL_DATA_PKT_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_LOWER_NORMAL_DATA_PKT_CNT(target)  VTSS_IOREG(target,0x423)

/** 
 * \brief
 * Lower normal data packet counter value.
 * The lower normal data packet counter value bits are used to accumulate
 * the number incoming valid 10GE normal data packets received. A normal
 * data packet is counted when a valid start character is received followed
 * by a valid terminate character without error detection (for more
 * information, see error conditions described in Tx PP/OS error counter
 * description). The lower normal data packet counter value bits contain
 * the lower 16 bits of the 32 bit Tx PP/OS normal data packet counter.
 * This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_LOWER_NORMAL_DATA_PKT_CNT . LOWER_NORMAL_DATA_PKT_CNT_VALUE
 */
#define  VTSS_F_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_LOWER_NORMAL_DATA_PKT_CNT_LOWER_NORMAL_DATA_PKT_CNT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_LOWER_NORMAL_DATA_PKT_CNT_LOWER_NORMAL_DATA_PKT_CNT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_LOWER_NORMAL_DATA_PKT_CNT_LOWER_NORMAL_DATA_PKT_CNT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Tx PP/OS upper sequence ordered set counter
 *
 * \details
 * Register: \a GFP::GFPM_TX_10GE_PPOS::TX_PPOS_UPPER_SEQ_ORD_SET_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPPER_SEQ_ORD_SET_CNT(target)  VTSS_IOREG(target,0x424)

/** 
 * \brief
 * Upper sequence ordered set counter value.
 * The upper sequence ordered set counter value bits are used to accumulate
 * the number valid incoming sequence ordered sets received. The upper
 * sequence ordered set counter value bits contain the upper 16 bits of the
 * 32 bit Tx PP/OS sequence ordered set counter. This is a saturating read
 * only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPPER_SEQ_ORD_SET_CNT . UPPER_SEQ_ORD_SET_CNT_VALUE
 */
#define  VTSS_F_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPPER_SEQ_ORD_SET_CNT_UPPER_SEQ_ORD_SET_CNT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPPER_SEQ_ORD_SET_CNT_UPPER_SEQ_ORD_SET_CNT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPPER_SEQ_ORD_SET_CNT_UPPER_SEQ_ORD_SET_CNT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Tx PP/OS lower sequence ordered set counter
 *
 * \details
 * Register: \a GFP::GFPM_TX_10GE_PPOS::TX_PPOS_LOWER_SEQ_ORD_SET_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_LOWER_SEQ_ORD_SET_CNT(target)  VTSS_IOREG(target,0x425)

/** 
 * \brief
 * Lower sequence ordered set counter value.
 * The lower sequence ordered set counter value bits are used to accumulate
 * the number valid incoming sequence ordered sets received. The lower
 * sequence ordered set counter value bits contain the lower 16 bits of the
 * 32 bit Tx PP/OS sequence ordered set counter. This is a saturating read
 * only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_LOWER_SEQ_ORD_SET_CNT . LOWER_SEQ_ORD_SET_CNT_VALUE
 */
#define  VTSS_F_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_LOWER_SEQ_ORD_SET_CNT_LOWER_SEQ_ORD_SET_CNT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_LOWER_SEQ_ORD_SET_CNT_LOWER_SEQ_ORD_SET_CNT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_LOWER_SEQ_ORD_SET_CNT_LOWER_SEQ_ORD_SET_CNT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Tx PP/OS upper signal ordered set counter
 *
 * \details
 * Register: \a GFP::GFPM_TX_10GE_PPOS::TX_PPOS_UPPER_SIG_ORD_SET_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPPER_SIG_ORD_SET_CNT(target)  VTSS_IOREG(target,0x426)

/** 
 * \brief
 * Upper signal ordered set counter value.
 * The upper signal ordered set counter value bits are used to accumulate
 * the number valid incoming signal ordered sets received. The upper signal
 * ordered set counter value bits contain the upper 16 bits of the 32 bit
 * Tx PP/OS signal ordered set counter. This is a saturating read only
 * counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPPER_SIG_ORD_SET_CNT . UPPER_SIG_ORD_SET_CNT_VALUE
 */
#define  VTSS_F_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPPER_SIG_ORD_SET_CNT_UPPER_SIG_ORD_SET_CNT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPPER_SIG_ORD_SET_CNT_UPPER_SIG_ORD_SET_CNT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPPER_SIG_ORD_SET_CNT_UPPER_SIG_ORD_SET_CNT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Tx PP/OS lower signal ordered set counter
 *
 * \details
 * Register: \a GFP::GFPM_TX_10GE_PPOS::TX_PPOS_LOWER_SIG_ORD_SET_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_LOWER_SIG_ORD_SET_CNT(target)  VTSS_IOREG(target,0x427)

/** 
 * \brief
 * Lower signal ordered set counter value.
 * The lower signal ordered set counter value bits are used to accumulate
 * the number valid incoming signal ordered sets received. The lower signal
 * ordered set counter value bits contain the lower 16 bits of the 32 bit
 * Tx PP/OS signal ordered set counter. This is a saturating read only
 * counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_LOWER_SIG_ORD_SET_CNT . LOWER_SIG_ORD_SET_CNT_VALUE
 */
#define  VTSS_F_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_LOWER_SIG_ORD_SET_CNT_LOWER_SIG_ORD_SET_CNT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_LOWER_SIG_ORD_SET_CNT_LOWER_SIG_ORD_SET_CNT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_LOWER_SIG_ORD_SET_CNT_LOWER_SIG_ORD_SET_CNT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Tx PP/OS error counter
 *
 * \details
 * Register: \a GFP::GFPM_TX_10GE_PPOS::TX_PPOS_ERROR_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_ERROR_CNT(target)  VTSS_IOREG(target,0x428)

/** 
 * \brief
 * Error data packet counter value.
 * The error data packet counter value bits are used to accumulate the
 * number incoming 10GE error data packets received. An error data packet
 * is counted when any of the following conditions occur:
 *  1. An error character is detected during a data packet reception. 
 *  2. The incoming data packet minimum size is violated.
 *  3. The incoming data packet maximum size is violated.
 *  4. Start of data packet or ordered set character is in an invalid byte
 * lane.
 *  5. A control character other than TERMINATE or ERROR is detected after
 * data packet reception has started.
 *  6. The external packet length (when valid) does not match the
 * calculated incoming data packet length.
 *  7. Invalid IPG is detected.
 *  8. EOP does not occur when expected as specified by length
 * input/calculation.
 *  9. Incoming control occurs on an invalid byte lane.
 * This is a saturating counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_ERROR_CNT . ERROR_DATA_PKT_CNT_VALUE
 */
#define  VTSS_F_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_ERROR_CNT_ERROR_DATA_PKT_CNT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_ERROR_CNT_ERROR_DATA_PKT_CNT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_ERROR_CNT_ERROR_DATA_PKT_CNT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a GFP::GFPM_TX_8B10B_DECODER
 *
 * GFP mapper Tx 8B/10B registers
 */


/** 
 * \brief GFPM Tx 8B/10B global control register
 *
 * \details
 * Register: \a GFP::GFPM_TX_8B10B_DECODER::TX_8B10B_GLOBAL_CONTROL
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL(target)  VTSS_IOREG(target,0x480)

/** 
 * \brief
 * Force LOS.
 * If the force LOS bit is set, the synchronization state machine will
 * immediately be placed in the loss of synchronization state and an LOS
 * interrupt will be generated. The force LOS bit is cleared for normal
 * operation.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL . FORCE_LOS
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_FORCE_LOS(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_FORCE_LOS     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_FORCE_LOS(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Force LSYNC.
 * If the force LSYNC bit is set, the synchronization state machine will
 * immediately be placed in the loss of synchronization state and an LSYNC
 * interrupt will be generated. The force LSYNC bit is cleared for normal
 * operation.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL . FORCE_LSYNC
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_FORCE_LSYNC(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_FORCE_LSYNC     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_FORCE_LSYNC(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * K30.7 error character select.
 * If the K30.7 error character select bit is set and the synchronization
 * state machine is in the sync state, the 8B/10B decoder will output a
 * K30.7 (0xfe) control character when a disparity error or invalid special
 * character is detected. 
 * If the K30.7 error character select bit is cleared and the
 * synchronization state machine is in the sync state, the 8B/10B decoder
 * will output a 10B_ERR control character (0x01) when a disparity error or
 * invalid special character is detected. 
 * If the synchronization state machine is in the loss of synchronization
 * state, this bit is ignored and no characters are generated by the 8B/10B
 * decoder.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL . K30_7_ERROR_CHARACTER_SELECT
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_K30_7_ERROR_CHARACTER_SELECT(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_K30_7_ERROR_CHARACTER_SELECT     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_K30_7_ERROR_CHARACTER_SELECT(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Err sync mode.
 * If the ERR sync mode bit is set, all internal counters will be captured
 * on the rising edge of the incoming ERR SYNC IN internal input
 * (accessible as PMTICK) and transferred to their respective shadow memory
 * locations for user access (user read operations). In the err sync mode
 * of operation, latch and clear operation for all counters is still valid
 * if the user wants the updated count between error sync intervals. If the
 * err sync mode bit is cleared, counter latch and clear mode is enabled
 * and internal counters are captured and made available for the user to
 * read only when the designated counter latch and clear bit is asserted.
 * Internal counters are always cleared when counts are captured and
 * transferred through either err sync or latch and clear modes of
 * operation.
 * NOTE: The minimum pulse width for the ERR SYNC IN (PMTICK) signal is two
 * register interface clocks.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL . ERR_SYNC_MODE
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_ERR_SYNC_MODE(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_ERR_SYNC_MODE     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_ERR_SYNC_MODE(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Disparity error latch and clear.
 * If the disparity error latch and clear bit is set, the core will latch
 * the current disparity error count value and transfer it to the disparity
 * error counter value bits of the Tx 8B/10B disparity error counter. The
 * disparity error counter value is available to be read until the next
 * disparity error latch and clear event occurs. The Tx 8B/10B disparity
 * error counter should not be read after asserting the disparity error
 * latch and clear bit until this bit is auto cleared.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL . DISPARITY_ERROR_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_DISPARITY_ERROR_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_DISPARITY_ERROR_LAT_CLR     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_DISPARITY_ERROR_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Invalid character latch and clear.
 * If the invalid character latch and clear bit is set, the core will latch
 * the current invalid character count value and transfer it to the invalid
 * character counter value bits of the Tx 8B/10B invalid character counter.
 * The invalid character counter value is available to be read until the
 * next invalid character latch and clear event occurs. The Tx 8B/10B
 * invalid character counter should not be read after asserting the invalid
 * character latch and clear bit until this bit is auto cleared.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL . INVALID_CHARACTER_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_INVALID_CHARACTER_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_INVALID_CHARACTER_LAT_CLR     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_INVALID_CHARACTER_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Valid codeword latch and clear.
 * If the valid codeword latch and clear bit is set, the core will latch
 * the current valid codeword count value and transfer it to the valid
 * codeword counter value bits of the Tx 8B/10B valid codeword counter. The
 * valid codeword counter value is available to be read until the next
 * valid codeword latch and clear event occurs. The Tx 8B/10B valid
 * codeword counter should not be read after asserting the valid codeword
 * latch and clear bit until this bit is auto cleared.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL . VALID_CODEWORD_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_VALID_CODEWORD_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_VALID_CODEWORD_LAT_CLR     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_VALID_CODEWORD_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * LSYNC latch and clear.
 * If the LSYNC latch and clear bit is set, the core will latch the current
 * LSYNC count value and transfer it to the LSYNC counter value bits of the
 * Tx 8B/10B LSYNC counter. The LSYNC counter value is available to be read
 * until the next LSYNC latch and clear event occurs. The Tx 8B/10B LSYNC
 * counter should not be read after asserting the LSYNC latch and clear bit
 * until this bit is auto cleared.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL . LSYNC_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_LSYNC_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_LSYNC_LAT_CLR     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_LSYNC_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,3,1)


/** 
 * \brief GFPM Tx 8B/10B disparity error counter
 *
 * \details
 * Register: \a GFP::GFPM_TX_8B10B_DECODER::TX_8B10B_DISPARITY_ERROR_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_DISPARITY_ERROR_CNT(target)  VTSS_IOREG(target,0x483)

/** 
 * \brief
 * Disparity error value.
 * The disparity error counter value bits are used to accumulate the number
 * of disparity errors detected when the synchronization state machine is
 * in the SYNC state. This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_DISPARITY_ERROR_CNT . DISPARITY_ERROR_VALUE
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_DISPARITY_ERROR_CNT_DISPARITY_ERROR_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_DISPARITY_ERROR_CNT_DISPARITY_ERROR_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_DISPARITY_ERROR_CNT_DISPARITY_ERROR_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Tx 8B/10B invalid character counter
 *
 * \details
 * Register: \a GFP::GFPM_TX_8B10B_DECODER::TX_8B10B_INVALID_CHARACTER_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_INVALID_CHARACTER_CNT(target)  VTSS_IOREG(target,0x484)

/** 
 * \brief
 * Invalid character counter value.
 * The invalid character counter value bits are used to accumulate the
 * number of invalid characters detected when the synchronization state
 * machine is in the SYNC state. This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_INVALID_CHARACTER_CNT . INVALID_CHARACTER_CNT_VALUE
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_INVALID_CHARACTER_CNT_INVALID_CHARACTER_CNT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_INVALID_CHARACTER_CNT_INVALID_CHARACTER_CNT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_INVALID_CHARACTER_CNT_INVALID_CHARACTER_CNT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Tx 8B/10B upper valid codeword counter
 *
 * \details
 * Register: \a GFP::GFPM_TX_8B10B_DECODER::TX_8B10B_UPPER_VALID_CODEWORD_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_UPPER_VALID_CODEWORD_CNT(target)  VTSS_IOREG(target,0x485)

/** 
 * \brief
 * Upper valid codeword counter value.
 * The upper valid codeword counter value bits are used to accumulate the
 * number of valid codewords detected when the synchronization state
 * machine is in the SYNC state. The upper valid codeword counter value
 * bits contain the upper 8 bits of the 40 bit TX 8B/10B valid codeword
 * counter. This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_UPPER_VALID_CODEWORD_CNT . UPPER_VALID_CODEWORD_CNT_VALUE
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_UPPER_VALID_CODEWORD_CNT_UPPER_VALID_CODEWORD_CNT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_UPPER_VALID_CODEWORD_CNT_UPPER_VALID_CODEWORD_CNT_VALUE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_UPPER_VALID_CODEWORD_CNT_UPPER_VALID_CODEWORD_CNT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief GFPM Tx 8B/10B mid valid codeword counter
 *
 * \details
 * Register: \a GFP::GFPM_TX_8B10B_DECODER::TX_8B10B_MID_VALID_CODEWORD_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_MID_VALID_CODEWORD_CNT(target)  VTSS_IOREG(target,0x486)

/** 
 * \brief
 * Mid valid codeword counter value.
 * The mid valid codeword counter value bits are used to accumulate the
 * number of valid codewords detected when the synchronization state
 * machine is in the SYNC state. The mid valid codeword counter value bits
 * contain bits 31 down to 16 of the 40 bit TX 8B/10B valid codeword
 * counter. This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_MID_VALID_CODEWORD_CNT . MID_VALID_CODEWORD_CNT_VALUE
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_MID_VALID_CODEWORD_CNT_MID_VALID_CODEWORD_CNT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_MID_VALID_CODEWORD_CNT_MID_VALID_CODEWORD_CNT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_MID_VALID_CODEWORD_CNT_MID_VALID_CODEWORD_CNT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Tx 8B/10B lower valid codeword counter
 *
 * \details
 * Register: \a GFP::GFPM_TX_8B10B_DECODER::TX_8B10B_LOWER_VALID_CODEWORD_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_LOWER_VALID_CODEWORD_CNT(target)  VTSS_IOREG(target,0x487)

/** 
 * \brief
 * Lower valid codeword counter value.
 * The lower valid codeword counter value bits are used to accumulate the
 * number of valid codewords detected when the synchronization state
 * machine is in the SYNC state. The lower valid codeword counter value
 * bits contain the lower 16 bits of the 40 bit TX 8B/10B valid codeword
 * counter. This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_LOWER_VALID_CODEWORD_CNT . LOWER_VALID_CODEWORD_CNT_VALUE
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_LOWER_VALID_CODEWORD_CNT_LOWER_VALID_CODEWORD_CNT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_LOWER_VALID_CODEWORD_CNT_LOWER_VALID_CODEWORD_CNT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_LOWER_VALID_CODEWORD_CNT_LOWER_VALID_CODEWORD_CNT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Tx 8B/10B LSYNC counter
 *
 * \details
 * Register: \a GFP::GFPM_TX_8B10B_DECODER::TX_8B10B_LSYNC_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_LSYNC_CNT(target)  VTSS_IOREG(target,0x488)

/** 
 * \brief
 * LSYNC counter value.
 * The LSYNC counter value bits are used to accumulate the number of
 * transitions from the SYNC state to the loss of synchronization (LSYNC)
 * state detected in the synchronization state machine. This is a
 * saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_LSYNC_CNT . LSYNC_CNT_VALUE
 */
#define  VTSS_F_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_LSYNC_CNT_LSYNC_CNT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_LSYNC_CNT_LSYNC_CNT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_LSYNC_CNT_LSYNC_CNT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a GFP::GFPM_TX_GFP
 *
 * GFP mapper Tx GFP registers
 */


/** 
 * \brief GFPM Tx GFP frame control register
 *
 * \details
 * Register: \a GFP::GFPM_TX_GFP::TX_GFP_FRAME_CONTROL
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL(target)  VTSS_IOREG(target,0x500)

/** 
 * \brief
 * GFP test mode.
 * The GFP test mode bit is used to enable the generation of GFP test
 * frames. A client data proprietary UPI code of 11110000 is inserted for
 * all test frames. For more information about GFP test frame insertion,
 * see the Tx GFP test frame control register bit descriptions.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL . GFP_TEST_MODE
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_GFP_TEST_MODE(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_GFP_TEST_MODE     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_GFP_TEST_MODE(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Force control frame insert.
 * If the force control frame insert bit is set, GFP control frames are
 * generated unconditionally. 
 * If this bit is cleared, normal operation is assumed.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL . FORCE_CONTROL_FRAME_INSERT
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_FORCE_CONTROL_FRAME_INSERT(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_FORCE_CONTROL_FRAME_INSERT     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_FORCE_CONTROL_FRAME_INSERT(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Core header scramble enable.
 * If the core header scramble enable bit is set, the core header for each
 * GFP frame is scrambled as per the GFP specification.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL . CORE_HDR_SCRAMBLE_ENABLE
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_CORE_HDR_SCRAMBLE_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_CORE_HDR_SCRAMBLE_ENABLE     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_CORE_HDR_SCRAMBLE_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Payload scramble enable.
 * If the payload scramble enable bit is set, the payload information for
 * each GFP frame is scrambled as per the GFP specification.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL . PYLD_SCRAMBLE_ENABLE
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_PYLD_SCRAMBLE_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_PYLD_SCRAMBLE_ENABLE     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_PYLD_SCRAMBLE_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * cHEC corrupt enable.
 * If the cHEC corrupt enable bit is set, the calculated cHEC value for
 * each GFP frame will be added (modulo 2) to the HEC/CRC corrupt mask
 * bits. This bit will be auto cleared when set after the number of GFP
 * frames indicated by the value programmed into the corrupt frame count
 * bits.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL . CHEC_CORRUPT_ENABLE
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_CHEC_CORRUPT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_CHEC_CORRUPT_ENABLE     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_CHEC_CORRUPT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * tHEC corrupt enable.
 * If the tHEC corrupt enable bit is set, the calculated tHEC value for
 * each GFP frame will be added (modulo 2) to the HEC/CRC corrupt mask
 * bits. This bit will be auto cleared when set after the number of GFP
 * frames indicated by the value programmed into the corrupt frame count
 * bits.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL . THEC_CORRUPT_ENABLE
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_THEC_CORRUPT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_THEC_CORRUPT_ENABLE     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_THEC_CORRUPT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * FCS corrupt.
 * The FCS corrupt bit is asserted to corrupt the FCS field of transmitted
 * GFP frames when FCS insertion is enabled through the insert PFI bit of
 * the TX GFP frame control register.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL . FCS_CORRUPT
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_FCS_CORRUPT(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_FCS_CORRUPT     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_FCS_CORRUPT(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Insert PFI.
 * If the insert PFI bit is set, the GFP transmit processor will include
 * the calculated 16-bit payload FCS sequence. If this bit is cleared, the
 * FCS will not be included.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL . INSERT_PFI
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_INSERT_PFI(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_INSERT_PFI     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_INSERT_PFI(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Transparent mode CRC16 corrupt enable.
 * If the transparent mode CRC-16 corrupt enable bit is set, the calculated
 * transparent mode CRC-16 value for each GFP superframe frame will be
 * added (modulo 2) to the HEC/CRC corrupt mask bits. This bit will be auto
 * cleared when set after the number of GFP frames indicated by the value
 * programmed into the corrupt frame count bits.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL . TRANSP_MODE_CRC16_CORRUPT_ENABLE
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_TRANSP_MODE_CRC16_CORRUPT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_TRANSP_MODE_CRC16_CORRUPT_ENABLE     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_TRANSP_MODE_CRC16_CORRUPT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Err sync mode.
 * If the ERR sync mode bit is set, all internal counters will be captured
 * on the rising edge of the incoming ERR SYNC IN internal input
 * (accessible as PMTICK) and transferred to their respective shadow memory
 * locations for user access (user read operations). In the err sync mode
 * of operation, latch and clear functionality for all counters is still
 * valid if the user wants the updated count between error sync intervals.
 * If the err sync mode bit is cleared, counter latch and clear mode is
 * enabled and internal counters are captured and made available for the
 * user to read only when the designated counter latch and clear bit is
 * asserted. Internal counters are always cleared when counts are captured
 * and transferred through either err sync or latch and clear modes of
 * operation.
 * NOTE: The minimum pulse width for the ERR SYNC IN (PMTICK) signal is two
 * register interface clocks.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL . ERR_SYNC_MODE
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_ERR_SYNC_MODE(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_ERR_SYNC_MODE     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_ERR_SYNC_MODE(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Control frame counter latch and clear.
 * If the control frame counter latch and clear bit is set, the GFP
 * transmit processor will latch the current internal GFP control frame
 * count value and transfer it into the control frame count bits of the TX
 * GFP control frame counter register. The internal GFP control frame count
 * register will then be cleared. The TX GFP control frame counter register
 * contents are available to be read until the next latch and clear event
 * occurs.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL . CONTROL_FRAME_CNT_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_CONTROL_FRAME_CNT_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_CONTROL_FRAME_CNT_LAT_CLR     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_CONTROL_FRAME_CNT_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Client management packet counter latch and clear.
 * If the client management packet counter latch and clear bit is set, the
 * GFP transmit processor will latch the current internal GFP client
 * management packet count value and transfer it into the client management
 * packet count bits of the Tx GFP client management packet counter
 * register. The internal GFP client management packet count register will
 * then be cleared. The Tx GFP client management packet counter register
 * contents are available to be read until the next latch and clear event
 * occurs.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL . CLIENT_MGMT_PKT_CNT_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_CLIENT_MGMT_PKT_CNT_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_CLIENT_MGMT_PKT_CNT_LAT_CLR     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_CLIENT_MGMT_PKT_CNT_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Client data packet counter latch and clear.
 * If the client data packet counter latch and clear bit is set, the GFP
 * transmit processor will latch the current internal GFP client data
 * packet count value and transfer it into the client data packet count
 * bits of the Tx GFP client data packet counter register. The internal GFP
 * client data packet count register will then be cleared. The Tx GFP
 * client data packet counter register contents are available to be read
 * until the next latch and clear event occurs.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL . CLIENT_DATA_PKT_CNT_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_CLIENT_DATA_PKT_CNT_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_CLIENT_DATA_PKT_CNT_LAT_CLR     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_CLIENT_DATA_PKT_CNT_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Corrupt frame count.
 * The corrupt frame count bits are used to indicate how many GFP frames
 * (superframes for transparent mode) are corrupted when cHEC, tHEC, or
 * transparent mode CRC-16 corruption is enabled.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL . CORRUPT_FRAME_COUNT
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_CORRUPT_FRAME_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_CORRUPT_FRAME_COUNT     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_CORRUPT_FRAME_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief GFPM Tx GFP test frame control register
 *
 * \details
 * Register: \a GFP::GFPM_TX_GFP::TX_GFP_TEST_FRAME_CONTROL
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_GFP_TX_GFP_TEST_FRAME_CONTROL(target)  VTSS_IOREG(target,0x501)

/** 
 * \brief
 * Test packet length value.
 * The test packet length value bits are used for calculating the PLI field
 * of the core header for test frames.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_TEST_FRAME_CONTROL . TEST_PKT_LENGTH_VALUE
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_TEST_FRAME_CONTROL_TEST_PKT_LENGTH_VALUE(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_TEST_FRAME_CONTROL_TEST_PKT_LENGTH_VALUE     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_TEST_FRAME_CONTROL_TEST_PKT_LENGTH_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Test payload value.
 * The test payload value bits are used for the test frame payload
 * information.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_TEST_FRAME_CONTROL . TEST_PYLD_VALUE
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_TEST_FRAME_CONTROL_TEST_PYLD_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_TEST_FRAME_CONTROL_TEST_PYLD_VALUE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_TEST_FRAME_CONTROL_TEST_PYLD_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief GFPM Tx GFP transparent minimum superblock control register
 *
 * \details
 * Register: \a GFP::GFPM_TX_GFP::TX_GFP_TRANSP_MIN_SBLOCK_CONTROL
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_GFP_TX_GFP_TRANSP_MIN_SBLOCK_CONTROL(target)  VTSS_IOREG(target,0x502)

/** 
 * \brief
 * Minimum superblock length.
 * The minimum superblock length bits are used to indicate the minimum
 * number of superblocks used for transparent mode GFP frame transmissions.
 * NOTE: The minimum value allowed for this register is 1. If a minimum
 * superblock length of 0 is programmed, a value of 1 will be used.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_TRANSP_MIN_SBLOCK_CONTROL . MIN_SBLOCK_LENGTH
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_TRANSP_MIN_SBLOCK_CONTROL_MIN_SBLOCK_LENGTH(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_TRANSP_MIN_SBLOCK_CONTROL_MIN_SBLOCK_LENGTH     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_TRANSP_MIN_SBLOCK_CONTROL_MIN_SBLOCK_LENGTH(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief GFPM Tx GFP client management frame control register
 *
 * \details
 * Register: \a GFP::GFPM_TX_GFP::TX_GFP_CLIENT_MGMT_FRAME_CONTROL
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL(target)  VTSS_IOREG(target,0x503)

/** 
 * \brief
 * Force management insert.
 * The force management insert bit is set to force client management frames
 * defined by the Tx GFP client management packet length register and Tx
 * GFP client management packet memory at the interval specified by the
 * management frame interval select bits of the Tx GFP client management
 * frame interval register using the UPI code specified by the force
 * management UPI value bits.
 * NOTE: Generation of client management frames due to the assertion of
 * this bit has priority over the assertion of client management frames due
 * to a valid LOS or LSYNC condition.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL . FORCE_MGMT_INSERT
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_FORCE_MGMT_INSERT(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_FORCE_MGMT_INSERT     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_FORCE_MGMT_INSERT(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * LOS management insert.
 * The LOS management insert bit is set to enable client management frame
 * insertion (as defined by the Tx GFP client management packet length
 * register and Tx GFP client management packet memory) at the interval
 * specified by the management frame interval select bits of the Tx GFP
 * client management frame interval register when an incoming loss of
 * signal is detected. Client management frames generated due to an
 * incoming loss of signal detection contain a value of 00000001 in the GFP
 * frame UPI field as specified in the G.7041 specification. If the LOS
 * management insert bit is cleared and an incoming loss of signal
 * condition is detected, client management frames are not inserted due to
 * the loss of signal condition.
 * NOTE: Generation of client management frames due to the assertion of
 * this bit and incoming LOS detection has priority over the assertion of
 * client management frames due to a valid LSYNC condition but not for
 * client management frames due to the assertion of the force management
 * insert bit.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL . LOS_MGMT_INSERT
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_LOS_MGMT_INSERT(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_LOS_MGMT_INSERT     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_LOS_MGMT_INSERT(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * LSYNC management insert.
 * The LSYNC management insert bit is used to enable client management
 * frame insertion (as defined by the Tx GFP client management packet
 * length register and Tx GFP client management packet memory) at the
 * interval specified by the management frame interval select bits of the
 * Tx GFP client management frame interval register when an incoming loss
 * of synchronization is detected. Client management frames generated due
 * to an incoming loss of signal detection contain a value of 00000010 in
 * the GFP frame UPI field as specified in the G.7041 specification. If the
 * LSYNC management insert bit is cleared and an incoming loss of
 * synchronization condition is detected, client management frames are not
 * inserted due to the loss of synchronization condition.
 * NOTE: Generation of client management frames due to the assertion of
 * this bit and incoming LSYNC detection have lower priority than LOS or
 * force management insert conditions.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL . LSYNC_MGMT_INSERT
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_LSYNC_MGMT_INSERT(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_LSYNC_MGMT_INSERT     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_LSYNC_MGMT_INSERT(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Management insert PFI.
 * If the management insert PFI bit is set, the GFP transmit processor will
 * include the calculated 16-bit payload FCS sequence for frames generated
 * through Tx GFP client management frame control register. If this bit is
 * cleared, the FCS will not be included.
 * NOTE: This bit is only valid when the management PTI value bits are not
 * equal to 100. PFI will always be 0 when the management PTI value bits
 * are equal to 100.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL . MGMT_INSERT_PFI
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_MGMT_INSERT_PFI(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_MGMT_INSERT_PFI     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_MGMT_INSERT_PFI(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * GFPT force first control.
 * If the GFP-T force first control bit is set (the force management insert
 * bit must also be set and the force management UPI value must also be
 * programmed to a value of x'03') the first byte in the generated GFP-T
 * superfame will be coded as a control character.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL . GFPT_FORCE_FIRST_CONTROL
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_GFPT_FORCE_FIRST_CONTROL(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_GFPT_FORCE_FIRST_CONTROL     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_GFPT_FORCE_FIRST_CONTROL(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Management PTI value.
 * The management PTI value bits are inserted into the PTI field of client
 * management frames generated as a consequence of the actions from the
 * assertion of the force management insert, LOS management insert, or
 * LSYNC insert bits.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL . MGMT_PTI_VALUE
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_MGMT_PTI_VALUE(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_MGMT_PTI_VALUE     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_MGMT_PTI_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * Force management UPI value.
 * The force management UPI value bits are inserted into the UPI field of
 * client management frames that are generated due to the assertion of the
 * force management insert bit only. These bits are not used for any client
 * management frames generated for any other condition except the assertion
 * of the force management insert bit condition.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL . FORCE_MGMT_UPI_VALUE
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_FORCE_MGMT_UPI_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_FORCE_MGMT_UPI_VALUE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_FORCE_MGMT_UPI_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief GFPM Tx GFP client management frame interval register
 *
 * \details
 * Register: \a GFP::GFPM_TX_GFP::TX_GFP_CLIENT_MGMT_FRAME_INTERVAL
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_INTERVAL(target)  VTSS_IOREG(target,0x504)

/** 
 * \brief
 * Management frame interval select.
 * The management frame interval select bits are used to set the number of
 * 100 ms intervals in which client management frames will be sent when a
 * valid client management generation event occurs. A valid client
 * management event occurs if the force management insert bit of the Tx GFP
 * client management frame control register is set or the LOS management
 * insert bit of the Tx GFP client management frame control register is set
 * and an incoming loss of signal condition is detected, or the LSYNC
 * management insert bit of the Tx GFP client management frame control
 * register is set and an incoming loss of synchronization condition is
 * detected.
 * NOTE: GFP idles will be sent between client management frames when an
 * event occurs that generates client management frames. 
 * The interval between client management frames (idles sent in between) is
 * defined as follows. 
 *
 * \details 
 * 0000: Client management frames inserted constantly (no idles, no gap)
 * 0001: Client management frames inserted every 100 ms (idles sent in
 * between)
 * 0010: Client management frames inserted every 200 ms (idles sent in
 * between)
 * 0011: Client management frames inserted every 300 ms 
 * 0100: Client management frames inserted every 400 ms 
 * 0101: Client management frames inserted every 500 ms 
 * 0110: Client management frames inserted every 600 ms 
 * 0111: Client management frames inserted every 700 ms 
 * 1000: Client management frames inserted every 800 ms 
 * 1001: Client management frames inserted every 900 ms 
 * 1010: Client management frames inserted every 1000 ms 
 * >1010: Client management frames are not inserted (interval = 0, all
 * idles)
 *
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_INTERVAL . MGMT_FRAME_INTERVAL_SELECT
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_INTERVAL_MGMT_FRAME_INTERVAL_SELECT(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_INTERVAL_MGMT_FRAME_INTERVAL_SELECT     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_INTERVAL_MGMT_FRAME_INTERVAL_SELECT(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief GFPM Tx GFP HEC/CRC corrupt mask register
 *
 * \details
 * Register: \a GFP::GFPM_TX_GFP::TX_GFP_HEC_CRC_CORRUPT_MASK
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_GFP_TX_GFP_HEC_CRC_CORRUPT_MASK(target)  VTSS_IOREG(target,0x505)

/** 
 * \brief
 * HEC corrupt mask.
 * The HEC/CRC corrupt mask bits are used for diagnostic purposes to
 * corrupt the calculated cHEC and/or tHEC and/or transparent mode CRC-16
 * values for GFP frames. 
 * The HEC/CRC corrupt mask bits are added (modulo 2) to the cHEC value
 * when the cHEC corrupt enable bit is set. 
 * The HEC/CRC corrupt mask bits are added (modulo 2) to the tHEC value
 * when the tHEC corrupt enable bit is set. 
 * The HEC/CRC corrupt mask bits are added (modulo 2) to the superblock
 * CRC-16 value when the transparent mode CRC-16 corrupt enable bit is set.
 * 
 * It is possible for cHEC, tHEC, and transparent mode CRC-16 corruption to
 * occur independently or simultaneously as programmed by the corresponding
 * enable bits.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_HEC_CRC_CORRUPT_MASK . HEC_CORRUPT_MASK
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_HEC_CRC_CORRUPT_MASK_HEC_CORRUPT_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_HEC_CRC_CORRUPT_MASK_HEC_CORRUPT_MASK     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_HEC_CRC_CORRUPT_MASK_HEC_CORRUPT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Tx GFP upper control frame counter
 *
 * \details
 * Register: \a GFP::GFPM_TX_GFP::TX_GFP_UPPER_CONTROL_FRAME_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_GFP_TX_GFP_UPPER_CONTROL_FRAME_CNT(target)  VTSS_IOREG(target,0x506)

/** 
 * \brief
 * Upper control frame count.
 * The upper control frame count bits are used for the accumulation of
 * control frames generated. The upper control frame count bits contain the
 * upper 16 bits of the 32 bit Tx GFP control counter. This is a saturating
 * read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_UPPER_CONTROL_FRAME_CNT . UPPER_CONTROL_FRAME_COUNT
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_UPPER_CONTROL_FRAME_CNT_UPPER_CONTROL_FRAME_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_UPPER_CONTROL_FRAME_CNT_UPPER_CONTROL_FRAME_COUNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_UPPER_CONTROL_FRAME_CNT_UPPER_CONTROL_FRAME_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Tx GFP lower control frame counter
 *
 * \details
 * Register: \a GFP::GFPM_TX_GFP::TX_GFP_LOWER_CONTROL_FRAME_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_GFP_TX_GFP_LOWER_CONTROL_FRAME_CNT(target)  VTSS_IOREG(target,0x507)

/** 
 * \brief
 * Lower control frame count
 * The lower control frame count bits are used for the accumulation of
 * control frames generated. The lower control frame count bits contain the
 * lower 16 bits of the 32 bit Tx GFP control counter. This is a saturating
 * read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_LOWER_CONTROL_FRAME_CNT . LOWER_CONTROL_FRAME_COUNT
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_LOWER_CONTROL_FRAME_CNT_LOWER_CONTROL_FRAME_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_LOWER_CONTROL_FRAME_CNT_LOWER_CONTROL_FRAME_COUNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_LOWER_CONTROL_FRAME_CNT_LOWER_CONTROL_FRAME_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Tx GFP upper client management packet counter
 *
 * \details
 * Register: \a GFP::GFPM_TX_GFP::TX_GFP_UPPER_CLIENT_MGMT_PKT_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_GFP_TX_GFP_UPPER_CLIENT_MGMT_PKT_CNT(target)  VTSS_IOREG(target,0x508)

/** 
 * \brief
 * Upper client management packet count.
 * The upper client management packet count bits are used for the
 * accumulation of client management packets generated. The upper client
 * management packet count bits contain the upper 16 bits of the 32 bit Tx
 * GFP client management counter. This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_UPPER_CLIENT_MGMT_PKT_CNT . UPPER_CLIENT_MGMT_PKT_COUNT
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_UPPER_CLIENT_MGMT_PKT_CNT_UPPER_CLIENT_MGMT_PKT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_UPPER_CLIENT_MGMT_PKT_CNT_UPPER_CLIENT_MGMT_PKT_COUNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_UPPER_CLIENT_MGMT_PKT_CNT_UPPER_CLIENT_MGMT_PKT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Tx GFP lower client management packet counter
 *
 * \details
 * Register: \a GFP::GFPM_TX_GFP::TX_GFP_LOWER_CLIENT_MGMT_PKT_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_GFP_TX_GFP_LOWER_CLIENT_MGMT_PKT_CNT(target)  VTSS_IOREG(target,0x509)

/** 
 * \brief
 * Lower client management packet count.
 * The lower client management packet count bits are used for the
 * accumulation of client management packets generated. The lower client
 * management packet count bits contain the lower 16 bits of the 32 bit Tx
 * GFP client management counter. This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_LOWER_CLIENT_MGMT_PKT_CNT . LOWER_CLIENT_MGMT_PKT_COUNT
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_LOWER_CLIENT_MGMT_PKT_CNT_LOWER_CLIENT_MGMT_PKT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_LOWER_CLIENT_MGMT_PKT_CNT_LOWER_CLIENT_MGMT_PKT_COUNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_LOWER_CLIENT_MGMT_PKT_CNT_LOWER_CLIENT_MGMT_PKT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Tx GFP upper client data packet counter
 *
 * \details
 * Register: \a GFP::GFPM_TX_GFP::TX_GFP_UPPER_CLIENT_DATA_PKT_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_GFP_TX_GFP_UPPER_CLIENT_DATA_PKT_CNT(target)  VTSS_IOREG(target,0x50a)

/** 
 * \brief
 * Upper client data frame count.
 * The upper client data packet count bits are used for the accumulation of
 * client data packets generated. The upper client data packet count bits
 * contain the upper 16 bits of the 32 bit Tx GFP client data packet
 * counter. This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_UPPER_CLIENT_DATA_PKT_CNT . UPPER_CLIENT_DATA_FRAME_COUNT
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_UPPER_CLIENT_DATA_PKT_CNT_UPPER_CLIENT_DATA_FRAME_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_UPPER_CLIENT_DATA_PKT_CNT_UPPER_CLIENT_DATA_FRAME_COUNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_UPPER_CLIENT_DATA_PKT_CNT_UPPER_CLIENT_DATA_FRAME_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Tx GFP lower client data packet counter
 *
 * \details
 * Register: \a GFP::GFPM_TX_GFP::TX_GFP_LOWER_CLIENT_DATA_PKT_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_GFP_TX_GFP_LOWER_CLIENT_DATA_PKT_CNT(target)  VTSS_IOREG(target,0x50b)

/** 
 * \brief
 * Lower client data frame count.
 * The lower client data packet count bits are used for the accumulation of
 * client data packets generated. The lower client data packet count bits
 * contain the lower 16 bits of the 32 bit Tx GFP client data packet
 * counter. This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_LOWER_CLIENT_DATA_PKT_CNT . LOWER_CLIENT_DATA_FRAME_COUNT
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_LOWER_CLIENT_DATA_PKT_CNT_LOWER_CLIENT_DATA_FRAME_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_LOWER_CLIENT_DATA_PKT_CNT_LOWER_CLIENT_DATA_FRAME_COUNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_LOWER_CLIENT_DATA_PKT_CNT_LOWER_CLIENT_DATA_FRAME_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Tx GFP client management packet length register
 *
 * \details
 * Register: \a GFP::GFPM_TX_GFP::TX_GFP_CLIENT_MGMT_PKT_LENGTH
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_PKT_LENGTH(target)  VTSS_IOREG(target,0x50c)

/** 
 * \brief
 * Client management packet length.
 * The client management packet length bits are used to indicate the length
 * of automatically generated client management frames and the
 * corresponding number of bytes used in the Tx GFP client management
 * packet memory.
 * NOTE: If the client management packet length bits are programmed to a
 * value that exceeds the number of bytes contained in the Tx GFP client
 * management packet memory, the maximum number of Tx GFP client management
 * packet memory bytes will be used for generated client management frames.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_PKT_LENGTH . CLIENT_MGMT_PKT_LENGTH
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_PKT_LENGTH_CLIENT_MGMT_PKT_LENGTH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_PKT_LENGTH_CLIENT_MGMT_PKT_LENGTH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_PKT_LENGTH_CLIENT_MGMT_PKT_LENGTH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Tx GFP client management upper interval timer control register
 *
 * \details
 * Register: \a GFP::GFPM_TX_GFP::TX_GFP_CLIENT_MGMT_UPPER_INTERVAL
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_UPPER_INTERVAL(target)  VTSS_IOREG(target,0x50d)

/** 
 * \brief
 * Upper timer count.
 * The upper timer count bits are used to support multiple line clock
 * applications to ensure accurate client management interval timer values.
 * The upper timer count bits contain the upper 8 bits of the 24 bit Tx GFP
 * client management interval timer count.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_UPPER_INTERVAL . UPPER_TIMER_COUNT
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_UPPER_INTERVAL_UPPER_TIMER_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_UPPER_INTERVAL_UPPER_TIMER_COUNT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_UPPER_INTERVAL_UPPER_TIMER_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief GFPM Tx GFP client management lower interval timer control register
 *
 * \details
 * Register: \a GFP::GFPM_TX_GFP::TX_GFP_CLIENT_MGMT_LOWER_INTERVAL
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_LOWER_INTERVAL(target)  VTSS_IOREG(target,0x50e)

/** 
 * \brief
 * Lower timer count.
 * The lower timer count bits are used to support multiple line clock
 * applications to ensure accurate client management interval timer values.
 * The lower timer count bits contain the lower 16 bits of the 24 bit Tx
 * GFP client management interval timer count.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_LOWER_INTERVAL . LOWER_TIMER_COUNT
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_LOWER_INTERVAL_LOWER_TIMER_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_LOWER_INTERVAL_LOWER_TIMER_COUNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_LOWER_INTERVAL_LOWER_TIMER_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Tx GFP client management packet memory
 *
 * \details
 * Register: \a GFP::GFPM_TX_GFP::TX_GFP_CLIENT_MGMT_PKT_MEMORY
 *
 * @param target A \a ::vtss_target_GFP_e target
 * @param ri Register: TX_GFP_CLIENT_MGMT_PKT_MEMORY (??), 0-63
 */
#define VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_PKT_MEMORY(target,ri)  VTSS_IOREG(target,0x5c0 + (ri))

/** 
 * \brief
 * Client management packet value.
 * The client management packet value bits contain the packet value
 * inserted for automatically inserted client management frames. The number
 * of bytes used from this memory for client management frames is
 * programmed through the client management packet length bits of the Tx
 * GFP client management packet length register.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_PKT_MEMORY . CLIENT_MGMT_PKT_VALUE
 */
#define  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_PKT_MEMORY_CLIENT_MGMT_PKT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_PKT_MEMORY_CLIENT_MGMT_PKT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_PKT_MEMORY_CLIENT_MGMT_PKT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a GFP::GFPM_RX_GLOBAL
 *
 * GFP mapper Rx global registers
 */


/** 
 * \brief GFPM Rx global control register
 *
 * \details
 * Register: \a GFP::GFPM_RX_GLOBAL::RX_GLOBAL_CONTROL
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL(target)  VTSS_IOREG(target,0x600)

/** 
 * \brief
 * Mapper mode.
 * The mapper mode bits are used to configure the GFP mapper mode of
 * operation.
 *
 * \details 
 * 00: 8 Gigabit Fibre Channel
 * 01: Reserved
 * 10: 10 Gigabit Ethernet Packet (G.Sup43.6.2)
 * 11: 10 Gigabit Ethernet PP/OS (G.Sup43.7.3)
 *
 * Field: ::VTSS_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL . MAPPER_MODE
 */
#define  VTSS_F_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_MAPPER_MODE(x)  VTSS_ENCODE_BITFIELD(x,14,2)
#define  VTSS_M_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_MAPPER_MODE     VTSS_ENCODE_BITMASK(14,2)
#define  VTSS_X_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_MAPPER_MODE(x)  VTSS_EXTRACT_BITFIELD(x,14,2)

/** 
 * \brief
 * FIFO reset.
 * If the FIFO reset bit is set, the core will reset the receive processor
 * FIFO read pointer. When the FIFO is reset, the read pointer (system side
 * of FIFO) address is reset to a value that is a distance (set through
 * design package) typically equal to half the depth of the FIFO away from
 * the write pointer. This bit is auto cleared when set.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL . FIFO_RESET
 */
#define  VTSS_F_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_FIFO_RESET(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_FIFO_RESET     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_FIFO_RESET(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * SW reset.
 * If the SW reset bit is set, the GFP Rx processor will perform a
 * synchronous reset operation. This state is equivalent to an external
 * hardware reset of the device.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL . SW_RESET
 */
#define  VTSS_F_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_SW_RESET(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_SW_RESET     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_SW_RESET(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Rx terminal loopback.
 * If the Rx terminal loopback bit is set, incoming receive data is looped
 * back to the transmit line side interface after GFP and FIFO processing.
 * NOTE: This bit is valid for all modes of operation except 10 Gigabit
 * Ethernet packet mode.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL . RX_TERMINAL_LOOPBACK
 */
#define  VTSS_F_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_RX_TERMINAL_LOOPBACK(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_RX_TERMINAL_LOOPBACK     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_GFP_GFPM_RX_GLOBAL_RX_GLOBAL_CONTROL_RX_TERMINAL_LOOPBACK(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief GFPM Rx FIFO write high threshold register
 *
 * \details
 * Register: \a GFP::GFPM_RX_GLOBAL::RX_FIFO_WRITE_HIGH_THRESHOLD
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GLOBAL_RX_FIFO_WRITE_HIGH_THRESHOLD(target)  VTSS_IOREG(target,0x601)

/** 
 * \brief
 * FIFO write high threshold value.
 * If the difference between the Rx FIFO write address (system side) and
 * the Rx FIFO read address (line side) is more than the value programmed
 * into the FIFO write high threshold value bits, a FIFO write high
 * threshold interrupt will be asserted.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GLOBAL_RX_FIFO_WRITE_HIGH_THRESHOLD . FIFO_WRITE_HIGH_THRESHOLD_VALUE
 */
#define  VTSS_F_GFP_GFPM_RX_GLOBAL_RX_FIFO_WRITE_HIGH_THRESHOLD_FIFO_WRITE_HIGH_THRESHOLD_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_GLOBAL_RX_FIFO_WRITE_HIGH_THRESHOLD_FIFO_WRITE_HIGH_THRESHOLD_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_GLOBAL_RX_FIFO_WRITE_HIGH_THRESHOLD_FIFO_WRITE_HIGH_THRESHOLD_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx FIFO write low threshold register
 *
 * \details
 * Register: \a GFP::GFPM_RX_GLOBAL::RX_FIFO_WRITE_LOW_THRESHOLD
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GLOBAL_RX_FIFO_WRITE_LOW_THRESHOLD(target)  VTSS_IOREG(target,0x602)

/** 
 * \brief
 * FIFO write low threshold value.
 * If the difference between the Rx FIFO write address (system side) and
 * the Rx FIFO read address (line side) is less than the value programmed
 * into the FIFO write low threshold value bits, a FIFO write low threshold
 * interrupt will be asserted.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GLOBAL_RX_FIFO_WRITE_LOW_THRESHOLD . FIFO_WRITE_LOW_THRESHOLD_VALUE
 */
#define  VTSS_F_GFP_GFPM_RX_GLOBAL_RX_FIFO_WRITE_LOW_THRESHOLD_FIFO_WRITE_LOW_THRESHOLD_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_GLOBAL_RX_FIFO_WRITE_LOW_THRESHOLD_FIFO_WRITE_LOW_THRESHOLD_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_GLOBAL_RX_FIFO_WRITE_LOW_THRESHOLD_FIFO_WRITE_LOW_THRESHOLD_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx FIFO read high threshold register
 *
 * \details
 * Register: \a GFP::GFPM_RX_GLOBAL::RX_FIFO_READ_HIGH_THRESHOLD
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GLOBAL_RX_FIFO_READ_HIGH_THRESHOLD(target)  VTSS_IOREG(target,0x603)

/** 
 * \brief
 * FIFO read high threshold value.
 * If the difference between the Rx FIFO write address (system side) and
 * the Rx FIFO read address (line side) is more than the value programmed
 * into the FIFO read high threshold value bits, a FIFO read high threshold
 * interrupt will be asserted. 
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GLOBAL_RX_FIFO_READ_HIGH_THRESHOLD . FIFO_READ_HIGH_THRESHOLD_VALUE
 */
#define  VTSS_F_GFP_GFPM_RX_GLOBAL_RX_FIFO_READ_HIGH_THRESHOLD_FIFO_READ_HIGH_THRESHOLD_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_GLOBAL_RX_FIFO_READ_HIGH_THRESHOLD_FIFO_READ_HIGH_THRESHOLD_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_GLOBAL_RX_FIFO_READ_HIGH_THRESHOLD_FIFO_READ_HIGH_THRESHOLD_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx FIFO read low threshold register
 *
 * \details
 * Register: \a GFP::GFPM_RX_GLOBAL::RX_FIFO_READ_LOW_THRESHOLD
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GLOBAL_RX_FIFO_READ_LOW_THRESHOLD(target)  VTSS_IOREG(target,0x604)

/** 
 * \brief
 * FIFO read low threshold value
 * If the difference between the Rx FIFO write address (system side) and
 * the Rx FIFO read address (line side) is less than the value programmed
 * into the FIFO read low threshold value bits, a FIFO read low threshold
 * interrupt will be asserted.	 
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GLOBAL_RX_FIFO_READ_LOW_THRESHOLD . FIFO_READ_LOW_THRESHOLD_VALUE
 */
#define  VTSS_F_GFP_GFPM_RX_GLOBAL_RX_FIFO_READ_LOW_THRESHOLD_FIFO_READ_LOW_THRESHOLD_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_GLOBAL_RX_FIFO_READ_LOW_THRESHOLD_FIFO_READ_LOW_THRESHOLD_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_GLOBAL_RX_FIFO_READ_LOW_THRESHOLD_FIFO_READ_LOW_THRESHOLD_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a GFP::GFPM_RX_10GE_PPOS
 *
 * GFP mapper Rx 10GE PP/OS registers
 */


/** 
 * \brief GFPM Rx PP/OS global control register
 *
 * \details
 * Register: \a GFP::GFPM_RX_10GE_PPOS::RX_PPOS_GLOBAL_CONTROL
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL(target)  VTSS_IOREG(target,0x620)

/** 
 * \brief
 * Test mode.
 * If the test mode bit is set, test mode is enabled and incoming data is
 * dropped and replaced with idles. Incoming data is still interpreted but
 * is not retransmitted toward the client interface. This bit is cleared
 * for normal operation.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL . TEST_MODE
 */
#define  VTSS_F_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_TEST_MODE(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_TEST_MODE     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_TEST_MODE(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Append preamble.
 * If the append preamble bit is set, incoming data frames will append the
 * 7 byte preamble and 1 byte frame start delimiter to generated ethernet
 * frames. If the append preamble bit is cleared, it is assumed that
 * incoming ethernet frames already contain the preamble and start of frame
 * delimiter characters.
 * NOTE: The /S/ character will replace the first preamble byte for
 * generated frames.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL . APPEND_PREAMBLE
 */
#define  VTSS_F_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_APPEND_PREAMBLE(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_APPEND_PREAMBLE     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_APPEND_PREAMBLE(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Minimum IFG value.
 * The minimum IFG value bits are used to set the minimum inter frame gap
 * between outgoing data frames. The minimum IFG value is measured in
 * bytes.
 * NOTE: This is only a minimum value; the actual number of idles inserted
 * may actually exceed this value.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL . MIN_IFG_VALUE
 */
#define  VTSS_F_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_MIN_IFG_VALUE(x)  VTSS_ENCODE_BITFIELD(x,8,5)
#define  VTSS_M_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_MIN_IFG_VALUE     VTSS_ENCODE_BITMASK(8,5)
#define  VTSS_X_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_MIN_IFG_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,8,5)

/** 
 * \brief
 * Err sync mode.
 * If the err sync mode bit is set, all internal counters will be captured
 * on the rising edge of the incoming ERR SYNC IN internal input
 * (accessible as PMTICK) and transferred to their respective shadow memory
 * locations for user access (user read operations). In the err sync mode
 * of operation, latch and clear functionality for all counters is still
 * valid if the user wants the updated count between error sync intervals.
 * If the err sync mode bit is cleared, counter latch and clear mode is
 * enabled and internal counters are captured and made available for the
 * user to read only when the designated counter latch and clear bit is
 * asserted. Internal counters are always cleared when counts are captured
 * and transferred through either err sync or latch and clear modes of
 * operation. 
 * NOTE: The minimum pulse width for the ERR SYNC IN (PMTICK) signal is two
 * register interface clocks.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL . ERR_SYNC_MODE
 */
#define  VTSS_F_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_ERR_SYNC_MODE(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_ERR_SYNC_MODE     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_ERR_SYNC_MODE(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Normal data packet latch and clear.
 * If the normal data packet latch and clear bit is set, the core will
 * latch the current normal data packet count value and transfer it to the
 * normal data packet counter value bits of the Rx PP/OS normal data packet
 * counter. The Rx PP/OS normal data packet counter should not be read
 * after asserting the sync block error latch and clear bit until this bit
 * is auto cleared.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL . NORMAL_DATA_PKT_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_NORMAL_DATA_PKT_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_NORMAL_DATA_PKT_LAT_CLR     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_NORMAL_DATA_PKT_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Sequence OS latch and clear.
 * If the sequence OS latch and clear bit is set, the core will latch the
 * current sequence ordered set count value and transfer it to the sequence
 * ordered set counter value bits of the Rx PP/OS sequence ordered set
 * counter. The Rx PP/OS sequence ordered set counter should not be read
 * after asserting the sequence OS latch and clear bit until this bit is
 * auto cleared.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL . SEQUENCE_OS_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_SEQUENCE_OS_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_SEQUENCE_OS_LAT_CLR     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_SEQUENCE_OS_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Signal OS latch and clear.
 * If the signal OS latch and clear bit is set, the core will latch the
 * current signal ordered set count value and transfer it to the signal
 * ordered set counter value bits of the Rx PP/OS signal ordered set
 * counter. The Rx PP/OS signal ordered set counter should not be read
 * after asserting the signal OS latch and clear bit until this bit is auto
 * cleared.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL . SIGNAL_OS_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_SIGNAL_OS_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_SIGNAL_OS_LAT_CLR     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_SIGNAL_OS_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Error latch and clear.
 * If the error latch and clear bit is set, the core will latch the error
 * count value and transfer it to the error counter value bits of the Rx
 * PP/OS error counter. The Rx PP/OS error counter should not be read after
 * asserting the error latch and clear bit until this bit is auto cleared.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL . ERROR_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_ERROR_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_ERROR_LAT_CLR     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_ERROR_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Block OS decode.
 * If the block OS decode bit is set, incoming signal or sequence ordered
 * sets will be blocked and not passed to the XGMII interface. If the block
 * OS decode bit is cleared, incoming signal or sequence ordered sets will
 * be decoded and sent to the XGMII interface.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL . BLOCK_OS_DECODE
 */
#define  VTSS_F_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_BLOCK_OS_DECODE(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_BLOCK_OS_DECODE     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_BLOCK_OS_DECODE(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Force idles SSF.
 * If the force idles SSF bit is set, idles are continuously generated
 * toward the client side interface when an incoming SSF condition is
 * detected from the GFP receive processor. This bit has priority over the
 * force OS SSF bit if both are set. 
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL . FORCE_IDLES_SSF
 */
#define  VTSS_F_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_FORCE_IDLES_SSF(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_FORCE_IDLES_SSF     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_FORCE_IDLES_SSF(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Force OS SSF.
 * If the force OS SSF bit is set, ordered sets are continuously generated
 * toward the client side interface when an incoming SSF condition is
 * detected from the GFP receive processor. The generated ordered sets
 * generated are configured through programming of the Rx PP/OS SSF ordered
 * set 0 register and Rx PP/OS SSF ordered set 1 register. This force idles
 * SSF bit has priority over the force OS SSF bit if both are set.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL . FORCE_OS_SSF
 */
#define  VTSS_F_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_FORCE_OS_SSF(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_FORCE_OS_SSF     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_GLOBAL_CONTROL_FORCE_OS_SSF(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief GFPM Rx PP/OS UPI register
 *
 * \details
 * Register: \a GFP::GFPM_RX_10GE_PPOS::RX_PPOS_UPI
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPI(target)  VTSS_IOREG(target,0x621)

/** 
 * \brief
 * Data UPI value.
 * If the incoming GFP-F frame UPI value matches the data UPI value bits,
 * the incoming frame is accepted and processed as an Ethernet data frame.
 * If the incoming GFP-F frame UPI value does not match the data UPI value
 * bits or the ordered set UPI value bits, the incoming frame is dropped.
 * The data UPI value bits have a default value of 00010011 as recommended
 * by specification G.SUP43 section 7.3.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPI . DATA_UPI_VALUE
 */
#define  VTSS_F_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPI_DATA_UPI_VALUE(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPI_DATA_UPI_VALUE     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPI_DATA_UPI_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Ordered set UPI value.
 * If the incoming GFP-F frame UPI value matches the ordered set UPI value
 * bits, the incoming frame is accepted and processed as an Ethernet
 * ordered set. If the incoming GFP-F frame UPI value does not match the
 * data UPI value bits or the ordered set UPI value bits, the incoming
 * frame is dropped. The ordered set UPI value bits have a default value of
 * 0001 0100 as recommended by specification G.SUP43 section 7.3.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPI . ORDERED_SET_UPI_VALUE
 */
#define  VTSS_F_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPI_ORDERED_SET_UPI_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPI_ORDERED_SET_UPI_VALUE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPI_ORDERED_SET_UPI_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief GFPM Rx PP/OS SSF ordered set 0 register
 *
 * \details
 * Register: \a GFP::GFPM_RX_10GE_PPOS::RX_PPOS_SSF_ORDERED_SET_0
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_SSF_ORDERED_SET_0(target)  VTSS_IOREG(target,0x622)

/** 
 * \brief
 * Ordered set lane 0.
 * The ordered set lane 0 bits are used for ordered set generation when the
 * force OS SSF bit of the Rx PP/OS global control register is asserted and
 * an incoming SSF condition is detected.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_SSF_ORDERED_SET_0 . ORDERED_SET_LANE_0
 */
#define  VTSS_F_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_SSF_ORDERED_SET_0_ORDERED_SET_LANE_0(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_SSF_ORDERED_SET_0_ORDERED_SET_LANE_0     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_SSF_ORDERED_SET_0_ORDERED_SET_LANE_0(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Ordered set lane 1.
 * The ordered set lane 1 bits are used for ordered set generation when the
 * force OS SSF bit of the Rx PP/OS global control register is asserted and
 * an incoming SSF condition is detected.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_SSF_ORDERED_SET_0 . ORDERED_SET_LANE_1
 */
#define  VTSS_F_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_SSF_ORDERED_SET_0_ORDERED_SET_LANE_1(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_SSF_ORDERED_SET_0_ORDERED_SET_LANE_1     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_SSF_ORDERED_SET_0_ORDERED_SET_LANE_1(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief GFPM Rx PP/OS SSF ordered set 1 register
 *
 * \details
 * Register: \a GFP::GFPM_RX_10GE_PPOS::RX_PPOS_SSF_ORDERED_SET_1
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_SSF_ORDERED_SET_1(target)  VTSS_IOREG(target,0x623)

/** 
 * \brief
 * Ordered set lane 2.
 * The ordered set lane 2 bits are used for ordered set generation when the
 * force OS SSF bit of the Rx PP/OS global control register is asserted and
 * an incoming SSF condition is detected.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_SSF_ORDERED_SET_1 . ORDERED_SET_LANE_2
 */
#define  VTSS_F_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_SSF_ORDERED_SET_1_ORDERED_SET_LANE_2(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_SSF_ORDERED_SET_1_ORDERED_SET_LANE_2     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_SSF_ORDERED_SET_1_ORDERED_SET_LANE_2(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Ordered set lane 3.
 * The ordered set lane 3 bits are used for ordered set generation when the
 * force OS SSF bit of the Rx PP/OS global control register is asserted and
 * an incoming SSF condition is detected.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_SSF_ORDERED_SET_1 . ORDERED_SET_LANE_3
 */
#define  VTSS_F_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_SSF_ORDERED_SET_1_ORDERED_SET_LANE_3(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_SSF_ORDERED_SET_1_ORDERED_SET_LANE_3     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_SSF_ORDERED_SET_1_ORDERED_SET_LANE_3(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief GFPM Rx PP/OS upper normal data packet counter
 *
 * \details
 * Register: \a GFP::GFPM_RX_10GE_PPOS::RX_PPOS_UPPER_NORMAL_DATA_PKT_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPPER_NORMAL_DATA_PKT_CNT(target)  VTSS_IOREG(target,0x624)

/** 
 * \brief
 * Upper normal data packet counter value.
 * The upper normal data packet counter value bits are used to accumulate
 * the number incoming valid 10GE normal data packets received. A normal
 * data packet is counted when the received GFP-F UPI value matches the
 * data UPI value bits of the Rx PP/OS UPI register with valid SOP and EOP
 * indicators without error detection. For more information, see error
 * conditions described in Rx PP/OS error counter. The upper normal data
 * packet counter value bits contain the upper 16 bits of the 32 bit Rx
 * PP/OS normal data packet counter. This is a saturating read only
 * counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPPER_NORMAL_DATA_PKT_CNT . UPPER_NORMAL_DATA_PKT_CNT_VALUE
 */
#define  VTSS_F_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPPER_NORMAL_DATA_PKT_CNT_UPPER_NORMAL_DATA_PKT_CNT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPPER_NORMAL_DATA_PKT_CNT_UPPER_NORMAL_DATA_PKT_CNT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPPER_NORMAL_DATA_PKT_CNT_UPPER_NORMAL_DATA_PKT_CNT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx PP/OS lower normal data packet counter
 *
 * \details
 * Register: \a GFP::GFPM_RX_10GE_PPOS::RX_PPOS_LOWER_NORMAL_DATA_PKT_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_LOWER_NORMAL_DATA_PKT_CNT(target)  VTSS_IOREG(target,0x625)

/** 
 * \brief
 * Lower normal data packet counter value.
 * The lower normal data packet counter value bits are used to accumulate
 * the number incoming valid 10GE normal data packets received. A normal
 * data packet is counted when the received GFP-F UPI value matches the
 * data UPI value bits of the Rx PP/OS UPI register with valid SOP and EOP
 * indicators without error detection. For more information, see error
 * conditions described in Rx PP/OS error counter. The lower normal data
 * packet counter value bits contain the lower 16 bits of the 32 bit Rx
 * PP/OS normal data packet counter. This is a saturating read only
 * counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_LOWER_NORMAL_DATA_PKT_CNT . LOWER_NORMAL_DATA_PKT_CNT_VALUE
 */
#define  VTSS_F_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_LOWER_NORMAL_DATA_PKT_CNT_LOWER_NORMAL_DATA_PKT_CNT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_LOWER_NORMAL_DATA_PKT_CNT_LOWER_NORMAL_DATA_PKT_CNT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_LOWER_NORMAL_DATA_PKT_CNT_LOWER_NORMAL_DATA_PKT_CNT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx PP/OS upper sequence ordered set counter
 *
 * \details
 * Register: \a GFP::GFPM_RX_10GE_PPOS::RX_PPOS_UPPER_SEQ_ORD_SET_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPPER_SEQ_ORD_SET_CNT(target)  VTSS_IOREG(target,0x626)

/** 
 * \brief
 * Upper sequence ordered set counter value.
 * The upper sequence ordered set counter value bits are used to accumulate
 * the number valid incoming sequence ordered sets received. A valid
 * sequence ordered set is counted when the received GFP-F UPI value
 * matches the ordered set UPI value bits of the Rx PP/OS UPI register and
 * an O code value of 0000 is detected. The upper sequence ordered set
 * counter value bits contain the upper 16 bits of the 32 bit Rx PP/OS
 * sequence ordered set counter. This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPPER_SEQ_ORD_SET_CNT . UPPER_SEQ_ORD_SET_CNT_VALUE
 */
#define  VTSS_F_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPPER_SEQ_ORD_SET_CNT_UPPER_SEQ_ORD_SET_CNT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPPER_SEQ_ORD_SET_CNT_UPPER_SEQ_ORD_SET_CNT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPPER_SEQ_ORD_SET_CNT_UPPER_SEQ_ORD_SET_CNT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx PP/OS lower sequence ordered set counter
 *
 * \details
 * Register: \a GFP::GFPM_RX_10GE_PPOS::RX_PPOS_LOWER_SEQ_ORD_SET_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_LOWER_SEQ_ORD_SET_CNT(target)  VTSS_IOREG(target,0x627)

/** 
 * \brief
 * Lower sequence ordered set counter value.
 * The lower sequence ordered set counter value bits are used to accumulate
 * the number valid incoming sequence ordered sets received. A valid
 * sequence ordered set is counted when the received GFP-F UPI value
 * matches the ordered set UPI value bits of the Rx PP/OS UPI register and
 * an O code value of 0000 is detected. The lower sequence ordered set
 * counter value bits contain the lower 16 bits of the 32 bit Rx PP/OS
 * sequence ordered set counter. This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_LOWER_SEQ_ORD_SET_CNT . LOWER_SEQ_ORD_SET_CNT_VALUE
 */
#define  VTSS_F_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_LOWER_SEQ_ORD_SET_CNT_LOWER_SEQ_ORD_SET_CNT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_LOWER_SEQ_ORD_SET_CNT_LOWER_SEQ_ORD_SET_CNT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_LOWER_SEQ_ORD_SET_CNT_LOWER_SEQ_ORD_SET_CNT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx PP/OS upper signal ordered set counter
 *
 * \details
 * Register: \a GFP::GFPM_RX_10GE_PPOS::RX_PPOS_UPPER_SIG_ORD_SET_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPPER_SIG_ORD_SET_CNT(target)  VTSS_IOREG(target,0x628)

/** 
 * \brief
 * Upper signal ordered set counter value.
 * The upper signal ordered set counter value bits are used to accumulate
 * the number valid incoming signal ordered sets received. A valid signal
 * ordered set is counted when the received GFP-F UPI value matches the
 * ordered set UPI value bits of the Rx PP/OS UPI register and an O code
 * value of 1111 is detected. The upper signal ordered set counter value
 * bits contain the upper 16 bits of the 32 bit Rx PP/OS signal ordered set
 * counter. This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPPER_SIG_ORD_SET_CNT . UPPER_SIG_ORD_SET_CNT_VALUE
 */
#define  VTSS_F_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPPER_SIG_ORD_SET_CNT_UPPER_SIG_ORD_SET_CNT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPPER_SIG_ORD_SET_CNT_UPPER_SIG_ORD_SET_CNT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPPER_SIG_ORD_SET_CNT_UPPER_SIG_ORD_SET_CNT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx PP/OS lower signal ordered set counter
 *
 * \details
 * Register: \a GFP::GFPM_RX_10GE_PPOS::RX_PPOS_LOWER_SIG_ORD_SET_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_LOWER_SIG_ORD_SET_CNT(target)  VTSS_IOREG(target,0x629)

/** 
 * \brief
 * Lower signal ordered set counter value.
 * The lower signal ordered set counter value bits are used to accumulate
 * the number valid incoming signal ordered sets received. A valid signal
 * ordered set is counted when the received GFP-F UPI value matches the
 * ordered set UPI value bits of the Rx PP/OS UPI register and an O code
 * value of 1111 is detected. The lower signal ordered set counter value
 * bits contain the lower 16 bits of the 32 bit Rx PP/OS signal ordered set
 * counter. This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_LOWER_SIG_ORD_SET_CNT . LOWER_SIG_ORD_SET_CNT_VALUE
 */
#define  VTSS_F_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_LOWER_SIG_ORD_SET_CNT_LOWER_SIG_ORD_SET_CNT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_LOWER_SIG_ORD_SET_CNT_LOWER_SIG_ORD_SET_CNT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_LOWER_SIG_ORD_SET_CNT_LOWER_SIG_ORD_SET_CNT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx PP/OS error counter
 *
 * \details
 * Register: \a GFP::GFPM_RX_10GE_PPOS::RX_PPOS_ERROR_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_ERROR_CNT(target)  VTSS_IOREG(target,0x62a)

/** 
 * \brief
 * Error data packet counter value.
 * The error data packet counter value bits are used to accumulate the
 * number incoming GFP errors received. An error data packet is counted
 * when any of the following conditions occur:
 *  1. An incoming GFP packet invalid error is detected
 *  2. An incoming unrecognized GFP UPI value is detected (occurs when the
 * incoming UPI does not match the programmed data UPI value or ordered set
 * UPI value)
 *  3. An incoming unrecognized ordered set O-code
 *  4. The incoming data packet minimum size is violated
 *  5. The incoming data packet maximum size is violated
 *  6. Incoming invalid packet size detected (due to FIFO error, etc.) 
 * This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_ERROR_CNT . ERROR_DATA_PKT_CNT_VALUE
 */
#define  VTSS_F_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_ERROR_CNT_ERROR_DATA_PKT_CNT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_ERROR_CNT_ERROR_DATA_PKT_CNT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_ERROR_CNT_ERROR_DATA_PKT_CNT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a GFP::GFPM_RX_8B10B_ENCODER
 *
 * GFP mapper Rx 8B/10B registers
 */


/** 
 * \brief GFPM Rx 8B/10B global control register
 *
 * \details
 * Register: \a GFP::GFPM_RX_8B10B_ENCODER::RX_8B10B_GLOBAL_CONTROL
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL(target)  VTSS_IOREG(target,0x680)

/** 
 * \brief
 * 10B_ERR map K30.7.
 * The 10B_ERR map K30.7 bit is set to indicate that incoming 10B_ERR
 * characters will be replaced with the K30.7 special character prior to 10
 * bit encoding. If the 10B_ERR map K30.7 bit is cleared, incoming 10B_ERR
 * characters will be encoded to the 10 bit (ITU G7041 section 8.1.1.1)
 * character of either 0011110001 (RD-) or 1100001110 (RD+) with neutral
 * disparity. These are encoded illegal 8B/10B codeword values with neutral
 * disparity. The 10B_ERR map K30.7 bit is set as a default value.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL . RX_10B_ERR_MAP_K30_7
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL_RX_10B_ERR_MAP_K30_7(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL_RX_10B_ERR_MAP_K30_7     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL_RX_10B_ERR_MAP_K30_7(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Minimum IFG value.
 * The minimum IFG value bits are used to indicate the minimum number of
 * idles (idle sequence =>K28.5-D21.4-D21.5-D21.5) that must occur before
 * the start of each frame (SOF).
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL . MIN_IFG_VALUE
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL_MIN_IFG_VALUE(x)  VTSS_ENCODE_BITFIELD(x,8,7)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL_MIN_IFG_VALUE     VTSS_ENCODE_BITMASK(8,7)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL_MIN_IFG_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,8,7)

/** 
 * \brief
 * Err sync mode.
 * If the err sync mode bit is set, all internal counters will be captured
 * on the rising edge of the incoming ERR SYNC IN internal input
 * (accessible as PMTICK) and transferred to their respective shadow memory
 * locations for user access (user read operations). In the err sync mode
 * of operation, latch and clear operation for all counters is still valid
 * if the user wants the updated count between error sync intervals. If the
 * err sync mode bit is cleared, counter latch and clear mode is enabled
 * and internal counters are captured and made available for the user to
 * read only when the designated counter latch and clear bit is asserted.
 * Internal counters are always cleared when counts are captured and
 * transferred through either err sync or latch and clear modes of
 * operation. 
 * Note: The minimum pulse width for the ERR SYNC IN (PMTICK) signal is 2
 * register interface clocks.	     
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL . ERR_SYNC_MODE
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL_ERR_SYNC_MODE(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL_ERR_SYNC_MODE     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL_ERR_SYNC_MODE(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * 10B_ERR latch and clear.
 * If the 10B_ERR latch and clear bit is set, the core will latch the
 * current 10B_ERR character count value and transfer it to the 10B_ERR
 * counter value bits of the Rx 8B/10B 10B_ERR Counter. The 10B_ERR
 * character counter value is available to be read until the next 10B_ERR
 * latch and clear event occurs. The Rx 8B/10B 10B_ERR counter should not
 * be read after asserting the 10B_ERR latch and clear bit until this bit
 * is auto cleared.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL . RX_10B_ERR_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL_RX_10B_ERR_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL_RX_10B_ERR_LAT_CLR     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL_RX_10B_ERR_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Invalid character latch and clear.
 * If the invalid character latch and clear bit is set, the core will latch
 * the current invalid character count value and transfer it to the invalid
 * character counter value bits of the Rx 8B/10B invalid character counter.
 * The invalid character counter value is available to be read until the
 * next invalid character latch and clear event occurs. The Rx 8B/10B
 * invalid character counter should not be read after asserting the invalid
 * character latch and clear bit until this bit is auto cleared.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL . INVALID_CHARACTER_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL_INVALID_CHARACTER_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL_INVALID_CHARACTER_LAT_CLR     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL_INVALID_CHARACTER_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Valid codeword latch and clear.
 * If the valid codeword latch and clear bit is set, the core will latch
 * the current valid codeword count value and transfer it to the valid
 * codeword counter value bits of the Rx 8B/10B valid codeword counter. The
 * valid codeword counter value is available to be read until the next
 * valid codeword latch and clear event occurs. The Rx 8B/10B valid
 * codeword counter should not be read after asserting the valid codeword
 * latch and clear bit until this bit is auto cleared.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL . VALID_CODEWORD_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL_VALID_CODEWORD_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL_VALID_CODEWORD_LAT_CLR     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL_VALID_CODEWORD_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Force K30.7 characters.
 * If the force K30.7 characters bit is set, the 8B/10B encoder will
 * generate continuous K30.7 characters. When this bit is cleared, normal
 * operation is enabled.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL . FORCE_K30_7_CHARACTERS
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL_FORCE_K30_7_CHARACTERS(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL_FORCE_K30_7_CHARACTERS     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL_FORCE_K30_7_CHARACTERS(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Enable rate adapt.
 * If the enable rate adapt bit is set, 8B/10B rate adaptation is enabled.
 * When this bit is cleared, rate adaptation is disabled.
 * NOTE: 8B/10B rate adaptation must be enabled to ensure correct 8 GFC
 * operation.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL . ENABLE_RATE_ADAPT
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL_ENABLE_RATE_ADAPT(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL_ENABLE_RATE_ADAPT     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_GLOBAL_CONTROL_ENABLE_RATE_ADAPT(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief GFPM Rx 8B/10B 10B_ERR counter
 *
 * \details
 * Register: \a GFP::GFPM_RX_8B10B_ENCODER::RX_8B10B_10B_ERR_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_10B_ERR_CNT(target)  VTSS_IOREG(target,0x683)

/** 
 * \brief
 * 10B_ERR counter value.
 * The 10B_ERR counter value bits are used to accumulate the number of
 * incoming 10B_ERR characters detected. This is a saturating read only
 * counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_10B_ERR_CNT . RX_10B_ERR_CNT_VALUE
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_10B_ERR_CNT_RX_10B_ERR_CNT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_10B_ERR_CNT_RX_10B_ERR_CNT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_10B_ERR_CNT_RX_10B_ERR_CNT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx 8B/10B invalid character counter
 *
 * \details
 * Register: \a GFP::GFPM_RX_8B10B_ENCODER::RX_8B10B_INVALID_CHARACTER_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_INVALID_CHARACTER_CNT(target)  VTSS_IOREG(target,0x684)

/** 
 * \brief
 * Invalid character counter value.
 * The invalid character counter value bits are used to accumulate the
 * number of incoming invalid characters detected. This is a saturating
 * read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_INVALID_CHARACTER_CNT . INVALID_CHARACTER_CNT_VALUE
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_INVALID_CHARACTER_CNT_INVALID_CHARACTER_CNT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_INVALID_CHARACTER_CNT_INVALID_CHARACTER_CNT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_INVALID_CHARACTER_CNT_INVALID_CHARACTER_CNT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx 8B/10B upper valid codeword counter
 *
 * \details
 * Register: \a GFP::GFPM_RX_8B10B_ENCODER::RX_8B10B_UPPER_VALID_CODEWORD_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_UPPER_VALID_CODEWORD_CNT(target)  VTSS_IOREG(target,0x685)

/** 
 * \brief
 * Upper valid codeword counter value.
 * The upper valid codeword counter value bits are used to accumulate the
 * number of valid codewords detected. The upper valid codeword counter
 * value bits contain the upper 8 bits of the 40 bit Rx 8B/10B valid
 * codeword counter. This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_UPPER_VALID_CODEWORD_CNT . UPPER_VALID_CODEWORD_CNT_VALUE
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_UPPER_VALID_CODEWORD_CNT_UPPER_VALID_CODEWORD_CNT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_UPPER_VALID_CODEWORD_CNT_UPPER_VALID_CODEWORD_CNT_VALUE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_UPPER_VALID_CODEWORD_CNT_UPPER_VALID_CODEWORD_CNT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief GFPM Rx 8B/10B mid valid codeword counter
 *
 * \details
 * Register: \a GFP::GFPM_RX_8B10B_ENCODER::RX_8B10B_MID_VALID_CODEWORD_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_MID_VALID_CODEWORD_CNT(target)  VTSS_IOREG(target,0x686)

/** 
 * \brief
 * Mid valid codeword counter value.
 * The mid valid codeword counter value bits are used to accumulate the
 * number of valid codewords detected. The mid valid codeword counter value
 * bits contain bits 31 down to 16 of the 40 bit Rx 8B/10B valid codeword
 * counter. This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_MID_VALID_CODEWORD_CNT . MID_VALID_CODEWORD_CNT_VALUE
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_MID_VALID_CODEWORD_CNT_MID_VALID_CODEWORD_CNT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_MID_VALID_CODEWORD_CNT_MID_VALID_CODEWORD_CNT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_MID_VALID_CODEWORD_CNT_MID_VALID_CODEWORD_CNT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx 8B/10B lower valid codeword counter
 *
 * \details
 * Register: \a GFP::GFPM_RX_8B10B_ENCODER::RX_8B10B_LOWER_VALID_CODEWORD_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_LOWER_VALID_CODEWORD_CNT(target)  VTSS_IOREG(target,0x687)

/** 
 * \brief
 * Lower valid codeword counter value.
 * The lower valid codeword counter value bits are used to accumulate the
 * number of valid codewords detected. The lower valid codeword counter
 * value bits contain the lower 16 bits of the 40 bit Rx 8B/10B valid
 * codeword counter. This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_LOWER_VALID_CODEWORD_CNT . LOWER_VALID_CODEWORD_CNT_VALUE
 */
#define  VTSS_F_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_LOWER_VALID_CODEWORD_CNT_LOWER_VALID_CODEWORD_CNT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_LOWER_VALID_CODEWORD_CNT_LOWER_VALID_CODEWORD_CNT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_8B10B_ENCODER_RX_8B10B_LOWER_VALID_CODEWORD_CNT_LOWER_VALID_CODEWORD_CNT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a GFP::GFPM_RX_GFP
 *
 * GFP mapper Rx GFP registers
 */


/** 
 * \brief GFPM Rx GFP control register
 *
 * \details
 * Register: \a GFP::GFPM_RX_GFP::RX_GFP_CONTROL
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GFP_RX_GFP_CONTROL(target)  VTSS_IOREG(target,0x700)

/** 
 * \brief
 * Core header descramble enable.
 * The core header descramble enable bit is set to enable core header PTI
 * and cHEC field descrambling. Core header descrambling implies the core
 * header is XOR'd with the barker like sequence value of hex B6AB31E0.   
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CONTROL . CORE_HDR_DESCRAMBLE_ENABLE
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_CORE_HDR_DESCRAMBLE_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_CORE_HDR_DESCRAMBLE_ENABLE     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_CORE_HDR_DESCRAMBLE_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Frame delinear delta count.
 * The frame delineator delta count bits are used to indicate how many
 * valid core header values are required for the delineator state machine
 * to transition from the PRESYNC to SYNC state.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CONTROL . FRAME_DELINEAR_DELTA_COUNT
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_FRAME_DELINEAR_DELTA_COUNT(x)  VTSS_ENCODE_BITFIELD(x,12,3)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_FRAME_DELINEAR_DELTA_COUNT     VTSS_ENCODE_BITMASK(12,3)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_FRAME_DELINEAR_DELTA_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,12,3)

/** 
 * \brief
 * Core header error correction enable.
 * The core header error correction enable bit is set to enable single bit
 * error correction of the GFP core header when the frame delineator is in
 * the SYNC state.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CONTROL . CORE_HDR_ERROR_CORRECT_ENABLE
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_CORE_HDR_ERROR_CORRECT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_CORE_HDR_ERROR_CORRECT_ENABLE     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_CORE_HDR_ERROR_CORRECT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Payload descramble enable.
 * The payload descramble enable bit is set to enable descrambling of the
 * GFP payload area when the frame delineator is in the SYNC state.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CONTROL . PYLD_DESCRAMBLE_ENABLE
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_PYLD_DESCRAMBLE_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_PYLD_DESCRAMBLE_ENABLE     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_PYLD_DESCRAMBLE_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Payload header error correction enable.
 * The payload header error correction enable bit is set to enable single
 * bit error correction of the GFP payload header when the frame delineator
 * is in the SYNC state.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CONTROL . PYLD_HDR_ERROR_CORRECT_ENABLE
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_PYLD_HDR_ERROR_CORRECT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_PYLD_HDR_ERROR_CORRECT_ENABLE     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_PYLD_HDR_ERROR_CORRECT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Transparent CRC-16 error correction enable.
 * The transparent CRC-16 error correction enable bit is set to enable
 * single bit error correction of the GFP transparent CRC-16 field when the
 * frame delineator is in the SYNC state.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CONTROL . TRANSP_CRC16_ERROR_CORRECT_ENABLE
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_TRANSP_CRC16_ERROR_CORRECT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_TRANSP_CRC16_ERROR_CORRECT_ENABLE     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_TRANSP_CRC16_ERROR_CORRECT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Client management frame filter count.
 * The client management frame filter count bits are used to determine the
 * number of identical client management frames that must be received
 * consecutively to update the client management capture packet value bits
 * of the Rx GFP client management capture packet memory. If the number of
 * consecutive client management frames received matches the client
 * management frame filter count, a client management frame accepted
 * interrupt is declared by setting the client management accept IRQ bit of
 * the Rx GFP client interrupt register.
 * NOTE: The client management accept overflow IRQ bit of the Rx GFP client
 * interrupt register will be set if the incoming accepted client
 * management frame length exceeds the length of the Rx GFP client
 * management capture packet memory.
 * The value of the client management accept packet length bits in the Rx
 * GFP client management accept packet length register is updated when an
 * accepted client management frame condition is detected to indicate the
 * length of the accepted client management frame accepted. The client
 * management frame filter count bits are similarly used for inconsistent
 * client management frame detection. The client management frame filter
 * count bits are used to determine the number of non identical client
 * management frames that must be received to declare a client management
 * frame inconsistent condition. A client management frame inconsistent
 * condition is indicated by setting the client management inconsistent IRQ
 * bit of the Rx GFP client interrupt register. 
 * NOTE: A value of 0 programmed for the client management frame filter
 * count bits will have the same effect as a programmed value of 1. If the
 * number of bytes contained in the incoming client management frame
 * exceeds the number of bytes available in the Rx GFP client management
 * capture packet memory, the incoming message will be treated as if it was
 * only as long as the available memory space.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CONTROL . CLIENT_MGMT_FRAME_FILTER_COUNT
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_CLIENT_MGMT_FRAME_FILTER_COUNT(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_CLIENT_MGMT_FRAME_FILTER_COUNT     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_CLIENT_MGMT_FRAME_FILTER_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,5,3)


/** 
 * \brief GFPM Rx GFP counter control register
 *
 * \details
 * Register: \a GFP::GFPM_RX_GFP::RX_GFP_CNT_CONTROL
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL(target)  VTSS_IOREG(target,0x701)

/** 
 * \brief
 * Err sync mode.
 * If the err sync mode bit is set, all internal counters will be captured
 * on the rising edge of the incoming ERR SYNC IN internal input
 * (accessible as PMTICK) and transferred to their respective shadow memory
 * locations for user access (user read operations). In the err sync mode
 * of operation, latch and clear functionality for all counters is still
 * valid if the user wants the updated count between error sync intervals.
 * If the err sync mode bit is cleared, counter latch and clear mode is
 * enabled and internal counters are captured and made available for the
 * user to read only when the designated counter latch and clear bit is
 * asserted. Internal counters are always cleared when counts are captured
 * and transferred through either err sync or latch and clear modes of
 * operation. 
 * NOTE: The minimum pulse width for the ERR SYNC IN (PMTICK) signal is two
 * register interface clocks.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL . ERR_SYNC_MODE
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_ERR_SYNC_MODE(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_ERR_SYNC_MODE     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_ERR_SYNC_MODE(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Sync counter latch and clear.
 * If the sync counter latch and clear bit is set, the GFP receive
 * processor will latch the current internal sync error count value and
 * transfer it into the sync error count bits of the Rx GFP sync error
 * counter register. The internal sync error count register will then be
 * cleared. The Rx GFP sync error counter register contents are available
 * to be read until the next latch and clear event occurs.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL . SYNC_CNT_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_SYNC_CNT_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_SYNC_CNT_LAT_CLR     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_SYNC_CNT_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Core header corrected counter latch and clear.
 * If the core header corrected counter latch and clear bit is set, the GFP
 * receive processor will latch the current internal core header corrected
 * count value and transfer it into the core header corrected count bits of
 * the Rx GFP core header corrected error counter register. The internal
 * core header corrected count register will then be cleared. The Rx GFP
 * core header corrected counter register contents are available to be read
 * until the next latch and clear event occurs.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL . CORE_HDR_COR_CNT_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_CORE_HDR_COR_CNT_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_CORE_HDR_COR_CNT_LAT_CLR     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_CORE_HDR_COR_CNT_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Payload header corrected counter latch and clear.
 * If the payload header corrected counter latch and clear bit is set, the
 * GFP receive processor will latch the current internal payload corrected
 * count value and transfer it into the payload header corrected count bits
 * of the Rx GFP payload header corrected counter register. The internal
 * payload header corrected count register will then be cleared. The Rx GFP
 * payload header corrected counter register contents are available to be
 * read until the next latch and clear event occurs.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL . PYLD_HDR_COR_CNT_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_PYLD_HDR_COR_CNT_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_PYLD_HDR_COR_CNT_LAT_CLR     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_PYLD_HDR_COR_CNT_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Payload header uncorrected counter latch and clear.
 * If the payload header uncorrected counter latch and clear bit is set,
 * the GFP receive processor will latch the current internal payload
 * uncorrected count value and transfer it into the payload header
 * uncorrected count bits of the Rx GFP payload header uncorrected counter
 * register. The internal payload header uncorrected count register will
 * then be cleared. The Rx GFP payload header uncorrected counter register
 * contents are available to be read until the next latch and clear event
 * occurs.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL . PYLD_HDR_UNCOR_CNT_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_PYLD_HDR_UNCOR_CNT_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_PYLD_HDR_UNCOR_CNT_LAT_CLR     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_PYLD_HDR_UNCOR_CNT_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Transparent CRC-16 corrected counter latch and clear.
 * If the transparent CRC-16 corrected counter latch and clear bit is set,
 * the GFP receive processor will latch the current internal transparent
 * CRC-16 corrected count value and transfer it into the transparent CRC-16
 * corrected count bits of the Rx GFP transparent CRC-16 corrected counter
 * register. The internal transparent CRC-16 corrected count register will
 * then be cleared. The Rx GFP transparent CRC-16 corrected counter
 * register contents are available to be read until the next latch and
 * clear event occurs.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL . TRANSP_CRC16_COR_CNT_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_TRANSP_CRC16_COR_CNT_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_TRANSP_CRC16_COR_CNT_LAT_CLR     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_TRANSP_CRC16_COR_CNT_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Transparent CRC-16 uncorrected counter latch and clear.
 * If the transparent CRC-16 uncorrected counter latch and clear bit is
 * set, the GFP receive processor will latch the current internal
 * transparent CRC-16 uncorrected count value and transfer it into the
 * transparent CRC-16 uncorrected count bits of the RX GFP transparent
 * CRC-16 uncorrected counter register. The internal transparent CRC-16
 * uncorrected count register will then be cleared. The RX GFP transparent
 * CRC-16 uncorrected counter register contents are available to be read
 * until the next latch and clear event occurs.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL . TRANSP_CRC16_UNCOR_CNT_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_TRANSP_CRC16_UNCOR_CNT_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_TRANSP_CRC16_UNCOR_CNT_LAT_CLR     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_TRANSP_CRC16_UNCOR_CNT_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Control frame counter latch and clear.
 * If the control frame counter latch and clear bit is set, the GFP receive
 * processor will latch the current internal control frame count value and
 * transfer it into the control frame count bits of the Rx GFP control
 * frame counter register. The internal control frame count register will
 * then be cleared. The Rx GFP control frame counter register contents are
 * available to be read until the next latch and clear event occurs.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL . CONTROL_FRAME_CNT_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_CONTROL_FRAME_CNT_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_CONTROL_FRAME_CNT_LAT_CLR     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_CONTROL_FRAME_CNT_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Client management packet counter latch and clear.
 * If the client management packet counter latch and clear bit is set, the
 * GFP receive processor will latch the current client management packet
 * count value and transfer it into the client management packet count bits
 * of the Rx GFP client management packet counter register. The internal
 * client management packet count register will then be cleared. The Rx GFP
 * client management packet counter register contents are available to be
 * read until the next latch and clear event occurs.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL . CLIENT_MGMT_PKT_CNT_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_CLIENT_MGMT_PKT_CNT_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_CLIENT_MGMT_PKT_CNT_LAT_CLR     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_CLIENT_MGMT_PKT_CNT_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Client data packet counter latch and clear.
 * If the client data packet counter latch and clear bit is set, the GFP
 * receive processor will latch the current client data frame count value
 * and transfer it into the client data packet count bits of the Rx GFP
 * client data packet counter register. The internal client data packet
 * count register will then be cleared. The Rx GFP client data packet
 * counter register contents are available to be read until the next latch
 * and clear event occurs.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL . CLIENT_DATA_PKT_CNT_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_CLIENT_DATA_PKT_CNT_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_CLIENT_DATA_PKT_CNT_LAT_CLR     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_CLIENT_DATA_PKT_CNT_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Client invalid packet counter latch and clear.
 * If the client invalid packet counter latch and clear bit is set, the GFP
 * receive processor will latch the current client invalid packet count
 * value and transfer it into the client invalid packet count bits of the
 * Rx GFP client invalid packet counter register. The internal client
 * invalid packet count register will then be cleared. The Rx GFP client
 * invalid packet counter register contents are available to be read until
 * the next latch and clear event occurs.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL . CLIENT_INVALID_PKT_CNT_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_CLIENT_INVALID_PKT_CNT_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_CLIENT_INVALID_PKT_CNT_LAT_CLR     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_CLIENT_INVALID_PKT_CNT_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * FCS counter latch and clear.
 * If the FCS counter latch and clear bit is set, the GFP receive processor
 * will latch the current internal FCS error count value and transfer it
 * into the FCS error count bits of the Rx GFP FCS error counter register.
 * The internal FCS error count register will then be cleared. The Rx GFP
 * FCS error counter register contents are available to be read until the
 * next latch and clear event occurs.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL . FCS_CNT_LAT_CLR
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_FCS_CNT_LAT_CLR(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_FCS_CNT_LAT_CLR     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CNT_CONTROL_FCS_CNT_LAT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief GFPM Rx GFP test frame expect register
 *
 * \details
 * Register: \a GFP::GFPM_RX_GFP::RX_GFP_TEST_FRAME_EXPECT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GFP_RX_GFP_TEST_FRAME_EXPECT(target)  VTSS_IOREG(target,0x702)

/** 
 * \brief
 * Expect test PLI value.
 * The expect test PLI value bits are used for the expect PLI field of the
 * core header for test frames.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_TEST_FRAME_EXPECT . EXPECT_TEST_PLI_VALUE
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_TEST_FRAME_EXPECT_EXPECT_TEST_PLI_VALUE(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_TEST_FRAME_EXPECT_EXPECT_TEST_PLI_VALUE     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_TEST_FRAME_EXPECT_EXPECT_TEST_PLI_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Expect test payload value.
 * The expect test payload value bits are used for the expect test frame
 * payload information.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_TEST_FRAME_EXPECT . EXPECT_TEST_PYLD_VALUE
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_TEST_FRAME_EXPECT_EXPECT_TEST_PYLD_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_TEST_FRAME_EXPECT_EXPECT_TEST_PYLD_VALUE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_TEST_FRAME_EXPECT_EXPECT_TEST_PYLD_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief GFPM Rx GFP sync error counter register
 *
 * \details
 * Register: \a GFP::GFPM_RX_GFP::RX_GFP_SYNC_ERROR_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GFP_RX_GFP_SYNC_ERROR_CNT(target)  VTSS_IOREG(target,0x703)

/** 
 * \brief
 * SSF error count.
 * The sync error count bits are used for the accumulation delineator SYNC
 * errors. The sync error count is incremented each time the frame
 * delineator transitions from the SYNC to hunt state.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_SYNC_ERROR_CNT . SSF_ERROR_COUNT
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_SYNC_ERROR_CNT_SSF_ERROR_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_SYNC_ERROR_CNT_SSF_ERROR_COUNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_SYNC_ERROR_CNT_SSF_ERROR_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx GFP core header corrected counter register
 *
 * \details
 * Register: \a GFP::GFPM_RX_GFP::RX_GFP_CORE_HDR_COR_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GFP_RX_GFP_CORE_HDR_COR_CNT(target)  VTSS_IOREG(target,0x704)

/** 
 * \brief
 * Core header corrected count.
 * The core header corrected count bits are used for the accumulation of
 * corrected core headers. Each time an incoming GFP frame core header is
 * corrected while the frame delineator is in the SYNC state, the core
 * header corrected count is incremented.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CORE_HDR_COR_CNT . CORE_HDR_COR_COUNT
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CORE_HDR_COR_CNT_CORE_HDR_COR_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CORE_HDR_COR_CNT_CORE_HDR_COR_COUNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CORE_HDR_COR_CNT_CORE_HDR_COR_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx GFP payload header corrected counter register
 *
 * \details
 * Register: \a GFP::GFPM_RX_GFP::RX_GFP_PYLD_HDR_COR_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GFP_RX_GFP_PYLD_HDR_COR_CNT(target)  VTSS_IOREG(target,0x705)

/** 
 * \brief
 * Payload header corrected count.
 * The payload header corrected count bits are used for the accumulation of
 * corrected payload headers. Each time an incoming GFP frame payload
 * header is corrected while the frame delineator is in the SYNC state, the
 * payload header corrected count is incremented.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_PYLD_HDR_COR_CNT . PYLD_HDR_COR_COUNT
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_PYLD_HDR_COR_CNT_PYLD_HDR_COR_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_PYLD_HDR_COR_CNT_PYLD_HDR_COR_COUNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_PYLD_HDR_COR_CNT_PYLD_HDR_COR_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx GFP payload header uncorrected counter register
 *
 * \details
 * Register: \a GFP::GFPM_RX_GFP::RX_GFP_PYLD_HDR_UNCOR_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GFP_RX_GFP_PYLD_HDR_UNCOR_CNT(target)  VTSS_IOREG(target,0x706)

/** 
 * \brief
 * Payload header uncorrected count.
 * The payload header uncorrected count bits are used for the accumulation
 * of uncorrected payload headers. Each time an incoming GFP frame payload
 * header is uncorrected while the frame delineator is in the SYNC state,
 * the payload header uncorrected count is incremented.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_PYLD_HDR_UNCOR_CNT . PYLD_HDR_UNCOR_COUNT
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_PYLD_HDR_UNCOR_CNT_PYLD_HDR_UNCOR_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_PYLD_HDR_UNCOR_CNT_PYLD_HDR_UNCOR_COUNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_PYLD_HDR_UNCOR_CNT_PYLD_HDR_UNCOR_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx GFP transparent CRC-16 corrected counter register
 *
 * \details
 * Register: \a GFP::GFPM_RX_GFP::RX_GFP_TRANSP_CRC16_COR_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GFP_RX_GFP_TRANSP_CRC16_COR_CNT(target)  VTSS_IOREG(target,0x707)

/** 
 * \brief
 * Transparent CRC-16 corrected count.
 * The transparent CRC-16 corrected count bits are used for the
 * accumulation of corrected transparent CRC-16 errors. Each time an
 * incoming GFP frame transparent CRC-16 error is corrected while the frame
 * delineator is in the SYNC state, the transparent CRC-16 corrected count
 * is incremented.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_TRANSP_CRC16_COR_CNT . TRANSP_CRC16_COR_COUNT
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_TRANSP_CRC16_COR_CNT_TRANSP_CRC16_COR_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_TRANSP_CRC16_COR_CNT_TRANSP_CRC16_COR_COUNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_TRANSP_CRC16_COR_CNT_TRANSP_CRC16_COR_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx GFP transparent CRC-16 uncorrected counter register
 *
 * \details
 * Register: \a GFP::GFPM_RX_GFP::RX_GFP_TRANSP_CRC16_UNCOR_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GFP_RX_GFP_TRANSP_CRC16_UNCOR_CNT(target)  VTSS_IOREG(target,0x708)

/** 
 * \brief
 * Transparent CRC-16 uncorrected count.
 * The transparent CRC-16 uncorrected count bits are used for the
 * accumulation of uncorrected transparent CRC-16 errors. Each time an
 * incoming GFP frame transparent CRC-16 error is not correctable while the
 * frame delineator is in the SYNC state, the transparent CRC-16
 * uncorrected count is incremented.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_TRANSP_CRC16_UNCOR_CNT . TRANSP_CRC16_UNCOR_COUNT
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_TRANSP_CRC16_UNCOR_CNT_TRANSP_CRC16_UNCOR_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_TRANSP_CRC16_UNCOR_CNT_TRANSP_CRC16_UNCOR_COUNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_TRANSP_CRC16_UNCOR_CNT_TRANSP_CRC16_UNCOR_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx GFP upper control frame counter
 *
 * \details
 * Register: \a GFP::GFPM_RX_GFP::RX_GFP_UPPER_CONTROL_FRAME_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GFP_RX_GFP_UPPER_CONTROL_FRAME_CNT(target)  VTSS_IOREG(target,0x709)

/** 
 * \brief
 * Upper control frame count.
 * The upper control frame count bits are used for the accumulation of
 * control frames received. Each time an incoming GFP control frame is
 * detected while the frame delineator is in the SYNC state, the control
 * frame count is incremented. The upper control frame count bits contain
 * the upper 16 bits of the 32 bit RX GFP control counter. This is a
 * saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_UPPER_CONTROL_FRAME_CNT . UPPER_CONTROL_FRAME_COUNT
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_UPPER_CONTROL_FRAME_CNT_UPPER_CONTROL_FRAME_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_UPPER_CONTROL_FRAME_CNT_UPPER_CONTROL_FRAME_COUNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_UPPER_CONTROL_FRAME_CNT_UPPER_CONTROL_FRAME_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx GFP lower control frame counter
 *
 * \details
 * Register: \a GFP::GFPM_RX_GFP::RX_GFP_LOWER_CONTROL_FRAME_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GFP_RX_GFP_LOWER_CONTROL_FRAME_CNT(target)  VTSS_IOREG(target,0x70a)

/** 
 * \brief
 * Lower control frame count.
 * The lower control frame count bits are used for the accumulation of
 * control frames received. Each time an incoming GFP control frame is
 * detected while the frame delineator is in the SYNC state, the control
 * frame count is incremented. The lower control frame count bits contain
 * the lower 16 bits of the 32 bit Rx GFP control counter. This is a
 * saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_LOWER_CONTROL_FRAME_CNT . LOWER_CONTROL_FRAME_COUNT
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_LOWER_CONTROL_FRAME_CNT_LOWER_CONTROL_FRAME_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_LOWER_CONTROL_FRAME_CNT_LOWER_CONTROL_FRAME_COUNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_LOWER_CONTROL_FRAME_CNT_LOWER_CONTROL_FRAME_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx GFP upper client management packet counter
 *
 * \details
 * Register: \a GFP::GFPM_RX_GFP::RX_GFP_UPPER_CLIENT_MGMT_PKT_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GFP_RX_GFP_UPPER_CLIENT_MGMT_PKT_CNT(target)  VTSS_IOREG(target,0x70b)

/** 
 * \brief
 * Upper client management packet count.
 * The upper client management packet count bits are used for the
 * accumulation of client management packets received. Each time an
 * incoming client management packet is detected while the frame delineator
 * is in the SYNC state, the client management packet count is incremented.
 * The upper client management packet count bits contain the upper 16 bits
 * of the 32 bit Rx GFP client management counter. This is a saturating
 * read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_UPPER_CLIENT_MGMT_PKT_CNT . UPPER_CLIENT_MGMT_PKT_COUNT
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_UPPER_CLIENT_MGMT_PKT_CNT_UPPER_CLIENT_MGMT_PKT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_UPPER_CLIENT_MGMT_PKT_CNT_UPPER_CLIENT_MGMT_PKT_COUNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_UPPER_CLIENT_MGMT_PKT_CNT_UPPER_CLIENT_MGMT_PKT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx GFP lower client management packet counter
 *
 * \details
 * Register: \a GFP::GFPM_RX_GFP::RX_GFP_LOWER_CLIENT_MGMT_PKT_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GFP_RX_GFP_LOWER_CLIENT_MGMT_PKT_CNT(target)  VTSS_IOREG(target,0x70c)

/** 
 * \brief
 * Lower client management packet count.
 * The lower client management packet count bits are used for the
 * accumulation of client management packets received. Each time an
 * incoming client management packet is detected while the frame delineator
 * is in the SYNC state, the client management packet count is incremented.
 * The lower client management packet count bits contain the lower 16 bits
 * of the 32 bit RX GFP client management counter. This is a saturating
 * read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_LOWER_CLIENT_MGMT_PKT_CNT . LOWER_CLIENT_MGMT_PKT_COUNT
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_LOWER_CLIENT_MGMT_PKT_CNT_LOWER_CLIENT_MGMT_PKT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_LOWER_CLIENT_MGMT_PKT_CNT_LOWER_CLIENT_MGMT_PKT_COUNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_LOWER_CLIENT_MGMT_PKT_CNT_LOWER_CLIENT_MGMT_PKT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx GFP upper client data packet counter
 *
 * \details
 * Register: \a GFP::GFPM_RX_GFP::RX_GFP_UPPER_CLIENT_DATA_PKT_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GFP_RX_GFP_UPPER_CLIENT_DATA_PKT_CNT(target)  VTSS_IOREG(target,0x70d)

/** 
 * \brief
 * Upper client data frame count.
 * The upper client data packet count bits are used for the accumulation of
 * client data packets received. Each time an incoming client data packet
 * is detected while the frame delineator is in the SYNC state, the client
 * data packet count is incremented. The upper client data packet count
 * bits contain the upper 16 bits of the 32 bit RX GFP client data packet
 * counter. This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_UPPER_CLIENT_DATA_PKT_CNT . UPPER_CLIENT_DATA_FRAME_COUNT
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_UPPER_CLIENT_DATA_PKT_CNT_UPPER_CLIENT_DATA_FRAME_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_UPPER_CLIENT_DATA_PKT_CNT_UPPER_CLIENT_DATA_FRAME_COUNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_UPPER_CLIENT_DATA_PKT_CNT_UPPER_CLIENT_DATA_FRAME_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx GFP lower client data packet counter
 *
 * \details
 * Register: \a GFP::GFPM_RX_GFP::RX_GFP_LOWER_CLIENT_DATA_PKT_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GFP_RX_GFP_LOWER_CLIENT_DATA_PKT_CNT(target)  VTSS_IOREG(target,0x70e)

/** 
 * \brief
 * Lower client data frame count.
 * The lower client data packet count bits are used for the accumulation of
 * client data packets received. Each time an incoming client data packet
 * is detected while the frame delineator is in the SYNC state, the client
 * data packet count is incremented. The lower client data packet count
 * bits contain the lower 16 bits of the 32 bit Rx GFP client data packet
 * counter. This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_LOWER_CLIENT_DATA_PKT_CNT . LOWER_CLIENT_DATA_FRAME_COUNT
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_LOWER_CLIENT_DATA_PKT_CNT_LOWER_CLIENT_DATA_FRAME_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_LOWER_CLIENT_DATA_PKT_CNT_LOWER_CLIENT_DATA_FRAME_COUNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_LOWER_CLIENT_DATA_PKT_CNT_LOWER_CLIENT_DATA_FRAME_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx GFP client invalid packet counter
 *
 * \details
 * Register: \a GFP::GFPM_RX_GFP::RX_GFP_CLIENT_INVALID_PKT_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_INVALID_PKT_CNT(target)  VTSS_IOREG(target,0x70f)

/** 
 * \brief
 * Client invalid packet count.
 * The client invalid packet count bits are used for the accumulation of
 * client invalid packets received. Each time an invalid client packet is
 * detected while the frame delineator is in the SYNC state, the client
 * invalid packet count is incremented. This is a saturating read only
 * counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_INVALID_PKT_CNT . CLIENT_INVALID_PKT_COUNT
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_INVALID_PKT_CNT_CLIENT_INVALID_PKT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_INVALID_PKT_CNT_CLIENT_INVALID_PKT_COUNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_INVALID_PKT_CNT_CLIENT_INVALID_PKT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx GFP FCS error counter
 *
 * \details
 * Register: \a GFP::GFPM_RX_GFP::RX_GFP_FCS_ERROR_CNT
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GFP_RX_GFP_FCS_ERROR_CNT(target)  VTSS_IOREG(target,0x710)

/** 
 * \brief
 * FCS error count.
 * The FCS error count bits are used for the accumulation of frame check
 * sequence errors The FCS error count is incremented each time an FCS
 * error is detected. This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_FCS_ERROR_CNT . FCS_ERROR_COUNT
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_FCS_ERROR_CNT_FCS_ERROR_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_FCS_ERROR_CNT_FCS_ERROR_COUNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_FCS_ERROR_CNT_FCS_ERROR_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx GFP client management accept packet length register
 *
 * \details
 * Register: \a GFP::GFPM_RX_GFP::RX_GFP_CLIENT_MGMT_PKT_LENGTH
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PKT_LENGTH(target)  VTSS_IOREG(target,0x715)

/** 
 * \brief
 * Client management accept packet length.
 * The client management accept packet length bits are used to indicate the
 * length of the accepted client management frame contained in the Rx GFP
 * client management capture packet memory, and are updated each time a
 * client management frame accept interrupt occurs.
 * NOTE: If the number of bytes contained in the accepted client management
 * frame exceeds the number of bytes available in the Rx GFP client
 * management capture packet memory, the client management accept packet
 * length bits value will be the length of the available Rx GFP client
 * management capture packet memory.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PKT_LENGTH . CLIENT_MGMT_ACCEPT_PKT_LENGTH
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PKT_LENGTH_CLIENT_MGMT_ACCEPT_PKT_LENGTH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PKT_LENGTH_CLIENT_MGMT_ACCEPT_PKT_LENGTH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PKT_LENGTH_CLIENT_MGMT_ACCEPT_PKT_LENGTH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GFPM Rx GFP client management PTI/UPI capture register
 *
 * \details
 * Register: \a GFP::GFPM_RX_GFP::RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE
 *
 * @param target A \a ::vtss_target_GFP_e target
 */
#define VTSS_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE(target)  VTSS_IOREG(target,0x716)

/** 
 * \brief
 * Capture PTI 000 types.
 * If the capture PTI 000 types bit is set, incoming frame types with a
 * value of 000 will be analyzed and captured when identical extracted
 * client management frames values are received for the number of
 * consecutive sequences programmed in the client management frame filter
 * count bits of the Rx GFP control register. If the capture PTI 000 types
 * bit is cleared, incoming frames with a PTI field value of 000 will be
 * ignored.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE . CAPTURE_PTI_000_TYPES
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE_CAPTURE_PTI_000_TYPES(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE_CAPTURE_PTI_000_TYPES     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE_CAPTURE_PTI_000_TYPES(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Capture PTI 100 types.
 * If the capture PTI 100 types bit is set, incoming frame types with a
 * value of 100 will be analyzed and captured when identical extracted
 * client management frames values are received for the number of
 * consecutive sequences programmed in the client management frame filter
 * count bits of the Rx GFP control register. If the capture PTI 100 types
 * bit is cleared, incoming frames with a PTI field value of 100 will be
 * ignored.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE . CAPTURE_PTI_100_TYPES
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE_CAPTURE_PTI_100_TYPES(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE_CAPTURE_PTI_100_TYPES     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE_CAPTURE_PTI_100_TYPES(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Capture PTI 101 types.
 * If the capture PTI 101 types bit is set, incoming frame types with a
 * value of 101 will be analyzed and captured when identical extracted
 * client management frames values are received for the number of
 * consecutive sequences programmed in the client management frame filter
 * count bits of the RX GFP control register. If the capture PTI 101 types
 * bit is cleared, incoming frames with a PTI field value of 101 will be
 * ignored.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE . CAPTURE_PTI_101_TYPES
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE_CAPTURE_PTI_101_TYPES(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE_CAPTURE_PTI_101_TYPES     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE_CAPTURE_PTI_101_TYPES(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Capture all PTI types.
 * If the capture all PTI types bit is set, incoming frame types with any
 * value will be analyzed and captured when identical extracted client
 * management frames values are received for the number of consecutive
 * sequences programmed in the client management frame filter count bits of
 * the Rx GFP control register. This bit has priority over bits 15-13.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE . CAPTURE_ALL_PTI_TYPES
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE_CAPTURE_ALL_PTI_TYPES(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE_CAPTURE_ALL_PTI_TYPES     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE_CAPTURE_ALL_PTI_TYPES(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Capture PTI value.
 * The capture PTI value bits are updated when identical extracted client
 * management frames values are received (including appropriate PTI values
 * as specified by bits 15-12) for the number of consecutive sequences
 * programmed in the client management frame filter count bits of the Rx
 * GFP control register.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE . CAPTURE_PTI_VALUE
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE_CAPTURE_PTI_VALUE(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE_CAPTURE_PTI_VALUE     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE_CAPTURE_PTI_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * Capture UPI value.
 * The capture UPI value bits are updated when identical extracted client
 * management frames values are received (including appropriate PTI values
 * as specified by bits 15-12) for the number of consecutive sequences
 * programmed in the client management frame filter count bits of the Rx
 * GFP control register. 
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE . CAPTURE_UPI_VALUE
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE_CAPTURE_UPI_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE_CAPTURE_UPI_VALUE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE_CAPTURE_UPI_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief GFPM Rx GFP client management capture packet memory
 *
 * \details
 * Register: \a GFP::GFPM_RX_GFP::RX_GFP_CLIENT_MGMT_PKT_MEMORY
 *
 * @param target A \a ::vtss_target_GFP_e target
 * @param ri Register: RX_GFP_CLIENT_MGMT_PKT_MEMORY (??), 0-63
 */
#define VTSS_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PKT_MEMORY(target,ri)  VTSS_IOREG(target,0x7c0 + (ri))

/** 
 * \brief
 * Client management capture packet value.
 * The client management capture packet value bits are updated when
 * identical extracted client management frame values are received
 * (including appropriate PTI values as specified by bits 15-12) for the
 * number of consecutive sequences programmed in the client management
 * frame filter count bits of the Rx GFP control register.
 *
 * \details 
 * Field: ::VTSS_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PKT_MEMORY . CLIENT_MGMT_CAPTURE_PKT_VALUE
 */
#define  VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PKT_MEMORY_CLIENT_MGMT_CAPTURE_PKT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PKT_MEMORY_CLIENT_MGMT_CAPTURE_PKT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PKT_MEMORY_CLIENT_MGMT_CAPTURE_PKT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


#endif /* _VTSS_DAYTONA_REGS_GFP_H_ */
