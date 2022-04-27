#ifndef _VTSS_DAYTONA_REGS_TFI_5_H_
#define _VTSS_DAYTONA_REGS_TFI_5_H_

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
 * Target: \a TFI_5
 *
 * \see vtss_target_TFI_5_e
 *
 * TFI_5 registers
 *
 ***********************************************************************/

/**
 * Register Group: \a TFI_5::RX
 *
 * TFI_5 receiver registers
 */


/** 
 * \brief Receiver control register
 *
 * \details
 * Register: \a TFI_5::RX::RX_CONTROL
 *
 * @param target A \a ::vtss_target_TFI_5_e target
 */
#define VTSS_TFI_5_RX_RX_CONTROL(target)     VTSS_IOREG(target,0x0)

/** 
 * \brief
 * Allows rx_signal_detect input polarity inversion
 *
 * \details 
 * 0: rx_signal_detect input is active high
 * 1: rx_signal_detect input is active low
 *
 * Field: ::VTSS_TFI_5_RX_RX_CONTROL . SD_POL_INV
 */
#define  VTSS_F_TFI_5_RX_RX_CONTROL_SD_POL_INV(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_TFI_5_RX_RX_CONTROL_SD_POL_INV     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_TFI_5_RX_RX_CONTROL_SD_POL_INV(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Allows LOS input polarity inversion
 *
 * \details 
 * 0: LOS input is active high
 * 1: LOS input is active low
 *
 * Field: ::VTSS_TFI_5_RX_RX_CONTROL . LOS_POL_INV
 */
#define  VTSS_F_TFI_5_RX_RX_CONTROL_LOS_POL_INV(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_TFI_5_RX_RX_CONTROL_LOS_POL_INV     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_TFI_5_RX_RX_CONTROL_LOS_POL_INV(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Enables swapping of bytes received on lane 3 in dual rate mode (RTFI)
 *
 * \details 
 * 0: Swapping disabled, default byte order: serdes_rx[7:0] => lane 2,
 * serdes_rx[15:8] => lane 3
 * 1: Swapping enabled, inverse byte order: serdes_rx[7:0] => lane 3,
 * serdes_rx[15:8] => lane 2
 *
 * Field: ::VTSS_TFI_5_RX_RX_CONTROL . RTFI_L3_RX_BSWAP
 */
#define  VTSS_F_TFI_5_RX_RX_CONTROL_RTFI_L3_RX_BSWAP(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_TFI_5_RX_RX_CONTROL_RTFI_L3_RX_BSWAP     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_TFI_5_RX_RX_CONTROL_RTFI_L3_RX_BSWAP(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Enables swapping of bytes received on lane 0 in dual rate mode (RTFI)
 *
 * \details 
 * 0: Swapping disabled, default byte order: serdes_rx[7:0] => lane 0,
 * serdes_rx[15:8] => lane 1
 * 1: Swapping enabled, inverse byte order: serdes_rx[7:0] => lane 1,
 * serdes_rx[15:8] => lane 0
 *
 * Field: ::VTSS_TFI_5_RX_RX_CONTROL . RTFI_L0_RX_BSWAP
 */
#define  VTSS_F_TFI_5_RX_RX_CONTROL_RTFI_L0_RX_BSWAP(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_TFI_5_RX_RX_CONTROL_RTFI_L0_RX_BSWAP     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_TFI_5_RX_RX_CONTROL_RTFI_L0_RX_BSWAP(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Receiver force AIS
 *
 * \details 
 * 0: Do not force AIS in receive data stream
 * 1: Force AIS in receive data stream
 *
 * Field: ::VTSS_TFI_5_RX_RX_CONTROL . FRC_AIS
 */
#define  VTSS_F_TFI_5_RX_RX_CONTROL_FRC_AIS(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_TFI_5_RX_RX_CONTROL_FRC_AIS     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_TFI_5_RX_RX_CONTROL_FRC_AIS(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * In quick frame mode transition from out of frame to in frame occurs
 * after one framing pattern is detected. In frame to out of frame occurs
 * after one framing pattern is missed.
 *
 * \details 
 * 0: Quick framing disabled
 * 1: Quick framing enabled
 *
 * Field: ::VTSS_TFI_5_RX_RX_CONTROL . QUICK_FRAME
 */
#define  VTSS_F_TFI_5_RX_RX_CONTROL_QUICK_FRAME(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_TFI_5_RX_RX_CONTROL_QUICK_FRAME     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_TFI_5_RX_RX_CONTROL_QUICK_FRAME(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \details 
 * 0: All four receivers descramblers are enabled
 * 1: All four receivers descramblers are disabled
 *
 * Field: ::VTSS_TFI_5_RX_RX_CONTROL . DIS_DESCRAM
 */
#define  VTSS_F_TFI_5_RX_RX_CONTROL_DIS_DESCRAM(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_TFI_5_RX_RX_CONTROL_DIS_DESCRAM     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_TFI_5_RX_RX_CONTROL_DIS_DESCRAM(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Two bit field that selects which receive lane is the sync master. The
 * interpretation of this field differs based on the setting of RTFI_EN in
 * the GLOBAL_CONTROL register because lanes 1 and 2 are not used in RTFI
 * mode.
 *
 * \details 
 * For RTFI_EN=0:
 * 00: Lane 0 is sync master
 * 01: Lane 1 is sync master
 * 10: Lane 2 is sync master
 * 11: Lane 3 is sync master
 * For RTFI_EN=1:
 * 00: Lane 0 is sync master
 * 01: Lane 0 is sync master
 * 10: Lane 3 is sync master
 * 11: Lane 3 is sync master
 *
 * Field: ::VTSS_TFI_5_RX_RX_CONTROL . SYNC_SEL
 */
