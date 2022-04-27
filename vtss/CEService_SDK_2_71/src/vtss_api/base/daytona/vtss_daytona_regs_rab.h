#ifndef _VTSS_DAYTONA_REGS_RAB_H_
#define _VTSS_DAYTONA_REGS_RAB_H_

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
 * Target: \a RAB
 *
 * \see vtss_target_RAB_e
 *
 * RAB configuration and status registers
 *
 ***********************************************************************/

/**
 * Register Group: \a RAB::RAB_BASE
 *
 * RAB base registers
 */


/** 
 * \brief RAB control
 *
 * \details
 * Register: \a RAB::RAB_BASE::RAB_CONTROL
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RAB_BASE_RAB_CONTROL(target)  VTSS_IOREG(target,0x0)

/** 
 * \brief
 * RAB Rx bypass select
 * Selects the input interface data used for bypassing the RAB Rx FIFO. The
 * GFP2 RXxXGMII interface should be selected only when the
 * CLIENT_MODE_SELECT in the GFP2 mapper is configured to 10 Gigabit
 * Ethernet XGMII.
 *
 * \details 
 * 0: MAC2 Rx XGMII interface
 * 1: GFP2 Rx XGMII interface
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_CONTROL . RX_BYPASS_SELECT
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_CONTROL_RX_BYPASS_SELECT(x)  VTSS_ENCODE_BITFIELD(x,18,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_CONTROL_RX_BYPASS_SELECT     VTSS_ENCODE_BITMASK(18,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_CONTROL_RX_BYPASS_SELECT(x)  VTSS_EXTRACT_BITFIELD(x,18,1)

/** 
 * \brief
 * RAB Rx software reset
 *
 * \details 
 * 0: Rx logic operates normally
 * 1: Rx logic other than CSR target is held in reset, clocks are NOT gated 
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_CONTROL . RX_SW_RST
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_CONTROL_RX_SW_RST(x)  VTSS_ENCODE_BITFIELD(x,17,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_CONTROL_RX_SW_RST     VTSS_ENCODE_BITMASK(17,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_CONTROL_RX_SW_RST(x)  VTSS_EXTRACT_BITFIELD(x,17,1)

/** 
 * \brief
 * RAB Rx clock enable
 *
 * \details 
 * 0: Rx logic clocks (except csr_clk) are disabled, Rx logic is held in
 * reset.
 * 1: Rx logic clocks are enabled.
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_CONTROL . RX_CLK_ENA
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_CONTROL_RX_CLK_ENA(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_CONTROL_RX_CLK_ENA     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_CONTROL_RX_CLK_ENA(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * RAB Tx bypass select
 * Selects the input interface data used for bypassing the RAB Tx FIFO. The
 * GFP1 Rx XGMII interface should be selected only when the
 * CLIENT_MODE_SELECT in the GFP1 mapper is configured to 10 Gigabit
 * Ethernet XGMII.
 *
 * \details 
 * 0: MAC1 Rx XGMII interface
 * 1: GFP1 Rx XGMII interface
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_CONTROL . TX_BYPASS_SELECT
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_CONTROL_TX_BYPASS_SELECT(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_CONTROL_TX_BYPASS_SELECT     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_CONTROL_TX_BYPASS_SELECT(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * RAB Tx software reset
 *
 * \details 
 * 0: Tx logic operates normally.
 * 1: Tx logic other than CSR target is held in reset, clocks are NOT
 * gated.	
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_CONTROL . TX_SW_RST
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_CONTROL_TX_SW_RST(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_CONTROL_TX_SW_RST     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_CONTROL_TX_SW_RST(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * RAB Tx clock enable
 *
 * \details 
 * 0: Tx logic clocks (except csr_clk) are disabled, Tx logic is held in
 * reset.
 * 1: Tx logic clocks are enabled. 
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_CONTROL . TX_CLK_ENA
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_CONTROL_TX_CLK_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_CONTROL_TX_CLK_ENA     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_CONTROL_TX_CLK_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief RAB interrupt sticky bits
 *
 * \details
 * A sticky bit is cleared by writing a 1 to its position. Bit(s) read as 1
 * indicate that the event(s) occurred.   
 *
 * Register: \a RAB::RAB_BASE::RAB_INTERRUPT
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RAB_BASE_RAB_INTERRUPT(target)  VTSS_IOREG(target,0x1)

/** 
 * \brief
 * Rx extract buffer memory error (multi-bit) detected upon read access
 * sticky bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . RX_EXTRACT_MERR_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_RX_EXTRACT_MERR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,30,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_RX_EXTRACT_MERR_STICKY     VTSS_ENCODE_BITMASK(30,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_RX_EXTRACT_MERR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,30,1)

/** 
 * \brief
 * Rx extract buffer memory single-bit correction detected upon read access
 * sticky bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . RX_EXTRACT_SBC_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_RX_EXTRACT_SBC_STICKY(x)  VTSS_ENCODE_BITFIELD(x,29,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_RX_EXTRACT_SBC_STICKY     VTSS_ENCODE_BITMASK(29,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_RX_EXTRACT_SBC_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,29,1)

/** 
 * \brief
 * Rx extract buffer packet captured sticky bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . RX_EXTRACT_PKT_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_RX_EXTRACT_PKT_STICKY(x)  VTSS_ENCODE_BITFIELD(x,28,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_RX_EXTRACT_PKT_STICKY     VTSS_ENCODE_BITMASK(28,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_RX_EXTRACT_PKT_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,28,1)

/** 
 * \brief
 * Rx insert buffer underflow detected sticky bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . RX_INSERT_UFLW_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_RX_INSERT_UFLW_STICKY(x)  VTSS_ENCODE_BITFIELD(x,27,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_RX_INSERT_UFLW_STICKY     VTSS_ENCODE_BITMASK(27,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_RX_INSERT_UFLW_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,27,1)

/** 
 * \brief
 * Rx insert buffer packet sent sticky bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . RX_INSERT_PKT_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_RX_INSERT_PKT_STICKY(x)  VTSS_ENCODE_BITFIELD(x,26,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_RX_INSERT_PKT_STICKY     VTSS_ENCODE_BITMASK(26,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_RX_INSERT_PKT_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,26,1)

/** 
 * \brief
 * Rx synchronization buffer memory error (multi-bit) detected upon read
 * access sticky bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . RX_SYNC_MERR_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_RX_SYNC_MERR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,25,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_RX_SYNC_MERR_STICKY     VTSS_ENCODE_BITMASK(25,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_RX_SYNC_MERR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,25,1)

/** 
 * \brief
 * Rx synchronization buffer memory single-bit correction detected upon
 * read access sticky bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . RX_SYNC_SBC_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_RX_SYNC_SBC_STICKY(x)  VTSS_ENCODE_BITFIELD(x,24,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_RX_SYNC_SBC_STICKY     VTSS_ENCODE_BITMASK(24,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_RX_SYNC_SBC_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,24,1)

/** 
 * \brief
 * Rx synchronization buffer underflow detected sticky bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . RX_SYNC_UFLW_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_RX_SYNC_UFLW_STICKY(x)  VTSS_ENCODE_BITFIELD(x,23,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_RX_SYNC_UFLW_STICKY     VTSS_ENCODE_BITMASK(23,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_RX_SYNC_UFLW_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,23,1)

/** 
 * \brief
 * Rx synchronization buffer overflow detected sticky bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . RX_SYNC_OFLW_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_RX_SYNC_OFLW_STICKY(x)  VTSS_ENCODE_BITFIELD(x,22,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_RX_SYNC_OFLW_STICKY     VTSS_ENCODE_BITMASK(22,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_RX_SYNC_OFLW_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,22,1)

/** 
 * \brief
 * Rx XGMII PLI memory error (multi-bit) detected upon read access sticky
 * bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . RX_XPLI_MERR_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_RX_XPLI_MERR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,21,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_RX_XPLI_MERR_STICKY     VTSS_ENCODE_BITMASK(21,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_RX_XPLI_MERR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,21,1)

/** 
 * \brief
 * Rx XGMII PLI memory single-bit correction detected upon read access
 * sticky bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . RX_XPLI_SBC_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_RX_XPLI_SBC_STICKY(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_RX_XPLI_SBC_STICKY     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_RX_XPLI_SBC_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \brief
 * Rx FIFO memory error (multi-bit) detected upon read access sticky bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . RX_FIFO_MERR_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_RX_FIFO_MERR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,19,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_RX_FIFO_MERR_STICKY     VTSS_ENCODE_BITMASK(19,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_RX_FIFO_MERR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,19,1)

/** 
 * \brief
 * Rx FIFO memory single-bit correction detected upon read access sticky
 * bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . RX_FIFO_SBC_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_RX_FIFO_SBC_STICKY(x)  VTSS_ENCODE_BITFIELD(x,18,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_RX_FIFO_SBC_STICKY     VTSS_ENCODE_BITMASK(18,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_RX_FIFO_SBC_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,18,1)

/** 
 * \brief
 * Rx FIFO underflow detected sticky bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . RX_FIFO_UFLW_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_RX_FIFO_UFLW_STICKY(x)  VTSS_ENCODE_BITFIELD(x,17,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_RX_FIFO_UFLW_STICKY     VTSS_ENCODE_BITMASK(17,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_RX_FIFO_UFLW_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,17,1)

/** 
 * \brief
 * Rx FIFO overflow detected sticky bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . RX_FIFO_OFLW_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_RX_FIFO_OFLW_STICKY(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_RX_FIFO_OFLW_STICKY     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_RX_FIFO_OFLW_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Tx extract buffer memory error (multi-bit) detected upon read access
 * sticky bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . TX_EXTRACT_MERR_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_TX_EXTRACT_MERR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_TX_EXTRACT_MERR_STICKY     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_TX_EXTRACT_MERR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Tx extract buffer memory single-bit correction detected upon read access
 * sticky bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . TX_EXTRACT_SBC_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_TX_EXTRACT_SBC_STICKY(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_TX_EXTRACT_SBC_STICKY     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_TX_EXTRACT_SBC_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Tx extract buffer packet captured sticky bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . TX_EXTRACT_PKT_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_TX_EXTRACT_PKT_STICKY(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_TX_EXTRACT_PKT_STICKY     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_TX_EXTRACT_PKT_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Tx insert buffer underflow detected sticky bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . TX_INSERT_UFLW_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_TX_INSERT_UFLW_STICKY(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_TX_INSERT_UFLW_STICKY     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_TX_INSERT_UFLW_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Tx insert buffer packet sent sticky bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . TX_INSERT_PKT_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_TX_INSERT_PKT_STICKY(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_TX_INSERT_PKT_STICKY     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_TX_INSERT_PKT_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Tx synchronization buffer memory error (multi-bit) detected upon read
 * access sticky bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . TX_SYNC_MERR_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_TX_SYNC_MERR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_TX_SYNC_MERR_STICKY     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_TX_SYNC_MERR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Tx synchronization buffer memory single-bit correction detected upon
 * read access sticky bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . TX_SYNC_SBC_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_TX_SYNC_SBC_STICKY(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_TX_SYNC_SBC_STICKY     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_TX_SYNC_SBC_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Tx synchronization buffer underflow detected sticky bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . TX_SYNC_UFLW_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_TX_SYNC_UFLW_STICKY(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_TX_SYNC_UFLW_STICKY     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_TX_SYNC_UFLW_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Tx synchronization buffer overflow detected sticky bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . TX_SYNC_OFLW_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_TX_SYNC_OFLW_STICKY(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_TX_SYNC_OFLW_STICKY     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_TX_SYNC_OFLW_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Tx XGMII PLI memory error (multi-bit) detected upon read access sticky
 * bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . TX_XPLI_MERR_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_TX_XPLI_MERR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_TX_XPLI_MERR_STICKY     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_TX_XPLI_MERR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Tx XGMII PLI memory single-bit correction detected upon read access
 * sticky bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . TX_XPLI_SBC_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_TX_XPLI_SBC_STICKY(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_TX_XPLI_SBC_STICKY     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_TX_XPLI_SBC_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Tx FIFO memory error (multi-bit) detected upon read access sticky bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . TX_FIFO_MERR_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_TX_FIFO_MERR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_TX_FIFO_MERR_STICKY     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_TX_FIFO_MERR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Tx FIFO memory single-bit correction detected upon read access sticky
 * bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . TX_FIFO_SBC_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_TX_FIFO_SBC_STICKY(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_TX_FIFO_SBC_STICKY     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_TX_FIFO_SBC_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Tx FIFO underflow detected sticky bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . TX_FIFO_UFLW_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_TX_FIFO_UFLW_STICKY(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_TX_FIFO_UFLW_STICKY     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_TX_FIFO_UFLW_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Tx FIFO overflow detected sticky bit
 *
 * \details 
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT . TX_FIFO_OFLW_STICKY
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_TX_FIFO_OFLW_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_TX_FIFO_OFLW_STICKY     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_TX_FIFO_OFLW_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief RAB interrupt mask bits
 *
 * \details
 * Register: \a RAB::RAB_BASE::RAB_INTERRUPT_MASK
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK(target)  VTSS_IOREG(target,0x2)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . RX_EXTRACT_MERR_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_EXTRACT_MERR_MASK(x)  VTSS_ENCODE_BITFIELD(x,30,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_EXTRACT_MERR_MASK     VTSS_ENCODE_BITMASK(30,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_EXTRACT_MERR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,30,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . RX_EXTRACT_SBC_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_EXTRACT_SBC_MASK(x)  VTSS_ENCODE_BITFIELD(x,29,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_EXTRACT_SBC_MASK     VTSS_ENCODE_BITMASK(29,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_EXTRACT_SBC_MASK(x)  VTSS_EXTRACT_BITFIELD(x,29,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . RX_EXTRACT_PKT_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_EXTRACT_PKT_MASK(x)  VTSS_ENCODE_BITFIELD(x,28,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_EXTRACT_PKT_MASK     VTSS_ENCODE_BITMASK(28,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_EXTRACT_PKT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,28,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . RX_INSERT_UFLW_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_INSERT_UFLW_MASK(x)  VTSS_ENCODE_BITFIELD(x,27,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_INSERT_UFLW_MASK     VTSS_ENCODE_BITMASK(27,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_INSERT_UFLW_MASK(x)  VTSS_EXTRACT_BITFIELD(x,27,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . RX_INSERT_PKT_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_INSERT_PKT_MASK(x)  VTSS_ENCODE_BITFIELD(x,26,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_INSERT_PKT_MASK     VTSS_ENCODE_BITMASK(26,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_INSERT_PKT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,26,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . RX_SYNC_MERR_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_SYNC_MERR_MASK(x)  VTSS_ENCODE_BITFIELD(x,25,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_SYNC_MERR_MASK     VTSS_ENCODE_BITMASK(25,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_SYNC_MERR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,25,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . RX_SYNC_SBC_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_SYNC_SBC_MASK(x)  VTSS_ENCODE_BITFIELD(x,24,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_SYNC_SBC_MASK     VTSS_ENCODE_BITMASK(24,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_SYNC_SBC_MASK(x)  VTSS_EXTRACT_BITFIELD(x,24,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . RX_SYNC_UFLW_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_SYNC_UFLW_MASK(x)  VTSS_ENCODE_BITFIELD(x,23,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_SYNC_UFLW_MASK     VTSS_ENCODE_BITMASK(23,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_SYNC_UFLW_MASK(x)  VTSS_EXTRACT_BITFIELD(x,23,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . RX_SYNC_OFLW_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_SYNC_OFLW_MASK(x)  VTSS_ENCODE_BITFIELD(x,22,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_SYNC_OFLW_MASK     VTSS_ENCODE_BITMASK(22,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_SYNC_OFLW_MASK(x)  VTSS_EXTRACT_BITFIELD(x,22,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . RX_XPLI_MERR_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_XPLI_MERR_MASK(x)  VTSS_ENCODE_BITFIELD(x,21,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_XPLI_MERR_MASK     VTSS_ENCODE_BITMASK(21,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_XPLI_MERR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,21,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . RX_XPLI_SBC_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_XPLI_SBC_MASK(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_XPLI_SBC_MASK     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_XPLI_SBC_MASK(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . RX_FIFO_MERR_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_FIFO_MERR_MASK(x)  VTSS_ENCODE_BITFIELD(x,19,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_FIFO_MERR_MASK     VTSS_ENCODE_BITMASK(19,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_FIFO_MERR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,19,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . RX_FIFO_SBC_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_FIFO_SBC_MASK(x)  VTSS_ENCODE_BITFIELD(x,18,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_FIFO_SBC_MASK     VTSS_ENCODE_BITMASK(18,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_FIFO_SBC_MASK(x)  VTSS_EXTRACT_BITFIELD(x,18,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . RX_FIFO_UFLW_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_FIFO_UFLW_MASK(x)  VTSS_ENCODE_BITFIELD(x,17,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_FIFO_UFLW_MASK     VTSS_ENCODE_BITMASK(17,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_FIFO_UFLW_MASK(x)  VTSS_EXTRACT_BITFIELD(x,17,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . RX_FIFO_OFLW_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_FIFO_OFLW_MASK(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_FIFO_OFLW_MASK     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_RX_FIFO_OFLW_MASK(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . TX_EXTRACT_MERR_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_EXTRACT_MERR_MASK(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_EXTRACT_MERR_MASK     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_EXTRACT_MERR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . TX_EXTRACT_SBC_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_EXTRACT_SBC_MASK(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_EXTRACT_SBC_MASK     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_EXTRACT_SBC_MASK(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . TX_EXTRACT_PKT_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_EXTRACT_PKT_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_EXTRACT_PKT_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_EXTRACT_PKT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . TX_INSERT_UFLW_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_INSERT_UFLW_MASK(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_INSERT_UFLW_MASK     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_INSERT_UFLW_MASK(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . TX_INSERT_PKT_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_INSERT_PKT_MASK(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_INSERT_PKT_MASK     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_INSERT_PKT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . TX_SYNC_MERR_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_SYNC_MERR_MASK(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_SYNC_MERR_MASK     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_SYNC_MERR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . TX_SYNC_SBC_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_SYNC_SBC_MASK(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_SYNC_SBC_MASK     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_SYNC_SBC_MASK(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . TX_SYNC_UFLW_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_SYNC_UFLW_MASK(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_SYNC_UFLW_MASK     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_SYNC_UFLW_MASK(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . TX_SYNC_OFLW_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_SYNC_OFLW_MASK(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_SYNC_OFLW_MASK     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_SYNC_OFLW_MASK(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . TX_XPLI_MERR_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_XPLI_MERR_MASK(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_XPLI_MERR_MASK     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_XPLI_MERR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . TX_XPLI_SBC_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_XPLI_SBC_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_XPLI_SBC_MASK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_XPLI_SBC_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . TX_FIFO_MERR_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_FIFO_MERR_MASK(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_FIFO_MERR_MASK     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_FIFO_MERR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . TX_FIFO_SBC_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_FIFO_SBC_MASK(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_FIFO_SBC_MASK     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_FIFO_SBC_MASK(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . TX_FIFO_UFLW_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_FIFO_UFLW_MASK(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_FIFO_UFLW_MASK     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_FIFO_UFLW_MASK(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the sticky bit to be an interrupt source
 * 1: Enables the sticky bit to be an interrupt source
 *
 * Field: ::VTSS_RAB_RAB_BASE_RAB_INTERRUPT_MASK . TX_FIFO_OFLW_MASK
 */
