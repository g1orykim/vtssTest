#ifndef _VTSS_DAYTONA_REGS_PCS_XAUI_H_
#define _VTSS_DAYTONA_REGS_PCS_XAUI_H_

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
 * Target: \a PCS_XAUI
 *
 * \see vtss_target_PCS_XAUI_e
 *
 * XAUI compliant PCS used for 10 Gbps data transfer
 *
 ***********************************************************************/

/**
 * Register Group: \a PCS_XAUI::PCS_XAUI_CONFIGURATION
 *
 * PCS XAUI configuration
 */


/** 
 * \brief PCS XAUI configuration
 *
 * \details
 * PCS XAUI configuration register
 *
 * Register: \a PCS_XAUI::PCS_XAUI_CONFIGURATION::PCS_XAUI_CFG
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG(target)  VTSS_IOREG(target,0x0)

/** 
 * \brief
 * Line loopback that passes through the entire PCS in both directions
 *
 * \details 
 * 0: Normal operation
 * 1: Enable line loopback
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG . LINE_LOOP_ENA
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_LINE_LOOP_ENA(x)  VTSS_ENCODE_BITFIELD(x,21,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_LINE_LOOP_ENA     VTSS_ENCODE_BITMASK(21,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_LINE_LOOP_ENA(x)  VTSS_EXTRACT_BITFIELD(x,21,1)

/** 
 * \brief
 * Test loop, if enabled (XAUI encoded) data are looped from Tx path to Rx
 * path just before the SERDES
 *
 * \details 
 * 0: Normal operation
 * 1: Enable XAUI loopback
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG . XAUI_LOOP_ENA
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_XAUI_LOOP_ENA(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_XAUI_LOOP_ENA     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_XAUI_LOOP_ENA(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \brief
 * Enables the reporting of disparity and illegal symbol errors on the
 * XGMII interface with the K30.7 code when bad symbols are received.
 *
 * \details 
 * 0: Do not report disparity and illegal symbol errors on XGMII
 * 1: Report disparity and illegal symbol errors on XGMII using the K30.7
 * code
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG . RX_INGR_ERR_ENA
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_RX_INGR_ERR_ENA(x)  VTSS_ENCODE_BITFIELD(x,17,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_RX_INGR_ERR_ENA     VTSS_ENCODE_BITMASK(17,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_RX_INGR_ERR_ENA(x)  VTSS_EXTRACT_BITFIELD(x,17,1)

/** 
 * \brief
 * Idle sequencing mode (IPG shrink mode support). When active, the first
 * ||I|| after ||T|| will be alternatingly ||K||, ||A|| or ||R|| instead of
 * ||K|| or ||A|| only in normal mode
 *
 * \details 
 * 0: Normal idle sequencing
 * 1: Modified idle sequencing for IPG shrink mode support

 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG . IDLE_SEQ_MODE
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_IDLE_SEQ_MODE(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_IDLE_SEQ_MODE     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_IDLE_SEQ_MODE(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Disable Rx local fault generation when no alignment has been reached
 *
 * \details 
 * 0: Output local fault symbol at XGMII when not aligned
 * 1: Output IDLE symbols at XGMII when not aligned
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG . LF_GEN_DIS
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_LF_GEN_DIS(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_LF_GEN_DIS     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_LF_GEN_DIS(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Disable Rx pad/truncate mode
 *
 * \details 
 * 0: Normal operation
 * 1: Disable pad/truncate
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG . PT_DIS
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_PT_DIS(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_PT_DIS     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_PT_DIS(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * PCS enable
 *
 * \details 
 * 0: Disable PCS
 * 1: Enable PCS
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG . PCS_ENA
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_PCS_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_PCS_ENA     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_PCS_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief PCS XAUI extended configuration
 *
 * \details
 * Special configuration bit groups for PCS XAUI
 *
 * Register: \a PCS_XAUI::PCS_XAUI_CONFIGURATION::PCS_XAUI_EXT_CFG
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_EXT_CFG(target)  VTSS_IOREG(target,0x1)


/** 
 * \brief PCS XAUI SignalDetect configuration
 *
 * \details
 * PCS_XAUI signal_detect configuration
 *
 * Register: \a PCS_XAUI::PCS_XAUI_CONFIGURATION::PCS_XAUI_SD_CFG
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG(target)  VTSS_IOREG(target,0x2)

/** 
 * \brief
 * Bit to force the signal detect/LOS circuitry to indicate a loss of
 * signal (no valid signal). When asserted, the signal detect is forced low
 * internally causing a loss of synchronization.
 *
 * \details 
 * 0: Normal operation
 * 1: Signal detection is forced to 0 (signal lost).
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG . FORCE_LOS
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG_FORCE_LOS(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG_FORCE_LOS     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG_FORCE_LOS(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * LOS polarity
 * The signal level on LOS input pin must be equal to LOS_POL to indicate
 * loss of signal (LOS_ENA must be set).
 *
 * \details 
 * 0: LOS input pin must be 0 to indicate a loss of signal
 * 1: LOS input pin must be 1 to indicate a loss of signal
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG . LOS_POL
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG_LOS_POL(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG_LOS_POL     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG_LOS_POL(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Signal detect polarity
 * The signal level on the signal detect input pin must be equal to SD_POL
 * to indicate signal detection (SD_ENA must be set).
 *
 * \details 
 * 0: Signal detect input pin must be 0 to indicate a signal detection
 * 1: Signal detect input pin must be 1 to indicate a signal detection
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG . SD_POL
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG_SD_POL(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG_SD_POL     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG_SD_POL(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Loss-of-Signal (LOS) enable
 * When enabled, the LOS signal from the external device is used to
 * determine if a valid signal is available. When disabled, a valid signal
 * is assumed. The signal detect is ANDed with the LOS if both are enabled.
 * When both SD and LOS are enabled, both must show valid signal for the
 * PCS to see a valid signal.
 *
 * \details 
 * 0: The LOS input pin is ignored. The PCS assumes a valid signal at all
 * times.
 * 1: The LOS input pin is used to determine if a signal is detected.
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG . LOS_ENA
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG_LOS_ENA(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG_LOS_ENA     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG_LOS_ENA(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Signal detect enable
 * When enabled, the four signal detect signals from the four lanes are
 * used to determine if a valid signal is available. When disabled, a valid
 * signal is assumed. The signal detect is ANDed with the LOS if both are
 * enabled. When both SD and LOS are enabled, both must show valid signal
 * for the PCS to see a valid signal.
 *
 * \details 
 * 0: The signal detect input pins are ignored. The PCS assumes a valid
 * signal detect at all times.
 * 1: The signal detect input pins are used to determine if a signal is
 * detected.
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG . SD_ENA
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG_SD_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG_SD_ENA     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG_SD_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief PCS transmitter sequence configuration register
 *
 * \details
 * Sequence transmit configuration register
 *
 * Register: \a PCS_XAUI::PCS_XAUI_CONFIGURATION::PCS_XAUI_TX_SEQ_CFG
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_TX_SEQ_CFG(target)  VTSS_IOREG(target,0x3)

/** 
 * \brief
 * Transmit ||Q|| code (sequence information, i.e. lower 24 bit of a
 * sequence)
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_TX_SEQ_CFG . TX_Q
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_TX_SEQ_CFG_TX_Q(x)  VTSS_ENCODE_BITFIELD(x,8,24)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_TX_SEQ_CFG_TX_Q     VTSS_ENCODE_BITMASK(8,24)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_TX_SEQ_CFG_TX_Q(x)  VTSS_EXTRACT_BITFIELD(x,8,24)

/** 
 * \brief
 * Disable transmit ||Q|| code replacement
 *
 * \details 
 * 0: Enable
 * 1: Disable
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_TX_SEQ_CFG . TX_Q_DIS
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_TX_SEQ_CFG_TX_Q_DIS(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_TX_SEQ_CFG_TX_Q_DIS     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_TX_SEQ_CFG_TX_Q_DIS(x)  VTSS_EXTRACT_BITFIELD(x,7,1)


/** 
 * \brief PCS XAUI receiver error counter configuration
 *
 * \details
 * Error counter configuration register (if a bit in the mask field is set,
 * the errors of that lane are not counted).
 *
 * Register: \a PCS_XAUI::PCS_XAUI_CONFIGURATION::PCS_XAUI_RX_ERR_CNT_CFG
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG(target)  VTSS_IOREG(target,0x4)

/** 
 * \brief
 * Disparity error counting mask, one mask bit per lane. The lane will
 * count errors when the associated mask bit is 0.
 * <lane_3><lane_2><lane_1><lane_0>
 *
 * \details 
 * 0000: Count errors of all lanes
 * 1110: Count error of lane 0 only
 * ...
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG . DERR_MASK
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG_DERR_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,4)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG_DERR_MASK     VTSS_ENCODE_BITMASK(12,4)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG_DERR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,4)

/** 
 * \brief
 * Codegroup error counting mask, one mask bit per lane. The lane will
 * count errors when the associated mask bit is 0.
 * <lane_3><lane_2><lane_1><lane_0>
 *
 * \details 
 * 0000: Count errors of all lanes
 * 1110: Count error of lane 0 only
 * ...
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG . CERR_MASK
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG_CERR_MASK(x)  VTSS_ENCODE_BITFIELD(x,8,4)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG_CERR_MASK     VTSS_ENCODE_BITMASK(8,4)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG_CERR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,8,4)

/** 
 * \brief
 * FIFO underflow error counting mask, one mask bit per lane. The lane will
 * count errors when the associated mask bit is 0.
 * <lane_3><lane_2><lane_1><lane_0>
 *
 * \details 
 * 0000: Count errors of all lanes
 * 1110: Count error of lane 0 only
 * ...
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG . UFERR_MASK
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG_UFERR_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG_UFERR_MASK     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG_UFERR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,4)

/** 
 * \brief
 * FIFO overflow error counting mask, one mask bit per lane. The lane will
 * count errors when the associated mask bit is 0.
 * <lane_3><lane_2><lane_1><lane_0>
 *
 * \details 
 * 0000: Count errors of all lanes
 * 1110: Count error of lane 0 only
 * ...
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG . OFERR_MASK
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG_OFERR_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG_OFERR_MASK     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG_OFERR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief PCS interleave mode configuration
 *
 * \details
 * Configuration register set for interleave mode (XAUI via two lanes) -
 * also known as DDR-XAUI
 *
 * Register: \a PCS_XAUI::PCS_XAUI_CONFIGURATION::PCS_XAUI_INTERLEAVE_MODE_CFG
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG(target)  VTSS_IOREG(target,0x5)

/** 
 * \brief
 * Dual column ||A|| alignment (||A|| are inserted on even columns only)
 *
 * \details 
 * 0: Normal insertion
 * 1: Even column insertion only
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG . DC_A_ALIGN_ENA
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_DC_A_ALIGN_ENA(x)  VTSS_ENCODE_BITFIELD(x,17,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_DC_A_ALIGN_ENA     VTSS_ENCODE_BITMASK(17,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_DC_A_ALIGN_ENA(x)  VTSS_EXTRACT_BITFIELD(x,17,1)

/** 
 * \brief
 * Comma synchronization mode
 *
 * \details 
 * 0: Synchronize on any 7-bit comma (XAUI compliant)
 * 1: Synchronize on K28.5 only (non XAUI compliant)
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG . K28_5_SYNC_ENA
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_K28_5_SYNC_ENA(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_K28_5_SYNC_ENA     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_K28_5_SYNC_ENA(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Comma replacement. 
 * In interleave mode (using /K/ byte ordering) one 20-bit word must have
 * only one comma for proper alignment. Misleading commas are replaced by
 * comma_repl in transmit direction and replaced by K28.5-commas again in
 * receive direction. Comma_repl has to be an unused valid special
 * code-group that does not contain a comma, i.e. K28.2, K28.6, or K23.7
 * are possible replacements.
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG . COMMA_REPL
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_COMMA_REPL(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_COMMA_REPL     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_COMMA_REPL(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Disable 8b10b decoding of interleaved data stream
 *
 * \details 
 * 0: Interleaved data stream is 8b10b decoded
 * 1: Each lane is individually 8b10b decoded
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG . RX_8B10B_ILV_DIS
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_RX_8B10B_ILV_DIS(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_RX_8B10B_ILV_DIS     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_RX_8B10B_ILV_DIS(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Disable 8b10b encoding of interleaved data stream
 *
 * \details 
 * 0: Interleaved data stream is 8b10b encoded
 * 1: Each lane is individually 8b10b encoded
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG . TX_8B10B_ILV_DIS
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_TX_8B10B_ILV_DIS(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_TX_8B10B_ILV_DIS     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_TX_8B10B_ILV_DIS(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Interleave mode selection. In interleave mode XAUI data are sent via two
 * 5 Gbps lanes
 *
 * \details 
 * 0: Interleave mode with /K/ comma based byte re-ordering (using comma
 * replacement)
 * 1: Interleave mode with /A/ alignment symbol based byte re-ordering
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG . ILV_MODE
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Interleave mode enable. In interleave mode XAUI data are sent via two 5
 * Gbps lanes
 *
 * \details 
 * 0: Normal XAUI mode
 * 1: Interleave mode
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG . ILV_MODE_ENA
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE_ENA     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Spare register
 *
 * \details
 * Register: \a PCS_XAUI::PCS_XAUI_CONFIGURATION::PCS_XAUI_SPARE
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SPARE(target)  VTSS_IOREG(target,0x6)

/** 
 * \details 
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SPARE . PCS_XAUI_SPARE
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SPARE_PCS_XAUI_SPARE(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SPARE_PCS_XAUI_SPARE     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_CONFIGURATION_PCS_XAUI_SPARE_PCS_XAUI_SPARE(x)  VTSS_EXTRACT_BITFIELD(x,0,32)

/**
 * Register Group: \a PCS_XAUI::PCS_XAUI_STATUS
 *
 * PCS XAUI status
 */


