#ifndef _VTSS_DAYTONA_REGS_SFI4_H_
#define _VTSS_DAYTONA_REGS_SFI4_H_

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
 * Target: \a SFI4
 *
 * \see vtss_target_SFI4_e
 *
 * SerDes Framer Interface phase 2 (SFI4.2)
 *
 ***********************************************************************/

/**
 * Register Group: \a SFI4::SFI4_CONFIGURATION
 *
 * SFI4.2 configuration
 */


/** 
 * \brief SFI4.2 configuration
 *
 * \details
 * Configuration register
 *
 * Register: \a SFI4::SFI4_CONFIGURATION::SFI4_CFG
 *
 * @param target A \a ::vtss_target_SFI4_e target
 */
#define VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG(target)  VTSS_IOREG(target,0x0)

/** 
 * \brief
 * Testloop, if active data are looped from the Rx path to Tx path (system
 * side)
 *
 * \details 
 * 0: Normal operation
 * 1: Testloop enable
 *
 * Field: ::VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG . RX_TO_TX_LOOP_ENA
 */
#define  VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_RX_TO_TX_LOOP_ENA(x)  VTSS_ENCODE_BITFIELD(x,21,1)
#define  VTSS_M_SFI4_SFI4_CONFIGURATION_SFI4_CFG_RX_TO_TX_LOOP_ENA     VTSS_ENCODE_BITMASK(21,1)
#define  VTSS_X_SFI4_SFI4_CONFIGURATION_SFI4_CFG_RX_TO_TX_LOOP_ENA(x)  VTSS_EXTRACT_BITFIELD(x,21,1)

/** 
 * \brief
 * Testloop, if active data are looped from the Tx path to Rx path without
 * using SERDES loops
 *
 * \details 
 * 0: Normal operation
 * 1: Testloop enable
 *
 * Field: ::VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG . SLOOP_ENA
 */
#define  VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SLOOP_ENA(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SLOOP_ENA     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SLOOP_ENA(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \brief
 * Disable scrambler in 64B/66B codec (to be used only for test purposes)
 *
 * \details 
 * 0: Enable scrambler
 * 1: Disable scrambler
 *
 * Field: ::VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG . SCRAM_DIS
 */
#define  VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SCRAM_DIS(x)  VTSS_ENCODE_BITFIELD(x,17,1)
#define  VTSS_M_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SCRAM_DIS     VTSS_ENCODE_BITMASK(17,1)
#define  VTSS_X_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SCRAM_DIS(x)  VTSS_EXTRACT_BITFIELD(x,17,1)

/** 
 * \brief
 * Force re-synchronization of receive logic
 *
 * \details 
 * 0: Normal operation
 * 1: Reset Synchronization
 *
 * Field: ::VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG . RESYNC_ENA
 */
#define  VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_RESYNC_ENA(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_SFI4_SFI4_CONFIGURATION_SFI4_CFG_RESYNC_ENA     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_SFI4_SFI4_CONFIGURATION_SFI4_CFG_RESYNC_ENA(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Number of sync headers required for block lock plus one (used in 10Gb-R
 * Clause 49 SSM)
 *
 * \details 
 * Field: ::VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG . SH_CNT_MAX
 */
#define  VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SH_CNT_MAX(x)  VTSS_ENCODE_BITFIELD(x,10,6)
#define  VTSS_M_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SH_CNT_MAX     VTSS_ENCODE_BITMASK(10,6)
#define  VTSS_X_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SH_CNT_MAX(x)  VTSS_EXTRACT_BITFIELD(x,10,6)

/** 
 * \brief
 * Select double data rate mode (DDR)
 * Double data rate uses physical lanes 0 and 3 since the pinout locates
 * these lanes for optimal swapping
 *
 * \details 
 * 0: Standard SFI4.2 rates selected
 * 1: Double SFI4.2 rates selected
 *
 * Field: ::VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG . SFI_DDR_SEL
 */