#define  VTSS_F_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_FIFO_OFLW_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_FIFO_OFLW_MASK     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_RAB_RAB_BASE_RAB_INTERRUPT_MASK_TX_FIFO_OFLW_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/**
 * Register Group: \a RAB::RX_FIFO
 *
 * RAB Rx FIFO registers
 */


/** 
 * \brief RAB Rx FIFO control register
 *
 * \details
 * Register: \a RAB::RX_FIFO::RX_FIFO_CONTROL
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_FIFO_RX_FIFO_CONTROL(target)  VTSS_IOREG(target,0x100)

/** 
 * \brief
 * Rx FIFO cut-through threshold.
 * Threshold value is in units of 8 byte words.    
 *
 * \details 
 * Field: ::VTSS_RAB_RX_FIFO_RX_FIFO_CONTROL . RX_FIFO_CUT_THRU_THRESH
 */
#define  VTSS_F_RAB_RX_FIFO_RX_FIFO_CONTROL_RX_FIFO_CUT_THRU_THRESH(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_RAB_RX_FIFO_RX_FIFO_CONTROL_RX_FIFO_CUT_THRU_THRESH     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_RAB_RX_FIFO_RX_FIFO_CONTROL_RX_FIFO_CUT_THRU_THRESH(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Rx FIFO cut-through mode.
 * Note: when RX_FIFO_OUTPUT_SELECT = 1, RX_FIFO_CUT_THRU has no effect and
 * the Rx FIFO will operate in store-and-forward.
 *
 * \details 
 * 0: Disable cut-through operation
 * 1: Enable cut-through operation
 *
 * Field: ::VTSS_RAB_RX_FIFO_RX_FIFO_CONTROL . RX_FIFO_CUT_THRU
 */
#define  VTSS_F_RAB_RX_FIFO_RX_FIFO_CONTROL_RX_FIFO_CUT_THRU(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_RAB_RX_FIFO_RX_FIFO_CONTROL_RX_FIFO_CUT_THRU     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_RAB_RX_FIFO_RX_FIFO_CONTROL_RX_FIFO_CUT_THRU(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Disable discarding of frames that arrive to the Rx FIFO marked with
 * abort.
 *
 * \details 
 * 0: Enable discarding of aborted frames
 * 1: Disable discarding of aborted frames
 *
 * Field: ::VTSS_RAB_RX_FIFO_RX_FIFO_CONTROL . RX_FIFO_DROP_DISABLE
 */
#define  VTSS_F_RAB_RX_FIFO_RX_FIFO_CONTROL_RX_FIFO_DROP_DISABLE(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_RAB_RX_FIFO_RX_FIFO_CONTROL_RX_FIFO_DROP_DISABLE     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_RAB_RX_FIFO_RX_FIFO_CONTROL_RX_FIFO_DROP_DISABLE(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Select input interface into Rx FIFO.
 *
 * \details 
 * 0: MAC2 Rx host interface
 * 1: MAC2 Rx XGMII interface
 * 2: GFP2 Rx client interface
 * 3: Reserved
 *
 * Field: ::VTSS_RAB_RX_FIFO_RX_FIFO_CONTROL . RX_FIFO_INPUT_SELECT
 */
#define  VTSS_F_RAB_RX_FIFO_RX_FIFO_CONTROL_RX_FIFO_INPUT_SELECT(x)  VTSS_ENCODE_BITFIELD(x,1,2)
#define  VTSS_M_RAB_RX_FIFO_RX_FIFO_CONTROL_RX_FIFO_INPUT_SELECT     VTSS_ENCODE_BITMASK(1,2)
#define  VTSS_X_RAB_RX_FIFO_RX_FIFO_CONTROL_RX_FIFO_INPUT_SELECT(x)  VTSS_EXTRACT_BITFIELD(x,1,2)

/** 
 * \brief
 * Select output interface from Rx FIFO.
 *
 * \details 
 * 0: MAC1 Tx host interface
 * 1: GFP1 Tx client interface
 *
 * Field: ::VTSS_RAB_RX_FIFO_RX_FIFO_CONTROL . RX_FIFO_OUTPUT_SELECT
 */
#define  VTSS_F_RAB_RX_FIFO_RX_FIFO_CONTROL_RX_FIFO_OUTPUT_SELECT(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_RAB_RX_FIFO_RX_FIFO_CONTROL_RX_FIFO_OUTPUT_SELECT     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_RAB_RX_FIFO_RX_FIFO_CONTROL_RX_FIFO_OUTPUT_SELECT(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief RAB Rx FIFO high threshold
 *
 * \details
 * Register: \a RAB::RX_FIFO::RX_FIFO_HIGH_THRESH
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_FIFO_RX_FIFO_HIGH_THRESH(target)  VTSS_IOREG(target,0x101)

/** 
 * \brief
 * High threshold for generation of PAUSE flow control on MAC2 Tx.
 * Threshold value is in units of 8 byte words.
 *
 * \details 
 * Field: ::VTSS_RAB_RX_FIFO_RX_FIFO_HIGH_THRESH . RX_FIFO_HIGH_THRESH
 */
#define  VTSS_F_RAB_RX_FIFO_RX_FIFO_HIGH_THRESH_RX_FIFO_HIGH_THRESH(x)  VTSS_ENCODE_BITFIELD(x,0,13)
#define  VTSS_M_RAB_RX_FIFO_RX_FIFO_HIGH_THRESH_RX_FIFO_HIGH_THRESH     VTSS_ENCODE_BITMASK(0,13)
#define  VTSS_X_RAB_RX_FIFO_RX_FIFO_HIGH_THRESH_RX_FIFO_HIGH_THRESH(x)  VTSS_EXTRACT_BITFIELD(x,0,13)


/** 
 * \brief RAB Rx FIFO low threshold
 *
 * \details
 * Register: \a RAB::RX_FIFO::RX_FIFO_LOW_THRESH
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_FIFO_RX_FIFO_LOW_THRESH(target)  VTSS_IOREG(target,0x102)

/** 
 * \brief
 * Low threshold for generation of PAUSE flow control on MAC2 Tx. Threshold
 * value is in units of 8 byte words.
 *
 * \details 
 * Field: ::VTSS_RAB_RX_FIFO_RX_FIFO_LOW_THRESH . RX_FIFO_LOW_THRESH
 */
#define  VTSS_F_RAB_RX_FIFO_RX_FIFO_LOW_THRESH_RX_FIFO_LOW_THRESH(x)  VTSS_ENCODE_BITFIELD(x,0,13)
#define  VTSS_M_RAB_RX_FIFO_RX_FIFO_LOW_THRESH_RX_FIFO_LOW_THRESH     VTSS_ENCODE_BITMASK(0,13)
#define  VTSS_X_RAB_RX_FIFO_RX_FIFO_LOW_THRESH_RX_FIFO_LOW_THRESH(x)  VTSS_EXTRACT_BITFIELD(x,0,13)


/** 
 * \brief RAB Rx FIFO adaptation threshold
 *
 * \details
 * Register: \a RAB::RX_FIFO::RX_FIFO_ADAPT_THRESH
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_FIFO_RX_FIFO_ADAPT_THRESH(target)  VTSS_IOREG(target,0x103)

/** 
 * \brief
 * Threshold for assertion of rate adaptation request to MAC1 Tx. Threshold
 * value is in units of 8 byte words.
 *
 * \details 
 * Field: ::VTSS_RAB_RX_FIFO_RX_FIFO_ADAPT_THRESH . RX_FIFO_ADAPT_THRESH
 */
#define  VTSS_F_RAB_RX_FIFO_RX_FIFO_ADAPT_THRESH_RX_FIFO_ADAPT_THRESH(x)  VTSS_ENCODE_BITFIELD(x,0,13)
#define  VTSS_M_RAB_RX_FIFO_RX_FIFO_ADAPT_THRESH_RX_FIFO_ADAPT_THRESH     VTSS_ENCODE_BITMASK(0,13)
#define  VTSS_X_RAB_RX_FIFO_RX_FIFO_ADAPT_THRESH_RX_FIFO_ADAPT_THRESH(x)  VTSS_EXTRACT_BITFIELD(x,0,13)


/** 
 * \brief RAB Rx FIFO packet drop count
 *
 * \details
 * Register: \a RAB::RX_FIFO::RX_PKT_DROP_CNT
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_FIFO_RX_PKT_DROP_CNT(target)  VTSS_IOREG(target,0x104)

/** 
 * \brief
 * Counts number of packets discarded in the Rx FIFO due to Rx FIFO
 * overflow
 *
 * \details 
 * Field: ::VTSS_RAB_RX_FIFO_RX_PKT_DROP_CNT . RX_PKT_DROP_CNT
 */