#define  VTSS_F_TFI_5_RX_RX_CONTROL_SYNC_SEL(x)  VTSS_ENCODE_BITFIELD(x,1,2)
#define  VTSS_M_TFI_5_RX_RX_CONTROL_SYNC_SEL     VTSS_ENCODE_BITMASK(1,2)
#define  VTSS_X_TFI_5_RX_RX_CONTROL_SYNC_SEL(x)  VTSS_EXTRACT_BITFIELD(x,1,2)


/** 
 * \brief Receiver status register
 *
 * \details
 * Register: \a TFI_5::RX::RX_STATUS
 *
 * @param target A \a ::vtss_target_TFI_5_e target
 */
#define VTSS_TFI_5_RX_RX_STATUS(target)      VTSS_IOREG(target,0x1)

/** 
 * \details 
 * 0: Loss of frame not detected
 * 1: Loss of frame detected
 *
 * Field: ::VTSS_TFI_5_RX_RX_STATUS . LOF
 */
#define  VTSS_F_TFI_5_RX_RX_STATUS_LOF(x)     VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_TFI_5_RX_RX_STATUS_LOF        VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_TFI_5_RX_RX_STATUS_LOF(x)     VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Out of frame detected for receiver lane 0
 *
 * \details 
 * 0: Lane in frame
 * 1: Lane out of frame
 *
 * Field: ::VTSS_TFI_5_RX_RX_STATUS . OOF_0
 */
#define  VTSS_F_TFI_5_RX_RX_STATUS_OOF_0(x)   VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_TFI_5_RX_RX_STATUS_OOF_0      VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_TFI_5_RX_RX_STATUS_OOF_0(x)   VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Out of frame detected for receiver lane 1
 *
 * \details 
 * 0: Lane in frame
 * 1: Lane out of frame
 *
 * Field: ::VTSS_TFI_5_RX_RX_STATUS . OOF_1
 */
#define  VTSS_F_TFI_5_RX_RX_STATUS_OOF_1(x)   VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_TFI_5_RX_RX_STATUS_OOF_1      VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_TFI_5_RX_RX_STATUS_OOF_1(x)   VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Out of frame detected for receiver lane 2
 *
 * \details 
 * 0: Lane in frame
 * 1: Lane out of frame
 *
 * Field: ::VTSS_TFI_5_RX_RX_STATUS . OOF_2
 */
#define  VTSS_F_TFI_5_RX_RX_STATUS_OOF_2(x)   VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_TFI_5_RX_RX_STATUS_OOF_2      VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_TFI_5_RX_RX_STATUS_OOF_2(x)   VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Out of frame detected for receiver lane 3
 *
 * \details 
 * 0: Lane in frame
 * 1: Lane out of frame
 *
 * Field: ::VTSS_TFI_5_RX_RX_STATUS . OOF_3
 */
#define  VTSS_F_TFI_5_RX_RX_STATUS_OOF_3(x)   VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_TFI_5_RX_RX_STATUS_OOF_3      VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_TFI_5_RX_RX_STATUS_OOF_3(x)   VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Out of alignment detected for receiver lane 0
 *
 * \details 
 * 0: Lane in alignment
 * 1: Lane out of alignment
 *
 * Field: ::VTSS_TFI_5_RX_RX_STATUS . OOA_0
 */
#define  VTSS_F_TFI_5_RX_RX_STATUS_OOA_0(x)   VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_TFI_5_RX_RX_STATUS_OOA_0      VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_TFI_5_RX_RX_STATUS_OOA_0(x)   VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Out of alignment detected for receiver lane 1
 *
 * \details 
 * 0: Lane in alignment
 * 1: Lane out of alignment
 *
 * Field: ::VTSS_TFI_5_RX_RX_STATUS . OOA_1
 */
#define  VTSS_F_TFI_5_RX_RX_STATUS_OOA_1(x)   VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_TFI_5_RX_RX_STATUS_OOA_1      VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_TFI_5_RX_RX_STATUS_OOA_1(x)   VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Out of alignment detected for receiver lane 2
 *
 * \details 
 * 0: Lane in alignment
 * 1: Lane out of alignment
 *
 * Field: ::VTSS_TFI_5_RX_RX_STATUS . OOA_2
 */
#define  VTSS_F_TFI_5_RX_RX_STATUS_OOA_2(x)   VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_TFI_5_RX_RX_STATUS_OOA_2      VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_TFI_5_RX_RX_STATUS_OOA_2(x)   VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Out of alignment detected for receiver lane 3
 *
 * \details 
 * 0: Lane in alignment
 * 1: Lane out of alignment
 *
 * Field: ::VTSS_TFI_5_RX_RX_STATUS . OOA_3
 */
#define  VTSS_F_TFI_5_RX_RX_STATUS_OOA_3(x)   VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_TFI_5_RX_RX_STATUS_OOA_3      VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_TFI_5_RX_RX_STATUS_OOA_3(x)   VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Receiver event register
 *
 * \details
 * Register: \a TFI_5::RX::RX_EVENT
 *
 * @param target A \a ::vtss_target_TFI_5_e target
 */
#define VTSS_TFI_5_RX_RX_EVENT(target)       VTSS_IOREG(target,0x2)

/** 
 * \brief
 * Loss of frame status
 *
 * \details 
 * 0: Normal operation
 * 1: Loss of frame status has changed value
 *
 * Field: ::VTSS_TFI_5_RX_RX_EVENT . LOF_STICKY
 */
#define  VTSS_F_TFI_5_RX_RX_EVENT_LOF_STICKY(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_TFI_5_RX_RX_EVENT_LOF_STICKY     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_TFI_5_RX_RX_EVENT_LOF_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Lane 0 B1 error threshold
 *
 * \details 
 * 0: Normal operation
 * 1: Lane 0 B1 error counter exceeded b1_err_threshhold_0
 *
 * Field: ::VTSS_TFI_5_RX_RX_EVENT . L0_B1_ERR_STICKY
 */
