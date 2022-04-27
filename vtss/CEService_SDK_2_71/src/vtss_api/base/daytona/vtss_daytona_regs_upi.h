#ifndef _VTSS_DAYTONA_REGS_UPI_H_
#define _VTSS_DAYTONA_REGS_UPI_H_

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
 * Target: \a UPI
 *
 * \see vtss_target_UPI_e
 *
 * Universal PHY Interface (UPI)
 *
 ***********************************************************************/

/**
 * Register Group: \a UPI::UPI_CONFIG
 *
 * UPI configuration
 */


/** 
 * \brief UPI configuration
 *
 * \details
 * Configuration register
 *
 * Register: \a UPI::UPI_CONFIG::UPI_CONFIG
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_CONFIG_UPI_CONFIG(target)  VTSS_IOREG(target,0x0)

/** 
 * \brief
 * Software enable will gate off clocks to UPI and assert async reset
 *
 * \details 
 * 0: UPI disabled
 * 1: UPI enabled
 *
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_CONFIG . SW_ENA
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_CONFIG_SW_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_CONFIG_SW_ENA     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_CONFIG_SW_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief UPI Tx configuration
 *
 * \details
 * Configuration register
 *
 * Register: \a UPI::UPI_CONFIG::UPI_TX_CONFIG
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_CONFIG_UPI_TX_CONFIG(target)  VTSS_IOREG(target,0x1)

/** 
 * \brief
 * Selects bit order for chip core side 64 bit data path.
 *
 * \details 
 * Internal 64 bit bus bit order:
 * 0: 0 to 63 (SFI4.2)
 * 3: 63 to 0 (XAUI, TFI5)
 * 1: 32 to 63 followed by 0 to 31 (unused)
 * 2: 31 to 0 followed by 63 to 32 (unused)
 *
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_TX_CONFIG . TX_ENDIAN
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_TX_CONFIG_TX_ENDIAN(x)  VTSS_ENCODE_BITFIELD(x,9,2)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_TX_CONFIG_TX_ENDIAN     VTSS_ENCODE_BITMASK(9,2)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_TX_CONFIG_TX_ENDIAN(x)  VTSS_EXTRACT_BITFIELD(x,9,2)

/** 
 * \brief
 * Selects double serial rate.
 *
 * \details 
 * 0: Normal mode
 * 1: Double data rate mode
 *
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_TX_CONFIG . TX_DDR_MODE
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_TX_CONFIG_TX_DDR_MODE(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_TX_CONFIG_TX_DDR_MODE     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_TX_CONFIG_TX_DDR_MODE(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Swaps (inverts) bits per lane.
 *
 * \details 
 * 0: No swap
 * 1: Swap
 *
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_TX_CONFIG . TX_PAIR_SWAP
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_TX_CONFIG_TX_PAIR_SWAP(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_TX_CONFIG_TX_PAIR_SWAP     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_TX_CONFIG_TX_PAIR_SWAP(x)  VTSS_EXTRACT_BITFIELD(x,4,4)

/** 
 * \brief
 * Tx software reset
 *
 * \details 
 * 0: Not reset
 * 1: Reset
 *
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_TX_CONFIG . TX_SW_RST
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_TX_CONFIG_TX_SW_RST(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_TX_CONFIG_TX_SW_RST     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_TX_CONFIG_TX_SW_RST(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Swaps lanes 0 <-> 3 and 1 <-> 2.
 *
 * \details 
 * 0: No swap
 * 1: Swap
 *
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_TX_CONFIG . TX_LANE_SWAP
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_TX_CONFIG_TX_LANE_SWAP(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_TX_CONFIG_TX_LANE_SWAP     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_TX_CONFIG_TX_LANE_SWAP(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Selects protocol.

 *
 * \details 
 * 0,3: XAUI
 * 1: SFI4.2
 * 2: TFI5
 *
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_TX_CONFIG . TX_PROTO_SEL
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_TX_CONFIG_TX_PROTO_SEL(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_TX_CONFIG_TX_PROTO_SEL     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_TX_CONFIG_TX_PROTO_SEL(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief UPI Rx configuration
 *
 * \details
 * Configuration register
 *
 * Register: \a UPI::UPI_CONFIG::UPI_RX_CONFIG
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_CONFIG_UPI_RX_CONFIG(target)  VTSS_IOREG(target,0x2)

/** 
 * \brief
 * SSF loss of signal polarity
 *
 * \details 
 * 0: Must be 0 to indicate loss of signal
 * 1: Must be 1 to indicate loss of signal
 *
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_RX_CONFIG . RX_SSF_LOS_POL
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_SSF_LOS_POL(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_SSF_LOS_POL     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_SSF_LOS_POL(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * SSF loss of signal enable
 *
 * \details 
 * 0: Disabled
 * 1: Enabled
 *
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_RX_CONFIG . RX_SSF_LOS_ENA
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_SSF_LOS_ENA(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_SSF_LOS_ENA     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_SSF_LOS_ENA(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * SSF signal detect polarity
 *
 * \details 
 * 0: Must be 0 to indicate a signal detection
 * 1: Must be 1 to indicate a signal detection
 *
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_RX_CONFIG . RX_SSF_SD_POL
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_SSF_SD_POL(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_SSF_SD_POL     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_SSF_SD_POL(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * SSF signal detect enable
 *
 * \details 
 * 0: Disabled
 * 1: Enabled
 *
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_RX_CONFIG . RX_SSF_SD_ENA
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_SSF_SD_ENA(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_SSF_SD_ENA     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_SSF_SD_ENA(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Selects bit order for chip core side 64 bit data path
 *
 * \details 
 * Internal 64 bit bus bit order:
 * 0: 0 to 63 (SFI4.2)
 * 3: 63 to 0 (XAUI, TFI5)
 * 1: 32 to 63 followed by 0 to 31 (unused)
 * 2: 31 to 0 followed by 63 to 32 (unused)
 *
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_RX_CONFIG . RX_ENDIAN
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_ENDIAN(x)  VTSS_ENCODE_BITFIELD(x,9,2)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_ENDIAN     VTSS_ENCODE_BITMASK(9,2)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_ENDIAN(x)  VTSS_EXTRACT_BITFIELD(x,9,2)

/** 
 * \brief
 * Selects double serial rate
 *
 * \details 
 * 0: Normal mode
 * 1: Double data rate mode
 *
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_RX_CONFIG . RX_DDR_MODE
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_DDR_MODE(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_DDR_MODE     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_DDR_MODE(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Swaps (inverts) bits per lane
 *
 * \details 
 * 0: No swap
 * 1: Swap
 *
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_RX_CONFIG . RX_PAIR_SWAP
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_PAIR_SWAP(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_PAIR_SWAP     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_PAIR_SWAP(x)  VTSS_EXTRACT_BITFIELD(x,4,4)

/** 
 * \brief
 * Rx software reset
 *
 * \details 
 * 0: Not reset
 * 1: Reset
 *
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_RX_CONFIG . RX_SW_RST
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_SW_RST(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_SW_RST     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_SW_RST(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Swaps lanes 0 <-> 3 and 1 <-> 2
 *
 * \details 
 * 0: No swap
 * 1: Swap
 *
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_RX_CONFIG . RX_LANE_SWAP
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_LANE_SWAP(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_LANE_SWAP     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_LANE_SWAP(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Selects protocol

 *
 * \details 
 * 0,3: XAUI
 * 1: SFI4.2
 * 2: TFI5
 *
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_RX_CONFIG . RX_PROTO_SEL
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_PROTO_SEL(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_PROTO_SEL     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_RX_CONFIG_RX_PROTO_SEL(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Status bits
 *
 * \details
 * Bit encoding for each status field:
 * 0: Status condition present
 * 1: Status condition not present
 *
 * Register: \a UPI::UPI_CONFIG::UPI_STATUS
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_CONFIG_UPI_STATUS(target)  VTSS_IOREG(target,0x3)

/** 
 * \brief
 * Status bits Rx PRBS synced indication (per lane)
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_STATUS . RX_HSS_PRBSSYNC
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_STATUS_RX_HSS_PRBSSYNC(x)  VTSS_ENCODE_BITFIELD(x,28,4)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_STATUS_RX_HSS_PRBSSYNC     VTSS_ENCODE_BITMASK(28,4)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_STATUS_RX_HSS_PRBSSYNC(x)  VTSS_EXTRACT_BITFIELD(x,28,4)

/** 
 * \brief
 * Status bits Tx PRBS synced indication
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_STATUS . TX_HSS_PRBSSYNC
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_STATUS_TX_HSS_PRBSSYNC(x)  VTSS_ENCODE_BITFIELD(x,24,4)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_STATUS_TX_HSS_PRBSSYNC     VTSS_ENCODE_BITMASK(24,4)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_STATUS_TX_HSS_PRBSSYNC(x)  VTSS_EXTRACT_BITFIELD(x,24,4)

/** 
 * \brief
 * Status bit PLL reset output indication
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_STATUS . HSS_RESETOUT
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_STATUS_HSS_RESETOUT(x)  VTSS_ENCODE_BITFIELD(x,22,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_STATUS_HSS_RESETOUT     VTSS_ENCODE_BITMASK(22,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_STATUS_HSS_RESETOUT(x)  VTSS_EXTRACT_BITFIELD(x,22,1)

/** 
 * \brief
 * Status bit from PIN indicates loss of signal
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_STATUS . RX_LOS
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_STATUS_RX_LOS(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_STATUS_RX_LOS     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_STATUS_RX_LOS(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \brief
 * Status bit from Rx SerDes indicates lane signal detects (per lane)
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_STATUS . RX_SIGDET
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_STATUS_RX_SIGDET(x)  VTSS_ENCODE_BITFIELD(x,16,4)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_STATUS_RX_SIGDET     VTSS_ENCODE_BITMASK(16,4)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_STATUS_RX_SIGDET(x)  VTSS_EXTRACT_BITFIELD(x,16,4)

/** 
 * \brief
 * Status bit from the UPI Tx FIFO indicates overflow
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_STATUS . TX_FIFO_OVERFLOW
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_STATUS_TX_FIFO_OVERFLOW(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_STATUS_TX_FIFO_OVERFLOW     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_STATUS_TX_FIFO_OVERFLOW(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Status bit from the UPI Tx FIFO indicates underflow
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_STATUS . TX_FIFO_UNDERFLOW
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_STATUS_TX_FIFO_UNDERFLOW(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_STATUS_TX_FIFO_UNDERFLOW     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_STATUS_TX_FIFO_UNDERFLOW(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Status bit from SerDes indicates the SerDes register access ready state
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_STATUS . HSS_PRTREADY
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_STATUS_HSS_PRTREADY(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_STATUS_HSS_PRTREADY     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_STATUS_HSS_PRTREADY(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Status bits per channel from RX SerDes indicate PRBS error
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_STATUS . RX_PRBS_ERR
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_STATUS_RX_PRBS_ERR(x)  VTSS_ENCODE_BITFIELD(x,8,4)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_STATUS_RX_PRBS_ERR     VTSS_ENCODE_BITMASK(8,4)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_STATUS_RX_PRBS_ERR(x)  VTSS_EXTRACT_BITFIELD(x,8,4)

/** 
 * \brief
 * Status bits per channel from Tx SerDes indicate PRBS error
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_STATUS . TX_PRBS_ERR
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_STATUS_TX_PRBS_ERR(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_STATUS_TX_PRBS_ERR     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_STATUS_TX_PRBS_ERR(x)  VTSS_EXTRACT_BITFIELD(x,4,4)

/** 
 * \brief
 * Status bit UPI indicates SSF (Server Signal Fail)
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_STATUS . SSF
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_STATUS_SSF(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_STATUS_SSF     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_STATUS_SSF(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Status bit from SerDes indicates PLL lock
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_STATUS . PLLLOCK
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_STATUS_PLLLOCK(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_STATUS_PLLLOCK     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_STATUS_PLLLOCK(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Status bit from SerDes indicates an eye quality problem state
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_STATUS . EYE_QUALITY_ERR
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_STATUS_EYE_QUALITY_ERR(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_STATUS_EYE_QUALITY_ERR     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_STATUS_EYE_QUALITY_ERR(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Status bit change detection (sticky) for interrupt generation
 *
 * \details
 * Bit encoding for each interrupt source:
 * 0: No sticky condition change detected
 * 1: Sticky condition change detected
 *
 * Register: \a UPI::UPI_CONFIG::UPI_INTR_STICKY
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_CONFIG_UPI_INTR_STICKY(target)  VTSS_IOREG(target,0x4)

/** 
 * \brief
 * Sticky bits Rx PRBS synced indication (per lane)
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INTR_STICKY . RX_HSS_PRBSSYNC_STICKY
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_HSS_PRBSSYNC_STICKY(x)  VTSS_ENCODE_BITFIELD(x,28,4)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_HSS_PRBSSYNC_STICKY     VTSS_ENCODE_BITMASK(28,4)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_HSS_PRBSSYNC_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,28,4)

/** 
 * \brief
 * Sticky bits Tx PRBS synced indication (per lane)
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INTR_STICKY . TX_HSS_PRBSSYNC_STICKY
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_HSS_PRBSSYNC_STICKY(x)  VTSS_ENCODE_BITFIELD(x,24,4)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_HSS_PRBSSYNC_STICKY     VTSS_ENCODE_BITMASK(24,4)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_HSS_PRBSSYNC_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,24,4)

/** 
 * \brief
 * Sticky bit PLL reset output change indication
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INTR_STICKY . HSS_RESETOUT_STICKY
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_HSS_RESETOUT_STICKY(x)  VTSS_ENCODE_BITFIELD(x,22,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_HSS_RESETOUT_STICKY     VTSS_ENCODE_BITMASK(22,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INTR_STICKY_HSS_RESETOUT_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,22,1)

/** 
 * \brief
 * Sticky bit from PIN indicates a change to loss of signal
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INTR_STICKY . RX_LOS_STICKY
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_LOS_STICKY(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_LOS_STICKY     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_LOS_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \brief
 * Sticky bit from Rx SerDes indicates a change to lane signal detects (per
 * lane)
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INTR_STICKY . RX_SIGDET_STICKY
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_SIGDET_STICKY(x)  VTSS_ENCODE_BITFIELD(x,16,4)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_SIGDET_STICKY     VTSS_ENCODE_BITMASK(16,4)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_SIGDET_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,16,4)

/** 
 * \brief
 * Sticky bit from the UPI Tx FIFO indicates overflow
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INTR_STICKY . TX_FIFO_OVERFLOW_STICKY
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_FIFO_OVERFLOW_STICKY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_FIFO_OVERFLOW_STICKY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_FIFO_OVERFLOW_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Sticky bit from the UPI Tx FIFO indicates underflow
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INTR_STICKY . TX_FIFO_UNDERFLOW_STICKY
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_FIFO_UNDERFLOW_STICKY(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_FIFO_UNDERFLOW_STICKY     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_FIFO_UNDERFLOW_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Sticky bit from SerDes indicates a change to the SerDes register access
 * ready state
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INTR_STICKY . HSS_PRTREADY_STICKY
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_HSS_PRTREADY_STICKY(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_HSS_PRTREADY_STICKY     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INTR_STICKY_HSS_PRTREADY_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Sticky bits per channel from Rx SerDes indicate PRBS error change
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INTR_STICKY . RX_PRBS_ERR_STICKY
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_PRBS_ERR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,8,4)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_PRBS_ERR_STICKY     VTSS_ENCODE_BITMASK(8,4)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INTR_STICKY_RX_PRBS_ERR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,8,4)

/** 
 * \brief
 * Sticky bits per channel from Tx SerDes indicate PRBS error change
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INTR_STICKY . TX_PRBS_ERR_STICKY
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_PRBS_ERR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_PRBS_ERR_STICKY     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INTR_STICKY_TX_PRBS_ERR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,4,4)

/** 
 * \brief
 * Sticky bit UPI indicates SSF change
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INTR_STICKY . SSF_STICKY
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_SSF_STICKY(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_SSF_STICKY     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INTR_STICKY_SSF_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Sticky bit from SerDes indicates PLL lock change
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INTR_STICKY . PLLLOCK_STICKY
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_PLLLOCK_STICKY(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_PLLLOCK_STICKY     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INTR_STICKY_PLLLOCK_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Sticky bit from SerDes indicates an eye quality problem state change
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INTR_STICKY . EYE_QUALITY_ERR_STICKY
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INTR_STICKY_EYE_QUALITY_ERR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INTR_STICKY_EYE_QUALITY_ERR_STICKY     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INTR_STICKY_EYE_QUALITY_ERR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Mask bits for interrupt generation
 *
 * \details
 * Encoding for each interrupt mask bit:
 * 0: Associated sticky condition not enabled to be a source of interrupt
 * 1: Associated sticky condition is enabled to be a source of interrupt
 *
 * Register: \a UPI::UPI_CONFIG::UPI_INTR_MASK
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_CONFIG_UPI_INTR_MASK(target)  VTSS_IOREG(target,0x5)

/** 
 * \brief
 * Mask sticky bits Rx PRBS synced change indication (per lane)
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INTR_MASK . RX_HSS_PRBSSYNC_MASK
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_RX_HSS_PRBSSYNC_MASK(x)  VTSS_ENCODE_BITFIELD(x,28,4)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INTR_MASK_RX_HSS_PRBSSYNC_MASK     VTSS_ENCODE_BITMASK(28,4)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INTR_MASK_RX_HSS_PRBSSYNC_MASK(x)  VTSS_EXTRACT_BITFIELD(x,28,4)

/** 
 * \brief
 * Mask sticky bits Tx PRBS synced change indication (per lane)
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INTR_MASK . TX_HSS_PRBSSYNC_MASK
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_TX_HSS_PRBSSYNC_MASK(x)  VTSS_ENCODE_BITFIELD(x,24,4)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INTR_MASK_TX_HSS_PRBSSYNC_MASK     VTSS_ENCODE_BITMASK(24,4)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INTR_MASK_TX_HSS_PRBSSYNC_MASK(x)  VTSS_EXTRACT_BITFIELD(x,24,4)

/** 
 * \brief
 * Mask sticky bit PLL reset output change indication
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INTR_MASK . HSS_RESETOUT_MASK
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_HSS_RESETOUT_MASK(x)  VTSS_ENCODE_BITFIELD(x,22,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INTR_MASK_HSS_RESETOUT_MASK     VTSS_ENCODE_BITMASK(22,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INTR_MASK_HSS_RESETOUT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,22,1)

/** 
 * \brief
 * Mask sticky bit from PIN indicates a change to loss of signal
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INTR_MASK . RX_LOS_MASK
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_RX_LOS_MASK(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INTR_MASK_RX_LOS_MASK     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INTR_MASK_RX_LOS_MASK(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \brief
 * Mask sticky bits from Rx SerDes indicates a change to lane signal
 * detects (per lane)
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INTR_MASK . RX_SIGDET_MASK
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_RX_SIGDET_MASK(x)  VTSS_ENCODE_BITFIELD(x,16,4)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INTR_MASK_RX_SIGDET_MASK     VTSS_ENCODE_BITMASK(16,4)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INTR_MASK_RX_SIGDET_MASK(x)  VTSS_EXTRACT_BITFIELD(x,16,4)

/** 
 * \brief
 * Mask sticky bit from the UPI Tx FIFO indicates overflow
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INTR_MASK . TX_FIFO_OVERFLOW_MASK
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_TX_FIFO_OVERFLOW_MASK(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INTR_MASK_TX_FIFO_OVERFLOW_MASK     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INTR_MASK_TX_FIFO_OVERFLOW_MASK(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Mask sticky bit from the UPI Tx FIFO indicates underflow
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INTR_MASK . TX_FIFO_UNDERFLOW_MASK
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_TX_FIFO_UNDERFLOW_MASK(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INTR_MASK_TX_FIFO_UNDERFLOW_MASK     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INTR_MASK_TX_FIFO_UNDERFLOW_MASK(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Mask sticky bit from SerDes that indicates the SerDes register access
 * ready state change
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INTR_MASK . HSS_PRTREADY_MASK
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_HSS_PRTREADY_MASK(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INTR_MASK_HSS_PRTREADY_MASK     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INTR_MASK_HSS_PRTREADY_MASK(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Mask sticky bits per channel from Rx SerDes that indicate PRBS error
 * change
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INTR_MASK . RX_PRBS_ERR_MASK
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_RX_PRBS_ERR_MASK(x)  VTSS_ENCODE_BITFIELD(x,8,4)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INTR_MASK_RX_PRBS_ERR_MASK     VTSS_ENCODE_BITMASK(8,4)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INTR_MASK_RX_PRBS_ERR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,8,4)

/** 
 * \brief
 * Mask sticky bits per channel from Tx SerDes that indicate PRBS error
 * change
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INTR_MASK . TX_PRBS_ERR_MASK
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_TX_PRBS_ERR_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INTR_MASK_TX_PRBS_ERR_MASK     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INTR_MASK_TX_PRBS_ERR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,4)

/** 
 * \brief
 * Mask sticky bit UPI that indicates SSF change
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INTR_MASK . SSF_MASK
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_SSF_MASK(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INTR_MASK_SSF_MASK     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INTR_MASK_SSF_MASK(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Mask sticky bit from SerDes that indicates PLL lock change
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INTR_MASK . PLLLOCK_MASK
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_PLLLOCK_MASK(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INTR_MASK_PLLLOCK_MASK     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INTR_MASK_PLLLOCK_MASK(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Mask sticky bit from SerDes that indicates an eye quality problem change
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INTR_MASK . EYE_QUALITY_ERR_MASK
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INTR_MASK_EYE_QUALITY_ERR_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INTR_MASK_EYE_QUALITY_ERR_MASK     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INTR_MASK_EYE_QUALITY_ERR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief UPI loopback configuration
 *
 * \details
 * Configuration register
 *
 * Register: \a UPI::UPI_CONFIG::UPI_LOOPBACK_CONFIG
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_CONFIG_UPI_LOOPBACK_CONFIG(target)  VTSS_IOREG(target,0x6)

/** 
 * \brief
 * Select loopback mode
 *
 * \details 
 * 0: No loopback
 * 1: Loopback
 *
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_LOOPBACK_CONFIG . TX_TO_RX_LB
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_LOOPBACK_CONFIG_TX_TO_RX_LB(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_LOOPBACK_CONFIG_TX_TO_RX_LB     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_LOOPBACK_CONFIG_TX_TO_RX_LB(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Select loopback mode
 *
 * \details 
 * 0: No loopback
 * 1: Loopback
 *
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_LOOPBACK_CONFIG . RX_TO_TX_LB
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_LOOPBACK_CONFIG_RX_TO_TX_LB(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_LOOPBACK_CONFIG_RX_TO_TX_LB     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_LOOPBACK_CONFIG_RX_TO_TX_LB(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief UPI loopback status
 *
 * \details
 * Status register
 *
 * Register: \a UPI::UPI_CONFIG::UPI_LB_RX_TO_TX_FIFO_ERR
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_CONFIG_UPI_LB_RX_TO_TX_FIFO_ERR(target)  VTSS_IOREG(target,0x7)

/** 
 * \brief
 * Sticky bit indicates FIFO error detected
 *
 * \details 
 * 0: No loopback error
 * 1: Loopback error
 *
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_LB_RX_TO_TX_FIFO_ERR . RX_TO_TX_LB_ERR
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_LB_RX_TO_TX_FIFO_ERR_RX_TO_TX_LB_ERR(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_LB_RX_TO_TX_FIFO_ERR_RX_TO_TX_LB_ERR     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_LB_RX_TO_TX_FIFO_ERR_RX_TO_TX_LB_ERR(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief Address for indirect access to HSS6G
 *
 * \details
 * Register: \a UPI::UPI_CONFIG::UPI_INDIR_HSS6G_ADDR
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_ADDR(target)  VTSS_IOREG(target,0x8)

/** 
 * \brief
 * Indirect address sent to HSS6G
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_ADDR . IND_ADDR
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_ADDR_IND_ADDR(x)  VTSS_ENCODE_BITFIELD(x,0,9)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_ADDR_IND_ADDR     VTSS_ENCODE_BITMASK(0,9)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_ADDR_IND_ADDR(x)  VTSS_EXTRACT_BITFIELD(x,0,9)


/** 
 * \brief Write data from HSS6G indirect access
 *
 * \details
 * Register: \a UPI::UPI_CONFIG::UPI_INDIR_HSS6G_WDATA
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_WDATA(target)  VTSS_IOREG(target,0x9)

/** 
 * \brief
 * Write data to send to HSS6G
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_WDATA . WDATA
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_WDATA_WDATA(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_WDATA_WDATA     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_WDATA_WDATA(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Read data from HSS6G indirect access
 *
 * \details
 * Register: \a UPI::UPI_CONFIG::UPI_INDIR_HSS6G_RDATA
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_RDATA(target)  VTSS_IOREG(target,0xa)

/** 
 * \brief
 * Read data returned from HSS6G
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_RDATA . RDATA
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_RDATA_RDATA(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_RDATA_RDATA     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_RDATA_RDATA(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Command for HSS6G indirect access
 *
 * \details
 * Register: \a UPI::UPI_CONFIG::UPI_INDIR_HSS6G_CMD
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_CMD(target)  VTSS_IOREG(target,0xb)

/** 
 * \details 
 * 0: Read
 * 1: Write
 *
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_CMD . WRITE_CMD
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_CMD_WRITE_CMD(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_CMD_WRITE_CMD     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_CMD_WRITE_CMD(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Start an indirect command access of HSS6G
 *
 * \details 
 * 0: Indirect access not started (or completed)
 * 1: Start an indirect access
 *
 * Field: ::VTSS_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_CMD . START_CMD
 */