#define  VTSS_F_RAB_RX_FIFO_RX_PKT_DROP_CNT_RX_PKT_DROP_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_RAB_RX_FIFO_RX_PKT_DROP_CNT_RX_PKT_DROP_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_RAB_RX_FIFO_RX_PKT_DROP_CNT_RX_PKT_DROP_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief RAB Rx FIFO packet drop count performance monitor
 *
 * \details
 * Register: \a RAB::RX_FIFO::RX_PKT_DROP_CNT_PERF
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_FIFO_RX_PKT_DROP_CNT_PERF(target)  VTSS_IOREG(target,0x105)

/** 
 * \brief
 * Counts number of packets discarded in the Rx FIFO due to Rx FIFO
 * overflow
 *
 * \details 
 * Field: ::VTSS_RAB_RX_FIFO_RX_PKT_DROP_CNT_PERF . RX_PKT_DROP_CNT_PERF
 */
#define  VTSS_F_RAB_RX_FIFO_RX_PKT_DROP_CNT_PERF_RX_PKT_DROP_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_RAB_RX_FIFO_RX_PKT_DROP_CNT_PERF_RX_PKT_DROP_CNT_PERF     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_RAB_RX_FIFO_RX_PKT_DROP_CNT_PERF_RX_PKT_DROP_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief RAB Rx FIFO packets pending count
 *
 * \details
 * Register: \a RAB::RX_FIFO::RX_FIFO_PKT_PEND_CNT
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_FIFO_RX_FIFO_PKT_PEND_CNT(target)  VTSS_IOREG(target,0x106)

/** 
 * \brief
 * The number of packets currently queued in the Rx FIFO
 *
 * \details 
 * Field: ::VTSS_RAB_RX_FIFO_RX_FIFO_PKT_PEND_CNT . RX_FIFO_PKT_PEND_CNT
 */
#define  VTSS_F_RAB_RX_FIFO_RX_FIFO_PKT_PEND_CNT_RX_FIFO_PKT_PEND_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_RAB_RX_FIFO_RX_FIFO_PKT_PEND_CNT_RX_FIFO_PKT_PEND_CNT     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_RAB_RX_FIFO_RX_FIFO_PKT_PEND_CNT_RX_FIFO_PKT_PEND_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \brief RAB Rx FIFO high level status
 *
 * \details
 * Register: \a RAB::RX_FIFO::RX_FIFO_HIGH_STATUS
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_FIFO_RX_FIFO_HIGH_STATUS(target)  VTSS_IOREG(target,0x107)

/** 
 * \brief
 * Reset the RX_HIGH_LEVEL_STICKY to its default. This bit will self-clear
 * when the reset has completed.   
 *
 * \details 
 * 1: Reset high level status
 *
 * Field: ::VTSS_RAB_RX_FIFO_RX_FIFO_HIGH_STATUS . RX_HIGH_LEVEL_RST
 */
#define  VTSS_F_RAB_RX_FIFO_RX_FIFO_HIGH_STATUS_RX_HIGH_LEVEL_RST(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_RAB_RX_FIFO_RX_FIFO_HIGH_STATUS_RX_HIGH_LEVEL_RST     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_RAB_RX_FIFO_RX_FIFO_HIGH_STATUS_RX_HIGH_LEVEL_RST(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Latches the highest value of fill level observed in the Rx FIFO. Fill
 * level value is in units of 8 byte words.
 *
 * \details 
 * Field: ::VTSS_RAB_RX_FIFO_RX_FIFO_HIGH_STATUS . RX_HIGH_LEVEL_STICKY
 */
#define  VTSS_F_RAB_RX_FIFO_RX_FIFO_HIGH_STATUS_RX_HIGH_LEVEL_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,13)
#define  VTSS_M_RAB_RX_FIFO_RX_FIFO_HIGH_STATUS_RX_HIGH_LEVEL_STICKY     VTSS_ENCODE_BITMASK(0,13)
#define  VTSS_X_RAB_RX_FIFO_RX_FIFO_HIGH_STATUS_RX_HIGH_LEVEL_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,13)


/** 
 * \brief RAB Rx FIFO low level status
 *
 * \details
 * Register: \a RAB::RX_FIFO::RX_FIFO_LOW_STATUS
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_FIFO_RX_FIFO_LOW_STATUS(target)  VTSS_IOREG(target,0x108)

/** 
 * \brief
 * Reset the RX_LOW_LEVEL_STICKY to its default. This bit will self-clear
 * when the reset has completed.
 *
 * \details 
 * 1: Reset low level status
 *
 * Field: ::VTSS_RAB_RX_FIFO_RX_FIFO_LOW_STATUS . RX_LOW_LEVEL_RST
 */
#define  VTSS_F_RAB_RX_FIFO_RX_FIFO_LOW_STATUS_RX_LOW_LEVEL_RST(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_RAB_RX_FIFO_RX_FIFO_LOW_STATUS_RX_LOW_LEVEL_RST     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_RAB_RX_FIFO_RX_FIFO_LOW_STATUS_RX_LOW_LEVEL_RST(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Latches the lowest value of fill level observed in the Rx FIFO. Fill
 * level value is in units of 8 byte words.
 *
 * \details 
 * Field: ::VTSS_RAB_RX_FIFO_RX_FIFO_LOW_STATUS . RX_LOW_LEVEL_STICKY
 */
#define  VTSS_F_RAB_RX_FIFO_RX_FIFO_LOW_STATUS_RX_LOW_LEVEL_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,13)
#define  VTSS_M_RAB_RX_FIFO_RX_FIFO_LOW_STATUS_RX_LOW_LEVEL_STICKY     VTSS_ENCODE_BITMASK(0,13)
#define  VTSS_X_RAB_RX_FIFO_RX_FIFO_LOW_STATUS_RX_LOW_LEVEL_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,13)


/** 
 * \brief RAB Rx sync buffer control
 *
 * \details
 * Register: \a RAB::RX_FIFO::RX_SYNC_CONTROL
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_FIFO_RX_SYNC_CONTROL(target)  VTSS_IOREG(target,0x109)

/** 
 * \brief
 * High threshold for generation of the synchronization buffer almost full
 * condition. Threshold value is in units of 8 byte words; legal values are
 * 0 to 31.	  
 *
 * \details 
 * Field: ::VTSS_RAB_RX_FIFO_RX_SYNC_CONTROL . RX_SYNC_AF_THRESH
 */
#define  VTSS_F_RAB_RX_FIFO_RX_SYNC_CONTROL_RX_SYNC_AF_THRESH(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_RAB_RX_FIFO_RX_SYNC_CONTROL_RX_SYNC_AF_THRESH     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_RAB_RX_FIFO_RX_SYNC_CONTROL_RX_SYNC_AF_THRESH(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Low threshold for generation of the synchronization buffer almost empty
 * condition. Threshold value is in units of 8 byte words; legal values are
 * 0 to 31.	  
 *
 * \details 
 * Field: ::VTSS_RAB_RX_FIFO_RX_SYNC_CONTROL . RX_SYNC_AE_THRESH
 */
#define  VTSS_F_RAB_RX_FIFO_RX_SYNC_CONTROL_RX_SYNC_AE_THRESH(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_RAB_RX_FIFO_RX_SYNC_CONTROL_RX_SYNC_AE_THRESH     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_RAB_RX_FIFO_RX_SYNC_CONTROL_RX_SYNC_AE_THRESH(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief RAB Rx FIFO freeze control
 *
 * \details
 * Register: \a RAB::RX_FIFO::RX_FIFO_FREEZE
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_FIFO_RX_FIFO_FREEZE(target)  VTSS_IOREG(target,0x10a)

/** 
 * \brief
 * Freeze Rx FIFO.
 * Immediately stops FIFO activity, including queueing or de-queueing of
 * partial frames.
 *
 * \details 
 * 0: Normal FIFO operation
 * 1: Freeze FIFO
 *
 * Field: ::VTSS_RAB_RX_FIFO_RX_FIFO_FREEZE . RX_FIFO_FREEZE
 */
#define  VTSS_F_RAB_RX_FIFO_RX_FIFO_FREEZE_RX_FIFO_FREEZE(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_RAB_RX_FIFO_RX_FIFO_FREEZE_RX_FIFO_FREEZE     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_RAB_RX_FIFO_RX_FIFO_FREEZE_RX_FIFO_FREEZE(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief RAB Rx FIFO flush control
 *
 * \details
 * Register: \a RAB::RX_FIFO::RX_FIFO_FLUSH
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_FIFO_RX_FIFO_FLUSH(target)  VTSS_IOREG(target,0x10b)

/** 
 * \brief
 * Rx FIFO flush request bit.
 * This bit will self-clear when the flush has completed. It has no effect
 * when RX_FIFO_FREEZE = 0.
 *
 * \details 
 * 1: Flush FIFO
 *
 * Field: ::VTSS_RAB_RX_FIFO_RX_FIFO_FLUSH . RX_FIFO_FLUSH
 */
#define  VTSS_F_RAB_RX_FIFO_RX_FIFO_FLUSH_RX_FIFO_FLUSH(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_RAB_RX_FIFO_RX_FIFO_FLUSH_RX_FIFO_FLUSH     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_RAB_RX_FIFO_RX_FIFO_FLUSH_RX_FIFO_FLUSH(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/**
 * Register Group: \a RAB::RX_INSERT
 *
 * RAB Rx packet insert registers
 */


/** 
 * \brief RAB Rx insert buffer control
 *
 * \details
 * Register: \a RAB::RX_INSERT::RX_INSERT_CONTROL
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_INSERT_RX_INSERT_CONTROL(target)  VTSS_IOREG(target,0x200)

/** 
 * \brief
 * Insert frame go request bit.
 * This bit will self-clear when the insert request has been serviced. If
 * auto-insertion is enabled by the value in RX_INSERT_INTERVAL, the frame
 * will continue to be inserted at the selected interval.
 *
 * \details 
 * 1: Insert frame
 *
 * Field: ::VTSS_RAB_RX_INSERT_RX_INSERT_CONTROL . RX_INSERT_GO
 */
#define  VTSS_F_RAB_RX_INSERT_RX_INSERT_CONTROL_RX_INSERT_GO(x)  VTSS_ENCODE_BITFIELD(x,31,1)
#define  VTSS_M_RAB_RX_INSERT_RX_INSERT_CONTROL_RX_INSERT_GO     VTSS_ENCODE_BITMASK(31,1)
#define  VTSS_X_RAB_RX_INSERT_RX_INSERT_CONTROL_RX_INSERT_GO(x)  VTSS_EXTRACT_BITFIELD(x,31,1)

/** 
 * \brief
 * Insert frame interval select
 *
 * \details 
 * 000: Frame auto-insert constantly
 * 001: Frame auto-insert every 200 ms
 * 010: Frame auto-insert every 400 ms
 * 011: Frame auto-insert every 600 ms
 * 100: Frame auto-insert every 800 ms
 * 101: Frame auto-insert every 1000 ms
 * 110: Frame auto-insert every 1200 ms
 * 111: Auto-insertion disabled
 *
 * Field: ::VTSS_RAB_RX_INSERT_RX_INSERT_CONTROL . RX_INSERT_INTERVAL
 */
#define  VTSS_F_RAB_RX_INSERT_RX_INSERT_CONTROL_RX_INSERT_INTERVAL(x)  VTSS_ENCODE_BITFIELD(x,24,3)
#define  VTSS_M_RAB_RX_INSERT_RX_INSERT_CONTROL_RX_INSERT_INTERVAL     VTSS_ENCODE_BITMASK(24,3)
#define  VTSS_X_RAB_RX_INSERT_RX_INSERT_CONTROL_RX_INSERT_INTERVAL(x)  VTSS_EXTRACT_BITFIELD(x,24,3)

/** 
 * \brief
 * Insert buffer address.
 * The address which will be written to (or read from) when writing (or
 * reading) the insert buffer data registers. Always access
 * RX_INSERT_DATA_LSB before RX_INSERT_DATA_MSB for a given address.
 * RX_INSERT_ADDR will auto-increment when RX_INSERT_DATA_MSB is accessed.
 * Note: always set RX_INSERT_ADDR to 0 before loading a frame in the
 * insert buffer.
 *
 * \details 
 * Field: ::VTSS_RAB_RX_INSERT_RX_INSERT_CONTROL . RX_INSERT_ADDR
 */
#define  VTSS_F_RAB_RX_INSERT_RX_INSERT_CONTROL_RX_INSERT_ADDR(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_RAB_RX_INSERT_RX_INSERT_CONTROL_RX_INSERT_ADDR     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_RAB_RX_INSERT_RX_INSERT_CONTROL_RX_INSERT_ADDR(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief RAB Rx insert buffer data LSB
 *
 * \details
 * Register: \a RAB::RX_INSERT::RX_INSERT_DATA_LSB
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_INSERT_RX_INSERT_DATA_LSB(target)  VTSS_IOREG(target,0x201)

/** 
 * \brief
 * Bits [31:0] of the insert buffer data
 *
 * \details 
 * Field: ::VTSS_RAB_RX_INSERT_RX_INSERT_DATA_LSB . RX_INSERT_DATA_LSB
 */
#define  VTSS_F_RAB_RX_INSERT_RX_INSERT_DATA_LSB_RX_INSERT_DATA_LSB(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_RAB_RX_INSERT_RX_INSERT_DATA_LSB_RX_INSERT_DATA_LSB     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_RAB_RX_INSERT_RX_INSERT_DATA_LSB_RX_INSERT_DATA_LSB(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief RAB Rx insert buffer data MSB
 *
 * \details
 * Register: \a RAB::RX_INSERT::RX_INSERT_DATA_MSB
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_INSERT_RX_INSERT_DATA_MSB(target)  VTSS_IOREG(target,0x202)

/** 
 * \brief
 * Bits [63:32] of the insert buffer data
 *
 * \details 
 * Field: ::VTSS_RAB_RX_INSERT_RX_INSERT_DATA_MSB . RX_INSERT_DATA_MSB
 */
