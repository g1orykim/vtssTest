#ifndef _VTSS_DAYTONA_REGS_MAC10G_H_
#define _VTSS_DAYTONA_REGS_MAC10G_H_

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
 * Target: \a MAC10G
 *
 * \see vtss_target_MAC10G_e
 *
 * 10G MAC
 *
 ***********************************************************************/

/**
 * Register Group: \a MAC10G::DEV_CFG_STATUS
 *
 * 10G MAC configuration and status registers
 */


/** 
 * \brief MAC 10G clock/reset control register
 *
 * \details
 * Register: \a MAC10G::DEV_CFG_STATUS::MAC_RST_CTRL
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL(target)  VTSS_IOREG(target,0x0)

/** 
 * \brief
 * Link state.
 * This is a combination of LFS_LOCAL_FAULT and LFS_REMOTE_FAULT.
 *
 * \details 
 * 0: Link state is DOWN
 * 1: Link state is UP
 *
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL . LINK_STATE
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_LINK_STATE(x)  VTSS_ENCODE_BITFIELD(x,28,1)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_LINK_STATE     VTSS_ENCODE_BITMASK(28,1)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_LINK_STATE(x)  VTSS_EXTRACT_BITFIELD(x,28,1)

/** 
 * \brief
 * Used for modes that do not use a PCS (XGMII data comes directly from
 * another block), ignore the PCS SYNC state for generating the LINK_STATE
 * bit.
 *
 * \details 
 * 0: PCS block attached, honor the PCS sync state bits
 * 1: No PCS block attached, ignore the PCS sync 
 *
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL . IGNORE_PCS_SYNC_STATE
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_IGNORE_PCS_SYNC_STATE(x)  VTSS_ENCODE_BITFIELD(x,21,1)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_IGNORE_PCS_SYNC_STATE     VTSS_ENCODE_BITMASK(21,1)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_IGNORE_PCS_SYNC_STATE(x)  VTSS_EXTRACT_BITFIELD(x,21,1)

/** 
 * \brief
 * Used to select if the attached PCS is a XAUI PCS or not.
 *
 * \details 
 * 0: Not a XAUI PCS
 * 1: XAUI PCS
 *
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL . XAUI_PCS_SEL
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_XAUI_PCS_SEL(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_XAUI_PCS_SEL     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_XAUI_PCS_SEL(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \brief
 * Select the PMA clock or the XGMII clock for the Rx portion of the MAC.
 *
 * \details 
 * 0: Use the XGMII clock for the Rx MAC
 * 1: Use the PMA clock for the Rx MAC
 *
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL . MAC_PMA_RX_CLK_SEL
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_MAC_PMA_RX_CLK_SEL(x)  VTSS_ENCODE_BITFIELD(x,19,1)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_MAC_PMA_RX_CLK_SEL     VTSS_ENCODE_BITMASK(19,1)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_MAC_PMA_RX_CLK_SEL(x)  VTSS_EXTRACT_BITFIELD(x,19,1)

/** 
 * \brief
 * Enable a bypass path that takes XGMII TX bypass data and control clocked
 * with the tx_xgmii_clk, and routes it to the PMA data and control bypass
 * path. Data is routed through a retimer and clocked with the tx_pma_clk
 * before leaving the block. This bypass effectively bypasses the entire
 * MAC and PCS blocks.
 *
 * \details 
 * 0: Normal operation
 * 1: Bypass the entire MAC and PCS blocks in the Tx direction
 *
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL . PMA_TX_BYPASS_SEL
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_PMA_TX_BYPASS_SEL(x)  VTSS_ENCODE_BITFIELD(x,18,1)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_PMA_TX_BYPASS_SEL     VTSS_ENCODE_BITMASK(18,1)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_PMA_TX_BYPASS_SEL(x)  VTSS_EXTRACT_BITFIELD(x,18,1)

/** 
 * \brief
 * Enable a bypass path that takes PMA Rx data clocked with the rx_pma_clk
 * and routes it to the XGMII bypass path.
 *
 * \details 
 * 0: Normal operation
 * 1: Route the Rx PMA data and clock to the XGMII bypass path
 *
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL . PMA_RX_BYPASS_SEL
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_PMA_RX_BYPASS_SEL(x)  VTSS_ENCODE_BITFIELD(x,17,1)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_PMA_RX_BYPASS_SEL     VTSS_ENCODE_BITMASK(17,1)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_PMA_RX_BYPASS_SEL(x)  VTSS_EXTRACT_BITFIELD(x,17,1)

/** 
 * \brief
 * Select the source for the Tx XGMII outputs.
 *
 * \details 
 * 0: KERNEL for normal operation
 * 1: Bypass path
 *
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL . MAC_TX_SOURCE_SEL
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_MAC_TX_SOURCE_SEL(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_MAC_TX_SOURCE_SEL     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_MAC_TX_SOURCE_SEL(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Select the source of XGMII data for the Rx path.
 *
 * \details 
 * 0: Normal path
 * 1: Monitoring path
 * 2: GFP monitoring path
 * 3: Reserved
 *
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL . MAC_RX_SOURCE_SEL
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_MAC_RX_SOURCE_SEL(x)  VTSS_ENCODE_BITFIELD(x,10,2)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_MAC_RX_SOURCE_SEL     VTSS_ENCODE_BITMASK(10,2)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_MAC_RX_SOURCE_SEL(x)  VTSS_EXTRACT_BITFIELD(x,10,2)

/** 
 * \brief
 * Enables the Tx clocks to the MAC. When the clock is shut off, it forces
 * the Tx portion of the MAC to be in reset. Bypass paths and the CSR
 * register block are always enabled.
 *
 * \details 
 * 0: Disable MAC Tx clocks
 * 1: Enable MAC Tx clocks
 *
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL . MAC_TX_CLOCK_EN
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_MAC_TX_CLOCK_EN(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_MAC_TX_CLOCK_EN     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_MAC_TX_CLOCK_EN(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Enables the Rx clocks to the MAC. When the clock is shut off, it forces
 * the Rx portion of the MAC to be in reset. Bypass paths and the CSR
 * register block are always enabled.
 *
 * \details 
 * 0: Disable MAC Rx clocks
 * 1: Enable MAC Rx clocks
 *
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL . MAC_RX_CLOCK_EN
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_MAC_RX_CLOCK_EN(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_MAC_RX_CLOCK_EN     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_MAC_RX_CLOCK_EN(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Reset MAC Tx clock domains of the device.
 * Note: MAC_TX_RST is NOT a one-shot operation. The MAC Tx clock domain
 * remains reset until a 0 is written to MAC_TX_RST.
 *
 * \details 
 * 0: Do not reset MAC Tx clock domain
 * 1: Reset MAC Tx clock domain
 *
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL . MAC_TX_RST
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_MAC_TX_RST(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_MAC_TX_RST     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_MAC_TX_RST(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Reset MAC Rx clock domains of the device.
 * Note: MAC_RX_RST is NOT a one-shot operation. The MAC Rx clock domain
 * remains reset until a 0 is written to MAC_RX_RST.
 *
 * \details 
 * 0: Do not reset MAC Rx clock domain
 * 1: Reset MAC Rx clock domain
 *
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL . MAC_RX_RST
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_MAC_RX_RST(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_MAC_RX_RST     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_MAC_RST_CTRL_MAC_RX_RST(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief MAC 10G miscellaneous delta bit register
 *
 * \details
 * Clear the delta bits by writing a 1 in the relevant bit groups
 *
 * Register: \a MAC10G::DEV_CFG_STATUS::LINK_DELTA
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_CFG_STATUS_LINK_DELTA(target)  VTSS_IOREG(target,0x1)

/** 
 * \brief
 * Link delta bit
 *
 * \details 
 * 1: Link state has changed
 * 0: Link state has not changed
 *
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_LINK_DELTA . LINK_DELTA
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_LINK_DELTA_LINK_DELTA(x)  VTSS_ENCODE_BITFIELD(x,24,1)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_LINK_DELTA_LINK_DELTA     VTSS_ENCODE_BITMASK(24,1)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_LINK_DELTA_LINK_DELTA(x)  VTSS_EXTRACT_BITFIELD(x,24,1)


/** 
 * \brief Sticky bit interrupt enables
 *
 * \details
 * Register: \a MAC10G::DEV_CFG_STATUS::LINK_INT_MASK
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_CFG_STATUS_LINK_INT_MASK(target)  VTSS_IOREG(target,0x2)

/** 
 * \brief
 * Interrupt mask for LINK_DELTA
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled

 *
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_LINK_INT_MASK . LINK_DELTA_INT_MASK
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_LINK_INT_MASK_LINK_DELTA_INT_MASK(x)  VTSS_ENCODE_BITFIELD(x,24,1)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_LINK_INT_MASK_LINK_DELTA_INT_MASK     VTSS_ENCODE_BITMASK(24,1)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_LINK_INT_MASK_LINK_DELTA_INT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,24,1)


/** 
 * \brief Transmit pause frame control
 *
 * \details
 * Register: \a MAC10G::DEV_CFG_STATUS::PAUSE_TX_FRAME_CONTROL
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL(target)  VTSS_IOREG(target,0x3)

/** 
 * \brief
 * Pause value to be used when generating pause frames, except XON frames
 * in mode 2.
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL . MAC_TX_PAUSE_VALUE
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL_MAC_TX_PAUSE_VALUE(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL_MAC_TX_PAUSE_VALUE     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL_MAC_TX_PAUSE_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Used to force the pause frame generator to send duplicate pause frames
 * instead of one when enabled. This mode is used to overcome a problem
 * with Smartbits inability to detect every pause frame.

 *
 * \details 
 * 0: Normal mode
 * 1: Send duplicate pause frames
 *
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL . MAC_TX_PAUSE_REPL_MODE
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL_MAC_TX_PAUSE_REPL_MODE(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL_MAC_TX_PAUSE_REPL_MODE     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL_MAC_TX_PAUSE_REPL_MODE(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Indicates the current value of the pause frame signal.
 * For the active status bit, see PAUSE_STATE.
 *
 * \details 
 * 0: Below the threshold, do not generate pause frames
 * 1: Above the threshold, generate pause frames if enabled
 *
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL . MAC_TX_PAUSE_GEN
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL_MAC_TX_PAUSE_GEN(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL_MAC_TX_PAUSE_GEN     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL_MAC_TX_PAUSE_GEN(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Force a pause frame with MAC_TX_PAUSE_VALUE (the number of pause quanta)
 * to be sent.
 *
 * \details 
 * 0: Normal operation
 * 1: Force a single pause frame to be sent, self clearing
 *
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL . MAC_TX_PAUSE_FRC_FRAME
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL_MAC_TX_PAUSE_FRC_FRAME(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL_MAC_TX_PAUSE_FRC_FRAME     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL_MAC_TX_PAUSE_FRC_FRAME(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Pause frame generator mode.
 *
 * \details 
 * 0: Pause frame generation is disabled
 * 1: Pause frames are generated only with the pause-value specified in the
 * MAC_PAUSE_VALUE register
 * 2: XON mode, pause frames are generated with a pause-value of 0 are
 * generated when traffic is to be restarted, in addition to generating
 * pause frames as in mode 1
 * 3: Reserved
 *
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL . MAC_TX_PAUSE_MODE
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL_MAC_TX_PAUSE_MODE(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL_MAC_TX_PAUSE_MODE     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL_MAC_TX_PAUSE_MODE(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Transmit pause frame control part 2
 *
 * \details
 * Register: \a MAC10G::DEV_CFG_STATUS::PAUSE_TX_FRAME_CONTROL_2
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL_2(target)  VTSS_IOREG(target,0x4)

/** 
 * \brief
 * Pause frame generation interval.
 * Each count in the pause frame interval value corresponds to one cycle of
 * the MAC clock (PCS clock divided by 2), typically 156.25 MHz (6.4 ns
 * period).
 * The interval is counted from the end of one pause frame to the beginning
 * of the next (assuming no other Tx traffic). In Tx pause mode 2 only, the
 * internal pause interval timer will be cleared whenever an XON pause
 * frame is sent.
 * Note: the pause interval value of 0xffff gives the same pause frame
 * interval as the pause interval value of 0xfffe. Also, do not use a value
 * of 0.
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL_2 . MAC_TX_PAUSE_INTERVAL
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL_2_MAC_TX_PAUSE_INTERVAL(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL_2_MAC_TX_PAUSE_INTERVAL     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_PAUSE_TX_FRAME_CONTROL_2_MAC_TX_PAUSE_INTERVAL(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Receive pause frame control
 *
 * \details
 * Register: \a MAC10G::DEV_CFG_STATUS::PAUSE_RX_FRAME_CONTROL
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_CFG_STATUS_PAUSE_RX_FRAME_CONTROL(target)  VTSS_IOREG(target,0x5)

/** 
 * \brief
 * Controls if pause frames are dropped in the pause frame detector or
 * passed along
 *
 * \details 
 * 0: Drop pause frames
 * 1: Pass along pause frames
 *
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_PAUSE_RX_FRAME_CONTROL . MAC_RX_PAUSE_FRAME_DROP_DIS
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_PAUSE_RX_FRAME_CONTROL_MAC_RX_PAUSE_FRAME_DROP_DIS(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_PAUSE_RX_FRAME_CONTROL_MAC_RX_PAUSE_FRAME_DROP_DIS     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_PAUSE_RX_FRAME_CONTROL_MAC_RX_PAUSE_FRAME_DROP_DIS(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Rx pause frame reaction
 *
 * \details 
 * 0: Disable pause frame reaction
 * 1: Enable pause frame reaction
 *
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_PAUSE_RX_FRAME_CONTROL . MAC_RX_PAUSE_MODE
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_PAUSE_RX_FRAME_CONTROL_MAC_RX_PAUSE_MODE(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_PAUSE_RX_FRAME_CONTROL_MAC_RX_PAUSE_MODE     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_PAUSE_RX_FRAME_CONTROL_MAC_RX_PAUSE_MODE(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Pause detector state register
 *
 * \details
 * Register: \a MAC10G::DEV_CFG_STATUS::PAUSE_STATE
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_CFG_STATUS_PAUSE_STATE(target)  VTSS_IOREG(target,0x6)

/** 
 * \brief
 * Pause state indicator
 * Interface is paused when the pause timer is a non-zero value
 *
 * \details 
 * 0: Not paused
 * 1: Paused
 *
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_PAUSE_STATE . PAUSE_STATE
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_PAUSE_STATE_PAUSE_STATE(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_PAUSE_STATE_PAUSE_STATE     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_PAUSE_STATE_PAUSE_STATE(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief MAC address LSB
 *
 * \details
 * Register: \a MAC10G::DEV_CFG_STATUS::MAC_ADDRESS_LSB
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_CFG_STATUS_MAC_ADDRESS_LSB(target)  VTSS_IOREG(target,0x7)

/** 
 * \brief
 * Lower 32 bits of the MAC address
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_MAC_ADDRESS_LSB . MAC_ADDRESS_LSB
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_MAC_ADDRESS_LSB_MAC_ADDRESS_LSB(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_MAC_ADDRESS_LSB_MAC_ADDRESS_LSB     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_MAC_ADDRESS_LSB_MAC_ADDRESS_LSB(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief MAC address MSB
 *
 * \details
 * Register: \a MAC10G::DEV_CFG_STATUS::MAC_ADDRESS_MSB
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_CFG_STATUS_MAC_ADDRESS_MSB(target)  VTSS_IOREG(target,0x8)

/** 
 * \brief
 * Upper 16 bits of the MAC address
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_MAC_ADDRESS_MSB . MAC_ADDRESS_MSB
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_MAC_ADDRESS_MSB_MAC_ADDRESS_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_MAC_ADDRESS_MSB_MAC_ADDRESS_MSB     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_MAC_ADDRESS_MSB_MAC_ADDRESS_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Clock PPM compensation control
 *
 * \details
 * Register: \a MAC10G::DEV_CFG_STATUS::PPM_COMPENSATION_CONTROL
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_CFG_STATUS_PPM_COMPENSATION_CONTROL(target)  VTSS_IOREG(target,0x9)

/** 
 * \brief
 * Timer to determine how often a column of idles should be removed from
 * the transmit direction. Each count value is one MAC_TX_CLK period
 * (nominally 6.4ns). The value should never correspond to less than one
 * maximum length frame (as configured in the MAX_LEN configuration field).
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_PPM_COMPENSATION_CONTROL . PPM_CORRECTION_PERIOD
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_PPM_COMPENSATION_CONTROL_PPM_CORRECTION_PERIOD(x)  VTSS_ENCODE_BITFIELD(x,16,12)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_PPM_COMPENSATION_CONTROL_PPM_CORRECTION_PERIOD     VTSS_ENCODE_BITMASK(16,12)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_PPM_COMPENSATION_CONTROL_PPM_CORRECTION_PERIOD(x)  VTSS_EXTRACT_BITFIELD(x,16,12)

/** 
 * \brief
 * Control deletion of IPG idle columns to correct for +/-200 PPM
 * differences in clocking in the Tx direction
 *
 * \details 
 * 0: Disable idle column deletion
 * 1: Enable idle column deletion
 *
 * Field: ::VTSS_MAC10G_DEV_CFG_STATUS_PPM_COMPENSATION_CONTROL . PPM_CORRECTION_ENA
 */