#define  VTSS_F_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_CMD_START_CMD(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_CMD_START_CMD     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_UPI_UPI_CONFIG_UPI_INDIR_HSS6G_CMD_START_CMD(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/**
 * Register Group: \a UPI::UPI_HSS6G_CONFIG_STAT
 *
 * UPI HSS6G configuration and status
 */


/** 
 * \brief UPI Tx HSS6G_PRBS configuration and status
 *
 * \details
 * UPI Tx HSS6G_PRBS configuration and status (per lane)
 *
 * Register: \a UPI::UPI_HSS6G_CONFIG_STAT::UPI_TX_HSS6G_PRBS
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS(target)  VTSS_IOREG(target,0x400)

/** 
 * \brief
 * Enable corresponding TX_PRBSERR_CNTR (increment clears TX_PRBSERR)
 *
 * \details 
 * 0: PRBS counter not enabled
 * 1: Enable PRBS counter
 *
 * Field: ::VTSS_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS . TX_PRBSERR_CNTR_EN
 */
#define  VTSS_F_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS_TX_PRBSERR_CNTR_EN(x)  VTSS_ENCODE_BITFIELD(x,24,4)
#define  VTSS_M_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS_TX_PRBSERR_CNTR_EN     VTSS_ENCODE_BITMASK(24,4)
#define  VTSS_X_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS_TX_PRBSERR_CNTR_EN(x)  VTSS_EXTRACT_BITFIELD(x,24,4)

/** 
 * \brief
 * Latched errors status
 *
 * \details 
 * 0: PRBS latch error not present
 * 1: PRBS latch error present
 *
 * Field: ::VTSS_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS . TX_PRBSERR
 */
#define  VTSS_F_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS_TX_PRBSERR(x)  VTSS_ENCODE_BITFIELD(x,20,4)
#define  VTSS_M_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS_TX_PRBSERR     VTSS_ENCODE_BITMASK(20,4)
#define  VTSS_X_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS_TX_PRBSERR(x)  VTSS_EXTRACT_BITFIELD(x,20,4)

/** 
 * \brief
 * Checker in sync status
 *
 * \details 
 * 0: Checker is not in sync
 * 1: Checker is in sync
 *
 * Field: ::VTSS_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS . TX_PRBSSYNC
 */
#define  VTSS_F_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS_TX_PRBSSYNC(x)  VTSS_ENCODE_BITFIELD(x,16,4)
#define  VTSS_M_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS_TX_PRBSSYNC     VTSS_ENCODE_BITMASK(16,4)
#define  VTSS_X_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS_TX_PRBSSYNC(x)  VTSS_EXTRACT_BITFIELD(x,16,4)

/** 
 * \brief
 * Tri-states
 *
 * \details 
 * 0: Tri-stated
 * 1: Not tri-stated
 *
 * Field: ::VTSS_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS . TX_TS
 */
#define  VTSS_F_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS_TX_TS(x)  VTSS_ENCODE_BITFIELD(x,12,4)
#define  VTSS_M_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS_TX_TS     VTSS_ENCODE_BITMASK(12,4)
#define  VTSS_X_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS_TX_TS(x)  VTSS_EXTRACT_BITFIELD(x,12,4)

/** 
 * \brief
 * Resets PRBS
 *
 * \details 
 * 0: PRBS not reset
 * 1: PRBS reset
 *
 * Field: ::VTSS_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS . TX_PRBS_RST
 */