#define  VTSS_F_RAB_RX_INSERT_RX_INSERT_DATA_MSB_RX_INSERT_DATA_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_RAB_RX_INSERT_RX_INSERT_DATA_MSB_RX_INSERT_DATA_MSB     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_RAB_RX_INSERT_RX_INSERT_DATA_MSB_RX_INSERT_DATA_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief RAB Rx insert packet preamble LSB
 *
 * \details
 * Register: \a RAB::RX_INSERT::RX_INSERT_PREAMBLE_LSB
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_INSERT_RX_INSERT_PREAMBLE_LSB(target)  VTSS_IOREG(target,0x203)

/** 
 * \brief
 * Bits [31:0] of the insert packet preamble + SFD
 *
 * \details 
 * Field: ::VTSS_RAB_RX_INSERT_RX_INSERT_PREAMBLE_LSB . RX_INSERT_PREAMBLE_LSB
 */
#define  VTSS_F_RAB_RX_INSERT_RX_INSERT_PREAMBLE_LSB_RX_INSERT_PREAMBLE_LSB(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_RAB_RX_INSERT_RX_INSERT_PREAMBLE_LSB_RX_INSERT_PREAMBLE_LSB     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_RAB_RX_INSERT_RX_INSERT_PREAMBLE_LSB_RX_INSERT_PREAMBLE_LSB(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief RAB Rx insert packet preamble MSB
 *
 * \details
 * Register: \a RAB::RX_INSERT::RX_INSERT_PREAMBLE_MSB
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_INSERT_RX_INSERT_PREAMBLE_MSB(target)  VTSS_IOREG(target,0x204)

/** 
 * \brief
 * Bits [63:32] of the insert packet preamble + SFD
 *
 * \details 
 * Field: ::VTSS_RAB_RX_INSERT_RX_INSERT_PREAMBLE_MSB . RX_INSERT_PREAMBLE_MSB
 */
#define  VTSS_F_RAB_RX_INSERT_RX_INSERT_PREAMBLE_MSB_RX_INSERT_PREAMBLE_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_RAB_RX_INSERT_RX_INSERT_PREAMBLE_MSB_RX_INSERT_PREAMBLE_MSB     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_RAB_RX_INSERT_RX_INSERT_PREAMBLE_MSB_RX_INSERT_PREAMBLE_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief RAB Rx insert packet count
 *
 * \details
 * Register: \a RAB::RX_INSERT::RX_INSERT_PKT_CNT
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_INSERT_RX_INSERT_PKT_CNT(target)  VTSS_IOREG(target,0x205)

/** 
 * \brief
 * Counts number of packets inserted
 *
 * \details 
 * Field: ::VTSS_RAB_RX_INSERT_RX_INSERT_PKT_CNT . RX_INSERT_PKT_CNT
 */
#define  VTSS_F_RAB_RX_INSERT_RX_INSERT_PKT_CNT_RX_INSERT_PKT_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_RAB_RX_INSERT_RX_INSERT_PKT_CNT_RX_INSERT_PKT_CNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_RAB_RX_INSERT_RX_INSERT_PKT_CNT_RX_INSERT_PKT_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief RAB Rx insert timer
 *
 * \details
 * Register: \a RAB::RX_INSERT::RX_INSERT_TIMER
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_INSERT_RX_INSERT_TIMER(target)  VTSS_IOREG(target,0x206)

/** 
 * \brief
 * Insert interval timer count.
 * The interval timer count is used to support multiple clock frequencies
 * to ensure accurate frame insertion intervals. The interval timer count
 * value specifies the number of clock cycles per 100 ms.
 *
 * \details 
 * Field: ::VTSS_RAB_RX_INSERT_RX_INSERT_TIMER . RX_INSERT_TIMER_COUNT
 */
#define  VTSS_F_RAB_RX_INSERT_RX_INSERT_TIMER_RX_INSERT_TIMER_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_RAB_RX_INSERT_RX_INSERT_TIMER_RX_INSERT_TIMER_COUNT     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_RAB_RX_INSERT_RX_INSERT_TIMER_RX_INSERT_TIMER_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,24)

/**
 * Register Group: \a RAB::RX_EXTRACT
 *
 * RAB Rx packet extract registers
 */


/** 
 * \brief RAB Rx extract filter 0 Ethertype
 *
 * \details
 * Register: \a RAB::RX_EXTRACT::RX_FILTER_0_ETYPE
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_EXTRACT_RX_FILTER_0_ETYPE(target)  VTSS_IOREG(target,0x300)

/** 
 * \brief
 * Enable for Rx extract filter 0
 *
 * \details 
 * 0: Disabled
 * 1: Enabled
 *
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_FILTER_0_ETYPE . RX_FILTER_0_ENABLE
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_FILTER_0_ETYPE_RX_FILTER_0_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_RAB_RX_EXTRACT_RX_FILTER_0_ETYPE_RX_FILTER_0_ENABLE     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_RAB_RX_EXTRACT_RX_FILTER_0_ETYPE_RX_FILTER_0_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Ethertype value to match for Rx extract filter 0
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_FILTER_0_ETYPE . RX_FILTER_0_ETYPE
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_FILTER_0_ETYPE_RX_FILTER_0_ETYPE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_RAB_RX_EXTRACT_RX_FILTER_0_ETYPE_RX_FILTER_0_ETYPE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_RAB_RX_EXTRACT_RX_FILTER_0_ETYPE_RX_FILTER_0_ETYPE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief RAB Rx extract filter 0 next two bytes
 *
 * \details
 * Register: \a RAB::RX_EXTRACT::RX_FILTER_0_N2B
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_EXTRACT_RX_FILTER_0_N2B(target)  VTSS_IOREG(target,0x301)

/** 
 * \brief
 * Next 2 bytes bit mask for Rx extract filter 0
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_FILTER_0_N2B . RX_FILTER_0_N2B_MASK
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_FILTER_0_N2B_RX_FILTER_0_N2B_MASK(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_RAB_RX_EXTRACT_RX_FILTER_0_N2B_RX_FILTER_0_N2B_MASK     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_RAB_RX_EXTRACT_RX_FILTER_0_N2B_RX_FILTER_0_N2B_MASK(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Next 2 bytes value to match for Rx extract filter 0
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_FILTER_0_N2B . RX_FILTER_0_N2B
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_FILTER_0_N2B_RX_FILTER_0_N2B(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_RAB_RX_EXTRACT_RX_FILTER_0_N2B_RX_FILTER_0_N2B     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_RAB_RX_EXTRACT_RX_FILTER_0_N2B_RX_FILTER_0_N2B(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief RAB Rx extract filter 1 Ethertype
 *
 * \details
 * Register: \a RAB::RX_EXTRACT::RX_FILTER_1_ETYPE
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_EXTRACT_RX_FILTER_1_ETYPE(target)  VTSS_IOREG(target,0x302)

/** 
 * \brief
 * Enable for Rx extract filter 1
 *
 * \details 
 * 0: Disabled
 * 1: Enabled
 *
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_FILTER_1_ETYPE . RX_FILTER_1_ENABLE
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_FILTER_1_ETYPE_RX_FILTER_1_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_RAB_RX_EXTRACT_RX_FILTER_1_ETYPE_RX_FILTER_1_ENABLE     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_RAB_RX_EXTRACT_RX_FILTER_1_ETYPE_RX_FILTER_1_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Ethertype value to match for Rx extract filter 1
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_FILTER_1_ETYPE . RX_FILTER_1_ETYPE
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_FILTER_1_ETYPE_RX_FILTER_1_ETYPE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_RAB_RX_EXTRACT_RX_FILTER_1_ETYPE_RX_FILTER_1_ETYPE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_RAB_RX_EXTRACT_RX_FILTER_1_ETYPE_RX_FILTER_1_ETYPE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief RAB Rx extract filter 1 next two bytes
 *
 * \details
 * Register: \a RAB::RX_EXTRACT::RX_FILTER_1_N2B
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_EXTRACT_RX_FILTER_1_N2B(target)  VTSS_IOREG(target,0x303)

/** 
 * \brief
 * Next two bytes bit mask for Rx extract filter 1
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_FILTER_1_N2B . RX_FILTER_1_N2B_MASK
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_FILTER_1_N2B_RX_FILTER_1_N2B_MASK(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_RAB_RX_EXTRACT_RX_FILTER_1_N2B_RX_FILTER_1_N2B_MASK     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_RAB_RX_EXTRACT_RX_FILTER_1_N2B_RX_FILTER_1_N2B_MASK(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Next two bytes value to match for Rx extract filter 1
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_FILTER_1_N2B . RX_FILTER_1_N2B
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_FILTER_1_N2B_RX_FILTER_1_N2B(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_RAB_RX_EXTRACT_RX_FILTER_1_N2B_RX_FILTER_1_N2B     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_RAB_RX_EXTRACT_RX_FILTER_1_N2B_RX_FILTER_1_N2B(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief RAB Rx extract filter 2 Ethertype
 *
 * \details
 * Register: \a RAB::RX_EXTRACT::RX_FILTER_2_ETYPE
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_EXTRACT_RX_FILTER_2_ETYPE(target)  VTSS_IOREG(target,0x304)

/** 
 * \brief
 * Enable for Rx extract filter 2
 *
 * \details 
 * 0: Disabled
 * 1: Enabled	 
 *
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_FILTER_2_ETYPE . RX_FILTER_2_ENABLE
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_FILTER_2_ETYPE_RX_FILTER_2_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_RAB_RX_EXTRACT_RX_FILTER_2_ETYPE_RX_FILTER_2_ENABLE     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_RAB_RX_EXTRACT_RX_FILTER_2_ETYPE_RX_FILTER_2_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Ethertype value to match for Rx extract filter 2
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_FILTER_2_ETYPE . RX_FILTER_2_ETYPE
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_FILTER_2_ETYPE_RX_FILTER_2_ETYPE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_RAB_RX_EXTRACT_RX_FILTER_2_ETYPE_RX_FILTER_2_ETYPE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_RAB_RX_EXTRACT_RX_FILTER_2_ETYPE_RX_FILTER_2_ETYPE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief RAB Rx extract filter 2 next two bytes
 *
 * \details
 * Register: \a RAB::RX_EXTRACT::RX_FILTER_2_N2B
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_EXTRACT_RX_FILTER_2_N2B(target)  VTSS_IOREG(target,0x305)

/** 
 * \brief
 * Next two bytes bit mask for Rx extract filter 2
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_FILTER_2_N2B . RX_FILTER_2_N2B_MASK
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_FILTER_2_N2B_RX_FILTER_2_N2B_MASK(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_RAB_RX_EXTRACT_RX_FILTER_2_N2B_RX_FILTER_2_N2B_MASK     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_RAB_RX_EXTRACT_RX_FILTER_2_N2B_RX_FILTER_2_N2B_MASK(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Next two bytes value to match for Rx extract filter 2
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_FILTER_2_N2B . RX_FILTER_2_N2B
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_FILTER_2_N2B_RX_FILTER_2_N2B(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_RAB_RX_EXTRACT_RX_FILTER_2_N2B_RX_FILTER_2_N2B     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_RAB_RX_EXTRACT_RX_FILTER_2_N2B_RX_FILTER_2_N2B(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief RAB Rx extract filter 3 Ethertype
 *
 * \details
 * Register: \a RAB::RX_EXTRACT::RX_FILTER_3_ETYPE
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_EXTRACT_RX_FILTER_3_ETYPE(target)  VTSS_IOREG(target,0x306)

/** 
 * \brief
 * Enable for Rx extract filter 3
 *
 * \details 
 * 0: Disabled
 * 1: Enabled
 *
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_FILTER_3_ETYPE . RX_FILTER_3_ENABLE
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_FILTER_3_ETYPE_RX_FILTER_3_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_RAB_RX_EXTRACT_RX_FILTER_3_ETYPE_RX_FILTER_3_ENABLE     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_RAB_RX_EXTRACT_RX_FILTER_3_ETYPE_RX_FILTER_3_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Ethertype value to match for Rx extract filter 3
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_FILTER_3_ETYPE . RX_FILTER_3_ETYPE
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_FILTER_3_ETYPE_RX_FILTER_3_ETYPE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_RAB_RX_EXTRACT_RX_FILTER_3_ETYPE_RX_FILTER_3_ETYPE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_RAB_RX_EXTRACT_RX_FILTER_3_ETYPE_RX_FILTER_3_ETYPE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief RAB Rx extract filter 3 next two bytes
 *
 * \details
 * Register: \a RAB::RX_EXTRACT::RX_FILTER_3_N2B
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_EXTRACT_RX_FILTER_3_N2B(target)  VTSS_IOREG(target,0x307)

/** 
 * \brief
 * Next two bytes bit mask for Rx extract filter 3 
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_FILTER_3_N2B . RX_FILTER_3_N2B_MASK
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_FILTER_3_N2B_RX_FILTER_3_N2B_MASK(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_RAB_RX_EXTRACT_RX_FILTER_3_N2B_RX_FILTER_3_N2B_MASK     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_RAB_RX_EXTRACT_RX_FILTER_3_N2B_RX_FILTER_3_N2B_MASK(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Next two bytes value to match for Rx extract filter 3
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_FILTER_3_N2B . RX_FILTER_3_N2B
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_FILTER_3_N2B_RX_FILTER_3_N2B(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_RAB_RX_EXTRACT_RX_FILTER_3_N2B_RX_FILTER_3_N2B     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_RAB_RX_EXTRACT_RX_FILTER_3_N2B_RX_FILTER_3_N2B(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief RAB Rx extract buffer status
 *
 * \details
 * Register: \a RAB::RX_EXTRACT::RX_EXTRACT_STATUS
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_EXTRACT_RX_EXTRACT_STATUS(target)  VTSS_IOREG(target,0x308)

/** 
 * \brief
 * Extract buffer flush request bit.
 * This bit will self-clear when the flush has completed.
 *
 * \details 
 * 1: Flush buffer
 *
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_EXTRACT_STATUS . RX_EXTRACT_FLUSH
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_EXTRACT_STATUS_RX_EXTRACT_FLUSH(x)  VTSS_ENCODE_BITFIELD(x,31,1)
#define  VTSS_M_RAB_RX_EXTRACT_RX_EXTRACT_STATUS_RX_EXTRACT_FLUSH     VTSS_ENCODE_BITMASK(31,1)
#define  VTSS_X_RAB_RX_EXTRACT_RX_EXTRACT_STATUS_RX_EXTRACT_FLUSH(x)  VTSS_EXTRACT_BITFIELD(x,31,1)

/** 
 * \brief
 * Extract buffer packet truncated.
 * The packet at the head of the extract buffer was truncated to 128 bytes.
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_EXTRACT_STATUS . RX_EXTRACT_PKT_TRUNC
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_EXTRACT_STATUS_RX_EXTRACT_PKT_TRUNC(x)  VTSS_ENCODE_BITFIELD(x,24,1)
#define  VTSS_M_RAB_RX_EXTRACT_RX_EXTRACT_STATUS_RX_EXTRACT_PKT_TRUNC     VTSS_ENCODE_BITMASK(24,1)
#define  VTSS_X_RAB_RX_EXTRACT_RX_EXTRACT_STATUS_RX_EXTRACT_PKT_TRUNC(x)  VTSS_EXTRACT_BITFIELD(x,24,1)

/** 
 * \brief
 * Extract buffer packet length.
 * The byte length of the packet at the head of the extract buffer.
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_EXTRACT_STATUS . RX_EXTRACT_PKT_LENGTH
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_EXTRACT_STATUS_RX_EXTRACT_PKT_LENGTH(x)  VTSS_ENCODE_BITFIELD(x,16,8)
#define  VTSS_M_RAB_RX_EXTRACT_RX_EXTRACT_STATUS_RX_EXTRACT_PKT_LENGTH     VTSS_ENCODE_BITMASK(16,8)
#define  VTSS_X_RAB_RX_EXTRACT_RX_EXTRACT_STATUS_RX_EXTRACT_PKT_LENGTH(x)  VTSS_EXTRACT_BITFIELD(x,16,8)

/** 
 * \brief
 * Extract buffer packet filter.
 * The packet filter that matched the packet at the head of the extract
 * buffer.
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_EXTRACT_STATUS . RX_EXTRACT_PKT_FILTER
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_EXTRACT_STATUS_RX_EXTRACT_PKT_FILTER(x)  VTSS_ENCODE_BITFIELD(x,8,2)
#define  VTSS_M_RAB_RX_EXTRACT_RX_EXTRACT_STATUS_RX_EXTRACT_PKT_FILTER     VTSS_ENCODE_BITMASK(8,2)
#define  VTSS_X_RAB_RX_EXTRACT_RX_EXTRACT_STATUS_RX_EXTRACT_PKT_FILTER(x)  VTSS_EXTRACT_BITFIELD(x,8,2)

/** 
 * \brief
 * Extract buffer packet pending count.
 * The number of packets currently in the extrac.t buffer.
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_EXTRACT_STATUS . RX_EXTRACT_PKT_PEND_CNT
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_EXTRACT_STATUS_RX_EXTRACT_PKT_PEND_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_RAB_RX_EXTRACT_RX_EXTRACT_STATUS_RX_EXTRACT_PKT_PEND_CNT     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_RAB_RX_EXTRACT_RX_EXTRACT_STATUS_RX_EXTRACT_PKT_PEND_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief RAB Rx extract buffer data LSB
 *
 * \details
 * Register: \a RAB::RX_EXTRACT::RX_EXTRACT_DATA_LSB
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_EXTRACT_RX_EXTRACT_DATA_LSB(target)  VTSS_IOREG(target,0x309)

/** 
 * \brief
 * Bits [31:0] of the extract buffer data
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_EXTRACT_DATA_LSB . RX_EXTRACT_DATA_LSB
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_EXTRACT_DATA_LSB_RX_EXTRACT_DATA_LSB(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_RAB_RX_EXTRACT_RX_EXTRACT_DATA_LSB_RX_EXTRACT_DATA_LSB     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_RAB_RX_EXTRACT_RX_EXTRACT_DATA_LSB_RX_EXTRACT_DATA_LSB(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief RAB Rx extract buffer data MSB
 *
 * \details
 * Register: \a RAB::RX_EXTRACT::RX_EXTRACT_DATA_MSB
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_EXTRACT_RX_EXTRACT_DATA_MSB(target)  VTSS_IOREG(target,0x30a)

/** 
 * \brief
 * Bits [63:32] of the extract buffer data
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_EXTRACT_DATA_MSB . RX_EXTRACT_DATA_MSB
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_EXTRACT_DATA_MSB_RX_EXTRACT_DATA_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_RAB_RX_EXTRACT_RX_EXTRACT_DATA_MSB_RX_EXTRACT_DATA_MSB     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_RAB_RX_EXTRACT_RX_EXTRACT_DATA_MSB_RX_EXTRACT_DATA_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief RAB Rx extract packet preamble LSB
 *
 * \details
 * Register: \a RAB::RX_EXTRACT::RX_EXTRACT_PREAMBLE_LSB
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_EXTRACT_RX_EXTRACT_PREAMBLE_LSB(target)  VTSS_IOREG(target,0x30b)

/** 
 * \brief
 * Bits [31:0] of the extract buffer packet preamble + SFD. The preamble +
 * SFD of the packet at the head of the extract buffer.
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_EXTRACT_PREAMBLE_LSB . RX_EXTRACT_PREAMBLE_LSB
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_EXTRACT_PREAMBLE_LSB_RX_EXTRACT_PREAMBLE_LSB(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_RAB_RX_EXTRACT_RX_EXTRACT_PREAMBLE_LSB_RX_EXTRACT_PREAMBLE_LSB     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_RAB_RX_EXTRACT_RX_EXTRACT_PREAMBLE_LSB_RX_EXTRACT_PREAMBLE_LSB(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief RAB Rx extract packet preamble MSB
 *
 * \details
 * Register: \a RAB::RX_EXTRACT::RX_EXTRACT_PREAMBLE_MSB
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_EXTRACT_RX_EXTRACT_PREAMBLE_MSB(target)  VTSS_IOREG(target,0x30c)

/** 
 * \brief
 * Bits [63:32] of the extract buffer packet preamble + SFD. The preamble +
 * SFD of the packet at the head of the extract buffer.
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_EXTRACT_PREAMBLE_MSB . RX_EXTRACT_PREAMBLE_MSB
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_EXTRACT_PREAMBLE_MSB_RX_EXTRACT_PREAMBLE_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_RAB_RX_EXTRACT_RX_EXTRACT_PREAMBLE_MSB_RX_EXTRACT_PREAMBLE_MSB     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_RAB_RX_EXTRACT_RX_EXTRACT_PREAMBLE_MSB_RX_EXTRACT_PREAMBLE_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief RAB Rx extract buffer packet count 0
 *
 * \details
 * Register: \a RAB::RX_EXTRACT::RX_EXTRACT_PKT_CNT_0
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_0(target)  VTSS_IOREG(target,0x30d)

/** 
 * \brief
 * Counts number of packets that matched extract filter 0, but were
 * discarded due to a full extract buffer.
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_0 . RX_EXTRACT_DROP_CNT_0
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_0_RX_EXTRACT_DROP_CNT_0(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_0_RX_EXTRACT_DROP_CNT_0     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_0_RX_EXTRACT_DROP_CNT_0(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Counts number of packets captured in the extract buffer that matched
 * extract filter 0
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_0 . RX_EXTRACT_PKT_CNT_0
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_0_RX_EXTRACT_PKT_CNT_0(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_0_RX_EXTRACT_PKT_CNT_0     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_0_RX_EXTRACT_PKT_CNT_0(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief RAB Rx extract buffer packet count 1
 *
 * \details
 * Register: \a RAB::RX_EXTRACT::RX_EXTRACT_PKT_CNT_1
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_1(target)  VTSS_IOREG(target,0x30e)

/** 
 * \brief
 * Counts number of packets that matched extract filter 1, but were
 * discarded due to a full extract buffer.
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_1 . RX_EXTRACT_DROP_CNT_1
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_1_RX_EXTRACT_DROP_CNT_1(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_1_RX_EXTRACT_DROP_CNT_1     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_1_RX_EXTRACT_DROP_CNT_1(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Counts number of packets captured in the extract buffer that matched
 * extract filter 1
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_1 . RX_EXTRACT_PKT_CNT_1
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_1_RX_EXTRACT_PKT_CNT_1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_1_RX_EXTRACT_PKT_CNT_1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_1_RX_EXTRACT_PKT_CNT_1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief RAB Rx extract buffer packet count 2
 *
 * \details
 * Register: \a RAB::RX_EXTRACT::RX_EXTRACT_PKT_CNT_2
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_2(target)  VTSS_IOREG(target,0x30f)

/** 
 * \brief
 * Counts number of packets that matched extract filter 2, but were
 * discarded due to a full extract buffer.
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_2 . RX_EXTRACT_DROP_CNT_2
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_2_RX_EXTRACT_DROP_CNT_2(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_2_RX_EXTRACT_DROP_CNT_2     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_2_RX_EXTRACT_DROP_CNT_2(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Counts number of packets captured in the extract buffer that matched
 * extract filter 2
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_2 . RX_EXTRACT_PKT_CNT_2
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_2_RX_EXTRACT_PKT_CNT_2(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_2_RX_EXTRACT_PKT_CNT_2     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_2_RX_EXTRACT_PKT_CNT_2(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief RAB Rx extract buffer packet count 3
 *
 * \details
 * Register: \a RAB::RX_EXTRACT::RX_EXTRACT_PKT_CNT_3
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_3(target)  VTSS_IOREG(target,0x310)

/** 
 * \brief
 * Counts number of packets that matched extract filter 3, but were
 * discarded due to a full extract buffer.
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_3 . RX_EXTRACT_DROP_CNT_3
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_3_RX_EXTRACT_DROP_CNT_3(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_3_RX_EXTRACT_DROP_CNT_3     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_3_RX_EXTRACT_DROP_CNT_3(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Counts number of packets captured in the extract buffer that matched
 * extract filter 3
 *
 * \details 
 * Field: ::VTSS_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_3 . RX_EXTRACT_PKT_CNT_3
 */