/** 
 * \brief PCS status register
 *
 * \details
 * PCS status register
 *
 * Register: \a PCS_XAUI::PCS_XAUI_STATUS::PCS_XAUI_STATUS
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_STATUS(target)  VTSS_IOREG(target,0x10)

/** 
 * \brief
 * Status of the link
 *
 * \details 
 * 0: Link is not in LINK_OK state
 * 1: Link is in LINK_OK state
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_STATUS . LINK_STATE
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_STATUS_LINK_STATE(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_STATUS_LINK_STATE     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_STATUS_LINK_STATE(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Status of lane alignment
 *
 * \details 
 * 0: No alignment reached
 * 1: All lanes are aligned

 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_STATUS . ALIGNMENT_STATUS
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_STATUS_ALIGNMENT_STATUS(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_STATUS_ALIGNMENT_STATUS     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_STATUS_ALIGNMENT_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Status of code group alignment (lane independent), one bit for each
 * lane. The order of the bits is:
 * <lane_3><lane_2><lane_1><lane_0>
 *
 * \details 
 * 1111: All lanes in sync
 * 0001: Lane 0 is in sync
 * ...
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_STATUS . SYNC_STATUS
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_STATUS_SYNC_STATUS(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_STATUS_SYNC_STATUS     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_STATUS_SYNC_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief Interrupt register
 *
 * \details
 * Register of interrupt generating sticky bits
 *
 * Register: \a PCS_XAUI::PCS_XAUI_STATUS::PCS_XAUI_INT
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT(target)  VTSS_IOREG(target,0x11)

/** 
 * \brief
 * Interrupt indicating that the ordered set FIFO is full
 *
 * \details 
 * 0: Overhead FIFO not full
 * 1: Overhead FIFO full
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT . RX_FSET_FIFO_FULL_STICKY
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_RX_FSET_FIFO_FULL_STICKY(x)  VTSS_ENCODE_BITFIELD(x,25,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_RX_FSET_FIFO_FULL_STICKY     VTSS_ENCODE_BITMASK(25,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_RX_FSET_FIFO_FULL_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,25,1)

/** 
 * \brief
 * Interrupt indicating that an ordered set was received and captured in
 * the FIFO
 *
 * \details 
 * 0: No ordered set captured
 * 1: Ordered set captured in FIFO
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT . RX_FSET_STICKY
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_RX_FSET_STICKY(x)  VTSS_ENCODE_BITFIELD(x,24,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_RX_FSET_STICKY     VTSS_ENCODE_BITMASK(24,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_RX_FSET_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,24,1)

/** 
 * \brief
 * Received ||Fsig|| code changed
 *
 * \details 
 * 1: New ||Fsig|| has been received
 * 0: No new ||Fsig|| since last read
 * Bit is cleared by writing a 1 to this position
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT . RX_FSIG_CHANGED_STICKY
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_RX_FSIG_CHANGED_STICKY(x)  VTSS_ENCODE_BITFIELD(x,23,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_RX_FSIG_CHANGED_STICKY     VTSS_ENCODE_BITMASK(23,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_RX_FSIG_CHANGED_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,23,1)

/** 
 * \brief
 * Received ||Q|| code changed
 *
 * \details 
 * 1: New ||Q|| has been received
 * 0: No new ||Q|| since last read
 * Bit is cleared by writing a 1 to this position
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT . RX_Q_CHANGED_STICKY
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_RX_Q_CHANGED_STICKY(x)  VTSS_ENCODE_BITFIELD(x,22,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_RX_Q_CHANGED_STICKY     VTSS_ENCODE_BITMASK(22,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_RX_Q_CHANGED_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,22,1)

/** 
 * \brief
 * Coding error detected in received 8B/10B encoded data
 *
 * \details 
 * 0: No error found
 * 1: Coding error detected
 * Bit is cleared by writing a 1 to this position
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT . C8B10B_ERR_STICKY
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_C8B10B_ERR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,21,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_C8B10B_ERR_STICKY     VTSS_ENCODE_BITMASK(21,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_C8B10B_ERR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,21,1)

/** 
 * \brief
 * Alignment lost in deskew logic
 *
 * \details 
 * 0: No misalignment occurred
 * 1: A (temporary) misalignment has been detected
 * Bit is cleared by writing a 1 to this position
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT . ALIGNMENT_LOST_STICKY
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_ALIGNMENT_LOST_STICKY(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_ALIGNMENT_LOST_STICKY     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_ALIGNMENT_LOST_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \brief
 * Synchronization lost in lane i (i = 0...3, one bit per lane) The order
 * of the bits is:
 * <lane_3><lane_2><lane_1><lane_0>
 *
 * \details 
 * 0: No sync lost occurred
 * 1: Synchronization lost in lane i (temporarily)
 * Bit is cleared by writing a 1 to this position
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT . SYNC_LOST_STICKY
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_SYNC_LOST_STICKY(x)  VTSS_ENCODE_BITFIELD(x,16,4)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_SYNC_LOST_STICKY     VTSS_ENCODE_BITMASK(16,4)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_SYNC_LOST_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,16,4)

/** 
 * \brief
 * Local fault status (one or more of sync/align/fifo_of/fifo_uf/8b10b
 * error), one bit for each lane. The order of the bits is:
 * <lane_3><lane_2><lane_1><lane_0>
 *
 * \details 
 * 1: Fault occurred
 * 0: No fault detected
 * Bit is cleared by writing a 1 to this position
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT . LOCAL_FAULT_STICKY
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_LOCAL_FAULT_STICKY(x)  VTSS_ENCODE_BITFIELD(x,12,4)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_LOCAL_FAULT_STICKY     VTSS_ENCODE_BITMASK(12,4)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_LOCAL_FAULT_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,12,4)

/** 
 * \brief
 * Interrupt indicating that the ordered set FIFO is full
 *
 * \details 
 * 0: Overhead FIFO not full
 * 1: Overhead FIFO full
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT . RX_OSET_FIFO_FULL_STICKY
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_RX_OSET_FIFO_FULL_STICKY(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_RX_OSET_FIFO_FULL_STICKY     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_RX_OSET_FIFO_FULL_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Interrupt indicating that an ordered set was received and captured in
 * the FIFO
 *
 * \details 
 * 0: No ordered set captured
 * 1: Ordered set captured in FIFO
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT . RX_OSET_STICKY
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_RX_OSET_STICKY(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_RX_OSET_STICKY     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_RX_OSET_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * This bit is asserted when the PCS enters or leaves the LINK_OK state
 *
 * \details 
 * 0: No change
 * 1: Link has changed into, or out of, the LINK_OK state
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT . LINK_CHANGE_STICKY
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_LINK_CHANGE_STICKY(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_LINK_CHANGE_STICKY     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_LINK_CHANGE_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * A change was detected in ALIGNMENT_STATUS
 *
 * \details 
 * 0: No change
 * 1: A change was detected (rising or falling)
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT . ALIGNMENT_CHANGE_STICKY
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_ALIGNMENT_CHANGE_STICKY(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_ALIGNMENT_CHANGE_STICKY     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_ALIGNMENT_CHANGE_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * A change was detected in SYNC_STATUS. One bit is asserted per lane: 
 * <lane_3><lane_2><lane_1><lane_0>
 *
 * \details 
 * 0: No change
 * 1: A change was detected (rising or falling)
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT . SYNC_CHANGE_STICKY
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_SYNC_CHANGE_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_SYNC_CHANGE_STICKY     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_INT_SYNC_CHANGE_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief Mask register
 *
 * \details
 * Register of mask bits for PCS_XAUI_INT
 *
 * Register: \a PCS_XAUI::PCS_XAUI_STATUS::PCS_XAUI_MASK
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK(target)  VTSS_IOREG(target,0x12)

/** 
 * \brief
 * Interrupt mask bit for RX_FSET_FIFO_FULL_STICKY
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK . RX_FSET_FIFO_FULL_MASK
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_RX_FSET_FIFO_FULL_MASK(x)  VTSS_ENCODE_BITFIELD(x,25,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_RX_FSET_FIFO_FULL_MASK     VTSS_ENCODE_BITMASK(25,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_RX_FSET_FIFO_FULL_MASK(x)  VTSS_EXTRACT_BITFIELD(x,25,1)

/** 
 * \brief
 * Interrupt mask bit for RX_FSET_STICKY
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK . RX_FSET_MASK
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_RX_FSET_MASK(x)  VTSS_ENCODE_BITFIELD(x,24,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_RX_FSET_MASK     VTSS_ENCODE_BITMASK(24,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_RX_FSET_MASK(x)  VTSS_EXTRACT_BITFIELD(x,24,1)

/** 
 * \brief
 * Interrupt mask for RX_FSIG_CHANGED_STICKY
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK . RX_FSIG_CHANGED_MASK
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_RX_FSIG_CHANGED_MASK(x)  VTSS_ENCODE_BITFIELD(x,23,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_RX_FSIG_CHANGED_MASK     VTSS_ENCODE_BITMASK(23,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_RX_FSIG_CHANGED_MASK(x)  VTSS_EXTRACT_BITFIELD(x,23,1)

/** 
 * \brief
 * Interrupt mask for RX_Q_CHANGED_STICKY
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK . RX_Q_CHANGED_INT_MASK
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_RX_Q_CHANGED_INT_MASK(x)  VTSS_ENCODE_BITFIELD(x,22,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_RX_Q_CHANGED_INT_MASK     VTSS_ENCODE_BITMASK(22,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_RX_Q_CHANGED_INT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,22,1)

/** 
 * \brief
 * Interrupt mask for C8B10B_ERR_STICKY
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK . C8B10B_ERR_INT_MASK
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_C8B10B_ERR_INT_MASK(x)  VTSS_ENCODE_BITFIELD(x,21,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_C8B10B_ERR_INT_MASK     VTSS_ENCODE_BITMASK(21,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_C8B10B_ERR_INT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,21,1)

/** 
 * \brief
 * Interrupt mask for ALIGNMENT_LOST_STICKY
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK . ALIGNMENT_LOST_INT_MASK
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_ALIGNMENT_LOST_INT_MASK(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_ALIGNMENT_LOST_INT_MASK     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_ALIGNMENT_LOST_INT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \brief
 * Interrupt mask for SYNC_LOST_STICKY, one bit per lane.   
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK . SYNC_LOST_INT_MASK
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_SYNC_LOST_INT_MASK(x)  VTSS_ENCODE_BITFIELD(x,16,4)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_SYNC_LOST_INT_MASK     VTSS_ENCODE_BITMASK(16,4)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_SYNC_LOST_INT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,16,4)

/** 
 * \brief
 * Interrupt mask for LOCAL_FAULT_STICKY, one bit for each lane. The order
 * of the bits is:
 * <lane_3><lane_2><lane_1><lane_0>
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK . LOCAL_FAULT_MASK
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_LOCAL_FAULT_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,4)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_LOCAL_FAULT_MASK     VTSS_ENCODE_BITMASK(12,4)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_LOCAL_FAULT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,4)

/** 
 * \brief
 * Interrupt mask for RX_OSET_FIFO_FULL_STICKY
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK . RX_OSET_FIFO_FULL_MASK
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_RX_OSET_FIFO_FULL_MASK(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_RX_OSET_FIFO_FULL_MASK     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_RX_OSET_FIFO_FULL_MASK(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Interrupt mask for the RX_OSET_STICKY bit
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK . RX_OSET_MASK
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_RX_OSET_MASK(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_RX_OSET_MASK     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_RX_OSET_MASK(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Interrupt mask for LINK_CHANGE_STICKY
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK . LINK_CHANGE_MASK
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_LINK_CHANGE_MASK(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_LINK_CHANGE_MASK     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_LINK_CHANGE_MASK(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Interrupt mask for ALIGNMENT_CHANGE_STICKY
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK . ALIGNMENT_CHANGE_MASK
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_ALIGNMENT_CHANGE_MASK(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_ALIGNMENT_CHANGE_MASK     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_ALIGNMENT_CHANGE_MASK(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Interrupt mask for SYNC_CHANGE_STICKY, one bit per lane.
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK . SYNC_CHANGE_MASK
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_SYNC_CHANGE_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_SYNC_CHANGE_MASK     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_MASK_SYNC_CHANGE_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief PCS receiver sequence result register
 *
 * \details
 * ||Q|| Sequence receive status register
 *
 * Register: \a PCS_XAUI::PCS_XAUI_STATUS::PCS_XAUI_RX_SEQ_REC_STATUS
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_RX_SEQ_REC_STATUS(target)  VTSS_IOREG(target,0x13)

/** 
 * \brief
 * Received ||Q|| code (sequence information, i.e. lower 24 bit of a
 * sequence)
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_RX_SEQ_REC_STATUS . RX_Q
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_RX_SEQ_REC_STATUS_RX_Q(x)  VTSS_ENCODE_BITFIELD(x,8,24)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_RX_SEQ_REC_STATUS_RX_Q     VTSS_ENCODE_BITMASK(8,24)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_RX_SEQ_REC_STATUS_RX_Q(x)  VTSS_EXTRACT_BITFIELD(x,8,24)


/** 
 * \brief PCS receiver signal ordered set result register
 *
 * \details
 * ||Fsig|| sequence receive status register
 *
 * Register: \a PCS_XAUI::PCS_XAUI_STATUS::PCS_XAUI_RX_FSIG_REC_STATUS
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_RX_FSIG_REC_STATUS(target)  VTSS_IOREG(target,0x14)

/** 
 * \brief
 * Received ||Fsig|| code (sequence information, i.e. lower 24 bit of a
 * sequence)
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_RX_FSIG_REC_STATUS . RX_FSIG
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_RX_FSIG_REC_STATUS_RX_FSIG(x)  VTSS_ENCODE_BITFIELD(x,8,24)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_RX_FSIG_REC_STATUS_RX_FSIG     VTSS_ENCODE_BITMASK(8,24)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_PCS_XAUI_RX_FSIG_REC_STATUS_RX_FSIG(x)  VTSS_EXTRACT_BITFIELD(x,8,24)


/** 
 * \brief Status of ||Q|| overhead FIFO
 *
 * \details
 * Contains status information for the FIFO containing captured Rx sequence
 * ordered sets
 *
 * Register: \a PCS_XAUI::PCS_XAUI_STATUS::RX_OSET_FIFO_STAT
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_STATUS_RX_OSET_FIFO_STAT(target)  VTSS_IOREG(target,0x15)

/** 
 * \brief
 * Indicates if the FIFO is full
 *
 * \details 
 * 0: FIFO not full
 * 1: FIFO full
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_RX_OSET_FIFO_STAT . RX_OSET_FIFO_FULL
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_RX_OSET_FIFO_STAT_RX_OSET_FIFO_FULL(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_RX_OSET_FIFO_STAT_RX_OSET_FIFO_FULL     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_RX_OSET_FIFO_STAT_RX_OSET_FIFO_FULL(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Number of valid ordered sets in the FIFO that can be read
 *
 * \details 
 * Binary number
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_RX_OSET_FIFO_STAT . RX_OSET_FIFO_NUM
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_RX_OSET_FIFO_STAT_RX_OSET_FIFO_NUM(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_RX_OSET_FIFO_STAT_RX_OSET_FIFO_NUM     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_RX_OSET_FIFO_STAT_RX_OSET_FIFO_NUM(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Rx ||Q|| overhead FIFO data
 *
 * \details
 * The register interface to the sequence ordered set data
 *
 * Register: \a PCS_XAUI::PCS_XAUI_STATUS::RX_OSET_FIFO_DATA
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_STATUS_RX_OSET_FIFO_DATA(target)  VTSS_IOREG(target,0x16)

/** 
 * \brief
 * Register interface to the FIFO containing captured ordered sets. Each
 * read of this register pops a 24-bit ordered set off the FIFO and
 * increments the FIFO pointer. The data is only the upper 24 bits and does
 * not include the control character.
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_RX_OSET_FIFO_DATA . RX_OSET_FIFO_DATA
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_RX_OSET_FIFO_DATA_RX_OSET_FIFO_DATA(x)  VTSS_ENCODE_BITFIELD(x,8,24)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_RX_OSET_FIFO_DATA_RX_OSET_FIFO_DATA     VTSS_ENCODE_BITMASK(8,24)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_RX_OSET_FIFO_DATA_RX_OSET_FIFO_DATA(x)  VTSS_EXTRACT_BITFIELD(x,8,24)


/** 
 * \brief Status of ||Fsig|| overhead FIFO
 *
 * \details
 * Contains status information for the FIFO containing captured Rx signal
 * ordered sets
 *
 * Register: \a PCS_XAUI::PCS_XAUI_STATUS::RX_FSET_FIFO_STAT
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_STATUS_RX_FSET_FIFO_STAT(target)  VTSS_IOREG(target,0x17)

/** 
 * \brief
 * Indicates if the FIFO is full
 *
 * \details 
 * 0: FIFO not full
 * 1: FIFO full
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_RX_FSET_FIFO_STAT . RX_FSET_FIFO_FULL
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_RX_FSET_FIFO_STAT_RX_FSET_FIFO_FULL(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_RX_FSET_FIFO_STAT_RX_FSET_FIFO_FULL     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_RX_FSET_FIFO_STAT_RX_FSET_FIFO_FULL(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Number of valid ordered sets in the FIFO that can be read
 *
 * \details 
 * Binary number
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_RX_FSET_FIFO_STAT . RX_FSET_FIFO_NUM
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_RX_FSET_FIFO_STAT_RX_FSET_FIFO_NUM(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_RX_FSET_FIFO_STAT_RX_FSET_FIFO_NUM     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_RX_FSET_FIFO_STAT_RX_FSET_FIFO_NUM(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Rx ||Fsig|| overhead FIFO data
 *
 * \details
 * The register interface to the signal ordered set data
 *
 * Register: \a PCS_XAUI::PCS_XAUI_STATUS::RX_FSET_FIFO_DATA
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_STATUS_RX_FSET_FIFO_DATA(target)  VTSS_IOREG(target,0x18)

/** 
 * \brief
 * Register interface to the FIFO containing captured ordered sets. Each
 * read of this register pops a 24-bit ordered set off the FIFO and
 * increments the FIFO pointer. The data is only the upper 24 bits and does
 * not include the control character.
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_STATUS_RX_FSET_FIFO_DATA . RX_FSET_FIFO_DATA
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_STATUS_RX_FSET_FIFO_DATA_RX_FSET_FIFO_DATA(x)  VTSS_ENCODE_BITFIELD(x,8,24)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_STATUS_RX_FSET_FIFO_DATA_RX_FSET_FIFO_DATA     VTSS_ENCODE_BITMASK(8,24)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_STATUS_RX_FSET_FIFO_DATA_RX_FSET_FIFO_DATA(x)  VTSS_EXTRACT_BITFIELD(x,8,24)

/**
 * Register Group: \a PCS_XAUI::PCS_XAUI_ERR_COUNTERS
 *
 * PCS error counters
 */