#define  VTSS_F_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS_TX_PRBS_RST(x)  VTSS_ENCODE_BITFIELD(x,8,4)
#define  VTSS_M_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS_TX_PRBS_RST     VTSS_ENCODE_BITMASK(8,4)
#define  VTSS_X_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS_TX_PRBS_RST(x)  VTSS_EXTRACT_BITFIELD(x,8,4)

/** 
 * \brief
 * Enables PRBS
 *
 * \details 
 * 0: PRBS not enabled
 * 1: PRBS enabled
 *
 * Field: ::VTSS_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS . TX_PRBS_EN
 */
#define  VTSS_F_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS_TX_PRBS_EN(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS_TX_PRBS_EN     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_UPI_UPI_HSS6G_CONFIG_STAT_UPI_TX_HSS6G_PRBS_TX_PRBS_EN(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief UPI Rx HSS6G_PRBS configuration and status
 *
 * \details
 * UPI Rx HSS6G_PRBS configuration and status (per lane)
 *
 * Register: \a UPI::UPI_HSS6G_CONFIG_STAT::UPI_RX_HSS6G_PRBS
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS(target)  VTSS_IOREG(target,0x401)

/** 
 * \brief
 * Data synchronization
 * Cause 1 bit to be discarded from recovered data
 *
 * \details 
 * 0: Normal operation
 * 1: Discard 1 bit
 *
 * Field: ::VTSS_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS . RX_DATASYNC
 */
#define  VTSS_F_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS_RX_DATASYNC(x)  VTSS_ENCODE_BITFIELD(x,28,4)
#define  VTSS_M_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS_RX_DATASYNC     VTSS_ENCODE_BITMASK(28,4)
#define  VTSS_X_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS_RX_DATASYNC(x)  VTSS_EXTRACT_BITFIELD(x,28,4)

/** 
 * \brief
 * Enable corresponding RX_PRBSERR_CNTR (increment clears RX_PRBSERR)
 *
 * \details 
 * 0: PRBS counter not enabled
 * 1: enable PRBS counter
 *
 * Field: ::VTSS_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS . RX_PRBSERR_CNTR_EN
 */
#define  VTSS_F_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS_RX_PRBSERR_CNTR_EN(x)  VTSS_ENCODE_BITFIELD(x,24,4)
#define  VTSS_M_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS_RX_PRBSERR_CNTR_EN     VTSS_ENCODE_BITMASK(24,4)
#define  VTSS_X_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS_RX_PRBSERR_CNTR_EN(x)  VTSS_EXTRACT_BITFIELD(x,24,4)

/** 
 * \brief
 * Latched errors status
 *
 * \details 
 * 0: PRBS latch error not present
 * 1: PRBS latch error present
 *
 * Field: ::VTSS_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS . RX_PRBSERR
 */
#define  VTSS_F_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS_RX_PRBSERR(x)  VTSS_ENCODE_BITFIELD(x,20,4)
#define  VTSS_M_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS_RX_PRBSERR     VTSS_ENCODE_BITMASK(20,4)
#define  VTSS_X_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS_RX_PRBSERR(x)  VTSS_EXTRACT_BITFIELD(x,20,4)

/** 
 * \brief
 * Checker in sync status
 *
 * \details 
 * 0: checker is not in sync
 * 1: checker is in sync
 *
 * Field: ::VTSS_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS . RX_PRBSSYNC
 */
#define  VTSS_F_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS_RX_PRBSSYNC(x)  VTSS_ENCODE_BITFIELD(x,16,4)
#define  VTSS_M_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS_RX_PRBSSYNC     VTSS_ENCODE_BITMASK(16,4)
#define  VTSS_X_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS_RX_PRBSSYNC(x)  VTSS_EXTRACT_BITFIELD(x,16,4)

/** 
 * \brief
 * PRBS force error
 *
 * \details 
 * 0: PRBS not forcing error
 * 1: PRBS forcing error
 *
 * Field: ::VTSS_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS . RX_PRBSFRCERR
 */
#define  VTSS_F_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS_RX_PRBSFRCERR(x)  VTSS_ENCODE_BITFIELD(x,12,4)
#define  VTSS_M_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS_RX_PRBSFRCERR     VTSS_ENCODE_BITMASK(12,4)
#define  VTSS_X_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS_RX_PRBSFRCERR(x)  VTSS_EXTRACT_BITFIELD(x,12,4)

/** 
 * \brief
 * Resets PRBS
 *
 * \details 
 * 0: PRBS not reset
 * 1: PRBS reset
 *
 * Field: ::VTSS_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS . RX_PRBS_RST
 */
#define  VTSS_F_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS_RX_PRBS_RST(x)  VTSS_ENCODE_BITFIELD(x,8,4)
#define  VTSS_M_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS_RX_PRBS_RST     VTSS_ENCODE_BITMASK(8,4)
#define  VTSS_X_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS_RX_PRBS_RST(x)  VTSS_EXTRACT_BITFIELD(x,8,4)

/** 
 * \brief
 * Enables PRBS
 *
 * \details 
 * 0: PRBS not enabled
 * 1: PRBS enabled
 *
 * Field: ::VTSS_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS . RX_PRBS_EN
 */
#define  VTSS_F_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS_RX_PRBS_EN(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS_RX_PRBS_EN     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_UPI_UPI_HSS6G_CONFIG_STAT_UPI_RX_HSS6G_PRBS_RX_PRBS_EN(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief Tx PRBS error counter
 *
 * \details
 * Approximate counter for Tx PRBS errors (misses errors which are too
 * closely bunched)
 *
 * Register: \a UPI::UPI_HSS6G_CONFIG_STAT::TX_PRBSERR_CNTR
 *
 * @param target A \a ::vtss_target_UPI_e target
 * @param ri Register: TX_PRBSERR_CNTR (??), 0-3
 */
#define VTSS_UPI_UPI_HSS6G_CONFIG_STAT_TX_PRBSERR_CNTR(target,ri)  VTSS_IOREG(target,0x402 + (ri))

/** 
 * \brief
 * Tx PRBS error counter
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_HSS6G_CONFIG_STAT_TX_PRBSERR_CNTR . TX_PRBSERR_CNTR
 */
#define  VTSS_F_UPI_UPI_HSS6G_CONFIG_STAT_TX_PRBSERR_CNTR_TX_PRBSERR_CNTR(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_UPI_UPI_HSS6G_CONFIG_STAT_TX_PRBSERR_CNTR_TX_PRBSERR_CNTR     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_UPI_UPI_HSS6G_CONFIG_STAT_TX_PRBSERR_CNTR_TX_PRBSERR_CNTR(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Rx PRBS error counter
 *
 * \details
 * Approximate counter for Rx PRBS errors (misses errors which are too
 * closely bunched).
 *
 * Register: \a UPI::UPI_HSS6G_CONFIG_STAT::RX_PRBSERR_CNTR
 *
 * @param target A \a ::vtss_target_UPI_e target
 * @param ri Register: RX_PRBSERR_CNTR (??), 0-3
 */
#define VTSS_UPI_UPI_HSS6G_CONFIG_STAT_RX_PRBSERR_CNTR(target,ri)  VTSS_IOREG(target,0x406 + (ri))

/** 
 * \brief
 * Rx PRBS error counter
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_HSS6G_CONFIG_STAT_RX_PRBSERR_CNTR . RX_PRBSERR_CNTR
 */
#define  VTSS_F_UPI_UPI_HSS6G_CONFIG_STAT_RX_PRBSERR_CNTR_RX_PRBSERR_CNTR(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_UPI_UPI_HSS6G_CONFIG_STAT_RX_PRBSERR_CNTR_RX_PRBSERR_CNTR     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_UPI_UPI_HSS6G_CONFIG_STAT_RX_PRBSERR_CNTR_RX_PRBSERR_CNTR(x)  VTSS_EXTRACT_BITFIELD(x,0,32)

/**
 * Register Group: \a UPI::UPI_HSS6G_PLL_RST_CONFIG_STAT
 *
 * UPI HSS6G PLL configuration and status
 */


/** 
 * \brief UPI HSS6G_PLL configuration and status
 *
 * \details
 * UPI HSS6G_PLL configuration
 *
 * Register: \a UPI::UPI_HSS6G_PLL_RST_CONFIG_STAT::UPI_HSS6G_PLL
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL(target)  VTSS_IOREG(target,0x800)

/** 
 * \brief
 * High VTT selector
 * Set to 0
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL . VTTEN
 */
#define  VTSS_F_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_VTTEN(x)  VTSS_ENCODE_BITFIELD(x,17,1)
#define  VTSS_M_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_VTTEN     VTSS_ENCODE_BITMASK(17,1)
#define  VTSS_X_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_VTTEN(x)  VTSS_EXTRACT_BITFIELD(x,17,1)

/** 
 * \brief
 * AC mode selector
 *
 * \details 
 * 0: DC coupling
 * 1: AC coupling 
 *
 * Field: ::VTSS_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL . ACMODE
 */
#define  VTSS_F_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_ACMODE(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_ACMODE     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_ACMODE(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Reset configuration
 *
 * \details 
 * Bit 2:
 * 0: Normal operation
 * 1: Reset over-ride used during manufacturing test
 * 
 * Bits 1:0:
 * 00: normal
 * 01: RXA master, RXB slave
 * 10: RXA slave, RXB master
 * 11: Unused
 *
 * Field: ::VTSS_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL . RSTCFG
 */
#define  VTSS_F_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_RSTCFG(x)  VTSS_ENCODE_BITFIELD(x,12,3)
#define  VTSS_M_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_RSTCFG     VTSS_ENCODE_BITMASK(12,3)
#define  VTSS_X_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_RSTCFG(x)  VTSS_EXTRACT_BITFIELD(x,12,3)

/** 
 * \brief
 * PRBS enable
 * This signal provides a single point of control over the disabling of
 * loopback tests
 *
 * \details 
 * 1: Normal operation 
 * 0: Gate off the enable to the PRBS checker of all ports (Tx and Rx)
 *
 * Field: ::VTSS_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL . PRBSEN
 */
#define  VTSS_F_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_PRBSEN(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_PRBSEN     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_PRBSEN(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * PLL high VRT selector
 *
 * \details 
 * 1: AVTR > 1.3 V
 * 0: Otherwise
 *
 * Field: ::VTSS_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL . OV
 */
#define  VTSS_F_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_OV(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_OV     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_OV(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Send resync input signal to HSS6G
 *
 * \details 
 * 1: Send RESYNC input 
 * 0: Don't send RESYNC input
 *
 * Field: ::VTSS_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL . RESYNC_INPUT
 */
#define  VTSS_F_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_RESYNC_INPUT(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_RESYNC_INPUT     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_RESYNC_INPUT(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * PLL low frequency range
 *
 * \details 
 * 0: Full frequency clock sent to links
 * 1: Two thirds rate clock sent to links
 *
 * Field: ::VTSS_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL . LOFREQ
 */
#define  VTSS_F_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_LOFREQ(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_LOFREQ     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_LOFREQ(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * HSS complete core reset
 *
 * \details 
 * 0: Normal operation
 * 1: Reset
 *
 * Field: ::VTSS_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL . RESET
 */
#define  VTSS_F_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_RESET(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_RESET     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_RESET(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * PLL calibration request
 *
 * \details 
 * 0: Normal operation
 * 1: Restart VCO coarse calibration
 *
 * Field: ::VTSS_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL . RECCAL
 */
#define  VTSS_F_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_RECCAL(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_RECCAL     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_RECCAL(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * PLL power down select
 *
 * \details 
 * 0: Normal operation
 * 1: Power down the HSPLL
 *
 * Field: ::VTSS_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL . PWDNPLL
 */
#define  VTSS_F_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_PWDNPLL(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_PWDNPLL     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_PWDNPLL(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * PLL bypass select
 *
 * \details 
 * 0: Normal operation
 * 1: Bypass the HSPLL clock output with the reference clock
 *
 * Field: ::VTSS_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL . PLLBYP
 */
#define  VTSS_F_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_PLLBYP(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_PLLBYP     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_PLLBYP(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * PLL divide select
 *
 * \details 
 * 00: 8x
 * 01: 10x
 * 10: 16x
 * 11: 20x
 *
 * Field: ::VTSS_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL . DIVSEL
 */
#define  VTSS_F_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_DIVSEL(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_DIVSEL     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_DIVSEL(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief PLL locked status
 *
 * \details
 * Register: \a UPI::UPI_HSS6G_PLL_RST_CONFIG_STAT::UPI_HSS6G_PLL_LOCK
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_LOCK(target)  VTSS_IOREG(target,0x801)

/** 
 * \brief
 * PLL reset output indication
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_LOCK . RESETOUT
 */
#define  VTSS_F_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_LOCK_RESETOUT(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_LOCK_RESETOUT     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_LOCK_RESETOUT(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * PLL locked indication
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_LOCK . PLLLOCK
 */
#define  VTSS_F_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_LOCK_PLLLOCK(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_LOCK_PLLLOCK     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_UPI_UPI_HSS6G_PLL_RST_CONFIG_STAT_UPI_HSS6G_PLL_LOCK_PLLLOCK(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/**
 * Register Group: \a UPI::UPI_HSS6G_RX_PHASE_ROTATOR
 *
 * UPI HSS6G phase rotator configuration and status
 */


/** 
 * \brief UPI Rx HSS6G phase rotator configuration
 *
 * \details
 * UPI RX HSS6G phase rotator configuration
 *
 * Register: \a UPI::UPI_HSS6G_RX_PHASE_ROTATOR::UPI_HSS6G_RX_PHASE_ROTATOR_CONFIG
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_HSS6G_RX_PHASE_ROTATOR_UPI_HSS6G_RX_PHASE_ROTATOR_CONFIG(target)  VTSS_IOREG(target,0xc00)

/** 
 * \brief
 * Phase rotator early in (step rotator down)
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_HSS6G_RX_PHASE_ROTATOR_UPI_HSS6G_RX_PHASE_ROTATOR_CONFIG . RX_EARLYIN
 */
#define  VTSS_F_UPI_UPI_HSS6G_RX_PHASE_ROTATOR_UPI_HSS6G_RX_PHASE_ROTATOR_CONFIG_RX_EARLYIN(x)  VTSS_ENCODE_BITFIELD(x,20,4)
#define  VTSS_M_UPI_UPI_HSS6G_RX_PHASE_ROTATOR_UPI_HSS6G_RX_PHASE_ROTATOR_CONFIG_RX_EARLYIN     VTSS_ENCODE_BITMASK(20,4)
#define  VTSS_X_UPI_UPI_HSS6G_RX_PHASE_ROTATOR_UPI_HSS6G_RX_PHASE_ROTATOR_CONFIG_RX_EARLYIN(x)  VTSS_EXTRACT_BITFIELD(x,20,4)