#define  VTSS_F_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_3_RX_EXTRACT_PKT_CNT_3(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_3_RX_EXTRACT_PKT_CNT_3     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_RAB_RX_EXTRACT_RX_EXTRACT_PKT_CNT_3_RX_EXTRACT_PKT_CNT_3(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a RAB::TX_FIFO
 *
 * RAB Tx FIFO registers
 */


/** 
 * \brief RAB Tx FIFO control register
 *
 * \details
 * Register: \a RAB::TX_FIFO::TX_FIFO_CONTROL
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_FIFO_TX_FIFO_CONTROL(target)  VTSS_IOREG(target,0x400)

/** 
 * \brief
 * Tx FIFO cut-through threshold.
 * Threshold value is in units of 8 byte words.
 *
 * \details 
 * Field: ::VTSS_RAB_TX_FIFO_TX_FIFO_CONTROL . TX_FIFO_CUT_THRU_THRESH
 */
#define  VTSS_F_RAB_TX_FIFO_TX_FIFO_CONTROL_TX_FIFO_CUT_THRU_THRESH(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_RAB_TX_FIFO_TX_FIFO_CONTROL_TX_FIFO_CUT_THRU_THRESH     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_RAB_TX_FIFO_TX_FIFO_CONTROL_TX_FIFO_CUT_THRU_THRESH(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Tx FIFO cut-through mode.
 * Note: when TX_FIFO_OUTPUT_SELECT = 1, TX_FIFO_CUT_THRU has no effect and
 * the Tx FIFO will operate in store-and-forward.
 *
 * \details 
 * 0: Disable cut-through operation
 * 1: Enable cut-through operation
 *
 * Field: ::VTSS_RAB_TX_FIFO_TX_FIFO_CONTROL . TX_FIFO_CUT_THRU
 */
#define  VTSS_F_RAB_TX_FIFO_TX_FIFO_CONTROL_TX_FIFO_CUT_THRU(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_RAB_TX_FIFO_TX_FIFO_CONTROL_TX_FIFO_CUT_THRU     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_RAB_TX_FIFO_TX_FIFO_CONTROL_TX_FIFO_CUT_THRU(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Disable discarding of frames that arrive to the Tx FIFO marked with
 * abort
 *
 * \details 
 * 0: Enable discarding of aborted frames
 * 1: Disable discarding of aborted frames
 *
 * Field: ::VTSS_RAB_TX_FIFO_TX_FIFO_CONTROL . TX_FIFO_DROP_DISABLE
 */
#define  VTSS_F_RAB_TX_FIFO_TX_FIFO_CONTROL_TX_FIFO_DROP_DISABLE(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_RAB_TX_FIFO_TX_FIFO_CONTROL_TX_FIFO_DROP_DISABLE     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_RAB_TX_FIFO_TX_FIFO_CONTROL_TX_FIFO_DROP_DISABLE(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Select input interface into Tx FIFO
 *
 * \details 
 * 0: MAC1 Rx host interface
 * 1: MAC1 Rx XGMII interface
 * 2: GFP1 Rx client interface
 * 3: Reserved
 *
 * Field: ::VTSS_RAB_TX_FIFO_TX_FIFO_CONTROL . TX_FIFO_INPUT_SELECT
 */
#define  VTSS_F_RAB_TX_FIFO_TX_FIFO_CONTROL_TX_FIFO_INPUT_SELECT(x)  VTSS_ENCODE_BITFIELD(x,1,2)
#define  VTSS_M_RAB_TX_FIFO_TX_FIFO_CONTROL_TX_FIFO_INPUT_SELECT     VTSS_ENCODE_BITMASK(1,2)
#define  VTSS_X_RAB_TX_FIFO_TX_FIFO_CONTROL_TX_FIFO_INPUT_SELECT(x)  VTSS_EXTRACT_BITFIELD(x,1,2)

/** 
 * \brief
 * Select output interface from Tx FIFO
 *
 * \details 
 * 0: MAC2 Tx host interface
 * 1: GFP2 Tx client interface
 *
 * Field: ::VTSS_RAB_TX_FIFO_TX_FIFO_CONTROL . TX_FIFO_OUTPUT_SELECT
 */
