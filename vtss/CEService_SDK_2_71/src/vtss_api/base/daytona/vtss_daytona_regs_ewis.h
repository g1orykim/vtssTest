#ifndef _VTSS_DAYTONA_REGS_EWIS_H_
#define _VTSS_DAYTONA_REGS_EWIS_H_

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
 * Target: \a EWIS
 *
 * \see vtss_target_EWIS_e
 *
 * Enhanced WAN interface sublayer (eWIS) supports 10 GbE WAN, OC-192, and
 * STM-64 signals.
 *
 ***********************************************************************/

/**
 * Register Group: \a EWIS::RX_WIS_CTRL
 *
 * Rx WIS controls
 */


/** 
 * \brief eWIS Rx top level controls
 *
 * \details
 * Contains controls for the eWIS Rx that exist above the block level.
 * These are chip specific controls.
 *
 * Register: \a EWIS::RX_WIS_CTRL::RX_TOP_LEVEL
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_WIS_CTRL_RX_TOP_LEVEL(target)  VTSS_IOREG(target,0x0)

/** 
 * \brief
 * Clock gate for Rx section of eWIS block
 *
 * \details 
 * 0: Clock is gated and block is held in reset
 * 1: Normal operation
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_RX_TOP_LEVEL . RX_WIS_CLK_ENA
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_RX_TOP_LEVEL_RX_WIS_CLK_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_EWIS_RX_WIS_CTRL_RX_TOP_LEVEL_RX_WIS_CLK_ENA     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_EWIS_RX_WIS_CTRL_RX_TOP_LEVEL_RX_WIS_CLK_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/** 
 * \brief
 * Bypasses the Rx section of the eWIS block.
 * When bypassed, data is passed from the input directly to the output with
 * no modification. The overhead/stuff byte indicator applies to the
 * passive monitoring output. When not bypassed, data is processed by the
 * eWIS block before being output.
 *
 * \details 
 * 0: Normal operation, data is processed before output
 * 1: Block is bypassed
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_RX_TOP_LEVEL . RX_WIS_BYPASS
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_RX_TOP_LEVEL_RX_WIS_BYPASS(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_EWIS_RX_WIS_CTRL_RX_TOP_LEVEL_RX_WIS_BYPASS     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_EWIS_RX_WIS_CTRL_RX_TOP_LEVEL_RX_WIS_BYPASS(x)  VTSS_EXTRACT_BITFIELD(x,8,1)


/** 
 * \brief Controls for Loss of Signal (LOS)
 *
 * \details
 * Contains the control bits for the LOS features.
 *
 * Register: \a EWIS::RX_WIS_CTRL::LOS_CTRL
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_WIS_CTRL_LOS_CTRL(target)  VTSS_IOREG(target,0x1)

/** 
 * \brief
 * LOS detection suppression control
 *
 * \details 
 * 0: Normal operation
 * 1: LOS detection is suppressed
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_LOS_CTRL . RX_SUPPRESS_RX_LOS_DET
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_LOS_CTRL_RX_SUPPRESS_RX_LOS_DET(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_EWIS_RX_WIS_CTRL_LOS_CTRL_RX_SUPPRESS_RX_LOS_DET     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_EWIS_RX_WIS_CTRL_LOS_CTRL_RX_SUPPRESS_RX_LOS_DET(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/** 
 * \brief
 * Control for AIS-L insertion on LOS
 *
 * \details 
 * 0: No AIS-L insertion when LOS detection
 * 1: AIS-L is inserted into the datastream when LOS is detected
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_LOS_CTRL . RX_WIS_INS_AISL_ON_LOS
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_LOS_CTRL_RX_WIS_INS_AISL_ON_LOS(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_EWIS_RX_WIS_CTRL_LOS_CTRL_RX_WIS_INS_AISL_ON_LOS     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_EWIS_RX_WIS_CTRL_LOS_CTRL_RX_WIS_INS_AISL_ON_LOS(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Control for RDI-L backreporting on LOS
 *
 * \details 
 * 0: LOS is not backreported
 * 1: LOS is backreported
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_LOS_CTRL . RX_WIS_INS_RDIL_ON_LOS
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_LOS_CTRL_RX_WIS_INS_RDIL_ON_LOS(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_EWIS_RX_WIS_CTRL_LOS_CTRL_RX_WIS_INS_RDIL_ON_LOS     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_EWIS_RX_WIS_CTRL_LOS_CTRL_RX_WIS_INS_RDIL_ON_LOS(x)  VTSS_EXTRACT_BITFIELD(x,12,1)


/** 
 * \brief Controls for word alignment
 *
 * \details
 * Controls for the word alignment that precedes frame alignment.
 *
 * Register: \a EWIS::RX_WIS_CTRL::WORD_ALIGN
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_WIS_CTRL_WORD_ALIGN(target)  VTSS_IOREG(target,0x2)

/** 
 * \brief
 * Number of A1 bytes to be found for word alignment
 *
 * \details 
 * 0: Behaves as if set to 1
 * 16:1: Valid range
 * 31:17: Behaves as if set to 16
 * Note: Default value recommended.
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_WORD_ALIGN . RX_A1_HUNT_PAT_WIDTH
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_WORD_ALIGN_RX_A1_HUNT_PAT_WIDTH(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_EWIS_RX_WIS_CTRL_WORD_ALIGN_RX_A1_HUNT_PAT_WIDTH     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_EWIS_RX_WIS_CTRL_WORD_ALIGN_RX_A1_HUNT_PAT_WIDTH(x)  VTSS_EXTRACT_BITFIELD(x,0,5)

/** 
 * \brief
 * Number of A1 bytes used for word alignment, after the number of A1s
 * specified by RX_A1_HUNT_PAT_WIDTH have been found. Used in conjunction
 * with RX_PRESYNC_A2_PAT_WIDTH.
 *
 * \details 
 * 0: Behaves as if set to 1
 * 16:1: Valid range
 * 31:17: Behaves as if set to 16
 * Note: Default value recommended. IEEE 802.3 does not allow values below
 * 16.
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_WORD_ALIGN . RX_PRESYNC_A1_PAT_WIDTH
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_WORD_ALIGN_RX_PRESYNC_A1_PAT_WIDTH(x)  VTSS_ENCODE_BITFIELD(x,16,5)
#define  VTSS_M_EWIS_RX_WIS_CTRL_WORD_ALIGN_RX_PRESYNC_A1_PAT_WIDTH     VTSS_ENCODE_BITMASK(16,5)
#define  VTSS_X_EWIS_RX_WIS_CTRL_WORD_ALIGN_RX_PRESYNC_A1_PAT_WIDTH(x)  VTSS_EXTRACT_BITFIELD(x,16,5)

/** 
 * \brief
 * Number of A2 bytes used for word alignment, after the number of A1s
 * specified by RX_A1_HUNT_PAT_WIDTH have been found. Used in conjunction
 * with RX_PRESYNC_A1_PAT_WIDTH.
 *
 * \details 
 * 0: Behaves as if set to 1
 * 16:1: Valid range
 * 31:17: Behaves as if set to 16
 * Note: Default value recommended. IEEE 802.3 does not allow values below
 * 16.
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_WORD_ALIGN . RX_PRESYNC_A2_PAT_WIDTH
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_WORD_ALIGN_RX_PRESYNC_A2_PAT_WIDTH(x)  VTSS_ENCODE_BITFIELD(x,24,5)
#define  VTSS_M_EWIS_RX_WIS_CTRL_WORD_ALIGN_RX_PRESYNC_A2_PAT_WIDTH     VTSS_ENCODE_BITMASK(24,5)
#define  VTSS_X_EWIS_RX_WIS_CTRL_WORD_ALIGN_RX_PRESYNC_A2_PAT_WIDTH(x)  VTSS_EXTRACT_BITFIELD(x,24,5)


/** 
 * \brief Controls for frame alignment
 *
 * \details
 * Controls for the frame alignment and out of frame (OOF).
 *
 * Register: \a EWIS::RX_WIS_CTRL::FRAME_ALIGN
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_WIS_CTRL_FRAME_ALIGN(target)  VTSS_IOREG(target,0x3)

/** 
 * \brief
 * Controls the pattern used for framing. The pattern is a number of A1s
 * immediately followed by the same number of A2s.
 *
 * \details 
 * 0: Pattern used is A1 + 4 MSB of A2
 * 1: Pattern used is A1A1A2
 * 16:2: Valid range
 * 31:17: Behaves as if set to 16
 * Note: Default value recommended. IEEE 802.3 does not allow values below
 * 2.
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_FRAME_ALIGN . RX_SYNC_PATTERN
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_FRAME_ALIGN_RX_SYNC_PATTERN(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_EWIS_RX_WIS_CTRL_FRAME_ALIGN_RX_SYNC_PATTERN     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_EWIS_RX_WIS_CTRL_FRAME_ALIGN_RX_SYNC_PATTERN(x)  VTSS_EXTRACT_BITFIELD(x,0,5)

/** 
 * \brief
 * Number of consecutive frame boundaries needed to be found after entering
 * the PRESYNC state in order to enter the SYNC state, which will end the
 * out of frame condition (OOF). The number of consecutive good frames to
 * exit OOF will be the value of this field plus one.
 *
 * \details 
 * 0: Behaves as if set to 1
 * 15:1: Valid range16
 * Note: Default value recommended
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_FRAME_ALIGN . RX_SYNC_ENTRY_CNT
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_FRAME_ALIGN_RX_SYNC_ENTRY_CNT(x)  VTSS_ENCODE_BITFIELD(x,16,4)
#define  VTSS_M_EWIS_RX_WIS_CTRL_FRAME_ALIGN_RX_SYNC_ENTRY_CNT     VTSS_ENCODE_BITMASK(16,4)
#define  VTSS_X_EWIS_RX_WIS_CTRL_FRAME_ALIGN_RX_SYNC_ENTRY_CNT(x)  VTSS_EXTRACT_BITFIELD(x,16,4)

/** 
 * \brief
 * Number of consecutive frame boundary location errors needed to be found
 * before declaring the out of frame (OOF) condition.
 *
 * \details 
 * 0: Behaves as if set to 1
 * 15:1: Valid range16
 * Note: Default value recommended
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_FRAME_ALIGN . RX_SYNC_EXIT_CNT
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_FRAME_ALIGN_RX_SYNC_EXIT_CNT(x)  VTSS_ENCODE_BITFIELD(x,24,4)
#define  VTSS_M_EWIS_RX_WIS_CTRL_FRAME_ALIGN_RX_SYNC_EXIT_CNT     VTSS_ENCODE_BITMASK(24,4)
#define  VTSS_X_EWIS_RX_WIS_CTRL_FRAME_ALIGN_RX_SYNC_EXIT_CNT(x)  VTSS_EXTRACT_BITFIELD(x,24,4)


/** 
 * \brief Controls for Loss of Frame (LOF)
 *
 * \details
 * Contains the control bits for some LOF features.
 *
 * Register: \a EWIS::RX_WIS_CTRL::LOF_CTRL
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_WIS_CTRL_LOF_CTRL(target)  VTSS_IOREG(target,0x4)

/** 
 * \brief
 * Control for AIS-L insertion on LOF
 *
 * \details 
 * 0: No AIS-L insertion when LOF detection
 * 1: AIS-L is inserted into the datastream when LOF is detected
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_LOF_CTRL . RX_WIS_AISL_ON_LOF
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_LOF_CTRL_RX_WIS_AISL_ON_LOF(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_EWIS_RX_WIS_CTRL_LOF_CTRL_RX_WIS_AISL_ON_LOF     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_EWIS_RX_WIS_CTRL_LOF_CTRL_RX_WIS_AISL_ON_LOF(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/** 
 * \brief
 * Control for RDI-L backreporting on LOF
 *
 * \details 
 * 0: LOF is not backreported
 * 1: LOF is backreported
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_LOF_CTRL . RX_WIS_INS_RDIL_ON_LOF
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_LOF_CTRL_RX_WIS_INS_RDIL_ON_LOF(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_EWIS_RX_WIS_CTRL_LOF_CTRL_RX_WIS_INS_RDIL_ON_LOF     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_EWIS_RX_WIS_CTRL_LOF_CTRL_RX_WIS_INS_RDIL_ON_LOF(x)  VTSS_EXTRACT_BITFIELD(x,4,1)


/** 
 * \brief Controls for LOF timers
 *
 * \details
 * Contains the control bits for the LOF timers.
 *
 * Register: \a EWIS::RX_WIS_CTRL::LOF_TIMER_CTRL
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_WIS_CTRL_LOF_TIMER_CTRL(target)  VTSS_IOREG(target,0x5)

/** 
 * \brief
 * Integrating timer T1 for LOF detection.
 * When OOF is persistent for T1 frames, LOF is declared.
 *
 * \details 
 * 0: Invalid
 * 63:1: Valid range
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_LOF_TIMER_CTRL . RX_PRGLOFT1
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_LOF_TIMER_CTRL_RX_PRGLOFT1(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_EWIS_RX_WIS_CTRL_LOF_TIMER_CTRL_RX_PRGLOFT1     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_EWIS_RX_WIS_CTRL_LOF_TIMER_CTRL_RX_PRGLOFT1(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

/** 
 * \brief
 * Integrating timer T2 for LOF detection.
 * Used for hysteresis for the T1 timer. The T1 timer will not reset until
 * T2 frames are seen as being in the in-frame state.
 *
 * \details 
 * 0: Invalid
 * 63:1: Valid range
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_LOF_TIMER_CTRL . RX_PRGLOFT2
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_LOF_TIMER_CTRL_RX_PRGLOFT2(x)  VTSS_ENCODE_BITFIELD(x,8,6)
#define  VTSS_M_EWIS_RX_WIS_CTRL_LOF_TIMER_CTRL_RX_PRGLOFT2     VTSS_ENCODE_BITMASK(8,6)
#define  VTSS_X_EWIS_RX_WIS_CTRL_LOF_TIMER_CTRL_RX_PRGLOFT2(x)  VTSS_EXTRACT_BITFIELD(x,8,6)

/** 
 * \brief
 * Integrating timer T3 for LOF detection.
 * When in-frame status is persistent for T3 frames, the LOF condition is
 * cleared.
 *
 * \details 
 * 0: Invalid
 * 63:1: Valid range
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_LOF_TIMER_CTRL . RX_PRGLOFT3
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_LOF_TIMER_CTRL_RX_PRGLOFT3(x)  VTSS_ENCODE_BITFIELD(x,16,6)
#define  VTSS_M_EWIS_RX_WIS_CTRL_LOF_TIMER_CTRL_RX_PRGLOFT3     VTSS_ENCODE_BITMASK(16,6)
#define  VTSS_X_EWIS_RX_WIS_CTRL_LOF_TIMER_CTRL_RX_PRGLOFT3(x)  VTSS_EXTRACT_BITFIELD(x,16,6)


/** 
 * \brief Miscellaneous section overhead controls
 *
 * \details
 * Contains control bits for frame sync, J0, B1, and descrambling.
 *
 * Register: \a EWIS::RX_WIS_CTRL::MISC_SOH_CTRL
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_WIS_CTRL_MISC_SOH_CTRL(target)  VTSS_IOREG(target,0x6)

/** 
 * \brief
 * Control for the frame sync.
 * When the frame sync input is used, the framer logic is bypassed. Out of
 * Frame (OOF) conditions are determined by an input to the block.
 *
 * \details 
 * 0: Use framer logic
 * 1: Use frame sync input
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_MISC_SOH_CTRL . RX_FRM_SYNC_ENA
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_MISC_SOH_CTRL_RX_FRM_SYNC_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_EWIS_RX_WIS_CTRL_MISC_SOH_CTRL_RX_FRM_SYNC_ENA     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_EWIS_RX_WIS_CTRL_MISC_SOH_CTRL_RX_FRM_SYNC_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/** 
 * \brief
 * Control for the J0 byte trace length.
 *
 * \details 
 * 0: 16 byte trace
 * 1: 64 byte trace length
 * 3:2: 1 byte trace length
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_MISC_SOH_CTRL . RX_J0_64BYTE_TRACE
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_MISC_SOH_CTRL_RX_J0_64BYTE_TRACE(x)  VTSS_ENCODE_BITFIELD(x,8,2)
#define  VTSS_M_EWIS_RX_WIS_CTRL_MISC_SOH_CTRL_RX_J0_64BYTE_TRACE     VTSS_ENCODE_BITMASK(8,2)
#define  VTSS_X_EWIS_RX_WIS_CTRL_MISC_SOH_CTRL_RX_J0_64BYTE_TRACE(x)  VTSS_EXTRACT_BITFIELD(x,8,2)

/** 
 * \brief
 * Controls bit vs block mode for B1 performance monitoring counter. 
 * Bit mode will count all B1 parity errors received in the frame. Block
 * mode will count all frames received with B1 parity errors.
 *
 * \details 
 * 0: Bit mode
 * 1: Block mode
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_MISC_SOH_CTRL . RX_B1_BLOCK
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_MISC_SOH_CTRL_RX_B1_BLOCK(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_EWIS_RX_WIS_CTRL_MISC_SOH_CTRL_RX_B1_BLOCK     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_EWIS_RX_WIS_CTRL_MISC_SOH_CTRL_RX_B1_BLOCK(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Control for the descrambler.
 *
 * \details 
 * 0: Descrambler disabled
 * 1:Descrambler enabled
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_MISC_SOH_CTRL . RX_DESCRAM_EN
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_MISC_SOH_CTRL_RX_DESCRAM_EN(x)  VTSS_ENCODE_BITFIELD(x,24,1)
#define  VTSS_M_EWIS_RX_WIS_CTRL_MISC_SOH_CTRL_RX_DESCRAM_EN     VTSS_ENCODE_BITMASK(24,1)
#define  VTSS_X_EWIS_RX_WIS_CTRL_MISC_SOH_CTRL_RX_DESCRAM_EN(x)  VTSS_EXTRACT_BITFIELD(x,24,1)


/** 
 * \brief Miscellaneous line overhead controls
 *
 * \details
 * Contains controls for B2, K1/K2, and M0/M1.
 *
 * Register: \a EWIS::RX_WIS_CTRL::MISC_LOH_CTRL
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL(target)  VTSS_IOREG(target,0x7)

/** 
 * \brief
 * Controls bit vs block mode for B2 performance monitoring counter. 
 * Bit mode will count all B2 parity errors received in the frame. Block
 * mode will count all frames received with B1 parity errors.
 *
 * \details 
 * 0: Bit mode
 * 1: Block mode
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL . RX_B2_BLOCK
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_RX_B2_BLOCK(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_RX_B2_BLOCK     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_RX_B2_BLOCK(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/** 
 * \brief
 * Controls bit vs block mode for the REI-L performance monitoring counter.
 * 
 * Bit mode will add the value from the received REI-L to the counter.
 * Block mode will add 1 to the counter for every REI-L received that is
 * non-zero. When RX_G707_2000_REI is set, values greater than 1536 in the
 * REI field are treated as 0.
 *
 * \details 
 * 0: Bit mode
 * 1: Block mode
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL . RX_REIL_BLOCK
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_RX_REIL_BLOCK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_RX_REIL_BLOCK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_RX_REIL_BLOCK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Controls whether the M0 byte is treated as part of the REI field.
 * When the M0 byte is not included, only the M1 byte is used and a maximum
 * of 255 errors may be reported.
 * When the M0 byte is included, both the M0 and M1 byte are used and a
 * maximum of 1536 errors may be reported. Values greater than 1536 are
 * treated as 0.
 *
 * \details 
 * 0: REI is M1 byte only
 * 1: REI is M0 and M1 byte
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL . RX_G707_2000_REI
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_RX_G707_2000_REI(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_RX_G707_2000_REI     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_RX_G707_2000_REI(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Controls backreporting of AIS-L. 
 * When enabled, detection of AIS-L in the Rx will cause RDI-L
 * backreporting in the Tx.
 *
 * \details 
 * 0: Do not backreport AIS-L
 * 1: Normal operation
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL . RX_WIS_INS_RDIL_ON_AISL
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_RX_WIS_INS_RDIL_ON_AISL(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_RX_WIS_INS_RDIL_ON_AISL     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_RX_WIS_INS_RDIL_ON_AISL(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Sets the number of frames that the three least significant bits of the
 * K2 byte must be persistent for detection of RDI-L and AIS-L.
 *
 * \details 
 * 0: Behaves as if set to 1
 * 15:1: Valid range
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL . RX_K2_QUAL_FRM_CNT
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_RX_K2_QUAL_FRM_CNT(x)  VTSS_ENCODE_BITFIELD(x,16,4)
#define  VTSS_M_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_RX_K2_QUAL_FRM_CNT     VTSS_ENCODE_BITMASK(16,4)
#define  VTSS_X_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_RX_K2_QUAL_FRM_CNT(x)  VTSS_EXTRACT_BITFIELD(x,16,4)


/** 
 * \brief Controls for the pointer interpreter
 *
 * \details
 * Controls for the pointer interpreter, which processes the H1 and H2
 * bytes.
 *
 * Register: \a EWIS::RX_WIS_CTRL::PNTR_INTRPRTR_CTRL
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL(target)  VTSS_IOREG(target,0x8)

/** 
 * \brief
 * Bypasses the pointer interpreter and path overhead processing. This is
 * used when the incoming signal will not contain concatenation levels
 * other than STS-192c or VC-4-64c.
 *
 * \details 
 * 0: Perform pointer interpreter and path overhead processing
 * 1: Bypass pointer interpreter and path overhead processing
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL . RX_PI_BYPASS
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_PI_BYPASS(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_PI_BYPASS     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_PI_BYPASS(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/** 
 * \brief
 * Controls the rules used for determining a pointer justification.
 * The Objective rules of GR-253 specify an increment is indicated by eight
 * or more bits matching inverted I bits and non-inverted D bits, and a
 * decrement is indicated by eight or more bits matching inverted D bits
 * and non-inverted I bits. This is also known as the 8-out-of-10 rule.
 * The Standards rules of ANSI T1.416-1999 specify an increment is
 * indicated by three or more inverted I bits and two or more non-inverted
 * D bits, and a decrement is indicated by three or more inverted D bits
 * and two or more non-inverted I bits. In cases with three or more of each
 * inverted, no justification occurs. This is also known as the Majority
 * rule.
 *
 * \details 
 * 0: Use Objective rules
 * 1: Use Standards rules
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL . RX_JUST_MODE
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_JUST_MODE(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_JUST_MODE     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_JUST_MODE(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Controls the rules for which secondary H1 and H2 bytes are checked when
 * determining the pointer. For SONET all 191 secondary pointers are
 * checked. For SDH only the first 63 secondary pointers (as they appear on
 * the line) are checked.
 *
 * \details 
 * 0: Use SONET rules
 * 1: Use SDH rules
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL . RX_SEC_PTR_MODE
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_SEC_PTR_MODE(x)  VTSS_ENCODE_BITFIELD(x,17,1)
#define  VTSS_M_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_SEC_PTR_MODE     VTSS_ENCODE_BITMASK(17,1)
#define  VTSS_X_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_SEC_PTR_MODE(x)  VTSS_EXTRACT_BITFIELD(x,17,1)

/** 
 * \brief
 * Controls SS comparison.
 * In SONET systems, the SS bits are ignored. In some SDH systems, the SS
 * bits must equal 10 for the pointer to be valid.
 *
 * \details 
 * 0: Ignore SS bits
 * 1: Check SS bits
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL . RX_SS_ENA
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_SS_ENA(x)  VTSS_ENCODE_BITFIELD(x,18,1)
#define  VTSS_M_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_SS_ENA     VTSS_ENCODE_BITMASK(18,1)
#define  VTSS_X_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_SS_ENA(x)  VTSS_EXTRACT_BITFIELD(x,18,1)

/** 
 * \brief
 * Controls the definition of a valid pointer. 
 * There are three possible definitions:
 * GR-253 definition: Normal pointer with secondary pointers containing
 * concatenation indication received identically in three consecutive
 * frames.
 * VSC9128 definition: GR-253 definition plus a single normal pointer that
 * exactly matches the current valid pointer value.
 * ITU G.783 definition: VSC9128 definition plus incremented, decremented,
 * and NDF-enabled pointers.
 *
 * \details 
 * 0: VSC9128 definition
 * 1: ITU G.783 definition
 * 3:2: GR-253 definition
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL . RX_H1_NORM_PTR_VALID_MODE
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_H1_NORM_PTR_VALID_MODE(x)  VTSS_ENCODE_BITFIELD(x,20,2)
#define  VTSS_M_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_H1_NORM_PTR_VALID_MODE     VTSS_ENCODE_BITMASK(20,2)
#define  VTSS_X_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_H1_NORM_PTR_VALID_MODE(x)  VTSS_EXTRACT_BITFIELD(x,20,2)


/** 
 * \brief C2 byte controls
 *
 * \details
 * Controls for the C2 byte processing functions.
 *
 * Register: \a EWIS::RX_WIS_CTRL::C2_CTRL
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_WIS_CTRL_C2_CTRL(target)  VTSS_IOREG(target,0x9)

/** 
 * \brief
 * Contains the expected value for the C2 byte for payload mismatch
 * detection. For WIS applications the expected value is 0x1A.
 *
 * \details 
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_C2_CTRL . RX_EXPECTED_C2
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_C2_CTRL_RX_EXPECTED_C2(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_RX_WIS_CTRL_C2_CTRL_RX_EXPECTED_C2     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_RX_WIS_CTRL_C2_CTRL_RX_EXPECTED_C2(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Miscellaneous path overhead controls
 *
 * \details
 * Contains controls for J1, B3, and G1 bytes.
 *
 * Register: \a EWIS::RX_WIS_CTRL::MISC_POH_CTRL
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_WIS_CTRL_MISC_POH_CTRL(target)  VTSS_IOREG(target,0xa)

/** 
 * \brief
 * Control for the J1 byte trace length.
 *
 * \details 
 * 0: 16 byte trace
 * 1: 64 byte trace length
 * 3:2: 1 byte trace length
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_MISC_POH_CTRL . RX_J1_64BYTE_TRACE
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_MISC_POH_CTRL_RX_J1_64BYTE_TRACE(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_EWIS_RX_WIS_CTRL_MISC_POH_CTRL_RX_J1_64BYTE_TRACE     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_EWIS_RX_WIS_CTRL_MISC_POH_CTRL_RX_J1_64BYTE_TRACE(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

/** 
 * \brief
 * Controls bit vs block mode for B3 performance monitoring counter. Bit
 * mode will count all B3 parity errors received in the frame. Block mode
 * will count all frames received with B3 parity errors.
 *
 * \details 
 * 0: Bit mode
 * 1: Block mode
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_MISC_POH_CTRL . RX_B3_BLOCK
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_MISC_POH_CTRL_RX_B3_BLOCK(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_EWIS_RX_WIS_CTRL_MISC_POH_CTRL_RX_B3_BLOCK     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_EWIS_RX_WIS_CTRL_MISC_POH_CTRL_RX_B3_BLOCK(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Controls whether the fixed stuff bytes are included in the B3 parity
 * error checking.
 *
 * \details 
 * 0: Exclude fixed stuff bytes from B3 parity error checking
 * 1: Normal operation
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_MISC_POH_CTRL . RX_WIS_B3_FS_EN
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_MISC_POH_CTRL_RX_WIS_B3_FS_EN(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_EWIS_RX_WIS_CTRL_MISC_POH_CTRL_RX_WIS_B3_FS_EN     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_EWIS_RX_WIS_CTRL_MISC_POH_CTRL_RX_WIS_B3_FS_EN(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Controls bit vs block mode for the REI-P performance monitoring counter.
 * Bit mode will add the value from the received REI-P to the counter.
 * Block mode will add 1 to the counter for every REI-P received that is
 * non-zero. Values received that are greater than 8 will be treated as 0.
 *
 * \details 
 * 0: Bit mode
 * 1: Block mode
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_MISC_POH_CTRL . RX_REIP_BLOCK
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_MISC_POH_CTRL_RX_REIP_BLOCK(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_EWIS_RX_WIS_CTRL_MISC_POH_CTRL_RX_REIP_BLOCK     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_EWIS_RX_WIS_CTRL_MISC_POH_CTRL_RX_REIP_BLOCK(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Controls whether to use 1-bit RDI mode or 3-bit ERDI.
 *
 * \details 
 * 0: RDI mode
 * 1: ERDI mode
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_MISC_POH_CTRL . RX_G1_ERDI_MODE
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_MISC_POH_CTRL_RX_G1_ERDI_MODE(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_EWIS_RX_WIS_CTRL_MISC_POH_CTRL_RX_G1_ERDI_MODE     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_EWIS_RX_WIS_CTRL_MISC_POH_CTRL_RX_G1_ERDI_MODE(x)  VTSS_EXTRACT_BITFIELD(x,20,1)


/** 
 * \brief Controls for the ROSI
 *
 * \details
 * Contains controls for the receive overhead serial interface (ROSI)
 *
 * Register: \a EWIS::RX_WIS_CTRL::ROSI_CTRL
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_WIS_CTRL_ROSI_CTRL(target)  VTSS_IOREG(target,0xb)

/** 
 * \brief
 * Enables/disables the ROSI output when disabled the pins are tri-stated
 *
 * \details 
 * 0: ROSI tristated
 * 1: ROSI operational
 *
 * Field: ::VTSS_EWIS_RX_WIS_CTRL_ROSI_CTRL . ROSI_ENA
 */