/** 
 * \brief Alignment error counter
 *
 * \details
 * Register: \a PCS_XAUI::PCS_XAUI_ERR_COUNTERS::PCS_XAUI_RX_ALIGN_ERR_CNT
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_ALIGN_ERR_CNT(target)  VTSS_IOREG(target,0x30)

/** 
 * \brief
 * This counter counts the number of alignment errors in the Rx direction
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_ALIGN_ERR_CNT . PCS_XAUI_RX_ALIGN_ERR_CNT
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_ALIGN_ERR_CNT_PCS_XAUI_RX_ALIGN_ERR_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_ALIGN_ERR_CNT_PCS_XAUI_RX_ALIGN_ERR_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_ALIGN_ERR_CNT_PCS_XAUI_RX_ALIGN_ERR_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief XGMII sequence error counter
 *
 * \details
 * Counts the number of invalid XGMII codes in the Tx direction
 *
 * Register: \a PCS_XAUI::PCS_XAUI_ERR_COUNTERS::PCS_XAUI_XGMII_ERR_CNT
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_XGMII_ERR_CNT(target)  VTSS_IOREG(target,0x31)

/** 
 * \brief
 * Counts the number of /E/ control codes that are generated in the Tx
 * path. This may be more than the number of invalid XGMII errors present
 * at the input XGMII interface. e.g. if 1 character in an input IDLE
 * column is corrupted, this will result in an invalid XGMII control code
 * and 4 output error characters. This counter will increment by 4.
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_XGMII_ERR_CNT . PCS_XAUI_XGMII_ERR_CNT
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_XGMII_ERR_CNT_PCS_XAUI_XGMII_ERR_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_XGMII_ERR_CNT_PCS_XAUI_XGMII_ERR_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_XGMII_ERR_CNT_PCS_XAUI_XGMII_ERR_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief PCS Rx FIFO overflow error and lane 0 error counter
 *
 * \details
 * Receive FIFO overflow error counter. 
 * In normal mode provides the count of FIFO overflow errors. In test
 * pattern check mode, this counter counts the errors of lane 0. In the
 * latter case the counter is incremented by one whenever at least one out
 * of eighty received bits (eight symbols) is corrupted. In test-pattern
 * mode, this counter is shared between normal and PRBS pattern blocks.
 *
 * Register: \a PCS_XAUI::PCS_XAUI_ERR_COUNTERS::PCS_XAUI_RX_FIFO_OF_ERR_L0_CNT_STATUS
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_OF_ERR_L0_CNT_STATUS(target)  VTSS_IOREG(target,0x32)

/** 
 * \brief
 * Number of detected FIFO overflow errors/number of errors in lane 0
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_OF_ERR_L0_CNT_STATUS . ERR_CNT_FIFO_OF_L0
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_OF_ERR_L0_CNT_STATUS_ERR_CNT_FIFO_OF_L0(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_OF_ERR_L0_CNT_STATUS_ERR_CNT_FIFO_OF_L0     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_OF_ERR_L0_CNT_STATUS_ERR_CNT_FIFO_OF_L0(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief PCS Rx FIFO underflow error and lane 1 error counter
 *
 * \details
 * Receive FIFO underflow error counter. In normal mode provides the count
 * of FIFO underflow errors. In test pattern check mode, this counter
 * counts the errors of lane 1. In the latter case the counter is
 * incremented by one whenever at least one out of eighty received bits
 * (eight symbols) is corrupted. In test-pattern mode, this counter is
 * shared between normal and PRBS pattern blocks.
 *
 * Register: \a PCS_XAUI::PCS_XAUI_ERR_COUNTERS::PCS_XAUI_RX_FIFO_UF_ERR_L1_CNT_STATUS
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_UF_ERR_L1_CNT_STATUS(target)  VTSS_IOREG(target,0x33)

/** 
 * \brief
 * Number of detected FIFO underflow errors/number of errors in lane 1
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_UF_ERR_L1_CNT_STATUS . ERR_CNT_FIFO_UF_L1
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_UF_ERR_L1_CNT_STATUS_ERR_CNT_FIFO_UF_L1(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_UF_ERR_L1_CNT_STATUS_ERR_CNT_FIFO_UF_L1     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_UF_ERR_L1_CNT_STATUS_ERR_CNT_FIFO_UF_L1(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief PCS Rx 10b8b disparity error and lane 2 error counter
 *
 * \details
 * 10b8b decoder disparity error counter. In normal mode provides the count
 * of disparity errors. In test pattern check mode, this counter counts the
 * errors of lane 2. In the latter case the counter is incremented by one
 * whenever at least one out of eighty received bits (eight symbols) is
 * corrupted. In test-pattern mode, this counter is shared between normal
 * and PRBS pattern blocks.
 *
 * Register: \a PCS_XAUI::PCS_XAUI_ERR_COUNTERS::PCS_XAUI_RX_FIFO_D_ERR_L2_CNT_STATUS
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_D_ERR_L2_CNT_STATUS(target)  VTSS_IOREG(target,0x34)

/** 
 * \brief
 * Number of detected disparity errors/number of errors in lane 2
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_D_ERR_L2_CNT_STATUS . ERR_CNT_10B8B_D_L2
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_D_ERR_L2_CNT_STATUS_ERR_CNT_10B8B_D_L2(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_D_ERR_L2_CNT_STATUS_ERR_CNT_10B8B_D_L2     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_D_ERR_L2_CNT_STATUS_ERR_CNT_10B8B_D_L2(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief PCS Rx 10b8b codegroup error and lane 3 error counter
 *
 * \details
 * 10b8b decoder codegroup error counter. In normal mode provides the count
 * of codegroup errors. In test pattern check mode, this counter counts the
 * errors of lane 3. In the latter case the counter is incremented by one
 * whenever at least one out of eighty received bits (eight symbols) is
 * corrupted. In test-pattern mode, this counter is shared between normal
 * and PRBS pattern blocks.
 *
 * Register: \a PCS_XAUI::PCS_XAUI_ERR_COUNTERS::PCS_XAUI_RX_FIFO_CG_ERR_L3_CNT_STATUS
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_CG_ERR_L3_CNT_STATUS(target)  VTSS_IOREG(target,0x35)

/** 
 * \brief
 * Number of detected codegroup errors/number of errors in lane 3
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_CG_ERR_L3_CNT_STATUS . ERR_CNT_10B8B_CG_L3
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_CG_ERR_L3_CNT_STATUS_ERR_CNT_10B8B_CG_L3(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_CG_ERR_L3_CNT_STATUS_ERR_CNT_10B8B_CG_L3     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_ERR_COUNTERS_PCS_XAUI_RX_FIFO_CG_ERR_L3_CNT_STATUS_ERR_CNT_10B8B_CG_L3(x)  VTSS_EXTRACT_BITFIELD(x,0,32)

/**
 * Register Group: \a PCS_XAUI::PCS_XAUI_COUNTERS
 *
 * PCS counters
 */