/** 
 * \brief
 * Phase rotator late in (step rotator up)
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_HSS6G_RX_PHASE_ROTATOR_UPI_HSS6G_RX_PHASE_ROTATOR_CONFIG . RX_LATEIN
 */
#define  VTSS_F_UPI_UPI_HSS6G_RX_PHASE_ROTATOR_UPI_HSS6G_RX_PHASE_ROTATOR_CONFIG_RX_LATEIN(x)  VTSS_ENCODE_BITFIELD(x,16,4)
#define  VTSS_M_UPI_UPI_HSS6G_RX_PHASE_ROTATOR_UPI_HSS6G_RX_PHASE_ROTATOR_CONFIG_RX_LATEIN     VTSS_ENCODE_BITMASK(16,4)
#define  VTSS_X_UPI_UPI_HSS6G_RX_PHASE_ROTATOR_UPI_HSS6G_RX_PHASE_ROTATOR_CONFIG_RX_LATEIN(x)  VTSS_EXTRACT_BITFIELD(x,16,4)

/** 
 * \brief
 * Phase rotator down in (retard by 1 increment)
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_HSS6G_RX_PHASE_ROTATOR_UPI_HSS6G_RX_PHASE_ROTATOR_CONFIG . RX_PHSDNIN
 */
#define  VTSS_F_UPI_UPI_HSS6G_RX_PHASE_ROTATOR_UPI_HSS6G_RX_PHASE_ROTATOR_CONFIG_RX_PHSDNIN(x)  VTSS_ENCODE_BITFIELD(x,12,4)
#define  VTSS_M_UPI_UPI_HSS6G_RX_PHASE_ROTATOR_UPI_HSS6G_RX_PHASE_ROTATOR_CONFIG_RX_PHSDNIN     VTSS_ENCODE_BITMASK(12,4)
#define  VTSS_X_UPI_UPI_HSS6G_RX_PHASE_ROTATOR_UPI_HSS6G_RX_PHASE_ROTATOR_CONFIG_RX_PHSDNIN(x)  VTSS_EXTRACT_BITFIELD(x,12,4)

/** 
 * \brief
 * Phase rotator up in (advance by 1 increment)
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_HSS6G_RX_PHASE_ROTATOR_UPI_HSS6G_RX_PHASE_ROTATOR_CONFIG . RX_PHSUPIN
 */
#define  VTSS_F_UPI_UPI_HSS6G_RX_PHASE_ROTATOR_UPI_HSS6G_RX_PHASE_ROTATOR_CONFIG_RX_PHSUPIN(x)  VTSS_ENCODE_BITFIELD(x,8,4)
#define  VTSS_M_UPI_UPI_HSS6G_RX_PHASE_ROTATOR_UPI_HSS6G_RX_PHASE_ROTATOR_CONFIG_RX_PHSUPIN     VTSS_ENCODE_BITMASK(8,4)
#define  VTSS_X_UPI_UPI_HSS6G_RX_PHASE_ROTATOR_UPI_HSS6G_RX_PHASE_ROTATOR_CONFIG_RX_PHSUPIN(x)  VTSS_EXTRACT_BITFIELD(x,8,4)

/** 
 * \brief
 * Phase rotator lock adjustment enable
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_HSS6G_RX_PHASE_ROTATOR_UPI_HSS6G_RX_PHASE_ROTATOR_CONFIG . RX_PHSLOCK
 */
#define  VTSS_F_UPI_UPI_HSS6G_RX_PHASE_ROTATOR_UPI_HSS6G_RX_PHASE_ROTATOR_CONFIG_RX_PHSLOCK(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_UPI_UPI_HSS6G_RX_PHASE_ROTATOR_UPI_HSS6G_RX_PHASE_ROTATOR_CONFIG_RX_PHSLOCK     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_UPI_UPI_HSS6G_RX_PHASE_ROTATOR_UPI_HSS6G_RX_PHASE_ROTATOR_CONFIG_RX_PHSLOCK(x)  VTSS_EXTRACT_BITFIELD(x,0,4)

/**
 * Register Group: \a UPI::TESTPAT
 *
 * Test pattern buffer
 */


/** 
 * \brief Test pattern control
 *
 * \details
 * Register: \a UPI::TESTPAT::TESTPAT_MODE
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_TESTPAT_TESTPAT_MODE(target)  VTSS_IOREG(target,0x1000)

/** 
 * \brief
 * When this bit is asserted, values in the TESTPAT_ERRORS and the
 * TESTPAT_CYCS are 'snapshot' at their current values and remain so until
 * this bit is deasserted. Note that the counters continue to accumulate
 * Errors and Cycles while this bit is set, though the CSR Read-Values are
 * 'latched'.
 *
 * \details 
 * 0: Counter values are updated at the completion of each Pattern-Cycle
 * (default);
 * 1: Counters values are 'Snapshot' at the assertion of this bit.
 *
 * Field: ::VTSS_UPI_TESTPAT_TESTPAT_MODE . TP_SNAPSHOT
 */
#define  VTSS_F_UPI_TESTPAT_TESTPAT_MODE_TP_SNAPSHOT(x)  VTSS_ENCODE_BITFIELD(x,28,4)
#define  VTSS_M_UPI_TESTPAT_TESTPAT_MODE_TP_SNAPSHOT     VTSS_ENCODE_BITMASK(28,4)
#define  VTSS_X_UPI_TESTPAT_TESTPAT_MODE_TP_SNAPSHOT(x)  VTSS_EXTRACT_BITFIELD(x,28,4)

/** 
 * \brief
 * Encoded 'wave_gen' word width.
 *
 * \details 
 * 00: 8-bit words
 * 01: 10-bit words
 * 10: 16-bit words
 * 11: 20-bit words
 *
 * Field: ::VTSS_UPI_TESTPAT_TESTPAT_MODE . TP_WDWIDTH
 */
#define  VTSS_F_UPI_TESTPAT_TESTPAT_MODE_TP_WDWIDTH(x)  VTSS_ENCODE_BITFIELD(x,13,2)
#define  VTSS_M_UPI_TESTPAT_TESTPAT_MODE_TP_WDWIDTH     VTSS_ENCODE_BITMASK(13,2)
#define  VTSS_X_UPI_TESTPAT_TESTPAT_MODE_TP_WDWIDTH(x)  VTSS_EXTRACT_BITFIELD(x,13,2)

/** 
 * \brief
 * Reset the TESTPAT_ERRORS counter
 *
 * \details 
 * Field: ::VTSS_UPI_TESTPAT_TESTPAT_MODE . TPERR_RST
 */
#define  VTSS_F_UPI_TESTPAT_TESTPAT_MODE_TPERR_RST(x)  VTSS_ENCODE_BITFIELD(x,9,4)
#define  VTSS_M_UPI_TESTPAT_TESTPAT_MODE_TPERR_RST     VTSS_ENCODE_BITMASK(9,4)
#define  VTSS_X_UPI_TESTPAT_TESTPAT_MODE_TPERR_RST(x)  VTSS_EXTRACT_BITFIELD(x,9,4)

/** 
 * \brief
 * Test pattern checker scan mode. 
 * When set, if the number of errors (mismatched bits) detected in one
 * complete TP_BITLENGTH scan equals or exceeds the TP_THRESHOLD, the
 * pattern checker auto-skips one bit position (retarding the check phase
 * by one bit clock) and compares the new pattern to the input. The
 * bit-error counter (TESTPAT_ERRORS) is also reset at the beginning of the
 * new scan cycle. This continues until the number of errors detected is
 * less than or equal to TP_THRESHOLD at which point the scan mode (this
 * bit) switches to locked (deasserted). When deasserted, the pattern
 * checker scan mode is disabled.
 *
 * \details 
 * 0: Lock pattern detection to its current position
 * 1: Scan mode, skip a bit position at the beginning of a pattern sequence
 * whenever the number of bit errors detected equal or exceed TP_THRESHOLD.
 * This bit is auto-cleared when the number of errors in one cycle is less
 * than or equal to TP_THRESHOLD.
 *
 * Field: ::VTSS_UPI_TESTPAT_TESTPAT_MODE . TPCHK_SCAN
 */
#define  VTSS_F_UPI_TESTPAT_TESTPAT_MODE_TPCHK_SCAN(x)  VTSS_ENCODE_BITFIELD(x,5,4)
#define  VTSS_M_UPI_TESTPAT_TESTPAT_MODE_TPCHK_SCAN     VTSS_ENCODE_BITMASK(5,4)
#define  VTSS_X_UPI_TESTPAT_TESTPAT_MODE_TPCHK_SCAN(x)  VTSS_EXTRACT_BITFIELD(x,5,4)

/** 
 * \brief
 * Enable test-pattern checker
 *
 * \details 
 * Field: ::VTSS_UPI_TESTPAT_TESTPAT_MODE . TPCHK_ENB
 */
#define  VTSS_F_UPI_TESTPAT_TESTPAT_MODE_TPCHK_ENB(x)  VTSS_ENCODE_BITFIELD(x,1,4)
#define  VTSS_M_UPI_TESTPAT_TESTPAT_MODE_TPCHK_ENB     VTSS_ENCODE_BITMASK(1,4)
#define  VTSS_X_UPI_TESTPAT_TESTPAT_MODE_TPCHK_ENB(x)  VTSS_EXTRACT_BITFIELD(x,1,4)

/** 
 * \brief
 * Enable test-pattern generator
 *
 * \details 
 * Field: ::VTSS_UPI_TESTPAT_TESTPAT_MODE . TPGEN_ENB
 */
#define  VTSS_F_UPI_TESTPAT_TESTPAT_MODE_TPGEN_ENB(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_UPI_TESTPAT_TESTPAT_MODE_TPGEN_ENB     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_UPI_TESTPAT_TESTPAT_MODE_TPGEN_ENB(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Bit-length of test pattern
 *
 * \details
 * Register: \a UPI::TESTPAT::TP_BITLENGTH
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_TESTPAT_TP_BITLENGTH(target)  VTSS_IOREG(target,0x1001)

/** 
 * \brief
 * Bit-length of test pattern
 *
 * \details 
 * 0: 8K
 * else: Direct encoding (in bits) of pattern length
 *
 * Field: ::VTSS_UPI_TESTPAT_TP_BITLENGTH . TP_BITLENGTH
 */
#define  VTSS_F_UPI_TESTPAT_TP_BITLENGTH_TP_BITLENGTH(x)  VTSS_ENCODE_BITFIELD(x,0,14)
#define  VTSS_M_UPI_TESTPAT_TP_BITLENGTH_TP_BITLENGTH     VTSS_ENCODE_BITMASK(0,14)
#define  VTSS_X_UPI_TESTPAT_TP_BITLENGTH_TP_BITLENGTH(x)  VTSS_EXTRACT_BITFIELD(x,0,14)


/** 
 * \brief Pattern checker threshold
 *
 * \details
 * Register: \a UPI::TESTPAT::TESTPAT_THRESHOLD
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_TESTPAT_TESTPAT_THRESHOLD(target)  VTSS_IOREG(target,0x1002)

/** 
 * \brief
 * Test pattern threshold
 *
 * \details 
 * Field: ::VTSS_UPI_TESTPAT_TESTPAT_THRESHOLD . TP_THRESHOLD
 */
#define  VTSS_F_UPI_TESTPAT_TESTPAT_THRESHOLD_TP_THRESHOLD(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_UPI_TESTPAT_TESTPAT_THRESHOLD_TP_THRESHOLD     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_UPI_TESTPAT_TESTPAT_THRESHOLD_TP_THRESHOLD(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Pattern checker errors
 *
 * \details
 * Total number of bit errors detected since last TPERR_RST issued. The
 * TPCHK_ENB must be asserted for this counter to operate. If this register
 * value reaches full scale (0xffffffff), further counting of errors is
 * disabled until a TPERR_RST is issued.
 *
 * Register: \a UPI::TESTPAT::TESTPAT_ERRORS
 *
 * @param target A \a ::vtss_target_UPI_e target
 * @param ri Register: TESTPAT_ERRORS (??), 0-3
 */
#define VTSS_UPI_TESTPAT_TESTPAT_ERRORS(target,ri)  VTSS_IOREG(target,0x1003 + (ri))

/** 
 * \brief
 * Test pattern bit errors
 *
 * \details 
 * Field: ::VTSS_UPI_TESTPAT_TESTPAT_ERRORS . TP_ERRORS
 */
#define  VTSS_F_UPI_TESTPAT_TESTPAT_ERRORS_TP_ERRORS(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_UPI_TESTPAT_TESTPAT_ERRORS_TP_ERRORS     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_UPI_TESTPAT_TESTPAT_ERRORS_TP_ERRORS(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Pattern cycles
 *
 * \details
 * Total number of the specified TP_BITLENGTH cycles completed since the
 * last TPERR_RST was issued or TPCHK_SCAN was deasserted (lock achieved).
 * Multiply the pattern bit-length (TESTPAT_CYCS) by that value
 * (TP_BITLENGTH) to determine the total number of bits received.
 *
 * Register: \a UPI::TESTPAT::TESTPAT_CYCS
 *
 * @param target A \a ::vtss_target_UPI_e target
 * @param ri Register: TESTPAT_CYCS (??), 0-3
 */
#define VTSS_UPI_TESTPAT_TESTPAT_CYCS(target,ri)  VTSS_IOREG(target,0x1007 + (ri))

/** 
 * \brief
 * Test pattern cycles
 *
 * \details 
 * Field: ::VTSS_UPI_TESTPAT_TESTPAT_CYCS . TP_CYCLES
 */
#define  VTSS_F_UPI_TESTPAT_TESTPAT_CYCS_TP_CYCLES(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_UPI_TESTPAT_TESTPAT_CYCS_TP_CYCLES     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_UPI_TESTPAT_TESTPAT_CYCS_TP_CYCLES(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Pattern generation/checker buffer memory
 *
 * \details
 * 64 32-bit words used as the test pattern source-buffer/check-buffer 
 *
 * Register: \a UPI::TESTPAT::PATTERN_BUFFER
 *
 * @param target A \a ::vtss_target_UPI_e target
 * @param ri Register: PATTERN_BUFFER (??), 0-255
 */