#define  VTSS_F_EWIS_RX_WIS_CTRL_ROSI_CTRL_ROSI_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_EWIS_RX_WIS_CTRL_ROSI_CTRL_ROSI_ENA     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_EWIS_RX_WIS_CTRL_ROSI_CTRL_ROSI_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/**
 * Register Group: \a EWIS::RX_WIS_STATUS
 *
 * Rx WIS status
 */


/** 
 * \brief Rx WIS status
 *
 * \details
 * Status bits for the Rx section of the eWIS
 *
 * Register: \a EWIS::RX_WIS_STATUS::RX_WIS_STATUS
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_WIS_STATUS_RX_WIS_STATUS(target)  VTSS_IOREG(target,0x10)

/** 
 * \brief
 * Loss of signal (LOS) status
 *
 * \details 
 * 0: Normal operation
 * 1: LOS condition
 *
 * Field: ::VTSS_EWIS_RX_WIS_STATUS_RX_WIS_STATUS . RX_STATUS_LOS
 */
#define  VTSS_F_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_LOS(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_LOS     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_LOS(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/** 
 * \brief
 * Out of frame (OOF) status
 *
 * \details 
 * 0: Normal operation
 * 1: OOF condition
 *
 * Field: ::VTSS_EWIS_RX_WIS_STATUS_RX_WIS_STATUS . RX_FRMR_OOF_ST
 */