/** 
 * \brief Good byte counter, Rx
 *
 * \details
 * Register: \a PCS_XAUI::PCS_XAUI_COUNTERS::PCS_XAUI_RX_OK_BYTES_CNT
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_RX_OK_BYTES_CNT(target)  VTSS_IOREG(target,0x36)

/** 
 * \brief
 * Counts the number of good payload bytes in the Rx direction. Payload is
 * defined as any byte that is marked as data that is not part of an
 * ordered set. This is meant for 10 G Fibre Channel but works in Ethernet
 * mode as well. Read the LSB portion first to capture the upper portion.
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_RX_OK_BYTES_CNT . PCS_XAUI_RX_OK_BYTES_CNT
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_RX_OK_BYTES_CNT_PCS_XAUI_RX_OK_BYTES_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_RX_OK_BYTES_CNT_PCS_XAUI_RX_OK_BYTES_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_RX_OK_BYTES_CNT_PCS_XAUI_RX_OK_BYTES_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Good byte counter, Rx, MSB portion
 *
 * \details
 * Register: \a PCS_XAUI::PCS_XAUI_COUNTERS::PCS_XAUI_RX_OK_BYTES_CNT_MSB
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_RX_OK_BYTES_CNT_MSB(target)  VTSS_IOREG(target,0x37)

/** 
 * \brief
 * Counts the number of good payload bytes in the Rx direction. Payload is
 * defined as any byte that is marked as data that is not part of an
 * ordered set. This is meant for 10 G Fibre Channel but works in Ethernet
 * mode as well. MSB portion.
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_RX_OK_BYTES_CNT_MSB . PCS_XAUI_RX_OK_BYTES_CNT_MSB
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_RX_OK_BYTES_CNT_MSB_PCS_XAUI_RX_OK_BYTES_CNT_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_RX_OK_BYTES_CNT_MSB_PCS_XAUI_RX_OK_BYTES_CNT_MSB     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_RX_OK_BYTES_CNT_MSB_PCS_XAUI_RX_OK_BYTES_CNT_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Good byte counter, Tx
 *
 * \details
 * Register: \a PCS_XAUI::PCS_XAUI_COUNTERS::PCS_XAUI_TX_OK_BYTES_CNT
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_TX_OK_BYTES_CNT(target)  VTSS_IOREG(target,0x38)

/** 
 * \brief
 * Counts the number of good payload bytes in the Tx direction. Payload is
 * defined as any byte that is marked as data that is not part of an
 * ordered set. This is meant for 10 G Fibre Channel but works in Ethernet
 * mode as well. Read the LSB portion first to capture the upper portion.
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_TX_OK_BYTES_CNT . PCS_XAUI_TX_OK_BYTES_CNT
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_TX_OK_BYTES_CNT_PCS_XAUI_TX_OK_BYTES_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_TX_OK_BYTES_CNT_PCS_XAUI_TX_OK_BYTES_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_TX_OK_BYTES_CNT_PCS_XAUI_TX_OK_BYTES_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Good byte counter, Tx, MSB portion
 *
 * \details
 * Register: \a PCS_XAUI::PCS_XAUI_COUNTERS::PCS_XAUI_TX_OK_BYTES_CNT_MSB
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_TX_OK_BYTES_CNT_MSB(target)  VTSS_IOREG(target,0x39)

/** 
 * \brief
 * Counts the number of good payload bytes in the Tx direction. Payload is
 * defined as any byte that is marked as data that is not part of an
 * ordered set. This is meant for 10G Fibre Channel but works in Ethernet
 * mode as well. MSB portion.
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_TX_OK_BYTES_CNT_MSB . PCS_XAUI_TX_OK_BYTES_CNT_MSB
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_TX_OK_BYTES_CNT_MSB_PCS_XAUI_TX_OK_BYTES_CNT_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_TX_OK_BYTES_CNT_MSB_PCS_XAUI_TX_OK_BYTES_CNT_MSB     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_COUNTERS_PCS_XAUI_TX_OK_BYTES_CNT_MSB_PCS_XAUI_TX_OK_BYTES_CNT_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a PCS_XAUI::PCS_XAUI_TSTPAT_CONFIGURATION
 *
 * PCS XAUI test pattern configuration
 */