#define  VTSS_F_MAC10G_DEV_CFG_STATUS_PPM_COMPENSATION_CONTROL_PPM_CORRECTION_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_MAC10G_DEV_CFG_STATUS_PPM_COMPENSATION_CONTROL_PPM_CORRECTION_ENA     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_MAC10G_DEV_CFG_STATUS_PPM_COMPENSATION_CONTROL_PPM_CORRECTION_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/**
 * Register Group: \a MAC10G::MAC_CFG_STATUS
 *
 * 10G MAC configuration and status registers
 */


/** 
 * \brief MAC enable register
 *
 * \details
 * Register: \a MAC10G::MAC_CFG_STATUS::MAC_ENA_CFG
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_MAC_CFG_STATUS_MAC_ENA_CFG(target)  VTSS_IOREG(target,0x40)

/** 
 * \brief
 * Receiver enable
 *
 * \details 
 * 0: Receiver disabled
 * 1: Receiver enabled
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_ENA_CFG . RX_ENA
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_ENA_CFG_RX_ENA(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_ENA_CFG_RX_ENA     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_ENA_CFG_RX_ENA(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Transmitter enable
 *
 * \details 
 * 0: Transmitter disabled
 * 1: Transmitter enabled
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_ENA_CFG . TX_ENA
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_ENA_CFG_TX_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_ENA_CFG_TX_ENA     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_ENA_CFG_TX_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Mode configuration register
 *
 * \details
 * Register: \a MAC10G::MAC_CFG_STATUS::MAC_MODE_CFG
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_MAC_CFG_STATUS_MAC_MODE_CFG(target)  VTSS_IOREG(target,0x41)

/** 
 * \brief
 * Adjust mode.
 * Always write to 1.
 *
 * \details 
 * 0: Reserved, do not use
 * 1: Normal operation
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_MODE_CFG . TX_PPM_ADJUST_MODE
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_MODE_CFG_TX_PPM_ADJUST_MODE(x)  VTSS_ENCODE_BITFIELD(x,24,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_MODE_CFG_TX_PPM_ADJUST_MODE     VTSS_ENCODE_BITMASK(24,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_MODE_CFG_TX_PPM_ADJUST_MODE(x)  VTSS_EXTRACT_BITFIELD(x,24,1)

/** 
 * \brief
 * Force the FCS to be overwritten on all packets
 *
 * \details 
 * 0: FCS not overwritten
 * 1: FCS overwritten on all packets
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_MODE_CFG . FCS_OVERWRITE_ENA
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_MODE_CFG_FCS_OVERWRITE_ENA(x)  VTSS_ENCODE_BITFIELD(x,22,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_MODE_CFG_FCS_OVERWRITE_ENA     VTSS_ENCODE_BITMASK(22,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_MODE_CFG_FCS_OVERWRITE_ENA(x)  VTSS_EXTRACT_BITFIELD(x,22,1)

/** 
 * \brief
 * Allow short packets (as small as 17 bytes) for MPLS support. Setting
 * this bit also disables the reaction of pause frames.
 *
 * \details 
 * 0: Normal operation, minimum packet length is 64 bytes
 * 1: Allow short packets
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_MODE_CFG . ALLOW_SHORT_PACKETS
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_MODE_CFG_ALLOW_SHORT_PACKETS(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_MODE_CFG_ALLOW_SHORT_PACKETS     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_MODE_CFG_ALLOW_SHORT_PACKETS(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \brief
 * Determines if the preamble needs to be added in the Tx direction or used
 * from RAB.
 *
 * \details 
 * 0: Preamble was not stored in RAB, use preamble from MAC_TX_PREAMBLE
 * register
 * 1: Preamble received from RAB
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_MODE_CFG . MAC_TX_PREAMBLE_MODE
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_MODE_CFG_MAC_TX_PREAMBLE_MODE(x)  VTSS_ENCODE_BITFIELD(x,18,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_MODE_CFG_MAC_TX_PREAMBLE_MODE     VTSS_ENCODE_BITMASK(18,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_MODE_CFG_MAC_TX_PREAMBLE_MODE(x)  VTSS_EXTRACT_BITFIELD(x,18,1)

/** 
 * \brief
 * Configure preamble format
 * Supports standard preamble = 0xFB5555555555555D
 * Host Interface Header (HIH) located before the SFD
 * Host Interface Header (HIH) located after the SFD - default CRC
 * Host Interface Header (HIH) located after the SFD - HIH included in CRC
 * For any mode other than 0, MAC_RX_PREAMBLE_MODE and MAC_TX_PREAMBLE_MODE
 * must be set to 1 to have the preamble stored/received from the RAB.
 *
 * \details 
 * 0: Standard preamble
 * 1: Reserved
 * 2: HIH before SFD - default CRC = 0xFB H0 H1 H2 H3 0x55 0x55 0xD5
 * 3: HIH after SFD - default CRC = 0xFB 0x55 0x55 0xD5 H0 H1 H2 H3
 * 4: HIH after SFD - HIH in CRC = 0xFB 0x55 0x55 0xD5 H0 H1 H2 H3
 * 5: 4 byte preamble, used for 1588 passive monitoring ONLY
 * 6 - 7: Reserved
 * HIH is 4 bytes: H0 H1 H2 H3
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_MODE_CFG . MAC_PREAMBLE_CFG
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_MODE_CFG_MAC_PREAMBLE_CFG(x)  VTSS_ENCODE_BITFIELD(x,12,3)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_MODE_CFG_MAC_PREAMBLE_CFG     VTSS_ENCODE_BITMASK(12,3)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_MODE_CFG_MAC_PREAMBLE_CFG(x)  VTSS_EXTRACT_BITFIELD(x,12,3)

/** 
 * \brief
 * Pace/Stretch mode enable.
 * Enables the open loop rate control scheme used to support OC192c based
 * PHYs. The average bandwidth will be kept below 9.58 Gbps (including
 * preamble and SFD).
 *
 * \details 
 * 0: Pace/Stretch mode disabled
 * 1: Pace/Stretch mode enabled
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_MODE_CFG . PACE_MODE_ENA
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_MODE_CFG_PACE_MODE_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_MODE_CFG_PACE_MODE_ENA     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_MODE_CFG_PACE_MODE_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief MAC Tx preamble, MSB portion
 *
 * \details
 * Register: \a MAC10G::MAC_CFG_STATUS::MAC_TX_PREAMBLE_MSB
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_MAC_CFG_STATUS_MAC_TX_PREAMBLE_MSB(target)  VTSS_IOREG(target,0x42)

/** 
 * \brief
 * Preamble used when the MAC inserts it in the Tx direction, MSB portion
 *
 * \details 
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_TX_PREAMBLE_MSB . MAC_TX_PREAMBLE_MSB
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_TX_PREAMBLE_MSB_MAC_TX_PREAMBLE_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_TX_PREAMBLE_MSB_MAC_TX_PREAMBLE_MSB     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_TX_PREAMBLE_MSB_MAC_TX_PREAMBLE_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief MAC Tx preamble, LSB portion
 *
 * \details
 * Register: \a MAC10G::MAC_CFG_STATUS::MAC_TX_PREAMBLE_LSB
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_MAC_CFG_STATUS_MAC_TX_PREAMBLE_LSB(target)  VTSS_IOREG(target,0x43)

/** 
 * \brief
 * Preamble used when the MAC inserts it in the Tx direction, LSB portion
 *
 * \details 
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_TX_PREAMBLE_LSB . MAC_TX_PREAMBLE_LSB
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_TX_PREAMBLE_LSB_MAC_TX_PREAMBLE_LSB(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_TX_PREAMBLE_LSB_MAC_TX_PREAMBLE_LSB     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_TX_PREAMBLE_LSB_MAC_TX_PREAMBLE_LSB(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Maximum length configuration register
 *
 * \details
 * Register: \a MAC10G::MAC_CFG_STATUS::MAC_MAXLEN_CFG
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_MAC_CFG_STATUS_MAC_MAXLEN_CFG(target)  VTSS_IOREG(target,0x44)

/** 
 * \brief
 * Configures whether the max length check takes the number of Q tags into
 * consideration when assessing if a frame is too long. If asserted, 
 *  - 4 bytes will be added to MAX_LEN for single tagged frame
 *  - 8 bytes will be added for MAX_LEN double tagged frame
 *
 * \details 
 * 0: Only check max frame length against MAX_LEN
 * 1: Add 4/8 bytes to MAX_LEN when checking for max frame length
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_MAXLEN_CFG . MAX_LEN_TAG_CHK
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_MAXLEN_CFG_MAX_LEN_TAG_CHK(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_MAXLEN_CFG_MAX_LEN_TAG_CHK     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_MAXLEN_CFG_MAX_LEN_TAG_CHK(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * The maximum frame length accepted by the receive module. If the length
 * is exceeded, this is indicated in the statistics engine (LONG_FRAME).
 * The maximum length is automatically adjusted to accommodate maximum
 * sized frames containing a VLAN tag - given that the MAC is configured to
 * be VLAN aware (default):
 * The maximum allowable MTU size = 10240 Bytes. This includes all
 * encapsulations and TAGs. 
 *
 * \details 
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_MAXLEN_CFG . MAX_LEN
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_MAXLEN_CFG_MAX_LEN(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_MAXLEN_CFG_MAX_LEN     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_MAXLEN_CFG_MAX_LEN(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief VLAN/Service tag configuration register
 *
 * \details
 * Register: \a MAC10G::MAC_CFG_STATUS::MAC_TAGS_CFG
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_MAC_CFG_STATUS_MAC_TAGS_CFG(target)  VTSS_IOREG(target,0x45)

/** 
 * \brief
 * This field defines which value is regarded as a VLAN/Service tag -
 * besides 0x8100. The value is used for all ALL tag positions. A double
 * tagged frame can have the following tag values:
 * (INNER_TAG, OUTER_TAG):
 * (0x8100, 0x8100), (0x8100, TAG_ID), or (TAG_ID, TAG_ID)
 * Single tagged frame can have the following tag values, regardless of the
 * DOUBLE_TAG_ENA setting: 
 * 0x8100 or TAG_ID. TAG_ID should therefore normally be set to 0x8100 if
 * DOUBLE_TAG_ENA = 0
 *
 * \details 
 * 0x8100: Standard Ethernet bridge ethertype (C-tag)
 * 0x88A8: Provider bridge ethertype (S-tag)
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_TAGS_CFG . TAG_ID
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_TAGS_CFG_TAG_ID(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_TAGS_CFG_TAG_ID     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_TAGS_CFG_TAG_ID(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Provider bridge (double tag) enable.
 * VLAN_AWR_ENA must be set to 1 when double tag is enabled (DOUBLE_TAG_ENA
 * = 1).
 *
 * \details 
 * 0: MAC looks only for a single tag (0x8100 or TAG_ID)
 * 1: MAC looks for two tags according to encoding of TAG_ID
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_TAGS_CFG . DOUBLE_TAG_ENA
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_TAGS_CFG_DOUBLE_TAG_ENA(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_TAGS_CFG_DOUBLE_TAG_ENA     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_TAGS_CFG_DOUBLE_TAG_ENA(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * VLAN aware.
 * Enables the MAC to work in a VLAN aware environment.
 *
 * \details 
 * 0: VLAN disabled
 * 1: VLAN enabled
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_TAGS_CFG . VLAN_AWR_ENA
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_TAGS_CFG_VLAN_AWR_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_TAGS_CFG_VLAN_AWR_ENA     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_TAGS_CFG_VLAN_AWR_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Advanced check feature configuration register
 *
 * \details
 * Register: \a MAC10G::MAC_CFG_STATUS::MAC_ADV_CHK_CFG
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG(target)  VTSS_IOREG(target,0x46)

/** 
 * \brief
 * Disables the counting of packets with FCS errors. Affects both the
 * RX_CRC_ERR_CNT and RX_BAD_BYTES_CNT counters along with their
 * performance monitor versions.
 *
 * \details 
 * 0: Counter FCS errors in packets
 * 1: Disable the counting of FCS errors in packets
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG . FCS_ERROR_COUNT_DIS
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_FCS_ERROR_COUNT_DIS(x)  VTSS_ENCODE_BITFIELD(x,29,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_FCS_ERROR_COUNT_DIS     VTSS_ENCODE_BITMASK(29,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_FCS_ERROR_COUNT_DIS(x)  VTSS_EXTRACT_BITFIELD(x,29,1)

/** 
 * \brief
 * Enables the passing of data and control frames (other than pause frames)
 * with invalid FCS.
 * Pause frames with invalid FCS will be reacted upon when this bit is set,
 * it is therefore HIGHLY advisable to not send pause frames into the MAC
 * when using this mode is there is any chance that they will have bad FCS.
 * Frames with invalid FCS will be counted (if FCS_ERROR_COUNT_DIS is 0) as
 * errored frames regardless of the setting of this bit.
 *
 * \details 
 * 0: Normal operation, frames with bad FCS are marked to be aborted
 * 1: Frames with bad FCS are not marked to be aborted and are passed along
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG . FCS_ERROR_DISCARD_DIS
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_FCS_ERROR_DISCARD_DIS(x)  VTSS_ENCODE_BITFIELD(x,28,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_FCS_ERROR_DISCARD_DIS     VTSS_ENCODE_BITMASK(28,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_FCS_ERROR_DISCARD_DIS(x)  VTSS_EXTRACT_BITFIELD(x,28,1)

/** 
 * \brief
 * Extended end of packet check.
 * Specifies the requirement for the Rx column when holding an EOP
 * character.
 *
 * \details 
 * 0: The values of the remaining Rx lanes of a column holding an EOP are
 * ignored. e.g. if lane 1 holds an EOP, the value of lanes 2 and 3 are
 * ignored
 * 1: A received frame is error-marked if an error character is received in
 * any lane of the column holding the EOP character. e.g. if lane 1 holds
 * an EOP, the frame is error-marked if lanes 0, 2, or 3 holds an error
 * character.
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG . EXT_EOP_CHK_ENA
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_EXT_EOP_CHK_ENA(x)  VTSS_ENCODE_BITFIELD(x,24,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_EXT_EOP_CHK_ENA     VTSS_ENCODE_BITMASK(24,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_EXT_EOP_CHK_ENA(x)  VTSS_EXTRACT_BITFIELD(x,24,1)

/** 
 * \brief
 * Extended start of packet check enable.
 * Specifies the requirement for the Rx column prior to SOP character.
 *
 * \details 
 * 0: Value of Rx column at the XGMII interface prior to a SOP character is
 * ignored
 * 1: An IDLE column at the XGMII interface must be received prior to a SOP
 * character for the MAC to detect a start of frame
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG . EXT_SOP_CHK_ENA
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_EXT_SOP_CHK_ENA(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_EXT_SOP_CHK_ENA     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_EXT_SOP_CHK_ENA(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \brief
 * Start-of-frame-delimiter check enable
 * Specifies the requirement for a successful frame reception. When
 * disabled (=0), MAC10G will assume that preamble is 8 bytes (incl. SOP &
 * SFD) when SOP is received. No checking of SFD is carried out.
 * When enabled (=1), MAC10G will search for SFD in lane 3/7 after
 * reception of SOP, before accepting frame data. MAC10G will search for
 * SFD until SFD is found or a control character is encountered.
 *
 * \details 
 * 0: Skip SFD check
 * 1: Strict SFD check enabled, i.e. the SFD must be D5 for a successful
 * frame reception
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG . SFD_CHK_ENA
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_SFD_CHK_ENA(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_SFD_CHK_ENA     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_SFD_CHK_ENA(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Preamble shrink check disable
 * The MAC can be setup to discard a frame, if a SFD is received in lane 3
 * of the column containing the SOP. If stacking tags must be supported,
 * the MAC must accept any data byte at this position.
 *
 * \details 
 * 0: Frames with a SFD in lane 3 of the column containing the SOP are
 * discarded
 * 1: Frames with a SFD in lane 3 of the column containing the SOP are NOT
 * discarded
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG . PRM_SHK_CHK_DIS
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_PRM_SHK_CHK_DIS(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_PRM_SHK_CHK_DIS     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_PRM_SHK_CHK_DIS(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Preamble check enable
 * Specifies the preamble requirement for a successful frame reception
 *
 * \details 
 * 0: Skip preamble check
 * A SOP control character is sufficient for a successful frame reception.
 * The minimum allowed preamble size is still 8 bytes (incl. SOP and SFD)
 * but the preamble bytes between the SOP and SFD can have any data value.
 * 1: Strict preamble check enabled,
 * The last 6 bytes of a preamble - prior to the SFD - must all be equal to
 * 0x55 for a successful frame reception. For preambles larger than 8
 * bytes, only the last 6 preamble bytes prior to the SFD are checked when
 * this bit is set to 1.
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG . PRM_CHK_ENA
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_PRM_CHK_ENA(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_PRM_CHK_ENA     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_PRM_CHK_ENA(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Out-of-range error check enable
 * Determines whether or not a received frame should be discarded if the
 * frame length field is out of range
 *
 * \details 
 * 0: Out-of-range errors are ignored
 * 1: A frame is discarded if the frame length field value is out of range
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG . OOR_ERR_ENA
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_OOR_ERR_ENA(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_OOR_ERR_ENA     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_OOR_ERR_ENA(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * In-range error check enable
 * Determines whether or not a received frame should be discarded if the
 * frame length does not match the frame PDU size
 *
 * \details 
 * 0: Frames that have a frame length field inconsistent with the actual
 * frame length are not error-marked
 * 1: Frames with inconsistent frame length fields are error marked and
 * will be discarded by the RAB
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG . INR_ERR_ENA
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_INR_ERR_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_INR_ERR_ENA     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_INR_ERR_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Link fault signaling register
 *
 * \details
 * Register: \a MAC10G::MAC_CFG_STATUS::MAC_LFS_CFG
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_MAC_CFG_STATUS_MAC_LFS_CFG(target)  VTSS_IOREG(target,0x47)

/** 
 * \brief
 * Causes local fault messages to be generated in the Tx direction.
 *
 * \details 
 * 0: Normal operation
 * 1: Force local fault messages
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_LFS_CFG . LOCAL_FAULT_FORCE_ENA
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_LFS_CFG_LOCAL_FAULT_FORCE_ENA(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_LFS_CFG_LOCAL_FAULT_FORCE_ENA     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_LFS_CFG_LOCAL_FAULT_FORCE_ENA(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * This bit causes remote fault messages to be generated in the Tx
 * direction. LFS_UNIDIR_ENA still determines if traffic stops or not.
 *
 * \details 
 * 0: Normal operation
 * 1: Force remote fault messages
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_LFS_CFG . REMOTE_FAULT_FORCE_ENA
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_LFS_CFG_REMOTE_FAULT_FORCE_ENA(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_LFS_CFG_REMOTE_FAULT_FORCE_ENA     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_LFS_CFG_REMOTE_FAULT_FORCE_ENA(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * LFS unidirectional mode enable. Implementation of 802.3 clause 66. When
 * asserted, this enables MAC to transmit data during reception of local
 * fault and remote fault ordered sets from the PHY. 
 * When in unidirectional mode:
 * When receiving LF, frames are transmitted separated by RF ordered sets
 * When receiving RF, frames are transmitted separated by IDLE symbols
 *
 * \details 
 * 0: LFS unidirectional mode is disabled
 * 1: LFS unidirectional mode is enabled
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_LFS_CFG . LFS_UNIDIR_ENA
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_LFS_CFG_LFS_UNIDIR_ENA(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_LFS_CFG_LFS_UNIDIR_ENA     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_LFS_CFG_LFS_UNIDIR_ENA(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Link fault signaling mode enable
 * If enabled, the transmitter reacts on received link fault indications
 *
 * \details 
 * 0: Ignore link faults detected by the MAC receiver module
 * 1: React on detected link faults and transmit the appropriate sequence
 * ordered set
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_LFS_CFG . LFS_MODE_ENA
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_LFS_CFG_LFS_MODE_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_LFS_CFG_LFS_MODE_ENA     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_LFS_CFG_LFS_MODE_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Miscellaneous configuration register
 *
 * \details
 * Register: \a MAC10G::MAC_CFG_STATUS::MAC_LB_CFG
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_MAC_CFG_STATUS_MAC_LB_CFG(target)  VTSS_IOREG(target,0x48)

/** 
 * \brief
 * Enables loopback from Tx to Rx in the device. 
 * The MAC Rx clock is automatically set equal to the MAC Tx clock when the
 * loop back is enabled.
 *
 * \details 
 * 0: TX_RX loopback disabled at XGMII interface
 * 1: TX_RX loopback enabled at XGMII interface
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_LB_CFG . XGMII_TX_RX_LB_ENA
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_LB_CFG_XGMII_TX_RX_LB_ENA(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_LB_CFG_XGMII_TX_RX_LB_ENA     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_LB_CFG_XGMII_TX_RX_LB_ENA(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Enables loopback from Rx to Tx data path at XGMII interface.
 *
 * \details 
 * 0: RX_TX loopback disabled at XGMII interface
 * 1: RX_TX loopback enabled at XGMII interface
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_LB_CFG . XGMII_RX_TX_LB_ENA
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_LB_CFG_XGMII_RX_TX_LB_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_LB_CFG_XGMII_RX_TX_LB_ENA     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_LB_CFG_XGMII_RX_TX_LB_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Link fault state monitor bits
 *
 * \details
 * Register: \a MAC10G::MAC_CFG_STATUS::MAC_TX_MONITOR
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR(target)  VTSS_IOREG(target,0x49)

/** 
 * \brief
 * Link local fault state
 *
 * \details 
 * 0: No local fault
 * 1: Local fault detected
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR . LFS_LOCAL_FAULT
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_LFS_LOCAL_FAULT(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_LFS_LOCAL_FAULT     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_LFS_LOCAL_FAULT(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Link remote fault state
 *
 * \details 
 * 0: No remote fault
 * 1: Remote fault detected
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR . LFS_REMOTE_FAULT
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_LFS_REMOTE_FAULT(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_LFS_REMOTE_FAULT     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_LFS_REMOTE_FAULT(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief MAC 10G Tx monitor sticky bit register
 *
 * \details
 * Register: \a MAC10G::MAC_CFG_STATUS::MAC_TX_MONITOR_STICKY
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_STICKY(target)  VTSS_IOREG(target,0x4c)

/** 
 * \brief
 * LFS local fault state changed in Tx module of 10G MAC. Writing a 1
 * clears the sticky bit.
 * The real-time state of LOCAL_FAULT can be read using the LFS_LOCAL_FAULT
 * bit.
 *
 * \details 
 * 0: The LOCAL_FAULT state has not changed
 * 1: The LOCAL_FAULT state has either been entered or exited
 * Bit is cleared by writing a 1 to this position
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_STICKY . LOCAL_FAULT_STATE_DELTA
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_STICKY_LOCAL_FAULT_STATE_DELTA(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_STICKY_LOCAL_FAULT_STATE_DELTA     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_STICKY_LOCAL_FAULT_STATE_DELTA(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * LFS remote fault state changed in Tx module of 10G MAC. Writing a 1
 * clears the sticky bit.
 * The real-time state of REMOTE_FAULT can be read using the
 * LFS_REMOTE_FAULT bit.
 *
 * \details 
 * 0: The REMOTE_FAULT state has not changed
 * 1: The REMOTE_FAULT state has either been entered or exited
 * Bit is cleared by writing a 1 to this position
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_STICKY . REMOTE_FAULT_STATE_DELTA
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_STICKY_REMOTE_FAULT_STATE_DELTA(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_STICKY_REMOTE_FAULT_STATE_DELTA     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_STICKY_REMOTE_FAULT_STATE_DELTA(x)  VTSS_EXTRACT_BITFIELD(x,3,1)


/** 
 * \brief Sticky bit register
 *
 * \details
 * Clear the sticky bits by writing a 0 in the relevant bit groups (writing
 * a 1 sets the bit)
 *
 * Register: \a MAC10G::MAC_CFG_STATUS::MAC_STICKY
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_MAC_CFG_STATUS_MAC_STICKY(target)  VTSS_IOREG(target,0x4d)

/** 
 * \brief
 * This sticky bit indicates if an idle column has been dropped due to
 * clock differences in the Tx path
 *
 * \details 
 * 0: No idles have been dropped
 * 1: An idle column has been dropped
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_STICKY . TX_IFG_COLUMN_DROP_STICKY
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_STICKY_TX_IFG_COLUMN_DROP_STICKY(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_STICKY_TX_IFG_COLUMN_DROP_STICKY     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_STICKY_TX_IFG_COLUMN_DROP_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Indicates that a preamble shrink was detected (preamble < 8 bytes).
 * This sticky bit can only be set when the port is set up in 10 Gbps mode,
 * where frames with a 4 bytes preamble for example, will be discarded, and
 * it requires that PRM_SHK_CHK_DIS = 0 and SFD_CHK_ENA = 1.
 * In SGMII mode, all preamble sizes down to 3 bytes (including SFD) are
 * accepted and will not cause this sticky bit to be set.
 *
 * \details 
 * 0: No preamble shrink detected
 * 1: Preamble shrink detected
 * Bit is cleared by writing a 1 to this position
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_STICKY . RX_PREAM_SHRINK_STICKY
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_STICKY_RX_PREAM_SHRINK_STICKY(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_STICKY_RX_PREAM_SHRINK_STICKY     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_STICKY_RX_PREAM_SHRINK_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * If preamble check is enabled and a SOP is received, this bit is set if
 * the following bytes do not match a 5555555555.55D5 pattern. A 12 bytes
 * preamble of 55555555.55555555.555555D5 will not cause this sticky bit to
 * be set. This sticky bit can only be set when the port is setup in 10
 * Gbps mode.
 *
 * \details 
 * 0: No non-standard preamble has been received
 * 1: A non-standard preamble has been received
 * Bit is cleared by writing a 1 to this position
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_STICKY . RX_PREAM_MISMATCH_STICKY
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_STICKY_RX_PREAM_MISMATCH_STICKY(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_STICKY_RX_PREAM_MISMATCH_STICKY     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_STICKY_RX_PREAM_MISMATCH_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * If an SOP is received and a following control character is received
 * within the preamble, this bit is set. (No data is passed to the host
 * interface of the MAC).
 *
 * \details 
 * 0: No preamble error detected
 * 1: Preamble error detected
 * Bit is cleared by writing a 1 to this position
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_STICKY . RX_PREAM_ERR_STICKY
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_STICKY_RX_PREAM_ERR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_STICKY_RX_PREAM_ERR_STICKY     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_STICKY_RX_PREAM_ERR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Indicates that a frame was received with a non-standard preamble. This
 * sticky bit is unaffected by the setting of the PRM_CHK_ENA bit.
 *
 * \details 
 * 0: Only frames with a standard preamble have been received
 * 1: A frame with a non-standard preamble has been received
 * Bit is cleared by writing a 1 to this position
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_STICKY . RX_NON_STD_PREAM_STICKY
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_STICKY_RX_NON_STD_PREAM_STICKY(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_STICKY_RX_NON_STD_PREAM_STICKY     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_STICKY_RX_NON_STD_PREAM_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Indicates that the transmit host initiated abort was executed
 *
 * \details 
 * 0: No Tx frame was aborted
 * 1: One or more Tx frames were aborted
 * Bit is cleared by writing a 1 to this position
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_STICKY . TX_ABORT_STICKY
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_STICKY_TX_ABORT_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_STICKY_TX_ABORT_STICKY     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_STICKY_TX_ABORT_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief MAC transmit monitor interrupt mask
 *
 * \details
 * Register: \a MAC10G::MAC_CFG_STATUS::MAC_TX_MONITOR_INT_MASK
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_INT_MASK(target)  VTSS_IOREG(target,0x50)

/** 
 * \brief
 * Interrupt mask for LOCAL_FAULT_STATE_DELTA
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_INT_MASK . LOCAL_FAULT_STATE_INT_MASK
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_INT_MASK_LOCAL_FAULT_STATE_INT_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_INT_MASK_LOCAL_FAULT_STATE_INT_MASK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_INT_MASK_LOCAL_FAULT_STATE_INT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Interrupt mask for REMOTE_FAULT_STATE_DELTA
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_INT_MASK . REMOTE_FAULT_STATE_INT_MASK
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_INT_MASK_REMOTE_FAULT_STATE_INT_MASK(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_INT_MASK_REMOTE_FAULT_STATE_INT_MASK     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_TX_MONITOR_INT_MASK_REMOTE_FAULT_STATE_INT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,3,1)


/** 
 * \brief MAC sticky bit interrupt masks
 *
 * \details
 * Register: \a MAC10G::MAC_CFG_STATUS::MAC_INT_MASK
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK(target)  VTSS_IOREG(target,0x51)

/** 
 * \brief
 * Interrupt mask for RX_PREAM_SHRINK_STICKY
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK . RX_PREAM_SHRINK_INT_MASK
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK_RX_PREAM_SHRINK_INT_MASK(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK_RX_PREAM_SHRINK_INT_MASK     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK_RX_PREAM_SHRINK_INT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Interrupt mask for RX_PREAM_MISMATCH_STICKY
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK . RX_PREAM_MISMATCH_INT_MASK
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK_RX_PREAM_MISMATCH_INT_MASK(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK_RX_PREAM_MISMATCH_INT_MASK     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK_RX_PREAM_MISMATCH_INT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Interrupt mask for RX_PREAM_ERR_STICKY
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK . RX_PREAM_ERR_INT_MASK
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK_RX_PREAM_ERR_INT_MASK(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK_RX_PREAM_ERR_INT_MASK     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK_RX_PREAM_ERR_INT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Interrupt mask for RX_NON_STD_PREAM_STICKY
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK . RX_NON_STD_PREAM_INT_MASK
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK_RX_NON_STD_PREAM_INT_MASK(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK_RX_NON_STD_PREAM_INT_MASK     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK_RX_NON_STD_PREAM_INT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Interrupt mask for TX_ABORT_STICKY
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK . TX_ABORT_INT_MASK
 */