#define  VTSS_F_RAB_TX_FIFO_TX_FIFO_CONTROL_TX_FIFO_OUTPUT_SELECT(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_RAB_TX_FIFO_TX_FIFO_CONTROL_TX_FIFO_OUTPUT_SELECT     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_RAB_TX_FIFO_TX_FIFO_CONTROL_TX_FIFO_OUTPUT_SELECT(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief RAB Tx FIFO high threshold
 *
 * \details
 * Register: \a RAB::TX_FIFO::TX_FIFO_HIGH_THRESH
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_FIFO_TX_FIFO_HIGH_THRESH(target)  VTSS_IOREG(target,0x401)

/** 
 * \brief
 * High threshold for generation of PAUSE flow control on MAC1 Tx.
 * Threshold value is in units of 8 byte words.
 *
 * \details 
 * Field: ::VTSS_RAB_TX_FIFO_TX_FIFO_HIGH_THRESH . TX_FIFO_HIGH_THRESH
 */
#define  VTSS_F_RAB_TX_FIFO_TX_FIFO_HIGH_THRESH_TX_FIFO_HIGH_THRESH(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_RAB_TX_FIFO_TX_FIFO_HIGH_THRESH_TX_FIFO_HIGH_THRESH     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_RAB_TX_FIFO_TX_FIFO_HIGH_THRESH_TX_FIFO_HIGH_THRESH(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \brief RAB Tx FIFO low threshold
 *
 * \details
 * Register: \a RAB::TX_FIFO::TX_FIFO_LOW_THRESH
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_FIFO_TX_FIFO_LOW_THRESH(target)  VTSS_IOREG(target,0x402)

/** 
 * \brief
 * Low threshold for generation of PAUSE flow control on MAC1 Tx. Threshold
 * value is in units of 8 byte words.
 *
 * \details 
 * Field: ::VTSS_RAB_TX_FIFO_TX_FIFO_LOW_THRESH . TX_FIFO_LOW_THRESH
 */
#define  VTSS_F_RAB_TX_FIFO_TX_FIFO_LOW_THRESH_TX_FIFO_LOW_THRESH(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_RAB_TX_FIFO_TX_FIFO_LOW_THRESH_TX_FIFO_LOW_THRESH     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_RAB_TX_FIFO_TX_FIFO_LOW_THRESH_TX_FIFO_LOW_THRESH(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \brief RAB Tx FIFO adaptation threshold
 *
 * \details
 * Register: \a RAB::TX_FIFO::TX_FIFO_ADAPT_THRESH
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_FIFO_TX_FIFO_ADAPT_THRESH(target)  VTSS_IOREG(target,0x403)

/** 
 * \brief
 * Threshold for assertion of rate adaptation request to MAC2 Tx. Threshold
 * value is in units of 8 byte words.
 *
 * \details 
 * Field: ::VTSS_RAB_TX_FIFO_TX_FIFO_ADAPT_THRESH . TX_FIFO_ADAPT_THRESH
 */
#define  VTSS_F_RAB_TX_FIFO_TX_FIFO_ADAPT_THRESH_TX_FIFO_ADAPT_THRESH(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_RAB_TX_FIFO_TX_FIFO_ADAPT_THRESH_TX_FIFO_ADAPT_THRESH     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_RAB_TX_FIFO_TX_FIFO_ADAPT_THRESH_TX_FIFO_ADAPT_THRESH(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \brief RAB Tx FIFO packet drop count
 *
 * \details
 * Register: \a RAB::TX_FIFO::TX_PKT_DROP_CNT
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_FIFO_TX_PKT_DROP_CNT(target)  VTSS_IOREG(target,0x404)

/** 
 * \brief
 * Counts number of packets discarded in the Tx FIFO due to Tx FIFO
 * overflow
 *
 * \details 
 * Field: ::VTSS_RAB_TX_FIFO_TX_PKT_DROP_CNT . TX_PKT_DROP_CNT
 */
#define  VTSS_F_RAB_TX_FIFO_TX_PKT_DROP_CNT_TX_PKT_DROP_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_RAB_TX_FIFO_TX_PKT_DROP_CNT_TX_PKT_DROP_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_RAB_TX_FIFO_TX_PKT_DROP_CNT_TX_PKT_DROP_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief RAB Tx FIFO packet drop count performance monitor
 *
 * \details
 * Register: \a RAB::TX_FIFO::TX_PKT_DROP_CNT_PERF
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_FIFO_TX_PKT_DROP_CNT_PERF(target)  VTSS_IOREG(target,0x405)

/** 
 * \brief
 * Counts number of packets discarded in the Tx FIFO due to Tx FIFO
 * overflow
 *
 * \details 
 * Field: ::VTSS_RAB_TX_FIFO_TX_PKT_DROP_CNT_PERF . TX_PKT_DROP_CNT_PERF
 */
#define  VTSS_F_RAB_TX_FIFO_TX_PKT_DROP_CNT_PERF_TX_PKT_DROP_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_RAB_TX_FIFO_TX_PKT_DROP_CNT_PERF_TX_PKT_DROP_CNT_PERF     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_RAB_TX_FIFO_TX_PKT_DROP_CNT_PERF_TX_PKT_DROP_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief RAB Tx FIFO packets pending count
 *
 * \details
 * Register: \a RAB::TX_FIFO::TX_FIFO_PKT_PEND_CNT
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_FIFO_TX_FIFO_PKT_PEND_CNT(target)  VTSS_IOREG(target,0x406)

/** 
 * \brief
 * The number of packets currently queued in the Tx FIFO
 *
 * \details 
 * Field: ::VTSS_RAB_TX_FIFO_TX_FIFO_PKT_PEND_CNT . TX_FIFO_PKT_PEND_CNT
 */
#define  VTSS_F_RAB_TX_FIFO_TX_FIFO_PKT_PEND_CNT_TX_FIFO_PKT_PEND_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_RAB_TX_FIFO_TX_FIFO_PKT_PEND_CNT_TX_FIFO_PKT_PEND_CNT     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_RAB_TX_FIFO_TX_FIFO_PKT_PEND_CNT_TX_FIFO_PKT_PEND_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \brief RAB Tx FIFO high level status
 *
 * \details
 * Register: \a RAB::TX_FIFO::TX_FIFO_HIGH_STATUS
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_FIFO_TX_FIFO_HIGH_STATUS(target)  VTSS_IOREG(target,0x407)

/** 
 * \brief
 * Reset the TX_HIGH_LEVEL_STICKY to its default. This bit will self-clear
 * when the reset has completed.
 *
 * \details 
 * 1: Reset high level status
 *
 * Field: ::VTSS_RAB_TX_FIFO_TX_FIFO_HIGH_STATUS . TX_HIGH_LEVEL_RST
 */
#define  VTSS_F_RAB_TX_FIFO_TX_FIFO_HIGH_STATUS_TX_HIGH_LEVEL_RST(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_RAB_TX_FIFO_TX_FIFO_HIGH_STATUS_TX_HIGH_LEVEL_RST     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_RAB_TX_FIFO_TX_FIFO_HIGH_STATUS_TX_HIGH_LEVEL_RST(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Latches the highest value of fill level observed in the Tx FIFO. Fill
 * level value is in units of 8 byte words.  
 *
 * \details 
 * Field: ::VTSS_RAB_TX_FIFO_TX_FIFO_HIGH_STATUS . TX_HIGH_LEVEL_STICKY
 */
#define  VTSS_F_RAB_TX_FIFO_TX_FIFO_HIGH_STATUS_TX_HIGH_LEVEL_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_RAB_TX_FIFO_TX_FIFO_HIGH_STATUS_TX_HIGH_LEVEL_STICKY     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_RAB_TX_FIFO_TX_FIFO_HIGH_STATUS_TX_HIGH_LEVEL_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \brief RAB Tx FIFO low level status
 *
 * \details
 * Register: \a RAB::TX_FIFO::TX_FIFO_LOW_STATUS
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_FIFO_TX_FIFO_LOW_STATUS(target)  VTSS_IOREG(target,0x408)

/** 
 * \brief
 * Reset the TX_LOW_LEVEL_STICKY to its default. This bit will self-clear
 * when the reset has completed.
 *
 * \details 
 * 1: Reset low level status
 *
 * Field: ::VTSS_RAB_TX_FIFO_TX_FIFO_LOW_STATUS . TX_LOW_LEVEL_RST
 */
#define  VTSS_F_RAB_TX_FIFO_TX_FIFO_LOW_STATUS_TX_LOW_LEVEL_RST(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_RAB_TX_FIFO_TX_FIFO_LOW_STATUS_TX_LOW_LEVEL_RST     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_RAB_TX_FIFO_TX_FIFO_LOW_STATUS_TX_LOW_LEVEL_RST(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Latches the lowest value of fill level observed in the Tx FIFO. Fill
 * level value is in units of 8 byte words.
 *
 * \details 
 * Field: ::VTSS_RAB_TX_FIFO_TX_FIFO_LOW_STATUS . TX_LOW_LEVEL_STICKY
 */
#define  VTSS_F_RAB_TX_FIFO_TX_FIFO_LOW_STATUS_TX_LOW_LEVEL_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_RAB_TX_FIFO_TX_FIFO_LOW_STATUS_TX_LOW_LEVEL_STICKY     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_RAB_TX_FIFO_TX_FIFO_LOW_STATUS_TX_LOW_LEVEL_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \brief RAB Tx sync buffer control
 *
 * \details
 * Register: \a RAB::TX_FIFO::TX_SYNC_CONTROL
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_FIFO_TX_SYNC_CONTROL(target)  VTSS_IOREG(target,0x409)

/** 
 * \brief
 * High threshold for generation of the synchronization buffer almost full
 * condition. Threshold value is in units of 8 byte words; legal values are
 * 0 to 31.	  
 *
 * \details 
 * Field: ::VTSS_RAB_TX_FIFO_TX_SYNC_CONTROL . TX_SYNC_AF_THRESH
 */
#define  VTSS_F_RAB_TX_FIFO_TX_SYNC_CONTROL_TX_SYNC_AF_THRESH(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_RAB_TX_FIFO_TX_SYNC_CONTROL_TX_SYNC_AF_THRESH     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_RAB_TX_FIFO_TX_SYNC_CONTROL_TX_SYNC_AF_THRESH(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Low threshold for generation of the synchronization buffer almost empty
 * condition. Threshold value is in units of 8 byte words; legal values are
 * 0 to 31.	  
 *
 * \details 
 * Field: ::VTSS_RAB_TX_FIFO_TX_SYNC_CONTROL . TX_SYNC_AE_THRESH
 */
#define  VTSS_F_RAB_TX_FIFO_TX_SYNC_CONTROL_TX_SYNC_AE_THRESH(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_RAB_TX_FIFO_TX_SYNC_CONTROL_TX_SYNC_AE_THRESH     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_RAB_TX_FIFO_TX_SYNC_CONTROL_TX_SYNC_AE_THRESH(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief RAB Tx FIFO freeze control
 *
 * \details
 * Register: \a RAB::TX_FIFO::TX_FIFO_FREEZE
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_FIFO_TX_FIFO_FREEZE(target)  VTSS_IOREG(target,0x40a)

/** 
 * \brief
 * Freeze Tx FIFO.
 * Immediately stops FIFO activity, including queueing or de-queueing of
 * partial frames.   
 *
 * \details 
 * 0: Normal FIFO operation
 * 1: Freeze FIFO
 *
 * Field: ::VTSS_RAB_TX_FIFO_TX_FIFO_FREEZE . TX_FIFO_FREEZE
 */
#define  VTSS_F_RAB_TX_FIFO_TX_FIFO_FREEZE_TX_FIFO_FREEZE(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_RAB_TX_FIFO_TX_FIFO_FREEZE_TX_FIFO_FREEZE     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_RAB_TX_FIFO_TX_FIFO_FREEZE_TX_FIFO_FREEZE(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief RAB Tx FIFO flush control
 *
 * \details
 * Register: \a RAB::TX_FIFO::TX_FIFO_FLUSH
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_FIFO_TX_FIFO_FLUSH(target)  VTSS_IOREG(target,0x40b)

/** 
 * \brief
 * Tx FIFO flush request bit.
 * This bit will self-clear when the flush has completed. Has no effect
 * when TX_FIFO_FREEZE = 0.   
 *
 * \details 
 * 1: Flush FIFO
 *
 * Field: ::VTSS_RAB_TX_FIFO_TX_FIFO_FLUSH . TX_FIFO_FLUSH
 */
#define  VTSS_F_RAB_TX_FIFO_TX_FIFO_FLUSH_TX_FIFO_FLUSH(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_RAB_TX_FIFO_TX_FIFO_FLUSH_TX_FIFO_FLUSH     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_RAB_TX_FIFO_TX_FIFO_FLUSH_TX_FIFO_FLUSH(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/**
 * Register Group: \a RAB::TX_INSERT
 *
 * RAB Tx packet insert registers
 */


/** 
 * \brief RAB Tx insert buffer control
 *
 * \details
 * Register: \a RAB::TX_INSERT::TX_INSERT_CONTROL
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_INSERT_TX_INSERT_CONTROL(target)  VTSS_IOREG(target,0x500)

/** 
 * \brief
 * Insert frame go request bit.
 * This bit self-clears when the insert request has been serviced. If
 * auto-insertion is enabled by the value in TX_INSERT_INTERVAL, the frame
 * continues to be inserted at the selected interval.
 *
 * \details 
 * 1: Insert frame 
 *
 * Field: ::VTSS_RAB_TX_INSERT_TX_INSERT_CONTROL . TX_INSERT_GO
 */
#define  VTSS_F_RAB_TX_INSERT_TX_INSERT_CONTROL_TX_INSERT_GO(x)  VTSS_ENCODE_BITFIELD(x,31,1)
#define  VTSS_M_RAB_TX_INSERT_TX_INSERT_CONTROL_TX_INSERT_GO     VTSS_ENCODE_BITMASK(31,1)
#define  VTSS_X_RAB_TX_INSERT_TX_INSERT_CONTROL_TX_INSERT_GO(x)  VTSS_EXTRACT_BITFIELD(x,31,1)