/** 
 * \brief PCS XAUI test pattern configuration
 *
 * \details
 * Test pattern generator/checker control register
 *
 * Register: \a PCS_XAUI::PCS_XAUI_TSTPAT_CONFIGURATION::PCS_XAUI_TSTPAT_CFG
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG(target)  VTSS_IOREG(target,0x3a)

/** 
 * \brief
 * Enable test pattern checker
 *
 * \details 
 * 0: No checking
 * 1: Check
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG . VT_CHK_ENA
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG_VT_CHK_ENA(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG_VT_CHK_ENA     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG_VT_CHK_ENA(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Check test pattern
 *
 * \details 
 * 000: Idle
 * 011: Reserved
 * 100: CRPAT
 * 101: CJPAT
 * all others: Idle
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG . VT_CHK_SEL
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG_VT_CHK_SEL(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG_VT_CHK_SEL     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG_VT_CHK_SEL(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * Enable test pattern generator
 *
 * \details 
 * 0: Normal operation
 * 1: Generate test vectors
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG . VT_GEN_ENA
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG_VT_GEN_ENA(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG_VT_GEN_ENA     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG_VT_GEN_ENA(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Generate test pattern
 *
 * \details 
 * 000: Idle
 * 001: Reserved
 * 010: Reserved
 * 011: Reserved
 * 100: CRPAT
 * 101: CJPAT
 * All others: Idle
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG . VT_GEN_SEL
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG_VT_GEN_SEL(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG_VT_GEN_SEL     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG_VT_GEN_SEL(x)  VTSS_EXTRACT_BITFIELD(x,0,3)

/**
 * Register Group: \a PCS_XAUI::PCS_XAUI_TSTPAT_STATUS
 *
 * PCS XAUI test pattern status
 */

/**
 * Register Group: \a PCS_XAUI::PCS_XAUI_PRBS_TSTPAT_CONFIGURATION
 *
 * XAUI PRBS test pattern generator
 */


/** 
 * \brief Test pattern generator/checker control
 *
 * \details
 * Register: \a PCS_XAUI::PCS_XAUI_PRBS_TSTPAT_CONFIGURATION::PCS10G_TSTPAT_CTRL_CFG
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG(target)  VTSS_IOREG(target,0x5a)

/** 
 * \brief
 * PRBS flip pattern
 *
 * \details 
 * 0: Normal use of PRBS
 * 1: Flip pattern from PRBS generator
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG . PRBS_BUS_FLIP
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG_PRBS_BUS_FLIP(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG_PRBS_BUS_FLIP     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG_PRBS_BUS_FLIP(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * PRBS pattern inversion
 *
 * \details 
 * 0: Normal polarity of polynomial
 * 1: Invert PRBS polynomial
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG . PRBS_POLY_INV
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG_PRBS_POLY_INV(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG_PRBS_POLY_INV     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG_PRBS_POLY_INV(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Capture current error counter values
 *
 * \details 
 * 0: Normal operation
 * 1: Capture 
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG . FREEZE_ERR_CNT_ENA
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG_FREEZE_ERR_CNT_ENA(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG_FREEZE_ERR_CNT_ENA     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG_FREEZE_ERR_CNT_ENA(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Enable test pattern checker
 *
 * \details 
 * 0: No checking
 * 1: Check
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG . VT_CHK_ENA
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG_VT_CHK_ENA(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG_VT_CHK_ENA     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG_VT_CHK_ENA(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Check test pattern
 *
 * \details 
 * 0000: Idle
 * 0001: HFPAT
 * 0010: LFPAT
 * 0011: MFPAT
 * 0100: Reserved
 * 0101: Reserved
 * 0110: Reserved
 * 0111: PRBS (2^7-1)
 * 1000: PRBS (2^23-1)
 * 1001: PRBS (2^31-1)
 * 1010: PRPAT
 * 1011...1111: Idle
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG . VT_CHK_SEL
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG_VT_CHK_SEL(x)  VTSS_ENCODE_BITFIELD(x,5,4)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG_VT_CHK_SEL     VTSS_ENCODE_BITMASK(5,4)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG_VT_CHK_SEL(x)  VTSS_EXTRACT_BITFIELD(x,5,4)

/** 
 * \brief
 * Enable test pattern generator
 *
 * \details 
 * 0: Normal operation
 * 1: Generate
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG . VT_GEN_ENA
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG_VT_GEN_ENA(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG_VT_GEN_ENA     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG_VT_GEN_ENA(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Generate test pattern
 *
 * \details 
 * 0000: Idle
 * 0001: HFPAT
 * 0010: LFPAT
 * 0011: MFPAT
 * 0100: Reserved
 * 0101: Reserved
 * 0110: Reserved
 * 0111: PRBS (2^7-1)
 * 1000: PRBS (2^23-1)
 * 1001: PRBS (2^31-1)
 * 1010: PRPAT
 * 1011...1111: Idle
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG . VT_GEN_SEL
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG_VT_GEN_SEL(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG_VT_GEN_SEL     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_CTRL_CFG_VT_GEN_SEL(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief Programmable pattern 0
 *
 * \details
 * Pattern provided at transmitter resp. pattern to check against in
 * receiver when test pattern generator/checker is in PRPAT (PRogrammable
 * PATtern) mode. This register is for lane 0 only.
 * The specified pattern is sent directly out on the wire, no disparity
 * protection is performed. Try to use DC-balanced patterns otherwise
 * AC-coupled lines might saturate at a rail and cause errors.
 *
 * Register: \a PCS_XAUI::PCS_XAUI_PRBS_TSTPAT_CONFIGURATION::PCS10G_TSTPAT_PRPAT_L0_CFG
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L0_CFG(target)  VTSS_IOREG(target,0x5b)

/** 
 * \brief
 * Constant checker pattern for lane 0
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L0_CFG . CHK_PAT_L0
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L0_CFG_CHK_PAT_L0(x)  VTSS_ENCODE_BITFIELD(x,16,10)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L0_CFG_CHK_PAT_L0     VTSS_ENCODE_BITMASK(16,10)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L0_CFG_CHK_PAT_L0(x)  VTSS_EXTRACT_BITFIELD(x,16,10)

/** 
 * \brief
 * Constant generator pattern for lane 0
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L0_CFG . GEN_PAT_L0
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L0_CFG_GEN_PAT_L0(x)  VTSS_ENCODE_BITFIELD(x,0,10)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L0_CFG_GEN_PAT_L0     VTSS_ENCODE_BITMASK(0,10)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L0_CFG_GEN_PAT_L0(x)  VTSS_EXTRACT_BITFIELD(x,0,10)


/** 
 * \brief Programmable pattern 1
 *
 * \details
 * Pattern provided at transmitter resp. pattern to check against in
 * receiver when test pattern generator/checker is in PRPAT (PRogrammable
 * PATtern) mode. This register is for lane 1 only.
 * The specified pattern is sent directly out on the wire, no disparity
 * protection is performed. Try to use DC-balanced patterns otherwise
 * AC-coupled lines might saturate at a rail and cause errors.
 *
 * Register: \a PCS_XAUI::PCS_XAUI_PRBS_TSTPAT_CONFIGURATION::PCS10G_TSTPAT_PRPAT_L1_CFG
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L1_CFG(target)  VTSS_IOREG(target,0x5c)

/** 
 * \brief
 * Constant checker pattern for lane 1
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L1_CFG . CHK_PAT_L1
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L1_CFG_CHK_PAT_L1(x)  VTSS_ENCODE_BITFIELD(x,16,10)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L1_CFG_CHK_PAT_L1     VTSS_ENCODE_BITMASK(16,10)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L1_CFG_CHK_PAT_L1(x)  VTSS_EXTRACT_BITFIELD(x,16,10)

/** 
 * \brief
 * Constant generator pattern for lane 1
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L1_CFG . GEN_PAT_L1
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L1_CFG_GEN_PAT_L1(x)  VTSS_ENCODE_BITFIELD(x,0,10)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L1_CFG_GEN_PAT_L1     VTSS_ENCODE_BITMASK(0,10)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L1_CFG_GEN_PAT_L1(x)  VTSS_EXTRACT_BITFIELD(x,0,10)


/** 
 * \brief Programmable pattern 2
 *
 * \details
 * Pattern provided at transmitter resp. pattern to check against in
 * receiver when test pattern generator/checker is in PRPAT (PRogrammable
 * PATtern) mode. This register is for lane 2 only.
 * The specified pattern is sent directly out on the wire, no disparity
 * protection is performed. Try to use DC-balanced patterns otherwise
 * AC-coupled lines might saturate at a rail and cause errors.
 *
 * Register: \a PCS_XAUI::PCS_XAUI_PRBS_TSTPAT_CONFIGURATION::PCS10G_TSTPAT_PRPAT_L2_CFG
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L2_CFG(target)  VTSS_IOREG(target,0x5d)

/** 
 * \brief
 * Constant checker pattern for lane 2
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L2_CFG . CHK_PAT_L2
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L2_CFG_CHK_PAT_L2(x)  VTSS_ENCODE_BITFIELD(x,16,10)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L2_CFG_CHK_PAT_L2     VTSS_ENCODE_BITMASK(16,10)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L2_CFG_CHK_PAT_L2(x)  VTSS_EXTRACT_BITFIELD(x,16,10)

/** 
 * \brief
 * Constant generator pattern for lane 2
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L2_CFG . GEN_PAT_L2
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L2_CFG_GEN_PAT_L2(x)  VTSS_ENCODE_BITFIELD(x,0,10)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L2_CFG_GEN_PAT_L2     VTSS_ENCODE_BITMASK(0,10)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L2_CFG_GEN_PAT_L2(x)  VTSS_EXTRACT_BITFIELD(x,0,10)


/** 
 * \brief Programmable pattern 3
 *
 * \details
 * Pattern provided at transmitter resp. pattern to check against in
 * receiver when test pattern generator/checker is in PRPAT (PRogrammable
 * PATtern) mode. This register is for lane 3 only.
 * The specified pattern is sent directly out on the wire, no disparity
 * protection is performed. Try to use DC-balanced patterns otherwise
 * AC-coupled lines might saturate at a rail and cause errors.
 *
 * Register: \a PCS_XAUI::PCS_XAUI_PRBS_TSTPAT_CONFIGURATION::PCS10G_TSTPAT_PRPAT_L3_CFG
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L3_CFG(target)  VTSS_IOREG(target,0x5e)

/** 
 * \brief
 * Constant checker pattern for lane 3
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L3_CFG . CHK_PAT_L3
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L3_CFG_CHK_PAT_L3(x)  VTSS_ENCODE_BITFIELD(x,16,10)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L3_CFG_CHK_PAT_L3     VTSS_ENCODE_BITMASK(16,10)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L3_CFG_CHK_PAT_L3(x)  VTSS_EXTRACT_BITFIELD(x,16,10)

/** 
 * \brief
 * Constant generator pattern for lane 3
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L3_CFG . GEN_PAT_L3
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L3_CFG_GEN_PAT_L3(x)  VTSS_ENCODE_BITFIELD(x,0,10)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L3_CFG_GEN_PAT_L3     VTSS_ENCODE_BITMASK(0,10)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_CONFIGURATION_PCS10G_TSTPAT_PRPAT_L3_CFG_GEN_PAT_L3(x)  VTSS_EXTRACT_BITFIELD(x,0,10)

/**
 * Register Group: \a PCS_XAUI::PCS_XAUI_PRBS_TSTPAT_STATUS
 *
 * XAUI PRBS test pattern generator
 */