#define  VTSS_F_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_FRMR_OOF_ST(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_FRMR_OOF_ST     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_FRMR_OOF_ST(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Loss of frame (LOF) status
 *
 * \details 
 * 0: Normal operation
 * 1: LOF condition
 *
 * Field: ::VTSS_EWIS_RX_WIS_STATUS_RX_WIS_STATUS . RX_STATUS_LOF
 */
#define  VTSS_F_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_LOF(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_LOF     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_LOF(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Line remote defect indication (RDI-L) status
 *
 * \details 
 * 0: Normal operation
 * 1: RDI-L condition
 *
 * Field: ::VTSS_EWIS_RX_WIS_STATUS_RX_WIS_STATUS . RX_STATUS_RDI_L
 */
#define  VTSS_F_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_RDI_L(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_RDI_L     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_RDI_L(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Line alarm indication signal (AIS-L) status
 *
 * \details 
 * 0: Normal operation
 * 1: AIS-L condition
 *
 * Field: ::VTSS_EWIS_RX_WIS_STATUS_RX_WIS_STATUS . RX_STATUS_AIS_L
 */
#define  VTSS_F_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_AIS_L(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_AIS_L     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_AIS_L(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Loss of pointer (LOP) status
 *
 * \details 
 * 0: Normal operation
 * 1: LOP condition
 *
 * Field: ::VTSS_EWIS_RX_WIS_STATUS_RX_WIS_STATUS . RX_STATUS_LOP
 */
#define  VTSS_F_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_LOP(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_LOP     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_LOP(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Path alarm indication signal (AIS-P) status
 *
 * \details 
 * 0: Normal operation
 * 1: AIS-P condition
 *
 * Field: ::VTSS_EWIS_RX_WIS_STATUS_RX_WIS_STATUS . RX_STATUS_AIS_P
 */
#define  VTSS_F_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_AIS_P(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_AIS_P     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_AIS_P(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Path payload mismatch (PLM-P) status
 *
 * \details 
 * 0: Normal operation
 * 1: PLM-P condition
 *
 * Field: ::VTSS_EWIS_RX_WIS_STATUS_RX_WIS_STATUS . RX_STATUS_PLM_P
 */
#define  VTSS_F_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_PLM_P(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_PLM_P     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_PLM_P(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Path unequipped (UNEQ-P) status
 *
 * \details 
 * 0: Normal operation
 * 1: UNEQ-P condition
 *
 * Field: ::VTSS_EWIS_RX_WIS_STATUS_RX_WIS_STATUS . RX_STATUS_UNEQ_P
 */
#define  VTSS_F_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_UNEQ_P(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_UNEQ_P     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_UNEQ_P(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Path remote defect indication (RDI-P) status
 *
 * \details 
 * 0: Normal operation
 * 1: RDI-P condition
 *
 * Field: ::VTSS_EWIS_RX_WIS_STATUS_RX_WIS_STATUS . RX_STATUS_RDIP
 */
#define  VTSS_F_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_RDIP(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_RDIP     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_RDIP(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Far end path alarm indication signal (AIS-P) status
 *
 * \details 
 * 0: Normal operation
 * 1: Far End AIS-P condition
 *
 * Field: ::VTSS_EWIS_RX_WIS_STATUS_RX_WIS_STATUS . RX_STATUS_AIS_P_F
 */