#define VTSS_UPI_TESTPAT_PATTERN_BUFFER(target,ri)  VTSS_IOREG(target,0x100b + (ri))

/** 
 * \brief
 * Only the number of bits specified by the TP_BITLENGTH are used
 *
 * \details 
 * Field: ::VTSS_UPI_TESTPAT_PATTERN_BUFFER . PATTERN_BUFFER
 */
#define  VTSS_F_UPI_TESTPAT_PATTERN_BUFFER_PATTERN_BUFFER(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_UPI_TESTPAT_PATTERN_BUFFER_PATTERN_BUFFER     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_UPI_TESTPAT_PATTERN_BUFFER_PATTERN_BUFFER(x)  VTSS_EXTRACT_BITFIELD(x,0,32)

/**
 * Register Group: \a UPI::UPI_SPARE
 *
 * Not documented
 */

/**
 * Register Group: \a UPI::UPI_RX_OTN_FRMR
 *
 * UPI Rx OTN framer configuration and status
 */


/** 
 * \brief UPI Rx OTN framer enable
 *
 * \details
 * UPI Rx OTN framer enable
 *
 * Register: \a UPI::UPI_RX_OTN_FRMR::UPI_RX_OTN_FRMR_ENA
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_ENA(target)  VTSS_IOREG(target,0x1800)

/** 
 * \brief
 * Enables the UPI Rx OTN Framer
 *
 * \details 
 * 0: Framer is bypassed (monitoring only)
 * 1: Framer is enabled to be the source of downstream data
 *
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_ENA . FRMR_ENA
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_ENA_FRMR_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_ENA_FRMR_ENA     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_ENA_FRMR_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief UPI Rx OTN frame alignment control
 *
 * \details
 * Frame alignment control
 *
 * Register: \a UPI::UPI_RX_OTN_FRMR::UPI_RX_FA_CTRL1
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL1(target)  VTSS_IOREG(target,0x1801)

/** 
 * \brief
 * Forces the OOF state
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL1 . FORCE_OOF
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL1_FORCE_OOF(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL1_FORCE_OOF     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL1_FORCE_OOF(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Forces the LOF state
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL1 . FORCE_LOF
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL1_FORCE_LOF(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL1_FORCE_LOF     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL1_FORCE_LOF(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Number of frames (time in the OOF state) to enter the LOF state. 
 * Default is 3 ms.
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL1 . COUNT_TO_ENTER_LOF
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL1_COUNT_TO_ENTER_LOF(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL1_COUNT_TO_ENTER_LOF     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL1_COUNT_TO_ENTER_LOF(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \brief UPI Rx OTN frame alignment control 2
 *
 * \details
 * Frame alignment control
 *
 * Register: \a UPI::UPI_RX_OTN_FRMR::UPI_RX_FA_CTRL2
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL2(target)  VTSS_IOREG(target,0x1802)

/** 
 * \brief
 * Number of consecutive frames to enter the OOF state (from the IF state).
 * 
 * Default is 5 frames.
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL2 . COUNT_TO_ENTER_OOF
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL2_COUNT_TO_ENTER_OOF(x)  VTSS_ENCODE_BITFIELD(x,12,4)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL2_COUNT_TO_ENTER_OOF     VTSS_ENCODE_BITMASK(12,4)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL2_COUNT_TO_ENTER_OOF(x)  VTSS_EXTRACT_BITFIELD(x,12,4)

/** 
 * \brief
 * Number of frames (continuous time in the IF state) to exit the LOF
 * state.
 * Default is 3 ms.
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL2 . COUNT_TO_EXIT_LOF
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL2_COUNT_TO_EXIT_LOF(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL2_COUNT_TO_EXIT_LOF     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL2_COUNT_TO_EXIT_LOF(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \brief UPI Rx OTN multiframe alignment control
 *
 * \details
 * Multiframe alignment control
 *
 * Register: \a UPI::UPI_RX_OTN_FRMR::UPI_RX_MFA_CTRL1
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL1(target)  VTSS_IOREG(target,0x1803)

/** 
 * \brief
 * Forces the OOM state
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL1 . FORCE_OOM
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL1_FORCE_OOM(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL1_FORCE_OOM     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL1_FORCE_OOM(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Forces the LOM state
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL1 . FORCE_LOM
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL1_FORCE_LOM(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL1_FORCE_LOM     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL1_FORCE_LOM(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Number of frames (time in the LOM state) to enter the LOM state.
 * Default is 3 ms.
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL1 . COUNT_TO_ENTER_LOM
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL1_COUNT_TO_ENTER_LOM(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL1_COUNT_TO_ENTER_LOM     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL1_COUNT_TO_ENTER_LOM(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \brief UPI Rx OTN multiframe alignment control 2
 *
 * \details
 * Multiframe alignment control
 *
 * Register: \a UPI::UPI_RX_OTN_FRMR::UPI_RX_MFA_CTRL2
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL2(target)  VTSS_IOREG(target,0x1804)

/** 
 * \brief
 * Number of consecutive frames to enter the OOM state (from the IM state).
 * Default is 5 frames.
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL2 . COUNT_TO_ENTER_OOM
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL2_COUNT_TO_ENTER_OOM(x)  VTSS_ENCODE_BITFIELD(x,12,4)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL2_COUNT_TO_ENTER_OOM     VTSS_ENCODE_BITMASK(12,4)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL2_COUNT_TO_ENTER_OOM(x)  VTSS_EXTRACT_BITFIELD(x,12,4)

/** 
 * \brief
 * Number of frames (continuos time in the IM state) to exit the LOM state.
 * Default is 0 ms.
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL2 . COUNT_TO_EXIT_LOM
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL2_COUNT_TO_EXIT_LOM(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL2_COUNT_TO_EXIT_LOM     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL2_COUNT_TO_EXIT_LOM(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief Out of frame counter
 *
 * \details
 * Count the number of times the OOF state was entered
 *
 * Register: \a UPI::UPI_RX_OTN_FRMR::UPI_RX_OOF_CNTR
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OOF_CNTR(target)  VTSS_IOREG(target,0x1805)

/** 
 * \brief
 * OOF counter value (saturates at the maximum value)
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OOF_CNTR . UPI_RX_OOF_CNTR
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_OOF_CNTR_UPI_RX_OOF_CNTR(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OOF_CNTR_UPI_RX_OOF_CNTR     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_OOF_CNTR_UPI_RX_OOF_CNTR(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Out of multiframe counter
 *
 * \details
 * Count the number of times the OOM state was entered
 *
 * Register: \a UPI::UPI_RX_OTN_FRMR::UPI_RX_OOM_CNTR
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OOM_CNTR(target)  VTSS_IOREG(target,0x1806)

/** 
 * \brief
 * OOM counter value (saturates at the maximum value)
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OOM_CNTR . UPI_RX_OOM_CNTR
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_OOM_CNTR_UPI_RX_OOM_CNTR(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OOM_CNTR_UPI_RX_OOM_CNTR     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_OOM_CNTR_UPI_RX_OOM_CNTR(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Status bits
 *
 * \details
 * Bit encoding for each status condition:
 * 0: Status condition not present
 * 1: Status condition present
 *
 * Register: \a UPI::UPI_RX_OTN_FRMR::UPI_RX_OTN_FRMR_STATUS
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS(target)  VTSS_IOREG(target,0x1807)

/** 
 * \brief
 * Status bit from the OTN framer indicates the current LOM state
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS . LOM_STATUS
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS_LOM_STATUS(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS_LOM_STATUS     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS_LOM_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Status bit from the OTN framer indicates the current OOM state
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS . OOM_STATUS
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS_OOM_STATUS(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS_OOM_STATUS     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS_OOM_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Status bit from the OTN framer indicates the current LOF state
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS . LOF_STATUS
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS_LOF_STATUS(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS_LOF_STATUS     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS_LOF_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Status bit from the OTN framer indicates the current OOF state
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS . OOF_STATUS
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS_OOF_STATUS(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS_OOF_STATUS     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS_OOF_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Status bits (sticky) for interrupt generation
 *
 * \details
 * Bit encoding for each interrupt source:
 * 0: No sticky condition detected
 * 1: Sticky condition detected
 *
 * Register: \a UPI::UPI_RX_OTN_FRMR::UPI_RX_OTN_FRMR_STICKY
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY(target)  VTSS_IOREG(target,0x1808)

/** 
 * \brief
 * Sticky bit from the OTN framer indicates a change in the frame alignment
 * position
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY . FAS_POS_STICKY
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_FAS_POS_STICKY(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_FAS_POS_STICKY     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_FAS_POS_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Sticky bit from the OTN framer indicates a change in LOM state
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY . LOM_STICKY
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_LOM_STICKY(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_LOM_STICKY     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_LOM_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Sticky bit from the OTN framer indicates a change in OOM state
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY . OOM_STICKY
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_OOM_STICKY(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_OOM_STICKY     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_OOM_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Sticky bit from the OTN framer indicates a change in LOF state
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY . LOF_STICKY
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_LOF_STICKY(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_LOF_STICKY     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_LOF_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Sticky bit from the OTN framer indicates a change in OOF state
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY . OOF_STICKY
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_OOF_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_OOF_STICKY     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_OOF_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Mask bits for interrupt generation
 *
 * \details
 * Encoding for each interrupt mask bit:
 * 0: Associated sticky condition not enabled to be a source of interrupt
 * 1: Associated sticky condition is enabled to be a source of interrupt
 *
 * Register: \a UPI::UPI_RX_OTN_FRMR::UPI_RX_OTN_FRMR_MASK
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK(target)  VTSS_IOREG(target,0x1809)

/** 
 * \brief
 * Mask sticky bit from the OTN framer that indicates a change in the frame
 * alignment position
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK . FAS_POS_MASK
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_FAS_POS_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_FAS_POS_MASK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_FAS_POS_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Mask sticky bit from the OTN framer that indicates a change in LOM state
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK . LOM_MASK
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_LOM_MASK(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_LOM_MASK     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_LOM_MASK(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Mask sticky bit from the OTN framer that indicates a change in OOM state
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK . OOM_MASK
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_OOM_MASK(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_OOM_MASK     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_OOM_MASK(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Mask sticky bit from the OTN framer that indicates a change in LOF state
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK . LOF_MASK
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_LOF_MASK(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_LOF_MASK     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_LOF_MASK(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Mask sticky bit from the OTN framer that indicates a change in OOF state
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK . OOF_MASK
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_OOF_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_OOF_MASK     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_OOF_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief AIS frame control
 *
 * \details
 * AIS frames start/stop at a subsequent frame boundary in response to the
 * state of:
 *  - FORCE_ODU_AIS
 *  - LOS
 *  - LOF
 *  - LOM (if GEN_AIS_UPON_LOM)
 *
 * Register: \a UPI::UPI_RX_OTN_FRMR::UPI_RX_AIS_CTRL
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_AIS_CTRL(target)  VTSS_IOREG(target,0x180a)

/** 
 * \brief
 * Force all ones in the ODU frame (excluding FA OH)
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_AIS_CTRL . FORCE_ODU_AIS
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_AIS_CTRL_FORCE_ODU_AIS(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_AIS_CTRL_FORCE_ODU_AIS     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_AIS_CTRL_FORCE_ODU_AIS(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Generate ODU AIS upon LOM
 * (AIS is always generated upon LOF or LOS)
 *
 * \details 
 * Field: ::VTSS_UPI_UPI_RX_OTN_FRMR_UPI_RX_AIS_CTRL . GEN_AIS_UPON_LOM
 */
#define  VTSS_F_UPI_UPI_RX_OTN_FRMR_UPI_RX_AIS_CTRL_GEN_AIS_UPON_LOM(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_UPI_UPI_RX_OTN_FRMR_UPI_RX_AIS_CTRL_GEN_AIS_UPON_LOM     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_UPI_UPI_RX_OTN_FRMR_UPI_RX_AIS_CTRL_GEN_AIS_UPON_LOM(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/**
 * Register Group: \a UPI::HSS6G_PLL
 *
 * PLL registers 
 */


/** 
 * \brief HSS6G PLL link enable 0
 *
 * \details
 * The link enable registers are used to enable/disable clocks to the
 * individual links within the core. All bits exist as read/write although
 * not all are needed for the smaller port counts. The bits reset to a 1,
 * which is the enabled state. Access read/write.
 *
 * Register: \a UPI::HSS6G_PLL::HSS6G_LINK_ENABLE_0
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_HSS6G_PLL_HSS6G_LINK_ENABLE_0(target)  VTSS_IOREG(target,0x2806)

/** 
 * \brief
 * Link enable 03
 *
 * \details 
 * Simplex: link enable to Tx- or Rx-D
 * Duplex: Link enable to Rx-D 
 *
 * Field: ::VTSS_UPI_HSS6G_PLL_HSS6G_LINK_ENABLE_0 . LEN03
 */
#define  VTSS_F_UPI_HSS6G_PLL_HSS6G_LINK_ENABLE_0_LEN03(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_UPI_HSS6G_PLL_HSS6G_LINK_ENABLE_0_LEN03     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_UPI_HSS6G_PLL_HSS6G_LINK_ENABLE_0_LEN03(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Link enable 02
 *
 * \details 
 * Simplex: link enable to Tx- or Rx-C
 * Duplex: link enable to Rx-C
 *
 * Field: ::VTSS_UPI_HSS6G_PLL_HSS6G_LINK_ENABLE_0 . LEN02
 */
#define  VTSS_F_UPI_HSS6G_PLL_HSS6G_LINK_ENABLE_0_LEN02(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_UPI_HSS6G_PLL_HSS6G_LINK_ENABLE_0_LEN02     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_UPI_HSS6G_PLL_HSS6G_LINK_ENABLE_0_LEN02(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Link enable 01
 *
 * \details 
 * Simplex: link enable to Tx- or Rx-B
 * Duplex: link enable to Rx-B 
 *
 * Field: ::VTSS_UPI_HSS6G_PLL_HSS6G_LINK_ENABLE_0 . LEN01
 */
#define  VTSS_F_UPI_HSS6G_PLL_HSS6G_LINK_ENABLE_0_LEN01(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_UPI_HSS6G_PLL_HSS6G_LINK_ENABLE_0_LEN01     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_UPI_HSS6G_PLL_HSS6G_LINK_ENABLE_0_LEN01(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Link enable 00
 *
 * \details 
 * Simplex: link enable to Tx- or Rx-A
 * Duplex: Link enable to Rx-A 
 *
 * Field: ::VTSS_UPI_HSS6G_PLL_HSS6G_LINK_ENABLE_0 . LEN00
 */