/** 
 * \brief Test pattern status register
 *
 * \details
 * Register: \a PCS_XAUI::PCS_XAUI_PRBS_TSTPAT_STATUS::PCS10G_TSTPAT_STATUS
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_STATUS_PCS10G_TSTPAT_STATUS(target)  VTSS_IOREG(target,0x6a)

/** 
 * \brief
 * For each lane, indicates if the selected pattern is matching what is
 * being received
 *
 * \details 
 * 0: No match
 * 1: Match
 *
 * Field: ::VTSS_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_STATUS_PCS10G_TSTPAT_STATUS . PATTERN_MATCH
 */
#define  VTSS_F_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_STATUS_PCS10G_TSTPAT_STATUS_PATTERN_MATCH(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_STATUS_PCS10G_TSTPAT_STATUS_PATTERN_MATCH     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_PCS_XAUI_PCS_XAUI_PRBS_TSTPAT_STATUS_PCS10G_TSTPAT_STATUS_PATTERN_MATCH(x)  VTSS_EXTRACT_BITFIELD(x,0,4)

/**
 * Register Group: \a PCS_XAUI::ANEG_CFG
 *
 * ANEG configuration registers
 */


/** 
 * \brief ANEG configuration
 *
 * \details
 * Auto-negotiation configuration register. 
 * Note: Setting one of the parallel detect wait times to 0 disables
 * parallel detect function for that specific mode.
 *
 * Register: \a PCS_XAUI::ANEG_CFG::ANEG_CFG
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_ANEG_CFG_ANEG_CFG(target)  VTSS_IOREG(target,0x6b)

/** 
 * \brief
 * Disable automatic ANEG OB configuration
 *
 * \details 
 * 0: Allow ANEG block to control OB during auto-negotiation
 * 1: OB settings are not touched by ANEG block
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_CFG_ANEG_CFG . ANEG_OB_CTRL_DIS
 */
#define  VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_CFG_ANEG_OB_CTRL_DIS(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_CFG_ANEG_OB_CTRL_DIS     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_PCS_XAUI_ANEG_CFG_ANEG_CFG_ANEG_OB_CTRL_DIS(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Parallel detect wait time for 10G using four lanes
 *
 * \details 
 * 0: 0 ms
 * 1: 10 ms
 * 2: 20 ms
 * 3: 40 ms
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_CFG_ANEG_CFG . PD_TIMER_10GKX4
 */
#define  VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_CFG_PD_TIMER_10GKX4(x)  VTSS_ENCODE_BITFIELD(x,12,2)
#define  VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_CFG_PD_TIMER_10GKX4     VTSS_ENCODE_BITMASK(12,2)
#define  VTSS_X_PCS_XAUI_ANEG_CFG_ANEG_CFG_PD_TIMER_10GKX4(x)  VTSS_EXTRACT_BITFIELD(x,12,2)

/** 
 * \brief
 * Restart negotiation process. This is a one-shot and writing a 1 asserts
 * the restart. The bit is deasserted automatically.
 *
 * \details 
 * 1: Restart
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_CFG_ANEG_CFG . RESTART_ANEG_ONE_SHOT
 */
#define  VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_CFG_RESTART_ANEG_ONE_SHOT(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_CFG_RESTART_ANEG_ONE_SHOT     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_PCS_XAUI_ANEG_CFG_ANEG_CFG_RESTART_ANEG_ONE_SHOT(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Auto-negotiation enable
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_CFG_ANEG_CFG . ANEG_ENA
 */
#define  VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_CFG_ANEG_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_CFG_ANEG_ENA     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_PCS_XAUI_ANEG_CFG_ANEG_CFG_ANEG_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief ANEG advertised ability 0
 *
 * \details
 * 48 bits that contain the advertised abilities link code word for
 * auto-negotiation (here: lower 32 bit).
 *
 * Register: \a PCS_XAUI::ANEG_CFG::ANEG_ADV_ABILITY_0
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0(target)  VTSS_IOREG(target,0x6c)

/** 
 * \brief
 * Reserved for future technology as defined in IEEE 802.3ap clause 73.
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0 . ADV_ABIL_LSB
 */
#define  VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_ADV_ABIL_LSB(x)  VTSS_ENCODE_BITFIELD(x,24,8)
#define  VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_ADV_ABIL_LSB     VTSS_ENCODE_BITMASK(24,8)
#define  VTSS_X_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_ADV_ABIL_LSB(x)  VTSS_EXTRACT_BITFIELD(x,24,8)

/** 
 * \brief
 * Technology ability to be advertised (10GBASE-KR). Should be left at its
 * default value.
 *
 * \details 
 * 0: Do not advertise 10GBASE-KR capability
 * 1: Advertise 10GBASE-KR capability
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0 . CAP_10GKR
 */
#define  VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_10GKR(x)  VTSS_ENCODE_BITFIELD(x,23,1)
#define  VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_10GKR     VTSS_ENCODE_BITMASK(23,1)
#define  VTSS_X_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_10GKR(x)  VTSS_EXTRACT_BITFIELD(x,23,1)

/** 
 * \brief
 * Technology ability to be advertised (10GBASE-KX4)
 *
 * \details 
 * 0: Do not advertise 10GBASE-KX4 capability
 * 1: Advertise 10GBASE-KX4 capability
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0 . CAP_10GKX4
 */
#define  VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_10GKX4(x)  VTSS_ENCODE_BITFIELD(x,22,1)
#define  VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_10GKX4     VTSS_ENCODE_BITMASK(22,1)
#define  VTSS_X_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_10GKX4(x)  VTSS_EXTRACT_BITFIELD(x,22,1)

/** 
 * \brief
 * Technology ability to be advertised (1000BASE-KX). Should be left at its
 * default value.
 *
 * \details 
 * 0: Do not advertise 1GBASE-KX capability
 * 1: Advertise 1GBASE-KX capability
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0 . CAP_1GKX
 */
#define  VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_1GKX(x)  VTSS_ENCODE_BITFIELD(x,21,1)
#define  VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_1GKX     VTSS_ENCODE_BITMASK(21,1)
#define  VTSS_X_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_1GKX(x)  VTSS_EXTRACT_BITFIELD(x,21,1)

/** 
 * \brief
 * Initial value for transmit-nonce field
 *
 * \details 
 * 5-bit binary number
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0 . TX_NONCE
 */
#define  VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_TX_NONCE(x)  VTSS_ENCODE_BITFIELD(x,16,5)
#define  VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_TX_NONCE     VTSS_ENCODE_BITMASK(16,5)
#define  VTSS_X_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_TX_NONCE(x)  VTSS_EXTRACT_BITFIELD(x,16,5)