#define  VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SFI_DDR_SEL(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SFI_DDR_SEL     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SFI_DDR_SEL(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * SFI4.2 enable
 *
 * \details 
 * 0: Disable SFI4.2
 * 1: Enable SFI4.2
 *
 * Field: ::VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG . SFI4_ENA
 */
#define  VTSS_F_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SFI4_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SFI4_ENA     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SFI4_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/**
 * Register Group: \a SFI4::SFI4_STATUS
 *
 * SFI4.2 status
 */


/** 
 * \brief SFI4.2 status
 *
 * \details
 * Status of SFI4.2 logic
 *
 * Register: \a SFI4::SFI4_STATUS::SFI4_STATUS
 *
 * @param target A \a ::vtss_target_SFI4_e target
 */
#define VTSS_SFI4_SFI4_STATUS_SFI4_STATUS(target)  VTSS_IOREG(target,0x1)

/** 
 * \brief
 * Status of deskew logic
 *
 * \details 
 * 0: Lanes not aligned
 * 1: All lanes are aligned
 *
 * Field: ::VTSS_SFI4_SFI4_STATUS_SFI4_STATUS . RX_ALIGNMENT_STATUS
 */
#define  VTSS_F_SFI4_SFI4_STATUS_SFI4_STATUS_RX_ALIGNMENT_STATUS(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_SFI4_SFI4_STATUS_SFI4_STATUS_RX_ALIGNMENT_STATUS     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_SFI4_SFI4_STATUS_SFI4_STATUS_RX_ALIGNMENT_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Status of synchronization in lane i (i = 0...3, one bit per lane)
 *
 * \details 
 * 0: Lane i out of sync
 * 1: Lane i is in sync
 *
 * Field: ::VTSS_SFI4_SFI4_STATUS_SFI4_STATUS . RX_SYNC_STATUS
 */
#define  VTSS_F_SFI4_SFI4_STATUS_SFI4_STATUS_RX_SYNC_STATUS(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_SFI4_SFI4_STATUS_SFI4_STATUS_RX_SYNC_STATUS     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_SFI4_SFI4_STATUS_SFI4_STATUS_RX_SYNC_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief SFI4.2 error status
 *
 * \details
 * Status change indications for the Rx 64B/66B PCS logic
 *
 * Register: \a SFI4::SFI4_STATUS::SFI4_STATUS_STICKY
 *
 * @param target A \a ::vtss_target_SFI4_e target
 */
#define VTSS_SFI4_SFI4_STATUS_SFI4_STATUS_STICKY(target)  VTSS_IOREG(target,0x2)

/** 
 * \brief
 * Alignment status change in deskew logic
 *
 * \details 
 * 0: No alignment change occurred
 * 1: An alignment change has been detected
 * Bit is cleared by writing a 1 to this position
 *
 * Field: ::VTSS_SFI4_SFI4_STATUS_SFI4_STATUS_STICKY . RX_ALIGNMENT_STICKY
 */
#define  VTSS_F_SFI4_SFI4_STATUS_SFI4_STATUS_STICKY_RX_ALIGNMENT_STICKY(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_SFI4_SFI4_STATUS_SFI4_STATUS_STICKY_RX_ALIGNMENT_STICKY     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_SFI4_SFI4_STATUS_SFI4_STATUS_STICKY_RX_ALIGNMENT_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Synchronization status change in lane i (i = 0...3, one bit per lane)
 *
 * \details 
 * 0: No sync change occurred
 * 1: Synchronization change detected in lane i
 * Bit is cleared by writing a 1 to this position
 *
 * Field: ::VTSS_SFI4_SFI4_STATUS_SFI4_STATUS_STICKY . RX_SYNC_STICKY
 */
#define  VTSS_F_SFI4_SFI4_STATUS_SFI4_STATUS_STICKY_RX_SYNC_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_SFI4_SFI4_STATUS_SFI4_STATUS_STICKY_RX_SYNC_STICKY     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_SFI4_SFI4_STATUS_SFI4_STATUS_STICKY_RX_SYNC_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief SFI4.2 status change/error interrupt mask
 *
 * \details
 * Mask bits for interrupt sources in SFI4_STATUS_STICKY register
 *
 * Register: \a SFI4::SFI4_STATUS::SFI4_ERR_INTR_MASK
 *
 * @param target A \a ::vtss_target_SFI4_e target
 */