#define  VTSS_F_UPI_HSS6G_PLL_HSS6G_LINK_ENABLE_0_LEN00(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_UPI_HSS6G_PLL_HSS6G_LINK_ENABLE_0_LEN00     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_UPI_HSS6G_PLL_HSS6G_LINK_ENABLE_0_LEN00(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief HSS6G PLL link reset 0
 *
 * \details
 * The link reset registers are used to reset the individual links within
 * the core. All bits exist as read/write although not all are needed for
 * the smaller port counts. The bits reset to a 0 and are written to a 1 to
 * cause a reset. Access read/write.
 *
 * Register: \a UPI::HSS6G_PLL::HSS6G_LINK_RESET_0
 *
 * @param target A \a ::vtss_target_UPI_e target
 */
#define VTSS_UPI_HSS6G_PLL_HSS6G_LINK_RESET_0(target)  VTSS_IOREG(target,0x2808)

/** 
 * \brief
 * Link reset 03
 *
 * \details 
 * Simplex: Link reset to Tx- or Rx-D
 * Duplex: Link reset to Rx-D
 *
 * Field: ::VTSS_UPI_HSS6G_PLL_HSS6G_LINK_RESET_0 . LRST03
 */
#define  VTSS_F_UPI_HSS6G_PLL_HSS6G_LINK_RESET_0_LRST03(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_UPI_HSS6G_PLL_HSS6G_LINK_RESET_0_LRST03     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_UPI_HSS6G_PLL_HSS6G_LINK_RESET_0_LRST03(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Link reset 02
 *
 * \details 
 * Simplex: Link reset to Tx- or Rx-C
 * Duplex: Link reset to Rx-C
 *
 * Field: ::VTSS_UPI_HSS6G_PLL_HSS6G_LINK_RESET_0 . LRST02
 */
#define  VTSS_F_UPI_HSS6G_PLL_HSS6G_LINK_RESET_0_LRST02(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_UPI_HSS6G_PLL_HSS6G_LINK_RESET_0_LRST02     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_UPI_HSS6G_PLL_HSS6G_LINK_RESET_0_LRST02(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Link reset 01
 *
 * \details 
 * Simplex: Link reset to Tx- or Rx-B
 * Duplex: Link reset to Rx-B
 *
 * Field: ::VTSS_UPI_HSS6G_PLL_HSS6G_LINK_RESET_0 . LRST01
 */
#define  VTSS_F_UPI_HSS6G_PLL_HSS6G_LINK_RESET_0_LRST01(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_UPI_HSS6G_PLL_HSS6G_LINK_RESET_0_LRST01     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_UPI_HSS6G_PLL_HSS6G_LINK_RESET_0_LRST01(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Link reset 00
 *
 * \details 
 * Simplex: Link reset to Tx- or Rx-A
 * Duplex: Link reset to Rx-A
 *
 * Field: ::VTSS_UPI_HSS6G_PLL_HSS6G_LINK_RESET_0 . LRST00
 */
#define  VTSS_F_UPI_HSS6G_PLL_HSS6G_LINK_RESET_0_LRST00(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_UPI_HSS6G_PLL_HSS6G_LINK_RESET_0_LRST00     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_UPI_HSS6G_PLL_HSS6G_LINK_RESET_0_LRST00(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/**
 * Register Group: \a UPI::HSS6G_TX
 *
 * Tx registers (Base address = Ch0, Base + 1 = Ch1, Base + 2 = Ch2, Base + 3 = Ch3, Base + 4 = broadcast) 
 */


/** 
 * \brief HSS6G Tx transmit configuration mode
 *
 * \details
 * Access read/write 

 *
 * Register: \a UPI::HSS6G_TX::HSS6G_TRANSMIT_CONFIGURATION_MODE
 *
 * @param target A \a ::vtss_target_UPI_e target
 * @param ri Register: HSS6G_TRANSMIT_CONFIGURATION_MODE (??), 0-4
 */
#define VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_CONFIGURATION_MODE(target,ri)  VTSS_IOREG(target,0x2900 + (ri))

/** 
 * \brief
 * Data bus width select
 * Note: If word synchronization between ports is required, a resync
 * operation is required after changing the value of this register.
 *
 * \details 
 * 00: 8 bit 
 * 01: 10 bit
 * 10: 16 bit (default)
 * 11: 20 bit
 *
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_CONFIGURATION_MODE . BWSEL
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_CONFIGURATION_MODE_BWSEL(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_CONFIGURATION_MODE_BWSEL     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_CONFIGURATION_MODE_BWSEL(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Rate select
 * Note: If word synchronization between ports is required, a resync
 * operation is required after changing the value of this register.
 *
 * \details 
 * 00: Full rate (default)
 * 01: Half rate
 * 10: Quarter rate
 * 11: Reserved
 *
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_CONFIGURATION_MODE . RTSEL
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_CONFIGURATION_MODE_RTSEL(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_CONFIGURATION_MODE_RTSEL     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_CONFIGURATION_MODE_RTSEL(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief HSS6G Tx transmit test control
 *
 * \details
 * Access read/write
 *
 * Register: \a UPI::HSS6G_TX::HSS6G_TRANSMIT_TEST_CONTROL
 *
 * @param target A \a ::vtss_target_UPI_e target
 * @param ri Register: HSS6G_TRANSMIT_TEST_CONTROL (??), 0-4
 */
#define VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL(target,ri)  VTSS_IOREG(target,0x2905 + (ri))

/** 
 * \brief
 * Loopback reset
 * When de-asserted and loopback is enabled, initiates the synchronization
 * and subsequent checking of the loopback data.
 *
 * \details 
 * 0: De-assert reset to PRBS7 checker (default)
 * 1: Assert reset to PRBS7 checker
 *
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL . LBRST
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL_LBRST(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL_LBRST     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL_LBRST(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Loopback enable
 * Provides similar function as the TXPRBSEN bit in HSS_CONTROLS
 *
 * \details 
 * 0: Normal operation, PRBS7 loopback disabled (default)
 * 1: Enable the 4-bit loopback path in the analog section and the PRBS7
 * checker of the logic 
 *
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL . LBEN
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL_LBEN(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL_LBEN     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL_LBEN(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Test pattern select
 * For patterns 000 through 101, the bus width is forced to 16-bit wide if
 * the current setting is 8 or 16, and forced to 20-bit wide if the current
 * setting is 10 or 20. 
 * For patterns 110 and 111, the bus width is forced to 16-bit wide.
 * TXxDCLK responds accordingly, irrespective of the bus width setting in
 * register 0x00. 
 * This register is ignored and the test pattern is forced to PRBS7+ when
 * Loopback Enable (bit 4) or TXPRBSEN is asserted.
 *
 * \details 
 * 000: PRBS7+ (default)
 * 001: PRBS7-
 * 010: PRBS23+
 * 011: PRBS23-
 * 100: PRBS31+
 * 101: PRBS31-
 * 110: 1010101...
 * 111: Repeating pattern of 64 1s followed by 64 0s
 *
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL . PATSEL
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL_PATSEL(x)  VTSS_ENCODE_BITFIELD(x,1,3)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL_PATSEL     VTSS_ENCODE_BITMASK(1,3)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL_PATSEL(x)  VTSS_EXTRACT_BITFIELD(x,1,3)

/** 
 * \brief
 * Test pattern data select
 * Note: This register is ignored and the function is forced to use test
 * pattern data when the TXPRBSEN bit in HSS_CONTROLS is asserted.    
 *
 * \details 
 * 0: Use customer parallel data (default)
 * 1: Use test pattern data
 *
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL . TPDSEL
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL_TPDSEL(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL_TPDSEL     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_TEST_CONTROL_TPDSEL(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief HSS6G Tx transmit coefficient 0
 *
 * \details
 * Access mixed
 *
 * Register: \a UPI::HSS6G_TX::HSS6G_TRANSMIT_COEFFICIENT_0
 *
 * @param target A \a ::vtss_target_UPI_e target
 * @param ri Register: HSS6G_TRANSMIT_COEFFICIENT_0 (??), 0-4
 */
#define VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_0(target,ri)  VTSS_IOREG(target,0x2950 + (ri))

/** 
 * \brief
 * Current tap 0
 * Current tap 0 coefficient being used
 *
 * \details 
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_0 . CURT0
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_0_CURT0(x)  VTSS_ENCODE_BITFIELD(x,8,6)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_0_CURT0     VTSS_ENCODE_BITMASK(8,6)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_0_CURT0(x)  VTSS_EXTRACT_BITFIELD(x,8,6)

/** 
 * \brief
 * Next tap 0
 * Next tap 0 coefficient to be applied when the Apply Load bit is set in
 * the Transmit Coefficient Control register. This register's value is used
 * in the computation of the TAP 0 IDAC value that is sent to the analog
 * block. The value is an unsigned magnitude.
 *
 * \details 
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_0 . NXTT0
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_0_NXTT0(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_0_NXTT0     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_0_NXTT0(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief HSS6G Tx transmit coefficient 1
 *
 * \details
 * Access mixed
 *
 * Register: \a UPI::HSS6G_TX::HSS6G_TRANSMIT_COEFFICIENT_1
 *
 * @param target A \a ::vtss_target_UPI_e target
 * @param ri Register: HSS6G_TRANSMIT_COEFFICIENT_1 (??), 0-4
 */
#define VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_1(target,ri)  VTSS_IOREG(target,0x2955 + (ri))

/** 
 * \brief
 * Current tap 1
 * Current tap 1 coefficient being used
 *
 * \details 
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_1 . CURT1
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_1_CURT1(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_1_CURT1     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_1_CURT1(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Next tap 1
 * Next tap 1 coefficient to be applied when the Apply Load bit is set in
 * the Transmit Coefficient Control register. This register's value is used
 * in the computation of the TAP 1 IDAC value that is sent to the analog
 * block. The value is an unsigned magnitude.	
 *
 * \details 
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_1 . NXTT1
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_1_NXTT1(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_1_NXTT1     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_1_NXTT1(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief HSS6G Tx transmit coefficient 2
 *
 * \details
 * Access mixed
 *
 * Register: \a UPI::HSS6G_TX::HSS6G_TRANSMIT_COEFFICIENT_2
 *
 * @param target A \a ::vtss_target_UPI_e target
 * @param ri Register: HSS6G_TRANSMIT_COEFFICIENT_2 (??), 0-4
 */
#define VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_2(target,ri)  VTSS_IOREG(target,0x295a + (ri))

/** 
 * \brief
 * Current tap 2
 * Current tap 2 coefficient being used
 *
 * \details 
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_2 . CURT2
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_2_CURT2(x)  VTSS_ENCODE_BITFIELD(x,8,7)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_2_CURT2     VTSS_ENCODE_BITMASK(8,7)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_2_CURT2(x)  VTSS_EXTRACT_BITFIELD(x,8,7)

/** 
 * \brief
 * Next tap 2
 * Next tap 2 coefficient to be applied when the Apply Load bit is set in
 * the Transmit Coefficient Control register. This register's value is used
 * in the computation of the TAP 2 IDAC value that is sent to the analog
 * block. The value is an unsigned magnitude.
 *
 * \details 
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_2 . NXTT2
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_2_NXTT2(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_2_NXTT2     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_2_NXTT2(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief HSS6G Tx transmit coefficient 3
 *
 * \details
 * Access mixed
 *
 * Register: \a UPI::HSS6G_TX::HSS6G_TRANSMIT_COEFFICIENT_3
 *
 * @param target A \a ::vtss_target_UPI_e target
 * @param ri Register: HSS6G_TRANSMIT_COEFFICIENT_3 (??), 0-4
 */
#define VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_3(target,ri)  VTSS_IOREG(target,0x295f + (ri))

/** 
 * \brief
 * Current tap 3
 * Current tap 3 coefficient being used
 *
 * \details 
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_3 . CURT3
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_3_CURT3(x)  VTSS_ENCODE_BITFIELD(x,8,6)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_3_CURT3     VTSS_ENCODE_BITMASK(8,6)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_3_CURT3(x)  VTSS_EXTRACT_BITFIELD(x,8,6)

/** 
 * \brief
 * Next Tap 3
 * Next tap 3 coefficient to be applied when the Apply Load bit is set in
 * the Transmit Coefficient Control register. This register's value is used
 * in the computation of the TAP 3 IDAC value that is sent to the analog
 * block. The value is an unsigned magnitude.
 *
 * \details 
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_3 . NXTT3
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_3_NXTT3(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_3_NXTT3     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_3_NXTT3(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief HSS6G Tx transmit coefficient DAC 0
 *
 * \details
 * Access mixed
 * This register is used for diagnostic and test purposes to override the
 * internal calculations for the actual values of the tap coefficients sent
 * to the analog circuits. It is enabled by setting OVRDC in the Transmit
 * Coefficient Control register to override calculated coefficients.
 *
 * Register: \a UPI::HSS6G_TX::HSS6G_TRANSMIT_COEFFICIENT_DAC_0
 *
 * @param target A \a ::vtss_target_UPI_e target
 * @param ri Register: HSS6G_TRANSMIT_COEFFICIENT_DAC_0 (??), 0-4
 */
#define VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_0(target,ri)  VTSS_IOREG(target,0x2964 + (ri))

/** 
 * \brief
 * Current DAC 0
 * Tap 0 P*I coefficient currently being applied to the analog circuits
 *
 * \details 
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_0 . CURD0
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_0_CURD0(x)  VTSS_ENCODE_BITFIELD(x,8,6)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_0_CURD0     VTSS_ENCODE_BITMASK(8,6)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_0_CURD0(x)  VTSS_EXTRACT_BITFIELD(x,8,6)

/** 
 * \brief
 * Next DAC 0
 * Next tap 0 P*I coefficient to be used after completing an Apply Load
 * when OVRDC in the Transmit Coefficient Control register is set to
 * override calculated coefficients
 *
 * \details 
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_0 . NXTD0
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_0_NXTD0(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_0_NXTD0     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_0_NXTD0(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief HSS6G Tx transmit coefficient DAC 1
 *
 * \details
 * Access mixed
 * This register is used for diagnostic and test purposes to override the
 * internal calculations for the actual values of the tap coefficients sent
 * to the analog circuits. It is enabled by setting OVRDC in the Transmit
 * Coefficient Control register to override calculated coefficients.
 *
 * Register: \a UPI::HSS6G_TX::HSS6G_TRANSMIT_COEFFICIENT_DAC_1
 *
 * @param target A \a ::vtss_target_UPI_e target
 * @param ri Register: HSS6G_TRANSMIT_COEFFICIENT_DAC_1 (??), 0-4
 */