/** 
 * \brief
 * Next page exchange desired
 *
 * \details 
 * 0: Disable NP exchange
 * 1: Enable NP exchange
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0 . NP
 */
#define  VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_NP(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_NP     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_NP(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Remote fault (RF) bit (initial value)
 *
 * \details 
 * 0: No fault
 * 1: Fault
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0 . RF
 */
#define  VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_RF(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_RF     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_RF(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Pause field
 *
 * \details 
 * 0: Pause not supported
 * 1: Pause supported
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0 . PAUSE
 */
#define  VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_PAUSE(x)  VTSS_ENCODE_BITFIELD(x,10,3)
#define  VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_PAUSE     VTSS_ENCODE_BITMASK(10,3)
#define  VTSS_X_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_PAUSE(x)  VTSS_EXTRACT_BITFIELD(x,10,3)

/** 
 * \brief
 * Reserved for echoed nonce field. Should be 0 when ACKN is set to 0. Set
 * to the received NONCE from the link partner when ACKN is 1.
 *
 * \details 
 * Binary number
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0 . ECHOED_NONCE
 */
#define  VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_ECHOED_NONCE(x)  VTSS_ENCODE_BITFIELD(x,5,5)
#define  VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_ECHOED_NONCE     VTSS_ENCODE_BITMASK(5,5)
#define  VTSS_X_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_ECHOED_NONCE(x)  VTSS_EXTRACT_BITFIELD(x,5,5)

/** 
 * \brief
 * Selector field (must be 0x1)
 *
 * \details 
 * Binary number
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0 . SEL_FIELD
 */
#define  VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_SEL_FIELD(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_SEL_FIELD     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_SEL_FIELD(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief ANEG advertised ability 1
 *
 * \details
 * 48 bits that contain the advertised abilities link code word for
 * auto-negotiation (here: upper 16 bit).
 *
 * Register: \a PCS_XAUI::ANEG_CFG::ANEG_ADV_ABILITY_1
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_1(target)  VTSS_IOREG(target,0x6d)

/** 
 * \brief
 * FEC capability (bit 14: FEC ability, bit 15: FEC requested) - Only used
 * with 10GBASE-KR and should be set to 0 by default. Should be left at its
 * default value.
 *
 * \details 
 * Bit 14
 * 0: FEC not available
 * 1: FEC available
 * Bit 15
 * 0: Do Not Request FEC
 * 1: Request FEC
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_1 . FEC
 */
#define  VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_1_FEC(x)  VTSS_ENCODE_BITFIELD(x,14,2)
#define  VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_1_FEC     VTSS_ENCODE_BITMASK(14,2)
#define  VTSS_X_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_1_FEC(x)  VTSS_EXTRACT_BITFIELD(x,14,2)

/** 
 * \brief
 * Reserved for future technology as defined in IEEE 802.3ap clause 73.
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_1 . ADV_ABIL_MSB
 */
#define  VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_1_ADV_ABIL_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,14)
#define  VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_1_ADV_ABIL_MSB     VTSS_ENCODE_BITMASK(0,14)
#define  VTSS_X_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_1_ADV_ABIL_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,14)


/** 
 * \brief ANEG next page 0
 *
 * \details
 * 48 bits that contain the new next page to transmit during
 * auto-negotiation (here: lower 32 bits).
 *
 * Register: \a PCS_XAUI::ANEG_CFG::ANEG_NEXT_PAGE_0
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_ANEG_CFG_ANEG_NEXT_PAGE_0(target)  VTSS_IOREG(target,0x6e)

/** 
 * \brief
 * Lower 32 bits of next page link code word
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_ANEG_CFG_ANEG_NEXT_PAGE_0 . NP_TX_LSB
 */
#define  VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_NEXT_PAGE_0_NP_TX_LSB(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_NEXT_PAGE_0_NP_TX_LSB     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_PCS_XAUI_ANEG_CFG_ANEG_NEXT_PAGE_0_NP_TX_LSB(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief ANEG next page 1
 *
 * \details
 * 48 bits that contain the new next page to transmit during
 * auto-negotiation (here: upper 16 bits).
 *
 * Register: \a PCS_XAUI::ANEG_CFG::ANEG_NEXT_PAGE_1
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_ANEG_CFG_ANEG_NEXT_PAGE_1(target)  VTSS_IOREG(target,0x6f)

/** 
 * \brief
 * Must be set when a new next page is programmed (self-clearing)
 *
 * \details 
 * 1: Indicate that a page was programmed
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_CFG_ANEG_NEXT_PAGE_1 . NEXT_PAGE_LOADED_ONE_SHOT
 */
#define  VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_NEXT_PAGE_1_NEXT_PAGE_LOADED_ONE_SHOT(x)  VTSS_ENCODE_BITFIELD(x,31,1)
#define  VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_NEXT_PAGE_1_NEXT_PAGE_LOADED_ONE_SHOT     VTSS_ENCODE_BITMASK(31,1)
#define  VTSS_X_PCS_XAUI_ANEG_CFG_ANEG_NEXT_PAGE_1_NEXT_PAGE_LOADED_ONE_SHOT(x)  VTSS_EXTRACT_BITFIELD(x,31,1)

/** 
 * \brief
 * Upper 16 bits of next page link code word
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_ANEG_CFG_ANEG_NEXT_PAGE_1 . NP_TX_MSB
 */
#define  VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_NEXT_PAGE_1_NP_TX_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_NEXT_PAGE_1_NP_TX_MSB     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_PCS_XAUI_ANEG_CFG_ANEG_NEXT_PAGE_1_NP_TX_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask bits for interrupts
 *
 * \details
 * The bits in the interrupt mask register are used to enable the
 * associated interrupts. Status is available in ANEG_STATUS. Setting the
 * bit to 1 enables the interrupt.
 *
 * Register: \a PCS_XAUI::ANEG_CFG::ANEG_MASK
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_ANEG_CFG_ANEG_MASK(target)  VTSS_IOREG(target,0x70)

/** 
 * \brief
 * Mask bit for INCOMPATIBLE_LINK_STICKY
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_CFG_ANEG_MASK . INCOMPATIBLE_LINK_MASK
 */
#define  VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_MASK_INCOMPATIBLE_LINK_MASK(x)  VTSS_ENCODE_BITFIELD(x,21,1)
#define  VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_MASK_INCOMPATIBLE_LINK_MASK     VTSS_ENCODE_BITMASK(21,1)
#define  VTSS_X_PCS_XAUI_ANEG_CFG_ANEG_MASK_INCOMPATIBLE_LINK_MASK(x)  VTSS_EXTRACT_BITFIELD(x,21,1)

/** 
 * \brief
 * Interrupt mask for PAR_DETECT_FAULT
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_CFG_ANEG_MASK . PAR_DETECT_FAULT_MASK
 */
#define  VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_MASK_PAR_DETECT_FAULT_MASK(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_MASK_PAR_DETECT_FAULT_MASK     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_PCS_XAUI_ANEG_CFG_ANEG_MASK_PAR_DETECT_FAULT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Interrupt mask for PAGE_RX_STICKY
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_CFG_ANEG_MASK . PAGE_RX_MASK
 */
#define  VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_MASK_PAGE_RX_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_MASK_PAGE_RX_MASK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_PCS_XAUI_ANEG_CFG_ANEG_MASK_PAGE_RX_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Interrupt mask for ANEG_COMPLETE_STICKY
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_CFG_ANEG_MASK . ANEG_COMPLETE_MASK
 */
#define  VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_MASK_ANEG_COMPLETE_MASK(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_MASK_ANEG_COMPLETE_MASK     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_PCS_XAUI_ANEG_CFG_ANEG_MASK_ANEG_COMPLETE_MASK(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/**
 * Register Group: \a PCS_XAUI::ANEG_STATUS
 *
 * ANEG status registers
 */


/** 
 * \brief ANEG link partner advertised ability 0
 *
 * \details
 * 48 bits that contain the link partner's advertised abilities/next page
 * information (received link code word, lower 32 bits, received during
 * auto-negotiation). The bit groups are only valid for base pages; for
 * next page data exchange a different bit group coding has to be applied.
 *
 * Register: \a PCS_XAUI::ANEG_STATUS::ANEG_LP_ADV_ABILITY_0
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0(target)  VTSS_IOREG(target,0x71)

/** 
 * \brief
 * Bits 31 down to 24 of link code word received from link partner
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . LP_ADV_ABIL_LSB
 */
#define  VTSS_F_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_LP_ADV_ABIL_LSB(x)  VTSS_ENCODE_BITFIELD(x,24,8)
#define  VTSS_M_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_LP_ADV_ABIL_LSB     VTSS_ENCODE_BITMASK(24,8)
#define  VTSS_X_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_LP_ADV_ABIL_LSB(x)  VTSS_EXTRACT_BITFIELD(x,24,8)

/** 
 * \brief
 * Technology ability advertised by LP (10GBASE-KR)
 *
 * \details 
 * 0: LP is not 10GBASE-KR capable
 * 1: LP is 10GBASE-KR capable
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . CAP_10GKR
 */
#define  VTSS_F_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_CAP_10GKR(x)  VTSS_ENCODE_BITFIELD(x,23,1)
#define  VTSS_M_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_CAP_10GKR     VTSS_ENCODE_BITMASK(23,1)
#define  VTSS_X_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_CAP_10GKR(x)  VTSS_EXTRACT_BITFIELD(x,23,1)

/** 
 * \brief
 * Technology ability advertised by LP (10GBASE-KX4)
 *
 * \details 
 * 0: LP is not 10GBASE-KX4 capable
 * 1: LP is 10GBASE-KX4 capable
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . CAP_10GKX4
 */
#define  VTSS_F_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_CAP_10GKX4(x)  VTSS_ENCODE_BITFIELD(x,22,1)
#define  VTSS_M_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_CAP_10GKX4     VTSS_ENCODE_BITMASK(22,1)
#define  VTSS_X_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_CAP_10GKX4(x)  VTSS_EXTRACT_BITFIELD(x,22,1)

/** 
 * \brief
 * Technology ability advertised by LP (1000BASE-KX)
 *
 * \details 
 * 0: LP is not 1GBASE-KX capable
 * 1: LP is 1GBASE-KX capable
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . CAP_1GKX
 */