#define  VTSS_F_TFI_5_RX_RX_EVENT_L0_B1_ERR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_TFI_5_RX_RX_EVENT_L0_B1_ERR_STICKY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_TFI_5_RX_RX_EVENT_L0_B1_ERR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Lane 1 B1 error threshold
 *
 * \details 
 * 0: Normal operation
 * 1: Lane 1 B1 error counter exceeded b1_err_threshhold_1
 *
 * Field: ::VTSS_TFI_5_RX_RX_EVENT . L1_B1_ERR_STICKY
 */
#define  VTSS_F_TFI_5_RX_RX_EVENT_L1_B1_ERR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_TFI_5_RX_RX_EVENT_L1_B1_ERR_STICKY     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_TFI_5_RX_RX_EVENT_L1_B1_ERR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Lane 2 B1 error threshold
 *
 * \details 
 * 0: Normal operation
 * 1: Lane 2 B1 error counter exceeded b1_err_threshhold_2
 *
 * Field: ::VTSS_TFI_5_RX_RX_EVENT . L2_B1_ERR_STICKY
 */
#define  VTSS_F_TFI_5_RX_RX_EVENT_L2_B1_ERR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_TFI_5_RX_RX_EVENT_L2_B1_ERR_STICKY     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_TFI_5_RX_RX_EVENT_L2_B1_ERR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Lane 3 B1 error threshold
 *
 * \details 
 * 0: Normal operation
 * 1: Lane 3 B1 error counter exceeded b1_err_threshhold_3
 *
 * Field: ::VTSS_TFI_5_RX_RX_EVENT . L3_B1_ERR_STICKY
 */
#define  VTSS_F_TFI_5_RX_RX_EVENT_L3_B1_ERR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_TFI_5_RX_RX_EVENT_L3_B1_ERR_STICKY     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_TFI_5_RX_RX_EVENT_L3_B1_ERR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Lane 0 out of frame status
 *
 * \details 
 * 0: Normal operation
 * 1: Lane 0 OOF status has changed value
 *
 * Field: ::VTSS_TFI_5_RX_RX_EVENT . L0_OOF_STICKY
 */
#define  VTSS_F_TFI_5_RX_RX_EVENT_L0_OOF_STICKY(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_TFI_5_RX_RX_EVENT_L0_OOF_STICKY     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_TFI_5_RX_RX_EVENT_L0_OOF_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Lane 1 out of frame status
 *
 * \details 
 * 0: Normal operation
 * 1: Lane 1 OOF status has changed value
 *
 * Field: ::VTSS_TFI_5_RX_RX_EVENT . L1_OOF_STICKY
 */
#define  VTSS_F_TFI_5_RX_RX_EVENT_L1_OOF_STICKY(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_TFI_5_RX_RX_EVENT_L1_OOF_STICKY     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_TFI_5_RX_RX_EVENT_L1_OOF_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Lane 2 out of frame status
 *
 * \details 
 * 0: Normal operation
 * 1: Lane 2 OOF status has changed value
 *
 * Field: ::VTSS_TFI_5_RX_RX_EVENT . L2_OOF_STICKY
 */
#define  VTSS_F_TFI_5_RX_RX_EVENT_L2_OOF_STICKY(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_TFI_5_RX_RX_EVENT_L2_OOF_STICKY     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_TFI_5_RX_RX_EVENT_L2_OOF_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Lane 3 out of frame status
 *
 * \details 
 * 0: Normal operation
 * 1: Lane 3 OOF status has changed value
 *
 * Field: ::VTSS_TFI_5_RX_RX_EVENT . L3_OOF_STICKY
 */
#define  VTSS_F_TFI_5_RX_RX_EVENT_L3_OOF_STICKY(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_TFI_5_RX_RX_EVENT_L3_OOF_STICKY     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_TFI_5_RX_RX_EVENT_L3_OOF_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Lane 0 out of alignment status
 *
 * \details 
 * 0: Normal operation
 * 1: Lane 0 OOA status has changed value
 *
 * Field: ::VTSS_TFI_5_RX_RX_EVENT . L0_OOA_STICKY
 */
#define  VTSS_F_TFI_5_RX_RX_EVENT_L0_OOA_STICKY(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_TFI_5_RX_RX_EVENT_L0_OOA_STICKY     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_TFI_5_RX_RX_EVENT_L0_OOA_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Lane 1 out of alignment status
 *
 * \details 
 * 0: Normal operation
 * 1: Lane 1 OOA status has changed value
 *
 * Field: ::VTSS_TFI_5_RX_RX_EVENT . L1_OOA_STICKY
 */
#define  VTSS_F_TFI_5_RX_RX_EVENT_L1_OOA_STICKY(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_TFI_5_RX_RX_EVENT_L1_OOA_STICKY     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_TFI_5_RX_RX_EVENT_L1_OOA_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Lane 2 out of alignment status
 *
 * \details 
 * 0: Normal operation
 * 1: Lane 2 OOA status has changed value
 *
 * Field: ::VTSS_TFI_5_RX_RX_EVENT . L2_OOA_STICKY
 */
#define  VTSS_F_TFI_5_RX_RX_EVENT_L2_OOA_STICKY(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_TFI_5_RX_RX_EVENT_L2_OOA_STICKY     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_TFI_5_RX_RX_EVENT_L2_OOA_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Lane 3 out of alignment status
 *
 * \details 
 * 0: Normal operation
 * 1: Lane 3 OOA status has changed value
 *
 * Field: ::VTSS_TFI_5_RX_RX_EVENT . L3_OOA_STICKY
 */
