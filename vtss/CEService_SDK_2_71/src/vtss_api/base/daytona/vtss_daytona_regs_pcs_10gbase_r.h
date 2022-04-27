#ifndef _VTSS_DAYTONA_REGS_PCS_10GBASE_R_H_
#define _VTSS_DAYTONA_REGS_PCS_10GBASE_R_H_

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
 * Target: \a PCS_10GBASE_R
 *
 * \see vtss_target_PCS_10GBASE_R_e
 *
 * PCS configuration and status registers and counters
 *
 ***********************************************************************/

/**
 * Register Group: \a PCS_10GBASE_R::PCS_10GBR_CFG
 *
 * PCS configuration
 */


/** 
 * \brief PCS configuration
 *
 * \details
 * Configuration register
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_CFG::PCS_CFG
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG(target)  VTSS_IOREG(target,0x0)

/** 
 * \brief
 * When 1, selects the external XGMII datapath as the source for Rx rate
 * adapt data instead of the Rx decode block. This is to support IEEE1588
 * processing.
 *
 * \details 
 * 0: Normal PCS datapath
 * 1: External 1588 datapath
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG . SEL_EXT_RX_XGMII
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_SEL_EXT_RX_XGMII(x)  VTSS_ENCODE_BITFIELD(x,21,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_SEL_EXT_RX_XGMII     VTSS_ENCODE_BITMASK(21,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_SEL_EXT_RX_XGMII(x)  VTSS_EXTRACT_BITFIELD(x,21,1)

/** 
 * \brief
 * When 1, selects the external XGMII datapath as the source for Tx encoder
 * data instead of the Tx rate adapt block. This is to support IEEE1588
 * processing.
 *
 * \details 
 * 0: Normal PCS datapath
 * 1: External 1588 datapath
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG . SEL_EXT_TX_XGMII
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_SEL_EXT_TX_XGMII(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_SEL_EXT_TX_XGMII     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_SEL_EXT_TX_XGMII(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \brief
 * Enable loopback operation. When set, data is looped from the Tx XGMII
 * input to the Rx XGMII output. Transmit WIS/PMA data is the data value
 * 0x00FF and receive WIS/PMA data is ignored.
 *
 * \details 
 * 0: Normal mode
 * 1: Loopback mode
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG . LOOPBACK_ENA
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_LOOPBACK_ENA(x)  VTSS_ENCODE_BITFIELD(x,19,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_LOOPBACK_ENA     VTSS_ENCODE_BITMASK(19,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_LOOPBACK_ENA(x)  VTSS_EXTRACT_BITFIELD(x,19,1)

/** 
 * \brief
 * Flip the data bus bits at the WIS/PMA interface such that bit 63 is
 * mapped to bit 0 and bit 0 to 63. i.e. the output bus (63 down to 0) is
 * remapped to (0 to 63) and bit 63 is the first bit.
 *
 * \details 
 * 0: No flip (LSB first)
 * 1: Flip bus (MSB first)
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG . RX_DATA_FLIP
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_DATA_FLIP(x)  VTSS_ENCODE_BITFIELD(x,18,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_DATA_FLIP     VTSS_ENCODE_BITMASK(18,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_DATA_FLIP(x)  VTSS_EXTRACT_BITFIELD(x,18,1)

/** 
 * \brief
 * Allows the PCS logic to use the input LOS signal instead of the WIS
 * RX_SIGNAL_OK_I signal to determine if the signal is valid. When the bit
 * is asserted (1), the LOS input is used and a 1 on the input indicates
 * that signal is lost. When the bit is 0, the RX_SIGNAL_OK_I input is used
 * and a 0 on the input indicates that the signal is lost.
 *
 * \details 
 * 0: Use RX_SIGNAL_OK_I
 * 1: Use LOS_I
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG . USE_LOS
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_USE_LOS(x)  VTSS_ENCODE_BITFIELD(x,17,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_USE_LOS     VTSS_ENCODE_BITMASK(17,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_USE_LOS(x)  VTSS_EXTRACT_BITFIELD(x,17,1)

/** 
 * \brief
 * One-shot that causes the RX FIFO to be cleared and reset. Bit is reset
 * by hardware.
 *
 * \details 
 * 0: No action
 * 1: Clear FIFO
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG . RX_FIFO_FLUSH
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_FIFO_FLUSH(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_FIFO_FLUSH     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_FIFO_FLUSH(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Enable test pattern checking mode.
 *
 * \details 
 * 0: Normal operation
 * 1: Test pattern mode
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG . RX_TEST_MODE
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_TEST_MODE(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_TEST_MODE     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_TEST_MODE(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * This bit selects which input is routed to the PCS logic. When 1, the
 * monitoring input is connected to the PCS. When 0, the traffic signal is
 * connected.
 *
 * \details 
 * 0: Normal traffic sent to PCS
 * 1: Monitor data sent to PCS
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG . RX_INPUT_MON
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_INPUT_MON(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_INPUT_MON     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_INPUT_MON(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * This bit selects whether the output of the block is the PCS output or
 * input. If 1, then the PCS is bypassed and the output is the PCS block
 * input.
 *
 * \details 
 * 0: PCS logic used
 * 1: PCS logic bypassed
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG . RX_PCS_BYPASS
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PCS_BYPASS(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PCS_BYPASS     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PCS_BYPASS(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * PCS Rx clock enable. When disabled, the clock is gated off and reset is
 * asserted.
 *
 * \details 
 * 0: Clock gated off
 * 1: Clock enabled
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG . RX_PMA_CLK_ENA
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PMA_CLK_ENA(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PMA_CLK_ENA     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_PMA_CLK_ENA(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * PCS Rx clock enable. When disabled, the clock is gated off and reset is
 * asserted.
 *
 * \details 
 * 0: Clock gated off
 * 1: Clock enabled
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG . RX_XGMII_CLK_ENA
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_XGMII_CLK_ENA(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_XGMII_CLK_ENA     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_RX_XGMII_CLK_ENA(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Flip the data bus bits at the WIS/PMA interface such that bit 63 is
 * mapped to bit 0 and bit 0 to 63. i.e. the output bus (63 down to 0) is
 * remapped to (0 to 63) and bit 63 is the first bit.
 *
 * \details 
 * 0 = No flip (LSB first)
 * 1 = Flip bus (MSB first)
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG . TX_DATA_FLIP
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_DATA_FLIP(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_DATA_FLIP     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_DATA_FLIP(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * One-shot that causes the TX FIFO to be cleared and reset. Bit is reset
 * by hardware.
 *
 * \details 
 * 0: No action
 * 1: Clear FIFO
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG . TX_FIFO_FLUSH
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_FIFO_FLUSH(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_FIFO_FLUSH     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_FIFO_FLUSH(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Enable test pattern generation mode
 *
 * \details 
 * 0: Normal operation
 * 1: Test pattern mode
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG . TX_TEST_MODE
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_TEST_MODE(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_TEST_MODE     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_TEST_MODE(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * This bit selects whether the output of the block is the PCS output or
 * input. If 1, then the PCS is bypassed and the output is the PCS block
 * input.
 *
 * \details 
 * 0: PCS logic used
 * 1: PCS logic bypassed
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG . TX_PCS_BYPASS
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PCS_BYPASS(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PCS_BYPASS     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PCS_BYPASS(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * PCS Tx clock enable. When disabled, the clock is gated off and reset is
 * asserted.
 *
 * \details 
 * 0: Clock gated off
 * 1: Clock enabled
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG . TX_PMA_CLK_ENA
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PMA_CLK_ENA(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PMA_CLK_ENA     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_PMA_CLK_ENA(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * PCS Tx clock enable. When disabled, the clock is gated off and reset is
 * asserted.
 *
 * \details 
 * 0: Clock gated off
 * 1: Clock enabled
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG . TX_XGMII_CLK_ENA
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_XGMII_CLK_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_XGMII_CLK_ENA     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_CFG_TX_XGMII_CLK_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Tx FIFO threshold control
 *
 * \details
 * Set the add and drop thresholds for the Tx FIFO
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_CFG::TX_FIFO_CFG
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG(target)  VTSS_IOREG(target,0x1)

/** 
 * \brief
 * Enables rate adaptation
 *
 * \details 
 * 0: Disabled
 * 1: Enabled
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG . TX_RADAPT_ENABLE
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_RADAPT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,31,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_RADAPT_ENABLE     VTSS_ENCODE_BITMASK(31,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_RADAPT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,31,1)

/** 
 * \brief
 * Set the minimum inter-frame gap to maintain during rate adaptation when
 * dropping transactions. Setting the value to 1 means that 1 idle or
 * ordered set is kept after a terminate. When set to 2 or 3, that many
 * idle or ordered set transactions are kept. Each transaction is 4 bytes.
 *
 * \details 
 * 0: INVALID
 * 1: 1 Idle/O set column maintained
 * 2: 2 Idle/O set column maintained
 * 3: 3 Idle/O set column maintained
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG . TX_MIN_IFG
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_MIN_IFG(x)  VTSS_ENCODE_BITFIELD(x,24,2)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_MIN_IFG     VTSS_ENCODE_BITMASK(24,2)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_MIN_IFG(x)  VTSS_EXTRACT_BITFIELD(x,24,2)

/** 
 * \brief
 * Set the add threshold in the FIFO. Level is in 72-bit words.
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG . TX_ADD_LVL
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_ADD_LVL(x)  VTSS_ENCODE_BITFIELD(x,16,8)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_ADD_LVL     VTSS_ENCODE_BITMASK(16,8)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_ADD_LVL(x)  VTSS_EXTRACT_BITFIELD(x,16,8)

/** 
 * \brief
 * Set the drop threshold in the FIFO. Level is in 72-bit words.
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG . TX_DROP_LVL
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_DROP_LVL(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_DROP_LVL     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_TX_FIFO_CFG_TX_DROP_LVL(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Rx FIFO threshold control
 *
 * \details
 * Set the add and drop thresholds for the Rx FIFO
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_CFG::RX_FIFO_CFG
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG(target)  VTSS_IOREG(target,0x2)

/** 
 * \brief
 * Enables rate adaptation
 *
 * \details 
 * 0: Disabled
 * 1: Enabled
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG . RX_RADAPT_ENABLE
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_RADAPT_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,31,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_RADAPT_ENABLE     VTSS_ENCODE_BITMASK(31,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_RADAPT_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,31,1)

/** 
 * \brief
 * Set the minimum inter-frame gap to maintain during rate adaptation when
 * dropping transactions. Setting the value to 1 means that 1 Idle or
 * ordered set is kept after a terminate. When set to 2 or 3, that many
 * idle or ordered set transactions are kept. Each transaction is 4 bytes.
 *
 * \details 
 * 0: INVALID
 * 1: 1 Idle/O set column maintained
 * 2: 2 Idle/O set column maintained
 * 3: 3 Idle/O set column maintained
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG . RX_MIN_IFG
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_MIN_IFG(x)  VTSS_ENCODE_BITFIELD(x,24,2)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_MIN_IFG     VTSS_ENCODE_BITMASK(24,2)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_MIN_IFG(x)  VTSS_EXTRACT_BITFIELD(x,24,2)

/** 
 * \brief
 * Set the add threshold in the FIFO. Level is in 72-bit words.
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG . RX_ADD_LVL
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_ADD_LVL(x)  VTSS_ENCODE_BITFIELD(x,16,8)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_ADD_LVL     VTSS_ENCODE_BITMASK(16,8)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_ADD_LVL(x)  VTSS_EXTRACT_BITFIELD(x,16,8)

/** 
 * \brief
 * Set the drop threshold in the FIFO. Level is in 72-bit words.
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG . RX_DROP_LVL
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_DROP_LVL(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_DROP_LVL     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_RX_FIFO_CFG_RX_DROP_LVL(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Scrambler initialization
 *
 * \details
 * Most significant bits of the scrambler
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_CFG::TX_SEEDA_MSB
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_SEEDA_MSB(target)  VTSS_IOREG(target,0x3)

/** 
 * \brief
 * Most significant bits of scrambler used to initialize it during test
 * mode
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_SEEDA_MSB . TX_SEEDA_MSB
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_SEEDA_MSB_TX_SEEDA_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,26)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_TX_SEEDA_MSB_TX_SEEDA_MSB     VTSS_ENCODE_BITMASK(0,26)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_TX_SEEDA_MSB_TX_SEEDA_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,26)


/** 
 * \brief Scrambler initialization
 *
 * \details
 * Least significant bits of the scrambler. Used in pseudo-random and
 * PRBS31 modes.
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_CFG::TX_SEEDA_LSB
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_SEEDA_LSB(target)  VTSS_IOREG(target,0x4)

/** 
 * \brief
 * Least significant bits of scrambler used to initialize it during test
 * mode
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_SEEDA_LSB . TX_SEEDA_LSB
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_SEEDA_LSB_TX_SEEDA_LSB(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_TX_SEEDA_LSB_TX_SEEDA_LSB     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_TX_SEEDA_LSB_TX_SEEDA_LSB(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Scrambler initialization
 *
 * \details
 * Most significant bits of the scrambler
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_CFG::TX_SEEDB_MSB
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_SEEDB_MSB(target)  VTSS_IOREG(target,0x5)

/** 
 * \brief
 * Most significant bits of scrambler used to initialize it during test
 * mode
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_SEEDB_MSB . TX_SEEDB_MSB
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_SEEDB_MSB_TX_SEEDB_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,26)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_TX_SEEDB_MSB_TX_SEEDB_MSB     VTSS_ENCODE_BITMASK(0,26)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_TX_SEEDB_MSB_TX_SEEDB_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,26)


/** 
 * \brief Scrambler initialization
 *
 * \details
 * Least significant bits of the scrambler
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_CFG::TX_SEEDB_LSB
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_SEEDB_LSB(target)  VTSS_IOREG(target,0x6)

/** 
 * \brief
 * Least significant bits of scrambler used to initialize it during test
 * mode
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_SEEDB_LSB . TX_SEEDB_LSB
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_SEEDB_LSB_TX_SEEDB_LSB(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_TX_SEEDB_LSB_TX_SEEDB_LSB     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_TX_SEEDB_LSB_TX_SEEDB_LSB(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief PRBS31 initialization
 *
 * \details
 * Initial value to load into the PRBS31 linear-feedback shift register
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_CFG::RX_PRBS31_INIT
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_PRBS31_INIT(target)  VTSS_IOREG(target,0x7)

/** 
 * \brief
 * PRBS31 initial value.
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_PRBS31_INIT . RX_PRBS31_INIT
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_PRBS31_INIT_RX_PRBS31_INIT(x)  VTSS_ENCODE_BITFIELD(x,0,31)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_RX_PRBS31_INIT_RX_PRBS31_INIT     VTSS_ENCODE_BITMASK(0,31)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_RX_PRBS31_INIT_RX_PRBS31_INIT(x)  VTSS_EXTRACT_BITFIELD(x,0,31)


/** 
 * \brief MSB of test pattern
 *
 * \details
 * Most significant Tx bits of test pattern
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_CFG::TX_DATAPAT_MSB
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_DATAPAT_MSB(target)  VTSS_IOREG(target,0x8)

/** 
 * \brief
 * Most significant 32 bits of 64-bit data pattern used in pseudo-random
 * and user-defined test pattern mode
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_DATAPAT_MSB . TX_DATAPAT_MSB
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_DATAPAT_MSB_TX_DATAPAT_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_TX_DATAPAT_MSB_TX_DATAPAT_MSB     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_TX_DATAPAT_MSB_TX_DATAPAT_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief LSB of test pattern
 *
 * \details
 * Least significant Tx bits of test pattern
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_CFG::TX_DATAPAT_LSB
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_DATAPAT_LSB(target)  VTSS_IOREG(target,0x9)

/** 
 * \brief
 * Least significant 32 bits of 64-bit data pattern used in pseudo-random
 * and user-defined test pattern mode
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TX_DATAPAT_LSB . TX_DATAPAT_LSB
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TX_DATAPAT_LSB_TX_DATAPAT_LSB(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_TX_DATAPAT_LSB_TX_DATAPAT_LSB     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_TX_DATAPAT_LSB_TX_DATAPAT_LSB(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief MSB of test pattern
 *
 * \details
 * Most significant Rx bits of test pattern
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_CFG::RX_DATAPAT_MSB
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_DATAPAT_MSB(target)  VTSS_IOREG(target,0xa)

/** 
 * \brief
 * Most significant 32 bits of 64-bit data pattern used in pseudo-random
 * and user-defined test pattern mode
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_DATAPAT_MSB . RX_DATAPAT_MSB
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_DATAPAT_MSB_RX_DATAPAT_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_RX_DATAPAT_MSB_RX_DATAPAT_MSB     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_RX_DATAPAT_MSB_RX_DATAPAT_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief LSB of test pattern
 *
 * \details
 * Least significant Rx bits of test pattern
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_CFG::RX_DATAPAT_LSB
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_DATAPAT_LSB(target)  VTSS_IOREG(target,0xb)

/** 
 * \brief
 * Least significant 32 bits of 64-bit data pattern used in pseudo-random
 * and user-defined test pattern mode
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_RX_DATAPAT_LSB . RX_DATAPAT_LSB
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_RX_DATAPAT_LSB_RX_DATAPAT_LSB(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_RX_DATAPAT_LSB_RX_DATAPAT_LSB     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_RX_DATAPAT_LSB_RX_DATAPAT_LSB(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Test pattern mode configuration
 *
 * \details
 * Select test patterns when test mode is enabled
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_CFG::TEST_CFG
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TEST_CFG(target)  VTSS_IOREG(target,0xc)

/** 
 * \brief
 * Disables inversion of seeds and data in the pseudo-random test pattern
 *
 * \details 
 * 0: Inversion enabled
 * 1: Inversion disabled
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TEST_CFG . RX_DSBL_INV
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TEST_CFG_RX_DSBL_INV(x)  VTSS_ENCODE_BITFIELD(x,18,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_TEST_CFG_RX_DSBL_INV     VTSS_ENCODE_BITMASK(18,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_TEST_CFG_RX_DSBL_INV(x)  VTSS_EXTRACT_BITFIELD(x,18,1)

/** 
 * \brief
 * Select the test pattern used by the test pattern checker. This register
 * is only used if RX_TEST_MODE is active.
 *
 * \details 
 * 0: unused
 * 1: Pseudo random
 * 2: PRBS31
 * 3: User defined
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TEST_CFG . RX_TESTPAT_SEL
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TEST_CFG_RX_TESTPAT_SEL(x)  VTSS_ENCODE_BITFIELD(x,16,2)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_TEST_CFG_RX_TESTPAT_SEL     VTSS_ENCODE_BITMASK(16,2)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_TEST_CFG_RX_TESTPAT_SEL(x)  VTSS_EXTRACT_BITFIELD(x,16,2)

/** 
 * \brief
 * Disables inversion of seeds and data in the pseudo-random test pattern
 *
 * \details 
 * 0: Inversion enabled
 * 1: Inversion disabled
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TEST_CFG . TX_DSBL_INV
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TEST_CFG_TX_DSBL_INV(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_TEST_CFG_TX_DSBL_INV     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_TEST_CFG_TX_DSBL_INV(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Period of square wave. Value in the register is (Period-4). Valid values
 * of period are 4 to 11.
 *
 * \details 
 * Period = (TX_SQPW_4B+4)
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TEST_CFG . TX_SQPW_4B
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TEST_CFG_TX_SQPW_4B(x)  VTSS_ENCODE_BITFIELD(x,2,3)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_TEST_CFG_TX_SQPW_4B     VTSS_ENCODE_BITMASK(2,3)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_TEST_CFG_TX_SQPW_4B(x)  VTSS_EXTRACT_BITFIELD(x,2,3)

/** 
 * \brief
 * Select the test pattern used by the test pattern generator. This
 * register is only used if TX_TEST_MODE is active.
 *
 * \details 
 * 0: Square wave
 * 1: Pseudo random
 * 2: PRBS31
 * 3: User defined
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TEST_CFG . TX_TESTPAT_SEL
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TEST_CFG_TX_TESTPAT_SEL(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_TEST_CFG_TX_TESTPAT_SEL     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_TEST_CFG_TX_TESTPAT_SEL(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief PCS interrupt mask register
 *
 * \details
 * Masks for PCS interrupt sources and sticky bits in PCS_INTR_STAT
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_CFG::PCS_INTR_MASK
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK(target)  VTSS_IOREG(target,0xd)

/** 
 * \brief
 * Mask for the RX_FIFO_ECC_2BIT_STICKY bit
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK . RX_FIFO_ECC_2BIT_MASK
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_FIFO_ECC_2BIT_MASK(x)  VTSS_ENCODE_BITFIELD(x,17,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_FIFO_ECC_2BIT_MASK     VTSS_ENCODE_BITMASK(17,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_FIFO_ECC_2BIT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,17,1)

/** 
 * \brief
 * Mask for the RX_FIFO_ECC_1BIT_STICKY bit
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK . RX_FIFO_ECC_1BIT_MASK
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_FIFO_ECC_1BIT_MASK(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_FIFO_ECC_1BIT_MASK     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_FIFO_ECC_1BIT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Mask for the TX_FIFO_ECC_2BIT_STICKY bit
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK . TX_FIFO_ECC_2BIT_MASK
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_TX_FIFO_ECC_2BIT_MASK(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_TX_FIFO_ECC_2BIT_MASK     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_TX_FIFO_ECC_2BIT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Mask for the TX_FIFO_ECC_1BIT_STICKY bit
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK . TX_FIFO_ECC_1BIT_MASK
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_TX_FIFO_ECC_1BIT_MASK(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_TX_FIFO_ECC_1BIT_MASK     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_TX_FIFO_ECC_1BIT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Mask for the RX_FSET_FIFO_FULL_STICKY bit
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK . RX_FSET_FIFO_FULL_MASK
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_FSET_FIFO_FULL_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_FSET_FIFO_FULL_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_FSET_FIFO_FULL_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Mask for the RX_FSET_STICKY bit
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK . RX_FSET_MASK
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_FSET_MASK(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_FSET_MASK     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_FSET_MASK(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Mask for the XGMII_ERR_STICKY bit
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK . XGMII_ERR_MASK
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_XGMII_ERR_MASK(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_XGMII_ERR_MASK     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_XGMII_ERR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Mask for the TX_FIFO_UFLOW_STICKY bit
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK . TX_FIFO_UFLOW_MASK
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_TX_FIFO_UFLOW_MASK(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_TX_FIFO_UFLOW_MASK     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_TX_FIFO_UFLOW_MASK(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Mask for the TX_FIFO_OFLOW_STICKY bit
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK . TX_FIFO_OFLOW_MASK
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_TX_FIFO_OFLOW_MASK(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_TX_FIFO_OFLOW_MASK     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_TX_FIFO_OFLOW_MASK(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Mask for RX_OSET_FIFO_FULL_STICKY
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK . RX_OSET_FIFO_FULL_MASK
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_OSET_FIFO_FULL_MASK(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_OSET_FIFO_FULL_MASK     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_OSET_FIFO_FULL_MASK(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Mask for the RX_OSET_STICKY bit
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK . RX_OSET_MASK
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_OSET_MASK(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_OSET_MASK     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_OSET_MASK(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Mask for the C64B66B_ERR_STICKY bit
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK . C64B66B_ERR_MASK
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_C64B66B_ERR_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_C64B66B_ERR_MASK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_C64B66B_ERR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Mask for the LOCK_CHANGED_STICKY bit
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK . LOCK_CHANGED_MASK
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_LOCK_CHANGED_MASK(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_LOCK_CHANGED_MASK     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_LOCK_CHANGED_MASK(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Mask for the RX_FIFO_UFLOW_STICKY bit
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK . RX_FIFO_UFLOW_MASK
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_FIFO_UFLOW_MASK(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_FIFO_UFLOW_MASK     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_FIFO_UFLOW_MASK(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Mask for the RX_FIFO_OFLOW_STICKY bit
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK . RX_FIFO_OFLOW_MASK
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_FIFO_OFLOW_MASK(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_FIFO_OFLOW_MASK     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_FIFO_OFLOW_MASK(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Mask for the RX_HI_BER_STICKY bit
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK . RX_HI_BER_MASK
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_HI_BER_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_HI_BER_MASK     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_PCS_INTR_MASK_RX_HI_BER_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Counter for 125 microsecond period
 *
 * \details
 * This register sets the number of WIS/PMA divide-by-2 clocks in one 125
 * microsecond interval. The counter increments and wraps. It should be set
 * to (125 * freq_Mhz/2) where freq_Mhz is the WIS/PMA frequency in
 * Megahertz. 0 disables the counter.
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_CFG::TIMER_125
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TIMER_125(target)  VTSS_IOREG(target,0xe)

/** 
 * \brief
 * Sets the maximum count for the 125 microsecond counter. Counts input
 * clocks.
 *
 * \details 
 * 16-bit binary number
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_TIMER_125 . TIMER_125
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_TIMER_125_TIMER_125(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_TIMER_125_TIMER_125     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_TIMER_125_TIMER_125(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Spare register
 *
 * \details
 * Register: \a PCS_10GBASE_R::PCS_10GBR_CFG::SPARE_CFG
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_SPARE_CFG(target)  VTSS_IOREG(target,0xf)

/** 
 * \brief
 * Spare configuration register
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_CFG_SPARE_CFG . SPARE_CFG
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_CFG_SPARE_CFG_SPARE_CFG(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_CFG_SPARE_CFG_SPARE_CFG     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_CFG_SPARE_CFG_SPARE_CFG(x)  VTSS_EXTRACT_BITFIELD(x,0,32)

/**
 * Register Group: \a PCS_10GBASE_R::PCS_10GBR_STATUS
 *
 * PCS status
 */