#define  VTSS_F_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_AIS_P_F(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_AIS_P_F     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_AIS_P_F(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Far end path payload mismatch (PLM-P) status
 *
 * \details 
 * 0: Normal operation
 * 1: Far end PLM-P condition
 *
 * Field: ::VTSS_EWIS_RX_WIS_STATUS_RX_WIS_STATUS . RX_STATUS_PLM_P_F
 */
#define  VTSS_F_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_PLM_P_F(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_PLM_P_F     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_PLM_P_F(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Far end path unequipped (UNEQ-P) status
 *
 * \details 
 * 0: Normal operation
 * 1: Far end UNEQ-P condition
 *
 * Field: ::VTSS_EWIS_RX_WIS_STATUS_RX_WIS_STATUS . RX_STATUS_UNEQ_P_F
 */
#define  VTSS_F_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_UNEQ_P_F(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_UNEQ_P_F     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_UNEQ_P_F(x)  VTSS_EXTRACT_BITFIELD(x,15,1)


/** 
 * \brief S1 byte status
 *
 * \details
 * Contains status for the S1 byte
 *
 * Register: \a EWIS::RX_WIS_STATUS::RX_STATUS_S1
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_WIS_STATUS_RX_STATUS_S1(target)  VTSS_IOREG(target,0x12)

/** 
 * \brief
 * Contains the last persistent value of S1 received
 *
 * \details 
 * Field: ::VTSS_EWIS_RX_WIS_STATUS_RX_STATUS_S1 . RX_STATUS_S1
 */
#define  VTSS_F_EWIS_RX_WIS_STATUS_RX_STATUS_S1_RX_STATUS_S1(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_RX_WIS_STATUS_RX_STATUS_S1_RX_STATUS_S1     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_RX_WIS_STATUS_RX_STATUS_S1_RX_STATUS_S1(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a EWIS::RX_J0_STATUS
 *
 * J0 byte status
 */


/** 
 * \brief Captured J0 bytes
 *
 * \details
 * Contains the captured J0 bytes
 *
 * Register: \a EWIS::RX_J0_STATUS::RX_BYTE_J0
 *
 * @param target A \a ::vtss_target_EWIS_e target
 * @param ri Register: RX_BYTE_J0 (??), 0-63
 */
#define VTSS_EWIS_RX_J0_STATUS_RX_BYTE_J0(target,ri)  VTSS_IOREG(target,0x20 + (ri))

/** 
 * \brief
 * Contains the captured J0 bytes.
 * Depending on the setting in RX_J0_64_BYTE_TRACE, either the first 1, the
 * first 16, or all 64 bytes are valid.
 *
 * \details 
 * Field: ::VTSS_EWIS_RX_J0_STATUS_RX_BYTE_J0 . RX_BYTE_J0
 */
#define  VTSS_F_EWIS_RX_J0_STATUS_RX_BYTE_J0_RX_BYTE_J0(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_RX_J0_STATUS_RX_BYTE_J0_RX_BYTE_J0     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_RX_J0_STATUS_RX_BYTE_J0_RX_BYTE_J0(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a EWIS::RX_J1_STATUS
 *
 * J1 byte status
 */


/** 
 * \brief Captured J1 bytes
 *
 * \details
 * Contains the captured J1 bytes
 *
 * Register: \a EWIS::RX_J1_STATUS::RX_BYTE_J1
 *
 * @param target A \a ::vtss_target_EWIS_e target
 * @param ri Register: RX_BYTE_J1 (??), 0-63
 */
#define VTSS_EWIS_RX_J1_STATUS_RX_BYTE_J1(target,ri)  VTSS_IOREG(target,0x60 + (ri))

/** 
 * \brief
 * Contains the captured J1 bytes.
 * Depending on the setting in RX_J1_64_BYTE_TRACE, either the first 1, the
 * first 16, or all 64 bytes are valid.
 *
 * \details 
 * Field: ::VTSS_EWIS_RX_J1_STATUS_RX_BYTE_J1 . RX_BYTE_J1
 */
#define  VTSS_F_EWIS_RX_J1_STATUS_RX_BYTE_J1_RX_BYTE_J1(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_RX_J1_STATUS_RX_BYTE_J1_RX_BYTE_J1     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_RX_J1_STATUS_RX_BYTE_J1_RX_BYTE_J1(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a EWIS::RX_WIS_ERR_CNT
 *
 * Rx WIS error counters
 */


/** 
 * \brief B1 error counter
 *
 * \details
 * B1 parity error counter
 *
 * Register: \a EWIS::RX_WIS_ERR_CNT::RX_B1_ERR_CNT
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_WIS_ERR_CNT_RX_B1_ERR_CNT(target)  VTSS_IOREG(target,0xa0)

/** 
 * \brief
 * Counter for B1 BIP-8 errors.
 * All detected B1 BIP-8 errors are accumulated in this register. Upon
 * reaching maximum value, this counter will roll-over.
 *
 * \details 
 * Field: ::VTSS_EWIS_RX_WIS_ERR_CNT_RX_B1_ERR_CNT . RX_B1_ERR_CNT
 */
#define  VTSS_F_EWIS_RX_WIS_ERR_CNT_RX_B1_ERR_CNT_RX_B1_ERR_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_EWIS_RX_WIS_ERR_CNT_RX_B1_ERR_CNT_RX_B1_ERR_CNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_EWIS_RX_WIS_ERR_CNT_RX_B1_ERR_CNT_RX_B1_ERR_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief B2 error counter
 *
 * \details
 * B2 parity error counter
 *
 * Register: \a EWIS::RX_WIS_ERR_CNT::RX_B2_ERR_CNT
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_WIS_ERR_CNT_RX_B2_ERR_CNT(target)  VTSS_IOREG(target,0xa1)

/** 
 * \brief
 * Counter for B2 BIP-1536 errors.
 * All detected B2 BIP-1536 errors are accumulated in this register. Upon
 * reaching maximum value, this counter will roll-over.
 *
 * \details 
 * Field: ::VTSS_EWIS_RX_WIS_ERR_CNT_RX_B2_ERR_CNT . RX_B2_ERR_CNT
 */
#define  VTSS_F_EWIS_RX_WIS_ERR_CNT_RX_B2_ERR_CNT_RX_B2_ERR_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_EWIS_RX_WIS_ERR_CNT_RX_B2_ERR_CNT_RX_B2_ERR_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_EWIS_RX_WIS_ERR_CNT_RX_B2_ERR_CNT_RX_B2_ERR_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief REI-L error counter
 *
 * \details
 * Line REI counter
 *
 * Register: \a EWIS::RX_WIS_ERR_CNT::RX_REIL_ERR_CNT
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_WIS_ERR_CNT_RX_REIL_ERR_CNT(target)  VTSS_IOREG(target,0xa2)

/** 
 * \brief
 * Counter for line remote error indicator (REI-L) errors
 * All detected REI-L errors are accumulated in this register. Upon
 * reaching maximum value, this counter will roll-over.
 *
 * \details 
 * Field: ::VTSS_EWIS_RX_WIS_ERR_CNT_RX_REIL_ERR_CNT . RX_REIL_ERR_CNT
 */
#define  VTSS_F_EWIS_RX_WIS_ERR_CNT_RX_REIL_ERR_CNT_RX_REIL_ERR_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_EWIS_RX_WIS_ERR_CNT_RX_REIL_ERR_CNT_RX_REIL_ERR_CNT     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_EWIS_RX_WIS_ERR_CNT_RX_REIL_ERR_CNT_RX_REIL_ERR_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief B3 error counter
 *
 * \details
 * B3 parity error counter
 *
 * Register: \a EWIS::RX_WIS_ERR_CNT::RX_B3_ERR_CNT
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_WIS_ERR_CNT_RX_B3_ERR_CNT(target)  VTSS_IOREG(target,0xa3)

/** 
 * \brief
 * Counter for B3 BIP-8 errors.
 * All detected B3 BIP-8 errors are accumulated in this register. Upon
 * reaching maximum value, this counter will roll-over.
 *
 * \details 
 * Field: ::VTSS_EWIS_RX_WIS_ERR_CNT_RX_B3_ERR_CNT . RX_B3_ERR_CNT
 */
#define  VTSS_F_EWIS_RX_WIS_ERR_CNT_RX_B3_ERR_CNT_RX_B3_ERR_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_EWIS_RX_WIS_ERR_CNT_RX_B3_ERR_CNT_RX_B3_ERR_CNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_EWIS_RX_WIS_ERR_CNT_RX_B3_ERR_CNT_RX_B3_ERR_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief REI-P error counter
 *
 * \details
 * Path REI counter
 *
 * Register: \a EWIS::RX_WIS_ERR_CNT::RX_REIP_ERR_BLK_CNT
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_WIS_ERR_CNT_RX_REIP_ERR_BLK_CNT(target)  VTSS_IOREG(target,0xa4)

/** 
 * \brief
 * Counter for path remote error indicator (REI-P) errors.
 * This counter will count frames with errors (i.e. block mode) only. Upon
 * reaching maximum value, this counter will roll-over.
 *
 * \details 
 * Field: ::VTSS_EWIS_RX_WIS_ERR_CNT_RX_REIP_ERR_BLK_CNT . RX_REIP_ERR_BLK_CNT
 */
#define  VTSS_F_EWIS_RX_WIS_ERR_CNT_RX_REIP_ERR_BLK_CNT_RX_REIP_ERR_BLK_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_EWIS_RX_WIS_ERR_CNT_RX_REIP_ERR_BLK_CNT_RX_REIP_ERR_BLK_CNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_EWIS_RX_WIS_ERR_CNT_RX_REIP_ERR_BLK_CNT_RX_REIP_ERR_BLK_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a EWIS::RX_WIS_CNT_PERF
 *
 * Rx WIS performance counters
 */


/** 
 * \brief B1 performance counter
 *
 * \details
 * The B1 parity performance counter
 *
 * Register: \a EWIS::RX_WIS_CNT_PERF::RX_B1_ERR_CNT_PERF
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_WIS_CNT_PERF_RX_B1_ERR_CNT_PERF(target)  VTSS_IOREG(target,0xb0)

/** 
 * \brief
 * Performance counter for B1 BIP-8 errors.
 * Updates each performance monitoring interval.
 *
 * \details 
 * Field: ::VTSS_EWIS_RX_WIS_CNT_PERF_RX_B1_ERR_CNT_PERF . RX_B1_ERR_CNT_PERF
 */
#define  VTSS_F_EWIS_RX_WIS_CNT_PERF_RX_B1_ERR_CNT_PERF_RX_B1_ERR_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_EWIS_RX_WIS_CNT_PERF_RX_B1_ERR_CNT_PERF_RX_B1_ERR_CNT_PERF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_EWIS_RX_WIS_CNT_PERF_RX_B1_ERR_CNT_PERF_RX_B1_ERR_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief B2 performance counter
 *
 * \details
 * The B2 parity performance counter
 *
 * Register: \a EWIS::RX_WIS_CNT_PERF::RX_B2_ERR_CNT_PERF
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_WIS_CNT_PERF_RX_B2_ERR_CNT_PERF(target)  VTSS_IOREG(target,0xb1)

/** 
 * \brief
 * Performance counter for B2 BIP-1536 errors.
 * Updates each performance monitoring interval.
 *
 * \details 
 * Field: ::VTSS_EWIS_RX_WIS_CNT_PERF_RX_B2_ERR_CNT_PERF . RX_B2_ERR_CNT_PERF
 */