#define  VTSS_F_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK_TX_ABORT_INT_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK_TX_ABORT_INT_MASK     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_MAC10G_MAC_CFG_STATUS_MAC_INT_MASK_TX_ABORT_INT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/**
 * Register Group: \a MAC10G::DEV_STATISTICS
 *
 * Generic device statistics registers
 */


/** 
 * \brief Rx symbol carrier error counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::RX_SYMBOL_ERR_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_RX_SYMBOL_ERR_CNT(target)  VTSS_IOREG(target,0x80)

/** 
 * \brief
 * The number of frames received with one or more symbol errors
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_RX_SYMBOL_ERR_CNT . RX_SYMBOL_ERR_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_RX_SYMBOL_ERR_CNT_RX_SYMBOL_ERR_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_RX_SYMBOL_ERR_CNT_RX_SYMBOL_ERR_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_RX_SYMBOL_ERR_CNT_RX_SYMBOL_ERR_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx pause frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::RX_PAUSE_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_RX_PAUSE_CNT(target)  VTSS_IOREG(target,0x81)

/** 
 * \brief
 * Number of pause control frames received
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_RX_PAUSE_CNT . RX_PAUSE_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_RX_PAUSE_CNT_RX_PAUSE_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_RX_PAUSE_CNT_RX_PAUSE_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_RX_PAUSE_CNT_RX_PAUSE_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx control frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::RX_UNSUP_OPCODE_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_RX_UNSUP_OPCODE_CNT(target)  VTSS_IOREG(target,0x82)

/** 
 * \brief
 * Number of control frames with unsupported opcode received
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_RX_UNSUP_OPCODE_CNT . RX_UNSUP_OPCODE_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_RX_UNSUP_OPCODE_CNT_RX_UNSUP_OPCODE_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_RX_UNSUP_OPCODE_CNT_RX_UNSUP_OPCODE_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_RX_UNSUP_OPCODE_CNT_RX_UNSUP_OPCODE_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx bad byte counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::RX_BAD_BYTES_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_RX_BAD_BYTES_CNT(target)  VTSS_IOREG(target,0x83)

/** 
 * \brief
 * The number of received bytes in bad frames
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_RX_BAD_BYTES_CNT . RX_BAD_BYTES_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_RX_BAD_BYTES_CNT_RX_BAD_BYTES_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_RX_BAD_BYTES_CNT_RX_BAD_BYTES_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_RX_BAD_BYTES_CNT_RX_BAD_BYTES_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx unicast frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::RX_UC_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_RX_UC_CNT(target)  VTSS_IOREG(target,0x84)

/** 
 * \brief
 * The number of good unicast frames received
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_RX_UC_CNT . RX_UC_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_RX_UC_CNT_RX_UC_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_RX_UC_CNT_RX_UC_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_RX_UC_CNT_RX_UC_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx multicast frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::RX_MC_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_RX_MC_CNT(target)  VTSS_IOREG(target,0x85)

/** 
 * \brief
 * The number of good multicast frames received
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_RX_MC_CNT . RX_MC_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_RX_MC_CNT_RX_MC_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_RX_MC_CNT_RX_MC_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_RX_MC_CNT_RX_MC_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx broadcast frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::RX_BC_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_RX_BC_CNT(target)  VTSS_IOREG(target,0x86)

/** 
 * \brief
 * The number of good broadcast frames received
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_RX_BC_CNT . RX_BC_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_RX_BC_CNT_RX_BC_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_RX_BC_CNT_RX_BC_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_RX_BC_CNT_RX_BC_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx CRC error counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::RX_CRC_ERR_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_RX_CRC_ERR_CNT(target)  VTSS_IOREG(target,0x87)

/** 
 * \brief
 * The number of frames received with CRC error only
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_RX_CRC_ERR_CNT . RX_CRC_ERR_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_RX_CRC_ERR_CNT_RX_CRC_ERR_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_RX_CRC_ERR_CNT_RX_CRC_ERR_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_RX_CRC_ERR_CNT_RX_CRC_ERR_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx undersize counter (valid frame format)
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::RX_UNDERSIZE_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_RX_UNDERSIZE_CNT(target)  VTSS_IOREG(target,0x88)

/** 
 * \brief
 * The number of undersize well-formed frames received
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_RX_UNDERSIZE_CNT . RX_UNDERSIZE_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_RX_UNDERSIZE_CNT_RX_UNDERSIZE_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_RX_UNDERSIZE_CNT_RX_UNDERSIZE_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_RX_UNDERSIZE_CNT_RX_UNDERSIZE_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx undersize counter (CRC error)
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::RX_FRAGMENTS_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_RX_FRAGMENTS_CNT(target)  VTSS_IOREG(target,0x89)

/** 
 * \brief
 * The number of undersize frames with CRC error received
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_RX_FRAGMENTS_CNT . RX_FRAGMENTS_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_RX_FRAGMENTS_CNT_RX_FRAGMENTS_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_RX_FRAGMENTS_CNT_RX_FRAGMENTS_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_RX_FRAGMENTS_CNT_RX_FRAGMENTS_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx in-range length error counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::RX_IN_RANGE_LEN_ERR_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_RX_IN_RANGE_LEN_ERR_CNT(target)  VTSS_IOREG(target,0x8a)

/** 
 * \brief
 * The number of frames with legal length field that doesn't match length
 * of MAC client data. Frames with mismatched length fields will only be
 * dropped if the INR_ERR_ENA bit is set to 1 in the MAC_ADV_CHK_CFG
 * register.
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_RX_IN_RANGE_LEN_ERR_CNT . RX_IN_RANGE_LEN_ERR_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_RX_IN_RANGE_LEN_ERR_CNT_RX_IN_RANGE_LEN_ERR_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_RX_IN_RANGE_LEN_ERR_CNT_RX_IN_RANGE_LEN_ERR_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_RX_IN_RANGE_LEN_ERR_CNT_RX_IN_RANGE_LEN_ERR_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx out-of-range length error counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::RX_OUT_OF_RANGE_LEN_ERR_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_RX_OUT_OF_RANGE_LEN_ERR_CNT(target)  VTSS_IOREG(target,0x8b)

/** 
 * \brief
 * The number of frames with illegal length field (frames using type field
 * are not counted here). Frames with illegal length fields will only be
 * dropped if the OOR_ERR_ENA bit is set to 1 in the MAC_ADV_CHK_CFG
 * register.
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_RX_OUT_OF_RANGE_LEN_ERR_CNT . RX_OUT_OF_RANGE_LEN_ERR_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_RX_OUT_OF_RANGE_LEN_ERR_CNT_RX_OUT_OF_RANGE_LEN_ERR_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_RX_OUT_OF_RANGE_LEN_ERR_CNT_RX_OUT_OF_RANGE_LEN_ERR_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_RX_OUT_OF_RANGE_LEN_ERR_CNT_RX_OUT_OF_RANGE_LEN_ERR_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx oversize counter (valid frame format)
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::RX_OVERSIZE_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_RX_OVERSIZE_CNT(target)  VTSS_IOREG(target,0x8c)

/** 
 * \brief
 * The number of oversize well-formed frames received
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_RX_OVERSIZE_CNT . RX_OVERSIZE_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_RX_OVERSIZE_CNT_RX_OVERSIZE_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_RX_OVERSIZE_CNT_RX_OVERSIZE_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_RX_OVERSIZE_CNT_RX_OVERSIZE_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx jabbers counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::RX_JABBERS_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_RX_JABBERS_CNT(target)  VTSS_IOREG(target,0x8d)

/** 
 * \brief
 * The number of oversize frames with CRC error received
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_RX_JABBERS_CNT . RX_JABBERS_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_RX_JABBERS_CNT_RX_JABBERS_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_RX_JABBERS_CNT_RX_JABBERS_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_RX_JABBERS_CNT_RX_JABBERS_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx 64 byte frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::RX_SIZE64_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_RX_SIZE64_CNT(target)  VTSS_IOREG(target,0x8e)

/** 
 * \brief
 * The number of 64 bytes frames received
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_RX_SIZE64_CNT . RX_SIZE64_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_RX_SIZE64_CNT_RX_SIZE64_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_RX_SIZE64_CNT_RX_SIZE64_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_RX_SIZE64_CNT_RX_SIZE64_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx 65-127 byte frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::RX_SIZE65TO127_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_RX_SIZE65TO127_CNT(target)  VTSS_IOREG(target,0x8f)

/** 
 * \brief
 * The number of 65 to 127 bytes frames received
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_RX_SIZE65TO127_CNT . RX_SIZE65TO127_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_RX_SIZE65TO127_CNT_RX_SIZE65TO127_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_RX_SIZE65TO127_CNT_RX_SIZE65TO127_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_RX_SIZE65TO127_CNT_RX_SIZE65TO127_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx 128-255 byte frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::RX_SIZE128TO255_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_RX_SIZE128TO255_CNT(target)  VTSS_IOREG(target,0x90)

/** 
 * \brief
 * The number of 128 to 255 bytes frames received
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_RX_SIZE128TO255_CNT . RX_SIZE128TO255_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_RX_SIZE128TO255_CNT_RX_SIZE128TO255_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_RX_SIZE128TO255_CNT_RX_SIZE128TO255_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_RX_SIZE128TO255_CNT_RX_SIZE128TO255_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx 256-511 byte frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::RX_SIZE256TO511_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_RX_SIZE256TO511_CNT(target)  VTSS_IOREG(target,0x91)

/** 
 * \brief
 * The number of 256 to 511 bytes frames received
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_RX_SIZE256TO511_CNT . RX_SIZE256TO511_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_RX_SIZE256TO511_CNT_RX_SIZE256TO511_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_RX_SIZE256TO511_CNT_RX_SIZE256TO511_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_RX_SIZE256TO511_CNT_RX_SIZE256TO511_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx 512-1023 byte frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::RX_SIZE512TO1023_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_RX_SIZE512TO1023_CNT(target)  VTSS_IOREG(target,0x92)

/** 
 * \brief
 * The number of 512 to 1023 bytes frames received
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_RX_SIZE512TO1023_CNT . RX_SIZE512TO1023_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_RX_SIZE512TO1023_CNT_RX_SIZE512TO1023_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_RX_SIZE512TO1023_CNT_RX_SIZE512TO1023_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_RX_SIZE512TO1023_CNT_RX_SIZE512TO1023_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx 1024-1518 byte frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::RX_SIZE1024TO1518_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_RX_SIZE1024TO1518_CNT(target)  VTSS_IOREG(target,0x93)

/** 
 * \brief
 * The number of 1024 to 1518 bytes frames received
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_RX_SIZE1024TO1518_CNT . RX_SIZE1024TO1518_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_RX_SIZE1024TO1518_CNT_RX_SIZE1024TO1518_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_RX_SIZE1024TO1518_CNT_RX_SIZE1024TO1518_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_RX_SIZE1024TO1518_CNT_RX_SIZE1024TO1518_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx 1519 to maximum length byte frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::RX_SIZE1519TOMAX_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_RX_SIZE1519TOMAX_CNT(target)  VTSS_IOREG(target,0x94)

/** 
 * \brief
 * The number of frames received longer than 1518 bytes and not longer than
 * the maximum length register (maximum length register + 4 if the frame is
 * VLAN tagged).
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_RX_SIZE1519TOMAX_CNT . RX_SIZE1519TOMAX_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_RX_SIZE1519TOMAX_CNT_RX_SIZE1519TOMAX_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_RX_SIZE1519TOMAX_CNT_RX_SIZE1519TOMAX_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_RX_SIZE1519TOMAX_CNT_RX_SIZE1519TOMAX_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx HIH checksum error counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::RX_HIH_CHKSUM_ERR_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_RX_HIH_CHKSUM_ERR_CNT(target)  VTSS_IOREG(target,0x95)

/** 
 * \brief
 * Number of checksum errors detected in the HIH when HIH checksum checking
 * is enabled
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_RX_HIH_CHKSUM_ERR_CNT . RX_HIH_CHKSUM_ERR_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_RX_HIH_CHKSUM_ERR_CNT_RX_HIH_CHKSUM_ERR_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_RX_HIH_CHKSUM_ERR_CNT_RX_HIH_CHKSUM_ERR_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_RX_HIH_CHKSUM_ERR_CNT_RX_HIH_CHKSUM_ERR_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Tx unicast frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::TX_UC_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_TX_UC_CNT(target)  VTSS_IOREG(target,0x97)

/** 
 * \brief
 * The number of unicast frames transmitted
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_TX_UC_CNT . TX_UC_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_TX_UC_CNT_TX_UC_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_TX_UC_CNT_TX_UC_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_TX_UC_CNT_TX_UC_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Tx multicast frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::TX_MC_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_TX_MC_CNT(target)  VTSS_IOREG(target,0x98)

/** 
 * \brief
 * The number of multicast frames transmitted
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_TX_MC_CNT . TX_MC_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_TX_MC_CNT_TX_MC_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_TX_MC_CNT_TX_MC_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_TX_MC_CNT_TX_MC_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Tx broadcast frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::TX_BC_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_TX_BC_CNT(target)  VTSS_IOREG(target,0x99)

/** 
 * \brief
 * The number of broadcast frames transmitted
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_TX_BC_CNT . TX_BC_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_TX_BC_CNT_TX_BC_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_TX_BC_CNT_TX_BC_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_TX_BC_CNT_TX_BC_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Tx 64 byte frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::TX_SIZE64_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_TX_SIZE64_CNT(target)  VTSS_IOREG(target,0x9a)

/** 
 * \brief
 * The number of 64 bytes frames transmitted
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_TX_SIZE64_CNT . TX_SIZE64_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_TX_SIZE64_CNT_TX_SIZE64_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_TX_SIZE64_CNT_TX_SIZE64_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_TX_SIZE64_CNT_TX_SIZE64_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Tx 65-127 byte frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::TX_SIZE65TO127_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_TX_SIZE65TO127_CNT(target)  VTSS_IOREG(target,0x9b)

/** 
 * \brief
 * The number of 65 to 127 bytes frames transmitted
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_TX_SIZE65TO127_CNT . TX_SIZE65TO127_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_TX_SIZE65TO127_CNT_TX_SIZE65TO127_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_TX_SIZE65TO127_CNT_TX_SIZE65TO127_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_TX_SIZE65TO127_CNT_TX_SIZE65TO127_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Tx 128-255 byte frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::TX_SIZE128TO255_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_TX_SIZE128TO255_CNT(target)  VTSS_IOREG(target,0x9c)

/** 
 * \brief
 * The number of 128 to 255 bytes frames transmitted
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_TX_SIZE128TO255_CNT . TX_SIZE128TO255_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_TX_SIZE128TO255_CNT_TX_SIZE128TO255_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_TX_SIZE128TO255_CNT_TX_SIZE128TO255_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_TX_SIZE128TO255_CNT_TX_SIZE128TO255_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Tx 256-511 byte frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::TX_SIZE256TO511_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_TX_SIZE256TO511_CNT(target)  VTSS_IOREG(target,0x9d)

/** 
 * \brief
 * The number of 256 to 511 bytes frames transmitted
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_TX_SIZE256TO511_CNT . TX_SIZE256TO511_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_TX_SIZE256TO511_CNT_TX_SIZE256TO511_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_TX_SIZE256TO511_CNT_TX_SIZE256TO511_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_TX_SIZE256TO511_CNT_TX_SIZE256TO511_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Tx 512-1023 byte frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::TX_SIZE512TO1023_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_TX_SIZE512TO1023_CNT(target)  VTSS_IOREG(target,0x9e)

/** 
 * \brief
 * The number of 512 to 1023 bytes frames transmitted
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_TX_SIZE512TO1023_CNT . TX_SIZE512TO1023_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_TX_SIZE512TO1023_CNT_TX_SIZE512TO1023_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_TX_SIZE512TO1023_CNT_TX_SIZE512TO1023_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_TX_SIZE512TO1023_CNT_TX_SIZE512TO1023_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Tx 1024-1518 byte frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::TX_SIZE1024TO1518_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_TX_SIZE1024TO1518_CNT(target)  VTSS_IOREG(target,0x9f)

/** 
 * \brief
 * The number of 1024 to 1518 bytes frames transmitted
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_TX_SIZE1024TO1518_CNT . TX_SIZE1024TO1518_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_TX_SIZE1024TO1518_CNT_TX_SIZE1024TO1518_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_TX_SIZE1024TO1518_CNT_TX_SIZE1024TO1518_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_TX_SIZE1024TO1518_CNT_TX_SIZE1024TO1518_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Tx 1519 to maximum length byte frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::TX_SIZE1519TOMAX_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_TX_SIZE1519TOMAX_CNT(target)  VTSS_IOREG(target,0xa0)

/** 
 * \brief
 * The number of frames transmitted longer than 1518 bytes and not longer
 * than maximum length register (maximum length register + 4 if the frame
 * is VLAN tagged).
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_TX_SIZE1519TOMAX_CNT . TX_SIZE1519TOMAX_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_TX_SIZE1519TOMAX_CNT_TX_SIZE1519TOMAX_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_TX_SIZE1519TOMAX_CNT_TX_SIZE1519TOMAX_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_TX_SIZE1519TOMAX_CNT_TX_SIZE1519TOMAX_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Tx abort counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::TX_ABORT_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_TX_ABORT_CNT(target)  VTSS_IOREG(target,0xa1)

/** 
 * \brief
 * Counts the number of aborts in the Tx direction
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_TX_ABORT_CNT . TX_ABORT_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_TX_ABORT_CNT_TX_ABORT_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_TX_ABORT_CNT_TX_ABORT_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_TX_ABORT_CNT_TX_ABORT_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx byte counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::RX_IN_BYTES_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_RX_IN_BYTES_CNT(target)  VTSS_IOREG(target,0xa2)

/** 
 * \brief
 * The number of bytes received (good, bad, and framing). This counter also
 * counts LFS (link-state-fault) bytes. This counter counts bytes
 * regardless of the setting of RX_ENA.
 *
 * \details 
 * Counter can be written by software. Value will automatically wrap to
 * zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_RX_IN_BYTES_CNT . RX_IN_BYTES_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_RX_IN_BYTES_CNT_RX_IN_BYTES_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_RX_IN_BYTES_CNT_RX_IN_BYTES_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_RX_IN_BYTES_CNT_RX_IN_BYTES_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx byte counter, MSB
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::RX_IN_BYTES_CNT_MSB
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_RX_IN_BYTES_CNT_MSB(target)  VTSS_IOREG(target,0xa3)

/** 
 * \brief
 * The number of bytes received (good, bad, and framing), MSB portion. Read
 * the LSB portion first to capture the upper portion.
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_RX_IN_BYTES_CNT_MSB . RX_IN_BYTES_CNT_MSB
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_RX_IN_BYTES_CNT_MSB_RX_IN_BYTES_CNT_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_MAC10G_DEV_STATISTICS_RX_IN_BYTES_CNT_MSB_RX_IN_BYTES_CNT_MSB     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_MAC10G_DEV_STATISTICS_RX_IN_BYTES_CNT_MSB_RX_IN_BYTES_CNT_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Rx OK byte counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::RX_OK_BYTES_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_RX_OK_BYTES_CNT(target)  VTSS_IOREG(target,0xa4)

/** 
 * \brief
 * The number of received bytes in good frames
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_RX_OK_BYTES_CNT . RX_OK_BYTES_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_RX_OK_BYTES_CNT_RX_OK_BYTES_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_RX_OK_BYTES_CNT_RX_OK_BYTES_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_RX_OK_BYTES_CNT_RX_OK_BYTES_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx OK byte counter, MSB portion
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::RX_OK_BYTES_CNT_MSB
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_RX_OK_BYTES_CNT_MSB(target)  VTSS_IOREG(target,0xa5)

/** 
 * \brief
 * The number of received bytes in good frames, MSB portion. Read the LSB
 * portion first to capture the upper portion.
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_RX_OK_BYTES_CNT_MSB . RX_OK_BYTES_CNT_MSB
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_RX_OK_BYTES_CNT_MSB_RX_OK_BYTES_CNT_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_MAC10G_DEV_STATISTICS_RX_OK_BYTES_CNT_MSB_RX_OK_BYTES_CNT_MSB     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_MAC10G_DEV_STATISTICS_RX_OK_BYTES_CNT_MSB_RX_OK_BYTES_CNT_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Tx byte counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::TX_OUT_BYTES_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_TX_OUT_BYTES_CNT(target)  VTSS_IOREG(target,0xa6)

/** 
 * \brief
 * The number of bytes transmitted (good, bad and framing). This counter
 * also counts LFS (link-state-fault) bytes.
 *
 * \details 
 * Counter can be written by software. Value will automatically wrap to
 * zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_TX_OUT_BYTES_CNT . TX_OUT_BYTES_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_TX_OUT_BYTES_CNT_TX_OUT_BYTES_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_TX_OUT_BYTES_CNT_TX_OUT_BYTES_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_TX_OUT_BYTES_CNT_TX_OUT_BYTES_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Tx byte counter, MSB
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::TX_OUT_BYTES_CNT_MSB
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_TX_OUT_BYTES_CNT_MSB(target)  VTSS_IOREG(target,0xa7)

/** 
 * \brief
 * The number of bytes transmitted (good, bad and framing), MSB portion.
 * Read the LSB portion first to capture the upper portion.
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_TX_OUT_BYTES_CNT_MSB . TX_OUT_BYTES_CNT_MSB
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_TX_OUT_BYTES_CNT_MSB_TX_OUT_BYTES_CNT_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_MAC10G_DEV_STATISTICS_TX_OUT_BYTES_CNT_MSB_TX_OUT_BYTES_CNT_MSB     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_MAC10G_DEV_STATISTICS_TX_OUT_BYTES_CNT_MSB_TX_OUT_BYTES_CNT_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Tx OK byte counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS::TX_OK_BYTES_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_TX_OK_BYTES_CNT(target)  VTSS_IOREG(target,0xa8)

/** 
 * \brief
 * The number of bytes transmitted successfully. Read the LSB portion first
 * to capture the upper portion.
 *
 * \details 
 * Counter can be written by SW. Value will automatically wrap to zero.
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_TX_OK_BYTES_CNT . TX_OK_BYTES_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_TX_OK_BYTES_CNT_TX_OK_BYTES_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_TX_OK_BYTES_CNT_TX_OK_BYTES_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_TX_OK_BYTES_CNT_TX_OK_BYTES_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Tx OK byte counter, MSB
 *
 * \details
 * The number of bytes transmitted successfully (MSB portion)
 *
 * Register: \a MAC10G::DEV_STATISTICS::TX_OK_BYTES_CNT_MSB
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_TX_OK_BYTES_CNT_MSB(target)  VTSS_IOREG(target,0xa9)

/** 
 * \brief
 * The number of bytes transmitted successfully, MSB portion. Read the LSB
 * portion first to capture the upper portion.
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_TX_OK_BYTES_CNT_MSB . TX_OK_BYTES_CNT_MSB
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_TX_OK_BYTES_CNT_MSB_TX_OK_BYTES_CNT_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_MAC10G_DEV_STATISTICS_TX_OK_BYTES_CNT_MSB_TX_OK_BYTES_CNT_MSB     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_MAC10G_DEV_STATISTICS_TX_OK_BYTES_CNT_MSB_TX_OK_BYTES_CNT_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a MAC10G::DEV_STATISTICS_PERF
 *
 * Generic device statistics registers
 */