/** 
 * \brief
 * Insert frame interval select
 *
 * \details 
 * 000: Frame auto-insert constantly
 * 001: Frame auto-insert every 200 ms
 * 010: Frame auto-insert every 400 ms
 * 011: Frame auto-insert every 600 ms
 * 100: Frame auto-insert every 800 ms
 * 101: Frame auto-insert every 1000 ms
 * 110: Frame auto-insert every 1200 ms
 * 111: Auto-insertion disabled
 *
 * Field: ::VTSS_RAB_TX_INSERT_TX_INSERT_CONTROL . TX_INSERT_INTERVAL
 */
#define  VTSS_F_RAB_TX_INSERT_TX_INSERT_CONTROL_TX_INSERT_INTERVAL(x)  VTSS_ENCODE_BITFIELD(x,24,3)
#define  VTSS_M_RAB_TX_INSERT_TX_INSERT_CONTROL_TX_INSERT_INTERVAL     VTSS_ENCODE_BITMASK(24,3)
#define  VTSS_X_RAB_TX_INSERT_TX_INSERT_CONTROL_TX_INSERT_INTERVAL(x)  VTSS_EXTRACT_BITFIELD(x,24,3)

/** 
 * \brief
 * Insert buffer address.
 * The address which will be written to (or read from) when writing (or
 * reading) the insert buffer data registers. Always access
 * TX_INSERT_DATA_LSB before TX_INSERT_DATA_MSB for a given address.
 * TX_INSERT_ADDR will auto-increment when TX_INSERT_DATA_MSB is accessed.
 * Note: always set TX_INSERT_ADDR to 0 before loading a frame in the
 * insert buffer.
 *
 * \details 
 * Field: ::VTSS_RAB_TX_INSERT_TX_INSERT_CONTROL . TX_INSERT_ADDR
 */
#define  VTSS_F_RAB_TX_INSERT_TX_INSERT_CONTROL_TX_INSERT_ADDR(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_RAB_TX_INSERT_TX_INSERT_CONTROL_TX_INSERT_ADDR     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_RAB_TX_INSERT_TX_INSERT_CONTROL_TX_INSERT_ADDR(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief RAB Tx insert buffer data LSB
 *
 * \details
 * Register: \a RAB::TX_INSERT::TX_INSERT_DATA_LSB
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_INSERT_TX_INSERT_DATA_LSB(target)  VTSS_IOREG(target,0x501)

/** 
 * \brief
 * Bits [31:0] of the insert buffer data
 *
 * \details 
 * Field: ::VTSS_RAB_TX_INSERT_TX_INSERT_DATA_LSB . TX_INSERT_DATA_LSB
 */
#define  VTSS_F_RAB_TX_INSERT_TX_INSERT_DATA_LSB_TX_INSERT_DATA_LSB(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_RAB_TX_INSERT_TX_INSERT_DATA_LSB_TX_INSERT_DATA_LSB     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_RAB_TX_INSERT_TX_INSERT_DATA_LSB_TX_INSERT_DATA_LSB(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief RAB Tx insert buffer data MSB
 *
 * \details
 * Register: \a RAB::TX_INSERT::TX_INSERT_DATA_MSB
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_INSERT_TX_INSERT_DATA_MSB(target)  VTSS_IOREG(target,0x502)

/** 
 * \brief
 * Bits [63:32] of the insert buffer data
 *
 * \details 
 * Field: ::VTSS_RAB_TX_INSERT_TX_INSERT_DATA_MSB . TX_INSERT_DATA_MSB
 */
#define  VTSS_F_RAB_TX_INSERT_TX_INSERT_DATA_MSB_TX_INSERT_DATA_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_RAB_TX_INSERT_TX_INSERT_DATA_MSB_TX_INSERT_DATA_MSB     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_RAB_TX_INSERT_TX_INSERT_DATA_MSB_TX_INSERT_DATA_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief RAB Tx insert packet preamble LSB
 *
 * \details
 * Register: \a RAB::TX_INSERT::TX_INSERT_PREAMBLE_LSB
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_INSERT_TX_INSERT_PREAMBLE_LSB(target)  VTSS_IOREG(target,0x503)

/** 
 * \brief
 * Bits [31:0] of the insert packet preamble + SFD
 *
 * \details 
 * Field: ::VTSS_RAB_TX_INSERT_TX_INSERT_PREAMBLE_LSB . TX_INSERT_PREAMBLE_LSB
 */
#define  VTSS_F_RAB_TX_INSERT_TX_INSERT_PREAMBLE_LSB_TX_INSERT_PREAMBLE_LSB(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_RAB_TX_INSERT_TX_INSERT_PREAMBLE_LSB_TX_INSERT_PREAMBLE_LSB     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_RAB_TX_INSERT_TX_INSERT_PREAMBLE_LSB_TX_INSERT_PREAMBLE_LSB(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief RAB Tx insert packet preamble MSB
 *
 * \details
 * Register: \a RAB::TX_INSERT::TX_INSERT_PREAMBLE_MSB
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_INSERT_TX_INSERT_PREAMBLE_MSB(target)  VTSS_IOREG(target,0x504)

/** 
 * \brief
 * Bits [63:32] of the insert packet preamble + SFD
 *
 * \details 
 * Field: ::VTSS_RAB_TX_INSERT_TX_INSERT_PREAMBLE_MSB . TX_INSERT_PREAMBLE_MSB
 */
#define  VTSS_F_RAB_TX_INSERT_TX_INSERT_PREAMBLE_MSB_TX_INSERT_PREAMBLE_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_RAB_TX_INSERT_TX_INSERT_PREAMBLE_MSB_TX_INSERT_PREAMBLE_MSB     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_RAB_TX_INSERT_TX_INSERT_PREAMBLE_MSB_TX_INSERT_PREAMBLE_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief RAB Tx insert packet count
 *
 * \details
 * Register: \a RAB::TX_INSERT::TX_INSERT_PKT_CNT
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_INSERT_TX_INSERT_PKT_CNT(target)  VTSS_IOREG(target,0x505)

/** 
 * \brief
 * Counts number of packets inserted
 *
 * \details 
 * Field: ::VTSS_RAB_TX_INSERT_TX_INSERT_PKT_CNT . TX_INSERT_PKT_CNT
 */
#define  VTSS_F_RAB_TX_INSERT_TX_INSERT_PKT_CNT_TX_INSERT_PKT_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_RAB_TX_INSERT_TX_INSERT_PKT_CNT_TX_INSERT_PKT_CNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_RAB_TX_INSERT_TX_INSERT_PKT_CNT_TX_INSERT_PKT_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief RAB Tx insert timer
 *
 * \details
 * Register: \a RAB::TX_INSERT::TX_INSERT_TIMER
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_INSERT_TX_INSERT_TIMER(target)  VTSS_IOREG(target,0x506)

/** 
 * \brief
 * Insert interval timer count.
 * The interval timer count is used to support multiple clock frequencies
 * to ensure accurate frame insertion intervals. The interval timer count
 * value specifies the number of clock cycles per 100 ms.
 *
 * \details 
 * Field: ::VTSS_RAB_TX_INSERT_TX_INSERT_TIMER . TX_INSERT_TIMER_COUNT
 */
#define  VTSS_F_RAB_TX_INSERT_TX_INSERT_TIMER_TX_INSERT_TIMER_COUNT(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_RAB_TX_INSERT_TX_INSERT_TIMER_TX_INSERT_TIMER_COUNT     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_RAB_TX_INSERT_TX_INSERT_TIMER_TX_INSERT_TIMER_COUNT(x)  VTSS_EXTRACT_BITFIELD(x,0,24)

/**
 * Register Group: \a RAB::TX_EXTRACT
 *
 * RAB Tx packet extract registers
 */