#define  VTSS_F_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_CAP_1GKX(x)  VTSS_ENCODE_BITFIELD(x,21,1)
#define  VTSS_M_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_CAP_1GKX     VTSS_ENCODE_BITMASK(21,1)
#define  VTSS_X_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_CAP_1GKX(x)  VTSS_EXTRACT_BITFIELD(x,21,1)

/** 
 * \brief
 * Transmit-nonce field (received from link partner)
 *
 * \details 
 * 5-bit binary number
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . TX_NONCE
 */
#define  VTSS_F_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_TX_NONCE(x)  VTSS_ENCODE_BITFIELD(x,16,5)
#define  VTSS_M_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_TX_NONCE     VTSS_ENCODE_BITMASK(16,5)
#define  VTSS_X_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_TX_NONCE(x)  VTSS_EXTRACT_BITFIELD(x,16,5)

/** 
 * \brief
 * Next page exchange desired by LP
 *
 * \details 
 * 0: No NP exchange desired
 * 1: NP exchange desired
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . NP
 */
#define  VTSS_F_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_NP(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_NP     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_NP(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Acknowledge bit (this bit is automatically overwritten by ANEG)
 *
 * \details 
 * 0: Link codeword not received by partner
 * 1: Link codeword received by partner
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . ACKN
 */
#define  VTSS_F_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_ACKN(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_ACKN     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_ACKN(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Remote fault (RF) bit
 *
 * \details 
 * 0: No fault
 * 1: Fault
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . RF
 */
#define  VTSS_F_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_RF(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_RF     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_RF(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Pause field
 *
 * \details 
 * 0: Pause not supported
 * 1: Pause supported
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . PAUSE
 */
#define  VTSS_F_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_PAUSE(x)  VTSS_ENCODE_BITFIELD(x,10,3)
#define  VTSS_M_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_PAUSE     VTSS_ENCODE_BITMASK(10,3)
#define  VTSS_X_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_PAUSE(x)  VTSS_EXTRACT_BITFIELD(x,10,3)

/** 
 * \brief
 * Echoed nonce field
 *
 * \details 
 * Binary number
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . ECHOED_NONCE
 */
#define  VTSS_F_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_ECHOED_NONCE(x)  VTSS_ENCODE_BITFIELD(x,5,5)
#define  VTSS_M_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_ECHOED_NONCE     VTSS_ENCODE_BITMASK(5,5)
#define  VTSS_X_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_ECHOED_NONCE(x)  VTSS_EXTRACT_BITFIELD(x,5,5)

/** 
 * \brief
 * Selector field
 *
 * \details 
 * Binary number
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . SEL_FIELD
 */
#define  VTSS_F_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_SEL_FIELD(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_SEL_FIELD     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_SEL_FIELD(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief ANEG link partner advertised ability 1
 *
 * \details
 * 48 bits that contain the link partner's advertised abilities or next
 * page information (received link code word, upper 16 bits, received
 * during auto-negotiation). The bit groups are only valid for base pages;
 * for next page data exchange a different bit group coding has to be
 * applied.
 *
 * Register: \a PCS_XAUI::ANEG_STATUS::ANEG_LP_ADV_ABILITY_1
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1(target)  VTSS_IOREG(target,0x72)

/** 
 * \brief
 * FEC capability (bit 14: FEC ability, bit 15: FEC requested) - Only used
 * with 10GBASE-KR
 *
 * \details 
 * Bit 14 
 * 0: FEC not available
 * 1: FEC available
 * Bit 15
 * 0: Do not request FEC
 * 1: Request FEC
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1 . FEC
 */
#define  VTSS_F_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1_FEC(x)  VTSS_ENCODE_BITFIELD(x,14,2)
#define  VTSS_M_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1_FEC     VTSS_ENCODE_BITMASK(14,2)
#define  VTSS_X_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1_FEC(x)  VTSS_EXTRACT_BITFIELD(x,14,2)

/** 
 * \brief
 * Bits 45 down to 32 of link code word received from link partner
 *
 * \details 
 * Field: ::VTSS_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1 . LP_ADV_ABIL_MSB
 */
#define  VTSS_F_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1_LP_ADV_ABIL_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,14)
#define  VTSS_M_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1_LP_ADV_ABIL_MSB     VTSS_ENCODE_BITMASK(0,14)
#define  VTSS_X_PCS_XAUI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1_LP_ADV_ABIL_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,14)


/** 
 * \brief ANEG status
 *
 * \details
 * Auto negotiation status register
 *
 * Register: \a PCS_XAUI::ANEG_STATUS::ANEG_STATUS
 *
 * @param target A \a ::vtss_target_PCS_XAUI_e target
 */
#define VTSS_PCS_XAUI_ANEG_STATUS_ANEG_STATUS(target)  VTSS_IOREG(target,0x73)

/** 
 * \brief
 * Sticky bit for the INCOMPATIBLE_LINK status bit. The sticky is set
 * whenever the state of INCOMPATIBLE_LINK is set. Bit is cleared by
 * writing a 1 to this position.
 *
 * \details 
 * 0: INCOMPATIBLE_LINK is not set (link compatible)
 * 1: INCOMPATIBLE_LINK has been set
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_STATUS_ANEG_STATUS . INCOMPATIBLE_LINK_STICKY
 */
#define  VTSS_F_PCS_XAUI_ANEG_STATUS_ANEG_STATUS_INCOMPATIBLE_LINK_STICKY(x)  VTSS_ENCODE_BITFIELD(x,21,1)
#define  VTSS_M_PCS_XAUI_ANEG_STATUS_ANEG_STATUS_INCOMPATIBLE_LINK_STICKY     VTSS_ENCODE_BITMASK(21,1)
#define  VTSS_X_PCS_XAUI_ANEG_STATUS_ANEG_STATUS_INCOMPATIBLE_LINK_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,21,1)

/** 
 * \brief
 * Error condition indicating that no compatible link was found
 *
 * \details 
 * 0: Link is compatible
 * 1: Link is incompatible
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_STATUS_ANEG_STATUS . INCOMPATIBLE_LINK
 */
#define  VTSS_F_PCS_XAUI_ANEG_STATUS_ANEG_STATUS_INCOMPATIBLE_LINK(x)  VTSS_ENCODE_BITFIELD(x,17,1)
#define  VTSS_M_PCS_XAUI_ANEG_STATUS_ANEG_STATUS_INCOMPATIBLE_LINK     VTSS_ENCODE_BITMASK(17,1)
#define  VTSS_X_PCS_XAUI_ANEG_STATUS_ANEG_STATUS_INCOMPATIBLE_LINK(x)  VTSS_EXTRACT_BITFIELD(x,17,1)

/** 
 * \brief
 * Error condition indicating errors during parallel detection. Bit is
 * cleared by writing a 1 to this position.
 *
 * \details 
 * 0: No fault detected
 * 1: Parallel detection fault detected
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_STATUS_ANEG_STATUS . PAR_DETECT_FAULT_STICKY
 */
#define  VTSS_F_PCS_XAUI_ANEG_STATUS_ANEG_STATUS_PAR_DETECT_FAULT_STICKY(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_PCS_XAUI_ANEG_STATUS_ANEG_STATUS_PAR_DETECT_FAULT_STICKY     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_PCS_XAUI_ANEG_STATUS_ANEG_STATUS_PAR_DETECT_FAULT_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Sticky bit set when PAGE_RX is set
 *
 * \details 
 * 0: PAGE_RX bit has not been set
 * 1: PAGE_RX bit has changed from 0 to 1
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_STATUS_ANEG_STATUS . PAGE_RX_STICKY
 */
#define  VTSS_F_PCS_XAUI_ANEG_STATUS_ANEG_STATUS_PAGE_RX_STICKY(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_PCS_XAUI_ANEG_STATUS_ANEG_STATUS_PAGE_RX_STICKY     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_PCS_XAUI_ANEG_STATUS_ANEG_STATUS_PAGE_RX_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Status indicating whether a new page has been received
 *
 * \details 
 * 0: No page received
 * 1: Page received
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_STATUS_ANEG_STATUS . PAGE_RX
 */
#define  VTSS_F_PCS_XAUI_ANEG_STATUS_ANEG_STATUS_PAGE_RX(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_PCS_XAUI_ANEG_STATUS_ANEG_STATUS_PAGE_RX     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_PCS_XAUI_ANEG_STATUS_ANEG_STATUS_PAGE_RX(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Status indicating whether the link partner supports auto-negotiation
 *
 * \details 
 * 0: Link partner does NOT support ANEG
 * 1: Link partner supports ANEG
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_STATUS_ANEG_STATUS . LP_ANEG_ABLE
 */
#define  VTSS_F_PCS_XAUI_ANEG_STATUS_ANEG_STATUS_LP_ANEG_ABLE(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_PCS_XAUI_ANEG_STATUS_ANEG_STATUS_LP_ANEG_ABLE     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_PCS_XAUI_ANEG_STATUS_ANEG_STATUS_LP_ANEG_ABLE(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Sticky for ANEG_COMPLETE. Bit is set whenever the ANEG_COMPLETE status
 * is set
 *
 * \details 
 * 0: ANEG_COMPLETE status is not set
 * 1: ANEG_COMPLETE status set to 1
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_STATUS_ANEG_STATUS . ANEG_COMPLETE_STICKY
 */
#define  VTSS_F_PCS_XAUI_ANEG_STATUS_ANEG_STATUS_ANEG_COMPLETE_STICKY(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_PCS_XAUI_ANEG_STATUS_ANEG_STATUS_ANEG_COMPLETE_STICKY     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_PCS_XAUI_ANEG_STATUS_ANEG_STATUS_ANEG_COMPLETE_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Status indicating whether auto-negotiation has completed
 *
 * \details 
 * 0: Autonegotiation not started or not completed
 * 1: Autonegotiation complete
 *
 * Field: ::VTSS_PCS_XAUI_ANEG_STATUS_ANEG_STATUS . ANEG_COMPLETE
 */
#define  VTSS_F_PCS_XAUI_ANEG_STATUS_ANEG_STATUS_ANEG_COMPLETE(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_PCS_XAUI_ANEG_STATUS_ANEG_STATUS_ANEG_COMPLETE     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_PCS_XAUI_ANEG_STATUS_ANEG_STATUS_ANEG_COMPLETE(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


#endif /* _VTSS_DAYTONA_REGS_PCS_XAUI_H_ */