#define VTSS_SFI4_SFI4_STATUS_SFI4_ERR_INTR_MASK(target)  VTSS_IOREG(target,0x3)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the corresponding sticky bit to be an interrupt source
 * 1: Enables the corresponding sticky bit to be an interrupt source
 *
 * Field: ::VTSS_SFI4_SFI4_STATUS_SFI4_ERR_INTR_MASK . RX_ALIGNMENT_MASK
 */
#define  VTSS_F_SFI4_SFI4_STATUS_SFI4_ERR_INTR_MASK_RX_ALIGNMENT_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_SFI4_SFI4_STATUS_SFI4_ERR_INTR_MASK_RX_ALIGNMENT_MASK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_SFI4_SFI4_STATUS_SFI4_ERR_INTR_MASK_RX_ALIGNMENT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Interrupt mask bit for the corresponding sticky bit
 *
 * \details 
 * 0: Disables the corresponding sticky bit to be an interrupt source
 * 1: Enables the corresponding sticky bit to be an interrupt source
 *
 * Field: ::VTSS_SFI4_SFI4_STATUS_SFI4_ERR_INTR_MASK . RX_SYNC_MASK
 */
#define  VTSS_F_SFI4_SFI4_STATUS_SFI4_ERR_INTR_MASK_RX_SYNC_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_SFI4_SFI4_STATUS_SFI4_ERR_INTR_MASK_RX_SYNC_MASK     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_SFI4_SFI4_STATUS_SFI4_ERR_INTR_MASK_RX_SYNC_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,4)

/**
 * Register Group: \a SFI4::SFI4_EXT_CONFIGURATION
 *
 * SFI4.2 extended configuration
 */


/** 
 * \brief SFI4.2 extended configuration
 *
 * \details
 * Special configuration bit groups for SFI4.2.
 *
 * Register: \a SFI4::SFI4_EXT_CONFIGURATION::SFI4_EXT_CFG
 *
 * @param target A \a ::vtss_target_SFI4_e target
 */
#define VTSS_SFI4_SFI4_EXT_CONFIGURATION_SFI4_EXT_CFG(target)  VTSS_IOREG(target,0x4)

/** 
 * \brief
 * Invert SerDes-Bus, invert all data signals from SERDES
 *
 * \details 
 * 0: Normal operation
 * 1: Inverted SerDes-bus
 *
 * Field: ::VTSS_SFI4_SFI4_EXT_CONFIGURATION_SFI4_EXT_CFG . RX_INVERT_SERDES_BUS
 */
#define  VTSS_F_SFI4_SFI4_EXT_CONFIGURATION_SFI4_EXT_CFG_RX_INVERT_SERDES_BUS(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_SFI4_SFI4_EXT_CONFIGURATION_SFI4_EXT_CFG_RX_INVERT_SERDES_BUS     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_SFI4_SFI4_EXT_CONFIGURATION_SFI4_EXT_CFG_RX_INVERT_SERDES_BUS(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Invert SerDes-Bus, invert all data signals to SERDES
 *
 * \details 
 * 0: Normal operation
 * 1: Inverted SerDes-bus
 *
 * Field: ::VTSS_SFI4_SFI4_EXT_CONFIGURATION_SFI4_EXT_CFG . TX_INVERT_SERDES_BUS
 */
#define  VTSS_F_SFI4_SFI4_EXT_CONFIGURATION_SFI4_EXT_CFG_TX_INVERT_SERDES_BUS(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_SFI4_SFI4_EXT_CONFIGURATION_SFI4_EXT_CFG_TX_INVERT_SERDES_BUS     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_SFI4_SFI4_EXT_CONFIGURATION_SFI4_EXT_CFG_TX_INVERT_SERDES_BUS(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/**
 * Register Group: \a SFI4::SFI4_SPARE
 *
 * Not documented
 */


#endif /* _VTSS_DAYTONA_REGS_SFI4_H_ */