/** 
 * \brief PCS interrupt status register
 *
 * \details
 * Contains the sticky bits representing interrupts from the PCS. Set by
 * logic. Write 1 to clear.
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_STATUS::PCS_INTR_STAT
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT(target)  VTSS_IOREG(target,0x10)

/** 
 * \brief
 * Bit is set when an uncorrectable dual-bit error is detected by the Rx
 * FIFO RAM's ECC logic
 *
 * \details 
 * 0: No errors
 * 1: Dual-bit error detected
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT . RX_FIFO_ECC_2BIT_STICKY
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FIFO_ECC_2BIT_STICKY(x)  VTSS_ENCODE_BITFIELD(x,17,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FIFO_ECC_2BIT_STICKY     VTSS_ENCODE_BITMASK(17,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FIFO_ECC_2BIT_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,17,1)

/** 
 * \brief
 * Bit is set when a correctable single-bit error is detected by the Rx
 * FIFO RAM's ECC logic
 *
 * \details 
 * 0: No errors
 * 1: Single-bit error detected/corrected
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT . RX_FIFO_ECC_1BIT_STICKY
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FIFO_ECC_1BIT_STICKY(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FIFO_ECC_1BIT_STICKY     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FIFO_ECC_1BIT_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Bit is set when an uncorrectable dual-bit error is detected by the Tx
 * FIFO RAM's ECC logic
 *
 * \details 
 * 0: No errors
 * 1: Dual-bit error detected
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT . TX_FIFO_ECC_2BIT_STICKY
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_TX_FIFO_ECC_2BIT_STICKY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_TX_FIFO_ECC_2BIT_STICKY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_TX_FIFO_ECC_2BIT_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Bit is set when a correctable single-bit error is detected by the Tx
 * FIFO RAM's ECC logic
 *
 * \details 
 * 0: No errors
 * 1: Single-bit error detected/corrected
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT . TX_FIFO_ECC_1BIT_STICKY
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_TX_FIFO_ECC_1BIT_STICKY(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_TX_FIFO_ECC_1BIT_STICKY     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_TX_FIFO_ECC_1BIT_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Interrupt generated when the Rx signal ordered set FIFO is full or
 * becomes not full
 * Interrupt occurs whenever there is ANY change in the underlying status
 *
 * \details 
 * 0: Rx ordered set FIFO full status unchanged
 * 1: Rx ordered set FIFO full status changed
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT . RX_FSET_FIFO_FULL_STICKY
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FSET_FIFO_FULL_STICKY(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FSET_FIFO_FULL_STICKY     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FSET_FIFO_FULL_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Interrupt indicating that a signal ordered set was received and captured
 * (||Fsig||)
 *
 * \details 
 * 0: No ordered set captured
 * 1: Ordered set captured in FIFO
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT . RX_FSET_STICKY
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FSET_STICKY(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FSET_STICKY     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FSET_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Interrupt if an invalid Tx XGMII character is detected
 *
 * \details 
 * 0: No invalid character
 * 1: Invalid character detected
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT . XGMII_ERR_STICKY
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_XGMII_ERR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_XGMII_ERR_STICKY     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_XGMII_ERR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * FIFO underflow interrupt
 *
 * \details 
 * 0: No underflow
 * 1: FIFO underflow
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT . TX_FIFO_UFLOW_STICKY
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_TX_FIFO_UFLOW_STICKY(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_TX_FIFO_UFLOW_STICKY     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_TX_FIFO_UFLOW_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * FIFO overflow interrupt
 *
 * \details 
 * 0: No overflow
 * 1: FIFO overflow
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT . TX_FIFO_OFLOW_STICKY
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_TX_FIFO_OFLOW_STICKY(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_TX_FIFO_OFLOW_STICKY     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_TX_FIFO_OFLOW_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Interrupt generated when the Rx sequence ordered set FIFO is full or
 * becomes not full
 * Interrupt occurs whenever there is ANY change in the underlying status
 *
 * \details 
 * 0: Rx ordered set FIFO full status unchanged
 * 1: Rx ordered set FIFO full status changed
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT . RX_OSET_FIFO_FULL_STICKY
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_OSET_FIFO_FULL_STICKY(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_OSET_FIFO_FULL_STICKY     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_OSET_FIFO_FULL_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Interrupt indicating that a sequence ordered set was received and
 * captured (||Q||)
 *
 * \details 
 * 0: No ordered set captured
 * 1: Ordered set captured in FIFO
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT . RX_OSET_STICKY
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_OSET_STICKY(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_OSET_STICKY     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_OSET_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Interrupt when an invalid Rx block is detected
 *
 * \details 
 * 0: No invalid blocks
 * 1: Invalid block detected
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT . C64B66B_ERR_STICKY
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_C64B66B_ERR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_C64B66B_ERR_STICKY     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_C64B66B_ERR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Lock (loss of synchronization) changed interrupt
 * Interrupt occurs whenever there is ANY change in the underlying status
 * of the LOCK signal
 *
 * \details 
 * 0: Synchronization has not changed
 * 1: Synchronization (lock) status changed
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT . LOCK_CHANGED_STICKY
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_LOCK_CHANGED_STICKY(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_LOCK_CHANGED_STICKY     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_LOCK_CHANGED_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * FIFO underflow interrupt
 * Interrupt occurs whenever there is ANY change in the underlying status
 *
 * \details 
 * 0: No change in underflow status
 * 1: FIFO underflow status changed
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT . RX_FIFO_UFLOW_STICKY
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FIFO_UFLOW_STICKY(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FIFO_UFLOW_STICKY     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FIFO_UFLOW_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * FIFO overflow interrupt
 * Interrupt occurs whenever there is ANY change in the underlying status
 *
 * \details 
 * 0: No change in overflow status
 * 1: FIFO overflow status changed
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT . RX_FIFO_OFLOW_STICKY
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FIFO_OFLOW_STICKY(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FIFO_OFLOW_STICKY     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_FIFO_OFLOW_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Set by the Rx BER state machine when a high bit error rate condition is
 * detected or cleared
 * Interrupt occurs whenever there is ANY change in the underlying status
 *
 * \details 
 * 0: High BER status unchanged
 * 1: High BER status changed
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT . RX_HI_BER_STICKY
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_HI_BER_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_HI_BER_STICKY     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_INTR_STAT_RX_HI_BER_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief PCS status register
 *
 * \details
 * Contains status information from the PCS core
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_STATUS::PCS_STATUS
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_STATUS(target)  VTSS_IOREG(target,0x11)

/** 
 * \brief
 * When in test pattern check mode, this bit will read 1 if the test
 * pattern checker detects a match. When 0, the test pattern does not
 * match. The test pattern error counts should still be used along with
 * this register bit to determine proper test match status. The bit will
 * read back 1 only when the test pattern is matching. This may happen even
 * while test pattern errors are counted on other clock cycles.
 *
 * \details 
 * 0: Test pattern mismatch
 * 1: Test pattern match
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_STATUS . TESTPAT_MATCH
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_STATUS_TESTPAT_MATCH(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_STATUS_TESTPAT_MATCH     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_STATUS_TESTPAT_MATCH(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * The block_lock status from the synchronization state machine
 *
 * \details 
 * 0: Not synchronized
 * 1: Synchronized, lock obtained
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_STATUS . RX_BLOCK_LOCK
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_STATUS_RX_BLOCK_LOCK(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_STATUS_RX_BLOCK_LOCK     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_STATUS_RX_BLOCK_LOCK(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Set by the Rx BER state machine when a high bit-error-rate condition is
 * detected
 *
 * \details 
 * 0: Normal BER
 * 1: High BER
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_STATUS . RX_HI_BER
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_STATUS_RX_HI_BER(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_STATUS_RX_HI_BER     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_PCS_STATUS_RX_HI_BER(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief PMTICK Tx errored block counter
 *
 * \details
 * Register: \a PCS_10GBASE_R::PCS_10GBR_STATUS::TX_ERRBLK_CNT_PERF
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_TX_ERRBLK_CNT_PERF(target)  VTSS_IOREG(target,0x12)

/** 
 * \brief
 * Count of the errored Tx blocks
 * Register updates every PMTICK
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_TX_ERRBLK_CNT_PERF . TX_ERRBLK_CNT_PERF
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_TX_ERRBLK_CNT_PERF_TX_ERRBLK_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_TX_ERRBLK_CNT_PERF_TX_ERRBLK_CNT_PERF     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_TX_ERRBLK_CNT_PERF_TX_ERRBLK_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief PMTICK invalid character counter
 *
 * \details
 * Register: \a PCS_10GBASE_R::PCS_10GBR_STATUS::TX_CHARERR_CNT_PERF
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_TX_CHARERR_CNT_PERF(target)  VTSS_IOREG(target,0x13)

/** 
 * \brief
 * Register updates every PMTICK
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_TX_CHARERR_CNT_PERF . TX_CHARERR_CNT_PERF
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_TX_CHARERR_CNT_PERF_TX_CHARERR_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_TX_CHARERR_CNT_PERF_TX_CHARERR_CNT_PERF     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_TX_CHARERR_CNT_PERF_TX_CHARERR_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief PMTICK count of added IDLEs
 *
 * \details
 * Register: \a PCS_10GBASE_R::PCS_10GBR_STATUS::TX_ADD_CNT_PERF
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_TX_ADD_CNT_PERF(target)  VTSS_IOREG(target,0x14)

/** 
 * \brief
 * Count of IDLEs inserted
 * Register updates every PMTICK
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_TX_ADD_CNT_PERF . TX_ADD_CNT_PERF
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_TX_ADD_CNT_PERF_TX_ADD_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_TX_ADD_CNT_PERF_TX_ADD_CNT_PERF     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_TX_ADD_CNT_PERF_TX_ADD_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief PMTICK count of IDLEs dropped
 *
 * \details
 * Register: \a PCS_10GBASE_R::PCS_10GBR_STATUS::TX_DROP_CNT_PERF
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_TX_DROP_CNT_PERF(target)  VTSS_IOREG(target,0x15)

/** 
 * \brief
 * Count of dropped IDLEs and ordered sets
 * Register updates every PMTICK
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_TX_DROP_CNT_PERF . TX_DROP_CNT_PERF
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_TX_DROP_CNT_PERF_TX_DROP_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_TX_DROP_CNT_PERF_TX_DROP_CNT_PERF     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_TX_DROP_CNT_PERF_TX_DROP_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief PMTICK BER count
 *
 * \details
 * This is the BER count from IEEE802.3 section 49.2.14.2.
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_STATUS::RX_BER_CNT_PERF
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_RX_BER_CNT_PERF(target)  VTSS_IOREG(target,0x16)

/** 
 * \brief
 * BER count from the BER state machine
 * Register updates every PMTICK
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_RX_BER_CNT_PERF . RX_BER_CNT_PERF
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_RX_BER_CNT_PERF_RX_BER_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_RX_BER_CNT_PERF_RX_BER_CNT_PERF     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_RX_BER_CNT_PERF_RX_BER_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief PMTICK Rx errored block counter
 *
 * \details
 * Register: \a PCS_10GBASE_R::PCS_10GBR_STATUS::RX_ERRBLK_CNT_PERF
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_RX_ERRBLK_CNT_PERF(target)  VTSS_IOREG(target,0x17)

/** 
 * \brief
 * Count of the errored Rx blocks
 * Register updates every PMTICK 
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_RX_ERRBLK_CNT_PERF . RX_ERRBLK_CNT_PERF
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_RX_ERRBLK_CNT_PERF_RX_ERRBLK_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_RX_ERRBLK_CNT_PERF_RX_ERRBLK_CNT_PERF     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_RX_ERRBLK_CNT_PERF_RX_ERRBLK_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief PMTICK invalid character counter
 *
 * \details
 * Register: \a PCS_10GBASE_R::PCS_10GBR_STATUS::RX_CHARERR_CNT_PERF
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_RX_CHARERR_CNT_PERF(target)  VTSS_IOREG(target,0x18)

/** 
 * \brief
 * Register updates every PMTICK
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_RX_CHARERR_CNT_PERF . RX_CHARERR_CNT_PERF
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_RX_CHARERR_CNT_PERF_RX_CHARERR_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_RX_CHARERR_CNT_PERF_RX_CHARERR_CNT_PERF     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_RX_CHARERR_CNT_PERF_RX_CHARERR_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief PMTICK count of added IDLEs
 *
 * \details
 * Register: \a PCS_10GBASE_R::PCS_10GBR_STATUS::RX_ADD_CNT_PERF
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_RX_ADD_CNT_PERF(target)  VTSS_IOREG(target,0x19)

/** 
 * \brief
 * Count of IDLEs inserted
 * Register updates every PMTICK.
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_RX_ADD_CNT_PERF . RX_ADD_CNT_PERF
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_RX_ADD_CNT_PERF_RX_ADD_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_RX_ADD_CNT_PERF_RX_ADD_CNT_PERF     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_RX_ADD_CNT_PERF_RX_ADD_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief PMTICK count of IDLEs dropped
 *
 * \details
 * Register: \a PCS_10GBASE_R::PCS_10GBR_STATUS::RX_DROP_CNT_PERF
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_RX_DROP_CNT_PERF(target)  VTSS_IOREG(target,0x1a)

/** 
 * \brief
 * Count of dropped IDLEs and ordered sets
 * Register updates every PMTICK
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_STATUS_RX_DROP_CNT_PERF . RX_DROP_CNT_PERF
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_STATUS_RX_DROP_CNT_PERF_RX_DROP_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_STATUS_RX_DROP_CNT_PERF_RX_DROP_CNT_PERF     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_STATUS_RX_DROP_CNT_PERF_RX_DROP_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,32)

/**
 * Register Group: \a PCS_10GBASE_R::PCS_10GBR_HA_STATUS
 *
 * PCS status registers and counters
 */