#define  VTSS_F_TFI_5_RX_RX_EVENT_L3_OOA_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_TFI_5_RX_RX_EVENT_L3_OOA_STICKY     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_TFI_5_RX_RX_EVENT_L3_OOA_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Receiver mask register
 *
 * \details
 * This register contains active high mask bits for the events detected in
 * the rx_event register. The bit positions of the masks are identical to
 * the corresponding events in the rx_event register.
 *
 * Register: \a TFI_5::RX::RX_MASK
 *
 * @param target A \a ::vtss_target_TFI_5_e target
 */
#define VTSS_TFI_5_RX_RX_MASK(target)        VTSS_IOREG(target,0x3)

/** 
 * \brief
 * Loss of frame mask
 *
 * \details 
 * 0: Mask LOF event such that it does not cause a tfi_5_int
 * 1: Allow LOF event to cause a tfi_5_int
 *
 * Field: ::VTSS_TFI_5_RX_RX_MASK . LOF_MASK
 */
#define  VTSS_F_TFI_5_RX_RX_MASK_LOF_MASK(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_TFI_5_RX_RX_MASK_LOF_MASK     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_TFI_5_RX_RX_MASK_LOF_MASK(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Lane 0 B1 error mask
 *
 * \details 
 * 0: Mask Lane 0 B1 error threshold event such that it does not cause a
 * tfi_5_int
 * 1: Allow Lane 0 B1 error threshold event to cause a tfi_5_int
 *
 * Field: ::VTSS_TFI_5_RX_RX_MASK . L0_B1_ERR_MASK
 */
#define  VTSS_F_TFI_5_RX_RX_MASK_L0_B1_ERR_MASK(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_TFI_5_RX_RX_MASK_L0_B1_ERR_MASK     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_TFI_5_RX_RX_MASK_L0_B1_ERR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Lane 1 B1 error mask
 *
 * \details 
 * 0: Mask Lane 1 B1 error threshold event such that it does not cause a
 * tfi_5_int
 * 1: Allow Lane 1 B1 error threshold event to cause a tfi_5_int
 *
 * Field: ::VTSS_TFI_5_RX_RX_MASK . L1_B1_ERR_MASK
 */
#define  VTSS_F_TFI_5_RX_RX_MASK_L1_B1_ERR_MASK(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_TFI_5_RX_RX_MASK_L1_B1_ERR_MASK     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_TFI_5_RX_RX_MASK_L1_B1_ERR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Lane 2 B1 error mask
 *
 * \details 
 * 0: Mask Lane 2 B1 error threshold event such that it does not cause a
 * tfi_5_int
 * 1: Allow Lane 2 B1 error threshold event to cause a tfi_5_int
 *
 * Field: ::VTSS_TFI_5_RX_RX_MASK . L2_B1_ERR_MASK
 */
#define  VTSS_F_TFI_5_RX_RX_MASK_L2_B1_ERR_MASK(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_TFI_5_RX_RX_MASK_L2_B1_ERR_MASK     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_TFI_5_RX_RX_MASK_L2_B1_ERR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Lane 3 B1 error mask
 *
 * \details 
 * 0: Mask Lane 3 B1 error threshold event such that it does not cause a
 * tfi_5_int
 * 1: Allow Lane 3 B1 error threshold event to cause a tfi_5_int
 *
 * Field: ::VTSS_TFI_5_RX_RX_MASK . L3_B1_ERR_MASK
 */
#define  VTSS_F_TFI_5_RX_RX_MASK_L3_B1_ERR_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_TFI_5_RX_RX_MASK_L3_B1_ERR_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_TFI_5_RX_RX_MASK_L3_B1_ERR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Lane 0 out of frame mask
 *
 * \details 
 * 0: Mask Lane 0 out of frame event such that it does not cause a
 * tfi_5_int
 * 1: Allow Lane 0 out of frame event to cause a tfi_5_int
 *
 * Field: ::VTSS_TFI_5_RX_RX_MASK . L0_OOF_MASK
 */
#define  VTSS_F_TFI_5_RX_RX_MASK_L0_OOF_MASK(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_TFI_5_RX_RX_MASK_L0_OOF_MASK     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_TFI_5_RX_RX_MASK_L0_OOF_MASK(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Lane 1 out of frame mask
 *
 * \details 
 * 0: Mask Lane 1 out of frame event such that it does not cause a
 * tfi_5_int
 * 1: Allow Lane 0 out of frame event to cause a tfi_5_int
 *
 * Field: ::VTSS_TFI_5_RX_RX_MASK . L1_OOF_MASK
 */
#define  VTSS_F_TFI_5_RX_RX_MASK_L1_OOF_MASK(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_TFI_5_RX_RX_MASK_L1_OOF_MASK     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_TFI_5_RX_RX_MASK_L1_OOF_MASK(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Lane 2 out of frame mask
 *
 * \details 
 * 0: Mask Lane 2 out of frame event such that it does not cause a
 * tfi_5_int
 * 1: Allow Lane 2 out of frame event to cause a tfi_5_int
 *
 * Field: ::VTSS_TFI_5_RX_RX_MASK . L2_OOF_MASK
 */
#define  VTSS_F_TFI_5_RX_RX_MASK_L2_OOF_MASK(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_TFI_5_RX_RX_MASK_L2_OOF_MASK     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_TFI_5_RX_RX_MASK_L2_OOF_MASK(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Lane 3 out of frame mask
 *
 * \details 
 * 0: Mask Lane 3 out of frame event such that it does not cause a
 * tfi_5_int
 * 1: Allow Lane 3 out of frame event to cause a tfi_5_int
 *
 * Field: ::VTSS_TFI_5_RX_RX_MASK . L3_OOF_MASK
 */