#define  VTSS_F_EWIS_RX_WIS_CNT_PERF_RX_B2_ERR_CNT_PERF_RX_B2_ERR_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_EWIS_RX_WIS_CNT_PERF_RX_B2_ERR_CNT_PERF_RX_B2_ERR_CNT_PERF     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_EWIS_RX_WIS_CNT_PERF_RX_B2_ERR_CNT_PERF_RX_B2_ERR_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief REI-L performance counter
 *
 * \details
 * The REI-L Performance counter
 *
 * Register: \a EWIS::RX_WIS_CNT_PERF::RX_REIL_ERR_CNT_PERF
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_WIS_CNT_PERF_RX_REIL_ERR_CNT_PERF(target)  VTSS_IOREG(target,0xb2)

/** 
 * \brief
 * Performance counter for REI-L.
 * Updates each performance monitoring interval.
 *
 * \details 
 * Field: ::VTSS_EWIS_RX_WIS_CNT_PERF_RX_REIL_ERR_CNT_PERF . RX_REIL_ERR_CNT_PERF
 */
#define  VTSS_F_EWIS_RX_WIS_CNT_PERF_RX_REIL_ERR_CNT_PERF_RX_REIL_ERR_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_EWIS_RX_WIS_CNT_PERF_RX_REIL_ERR_CNT_PERF_RX_REIL_ERR_CNT_PERF     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_EWIS_RX_WIS_CNT_PERF_RX_REIL_ERR_CNT_PERF_RX_REIL_ERR_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief B3 performance counter
 *
 * \details
 * The B3 parity performance counter
 *
 * Register: \a EWIS::RX_WIS_CNT_PERF::RX_B3_ERR_CNT_PERF
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_WIS_CNT_PERF_RX_B3_ERR_CNT_PERF(target)  VTSS_IOREG(target,0xb3)

/** 
 * \brief
 * Performance counter for B3 BIP-8 errors.
 * Updates each performance monitoring interval.
 *
 * \details 
 * Field: ::VTSS_EWIS_RX_WIS_CNT_PERF_RX_B3_ERR_CNT_PERF . RX_B3_ERR_CNT_PERF
 */
#define  VTSS_F_EWIS_RX_WIS_CNT_PERF_RX_B3_ERR_CNT_PERF_RX_B3_ERR_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_EWIS_RX_WIS_CNT_PERF_RX_B3_ERR_CNT_PERF_RX_B3_ERR_CNT_PERF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_EWIS_RX_WIS_CNT_PERF_RX_B3_ERR_CNT_PERF_RX_B3_ERR_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief REI-P performance counter
 *
 * \details
 * The REI-P performance counter
 *
 * Register: \a EWIS::RX_WIS_CNT_PERF::RX_REIP_ERR_CNT_PERF
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_WIS_CNT_PERF_RX_REIP_ERR_CNT_PERF(target)  VTSS_IOREG(target,0xb4)

/** 
 * \brief
 * Performance counter for REI-P.
 * Updates each performance monitoring interval.
 *
 * \details 
 * Field: ::VTSS_EWIS_RX_WIS_CNT_PERF_RX_REIP_ERR_CNT_PERF . RX_REIP_ERR_CNT_PERF
 */
#define  VTSS_F_EWIS_RX_WIS_CNT_PERF_RX_REIP_ERR_CNT_PERF_RX_REIP_ERR_CNT_PERF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_EWIS_RX_WIS_CNT_PERF_RX_REIP_ERR_CNT_PERF_RX_REIP_ERR_CNT_PERF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_EWIS_RX_WIS_CNT_PERF_RX_REIP_ERR_CNT_PERF_RX_REIP_ERR_CNT_PERF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a EWIS::RX_INTRPT
 *
 * Rx interrupts
 */


/** 
 * \brief Rx status interrupts
 *
 * \details
 * Contains the interrupt bits for the status bits
 *
 * Register: \a EWIS::RX_INTRPT::RX_STATUS_INTRPT
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_INTRPT_RX_STATUS_INTRPT(target)  VTSS_IOREG(target,0xc1)

/** 
 * \brief
 * Loss of signal (LOS) interrupt pending
 *
 * \details 
 * 0: Normal operation
 * 1: LOS status has changed value
 *
 * Field: ::VTSS_EWIS_RX_INTRPT_RX_STATUS_INTRPT . INTR_LOS_PEND_STICKY
 */
#define  VTSS_F_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_LOS_PEND_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_LOS_PEND_STICKY     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_LOS_PEND_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/** 
 * \brief
 * Out of frame (OOF) interrupt pending
 *
 * \details 
 * 0: Normal operation
 * 1: OOF status has changed value
 *
 * Field: ::VTSS_EWIS_RX_INTRPT_RX_STATUS_INTRPT . INTR_OOF_PEND_STICKY
 */
#define  VTSS_F_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_OOF_PEND_STICKY(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_OOF_PEND_STICKY     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_OOF_PEND_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Loss of frame (LOF) interrupt pending
 *
 * \details 
 * 0: Normal operation
 * 1: LOF status has changed value
 *
 * Field: ::VTSS_EWIS_RX_INTRPT_RX_STATUS_INTRPT . INTR_LOF_PEND_STICKY
 */
#define  VTSS_F_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_LOF_PEND_STICKY(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_LOF_PEND_STICKY     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_LOF_PEND_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Line remote defect indication (RDI-L) interrupt pending
 *
 * \details 
 * 0: Normal operation
 * 1: RDI-L condition
 *
 * Field: ::VTSS_EWIS_RX_INTRPT_RX_STATUS_INTRPT . INTR_RDIL_PEND_STICKY
 */
#define  VTSS_F_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_RDIL_PEND_STICKY(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_RDIL_PEND_STICKY     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_RDIL_PEND_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Line alarm indication signal (AIS-L) interrupt pending
 *
 * \details 
 * 0: Normal operation
 * 1: AIS-L status has changed value
 *
 * Field: ::VTSS_EWIS_RX_INTRPT_RX_STATUS_INTRPT . INTR_AISL_PEND_STICKY
 */
#define  VTSS_F_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_AISL_PEND_STICKY(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_AISL_PEND_STICKY     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_AISL_PEND_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Synchronous message (S1) interrupt pending
 *
 * \details 
 * 0: Normal operation
 * 1: S1 status has changed value
 *
 * Field: ::VTSS_EWIS_RX_INTRPT_RX_STATUS_INTRPT . INTR_S1_PEND_STICKY
 */
#define  VTSS_F_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_S1_PEND_STICKY(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_S1_PEND_STICKY     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_S1_PEND_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Loss of pointer (LOP) interrupt pending
 *
 * \details 
 * 0: Normal operation
 * 1: LOP status has changed value
 *
 * Field: ::VTSS_EWIS_RX_INTRPT_RX_STATUS_INTRPT . INTR_LOP_PEND_STICKY
 */
#define  VTSS_F_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_LOP_PEND_STICKY(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_LOP_PEND_STICKY     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_LOP_PEND_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Path alarm indication signal (AIS-P) interrupt pending
 *
 * \details 
 * 0: Normal operation
 * 1: AIS-P status has changed value
 *
 * Field: ::VTSS_EWIS_RX_INTRPT_RX_STATUS_INTRPT . INTR_AISP_PEND_STICKY
 */
#define  VTSS_F_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_AISP_PEND_STICKY(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_AISP_PEND_STICKY     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_AISP_PEND_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Path payload mismatch (PLM-P) interrupt pending
 *
 * \details 
 * 0: Normal operation
 * 1: PLM-P status has changed value
 *
 * Field: ::VTSS_EWIS_RX_INTRPT_RX_STATUS_INTRPT . INTR_PLMP_PEND_STICKY
 */
#define  VTSS_F_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_PLMP_PEND_STICKY(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_PLMP_PEND_STICKY     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_PLMP_PEND_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Path unequipped (UNEQ-P) interrupt pending
 *
 * \details 
 * 0: Normal operation
 * 1: UNEQ-P status has changed value
 *
 * Field: ::VTSS_EWIS_RX_INTRPT_RX_STATUS_INTRPT . INTR_UNEQP_PEND_STICKY
 */
#define  VTSS_F_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_UNEQP_PEND_STICKY(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_UNEQP_PEND_STICKY     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_UNEQP_PEND_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Path remote defect indication (RDI-P) interrupt pending
 *
 * \details 
 * 0: Normal operation
 * 1: RDI-P status has changed value
 *
 * Field: ::VTSS_EWIS_RX_INTRPT_RX_STATUS_INTRPT . INTR_RDIP_PEND_STICKY
 */
#define  VTSS_F_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_RDIP_PEND_STICKY(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_RDIP_PEND_STICKY     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_RDIP_PEND_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Far end path alarm indication signal (AIS-P) interrupt pending
 *
 * \details 
 * 0: Normal operation
 * 1: Far End AIS-P status has changed value
 *
 * Field: ::VTSS_EWIS_RX_INTRPT_RX_STATUS_INTRPT . INTR_FE_AIS_P_PEND_STICKY
 */
#define  VTSS_F_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_FE_AIS_P_PEND_STICKY(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_FE_AIS_P_PEND_STICKY     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_FE_AIS_P_PEND_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Far end path payload mismatch (PLM-P) interrupt pending
 *
 * \details 
 * 0: Normal operation
 * 1: Far End PLM-P status has changed value
 *
 * Field: ::VTSS_EWIS_RX_INTRPT_RX_STATUS_INTRPT . INTR_FEPLMP_PEND_STICKY
 */
#define  VTSS_F_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_FEPLMP_PEND_STICKY(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_FEPLMP_PEND_STICKY     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_FEPLMP_PEND_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Far end path unequipped (UNEQ-P) interrupt pending
 *
 * \details 
 * 0: Normal operation
 * 1: Far End UNEQ-P status has changed value
 *
 * Field: ::VTSS_EWIS_RX_INTRPT_RX_STATUS_INTRPT . INTR_FE_UNEQP_PEND_STICKY
 */
#define  VTSS_F_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_FE_UNEQP_PEND_STICKY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_FE_UNEQP_PEND_STICKY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_FE_UNEQP_PEND_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/**
 * Register Group: \a EWIS::RX_MASK
 *
 * Rx interrupt masks
 */


/** 
 * \brief Rx status interrupt masks
 *
 * \details
 * Contains the interrupt mask bits for the interrupts for the status bits
 *
 * Register: \a EWIS::RX_MASK::RX_STATUS_MASK
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_MASK_RX_STATUS_MASK(target)  VTSS_IOREG(target,0xd0)

/** 
 * \brief
 * Mask for loss of signal (LOS) interrupt pending
 *
 * \details 
 * 0: LOS interrupt pending will not cause device interrupt
 * 1: LOS interrupt pending will cause device interrupt
 *
 * Field: ::VTSS_EWIS_RX_MASK_RX_STATUS_MASK . INTR_LOS_MASK
 */
#define  VTSS_F_EWIS_RX_MASK_RX_STATUS_MASK_INTR_LOS_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_LOS_MASK     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_EWIS_RX_MASK_RX_STATUS_MASK_INTR_LOS_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/** 
 * \brief
 * Mask for out of frame (OOF) interrupt pending
 *
 * \details 
 * 0: OOF interrupt pending will not cause device interrupt
 * 1: OOF interrupt pending will cause device interrupt
 *
 * Field: ::VTSS_EWIS_RX_MASK_RX_STATUS_MASK . INTR_OOF_MASK
 */