/** 
 * \brief Test pattern mode error counts
 *
 * \details
 * Count of the errors detected in test pattern mode
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_HA_STATUS::TEST_ERR_CNT
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TEST_ERR_CNT(target)  VTSS_IOREG(target,0x1b)

/** 
 * \brief
 * Count of detected test pattern errors in Rx test pattern checker. Write
 * 0 to clear.
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TEST_ERR_CNT . TEST_ERR_CNT
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TEST_ERR_CNT_TEST_ERR_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TEST_ERR_CNT_TEST_ERR_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TEST_ERR_CNT_TEST_ERR_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Tx errored block counter
 *
 * \details
 * Count of the Tx errored blocks
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_HA_STATUS::TX_ERRBLK_CNT
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_ERRBLK_CNT(target)  VTSS_IOREG(target,0x1c)

/** 
 * \brief
 * Count of the errored Tx blocks
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_ERRBLK_CNT . TX_ERRBLK_CNT
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_ERRBLK_CNT_TX_ERRBLK_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_ERRBLK_CNT_TX_ERRBLK_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_ERRBLK_CNT_TX_ERRBLK_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Invalid character counter
 *
 * \details
 * Counts the number of invalid control characters
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_HA_STATUS::TX_CHARERR_CNT
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_CHARERR_CNT(target)  VTSS_IOREG(target,0x1d)

/** 
 * \brief
 * Count of the number of invalid control characters
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_CHARERR_CNT . TX_CHARERR_CNT
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_CHARERR_CNT_TX_CHARERR_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_CHARERR_CNT_TX_CHARERR_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_CHARERR_CNT_TX_CHARERR_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Count of added IDLEs
 *
 * \details
 * Counts the number of added IDLEs
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_HA_STATUS::TX_ADD_CNT
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_ADD_CNT(target)  VTSS_IOREG(target,0x1e)

/** 
 * \brief
 * Count of IDLEs inserted
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_ADD_CNT . TX_ADD_CNT
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_ADD_CNT_TX_ADD_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_ADD_CNT_TX_ADD_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_ADD_CNT_TX_ADD_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Count of IDLEs dropped
 *
 * \details
 * Counts the number of dropped IDLEs
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_HA_STATUS::TX_DROP_CNT
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_DROP_CNT(target)  VTSS_IOREG(target,0x1f)

/** 
 * \brief
 * Count of dropped IDLEs and ordered sets
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_DROP_CNT . TX_DROP_CNT
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_DROP_CNT_TX_DROP_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_DROP_CNT_TX_DROP_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_DROP_CNT_TX_DROP_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief BER count
 *
 * \details
 * BER count from IEEE802.3 section 49.2.14.2.
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_HA_STATUS::RX_BER_CNT
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_BER_CNT(target)  VTSS_IOREG(target,0x20)

/** 
 * \brief
 * BER count from the BER state machine
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_BER_CNT . RX_BER_CNT
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_BER_CNT_RX_BER_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_BER_CNT_RX_BER_CNT     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_BER_CNT_RX_BER_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief Rx errored block counter
 *
 * \details
 * Count of the Rx errored blocks
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_HA_STATUS::RX_ERRBLK_CNT
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_ERRBLK_CNT(target)  VTSS_IOREG(target,0x21)

/** 
 * \brief
 * Count of the errored Rx blocks
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_ERRBLK_CNT . RX_ERRBLK_CNT
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_ERRBLK_CNT_RX_ERRBLK_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_ERRBLK_CNT_RX_ERRBLK_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_ERRBLK_CNT_RX_ERRBLK_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Invalid character counter
 *
 * \details
 * Counts the number of invalid control characters
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_HA_STATUS::RX_CHARERR_CNT
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_CHARERR_CNT(target)  VTSS_IOREG(target,0x22)

/** 
 * \brief
 * Count of the number of invalid control characters
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_CHARERR_CNT . RX_CHARERR_CNT
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_CHARERR_CNT_RX_CHARERR_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_CHARERR_CNT_RX_CHARERR_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_CHARERR_CNT_RX_CHARERR_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Added IDLEs counter
 *
 * \details
 * Counts the number of added IDLEs
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_HA_STATUS::RX_ADD_CNT
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_ADD_CNT(target)  VTSS_IOREG(target,0x23)

/** 
 * \brief
 * Count of IDLEs inserted
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_ADD_CNT . RX_ADD_CNT
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_ADD_CNT_RX_ADD_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_ADD_CNT_RX_ADD_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_ADD_CNT_RX_ADD_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Dropped IDLEs counter
 *
 * \details
 * Counts the number of dropped IDLEs
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_HA_STATUS::RX_DROP_CNT
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_DROP_CNT(target)  VTSS_IOREG(target,0x24)

/** 
 * \brief
 * Count of dropped IDLEs and ordered sets
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_DROP_CNT . RX_DROP_CNT
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_DROP_CNT_RX_DROP_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_DROP_CNT_RX_DROP_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_DROP_CNT_RX_DROP_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Status of Rx sequence ordered set FIFO
 *
 * \details
 * Contains status information for the FIFO containing captured Rx ordered
 * sets
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_HA_STATUS::RX_OSET_FIFO_STAT
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_OSET_FIFO_STAT(target)  VTSS_IOREG(target,0x25)

/** 
 * \brief
 * Indicates if the FIFO is full
 *
 * \details 
 * 0: FIFO not full
 * 1: FIFO full
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_OSET_FIFO_STAT . RX_OSET_FIFO_FULL
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_OSET_FIFO_STAT_RX_OSET_FIFO_FULL(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_OSET_FIFO_STAT_RX_OSET_FIFO_FULL     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_OSET_FIFO_STAT_RX_OSET_FIFO_FULL(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Number of valid ordered sets in the FIFO that can be read
 *
 * \details 
 * Binary number
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_OSET_FIFO_STAT . RX_OSET_FIFO_NUM
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_OSET_FIFO_STAT_RX_OSET_FIFO_NUM(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_OSET_FIFO_STAT_RX_OSET_FIFO_NUM     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_OSET_FIFO_STAT_RX_OSET_FIFO_NUM(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Rx ordered set FIFO data
 *
 * \details
 * The register interface to the ordered set data
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_HA_STATUS::RX_OSET_FIFO_DATA
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_OSET_FIFO_DATA(target)  VTSS_IOREG(target,0x26)

/** 
 * \brief
 * Register interface to the FIFO containing captured ordered sets. Each
 * read of this register pops a 24-bit ordered set off the FIFO and
 * increments the FIFO pointer.
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_OSET_FIFO_DATA . RX_OSET_FIFO_DATA
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_OSET_FIFO_DATA_RX_OSET_FIFO_DATA(x)  VTSS_ENCODE_BITFIELD(x,8,24)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_OSET_FIFO_DATA_RX_OSET_FIFO_DATA     VTSS_ENCODE_BITMASK(8,24)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_OSET_FIFO_DATA_RX_OSET_FIFO_DATA(x)  VTSS_EXTRACT_BITFIELD(x,8,24)


/** 
 * \brief Status of Rx signal ordered set FIFO
 *
 * \details
 * Contains status information for the FIFO containing captured Rx ordered
 * sets
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_HA_STATUS::RX_FSET_FIFO_STAT
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_FSET_FIFO_STAT(target)  VTSS_IOREG(target,0x27)

/** 
 * \brief
 * Indicates if the FIFO is full
 *
 * \details 
 * 0: FIFO not full
 * 1: FIFO full
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_FSET_FIFO_STAT . RX_FSET_FIFO_FULL
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_FSET_FIFO_STAT_RX_FSET_FIFO_FULL(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_FSET_FIFO_STAT_RX_FSET_FIFO_FULL     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_FSET_FIFO_STAT_RX_FSET_FIFO_FULL(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Number of valid ordered sets in the FIFO that can be read
 *
 * \details 
 * Binary number
 *
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_FSET_FIFO_STAT . RX_FSET_FIFO_NUM
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_FSET_FIFO_STAT_RX_FSET_FIFO_NUM(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_FSET_FIFO_STAT_RX_FSET_FIFO_NUM     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_FSET_FIFO_STAT_RX_FSET_FIFO_NUM(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Rx ordered set FIFO data
 *
 * \details
 * The register interface to the ordered set data
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_HA_STATUS::RX_FSET_FIFO_DATA
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_FSET_FIFO_DATA(target)  VTSS_IOREG(target,0x28)

/** 
 * \brief
 * Register interface to the FIFO containing captured ordered sets. Each
 * read of this register pops a 24-bit ordered set off the FIFO and
 * increments the FIFO pointer.
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_FSET_FIFO_DATA . RX_FSET_FIFO_DATA
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_FSET_FIFO_DATA_RX_FSET_FIFO_DATA(x)  VTSS_ENCODE_BITFIELD(x,8,24)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_FSET_FIFO_DATA_RX_FSET_FIFO_DATA     VTSS_ENCODE_BITMASK(8,24)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_FSET_FIFO_DATA_RX_FSET_FIFO_DATA(x)  VTSS_EXTRACT_BITFIELD(x,8,24)


/** 
 * \brief Spare PCS register
 *
 * \details
 * Spare handshake access register
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_HA_STATUS::TX_SPARE_REG
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_SPARE_REG(target)  VTSS_IOREG(target,0x29)

/** 
 * \brief
 * Spare PCS handshake access register
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_SPARE_REG . TX_SPARE_REG
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_SPARE_REG_TX_SPARE_REG(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_SPARE_REG_TX_SPARE_REG     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_TX_SPARE_REG_TX_SPARE_REG(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Spare PCS register
 *
 * \details
 * Spare handshake access register
 *
 * Register: \a PCS_10GBASE_R::PCS_10GBR_HA_STATUS::RX_SPARE_REG
 *
 * @param target A \a ::vtss_target_PCS_10GBASE_R_e target
 */
#define VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_SPARE_REG(target)  VTSS_IOREG(target,0x2a)

/** 
 * \brief
 * Spare PCS handshake access register
 *
 * \details 
 * Field: ::VTSS_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_SPARE_REG . RX_SPARE_REG
 */
#define  VTSS_F_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_SPARE_REG_RX_SPARE_REG(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_SPARE_REG_RX_SPARE_REG     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_10GBASE_R_PCS_10GBR_HA_STATUS_RX_SPARE_REG_RX_SPARE_REG(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


#endif /* _VTSS_DAYTONA_REGS_PCS_10GBASE_R_H_ */