#define  VTSS_F_TFI_5_RX_RX_MASK_L3_OOF_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_TFI_5_RX_RX_MASK_L3_OOF_MASK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_TFI_5_RX_RX_MASK_L3_OOF_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Lane 0 out of alignment mask
 *
 * \details 
 * 0: Mask Lane 0 out of alignment event such that it does not cause a
 * tfi_5_int
 * 1: Allow Lane 0 out of alignment event to cause a tfi_5_int
 *
 * Field: ::VTSS_TFI_5_RX_RX_MASK . L0_OOA_MASK
 */
#define  VTSS_F_TFI_5_RX_RX_MASK_L0_OOA_MASK(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_TFI_5_RX_RX_MASK_L0_OOA_MASK     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_TFI_5_RX_RX_MASK_L0_OOA_MASK(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Lane 1 out of alignment mask
 *
 * \details 
 * 0: Mask Lane 1 out of alignment event such that it does not cause a
 * tfi_5_int
 * 1: Allow Lane 1 out of alignment event to cause a tfi_5_int
 *
 * Field: ::VTSS_TFI_5_RX_RX_MASK . L1_OOA_MASK
 */
#define  VTSS_F_TFI_5_RX_RX_MASK_L1_OOA_MASK(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_TFI_5_RX_RX_MASK_L1_OOA_MASK     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_TFI_5_RX_RX_MASK_L1_OOA_MASK(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Lane 2 out of alignment mask
 *
 * \details 
 * 0: Mask Lane 2 out of alignment event such that it does not cause a
 * tfi_5_int
 * 1: Allow Lane 2 out of alignment event to cause a tfi_5_int
 *
 * Field: ::VTSS_TFI_5_RX_RX_MASK . L2_OOA_MASK
 */
#define  VTSS_F_TFI_5_RX_RX_MASK_L2_OOA_MASK(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_TFI_5_RX_RX_MASK_L2_OOA_MASK     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_TFI_5_RX_RX_MASK_L2_OOA_MASK(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Lane 3 out of alignment mask
 *
 * \details 
 * 0: Mask Lane 3 out of alignment event such that it does not cause a
 * tfi_5_int
 * 1: Allow Lane 3 out of alignment event to cause a tfi_5_int
 *
 * Field: ::VTSS_TFI_5_RX_RX_MASK . L3_OOA_MASK
 */
#define  VTSS_F_TFI_5_RX_RX_MASK_L3_OOA_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_TFI_5_RX_RX_MASK_L3_OOA_MASK     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_TFI_5_RX_RX_MASK_L3_OOA_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Receiver b1 frame count register
 *
 * \details
 * Register: \a TFI_5::RX::RX_B1_ERR_FRM_CNT
 *
 * @param target A \a ::vtss_target_TFI_5_e target
 */
#define VTSS_TFI_5_RX_RX_B1_ERR_FRM_CNT(target)  VTSS_IOREG(target,0x4)

/** 
 * \brief
 * This field represents the number of frames over which parity is checked
 * before the parity error counter is reset.
 *
 * \details 
 * Field: ::VTSS_TFI_5_RX_RX_B1_ERR_FRM_CNT . B1_ERR_FRM_CNT
 */
#define  VTSS_F_TFI_5_RX_RX_B1_ERR_FRM_CNT_B1_ERR_FRM_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_TFI_5_RX_RX_B1_ERR_FRM_CNT_B1_ERR_FRM_CNT     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_TFI_5_RX_RX_B1_ERR_FRM_CNT_B1_ERR_FRM_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief Receiver b1 error threshold register
 *
 * \details
 * Register: \a TFI_5::RX::RX_B1_ERR_THRESH
 *
 * @param target A \a ::vtss_target_TFI_5_e target
 */
#define VTSS_TFI_5_RX_RX_B1_ERR_THRESH(target)  VTSS_IOREG(target,0x5)

/** 
 * \brief
 * Sets the thresh hold at which b1 errors cause b1_err events. i.e. if
 * there are b1_err_thresh b1 errors in a b1_err_frm_cnt period then a
 * b1_err event will be asserted for that channel.
 *
 * \details 
 * Field: ::VTSS_TFI_5_RX_RX_B1_ERR_THRESH . B1_ERR_THRESH
 */
#define  VTSS_F_TFI_5_RX_RX_B1_ERR_THRESH_B1_ERR_THRESH(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_TFI_5_RX_RX_B1_ERR_THRESH_B1_ERR_THRESH     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_TFI_5_RX_RX_B1_ERR_THRESH_B1_ERR_THRESH(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief Receiver lane 0 sync difference register
 *
 * \details
 * Read only register containing the number of clock cycles between lane
 * 0's frame sync and the synch master's frame sync.
 *
 * Register: \a TFI_5::RX::L0_SYNC_DIFF
 *
 * @param target A \a ::vtss_target_TFI_5_e target
 */
#define VTSS_TFI_5_RX_L0_SYNC_DIFF(target)   VTSS_IOREG(target,0x6)

/** 
 * \brief
 * Read only register containing the number of clock cycles between lane
 * 0's frame sync and the sync master's frame sync. This register contains
 * the value of a counter that free runs until it's lanes frame_sync pulse
 * is received at which time it resets to 0. The count ranges from 0 to
 * 38879 (and then wraps back to 0). So for example a value of 38878
 * represents a sync difference of -2 with respect to the master lane. This
 * register should always contain a value of 0 for the sync master lane.
 *
 * \details 
 * Field: ::VTSS_TFI_5_RX_L0_SYNC_DIFF . L0_SYNC_DIFF
 */
#define  VTSS_F_TFI_5_RX_L0_SYNC_DIFF_L0_SYNC_DIFF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_TFI_5_RX_L0_SYNC_DIFF_L0_SYNC_DIFF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_TFI_5_RX_L0_SYNC_DIFF_L0_SYNC_DIFF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Receiver lane 1 sync difference register
 *
 * \details
 * Read only register containing the number of clock cycles between lane
 * 1's frame sync and the synch master's frame sync.
 *
 * Register: \a TFI_5::RX::L1_SYNC_DIFF
 *
 * @param target A \a ::vtss_target_TFI_5_e target
 */