#define VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_1(target,ri)  VTSS_IOREG(target,0x2969 + (ri))

/** 
 * \brief
 * Current DAC 1
 * Tap 1 P*I coefficient currently being applied to the analog circuits
 *
 * \details 
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_1 . CURD1
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_1_CURD1(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_1_CURD1     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_1_CURD1(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Next DAC 1
 * Next tap 1 P*I coefficient to be used after completing an Apply Load
 * when OVRDC in the Transmit Coefficient Control register is set to
 * override calculated coefficients 
 *
 * \details 
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_1 . NXTD1
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_1_NXTD1(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_1_NXTD1     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_1_NXTD1(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief HSS6G Tx transmit coefficient DAC 2
 *
 * \details
 * Access mixed
 * This register is used for diagnostic and test purposes to override the
 * internal calculations for the actual values of the tap coefficients sent
 * to the analog circuits. It is enabled by setting OVRDC in the Transmit
 * Coefficient Control register to override calculated coefficients.
 *
 * Register: \a UPI::HSS6G_TX::HSS6G_TRANSMIT_COEFFICIENT_DAC_2
 *
 * @param target A \a ::vtss_target_UPI_e target
 * @param ri Register: HSS6G_TRANSMIT_COEFFICIENT_DAC_2 (??), 0-4
 */
#define VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_2(target,ri)  VTSS_IOREG(target,0x296e + (ri))

/** 
 * \brief
 * Current DAC 2
 * Tap 2 P*I coefficient currently being applied to the analog circuits
 *
 * \details 
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_2 . CURD2
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_2_CURD2(x)  VTSS_ENCODE_BITFIELD(x,8,7)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_2_CURD2     VTSS_ENCODE_BITMASK(8,7)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_2_CURD2(x)  VTSS_EXTRACT_BITFIELD(x,8,7)

/** 
 * \brief
 * Next DAC 2
 * Next tap 2 P*I coefficient to be used after completing an Apply Load
 * when OVRDC in the Transmit Coefficient Control register is set to
 * override calculated coefficients
 *
 * \details 
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_2 . NXTD2
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_2_NXTD2(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_2_NXTD2     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_2_NXTD2(x)  VTSS_EXTRACT_BITFIELD(x,0,7)


/** 
 * \brief HSS6G Tx transmit coefficient DAC 3
 *
 * \details
 * Access mixed
 * This register is used for diagnostic and test purposes to override the
 * internal calculations for the actual values of the tap coefficients sent
 * to the analog circuits. It is enabled by setting OVRDC in the Transmit
 * Coefficient Control register to override calculated coefficients.
 *
 * Register: \a UPI::HSS6G_TX::HSS6G_TRANSMIT_COEFFICIENT_DAC_3
 *
 * @param target A \a ::vtss_target_UPI_e target
 * @param ri Register: HSS6G_TRANSMIT_COEFFICIENT_DAC_3 (??), 0-4
 */
#define VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_3(target,ri)  VTSS_IOREG(target,0x2973 + (ri))

/** 
 * \brief
 * Current DAC 3
 * Tap 3 P*I coefficient currently being applied to the analog circuits
 *
 * \details 
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_3 . CURD3
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_3_CURD3(x)  VTSS_ENCODE_BITFIELD(x,8,6)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_3_CURD3     VTSS_ENCODE_BITMASK(8,6)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_3_CURD3(x)  VTSS_EXTRACT_BITFIELD(x,8,6)

/** 
 * \brief
 * Next DAC 3
 * Next tap 3 P*I coefficient to be used after completing an Apply Load
 * when OVRDC in the Transmit Coefficient Control register is set to
 * override calculated coefficients
 *
 * \details 
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_3 . NXTD3
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_3_NXTD3(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_3_NXTD3     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_DAC_3_NXTD3(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief HSS6G Tx transmit coefficient control
 *
 * \details
 * Access read/write
 *
 * Register: \a UPI::HSS6G_TX::HSS6G_TRANSMIT_COEFFICIENT_CONTROL
 *
 * @param target A \a ::vtss_target_UPI_e target
 * @param ri Register: HSS6G_TRANSMIT_COEFFICIENT_CONTROL (??), 0-4
 */
#define VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL(target,ri)  VTSS_IOREG(target,0x2978 + (ri))

/** 
 * \brief
 * Override calculated coefficients
 * This bit is typically only used for test and diagnostic purposes
 * This bit, when asserted, bypasses the coefficient calculation logic and
 * instead applies the Transmit P*I Coefficient DAC values in the Transmit
 * Coefficient DAC registers to the analog inputs. An Apply Load must be
 * pulsed for the new values to be made available to the analog block.
 *
 * \details 
 * 0: Normal (default)
 * 1: Override internal calculations
 *
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL . OVRDC
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_OVRDC(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_OVRDC     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_OVRDC(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Reset coefficient logic
 *
 * \details 
 * 0: Normal (default)
 * 1: Reset
 *
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL . RSTCL
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_RSTCL(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_RSTCL     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_RSTCL(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Upchannel external clock enable
 * No longer supported. Always write to 0.   
 *
 * \details 
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL . UPCEN
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_UPCEN(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_UPCEN     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_UPCEN(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Apply load
 * This bit applies the register loaded values of coefficients, power,
 * polarity, and FFE mode to the coefficient recalculation logic and
 * presents this new value to the analog circuits. The following registers
 * are affected: 
 * HSS6G_TRANSMIT_TEST_CONTROL 
 * HSS6G_TRANSMIT_COEFFICIENT_0
 * HSS6G_TRANSMIT_COEFFICIENT_1
 * HSS6G_TRANSMIT_COEFFICIENT_2
 * HSS11G_DFE_H1
 * HSS11G_DFE_H3
 * HSS11G_DFE_H4
 * HSS11G_DFE_H5 
 * For the new values to be applied, the logic must see a transition from
 * low to high on this signal. It is recommended that after this bit is
 * written to a 1, it is followed up with a write back to 0.
 *
 * \details 
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL . APPLD
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_COEFFICIENT_CONTROL_APPLD(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief HSS6G Tx transmit driver mode control
 *
 * \details
 * Access read/write
 *
 * Register: \a UPI::HSS6G_TX::HSS6G_TRANSMIT_DRIVER_MODE_CONTROL
 *
 * @param target A \a ::vtss_target_UPI_e target
 * @param ri Register: HSS6G_TRANSMIT_DRIVER_MODE_CONTROL (??), 0-4
 */
#define VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL(target,ri)  VTSS_IOREG(target,0x297d + (ri))

/** 
 * \brief
 * Predriver bias enable
 * For manufacturing test use only
 *
 * \details 
 * 0: Disable bias
 * 1: Enable bias (default)
 *
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL . PBIASEN
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL_PBIASEN(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL_PBIASEN     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL_PBIASEN(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Minimum slew rate control
 *
 * \details 
 * 0: 30 ps (default)
 * 1: 60 ps
 *
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL . MNSLEW
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL_MNSLEW(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL_MNSLEW     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL_MNSLEW(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Supply equal enable
 * Unused
 *
 * \details 
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL . SEQEN
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL_SEQEN(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL_SEQEN     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL_SEQEN(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * FFE mode select
 * These values take effect when the Apply Load bit is set in the Transmit
 * Coefficient Control register except for the Force Hi-Z state, which
 * takes effect immediately.
 *
 * \details 
 * 00: FFE2 (default)
 * 01: FFE3
 * 10: FFE4
 * 11: Force Hi-Z
 *
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL . FFEMD
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL_FFEMD(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL_FFEMD     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_DRIVER_MODE_CONTROL_FFEMD(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief HSS6G Tx transmit polarity
 *
 * \details
 * Access mixed
 *
 * Register: \a UPI::HSS6G_TX::HSS6G_TRANSMIT_POLARITY
 *
 * @param target A \a ::vtss_target_UPI_e target
 * @param ri Register: HSS6G_TRANSMIT_POLARITY (??), 0-4
 */
#define VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY(target,ri)  VTSS_IOREG(target,0x2982 + (ri))

/** 
 * \brief
 * Current polarity
 * Polarity currently being applied to the analog circuits. 
 * Coefficients [3:0] = bits [11:8] 
 *
 * \details 
 * 0: Negative polarity
 * 1: Positive polarity
 *
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY . CURPOL
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY_CURPOL(x)  VTSS_ENCODE_BITFIELD(x,8,4)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY_CURPOL     VTSS_ENCODE_BITMASK(8,4)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY_CURPOL(x)  VTSS_EXTRACT_BITFIELD(x,8,4)

/** 
 * \brief
 * Next polarity
 * Next polarity values to be used when the Apply Load bit is set in the
 * Transmit Coefficient Control register. 
 * Coefficients [3:0] = bits [3:0] 
 *
 * \details 
 * 0: Negative polarity
 * 1: Positive polarity
 *
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY . NXTPOL
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY_NXTPOL(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY_NXTPOL     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_POLARITY_NXTPOL(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief HSS6G Tx transmit power
 *
 * \details
 * Access read/write 

 *
 * Register: \a UPI::HSS6G_TX::HSS6G_TRANSMIT_POWER
 *
 * @param target A \a ::vtss_target_UPI_e target
 * @param ri Register: HSS6G_TRANSMIT_POWER (??), 0-4
 */
#define VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_POWER(target,ri)  VTSS_IOREG(target,0x2987 + (ri))

/** 
 * \brief
 * Maximum power setting
 * These values take effect when the Apply Load bit is set in the Transmit
 * Coefficient Control register. 
 * Valid values are decimal 20 through 127.	  
 *
 * \details 
 * Decimal 115: 100% power
 * Decimal 127: 110% power
 *
 * Field: ::VTSS_UPI_HSS6G_TX_HSS6G_TRANSMIT_POWER . MXPWR
 */
#define  VTSS_F_UPI_HSS6G_TX_HSS6G_TRANSMIT_POWER_MXPWR(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_UPI_HSS6G_TX_HSS6G_TRANSMIT_POWER_MXPWR     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_UPI_HSS6G_TX_HSS6G_TRANSMIT_POWER_MXPWR(x)  VTSS_EXTRACT_BITFIELD(x,0,7)

/**
 * Register Group: \a UPI::HSS6G_RX
 *
 * Rx registers (Base address = Ch0, Base + 1 = Ch1, Base + 2 = Ch2, Base + 3 = Ch3, Base + 4 = broadcast) 
 */


/** 
 * \brief HSS6G Rx receiver configuration mode
 *
 * \details
 * Access read/write 

 *
 * Register: \a UPI::HSS6G_RX::HSS6G_RECEIVER_CONFIGURATION_MODE
 *
 * @param target A \a ::vtss_target_UPI_e target
 * @param ri Register: HSS6G_RECEIVER_CONFIGURATION_MODE (??), 0-4
 */
#define VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_CONFIGURATION_MODE(target,ri)  VTSS_IOREG(target,0x2a00 + (ri))

/** 
 * \brief
 * DFE mode select
 * An internal DFE reset operation automatically generates after changing
 * the value of this register.
 * Note: Non-DFE LP mode disables AMP sample processing, Ap/An/Amin loops,
 * and the VGA gain and offset loops unless bit 4 of the DFE Control
 * register is set.
 *
 * \details 
 * 00: DFE5
 * 01: DFE3
 * 10: Non-DFE (default)
 * 11: Non-DFE LP
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_CONFIGURATION_MODE . DMSEL
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_RECEIVER_CONFIGURATION_MODE_DMSEL(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_RECEIVER_CONFIGURATION_MODE_DMSEL     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_RECEIVER_CONFIGURATION_MODE_DMSEL(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * Data bus width select     
 *
 * \details 
 * 00: 8 bit parallel bus 
 * 01: 10 bit parallel bus 
 * 10: 16 bit parallel bus (default) 
 * 11: 20 bit parallel bus
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_CONFIGURATION_MODE . BWSEL
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_RECEIVER_CONFIGURATION_MODE_BWSEL(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_RECEIVER_CONFIGURATION_MODE_BWSEL     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_RECEIVER_CONFIGURATION_MODE_BWSEL(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Rate select
 * An internal DFE reset operation automatically generates after changing
 * the value of this register
 *
 * \details 
 * 00: Full (default)
 * 01: Half
 * 10: Quarter
 * 11: Reserved
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_CONFIGURATION_MODE . RTSEL
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_RECEIVER_CONFIGURATION_MODE_RTSEL(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_RECEIVER_CONFIGURATION_MODE_RTSEL     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_RECEIVER_CONFIGURATION_MODE_RTSEL(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief HSS6G Rx receiver test control
 *
 * \details
 * Access mixed
 *
 * Register: \a UPI::HSS6G_RX::HSS6G_RECEIVER_TEST_CONTROL
 *
 * @param target A \a ::vtss_target_UPI_e target
 * @param ri Register: HSS6G_RECEIVER_TEST_CONTROL (??), 0-4
 */
#define VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL(target,ri)  VTSS_IOREG(target,0x2a05 + (ri))