/** 
 * \brief RAB Tx extract filter 0 Ethertype
 *
 * \details
 * Register: \a RAB::TX_EXTRACT::TX_FILTER_0_ETYPE
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_EXTRACT_TX_FILTER_0_ETYPE(target)  VTSS_IOREG(target,0x600)

/** 
 * \brief
 * Enable for Tx extract filter 0
 *
 * \details 
 * 0: Disabled
 * 1: Enabled	
 *
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_FILTER_0_ETYPE . TX_FILTER_0_ENABLE
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_FILTER_0_ETYPE_TX_FILTER_0_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_RAB_TX_EXTRACT_TX_FILTER_0_ETYPE_TX_FILTER_0_ENABLE     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_RAB_TX_EXTRACT_TX_FILTER_0_ETYPE_TX_FILTER_0_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Ethertype value to match for Tx extract filter 0
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_FILTER_0_ETYPE . TX_FILTER_0_ETYPE
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_FILTER_0_ETYPE_TX_FILTER_0_ETYPE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_RAB_TX_EXTRACT_TX_FILTER_0_ETYPE_TX_FILTER_0_ETYPE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_RAB_TX_EXTRACT_TX_FILTER_0_ETYPE_TX_FILTER_0_ETYPE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief RAB Tx extract filter 0 next two bytes
 *
 * \details
 * Register: \a RAB::TX_EXTRACT::TX_FILTER_0_N2B
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_EXTRACT_TX_FILTER_0_N2B(target)  VTSS_IOREG(target,0x601)

/** 
 * \brief
 * Next two bytes bit mask for Tx extract filter 0
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_FILTER_0_N2B . TX_FILTER_0_N2B_MASK
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_FILTER_0_N2B_TX_FILTER_0_N2B_MASK(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_RAB_TX_EXTRACT_TX_FILTER_0_N2B_TX_FILTER_0_N2B_MASK     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_RAB_TX_EXTRACT_TX_FILTER_0_N2B_TX_FILTER_0_N2B_MASK(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Next two bytes value to match for Tx extract filter 0
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_FILTER_0_N2B . TX_FILTER_0_N2B
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_FILTER_0_N2B_TX_FILTER_0_N2B(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_RAB_TX_EXTRACT_TX_FILTER_0_N2B_TX_FILTER_0_N2B     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_RAB_TX_EXTRACT_TX_FILTER_0_N2B_TX_FILTER_0_N2B(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief RAB Tx extract filter 1 Ethertype
 *
 * \details
 * Register: \a RAB::TX_EXTRACT::TX_FILTER_1_ETYPE
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_EXTRACT_TX_FILTER_1_ETYPE(target)  VTSS_IOREG(target,0x602)

/** 
 * \brief
 * Enable for Tx extract filter 1
 *
 * \details 
 * 0: Disabled
 * 1: Enabled
 *
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_FILTER_1_ETYPE . TX_FILTER_1_ENABLE
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_FILTER_1_ETYPE_TX_FILTER_1_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_RAB_TX_EXTRACT_TX_FILTER_1_ETYPE_TX_FILTER_1_ENABLE     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_RAB_TX_EXTRACT_TX_FILTER_1_ETYPE_TX_FILTER_1_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Ethertype value to match for Tx extract filter 1
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_FILTER_1_ETYPE . TX_FILTER_1_ETYPE
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_FILTER_1_ETYPE_TX_FILTER_1_ETYPE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_RAB_TX_EXTRACT_TX_FILTER_1_ETYPE_TX_FILTER_1_ETYPE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_RAB_TX_EXTRACT_TX_FILTER_1_ETYPE_TX_FILTER_1_ETYPE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief RAB Tx extract filter 1 next two bytes
 *
 * \details
 * Register: \a RAB::TX_EXTRACT::TX_FILTER_1_N2B
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_EXTRACT_TX_FILTER_1_N2B(target)  VTSS_IOREG(target,0x603)

/** 
 * \brief
 * Next two bytes bit mask for Tx extract filter 1
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_FILTER_1_N2B . TX_FILTER_1_N2B_MASK
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_FILTER_1_N2B_TX_FILTER_1_N2B_MASK(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_RAB_TX_EXTRACT_TX_FILTER_1_N2B_TX_FILTER_1_N2B_MASK     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_RAB_TX_EXTRACT_TX_FILTER_1_N2B_TX_FILTER_1_N2B_MASK(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Next two bytes value to match for Tx extract filter 1
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_FILTER_1_N2B . TX_FILTER_1_N2B
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_FILTER_1_N2B_TX_FILTER_1_N2B(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_RAB_TX_EXTRACT_TX_FILTER_1_N2B_TX_FILTER_1_N2B     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_RAB_TX_EXTRACT_TX_FILTER_1_N2B_TX_FILTER_1_N2B(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief RAB Tx extract filter 2 Ethertype
 *
 * \details
 * Register: \a RAB::TX_EXTRACT::TX_FILTER_2_ETYPE
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_EXTRACT_TX_FILTER_2_ETYPE(target)  VTSS_IOREG(target,0x604)

/** 
 * \brief
 * Enable for Tx extract filter 2
 *
 * \details 
 * 0: Disabled
 * 1: Enabled
 *
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_FILTER_2_ETYPE . TX_FILTER_2_ENABLE
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_FILTER_2_ETYPE_TX_FILTER_2_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_RAB_TX_EXTRACT_TX_FILTER_2_ETYPE_TX_FILTER_2_ENABLE     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_RAB_TX_EXTRACT_TX_FILTER_2_ETYPE_TX_FILTER_2_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Ethertype value to match for Tx extract filter 2
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_FILTER_2_ETYPE . TX_FILTER_2_ETYPE
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_FILTER_2_ETYPE_TX_FILTER_2_ETYPE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_RAB_TX_EXTRACT_TX_FILTER_2_ETYPE_TX_FILTER_2_ETYPE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_RAB_TX_EXTRACT_TX_FILTER_2_ETYPE_TX_FILTER_2_ETYPE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief RAB Tx extract filter 2 next two bytes
 *
 * \details
 * Register: \a RAB::TX_EXTRACT::TX_FILTER_2_N2B
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_EXTRACT_TX_FILTER_2_N2B(target)  VTSS_IOREG(target,0x605)

/** 
 * \brief
 * Next two bytes bit mask for Tx extract filter 2
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_FILTER_2_N2B . TX_FILTER_2_N2B_MASK
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_FILTER_2_N2B_TX_FILTER_2_N2B_MASK(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_RAB_TX_EXTRACT_TX_FILTER_2_N2B_TX_FILTER_2_N2B_MASK     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_RAB_TX_EXTRACT_TX_FILTER_2_N2B_TX_FILTER_2_N2B_MASK(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Next two bytes value to match for Tx extract filter 2
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_FILTER_2_N2B . TX_FILTER_2_N2B
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_FILTER_2_N2B_TX_FILTER_2_N2B(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_RAB_TX_EXTRACT_TX_FILTER_2_N2B_TX_FILTER_2_N2B     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_RAB_TX_EXTRACT_TX_FILTER_2_N2B_TX_FILTER_2_N2B(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief RAB Tx extract filter 3 Ethertype
 *
 * \details
 * Register: \a RAB::TX_EXTRACT::TX_FILTER_3_ETYPE
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_EXTRACT_TX_FILTER_3_ETYPE(target)  VTSS_IOREG(target,0x606)

/** 
 * \brief
 * Enable for Tx extract filter 3
 *
 * \details 
 * 0: Disabled
 * 1: Enabled
 *
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_FILTER_3_ETYPE . TX_FILTER_3_ENABLE
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_FILTER_3_ETYPE_TX_FILTER_3_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_RAB_TX_EXTRACT_TX_FILTER_3_ETYPE_TX_FILTER_3_ENABLE     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_RAB_TX_EXTRACT_TX_FILTER_3_ETYPE_TX_FILTER_3_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Ethertype value to match for Tx extract filter 3
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_FILTER_3_ETYPE . TX_FILTER_3_ETYPE
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_FILTER_3_ETYPE_TX_FILTER_3_ETYPE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_RAB_TX_EXTRACT_TX_FILTER_3_ETYPE_TX_FILTER_3_ETYPE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_RAB_TX_EXTRACT_TX_FILTER_3_ETYPE_TX_FILTER_3_ETYPE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief RAB Tx extract filter 3 next two bytes
 *
 * \details
 * Register: \a RAB::TX_EXTRACT::TX_FILTER_3_N2B
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_EXTRACT_TX_FILTER_3_N2B(target)  VTSS_IOREG(target,0x607)

/** 
 * \brief
 * Next two bytes bit mask for Tx extract filter 3
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_FILTER_3_N2B . TX_FILTER_3_N2B_MASK
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_FILTER_3_N2B_TX_FILTER_3_N2B_MASK(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_RAB_TX_EXTRACT_TX_FILTER_3_N2B_TX_FILTER_3_N2B_MASK     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_RAB_TX_EXTRACT_TX_FILTER_3_N2B_TX_FILTER_3_N2B_MASK(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Next two bytes value to match for Tx extract filter 3
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_FILTER_3_N2B . TX_FILTER_3_N2B
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_FILTER_3_N2B_TX_FILTER_3_N2B(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_RAB_TX_EXTRACT_TX_FILTER_3_N2B_TX_FILTER_3_N2B     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_RAB_TX_EXTRACT_TX_FILTER_3_N2B_TX_FILTER_3_N2B(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief RAB Tx extract buffer status
 *
 * \details
 * Register: \a RAB::TX_EXTRACT::TX_EXTRACT_STATUS
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_EXTRACT_TX_EXTRACT_STATUS(target)  VTSS_IOREG(target,0x608)

/** 
 * \brief
 * Extract buffer flush request bit.
 * This bit will self-clear when the flush has completed.
 *
 * \details 
 * 1: Flush buffer
 *
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_EXTRACT_STATUS . TX_EXTRACT_FLUSH
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_EXTRACT_STATUS_TX_EXTRACT_FLUSH(x)  VTSS_ENCODE_BITFIELD(x,31,1)
#define  VTSS_M_RAB_TX_EXTRACT_TX_EXTRACT_STATUS_TX_EXTRACT_FLUSH     VTSS_ENCODE_BITMASK(31,1)
#define  VTSS_X_RAB_TX_EXTRACT_TX_EXTRACT_STATUS_TX_EXTRACT_FLUSH(x)  VTSS_EXTRACT_BITFIELD(x,31,1)

/** 
 * \brief
 * Extract buffer packet truncated.
 * The packet at the head of the extract buffer was truncated to 128 bytes.
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_EXTRACT_STATUS . TX_EXTRACT_PKT_TRUNC
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_EXTRACT_STATUS_TX_EXTRACT_PKT_TRUNC(x)  VTSS_ENCODE_BITFIELD(x,24,1)
#define  VTSS_M_RAB_TX_EXTRACT_TX_EXTRACT_STATUS_TX_EXTRACT_PKT_TRUNC     VTSS_ENCODE_BITMASK(24,1)
#define  VTSS_X_RAB_TX_EXTRACT_TX_EXTRACT_STATUS_TX_EXTRACT_PKT_TRUNC(x)  VTSS_EXTRACT_BITFIELD(x,24,1)

/** 
 * \brief
 * Extract buffer packet length.
 * The byte length of the packet at the head of the extract buffer.
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_EXTRACT_STATUS . TX_EXTRACT_PKT_LENGTH
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_EXTRACT_STATUS_TX_EXTRACT_PKT_LENGTH(x)  VTSS_ENCODE_BITFIELD(x,16,8)
#define  VTSS_M_RAB_TX_EXTRACT_TX_EXTRACT_STATUS_TX_EXTRACT_PKT_LENGTH     VTSS_ENCODE_BITMASK(16,8)
#define  VTSS_X_RAB_TX_EXTRACT_TX_EXTRACT_STATUS_TX_EXTRACT_PKT_LENGTH(x)  VTSS_EXTRACT_BITFIELD(x,16,8)

/** 
 * \brief
 * Extract buffer packet filter.
 * The packet filter that matched the packet at the head of the extract
 * buffer.
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_EXTRACT_STATUS . TX_EXTRACT_PKT_FILTER
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_EXTRACT_STATUS_TX_EXTRACT_PKT_FILTER(x)  VTSS_ENCODE_BITFIELD(x,8,2)
#define  VTSS_M_RAB_TX_EXTRACT_TX_EXTRACT_STATUS_TX_EXTRACT_PKT_FILTER     VTSS_ENCODE_BITMASK(8,2)
#define  VTSS_X_RAB_TX_EXTRACT_TX_EXTRACT_STATUS_TX_EXTRACT_PKT_FILTER(x)  VTSS_EXTRACT_BITFIELD(x,8,2)

/** 
 * \brief
 * Extract buffer packet pending count.
 * The number of packets currently in the extract buffer.
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_EXTRACT_STATUS . TX_EXTRACT_PKT_PEND_CNT
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_EXTRACT_STATUS_TX_EXTRACT_PKT_PEND_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_RAB_TX_EXTRACT_TX_EXTRACT_STATUS_TX_EXTRACT_PKT_PEND_CNT     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_RAB_TX_EXTRACT_TX_EXTRACT_STATUS_TX_EXTRACT_PKT_PEND_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief RAB Tx extract buffer data LSB
 *
 * \details
 * Register: \a RAB::TX_EXTRACT::TX_EXTRACT_DATA_LSB
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_EXTRACT_TX_EXTRACT_DATA_LSB(target)  VTSS_IOREG(target,0x609)

/** 
 * \brief
 * Bits [31:0] of the extract buffer data
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_EXTRACT_DATA_LSB . TX_EXTRACT_DATA_LSB
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_EXTRACT_DATA_LSB_TX_EXTRACT_DATA_LSB(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_RAB_TX_EXTRACT_TX_EXTRACT_DATA_LSB_TX_EXTRACT_DATA_LSB     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_RAB_TX_EXTRACT_TX_EXTRACT_DATA_LSB_TX_EXTRACT_DATA_LSB(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief RAB Tx extract buffer data MSB
 *
 * \details
 * Register: \a RAB::TX_EXTRACT::TX_EXTRACT_DATA_MSB
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_EXTRACT_TX_EXTRACT_DATA_MSB(target)  VTSS_IOREG(target,0x60a)

/** 
 * \brief
 * Bits [63:32] of the extract buffer data
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_EXTRACT_DATA_MSB . TX_EXTRACT_DATA_MSB
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_EXTRACT_DATA_MSB_TX_EXTRACT_DATA_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_RAB_TX_EXTRACT_TX_EXTRACT_DATA_MSB_TX_EXTRACT_DATA_MSB     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_RAB_TX_EXTRACT_TX_EXTRACT_DATA_MSB_TX_EXTRACT_DATA_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief RAB Tx extract packet preamble LSB
 *
 * \details
 * Register: \a RAB::TX_EXTRACT::TX_EXTRACT_PREAMBLE_LSB
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_EXTRACT_TX_EXTRACT_PREAMBLE_LSB(target)  VTSS_IOREG(target,0x60b)

/** 
 * \brief
 * Bits [31:0] of the extract buffer packet preamble + SFD. The preamble of
 * the packet + SFD at the head of the extract buffer.	  
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_EXTRACT_PREAMBLE_LSB . TX_EXTRACT_PREAMBLE_LSB
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_EXTRACT_PREAMBLE_LSB_TX_EXTRACT_PREAMBLE_LSB(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_RAB_TX_EXTRACT_TX_EXTRACT_PREAMBLE_LSB_TX_EXTRACT_PREAMBLE_LSB     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_RAB_TX_EXTRACT_TX_EXTRACT_PREAMBLE_LSB_TX_EXTRACT_PREAMBLE_LSB(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief RAB Tx extract packet preamble MSB
 *
 * \details
 * Register: \a RAB::TX_EXTRACT::TX_EXTRACT_PREAMBLE_MSB
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_EXTRACT_TX_EXTRACT_PREAMBLE_MSB(target)  VTSS_IOREG(target,0x60c)

/** 
 * \brief
 * Bits [63:32] of the extract buffer packet preamble + SFD. The preamble
 * of the packet + SFD at the head of the extract buffer.
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_EXTRACT_PREAMBLE_MSB . TX_EXTRACT_PREAMBLE_MSB
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_EXTRACT_PREAMBLE_MSB_TX_EXTRACT_PREAMBLE_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_RAB_TX_EXTRACT_TX_EXTRACT_PREAMBLE_MSB_TX_EXTRACT_PREAMBLE_MSB     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_RAB_TX_EXTRACT_TX_EXTRACT_PREAMBLE_MSB_TX_EXTRACT_PREAMBLE_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief RAB Tx extract buffer packet count 0
 *
 * \details
 * Register: \a RAB::TX_EXTRACT::TX_EXTRACT_PKT_CNT_0
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_0(target)  VTSS_IOREG(target,0x60d)

/** 
 * \brief
 * Counts number of packets that matched extract filter 0, but were
 * discarded due to a full extract buffer.
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_0 . TX_EXTRACT_DROP_CNT_0
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_0_TX_EXTRACT_DROP_CNT_0(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_0_TX_EXTRACT_DROP_CNT_0     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_0_TX_EXTRACT_DROP_CNT_0(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Counts number of packets captured in the extract buffer that matched
 * extract filter 0
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_0 . TX_EXTRACT_PKT_CNT_0
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_0_TX_EXTRACT_PKT_CNT_0(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_0_TX_EXTRACT_PKT_CNT_0     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_0_TX_EXTRACT_PKT_CNT_0(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief RAB Tx extract buffer packet count 1
 *
 * \details
 * Register: \a RAB::TX_EXTRACT::TX_EXTRACT_PKT_CNT_1
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_1(target)  VTSS_IOREG(target,0x60e)

/** 
 * \brief
 * Counts number of packets that matched extract filter 1, but were
 * discarded due to a full extract buffer.	  
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_1 . TX_EXTRACT_DROP_CNT_1
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_1_TX_EXTRACT_DROP_CNT_1(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_1_TX_EXTRACT_DROP_CNT_1     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_1_TX_EXTRACT_DROP_CNT_1(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Counts number of packets captured in the extract buffer that matched
 * extract filter 1
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_1 . TX_EXTRACT_PKT_CNT_1
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_1_TX_EXTRACT_PKT_CNT_1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_1_TX_EXTRACT_PKT_CNT_1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_1_TX_EXTRACT_PKT_CNT_1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief RAB Tx extract buffer packet count 2
 *
 * \details
 * Register: \a RAB::TX_EXTRACT::TX_EXTRACT_PKT_CNT_2
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_2(target)  VTSS_IOREG(target,0x60f)

/** 
 * \brief
 * Counts number of packets that matched extract filter 2, but were
 * discarded due to a full extract buffer.
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_2 . TX_EXTRACT_DROP_CNT_2
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_2_TX_EXTRACT_DROP_CNT_2(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_2_TX_EXTRACT_DROP_CNT_2     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_2_TX_EXTRACT_DROP_CNT_2(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Counts number of packets captured in the extract buffer that matched
 * extract filter 2
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_2 . TX_EXTRACT_PKT_CNT_2
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_2_TX_EXTRACT_PKT_CNT_2(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_2_TX_EXTRACT_PKT_CNT_2     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_2_TX_EXTRACT_PKT_CNT_2(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief RAB Tx extract buffer packet count 3
 *
 * \details
 * Register: \a RAB::TX_EXTRACT::TX_EXTRACT_PKT_CNT_3
 *
 * @param target A \a ::vtss_target_RAB_e target
 */
#define VTSS_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_3(target)  VTSS_IOREG(target,0x610)

/** 
 * \brief
 * Counts number of packets that matched extract filter 3, but were
 * discarded due to a full extract buffer.
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_3 . TX_EXTRACT_DROP_CNT_3
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_3_TX_EXTRACT_DROP_CNT_3(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_3_TX_EXTRACT_DROP_CNT_3     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_3_TX_EXTRACT_DROP_CNT_3(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Counts number of packets captured in the extract buffer that matched
 * extract filter 3
 *
 * \details 
 * Field: ::VTSS_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_3 . TX_EXTRACT_PKT_CNT_3
 */
#define  VTSS_F_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_3_TX_EXTRACT_PKT_CNT_3(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_3_TX_EXTRACT_PKT_CNT_3     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_RAB_TX_EXTRACT_TX_EXTRACT_PKT_CNT_3_TX_EXTRACT_PKT_CNT_3(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


#endif /* _VTSS_DAYTONA_REGS_RAB_H_ */