#define VTSS_TFI_5_RX_L1_SYNC_DIFF(target)   VTSS_IOREG(target,0x7)

/** 
 * \brief
 * Read only register containing the number of clock cycles between lane
 * 0's frame sync and the sync master's frame sync. This register contains
 * the value of a counter that free runs until it's lanes frame_sync pulse
 * is received at which time it resets to 0. The count ranges from 0 to
 * 38879 (and then wraps back to 0). So for example a value of 38878
 * represents a sync difference of -2 with respect to the master lane. This
 * register should always contain a value of 0 for the sync master lane.
 *
 * \details 
 * Field: ::VTSS_TFI_5_RX_L1_SYNC_DIFF . L1_SYNC_DIFF
 */
#define  VTSS_F_TFI_5_RX_L1_SYNC_DIFF_L1_SYNC_DIFF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_TFI_5_RX_L1_SYNC_DIFF_L1_SYNC_DIFF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_TFI_5_RX_L1_SYNC_DIFF_L1_SYNC_DIFF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Receiver lane 2 sync difference register
 *
 * \details
 * Read only register containing the number of clock cycles between lane
 * 2's frame sync and the synch master's frame sync.
 *
 * Register: \a TFI_5::RX::L2_SYNC_DIFF
 *
 * @param target A \a ::vtss_target_TFI_5_e target
 */
#define VTSS_TFI_5_RX_L2_SYNC_DIFF(target)   VTSS_IOREG(target,0x8)

/** 
 * \brief
 * Read only register containing the number of clock cycles between lane
 * 0's frame sync and the sync master's frame sync. This register contains
 * the value of a counter that free runs until it's lanes frame_sync pulse
 * is received at which time it resets to 0. The count ranges from 0 to
 * 38879 (and then wraps back to 0). So for example a value of 38878
 * represents a sync difference of -2 with respect to the master lane. This
 * register should always contain a value of 0 for the sync master lane.
 *
 * \details 
 * Field: ::VTSS_TFI_5_RX_L2_SYNC_DIFF . L2_SYNC_DIFF
 */
#define  VTSS_F_TFI_5_RX_L2_SYNC_DIFF_L2_SYNC_DIFF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_TFI_5_RX_L2_SYNC_DIFF_L2_SYNC_DIFF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_TFI_5_RX_L2_SYNC_DIFF_L2_SYNC_DIFF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Receiver lane 3 sync difference register
 *
 * \details
 * Read only register containing the number of clock cycles between lane
 * 3's frame sync and the synch master's frame sync.
 *
 * Register: \a TFI_5::RX::L3_SYNC_DIFF
 *
 * @param target A \a ::vtss_target_TFI_5_e target
 */
#define VTSS_TFI_5_RX_L3_SYNC_DIFF(target)   VTSS_IOREG(target,0x9)

/** 
 * \brief
 * Read only register containing the number of clock cycles between lane
 * 0's frame sync and the sync master's frame sync. This register contains
 * the value of a counter that free runs until it's lanes frame_sync pulse
 * is received at which time it resets to 0. The count ranges from 0 to
 * 38879 (and then wraps back to 0). So for example a value of 38878
 * represents a sync difference of -2 with respect to the master lane. This
 * register should always contain a value of 0 for the sync master lane.
 *
 * \details 
 * Field: ::VTSS_TFI_5_RX_L3_SYNC_DIFF . L3_SYNC_DIFF
 */
#define  VTSS_F_TFI_5_RX_L3_SYNC_DIFF_L3_SYNC_DIFF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_TFI_5_RX_L3_SYNC_DIFF_L3_SYNC_DIFF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_TFI_5_RX_L3_SYNC_DIFF_L3_SYNC_DIFF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Control bits for LOF timers
 *
 * \details
 * Register: \a TFI_5::RX::LOF_TIMER_CTL
 *
 * @param target A \a ::vtss_target_TFI_5_e target
 */
#define VTSS_TFI_5_RX_LOF_TIMER_CTL(target)  VTSS_IOREG(target,0xa)

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
 * Field: ::VTSS_TFI_5_RX_LOF_TIMER_CTL . RX_LOF_T3_VAL
 */
#define  VTSS_F_TFI_5_RX_LOF_TIMER_CTL_RX_LOF_T3_VAL(x)  VTSS_ENCODE_BITFIELD(x,16,6)
#define  VTSS_M_TFI_5_RX_LOF_TIMER_CTL_RX_LOF_T3_VAL     VTSS_ENCODE_BITMASK(16,6)
#define  VTSS_X_TFI_5_RX_LOF_TIMER_CTL_RX_LOF_T3_VAL(x)  VTSS_EXTRACT_BITFIELD(x,16,6)

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
 * Field: ::VTSS_TFI_5_RX_LOF_TIMER_CTL . RX_LOF_T2_VAL
 */
#define  VTSS_F_TFI_5_RX_LOF_TIMER_CTL_RX_LOF_T2_VAL(x)  VTSS_ENCODE_BITFIELD(x,8,6)
#define  VTSS_M_TFI_5_RX_LOF_TIMER_CTL_RX_LOF_T2_VAL     VTSS_ENCODE_BITMASK(8,6)
#define  VTSS_X_TFI_5_RX_LOF_TIMER_CTL_RX_LOF_T2_VAL(x)  VTSS_EXTRACT_BITFIELD(x,8,6)

/** 
 * \brief
 * Integrating timer T1 for LOF detection.
 * When OOF is persistent for T1 frames, LOF is declared.
 *
 * \details 
 * 0: Invalid
 * 63:1: Valid range
 *
 * Field: ::VTSS_TFI_5_RX_LOF_TIMER_CTL . RX_LOF_T1_VAL
 */