#define  VTSS_F_EWIS_RX_MASK_RX_STATUS_MASK_INTR_OOF_MASK(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_OOF_MASK     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_EWIS_RX_MASK_RX_STATUS_MASK_INTR_OOF_MASK(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Mask for loss of frame (LOF) interrupt pending
 *
 * \details 
 * 0: LOF interrupt pending will not cause device interrupt
 * 1: LOF interrupt pending will cause device interrupt
 *
 * Field: ::VTSS_EWIS_RX_MASK_RX_STATUS_MASK . INTR_LOF_MASK
 */
#define  VTSS_F_EWIS_RX_MASK_RX_STATUS_MASK_INTR_LOF_MASK(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_LOF_MASK     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_EWIS_RX_MASK_RX_STATUS_MASK_INTR_LOF_MASK(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Mask for line remote defect indication (RDI-L) interrupt pending
 *
 * \details 
 * 0: RDI-L interrupt pending will not cause device interrupt
 * 1: RDI-L interrupt pending will cause device interrupt
 *
 * Field: ::VTSS_EWIS_RX_MASK_RX_STATUS_MASK . INTR_RDIL_MASK
 */
#define  VTSS_F_EWIS_RX_MASK_RX_STATUS_MASK_INTR_RDIL_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_RDIL_MASK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_EWIS_RX_MASK_RX_STATUS_MASK_INTR_RDIL_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Mask for line alarm indication signal (AIS-L) interrupt pending
 *
 * \details 
 * 0: AIS-L interrupt pending will not cause device interrupt
 * 1: AIS-L interrupt pending will cause device interrupt
 *
 * Field: ::VTSS_EWIS_RX_MASK_RX_STATUS_MASK . INTR_AISL_MASK
 */
#define  VTSS_F_EWIS_RX_MASK_RX_STATUS_MASK_INTR_AISL_MASK(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_AISL_MASK     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_EWIS_RX_MASK_RX_STATUS_MASK_INTR_AISL_MASK(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Mask for synchronous message (S1) interrupt pending
 *
 * \details 
 * 0: S1 interrupt pending will not cause device interrupt
 * 1: S1 interrupt pending will cause device interrupt
 *
 * Field: ::VTSS_EWIS_RX_MASK_RX_STATUS_MASK . INTR_S1_MASK
 */
#define  VTSS_F_EWIS_RX_MASK_RX_STATUS_MASK_INTR_S1_MASK(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_S1_MASK     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_EWIS_RX_MASK_RX_STATUS_MASK_INTR_S1_MASK(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Mask for loss of pointer (LOP) interrupt pending
 *
 * \details 
 * 0: LOP interrupt pending will not cause device interrupt
 * 1: LOP interrupt pending will cause device interrupt
 *
 * Field: ::VTSS_EWIS_RX_MASK_RX_STATUS_MASK . INTR_LOP_MASK
 */
#define  VTSS_F_EWIS_RX_MASK_RX_STATUS_MASK_INTR_LOP_MASK(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_LOP_MASK     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_EWIS_RX_MASK_RX_STATUS_MASK_INTR_LOP_MASK(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Mask for path alarm indication signal (AIS-P) interrupt pending
 *
 * \details 
 * 0: AIS-P interrupt pending will not cause device interrupt
 * 1: AIS-P interrupt pending will cause device interrupt
 *
 * Field: ::VTSS_EWIS_RX_MASK_RX_STATUS_MASK . INTR_AISP_MASK
 */
#define  VTSS_F_EWIS_RX_MASK_RX_STATUS_MASK_INTR_AISP_MASK(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_AISP_MASK     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_EWIS_RX_MASK_RX_STATUS_MASK_INTR_AISP_MASK(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Mask for path payload mismatch (PLM-P) interrupt pending
 *
 * \details 
 * 0: PLM-P interrupt pending will not cause device interrupt
 * 1: PLM-P interrupt pending will cause device interrupt
 *
 * Field: ::VTSS_EWIS_RX_MASK_RX_STATUS_MASK . INTR_PLMP_MASK
 */
#define  VTSS_F_EWIS_RX_MASK_RX_STATUS_MASK_INTR_PLMP_MASK(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_PLMP_MASK     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_EWIS_RX_MASK_RX_STATUS_MASK_INTR_PLMP_MASK(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Mask for path unequipped (UNEQ-P) interrupt pending
 *
 * \details 
 * 0: UNEQ-P interrupt pending will not cause device interrupt
 * 1: UNEQ-P interrupt pending will cause device interrupt
 *
 * Field: ::VTSS_EWIS_RX_MASK_RX_STATUS_MASK . INTR_UNEQP_MASK
 */
#define  VTSS_F_EWIS_RX_MASK_RX_STATUS_MASK_INTR_UNEQP_MASK(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_UNEQP_MASK     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_EWIS_RX_MASK_RX_STATUS_MASK_INTR_UNEQP_MASK(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Mask for path remote defect indication (RDI-P) interrupt pending
 *
 * \details 
 * 0: RDI-P interrupt pending will not cause device interrupt
 * 1: RDI-P interrupt pending will cause device interrupt
 *
 * Field: ::VTSS_EWIS_RX_MASK_RX_STATUS_MASK . INTR_RDIP_MASK
 */
#define  VTSS_F_EWIS_RX_MASK_RX_STATUS_MASK_INTR_RDIP_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_RDIP_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_EWIS_RX_MASK_RX_STATUS_MASK_INTR_RDIP_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Mask for far end path alarm indication signal (AIS-P) interrupt pending
 *
 * \details 
 * 0: Far End AIS-P interrupt pending will not cause device interrupt
 * 1: Far End AIS-P interrupt pending will cause device interrupt
 *
 * Field: ::VTSS_EWIS_RX_MASK_RX_STATUS_MASK . INTR_FE_AIS_P_MASK
 */
#define  VTSS_F_EWIS_RX_MASK_RX_STATUS_MASK_INTR_FE_AIS_P_MASK(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_FE_AIS_P_MASK     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_EWIS_RX_MASK_RX_STATUS_MASK_INTR_FE_AIS_P_MASK(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Mask for far end path payload mismatch (PLM-P) interrupt pending
 *
 * \details 
 * 0: Far End PLM-P interrupt pending will not cause device interrupt
 * 1: Far End PLM-P interrupt pending will cause device interrupt
 *
 * Field: ::VTSS_EWIS_RX_MASK_RX_STATUS_MASK . INTR_FEPLMP_MASK
 */
#define  VTSS_F_EWIS_RX_MASK_RX_STATUS_MASK_INTR_FEPLMP_MASK(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_FEPLMP_MASK     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_EWIS_RX_MASK_RX_STATUS_MASK_INTR_FEPLMP_MASK(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Mask for far end path unequipped (UNEQ-P) interrupt pending
 *
 * \details 
 * 0: Far End UNEQ-P interrupt pending will not cause device interrupt
 * 1: Far End UNEQ-P interrupt pending will cause device interrupt
 *
 * Field: ::VTSS_EWIS_RX_MASK_RX_STATUS_MASK . INTR_FE_UNEQP_MASK
 */
#define  VTSS_F_EWIS_RX_MASK_RX_STATUS_MASK_INTR_FE_UNEQP_MASK(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_FE_UNEQP_MASK     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_EWIS_RX_MASK_RX_STATUS_MASK_INTR_FE_UNEQP_MASK(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/**
 * Register Group: \a EWIS::TX_WIS_CTRL
 *
 * Tx WIS controls
 */