/** 
 * \brief Rx symbol carrier error counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS_PERF::RX_SYMBOL_ERR_CNT_PERF
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_PERF_RX_SYMBOL_ERR_CNT_PERF(target)  VTSS_IOREG(target,0xc0)

/** 
 * \brief
 * The number of frames received with one or more symbol errors.
 * Performance monitor version.
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_PERF_RX_SYMBOL_ERR_CNT_PERF . RX_SYMBOL_ERR_CNT_PERF
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_PERF_RX_SYMBOL_ERR_CNT_PERF_RX_SYMBOL_ERR_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_MAC10G_DEV_STATISTICS_PERF_RX_SYMBOL_ERR_CNT_PERF_RX_SYMBOL_ERR_CNT_PERF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_MAC10G_DEV_STATISTICS_PERF_RX_SYMBOL_ERR_CNT_PERF_RX_SYMBOL_ERR_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Rx pause frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS_PERF::RX_PAUSE_CNT_PERF
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_PERF_RX_PAUSE_CNT_PERF(target)  VTSS_IOREG(target,0xc1)

/** 
 * \brief
 * Number of pause control frames received (performance monitor version)
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_PERF_RX_PAUSE_CNT_PERF . RX_PAUSE_CNT_PERF
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_PERF_RX_PAUSE_CNT_PERF_RX_PAUSE_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_MAC10G_DEV_STATISTICS_PERF_RX_PAUSE_CNT_PERF_RX_PAUSE_CNT_PERF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_MAC10G_DEV_STATISTICS_PERF_RX_PAUSE_CNT_PERF_RX_PAUSE_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Rx control frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS_PERF::RX_UNSUP_OPCODE_CNT_PERF
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_PERF_RX_UNSUP_OPCODE_CNT_PERF(target)  VTSS_IOREG(target,0xc2)

/** 
 * \brief
 * Number of control frames with unsupported opcode received (performance
 * monitor version)
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_PERF_RX_UNSUP_OPCODE_CNT_PERF . RX_UNSUP_OPCODE_CNT_PERF
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_PERF_RX_UNSUP_OPCODE_CNT_PERF_RX_UNSUP_OPCODE_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_MAC10G_DEV_STATISTICS_PERF_RX_UNSUP_OPCODE_CNT_PERF_RX_UNSUP_OPCODE_CNT_PERF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_MAC10G_DEV_STATISTICS_PERF_RX_UNSUP_OPCODE_CNT_PERF_RX_UNSUP_OPCODE_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Rx bad byte counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS_PERF::RX_BAD_BYTES_CNT_PERF
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_PERF_RX_BAD_BYTES_CNT_PERF(target)  VTSS_IOREG(target,0xc3)

/** 
 * \brief
 * The number of received bytes in bad frames. Performance monitor version.
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_PERF_RX_BAD_BYTES_CNT_PERF . RX_BAD_BYTES_CNT_PERF
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_PERF_RX_BAD_BYTES_CNT_PERF_RX_BAD_BYTES_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_PERF_RX_BAD_BYTES_CNT_PERF_RX_BAD_BYTES_CNT_PERF     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_PERF_RX_BAD_BYTES_CNT_PERF_RX_BAD_BYTES_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx unicast frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS_PERF::RX_UC_CNT_PERF
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_PERF_RX_UC_CNT_PERF(target)  VTSS_IOREG(target,0xc4)

/** 
 * \brief
 * The number of good unicast frames received (performance monitor version)
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_PERF_RX_UC_CNT_PERF . RX_UC_CNT_PERF
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_PERF_RX_UC_CNT_PERF_RX_UC_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_PERF_RX_UC_CNT_PERF_RX_UC_CNT_PERF     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_PERF_RX_UC_CNT_PERF_RX_UC_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx multicast frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS_PERF::RX_MC_CNT_PERF
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_PERF_RX_MC_CNT_PERF(target)  VTSS_IOREG(target,0xc5)

/** 
 * \brief
 * The number of good multicast frames received (performance monitor
 * version)
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_PERF_RX_MC_CNT_PERF . RX_MC_CNT_PERF
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_PERF_RX_MC_CNT_PERF_RX_MC_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_MAC10G_DEV_STATISTICS_PERF_RX_MC_CNT_PERF_RX_MC_CNT_PERF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_MAC10G_DEV_STATISTICS_PERF_RX_MC_CNT_PERF_RX_MC_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Rx broadcast frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS_PERF::RX_BC_CNT_PERF
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_PERF_RX_BC_CNT_PERF(target)  VTSS_IOREG(target,0xc6)

/** 
 * \brief
 * The number of good broadcast frames received (performance monitor
 * version)
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_PERF_RX_BC_CNT_PERF . RX_BC_CNT_PERF
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_PERF_RX_BC_CNT_PERF_RX_BC_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_MAC10G_DEV_STATISTICS_PERF_RX_BC_CNT_PERF_RX_BC_CNT_PERF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_MAC10G_DEV_STATISTICS_PERF_RX_BC_CNT_PERF_RX_BC_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Rx CRC error counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS_PERF::RX_CRC_ERR_CNT_PERF
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_PERF_RX_CRC_ERR_CNT_PERF(target)  VTSS_IOREG(target,0xc7)

/** 
 * \brief
 * The number of frames received with CRC error only (performance monitor
 * version)
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_PERF_RX_CRC_ERR_CNT_PERF . RX_CRC_ERR_CNT_PERF
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_PERF_RX_CRC_ERR_CNT_PERF_RX_CRC_ERR_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_MAC10G_DEV_STATISTICS_PERF_RX_CRC_ERR_CNT_PERF_RX_CRC_ERR_CNT_PERF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_MAC10G_DEV_STATISTICS_PERF_RX_CRC_ERR_CNT_PERF_RX_CRC_ERR_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Rx undersize counter (valid frame format)
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS_PERF::RX_UNDERSIZE_CNT_PERF
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_PERF_RX_UNDERSIZE_CNT_PERF(target)  VTSS_IOREG(target,0xc8)

/** 
 * \brief
 * The number of undersize well-formed frames received (performance monitor
 * version)
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_PERF_RX_UNDERSIZE_CNT_PERF . RX_UNDERSIZE_CNT_PERF
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_PERF_RX_UNDERSIZE_CNT_PERF_RX_UNDERSIZE_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_MAC10G_DEV_STATISTICS_PERF_RX_UNDERSIZE_CNT_PERF_RX_UNDERSIZE_CNT_PERF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_MAC10G_DEV_STATISTICS_PERF_RX_UNDERSIZE_CNT_PERF_RX_UNDERSIZE_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Rx undersize counter (CRC error)
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS_PERF::RX_FRAGMENTS_CNT_PERF
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_PERF_RX_FRAGMENTS_CNT_PERF(target)  VTSS_IOREG(target,0xc9)

/** 
 * \brief
 * The number of undersize frames with CRC error received (performance
 * monitor version)
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_PERF_RX_FRAGMENTS_CNT_PERF . RX_FRAGMENTS_CNT_PERF
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_PERF_RX_FRAGMENTS_CNT_PERF_RX_FRAGMENTS_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_MAC10G_DEV_STATISTICS_PERF_RX_FRAGMENTS_CNT_PERF_RX_FRAGMENTS_CNT_PERF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_MAC10G_DEV_STATISTICS_PERF_RX_FRAGMENTS_CNT_PERF_RX_FRAGMENTS_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Rx in-range length error counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS_PERF::RX_IN_RANGE_LEN_ERR_CNT_PERF
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_PERF_RX_IN_RANGE_LEN_ERR_CNT_PERF(target)  VTSS_IOREG(target,0xca)

/** 
 * \brief
 * The number of frames with legal length field that doesn't match length
 * of MAC client data. Frames with mismatched length fields will only be
 * dropped if the INR_ERR_ENA bit is set to 1 in the MAC_ADV_CHK_CFG
 * register. Performance monitor version.
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_PERF_RX_IN_RANGE_LEN_ERR_CNT_PERF . RX_IN_RANGE_LEN_ERR_CNT_PERF
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_PERF_RX_IN_RANGE_LEN_ERR_CNT_PERF_RX_IN_RANGE_LEN_ERR_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_MAC10G_DEV_STATISTICS_PERF_RX_IN_RANGE_LEN_ERR_CNT_PERF_RX_IN_RANGE_LEN_ERR_CNT_PERF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_MAC10G_DEV_STATISTICS_PERF_RX_IN_RANGE_LEN_ERR_CNT_PERF_RX_IN_RANGE_LEN_ERR_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Rx out-of-range length error counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS_PERF::RX_OUT_OF_RANGE_LEN_ERR_CNT_PERF
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_PERF_RX_OUT_OF_RANGE_LEN_ERR_CNT_PERF(target)  VTSS_IOREG(target,0xcb)

/** 
 * \brief
 * The number of frames with illegal length field (frames using type field
 * are not counted here). Frames with illegal length fields will only be
 * dropped if the OOR_ERR_ENA bit is set to 1 in the MAC_ADV_CHK_CFG
 * register. Performance monitor version.
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_PERF_RX_OUT_OF_RANGE_LEN_ERR_CNT_PERF . RX_OUT_OF_RANGE_LEN_ERR_CNT_PERF
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_PERF_RX_OUT_OF_RANGE_LEN_ERR_CNT_PERF_RX_OUT_OF_RANGE_LEN_ERR_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_MAC10G_DEV_STATISTICS_PERF_RX_OUT_OF_RANGE_LEN_ERR_CNT_PERF_RX_OUT_OF_RANGE_LEN_ERR_CNT_PERF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_MAC10G_DEV_STATISTICS_PERF_RX_OUT_OF_RANGE_LEN_ERR_CNT_PERF_RX_OUT_OF_RANGE_LEN_ERR_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Rx oversize counter (valid frame format)
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS_PERF::RX_OVERSIZE_CNT_PERF
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_PERF_RX_OVERSIZE_CNT_PERF(target)  VTSS_IOREG(target,0xcc)

/** 
 * \brief
 * The number of oversize well-formed frames received (performance monitor
 * version)
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_PERF_RX_OVERSIZE_CNT_PERF . RX_OVERSIZE_CNT_PERF
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_PERF_RX_OVERSIZE_CNT_PERF_RX_OVERSIZE_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_MAC10G_DEV_STATISTICS_PERF_RX_OVERSIZE_CNT_PERF_RX_OVERSIZE_CNT_PERF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_MAC10G_DEV_STATISTICS_PERF_RX_OVERSIZE_CNT_PERF_RX_OVERSIZE_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Rx jabbers counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS_PERF::RX_JABBERS_CNT_PERF
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_PERF_RX_JABBERS_CNT_PERF(target)  VTSS_IOREG(target,0xcd)

/** 
 * \brief
 * The number of oversize frames with CRC error received (performance
 * monitor version)
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_PERF_RX_JABBERS_CNT_PERF . RX_JABBERS_CNT_PERF
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_PERF_RX_JABBERS_CNT_PERF_RX_JABBERS_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_MAC10G_DEV_STATISTICS_PERF_RX_JABBERS_CNT_PERF_RX_JABBERS_CNT_PERF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_MAC10G_DEV_STATISTICS_PERF_RX_JABBERS_CNT_PERF_RX_JABBERS_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Rx HIH checksum error counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS_PERF::RX_HIH_CHKSUM_ERR_CNT_PERF
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_PERF_RX_HIH_CHKSUM_ERR_CNT_PERF(target)  VTSS_IOREG(target,0xce)

/** 
 * \brief
 * Number of checksum errors detected in the HIH when HIH checksum checking
 * is enabled (performance monitor version)
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_PERF_RX_HIH_CHKSUM_ERR_CNT_PERF . RX_HIH_CHKSUM_ERR_CNT_PERF
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_PERF_RX_HIH_CHKSUM_ERR_CNT_PERF_RX_HIH_CHKSUM_ERR_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_MAC10G_DEV_STATISTICS_PERF_RX_HIH_CHKSUM_ERR_CNT_PERF_RX_HIH_CHKSUM_ERR_CNT_PERF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_MAC10G_DEV_STATISTICS_PERF_RX_HIH_CHKSUM_ERR_CNT_PERF_RX_HIH_CHKSUM_ERR_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Tx pause frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS_PERF::TX_PAUSE_CNT_PERF
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_PERF_TX_PAUSE_CNT_PERF(target)  VTSS_IOREG(target,0xcf)

/** 
 * \brief
 * The number of pause control frames transmitted (performance monitor
 * version)
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_PERF_TX_PAUSE_CNT_PERF . TX_PAUSE_CNT_PERF
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_PERF_TX_PAUSE_CNT_PERF_TX_PAUSE_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_MAC10G_DEV_STATISTICS_PERF_TX_PAUSE_CNT_PERF_TX_PAUSE_CNT_PERF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_MAC10G_DEV_STATISTICS_PERF_TX_PAUSE_CNT_PERF_TX_PAUSE_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Tx unicast frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS_PERF::TX_UC_CNT_PERF
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_PERF_TX_UC_CNT_PERF(target)  VTSS_IOREG(target,0xd0)

/** 
 * \brief
 * The number of unicast frames transmitted (performance monitor version)
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_PERF_TX_UC_CNT_PERF . TX_UC_CNT_PERF
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_PERF_TX_UC_CNT_PERF_TX_UC_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_PERF_TX_UC_CNT_PERF_TX_UC_CNT_PERF     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_PERF_TX_UC_CNT_PERF_TX_UC_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Tx multicast frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS_PERF::TX_MC_CNT_PERF
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_PERF_TX_MC_CNT_PERF(target)  VTSS_IOREG(target,0xd1)

/** 
 * \brief
 * The number of multicast frames transmitted (performance monitor version)
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_PERF_TX_MC_CNT_PERF . TX_MC_CNT_PERF
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_PERF_TX_MC_CNT_PERF_TX_MC_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_MAC10G_DEV_STATISTICS_PERF_TX_MC_CNT_PERF_TX_MC_CNT_PERF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_MAC10G_DEV_STATISTICS_PERF_TX_MC_CNT_PERF_TX_MC_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Tx broadcast frame counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS_PERF::TX_BC_CNT_PERF
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_PERF_TX_BC_CNT_PERF(target)  VTSS_IOREG(target,0xd2)

/** 
 * \brief
 * The number of broadcast frames transmitted (performance monitor version)
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_PERF_TX_BC_CNT_PERF . TX_BC_CNT_PERF
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_PERF_TX_BC_CNT_PERF_TX_BC_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_MAC10G_DEV_STATISTICS_PERF_TX_BC_CNT_PERF_TX_BC_CNT_PERF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_MAC10G_DEV_STATISTICS_PERF_TX_BC_CNT_PERF_TX_BC_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Tx abort counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS_PERF::TX_ABORT_CNT_PERF
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_PERF_TX_ABORT_CNT_PERF(target)  VTSS_IOREG(target,0xd3)

/** 
 * \brief
 * The number of aborted frames transmitted (performance monitor version)
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_PERF_TX_ABORT_CNT_PERF . TX_ABORT_CNT_PERF
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_PERF_TX_ABORT_CNT_PERF_TX_ABORT_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_MAC10G_DEV_STATISTICS_PERF_TX_ABORT_CNT_PERF_TX_ABORT_CNT_PERF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_MAC10G_DEV_STATISTICS_PERF_TX_ABORT_CNT_PERF_TX_ABORT_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Rx OK byte counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS_PERF::RX_OK_BYTES_CNT_PERF
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_PERF_RX_OK_BYTES_CNT_PERF(target)  VTSS_IOREG(target,0xd6)

/** 
 * \brief
 * The number of received bytes in good frames (performance monitor
 * version)
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_PERF_RX_OK_BYTES_CNT_PERF . RX_OK_BYTES_CNT_PERF
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_PERF_RX_OK_BYTES_CNT_PERF_RX_OK_BYTES_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_PERF_RX_OK_BYTES_CNT_PERF_RX_OK_BYTES_CNT_PERF     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_PERF_RX_OK_BYTES_CNT_PERF_RX_OK_BYTES_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx OK byte counter, MSB portion
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS_PERF::RX_OK_BYTES_CNT_MSB_PERF
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_PERF_RX_OK_BYTES_CNT_MSB_PERF(target)  VTSS_IOREG(target,0xd7)

/** 
 * \brief
 * The number of received bytes in good frames, MSB portion. Read the LSB
 * portion first to capture the upper portion. Performance monitor version.
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_PERF_RX_OK_BYTES_CNT_MSB_PERF . RX_OK_BYTES_CNT_MSB_PERF
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_PERF_RX_OK_BYTES_CNT_MSB_PERF_RX_OK_BYTES_CNT_MSB_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_MAC10G_DEV_STATISTICS_PERF_RX_OK_BYTES_CNT_MSB_PERF_RX_OK_BYTES_CNT_MSB_PERF     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_MAC10G_DEV_STATISTICS_PERF_RX_OK_BYTES_CNT_MSB_PERF_RX_OK_BYTES_CNT_MSB_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Tx OK byte counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS_PERF::TX_OK_BYTES_CNT_PERF
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_PERF_TX_OK_BYTES_CNT_PERF(target)  VTSS_IOREG(target,0xda)

/** 
 * \brief
 * The number of bytes transmitted successfully. Read the LSB portion first
 * to capture the upper portion. Performance monitor version.
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_PERF_TX_OK_BYTES_CNT_PERF . TX_OK_BYTES_CNT_PERF
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_PERF_TX_OK_BYTES_CNT_PERF_TX_OK_BYTES_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_PERF_TX_OK_BYTES_CNT_PERF_TX_OK_BYTES_CNT_PERF     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_PERF_TX_OK_BYTES_CNT_PERF_TX_OK_BYTES_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Tx OK byte counter, MSB
 *
 * \details
 * The number of bytes transmitted successfully (MSB portion)
 *
 * Register: \a MAC10G::DEV_STATISTICS_PERF::TX_OK_BYTES_CNT_MSB_PERF
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_PERF_TX_OK_BYTES_CNT_MSB_PERF(target)  VTSS_IOREG(target,0xdb)

/** 
 * \brief
 * The number of bytes transmitted successfully, MSB portion. Read the LSB
 * portion first to capture the upper portion. Performance monitor version.
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_PERF_TX_OK_BYTES_CNT_MSB_PERF . TX_OK_BYTES_CNT_MSB_PERF
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_PERF_TX_OK_BYTES_CNT_MSB_PERF_TX_OK_BYTES_CNT_MSB_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_MAC10G_DEV_STATISTICS_PERF_TX_OK_BYTES_CNT_MSB_PERF_TX_OK_BYTES_CNT_MSB_PERF     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_MAC10G_DEV_STATISTICS_PERF_TX_OK_BYTES_CNT_MSB_PERF_TX_OK_BYTES_CNT_MSB_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a MAC10G::DEV_STATISTICS_XGMII
 *
 * XGMII statistics registers
 */