#define  VTSS_F_TFI_5_RX_LOF_TIMER_CTL_RX_LOF_T1_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_TFI_5_RX_LOF_TIMER_CTL_RX_LOF_T1_VAL     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_TFI_5_RX_LOF_TIMER_CTL_RX_LOF_T1_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

/**
 * Register Group: \a TFI_5::TX
 *
 * TFI_5 transmitter registers
 */


/** 
 * \brief Transmitter control register
 *
 * \details
 * Register: \a TFI_5::TX::TX_CONTROL
 *
 * @param target A \a ::vtss_target_TFI_5_e target
 */
#define VTSS_TFI_5_TX_TX_CONTROL(target)     VTSS_IOREG(target,0x10)

/** 
 * \brief
 * Enables swapping of bytes transmitted on lane 3 in dual rate mode (RTFI)
 *
 * \details 
 * 0: Swapping disabled, default byte order: lane 2 => serdes_tx[7:0], lane
 * 3 => serdes_tx[15:8]
 * 1: Swapping disabled, default byte order: lane 3 => serdes_tx[7:0], lane
 * 02=> serdes_tx[15:8]
 *
 * Field: ::VTSS_TFI_5_TX_TX_CONTROL . RTFI_L3_TX_BSWAP
 */
#define  VTSS_F_TFI_5_TX_TX_CONTROL_RTFI_L3_TX_BSWAP(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_TFI_5_TX_TX_CONTROL_RTFI_L3_TX_BSWAP     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_TFI_5_TX_TX_CONTROL_RTFI_L3_TX_BSWAP(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Enables swapping of bytes transmitted on lane 0 in dual rate mode (RTFI)
 *
 * \details 
 * 0: Swapping disabled, default byte order: lane 0 => serdes_tx[7:0], lane
 * 1 => serdes_tx[15:8]
 * 1: Swapping disabled, default byte order: lane 1 => serdes_tx[7:0], lane
 * 0 => serdes_tx[15:8]

 *
 * Field: ::VTSS_TFI_5_TX_TX_CONTROL . RTFI_L0_TX_BSWAP
 */
#define  VTSS_F_TFI_5_TX_TX_CONTROL_RTFI_L0_TX_BSWAP(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_TFI_5_TX_TX_CONTROL_RTFI_L0_TX_BSWAP     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_TFI_5_TX_TX_CONTROL_RTFI_L0_TX_BSWAP(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \details 
 * 0: a1a2 error is not inserted
 * 1: a1a2 error is inserted for the channel selected by a1a2_err_ch
 *
 * Field: ::VTSS_TFI_5_TX_TX_CONTROL . A1A2_ERR_INS
 */
#define  VTSS_F_TFI_5_TX_TX_CONTROL_A1A2_ERR_INS(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_TFI_5_TX_TX_CONTROL_A1A2_ERR_INS     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_TFI_5_TX_TX_CONTROL_A1A2_ERR_INS(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \details 
 * 0: a1a2 error not inserted
 * 1: a1a2 error inserted for all channels (regardless of the setting of
 * a1a2_err_ins and a1_a2_err_ch)
 *
 * Field: ::VTSS_TFI_5_TX_TX_CONTROL . A1A2_ERR_ALL
 */
#define  VTSS_F_TFI_5_TX_TX_CONTROL_A1A2_ERR_ALL(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_TFI_5_TX_TX_CONTROL_A1A2_ERR_ALL     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_TFI_5_TX_TX_CONTROL_A1A2_ERR_ALL(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Two bit field indicating which transmit channel to insert an a1a2 error
 * into if a1a2_err_ins is asserted
 *
 * \details 
 * 00: Insert error in channel 0
 * 01: Insert error in channel 1
 * 10: Insert error in channel 2
 * 11: Insert error in channel 3
 *
 * Field: ::VTSS_TFI_5_TX_TX_CONTROL . A1A2_ERR_CH
 */
#define  VTSS_F_TFI_5_TX_TX_CONTROL_A1A2_ERR_CH(x)  VTSS_ENCODE_BITFIELD(x,9,2)
#define  VTSS_M_TFI_5_TX_TX_CONTROL_A1A2_ERR_CH     VTSS_ENCODE_BITMASK(9,2)
#define  VTSS_X_TFI_5_TX_TX_CONTROL_A1A2_ERR_CH(x)  VTSS_EXTRACT_BITFIELD(x,9,2)

/** 
 * \details 
 * 0: b1error is not inserted
 * 1: b1 error is inserted for the channel selected by b1_err_ch
 *
 * Field: ::VTSS_TFI_5_TX_TX_CONTROL . B1_ERR_INS
 */
#define  VTSS_F_TFI_5_TX_TX_CONTROL_B1_ERR_INS(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_TFI_5_TX_TX_CONTROL_B1_ERR_INS     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_TFI_5_TX_TX_CONTROL_B1_ERR_INS(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \details 
 * 0: b1 error not inserted
 * 1: b1 error inserted for all channels (regardless of the setting of
 * b1_err_ins and a1_b1_err_ch)
 *
 * Field: ::VTSS_TFI_5_TX_TX_CONTROL . B1_ERR_ALL
 */
#define  VTSS_F_TFI_5_TX_TX_CONTROL_B1_ERR_ALL(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_TFI_5_TX_TX_CONTROL_B1_ERR_ALL     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_TFI_5_TX_TX_CONTROL_B1_ERR_ALL(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Two bit field indicating which transmit channel to insert an b1 error
 * into if b1_err_ins is asserted
 *
 * \details 
 * 00: insert error in channel 0
 * 01: insert error in channel 1
 * 10: insert error in channel 2
 * 11: insert error in channel 3
 *
 * Field: ::VTSS_TFI_5_TX_TX_CONTROL . B1_ERR_CH
 */