/** 
 * \brief eWIS Tx top level controls
 *
 * \details
 * Contains controls for the eWIS Tx that exist above the block level.
 * These are chip specific controls.
 *
 * Register: \a EWIS::TX_WIS_CTRL::TX_TOP_LEVEL
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_WIS_CTRL_TX_TOP_LEVEL(target)  VTSS_IOREG(target,0xe0)

/** 
 * \brief
 * Clock gate for Tx section of eWIS block.
 *
 * \details 
 * 0: Clock is gated and block is held in reset
 * 1: Normal operation
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_TOP_LEVEL . TX_WIS_CLK_ENA
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_TOP_LEVEL_TX_WIS_CLK_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_TOP_LEVEL_TX_WIS_CLK_ENA     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_TOP_LEVEL_TX_WIS_CLK_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/** 
 * \brief
 * Bypasses the Tx section of the eWIS block.
 * When bypassed, data is passed from the input directly to the output with
 * no modification. The overhead/stuff byte indicator applies to the
 * passive monitoring output.
 * When not bypassed, data is processed by the eWIS block before being
 * output.
 *
 * \details 
 * 0: Normal operation, data is processed before output
 * 1: Block is bypassed
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_TOP_LEVEL . TX_WIS_BYPASS
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_TOP_LEVEL_TX_WIS_BYPASS(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_TOP_LEVEL_TX_WIS_BYPASS     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_TOP_LEVEL_TX_WIS_BYPASS(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Controls whether the incoming data to the TX eWIS needs to be mapped
 * into a frame or not.
 *
 * \details 
 * 0: Input data is SONET/SDH/10GEWAN frame
 * 1: Input data is PCS encoded 10GbE to be mapped into a frame
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_TOP_LEVEL . TX_MAP_MODE
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_TOP_LEVEL_TX_MAP_MODE(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_TOP_LEVEL_TX_MAP_MODE     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_TOP_LEVEL_TX_MAP_MODE(x)  VTSS_EXTRACT_BITFIELD(x,16,1)


/** 
 * \brief eWIS Tx misc controls
 *
 * \details
 * Miscellaneous controls for the eWIS Tx
 *
 * Register: \a EWIS::TX_WIS_CTRL::TX_MISC_CTRL
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_WIS_CTRL_TX_MISC_CTRL(target)  VTSS_IOREG(target,0xe1)

/** 
 * \brief
 * Control for the scrambler
 *
 * \details 
 * 0: Scrambler disabled
 * 1: Scrambler enabled
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_MISC_CTRL . TX_SCRAMEN
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_SCRAMEN(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_SCRAMEN     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_SCRAMEN(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/** 
 * \brief
 * Control for the J0 byte trace length
 *
 * \details 
 * 0: 16 byte trace
 * 1: 64 byte trace length
 * 3:2: 1 byte trace length
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_MISC_CTRL . TX_J0_TRACE_LENGTH
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_J0_TRACE_LENGTH(x)  VTSS_ENCODE_BITFIELD(x,1,2)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_J0_TRACE_LENGTH     VTSS_ENCODE_BITMASK(1,2)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_J0_TRACE_LENGTH(x)  VTSS_EXTRACT_BITFIELD(x,1,2)

/** 
 * \brief
 * Controls the setting of the SS bits transmitted in the first H1 byte
 *
 * \details 
 * 0: SS bits are 00
 * 1: SS bits are 10
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_MISC_CTRL . TX_SS_BIT
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_SS_BIT(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_SS_BIT     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_SS_BIT(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Forces transmission of RDI-L
 *
 * \details 
 * 0: Normal operation
 * 1: Force transmission of RDI-L
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_MISC_CTRL . TX_FORCE_RDI_L
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_FORCE_RDI_L(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_FORCE_RDI_L     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_FORCE_RDI_L(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Forces transmission of AIS-L
 *
 * \details 
 * 0: Normal operation
 * 1: Force transmission of AIS-L
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_MISC_CTRL . TX_FORCE_AIS_L
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_FORCE_AIS_L(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_FORCE_AIS_L     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_FORCE_AIS_L(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Controls whether REI-L is the M1 byte or M0 and M1 bytes
 *
 * \details 
 * 0: REI-L is in the M1 byte
 * 1: REI-L is in the M0 and M1 bytes
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_MISC_CTRL . TX_G707_2000_REI
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_G707_2000_REI(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_G707_2000_REI     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_G707_2000_REI(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Controls whether to backreport REI-L in bit or block mode
 *
 * \details 
 * 0: Bit mode
 * 1: Block mode
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_MISC_CTRL . TX_REIL_BLOCK
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_REIL_BLOCK(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_REIL_BLOCK     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_REIL_BLOCK(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Control for the J1 byte trace length
 *
 * \details 
 * 0: 16 byte trace
 * 1: 64 byte trace length
 * 3:2: 1 byte trace length
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_MISC_CTRL . TX_J1_TRACE_LENGTH
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_J1_TRACE_LENGTH(x)  VTSS_ENCODE_BITFIELD(x,16,2)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_J1_TRACE_LENGTH     VTSS_ENCODE_BITMASK(16,2)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_J1_TRACE_LENGTH(x)  VTSS_EXTRACT_BITFIELD(x,16,2)

/** 
 * \brief
 * Controls whether to backreport REI-P in bit or block mode
 *
 * \details 
 * 0: Bit mode
 * 1: Block mode
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_MISC_CTRL . TX_REIP_BLOCK
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_REIP_BLOCK(x)  VTSS_ENCODE_BITFIELD(x,18,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_REIP_BLOCK     VTSS_ENCODE_BITMASK(18,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_REIP_BLOCK(x)  VTSS_EXTRACT_BITFIELD(x,18,1)

/** 
 * \brief
 * Controls whether to transmit 1-bit RDI code or 3-bit ERDI codes
 *
 * \details 
 * 0: RDI mode
 * 1: ERDI mode
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_MISC_CTRL . TX_ERDI_RDIN
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_ERDI_RDIN(x)  VTSS_ENCODE_BITFIELD(x,19,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_ERDI_RDIN     VTSS_ENCODE_BITMASK(19,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_ERDI_RDIN(x)  VTSS_EXTRACT_BITFIELD(x,19,1)

/** 
 * \brief
 * Indicates that a TIM-P condition was detected and TIM-P should be
 * backreported
 *
 * \details 
 * 0: Normal operation
 * 1: TIM-P condition was detected
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_MISC_CTRL . TX_TIM_P
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_TIM_P(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_TIM_P     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_TIM_P(x)  VTSS_EXTRACT_BITFIELD(x,20,1)


/** 
 * \brief Tx eWIS passthru controls
 *
 * \details
 * Controls whether various overhead fields are passed through or generated
 *
 * Register: \a EWIS::TX_WIS_CTRL::TX_PASSTHRU
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU(target)  VTSS_IOREG(target,0xe3)

/** 
 * \brief
 * Controls passthru of the J0 byte
 *
 * \details 
 * 0: J0 is generated internally or from TOSI
 * 1: J0 is passed through unchanged
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU . TX_J0_PASSTHRU
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_J0_PASSTHRU(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_J0_PASSTHRU     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_J0_PASSTHRU(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/** 
 * \brief
 * Controls passthru of the Z0 byte
 *
 * \details 
 * 0: Z0 is generated internally or from TOSI
 * 1: Z0 is passed through unchanged
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU . TX_Z0_PASSTHRU
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_Z0_PASSTHRU(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_Z0_PASSTHRU     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_Z0_PASSTHRU(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Controls passthru of the B1 byte
 *
 * \details 
 * 0: B1 is generated internally or from TOSI
 * 1: B1 is passed through unchanged
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU . TX_B1_PASSTHRU
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_B1_PASSTHRU(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_B1_PASSTHRU     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_B1_PASSTHRU(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Controls passthru of the E1 byte
 *
 * \details 
 * 0: E1 is generated internally or from TOSI
 * 1: E1 is passed through unchanged
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU . TX_E1_PASSTHRU
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_E1_PASSTHRU(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_E1_PASSTHRU     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_E1_PASSTHRU(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Controls passthru of the F1 byte
 *
 * \details 
 * 0: F1 is generated internally or from TOSI
 * 1: F1 is passed through unchanged
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU . TX_F1_PASSTHRU
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_F1_PASSTHRU(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_F1_PASSTHRU     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_F1_PASSTHRU(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Controls passthru of the D1, D2, and D3 bytes
 *
 * \details 
 * 0: D1, D2, and D3 bytes are generated internally or from TOSI
 * 1: D1, D2, and D3 bytes are passed thru unchanged
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU . TX_SDCC_PASSTHRU
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_SDCC_PASSTHRU(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_SDCC_PASSTHRU     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_SDCC_PASSTHRU(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Controls passthru of the section reserved, national, and unused bytes
 *
 * \details 
 * 0: Section reserved, national, and unused bytes are generated internally
 * or from TOSI
 * 1: Section reserved, national, and unused bytes are passed through
 * unchanged
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU . TX_SOH_PASSTHRU
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_SOH_PASSTHRU(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_SOH_PASSTHRU     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_SOH_PASSTHRU(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Controls passthru of the B2 byte
 *
 * \details 
 * 0: B2 is generated internally or from TOSI
 * 1: B2 is passed through unchanged
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU . TX_B2_PASSTHRU
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_B2_PASSTHRU(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_B2_PASSTHRU     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_B2_PASSTHRU(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Controls passthru of the K1 byte
 *
 * \details 
 * 0: K1 is generated internally or from TOSI
 * 1: K1 is passed through unchanged
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU . TX_K1_PASSTHRU
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_K1_PASSTHRU(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_K1_PASSTHRU     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_K1_PASSTHRU(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Controls passthru of the K2 byte
 *
 * \details 
 * 0: K2 is generated internally or from TOSI
 * 1: K2 is passed through unchanged
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU . TX_K2_PASSTHRU
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_K2_PASSTHRU(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_K2_PASSTHRU     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_K2_PASSTHRU(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Controls passthru of the REIL field bytes
 *
 * \details 
 * 0: REIL field bytes are generated internally or from TOSI
 * 1: REIL field bytes are passed through unchanged
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU . TX_REIL_PASSTHRU
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_REIL_PASSTHRU(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_REIL_PASSTHRU     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_REIL_PASSTHRU(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Controls passthru of the D4 - D12 bytes
 *
 * \details 
 * 0: D4 - D12 bytes are generated internally or from TOSI
 * 1: D4 - D12 bytes are passed through unchanged
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU . TX_LDCC_PASSTHRU
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_LDCC_PASSTHRU(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_LDCC_PASSTHRU     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_LDCC_PASSTHRU(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Controls passthru of the S1 byte
 *
 * \details 
 * 0: S1 is generated internally or from TOSI
 * 1: S1 is passed through unchanged
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU . TX_S1_PASSTHRU
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_S1_PASSTHRU(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_S1_PASSTHRU     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_S1_PASSTHRU(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Controls passthru of the E2 byte
 *
 * \details 
 * 0: E2 is generated internally or from TOSI
 * 1: E2 is passed through unchanged
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU . TX_E2_PASSTHRU
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_E2_PASSTHRU(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_E2_PASSTHRU     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_E2_PASSTHRU(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Controls passthru of the Z1 and Z2 bytes
 *
 * \details 
 * 0: Z1 and Z2 bytes are generated internally or from TOSI
 * 1: Z1 and Z2 bytes are passed through unchanged
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU . TX_Z1Z2_PASSTHRU
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_Z1Z2_PASSTHRU(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_Z1Z2_PASSTHRU     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_Z1Z2_PASSTHRU(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Controls passthru of the line reserved, national, and unused bytes
 *
 * \details 
 * 0: Line reserved, national, and unused bytes are generated internally or
 * from TOSI
 * 1: Line reserved, national, and unused bytes are passed through
 * unchanged
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU . TX_LOH_PASSTHRU
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_LOH_PASSTHRU(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_LOH_PASSTHRU     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_LOH_PASSTHRU(x)  VTSS_EXTRACT_BITFIELD(x,15,1)


/** 
 * \brief Controls for the TOSI
 *
 * \details
 * Contains controls for the transmit overhead serial interface (TOSI)
 *
 * Register: \a EWIS::TX_WIS_CTRL::TOSI_CTRL
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_WIS_CTRL_TOSI_CTRL(target)  VTSS_IOREG(target,0xe4)

/** 
 * \brief
 * Enables/disables the TOSI outputs when disabled the pins are tri-stated
 *
 * \details 
 * 0: TOSI tristated
 * 1: TOSI operational
 *
 * Field: ::VTSS_EWIS_TX_WIS_CTRL_TOSI_CTRL . TOSI_ENA
 */
#define  VTSS_F_EWIS_TX_WIS_CTRL_TOSI_CTRL_TOSI_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_EWIS_TX_WIS_CTRL_TOSI_CTRL_TOSI_ENA     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_EWIS_TX_WIS_CTRL_TOSI_CTRL_TOSI_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/**
 * Register Group: \a EWIS::TX_BYTE
 *
 * Tx byte registers
 */


