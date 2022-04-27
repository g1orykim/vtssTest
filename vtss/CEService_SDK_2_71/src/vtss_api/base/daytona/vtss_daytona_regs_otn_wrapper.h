#ifndef _VTSS_DAYTONA_REGS_OTN_WRAPPER_H_
#define _VTSS_DAYTONA_REGS_OTN_WRAPPER_H_

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
 * Target: \a OTN_WRAPPER
 *
 * \see vtss_target_OTN_WRAPPER_e
 *
 * OTN IP wrapper register map
 *
 ***********************************************************************/

/**
 * Register Group: \a OTN_WRAPPER::WRAPPER_GROUP
 *
 * Registers outside XCO2
 */


/** 
 * \brief Wrapper control register
 *
 * \details
 * Register: \a OTN_WRAPPER::WRAPPER_GROUP::CONTROL_REG
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG(target)  VTSS_IOREG(target,0x0)

/** 
 * \brief
 * Selects either the I.4 or the I.7 eFEC.
 * The EXTERNAL_FEC bits in the XCO2_TX_OTN_GLOBAL_CONTROL and
 * XCO2_RX_OTN_GLOBAL_CONTROL registers must be set to use the I.4 or I.7
 * FECs.
 *
 * \details 
 * 0: I.4 eFEC selected
 * 1: I.7 eFEC selected
 *
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG . FEC_SEL
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_FEC_SEL(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_FEC_SEL     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_FEC_SEL(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Allows selection of the GFP data as input to the Tx system side of the
 * otn_mapper core. Used for modes where the GFP bock is active.
 *
 * \details 
 * 0: The normal core data input is used as the tx_sys_data input to the
 * otn_mapper core.
 * 1: The GFP data input is used as the tx_sys_data input to the otn_mapper
 * core.
 *
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG . GFP_SEL
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_GFP_SEL(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_GFP_SEL     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_GFP_SEL(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Enables the system side OTU to ODU converter module.
 *
 * \details 
 * 0: OTU to ODU converter disabled
 * 1: OTU to ODU converter enabled
 *
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG . OTU2ODU_EN
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_OTU2ODU_EN(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_OTU2ODU_EN     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_OTU2ODU_EN(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \details 
 * 000: Clock gapping circuit is in 3808 col/row mode
 * 001: Clock gapping circuit is in 3792 col/row mode
 * 010: Clock gapping circuit is in 3808 col/row mode from SFI4.2
 * 011: Clock gapping circuit is in 3792 col/row mode from SFI4.2
 * 100: Clock gapping circuit is in 3809.75 col/row mode
 * 101: User controlled gap mode
 * All other values are undefined and will cause unpredictable behavior.
 *
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG . GAP_MODE
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_GAP_MODE(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_GAP_MODE     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_GAP_MODE(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \details 
 * 0: Tx overhead output ports driven normally by internal logic.
 * 1: tristate buffers on device pins for all Tx overhead output ports are
 * driven to HI-Z
 *
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG . TX_OH_DISABLE
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_TX_OH_DISABLE(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_TX_OH_DISABLE     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_TX_OH_DISABLE(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \details 
 * 0: Rx overhead output ports driven normally by internal logic.
 * 1: tristate buffers on device pins for all Rx overhead output ports are
 * driven to HI-Z
 *
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG . RX_OH_DISABLE
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_RX_OH_DISABLE(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_RX_OH_DISABLE     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_RX_OH_DISABLE(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \details 
 * 0: Tx overhead port clock is inverted
 * 1: Tx overhead port clock is not inverted
 *
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG . TX_OH_CLK_INV
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_TX_OH_CLK_INV(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_TX_OH_CLK_INV     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_TX_OH_CLK_INV(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \details 
 * 0: Rx overhead port clock is inverted
 * 1: Rx overhead port clock is not inverted
 *
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG . RX_OH_CLK_INV
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_RX_OH_CLK_INV(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_RX_OH_CLK_INV     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_RX_OH_CLK_INV(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \details 
 * 0: Block operates normally
 * 1: All logic other than CSR target is held in reset, clocks are NOT
 * gated
 *
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG . SW_RST
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_SW_RST(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_SW_RST     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_SW_RST(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \details 
 * 0: All clocks for this module (with the exception of the csr_clk) are
 * disabled. XCO2 is held in reset.
 * 1: All clocks for this module are enabled.
 *
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG . CLK_ENA
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_CLK_ENA(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_CLK_ENA     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_CLK_ENA(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \details 
 * 0: Transmit data is processed by the XCO2 module
 * 1: Transmit data bypasses the XCO2 module
 *
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG . TX_BYPASS
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_TX_BYPASS(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_TX_BYPASS     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_TX_BYPASS(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \details 
 * 0: Receive data is processed by the XCO2 module
 * 1: Receive data bypasses the XCO2 module
 *
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG . RX_BYPASS
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_RX_BYPASS(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_RX_BYPASS     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_RX_BYPASS(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Spare register
 *
 * \details
 * 32 bit read/writable spare register
 *
 * Register: \a OTN_WRAPPER::WRAPPER_GROUP::SPARE
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_WRAPPER_GROUP_SPARE(target)  VTSS_IOREG(target,0x3)

/** 
 * \brief
 * 32 bit read/writable spare register
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_SPARE . SPARE
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_SPARE_SPARE(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_SPARE_SPARE     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_SPARE_SPARE(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief XCO2 Tx OTN interrupt register 
 *
 * \details
 * Register: \a OTN_WRAPPER::WRAPPER_GROUP::TX_OTN_INTERRUPT_STATUS
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STATUS(target)  VTSS_IOREG(target,0x4)

/** 
 * \brief
 * FIFO thresh high IRQ
 * The FIFO thresh high IRQ bit is set when the difference between the
 * transmit FIFO read and write pointers exceeds the programmed high
 * threshold value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STATUS . FIFO_THRESH_HIGH_IRQ
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STATUS_FIFO_THRESH_HIGH_IRQ(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STATUS_FIFO_THRESH_HIGH_IRQ     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STATUS_FIFO_THRESH_HIGH_IRQ(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * FIFO thresh low IRQ
 * The FIFO thresh low IRQ bit is set when the difference between the
 * transmit FIFO read and write pointers falls below the programmed low
 * threshold value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STATUS . FIFO_THRESH_LOW_IRQ
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STATUS_FIFO_THRESH_LOW_IRQ(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STATUS_FIFO_THRESH_LOW_IRQ     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STATUS_FIFO_THRESH_LOW_IRQ(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * FIFO full IRQ
 * The FIFO full IRQ bit is set when a transmit FIFO full condition is
 * detected.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STATUS . FIFO_FULL_IRQ
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STATUS_FIFO_FULL_IRQ(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STATUS_FIFO_FULL_IRQ     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STATUS_FIFO_FULL_IRQ(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * LOS detect IRQ
 * The LOS detect IRQ bit is set when an incoming transmit LOS condition is
 * detected.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STATUS . LOS_DETECT_IRQ
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STATUS_LOS_DETECT_IRQ(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STATUS_LOS_DETECT_IRQ     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STATUS_LOS_DETECT_IRQ(x)  VTSS_EXTRACT_BITFIELD(x,7,1)


/** 
 * \brief XCO2 Tx OTN interrupt register 
 *
 * \details
 * Register: \a OTN_WRAPPER::WRAPPER_GROUP::TX_OTN_INTERRUPT_STICKY
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY(target)  VTSS_IOREG(target,0x5)

/** 
 * \brief
 * IAE IRQ
 * The IAE IRQ bit is set when streaming mode is enabled and an incoming
 * alignment error is detected.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY . IAE_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_IAE_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_IAE_IRQ_STICKY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_IAE_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * PJ IRQ
 * The PJ IRQ bit is set when an asynchronous mapping payload type is
 * configured and a positive justification event occurs. This is a sticky
 * bit meaning that once set, it cannot be cleared until a value of 1 is
 * written to it and the interrupt condition no longer exists. This bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY . PJ_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_PJ_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_PJ_IRQ_STICKY     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_PJ_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * NJ IRQ
 * The NJ IRQ bit is set when an asynchronous mapping payload type is
 * configured and a negative justification event occurs. This is a sticky
 * bit meaning that once set, it cannot be cleared until a value of 1 is
 * written to it and the interrupt condition no longer exists. This bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY . NJ_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_NJ_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_NJ_IRQ_STICKY     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_NJ_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * FIFO thresh high IRQ
 * The FIFO thresh high IRQ bit is set when the difference between the
 * transmit FIFO read and write pointers crosses the programmed high
 * threshold value (in either direction). This is a sticky bit meaning that
 * once set, it cannot be cleared until a value of 1 is written to it and
 * the interrupt condition no longer exists. This bit is cleared as a
 * default value.	 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY . FIFO_THRESH_HIGH_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_FIFO_THRESH_HIGH_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_FIFO_THRESH_HIGH_IRQ_STICKY     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_FIFO_THRESH_HIGH_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * FIFO thresh low IRQ
 * The FIFO thresh low IRQ bit is set when the difference between the
 * transmit FIFO read and write pointers crosses the programmed low
 * threshold value (in either direction). This is a sticky bit meaning that
 * once set, it cannot be cleared until a value of 1 is written to it and
 * the interrupt condition no longer exists. This bit is cleared as a
 * default value.	  
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY . FIFO_THRESH_LOW_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_FIFO_THRESH_LOW_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_FIFO_THRESH_LOW_IRQ_STICKY     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_FIFO_THRESH_LOW_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * FIFO overflow IRQ
 * The FIFO overflow IRQ bit is set when a transmit FIFO overflow condition
 * is detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY . FIFO_OVERFLOW_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_FIFO_OVERFLOW_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_FIFO_OVERFLOW_IRQ_STICKY     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_FIFO_OVERFLOW_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * FIFO underflow IRQ
 * The FIFO underflow IRQ bit is set when a transmit FIFO underflow
 * condition is detected. This is a sticky bit meaning that once set, it
 * cannot be cleared until a value of 1 is written to it and the interrupt
 * condition no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY . FIFO_UNDERFLOW_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_FIFO_UNDERFLOW_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_FIFO_UNDERFLOW_IRQ_STICKY     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_FIFO_UNDERFLOW_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * FIFO full IRQ
 * The FIFO full IRQ bit is set when a transmit FIFO full condition is
 * detected or when the full condition goes away. This is a sticky bit
 * meaning that once set, it cannot be cleared until a value of 1 is
 * written to it and the interrupt condition no longer exists. This bit is
 * cleared as a default value.	     
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY . FIFO_FULL_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_FIFO_FULL_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_FIFO_FULL_IRQ_STICKY     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_FIFO_FULL_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * LOS detect IRQ
 * The LOS detect IRQ bit is set when an incoming transmit LOS condition is
 * detected or when this condition goes away. This is a sticky bit meaning
 * that once set, it cannot be cleared until a value of 1 is written to it
 * and the interrupt condition no longer exists. This bit is cleared as a
 * default value.	 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY . LOS_DETECT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_LOS_DETECT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_LOS_DETECT_IRQ_STICKY     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_STICKY_LOS_DETECT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,7,1)


/** 
 * \brief XCO2 Tx OTN interrupt mask register 
 *
 * \details
 * Register: \a OTN_WRAPPER::WRAPPER_GROUP::TX_OTN_INTERRUPT_MASK
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK(target)  VTSS_IOREG(target,0x6)

/** 
 * \brief
 * IAE IRQ mask
 * The IAE IRQ mask bit is set to cause the assertion of the /CIRQ output
 * (active low output set to 0) when an IAE interrupt occurs. If the IAE
 * IRQ mask bit is cleared, an IAE interrupt condition does not cause the
 * assertion of the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK . IAE_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_IAE_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_IAE_IRQ_MASK     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_IAE_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * PJ IRQ mask
 * The PJ IRQ mask bit is set to cause the assertion of the /CIRQ output
 * (active low output set to 0) when a PJ interrupt occurs. If the PJ IRQ
 * mask bit is cleared, a PJ interrupt condition does not cause the
 * assertion of the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK . PJ_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_PJ_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_PJ_IRQ_MASK     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_PJ_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * NJ IRQ mask
 * The NJ IRQ mask bit is set to cause the assertion of the /CIRQ output
 * (active low output set to 0) when a NJ interrupt occurs. If the NJ IRQ
 * mask bit is cleared, an NJ interrupt condition does not cause the
 * assertion of the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK . NJ_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_NJ_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_NJ_IRQ_MASK     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_NJ_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * FIFO thresh high IRQ mask
 * The FIFO thresh high IRQ mask bit is set to cause the assertion of the
 * /CIRQ output (active low output set to 0) when a FIFO threshold high
 * interrupt occurs. If the FIFO thresh high IRQ mask bit is cleared, a
 * FIFO threshold high interrupt condition does not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK . FIFO_THRESH_HIGH_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_FIFO_THRESH_HIGH_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_FIFO_THRESH_HIGH_IRQ_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_FIFO_THRESH_HIGH_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * FIFO thresh low IRQ mask
 * The FIFO thresh low IRQ mask bit is set to cause the assertion of the
 * /CIRQ output (active low output set to 0) when a FIFO threshold low
 * interrupt occurs. If the FIFO thresh low IRQ mask bit is cleared, a FIFO
 * threshold low interrupt condition does not cause the assertion of the
 * /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK . FIFO_THRESH_LOW_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_FIFO_THRESH_LOW_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_FIFO_THRESH_LOW_IRQ_MASK     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_FIFO_THRESH_LOW_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * FIFO overflow IRQ mask
 * The FIFO overflow IRQ mask bit is set to cause the assertion of the
 * /CIRQ output (active low output set to 0) when a FIFO overflow interrupt
 * occurs. If the FIFO overflow IRQ mask bit is cleared, a FIFO overflow
 * interrupt condition does not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK . FIFO_OVERFLOW_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_FIFO_OVERFLOW_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_FIFO_OVERFLOW_IRQ_MASK     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_FIFO_OVERFLOW_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * FIFO underflow IRQ mask
 * The FIFO underflow IRQ mask bit is set to cause the assertion of the
 * /CIRQ output (active low output set to 0) when a FIFO underflow
 * interrupt occurs. If the FIFO underflow IRQ mask bit is cleared, a FIFO
 * underflow interrupt condition does not cause the assertion of the /CIRQ
 * output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK . FIFO_UNDERFLOW_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_FIFO_UNDERFLOW_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_FIFO_UNDERFLOW_IRQ_MASK     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_FIFO_UNDERFLOW_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * FIFO full IRQ mask
 * The FIFO full IRQ mask bit is set to cause the assertion of the /CIRQ
 * output (active low output set to 0) when a FIFO full interrupt occurs.
 * If the FIFO full IRQ mask bit is cleared, a FIFO full interrupt
 * condition does not cause the assertion of the /CIRQ output. This bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK . FIFO_FULL_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_FIFO_FULL_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_FIFO_FULL_IRQ_MASK     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_FIFO_FULL_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * LOS detect IRQ mask
 * The LOS detect IRQ mask bit is set to cause the assertion of the /CIRQ
 * output (active low output set to 0) when a transmit LOS detect interrupt
 * occurs. If the LOS detect IRQ mask bit is cleared, an LOS detect
 * interrupt condition does not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK . LOS_DETECT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_LOS_DETECT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_LOS_DETECT_IRQ_MASK     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_INTERRUPT_MASK_LOS_DETECT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,7,1)


/** 
 * \brief XCO2 Tx OTN ODU TCMX interpret interrupt register 
 *
 * \details
 * Register: \a OTN_WRAPPER::WRAPPER_GROUP::TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY (??), 0-5
 */
#define VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY(target,ri)  VTSS_IOREG(target,0xd + (ri))

/** 
 * \brief
 * TCMi TIM IRQ
 * The TCMi TIM IRQ bit is set when an incoming TCMi TIM error condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY . TCMI_TIM_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_TIM_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_TIM_IRQ_STICKY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_TIM_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * TCMi TTI accept IRQ
 * The TCMi TTI accept IRQ bit is set when an incoming TCMi TTI accept
 * condition is detected. This is a sticky bit meaning that once set, it
 * cannot be cleared until a value of 1 is written to it and the interrupt
 * condition no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY . TCMI_TTI_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_TTI_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_TTI_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_TTI_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * TCMi TTI inconsistent IRQ
 * The TCMi TTI inconsistent IRQ bit is set when an incoming TCMi TTI
 * inconsistent condition is detected. This is a sticky bit meaning that
 * once set, it cannot be cleared until a value of 1 is written to it and
 * the interrupt condition no longer exists. This bit is cleared as a
 * default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY . TCMI_TTI_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_TTI_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_TTI_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_TTI_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * TCMi BIP IRQ
 * The TCMi BIP IRQ bit is set when an incoming TCMi BIP error condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY . TCMI_BIP_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_BIP_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_BIP_IRQ_STICKY     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_BIP_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * TCMi BEI IRQ
 * The TCMi BEI IRQ bit is set when an incoming TCMi BEI error condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY . TCMI_BEI_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_BEI_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_BEI_IRQ_STICKY     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_BEI_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * TCMi BIAE IRQ
 * The TCMi BIAE IRQ bit is set when an incoming TCMi BIAE error condition
 * is detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY . TCMI_BIAE_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_BIAE_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_BIAE_IRQ_STICKY     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_BIAE_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * TCMi BDI IRQ
 * The TCMi BDI IRQ bit is set when an incoming TCMi BDI error condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY . TCMI_BDI_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_BDI_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_BDI_IRQ_STICKY     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_BDI_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * TCMi STAT accept IRQ
 * The TCMi STAT accept IRQ bit is set when an incoming TCMi STAT accept
 * condition is detected. This is a sticky bit meaning that once set, it
 * cannot be cleared until a value of 1 is written to it and the interrupt
 * condition no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY . TCMI_STAT_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_STAT_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_STAT_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_STAT_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * TCMi STAT inconsistent IRQ
 * The TCMi STAT inconsistent IRQ bit is set when an incoming TCMi STAT
 * inconsistent condition is detected. This is a sticky bit meaning that
 * once set, it cannot be cleared until a value of 1 is written to it and
 * the interrupt condition no longer exists. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY . TCMI_STAT_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_STAT_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_STAT_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_STAT_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * TCMi LTC IRQ
 * The TCMi LTC IRQ bit is set when an incoming TCMi LTC error condition is
 * detected (accepted STAT bits => TCMi STAT extract value equals 000).
 * This is a sticky bit meaning that once set, it cannot be cleared until a
 * value of 1 is written to it and the interrupt condition no longer
 * exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY . TCMI_LTC_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_LTC_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_LTC_IRQ_STICKY     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_LTC_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * TCMi LCK IRQ
 * The TCMi LCK IRQ bit is set when an incoming TCMi LCK error condition is
 * detected (accepted STAT bits => TCMi STAT extract value equals 101).
 * This is a sticky bit meaning that once set, it cannot be cleared until a
 * value of 1 is written to it and the interrupt condition no longer
 * exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY . TCMI_LCK_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_LCK_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_LCK_IRQ_STICKY     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_LCK_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * TCMi OCI IRQ
 * The TCMi OCI IRQ bit is set when an incoming TCMi OCI error condition is
 * detected (accepted STAT bits => TCMi STAT extract value equals 110).
 * This is a sticky bit meaning that once set, it cannot be cleared until a
 * value of 1 is written to it and the interrupt condition no longer
 * exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY . TCMI_OCI_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_OCI_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_OCI_IRQ_STICKY     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_OCI_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * TCMi AIS IRQ
 * The TCMi AIS IRQ bit is set when an incoming TCMi AIS error condition is
 * detected (accepted STAT bits => TCMi STAT extract value equals 111).
 * This is a sticky bit meaning that once set, it cannot be cleared until a
 * value of 1 is written to it and the interrupt condition no longer
 * exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY . TCMI_AIS_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_AIS_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_AIS_IRQ_STICKY     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_AIS_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * TCMi IAE IRQ
 * The TCMi IAE IRQ bit is set when an incoming TCMi IAE error condition is
 * detected (accepted STAT bits => TCMi STAT extract value equals 010).
 * This is a sticky bit meaning that once set, it cannot be cleared until a
 * value of 1 is written to it and the interrupt condition no longer
 * exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY . TCMI_IAE_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_IAE_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_IAE_IRQ_STICKY     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_STICKY_TCMI_IAE_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,1,1)


/** 
 * \brief XCO2 Tx OTN ODU TCMX interpret interrupt mask register 
 *
 * \details
 * Register: \a OTN_WRAPPER::WRAPPER_GROUP::TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK (??), 0-5
 */
#define VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK(target,ri)  VTSS_IOREG(target,0x13 + (ri))

/** 
 * \brief
 * TCMi TIM IRQ mask
 * The TCMi TIM IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a TCMi TIM interrupt occurs. If the TCMi TIM IRQ mask bit is
 * cleared, a TCMi TIM interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK . TCMI_TIM_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_TIM_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_TIM_IRQ_MASK     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_TIM_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * TCMi TTI accept IRQ mask
 * The TCMi TTI accept IRQ mask bit is set to cause the assertion of the
 * /CIRQ output when a TCMi TTI accept interrupt occurs. If the TCMi TTI
 * accept IRQ mask bit is cleared, a TCMi TTI accept interrupt condition
 * will not cause the assertion of the /CIRQ output. This bit is cleared as
 * a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK . TCMI_TTI_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_TTI_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_TTI_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_TTI_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * TCMi TTI inconsistent IRQ mask
 * The TCMi TTI inconsistent IRQ mask bit is set to cause the assertion of
 * the /CIRQ output when a TCMi TTI inconsistent interrupt occurs. If the
 * TCMi TTI inconsistent IRQ mask bit is cleared, a TCMi TTI inconsistent
 * interrupt condition will not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK . TCMI_TTI_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_TTI_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_TTI_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_TTI_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * TCMi BIP IRQ mask
 * The TCMi BIP IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a TCMi BIP interrupt occurs. If the TCMi BIP IRQ mask bit is
 * cleared, a TCMi BIP interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK . TCMI_BIP_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_BIP_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_BIP_IRQ_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_BIP_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * TCMi BEI IRQ mask
 * The TCMi BEI IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a TCMi BEI interrupt occurs. If the TCMi BEI IRQ mask bit is
 * cleared, a TCMi BEI interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK . TCMI_BEI_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_BEI_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_BEI_IRQ_MASK     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_BEI_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * TCMi BIAE IRQ mask
 * The TCMi BIAE IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a TCMi BIAE interrupt occurs. If the TCMi BIAE IRQ mask bit
 * is cleared, a TCMi BIAE interrupt condition will not cause the assertion
 * of the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK . TCMI_BIAE_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_BIAE_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_BIAE_IRQ_MASK     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_BIAE_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * TCMi BDI IRQ mask
 * The TCMi BDI IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a TCMi BDI interrupt occurs. If the TCMi BDI IRQ mask bit is
 * cleared, a TCMi BDI interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK . TCMI_BDI_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_BDI_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_BDI_IRQ_MASK     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_BDI_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * TCMi STAT accept IRQ mask
 * The TCMi STAT accept IRQ mask bit is set to cause the assertion of the
 * /CIRQ output when a TCMi STAT accept interrupt occurs. If the TCMi STAT
 * accept IRQ mask bit is cleared, a TCMi STAT accept interrupt condition
 * will not cause the assertion of the /CIRQ output. This bit is cleared as
 * a default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK . TCMI_STAT_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_STAT_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_STAT_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_STAT_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * TCMi STAT inconsistent IRQ mask
 * The TCMi STAT inconsistent IRQ mask bit is set to cause the assertion of
 * the /CIRQ output when a TCMi STAT inconsistent interrupt occurs. If the
 * TCMi STAT inconsistent IRQ mask bit is cleared, a TCMi STAT inconsistent
 * interrupt condition will not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK . TCMI_STAT_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_STAT_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_STAT_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_STAT_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * TCMi LTC IRQ mask
 * The TCMi LTC IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a TCMi LTC interrupt occurs. If the TCMi LTC IRQ mask bit is
 * cleared, a TCMi LTC interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK . TCMI_LTC_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_LTC_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_LTC_IRQ_MASK     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_LTC_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * TCMi LCK IRQ mask
 * The TCMi LCK IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a TCMi LCK interrupt occurs. If the TCMi LCK IRQ mask bit is
 * cleared, a TCMi LCK interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK . TCMI_LCK_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_LCK_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_LCK_IRQ_MASK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_LCK_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * TCMi OCI IRQ mask
 * The TCMi OCI IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a TCMi OCI interrupt occurs. If the TCMi OCI IRQ mask bit is
 * cleared, a TCMi OCI interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK . TCMI_OCI_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_OCI_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_OCI_IRQ_MASK     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_OCI_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * TCMi AIS IRQ mask
 * The TCMi AIS IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a TCMi AIS interrupt occurs. If the TCMi AIS IRQ mask bit is
 * cleared, a TCMi AIS interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK . TCMI_AIS_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_AIS_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_AIS_IRQ_MASK     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_AIS_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * TCMi IAE IRQ mask
 * The TCMi IAE IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a TCMi IAE interrupt occurs. If the TCMi IAE IRQ mask bit is
 * cleared, a TCMi IAE interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK . TCMI_IAE_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_IAE_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_IAE_IRQ_MASK     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_TX_OTN_ODU_TCMX_INTERPRET_INTERRUPT_MASK_TCMI_IAE_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,1,1)


/** 
 * \brief XCO2 Rx OTN FA/FEC interrupt register 
 *
 * \details
 * Register: \a OTN_WRAPPER::WRAPPER_GROUP::RX_OTN_FA_FEC_INTERRUPT_STATUS
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS(target)  VTSS_IOREG(target,0x19)

/** 
 * \brief
 * The LOS IRQ bit is set when an incoming LOS error is detected.	
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS . LOS_IRQ
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS_LOS_IRQ(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS_LOS_IRQ     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS_LOS_IRQ(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * The OOF IRQ bit is set when an incoming OOF error is detected.    
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS . OOF_IRQ
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS_OOF_IRQ(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS_OOF_IRQ     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS_OOF_IRQ(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * The LOA IRQ bit is set when an incoming LOA error is detected.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS . LOA_IRQ
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS_LOA_IRQ(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS_LOA_IRQ     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS_LOA_IRQ(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * The LOF IRQ bit is set when an incoming LOF error is detected.    
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS . LOF_IRQ
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS_LOF_IRQ(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS_LOF_IRQ     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS_LOF_IRQ(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * The OOM IRQ bit is set when an incoming OOM error is detected.	   
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS . OOM_IRQ
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS_OOM_IRQ(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS_OOM_IRQ     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS_OOM_IRQ(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * The LOMA IRQ bit is set when an incoming LOMA error is detected.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS . LOMA_IRQ
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS_LOMA_IRQ(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS_LOMA_IRQ     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS_LOMA_IRQ(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * The LOM IRQ bit is set when an incoming LOM error is detected.    
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS . LOM_IRQ
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS_LOM_IRQ(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS_LOM_IRQ     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS_LOM_IRQ(x)  VTSS_EXTRACT_BITFIELD(x,8,1)


/** 
 * \brief XCO2 Rx OTN FA/FEC interrupt register 
 *
 * \details
 * Register: \a OTN_WRAPPER::WRAPPER_GROUP::RX_OTN_FA_FEC_INTERRUPT_STICKY
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY(target)  VTSS_IOREG(target,0x1a)

/** 
 * \brief
 * LOS IRQ
 * The LOS IRQ bit is set when an incoming LOS error is detected. This is a
 * sticky bit meaning that once set, it cannot be cleared until a value of
 * 1 is written to it and the interrupt condition no longer exists. This
 * bit is cleared as a default value.	 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY . LOS_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_LOS_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_LOS_IRQ_STICKY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_LOS_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * OOF IRQ
 * The OOF IRQ bit is set when an incoming OOF error is detected. This is a
 * sticky bit meaning that once set, it cannot be cleared until a value of
 * 1 is written to it and the interrupt condition no longer exists. This
 * bit is cleared as a default value.	    
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY . OOF_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_OOF_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_OOF_IRQ_STICKY     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_OOF_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * LOA IRQ
 * The LOA IRQ bit is set when an incoming LOA error is detected. This is a
 * sticky bit meaning that once set, it cannot be cleared until a value of
 * 1 is written to it and the interrupt condition no longer exists. This
 * bit is cleared as a default value.	 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY . LOA_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_LOA_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_LOA_IRQ_STICKY     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_LOA_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * LOF IRQ
 * The LOF IRQ bit is set when an incoming LOF error is detected. This is a
 * sticky bit meaning that once set, it cannot be cleared until a value of
 * 1 is written to it and the interrupt condition no longer exists. This
 * bit is cleared as a default value.	 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY . LOF_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_LOF_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_LOF_IRQ_STICKY     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_LOF_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * FAS changed IRQ
 * The FAS changed IRQ bit is set when the incoming FAS changes (different
 * from previous FAS position). This is a sticky bit meaning that once set,
 * it cannot be cleared until a value of 1 is written to it and the
 * interrupt condition no longer exists. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY . FAS_CHANGED_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_FAS_CHANGED_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_FAS_CHANGED_IRQ_STICKY     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_FAS_CHANGED_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * OOM IRQ
 * The OOM IRQ bit is set when an incoming OOM error is detected. This is a
 * sticky bit meaning that once set, it cannot be cleared until a value of
 * 1 is written to it and the interrupt condition no longer exists. This
 * bit is cleared as a default value.	 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY . OOM_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_OOM_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_OOM_IRQ_STICKY     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_OOM_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * LOMA IRQ
 * The LOMA IRQ bit is set when an incoming LOMA error is detected. This is
 * a sticky bit meaning that once set, it cannot be cleared until a value
 * of 1 is written to it and the interrupt condition no longer exists. This
 * bit is cleared as a default value.	 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY . LOMA_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_LOMA_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_LOMA_IRQ_STICKY     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_LOMA_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * LOM IRQ
 * The LOM IRQ bit is set when an incoming LOM error is detected. This is a
 * sticky bit meaning that once set, it cannot be cleared until a value of
 * 1 is written to it and the interrupt condition no longer exists. This
 * bit is cleared as a default value.	 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY . LOM_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_LOM_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_LOM_IRQ_STICKY     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_LOM_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * MF changed IRQ
 * The MF changed IRQ bit is set when a new multiframe (different from
 * previous multiframe sequence) is detected when transitioning into the MF
 * sync state. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY . MF_CHANGED_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_MF_CHANGED_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_MF_CHANGED_IRQ_STICKY     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_MF_CHANGED_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * FEC corrected symbol IRQ
 * The FEC corrected symbol IRQ bit is set when a symbol is corrected by
 * the FEC decoder. This is a sticky bit meaning that once set, it cannot
 * be cleared until a value of 1 is written to it and the interrupt
 * condition no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY . FEC_CORRECTED_SYMBOL_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_FEC_CORRECTED_SYMBOL_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_FEC_CORRECTED_SYMBOL_IRQ_STICKY     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_FEC_CORRECTED_SYMBOL_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * FEC corrected zero IRQ
 * The FEC corrected zero IRQ bit is set when a 0  bit is corrected by the
 * FEC decoder. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY . FEC_CORRECTED_ZERO_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_FEC_CORRECTED_ZERO_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_FEC_CORRECTED_ZERO_IRQ_STICKY     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_FEC_CORRECTED_ZERO_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * FEC corrected one IRQ
 * The FEC corrected one IRQ bit is set when a 1 bit is corrected by the
 * FEC decoder. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY . FEC_CORRECTED_ONE_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_FEC_CORRECTED_ONE_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_FEC_CORRECTED_ONE_IRQ_STICKY     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_FEC_CORRECTED_ONE_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * FEC uncorrected codeword IRQ
 * The FEC uncorrected codeword IRQ bit is set when an uncorrectable
 * codeword is detected by the FEC decoder. This is a sticky bit meaning
 * that once set, it cannot be cleared until a value of 1 is written to it
 * and the interrupt condition no longer exists. This bit is cleared as a
 * default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY . FEC_UNCORRECTED_CODEWORD_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_FEC_UNCORRECTED_CODEWORD_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_FEC_UNCORRECTED_CODEWORD_IRQ_STICKY     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_FEC_UNCORRECTED_CODEWORD_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * FEC BER exceeded IRQ
 * The FEC BER exceeded IRQ bit is set when a BER exceeded condition is
 * detected by the FEC decoder. This is a sticky bit meaning that once set,
 * it cannot be cleared until a value of 1 is written to it and the
 * interrupt condition no longer exists. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY . FEC_BER_EXCEEDED_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_FEC_BER_EXCEEDED_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_FEC_BER_EXCEEDED_IRQ_STICKY     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_FEC_BER_EXCEEDED_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,2,1)


/** 
 * \brief XCO2 Rx OTN FA/FEC interrupt mask register 
 *
 * \details
 * Register: \a OTN_WRAPPER::WRAPPER_GROUP::RX_OTN_FA_FEC_INTERRUPT_MASK
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK(target)  VTSS_IOREG(target,0x1b)

/** 
 * \brief
 * LOS IRQ mask
 * The LOS IRQ mask bit is set to cause the assertion of the /CIRQ output
 * when an LOS interrupt occurs. If the LOS IRQ mask bit is cleared, an LOS
 * interrupt condition will not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK . LOS_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_LOS_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_LOS_IRQ_MASK     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_LOS_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * OOF IRQ mask
 * The OOF IRQ mask bit is set to cause the assertion of the /CIRQ output
 * when an OOF interrupt occurs. If the OOF IRQ mask bit is cleared, an OOF
 * interrupt condition will not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK . OOF_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_OOF_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_OOF_IRQ_MASK     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_OOF_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * LOA IRQ mask
 * The LOA IRQ mask bit is set to cause the assertion of the /CIRQ output
 * when an LOA interrupt occurs. If the LOA IRQ mask bit is cleared, an LOA
 * interrupt condition will not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK . LOA_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_LOA_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_LOA_IRQ_MASK     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_LOA_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * LOF IRQ mask
 * The LOF IRQ mask bit is set to cause the assertion of the /CIRQ output
 * when an LOF interrupt occurs. If the LOF IRQ mask bit is cleared, an LOF
 * interrupt condition will not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK . LOF_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_LOF_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_LOF_IRQ_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_LOF_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * FAS changed IRQ mask
 * The FAS changed IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when an FAS changed interrupt occurs. If the FAS changed IRQ mask
 * bit is cleared, an FAS changed interrupt condition will not cause the
 * assertion of the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK . FAS_CHANGED_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_FAS_CHANGED_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_FAS_CHANGED_IRQ_MASK     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_FAS_CHANGED_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * OOM IRQ mask
 * The OOM IRQ mask bit is set to cause the assertion of the /CIRQ output
 * when an OOM interrupt occurs. If the OOM IRQ mask bit is cleared, an OOM
 * interrupt condition will not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK . OOM_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_OOM_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_OOM_IRQ_MASK     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_OOM_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * LOMA IRQ mask
 * The LOMA IRQ mask bit is set to cause the assertion of the /CIRQ output
 * when an LOMA interrupt occurs. If the LOMA IRQ mask bit is cleared, an
 * LOMA interrupt condition will not cause the assertion of the /CIRQ
 * output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK . LOMA_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_LOMA_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_LOMA_IRQ_MASK     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_LOMA_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * LOM IRQ mask
 * The LOM IRQ mask bit is set to cause the assertion of the /CIRQ output
 * when an LOM interrupt occurs. If the LOM IRQ mask bit is cleared, an LOM
 * interrupt condition will not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK . LOM_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_LOM_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_LOM_IRQ_MASK     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_LOM_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * MF changed IRQ mask
 * The MF changed IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when an MF changed interrupt occurs. If the MF changed IRQ mask
 * bit is cleared, an MF changed interrupt condition will not cause the
 * assertion of the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK . MF_CHANGED_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_MF_CHANGED_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_MF_CHANGED_IRQ_MASK     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_MF_CHANGED_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * FEC corrected symbol IRQ mask
 * The FEC corrected symbol IRQ mask bit is set to cause the assertion of
 * the /CIRQ output when an FEC corrected symbol interrupt occurs. If the
 * FEC corrected symbol IRQ mask bit is cleared, an FEC corrected symbol
 * interrupt condition will not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK . FEC_CORRECTED_SYMBOL_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_FEC_CORRECTED_SYMBOL_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_FEC_CORRECTED_SYMBOL_IRQ_MASK     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_FEC_CORRECTED_SYMBOL_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * FEC corrected zero IRQ mask
 * The FEC corrected zero IRQ mask bit is set to cause the assertion of the
 * /CIRQ output when an FEC corrected zero interrupt occurs. If the FEC
 * corrected zero IRQ mask bit is cleared, an FEC corrected zero interrupt
 * condition will not cause the assertion of the /CIRQ output. This bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK . FEC_CORRECTED_ZERO_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_FEC_CORRECTED_ZERO_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_FEC_CORRECTED_ZERO_IRQ_MASK     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_FEC_CORRECTED_ZERO_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * FEC corrected one IRQ mask
 * The FEC corrected one IRQ mask bit is set to cause the assertion of the
 * /CIRQ output when an FEC corrected one interrupt occurs. If the FEC
 * corrected one IRQ mask bit is cleared, an FEC corrected one interrupt
 * condition will not cause the assertion of the /CIRQ output. This bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK . FEC_CORRECTED_ONE_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_FEC_CORRECTED_ONE_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_FEC_CORRECTED_ONE_IRQ_MASK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_FEC_CORRECTED_ONE_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * FEC uncorrected codeword IRQ mask
 * The FEC uncorrected codeword IRQ mask bit is set to cause the assertion
 * of the /CIRQ output when an FEC uncorrected codeword interrupt occurs.
 * If the FEC uncorrected codeword IRQ mask bit is cleared, an FEC
 * uncorrected codeword interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.	   
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK . FEC_UNCORRECTED_CODEWORD_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_FEC_UNCORRECTED_CODEWORD_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_FEC_UNCORRECTED_CODEWORD_IRQ_MASK     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_FEC_UNCORRECTED_CODEWORD_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * FEC BER exceeded IRQ mask
 * The FEC BER exceeded IRQ mask bit is set to cause the assertion of the
 * /CIRQ output when an FEC BER exceeded interrupt occurs. If the FEC BER
 * exceeded IRQ mask bit is cleared, an FEC BER exceeded interrupt
 * condition will not cause the assertion of the /CIRQ output. This bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK . FEC_BER_EXCEEDED_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_FEC_BER_EXCEEDED_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_FEC_BER_EXCEEDED_IRQ_MASK     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_FEC_BER_EXCEEDED_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,2,1)


/** 
 * \brief XCO2 Rx OTN OTU interrupt register 
 *
 * \details
 * Register: \a OTN_WRAPPER::WRAPPER_GROUP::RX_OTN_OTU_INTERRUPT_STICKY
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY(target)  VTSS_IOREG(target,0x1d)

/** 
 * \brief
 * OTUk AIS IRQ
 * The OTUk AIS IRQ bit is set when an incoming OTUk AIS condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY . OTUK_AIS_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_OTUK_AIS_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_OTUK_AIS_IRQ_STICKY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_OTUK_AIS_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * SM TIM IRQ
 * The SM TIM IRQ bit is set when an incoming SM TIM error condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY . SM_TIM_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_TIM_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_TIM_IRQ_STICKY     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_TIM_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * SM TTI accept IRQ
 * The SM TTI accept IRQ bit is set when an incoming SM TTI accept
 * condition is detected. This is a sticky bit meaning that once set, it
 * cannot be cleared until a value of 1 is written to it and the interrupt
 * condition no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY . SM_TTI_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_TTI_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_TTI_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_TTI_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * SM TTI inconsistent IRQ
 * The SM TTI inconsistent IRQ bit is set when an incoming SM TTI
 * inconsistent condition is detected. This is a sticky bit meaning that
 * once set, it cannot be cleared until a value of 1 is written to it and
 * the interrupt condition no longer exists. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY . SM_TTI_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_TTI_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_TTI_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_TTI_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * SM BIP IRQ
 * The SM BIP IRQ bit is set when an incoming SM BIP error condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY . SM_BIP_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_BIP_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_BIP_IRQ_STICKY     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_BIP_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * SM BEI IRQ
 * The SM BEI IRQ bit is set when an incoming SM BEI error condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY . SM_BEI_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_BEI_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_BEI_IRQ_STICKY     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_BEI_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * SM BIAE IRQ
 * The SM BIAE IRQ bit is set when an incoming SM BIAE error condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY . SM_BIAE_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_BIAE_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_BIAE_IRQ_STICKY     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_BIAE_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * SM BDI IRQ
 * The SM BDI IRQ bit is set when an incoming SM BDI error condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY . SM_BDI_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_BDI_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_BDI_IRQ_STICKY     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_BDI_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * SM IAE IRQ
 * The SM IAE IRQ bit is set when an incoming SM IAE error condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY . SM_IAE_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_IAE_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_IAE_IRQ_STICKY     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_IAE_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * SM RES accept IRQ
 * The SM RES accept IRQ bit is set when an incoming SM RES accept
 * condition is detected. This is a sticky bit meaning that once set, it
 * cannot be cleared until a value of 1 is written to it and the interrupt
 * condition no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY . SM_RES_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_RES_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_RES_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_RES_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * SM RES inconsistent IRQ
 * The SM RES inconsistent IRQ bit is set when an incoming SM RES
 * inconsistent condition is detected. This is a sticky bit meaning that
 * once set, it cannot be cleared until a value of 1 is written to it and
 * the interrupt condition no longer exists. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY . SM_RES_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_RES_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_RES_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_SM_RES_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * GCC0 accept IRQ
 * The GCC0 accept IRQ bit is set when an incoming GCC0 accept condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY . GCC0_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_GCC0_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_GCC0_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_GCC0_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * GCC0 inconsistent IRQ
 * The GCC0 inconsistent IRQ bit is set when an incoming GCC0 inconsistent
 * condition is detected. This is a sticky bit meaning that once set, it
 * cannot be cleared until a value of 1 is written to it and the interrupt
 * condition no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY . GCC0_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_GCC0_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_GCC0_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_GCC0_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * RES accept IRQ
 * The RES accept IRQ bit is set when an incoming RES accept condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY . RES_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_RES_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_RES_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_RES_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * RES inconsistent IRQ
 * The RES0 inconsistent IRQ bit is set when an incoming RES inconsistent
 * condition is detected. This is a sticky bit meaning that once set, it
 * cannot be cleared until a value of 1 is written to it and the interrupt
 * condition no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY . RES_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_RES_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_RES_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY_RES_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,1,1)


/** 
 * \brief XCO2 Rx OTN OTU interrupt mask register 
 *
 * \details
 * Register: \a OTN_WRAPPER::WRAPPER_GROUP::RX_OTN_OTU_INTERRUPT_MASK
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK(target)  VTSS_IOREG(target,0x1e)

/** 
 * \brief
 * OTUk AIS IRQ mask
 * The OTUk AIS IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when an OTUk AIS interrupt occurs. If the OTUk AIS IRQ mask bit
 * is cleared, an OTUk AIS interrupt condition will not cause the assertion
 * of the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK . OTUK_AIS_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_OTUK_AIS_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_OTUK_AIS_IRQ_MASK     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_OTUK_AIS_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * SM TIM IRQ mask
 * The SM TIM IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when an SM TIM interrupt occurs. If the SM TIM IRQ mask bit is
 * cleared, an SM TIM interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK . SM_TIM_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_TIM_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_TIM_IRQ_MASK     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_TIM_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * SM TTI accept IRQ mask
 * The SM TTI accept IRQ mask bit is set to cause the assertion of the
 * /CIRQ output when an SM TTI accept interrupt occurs. If the SM TTI
 * accept IRQ mask bit is cleared, an SM TTI accept interrupt condition
 * will not cause the assertion of the /CIRQ output. This bit is cleared as
 * a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK . SM_TTI_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_TTI_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_TTI_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_TTI_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * SM TTI inconsistent IRQ mask
 * The SM TTI inconsistent IRQ mask bit is set to cause the assertion of
 * the /CIRQ output when an SM TTI inconsistent interrupt occurs. If the SM
 * TTI inconsistent IRQ mask bit is cleared, an SM TTI inconsistent
 * interrupt condition will not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK . SM_TTI_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_TTI_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_TTI_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_TTI_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * SM BIP IRQ mask
 * The SM BIP IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when an SM BIP interrupt occurs. If the SM BIP IRQ mask bit is
 * cleared, an SM BIP interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK . SM_BIP_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_BIP_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_BIP_IRQ_MASK     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_BIP_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * SM BEI IRQ mask
 * The SM BEI IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when an SM BEI interrupt occurs. If the SM BEI IRQ mask bit is
 * cleared, an SM BEI interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK . SM_BEI_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_BEI_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_BEI_IRQ_MASK     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_BEI_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * SM BIAE IRQ mask
 * The SM BIAE IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when an SM BIAE interrupt occurs. If the SM BIAE IRQ mask bit is
 * cleared, an SM BIAE interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK . SM_BIAE_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_BIAE_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_BIAE_IRQ_MASK     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_BIAE_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * SM BDI IRQ mask
 * The SM BDI IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when an SM BDI interrupt occurs. If the SM BDI IRQ mask bit is
 * cleared, an SM BDI interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK . SM_BDI_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_BDI_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_BDI_IRQ_MASK     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_BDI_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * SM IAE IRQ mask
 * The SM IAE IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when an SM IAE interrupt occurs. If the SM IAE IRQ mask bit is
 * cleared, an SM IAE interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.	
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK . SM_IAE_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_IAE_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_IAE_IRQ_MASK     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_IAE_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * SM RES accept IRQ mask
 * The SM RES accept IRQ mask bit is set to cause the assertion of the
 * /CIRQ output when an SM RES accept interrupt occurs. If the SM RES
 * accept IRQ mask bit is cleared, an SM RES accept interrupt condition
 * will not cause the assertion of the /CIRQ output. This bit is cleared as
 * a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK . SM_RES_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_RES_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_RES_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_RES_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * SM RES inconsistent IRQ mask
 * The SM RES inconsistent IRQ mask bit is set to cause the assertion of
 * the /CIRQ output when an SM RES inconsistent interrupt occurs. If the SM
 * RES inconsistent IRQ mask bit is cleared, an SM RES inconsistent
 * interrupt condition will not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK . SM_RES_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_RES_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_RES_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_RES_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * GCC0 accept IRQ mask
 * The GCC0 accept IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a GCC0 accept interrupt occurs. If the GCC0 accept IRQ mask
 * bit is cleared, a GCC0 accept interrupt condition will not cause the
 * assertion of the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK . GCC0_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_GCC0_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_GCC0_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_GCC0_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * GCC0 inconsistent IRQ mask
 * The GCC0 inconsistent IRQ mask bit is set to cause the assertion of the
 * /CIRQ output when a GCC0 inconsistent interrupt occurs. If the GCC0
 * inconsistent IRQ mask bit is cleared, a GCC0 inconsistent interrupt
 * condition will not cause the assertion of the /CIRQ output. This bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK . GCC0_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_GCC0_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_GCC0_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_GCC0_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * RES accept IRQ mask
 * The RES accept IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a RES accept interrupt occurs. If the RES accept IRQ mask
 * bit is cleared, a RES accept interrupt condition will not cause the
 * assertion of the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK . RES_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_RES_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_RES_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_RES_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * RES inconsistent IRQ mask
 * The RES inconsistent IRQ mask bit is set to cause the assertion of the
 * /CIRQ output when a RES inconsistent interrupt occurs. If the RES
 * inconsistent IRQ mask bit is cleared, a RES inconsistent interrupt
 * condition will not cause the assertion of the /CIRQ output. This bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK . RES_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_RES_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_RES_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_RES_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,1,1)


/** 
 * \brief XCO2 Rx OTN ODU interrupt register 
 *
 * \details
 * Register: \a OTN_WRAPPER::WRAPPER_GROUP::RX_OTN_ODU_INTERRUPT_STICKY
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY(target)  VTSS_IOREG(target,0x20)

/** 
 * \brief
 * RES1 accept IRQ
 * The RES1 accept IRQ bit is set when an incoming RES1 accept condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY . RES1_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_RES1_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_RES1_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_RES1_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * RES1 inconsistent IRQ
 * The RES1 inconsistent IRQ bit is set when an incoming RES1 inconsistent
 * condition is detected. This is a sticky bit meaning that once set, it
 * cannot be cleared until a value of 1 is written to it and the interrupt
 * condition no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY . RES1_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_RES1_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_RES1_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_RES1_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * TCM ACT accept IRQ
 * The TCM ACT accept IRQ bit is set when an incoming TCM ACT accept
 * condition is detected. This is a sticky bit meaning that once set, it
 * cannot be cleared until a value of 1 is written to it and the interrupt
 * condition no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY . TCM_ACT_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_TCM_ACT_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_TCM_ACT_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_TCM_ACT_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * TCM ACT inconsistent IRQ
 * The TCM ACT inconsistent IRQ bit is set when an incoming TCM ACT
 * inconsistent condition is detected. This is a sticky bit meaning that
 * once set, it cannot be cleared until a value of 1 is written to it and
 * the interrupt condition no longer exists. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY . TCM_ACT_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_TCM_ACT_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_TCM_ACT_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_TCM_ACT_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * FTFL accept IRQ
 * The FTFL accept IRQ bit is set when an incoming FTFL accept condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY . FTFL_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_FTFL_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_FTFL_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_FTFL_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * FTFL inconsistent IRQ
 * The FTFL inconsistent IRQ bit is set when an incoming FTFL inconsistent
 * condition is detected. This is a sticky bit meaning that once set, it
 * cannot be cleared until a value of 1 is written to it and the interrupt
 * condition no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY . FTFL_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_FTFL_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_FTFL_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_FTFL_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * EXP accept IRQ
 * The EXP accept IRQ bit is set when an incoming EXP accept condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY . EXP_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_EXP_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_EXP_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_EXP_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * EXP inconsistent IRQ
 * The EXP inconsistent IRQ bit is set when an incoming EXP inconsistent
 * condition is detected. This is a sticky bit meaning that once set, it
 * cannot be cleared until a value of 1 is written to it and the interrupt
 * condition no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY . EXP_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_EXP_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_EXP_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_EXP_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * GCC1 accept IRQ
 * The GCC1 accept IRQ bit is set when an incoming GCC1 accept condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY . GCC1_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_GCC1_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_GCC1_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_GCC1_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * GCC1 inconsistent IRQ
 * The GCC1 inconsistent IRQ bit is set when an incoming GCC1 inconsistent
 * condition is detected. This is a sticky bit meaning that once set, it
 * cannot be cleared until a value of 1 is written to it and the interrupt
 * condition no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY . GCC1_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_GCC1_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_GCC1_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_GCC1_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * GCC2 accept IRQ
 * The GCC2 accept IRQ bit is set when an incoming GCC2 accept condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY . GCC2_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_GCC2_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_GCC2_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_GCC2_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * GCC2 inconsistent IRQ
 * The GCC2 inconsistent IRQ bit is set when an incoming GCC2 inconsistent
 * condition is detected. This is a sticky bit meaning that once set, it
 * cannot be cleared until a value of 1 is written to it and the interrupt
 * condition no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY . GCC2_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_GCC2_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_GCC2_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_GCC2_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * RES2 accept IRQ
 * The RES2 accept IRQ bit is set when an incoming RES2 accept condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY . RES2_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_RES2_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_RES2_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_RES2_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * RES2 inconsistent IRQ
 * The RES2 inconsistent IRQ bit is set when an incoming RES2 inconsistent
 * condition is detected. This is a sticky bit meaning that once set, it
 * cannot be cleared until a value of 1 is written to it and the interrupt
 * condition no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY . RES2_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_RES2_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_RES2_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY_RES2_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,2,1)


/** 
 * \brief XCO2 Rx OTN ODU interrupt mask register 
 *
 * \details
 * Register: \a OTN_WRAPPER::WRAPPER_GROUP::RX_OTN_ODU_INTERRUPT_MASK
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK(target)  VTSS_IOREG(target,0x21)

/** 
 * \brief
 * RES1 accept IRQ mask
 * The RES1 accept IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a RES1 accept interrupt occurs. If the RES1 accept IRQ mask
 * bit is cleared, a RES1 accept interrupt condition will not cause the
 * assertion of the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK . RES1_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_RES1_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_RES1_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_RES1_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * RES1 inconsistent IRQ mask
 * The RES1 inconsistent IRQ mask bit is set to cause the assertion of the
 * /CIRQ output when a RES1 inconsistent interrupt occurs. If the RES1
 * inconsistent IRQ mask bit is cleared, a RES1 inconsistent interrupt
 * condition will not cause the assertion of the /CIRQ output. This bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK . RES1_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_RES1_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_RES1_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_RES1_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * TCM ACT accept IRQ mask
 * The TCM ACT accept IRQ mask bit is set to cause the assertion of the
 * /CIRQ output when a TCM ACT accept interrupt occurs. If the TCM ACT
 * accept IRQ mask bit is cleared, a TCM ACT accept interrupt condition
 * will not cause the assertion of the /CIRQ output. This bit is cleared as
 * a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK . TCM_ACT_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_TCM_ACT_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_TCM_ACT_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_TCM_ACT_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * TCM ACT inconsistent IRQ mask
 * The TCM ACT inconsistent IRQ mask bit is set to cause the assertion of
 * the /CIRQ output when a TCM ACT inconsistent interrupt occurs. If the
 * TCM ACT inconsistent IRQ mask bit is cleared, a TCMACT inconsistent
 * interrupt condition will not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK . TCM_ACT_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_TCM_ACT_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_TCM_ACT_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_TCM_ACT_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * FTFL accept IRQ mask
 * The FTFL accept IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a FTFL accept interrupt occurs. If the FTFL accept IRQ mask
 * bit is cleared, a FTFL accept interrupt condition will not cause the
 * assertion of the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK . FTFL_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_FTFL_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_FTFL_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_FTFL_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * FTFL inconsistent IRQ mask
 * The FTFL inconsistent IRQ mask bit is set to cause the assertion of the
 * /CIRQ output when a FTFL inconsistent interrupt occurs. If the FTFL
 * inconsistent IRQ mask bit is cleared, a FTFL inconsistent interrupt
 * condition will not cause the assertion of the /CIRQ output. This bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK . FTFL_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_FTFL_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_FTFL_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_FTFL_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * EXP accept IRQ mask
 * The EXP accept IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when an EXP accept interrupt occurs. If the EXP accept IRQ mask
 * bit is cleared, a EXP accept interrupt condition will not cause the
 * assertion of the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK . EXP_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_EXP_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_EXP_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_EXP_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * EXP inconsistent IRQ mask
 * The EXP inconsistent IRQ mask bit is set to cause the assertion of the
 * /CIRQ output when an EXP inconsistent interrupt occurs. If the EXP
 * inconsistent IRQ mask bit is cleared, a EXP inconsistent interrupt
 * condition will not cause the assertion of the /CIRQ output. This bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK . EXP_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_EXP_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_EXP_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_EXP_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * GCC1 accept IRQ mask
 * The GCC1 accept IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a GCC1 accept interrupt occurs. If the GCC1 accept IRQ mask
 * bit is cleared, a GCC1 accept interrupt condition will not cause the
 * assertion of the /CIRQ output. This bit is cleared as a default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK . GCC1_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_GCC1_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_GCC1_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_GCC1_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * GCC1 inconsistent IRQ mask
 * The GCC1 inconsistent IRQ mask bit is set to cause the assertion of the
 * /CIRQ output when a GCC1 inconsistent interrupt occurs. If the GCC1
 * inconsistent IRQ mask bit is cleared, a GCC1 inconsistent interrupt
 * condition will not cause the assertion of the /CIRQ output. This bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK . GCC1_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_GCC1_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_GCC1_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_GCC1_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * GCC2 accept IRQ mask
 * The GCC2 accept IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a GCC2 accept interrupt occurs. If the GCC2 accept IRQ mask
 * bit is cleared, a GCC2 accept interrupt condition will not cause the
 * assertion of the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK . GCC2_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_GCC2_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_GCC2_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_GCC2_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * GCC2 inconsistent IRQ mask
 * The GCC2 inconsistent IRQ mask bit is set to cause the assertion of the
 * /CIRQ output when a GCC2 inconsistent interrupt occurs. If the GCC2
 * inconsistent IRQ mask bit is cleared, a GCC2 inconsistent interrupt
 * condition will not cause the assertion of the /CIRQ output. This bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK . GCC2_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_GCC2_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_GCC2_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_GCC2_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * RES2 accept IRQ mask
 * The RES2 accept IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a RES2 accept interrupt occurs. If the RES2 accept IRQ mask
 * bit is cleared, a RES2 accept interrupt condition will not cause the
 * assertion of the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK . RES2_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_RES2_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_RES2_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_RES2_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * RES2 inconsistent IRQ mask
 * The RES2 inconsistent IRQ mask bit is set to cause the assertion of the
 * /CIRQ output when a RES2 inconsistent interrupt occurs. If the RES2
 * inconsistent IRQ mask bit is cleared, a RES2 inconsistent interrupt
 * condition will not cause the assertion of the /CIRQ output. This bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK . RES2_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_RES2_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_RES2_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_RES2_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,2,1)


/** 
 * \brief XCO2 Rx OTN ODU APS/PCC IRQ register 
 *
 * \details
 * Register: \a OTN_WRAPPER::WRAPPER_GROUP::RX_OTN_ODU_APS_PCC_IRQ_STICKY
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY(target)  VTSS_IOREG(target,0x23)

/** 
 * \brief
 * APSPCC MFAS7 accept IRQ
 * The APS/PCC MFAS7 accept IRQ bit is set when an incoming APS/PCC MFAS7
 * accept condition is detected. This is a sticky bit meaning that once
 * set, it cannot be cleared until a value of 1 is written to it and the
 * interrupt condition no longer exists. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY . APSPCC_MFAS7_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS7_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS7_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS7_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * APSPCC MFAS6 accept IRQ
 * The APS/PCC MFAS6 accept IRQ bit is set when an incoming APS/PCC MFAS6
 * accept condition is detected. This is a sticky bit meaning that once
 * set, it cannot be cleared until a value of 1 is written to it and the
 * interrupt condition no longer exists. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY . APSPCC_MFAS6_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS6_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS6_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS6_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * APSPCC MFAS5 accept IRQ
 * The APS/PCC MFAS5 accept IRQ bit is set when an incoming APS/PCC MFAS5
 * accept condition is detected. This is a sticky bit meaning that once
 * set, it cannot be cleared until a value of 1 is written to it and the
 * interrupt condition no longer exists. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY . APSPCC_MFAS5_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS5_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS5_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS5_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * APSPCC MFAS4 accept IRQ
 * The APS/PCC MFAS4 accept IRQ bit is set when an incoming APS/PCC MFAS4
 * accept condition is detected. This is a sticky bit meaning that once
 * set, it cannot be cleared until a value of 1 is written to it and the
 * interrupt condition no longer exists. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY . APSPCC_MFAS4_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS4_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS4_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS4_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * APSPCC MFAS3 accept IRQ
 * The APS/PCC MFAS3 accept IRQ bit is set when an incoming APS/PCC MFAS3
 * accept condition is detected. This is a sticky bit meaning that once
 * set, it cannot be cleared until a value of 1 is written to it and the
 * interrupt condition no longer exists. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY . APSPCC_MFAS3_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS3_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS3_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS3_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * APSPCC MFAS2 accept IRQ
 * The APS/PCC MFAS2 accept IRQ bit is set when an incoming APS/PCC MFAS2
 * accept condition is detected. This is a sticky bit meaning that once
 * set, it cannot be cleared until a value of 1 is written to it and the
 * interrupt condition no longer exists. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY . APSPCC_MFAS2_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS2_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS2_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS2_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * APSPCC MFAS1 accept IRQ
 * The APS/PCC MFAS1 accept IRQ bit is set when an incoming APS/PCC MFAS1
 * accept condition is detected. This is a sticky bit meaning that once
 * set, it cannot be cleared until a value of 1 is written to it and the
 * interrupt condition no longer exists. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY . APSPCC_MFAS1_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS1_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS1_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS1_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * APSPCC MFAS0 accept IRQ
 * The APS/PCC MFAS0 accept IRQ bit is set when an incoming APS/PCC MFAS0
 * accept condition is detected. This is a sticky bit meaning that once
 * set, it cannot be cleared until a value of 1 is written to it and the
 * interrupt condition no longer exists. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY . APSPCC_MFAS0_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS0_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS0_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS0_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * APSPCC MFAS7 inconsistent IRQ
 * The APS/PCC MFAS7 inconsistent IRQ bit is set when an incoming APS/PCC
 * MFAS7 inconsistent condition is detected. This is a sticky bit meaning
 * that once set, it cannot be cleared until a value of 1 is written to it
 * and the interrupt condition no longer exists. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY . APSPCC_MFAS7_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS7_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS7_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS7_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * APSPCC MFAS6 inconsistent IRQ
 * The APS/PCC MFAS6 inconsistent IRQ bit is set when an incoming APS/PCC
 * MFAS6 inconsistent condition is detected. This is a sticky bit meaning
 * that once set, it cannot be cleared until a value of 1 is written to it
 * and the interrupt condition no longer exists. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY . APSPCC_MFAS6_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS6_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS6_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS6_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * APSPCC MFAS5 inconsistent IRQ
 * The APS/PCC MFAS5 inconsistent IRQ bit is set when an incoming APS/PCC
 * MFAS5 inconsistent condition is detected. This is a sticky bit meaning
 * that once set, it cannot be cleared until a value of 1 is written to it
 * and the interrupt condition no longer exists. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY . APSPCC_MFAS5_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS5_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS5_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS5_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * APSPCC MFAS4 inconsistent IRQ
 * The APS/PCC MFAS4 inconsistent IRQ bit is set when an incoming APS/PCC
 * MFAS4 inconsistent condition is detected. This is a sticky bit meaning
 * that once set, it cannot be cleared until a value of 1 is written to it
 * and the interrupt condition no longer exists. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY . APSPCC_MFAS4_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS4_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS4_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS4_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * APSPCC MFAS3 inconsistent IRQ
 * The APS/PCC MFAS3 inconsistent IRQ bit is set when an incoming APS/PCC
 * MFAS3 inconsistent condition is detected. This is a sticky bit meaning
 * that once set, it cannot be cleared until a value of 1 is written to it
 * and the interrupt condition no longer exists. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY . APSPCC_MFAS3_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS3_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS3_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS3_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * APSPCC MFAS2 inconsistent IRQ
 * The APS/PCC MFAS2 inconsistent IRQ bit is set when an incoming APS/PCC
 * MFAS2 inconsistent condition is detected. This is a sticky bit meaning
 * that once set, it cannot be cleared until a value of 1 is written to it
 * and the interrupt condition no longer exists. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY . APSPCC_MFAS2_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS2_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS2_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS2_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * APSPCC MFAS1 inconsistent IRQ
 * The APS/PCC MFAS1 inconsistent IRQ bit is set when an incoming APS/PCC
 * MFAS1 inconsistent condition is detected. This is a sticky bit meaning
 * that once set, it cannot be cleared until a value of 1 is written to it
 * and the interrupt condition no longer exists. This bit is cleared as a
 * default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY . APSPCC_MFAS1_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS1_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS1_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS1_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * APSPCC MFAS0 inconsistent IRQ
 * The APS/PCC MFAS0 inconsistent IRQ bit is set when an incoming APS/PCC
 * MFAS0 inconsistent condition is detected. This is a sticky bit meaning
 * that once set, it cannot be cleared until a value of 1 is written to it
 * and the interrupt condition no longer exists. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY . APSPCC_MFAS0_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS0_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS0_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY_APSPCC_MFAS0_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief XCO2 Rx OTN ODU APS/PCC IRQ mask register 
 *
 * \details
 * Register: \a OTN_WRAPPER::WRAPPER_GROUP::RX_OTN_ODU_APS_PCC_IRQ_MASK
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK(target)  VTSS_IOREG(target,0x24)

/** 
 * \brief
 * APSPCC MFAS7 accept IRQ mask
 * The APS/PCC MFAS7 accept IRQ mask bit is set to cause the assertion of
 * the /CIRQ output when an APS/PCC MFAS7 accept interrupt occurs. If the
 * APS/PCC MFAS7 accept IRQ mask bit is cleared, an APS/PCC MFAS7 accept
 * interrupt condition will not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK . APSPCC_MFAS7_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS7_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS7_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS7_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * APSPCC MFAS6 accept IRQ mask
 * The APS/PCC MFAS6 accept IRQ mask bit is set to cause the assertion of
 * the /CIRQ output when an APS/PCC MFAS6 accept interrupt occurs. If the
 * APS/PCC MFAS6 accept IRQ mask bit is cleared, an APS/PCC MFAS6 accept
 * interrupt condition will not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK . APSPCC_MFAS6_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS6_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS6_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS6_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * APSPCC MFAS5 accept IRQ mask
 * The APS/PCC MFAS5 accept IRQ mask bit is set to cause the assertion of
 * the /CIRQ output when an APS/PCC MFAS5 accept interrupt occurs. If the
 * APS/PCC MFAS5 accept IRQ mask bit is cleared, an APS/PCC MFAS5 accept
 * interrupt condition will not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK . APSPCC_MFAS5_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS5_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS5_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS5_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * APSPCC MFAS4 accept IRQ mask
 * The APS/PCC MFAS4 accept IRQ mask bit is set to cause the assertion of
 * the /CIRQ output when an APS/PCC MFAS4 accept interrupt occurs. If the
 * APS/PCC MFAS4 accept IRQ mask bit is cleared, an APS/PCC MFAS4 accept
 * interrupt condition will not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK . APSPCC_MFAS4_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS4_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS4_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS4_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * APSPCC MFAS3 accept IRQ mask
 * The APS/PCC MFAS3 accept IRQ mask bit is set to cause the assertion of
 * the /CIRQ output when an APS/PCC MFAS3 accept interrupt occurs. If the
 * APS/PCC MFAS3 accept IRQ mask bit is cleared, an APS/PCC MFAS3 accept
 * interrupt condition will not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK . APSPCC_MFAS3_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS3_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS3_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS3_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * APSPCC MFAS2 accept IRQ mask
 * The APS/PCC MFAS2 accept IRQ mask bit is set to cause the assertion of
 * the /CIRQ output when an APS/PCC MFAS2 accept interrupt occurs. If the
 * APS/PCC MFAS2 accept IRQ mask bit is cleared, an APS/PCC MFAS2 accept
 * interrupt condition will not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK . APSPCC_MFAS2_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS2_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS2_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS2_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * APSPCC MFAS1 accept IRQ mask
 * The APS/PCC MFAS1 accept IRQ mask bit is set to cause the assertion of
 * the /CIRQ output when an APS/PCC MFAS1 accept interrupt occurs. If the
 * APS/PCC MFAS1 accept IRQ mask bit is cleared, an APS/PCC MFAS1 accept
 * interrupt condition will not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK . APSPCC_MFAS1_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS1_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS1_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS1_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * APSPCC MFAS0 accept IRQ mask
 * The APS/PCC MFAS0 accept IRQ mask bit is set to cause the assertion of
 * the /CIRQ output when an APS/PCC MFAS0 accept interrupt occurs. If the
 * APS/PCC MFAS0 accept IRQ mask bit is cleared, an APS/PCC MFAS0 accept
 * interrupt condition will not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK . APSPCC_MFAS0_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS0_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS0_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS0_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * APSPCC MFAS7 inconsistent IRQ mask
 * The APS/PCC MFAS7 inconsistent IRQ mask bit is set to cause the
 * assertion of the /CIRQ output when an APS/PCC MFAS7 inconsistent
 * interrupt occurs. If the APS/PCC MFAS7 inconsistent IRQ mask bit is
 * cleared, an APS/PCC MFAS7 inconsistent interrupt condition will not
 * cause the assertion of the /CIRQ output. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK . APSPCC_MFAS7_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS7_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS7_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS7_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * APSPCC MFAS6 inconsistent IRQ mask
 * The APS/PCC MFAS6 inconsistent IRQ mask bit is set to cause the
 * assertion of the /CIRQ output when an APS/PCC MFAS6 inconsistent
 * interrupt occurs. If the APS/PCC MFAS6 inconsistent IRQ mask bit is
 * cleared, an APS/PCC MFAS6 inconsistent interrupt condition will not
 * cause the assertion of the /CIRQ output. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK . APSPCC_MFAS6_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS6_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS6_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS6_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * APSPCC MFAS5 inconsistent IRQ mask
 * The APS/PCC MFAS5 inconsistent IRQ mask bit is set to cause the
 * assertion of the /CIRQ output when an APS/PCC MFAS5 inconsistent
 * interrupt occurs. If the APS/PCC MFAS5 inconsistent IRQ mask bit is
 * cleared, an APS/PCC MFAS5 inconsistent interrupt condition will not
 * cause the assertion of the /CIRQ output. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK . APSPCC_MFAS5_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS5_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS5_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS5_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * APSPCC MFAS4 inconsistent IRQ mask
 * The APS/PCC MFAS4 inconsistent IRQ mask bit is set to cause the
 * assertion of the /CIRQ output when an APS/PCC MFAS4 inconsistent
 * interrupt occurs. If the APS/PCC MFAS4 inconsistent IRQ mask bit is
 * cleared, an APS/PCC MFAS4 inconsistent interrupt condition will not
 * cause the assertion of the /CIRQ output. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK . APSPCC_MFAS4_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS4_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS4_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS4_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * APSPCC MFAS3 inconsistent IRQ mask
 * The APS/PCC MFAS3 inconsistent IRQ mask bit is set to cause the
 * assertion of the /CIRQ output when an APS/PCC MFAS3 inconsistent
 * interrupt occurs. If the APS/PCC MFAS3 inconsistent IRQ mask bit is
 * cleared, an APS/PCC MFAS3 inconsistent interrupt condition will not
 * cause the assertion of the /CIRQ output. This bit is cleared as a
 * default value.     
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK . APSPCC_MFAS3_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS3_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS3_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS3_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * APSPCC MFAS2 inconsistent IRQ mask
 * The APS/PCC MFAS2 inconsistent IRQ mask bit is set to cause the
 * assertion of the /CIRQ output when an APS/PCC MFAS2 inconsistent
 * interrupt occurs. If the APS/PCC MFAS2 inconsistent IRQ mask bit is
 * cleared, an APS/PCC MFAS2 inconsistent interrupt condition will not
 * cause the assertion of the /CIRQ output. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK . APSPCC_MFAS2_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS2_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS2_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS2_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * APSPCC MFAS1 inconsistent IRQ mask
 * The APS/PCC MFAS1 inconsistent IRQ mask bit is set to cause the
 * assertion of the /CIRQ output when an APS/PCC MFAS1 inconsistent
 * interrupt occurs. If the APS/PCC MFAS1 inconsistent IRQ mask bit is
 * cleared, an APS/PCC MFAS1 inconsistent interrupt condition will not
 * cause the assertion of the /CIRQ output. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK . APSPCC_MFAS1_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS1_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS1_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS1_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * APSPCC MFAS0 inconsistent IRQ mask
 * The APS/PCC MFAS0 inconsistent IRQ mask bit is set to cause the
 * assertion of the /CIRQ output when an APS/PCC MFAS0 inconsistent
 * interrupt occurs. If the APS/PCC MFAS0 inconsistent IRQ mask bit is
 * cleared, an APS/PCC MFAS0 inconsistent interrupt condition will not
 * cause the assertion of the /CIRQ output. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK . APSPCC_MFAS0_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS0_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS0_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS0_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief XCO2 Rx OTN ODU TCMX interrupt register 
 *
 * \details
 * Register: \a OTN_WRAPPER::WRAPPER_GROUP::RX_OTN_ODU_TCMX_INTERRUPT_STICKY
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: RX_OTN_ODU_TCMX_INTERRUPT_STICKY (??), 0-5
 */
#define VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY(target,ri)  VTSS_IOREG(target,0x2b + (ri))

/** 
 * \brief
 * TCMi TIM IRQ
 * The TCMi TIM IRQ bit is set when an incoming TCMi TIM error condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY . RX_TCMI_TIM_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_TIM_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_TIM_IRQ_STICKY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_TIM_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * TCMi TTI accept IRQ
 * The TCMi TTI accept IRQ bit is set when an incoming TCMi TTI accept
 * condition is detected. This is a sticky bit meaning that once set, it
 * cannot be cleared until a value of 1 is written to it and the interrupt
 * condition no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY . RX_TCMI_TTI_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_TTI_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_TTI_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_TTI_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * TCMi TTI inconsistent IRQ
 * The TCMi TTI inconsistent IRQ bit is set when an incoming TCMi TTI
 * inconsistent condition is detected. This is a sticky bit meaning that
 * once set, it cannot be cleared until a value of 1 is written to it and
 * the interrupt condition no longer exists. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY . RX_TCMI_TTI_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_TTI_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_TTI_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_TTI_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * TCMi BIP IRQ
 * The TCMi BIP IRQ bit is set when an incoming TCMi BIP error condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY . RX_TCMI_BIP_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_BIP_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_BIP_IRQ_STICKY     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_BIP_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * TCMi BEI IRQ
 * The TCMi BEI IRQ bit is set when an incoming TCMi BEI error condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY . RX_TCMI_BEI_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_BEI_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_BEI_IRQ_STICKY     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_BEI_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * TCMi BIAE IRQ
 * The TCMi BIAE IRQ bit is set when an incoming TCMi BIAE error condition
 * is detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY . RX_TCMI_BIAE_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_BIAE_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_BIAE_IRQ_STICKY     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_BIAE_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * TCMi BDI IRQ
 * The TCMi BDI IRQ bit is set when an incoming TCMi BDI error condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY . RX_TCMI_BDI_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_BDI_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_BDI_IRQ_STICKY     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_BDI_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * TCMi STAT accept IRQ
 * The TCMi STAT accept IRQ bit is set when an incoming TCMi STAT accept
 * condition is detected. This is a sticky bit meaning that once set, it
 * cannot be cleared until a value of 1 is written to it and the interrupt
 * condition no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY . RX_TCMI_STAT_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_STAT_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_STAT_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_STAT_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * TCMi STAT inconsistent IRQ
 * The TCMi STAT inconsistent IRQ bit is set when an incoming TCMi STAT
 * inconsistent condition is detected. This is a sticky bit meaning that
 * once set, it cannot be cleared until a value of 1 is written to it and
 * the interrupt condition no longer exists. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY . RX_TCMI_STAT_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_STAT_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_STAT_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_STAT_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * TCMi LTC IRQ
 * The TCMi LTC IRQ bit is set when an incoming TCMi LTC error condition is
 * detected (accepted STAT bits => TCMi STAT extract value equals 000).
 * This is a sticky bit meaning that once set, it cannot be cleared until a
 * value of 1 is written to it and the interrupt condition no longer
 * exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY . RX_TCMI_LTC_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_LTC_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_LTC_IRQ_STICKY     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_LTC_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * TCMi LCK IRQ
 * The TCMi LCK IRQ bit is set when an incoming TCMi LCK error condition is
 * detected (accepted STAT bits => TCMi STAT extract value equals 101).
 * This is a sticky bit meaning that once set, it cannot be cleared until a
 * value of 1 is written to it and the interrupt condition no longer
 * exists. This bit is cleared as a default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY . RX_TCMI_LCK_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_LCK_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_LCK_IRQ_STICKY     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_LCK_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * TCMi OCI IRQ
 * The TCMi OCI IRQ bit is set when an incoming TCMi OCI error condition is
 * detected (accepted STAT bits => TCMi STAT extract value equals 110).
 * This is a sticky bit meaning that once set, it cannot be cleared until a
 * value of 1 is written to it and the interrupt condition no longer
 * exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY . RX_TCMI_OCI_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_OCI_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_OCI_IRQ_STICKY     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_OCI_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * TCMi AIS IRQ
 * The TCMi AIS IRQ bit is set when an incoming TCMi AIS error condition is
 * detected (accepted STAT bits => TCMi STAT extract value equals 111).
 * This is a sticky bit meaning that once set, it cannot be cleared until a
 * value of 1 is written to it and the interrupt condition no longer
 * exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY . RX_TCMI_AIS_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_AIS_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_AIS_IRQ_STICKY     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_AIS_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * TCMi IAE IRQ
 * The TCMi IAE IRQ bit is set when an incoming TCMi IAE error condition is
 * detected (accepted STAT bits => TCMi STAT extract value equals 010).
 * This is a sticky bit meaning that once set, it cannot be cleared until a
 * value of 1 is written to it and the interrupt condition no longer
 * exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY . RX_TCMI_IAE_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_IAE_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_IAE_IRQ_STICKY     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY_RX_TCMI_IAE_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,1,1)


/** 
 * \brief XCO2 Rx OTN ODU TCMX interrupt mask register 
 *
 * \details
 * Register: \a OTN_WRAPPER::WRAPPER_GROUP::RX_OTN_ODU_TCMX_INTERRUPT_MASK
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: RX_OTN_ODU_TCMX_INTERRUPT_MASK (??), 0-5
 */
#define VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK(target,ri)  VTSS_IOREG(target,0x31 + (ri))

/** 
 * \brief
 * TCMi TIM IRQ mask
 * The TCMi TIM IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a TCMi TIM interrupt occurs. If the TCMi TIM IRQ mask bit is
 * cleared, a TCMi TIM interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK . RX_TCMI_TIM_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_TIM_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_TIM_IRQ_MASK     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_TIM_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * TCMi TTI accept IRQ mask
 * The TCMi TTI accept IRQ mask bit is set to cause the assertion of the
 * /CIRQ output when a TCMi TTI accept interrupt occurs. If the TCMi TTI
 * accept IRQ mask bit is cleared, a TCMi TTI accept interrupt condition
 * will not cause the assertion of the /CIRQ output. This bit is cleared as
 * a default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK . RX_TCMI_TTI_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_TTI_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_TTI_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_TTI_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * TCMi TTI inconsistent IRQ mask
 * The TCMi TTI inconsistent IRQ mask bit is set to cause the assertion of
 * the /CIRQ output when a TCMi TTI inconsistent interrupt occurs. If the
 * TCMi TTI inconsistent IRQ mask bit is cleared, a TCMi TTI inconsistent
 * interrupt condition will not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK . RX_TCMI_TTI_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_TTI_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_TTI_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_TTI_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * TCMi BIP IRQ mask
 * The TCMi BIP IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a TCMi BIP interrupt occurs. If the TCMi BIP IRQ mask bit is
 * cleared, a TCMi BIP interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK . RX_TCMI_BIP_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_BIP_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_BIP_IRQ_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_BIP_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * TCMi BEI IRQ mask
 * The TCMi BEI IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a TCMi BEI interrupt occurs. If the TCMi BEI IRQ mask bit is
 * cleared, a TCMi BEI interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK . RX_TCMI_BEI_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_BEI_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_BEI_IRQ_MASK     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_BEI_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * TCMi BIAE IRQ mask
 * The TCMi BIAE IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a TCMi BIAE interrupt occurs. If the TCMi BIAE IRQ mask bit
 * is cleared, a TCMi BIAE interrupt condition will not cause the assertion
 * of the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK . RX_TCMI_BIAE_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_BIAE_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_BIAE_IRQ_MASK     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_BIAE_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * TCMi BDI IRQ mask
 * The TCMi BDI IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a TCMi BDI interrupt occurs. If the TCMi BDI IRQ mask bit is
 * cleared, a TCMi BDI interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK . RX_TCMI_BDI_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_BDI_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_BDI_IRQ_MASK     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_BDI_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * TCMi STAT accept IRQ mask
 * The TCMi STAT accept IRQ mask bit is set to cause the assertion of the
 * /CIRQ output when a TCMi STAT accept interrupt occurs. If the TCMi STAT
 * accept IRQ mask bit is cleared, a TCMi STAT accept interrupt condition
 * will not cause the assertion of the /CIRQ output. This bit is cleared as
 * a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK . RX_TCMI_STAT_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_STAT_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_STAT_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_STAT_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * TCMi STAT inconsistent IRQ mask
 * The TCMi STAT inconsistent IRQ mask bit is set to cause the assertion of
 * the /CIRQ output when a TCMi STAT inconsistent interrupt occurs. If the
 * TCMi STAT inconsistent IRQ mask bit is cleared, a TCMi STAT inconsistent
 * interrupt condition will not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK . RX_TCMI_STAT_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_STAT_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_STAT_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_STAT_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * TCMi LTC IRQ mask
 * The TCMi LTC IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a TCMi LTC interrupt occurs. If the TCMi LTC IRQ mask bit is
 * cleared, a TCMi LTC interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK . RX_TCMI_LTC_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_LTC_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_LTC_IRQ_MASK     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_LTC_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * TCMi LCK IRQ mask
 * The TCMi LCK IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a TCMi LCK interrupt occurs. If the TCMi LCK IRQ mask bit is
 * cleared, a TCMi LCK interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK . RX_TCMI_LCK_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_LCK_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_LCK_IRQ_MASK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_LCK_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * TCMi OCI IRQ mask
 * The TCMi OCI IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a TCMi OCI interrupt occurs. If the TCMi OCI IRQ mask bit is
 * cleared, a TCMi OCI interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK . RX_TCMI_OCI_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_OCI_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_OCI_IRQ_MASK     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_OCI_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * TCMi AIS IRQ mask
 * The TCMi AIS IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a TCMi AIS interrupt occurs. If the TCMi AIS IRQ mask bit is
 * cleared, a TCMi AIS interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK . RX_TCMI_AIS_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_AIS_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_AIS_IRQ_MASK     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_AIS_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * TCMi IAE IRQ mask
 * The TCMi IAE IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a TCMi IAE interrupt occurs. If the TCMi IAE IRQ mask bit is
 * cleared, a TCMi IAE interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK . RX_TCMI_IAE_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_IAE_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_IAE_IRQ_MASK     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_IAE_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,1,1)


/** 
 * \brief XCO2 Rx OTN ODU PM interrupt register 
 *
 * \details
 * Register: \a OTN_WRAPPER::WRAPPER_GROUP::RX_OTN_ODU_PM_INTERRUPT_STICKY
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY(target)  VTSS_IOREG(target,0x38)

/** 
 * \brief
 * PM TIM IRQ
 * The PM TIM IRQ bit is set when an incoming PM TIM error condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY . PM_TIM_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_TIM_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_TIM_IRQ_STICKY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_TIM_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * PM TTI accept IRQ
 * The PM TTI accept IRQ bit is set when an incoming PM TTI accept
 * condition is detected. This is a sticky bit meaning that once set, it
 * cannot be cleared until a value of 1 is written to it and the interrupt
 * condition no longer exists. This bit is cleared as a default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY . PM_TTI_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_TTI_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_TTI_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_TTI_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * PM TTI inconsistent IRQ
 * The PM TTI inconsistent IRQ bit is set when an incoming PM TTI
 * inconsistent condition is detected. This is a sticky bit meaning that
 * once set, it cannot be cleared until a value of 1 is written to it and
 * the interrupt condition no longer exists. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY . PM_TTI_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_TTI_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_TTI_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_TTI_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * PM BIP IRQ
 * The PM BIP IRQ bit is set when an incoming PM BIP error condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY . PM_BIP_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_BIP_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_BIP_IRQ_STICKY     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_BIP_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * PM BEI IRQ
 * The PM BEI IRQ bit is set when an incoming PM BEI error condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY . PM_BEI_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_BEI_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_BEI_IRQ_STICKY     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_BEI_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * PM BDI IRQ
 * The PM BDI IRQ bit is set when an incoming PM BDI error condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY . PM_BDI_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_BDI_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_BDI_IRQ_STICKY     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_BDI_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * PM STAT accept IRQ
 * The PM STAT accept IRQ bit is set when an incoming PM STAT accept
 * condition is detected. This is a sticky bit meaning that once set, it
 * cannot be cleared until a value of 1 is written to it and the interrupt
 * condition no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY . PM_STAT_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_STAT_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_STAT_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_STAT_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * PM STAT inconsistent IRQ
 * The PM STAT inconsistent IRQ bit is set when an incoming PM STAT
 * inconsistent condition is detected. This is a sticky bit meaning that
 * once set, it cannot be cleared until a value of 1 is written to it and
 * the interrupt condition no longer exists. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY . PM_STAT_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_STAT_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_STAT_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_STAT_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * PM LCK IRQ
 * The PM LCK IRQ bit is set when an incoming PM LCK error condition is
 * detected (accepted STAT bits => PM STAT extract value equals 101). This
 * is a sticky bit meaning that once set, it cannot be cleared until a
 * value of 1 is written to it and the interrupt condition no longer
 * exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY . PM_LCK_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_LCK_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_LCK_IRQ_STICKY     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_LCK_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * PM OCI IRQ
 * The PM OCI IRQ bit is set when an incoming PM OCI error condition is
 * detected (accepted STAT bits => PM STAT extract value equals 110). This
 * is a sticky bit meaning that once set, it cannot be cleared until a
 * value of 1 is written to it and the interrupt condition no longer
 * exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY . PM_OCI_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_OCI_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_OCI_IRQ_STICKY     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_OCI_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * PM AIS IRQ
 * The PM AIS IRQ bit is set when an incoming PM AIS error condition is
 * detected (accepted STAT bits => PM STAT extract value equals 111). This
 * is a sticky bit meaning that once set, it cannot be cleared until a
 * value of 1 is written to it and the interrupt condition no longer
 * exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY . PM_AIS_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_AIS_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_AIS_IRQ_STICKY     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY_PM_AIS_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,2,1)


/** 
 * \brief XCO2 Rx OTN ODU PM interrupt mask register 
 *
 * \details
 * Register: \a OTN_WRAPPER::WRAPPER_GROUP::RX_OTN_ODU_PM_INTERRUPT_MASK
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK(target)  VTSS_IOREG(target,0x39)

/** 
 * \brief
 * PM TIM IRQ mask
 * The PM TIM IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a PM TIM interrupt occurs. If the PM TIM IRQ mask bit is
 * cleared, a PM TIM interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK . PM_TIM_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_TIM_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_TIM_IRQ_MASK     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_TIM_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * PM TTI accept IRQ mask
 * The PM TTI accept IRQ mask bit is set to cause the assertion of the
 * /CIRQ output when a PM TTI accept interrupt occurs. If the PM TTI accept
 * IRQ mask bit is cleared, a PM TTI accept interrupt condition will not
 * cause the assertion of the /CIRQ output. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK . PM_TTI_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_TTI_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_TTI_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_TTI_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * PM TTI inconsistent IRQ mask
 * The PM TTI inconsistent IRQ mask bit is set to cause the assertion of
 * the /CIRQ output when a PM TTI inconsistent interrupt occurs. If the PM
 * TTI inconsistent IRQ mask bit is cleared, a PM TTI inconsistent
 * interrupt condition will not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK . PM_TTI_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_TTI_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_TTI_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_TTI_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * PM BIP IRQ mask
 * The PM BIP IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a PM BIP interrupt occurs. If the PM BIP IRQ mask bit is
 * cleared, a PM BIP interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK . PM_BIP_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_BIP_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_BIP_IRQ_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_BIP_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * PM BEI IRQ mask
 * The PM BEI IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a PM BEI interrupt occurs. If the PM BEI IRQ mask bit is
 * cleared, a PM BEI interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK . PM_BEI_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_BEI_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_BEI_IRQ_MASK     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_BEI_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * PM BDI IRQ mask
 * The PM BDI IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a PM BDI interrupt occurs. If the PM BDI IRQ mask bit is
 * cleared, a PM BDI interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK . PM_BDI_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_BDI_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_BDI_IRQ_MASK     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_BDI_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * PM STAT accept IRQ mask
 * The PM STAT accept IRQ mask bit is set to cause the assertion of the
 * /CIRQ output when a PM STAT accept interrupt occurs. If the PM STAT
 * accept IRQ mask bit is cleared, a PM STAT accept interrupt condition
 * will not cause the assertion of the /CIRQ output. This bit is cleared as
 * a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK . PM_STAT_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_STAT_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_STAT_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_STAT_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * PM STAT inconsistent IRQ mask
 * The PM STAT inconsistent IRQ mask bit is set to cause the assertion of
 * the /CIRQ output when a PM STAT inconsistent interrupt occurs. If the PM
 * STAT inconsistent IRQ mask bit is cleared, a PM STAT inconsistent
 * interrupt condition will not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK . PM_STAT_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_STAT_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_STAT_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_STAT_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * PM LCK IRQ mask
 * The PM LCK IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a PM LCK interrupt occurs. If the PM LCK IRQ mask bit is
 * cleared, a PM LCK interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK . PM_LCK_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_LCK_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_LCK_IRQ_MASK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_LCK_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * PM OCI IRQ mask
 * The PM OCI IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a PM OCI interrupt occurs. If the PM OCI IRQ mask bit is
 * cleared, a PM OCI interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK . PM_OCI_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_OCI_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_OCI_IRQ_MASK     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_OCI_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * PM AIS IRQ mask
 * The PM AIS IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a PM AIS interrupt occurs. If the PM AIS IRQ mask bit is
 * cleared, a PM AIS interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK . PM_AIS_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_AIS_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_AIS_IRQ_MASK     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_AIS_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,2,1)


/** 
 * \brief XCO2 Rx OTN OPU interrupt register 
 *
 * \details
 * Register: \a OTN_WRAPPER::WRAPPER_GROUP::RX_OTN_OPU_INTERRUPT_STICKY
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY(target)  VTSS_IOREG(target,0x3b)

/** 
 * \brief
 * PLM IRQ
 * The PLM IRQ bit is set when the incoming accepted PT value does not
 * match the expected PT value programmed into the Rx OTN OPU PT control
 * register. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY . RX_PLM_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_PLM_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_PLM_IRQ_STICKY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_PLM_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * PT accept IRQ
 * The PT accept IRQ bit is set when an incoming PT accept condition is
 * detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY . RX_PT_ACCEPT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_PT_ACCEPT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_PT_ACCEPT_IRQ_STICKY     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_PT_ACCEPT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * PT inconsistent IRQ
 * The PT inconsistent IRQ bit is set when an incoming PT inconsistent
 * condition is detected. This is a sticky bit meaning that once set, it
 * cannot be cleared until a value of 1 is written to it and the interrupt
 * condition no longer exists. This bit is cleared as a default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY . RX_PT_INCONSISTENT_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_PT_INCONSISTENT_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_PT_INCONSISTENT_IRQ_STICKY     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_PT_INCONSISTENT_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Client generic AIS IRQ
 * The client generic AIS IRQ bit is set when an incoming client generic
 * AIS condition is detected. This is a sticky bit meaning that once set,
 * it cannot be cleared until a value of 1 is written to it and the
 * interrupt condition no longer exists. This bit is cleared as a default
 * value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY . RX_CLIENT_GENERIC_AIS_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_CLIENT_GENERIC_AIS_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_CLIENT_GENERIC_AIS_IRQ_STICKY     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_CLIENT_GENERIC_AIS_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * FIFO thresh high IRQ
 * The FIFO thresh high IRQ bit is set when the difference between the
 * receive FIFO read and write pointers exceeds the programmed high
 * threshold value. This is a sticky bit meaning that once set, it cannot
 * be cleared until a value of 1 is written to it and the interrupt
 * condition no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY . RX_FIFO_THRESH_HIGH_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_FIFO_THRESH_HIGH_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_FIFO_THRESH_HIGH_IRQ_STICKY     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_FIFO_THRESH_HIGH_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * FIFO thresh low IRQ
 * The FIFO thresh low IRQ bit is set when the difference between the
 * receive FIFO read and write pointers falls below the programmed low
 * threshold value. This is a sticky bit meaning that once set, it cannot
 * be cleared until a value of 1 is written to it and the interrupt
 * condition no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY . RX_FIFO_THRESH_LOW_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_FIFO_THRESH_LOW_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_FIFO_THRESH_LOW_IRQ_STICKY     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_FIFO_THRESH_LOW_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * FIFO overflow IRQ
 * The FIFO overflow IRQ bit is set when a transmit FIFO overflow condition
 * is detected. This is a sticky bit meaning that once set, it cannot be
 * cleared until a value of 1 is written to it and the interrupt condition
 * no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY . RX_FIFO_OVERFLOW_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_FIFO_OVERFLOW_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_FIFO_OVERFLOW_IRQ_STICKY     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_FIFO_OVERFLOW_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * FIFO underflow IRQ
 * The FIFO underflow IRQ bit is set when a transmit FIFO underflow
 * condition is detected. This is a sticky bit meaning that once set, it
 * cannot be cleared until a value of 1 is written to it and the interrupt
 * condition no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY . RX_FIFO_UNDERFLOW_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_FIFO_UNDERFLOW_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_FIFO_UNDERFLOW_IRQ_STICKY     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_FIFO_UNDERFLOW_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Client fixed pyld IRQ
 * The client fixed pyld IRQ bit is set when the received OTN payload
 * matches the expected fixed payload value contained in the Rx OTN OPU
 * fixed payload expect registers. This is a sticky bit meaning that once
 * set, it cannot be cleared until a value of 1 is written to it and the
 * interrupt condition no longer exists. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY . RX_CLIENT_FIXED_PYLD_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_CLIENT_FIXED_PYLD_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_CLIENT_FIXED_PYLD_IRQ_STICKY     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_CLIENT_FIXED_PYLD_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Client PN23 pyld IRQ
 * The client PN23 pyld IRQ bit is set when the pseudo random PN23 pattern
 * is detected for an incoming payload. This is a sticky bit meaning that
 * once set, it cannot be cleared until a value of 1 is written to it and
 * the interrupt condition no longer exists. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY . RX_CLIENT_PN23_PYLD_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_CLIENT_PN23_PYLD_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_CLIENT_PN23_PYLD_IRQ_STICKY     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_CLIENT_PN23_PYLD_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Client PN31 pyld IRQ
 * The client PN31 pyld IRQ bit is set when the pseudo random PN31 pattern
 * is detected for an incoming payload. This is a sticky bit meaning that
 * once set, it cannot be cleared until a value of 1 is written to it and
 * the interrupt condition no longer exists. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY . RX_CLIENT_PN31_PYLD_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_CLIENT_PN31_PYLD_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_CLIENT_PN31_PYLD_IRQ_STICKY     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_CLIENT_PN31_PYLD_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * PJ IRQ
 * The PJ IRQ bit is set when a positive justification is detected for an
 * incoming payload. This is a sticky bit meaning that once set, it cannot
 * be cleared until a value of 1 is written to it and the interrupt
 * condition no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY . RX_PJ_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_PJ_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_PJ_IRQ_STICKY     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_PJ_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * NJ IRQ
 * The NJ IRQ bit is set when a negative justification is detected for an
 * incoming payload. This is a sticky bit meaning that once set, it cannot
 * be cleared until a value of 1 is written to it and the interrupt
 * condition no longer exists. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY . RX_NJ_IRQ_STICKY
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_NJ_IRQ_STICKY(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_NJ_IRQ_STICKY     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY_RX_NJ_IRQ_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,3,1)


/** 
 * \brief XCO2 Rx OTN OPU interrupt mask register 
 *
 * \details
 * Register: \a OTN_WRAPPER::WRAPPER_GROUP::RX_OTN_OPU_INTERRUPT_MASK
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK(target)  VTSS_IOREG(target,0x3c)

/** 
 * \brief
 * PLM IRQ mask
 * The PLM IRQ mask bit is set to cause the assertion of the /CIRQ output
 * when a PLM interrupt occurs. If the PLM IRQ mask bit is cleared, a PLM
 * interrupt condition will not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK . RX_PLM_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_PLM_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_PLM_IRQ_MASK     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_PLM_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * PT accept IRQ mask
 * The PT accept IRQ mask bit is set to cause the assertion of the /CIRQ
 * output when a PT accept interrupt occurs. If the PT accept IRQ mask bit
 * is cleared, a PT accept interrupt condition will not cause the assertion
 * of the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK . RX_PT_ACCEPT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_PT_ACCEPT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_PT_ACCEPT_IRQ_MASK     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_PT_ACCEPT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * PT inconsistent IRQ mask
 * The PT inconsistent IRQ mask bit is set to cause the assertion of the
 * /CIRQ output when a PT inconsistent interrupt occurs. If the PT
 * inconsistent IRQ mask bit is cleared, a PT inconsistent interrupt
 * condition will not cause the assertion of the /CIRQ output. This bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK . RX_PT_INCONSISTENT_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_PT_INCONSISTENT_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_PT_INCONSISTENT_IRQ_MASK     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_PT_INCONSISTENT_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Client generic AIS IRQ mask
 * The client generic AIS IRQ mask bit is set to cause the assertion of the
 * /CIRQ output when a client generic AIS interrupt occurs. If the client
 * generic AIS IRQ mask bit is cleared, a client generic AIS interrupt
 * condition will not cause the assertion of the /CIRQ output. This bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK . RX_CLIENT_GENERIC_AIS_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_CLIENT_GENERIC_AIS_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_CLIENT_GENERIC_AIS_IRQ_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_CLIENT_GENERIC_AIS_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * FIFO thresh high IRQ mask
 * The FIFO thresh high IRQ mask bit is set to cause the assertion of the
 * /CIRQ output (active low output set to 0) when a FIFO threshold high
 * interrupt occurs. If the FIFO thresh high IRQ mask bit is cleared, a
 * FIFO threshold high interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK . RX_FIFO_THRESH_HIGH_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_FIFO_THRESH_HIGH_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_FIFO_THRESH_HIGH_IRQ_MASK     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_FIFO_THRESH_HIGH_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * FIFO thresh low IRQ mask
 * The FIFO thresh low IRQ mask bit is set to cause the assertion of the
 * /CIRQ output (active low output set to 0) when a FIFO threshold low
 * interrupt occurs. If the FIFO thresh low IRQ mask bit is cleared, a FIFO
 * threshold low interrupt condition will not cause the assertion of the
 * /CIRQ output. This bit is cleared as a default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK . RX_FIFO_THRESH_LOW_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_FIFO_THRESH_LOW_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_FIFO_THRESH_LOW_IRQ_MASK     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_FIFO_THRESH_LOW_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * FIFO overflow IRQ mask
 * The FIFO overflow IRQ mask bit is set to cause the assertion of the
 * /CIRQ output (active low output set to 0) when a FIFO overflow interrupt
 * occurs. If the FIFO overflow IRQ mask bit is cleared, a FIFO overflow
 * interrupt condition will not cause the assertion of the /CIRQ output.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK . RX_FIFO_OVERFLOW_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_FIFO_OVERFLOW_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_FIFO_OVERFLOW_IRQ_MASK     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_FIFO_OVERFLOW_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * FIFO underflow IRQ mask
 * The FIFO underflow IRQ mask bit is set to cause the assertion of the
 * /CIRQ output (active low output set to 0) when a FIFO underflow
 * interrupt occurs. If the FIFO underflow IRQ mask bit is cleared, a FIFO
 * underflow interrupt condition will not cause the assertion of the /CIRQ
 * output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK . RX_FIFO_UNDERFLOW_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_FIFO_UNDERFLOW_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_FIFO_UNDERFLOW_IRQ_MASK     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_FIFO_UNDERFLOW_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Client fixed pyld IRQ mask
 * The client fixed pyld IRQ mask bit is set to cause the assertion of the
 * /CIRQ output (active low output set to 0) when a client fixed payload
 * interrupt occurs. If the client fixed pyld IRQ mask bit is cleared, a
 * client fixed payload interrupt condition will not cause the assertion of
 * the /CIRQ output. This bit is cleared as a default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK . RX_CLIENT_FIXED_PYLD_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_CLIENT_FIXED_PYLD_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_CLIENT_FIXED_PYLD_IRQ_MASK     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_CLIENT_FIXED_PYLD_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Client PN23 pyld IRQ mask
 * The client PN23 pyld IRQ mask bit is set to cause the assertion of the
 * /CIRQ output (active low output set to 0) when a client PN23 payload
 * interrupt occurs. If the client PN23 IRQ mask bit is cleared, a client
 * PN23 payload interrupt condition will not cause the assertion of the
 * /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK . RX_CLIENT_PN23_PYLD_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_CLIENT_PN23_PYLD_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_CLIENT_PN23_PYLD_IRQ_MASK     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_CLIENT_PN23_PYLD_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Client PN31 pyld IRQ mask
 * The client PN31 pyld IRQ mask bit is set to cause the assertion of the
 * /CIRQ output (active low output set to 0) when a client PN31 payload
 * interrupt occurs. If the client PN31 IRQ mask bit is cleared, a client
 * PN31 payload interrupt condition will not cause the assertion of the
 * /CIRQ output. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK . RX_CLIENT_PN31_PYLD_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_CLIENT_PN31_PYLD_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_CLIENT_PN31_PYLD_IRQ_MASK     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_CLIENT_PN31_PYLD_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * PJ IRQ mask
 * The PJ IRQ mask bit is set to cause the assertion of the /CIRQ output
 * (active low output set to 0) when a client payload positive
 * justification interrupt occurs. If the PJ IRQ mask bit is cleared, a
 * client payload positive justification interrupt condition will not cause
 * the assertion of the /CIRQ output. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK . RX_PJ_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_PJ_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_PJ_IRQ_MASK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_PJ_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * NJ IRQ mask
 * The NJ IRQ mask bit is set to cause the assertion of the /CIRQ output
 * (active low output set to 0) when a client payload negative
 * justification interrupt occurs. If the NJ IRQ mask bit is cleared, a
 * client payload negative justification interrupt condition will not cause
 * the assertion of the /CIRQ output. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK . RX_NJ_IRQ_MASK
 */
#define  VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_NJ_IRQ_MASK(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_NJ_IRQ_MASK     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_NJ_IRQ_MASK(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/**
 * Register Group: \a OTN_WRAPPER::XCO2_VERSION
 *
 * XCO2 version and revision register
 */


/** 
 * \brief XCO2 version register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_VERSION::XCO2_VERSION
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_VERSION_XCO2_VERSION(target)  VTSS_IOREG(target,0x800)

/** 
 * \brief
 * Version.
 * The version bits identifies the version of the released core.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_VERSION_XCO2_VERSION . VERSION
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_VERSION_XCO2_VERSION_VERSION(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_OTN_WRAPPER_XCO2_VERSION_XCO2_VERSION_VERSION     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_OTN_WRAPPER_XCO2_VERSION_XCO2_VERSION_VERSION(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Revision.
 * The revision bits indicate the revision of the core version specified.
 * NOTE: The value of this field is configured by Xelic when the core
 * database is delivered. Contact Xelic for the exact register field
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_VERSION_XCO2_VERSION . REVISION
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_VERSION_XCO2_VERSION_REVISION(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_OTN_WRAPPER_XCO2_VERSION_XCO2_VERSION_REVISION     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_OTN_WRAPPER_XCO2_VERSION_XCO2_VERSION_REVISION(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a OTN_WRAPPER::XCO2_TX_OTN
 *
 * XCO2 OTN block Tx registers
 */


/** 
 * \brief XCO2 Tx OTN global control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_GLOBAL_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL(target)  VTSS_IOREG(target,0x801)

/** 
 * \brief
 * Streaming mode
 * If the streaming mode bit is set, the XCO2 Tx processor synchronizes to
 * the TX SYS FAS IN and TX SYS MFAS IN input signals and accepts streaming
 * ODU frame information. If the streaming mode bit is cleared, normal mode
 * is enabled and payload information is received from the transmit system
 * side interface. Streaming mode implies ODUk frames are received at the
 * transmit system side interface and synchronized to the incoming TX SYS
 * FAS IN and TX SYS MFAS IN input signals. This bit is cleared as a
 * default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL . STREAMING_MODE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_STREAMING_MODE(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_STREAMING_MODE     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_STREAMING_MODE(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Bypass mode
 * If the bypass mode bit is set, the XCO2 Tx processor outputs data
 * directly from the system side FIFO without transmit processor OTN frame
 * processing of any kind. If the bypass mode bit is cleared, normal
 * transmit processing occurs. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL . BYPASS_MODE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_BYPASS_MODE(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_BYPASS_MODE     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_BYPASS_MODE(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Software reset
 * If the sw reset bit is set, the XCO2 Tx processor performs a synchronous
 * reset operation. This state is equivalent to an external hardware reset
 * of the device. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL . SW_RESET
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_SW_RESET(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_SW_RESET     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_SW_RESET(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Scramble enable
 * If the scramble enable bit is set, outgoing data is scrambled as per the
 * G.709 standard. If this bit is cleared, outgoing data scrambling is
 * inhibited. This bit is set as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL . SCRAMBLE_ENABLE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_SCRAMBLE_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_SCRAMBLE_ENABLE     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_SCRAMBLE_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Scramble corrupt
 * If the scramble corrupt bit is set, outgoing scrambled data is corrupted
 * by inverting the OPU scramble sequence value contained in the first row
 * and 96th column of generated OTN frames. If this bit is cleared, normal
 * OTN frame scrambling is enabled. This bit is cleared as a default value.
 * NOTE: scramble corrupt functionality is only valid when the scramble
 * enable bit is set. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL . SCRAMBLE_CORRUPT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_SCRAMBLE_CORRUPT(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_SCRAMBLE_CORRUPT     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_SCRAMBLE_CORRUPT(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * FIFO reset
 * If the FIFO reset bit is set, the XCO2 core resets the transmit
 * processor FIFO read pointer. When the FIFO is reset, the read pointer
 * (line side of FIFO) address is reset to a value that is a distance (set
 * through design package) typically equal to half the depth of the FIFO
 * away from the write pointer. This bit is auto cleared when set and it is
 * cleared as a default value.	  
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL . FIFO_RESET
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_FIFO_RESET(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_FIFO_RESET     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_FIFO_RESET(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * OTU frame type
 * If the OTU frame type bit is set, OTUk frames are generated with FEC
 * information inserted as outlined by the G.709 specification. If the OTU
 * frame type bit is cleared, ODUk frames are generated with no FEC
 * information inserted. This bit is set as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL . OTU_FRAME_TYPE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_OTU_FRAME_TYPE(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_OTU_FRAME_TYPE     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_OTU_FRAME_TYPE(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * FEC blanking
 * If the FEC blanking bit is set, all inserted FEC byte information is
 * cleared (0x00 hex) and inserted into outgoing ODU frames. If the FEC
 * blanking bit is cleared, normal FEC insertion is enabled. This bit is
 * only valid when the OTU frame type bit is set. This bit is cleared as a
 * default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL . FEC_BLANKING
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_FEC_BLANKING(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_FEC_BLANKING     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_FEC_BLANKING(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * External FEC
 * The external FEC bit is set to enable external FEC processing using the
 * XCO2 external FEC interface signaling inputs/outputs. This bit is
 * cleared to enable internal Reed Solomon RS(255,239) forward error
 * correction processing as outlined in the G.709 specification. This bit
 * is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL . EXTERNAL_FEC
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_EXTERNAL_FEC(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_EXTERNAL_FEC     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_EXTERNAL_FEC(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Scramble pre FEC
 * The scramble pre FEC bit is set to enable frame scrambling prior to FEC
 * encoding. This operation is required for some enhanced FEC modes
 * (enabled through external FEC register bit) of operation. The scramble
 * pre FEC bit is cleared to enable frame scrambling after FEC encoding.
 * This scramble pre FEC bit is cleared for normal Reed Solomon RS(255,239)
 * forward error correction processing. The scramble pre FEC bit is valid
 * only when the external FEC bit is set. This bit is cleared as a default
 * value.   
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL . SCRAMBLE_PRE_FEC
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_SCRAMBLE_PRE_FEC(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_SCRAMBLE_PRE_FEC     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_SCRAMBLE_PRE_FEC(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Extended OPU payload
 * If the extended OPU payload bit is set, all OPU overhead fields except
 * for the OPU PSI OH location are used for payload data as described in
 * G.709 clause 17.3. If this bit is cleared, normal OPU OH processing is
 * enabled. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL . EXTENDED_OPU_PAYLOAD
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_EXTENDED_OPU_PAYLOAD(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_EXTENDED_OPU_PAYLOAD     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_EXTENDED_OPU_PAYLOAD(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Tx facility loopback
 * If the Tx facility loopback bit is set, incoming Tx OTN frames are
 * looped back to the Rx processor system side outputs. This bit is cleared
 * as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL . TX_FACILITY_LOOPBACK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_TX_FACILITY_LOOPBACK(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_TX_FACILITY_LOOPBACK     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_TX_FACILITY_LOOPBACK(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Tx terminal loopback
 * If the Tx terminal loopback bit is set, outgoing Tx OTN frames are
 * looped back to the Rx processor line side inputs. This bit is cleared as
 * a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL . TX_TERMINAL_LOOPBACK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_TX_TERMINAL_LOOPBACK(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_TX_TERMINAL_LOOPBACK     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_TX_TERMINAL_LOOPBACK(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief XCO2 Tx OTN counter control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_CONTROL(target)  VTSS_IOREG(target,0x802)

/** 
 * \brief
 * Err sync mode
 * If the err sync mode bit is set, all internal counters are captured on
 * the rising edge of the incoming ERR SYNC IN internal input (accessible
 * as PMTICK) and transferred to their respective shadow memory locations
 * for access in user read operations. In the err sync mode of operation,
 * latch and clear functionality for all counters is still valid if the
 * user wants the updated count between error sync intervals. If the err
 * sync mode bit is cleared, counter latch and clear mode is enabled and
 * internal counters are captured and made available for the user to read
 * only when the designated counter latch and clear bit is asserted.
 * Internal counters are always cleared when counts are captured and
 * transferred through either err sync or latch and clear modes of
 * operation. This bit is cleared (latch and clear mode) as a default
 * value.
 * NOTE: the minimum pulse width for the ERR SYNC IN (PMTICK) signal is 2
 * register interface clocks.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_CONTROL . ERR_SYNC_MODE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_CONTROL_ERR_SYNC_MODE(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_CONTROL_ERR_SYNC_MODE     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_CONTROL_ERR_SYNC_MODE(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * PJ latch and clear
 * If the PJ latch and clear bit is set, the XCO2 will latch the current
 * positive justification count value and transfer it to the PJ counter
 * value bits of the TX OPU PJ counter. The TX OPU PJ counter should not be
 * read after asserting the PJ latch and clear bit until this bit is auto
 * cleared. This bit is cleared as a default and is self clearing after a
 * write.     
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_CONTROL . PJ_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_CONTROL_PJ_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_CONTROL_PJ_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_CONTROL_PJ_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * NJ latch and clear
 * If the NJ latch and clear bit is set, the XCO2 latches the current
 * negative justification count value and transfers it to the NJ counter
 * value bits of the TX OPU NJ counter. The TX OPU NJ counter should not be
 * read after asserting the NJ latch and clear bit until this bit is auto
 * cleared. The NJ latch and clear bit is cleared as a default and is self
 * clearing after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_CONTROL . NJ_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_CONTROL_NJ_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_CONTROL_NJ_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_CONTROL_NJ_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,13,1)


/** 
 * \brief XCO2 Tx OTN auto AIS control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_AUTO_AIS_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL(target)  VTSS_IOREG(target,0x803)

/** 
 * \brief
 * Auto OTUk AIS IAE
 * If the auto OTUk AIS IAE bit is set, the transmit processor
 * automatically generates OTUk AIS when streaming mode is enabled, bypass
 * mode is disabled, and an incoming IAE condition is detected. When the
 * auto OTUk AIS IAE bit is set and an IAE condition is detected, OTUk AIS
 * insertion occurs until IAE is no longer detected for the number of
 * multiframes programmed into the auto AIS insert multiframe count bits.
 * This bit is set as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL . AUTO_OTUK_AIS_IAE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_AUTO_OTUK_AIS_IAE(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_AUTO_OTUK_AIS_IAE     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_AUTO_OTUK_AIS_IAE(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Auto OTUk AIS FIFO error
 * If the auto OTUk AIS FIFO error bit is set, the transmit processor
 * automatically generates OTUk AIS when bypass mode is disabled and a FIFO
 * overflow or underflow condition is detected. When the auto OTUk AIS FIFO
 * error bit is set and a FIFO error condition is detected, OTUk AIS
 * insertion will occur until a FIFO error is no longer detected for the
 * number of multiframes programmed into the auto AIS insert multiframe
 * count bits. This bit is cleared as a default value.	      
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL . AUTO_OTUK_AIS_FIFO_ERROR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_AUTO_OTUK_AIS_FIFO_ERROR(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_AUTO_OTUK_AIS_FIFO_ERROR     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_AUTO_OTUK_AIS_FIFO_ERROR(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Auto OTUk AIS LOS
 * If the auto OTUk AIS LOS bit is set, the transmit processor
 * automatically generates OTUk AIS when bypass mode is disabled and an
 * incoming transmit LOS condition is detected. When the auto OTUk AIS LOS
 * bit is set and an LOS condition is detected, OTUk AIS insertion occurs
 * until LOS is no longer detected for the number of multiframes programmed
 * into the auto AIS insert multiframe count bits. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL . AUTO_OTUK_AIS_LOS
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_AUTO_OTUK_AIS_LOS(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_AUTO_OTUK_AIS_LOS     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_AUTO_OTUK_AIS_LOS(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Auto ODUk AIS IAE
 * If the auto ODUk AIS IAE bit is set, the transmit processor
 * automatically generates ODUk AIS when streaming mode is enabled, bypass
 * mode is disabled, and an incoming IAE condition is detected. When the
 * auto ODUk AIS IAE bit is set and an IAE condition is detected, ODUk AIS
 * insertion occurs until IAE is no longer detected for the number of
 * multiframes programmed into the auto AIS insert multiframe count bits.
 * This bit is cleared as a default value.	 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL . AUTO_ODUK_AIS_IAE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_AUTO_ODUK_AIS_IAE(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_AUTO_ODUK_AIS_IAE     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_AUTO_ODUK_AIS_IAE(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Auto ODUk AIS FIFO error
 * If the auto ODUk AIS FIFO error bit is set, the transmit processor
 * automatically generates ODUk AIS when bypass mode is disabled and a FIFO
 * overflow or underflow condition is detected. When the auto ODUk AIS FIFO
 * error bit is set and a FIFO error condition is detected, ODUk AIS
 * insertion occurs until a FIFO error is no longer detected for the number
 * of multiframes programmed into the auto AIS insert multiframe count
 * bits. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL . AUTO_ODUK_AIS_FIFO_ERROR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_AUTO_ODUK_AIS_FIFO_ERROR(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_AUTO_ODUK_AIS_FIFO_ERROR     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_AUTO_ODUK_AIS_FIFO_ERROR(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Auto ODUk AIS LOS
 * If the auto ODUk AIS LOS bit is set, the transmit processor
 * automatically generates ODUk AIS when bypass mode is disabled and an
 * incoming transmit LOS condition is detected. When the auto ODUk AIS LOS
 * bit is set and an LOS condition is detected, ODUk AIS insertion occurs
 * until LOS is no longer detected for the number of multiframes programmed
 * into the auto AIS insert multiframe count bits. This bit is set as a
 * default value.	 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL . AUTO_ODUK_AIS_LOS
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_AUTO_ODUK_AIS_LOS(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_AUTO_ODUK_AIS_LOS     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_AUTO_ODUK_AIS_LOS(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Auto OPUk AIS FIFO error
 * If the auto OPUk AIS FIFO error bit is set, the transmit processor
 * automatically generates OPUk AIS when bypass mode is disabled and a FIFO
 * overflow or underflow condition is detected. When the auto OPUk AIS FIFO
 * error bit is set and a FIFO error condition is detected, OPUk AIS
 * insertion occurs until a FIFO error is no longer detected for the number
 * of multiframes programmed into the auto AIS insert multiframe count
 * bits. This bit is set as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL . AUTO_OPUK_AIS_FIFO_ERROR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_AUTO_OPUK_AIS_FIFO_ERROR(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_AUTO_OPUK_AIS_FIFO_ERROR     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_AUTO_OPUK_AIS_FIFO_ERROR(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Auto OPUk AIS LOS
 * If the auto OPUk AIS LOS bit is set, the transmit processor
 * automatically generates OPUk AIS when bypass mode is disabled and an
 * incoming transmit LOS condition is detected. When the auto OPUk AIS LOS
 * bit is set and an LOS condition is detected, OPUk AIS insertion occurs
 * until LOS is no longer detected for the number of multiframes programmed
 * into the auto AIS insert multiframe count bits. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL . AUTO_OPUK_AIS_LOS
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_AUTO_OPUK_AIS_LOS(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_AUTO_OPUK_AIS_LOS     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_AUTO_OPUK_AIS_LOS(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Min auto AIS insert multiframe count
 * The min auto AIS insert multiframe count bits are used to indicate the
 * minimum number of multiframes OTUk AIS, ODUk AIS, or OPUk AIS inserted
 * when enabled. Each time an incoming error condition is detected and AIS
 * is inserted, an internal counter used to determine the exit of AIS
 * insertion is reset. This counter continues to accumulate frames until
 * the terminal count of the auto AIS insert multiframe count is reached
 * resulting in the removal of AIS insertion. The internal multiframe
 * minimum counter is restarted each time a new event triggered low to high
 * transition occurs. The min auto AIS insert multiframe count bits are set
 * to a value of 2 as a default.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL . MIN_AUTO_AIS_INSERT_MULTIFRAME_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_MIN_AUTO_AIS_INSERT_MULTIFRAME_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_MIN_AUTO_AIS_INSERT_MULTIFRAME_COUNT     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_AUTO_AIS_CONTROL_MIN_AUTO_AIS_INSERT_MULTIFRAME_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief XCO2 Tx OTN FA/OTU control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_FA_OTU_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL(target)  VTSS_IOREG(target,0x806)

/** 
 * \brief
 * Force OTUk AIS
 * If the force OTUk AIS bit is set, the OTUk alarm indication signal is
 * inserted as described in the G.709 specification. This bit is cleared as
 * a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL . FORCE_OTUK_AIS
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_FORCE_OTUK_AIS(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_FORCE_OTUK_AIS     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_FORCE_OTUK_AIS(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Insert FAS
 * If the insert FAS bit is set, a 6 byte Frame Alignment Signal (FAS)
 * pattern of 0xf6f6f6282828 (hex) is inserted into the first 6 columns of
 * the first row of the ODUk frame as described in section 15.6.2.1 of
 * G.709. This bit is set as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL . INSERT_FAS
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_INSERT_FAS(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_INSERT_FAS     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_INSERT_FAS(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * FAS corrupt
 * If the FAS corrupt bit is set, outgoing OTN frame FAS bytes (6 byte FAS)
 * is set to 0xf6f6f2282828 (hex). The FAS corrupt bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL . FAS_CORRUPT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_FAS_CORRUPT(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_FAS_CORRUPT     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_FAS_CORRUPT(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Insert MFAS
 * If the insert MFAS bit is set, a 1 byte Multi-Frame Alignment Signal
 * (MFAS) value is inserted into the 7th column of the first row of the
 * ODUk frame as specified by the auto FAS/MFAS bit setting in the TX OTN
 * Global Control register. 
 * If the auto MFAS bit is set, a 1 byte Multi-Frame Alignment Signal
 * (MFAS) value sequentially counting from 0 to 255 is inserted into the
 * 7th column of the first row of the ODUk frame (when the insert MFAS bit
 * is set). 
 * If the auto FAS/MFAS bit is cleared, a 1 byte Multi-Frame Alignment
 * Signal (MFAS) value sequentially counting from 0 to 255 is inserted into
 * the 7th column of the first row that is synchronized to the TX SYS MFAS
 * REF IN input signal (when the MFAS insert bit is set). This bit is set
 * as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL . INSERT_MFAS
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_INSERT_MFAS(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_INSERT_MFAS     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_INSERT_MFAS(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * MFAS corrupt
 * If the MFAS corrupt bit is set, outgoing OTN frame MFAS 255 count (0xff)
 * is replaced with a count of 0 (0x00). This implies that a count of 0
 * occurs twice consecutively in the 256 count sequence. This bit is
 * cleared as a default value.

 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL . MFAS_CORRUPT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_MFAS_CORRUPT(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_MFAS_CORRUPT     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_MFAS_CORRUPT(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Insert GCC0
 * If the insert GCC0 bit is set, the value programmed into the GCC0 value
 * bits of the TX OTN OTU GCC0 Value register is inserted into the GCC0
 * bits of outgoing ODU frames. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL . INSERT_GCC0
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_INSERT_GCC0(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_INSERT_GCC0     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_INSERT_GCC0(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Insert RES
 * If the insert RES bit is set, the value programmed into the RES value
 * bits of the TX OTN OTU RES Value register is inserted into the RES bits
 * of outgoing ODU frames. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL . INSERT_RES
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_INSERT_RES(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_INSERT_RES     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_INSERT_RES(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * OTU blanking
 * If the OTU blanking bit is set, all OTU overhead byte information is
 * cleared (0x00 hex) and inserted into outgoing frames. This bit is set as
 * a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL . OTU_BLANKING
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_BLANKING(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_BLANKING     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_BLANKING(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * OTU port insert
 * If the OTU port insert bit is set, outgoing OTN frame OTU byte
 * information is inserted from the external overhead port unless a higher
 * priority insertion is valid. If this bit is cleared, OTU overhead port
 * insertion does not occur. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL . OTU_PORT_INSERT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_PORT_INSERT(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_PORT_INSERT     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_PORT_INSERT(x)  VTSS_EXTRACT_BITFIELD(x,6,1)


/** 
 * \brief XCO2 Tx OTN OTU SM control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_OTU_SM_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL(target)  VTSS_IOREG(target,0x807)

/** 
 * \brief
 * Insert SM TTI
 * If the insert SM TTI bit is set, the section monitoring trail trace
 * identifier (SM TTI) is inserted from an SM TTI value contained in the TX
 * OTN OTU SM TTI memory. The SM TTI value inserted is aligned to
 * multiframe boundaries and repeats 4 times for each complete multiframe
 * sequence. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL . INSERT_SM_TTI
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_INSERT_SM_TTI(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_INSERT_SM_TTI     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_INSERT_SM_TTI(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Insert SM BIP
 * If the insert SM BIP bit is set, the value generated by the BIP-8
 * calculator module is inserted into the SM BIP-8 position of outgoing ODU
 * frames. This bit is set as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL . INSERT_SM_BIP
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_INSERT_SM_BIP(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_INSERT_SM_BIP     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_INSERT_SM_BIP(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * SM BIP corrupt
 * If the SM BIP corrupt bit is set, the value inserted for SM BIP-8 is
 * corrupted (calculated value inverted) when the insert SM BIP bit is set.
 * If the SM BIP corrupt bit is cleared, SM BIP-8 corruption is inhibited.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL . SM_BIP_CORRUPT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BIP_CORRUPT(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BIP_CORRUPT     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BIP_CORRUPT(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Insert SM BEIBIAE
 * If the insert SM BEI/BIAE bit is set, the value programmed into the SM
 * BEI/BIAE value bits of the TX OTN OTU SM Value register is inserted into
 * the SM BEI/BIAE bits of outgoing ODU frames. This bit is cleared as a
 * default value.
 * NOTE: SM BEI/BIAE has priority over auto SM BEI/BIAE. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL . INSERT_SM_BEIBIAE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_INSERT_SM_BEIBIAE(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_INSERT_SM_BEIBIAE     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_INSERT_SM_BEIBIAE(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Auto SM BEIBIAE
 * If the auto SM BEI/BIAE bit is set, the number of SM BIP errors detected
 * (count from 0 to 8) in Rx direction are inserted into the SM BEI/BIAE
 * field of outgoing ODU frames. If an incoming alignment error (IAE) is
 * detected in the Rx direction, a fixed value of 1011 is inserted into the
 * SM BEI/BIAE field of outgoing ODU frames when the auto SM BEI/BIAE bit
 * is set and the insert BEI/BIAE bit is cleared. If the auto SM BEI/BIAE
 * bit is cleared, auto insertion of the SM BEI/BIAE field is ignored. This
 * bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL . AUTO_SM_BEIBIAE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_AUTO_SM_BEIBIAE(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_AUTO_SM_BEIBIAE     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_AUTO_SM_BEIBIAE(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Insert SM BDI
 * If the insert SM BDI bit is set, the value programmed into the SM BDI
 * value bit of the TX OTN OTU SM Value register is inserted into the SM
 * BDI bit of outgoing ODU frames. This bit is cleared as a default value. 
 * NOTE: insert SM BDI has priority over auto SM BDI. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL . INSERT_SM_BDI
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_INSERT_SM_BDI(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_INSERT_SM_BDI     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_INSERT_SM_BDI(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Auto SM BDI
 * If the auto SM BDI bit is set, the SM BDI value is set for outgoing
 * frames if any of the following conditions is true; otherwise, the SM BDI
 * values cleared.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL . AUTO_SM_BDI
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_AUTO_SM_BDI(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_AUTO_SM_BDI     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_AUTO_SM_BDI(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * SM BDI LOS
 * If the SM BDI LOS bit is set, the SM BDI field for outgoing frames is
 * set if the auto SM BDI bit is set, an Rx LOS condition is detected, and
 * the insert SM BDI bit is cleared. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL . SM_BDI_LOS
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BDI_LOS(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BDI_LOS     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BDI_LOS(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * SM BDI LOF
 * If the SM BDI LOF bit is set, the SM BDI field for outgoing frames is
 * set if the auto SM BDI bit is set, an Rx LOF condition is detected, and
 * the insert SM BDI bit is cleared. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL . SM_BDI_LOF
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BDI_LOF(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BDI_LOF     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BDI_LOF(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * SM BDI LOM
 * If the SM BDI LOM bit is set, the SM BDI field for outgoing frames is
 * set if the auto SM BDI bit is set, an Rx LOM condition is detected, and
 * the insert SM BDI bit is cleared. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL . SM_BDI_LOM
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BDI_LOM(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BDI_LOM     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BDI_LOM(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * SM BDI OTUk AIS
 * If the SM BDI OTUk AIS bit is set, the SM BDI field for outgoing frames
 * is set if the auto SM BDI bit is set, an Rx OTUk AIS condition is
 * detected, and the insert SM BDI bit is cleared. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL . SM_BDI_OTUK_AIS
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BDI_OTUK_AIS(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BDI_OTUK_AIS     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BDI_OTUK_AIS(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * SM BDI TIM
 * If the SM BDI TIM bit is set, the SM BDI field for outgoing frames is
 * set if the auto SM BDI bit is set, an Rx TIM condition is detected, and
 * the insert SM BDI bit is cleared. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL . SM_BDI_TIM
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BDI_TIM(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BDI_TIM     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BDI_TIM(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Insert SM IAE
 * If the insert SM IAE bit is set, the value programmed into the SM IAE
 * value bit of the TX OTN OTU SM Value register is inserted into the SM
 * IAE bit of outgoing ODU frames. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL . INSERT_SM_IAE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_INSERT_SM_IAE(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_INSERT_SM_IAE     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_INSERT_SM_IAE(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Auto SM IAE
 * If the auto SM IAE bit is set, the SM IAE bit is set for the number of
 * multiframes programmed into the min auto AIS insert multiframe count
 * bits of the TX OTN Auto AIS Control register when an incoming transmit
 * system side IAE condition is detected in streaming mode. A 16 multiframe
 * counter restarts each time an incoming IAE condition is detected. This
 * bit is cleared as a default value.
 * NOTE: insert SM IAE has priority over auto SM IAE. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL . AUTO_SM_IAE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_AUTO_SM_IAE(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_AUTO_SM_IAE     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_AUTO_SM_IAE(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Insert SM RES
 * If the insert SM RES bit is set, the value is programmed into the SM RES
 * value bits of the TX OTN OTU SM value.	
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL . INSERT_SM_RES
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_INSERT_SM_RES(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_INSERT_SM_RES     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_INSERT_SM_RES(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief XCO2 Tx OTN OTU SM TTI memory 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_OTU_SM_TTI
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: XCO2_TX_OTN_OTU_SM_TTI (??), 0-31
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_TTI(target,ri)  VTSS_IOREG(target,0x808 + (ri))

/** 
 * \brief
 * SM TTI value
 * The SM TTI value bits contain the 64 byte trail trace identifier
 * (SAPI/DAPI/Operator Specific) message to be inserted into outgoing ODU
 * frames when the insert SM TTI bit of the TX OTN OTU Control register is
 * set. The TTI message is repetitively inserted for outgoing frames and is
 * aligned to multiframe boundaries repeating 4 times for each complete
 * multiframe. These bits are undefined as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_TTI . SM_TTI_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_TTI_SM_TTI_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_TTI_SM_TTI_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_TTI_SM_TTI_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Tx OTN OTU SM value register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_OTU_SM_VALUE
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_VALUE(target)  VTSS_IOREG(target,0x828)

/** 
 * \brief
 * SM BEIBIAE value
 * The value programmed into the SM BEI/BIAE value bits is inserted into
 * the SM BEI/BIAE bit positions of outgoing ODU frames when the insert SM
 * BEI/BIAE bit of the TX OTN OTU Control register is set. These bits are
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_VALUE . SM_BEIBIAE_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_VALUE_SM_BEIBIAE_VALUE(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_VALUE_SM_BEIBIAE_VALUE     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_VALUE_SM_BEIBIAE_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,4,4)

/** 
 * \brief
 * SM BDI value
 * The value programmed into the SM BDI value bit is inserted into the SM
 * BDI bit position of outgoing ODU frames when the insert SM BDI bit of
 * the TX OTN OTU Control register is set. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_VALUE . SM_BDI_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_VALUE_SM_BDI_VALUE(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_VALUE_SM_BDI_VALUE     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_VALUE_SM_BDI_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * SM IAE value
 * The value programmed into the SM IAE value bit is inserted into the SM
 * IAE bit position of outgoing ODU frames when the insert SM IAE bit of
 * the TX OTN OTU Control register is set. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_VALUE . SM_IAE_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_VALUE_SM_IAE_VALUE(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_VALUE_SM_IAE_VALUE     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_VALUE_SM_IAE_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * SM RES value
 * The value programmed into the RES value bits is inserted into the SM RES
 * bit positions of outgoing ODU frames when the insert SM RES bit of the
 * TX OTN OTU Control register is set. These bits are cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_VALUE . SM_RES_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_VALUE_SM_RES_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_VALUE_SM_RES_VALUE     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_VALUE_SM_RES_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief XCO2 Tx OTN OTU GCC0 value register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_OTU_GCC0_VALUE
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_GCC0_VALUE(target)  VTSS_IOREG(target,0x829)

/** 
 * \brief
 * GCC0 value
 * The GCC0 value bits contains the 2 byte general communication channel
 * message to be inserted into outgoing ODU frames when the insert GCC0 bit
 * of the TX OTN OTU Control register is set. These bits are cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_GCC0_VALUE . GCC0_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_GCC0_VALUE_GCC0_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_GCC0_VALUE_GCC0_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_GCC0_VALUE_GCC0_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Tx OTN OTU RES value register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_OTU_RES_VALUE
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_RES_VALUE(target)  VTSS_IOREG(target,0x82a)

/** 
 * \brief
 * RES value
 * The RES value bits contains the 2 byte reserved message to be inserted
 * into outgoing ODU frames when the insert RES bit of the TX OTN OTU
 * Control register is set. These bits are cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_RES_VALUE . RES_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_RES_VALUE_RES_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_RES_VALUE_RES_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_RES_VALUE_RES_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Tx OTN ODU control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_ODU_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL(target)  VTSS_IOREG(target,0x82b)

/** 
 * \brief
 * Force ODUk AIS
 * If the force ODUk AIS bit is set, the ODUk alarm indication maintenance
 * signal is inserted as described in the G.709 specification. This bit is
 * cleared as a default value.
 * Note: Force ODUk AIS has priority over force ODUk OCI and force ODUk
 * LCK.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL . FORCE_ODUK_AIS
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_FORCE_ODUK_AIS(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_FORCE_ODUK_AIS     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_FORCE_ODUK_AIS(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Force ODUk OCI
 * If the force ODUk OCI bit is set, the ODUk open connection indication
 * maintenance signal is inserted as described in the G.709 specification.
 * This bit is cleared as a default value.
 * Note: Force ODUk OCI has priority over force ODUk LCK but not over force
 * ODUk AIS.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL . FORCE_ODUK_OCI
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_FORCE_ODUK_OCI(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_FORCE_ODUK_OCI     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_FORCE_ODUK_OCI(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Force ODUk LCK
 * If the force ODUk LCK bit is set, the ODUk locked maintenance signal is
 * inserted as described in the G.709 specification. This bit is cleared as
 * a default value.
 * Note: ODUk LCK insertion occurs only if force ODUk LCK is asserted,
 * force ODUk AIS is deasserted, and force ODUk OCI is deasserted.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL . FORCE_ODUK_LCK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_FORCE_ODUK_LCK(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_FORCE_ODUK_LCK     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_FORCE_ODUK_LCK(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Insert RES1
 * If the insert RES1 bit is set, the value programmed into the RES1 value
 * bits of the TX OTN ODU RES1 High Value register and TX OTN ODU RES1
 * LOW/TCM ACT Value register are inserted into the RES1 bits of outgoing
 * ODU frames. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL . INSERT_RES1
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_RES1(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_RES1     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_RES1(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Insert TCM ACT
 * If the insert TCM ACT bit is set, the value programmed into the TCM ACT
 * value bits of the TX OTN ODU RES1 LOW/TCM ACT Value register are
 * inserted into the TCM ACT bits of outgoing ODU frames. This bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL . INSERT_TCM_ACT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_TCM_ACT(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_TCM_ACT     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_TCM_ACT(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * ODU blanking
 * If the ODU blanking bit is set, all ODU overhead byte information is
 * cleared (0x00 hex) and inserted into outgoing frames. This bit is set as
 * a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL . ODU_BLANKING
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_ODU_BLANKING(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_ODU_BLANKING     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_ODU_BLANKING(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Insert FTFL
 * If the insert FTFL bit is set, the value programmed into the FTFL value
 * bits of the TX OTN ODU FTFL Memory is inserted into the FTFL bits of
 * outgoing ODU frames. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL . INSERT_FTFL
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_FTFL(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_FTFL     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_FTFL(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * ODU port insert
 * If the ODU port insert bit is set, outgoing OTN frame ODU byte
 * information is inserted from the external overhead port unless a higher
 * priority insertion is valid. If this bit is cleared, ODU overhead port
 * insertion will not occur. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL . ODU_PORT_INSERT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_ODU_PORT_INSERT(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_ODU_PORT_INSERT     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_ODU_PORT_INSERT(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Insert EXP
 * If the insert EXP bit is set, the value programmed into the EXP value
 * bits of the TX OTN ODU EXP Value register is inserted into the EXP bits
 * of outgoing ODU frames. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL . INSERT_EXP
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_EXP(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_EXP     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_EXP(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Insert GCC1
 * If the insert GCC1 bit is set, the value programmed into the GCC1 value
 * bits of the TX OTN ODU GCC1 Value register is inserted into the GCC1
 * bits of outgoing ODU frames. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL . INSERT_GCC1
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_GCC1(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_GCC1     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_GCC1(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Insert GCC2
 * If the insert GCC2 bit is set, the value programmed into the GCC1 value
 * bits of the TX OTN ODU GCC2 Value register is inserted into the GCC2
 * bits of outgoing ODU frames. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL . INSERT_GCC2
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_GCC2(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_GCC2     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_GCC2(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Insert APSPCC
 * If the insert APS/PCC bit is set, the value programmed into the APS/PCC
 * value bits of the TX OTN ODU APS/PCC Value registers is inserted into
 * the APS/PCC bits of outgoing ODU frames for the multiframe programmed
 * into the APS/PCC MFAS select bits. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL . INSERT_APSPCC
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_APSPCC(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_APSPCC     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_APSPCC(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * APSPCC MFAS select
 * The insert APS/PCC MFAS select bits are used to identify the multiframe
 * APS/PCC bits inserted into outgoing frames. These bits are only valid
 * when the insert APS/PCC bit is set. These bits are cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL . APSPCC_MFAS_SELECT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_APSPCC_MFAS_SELECT(x)  VTSS_ENCODE_BITFIELD(x,1,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_APSPCC_MFAS_SELECT     VTSS_ENCODE_BITMASK(1,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_APSPCC_MFAS_SELECT(x)  VTSS_EXTRACT_BITFIELD(x,1,3)

/** 
 * \brief
 * Insert RES2
 * If the insert RES2 bit is set, the value programmed into the RES2 value
 * bits of the TX OTN ODU RES2 Value registers is inserted into the RES2
 * bits of outgoing ODU frames. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL . INSERT_RES2
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_RES2(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_RES2     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_RES2(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief XCO2 Tx OTN ODU TCMX control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_ODU_TCMX_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: XCO2_TX_OTN_ODU_TCMX_CONTROL (??), 0-5
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL(target,ri)  VTSS_IOREG(target,0x82c + (ri))

/** 
 * \brief
 * TCMi transparent
 * If the TCMi transparent bit is set, TCMi is in transparent mode and
 * overhead is not inserted into TCMi if streaming mode is enabled (all 0s
 * are inserted if streaming mode is disabled). 
 * If the TCMi transparent bit is cleared, TCMi is in active mode and the
 * tandem connection inserts the appropriate overhead as programmed. 
 * This bit is cleared as a default value.
 * NOTE: If transparent mode is enabled, no insertion of any kind for the
 * appropriate TCM overhead is allowed.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL . TCMI_TRANSPARENT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_TCMI_TRANSPARENT(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_TCMI_TRANSPARENT     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_TCMI_TRANSPARENT(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Insert TCMi TTI
 * If the insert TCMi TTI bit is set, the tandem connection monitoring
 * trail trace identifier (TCMi TTI) is inserted as specified by the TCMi
 * TTI value contained in the TX OTN ODU TCMi TTI memory. The TCMi TTI
 * value inserted is aligned to multiframe boundaries and repeats 4 times
 * for each complete multiframe sequence. This bit is only valid when the
 * TCMi transparent bit is cleared. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL . INSERT_TCMI_TTI
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_INSERT_TCMI_TTI(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_INSERT_TCMI_TTI     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_INSERT_TCMI_TTI(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Insert TCMi BIP
 * If the insert TCMi BIP bit is set, the value generated by the BIP-8
 * calculator module is inserted into the TCMi BIP-8 position of outgoing
 * ODU frames. This bit is only valid when the TCMi transparent bit is
 * cleared. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL . INSERT_TCMI_BIP
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_INSERT_TCMI_BIP(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_INSERT_TCMI_BIP     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_INSERT_TCMI_BIP(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * TCMi BIP corrupt
 * If the TCMi BIP corrupt bit is set, the value inserted for TCMi BIP-8 is
 * corrupted (calculated value inverted) when the insert TCMi BIP bit is
 * set. If the TCMi BIP corrupt bit is cleared, TCMi BIP-8 corruption is
 * inhibited. This bit is only valid when the TCMi transparent bit is
 * cleared. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL . TCMI_BIP_CORRUPT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_TCMI_BIP_CORRUPT(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_TCMI_BIP_CORRUPT     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_TCMI_BIP_CORRUPT(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Insert TCMi BEIBIAE
 * If the insert TCMi BEI/BIAE bit is set, the value programmed into the
 * TCMi BEI/BIAE value bits of the TX OTN ODU TCMi Value register is
 * inserted into the TCMi BEI/BIAE bits of outgoing ODU frames. This bit is
 * only valid when the TCMi transparent bit is cleared. This bit is cleared
 * as a default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL . INSERT_TCMI_BEIBIAE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_INSERT_TCMI_BEIBIAE(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_INSERT_TCMI_BEIBIAE     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_INSERT_TCMI_BEIBIAE(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Auto TCMi BEIBIAE
 * If the auto TCMi BEI/BIAE bit is set, the number of TCMi BIP errors
 * detected (count from 0 to 8) in the RX direction are inserted into the
 * TCMi BEI/BIAE field of outgoing ODU frames.
 * If the auto TCMi BEI/BIAE bit is cleared, auto insertion of the TCMi
 * BEI/BIAE field is ignored. 
 * This bit is only valid when the TCMi transparent and insert TCMi
 * BEI/BIAE bits are cleared. This bit is cleared as a default value.
 * NOTE: If an incoming alignment error (accepted RX TCMi STAT bits equal
 * 010) is detected in the Rx direction, a fixed value of 1011 is inserted
 * into the SM BEI/BIAE field of outgoing ODU frames when the auto TCMi
 * BEI/BIAE bit is set.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL . AUTO_TCMI_BEIBIAE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_AUTO_TCMI_BEIBIAE(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_AUTO_TCMI_BEIBIAE     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_AUTO_TCMI_BEIBIAE(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Insert TCMi BDI
 * If the insert TCMi BDI bit is set, the value programmed into the TCMi
 * BDI value bit of the TX OTN ODU TCMi Value register is inserted into the
 * TCMi BDI bit of outgoing ODU frames. This bit is only valid when the
 * TCMi transparent bit is cleared. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL . INSERT_TCMI_BDI
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_INSERT_TCMI_BDI(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_INSERT_TCMI_BDI     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_INSERT_TCMI_BDI(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Auto TCMi BDI
 * If the auto TCMi BDI bit is set, the TCMi BDI bit for outgoing ODU
 * frames is set according to the Rx direction backward defect state, which
 * is defined by the enabled bits of the TX OTN ODU TCMi Auto BDI Control
 * register. If the auto TCMi BDI bit is cleared, auto insertion of the
 * TCMi BDI field is ignored. This bit is only valid when the TCMi
 * transparent and insert TCMi BDI bits are cleared. This bit is cleared as
 * a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL . AUTO_TCMI_BDI
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_AUTO_TCMI_BDI(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_AUTO_TCMI_BDI     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_AUTO_TCMI_BDI(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Insert TCMi STAT
 * If the insert TCMi STAT bit is set, the value programmed into the TCMi
 * STAT value bits of the TX OTN ODU TCMi Value register is inserted into
 * the TCMi STAT bits of outgoing ODU frames. This bit is only valid when
 * the TCMi transparent bit is cleared. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL . INSERT_TCMI_STAT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_INSERT_TCMI_STAT(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_INSERT_TCMI_STAT     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_INSERT_TCMI_STAT(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Auto TCMi STAT IAE
 * If the auto TCMi STAT IAE bit is set, the TCMi STAT field bits for all
 * originating TCM s (any TCMi not configured for transparent mode) is set
 * to 010 (in use with IAE) for the number of multiframes programmed into
 * the IAE multiframe count bits of the TX OTN ODU TCMi Auto BDI Control
 * register when an incoming transmit system side IAE condition is detected
 * in streaming mode. An internal multiframe counter restarts each time an
 * incoming IAE condition is detected. The TCMi STAT field bits is set to
 * 001 (in use without IAE) after the multiframe counter reaches its
 * terminal count. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL . AUTO_TCMI_STAT_IAE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_AUTO_TCMI_STAT_IAE(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_AUTO_TCMI_STAT_IAE     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_AUTO_TCMI_STAT_IAE(x)  VTSS_EXTRACT_BITFIELD(x,6,1)


/** 
 * \brief XCO2 Tx OTN ODU TCMi auto BDI control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL(target)  VTSS_IOREG(target,0x832)

/** 
 * \brief
 * TCMi BDI LOS
 * If the TCMi BDI LOS bit is set, the BDI field of active TCMs is set for
 * outgoing frames if an Rx LOS condition is detected and the appropriate
 * auto TCMi BDI bit is set. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL . TCMI_BDI_LOS
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_LOS(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_LOS     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_LOS(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * TCMi BDI LOF
 * If the TCMi BDI LOF bit is set, the BDI field of active TCMs is set for
 * outgoing frames if an Rx LOF condition is detected and the appropriate
 * auto TCMi BDI bit is set. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL . TCMI_BDI_LOF
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_LOF(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_LOF     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_LOF(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * TCMi BDI LOM
 * If the TCMi BDI LOM bit is set, the BDI field of active TCMs is set for
 * outgoing frames if an Rx LOM condition is detected and the appropriate
 * auto TCMi BDI bit is set. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL . TCMI_BDI_LOM
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_LOM(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_LOM     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_LOM(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * TCMi BDI OTUk AIS
 * If the TCMi BDI OTUk AIS bit is set, the BDI field of active TCMs is set
 * for outgoing frames if an Rx OTUk AIS condition is detected and the
 * appropriate auto TCMi BDI bit is set. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL . TCMI_BDI_OTUK_AIS
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_OTUK_AIS(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_OTUK_AIS     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_OTUK_AIS(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * TCMi BDI TCMi ODUk AIS
 * If the TCMi BDI TCMi ODUk AIS bit is set, the BDI field of active TCMs
 * is set for outgoing frames if an Rx TCMi ODUk AIS condition is detected
 * (accepted TCMi STAT is 111) and the appropriate auto TCMi BDI bit is
 * set. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL . TCMI_BDI_TCMI_ODUK_AIS
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_TCMI_ODUK_AIS(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_TCMI_ODUK_AIS     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_TCMI_ODUK_AIS(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * TCMi BDI TCMi LTC
 * If the TCMi BDI TCMi LTC bit is set, the BDI field of all active TCMs is
 * set for outgoing frames if an Rx Loss of Tandem Connection - LTC
 * (accepted TCMi STAT is 000) condition is detected and the appropriate
 * auto TCMi BDI bit is set. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL . TCMI_BDI_TCMI_LTC
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_TCMI_LTC(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_TCMI_LTC     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_TCMI_LTC(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * TCMi BDI TCMi OCI
 * If the TCMi BDI TCMi OCI bit is set, the BDI field of active TCMs is set
 * for outgoing frames if an Rx TCMi OCI condition is detected (accepted
 * TCMi STAT is 110) and the appropriate auto TCMi BDI bit is set. This bit
 * is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL . TCMI_BDI_TCMI_OCI
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_TCMI_OCI(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_TCMI_OCI     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_TCMI_OCI(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * TCMi BDI TCMi LCK
 * If the TCMi BDI TCMi LCK bit is set, the BDI field of active TCMs is set
 * for outgoing frames if an Rx TCMi LCK condition is detected (accepted
 * TCMi STAT is 101) and the appropriate auto TCMi BDI bit is set. This bit
 * is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL . TCMI_BDI_TCMI_LCK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_TCMI_LCK(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_TCMI_LCK     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_TCMI_LCK(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * TCMi BDI TCMi TIM
 * If the TCMi BDI TCMi LCK bit is set, the BDI field of active TCMs is set
 * for outgoing frames if an Rx TCMi TIM condition is detected (accepted
 * TCMi TTI does not match expected TCMi TTI) and the appropriate auto TCMi
 * BDI bit is set. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL . TCMI_BDI_TCMI_TIM
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_TCMI_TIM(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_TCMI_TIM     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_TCMI_TIM(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * TCMi BDI SM TIM
 * If the TCMi BDI SM TIM bit is set, the BDI field of active TCMs is set
 * for outgoing frames if an Rx SM TIM condition is detected (accepted SM
 * TTI does not match expected SM TTI) and the appropriate auto TCMi BDI
 * bit is set. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL . TCMI_BDI_SM_TIM
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_SM_TIM(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_SM_TIM     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_SM_TIM(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * IAE multiframe count
 * The IAE multiframe count bits are used to indicate the number of
 * multiframes IAE inserted into the appropriate TCMi STAT field when an
 * incoming transmit IAE condition is detected. Each time an incoming IAE
 * condition is detected and IAE is inserted, an internal counter used to
 * determine the exit of IAE insertion is reset. This counter continues to
 * accumulate frames until the terminal count of the IAE multiframe count
 * is reached, resulting in the removal of TCMi IAE insertion. These bits
 * are set to a value of 2 as a default.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL . IAE_MULTIFRAME_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_IAE_MULTIFRAME_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_IAE_MULTIFRAME_COUNT     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_IAE_MULTIFRAME_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief XCO2 Tx OTN OTU TCMX TTI memory 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_OTU_TCMX_TTI
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: XCO2_TX_OTN_OTU_TCMX_TTI (??), 0-191
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_TCMX_TTI(target,ri)  VTSS_IOREG(target,0x833 + (ri))

/** 
 * \brief
 * TCMi TTI value
 * The TCMi TTI value bits contain the 64 byte trail trace identifier
 * (SAPI/DAPI/Operator Specific) message to be inserted into outgoing ODU
 * frames when the TCMi transparent bit is cleared and the insert TCMi TTI
 * bit of the TX OTN ODU TCMi Control register is set. The TCMi TTI message
 * is repetitively inserted for outgoing frames and is aligned to
 * multiframe boundaries repeating 4 times for each complete multiframe.
 * These bits are undefined as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_TCMX_TTI . TCMI_TTI_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_TCMX_TTI_TCMI_TTI_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_TCMX_TTI_TCMI_TTI_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_TCMX_TTI_TCMI_TTI_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Tx OTN ODU TCMX value register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_ODU_TCMX_VALUE
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: XCO2_TX_OTN_ODU_TCMX_VALUE (??), 0-5
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_VALUE(target,ri)  VTSS_IOREG(target,0x8f3 + (ri))

/** 
 * \brief
 * TCMi BEIBIAE value
 * The value programmed into the TCMi BEI/BIAE value bits are inserted into
 * the selected TCMi BEI/BIAE bit positions of outgoing frames when the
 * TCMi transparent bit is cleared and the insert TCMi BEI/BIAE bit of the
 * TX OTN ODU TCMi Control register is set. These bits are cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_VALUE . TCMI_BEIBIAE_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_VALUE_TCMI_BEIBIAE_VALUE(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_VALUE_TCMI_BEIBIAE_VALUE     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_VALUE_TCMI_BEIBIAE_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,4,4)

/** 
 * \brief
 * TCMi BDI value
 * The value programmed into the TCMi BDI value bit is inserted into the
 * selected TCMi BDI bit position of outgoing frames when the TCMi
 * transparent bit is cleared and the insert TCMi BDI bit of the TX OTN ODU
 * TCMi Control register is set. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_VALUE . TCMI_BDI_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_VALUE_TCMI_BDI_VALUE(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_VALUE_TCMI_BDI_VALUE     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_VALUE_TCMI_BDI_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * TCMi STAT value
 * The value programmed into the TCMi STAT value bits are inserted into the
 * selected TCMi STAT bit positions of outgoing frames when the TCMi
 * transparent bit is cleared and the insert TCMi STAT bit of the TX OTN
 * ODU TCMi Control register is set. These bits are cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_VALUE . TCMI_STAT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_VALUE_TCMI_STAT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_VALUE_TCMI_STAT_VALUE     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_VALUE_TCMI_STAT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief XCO2 Tx OTN ODU PM control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_ODU_PM_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL(target)  VTSS_IOREG(target,0x8f9)

/** 
 * \brief
 * Insert PM TTI
 * If the insert PM TTI bit is set, the path monitoring trail trace
 * identifier (PM TTI) is inserted as specified by the PM TTI value
 * contained in the Tx OTN ODU PM TTI memory. The PM TTI value inserted is
 * aligned to multiframe boundaries and repeats four times for each
 * complete multiframe sequence. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL . INSERT_PM_TTI
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_INSERT_PM_TTI(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_INSERT_PM_TTI     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_INSERT_PM_TTI(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Insert PM BIP
 * If the insert PM BIP bit is set, the value generated by the BIP-8
 * calculator module is inserted into the PM BIP-8 position of outgoing ODU
 * frames. The insert PM BIP bit is set as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL . INSERT_PM_BIP
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_INSERT_PM_BIP(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_INSERT_PM_BIP     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_INSERT_PM_BIP(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * PM BIP corrupt
 * If the PM BIP corrupt bit is set, the value inserted for PM BIP-8 is
 * corrupted (calculated value inverted) when the insert PM BIP bit is set.
 * If the PM BIP corrupt bit is cleared, PM BIP-8 corruption is inhibited.
 * The PM BIP corrupt bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL . PM_BIP_CORRUPT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_PM_BIP_CORRUPT(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_PM_BIP_CORRUPT     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_PM_BIP_CORRUPT(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Insert PM BEI
 * If the insert PM BEI bit is set, the value programmed into the PM BEI
 * value bits of the TX OTN ODU PM Value register is inserted into the PM
 * BEI bits of outgoing ODU frames. This bit is cleared as a default value.
 * 
 * NOTE: insert PM BEI has priority over auto PM BEI. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL . INSERT_PM_BEI
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_INSERT_PM_BEI(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_INSERT_PM_BEI     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_INSERT_PM_BEI(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Auto PM BEI
 * If the auto PM BEI bit is set, the number of PM BIP errors detected
 * (count from 0 to 8) in the Rx direction are inserted into the PM BEI
 * field of outgoing ODU frames. If the auto PM BEI bit is cleared, auto
 * insertion of the PM BEI field is ignored. This bit is set as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL . AUTO_PM_BEI
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_AUTO_PM_BEI(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_AUTO_PM_BEI     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_AUTO_PM_BEI(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Insert PM BDI
 * If the insert PM BDI bit is set, the value programmed into the PM BDI
 * value bit of the TX OTN ODU PM Value register is inserted into the PM
 * BDI bit of outgoing ODU frames. This bit is cleared as a default value. 
 * NOTE: insert PM BDI has priority over auto PM BDI. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL . INSERT_PM_BDI
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_INSERT_PM_BDI(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_INSERT_PM_BDI     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_INSERT_PM_BDI(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Auto PM BDI
 * If the auto PM BDI bit is set, the PM BDI bit for outgoing ODU frames is
 * set according to the Rx direction backward defect state (TIM error,
 * etc.). If the auto PM BDI bit is cleared, auto insertion of the PM BDI
 * field is ignored. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL . AUTO_PM_BDI
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_AUTO_PM_BDI(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_AUTO_PM_BDI     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_AUTO_PM_BDI(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Insert PM STAT
 * If the insert PM STAT bit is set, the value programmed into the PM STAT
 * value bits of the TX OTN ODU PM Value register is inserted into the PM
 * STAT bits of outgoing ODU frames. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL . INSERT_PM_STAT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_INSERT_PM_STAT(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_INSERT_PM_STAT     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_INSERT_PM_STAT(x)  VTSS_EXTRACT_BITFIELD(x,8,1)


/** 
 * \brief XCO2 Tx OTN ODU PM auto BDI control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL(target)  VTSS_IOREG(target,0x8fa)

/** 
 * \brief
 * PM BDI LOS
 * If the PM BDI LOS bit is set, the BDI field of PM will be set for
 * outgoing frames if an Rx LOS condition is detected and the auto PM BDI
 * bit is set. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL . PM_BDI_LOS
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_LOS(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_LOS     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_LOS(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * PM BDI LOF
 * If the PM BDI LOF bit is set, the BDI field of PM will be set for
 * outgoing frames if an Rx LOF condition is detected and the auto PM BDI
 * bit is set. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL . PM_BDI_LOF
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_LOF(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_LOF     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_LOF(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * PM BDI LOM
 * If the PM BDI LOM bit is set, the BDI field of PM will be set for
 * outgoing frames if an Rx LOM condition is detected and the auto PM BDI
 * bit is set. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL . PM_BDI_LOM
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_LOM(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_LOM     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_LOM(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * PM BDI OTUk AIS
 * If the PM BDI OTUk AIS bit is set, the BDI field of PM will be set for
 * outgoing frames if an Rx OTUk AIS condition is detected and the auto PM
 * BDI bit is set. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL . PM_BDI_OTUK_AIS
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_OTUK_AIS(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_OTUK_AIS     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_OTUK_AIS(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * PM BDI PM ODUk AIS
 * If the PM BDI PM ODUk AIS bit is set, the BDI field of PM will be set
 * for outgoing frames if an Rx PM ODUk AIS condition is detected (accepted
 * PM STAT is 111) and the auto PM BDI bit is set. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL . PM_BDI_PM_ODUK_AIS
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_PM_ODUK_AIS(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_PM_ODUK_AIS     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_PM_ODUK_AIS(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * PM BDI PM OCI
 * If the PM BDI PM OCI bit is set, the BDI field of PM will be set for
 * outgoing frames if an Rx PM OCI condition is detected (accepted PM STAT
 * is 110) and the auto PM BDI bit is set. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL . PM_BDI_PM_OCI
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_PM_OCI(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_PM_OCI     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_PM_OCI(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * PM BDI PM LCK
 * If the PM BDI PM LCK bit is set, the BDI field of PM will be set for
 * outgoing frames if an Rx PM LCK condition is detected (accepted PM STAT
 * is 101) and the auto PM BDI bit is set. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL . PM_BDI_PM_LCK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_PM_LCK(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_PM_LCK     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_PM_LCK(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * PM BDI PM TIM
 * If the PM BDI PM TIM bit is set, the BDI field of PM will be set for
 * outgoing frames if an Rx PM TIM condition is detected (accepted PM TTI
 * does not match expected PM TTI) and the auto PM BDI bit is set. This bit
 * is cleared as a default value. 

 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL . PM_BDI_PM_TIM
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_PM_TIM(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_PM_TIM     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_PM_TIM(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * PM BDI SM TIM
 * If the PM BDI SM TIM bit is set, the BDI field of PM will be set for
 * outgoing frames if an Rs SM TIM condition is detected (accepted SM TTI
 * does not match expected SM TTI) and the auto PM BDI bit is set. This bit
 * is cleared as a default value. 

 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL . PM_BDI_SM_TIM
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_SM_TIM(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_SM_TIM     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_SM_TIM(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * PM BDI TCM1 TIMAISOCILCK
 * If the PM BDI TCM1 TIM/AIS/OCI/LCK bit is set, the BDI field of PM will
 * be set for outgoing frames when an Rx TCM1 TIM/AIS/OCI/LCK condition is
 * detected and the auto PM BDI bit is set. If the PM BDI TCM1
 * TIM/AIS/OCI/LCK bit or auto PM BDI bit is cleared, PM BDI is not
 * inserted due to TCM1 TIM/AIS/OCI/LCK detection. The PM BDI TCM1
 * TIM/AIS/OCI/LCK bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL . PM_BDI_TCM1_TIMAISOCILCK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_TCM1_TIMAISOCILCK(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_TCM1_TIMAISOCILCK     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_TCM1_TIMAISOCILCK(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * PM BDI TCM2 TIMAISOCILCK
 * If the PM BDI TCM2 TIM/AIS/OCI/LCK bit is set, the BDI field of PM will
 * be set for outgoing frames when an Rx TCM2 TIM/AIS/OCI/LCK condition is
 * detected and the auto PM BDI bit is set. If the PM BDI TCM2
 * TIM/AIS/OCI/LCK bit or auto PM BDI bit is cleared, PM BDI is not
 * inserted due to TCM2 TIM/AIS/OCI/LCK detection. The PM BDI TCM2
 * TIM/AIS/OCI/LCK bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL . PM_BDI_TCM2_TIMAISOCILCK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_TCM2_TIMAISOCILCK(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_TCM2_TIMAISOCILCK     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_TCM2_TIMAISOCILCK(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * PM BDI TCM3 TIMAISOCILCK
 * If the PM BDI TCM3 TIM/AIS/OCI/LCK bit is set, the BDI field of PM will
 * be set for outgoing frames when an Rx TCM3 TIM/AIS/OCI/LCK condition is
 * detected and the auto PM BDI bit is set. If the PM BDI TCM3
 * TIM/AIS/OCI/LCK bit or auto PM BDI bit is cleared, PM BDI is not
 * inserted due to TCM3 TIM/AIS/OCI/LCK detection. The PM BDI TCM3
 * TIM/AIS/OCI/LCK bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL . PM_BDI_TCM3_TIMAISOCILCK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_TCM3_TIMAISOCILCK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_TCM3_TIMAISOCILCK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_TCM3_TIMAISOCILCK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * PM BDI TCM4 TIMAISOCILCK
 * If the PM BDI TCM4 TIM/AIS/OCI/LCK bit is set, the BDI field of PM will
 * be set for outgoing frames when an Rx TCM4 TIM/AIS/OCI/LCK condition is
 * detected and the auto PM BDI bit is set. If the PM BDI TCM4
 * TIM/AIS/OCI/LCK bit or auto PM BDI bit is cleared, PM BDI is not
 * inserted due to TCM4 TIM/AIS/OCI/LCK detection. The PM BDI TCM4
 * TIM/AIS/OCI/LCK bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL . PM_BDI_TCM4_TIMAISOCILCK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_TCM4_TIMAISOCILCK(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_TCM4_TIMAISOCILCK     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_TCM4_TIMAISOCILCK(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * PM BDI TCM5 TIMAISOCILCK
 * If the PM BDI TCM5 TIM/AIS/OCI/LCK bit is set, the BDI field of PM will
 * be set for outgoing frames when an Rx TCM5 TIM/AIS/OCI/LCK condition is
 * detected and the auto PM BDI bit is set. If the PM BDI TCM5
 * TIM/AIS/OCI/LCK bit or auto PM BDI bit is cleared, PM BDI is not
 * inserted due to TCM5 TIM/AIS/OCI/LCK detection. The PM BDI TCM5
 * TIM/AIS/OCI/LCK bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL . PM_BDI_TCM5_TIMAISOCILCK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_TCM5_TIMAISOCILCK(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_TCM5_TIMAISOCILCK     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_TCM5_TIMAISOCILCK(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * PM BDI TCM6 TIMAISOCILCK
 * If the PM BDI TCM6 TIM/AIS/OCI/LCK bit is set, the BDI field of PM will
 * be set for outgoing frames when an Rx TCM6 TIM/AIS/OCI/LCK condition is
 * detected and the auto PM BDI bit is set. If the PM BDI TCM6
 * TIM/AIS/OCI/LCK bit or auto PM BDI bit is cleared, PM BDI is not
 * inserted due to TCM6 TIM/AIS/OCI/LCK detection. The PM BDI TCM6
 * TIM/AIS/OCI/LCK bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL . PM_BDI_TCM6_TIMAISOCILCK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_TCM6_TIMAISOCILCK(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_TCM6_TIMAISOCILCK     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_TCM6_TIMAISOCILCK(x)  VTSS_EXTRACT_BITFIELD(x,1,1)


/** 
 * \brief XCO2 Tx OTN ODU PM TTI memory 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_ODU_PM_TTI
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: XCO2_TX_OTN_ODU_PM_TTI (??), 0-31
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_TTI(target,ri)  VTSS_IOREG(target,0x8fb + (ri))

/** 
 * \brief
 * PM TTI value
 * The PM TTI value bits contains the 64 byte trail trace identifier
 * (SAPI/DAPI/Operator Specific) message to be inserted into outgoing ODU
 * frames when the insert PM bit of the TX OTN ODU Control register and the
 * insert PM TTI bit of the TX OTN ODU PM Control register are set. The PM
 * TTI message is repetitively inserted for outgoing frames and is aligned
 * to multiframe boundaries repeating 4 times for each complete multiframe.
 * These bits are undefined as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_TTI . PM_TTI_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_TTI_PM_TTI_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_TTI_PM_TTI_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_TTI_PM_TTI_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Tx OTN ODU PM value register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_ODU_PM_VALUE
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_VALUE(target)  VTSS_IOREG(target,0x91b)

/** 
 * \brief
 * PM BEI value
 * The value programmed into the PM BEI value bits are inserted into the
 * selected PM BEI bit positions of outgoing frames when the insert PM bit
 * of the TX OTN ODU Control register and the insert PM BEI bit of the TX
 * OTN ODU PM Control register are set. The PM BEI value bits are cleared
 * as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_VALUE . PM_BEI_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_VALUE_PM_BEI_VALUE(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_VALUE_PM_BEI_VALUE     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_VALUE_PM_BEI_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,4,4)

/** 
 * \brief
 * PM BDI value
 * The value programmed into the PM BDI value bit is inserted into the
 * selected PM BDI bit position of outgoing frames when the insert PM bit
 * of the TX OTN ODU Control register and the insert PM BDI bit of the TX
 * OTN ODU PM Control register are set. The PM BDI value bit is cleared as
 * a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_VALUE . PM_BDI_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_VALUE_PM_BDI_VALUE(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_VALUE_PM_BDI_VALUE     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_VALUE_PM_BDI_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * PM STAT value
 * The value programmed into the PM STAT value bits are inserted into the
 * selected PM STAT bit positions of outgoing frames when the insert PM bit
 * of the TX OTN ODU Control register and the insert PM STAT bit of the TX
 * OTN ODU PM Control register are set. The PM STAT value bits are cleared
 * as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_VALUE . PM_STAT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_VALUE_PM_STAT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_VALUE_PM_STAT_VALUE     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_VALUE_PM_STAT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief XCO2 Tx OTN ODU RES1 high value register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_ODU_RES1_HIGH_VALUE
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES1_HIGH_VALUE(target)  VTSS_IOREG(target,0x91c)

/** 
 * \brief
 * RES1 high value
 * The RES1 high value bits contain the first 2 bytes of the reserved
 * message (RES1) to be inserted into the second row, first 2 columns of
 * outgoing frames when the insert RES1 bit of the TX OTN ODU Control
 * register is set. The RES1 high value bits are cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES1_HIGH_VALUE . RES1_HIGH_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES1_HIGH_VALUE_RES1_HIGH_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES1_HIGH_VALUE_RES1_HIGH_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES1_HIGH_VALUE_RES1_HIGH_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Tx OTN ODU RES1 low/TCM ACT value register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_ODU_RES1_LOW_TCM_ACT_VALUE
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES1_LOW_TCM_ACT_VALUE(target)  VTSS_IOREG(target,0x91d)

/** 
 * \brief
 * RES1 low value
 * The RES1 low value bits contains the third byte of the reserved message
 * (RES1) to be inserted into the second row, third column of outgoing
 * frames when the insert RES1 bit of the TX OTN ODU Control register is
 * set. The RES1 low value bits are cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES1_LOW_TCM_ACT_VALUE . RES1_LOW_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES1_LOW_TCM_ACT_VALUE_RES1_LOW_VALUE(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES1_LOW_TCM_ACT_VALUE_RES1_LOW_VALUE     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES1_LOW_TCM_ACT_VALUE_RES1_LOW_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * TCM ACT value
 * The TCM ACT value bits contains the TCM ACT information to be inserted
 * into the second row, fourth column of outgoing frames when the insert
 * TCM ACT bit of the TX OTN ODU Control register is set. The TCM ACT value
 * bits are cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES1_LOW_TCM_ACT_VALUE . TCM_ACT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES1_LOW_TCM_ACT_VALUE_TCM_ACT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES1_LOW_TCM_ACT_VALUE_TCM_ACT_VALUE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES1_LOW_TCM_ACT_VALUE_TCM_ACT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief XCO2 Tx OTN ODU FTFL memory 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_ODU_FTFL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: XCO2_TX_OTN_ODU_FTFL (??), 0-127
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_FTFL(target,ri)  VTSS_IOREG(target,0x91e + (ri))

/** 
 * \brief
 * FTFL value
 * The FTFL value bits contains the 256 byte fault type and fault location
 * message to be inserted into outgoing frames when the insert FTFL bit of
 * the TX OTN ODU Control register is set. The FTFL message is repetitively
 * inserted and aligned to the multiframe of outgoing frames. These bits
 * are undefined as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_FTFL . FTFL_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_FTFL_FTFL_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_FTFL_FTFL_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_FTFL_FTFL_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Tx OTN ODU EXP value register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_ODU_EXP_VALUE
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_EXP_VALUE(target)  VTSS_IOREG(target,0x99e)

/** 
 * \brief
 * EXP value
 * The EXP value bits contain the experimental overhead bytes (EXP) to be
 * inserted into the third row, columns 13 and 14 of outgoing frames when
 * the insert EXP bit of the TX OTN ODU Control register is set. These bits
 * are cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_EXP_VALUE . EXP_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_EXP_VALUE_EXP_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_EXP_VALUE_EXP_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_EXP_VALUE_EXP_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Tx OTN ODU GCC1 value register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_ODU_GCC1_VALUE
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_GCC1_VALUE(target)  VTSS_IOREG(target,0x99f)

/** 
 * \brief
 * GCC1 value
 * The GCC1 value bits contain the ODUk general communications channel 1
 * overhead bytes (GCC1) to be inserted into the fourth row, 1st and 2nd
 * columns of outgoing frames when the insert GCC1 bit of the TX OTN ODU
 * Control register is set. These bits are cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_GCC1_VALUE . GCC1_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_GCC1_VALUE_GCC1_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_GCC1_VALUE_GCC1_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_GCC1_VALUE_GCC1_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Tx OTN ODU GCC2 value register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_ODU_GCC2_VALUE
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_GCC2_VALUE(target)  VTSS_IOREG(target,0x9a0)

/** 
 * \brief
 * GCC2 value
 * The GCC2 value bits contain the ODUk general communications channel 2
 * overhead bytes (GCC2) to be inserted into the fourth row, columns 3 and
 * 4 of outgoing frames when the insert GCC2 bit of the TX OTN ODU Control
 * register is set. These bits are cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_GCC2_VALUE . GCC2_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_GCC2_VALUE_GCC2_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_GCC2_VALUE_GCC2_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_GCC2_VALUE_GCC2_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Tx OTN ODU APS/PCC high value register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_ODU_APS_PCC_HIGH_VALUE
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_APS_PCC_HIGH_VALUE(target)  VTSS_IOREG(target,0x9a1)

/** 
 * \brief
 * APSPCC high value
 * The APS/PCC high value bits contain the upper 2 bytes of the APS/PCC
 * message (APS/PCC) to be inserted into the fourth row, columns 5 and 6 of
 * outgoing frames when the insert APS/PCC bit of the TX OTN ODU Control
 * register is set. These bits are cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_APS_PCC_HIGH_VALUE . APSPCC_HIGH_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_APS_PCC_HIGH_VALUE_APSPCC_HIGH_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_APS_PCC_HIGH_VALUE_APSPCC_HIGH_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_APS_PCC_HIGH_VALUE_APSPCC_HIGH_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Tx OTN ODU APS/PCC low value register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_ODU_APS_PCC_LOW_VALUE
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_APS_PCC_LOW_VALUE(target)  VTSS_IOREG(target,0x9a2)

/** 
 * \brief
 * APSPCC low value
 * The APS/PCC low value bits contain the lower 2 bytes of the APS/PCC
 * message (APS/PCC) to be inserted into the fourth row, columns 7 and 8 of
 * outgoing frames when the insert APS/PCC bit of the TX OTN ODU Control
 * register is set. These bits are cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_APS_PCC_LOW_VALUE . APSPCC_LOW_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_APS_PCC_LOW_VALUE_APSPCC_LOW_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_APS_PCC_LOW_VALUE_APSPCC_LOW_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_APS_PCC_LOW_VALUE_APSPCC_LOW_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Tx OTN ODU RES2 high value register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_ODU_RES2_HIGH_VALUE
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES2_HIGH_VALUE(target)  VTSS_IOREG(target,0x9a3)

/** 
 * \brief
 * RES2 high value
 * The RES2 high value bits contain the upper 2 bytes of the RES2 message
 * (RES2) to be inserted into the fourth row, columns 9 and 10 of outgoing
 * frames when the insert RES2 bit of the TX OTN ODU Control register is
 * set. These bits are cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES2_HIGH_VALUE . RES2_HIGH_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES2_HIGH_VALUE_RES2_HIGH_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES2_HIGH_VALUE_RES2_HIGH_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES2_HIGH_VALUE_RES2_HIGH_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Tx OTN ODU RES2 mid value register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_ODU_RES2_MID_VALUE
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES2_MID_VALUE(target)  VTSS_IOREG(target,0x9a4)

/** 
 * \brief
 * RES2 mid value
 * The RES2 mid value bits contain the middle 2 bytes of the RES2 message
 * (RES2) to be inserted into the fourth row, columns 11 and 12 of outgoing
 * frames when the insert RES2 bit of the TX OTN ODU Control register is
 * set. These bits are cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES2_MID_VALUE . RES2_MID_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES2_MID_VALUE_RES2_MID_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES2_MID_VALUE_RES2_MID_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES2_MID_VALUE_RES2_MID_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Tx OTN ODU RES2 low value register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_ODU_RES2_LOW_VALUE
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES2_LOW_VALUE(target)  VTSS_IOREG(target,0x9a5)

/** 
 * \brief
 * RES2 low value
 * The RES2 low value bits contain the lower 2 bytes of the RES2 message
 * (RES2) to be inserted into the fourth row, columns 13 and 14 of outgoing
 * frames when the insert RES2 bit of the TX OTN ODU Control register is
 * set. These bits are cleared as a default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES2_LOW_VALUE . RES2_LOW_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES2_LOW_VALUE_RES2_LOW_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES2_LOW_VALUE_RES2_LOW_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES2_LOW_VALUE_RES2_LOW_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Tx OTN OPU control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_OPU_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL(target)  VTSS_IOREG(target,0x9a6)

/** 
 * \brief
 * Async mapping
 * If the async mapping bit is set, an asynchronous client signal is mapped
 * into the payload of generated OTN frames. Positive and negative
 * justifications are allowed for asynchronous mapping. If the async
 * mapping bit is cleared, a synchronous client signal is mapped into the
 * payload of generated OTN frames. Positive and negative justifications
 * are not allowed for synchronous mapping. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL . ASYNC_MAPPING
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_ASYNC_MAPPING(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_ASYNC_MAPPING     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_ASYNC_MAPPING(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Insert payload type
 * If the insert payload type bit is set, the value programmed into the
 * payload type bits of the TX OTN OPU Payload Type register is inserted
 * into the PT[0] field of outgoing frames. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL . INSERT_PAYLOAD_TYPE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_INSERT_PAYLOAD_TYPE(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_INSERT_PAYLOAD_TYPE     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_INSERT_PAYLOAD_TYPE(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Force generic AIS signal
 * If the force generic AIS signal bit is set, generic AIS (PN11) is
 * inserted into the payload of outgoing frames as specified by section
 * 16.6.1 of G.709. If this bit is cleared, normal payload insertion is
 * performed unless the insert test payload bit is set This bit is cleared
 * as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL . FORCE_GENERIC_AIS_SIGNAL
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_FORCE_GENERIC_AIS_SIGNAL(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_FORCE_GENERIC_AIS_SIGNAL     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_FORCE_GENERIC_AIS_SIGNAL(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Insert test payload
 * If the insert test payload bit is set and the force generic AIS signal
 * bit is cleared, a test payload as specified by the test payload type
 * bits is inserted into outgoing OTN frames. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL . INSERT_TEST_PAYLOAD
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_INSERT_TEST_PAYLOAD(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_INSERT_TEST_PAYLOAD     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_INSERT_TEST_PAYLOAD(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Test payload type
 * If the insert test payload bit is set and the force generic AIS signal
 * bit is cleared, a test payload as specified by the test payload type
 * bits is inserted into outgoing OTN frames. The test payload
 * configurations available are as follows:
 *
 * \details 
 * 00: 8 bit fixed payload as specified by the TX OTN OPU Fixed
 * Payload/Payload Type register
 * 01: Reserved
 * 10: Pseudo random PN-23 sequence
 * 11: Pseudo random PN-31 sequence
 *
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL . TEST_PAYLOAD_TYPE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_TEST_PAYLOAD_TYPE(x)  VTSS_ENCODE_BITFIELD(x,10,2)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_TEST_PAYLOAD_TYPE     VTSS_ENCODE_BITMASK(10,2)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_TEST_PAYLOAD_TYPE(x)  VTSS_EXTRACT_BITFIELD(x,10,2)

/** 
 * \brief
 * Stuff enable
 * If the stuff enable bit is set, column bit stuffing is implemented as
 * specified by G.709. 
 * NOTE: This bit is ignored when streaming mode is enabled. This bit is
 * set as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL . STUFF_ENABLE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_STUFF_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_STUFF_ENABLE     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_STUFF_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * OPU blanking
 * If the OPU blanking bit is set, all OPU overhead byte information is
 * cleared (0x00 hex) and inserted into outgoing frames. This bit is set as
 * a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL . OPU_BLANKING
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_OPU_BLANKING(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_OPU_BLANKING     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_OPU_BLANKING(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * OPU port insert
 * If the OPU port insert bit is set, outgoing OTN frame OPU byte
 * information is inserted from the external overhead port unless a higher
 * priority insertion is valid. If this bit is cleared, OPU overhead port
 * insertion will not occur. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL . OPU_PORT_INSERT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_OPU_PORT_INSERT(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_OPU_PORT_INSERT     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_OPU_PORT_INSERT(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Force PJ
 * If the force PJ bit is set, a positive justification is forced when the
 * payload type is configured for asynchronous mapping. A single positive
 * justification occurs when this bit is set (force PJ bit will auto clear
 * when the positive justification event is complete) and will auto clear
 * immediately. This bit is only valid in asynchronous mapping mode of
 * operation. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL . FORCE_PJ
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_FORCE_PJ(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_FORCE_PJ     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_FORCE_PJ(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Force NJ
 * If the force NJ bit is set, a negative justification is forced when the
 * payload type is configured for asynchronous mapping. A single negative
 * justification occurs when this bit is set (force NJ bit will auto clear
 * when the negative justification event is complete) and will auto clear
 * immediately. This bit is only valid in asynchronous mapping mode of
 * operation. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL . FORCE_NJ
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_FORCE_NJ(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_FORCE_NJ     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_FORCE_NJ(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Force JC1 corrupt
 * If the force JC1 corrupt bit is set, the justification byte for row 1
 * column 16 will have inverted values (inverted from normal values
 * generated) for the least significant 2 bits. This bit is valid only when
 * the payload type is configured for asynchronous mapping and will auto
 * clear immediately. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL . FORCE_JC1_CORRUPT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_FORCE_JC1_CORRUPT(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_FORCE_JC1_CORRUPT     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_FORCE_JC1_CORRUPT(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Force JC2 corrupt
 * If the force JC2 corrupt bit is set, the justification byte for row 2
 * column 16 will have inverted values (inverted from normal values
 * generated) for the least significant 2 bits. This bit is valid only when
 * the payload type is configured for asynchronous mapping and will auto
 * clear immediately. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL . FORCE_JC2_CORRUPT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_FORCE_JC2_CORRUPT(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_FORCE_JC2_CORRUPT     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_FORCE_JC2_CORRUPT(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Force JC3 corrupt
 * If the force JC3 corrupt bit is set, the justification byte for row 3
 * column 16 will have inverted values (inverted from normal values
 * generated) for the least significant 2 bits. This bit is valid only when
 * the payload type is configured for asynchronous mapping and will auto
 * clear immediately. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL . FORCE_JC3_CORRUPT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_FORCE_JC3_CORRUPT(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_FORCE_JC3_CORRUPT     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_FORCE_JC3_CORRUPT(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief XCO2 Tx OTN OPU fixed payload/payload type register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_OPU_FIXED_PAYLOAD_PAYLOAD_TYPE
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIXED_PAYLOAD_PAYLOAD_TYPE(target)  VTSS_IOREG(target,0x9a7)

/** 
 * \brief
 * Fixed payload value
 * The fixed payload value bits are inserted into generated frames when the
 * insert test payload bit of the TX OTN OPU Control register is set and
 * the force generic AIS signal bit is cleared. These bits are set to a
 * value of 0xAA (hex) as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIXED_PAYLOAD_PAYLOAD_TYPE . FIXED_PAYLOAD_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIXED_PAYLOAD_PAYLOAD_TYPE_FIXED_PAYLOAD_VALUE(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIXED_PAYLOAD_PAYLOAD_TYPE_FIXED_PAYLOAD_VALUE     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIXED_PAYLOAD_PAYLOAD_TYPE_FIXED_PAYLOAD_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Payload type
 * The payload type bits are inserted into the PT[0] field of outgoing
 * frames when the insert payload type bit of the TX OTN OPU Control
 * register is set. These bits are set to a value of 0x03 (hex) as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIXED_PAYLOAD_PAYLOAD_TYPE . PAYLOAD_TYPE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIXED_PAYLOAD_PAYLOAD_TYPE_PAYLOAD_TYPE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIXED_PAYLOAD_PAYLOAD_TYPE_PAYLOAD_TYPE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIXED_PAYLOAD_PAYLOAD_TYPE_PAYLOAD_TYPE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief XCO2 Tx OTN OPU PJ counter 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_OPU_PJ
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_PJ(target)  VTSS_IOREG(target,0x9a8)

/** 
 * \brief
 * PJ counter value
 * The TX OTN OPU PJ counter is used to accumulate the number of positive
 * justifications generated by the transmit frame controller (including
 * forced positive justifications through the register interface) when an
 * asynchronous mapping payload type is configured. This is a saturating
 * read only counter.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_PJ . PJ_COUNTER_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_PJ_PJ_COUNTER_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_PJ_PJ_COUNTER_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_PJ_PJ_COUNTER_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Tx OTN OPU NJ counter 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_OPU_NJ
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_NJ(target)  VTSS_IOREG(target,0x9a9)

/** 
 * \brief
 * NJ counter value
 * The TX OTN OPU NJ counter is used to accumulate the number of negative
 * justifications generated by the transmit frame controller (including
 * forced negative justifications through the register interface) when an
 * asynchronous mapping payload type is configured. This is a saturating
 * read only counter.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_NJ . NJ_COUNTER_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_NJ_NJ_COUNTER_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_NJ_NJ_COUNTER_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_NJ_NJ_COUNTER_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Tx OTN OPU FIFO high threshold register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_OPU_FIFO_HIGH_THRESHOLD
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIFO_HIGH_THRESHOLD(target)  VTSS_IOREG(target,0x9aa)

/** 
 * \brief
 * FIFO high threshold value
 * If the difference between the Tx FIFO write address (system side) and
 * the Tx FIFO read address (line side) is more than the value programmed
 * into the FIFO high threshold value bits, the Tx SYS UPPER THRESH OUT
 * output signal is asserted indicating a FIFO high threshold condition
 * exists.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIFO_HIGH_THRESHOLD . FIFO_HIGH_THRESHOLD_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIFO_HIGH_THRESHOLD_FIFO_HIGH_THRESHOLD_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIFO_HIGH_THRESHOLD_FIFO_HIGH_THRESHOLD_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIFO_HIGH_THRESHOLD_FIFO_HIGH_THRESHOLD_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Tx OTN OPU FIFO low threshold register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_OPU_FIFO_LOW_THRESHOLD
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIFO_LOW_THRESHOLD(target)  VTSS_IOREG(target,0x9ab)

/** 
 * \brief
 * FIFO low threshold value
 * If the difference between the Tx FIFO write address (system side) and
 * the Tx FIFO read address (line side) is less than the value programmed
 * into the FIFO low threshold value bits, the TX SYS LOWER THRESH OUT
 * output signal is asserted indicating a FIFO low threshold condition
 * exists.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIFO_LOW_THRESHOLD . FIFO_LOW_THRESHOLD_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIFO_LOW_THRESHOLD_FIFO_LOW_THRESHOLD_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIFO_LOW_THRESHOLD_FIFO_LOW_THRESHOLD_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIFO_LOW_THRESHOLD_FIFO_LOW_THRESHOLD_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Tx OTN OPU FIFO NJ threshold register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_OPU_FIFO_NJ_THRESHOLD
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIFO_NJ_THRESHOLD(target)  VTSS_IOREG(target,0x9ac)

/** 
 * \brief
 * FIFO NJ threshold value
 * If the difference between the Tx FIFO write address (system side) and
 * the Tx FIFO read address (line side) is more than the value programmed
 * into the FIFO NJ threshold value bits, and asynchronous mapping is
 * enabled (see description of the async mapping bits of the TX OTN OPU
 * Control register), a negative justification will be performed for the
 * current generated frame. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIFO_NJ_THRESHOLD . FIFO_NJ_THRESHOLD_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIFO_NJ_THRESHOLD_FIFO_NJ_THRESHOLD_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIFO_NJ_THRESHOLD_FIFO_NJ_THRESHOLD_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIFO_NJ_THRESHOLD_FIFO_NJ_THRESHOLD_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Tx OTN OPU FIFO PJ threshold register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_OPU_FIFO_PJ_THRESHOLD
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIFO_PJ_THRESHOLD(target)  VTSS_IOREG(target,0x9ad)

/** 
 * \brief
 * FIFO PJ threshold value
 * If the difference between the Tx FIFO write address (system side) and
 * the Tx FIFO read address (line side) is less than the value programmed
 * into the FIFO PJ threshold value bits, and asynchronous mapping is
 * enabled (see description of the async mapping bits of the TX OTN OPU
 * Control register), a positive justification will be performed for the
 * current generated frame. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIFO_PJ_THRESHOLD . FIFO_PJ_THRESHOLD_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIFO_PJ_THRESHOLD_FIFO_PJ_THRESHOLD_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIFO_PJ_THRESHOLD_FIFO_PJ_THRESHOLD_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIFO_PJ_THRESHOLD_FIFO_PJ_THRESHOLD_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Tx OTN ODU TCMi interpret counter control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL(target)  VTSS_IOREG(target,0x9ae)

/** 
 * \brief
 * TCM1 BIP latch and clear
 * If the TCM1 BIP latch and clear bit is set, the current TCM1 interpret
 * BIP error count value will be latched and transferred to the TCM1 BIP
 * counter value bits of the RX OTN ODU TCM1 Interpret BIP counter. The
 * TCM1 BIP counter value is available to be read until the next TCM1 BIP
 * latch and clear event occurs. The RX OTN ODU TCM1 Interpret BIP counter
 * should not be read after asserting the TCM1 BIP latch and clear bit
 * until this bit is auto cleared. The TCM1 BIP latch and clear bit is
 * cleared as a default and is self clearing after a write. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL . TCM1_BIP_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM1_BIP_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM1_BIP_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM1_BIP_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * TCM1 BEI latch and clear
 * If the TCM1 BEI latch and clear bit is set, the current TCM1 interpret
 * BEI error count value will be latched and transferred to the TCM1 BEI
 * counter value bits of the RX OTN ODU TCM1 Interpret BEI counter. The
 * TCM1 BEI counter value is available to be read until the next TCM1 BEI
 * latch and clear event occurs. The RX OTN ODU TCM1 Interpret BEI counter
 * should not be read after asserting the TCM1 BEI latch and clear bit
 * until this bit is auto cleared. The TCM1BEI latch and clear bit is
 * cleared as a default and is self clearing after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL . TCM1_BEI_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM1_BEI_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM1_BEI_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM1_BEI_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * TCM2 BIP latch and clear
 * If the TCM2 BIP latch and clear bit is set, the current TCM2 interpret
 * BIP error count value will be latched and transferred to the TCM2 BIP
 * counter value bits of the RX OTN ODU TCM2 Interpret BIP counter. The
 * TCM2 BIP counter value is available to be read until the next TCM2 BIP
 * latch and clear event occurs. The RX OTN ODU TCM2 Interpret BIP counter
 * should not be read after asserting the TCM2 BIP latch and clear bit
 * until this bit is auto cleared. The TCM2 BIP latch and clear bit is
 * cleared as a default and is self clearing after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL . TCM2_BIP_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM2_BIP_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM2_BIP_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM2_BIP_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * TCM2 BEI latch and clear
 * If the TCM2 BEI latch and clear bit is set, the current TCM2 interpret
 * BEI error count value will be latched and transferred to the TCM2 BEI
 * counter value bits of the RX OTN ODU TCM2 Interpret BEI counter. The
 * TCM2 BEI counter value is available to be read until the next TCM2 BEI
 * latch and clear event occurs. The RX OTN ODU TCM2 Interpret BEI counter
 * should not be read after asserting the TCM2 BEI latch and clear bit
 * until this bit is auto cleared. The TCM2BEI latch and clear bit is
 * cleared as a default and is self clearing after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL . TCM2_BEI_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM2_BEI_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM2_BEI_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM2_BEI_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * TCM3 BIP latch and clear
 * If the TCM3 BIP latch and clear bit is set, the current TCM3 interpret
 * BIP error count value is latched and transferred to the TCM3 BIP counter
 * value bits of the RX OTN ODU TCM3 Interpret BIP counter. The TCM3 BIP
 * counter value is available to be read until the next TCM3 BIP latch and
 * clear event occurs. The RX OTN ODU TCM3 Interpret BIP counter should not
 * be read after asserting the TCM3 BIP latch and clear bit until this bit
 * is auto cleared. This bit is cleared as a default and is self clearing
 * after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL . TCM3_BIP_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM3_BIP_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM3_BIP_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM3_BIP_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * TCM3 BEI latch and clear
 * If the TCM3 BEI latch and clear bit is set, the current TCM3 interpret
 * BEI error count value is latched and transferred to the TCM3 BEI counter
 * value bits of the RX OTN ODU TCM3 Interpret BEI counter. The TCM3 BEI
 * counter value is available to be read until the next TCM3 BEI latch and
 * clear event occurs. The RX OTN ODU TCM3 Interpret BEI counter should not
 * be read after asserting the TCM3 BEI latch and clear bit until this bit
 * is auto cleared. This bit is cleared as a default and is self clearing
 * after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL . TCM3_BEI_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM3_BEI_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM3_BEI_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM3_BEI_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * TCM4 BIP latch and clear
 * If the TCM4 BIP latch and clear bit is set, the current TCM4 interpret
 * BIP error count value is latched and transferred to the TCM4 BIP counter
 * value bits of the RX OTN ODU TCM4 Interpret BIP counter. The TCM4 BIP
 * counter value is available to be read until the next TCM4 BIP latch and
 * clear event occurs. The RX OTN ODU TCM4 Interpret BIP counter should not
 * be read after asserting the TCM4 BIP latch and clear bit until this bit
 * is auto cleared. This bit is cleared as a default and is self clearing
 * after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL . TCM4_BIP_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM4_BIP_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM4_BIP_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM4_BIP_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * TCM4 BEI latch and clear
 * If the TCM4 BEI latch and clear bit is set, the current TCM4 interpret
 * BEI error count value is latched and transferred to the TCM4 BEI counter
 * value bits of the RX OTN ODU TCM4 Interpret BEI counter. The TCM4 BEI
 * counter value is available to be read until the next TCM4 BEI latch and
 * clear event occurs. The RX OTN ODU TCM4 Interpret BEI counter should not
 * be read after asserting the TCM4 BEI latch and clear bit until this bit
 * is auto cleared. This bit is cleared as a default and is self clearing
 * after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL . TCM4_BEI_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM4_BEI_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM4_BEI_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM4_BEI_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * TCM5 BIP latch and clear
 * If the TCM5 BIP latch and clear bit is set, the current TCM5 interpret
 * BIP error count value is latched and transferred to the TCM5 BIP counter
 * value bits of the RX OTN ODU TCM5 Interpret BIP counter. The TCM5 BIP
 * counter value is available to be read until the next TCM5 BIP latch and
 * clear event occurs. The RX OTN ODU TCM5 Interpret BIP counter should not
 * be read after asserting the TCM5 BIP latch and clear bit until this bit
 * is auto cleared. This bit is cleared as a default and is self clearing
 * after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL . TCM5_BIP_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM5_BIP_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM5_BIP_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM5_BIP_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * TCM5 BEI latch and clear
 * If the TCM5 BEI latch and clear bit is set, the current TCM5 interpret
 * BEI error count value is latched and transferred to the TCM5 BEI counter
 * value bits of the RX OTN ODU TCM5 Interpret BEI counter. The TCM5 BEI
 * counter value is available to be read until the next TCM5 BEI latch and
 * clear event occurs. The RX OTN ODU TCM5 Interpret BEI counter should not
 * be read after asserting the TCM5 BEI latch and clear bit until this bit
 * is auto cleared. This bit is cleared as a default and is self clearing
 * after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL . TCM5_BEI_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM5_BEI_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM5_BEI_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM5_BEI_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * TCM6 BIP latch and clear
 * If the TCM6 BIP latch and clear bit is set, the current TCM6 interpret
 * BIP error count value is latched and transferred to the TCM6 BIP counter
 * value bits of the RX OTN ODU TCM6 Interpret BIP counter. The TCM6 BIP
 * counter value is available to be read until the next TCM6 BIP latch and
 * clear event occurs. The RX OTN ODU CM6 Interpret BIP counter should not
 * be read after asserting the TCM6 BIP latch and clear bit until this bit
 * is auto cleared. This bit is cleared as a default and is self clearing
 * after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL . TCM6_BIP_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM6_BIP_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM6_BIP_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM6_BIP_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * TCM6 BEI latch and clear
 * If the TCM6 BEI latch and clear bit is set, the current TCM6 interpret
 * BEI error count value is latched and transferred to the TCM6 BEI counter
 * value bits of the RX OTN ODU TCM6 Interpret BEI counter. The TCM6 BEI
 * counter value is available to be read until the next TCM6 BEI latch and
 * clear event occurs. The RX OTN ODU TCM6 Interpret BEI counter should not
 * be read after asserting the TCM6 BEI latch and clear bit until this bit
 * is auto cleared. The TCM6BEI latch and clear bit is cleared as a default
 * and is self clearing after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL . TCM6_BEI_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM6_BEI_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM6_BEI_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_TCM6_BEI_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Block mode
 * If the block mode bit is set, TCMi BEI and BIP error counts accumulate
 * on a block basis, and the interpret TCMi counts (BIP and BEI) only
 * increment by 1 (block count) each frame when 1 or more TCMi errors are
 * detected. If the block mode bit is cleared, TCMi BEI and BIP error
 * counts accumulate the actual number of errors (bit count) detected each
 * frame. This bit is set as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL . BLOCK_MODE0
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_BLOCK_MODE0(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_BLOCK_MODE0     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_CONTROL_BLOCK_MODE0(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief XCO2 Tx OTN ODU TCMi interpret AIS control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL(target)  VTSS_IOREG(target,0x9af)

/** 
 * \brief
 * Auto TCMi ODU AIS
 * If the auto TCMi ODU AIS bit is set, the transmit processor frame
 * controller automatically generates ODU AIS frames if any of the ODU AIS
 * TCMi TIM/OCI/LCK bits are set and the corresponding condition is
 * detected. ODU AIS is removed when either the appropriate condition is
 * removed or the enabling bit that caused ODU AIS is cleared. The Auto
 * TCMi ODU AIS bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL . AUTO_TCMI_ODU_AIS
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL_AUTO_TCMI_ODU_AIS(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL_AUTO_TCMI_ODU_AIS     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL_AUTO_TCMI_ODU_AIS(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * ODU AIS TCM1 TIMOCILCK
 * If the ODU AIS TCM1 TIM/OCI/LCK bit is set, the transmit processor frame
 * controller generates ODU AIS frames when a TX TCM1 TIM/OCI/LCK condition
 * is detected and the auto TCMi ODU AIS bit is set. If the ODU AIS TCM1
 * TIM/OCI/LCK bit or auto TCMi ODU AIS bit is cleared, ODU AIS is not
 * inserted due to TCM1 TIM/OCI/LCK detection. The ODU AIS TCM1 TIM/OCI/LCK
 * bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL . ODU_AIS_TCM1_TIMOCILCK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL_ODU_AIS_TCM1_TIMOCILCK(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL_ODU_AIS_TCM1_TIMOCILCK     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL_ODU_AIS_TCM1_TIMOCILCK(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * ODU AIS TCM2 TIMOCILCK
 * If the ODU AIS TCM2 TIM/OCI/LCK bit is set, the transmit processor frame
 * controller generates ODU AIS frames when a TX TCM2 TIM/OCI/LCK condition
 * is detected and the auto TCMi ODU AIS bit is set. If the ODU AIS TCM2
 * TIM/OCI/LCK bit or auto TCMi ODU AIS bit is cleared, ODU AIS is not
 * inserted due to TCM2 TIM/OCI/LCK detection. The ODU AIS TCM2 TIM/OCI/LCK
 * bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL . ODU_AIS_TCM2_TIMOCILCK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL_ODU_AIS_TCM2_TIMOCILCK(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL_ODU_AIS_TCM2_TIMOCILCK     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL_ODU_AIS_TCM2_TIMOCILCK(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * ODU AIS TCM3 TIMOCILCK
 * If the ODU AIS TCM3 TIM/OCI/LCK bit is set, the transmit processor frame
 * controller generates ODU AIS frames when a TX TCM3 TIM/OCI/LCK condition
 * is detected and the auto TCMi ODU AIS bit is set. If the ODU AIS TCM3
 * TIM/OCI/LCK bit or auto TCMi ODU AIS bit is cleared, ODU AIS is not
 * inserted due to TCM3 TIM/OCI/LCK detection. The ODU AIS TCM3 TIM/OCI/LCK
 * bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL . ODU_AIS_TCM3_TIMOCILCK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL_ODU_AIS_TCM3_TIMOCILCK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL_ODU_AIS_TCM3_TIMOCILCK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL_ODU_AIS_TCM3_TIMOCILCK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * ODU AIS TCM4 TIMOCILCK
 * If the ODU AIS TCM4 TIM/OCI/LCK bit is set, the transmit processor frame
 * controller generates ODU AIS frames when a TX TCM4 TIM/OCI/LCK condition
 * is detected and the auto TCMi ODU AIS bit is set. If the ODU AIS TCM4
 * TIM/OCI/LCK bit or auto TCMi ODU AIS bit is cleared, OTU/ODU AIS is not
 * inserted due to TCM4 TIM/OCI/LCK detection. The ODU AIS TCM4 TIM/OCI/LCK
 * bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL . ODU_AIS_TCM4_TIMOCILCK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL_ODU_AIS_TCM4_TIMOCILCK(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL_ODU_AIS_TCM4_TIMOCILCK     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL_ODU_AIS_TCM4_TIMOCILCK(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * ODU AIS TCM5 TIMOCILCK
 * If the ODU AIS TCM5 TIM/OCI/LCK bit is set, the transmit processor frame
 * controller generates ODU AIS frames when a TX TCM5 TIM/OCI/LCK condition
 * is detected and the auto TCMi ODU AIS bit is set. If the ODU AIS TCM5
 * TIM/OCI/LCK bit or auto TCMi ODU AIS bit is cleared, ODU AIS is not
 * inserted due to TCM5 TIM/OCI/LCK detection. The ODU AIS TCM5 TIM/OCI/LCK
 * bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL . ODU_AIS_TCM5_TIMOCILCK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL_ODU_AIS_TCM5_TIMOCILCK(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL_ODU_AIS_TCM5_TIMOCILCK     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL_ODU_AIS_TCM5_TIMOCILCK(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * ODU AIS TCM6 TIMOCILCK
 * If the ODU AIS TCM6 TIM/OCI/LCK bit is set, the transmit processor frame
 * controller generates ODU AIS frames when a TX TCM6 TIM/OCI/LCK condition
 * is detected and the auto TCMi ODU AIS bit is set. If the ODU AIS TCM6
 * TIM/OCI/LCK bit or auto TCMi ODU AIS bit is cleared, ODU AIS is not
 * inserted due to TCM6 TIM/OCI/LCK detection. The ODU AIS TCM6 TIM/OCI/LCK
 * bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL . ODU_AIS_TCM6_TIMOCILCK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL_ODU_AIS_TCM6_TIMOCILCK(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL_ODU_AIS_TCM6_TIMOCILCK     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_INTERPRET_AIS_CONTROL_ODU_AIS_TCM6_TIMOCILCK(x)  VTSS_EXTRACT_BITFIELD(x,9,1)


/** 
 * \brief XCO2 Tx OTN ODU TCMX interpret control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL (??), 0-5
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL(target,ri)  VTSS_IOREG(target,0x9b0 + (ri))

/** 
 * \brief
 * TCMi TTI SAPI TIM
 * The TCMi TTI SAPI TIM bit is set, the TCMi TTI SAPI bytes are included
 * in the comparison of the accepted TCMi TTI and expected TCMi TTI bytes
 * for trace identifier mismatch (TCMi TIM) detection. This bit is cleared
 * as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL . TCMI_TTI_SAPI_TIM
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_TTI_SAPI_TIM(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_TTI_SAPI_TIM     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_TTI_SAPI_TIM(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * TCMi TTI DAPI TIM
 * The TCMi TTI DAPI TIM bit is set, the TCMi TTI DAPI bytes are included
 * in the comparison of the accepted TCMi TTI and expected TCMi TTI bytes
 * for trace identifier mismatch (TCMi TIM) detection. This bit is cleared
 * as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL . TCMI_TTI_DAPI_TIM
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_TTI_DAPI_TIM(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_TTI_DAPI_TIM     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_TTI_DAPI_TIM(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * TCMi TTI OS TIM
 * The TCMi TTI OS TIM bit is set, the TCMi TTI operator specific bytes are
 * included in the comparison of the accepted TCMi TTI and expected TCMi
 * TTI bytes for trace identifier mismatch (TCMi TIM) detection. This bit
 * is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL . TCMI_TTI_OS_TIM
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_TTI_OS_TIM(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_TTI_OS_TIM     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_TTI_OS_TIM(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * TCMi STAT filter 5
 * The TCMi STAT filter 5 bit is set to indicate that 5 consecutive frames
 * with identical STAT fields are required to update the TCMi STAT value
 * bits. If this bit is cleared, 3 consecutive frames with identical STAT
 * field values update the TCMi STAT value bits. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL . TCMI_STAT_FILTER_5
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_STAT_FILTER_5(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_STAT_FILTER_5     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_STAT_FILTER_5(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * TCMi STAT accept value
 * The TCMi STAT accept value bits are updated when identical extracted
 * TCMi STAT bytes are received for the number of consecutive sequences
 * programmed through the TCMi STAT filter 5 bits. The TCMi STAT accept
 * value bits have a default value of 000 and are read only bits.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL . TCMI_STAT_ACCEPT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_STAT_ACCEPT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_STAT_ACCEPT_VALUE     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_STAT_ACCEPT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * TCMi BIAE filter 5
 * The TCMi BIAE filter 5 bit is set to indicate that 5 consecutive frames
 * with TCMi BIAE field values of 1011 are required to detect an incoming
 * TCMi BIAE error condition. If this bit is cleared, 3 consecutive frames
 * with TCMi BIAE field values of 1011 result in TCMi BIAE error detection.
 * Likewise, the TCMi BIAE filter 5 bit is used to determine the number of
 * consecutive (3 or 5) frames with TCMi BIAE field values not equal to
 * 1011 required to transition from a TCMi BIAE error state to an error
 * free TCMi BIAE condition. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL . TCMI_BIAE_FILTER_5
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_BIAE_FILTER_5(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_BIAE_FILTER_5     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_BIAE_FILTER_5(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * TCMi BDI filter 5
 * The TCMi BDI filter 5 bit is set to indicate that 5 consecutive frames
 * with TCMi BDI field values of 1 are required to detect an incoming TCMi
 * BDI error condition. If this bit is cleared, 3 consecutive frames with
 * TCMi BDI field value of 1 results in TCMi BDI error detection. Likewise,
 * the TCMi BDI filter 5 bit is used to determine the number of consecutive
 * (3 or 5) frames with TCMi BDI field values of 0 required to transition
 * from a TCMi BDI error state to an error free TCMi BDI condition. This
 * bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL . TCMI_BDI_FILTER_5
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_BDI_FILTER_5(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_BDI_FILTER_5     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_BDI_FILTER_5(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * TCMi TTI accept count
 * The TCMi TTI accept count bits are used to determine the number of
 * identical TCMi TTI messages that must be received consecutively in order
 * to update the TCMi TTI extract value bits of the RX OTN ODU TCMi TTI
 * Extract Memory with the current TCMi TTI message. 
 * NOTE: A count of 0 is invalid and should not be programmed. These bits
 * are set to a default value of 5. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL . TCMI_TTI_ACCEPT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_TTI_ACCEPT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,3,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_TTI_ACCEPT_COUNT     VTSS_ENCODE_BITMASK(3,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_TTI_ACCEPT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,3,3)

/** 
 * \brief
 * TCMi TTI inconsistent count
 * The TCMi TTI inconsistent count bits are used to determine the number of
 * non-identical TCMi TTI messages that must be received in order to
 * declare a TTI inconsistent error. 
 * NOTE: A count of 0 is invalid and should not be programmed. These bits
 * are set to a default value of 5.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL . TCMI_TTI_INCONSISTENT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_TTI_INCONSISTENT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_TTI_INCONSISTENT_COUNT     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_CONTROL_TCMI_TTI_INCONSISTENT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief XCO2 Tx OTN ODU TCMX interpret TTI extract memory 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_ODU_TCMX_INTERPRET_TTI_EXTRACT
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: XCO2_TX_OTN_ODU_TCMX_INTERPRET_TTI_EXTRACT (??), 0-191
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_TTI_EXTRACT(target,ri)  VTSS_IOREG(target,0x9b6 + (ri))

/** 
 * \brief
 * TCMi TTI extract value
 * The TCMi TTI extract value bits contain the accepted 64 byte trail trace
 * identifier (SAPI/DAPI/Operator Specific) message extracted from incoming
 * frames. This information is used for comparison to the expected TCMi TTI
 * message information for TCMi trail trace identifier mismatch (TCMi TIM)
 * detection. These bits are undefined as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_TTI_EXTRACT . TCMI_TTI_EXTRACT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_TTI_EXTRACT_TCMI_TTI_EXTRACT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_TTI_EXTRACT_TCMI_TTI_EXTRACT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_TTI_EXTRACT_TCMI_TTI_EXTRACT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Tx OTN ODU TCMX interpret TTI expect memory 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_ODU_TCMX_INTERPRET_TTI_EXPECT
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: XCO2_TX_OTN_ODU_TCMX_INTERPRET_TTI_EXPECT (??), 0-191
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_TTI_EXPECT(target,ri)  VTSS_IOREG(target,0xa76 + (ri))

/** 
 * \brief
 * TCMi TTI expect value
 * The TCMi TTI expect value bits contain the 64 byte trail trace
 * identifier (SAPI/DAPI/Operator Specific) message to be used for
 * comparison to the extracted TCMi TTI message information for TCMi trail
 * trace identifier mismatch (TCMi TIM) detection. These bits are undefined
 * as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_TTI_EXPECT . TCMI_TTI_EXPECT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_TTI_EXPECT_TCMI_TTI_EXPECT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_TTI_EXPECT_TCMI_TTI_EXPECT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_TTI_EXPECT_TCMI_TTI_EXPECT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Tx OTN ODU TCMX interpret BEI counter 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_ODU_TCMX_INTERPRET_BEI
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: XCO2_TX_OTN_ODU_TCMX_INTERPRET_BEI (??), 0-5
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_BEI(target,ri)  VTSS_IOREG(target,0xb42 + (ri))

/** 
 * \brief
 * TCMi BEI counter value
 * The TCMi BEI counter value bits are used to accumulate the number of BEI
 * errors detected in the TCMi BEI field of the ODU overhead. This is a
 * saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_BEI . TCMI_BEI_COUNTER_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_BEI_TCMI_BEI_COUNTER_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_BEI_TCMI_BEI_COUNTER_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_BEI_TCMI_BEI_COUNTER_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Tx OTN ODU TCMX interpret BIP counter 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_TX_OTN::XCO2_TX_OTN_ODU_TCMX_INTERPRET_BIP
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: XCO2_TX_OTN_ODU_TCMX_INTERPRET_BIP (??), 0-5
 */
#define VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_BIP(target,ri)  VTSS_IOREG(target,0xb48 + (ri))

/** 
 * \brief
 * TCMi BIP counter value
 * The TCMi BIP counter value bits are used to accumulate the number of
 * TCMi BIP errors detected for incoming frames. This is a saturating read
 * only counter.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_BIP . TCMI_BIP_COUNTER_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_BIP_TCMI_BIP_COUNTER_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_BIP_TCMI_BIP_COUNTER_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_INTERPRET_BIP_TCMI_BIP_COUNTER_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a OTN_WRAPPER::XCO2_RX_OTN
 *
 * XCO2 OTN block Rx registers
 */


/** 
 * \brief XCO2 Rx OTN global control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_GLOBAL_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL(target)  VTSS_IOREG(target,0xc00)

/** 
 * \brief
 * Streaming mode
 * If the streaming mode bit is set, the XCO2 Rx processor passes streaming
 * ODU frame information out the system side interface. If the streaming
 * mode bit is cleared, normal mode is enabled and payload information is
 * transmitted out the receive system side interface. This bit is cleared
 * as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL . STREAMING_MODE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_STREAMING_MODE(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_STREAMING_MODE     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_STREAMING_MODE(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Bypass mode
 * If the bypass mode bit is set, the XCO2 Rx processor writes incoming
 * line side data directly into the line side of the receive processor
 * without any OTN frame processing of any kind. If the bypass mode bit is
 * cleared, normal receive processing occurs. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL . BYPASS_MODE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_BYPASS_MODE(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_BYPASS_MODE     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_BYPASS_MODE(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * SW reset
 * If the sw reset bit is set, the XCO2 Rx processor performs a synchronous
 * reset operation. This state is equivalent to an external hardware reset
 * of the device for the Rx processor. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL . SW_RESET
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_SW_RESET(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_SW_RESET     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_SW_RESET(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Scramble enable
 * If the scramble enable bit is set, incoming data is descrambled as per
 * the G.709 standard. If this bit is cleared, incoming data scrambling is
 * inhibited. This bit is set as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL . SCRAMBLE_ENABLE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_SCRAMBLE_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_SCRAMBLE_ENABLE     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_SCRAMBLE_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Scramble corrupt
 * If the scramble corrupt bit is set, incoming scrambled data is corrupted
 * by inverting the OPU scramble sequence value contained in the first row
 * and 96th column of received OTN frames. If this bit is cleared, normal
 * OTN frame scrambling is enabled. This bit is cleared as a default value.
 * 
 * NOTE: Scramble corrupt functionality is only valid when the scramble
 * enable bit is set. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL . SCRAMBLE_CORRUPT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_SCRAMBLE_CORRUPT(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_SCRAMBLE_CORRUPT     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_SCRAMBLE_CORRUPT(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * FIFO reset
 * If the FIFO reset bit is set, the XCO2 core resets the receive processor
 * FIFO write pointer. When the FIFO is reset, the write pointer (line side
 * of FIFO) address is set to a value that is equal to half the depth of
 * the FIFO away from the read pointer (system side of FIFO). This bit is
 * auto cleared when set. This bit is auto cleared when set and it is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL . FIFO_RESET
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_FIFO_RESET(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_FIFO_RESET     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_FIFO_RESET(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * OTUk frame type
 * If the OTUk frame type bit is set, incoming frames are in OTUk format
 * with FEC information included. If the OTUk frame type bit is cleared,
 * incoming frames are in ODUk format with no FEC check bytes included.
 * This bit is set as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL . OTUK_FRAME_TYPE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_OTUK_FRAME_TYPE(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_OTUK_FRAME_TYPE     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_OTUK_FRAME_TYPE(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * FEC enable
 * The FEC enable bit is set to enable FEC decoder functionality including
 * error correction capability. This bit is only valid when the OTUk frame
 * type bit is asserted. This bit is set as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL . FEC_ENABLE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_FEC_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_FEC_ENABLE     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_FEC_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Err sync mode
 * If the err sync mode bit is set, all internal counters are captured on
 * the rising edge of the incoming ERR SYNC IN internal input (accessible
 * as PMTICK) and transferred to their respective shadow memory locations
 * for user access (user read operations). In the err sync mode of
 * operation, latch and clear operation for all counters is still valid if
 * the user wants the updated count between error sync intervals. If the
 * err sync mode bit is cleared, counter latch and clear mode is enabled
 * and internal counters are captured and made available for the user to
 * read only when the designated counter latch and clear bit is asserted.
 * Internal counters are always cleared when counts are captured and
 * transferred through either err sync or latch and clear modes of
 * operation. This bit is cleared (latch and clear mode enabled) as a
 * default value.
 * NOTE: Minimum pulse width for the ERR SYNC IN (PMTICK) signal is 2
 * register interface clocks.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL . ERR_SYNC_MODE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_ERR_SYNC_MODE(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_ERR_SYNC_MODE     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_ERR_SYNC_MODE(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * FEC BER select
 * If the FEC BER select bit is set and the incoming bit error rate exceeds
 * 10-4, an interrupt (BER exceeded IRQ bit in RX OTN FA Interrupt
 * register) is generated and the BER EXCEEDED output is asserted until the
 * bit error rate falls below 10-4. Likewise, if the FEC BER select bit is
 * cleared and the incoming bit error rate exceeds 10-3, an interrupt (BER
 * exceeded IRQ bit in RX OTN FA Interrupt register) is generated and the
 * BER EXCEEDED output is asserted until the bit error rate falls below
 * 10-3. This bit is cleared as a default value.
 * NOTE: The FEC BER select bit is only valid when the FEC decoder is
 * enabled.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL . FEC_BER_SELECT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_FEC_BER_SELECT(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_FEC_BER_SELECT     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_FEC_BER_SELECT(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * External FEC
 * The external FEC bit is set to enable external FEC processing using the
 * XCO2 external FEC interface signaling inputs/outputs. This bit is
 * cleared to enable internal Reed Solomon RS(255,239) Forward Error
 * Correction processing as outlined in the G.709 specification. This bit
 * is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL . EXTERNAL_FEC
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_EXTERNAL_FEC(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_EXTERNAL_FEC     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_EXTERNAL_FEC(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Scramble post FEC
 * The scramble post FEC bit is set to enable frame scrambling after to FEC
 * decoding. This operation is required for some enhanced FEC modes
 * (enabled through external FEC register bit) of operation. The scramble
 * post FEC bit is cleared to enable frame scrambling prior to FEC
 * decoding. This scramble post FEC bit is cleared for normal Reed Solomon
 * RS(255,239) FEC processing. The scramble post FEC bit is valid only when
 * the external FEC bit is set. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL . SCRAMBLE_POST_FEC
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_SCRAMBLE_POST_FEC(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_SCRAMBLE_POST_FEC     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_SCRAMBLE_POST_FEC(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Extended OPU payload
 * If the extended OPU payload bit is set, all OPU overhead fields except
 * for the OPU PSI OH location are used for payload data as described in
 * G.709 clause 17.3. If this bit is cleared, normal OPU OH processing is
 * enabled. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL . EXTENDED_OPU_PAYLOAD
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_EXTENDED_OPU_PAYLOAD(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_EXTENDED_OPU_PAYLOAD     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_EXTENDED_OPU_PAYLOAD(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Rx facility loopback
 * If the Rx facility loopback bit is set, incoming frames are looped back
 * to the Tx processor line side outputs. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL . RX_FACILITY_LOOPBACK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_RX_FACILITY_LOOPBACK(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_RX_FACILITY_LOOPBACK     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_RX_FACILITY_LOOPBACK(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Rx terminal loopback
 * If the Rx terminal loopback bit is set, outgoing frames are looped back
 * to the Tx processor system side inputs. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL . RX_TERMINAL_LOOPBACK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_RX_TERMINAL_LOOPBACK(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_RX_TERMINAL_LOOPBACK     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_RX_TERMINAL_LOOPBACK(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief XCO2 Rx OTN FA/FEC/OTU/OPU counter control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL(target)  VTSS_IOREG(target,0xc01)

/** 
 * \brief
 * OOF latch and clear
 * If the OOF latch and clear bit is set, the XCO2 latches the current OOF
 * error count value and transfers it to the OOF counter value bits of the
 * RX OTN FA OOF counter. The OOF counter value is available to be read
 * until the next OOF latch and clear event occurs. The RX OTN FA OOF
 * counter should not be read after asserting the OOF latch and clear bit
 * until this bit is auto cleared. The OOF latch and clear bit is cleared
 * as a default and is self clearing after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL . OOF_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_OOF_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_OOF_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_OOF_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * OOM latch and clear
 * If the OOM latch and clear bit is set, the XCO2 latches the current OOM
 * error count value and transfers it to the OOM counter value bits of the
 * RX OTN FA OOF counter. The OOM counter value is available to be read
 * until the next OOM latch and clear event occurs. The RX OTN FA OOF
 * counter should not be read after asserting the OOM latch and clear bit
 * until this bit is auto cleared. The OOM latch and clear bit is cleared
 * as a default and is self clearing after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL . OOM_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_OOM_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_OOM_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_OOM_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * FEC correctable symbols latch and clear
 * If the FEC correctable symbols latch and clear bit is set, the XCO2
 * latches the current FEC correctable symbols count value and transfers it
 * to the FEC correctable symbols counter value bits of the RX OTN FEC
 * Correctable Symbols counter. The FEC correctable symbols counter value
 * is available to be read until the next FEC correctable symbols latch and
 * clear event occurs. The RX OTN FEC Correctable Symbols counter should
 * not be read after asserting the FEC correctable symbols count error
 * latch and clear bit until this bit is auto cleared. The FEC correctable
 * symbols latch and clear bit is cleared as a default and is self clearing
 * after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL . FEC_CORRECTABLE_SYMBOLS_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_FEC_CORRECTABLE_SYMBOLS_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_FEC_CORRECTABLE_SYMBOLS_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_FEC_CORRECTABLE_SYMBOLS_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * FEC correctable zeros latch and clear
 * If the FEC correctable zeros latch and clear bit is set, the XCO2
 * latches the current FEC correctable zeros count value and transfers it
 * to the FEC correctable zeros counter value bits of the RX OTN FEC
 * Correctable Zeros counter. The FEC correctable zeros counter value is
 * available to be read until the next FEC correctable zeros latch and
 * clear event occurs. The RX OTN FEC Correctable Zeros counter should not
 * be read after asserting the FEC correctable zeros count error latch and
 * clear bit until this bit is auto cleared. The FEC correctable zeros
 * latch and clear bit is cleared as a default and is self clearing after a
 * write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL . FEC_CORRECTABLE_ZEROS_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_FEC_CORRECTABLE_ZEROS_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_FEC_CORRECTABLE_ZEROS_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_FEC_CORRECTABLE_ZEROS_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * FEC correctable ones latch and clear
 * If the FEC correctable ones latch and clear bit is set, the XCO2 latches
 * the current FEC correctable ones count value and transfers it to the FEC
 * correctable ones counter value bits of the RX OTN FEC Correctable Ones
 * counter. The FEC correctable ones counter value is available to be read
 * until the next FEC correctable ones latch and clear event occurs. The RX
 * OTN FEC Correctable Ones counter should not be read after asserting the
 * FEC correctable ones count error latch and clear bit until this bit is
 * auto cleared. The FEC correctable ones latch and clear bit is cleared as
 * a default and is self clearing after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL . FEC_CORRECTABLE_ONES_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_FEC_CORRECTABLE_ONES_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_FEC_CORRECTABLE_ONES_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_FEC_CORRECTABLE_ONES_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * FEC uncorrectable codewords latch and clear
 * If the FEC uncorrectable codewords error latch and clear bit is set, the
 * XCO2 latches the current FEC uncorrectable codewords count value and
 * transfers it to the FEC uncorrectable codewords counter value bits of
 * the RX OTN FEC Uncorrectable Codewords counter. The FEC uncorrectable
 * codewords counter value is available to be read until the next FEC
 * uncorrectable codewords count latch and clear event occurs. The RX OTN
 * FEC uncorrectable codewords counter should not be read after asserting
 * the FEC uncorrectable codewords count error latch and clear bit until
 * this bit is auto cleared. The FEC uncorrectable codewords count latch
 * and clear bit is cleared as a default and is self clearing after a
 * write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL . FEC_UNCORRECTABLE_CODEWORDS_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_FEC_UNCORRECTABLE_CODEWORDS_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_FEC_UNCORRECTABLE_CODEWORDS_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_FEC_UNCORRECTABLE_CODEWORDS_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * SM BIP latch and clear
 * If the SM BIP latch and clear bit is set, the XCO2 latches the current
 * SM BIP error count value and transfers it to the SM BIP counter value
 * bits of the RX OTN OTU SM BIP counter. The SM BIP counter value is
 * available to be read until the next SM BIP latch and clear event occurs.
 * The RX OTN OTU SM BIP counter should not be read after asserting the SM
 * BIP latch and clear bit until this bit is auto cleared. The SM BIP latch
 * and clear bit is cleared as a default and is self clearing after a
 * write. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL . SM_BIP_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_SM_BIP_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_SM_BIP_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_SM_BIP_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * SM BEI latch and clear
 * If the SM BEI latch and clear bit is set, the XCO2 latches the current
 * SM BEI error count value and transfers it to the SM BEI counter value
 * bits of the RX OTN OTU SM BEI counter. The SM BEI counter value is
 * available to be read until the next SM BEI latch and clear event occurs.
 * The RX OTN OTU SM BEI counter should not be read after asserting the SM
 * BEI latch and clear bit until this bit is auto cleared. The SM BEI latch
 * and clear bit is cleared as a default and is self clearing after a
 * write. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL . SM_BEI_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_SM_BEI_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_SM_BEI_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_SM_BEI_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * PJ latch and clear
 * If the PJ latch and clear bit is set, the XCO2 latches the current PJ
 * count value and transfers it to the PJ counter value bits of the RX OTN
 * OPU PJ counter. The PJ counter value is available to be read until the
 * next PJ latch and clear event occurs. The RX OTN OPU PJ counter should
 * not be read after asserting the PJ latch and clear bit until this bit is
 * auto cleared. The PJ latch and clear bit is cleared as a default and is
 * self clearing after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL . PJ_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_PJ_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_PJ_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_PJ_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * NJ latch and clear
 * If the NJ latch and clear bit is set, the XCO2 latches the current NJ
 * count value and transfers it to the NJ counter value bits of the RX OTN
 * OPU NJ counter. The NJ counter value is available to be read until the
 * next NJ latch and clear event occurs. The RX OTN OPU NJ counter should
 * not be read after asserting the NJ latch and clear bit until this bit is
 * auto cleared. The NJ latch and clear bit is cleared as a default and is
 * self clearing after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL . NJ_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_NJ_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_NJ_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_NJ_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * PRBS error latch and clear
 * If the PRBS error latch and clear bit is set, the XCO2 latches the
 * current PRBS error count value and transfers it to the PRBS error
 * counter value bits of the RX OTN OPU PRBS Error counter. The PRBS error
 * counter value is available to be read until the next PRBS error latch
 * and clear event occurs. The RX OTN OPU PRBS Error counter should not be
 * read after asserting the PRBS error latch and clear bit until this bit
 * is auto cleared. The PRBS error latch and clear bit is cleared as a
 * default and is self clearing after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL . PRBS_ERROR_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_PRBS_ERROR_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_PRBS_ERROR_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_PRBS_ERROR_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Block mode
 * If the block mode bit is set, SM BEI and BIP error counts accumulate on
 * a block basis. This implies that the SM counts (BIP and BEI) only
 * increment by 1 (block count) each frame when 1 or more SM errors are
 * detected. If the block mode bit is cleared, SM BEI and BIP error counts
 * accumulate the actual number of errors (bit count) detected each frame.
 * This bit is set as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL . BLOCK_MODE1
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_BLOCK_MODE1(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_BLOCK_MODE1     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FEC_OTU_OPU_CONTROL_BLOCK_MODE1(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief XCO2 Rx OTN ODU counter control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL(target)  VTSS_IOREG(target,0xc02)

/** 
 * \brief
 * TCM1 BIP latch and clear
 * If the TCM1 BIP latch and clear bit is set, the XCO2 latches the current
 * TCM1 BIP error count value and transfers it to the TCM1 BIP counter
 * value bits of the RX OTN ODU TCM1 BIP counter. The TCM 1 BIP counter
 * value is available to be read until the next TCM1 BIP latch and clear
 * event occurs. The RX OTN ODU TCM1 BIP counter should not be read after
 * asserting the TCM1 BIP latch and clear bit until this bit is auto
 * cleared. The TCM1 BIP latch and clear bit is cleared as a default and is
 * self clearing after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL . TCM1_BIP_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM1_BIP_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM1_BIP_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM1_BIP_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * TCM1 BEI latch and clear
 * If the TCM1 BEI latch and clear bit is set, the XCO2 latches the current
 * TCM1 BEI error count value and transfers it to the TCM1 BEI counter
 * value bits of the RX OTN ODU TCM1 BEI counter. The TCM1 BEI counter
 * value is available to be read until the next TCM1 BEI latch and clear
 * event occurs. The RX OTN ODU TCM1 BEI counter should not be read after
 * asserting the TCM1 BEI latch and clear bit until this bit is auto
 * cleared. The TCM1 BEI latch and clear bit is cleared as a default and is
 * self clearing after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL . TCM1_BEI_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM1_BEI_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM1_BEI_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM1_BEI_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * TCM2 BIP latch and clear
 * If the TCM2 BIP latch and clear bit is set, the XCO2 latches the current
 * TCM2 BIP error count value and transfers it to the TCM2 BIP counter
 * value bits of the RX OTN ODU TCM2 BIP counter. The TCM 2 BIP counter
 * value is available to be read until the next TCM2 BIP latch and clear
 * event occurs. The RX OTN ODU TCM2 BIP counter should not be read after
 * asserting the TCM2 BIP latch and clear bit until this bit is auto
 * cleared. The TCM2 BIP latch and clear bit is cleared as a default and is
 * self clearing after a write. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL . TCM2_BIP_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM2_BIP_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM2_BIP_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM2_BIP_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * TCM2 BEI latch and clear
 * If the TCM2 BEI latch and clear bit is set, the XCO2 latches the current
 * TCM2 BEI error count value and transfers it to the TCM2 BEI counter
 * value bits of the RX OTN ODU TCM2 BEI counter. The TCM2 BEI counter
 * value is available to be read until the next TCM2 BEI latch and clear
 * event occurs. The RX OTN ODU TCM2 BEI counter should not be read after
 * asserting the TCM2 BEI latch and clear bit until this bit is auto
 * cleared. The TCM2 BEI latch and clear bit is cleared as a default and is
 * self clearing after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL . TCM2_BEI_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM2_BEI_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM2_BEI_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM2_BEI_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * TCM3 BIP latch and clear
 * If the TCM3 BIP latch and clear bit is set, the XCO2 latches the current
 * TCM3 BIP error count value and transfers it to the TCM3 BIP counter
 * value bits of the RX OTN ODU TCM3 BIP counter. The TCM 3 BIP counter
 * value is available to be read until the next TCM3 BIP latch and clear
 * event occurs. The RX OTN ODU TCM3 BIP counter should not be read after
 * asserting the TCM3 BIP latch and clear bit until this bit is auto
 * cleared. The TCM3 BIP latch and clear bit is cleared as a default and is
 * self clearing after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL . TCM3_BIP_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM3_BIP_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM3_BIP_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM3_BIP_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * TCM3 BEI latch and clear
 * If the TCM3 BEI latch and clear bit is set, the XCO2 latches the current
 * TCM3 BEI error count value and transfers it to the TCM3 BEI counter
 * value bits of the RX OTN ODU TCM3 BEI counter. The TCM3 BEI counter
 * value is available to be read until the next TCM3 BEI latch and clear
 * event occurs. The RX OTN ODU TCM3 BEI counter should not be read after
 * asserting the TCM3 BEI latch and clear bit until this bit is auto
 * cleared. The TCM3 BEI latch and clear bit is cleared as a default and is
 * self clearing after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL . TCM3_BEI_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM3_BEI_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM3_BEI_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM3_BEI_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * TCM4 BIP latch and clear
 * If the TCM4 BIP latch and clear bit is set, the XCO2 latches the current
 * TCM4 BIP error count value and transfers it to the TCM4 BIP counter
 * value bits of the RX OTN ODU TCM4 BIP counter. The TCM 4 BIP counter
 * value is available to be read until the next TCM4 BIP latch and clear
 * event occurs. The RX OTN ODU TCM4 BIP counter should not be read after
 * asserting the TCM4 BIP latch and clear bit until this bit is auto
 * cleared. The TCM4 BIP latch and clear bit is cleared as a default and is
 * self clearing after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL . TCM4_BIP_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM4_BIP_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM4_BIP_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM4_BIP_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * TCM4 BEI latch and clear
 * If the TCM4 BEI latch and clear bit is set, the XCO2 latches the current
 * TCM4 BEI error count value and transfers it to the TCM4 BEI counter
 * value bits of the RX OTN ODU TCM4 BEI counter. The TCM4 BEI counter
 * value is available to be read until the next TCM4 BEI latch and clear
 * event occurs. The RX OTN ODU TCM4 BEI counter should not be read after
 * asserting the TCM4 BEI latch and clear bit until this bit is auto
 * cleared. The TCM4 BEI latch and clear bit is cleared as a default and is
 * self clearing after a write. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL . TCM4_BEI_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM4_BEI_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM4_BEI_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM4_BEI_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * TCM5 BIP latch and clear
 * If the TCM5 BIP latch and clear bit is set, the XCO2 latches the current
 * TCM5 BIP error count value and transfers it to the TCM5 BIP counter
 * value bits of the RX OTN ODU TCM5 BIP counter. The TCM 5 BIP counter
 * value is available to be read until the next TCM5 BIP latch and clear
 * event occurs. The RX OTN ODU TCM5 BIP counter should not be read after
 * asserting the TCM5 BIP latch and clear bit until this bit is auto
 * cleared. The TCM5 BIP latch and clear bit is cleared as a default and is
 * self clearing after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL . TCM5_BIP_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM5_BIP_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM5_BIP_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM5_BIP_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * TCM5 BEI latch and clear
 * If the TCM5 BEI latch and clear bit is set, the XCO2 latches the current
 * TCM5 BEI error count value and transfers it to the TCM5 BEI counter
 * value bits of the RX OTN ODU TCM5 BEI counter. The TCM5 BEI counter
 * value is available to be read until the next TCM5 BEI latch and clear
 * event occurs. The RX OTN ODU TCM5 BEI counter should not be read after
 * asserting the TCM5 BEI latch and clear bit until this bit is auto
 * cleared. The TCM5 BEI latch and clear bit is cleared as a default and is
 * self clearing after a write. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL . TCM5_BEI_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM5_BEI_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM5_BEI_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM5_BEI_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * TCM6 BIP latch and clear
 * If the TCM6 BIP latch and clear bit is set, the XCO2 latches the current
 * TCM6 BIP error count value and transfers it to the TCM6 BIP counter
 * value bits of the RX OTN ODU TCM6 BIP counter. The TCM 6 BIP counter
 * value is available to be read until the next TCM6 BIP latch and clear
 * event occurs. The RX OTN ODU TCM6 BIP counter should not be read after
 * asserting the TCM6 BIP latch and clear bit until this bit is auto
 * cleared. The TCM6 BIP latch and clear bit is cleared as a default and is
 * self clearing after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL . TCM6_BIP_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM6_BIP_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM6_BIP_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM6_BIP_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * TCM6 BEI latch and clear
 * If the TCM6 BEI latch and clear bit is set, the XCO2 latches the current
 * TCM6 BEI error count value and transfers it to the TCM6 BEI counter
 * value bits of the RX OTN ODU TCM6 BEI counter. The TCM6 BEI counter
 * value is available to be read until the next TCM6 BEI latch and clear
 * event occurs. The RX OTN ODU TCM6 BEI counter should not be read after
 * asserting the TCM6 BEI latch and clear bit until this bit is auto
 * cleared. The TCM6 BEI latch and clear bit is cleared as a default and is
 * self clearing after a write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL . TCM6_BEI_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM6_BEI_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM6_BEI_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_TCM6_BEI_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * PM BIP latch and clear
 * If the PM BIP latch and clear bit is set, the XCO2 latches the current
 * PM BIP error count value and transfers it to the PM BIP counter value
 * bits of the RX OTN ODU PM BIP counter. The TCM1 BIP counter value is
 * available to be read until the next PM BIP latch and clear event occurs.
 * The RX OTN ODU PM BIP counter should not be read after asserting the PM
 * BIP latch and clear bit until this bit is auto cleared. The PM BIP latch
 * and clear bit is cleared as a default and is self clearing after a
 * write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL . PM_BIP_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_PM_BIP_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_PM_BIP_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_PM_BIP_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * PM BEI latch and clear
 * If the PM BEI latch and clear bit is set, the XCO2 latches the current
 * PM BEI error count value and transfers it to the PM BEI counter value
 * bits of the RX OTN ODU PM BEI counter. The TCM1 BEI counter value is
 * available to be read until the next PM BEI latch and clear event occurs.
 * The RX OTN ODU PM BEI counter should not be read after asserting the PM
 * BEI latch and clear bit until this bit is auto cleared. The PM BEI latch
 * and clear bit is cleared as a default and is self clearing after a
 * write.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL . PM_BEI_LATCH_AND_CLEAR
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_PM_BEI_LATCH_AND_CLEAR(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_PM_BEI_LATCH_AND_CLEAR     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_PM_BEI_LATCH_AND_CLEAR(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Block mode
 * If the block mode bit is set, TCMi/PM BEI and BIP error counts
 * accumulate on a block basis. This implies that the TCMi/PM counts (BIP
 * and BEI) only increment by 1 (block count) each frame when 1 or more
 * TCMi/PM errors are detected. If the block mode bit is cleared, TCMi/PM
 * BEI and BIP error counts accumulate the actual number of errors (bit
 * count) detected each frame. This bit is set as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL . BLOCK_MODE2
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_BLOCK_MODE2(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_BLOCK_MODE2     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_CONTROL_BLOCK_MODE2(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief XCO2 Rx OTN FA FAU control 1 register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_FA_FAU_CONTROL_1
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FAU_CONTROL_1(target)  VTSS_IOREG(target,0xc03)

/** 
 * \brief
 * Force OOF
 * If the force OOF bit is set, the frame align sequencer block OOF state
 * machine is immediately placed in the OOF state resulting in an OOF
 * error. If this bit is set long enough, the result is an LOF error
 * condition. This bit is not self clearing. Normal state machine operation
 * occurs when this bit is cleared after being set. This bit is cleared as
 * a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FAU_CONTROL_1 . FORCE_OOF
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FAU_CONTROL_1_FORCE_OOF(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FAU_CONTROL_1_FORCE_OOF     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FAU_CONTROL_1_FORCE_OOF(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Force LOF
 * If the force LOF bit is set, the frame align sequencer block LOF state
 * machine is immediately placed in the 
 * LOF state resulting in an LOF error. This bit is not self clearing.
 * Normal state machine operation occurs when this bit is cleared after
 * being set. This bit is cleared as a default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FAU_CONTROL_1 . FORCE_LOF
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FAU_CONTROL_1_FORCE_LOF(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FAU_CONTROL_1_FORCE_LOF     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FAU_CONTROL_1_FORCE_LOF(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * n_LOF LOF count
 * The n_LOF--LOF count bits reside are used to determine the number of
 * consecutive OOF frame times (frame time has a fixed period) required to
 * transition from the n_LOF state to the LOF state. The minimum count
 * value allowed is 2. The G.709 standard states a 3 ms time of OOF
 * required to transition in and out of LOF. The default value used for
 * this count is programmed for 3 ms.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FAU_CONTROL_1 . N_LOF_LOF_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FAU_CONTROL_1_N_LOF_LOF_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FAU_CONTROL_1_N_LOF_LOF_COUNT     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FAU_CONTROL_1_N_LOF_LOF_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \brief XCO2 Rx OTN FA FAU control 2 register (address: 0x204) 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_FA_FAU_CONTROL_2
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FAU_CONTROL_2(target)  VTSS_IOREG(target,0xc04)

/** 
 * \brief
 * IF OOF count
 * The IF -- OOF count bits are used to determine the number of consecutive
 * OOF frames required to transition from the IF state to the OOF state.
 * The minimum count value allowed is 2. The default value used for this
 * count is 5 as per the digital wrapper standard.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FAU_CONTROL_2 . IF_OOF_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FAU_CONTROL_2_IF_OOF_COUNT(x)  VTSS_ENCODE_BITFIELD(x,12,4)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FAU_CONTROL_2_IF_OOF_COUNT     VTSS_ENCODE_BITMASK(12,4)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FAU_CONTROL_2_IF_OOF_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,12,4)

/** 
 * \brief
 * LOF n_LOF count
 * The LOF -- n_LOF count bits are used to determine the number of
 * consecutive IF states required to transition from the LOF state to the
 * n_LOF state. The minimum count value allowed is 2. The G.709 standard
 * states a 3 ms time of OOF/IF required to transition in and out of LOF.
 * The default value used for this count is programmed for 3 ms.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FAU_CONTROL_2 . LOF_N_LOF_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FAU_CONTROL_2_LOF_N_LOF_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FAU_CONTROL_2_LOF_N_LOF_COUNT     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_FAU_CONTROL_2_LOF_N_LOF_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \brief XCO2 Rx OTN FA MFAU control 1 register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_FA_MFAU_CONTROL_1
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_MFAU_CONTROL_1(target)  VTSS_IOREG(target,0xc05)

/** 
 * \brief
 * Force OOM
 * If the force OOM bit is set, the multiframe align sequencer block OOM
 * state machine is immediately placed in the OOM state resulting in an OOM
 * error. If this bit is set long enough, the result is an LOM error
 * condition. This bit is not self clearing. Normal state machine operation
 * occurs when this bit is cleared after being set. This bit is cleared as
 * a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_MFAU_CONTROL_1 . FORCE_OOM
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_MFAU_CONTROL_1_FORCE_OOM(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_MFAU_CONTROL_1_FORCE_OOM     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_MFAU_CONTROL_1_FORCE_OOM(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Force LOM
 * If the force LOM bit is set, the multiframe align sequencer block LOM
 * state machine is immediately placed in the LOM state resulting in an LOM
 * error. This bit is not self clearing. Normal state machine operation
 * occurs when this bit is cleared after being set. This bit is cleared as
 * a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_MFAU_CONTROL_1 . FORCE_LOM
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_MFAU_CONTROL_1_FORCE_LOM(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_MFAU_CONTROL_1_FORCE_LOM     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_MFAU_CONTROL_1_FORCE_LOM(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * n_LOM LOM count
 * The n_LOM - LOM count bits are used by the MFAS interpreter. A
 * transition from the n_LOM state to the LOM state (MFAS state machine)
 * occurs if an OOM condition is present for n_LOM - LOM count number of
 * frames. The minimum count value allowed is 2. The G.709 standard states
 * a 3 ms time of OOM required to transition in and out of LOM. The default
 * value used for this count is programmed for 3 ms.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_MFAU_CONTROL_1 . N_LOMLOM_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_MFAU_CONTROL_1_N_LOMLOM_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_MFAU_CONTROL_1_N_LOMLOM_COUNT     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_MFAU_CONTROL_1_N_LOMLOM_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \brief XCO2 Rx OTN FA MFAU control 2 register (address: 0x206) 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_FA_MFAU_CONTROL_2
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_MFAU_CONTROL_2(target)  VTSS_IOREG(target,0xc06)

/** 
 * \brief
 * IM OOM count
 * The IM -- OOM count bits are used to determine the number of consecutive
 * OOM frames required to transition from the IM state to the OOM state.
 * The minimum count value allowed is 2. The default value used for this
 * count is 5 as per the digital wrapper standard.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_MFAU_CONTROL_2 . IM_OOM_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_MFAU_CONTROL_2_IM_OOM_COUNT(x)  VTSS_ENCODE_BITFIELD(x,12,4)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_MFAU_CONTROL_2_IM_OOM_COUNT     VTSS_ENCODE_BITMASK(12,4)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_MFAU_CONTROL_2_IM_OOM_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,12,4)

/** 
 * \brief
 * LOM n_LOM count
 * The LOM -- n_LOM count bits are used to determine the number of
 * consecutive IM states required to transition from the LOM state to the
 * n_LOM state. The default value used for this count is 0000.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_MFAU_CONTROL_2 . LOMN_LOM_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_MFAU_CONTROL_2_LOMN_LOM_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_MFAU_CONTROL_2_LOMN_LOM_COUNT     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_MFAU_CONTROL_2_LOMN_LOM_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief XCO2 Rx OTN FA OOF counter 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_FA_OOF
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_OOF(target)  VTSS_IOREG(target,0xc09)

/** 
 * \brief
 * OOF counter value
 * The OOF counter value bits are used to accumulate the number of OOF
 * errors detected in the Frame Alignment Unit (FAU). This is a saturating
 * read only counter.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_OOF . OOF_COUNTER_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_OOF_OOF_COUNTER_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_OOF_OOF_COUNTER_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_OOF_OOF_COUNTER_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN FA OOM counter 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_FA_OOM
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_OOM(target)  VTSS_IOREG(target,0xc0a)

/** 
 * \brief
 * OOM counter value
 * The OOM counter value bits are used to accumulate the number of OOM
 * errors detected in the Multiframe Alignment Unit (MFAU). This is a
 * saturating read only counter. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_OOM . OOM_COUNTER_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_OOM_OOM_COUNTER_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_OOM_OOM_COUNTER_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FA_OOM_OOM_COUNTER_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN FEC upper correctable symbols counter 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_FEC_UPPER_CORRECTABLE_SYMBOLS
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_CORRECTABLE_SYMBOLS(target)  VTSS_IOREG(target,0xc0b)

/** 
 * \brief
 * Upper FEC correctable symbols counter value
 * The upper FEC correctable symbols counter value bits are used to
 * accumulate the number of FEC correctable symbols detected in FEC
 * decoder. The upper FEC correctable symbols counter value bits contain
 * the upper 8 bits of the 24 bit Rx OTN FEC correctable symbols counter.
 * This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_CORRECTABLE_SYMBOLS . UPPER_FEC_CORRECTABLE_SYMBOLS_COUNTER_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_CORRECTABLE_SYMBOLS_UPPER_FEC_CORRECTABLE_SYMBOLS_COUNTER_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_CORRECTABLE_SYMBOLS_UPPER_FEC_CORRECTABLE_SYMBOLS_COUNTER_VALUE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_CORRECTABLE_SYMBOLS_UPPER_FEC_CORRECTABLE_SYMBOLS_COUNTER_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief XCO2 Rx OTN FEC lower correctable symbols counter 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_FEC_LOWER_CORRECTABLE_SYMBOLS
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_CORRECTABLE_SYMBOLS(target)  VTSS_IOREG(target,0xc0c)

/** 
 * \brief
 * Lower FEC correctable symbols counter value
 * The lower FEC correctable symbols counter value bits are used to
 * accumulate the number of FEC correctable symbols detected in FEC
 * decoder. The lower FEC correctable symbols counter value bits contain
 * the lower 16 bits of the 24 bit Rx OTN FEC correctable symbols counter.
 * This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_CORRECTABLE_SYMBOLS . LOWER_FEC_CORRECTABLE_SYMBOLS_COUNTER_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_CORRECTABLE_SYMBOLS_LOWER_FEC_CORRECTABLE_SYMBOLS_COUNTER_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_CORRECTABLE_SYMBOLS_LOWER_FEC_CORRECTABLE_SYMBOLS_COUNTER_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_CORRECTABLE_SYMBOLS_LOWER_FEC_CORRECTABLE_SYMBOLS_COUNTER_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN FEC upper corrected zeros counter 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_FEC_UPPER_CORRECTED_ZEROS
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_CORRECTED_ZEROS(target)  VTSS_IOREG(target,0xc0d)

/** 
 * \brief
 * Upper FEC correctable zeros counter value
 * The upper FEC corrected zeros counter value bits are used to accumulate
 * the number of FEC corrected zeros detected in FEC decoder. The upper FEC
 * corrected zeros counter value bits contain the upper 8 bits of the 24
 * bit Rx OTN FEC corrected zeros counter. 
 * NOTE: The upper corrected zeros counter accumulates corrected 0s after
 * scrambling. For example, a bit transmitted after scrambling is a 0 and
 * gets corrupted to a 1 before being received. If the receiver corrects
 * the bit through the FEC decoder, the Rx OTN FEC corrected zeros counter
 * is incremented. This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_CORRECTED_ZEROS . UPPER_FEC_CORRECTABLE_ZEROS_COUNTER_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_CORRECTED_ZEROS_UPPER_FEC_CORRECTABLE_ZEROS_COUNTER_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_CORRECTED_ZEROS_UPPER_FEC_CORRECTABLE_ZEROS_COUNTER_VALUE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_CORRECTED_ZEROS_UPPER_FEC_CORRECTABLE_ZEROS_COUNTER_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief XCO2 Rx OTN FEC lower corrected zeros counter 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_FEC_LOWER_CORRECTED_ZEROS
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_CORRECTED_ZEROS(target)  VTSS_IOREG(target,0xc0e)

/** 
 * \brief
 * Lower FEC corrected zeros counter value
 * The lower FEC corrected zeros counter value bits are used to accumulate
 * the number of FEC corrected zeros detected in FEC decoder. The lower FEC
 * corrected zeros counter value bits contain the lower 16 bits of the 24
 * bit Rx OTN FEC corrected zeros counter. 
 * NOTE: The lower corrected zeros counter accumulates corrected 0s after
 * scrambling. For example, a bit transmitted after scrambling is a 0 and
 * gets corrupted to a 1 prior to being received. If the receiver corrects
 * the bit through the FEC decoder, the Rx OTN FEC corrected zeros counter
 * is incremented. This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_CORRECTED_ZEROS . LOWER_FEC_CORRECTED_ZEROS_COUNTER_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_CORRECTED_ZEROS_LOWER_FEC_CORRECTED_ZEROS_COUNTER_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_CORRECTED_ZEROS_LOWER_FEC_CORRECTED_ZEROS_COUNTER_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_CORRECTED_ZEROS_LOWER_FEC_CORRECTED_ZEROS_COUNTER_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN FEC upper corrected ones counter 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_FEC_UPPER_CORRECTED_ONES
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_CORRECTED_ONES(target)  VTSS_IOREG(target,0xc0f)

/** 
 * \brief
 * Upper FEC corrected ones counter value
 * The upper FEC corrected ones counter value bits are used to accumulate
 * the number of FEC corrected ones detected in FEC decoder. The upper FEC
 * corrected ones counter value bits contain the upper 8 bits of the 24 bit
 * Rx OTN FEC Corrected ones counter. 
 * NOTE: The upper corrected ones counter accumulates corrected 1s after
 * scrambling. For example, a bit transmitted after scrambling is a 1 and
 * gets corrupted to a 0 prior to being received. If the receiver corrects
 * the bit through the FEC decoder, the Rx OTN FEC corrected ones counter
 * is incremented. This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_CORRECTED_ONES . UPPER_FEC_CORRECTED_ONES_COUNTER_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_CORRECTED_ONES_UPPER_FEC_CORRECTED_ONES_COUNTER_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_CORRECTED_ONES_UPPER_FEC_CORRECTED_ONES_COUNTER_VALUE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_CORRECTED_ONES_UPPER_FEC_CORRECTED_ONES_COUNTER_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief XCO2 Rx OTN FEC lower corrected ones counter 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_FEC_LOWER_CORRECTED_ONES
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_CORRECTED_ONES(target)  VTSS_IOREG(target,0xc10)

/** 
 * \brief
 * Lower FEC corrected ones counter value
 * The lower FEC corrected ones counter value bits are used to accumulate
 * the number of FEC corrected ones detected in FEC decoder. The lower FEC
 * corrected ones counter value bits contain the lower 16 bits of the 24
 * bit Rx OTN FEC corrected ones counter. 
 * NOTE: The lower corrected ones counter accumulates corrected 1s after
 * scrambling. For example, a bit transmitted after scrambling is a 1 and
 * gets corrupted to a 0 prior to being received. If the receiver corrects
 * the bit through the FEC decoder, the Rx OTN FEC corrected ones counter
 * is incremented. This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_CORRECTED_ONES . LOWER_FEC_CORRECTED_ONES_COUNTER_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_CORRECTED_ONES_LOWER_FEC_CORRECTED_ONES_COUNTER_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_CORRECTED_ONES_LOWER_FEC_CORRECTED_ONES_COUNTER_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_CORRECTED_ONES_LOWER_FEC_CORRECTED_ONES_COUNTER_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN FEC upper uncorrectable codewords counter 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_FEC_UPPER_UNCORRECTABLE_CODEWORDS
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_UNCORRECTABLE_CODEWORDS(target)  VTSS_IOREG(target,0xc11)

/** 
 * \brief
 * Upper FEC uncorrectable codewords counter value
 * The upper FEC uncorrectable codewords counter value bits are used to
 * accumulate the number of FEC uncorrectable codewords detected in FEC
 * decoder. The upper FEC uncorrectable codewords counter value bits
 * contain the upper 8 bits of the 24 bit Rx OTN FEC uncorrectable
 * codewords counter. This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_UNCORRECTABLE_CODEWORDS . UPPER_FEC_UNCORRECTABLE_CODEWORDS_COUNTER_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_UNCORRECTABLE_CODEWORDS_UPPER_FEC_UNCORRECTABLE_CODEWORDS_COUNTER_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_UNCORRECTABLE_CODEWORDS_UPPER_FEC_UNCORRECTABLE_CODEWORDS_COUNTER_VALUE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_UNCORRECTABLE_CODEWORDS_UPPER_FEC_UNCORRECTABLE_CODEWORDS_COUNTER_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief XCO2 Rx OTN FEC lower uncorrectable codewords counter 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_FEC_LOWER_UNCORRECTABLE_CODEWORDS
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_UNCORRECTABLE_CODEWORDS(target)  VTSS_IOREG(target,0xc12)

/** 
 * \brief
 * Lower FEC uncorrectable codewords counter value
 * The lower FEC uncorrectable codewords counter value bits are used to
 * accumulate the number of FEC uncorrectable codewords detected in FEC
 * decoder. The lower FEC uncorrectable codewords counter value bits
 * contain the lower 16 bits of the 24 bit Rx OTN FEC uncorrectable
 * codewords counter. This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_UNCORRECTABLE_CODEWORDS . LOWER_FEC_UNCORRECTABLE_CODEWORDS_COUNTER_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_UNCORRECTABLE_CODEWORDS_LOWER_FEC_UNCORRECTABLE_CODEWORDS_COUNTER_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_UNCORRECTABLE_CODEWORDS_LOWER_FEC_UNCORRECTABLE_CODEWORDS_COUNTER_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_UNCORRECTABLE_CODEWORDS_LOWER_FEC_UNCORRECTABLE_CODEWORDS_COUNTER_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN OTU SM TTI control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_OTU_SM_TTI_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL(target)  VTSS_IOREG(target,0xc13)

/** 
 * \brief
 * SM TTI SAPI TIM
 * The SM TTI SAPI TIM bit is set, the SM TTI SAPI bytes are included in
 * the comparison of the accepted SM TTI and expected SM TTI bytes for
 * trace identifier mismatch (SM TIM) detection. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL . SM_TTI_SAPI_TIM
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL_SM_TTI_SAPI_TIM(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL_SM_TTI_SAPI_TIM     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL_SM_TTI_SAPI_TIM(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * SM TTI DAPI TIM
 * The SM TTI DAPI TIM bit is set, the SM TTI DAPI bytes are included in
 * the comparison of the accepted SM TTI and expected SM TTI bytes for
 * trace identifier mismatch (SM TIM) detection. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL . SM_TTI_DAPI_TIM
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL_SM_TTI_DAPI_TIM(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL_SM_TTI_DAPI_TIM     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL_SM_TTI_DAPI_TIM(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * SM TTI OS TIM
 * The SM TTI OS TIM bit is set, the SM TTI operator specific bytes are
 * included in the comparison of the accepted SM TTI and expected SM TTI
 * bytes for trace identifier mismatch (SM TIM) detection. This bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL . SM_TTI_OS_TIM
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL_SM_TTI_OS_TIM(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL_SM_TTI_OS_TIM     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL_SM_TTI_OS_TIM(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * SM TTI accept count
 * The SM TTI accept count bits are used to determine the number of
 * identical SM TTI messages that must be received consecutively to update
 * the SM TTI extract value bits of the Rx OTN OTU SM TTI extract memory
 * with the current SM TTI message. 
 * NOTE: A count of 0 is invalid and should not be programmed. The SM TTI
 * accept count bits are set to a default value of 2.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL . SM_TTI_ACCEPT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL_SM_TTI_ACCEPT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,3,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL_SM_TTI_ACCEPT_COUNT     VTSS_ENCODE_BITMASK(3,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL_SM_TTI_ACCEPT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,3,3)

/** 
 * \brief
 * SM TTI inconsistent count
 * The SM TTI inconsistent count bits are used to determine the number of
 * non identical SM TTI messages that must be received to declare a TTI
 * inconsistent error. 
 * NOTE: A count of 0 is invalid and should not be programmed. The SM TTI
 * inconsistent count bits are set to a default value of 5.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL . SM_TTI_INCONSISTENT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL_SM_TTI_INCONSISTENT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL_SM_TTI_INCONSISTENT_COUNT     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL_SM_TTI_INCONSISTENT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief XCO2 Rx OTN OTU SM BIAE/BDI/IAE/RES control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL(target)  VTSS_IOREG(target,0xc14)

/** 
 * \brief
 * SM BIAE filter 5
 * The SM BIAE filter 5 bit is set to indicate that 5 consecutive frames
 * with SM BIAE field values of 1011 are required to detect an incoming SM
 * BIAE error condition. If this bit is cleared, 3 consecutive frames with
 * SM BIAE field values of 1011 result in SM BIAE error detection.
 * Likewise, the SM BIAE filter 5 bit is used to determine the number of
 * consecutive (3 or 5) frames with SM BIAE field values not equal to 1011
 * required to transition from a SM BIAE error state to an error-free SM
 * BIAE condition. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL . SM_BIAE_FILTER_5
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL_SM_BIAE_FILTER_5(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL_SM_BIAE_FILTER_5     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL_SM_BIAE_FILTER_5(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * SM BDI filter 5
 * The SM BDI filter 5 bit is set to indicate that 5 consecutive frames
 * with SM BDI field values of 1 are required to detect an incoming SM BDI
 * error condition. If this bit is cleared, 3 consecutive frames with SM
 * BDI field value of 1 result in SM BDI error detection. Likewise, the SM
 * BDI filter 5 bit is used to determine the number of consecutive (3 or 5)
 * frames with SM BDI field values of 0 required to transition from a SM
 * BDI error state to an error-free SM BDI condition. This bit is set as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL . SM_BDI_FILTER_5
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL_SM_BDI_FILTER_5(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL_SM_BDI_FILTER_5     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL_SM_BDI_FILTER_5(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * SM IAE filter 5
 * The SM IAE filter 5 bit is set to indicate that 5 consecutive frames
 * with SM IAE field values of 1 are required to detect an incoming SM IAE
 * error condition. If this bit is cleared, 3 consecutive frames with SM
 * IAE field value of 1 result in SM IAE error detection. Likewise, the SM
 * IAE filter 5 bit is used to determine the number of consecutive (3 or 5)
 * frames with SM IAE field values of 0 required to transition from a SM
 * IAE error state to an error-free SM IAE condition. This bit is set as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL . SM_IAE_FILTER_5
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL_SM_IAE_FILTER_5(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL_SM_IAE_FILTER_5     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL_SM_IAE_FILTER_5(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * SM RES accept value
 * The SM RES accept value bits are updated when identical extracted SM RES
 * bits are received for the number of consecutive sequences as programmed
 * through the SM RES accept count bits. The SM RES accept value bits are
 * cleared as a default value and are read only.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL . SM_RES_ACCEPT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL_SM_RES_ACCEPT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,6,2)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL_SM_RES_ACCEPT_VALUE     VTSS_ENCODE_BITMASK(6,2)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL_SM_RES_ACCEPT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,6,2)

/** 
 * \brief
 * SM RES accept count
 * The SM RES accept count bits are used to determine the number of
 * identical SM RES bits that must be received consecutively in order to
 * update the SM RES extract value bits. 
 * NOTE: A count of 0 is invalid and should not be programmed. The SM RES
 * accept count bits are set to a value of 0101 as a default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL . SM_RES_ACCEPT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL_SM_RES_ACCEPT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,3,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL_SM_RES_ACCEPT_COUNT     VTSS_ENCODE_BITMASK(3,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL_SM_RES_ACCEPT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,3,3)

/** 
 * \brief
 * SM RES inconsistent count
 * The SM RES inconsistent count bits are used to determine the number of
 * non identical SM TTI messages that must be received in order to declare
 * a TTI inconsistent error. 
 * NOTE: A count of 0 is invalid and should not be programmed. The SM RES
 * inconsistent count bits are set to a value of 0101 as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL . SM_RES_INCONSISTENT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL_SM_RES_INCONSISTENT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL_SM_RES_INCONSISTENT_COUNT     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL_SM_RES_INCONSISTENT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief XCO2 Rx OTN OTU GCC0/RES control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_OTU_GCC0_RES_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_GCC0_RES_CONTROL(target)  VTSS_IOREG(target,0xc15)

/** 
 * \brief
 * GCC0 accept count
 * The GCC0 accept count bits are used to determine the number of identical
 * GCC0 bytes that must be received consecutively to update the GCC0
 * extract value bits of the Rx OTN OTU GCC0 Extract register. If the
 * number of consecutive GCC0 bytes received matches the GCC0 accept count,
 * a GCC0 accepted interrupt is declared by setting the GCC0 accept IRQ bit
 * of the Rx OTN OTU Interrupt register. 
 * NOTE: A count of 0 is invalid and should not be programmed. The GCC0
 * accept count bits are set to 0101 as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_GCC0_RES_CONTROL . GCC0_ACCEPT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_GCC0_RES_CONTROL_GCC0_ACCEPT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,11,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_GCC0_RES_CONTROL_GCC0_ACCEPT_COUNT     VTSS_ENCODE_BITMASK(11,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_GCC0_RES_CONTROL_GCC0_ACCEPT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,11,3)

/** 
 * \brief
 * GCC0 inconsistent count
 * The GCC0 inconsistent count bits are used to determine the number of non
 * identical GCC0 bytes that must be received to declare a GCC0
 * inconsistent condition. A GCC0 inconsistent condition is indicated by
 * setting the GCC0 inconsistent IRQ bit of the Rx OTN OTU Interrupt
 * register. 
 * NOTE: A count of 0 is invalid and should not be programmed. The GCC0
 * inconsistent count bits are set to 0101 as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_GCC0_RES_CONTROL . GCC0_INCONSISTENT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_GCC0_RES_CONTROL_GCC0_INCONSISTENT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_GCC0_RES_CONTROL_GCC0_INCONSISTENT_COUNT     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_GCC0_RES_CONTROL_GCC0_INCONSISTENT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * RES accept count 
 * The RES accept count bits are used to determine the number of identical
 * RES bytes that must be received consecutively to update the RES extract
 * value bits of the Rx OTN OTU RES Extract register. If the number of
 * consecutive RES bytes received matches the RES accept count, an RES
 * accepted interrupt is declared by setting the RES accept IRQ bit of the
 * Rx OTN OTU Interrupt register. 
 * NOTE: A count of 0 is invalid and should not be programmed. The OTU
 * accept count bits are set to 0101 as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_GCC0_RES_CONTROL . RES_ACCEPT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_GCC0_RES_CONTROL_RES_ACCEPT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,3,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_GCC0_RES_CONTROL_RES_ACCEPT_COUNT     VTSS_ENCODE_BITMASK(3,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_GCC0_RES_CONTROL_RES_ACCEPT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,3,3)

/** 
 * \brief
 * RES inconsistent count
 * The RES inconsistent count bits are used to determine the number of non
 * identical RES bytes that must be received to declare an RES inconsistent
 * condition. An RES inconsistent condition is indicated by setting the RES
 * inconsistent IRQ bit of the Rx OTN OTU Interrupt register. 
 * NOTE: A count of 0 is invalid and should not be programmed. The RES
 * inconsistent count bits are set to 0101 as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_GCC0_RES_CONTROL . RES_INCONSISTENT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_GCC0_RES_CONTROL_RES_INCONSISTENT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_GCC0_RES_CONTROL_RES_INCONSISTENT_COUNT     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_GCC0_RES_CONTROL_RES_INCONSISTENT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief XCO2 Rx OTN OTU SM TTI extract memory 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_OTU_SM_TTI_EXTRACT
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: XCO2_RX_OTN_OTU_SM_TTI_EXTRACT (??), 0-31
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_EXTRACT(target,ri)  VTSS_IOREG(target,0xc16 + (ri))

/** 
 * \brief
 * SM TTI extract value
 * The SM TTI extract value bits contain the accepted 64 byte trail trace
 * identifier (SAPI/DAPI/Operator Specific) message extracted from incoming
 * frames. This information is used for comparison to the expected SM TTI
 * message information for SM trail trace identifier mismatch (SM TIM)
 * detection. These bits are undefined as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_EXTRACT . SM_TTI_EXTRACT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_EXTRACT_SM_TTI_EXTRACT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_EXTRACT_SM_TTI_EXTRACT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_EXTRACT_SM_TTI_EXTRACT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN OTU SM TTI expect memory 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_OTU_SM_TTI_EXPECT
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: XCO2_RX_OTN_OTU_SM_TTI_EXPECT (??), 0-31
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_EXPECT(target,ri)  VTSS_IOREG(target,0xc36 + (ri))

/** 
 * \brief
 * SM TTI expect value
 * The SM TTI expect value bits contain the 64 byte trail trace identifier
 * (SAPI/DAPI/Operator Specific) message to be used for comparison to the
 * extracted SM TTI message information for SM trail trace identifier
 * mismatch (SM TIM) detection. These bits are undefined as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_EXPECT . SM_TTI_EXPECT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_EXPECT_SM_TTI_EXPECT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_EXPECT_SM_TTI_EXPECT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_EXPECT_SM_TTI_EXPECT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN OTU GCC0 accept register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_OTU_GCC0_ACCEPT
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_GCC0_ACCEPT(target)  VTSS_IOREG(target,0xc56)

/** 
 * \brief
 * GCC0 accept value
 * The GCC0 accept value bits are updated when identical extracted GCC0
 * bytes are received for the number of consecutive sequences as programmed
 * through the GCC0 accept count bits of the Rx OTN OTU GCC0/RES Control
 * register. The GCC0 accept value bits have default value of 0x00 (hex).   
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_GCC0_ACCEPT . GCC0_ACCEPT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_GCC0_ACCEPT_GCC0_ACCEPT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_GCC0_ACCEPT_GCC0_ACCEPT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_GCC0_ACCEPT_GCC0_ACCEPT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN OTU RES accept register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_OTU_RES_ACCEPT
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_RES_ACCEPT(target)  VTSS_IOREG(target,0xc57)

/** 
 * \brief
 * RES accept value
 * The RES accept value bits are updated when identical extracted RES bytes
 * are received for the number of consecutive sequences as programmed
 * through the RES accept count bits of the Rx OTN OTU GCC0/RES Control
 * register. The RES accept value bits have default value of 0x00 (hex).
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_RES_ACCEPT . RES_ACCEPT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_RES_ACCEPT_RES_ACCEPT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_RES_ACCEPT_RES_ACCEPT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_RES_ACCEPT_RES_ACCEPT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN OTU SM BEI counter 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_OTU_SM_BEI
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BEI(target)  VTSS_IOREG(target,0xc5a)

/** 
 * \brief
 * SM BEI counter value
 * The SM BEI counter value bits are used to accumulate the number of BEI
 * errors detected in the SM BEI field of the OTU overhead. This is a
 * saturating read only counter. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BEI . SM_BEI_COUNTER_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BEI_SM_BEI_COUNTER_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BEI_SM_BEI_COUNTER_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BEI_SM_BEI_COUNTER_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN OTU SM BIP counter (address: 0x25b) 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_OTU_SM_BIP
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIP(target)  VTSS_IOREG(target,0xc5b)

/** 
 * \brief
 * SM BIP counter value
 * The SM BIP counter value bits are used to accumulate the number of SM
 * BIP errors detected for incoming frames. This is a saturating read only
 * counter.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIP . SM_BIP_COUNTER_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIP_SM_BIP_COUNTER_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIP_SM_BIP_COUNTER_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIP_SM_BIP_COUNTER_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN ODU maintenance control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL(target)  VTSS_IOREG(target,0xc5c)

/** 
 * \brief
 * Auto ODU AIS
 * If the auto ODU AIS bit is set, the receive processor frame controller
 * automatically generates ODU AIS (all ones in the entire ODU signal
 * excluding FA OH and OTU OH) frames if either the ODU AIS LOM or ODU AIS
 * PT MM bits are set and the corresponding condition is detected. ODU AIS
 * is removed when either the appropriate condition is removed or the
 * enabling bit which caused ODU AIS is cleared. The auto ODU AIS bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL . AUTO_ODU_AIS
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_AUTO_ODU_AIS(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_AUTO_ODU_AIS     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_AUTO_ODU_AIS(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Force ODU AIS
 * If the force ODU AIS bit is set, the receive processor frame controller
 * generates ODU AIS (all ones in the entire ODU signal excluding FA OH and
 * OTU OH) frames continuously. If the force ODU AIS bit is cleared, ODU
 * AIS frames are not forced. The force ODU AIS bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL . FORCE_ODU_AIS
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_FORCE_ODU_AIS(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_FORCE_ODU_AIS     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_FORCE_ODU_AIS(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * ODU AIS LOM
 * If the ODU AIS LOM bit is set, the receive processor frame controller
 * generates ODU AIS frames when an LOM condition is detected and the auto
 * ODU AIS bit is set. If the ODU AIS LOM or auto ODU AIS bit is cleared or
 * an LOM condition is not detected, ODU AIS is not inserted due to any LOM
 * condition detected. The ODU AIS LOM bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL . ODU_AIS_LOM
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_LOM(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_LOM     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_LOM(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * ODU AIS PT MM
 * If the ODU AIS PT MM bit is set, the receive processor frame controller
 * generates ODU AIS frames when a PT MM condition is detected and the auto
 * ODU AIS bit is set. If the ODU AIS PT MM or auto ODU AIS bit is cleared
 * or a PT MM condition is not detected, ODU AIS is not inserted due to any
 * PT MM condition detected. The ODU AIS PT MM bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL . ODU_AIS_PT_MM
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_PT_MM(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_PT_MM     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_PT_MM(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * ODU AIS SM TIM
 * If the ODU AIS SM TIM bit is set, the receive processor frame controller
 * generates ODU AIS frames when an SM TIM condition is detected and the
 * auto ODU AIS bit is set. If the ODU AIS SM TIM bit or auto ODU AIS bit
 * is cleared, ODU AIS is not inserted due to SM TIM detection. The ODU AIS
 * SM TIM bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL . ODU_AIS_SM_TIM
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_SM_TIM(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_SM_TIM     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_SM_TIM(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * ODU AIS PM TIMOCILCK
 * If the ODU AIS PM TIM/OCI/LCK bit is set, the receive processor frame
 * controller generates ODU AIS frames when a PM TIM/OCI/LCK condition is
 * detected and the auto ODU AIS bit is set. If the ODU AIS PM TIM/OCI/LCK
 * bit or auto ODU AIS bit is cleared, ODU AIS is not inserted due to PM
 * TIM/OCI/LCK detection. The ODU AIS PM TIM/OCI/LCK bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL . ODU_AIS_PM_TIMOCILCK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_PM_TIMOCILCK(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_PM_TIMOCILCK     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_PM_TIMOCILCK(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * ODU AIS TCM1 TIMOCILCK
 * If the ODU AIS TCM1 TIM/OCI/LCK bit is set, the receive processor frame
 * controller generates ODU AIS frames when a TCM1 TIM/OCI/LCK condition is
 * detected and the auto ODU AIS bit is set. If the ODU AIS TCM1
 * TIM/OCI/LCK bit or auto ODU AIS bit is cleared, ODU AIS is not inserted
 * due to TCM1 TIM/OCI/LCK detection. The ODU AIS TCM1 TIM/OCI/LCK bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL . ODU_AIS_TCM1_TIMOCILCK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_TCM1_TIMOCILCK(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_TCM1_TIMOCILCK     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_TCM1_TIMOCILCK(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * ODU AIS TCM2 TIMOCILCK
 * If the ODU AIS TCM2 TIM/OCI/LCK bit is set, the receive processor frame
 * controller generates ODU AIS frames when a TCM2 TIM/OCI/LCK condition is
 * detected and the auto ODU AIS bit is set. If the ODU AIS TCM2
 * TIM/OCI/LCK bit or auto ODU AIS bit is cleared, ODU AIS is not inserted
 * due to TCM2 TIM/OCI/LCK detection. The ODU AIS TCM2 TIM/OCI/LCK bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL . ODU_AIS_TCM2_TIMOCILCK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_TCM2_TIMOCILCK(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_TCM2_TIMOCILCK     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_TCM2_TIMOCILCK(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * ODU AIS TCM3 TIMOCILCK
 * If the ODU AIS TCM3 TIM/OCI/LCK bit is set, the receive processor frame
 * controller generates ODU AIS frames when a TCM3 TIM/OCI/LCK condition is
 * detected and the auto ODU AIS bit is set. If the ODU AIS TCM3
 * TIM/OCI/LCK bit or auto ODU AIS bit is cleared, ODU AIS is not inserted
 * due to TCM3 TIM/OCI/LCK detection. The ODU AIS TCM3 TIM/OCI/LCK bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL . ODU_AIS_TCM3_TIMOCILCK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_TCM3_TIMOCILCK(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_TCM3_TIMOCILCK     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_TCM3_TIMOCILCK(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * ODU AIS TCM4 TIMOCILCK
 * If the ODU AIS TCM4 TIM/OCI/LCK bit is set, the receive processor frame
 * controller generates ODU AIS frames when a TCM4 TIM/OCI/LCK condition is
 * detected and the auto ODU AIS bit is set. If the ODU AIS TCM4
 * TIM/OCI/LCK bit or auto ODU AIS bit is cleared, ODU AIS is not inserted
 * due to TCM4 TIM/OCI/LCK detection. The ODU AIS TCM4 TIM/OCI/LCK bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL . ODU_AIS_TCM4_TIMOCILCK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_TCM4_TIMOCILCK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_TCM4_TIMOCILCK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_TCM4_TIMOCILCK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * ODU AIS TCM5 TIMOCILCK
 * If the ODU AIS TCM5 TIM/OCI/LCK bit is set, the receive processor frame
 * controller generates ODU AIS frames when a TCM5 TIM/OCI/LCK condition is
 * detected and the auto ODU AIS bit is set. If the ODU AIS TCM5
 * TIM/OCI/LCK bit or auto ODU AIS bit is cleared, ODU AIS is not inserted
 * due to TCM5 TIM/OCI/LCK detection. The ODU AIS TCM5 TIM/OCI/LCK bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL . ODU_AIS_TCM5_TIMOCILCK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_TCM5_TIMOCILCK(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_TCM5_TIMOCILCK     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_TCM5_TIMOCILCK(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * ODU AIS TCM6 TIMOCILCK
 * If the ODU AIS TCM6 TIM/OCI/LCK bit is set, the receive processor frame
 * controller generates ODU AIS frames when a TCM6 TIM/OCI/LCK condition is
 * detected and the auto ODU AIS bit is set. If the ODU AIS TCM6
 * TIM/OCI/LCK bit or auto ODU AIS bit is cleared, ODU AIS is not inserted
 * due to TCM6 TIM/OCI/LCK detection. The ODU AIS TCM6 TIM/OCI/LCK bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL . ODU_AIS_TCM6_TIMOCILCK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_TCM6_TIMOCILCK(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_TCM6_TIMOCILCK     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_TCM6_TIMOCILCK(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Force generic AIS
 * If the force generic AIS bit is set, the receive processor frame
 * controller generates generic AIS continuously (repeating PN11 sequence
 * replaces all payload data). If the force generic AIS bit is cleared,
 * generic AIS is not forced. ODU AIS and LCK AIS insertion has priority
 * over generic AIS insertion. The force generic AIS bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL . FORCE_GENERIC_AIS
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_FORCE_GENERIC_AIS(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_FORCE_GENERIC_AIS     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_FORCE_GENERIC_AIS(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Force ODU LCK
 * If the force ODU LCK bit is set, the receive processor frame controller
 * generates ODU LCK frames continuously (all 01010 0101 pattern in the
 * entire ODU signal excluding FA OH and OTU OH). If the force ODU LCK bit
 * is cleared, ODU LCK frames are not forced. The force ODU LCK bit is
 * cleared as a default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL . FORCE_ODU_LCK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_FORCE_ODU_LCK(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_FORCE_ODU_LCK     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_FORCE_ODU_LCK(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief XCO2 Rx OTN ODU RES1/TCM ACT control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_RES1_TCM_ACT_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_TCM_ACT_CONTROL(target)  VTSS_IOREG(target,0xc5d)

/** 
 * \brief
 * RES1 accept count
 * The RES1 accept count bits are used to determine the number of identical
 * RES1 bytes that must be received consecutively to update the RES1
 * extract value bits of the Rx OTN ODU RES1 Extract registers. If the
 * number of consecutive RES1 bytes received matches the RES1 accept count,
 * a RES1 accepted interrupt is declared by setting the RES1 accept IRQ bit
 * of the Rx OTN ODU Interrupt register. 
 * NOTE: A count of 0 is invalid and should not be programmed. The RES1
 * accept count bits are set to 0101 as a default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_TCM_ACT_CONTROL . RES1_ACCEPT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_TCM_ACT_CONTROL_RES1_ACCEPT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,11,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_TCM_ACT_CONTROL_RES1_ACCEPT_COUNT     VTSS_ENCODE_BITMASK(11,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_TCM_ACT_CONTROL_RES1_ACCEPT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,11,3)

/** 
 * \brief
 * RES1 inconsistent count
 * The RES1 inconsistent count bits are used to determine the number of non
 * identical RES1 bytes that must be received to declare a RES1
 * inconsistent condition. A RES1 inconsistent condition is indicated by
 * setting the RES1 inconsistent IRQ bit of the Rx OTN ODU Interrupt
 * register. 
 * NOTE: A count of 0 is invalid and should not be programmed. The RES1
 * inconsistent count bits are set to 0101 as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_TCM_ACT_CONTROL . RES1_INCONSISTENT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_TCM_ACT_CONTROL_RES1_INCONSISTENT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_TCM_ACT_CONTROL_RES1_INCONSISTENT_COUNT     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_TCM_ACT_CONTROL_RES1_INCONSISTENT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * TCM ACT accept count
 * The TCM ACT accept count bits are used to determine the number of
 * identical TCM ACT bytes that must be received consecutively to update
 * the TCM ACT extract value bits of the Rx OTN ODU RES1 LOW/TCM 
 * ACT Extract Value register. If the number of consecutive TCM ACT bytes
 * received matches the TCM ACT accept count, a TCM ACT accepted interrupt
 * is declared by setting the TCM ACT accept IRQ bit of the Rx OTN ODU
 * Interrupt register. 
 * NOTE: A count of 0 is invalid and should not be programmed. The TCM ACT
 * accept count bits are set to 0101 as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_TCM_ACT_CONTROL . TCM_ACT_ACCEPT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_TCM_ACT_CONTROL_TCM_ACT_ACCEPT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,3,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_TCM_ACT_CONTROL_TCM_ACT_ACCEPT_COUNT     VTSS_ENCODE_BITMASK(3,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_TCM_ACT_CONTROL_TCM_ACT_ACCEPT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,3,3)

/** 
 * \brief
 * TCM ACT inconsistent count
 * The TCM ACT inconsistent count bits are used to determine the number of
 * non identical TCM ACT bytes that must be received to declare a TCM ACT
 * inconsistent condition. A TCM ACT inconsistent condition is indicated by
 * setting the TCM ACT inconsistent IRQ bit of the Rx OTN ODU Interrupt
 * register. 
 * NOTE: A count of 0 is invalid and should not be programmed. The TCM ACT
 * inconsistent count bits are set to 0101 as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_TCM_ACT_CONTROL . TCM_ACT_INCONSISTENT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_TCM_ACT_CONTROL_TCM_ACT_INCONSISTENT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_TCM_ACT_CONTROL_TCM_ACT_INCONSISTENT_COUNT     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_TCM_ACT_CONTROL_TCM_ACT_INCONSISTENT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief XCO2 Rx OTN ODU FTFL/EXP control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_FTFL_EXP_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_FTFL_EXP_CONTROL(target)  VTSS_IOREG(target,0xc5e)

/** 
 * \brief
 * FTFL accept count
 * The FTFL accept count bits are used to determine the number of identical
 * 256 byte FTFL messages that must be received consecutively to update the
 * FTFL extract value bits of the Rx OTN ODU FTFL Extract Memory. If the
 * number of consecutive FTFL messages received matches the FTFL accept
 * count, an FTFL accepted interrupt is declared by setting the FTFL accept
 * IRQ bit of the Rx OTN ODU Interrupt register. 
 * NOTE: A count of 0 is invalid and should not be programmed. The FTFL
 * accept count bits are set to 0101 as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_FTFL_EXP_CONTROL . FTFL_ACCEPT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_FTFL_EXP_CONTROL_FTFL_ACCEPT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,11,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_FTFL_EXP_CONTROL_FTFL_ACCEPT_COUNT     VTSS_ENCODE_BITMASK(11,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_FTFL_EXP_CONTROL_FTFL_ACCEPT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,11,3)

/** 
 * \brief
 * FTFL inconsistent count
 * The FTFL inconsistent count bits are used to determine the number of non
 * identical FTFL messages that must be received in order to declare an
 * FTFL inconsistent condition. An FTFL inconsistent condition is indicated
 * by setting the FTFL inconsistent IRQ bit of the Rx OTN ODU Interrupt
 * register. 
 * NOTE: A count of 0 is invalid and should not be programmed. The FTFL
 * inconsistent count bits are set to 0101 as a default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_FTFL_EXP_CONTROL . FTFL_INCONSISTENT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_FTFL_EXP_CONTROL_FTFL_INCONSISTENT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_FTFL_EXP_CONTROL_FTFL_INCONSISTENT_COUNT     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_FTFL_EXP_CONTROL_FTFL_INCONSISTENT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * EXP accept count
 * The EXP accept count bits are used to determine the number of identical
 * EXP bytes that must be received consecutively to update the EXP extract
 * value bits of the Rx OTN ODU EXP Extract registers. If the number of
 * consecutive EXP bytes received matches the EXP accept count, an EXP
 * accepted interrupt is declared by setting the EXP accept IRQ bit of the
 * Rx OTN ODU Interrupt register. 
 * NOTE: A count of 0 is invalid and should not be programmed. The EXP
 * accept count bits are set to 0101 as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_FTFL_EXP_CONTROL . EXP_ACCEPT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_FTFL_EXP_CONTROL_EXP_ACCEPT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,3,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_FTFL_EXP_CONTROL_EXP_ACCEPT_COUNT     VTSS_ENCODE_BITMASK(3,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_FTFL_EXP_CONTROL_EXP_ACCEPT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,3,3)

/** 
 * \brief
 * EXP inconsistent count
 * The EXP inconsistent count bits are used to determine the number of non
 * identical EXP bytes that must be received to declare an EXP inconsistent
 * condition. An EXP inconsistent condition is indicated by setting the EXP
 * inconsistent IRQ bit of the Rx OTN ODU Interrupt register. 
 * NOTE: A count of 0 is invalid and should not be programmed. The EXP
 * inconsistent count bits are set to 0101 as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_FTFL_EXP_CONTROL . EXP_INCONSISTENT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_FTFL_EXP_CONTROL_EXP_INCONSISTENT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_FTFL_EXP_CONTROL_EXP_INCONSISTENT_COUNT     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_FTFL_EXP_CONTROL_EXP_INCONSISTENT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief XCO2 Rx OTN ODU GCC1/GCC2 control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_GCC1_GCC2_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC1_GCC2_CONTROL(target)  VTSS_IOREG(target,0xc5f)

/** 
 * \brief
 * GCC1 accept count
 * The GCC1 accept count bits are used to determine the number of identical
 * GCC1 bytes that must be received consecutively to update the GCC1
 * extract value bits of the Rx OTN ODU GCC1 Extract registers. If the
 * number of identical consecutive GCC1 bytes received matches the GCC1
 * accept count, a GCC1 accepted interrupt is declared by setting the GCC1
 * accept IRQ bit of the Rx OTN ODU Interrupt register. An accept IRQ only
 * occurs if the new accepted value is different from the previously
 * accepted value.
 * NOTE: A count of 0 is invalid and should not be programmed. The GCC1
 * accept count bits are set to 0101 as a default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC1_GCC2_CONTROL . GCC1_ACCEPT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC1_GCC2_CONTROL_GCC1_ACCEPT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,11,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC1_GCC2_CONTROL_GCC1_ACCEPT_COUNT     VTSS_ENCODE_BITMASK(11,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC1_GCC2_CONTROL_GCC1_ACCEPT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,11,3)

/** 
 * \brief
 * GCC1 inconsistent count
 * The GCC1 inconsistent count bits are used to determine the number of non
 * identical GCC1 bytes that must be received to declare a GCC1
 * inconsistent condition. If the number of consecutive frames for which
 * the contents of the GCC1 byte has differed from the previous frame is
 * equal to or greater than the value programmed into the GCC1 inconsistent
 * count bits, an inconsistent condition exists. An inconsistent interrupt
 * is generated for each frame in which the inconsistent condition exists.
 * A GCC1 inconsistent condition is indicated by setting the GCC1
 * inconsistent IRQ bit of the Rx OTN ODU Interrupt register. 
 * NOTE: A count of 0 is invalid and should not be programmed. The GCC1
 * inconsistent count bits are set to 0101 as a default value.	  
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC1_GCC2_CONTROL . GCC1_INCONSISTENT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC1_GCC2_CONTROL_GCC1_INCONSISTENT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC1_GCC2_CONTROL_GCC1_INCONSISTENT_COUNT     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC1_GCC2_CONTROL_GCC1_INCONSISTENT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * GCC2 accept count
 * The GCC2 accept count bits are used to determine the number of identical
 * GCC2 bytes that must be received consecutively to update the GCC2
 * extract value bits of the Rx OTN ODU GCC2 Extract registers. If the
 * number of identical consecutive GCC2 bytes received matches the GCC2
 * accept count, a GCC2 accepted interrupt is declared by setting the GCC2
 * accept IRQ bit of the Rx OTN ODU Interrupt register. An accept IRQ only
 * occurs if the new accepted value is different from the previously
 * accepted value.
 * NOTE: A count of 0 is invalid and should not be programmed. The GCC2
 * accept count bits are set to 0101 as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC1_GCC2_CONTROL . GCC2_ACCEPT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC1_GCC2_CONTROL_GCC2_ACCEPT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,3,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC1_GCC2_CONTROL_GCC2_ACCEPT_COUNT     VTSS_ENCODE_BITMASK(3,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC1_GCC2_CONTROL_GCC2_ACCEPT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,3,3)

/** 
 * \brief
 * GCC2 inconsistent count
 * The GCC2 inconsistent count bits are used to determine the number of non
 * identical GCC2 bytes that must be received to declare a GCC2
 * inconsistent condition. If the number of consecutive frames for which
 * the contents of the GCC2 byte has differed from the previous frame is
 * equal to or greater than the value programmed into the GCC2 inconsistent
 * count bits, an inconsistent condition exists. An inconsistent interrupt
 * is generated for each frame in which the inconsistent condition exists.
 * A GCC2 inconsistent condition is indicated by setting the GCC2
 * inconsistent IRQ bit of the Rx OTN ODU Interrupt register. 
 * NOTE: A count of 0 is invalid and should not be programmed. The GCC2
 * inconsistent count bits are set to 0101 as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC1_GCC2_CONTROL . GCC2_INCONSISTENT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC1_GCC2_CONTROL_GCC2_INCONSISTENT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC1_GCC2_CONTROL_GCC2_INCONSISTENT_COUNT     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC1_GCC2_CONTROL_GCC2_INCONSISTENT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief XCO2 Rx OTN ODU APS/PCC/RES2 control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_APS_PCC_RES2_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_RES2_CONTROL(target)  VTSS_IOREG(target,0xc60)

/** 
 * \brief
 * APS/PCC accept count
 * The APS/PCC accept count bits are used to determine the number of
 * identical APS PCC bytes for any nested channel that must be received
 * consecutively to update the appropriate bits of the Rx OTN ODU APS/PCC
 * Extract registers. If the number of consecutive APS/PCC bytes received
 * for any channel (ODU path, ODU TCM1, ODU TCM2, ODU TCM3, ODU TCM4, ODU
 * TCM5, ODU TCM6, OTU section) matches the APS/PCC accept count, an
 * APS/PCC accepted interrupt is declared by setting the APS/PCC accept IRQ
 * bit of the Rx OTN ODU Interrupt register. 
 * NOTE: A count of 0 is invalid and should not be programmed. The APS/PCC
 * accept count bits are set to 0101 as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_RES2_CONTROL . APSPCC_ACCEPT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_RES2_CONTROL_APSPCC_ACCEPT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,11,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_RES2_CONTROL_APSPCC_ACCEPT_COUNT     VTSS_ENCODE_BITMASK(11,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_RES2_CONTROL_APSPCC_ACCEPT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,11,3)

/** 
 * \brief
 * APS/PCC inconsistent count
 * The APS/PCC inconsistent count bits are used to determine the number of
 * non identical APS/PCC bytes for any nested channel that must be received
 * to declare an APS/PCC inconsistent condition. An APS/PCC inconsistent
 * condition for any channel is indicated by setting the APS/PCC
 * inconsistent IRQ bit of the Rx OTN ODU Interrupt register. 
 * NOTE: A count of 0 is invalid and should not be programmed. The APS/PCC
 * inconsistent count bits are set to 0101 as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_RES2_CONTROL . APSPCC_INCONSISTENT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_RES2_CONTROL_APSPCC_INCONSISTENT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_RES2_CONTROL_APSPCC_INCONSISTENT_COUNT     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_RES2_CONTROL_APSPCC_INCONSISTENT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * RES2 accept count
 * The RES2 accept count bits are used to determine the number of identical
 * RES2 bytes that must be received consecutively to update the RES2
 * extract value bits of the Rx OTN ODU RES2 Extract registers. If the
 * number of consecutive RES2 bytes received matches the RES2 accept count,
 * an RES2 accepted interrupt is declared by setting the RES2 accept IRQ
 * bit of the Rx OTN ODU Interrupt register. 
 * NOTE: A count of 0 is invalid and should not be programmed. The RES2
 * accept count bits are set to 0101 as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_RES2_CONTROL . RES2_ACCEPT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_RES2_CONTROL_RES2_ACCEPT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,3,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_RES2_CONTROL_RES2_ACCEPT_COUNT     VTSS_ENCODE_BITMASK(3,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_RES2_CONTROL_RES2_ACCEPT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,3,3)

/** 
 * \brief
 * RES2 inconsistent count
 * The RES2 inconsistent count bits are used to determine the number of non
 * identical RES2 bytes that must be received to declare a RES2
 * inconsistent condition. A RES2 inconsistent condition is indicated by
 * setting the GCC2 inconsistent IRQ bit of the Rx OTN ODU Interrupt
 * register. 
 * NOTE: A count of 0 is invalid and should not 
 * be programmed. The GCC2 inconsistent count bits are set to 0101 as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_RES2_CONTROL . RES2_INCONSISTENT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_RES2_CONTROL_RES2_INCONSISTENT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_RES2_CONTROL_RES2_INCONSISTENT_COUNT     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_RES2_CONTROL_RES2_INCONSISTENT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief XCO2 Rx OTN ODU RES1 high accept register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_RES1_HIGH_ACCEPT
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_HIGH_ACCEPT(target)  VTSS_IOREG(target,0xc61)

/** 
 * \brief
 * RES1 high accept value
 * The RES1 high accept value bits are updated when identical extracted
 * RES1 bytes are received for the number of consecutive frames programmed
 * through the RES1 accept count bits of the Rx OTN ODU RES1/TCM ACT
 * Control register. The RES1 high accept value bits have a default value
 * of 0x00 (hex). 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_HIGH_ACCEPT . RES1_HIGH_ACCEPT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_HIGH_ACCEPT_RES1_HIGH_ACCEPT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_HIGH_ACCEPT_RES1_HIGH_ACCEPT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_HIGH_ACCEPT_RES1_HIGH_ACCEPT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN ODU RES1 LOW/TCM ACT accept value register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_RES1_LOW_TCM_ACT_ACCEPT_VALUE
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_LOW_TCM_ACT_ACCEPT_VALUE(target)  VTSS_IOREG(target,0xc62)

/** 
 * \brief
 * RES1 low accept value
 * The RES1 low accept value bits are updated when identical extracted RES1
 * bytes are received for the number of consecutive frames programmed
 * through the RES1 accept count bits of the Rx OTN ODU RES1/TCM ACT
 * Control register. The RES1 low accept value bits have a default value of
 * 0x00 (hex).
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_LOW_TCM_ACT_ACCEPT_VALUE . RES1_LOW_ACCEPT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_LOW_TCM_ACT_ACCEPT_VALUE_RES1_LOW_ACCEPT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_LOW_TCM_ACT_ACCEPT_VALUE_RES1_LOW_ACCEPT_VALUE     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_LOW_TCM_ACT_ACCEPT_VALUE_RES1_LOW_ACCEPT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * TCM ACT accept value
 * The TCM ACT accept value bits are updated when identical extracted TCM
 * ACT bytes are received for the number of consecutive frames programmed
 * through the TCM ACT accept count bits of the Rx OTN ODU RES1/TCM ACT
 * Control register. The TCM ACT accept value bits have a default value of
 * 0x00 (hex).
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_LOW_TCM_ACT_ACCEPT_VALUE . TCM_ACT_ACCEPT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_LOW_TCM_ACT_ACCEPT_VALUE_TCM_ACT_ACCEPT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_LOW_TCM_ACT_ACCEPT_VALUE_TCM_ACT_ACCEPT_VALUE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_LOW_TCM_ACT_ACCEPT_VALUE_TCM_ACT_ACCEPT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief XCO2 Rx OTN ODU FTFL extract memory 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_FTFL_EXTRACT
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: XCO2_RX_OTN_ODU_FTFL_EXTRACT (??), 0-127
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_FTFL_EXTRACT(target,ri)  VTSS_IOREG(target,0xc63 + (ri))

/** 
 * \brief
 * FTFL extract value
 * The FTFL extract value bits are updated when identical extracted 256
 * byte FTFL messages are received for the number of consecutive sequences
 * programmed through the FTFL accept count bits of the Rx OTN ODU FTFL/EXP
 * Control register. These bits are undefined as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_FTFL_EXTRACT . FTFL_EXTRACT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_FTFL_EXTRACT_FTFL_EXTRACT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_FTFL_EXTRACT_FTFL_EXTRACT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_FTFL_EXTRACT_FTFL_EXTRACT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN ODU EXP accept register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_EXP_ACCEPT
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_EXP_ACCEPT(target)  VTSS_IOREG(target,0xce3)

/** 
 * \brief
 * EXP accept value
 * The EXP accept value bits are updated when identical extracted EXP bytes
 * are received for the number of consecutive frames programmed through the
 * EXP accept count bits of the Rx OTN ODU FTFL/EXP Control register. The
 * EXP accept value bits have a default value of 0x00 (hex).
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_EXP_ACCEPT . EXP_ACCEPT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_EXP_ACCEPT_EXP_ACCEPT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_EXP_ACCEPT_EXP_ACCEPT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_EXP_ACCEPT_EXP_ACCEPT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN ODU GCC1 accept register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_GCC1_ACCEPT
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC1_ACCEPT(target)  VTSS_IOREG(target,0xce4)

/** 
 * \brief
 * GCC1 accept value
 * The GCC1 accept value bits are updated when identical extracted GCC1
 * bytes are received for the number of consecutive sequences as programmed
 * through the GCC1 accept count bits of the Rx OTN ODU GCC1/GCC2 Control
 * register. The GCC1 accept value bits have a default value of 0x00 (hex).
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC1_ACCEPT . GCC1_ACCEPT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC1_ACCEPT_GCC1_ACCEPT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC1_ACCEPT_GCC1_ACCEPT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC1_ACCEPT_GCC1_ACCEPT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN ODU GCC2 accept register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_GCC2_ACCEPT
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC2_ACCEPT(target)  VTSS_IOREG(target,0xce5)

/** 
 * \brief
 * GCC2 accept value
 * The GCC2 accept value bits are updated when identical extracted GCC2
 * bytes are received for the number of consecutive sequences as programmed
 * through the GCC2 accept count bits of the Rx OTN ODU GCC1/GCC2 Control
 * register. The GCC2 accept value bits have a default value of 0x00 (hex). 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC2_ACCEPT . GCC2_ACCEPT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC2_ACCEPT_GCC2_ACCEPT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC2_ACCEPT_GCC2_ACCEPT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_GCC2_ACCEPT_GCC2_ACCEPT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN ODU APS/PCC MFASX high accept register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_APS_PCC_MFASX_HIGH_ACCEPT
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: XCO2_RX_OTN_ODU_APS_PCC_MFASX_HIGH_ACCEPT (??), 0-7
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_MFASX_HIGH_ACCEPT(target,ri)  VTSS_IOREG(target,0xce6 + (ri))

/** 
 * \brief
 * APSPCC MFASx high accept value
 * The APS/PCC MFASx high accept value bits are updated when identical
 * extracted APS/PCC MFASx bytes are received for the number of consecutive
 * sequences programmed through the APS/PCC accept count bits of the Rx OTN
 * ODU APS/PCC/RES2 Control register. The APS/PCC MFASx high accept value
 * bits have a default value of 0x00 (hex) and are read only.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_MFASX_HIGH_ACCEPT . APSPCC_MFASX_HIGH_ACCEPT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_MFASX_HIGH_ACCEPT_APSPCC_MFASX_HIGH_ACCEPT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_MFASX_HIGH_ACCEPT_APSPCC_MFASX_HIGH_ACCEPT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_MFASX_HIGH_ACCEPT_APSPCC_MFASX_HIGH_ACCEPT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN ODU APS/PCC MFASX low accept register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_APS_PCC_MFASX_LOW_ACCEPT
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: XCO2_RX_OTN_ODU_APS_PCC_MFASX_LOW_ACCEPT (??), 0-7
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_MFASX_LOW_ACCEPT(target,ri)  VTSS_IOREG(target,0xcee + (ri))

/** 
 * \brief
 * APSPCC MFASx low accept value
 * The APS/PCC MFASx low accept value bits are updated when identical
 * extracted APS/PCC MFASx bytes are received for the number of consecutive
 * frames programmed through the APS/PCC accept count bits of Rx OTN ODU
 * APS/PCC/RES2 Control register. The APS/PCC MFASx low accept value bits
 * have a default value of 0x00 (hex) and are read only.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_MFASX_LOW_ACCEPT . APSPCC_MFASX_LOW_ACCEPT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_MFASX_LOW_ACCEPT_APSPCC_MFASX_LOW_ACCEPT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_MFASX_LOW_ACCEPT_APSPCC_MFASX_LOW_ACCEPT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_MFASX_LOW_ACCEPT_APSPCC_MFASX_LOW_ACCEPT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN ODU RES2 high accept register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_RES2_HIGH_ACCEPT
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES2_HIGH_ACCEPT(target)  VTSS_IOREG(target,0xcf6)

/** 
 * \brief
 * RES2 high accept value
 * The RES2 high accept value bits are updated when identical extracted
 * RES2 bytes are received for the number of consecutive frames programmed
 * through the RES2 accept count bits of the Rx OTN ODU APS/PCC/RES2
 * Control register. The RES2 high accept value bits have a default value
 * of 0x00 (hex) and are read only.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES2_HIGH_ACCEPT . RES2_HIGH_ACCEPT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES2_HIGH_ACCEPT_RES2_HIGH_ACCEPT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES2_HIGH_ACCEPT_RES2_HIGH_ACCEPT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES2_HIGH_ACCEPT_RES2_HIGH_ACCEPT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN ODU RES2 mid accept register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_RES2_MID_ACCEPT
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES2_MID_ACCEPT(target)  VTSS_IOREG(target,0xcf7)

/** 
 * \brief
 * RES2 mid accept value
 * The RES2 mid accept value bits are updated when identical extracted RES2
 * bytes are received for the number of consecutive frames programmed
 * through the RES2 accept count bits of the Rx OTN ODU APS/PCC/RES2
 * Control register. The RES2 mid accept value bits have a default value of
 * 0x00 (hex) and are read only.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES2_MID_ACCEPT . RES2_MID_ACCEPT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES2_MID_ACCEPT_RES2_MID_ACCEPT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES2_MID_ACCEPT_RES2_MID_ACCEPT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES2_MID_ACCEPT_RES2_MID_ACCEPT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN ODU RES2 low accept register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_RES2_LOW_ACCEPT
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES2_LOW_ACCEPT(target)  VTSS_IOREG(target,0xcf8)

/** 
 * \brief
 * RES2 low accept value
 * The RES2 low accept value bits are updated when identical extracted RES2
 * bytes are received for the number of consecutive frames programmed
 * through the RES2 accept count bits of the Rx OTN ODU APS/PCC/RES2
 * Control register. The RES2 low accept value bits have a default value of
 * 0x00 (hex) and are read only. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES2_LOW_ACCEPT . RES2_LOW_ACCEPT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES2_LOW_ACCEPT_RES2_LOW_ACCEPT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES2_LOW_ACCEPT_RES2_LOW_ACCEPT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES2_LOW_ACCEPT_RES2_LOW_ACCEPT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN ODU TCMX control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_TCMX_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: XCO2_RX_OTN_ODU_TCMX_CONTROL (??), 0-5
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL(target,ri)  VTSS_IOREG(target,0xcfd + (ri))

/** 
 * \brief
 * TCMi TTI SAPI TIM
 * The TCMi TTI SAPI TIM bit is set, the TCMi TTI SAPI bytes are included
 * in the comparison of the accepted TCMi TTI and expected TCMi TTI bytes
 * for trace identifier mismatch (TCMi TIM) detection. This bit is cleared
 * as a default value.	 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL . TCMI_TTI_SAPI_TIM
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_TTI_SAPI_TIM(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_TTI_SAPI_TIM     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_TTI_SAPI_TIM(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * TCMi TTI DAPI TIM
 * The TCMi TTI DAPI TIM bit is set, the TCMi TTI DAPI bytes are included
 * in the comparison of the accepted TCMi TTI and expected TCMi TTI bytes
 * for trace identifier mismatch (TCMi TIM) detection. This bit is cleared
 * as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL . TCMI_TTI_DAPI_TIM
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_TTI_DAPI_TIM(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_TTI_DAPI_TIM     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_TTI_DAPI_TIM(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * TCMi TTI OS TIM
 * The TCMi TTI OS TIM bit is set, the TCMi TTI operator specific bytes are
 * included in the comparison of the accepted TCMi TTI and expected TCMi
 * TTI bytes for trace identifier mismatch (TCMi TIM) detection. This bit
 * is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL . TCMI_TTI_OS_TIM
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_TTI_OS_TIM(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_TTI_OS_TIM     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_TTI_OS_TIM(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * TCMi STAT filter 5
 * The TCMi STAT filter 5 bit is set to indicate that five consecutive
 * frames with identical STAT fields are required to update the TCMi STAT
 * value bits. If this bit is cleared, three consecutive frames with
 * identical STAT field values will update the TCMi STAT value bits. This
 * bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL . TCMI_STAT_FILTER_5
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_STAT_FILTER_5(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_STAT_FILTER_5     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_STAT_FILTER_5(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * TCMi STAT accept value
 * The TCMi STAT accept value bits are updated when identical extracted
 * TCMi STAT bytes are received for the number of consecutive sequences
 * programmed through the TCMi STAT filter 5 bits. The TCMi STAT accept
 * value bits have a default value of 000 and are read only bits.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL . TCMI_STAT_ACCEPT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_STAT_ACCEPT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_STAT_ACCEPT_VALUE     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_STAT_ACCEPT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * TCMi BIAE filter 5
 * The TCMi BIAE filter 5 bit is set to indicate that five consecutive
 * frames with TCMi BIAE field values of 1011 are required to detect an
 * incoming TCMi BIAE error condition. If this bit is cleared, three
 * consecutive frames with TCMi BIAE field values of 1011 will result in
 * TCMi BIAE error detection. Likewise, the TCMi BIAE filter 5 bit is used
 * to determine the number of consecutive (3 or 5) frames with TCMi BIAE
 * field values not equal to 1011 required to transition from a TCMi BIAE
 * error state to an error free TCMi BIAE condition. This bit is cleared as
 * a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL . TCMI_BIAE_FILTER_5
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_BIAE_FILTER_5(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_BIAE_FILTER_5     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_BIAE_FILTER_5(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * TCMi BDI filter 5
 * The TCMi BDI filter 5 bit is set to indicate that five consecutive
 * frames with TCMi BDI field values of 1 are required to detect an
 * incoming TCMi BDI error condition. If this bit is cleared, three
 * consecutive frames with TCMi BDI field value of 1 will result in TCMi
 * BDI error detection. Likewise, the TCMi BDI filter 5 bit is used to
 * determine the number of consecutive (3 or 5) frames with TCMi BDI field
 * values of 0 required to transition from a TCMi BDI error state to an
 * error free TCMi BDI condition. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL . TCMI_BDI_FILTER_5
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_BDI_FILTER_5(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_BDI_FILTER_5     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_BDI_FILTER_5(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * TCMi TTI accept count
 * The TCMi TTI accept count bits are used to determine the number of
 * identical TCMi TTI messages that must be received consecutively to
 * update the TCMi TTI extract value bits of the Rx OTN ODU TCMi TTI
 * Extract register with the current TCMi TTI message. 
 * NOTE: A count of 0 is invalid and should not be programmed. The TCMi TTI
 * accept count bits are set to a value of 0101 as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL . TCMI_TTI_ACCEPT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_TTI_ACCEPT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,3,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_TTI_ACCEPT_COUNT     VTSS_ENCODE_BITMASK(3,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_TTI_ACCEPT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,3,3)

/** 
 * \brief
 * TCMi TTI inconsistent count
 * The TCMi TTI inconsistent count bits are used to determine the number of
 * non identical TCMi TTI messages that must be received to declare a TTI
 * inconsistent error. 
 * NOTE: A count of 0 is invalid and should not be programmed. The TCMi TTI
 * inconsistent count bits are set to a value of 0101 as a default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL . TCMI_TTI_INCONSISTENT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_TTI_INCONSISTENT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_TTI_INCONSISTENT_COUNT     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_TTI_INCONSISTENT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief XCO2 Rx OTN ODU TCMX TTI extract memory 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_TCMX_TTI_EXTRACT
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: XCO2_RX_OTN_ODU_TCMX_TTI_EXTRACT (??), 0-191
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_TTI_EXTRACT(target,ri)  VTSS_IOREG(target,0xd03 + (ri))

/** 
 * \brief
 * TCMi TTI extract value
 * The TCMi TTI extract value bits contain the accepted 64 byte trail trace
 * identifier (SAPI/DAPI/Operator Specific) message extracted from incoming
 * frames. This information is used for comparison to the expected TCMi TTI
 * message information for TCMi trail trace identifier mismatch (TCMi TIM)
 * detection. These bits are undefined as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_TTI_EXTRACT . TCMI_TTI_EXTRACT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_TTI_EXTRACT_TCMI_TTI_EXTRACT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_TTI_EXTRACT_TCMI_TTI_EXTRACT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_TTI_EXTRACT_TCMI_TTI_EXTRACT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN ODU TCMX TTI expect memory 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_TCMX_TTI_EXPECT
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: XCO2_RX_OTN_ODU_TCMX_TTI_EXPECT (??), 0-191
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_TTI_EXPECT(target,ri)  VTSS_IOREG(target,0xdc3 + (ri))

/** 
 * \brief
 * TCMi TTI expect value
 * The TCMi TTI expect value bits contain the 64 byte trail trace
 * identifier (SAPI/DAPI/Operator Specific) message to be used for
 * comparison with the extracted TCMi TTI message information for TCMi
 * trail trace identifier mismatch (TCMi TIM) detection. These bits are
 * undefined as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_TTI_EXPECT . TCMI_TTI_EXPECT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_TTI_EXPECT_TCMI_TTI_EXPECT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_TTI_EXPECT_TCMI_TTI_EXPECT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_TTI_EXPECT_TCMI_TTI_EXPECT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN ODU TCMX BEI counter 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_TCMX_BEI
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: XCO2_RX_OTN_ODU_TCMX_BEI (??), 0-5
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_BEI(target,ri)  VTSS_IOREG(target,0xe8f + (ri))

/** 
 * \brief
 * TCMi BEI counter value
 * The TCMi BEI counter value bits are used to accumulate the number of BEI
 * errors detected in the TCMi BEI field of the ODU overhead. This is a
 * saturating read only counter. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_BEI . TCMI_BEI_COUNTER_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_BEI_TCMI_BEI_COUNTER_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_BEI_TCMI_BEI_COUNTER_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_BEI_TCMI_BEI_COUNTER_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN ODU TCMX BIP counter 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_TCMX_BIP
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: XCO2_RX_OTN_ODU_TCMX_BIP (??), 0-5
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_BIP(target,ri)  VTSS_IOREG(target,0xe95 + (ri))

/** 
 * \brief
 * TCMi BIP counter value
 * The TCMi BIP counter value bits are used to accumulate the number of
 * TCMi BIP errors detected for incoming frames. This is a saturating read
 * only counter.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_BIP . TCMI_BIP_COUNTER_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_BIP_TCMI_BIP_COUNTER_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_BIP_TCMI_BIP_COUNTER_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_BIP_TCMI_BIP_COUNTER_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN ODU PM control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_PM_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL(target)  VTSS_IOREG(target,0xe9b)

/** 
 * \brief
 * PM TTI SAPI TIM
 * If the PM TTI SAPI TIM bit is set, the PM TTI SAPI bytes are included in
 * the comparison of the accepted PM TTI and expected PM TTI bytes for
 * trace identifier mismatch (PM TIM) detection. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL . PM_TTI_SAPI_TIM
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_TTI_SAPI_TIM(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_TTI_SAPI_TIM     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_TTI_SAPI_TIM(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * PM TTI DAPI TIM
 * If the PM TTI DAPI TIM bit is set, the PM TTI DAPI bytes are included in
 * the comparison of the accepted PM TTI and expected PM TTI bytes for
 * trace identifier mismatch (PM TIM) detection. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL . PM_TTI_DAPI_TIM
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_TTI_DAPI_TIM(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_TTI_DAPI_TIM     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_TTI_DAPI_TIM(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * PM TTI OS TIM
 * If the PM TTI OS TIM bit is set, the PM TTI operator specific bytes are
 * included in the comparison of the accepted PM TTI and expected PM TTI
 * bytes for trace identifier mismatch (PM TIM) detection. This bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL . PM_TTI_OS_TIM
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_TTI_OS_TIM(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_TTI_OS_TIM     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_TTI_OS_TIM(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * PM STAT filter 5
 * The PM STAT filter 5 bit is set to indicate that five consecutive frames
 * with identical STAT fields are required to update the PM STAT value
 * bits. If this bit is cleared, three consecutive frames with identical
 * STAT field values will update the PM STAT value bits. This bit is
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL . PM_STAT_FILTER_5
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_STAT_FILTER_5(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_STAT_FILTER_5     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_STAT_FILTER_5(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * PM STAT accept value
 * The PM STAT accept value bits are updated when identical extracted PM
 * STAT bytes are received for the number of consecutive sequences
 * programmed through the PM STAT filter 5 bits. The PM STAT accept value
 * bits have a default value of 000 and are read only bits.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL . PM_STAT_ACCEPT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_STAT_ACCEPT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_STAT_ACCEPT_VALUE     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_STAT_ACCEPT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * PM BDI filter 5
 * The TCMi BDI filter 5 bit is set to indicate that five consecutive
 * frames with PM BDI field values of 1 are required to detect an incoming
 * PM BDI error condition. If this bit is cleared, three consecutive frames
 * with PM BDI field value of 1 will result in PM BDI error detection.
 * Likewise, the PM BDI filter 5 bit is used to determine the number of
 * consecutive (3 or 5) frames with PM BDI field values of 0 required to
 * transition from a PM BDI error state to an error free PM BDI condition.
 * This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL . PM_BDI_FILTER_5
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_BDI_FILTER_5(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_BDI_FILTER_5     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_BDI_FILTER_5(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * PM TTI accept count
 * The PM TTI accept count bits are used to determine the number of
 * identical PM TTI messages that must be received 
 * consecutively in order to update the PM TTI extract value bits of the Rx
 * OTN ODU PM TTI Extract register with the current PM TTI message. 
 * NOTE: A count of 0 is invalid and should not be programmed. The PM TTI
 * accept count bits are set to a value of 0101 as a default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL . PM_TTI_ACCEPT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_TTI_ACCEPT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,3,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_TTI_ACCEPT_COUNT     VTSS_ENCODE_BITMASK(3,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_TTI_ACCEPT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,3,3)

/** 
 * \brief
 * PM TTI inconsistent count
 * The PM TTI inconsistent count bits are used to determine the number of
 * non identical PM TTI messages that must be received in order to declare
 * a TTI inconsistent error. 
 * NOTE: A count of 0 is invalid and should not be programmed. The PM TTI
 * inconsistent count bits are set to a value of 0101 as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL . PM_TTI_INCONSISTENT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_TTI_INCONSISTENT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_TTI_INCONSISTENT_COUNT     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_TTI_INCONSISTENT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief XCO2 Rx OTN ODU PM TTI extract memory 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_PM_TTI_EXTRACT
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: XCO2_RX_OTN_ODU_PM_TTI_EXTRACT (??), 0-31
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_TTI_EXTRACT(target,ri)  VTSS_IOREG(target,0xe9c + (ri))

/** 
 * \brief
 * PM TTI extract value
 * The PM TTI extract value bits contain the accepted 64 byte trail trace
 * identifier (SAPI/DAPI/Operator Specific) message extracted from incoming
 * frames. This information is used for comparison to the expected PM TTI
 * message information for PM trail trace identifier mismatch (PM TIM)
 * detection. These bits are undefined as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_TTI_EXTRACT . PM_TTI_EXTRACT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_TTI_EXTRACT_PM_TTI_EXTRACT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_TTI_EXTRACT_PM_TTI_EXTRACT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_TTI_EXTRACT_PM_TTI_EXTRACT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN ODU PM TTI expect memory 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_PM_TTI_EXPECT
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: XCO2_RX_OTN_ODU_PM_TTI_EXPECT (??), 0-31
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_TTI_EXPECT(target,ri)  VTSS_IOREG(target,0xebc + (ri))

/** 
 * \brief
 * PM TTI expect value
 * The PM TTI expect value bits contain the 64 byte trail trace identifier
 * (SAPI/DAPI/Operator Specific) message to be used for comparison to the
 * extracted PM TTI message information for PM trail trace identifier
 * mismatch (PM TIM) detection. These bits are undefined as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_TTI_EXPECT . PM_TTI_EXPECT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_TTI_EXPECT_PM_TTI_EXPECT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_TTI_EXPECT_PM_TTI_EXPECT_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_TTI_EXPECT_PM_TTI_EXPECT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN ODU PM BEI counter 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_PM_BEI
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_BEI(target)  VTSS_IOREG(target,0xede)

/** 
 * \brief
 * PM BEI counter value
 * The PM BEI counter value bits are used to accumulate the number of BEI
 * errors detected in the PM BEI field of the ODU overhead. This is a
 * saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_BEI . PM_BEI_COUNTER_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_BEI_PM_BEI_COUNTER_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_BEI_PM_BEI_COUNTER_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_BEI_PM_BEI_COUNTER_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN ODU PM BIP counter 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_PM_BIP
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_BIP(target)  VTSS_IOREG(target,0xedf)

/** 
 * \brief
 * PM BIP counter value
 * The PM BIP counter value bits are used to accumulate the number of BIP
 * errors detected for incoming frames. This is a saturating read only
 * counter.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_BIP . PM_BIP_COUNTER_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_BIP_PM_BIP_COUNTER_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_BIP_PM_BIP_COUNTER_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_BIP_PM_BIP_COUNTER_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN OPU control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_OPU_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_CONTROL(target)  VTSS_IOREG(target,0xee0)

/** 
 * \brief
 * Async mapping
 * If the async mapping bit is set, asynchronous client signal payload
 * mapping is expected in received OTN frames. Positive and negative
 * justifications are processed for asynchronous mapping. If the async
 * mapping bit is cleared, synchronous client signal payload mapping is
 * expected in received OTN frames. Positive and negative justifications
 * are not processed for synchronous mapping. The async mapping bit is set
 * as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_CONTROL . ASYNC_MAPPING
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_CONTROL_ASYNC_MAPPING(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_CONTROL_ASYNC_MAPPING     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_CONTROL_ASYNC_MAPPING(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Stuff enable
 * If the stuff enable bit is set, column bit stuffing is expected in
 * received OTN frames as specified by G.709. 
 * NOTE: This bit is ignored when streaming mode is enabled. This bit is
 * set as a default value. 
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_CONTROL . STUFF_ENABLE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_CONTROL_STUFF_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_CONTROL_STUFF_ENABLE     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_CONTROL_STUFF_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * PT accept count
 * The PT accept count bits are used to determine the number of identical
 * PT bytes that must be received consecutively to update the PT extract
 * value bits of the Rx OTN OPU PT Extract register with the current PT
 * message. 
 * NOTE: A count of 0 is invalid and should not be programmed. The PT
 * accept count bits are set to a value of 0101 as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_CONTROL . PT_ACCEPT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_CONTROL_PT_ACCEPT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,11,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_CONTROL_PT_ACCEPT_COUNT     VTSS_ENCODE_BITMASK(11,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_CONTROL_PT_ACCEPT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,11,3)

/** 
 * \brief
 * PT inconsistent count
 * The PT inconsistent count bits are used to determine the number of non
 * identical PT bytes that must be received to declare a PT inconsistent
 * error. 
 * NOTE: A count of 0 is invalid and should not be programmed. The PT
 * inconsistent count bits are set to a value of 0101 as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_CONTROL . PT_INCONSISTENT_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_CONTROL_PT_INCONSISTENT_COUNT(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_CONTROL_PT_INCONSISTENT_COUNT     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_CONTROL_PT_INCONSISTENT_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * PT expect value
 * The PT expect value bits contain the expected payload type to be used
 * for comparison to the extracted PT information for payload label
 * mismatch (PLM) detection. These bits are set to a value of 2 as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_CONTROL . PT_EXPECT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_CONTROL_PT_EXPECT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_CONTROL_PT_EXPECT_VALUE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_CONTROL_PT_EXPECT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief XCO2 Rx OTN OPU fixed payload expect/PT accept register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_OPU_FIXED_PYLD_EXPECT_PT_ACCEPT
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_FIXED_PYLD_EXPECT_PT_ACCEPT(target)  VTSS_IOREG(target,0xee1)

/** 
 * \brief
 * Fixed payload expect value
 * The fixed payload expect value bits are compared to incoming OTN payload
 * bytes for the detection of the fixed payload value. If the expected
 * fixed payload value matches the incoming payload for a complete OTN
 * frame, a client fixed payload interrupt will be recorded into the Rx OTN
 * OPU Interrupt register. The fixed payload expect value bits are cleared
 * as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_FIXED_PYLD_EXPECT_PT_ACCEPT . FIXED_PAYLOAD_EXPECT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_FIXED_PYLD_EXPECT_PT_ACCEPT_FIXED_PAYLOAD_EXPECT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_FIXED_PYLD_EXPECT_PT_ACCEPT_FIXED_PAYLOAD_EXPECT_VALUE     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_FIXED_PYLD_EXPECT_PT_ACCEPT_FIXED_PAYLOAD_EXPECT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * PT accept value
 * The PT accept value bits are updated when identical extracted PT bytes
 * are received for the number of consecutive sequences programmed into the
 * PT accept count bits of the Rx OTN OPU Control register. The PT accept
 * value bits have a default value of 0x00 (hex) and are read only.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_FIXED_PYLD_EXPECT_PT_ACCEPT . PT_ACCEPT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_FIXED_PYLD_EXPECT_PT_ACCEPT_PT_ACCEPT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_FIXED_PYLD_EXPECT_PT_ACCEPT_PT_ACCEPT_VALUE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_FIXED_PYLD_EXPECT_PT_ACCEPT_PT_ACCEPT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief XCO2 Rx OTN OPU FIFO high threshold register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_OPU_FIFO_HIGH_THRESHOLD
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_FIFO_HIGH_THRESHOLD(target)  VTSS_IOREG(target,0xee4)

/** 
 * \brief
 * FIFO high threshold value
 * If the difference between the Rx FIFO write address (line side) and the
 * Rx FIFO read address (system side) is more than the value programmed
 * into the FIFO high threshold value bits, the Rx SYS UPPER THRESH OUT
 * output signal is asserted indicating a FIFO high threshold condition
 * exists.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_FIFO_HIGH_THRESHOLD . FIFO_HIGH_THRESHOLD_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_FIFO_HIGH_THRESHOLD_FIFO_HIGH_THRESHOLD_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_FIFO_HIGH_THRESHOLD_FIFO_HIGH_THRESHOLD_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_FIFO_HIGH_THRESHOLD_FIFO_HIGH_THRESHOLD_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN OPU FIFO low threshold register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_OPU_FIFO_LOW_THRESHOLD
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_FIFO_LOW_THRESHOLD(target)  VTSS_IOREG(target,0xee5)

/** 
 * \brief
 * FIFO low threshold value
 * If the difference between the Rx FIFO write address (line side) and the
 * Rx FIFO read address (system side) is less than the value programmed
 * into the FIFO low threshold value bits, the Rx SYS LOWER THRESH OUT
 * output signal is asserted indicating a FIFO low threshold condition
 * exists.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_FIFO_LOW_THRESHOLD . FIFO_LOW_THRESHOLD_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_FIFO_LOW_THRESHOLD_FIFO_LOW_THRESHOLD_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_FIFO_LOW_THRESHOLD_FIFO_LOW_THRESHOLD_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_FIFO_LOW_THRESHOLD_FIFO_LOW_THRESHOLD_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN OPU PJ counter 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_OPU_PJ
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_PJ(target)  VTSS_IOREG(target,0xee6)

/** 
 * \brief
 * PJ counter value
 * The Rx OTN OPU PJ counter is used to accumulate the number of positive
 * justifications detected in the receive frame controller when an
 * asynchronous mapping payload type is configured. This is a saturating
 * read only counter.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_PJ . PJ_COUNTER_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_PJ_PJ_COUNTER_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_PJ_PJ_COUNTER_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_PJ_PJ_COUNTER_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN OPU NJ counter 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_OPU_NJ
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_NJ(target)  VTSS_IOREG(target,0xee7)

/** 
 * \brief
 * NJ counter value
 * The Rx OTN OPU NJ counter is used to accumulate the number of negative
 * justifications detected in the receive frame controller when an
 * asynchronous mapping payload type is configured. This is a saturating
 * read only counter.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_NJ . NJ_COUNTER_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_NJ_NJ_COUNTER_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_NJ_NJ_COUNTER_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_NJ_NJ_COUNTER_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN OPU PRBS upper error counter 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_OPU_PRBS_UPPER_ERROR
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_PRBS_UPPER_ERROR(target)  VTSS_IOREG(target,0xee8)

/** 
 * \brief
 * PRBS upper error counter high value
 * The PRBS upper error counter value bits are used to accumulate the
 * number of PRBS errors detected. The PRBS upper error counter value bits
 * contain the upper 8 bits of the 24 bit Rx OTN OPU PRBS Error Counter.
 * This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_PRBS_UPPER_ERROR . PRBS_UPPER_ERROR_COUNTER_HIGH_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_PRBS_UPPER_ERROR_PRBS_UPPER_ERROR_COUNTER_HIGH_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_PRBS_UPPER_ERROR_PRBS_UPPER_ERROR_COUNTER_HIGH_VALUE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_PRBS_UPPER_ERROR_PRBS_UPPER_ERROR_COUNTER_HIGH_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief XCO2 Rx OTN OPU PRBS lower error counter 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_OPU_PRBS_LOWER_ERROR
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_PRBS_LOWER_ERROR(target)  VTSS_IOREG(target,0xee9)

/** 
 * \brief
 * PRBS lower error counter high value
 * The PRBS lower error counter value bits are used to accumulate the
 * number of PRBS errors detected. The PRBS lower error counter value bits
 * contain the lower 16 bits of the 24 bit Rx OTN OPU PRBS Error Counter.
 * This is a saturating read only counter.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_PRBS_LOWER_ERROR . PRBS_LOWER_ERROR_COUNTER_HIGH_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_PRBS_LOWER_ERROR_PRBS_LOWER_ERROR_COUNTER_HIGH_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_PRBS_LOWER_ERROR_PRBS_LOWER_ERROR_COUNTER_HIGH_VALUE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_PRBS_LOWER_ERROR_PRBS_LOWER_ERROR_COUNTER_HIGH_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief XCO2 Rx OTN ODU TCMX insert control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL (??), 0-5
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL(target,ri)  VTSS_IOREG(target,0xeea + (ri))

/** 
 * \brief
 * TCMi transparent
 * If the TCMi transparent bit is set, TCMi is in transparent mode and
 * overhead is not inserted into TCMi if streaming mode is enabled (all 0s
 * inserted if streaming mode is disabled). If the TCMi transparent bit is
 * cleared, TCMi is in active mode and the tandem connection inserts the
 * appropriate overhead as programmed. This bit is set as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL . TCMI_TRANSPARENT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_TCMI_TRANSPARENT(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_TCMI_TRANSPARENT     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_TCMI_TRANSPARENT(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Insert TCMi TTI
 * If the insert TCMi TTI bit is set, the tandem connection monitoring
 * trail trace identifier (TCMi TTI) is inserted as specified by the TCMi
 * TTI value contained in the Rx OTN ODU TCMi Insert TTI Memory. The TCMi
 * TTI value inserted is aligned to multiframe boundaries and repeats four
 * times for each complete multiframe sequence. This bit is only valid when
 * the TCMi transparent bit is cleared. This bit is set as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL . INSERT_TCMI_TTI
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_INSERT_TCMI_TTI(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_INSERT_TCMI_TTI     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_INSERT_TCMI_TTI(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Insert TCMi BIP
 * If the insert TCMi BIP bit is set, the value generated by the BIP-8
 * calculator module is inserted into the TCMi BIP-8 position of outgoing
 * ODU frames. This bit is only valid when the TCMi transparent bit is
 * cleared. The insert TCMi BIP bit is set as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL . INSERT_TCMI_BIP
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_INSERT_TCMI_BIP(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_INSERT_TCMI_BIP     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_INSERT_TCMI_BIP(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * TCMi BIP corrupt
 * If the TCMi BIP corrupt bit is set, the value inserted for TCMi BIP-8 is
 * corrupted (calculated value inverted) when the insert TCMi BIP bit is
 * set. If the TCMi BIP corrupt bit is cleared, TCMi BIP-8 corruption is
 * inhibited. This bit is only valid when the TCMi transparent bit is
 * cleared. The TCMi BIP corrupt bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL . TCMI_BIP_CORRUPT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_TCMI_BIP_CORRUPT(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_TCMI_BIP_CORRUPT     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_TCMI_BIP_CORRUPT(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Insert TCMi BEI/BIAE
 * If the insert TCMi BEI/BIAE bit is set, the value programmed into the
 * TCMi BEI/BIAE value bits of the Rx OTN ODU TCMi Value Insert register
 * will be inserted into the TCMi BEI/BIAE bits of outgoing ODU frames.
 * This bit is only valid when the TCMi transparent bit is cleared. This
 * bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL . INSERT_TCMI_BEIBIAE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_INSERT_TCMI_BEIBIAE(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_INSERT_TCMI_BEIBIAE     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_INSERT_TCMI_BEIBIAE(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Auto TCMi BEI/BIAE
 * If the auto TCMi BEI/BIAE bit is set, the number of TCMi BIP errors
 * detected (count from 0 to 8) in the Rx direction are inserted into the
 * TCMi BEI/BIAE field of outgoing ODU frames. If an incoming alignment
 * error (accepted Rx TCMi STAT bits equal 010) is detected in the Rx
 * direction, a fixed value of 1011 is inserted into the SM BEI/BIAE field
 * of outgoing ODU frames when the auto TCMi BEI/BIAE bit is set. If the
 * auto TCMi BEI/BIAE bit is cleared, auto insertion of the TCMi BEI/BIAE
 * field is ignored. This bit is only valid when the TCMi transparent bit
 * is cleared. This bit is set as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL . AUTO_TCMI_BEIBIAE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_AUTO_TCMI_BEIBIAE(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_AUTO_TCMI_BEIBIAE     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_AUTO_TCMI_BEIBIAE(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Insert TCMi BDI
 * If the insert TCMi BDI bit is set, the value programmed into the TCMi
 * BDI value bit of the Rx OTN ODU TCMi Value Insert register will be
 * inserted into the TCMi BDI bit of outgoing ODU frames. This bit is only
 * valid when the TCMi transparent bit is cleared. This bit is cleared as a
 * default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL . INSERT_TCMI_BDI
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_INSERT_TCMI_BDI(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_INSERT_TCMI_BDI     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_INSERT_TCMI_BDI(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Auto TCMi BDI
 * If the auto TCMi BDI bit is set, the TCMi BDI bit for outgoing ODU
 * frames will be set according to the Rx direction backward defect state
 * which is defined by the enabled bits of the Rx OTN ODU TCMi Auto BDI
 * Insert Control register. If the auto TCMi BDI bit is cleared, auto
 * insertion of the TCMi BDI field is ignored. This bit is only valid when
 * the TCMi transparent bit is cleared. This bit is cleared as a default
 * value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL . AUTO_TCMI_BDI
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_AUTO_TCMI_BDI(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_AUTO_TCMI_BDI     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_AUTO_TCMI_BDI(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Insert TCMi STAT
 * If the insert TCMi STAT bit is set, the value programmed into the TCMi
 * STAT value bits of the Rx OTN ODU TCMi Value Insert register will be
 * inserted into the TCMi STAT bits of outgoing ODU frames. This bit is
 * only valid when the TCMi transparent bit is cleared. This bit is cleared
 * as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL . INSERT_TCMI_STAT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_INSERT_TCMI_STAT(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_INSERT_TCMI_STAT     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_INSERT_TCMI_STAT(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Auto TCMi STAT IAE
 * If the auto TCMi STAT IAE bit is set, the TCMi STAT field bits for all
 * originating TCMs (any TCMi not configured for transparent mode) will be
 * set to 010 (in use with IAE) for the number of multiframes programmed
 * into the IAE multiframe count bits of the Rx OTN ODU TCMi Auto BDI
 * Control register when an incoming transmit system side IAE condition is
 * detected in streaming mode. An internal multiframe counter restarts each
 * time an incoming IAE condition is detected. The TCMi STAT field bits
 * will be set to 001 (in use without IAE) after the multiframe counter
 * reaches its terminal count. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL . AUTO_TCMI_STAT_IAE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_AUTO_TCMI_STAT_IAE(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_AUTO_TCMI_STAT_IAE     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_CONTROL_AUTO_TCMI_STAT_IAE(x)  VTSS_EXTRACT_BITFIELD(x,6,1)


/** 
 * \brief XCO2 Rx OTN ODU TCMi auto BDI insert control register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_TCMI_AUTO_BDI_INSERT_CONTROL
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMI_AUTO_BDI_INSERT_CONTROL(target)  VTSS_IOREG(target,0xef0)

/** 
 * \brief
 * TCMi BDI TCMi ODUk AIS
 * If the TCMi BDI TCMi ODUk AIS bit is set, the BDI field of active TCMs
 * is set for outgoing frames if a Tx TCMi ODUk AIS condition is detected
 * (accepted TCMi STAT is 111) and the appropriate auto TCMi BDI bit is
 * set. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMI_AUTO_BDI_INSERT_CONTROL . TCMI_BDI_TCMI_ODUK_AIS
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMI_AUTO_BDI_INSERT_CONTROL_TCMI_BDI_TCMI_ODUK_AIS(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMI_AUTO_BDI_INSERT_CONTROL_TCMI_BDI_TCMI_ODUK_AIS     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMI_AUTO_BDI_INSERT_CONTROL_TCMI_BDI_TCMI_ODUK_AIS(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * TCMi BDI TCMi LTC
 * If the TCMi BDI TCMi LTC bit is set, the BDI field of all active TCMs is
 * set for outgoing frames if a Tx Loss of Tandem Connection - LTC
 * (accepted TCMi STAT is 000) condition is detected and the appropriate
 * auto TCMi BDI bit is set. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMI_AUTO_BDI_INSERT_CONTROL . TCMI_BDI_TCMI_LTC
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMI_AUTO_BDI_INSERT_CONTROL_TCMI_BDI_TCMI_LTC(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMI_AUTO_BDI_INSERT_CONTROL_TCMI_BDI_TCMI_LTC     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMI_AUTO_BDI_INSERT_CONTROL_TCMI_BDI_TCMI_LTC(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * TCMi BDI TCMi OCI
 * If the TCMi BDI TCMi OCI bit is set, the BDI field of active TCMs is set
 * for outgoing frames if a Tx TCMi OCI condition is detected (accepted
 * TCMi STAT is 110) and the appropriate auto TCMi BDI bit is set. This bit
 * is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMI_AUTO_BDI_INSERT_CONTROL . TCMI_BDI_TCMI_OCI
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMI_AUTO_BDI_INSERT_CONTROL_TCMI_BDI_TCMI_OCI(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMI_AUTO_BDI_INSERT_CONTROL_TCMI_BDI_TCMI_OCI     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMI_AUTO_BDI_INSERT_CONTROL_TCMI_BDI_TCMI_OCI(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * TCMi BDI TCMi LCK
 * If the TCMi BDI TCMi LCK bit is set, the BDI field of active TCMs is set
 * for outgoing frames if a Tx TCMi LCK condition is detected (accepted
 * TCMi STAT is 101) and the appropriate auto TCMi BDI bit is set. This bit
 * is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMI_AUTO_BDI_INSERT_CONTROL . TCMI_BDI_TCMI_LCK
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMI_AUTO_BDI_INSERT_CONTROL_TCMI_BDI_TCMI_LCK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMI_AUTO_BDI_INSERT_CONTROL_TCMI_BDI_TCMI_LCK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMI_AUTO_BDI_INSERT_CONTROL_TCMI_BDI_TCMI_LCK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * TCMi BDI TCMi TIM
 * If the TCMi BDI TCMi LCK bit is set, the BDI field of active TCMs is set
 * for outgoing frames if a Tx TCMi TIM condition is detected (accepted
 * TCMi TTI does not match expected TCMi TTI) and the appropriate auto TCMi
 * BDI bit is set. This bit is cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMI_AUTO_BDI_INSERT_CONTROL . TCMI_BDI_TCMI_TIM
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMI_AUTO_BDI_INSERT_CONTROL_TCMI_BDI_TCMI_TIM(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMI_AUTO_BDI_INSERT_CONTROL_TCMI_BDI_TCMI_TIM     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMI_AUTO_BDI_INSERT_CONTROL_TCMI_BDI_TCMI_TIM(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * IAE multiframe count
 * The IAE multiframe count bits are used to indicate the number of
 * multiframes IAE inserted into the appropriate TCMi STAT field when an
 * incoming receive IAE condition is detected. Each time an incoming IAE
 * condition is detected and IAE is inserted, an internal counter used to
 * determine the exit of IAE insertion is reset. This counter continues to
 * accumulate frames until the terminal count of the IAE multiframe count
 * is reached, resulting in the removal of TCMi IAE insertion. The IAE
 * multiframe count bits are set to a value of 2 as a default.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMI_AUTO_BDI_INSERT_CONTROL . IAE_MULTIFRAME_COUNT
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMI_AUTO_BDI_INSERT_CONTROL_IAE_MULTIFRAME_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMI_AUTO_BDI_INSERT_CONTROL_IAE_MULTIFRAME_COUNT     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMI_AUTO_BDI_INSERT_CONTROL_IAE_MULTIFRAME_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief XCO2 Rx OTN ODU TCMX insert TTI memory 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_TCMX_INSERT_TTI
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: XCO2_RX_OTN_ODU_TCMX_INSERT_TTI (??), 0-191
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_TTI(target,ri)  VTSS_IOREG(target,0xef1 + (ri))

/** 
 * \brief
 * TCMi TTI value
 * The TCMi TTI value bits contains the 64 byte trail trace identifier
 * (SAPI/DAPI/Operator Specific) message to be inserted into outgoing ODU
 * frames when the TCMi transparent bit is cleared and the insert TCMi TTI
 * bit of the Rx OTN ODU TCMi Insert Control register is set. The TCMi TTI
 * message is repetitively inserted for outgoing frames and is aligned to
 * multiframe boundaries repeating four times for each complete multiframe.
 * These bits are undefined as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_TTI . TCMI_TTI_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_TTI_TCMI_TTI_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_TTI_TCMI_TTI_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_INSERT_TTI_TCMI_TTI_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief XCO2 Rx OTN ODU TCMX value insert register 
 *
 * \details
 * Register: \a OTN_WRAPPER::XCO2_RX_OTN::XCO2_RX_OTN_ODU_TCMX_VALUE_INSERT
 *
 * @param target A \a ::vtss_target_OTN_WRAPPER_e target
 * @param ri Register: XCO2_RX_OTN_ODU_TCMX_VALUE_INSERT (??), 0-5
 */
#define VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_VALUE_INSERT(target,ri)  VTSS_IOREG(target,0xfb1 + (ri))

/** 
 * \brief
 * TCMi BEI/BIAE value
 * The value programmed into the TCMi BEI/BIAE value bits are inserted into
 * the selected TCMi BEI/BIAE bit positions of outgoing frames when the
 * TCMi transparent bit is cleared and the insert TCMi BEI/BIAE bit of the
 * Rx OTN ODU TCMi Insert Control register is set. The TCMi BEI/BIAE value
 * bits are cleared as a default value.      
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_VALUE_INSERT . TCMI_BEIBIAE_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_VALUE_INSERT_TCMI_BEIBIAE_VALUE(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_VALUE_INSERT_TCMI_BEIBIAE_VALUE     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_VALUE_INSERT_TCMI_BEIBIAE_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,4,4)

/** 
 * \brief
 * TCMi BDI value
 * The value programmed into the TCMi BDI value bit is inserted into the
 * selected TCMi BDI bit position of outgoing frames when the TCMi
 * transparent bit is cleared and the insert TCMi BDI bit of the Rx OTN ODU
 * TCMi Insert Control register is set. The TCMi BDI value bit is cleared
 * as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_VALUE_INSERT . TCMI_BDI_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_VALUE_INSERT_TCMI_BDI_VALUE(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_VALUE_INSERT_TCMI_BDI_VALUE     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_VALUE_INSERT_TCMI_BDI_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * TCMi STAT value
 * The value programmed into the TCMi STAT value bits are inserted into the
 * selected TCMi STAT bit positions of outgoing frames when the TCMi
 * transparent bit is cleared and the insert TCMi STAT bit of the Rx OTN
 * ODU TCMi Insert Control register is set. The TCMi STAT value bits are
 * cleared as a default value.
 *
 * \details 
 * Field: ::VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_VALUE_INSERT . TCMI_STAT_VALUE
 */
#define  VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_VALUE_INSERT_TCMI_STAT_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_VALUE_INSERT_TCMI_STAT_VALUE     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_VALUE_INSERT_TCMI_STAT_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


#endif /* _VTSS_DAYTONA_REGS_OTN_WRAPPER_H_ */