#define  VTSS_F_TFI_5_TX_TX_CONTROL_B1_ERR_CH(x)  VTSS_ENCODE_BITFIELD(x,5,2)
#define  VTSS_M_TFI_5_TX_TX_CONTROL_B1_ERR_CH     VTSS_ENCODE_BITMASK(5,2)
#define  VTSS_X_TFI_5_TX_TX_CONTROL_B1_ERR_CH(x)  VTSS_EXTRACT_BITFIELD(x,5,2)

/** 
 * \brief
 * AIS insert
 *
 * \details 
 * 0: AIS is not inserted in data stream
 * 1: AIS is inserted in the data stream for all channels
 *
 * Field: ::VTSS_TFI_5_TX_TX_CONTROL . AIS_INS
 */
#define  VTSS_F_TFI_5_TX_TX_CONTROL_AIS_INS(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_TFI_5_TX_TX_CONTROL_AIS_INS     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_TFI_5_TX_TX_CONTROL_AIS_INS(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * This bit allows scrambling to be disabled. Applies to all transmit
 * channels.
 *
 * \details 
 * 0: All four transmit channel scramblers are enabled
 * 1: All four transmit channel scramblers are disabled
 *
 * Field: ::VTSS_TFI_5_TX_TX_CONTROL . DIS_SCRAM
 */
#define  VTSS_F_TFI_5_TX_TX_CONTROL_DIS_SCRAM(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_TFI_5_TX_TX_CONTROL_DIS_SCRAM     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_TFI_5_TX_TX_CONTROL_DIS_SCRAM(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \details 
 * 0: j0 and z0 bytes not modified by transmitter
 * 1: A fixed pattern of 0xCC is inserted in the j0 and z0 bytes on all
 * lanes
 *
 * Field: ::VTSS_TFI_5_TX_TX_CONTROL . J0_Z0_FILL_ENABLE
 */
#define  VTSS_F_TFI_5_TX_TX_CONTROL_J0_Z0_FILL_ENABLE(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_TFI_5_TX_TX_CONTROL_J0_Z0_FILL_ENABLE     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_TFI_5_TX_TX_CONTROL_J0_Z0_FILL_ENABLE(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \details 
 * 0: b1 byte replaced by calculated BIP-8 parity
 * 1: Inhibit b1 parity calculation for all lanes and pass through b1 byte
 * received from core interface
 *
 * Field: ::VTSS_TFI_5_TX_TX_CONTROL . B1_CALC_INH
 */
#define  VTSS_F_TFI_5_TX_TX_CONTROL_B1_CALC_INH(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_TFI_5_TX_TX_CONTROL_B1_CALC_INH     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_TFI_5_TX_TX_CONTROL_B1_CALC_INH(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * A write of a 1 resets all four transmitter lane's internal b1_parity
 * calculation registers. This is a self resetting bit.
 *
 * \details 
 * Field: ::VTSS_TFI_5_TX_TX_CONTROL . B1_PRTY_RST
 */
#define  VTSS_F_TFI_5_TX_TX_CONTROL_B1_PRTY_RST(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_TFI_5_TX_TX_CONTROL_B1_PRTY_RST     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_TFI_5_TX_TX_CONTROL_B1_PRTY_RST(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/**
 * Register Group: \a TFI_5::GLOBAL
 *
 * TFI_5 registers that apply to both transmitter and receiver
 */


/** 
 * \brief Global control register
 *
 * \details
 * Register: \a TFI_5::GLOBAL::GLOBAL_CONTROL
 *
 * @param target A \a ::vtss_target_TFI_5_e target
 */
#define VTSS_TFI_5_GLOBAL_GLOBAL_CONTROL(target)  VTSS_IOREG(target,0x12)

/** 
 * \brief
 * Rx to Tx loopback enable
 *
 * \details 
 * 0: Rx to Tx loopback disabled
 * 1: Rx to Tx loopback enabled
 *
 * Field: ::VTSS_TFI_5_GLOBAL_GLOBAL_CONTROL . LOOP_EN
 */
#define  VTSS_F_TFI_5_GLOBAL_GLOBAL_CONTROL_LOOP_EN(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_TFI_5_GLOBAL_GLOBAL_CONTROL_LOOP_EN     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_TFI_5_GLOBAL_GLOBAL_CONTROL_LOOP_EN(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \details 
 * 0: TFI-5 block in single rate mode
 * 1: TFI-5 block in dual rate mode
 *
 * Field: ::VTSS_TFI_5_GLOBAL_GLOBAL_CONTROL . RTFI_EN
 */
#define  VTSS_F_TFI_5_GLOBAL_GLOBAL_CONTROL_RTFI_EN(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_TFI_5_GLOBAL_GLOBAL_CONTROL_RTFI_EN     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_TFI_5_GLOBAL_GLOBAL_CONTROL_RTFI_EN(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Spare register
 *
 * \details
 * Register: \a TFI_5::GLOBAL::SPARE
 *
 * @param target A \a ::vtss_target_TFI_5_e target
 */
#define VTSS_TFI_5_GLOBAL_SPARE(target)      VTSS_IOREG(target,0x13)

/** 
 * \brief
 * Spare read/writable 32 bit register
 *
 * \details 
 * Field: ::VTSS_TFI_5_GLOBAL_SPARE . SPARE_REG
 */
#define  VTSS_F_TFI_5_GLOBAL_SPARE_SPARE_REG(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_TFI_5_GLOBAL_SPARE_SPARE_REG     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_TFI_5_GLOBAL_SPARE_SPARE_REG(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


#endif /* _VTSS_DAYTONA_REGS_TFI_5_H_ */