/** 
 * \brief
 * PRBS force error status
 * State of the RXPRBSFRCERR bit in HSS_CONTROLS
 *
 * \details 
 * 0: PRBS is in normal operation
 * 1: PRBS error is being forced by opening the wrap path
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL . FERRST
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_FERRST(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_FERRST     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_FERRST(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * PRBS error status
 * State of the RX_PRBSERR bit in HSS_CONTROLS that indicates the PRBS
 * checker status
 *
 * \details 
 * 0: PRBS pattern match, or PRBS sync not yet achieved
 * 1: PRBS error detected after PRBS sync has been achieved 
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL . ERRST
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_ERRST(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_ERRST     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_ERRST(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * PRBS sync status
 * State of the RX_PRBSSYNC bit in HSS_CONTROLS that indicates whether
 * pattern synchronization was achieved
 *
 * \details 
 * 0: PRBS checker not synchronized to incoming data
 * 1: PRBS checker synchronized and locked to incoming PRBS data
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL . SYNCST
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_SYNCST(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_SYNCST     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_SYNCST(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Loopback slave mode enable
 * The phase rotator of this link is used to generate an async input clock
 * to the Rx BIST generator of the adjacent Rx link. This can be used to
 * exercise the adjacent Rx in an async environment. This is intended for
 * manufacturing test only and imposes significant limitations for general
 * use. It renders this Rx link disabled for normal operation, it can only
 * be used in full-rate mode, and it freezes DFE functionality for this
 * link. This bit resets to 1 when HSSRSTCONFIG[1:0]=10.
 *
 * \details 
 * 0: Normal operation (default)
 * 1: Enables the loopback (wrapclock) slave mode
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL . LBSLV
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_LBSLV(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_LBSLV     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_LBSLV(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Wrap enable
 * This bit resets to 1 when HSSRSTCONFIG[1:0]=01.
 * Note: This register is ignored if RXPRBSFRCERR=1 in HSS_CONTROLS and the
 * primary input to the Rx is selected.
 *
 * \details 
 * 0: Normal operation - selects the primary input to the Rx and disables
 * the internal Tx to Rx wrap buffer (default)
 * 1: Wrap - either the loopback or the internal wrapback is selected based
 * on the value of bit 5
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL . WRPEN
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_WRPEN(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_WRPEN     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_WRPEN(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Wrap mode select
 * To ensure the Tx provides the correct wrap data, set the Transmit
 * Coefficient 0 register to a non-zero value and set the Transmit Polarity
 * register bit 8 to 1.
 *
 * \details 
 * 0: RXBIST to Rx loopback mode selected and the RXBIST generator is
 * enabled.
 * 1: Tx to Rx full duplex internal wrapback mode is selected and the
 * RXBIST generator is disabled. On the simplex cores a static zero will be
 * received.
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL . WRPMD
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_WRPMD(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_WRPMD     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_WRPMD(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * PRBS reset
 * Note: This bit is ignored if RXPRBSRST=1 in HSS_CONTROLS and a reset is
 * applied to the PRBS checker.
 *
 * \details 
 * 0: Normal operation (default)
 * 1: Reset is applied to the PRBS checker and RXBIST generator
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL . PRST
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_PRST(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_PRST     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_PRST(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * PRBS check enable
 * Note: This bit is ignored if RXPRBSEN=1 and HSSPRBSEN=1 in HSS_CONTROLS
 *
 * \details 
 * 0: Disabled (default) 
 * 1: Enabled 
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL . PCHKEN
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_PCHKEN(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_PCHKEN     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_PCHKEN(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Test pattern select
 * This bit is ignored when RXPRBSEN in HSS_CONTROLS is asserted
 *
 * \details 
 * 0x0: PRBS7+ (non-inverted) (default) 
 * 0x1: PRBS7- (inverted) 
 * 0x2: PRBS23+ (non-inverted) 
 * 0x3: PRBS23- (inverted) 
 * 0x4: PRBS31+ (non-inverted) 
 * 0x5: PRBS31- (inverted) 
 * 0x6: Unused 
 * 0x7: Unused 
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL . PATSEL
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_PATSEL(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_PATSEL     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_RECEIVER_TEST_CONTROL_PATSEL(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief HSS6G Rx phase rotator control
 *
 * \details
 * Access read/write 
 *
 * Register: \a UPI::HSS6G_RX::HSS6G_PHASE_ROTATOR_CONTROL
 *
 * @param target A \a ::vtss_target_UPI_e target
 * @param ri Register: HSS6G_PHASE_ROTATOR_CONTROL (??), 0-4
 */
#define VTSS_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL(target,ri)  VTSS_IOREG(target,0x2a0a + (ri))

/** 
 * \brief
 * Flywheel throttle
 * This value is used to limit the flywheel rate to between 427 and 7747
 * ppm. The default is 6283 ppm. It is calculated as: ((FTHROT x 8) + 7) x
 * 61 ppm.
 *
 * \details 
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL . FTHROT
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL_FTHROT(x)  VTSS_ENCODE_BITFIELD(x,12,4)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL_FTHROT     VTSS_ENCODE_BITMASK(12,4)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL_FTHROT(x)  VTSS_EXTRACT_BITFIELD(x,12,4)

/** 
 * \brief
 * Reset on throttle   
 *
 * \details 
 * 1: Reset the flywheel when the flywheel throttle is reached
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL . RTHROT
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL_RTHROT(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL_RTHROT     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL_RTHROT(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * CDR filter control
 * This register should be left at the default value. However, to support
 * characterization test, it can be used to adjust the CDR loop filter
 * bandwidth. This value controls the magnitude of phase inc/decs to the
 * phase accumulator of the CDR as follows:
 * Full rate mode: magnitude=1/2 this value
 * Half rate mode: magnitude= this value
 * Quarter rate mode: magnitude= 2x this value
 * All values from 0x2 to 0xF are legal. 0x2 is forced for writes below
 * this value.
 *
 * \details 
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL . FILTCTL
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL_FILTCTL(x)  VTSS_ENCODE_BITFIELD(x,7,4)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL_FILTCTL     VTSS_ENCODE_BITMASK(7,4)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL_FILTCTL(x)  VTSS_EXTRACT_BITFIELD(x,7,4)

/** 
 * \brief
 * External early/late control
 * Used to select the external early/late phase rotator controls
 *
 * \details 
 * 0: Internal only (default)
 * 1: External only
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL . EXTEL
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL_EXTEL(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL_EXTEL     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL_EXTEL(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Reset on stuck
 * Note: Stuck detection is only available in 1/2 and 1/4 rate modes.
 * Setting this bit is not recommended for applications exceeding +/-100
 * ppm clock tolerance.
 *
 * \details 
 * 0: Detection of stuck condition does not reset the flywheel (default)
 * 1: Detection of stuck condition does reset the flywheel
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL . RSTUCK
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL_RSTUCK(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL_RSTUCK     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL_RSTUCK(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Freeze flywheel   
 *
 * \details 
 * 0: Normal (default)
 * 1: Freeze the phase rotator flywheel at its current update rate. This
 * can be used to prevent periods of inactivity from altering the state of
 * the flywheel.
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL . FRZFW
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL_FRZFW(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL_FRZFW     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL_FRZFW(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Reset flywheel
 *
 * \details 
 * 0: Normal, the flywheel is enabled (default)
 * 1: Assert reset to the phase rotator flywheel (disable the flywheel)
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL . RSTFW
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL_RSTFW(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL_RSTFW     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL_RSTFW(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Spread spectrum clocking enable
 * This bit should not be enabled unless spread spectrum clocking input
 * data is applied or REFCLK is performing spread spectrum clocking. Rx
 * jitter tolerance is improved in non-spread spectrum mode.
 *
 * \details 
 * 0: Spread spectrum clocking support disabled (default)
 * 1: Spread spectrum clocking support enabled
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL . SSCEN
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL_SSCEN(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL_SSCEN     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_PHASE_ROTATOR_CONTROL_SSCEN(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief HSS6G Rx receiver sigdet control
 *
 * \details
 * Access mixed
 *
 * Register: \a UPI::HSS6G_RX::HSS6G_RECEIVER_SIGDET_CONTROL
 *
 * @param target A \a ::vtss_target_UPI_e target
 * @param ri Register: HSS6G_RECEIVER_SIGDET_CONTROL (??), 0-4
 */
#define VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_SIGDET_CONTROL(target,ri)  VTSS_IOREG(target,0x2a23 + (ri))

/** 
 * \brief
 * Offset sign
 * Sign of the offset on signal detect. Bit 14 is R/W, bit 13 is Read Only.
 *
 * \details 
 * 01: Positive
 * 10: Negative
 * 00: Zero (occurs when Offset Amplitude=0)
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_SIGDET_CONTROL . OFFSN
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_RECEIVER_SIGDET_CONTROL_OFFSN(x)  VTSS_ENCODE_BITFIELD(x,13,2)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_RECEIVER_SIGDET_CONTROL_OFFSN     VTSS_ENCODE_BITMASK(13,2)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_RECEIVER_SIGDET_CONTROL_OFFSN(x)  VTSS_EXTRACT_BITFIELD(x,13,2)

/** 
 * \brief
 * Offset amplitude
 * Unsigned absolute value of offset amplitude on SIGDET. This value is set
 * by the internal calibration logic after reset and remains constant after
 * calibration is done unless written here.
 *
 * \details 
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_SIGDET_CONTROL . OFFAMP
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_RECEIVER_SIGDET_CONTROL_OFFAMP(x)  VTSS_ENCODE_BITFIELD(x,8,5)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_RECEIVER_SIGDET_CONTROL_OFFAMP     VTSS_ENCODE_BITMASK(8,5)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_RECEIVER_SIGDET_CONTROL_OFFAMP(x)  VTSS_EXTRACT_BITFIELD(x,8,5)

/** 
 * \brief
 * Signal detect power down
 *
 * \details 
 * 0: Enable (default - required for DFE mode)
 * 1: Power down the signal detect circuit
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_SIGDET_CONTROL . SDPDN
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_RECEIVER_SIGDET_CONTROL_SDPDN(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_RECEIVER_SIGDET_CONTROL_SDPDN     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_RECEIVER_SIGDET_CONTROL_SDPDN(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Signal detected
 * In normal operation, bit 9 of register 0x1F is set to 1 resulting in the
 * SIGDET signal being passed through this register to the internal logic.
 * This register can be read to obtain the status of the SIGDET signal.
 * However, in order to support diagnostic tests, bit 9 of register 1F can
 * be set to 0, and this register can be written to test its effect on
 * internal logic.
 *
 * \details 
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_SIGDET_CONTROL . SIGDET
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_RECEIVER_SIGDET_CONTROL_SIGDET(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_RECEIVER_SIGDET_CONTROL_SIGDET     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_RECEIVER_SIGDET_CONTROL_SIGDET(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Signal detect level
 * Unsigned value of the comparator threshold used in the SIGDET circuit    
 *
 * \details 
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_RECEIVER_SIGDET_CONTROL . SDLVL
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_RECEIVER_SIGDET_CONTROL_SDLVL(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_RECEIVER_SIGDET_CONTROL_SDLVL     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_RECEIVER_SIGDET_CONTROL_SDLVL(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief HSS6G Rx DFE control
 *
 * \details
 * This register is used to control the DFE operation. Access read/write.
 *
 * Register: \a UPI::HSS6G_RX::HSS6G_DFE_CONTROL
 *
 * @param target A \a ::vtss_target_UPI_e target
 * @param ri Register: HSS6G_DFE_CONTROL (??), 0-4
 */
#define VTSS_UPI_HSS6G_RX_HSS6G_DFE_CONTROL(target,ri)  VTSS_IOREG(target,0x2a28 + (ri))

/** 
 * \brief
 * DFE sample request completed 
 *
 * \details 
 * 0: Inactive or not ready yet (normal)
 * 1: Requested sample snapshot is now valid and available in registers
 * 0x09 and 0x0A
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_DFE_CONTROL . REQCMP
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_REQCMP(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_REQCMP     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_REQCMP(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * DFE sample request
 * This signal should be held high until bit 15 is read as a 1. Writing
 * this bit to 0 will clear bit 15. Bit 15 must be at 0, indicating
 * readiness for a new sample request before writing this bit to a 1.
 *
 * \details 
 * 0: Inactive (default)
 * 1: A rising edge of this bit causes the pipeline sampling logic to
 * capture a new snapshot and makes the results available in registers 0x09
 * and 0x0A. 
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_DFE_CONTROL . DFEREQ
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_DFEREQ(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_DFEREQ     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_DFEREQ(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * DFE gate enable
 *
 * \details 
 * 0: DFE samples flow only when needed by internal logic (normal operation
 * for power savings) (default)
 * 1: DFE samples always enabled (used to enable sample updates when DFE
 * sample requests will be made, during Scopemode operation, or in standby
 * mode).
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_DFE_CONTROL . GATEEN
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_GATEEN(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_GATEEN     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_GATEEN(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * DFE standby
 * I standby mode, all internal DFE operations are halted at the next
 * available internal break point. Clocks continue to run, but state
 * machines are held idle. All DFE training (A, H, VGA) is halted and
 * coefficients remain fixed at their present values.
 * Note: Do not enable DFE standby until the VGA is locked as indicated by
 * bit 8 of the Receiver VGA Control register 3.   
 *
 * \details 
 * 0: Normal operation (default)
 * 1: Standby mode
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_DFE_CONTROL . STNDBY
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_STNDBY(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_STNDBY     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_STNDBY(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Force H updates
 * Under certain conditions, the DFE logic halts its training process.
 *
 * \details 
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_DFE_CONTROL . FRCH
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_FRCH(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_FRCH     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_FRCH(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Not random data (dynamic)
 * Proper training of the DFE coefficients requires sufficiently random
 * data flow. To ensure this, a random data detector  function is built
 * into the logic. This function continuously monitors data randomness and
 * dynamically enables/disables DFE training to maintain sufficient random
 * balance of the samples included. 
 * Depending on data flow, this signal might assert very frequently under
 * normal operation. In normal operation, bit 12 of register 0x1F is set to
 * 1, enabling the internal random data detector to inhibit internal
 * updates. This register can be read to obtain the status of this internal
 * signal. 
 * To support diagnostic tests, bit 12 of register 0x1F can be set to 0,
 * and this register can be written to 1 or 0 to test its effect.
 * Note: This function only works in full-rate and half-rate modes. The
 * response of this bit is unreliable in quarter-rate mode.
 *
 * \details 
 * 0: Data is sufficiently random and updates continue
 * 1: Data is not sufficiently random and updates cease until random again
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_DFE_CONTROL . NONRND
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_NONRND(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_NONRND     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_NONRND(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Not random data (filtered)
 * This is a filtered version of the NONRND bit. It provides a more general
 * indication of data randomness.
 * Note: This function only works in full-rate and half-rate modes. The
 * response of this bit is unreliable in quarter-rate mode.   
 *
 * \details 
 * 0: Data is sufficiently random and updates continue
 * 1: Data is not sufficiently random and updates cease until random again
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_DFE_CONTROL . NONRNF
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_NONRNF(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_NONRNF     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_NONRNF(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * DFE fast lock enable
 *
 * \details 
 * 0: Disabled. Max loop filtering at all times. 
 * 1: Enabled. Reduced loop filtering during initial lock, then max loop
 * filtering applied. This mode is required to support initial lock within
 * 1 ms following rate changes (default).    
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_DFE_CONTROL . FSTLCK
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_FSTLCK(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_FSTLCK     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_FSTLCK(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * DFE Reset
 * DFE control logic reset. This bit must be cleared back to 0 before DFE
 * and non-DFE function can resume.
 *
 * \details 
 * 0: Normal
 * 1: Triggers a reset of the DFE logic
 *
 * Field: ::VTSS_UPI_HSS6G_RX_HSS6G_DFE_CONTROL . DFERST
 */
#define  VTSS_F_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_DFERST(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_DFERST     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_UPI_HSS6G_RX_HSS6G_DFE_CONTROL_DFERST(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


#endif /* _VTSS_DAYTONA_REGS_UPI_H_ */