/** 
 * \brief Z0 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_Z0
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_Z0(target)      VTSS_IOREG(target,0xf0)

/** 
 * \brief
 * Contains the Z0 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_Z0 . TX_Z0
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_Z0_TX_Z0(x)   VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_Z0_TX_Z0      VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_Z0_TX_Z0(x)   VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief E1 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_E1
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_E1(target)      VTSS_IOREG(target,0xf1)

/** 
 * \brief
 * Contains the E1 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_E1 . TX_E1
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_E1_TX_E1(x)   VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_E1_TX_E1      VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_E1_TX_E1(x)   VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief F1 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_F1
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_F1(target)      VTSS_IOREG(target,0xf2)

/** 
 * \brief
 * Contains the E1 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_F1 . TX_F1
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_F1_TX_F1(x)   VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_F1_TX_F1      VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_F1_TX_F1(x)   VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief D1 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_D1
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_D1(target)      VTSS_IOREG(target,0xf3)

/** 
 * \brief
 * Contains the D1 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_D1 . TX_D1
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_D1_TX_D1(x)   VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_D1_TX_D1      VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_D1_TX_D1(x)   VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief D2 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_D2
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_D2(target)      VTSS_IOREG(target,0xf4)

/** 
 * \brief
 * Contains the D2 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_D2 . TX_D2
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_D2_TX_D2(x)   VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_D2_TX_D2      VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_D2_TX_D2(x)   VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief D3 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_D3
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_D3(target)      VTSS_IOREG(target,0xf5)

/** 
 * \brief
 * Contains the D3 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_D3 . TX_D3
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_D3_TX_D3(x)   VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_D3_TX_D3      VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_D3_TX_D3(x)   VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief K1 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_K1
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_K1(target)      VTSS_IOREG(target,0xf6)

/** 
 * \brief
 * Contains the K1 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_K1 . TX_K1
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_K1_TX_K1(x)   VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_K1_TX_K1      VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_K1_TX_K1(x)   VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief K2 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_K2
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_K2(target)      VTSS_IOREG(target,0xf7)

/** 
 * \brief
 * Contains the K2 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_K2 . TX_K2
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_K2_TX_K2(x)   VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_K2_TX_K2      VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_K2_TX_K2(x)   VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief D4 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_D4
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_D4(target)      VTSS_IOREG(target,0xf8)

/** 
 * \brief
 * Contains the D4 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_D4 . TX_D4
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_D4_TX_D4(x)   VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_D4_TX_D4      VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_D4_TX_D4(x)   VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief D5 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_D5
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_D5(target)      VTSS_IOREG(target,0xf9)

/** 
 * \brief
 * Contains the D5 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_D5 . TX_D5
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_D5_TX_D5(x)   VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_D5_TX_D5      VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_D5_TX_D5(x)   VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief D6 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_D6
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_D6(target)      VTSS_IOREG(target,0xfa)

/** 
 * \brief
 * Contains the D6 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_D6 . TX_D6
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_D6_TX_D6(x)   VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_D6_TX_D6      VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_D6_TX_D6(x)   VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief D7 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_D7
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_D7(target)      VTSS_IOREG(target,0xfb)

/** 
 * \brief
 * Contains the D7 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_D7 . TX_D7
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_D7_TX_D7(x)   VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_D7_TX_D7      VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_D7_TX_D7(x)   VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief D8 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_D8
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_D8(target)      VTSS_IOREG(target,0xfc)

/** 
 * \brief
 * Contains the D8 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_D8 . TX_D8
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_D8_TX_D8(x)   VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_D8_TX_D8      VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_D8_TX_D8(x)   VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief D9 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_D9
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_D9(target)      VTSS_IOREG(target,0xfd)

/** 
 * \brief
 * Contains the D9 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_D9 . TX_D9
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_D9_TX_D9(x)   VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_D9_TX_D9      VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_D9_TX_D9(x)   VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief D10 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_D10
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_D10(target)     VTSS_IOREG(target,0xfe)

/** 
 * \brief
 * Contains the D10 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_D10 . TX_D10
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_D10_TX_D10(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_D10_TX_D10     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_D10_TX_D10(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief D11 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_D11
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_D11(target)     VTSS_IOREG(target,0xff)

/** 
 * \brief
 * Contains the D11 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_D11 . TX_D11
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_D11_TX_D11(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_D11_TX_D11     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_D11_TX_D11(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief D12 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_D12
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_D12(target)     VTSS_IOREG(target,0x100)

/** 
 * \brief
 * Contains the D12 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_D12 . TX_D12
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_D12_TX_D12(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_D12_TX_D12     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_D12_TX_D12(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief S1 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_S1
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_S1(target)      VTSS_IOREG(target,0x101)

/** 
 * \brief
 * Contains the S1 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_S1 . TX_S1
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_S1_TX_S1(x)   VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_S1_TX_S1      VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_S1_TX_S1(x)   VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief E2 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_E2
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_E2(target)      VTSS_IOREG(target,0x102)

/** 
 * \brief
 * Contains the E2 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_E2 . TX_E2
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_E2_TX_E2(x)   VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_E2_TX_E2      VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_E2_TX_E2(x)   VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Z1 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_Z1
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_Z1(target)      VTSS_IOREG(target,0x103)

/** 
 * \brief
 * Contains the Z1 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_Z1 . TX_Z1
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_Z1_TX_Z1(x)   VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_Z1_TX_Z1      VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_Z1_TX_Z1(x)   VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Z2 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_Z2
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_Z2(target)      VTSS_IOREG(target,0x104)

/** 
 * \brief
 * Contains the Z2 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_Z2 . TX_Z2
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_Z2_TX_Z2(x)   VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_Z2_TX_Z2      VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_Z2_TX_Z2(x)   VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief C2 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_C2
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_C2(target)      VTSS_IOREG(target,0x105)

/** 
 * \brief
 * Contains the Z2 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_C2 . TX_C2
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_C2_TX_C2(x)   VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_C2_TX_C2      VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_C2_TX_C2(x)   VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief F2 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_F2
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_F2(target)      VTSS_IOREG(target,0x106)

/** 
 * \brief
 * Contains the F2 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_F2 . TX_F2
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_F2_TX_F2(x)   VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_F2_TX_F2      VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_F2_TX_F2(x)   VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief H4 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_H4
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_H4(target)      VTSS_IOREG(target,0x107)

/** 
 * \brief
 * Contains the H4 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_H4 . TX_H4
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_H4_TX_H4(x)   VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_H4_TX_H4      VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_H4_TX_H4(x)   VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Z3 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_Z3
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_Z3(target)      VTSS_IOREG(target,0x108)

/** 
 * \brief
 * Contains the Z3 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_Z3 . TX_Z3
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_Z3_TX_Z3(x)   VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_Z3_TX_Z3      VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_Z3_TX_Z3(x)   VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Z4 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_Z4
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_Z4(target)      VTSS_IOREG(target,0x109)

/** 
 * \brief
 * Contains the Z4 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_Z4 . TX_Z4
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_Z4_TX_Z4(x)   VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_Z4_TX_Z4      VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_Z4_TX_Z4(x)   VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief N1 byte value
 *
 * \details
 * Register: \a EWIS::TX_BYTE::TX_N1
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_BYTE_TX_N1(target)      VTSS_IOREG(target,0x10a)

/** 
 * \brief
 * Contains the N1 bytes for transmission
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_BYTE_TX_N1 . TX_N1
 */
#define  VTSS_F_EWIS_TX_BYTE_TX_N1_TX_N1(x)   VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_BYTE_TX_N1_TX_N1      VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_BYTE_TX_N1_TX_N1(x)   VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a EWIS::TX_J0_INS
 *
 * Tx J0 registers
 */


/** 
 * \brief J0 byte value
 *
 * \details
 * Value of the J0 bytes
 *
 * Register: \a EWIS::TX_J0_INS::TX_J0
 *
 * @param target A \a ::vtss_target_EWIS_e target
 * @param ri Register: TX_J0 (??), 0-63
 */
#define VTSS_EWIS_TX_J0_INS_TX_J0(target,ri)  VTSS_IOREG(target,0x110 + (ri))

/** 
 * \brief
 * Contains the J0 bytes for transmission.
 * Depending on the setting in TX_J0_TRACE_LENGTH, either the first 1, the
 * first 16, or all 64 bytes are valid.
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_J0_INS_TX_J0 . TX_J0
 */
#define  VTSS_F_EWIS_TX_J0_INS_TX_J0_TX_J0(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_J0_INS_TX_J0_TX_J0     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_J0_INS_TX_J0_TX_J0(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a EWIS::TX_J1_INS
 *
 * Tx J1 registers
 */


/** 
 * \brief J1 byte value
 *
 * \details
 * Value of the J1 bytes
 *
 * Register: \a EWIS::TX_J1_INS::TX_J1
 *
 * @param target A \a ::vtss_target_EWIS_e target
 * @param ri Register: TX_J1 (??), 0-63
 */
#define VTSS_EWIS_TX_J1_INS_TX_J1(target,ri)  VTSS_IOREG(target,0x150 + (ri))

/** 
 * \brief
 * Contains the J1 bytes for transmission.
 * Depending on the setting in TX_J1_TRACE_LENGTH, either the first 1, the
 * first 16, or all 64 bytes are valid.
 *
 * \details 
 * Field: ::VTSS_EWIS_TX_J1_INS_TX_J1 . TX_J1
 */
#define  VTSS_F_EWIS_TX_J1_INS_TX_J1_TX_J1(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_EWIS_TX_J1_INS_TX_J1_TX_J1     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_EWIS_TX_J1_INS_TX_J1_TX_J1(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a EWIS::RX_TST_PTN
 *
 * Rx test pattern registers
 */


/** 
 * \brief Rx test pattern controls
 *
 * \details
 * Controls for Rx test pattern 
 *
 * Register: \a EWIS::RX_TST_PTN::RX_TST_CTRL
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_RX_TST_PTN_RX_TST_CTRL(target)  VTSS_IOREG(target,0x190)

/** 
 * \brief
 * Mixed frequency test pattern checker enable
 *
 * \details 
 * 0: Normal operation
 * 1: Check mixed frequency pattern (TSS pattern)
 *
 * Field: ::VTSS_EWIS_RX_TST_PTN_RX_TST_CTRL . RX_MIXFREQ_CHKR_EN
 */
#define  VTSS_F_EWIS_RX_TST_PTN_RX_TST_CTRL_RX_MIXFREQ_CHKR_EN(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_EWIS_RX_TST_PTN_RX_TST_CTRL_RX_MIXFREQ_CHKR_EN     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_EWIS_RX_TST_PTN_RX_TST_CTRL_RX_MIXFREQ_CHKR_EN(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/**
 * Register Group: \a EWIS::TX_TST_PTN
 *
 * Tx test pattern registers
 */


/** 
 * \brief Tx test pattern controls
 *
 * \details
 * Controls for Tx test pattern 
 *
 * Register: \a EWIS::TX_TST_PTN::TX_TST_CTRL
 *
 * @param target A \a ::vtss_target_EWIS_e target
 */
#define VTSS_EWIS_TX_TST_PTN_TX_TST_CTRL(target)  VTSS_IOREG(target,0x1b0)

/** 
 * \brief
 * Used to select the type of test pattern to be used.
 *
 * \details 
 * 0: TSS pattern as described in IEEE 802.3 clause 50.2.8.3.1
 * 1: Square wave pattern
 *
 * Field: ::VTSS_EWIS_TX_TST_PTN_TX_TST_CTRL . TX_EWIS_TEST_PAT_SEL
 */
#define  VTSS_F_EWIS_TX_TST_PTN_TX_TST_CTRL_TX_EWIS_TEST_PAT_SEL(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_EWIS_TX_TST_PTN_TX_TST_CTRL_TX_EWIS_TEST_PAT_SEL     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_EWIS_TX_TST_PTN_TX_TST_CTRL_TX_EWIS_TEST_PAT_SEL(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/** 
 * \brief
 * Enables the test pattern specified by TX_EWIS_TEST_PAT_SEL.
 *
 * \details 
 * 0: test pattern disabled
 * 1: test pattern enabled
 *
 * Field: ::VTSS_EWIS_TX_TST_PTN_TX_TST_CTRL . TX_EWIS_TEST_PAT_EN
 */
#define  VTSS_F_EWIS_TX_TST_PTN_TX_TST_CTRL_TX_EWIS_TEST_PAT_EN(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_EWIS_TX_TST_PTN_TX_TST_CTRL_TX_EWIS_TEST_PAT_EN     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_EWIS_TX_TST_PTN_TX_TST_CTRL_TX_EWIS_TEST_PAT_EN(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Square wave test pattern frequency.
 * The frequency is between approximately 425 MHz and 1289 MHz, determined
 * by the pattern of 4 to 11 ones followed by the same number of zeros.
 *
 * \details 
 * 3:0: acts as if 4 was set
 * 11:4: valid range
 * 15:12: acts as if 11 was set 
 *
 * Field: ::VTSS_EWIS_TX_TST_PTN_TX_TST_CTRL . TX_SQLEN
 */
#define  VTSS_F_EWIS_TX_TST_PTN_TX_TST_CTRL_TX_SQLEN(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_EWIS_TX_TST_PTN_TX_TST_CTRL_TX_SQLEN     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_EWIS_TX_TST_PTN_TX_TST_CTRL_TX_SQLEN(x)  VTSS_EXTRACT_BITFIELD(x,4,4)

/**
 * Register Group: \a EWIS::SPARE_REG
 *
 * Spare registers
 */


#endif /* _VTSS_DAYTONA_REGS_EWIS_H_ */