/** 
 * \brief Rx XGMII protocol error counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS_XGMII::RX_XGMII_PROT_ERR_CNT
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_XGMII_RX_XGMII_PROT_ERR_CNT(target)  VTSS_IOREG(target,0x100)

/** 
 * \brief
 * Number of XGMII protocol errors detected
 *
 * \details 
 * Counter can be written by SW
 *
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_XGMII_RX_XGMII_PROT_ERR_CNT . RX_XGMII_PROT_ERR_CNT
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_XGMII_RX_XGMII_PROT_ERR_CNT_RX_XGMII_PROT_ERR_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_MAC10G_DEV_STATISTICS_XGMII_RX_XGMII_PROT_ERR_CNT_RX_XGMII_PROT_ERR_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_MAC10G_DEV_STATISTICS_XGMII_RX_XGMII_PROT_ERR_CNT_RX_XGMII_PROT_ERR_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)

/**
 * Register Group: \a MAC10G::DEV_STATISTICS_XGMII_PERF
 *
 * XGMII statistics performance registers
 */


/** 
 * \brief Rx XGMII protocol error counter
 *
 * \details
 * Register: \a MAC10G::DEV_STATISTICS_XGMII_PERF::RX_XGMII_PROT_ERR_CNT_PERF
 *
 * @param target A \a ::vtss_target_MAC10G_e target
 */
#define VTSS_MAC10G_DEV_STATISTICS_XGMII_PERF_RX_XGMII_PROT_ERR_CNT_PERF(target)  VTSS_IOREG(target,0x140)

/** 
 * \brief
 * Number of XGMII protocol errors detected (performance monitor version)
 *
 * \details 
 * Field: ::VTSS_MAC10G_DEV_STATISTICS_XGMII_PERF_RX_XGMII_PROT_ERR_CNT_PERF . RX_XGMII_PROT_ERR_CNT_PERF
 */
#define  VTSS_F_MAC10G_DEV_STATISTICS_XGMII_PERF_RX_XGMII_PROT_ERR_CNT_PERF_RX_XGMII_PROT_ERR_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_MAC10G_DEV_STATISTICS_XGMII_PERF_RX_XGMII_PROT_ERR_CNT_PERF_RX_XGMII_PROT_ERR_CNT_PERF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_MAC10G_DEV_STATISTICS_XGMII_PERF_RX_XGMII_PROT_ERR_CNT_PERF_RX_XGMII_PROT_ERR_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


#endif /* _VTSS_DAYTONA_REGS_MAC10G_H_ */
