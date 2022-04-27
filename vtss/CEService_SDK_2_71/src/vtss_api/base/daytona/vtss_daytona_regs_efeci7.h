#ifndef _VTSS_DAYTONA_REGS_EFECI7_H_
#define _VTSS_DAYTONA_REGS_EFECI7_H_

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
 * Target: \a EFECI7
 *
 * \see vtss_target_EFECI7_e
 *
 * XCO2 EFEC 1.7 interface
 *
 ***********************************************************************/

/**
 * Register Group: \a EFECI7::EFECI7_CONTROL
 *
 * Control/status interface to XCO2 EFEC 1.7 macro
 */


/** 
 * \brief Control inputs to EFEC 1.7 modules
 *
 * \details
 * Register: \a EFECI7::EFECI7_CONTROL::EFECI7_MODE
 *
 * @param target A \a ::vtss_target_EFECI7_e target
 */
#define VTSS_EFECI7_EFECI7_CONTROL_EFECI7_MODE(target)  VTSS_IOREG(target,0x0)

/** 
 * \brief
 * Bit error rate in limit 
 * If, during a period of 32 frames (4,177,920 bits), the number of
 * corrected errors plus 5 times the count of uncorrectable row codewords
 * in the final stage exceeds BER_LIMIT, then the BER_ALARM output of the
 * EFECI7 is set to a 1.
 * A setting of 8356 represents (approximately) 2x10**-3 error rate, and
 * should be the default. BER_LIMIT is NOT expected to change after setup.
 *
 * \details 
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_MODE . BER_LIMIT
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_MODE_BER_LIMIT(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_MODE_BER_LIMIT     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_MODE_BER_LIMIT(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Disable decoding of row 3
 *
 * \details 
 * 0: Deasserted (default)
 * 1: Asserted
 *
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_MODE . DISABLE_ROW3_DEC
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_MODE_DISABLE_ROW3_DEC(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_MODE_DISABLE_ROW3_DEC     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_MODE_DISABLE_ROW3_DEC(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Disable decoding of column 2
 *
 * \details 
 * 0: Deasserted (default)
 * 1: Asserted
 *
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_MODE . DISABLE_COL2_DEC
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_MODE_DISABLE_COL2_DEC(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_MODE_DISABLE_COL2_DEC     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_MODE_DISABLE_COL2_DEC(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Disable decoding of row 2
 *
 * \details 
 * 0: Deasserted (default)
 * 1: Asserted
 *
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_MODE . DISABLE_ROW2_DEC
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_MODE_DISABLE_ROW2_DEC(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_MODE_DISABLE_ROW2_DEC     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_MODE_DISABLE_ROW2_DEC(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Disable decoding of column 1
 *
 * \details 
 * 0: Deasserted (default)
 * 1: Asserted
 *
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_MODE . DISABLE_COL1_DEC
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_MODE_DISABLE_COL1_DEC(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_MODE_DISABLE_COL1_DEC     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_MODE_DISABLE_COL1_DEC(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Disable decoding of row 1
 *
 * \details 
 * 0: Deasserted (default)
 * 1: Asserted
 *
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_MODE . DISABLE_ROW1_DEC
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_MODE_DISABLE_ROW1_DEC(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_MODE_DISABLE_ROW1_DEC     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_MODE_DISABLE_ROW1_DEC(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Set the (one-shot) XCOEFEC.I.7 Encoder i_err_insert input.
 * Note: The XCO2 EFEC 1.7 encoder detects and uses the asserting (rising)
 * edge of this input to generate an error.
 *
 * \details 
 * 0: Deasserted (default)
 * 1: Asserted
 *
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_MODE . TX_ERROR_INSERT
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_MODE_TX_ERROR_INSERT(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_MODE_TX_ERROR_INSERT     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_MODE_TX_ERROR_INSERT(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * SW_ENA input to daytona_clk_rst_sync modules (internal tx-clock, core tx
 * clock-input, internal rx-clock)
 *
 * \details 
 * 0: SW_ENA deasserted (default)
 * 1: SW_ENA asserted
 *
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_MODE . SW_ENA
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_MODE_SW_ENA(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_MODE_SW_ENA     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_MODE_SW_ENA(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * SW_RST input to daytona_clk_rst_sync modules (internal tx-clock, core tx
 * clock-input, internal rx-clock)
 *
 * \details 
 * 0: SW_RST deasserted
 * 1: SW_RST asserted (default)

 *
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_MODE . SW_RST
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_MODE_SW_RST(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_MODE_SW_RST     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_MODE_SW_RST(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief EFEC 1.7 sticky bits
 *
 * \details
 * Register: \a EFECI7::EFECI7_CONTROL::EFECI7_STICKY
 *
 * @param target A \a ::vtss_target_EFECI7_e target
 */
#define VTSS_EFECI7_EFECI7_CONTROL_EFECI7_STICKY(target)  VTSS_IOREG(target,0x1)

/** 
 * \brief
 * Out of MSYNC1 
 *
 * \details 
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_STICKY . OOMS1_STICKY
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_STICKY_OOMS1_STICKY(x)  VTSS_ENCODE_BITFIELD(x,24,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_STICKY_OOMS1_STICKY     VTSS_ENCODE_BITMASK(24,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_STICKY_OOMS1_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,24,1)

/** 
 * \brief
 * Unexpected MSYNC1 value
 *
 * \details 
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_STICKY . MSYNC1_UNEXPECTED_STICKY
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_STICKY_MSYNC1_UNEXPECTED_STICKY(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_STICKY_MSYNC1_UNEXPECTED_STICKY     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_STICKY_MSYNC1_UNEXPECTED_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \brief
 * Out of MSYNC0
 *
 * \details 
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_STICKY . OOMS0_STICKY
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_STICKY_OOMS0_STICKY(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_STICKY_OOMS0_STICKY     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_STICKY_OOMS0_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Unexpected MSYNC0 value
 *
 * \details 
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_STICKY . MSYNC0_UNEXPECTED_STICKY
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_STICKY_MSYNC0_UNEXPECTED_STICKY(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_STICKY_MSYNC0_UNEXPECTED_STICKY     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_STICKY_MSYNC0_UNEXPECTED_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * CRC error on MSYNC
 *
 * \details 
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_STICKY . MSYNC_CRC_ERROR_STICKY
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_STICKY_MSYNC_CRC_ERROR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_STICKY_MSYNC_CRC_ERROR_STICKY     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_STICKY_MSYNC_CRC_ERROR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Bit error rate alarm
 *
 * \details 
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_STICKY . BER_ALARM_STICKY
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_STICKY_BER_ALARM_STICKY(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_STICKY_BER_ALARM_STICKY     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_STICKY_BER_ALARM_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,4,1)


/** 
 * \brief EFEC 1.7 block mask registers
 *
 * \details
 * Register: \a EFECI7::EFECI7_CONTROL::EFECI7_MASK
 *
 * @param target A \a ::vtss_target_EFECI7_e target
 */
#define VTSS_EFECI7_EFECI7_CONTROL_EFECI7_MASK(target)  VTSS_IOREG(target,0x2)

/** 
 * \details 
 * 0: Disable (respective) Sticky from setting EFECI7_INT (default)
 * 1: Enable (respective) Sticky to set EFECI7
 *
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_MASK . OOMS1_MASK
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_MASK_OOMS1_MASK(x)  VTSS_ENCODE_BITFIELD(x,24,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_MASK_OOMS1_MASK     VTSS_ENCODE_BITMASK(24,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_MASK_OOMS1_MASK(x)  VTSS_EXTRACT_BITFIELD(x,24,1)

/** 
 * \details 
 * 0: Disable (respective) Sticky from setting EFECI7_INT (default)
 * 1: Enable (respective) Sticky to set EFECI7
 *
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_MASK . MSYNC1_UNEXPECTED_MASK
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_MASK_MSYNC1_UNEXPECTED_MASK(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_MASK_MSYNC1_UNEXPECTED_MASK     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_MASK_MSYNC1_UNEXPECTED_MASK(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \details 
 * 0: Disable (respective) Sticky from setting EFECI7_INT (default)
 * 1: Enable (respective) Sticky to set EFECI7
 *
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_MASK . OOMS0_MASK
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_MASK_OOMS0_MASK(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_MASK_OOMS0_MASK     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_MASK_OOMS0_MASK(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \details 
 * 0: Disable (respective) Sticky from setting EFECI7_INT (default)
 * 1: Enable (respective) Sticky to set EFECI7
 *
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_MASK . MSYNC0_UNEXPECTED_MASK
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_MASK_MSYNC0_UNEXPECTED_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_MASK_MSYNC0_UNEXPECTED_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_MASK_MSYNC0_UNEXPECTED_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \details 
 * 0: Disable (respective) sticky from setting EFECI7_INT (default)
 * 1: Enable (respective) sticky to set EFECI7
 *
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_MASK . MSYNC_CRC_ERROR_MASK
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_MASK_MSYNC_CRC_ERROR_MASK(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_MASK_MSYNC_CRC_ERROR_MASK     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_MASK_MSYNC_CRC_ERROR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \details 
 * 0: Disable (respective) sticky from setting EFECI7_INT (default)
 * 1: Enable (respective) sticky to set EFECI7
 *
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_MASK . BER_ALARM_MASK
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_MASK_BER_ALARM_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_MASK_BER_ALARM_MASK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_MASK_BER_ALARM_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)


/** 
 * \brief EFEC 1.7 status registers
 *
 * \details
 * Register: \a EFECI7::EFECI7_CONTROL::EFECI7_STATUS
 *
 * @param target A \a ::vtss_target_EFECI7_e target
 */
#define VTSS_EFECI7_EFECI7_CONTROL_EFECI7_STATUS(target)  VTSS_IOREG(target,0x3)

/** 
 * \brief
 * Out of MSYNC1 status
 *
 * \details 
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_STATUS . OOMS1_STATUS
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_STATUS_OOMS1_STATUS(x)  VTSS_ENCODE_BITFIELD(x,24,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_STATUS_OOMS1_STATUS     VTSS_ENCODE_BITMASK(24,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_STATUS_OOMS1_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,24,1)

/** 
 * \brief
 * Unexpected MSYNC1 status
 *
 * \details 
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_STATUS . MSYNC1_UNEXPECTED_STATUS
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_STATUS_MSYNC1_UNEXPECTED_STATUS(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_STATUS_MSYNC1_UNEXPECTED_STATUS     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_STATUS_MSYNC1_UNEXPECTED_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \brief
 * Out of MSYNC0 status
 *
 * \details 
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_STATUS . OOMS0_STATUS
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_STATUS_OOMS0_STATUS(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_STATUS_OOMS0_STATUS     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_STATUS_OOMS0_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Unexpected MSYNC0 status
 *
 * \details 
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_STATUS . MSYNC0_UNEXPECTED_STATUS
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_STATUS_MSYNC0_UNEXPECTED_STATUS(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_STATUS_MSYNC0_UNEXPECTED_STATUS     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_STATUS_MSYNC0_UNEXPECTED_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * MSYNC CRC error status
 *
 * \details 
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_STATUS . MSYNC_CRC_ERROR_STATUS
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_STATUS_MSYNC_CRC_ERROR_STATUS(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_STATUS_MSYNC_CRC_ERROR_STATUS     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_STATUS_MSYNC_CRC_ERROR_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Bit error rate alarm status
 *
 * \details 
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_STATUS . BER_ALARM_STATUS
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_STATUS_BER_ALARM_STATUS(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_STATUS_BER_ALARM_STATUS     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_STATUS_BER_ALARM_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * EFEC interrupt status
 *
 * \details 
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_STATUS . EFECI7_INTR_STATUS
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_STATUS_EFECI7_INTR_STATUS(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_STATUS_EFECI7_INTR_STATUS     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_STATUS_EFECI7_INTR_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Sum of COR_ZEROES
 *
 * \details
 * Summation of number of COR_ZEROES indicated by EFEC I.7 since last
 * PMTICK event.
 * To preclude overflow, this register will stop summation when this value
 * exceeds X"ffffff00".
 *
 * Register: \a EFECI7::EFECI7_CONTROL::COR_ZEROES
 *
 * @param target A \a ::vtss_target_EFECI7_e target
 */
#define VTSS_EFECI7_EFECI7_CONTROL_COR_ZEROES(target)  VTSS_IOREG(target,0x4)

/** 
 * \brief
 * Corrected 0s counter.
 * Sum of corrected 0s since the last PMTICK event.
 *
 * \details 
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_COR_ZEROES . COR_ZEROES
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_COR_ZEROES_COR_ZEROES(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_COR_ZEROES_COR_ZEROES     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_COR_ZEROES_COR_ZEROES(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Sum of COR_ONES
 *
 * \details
 * Summation of number of COR_ONES indicated by EFEC I.7 since last PMTICK
 * event.
 * To preclude overflow, this register will stop summation when this value
 * exceeds X"ffffff00".
 *
 * Register: \a EFECI7::EFECI7_CONTROL::COR_ONES
 *
 * @param target A \a ::vtss_target_EFECI7_e target
 */
#define VTSS_EFECI7_EFECI7_CONTROL_COR_ONES(target)  VTSS_IOREG(target,0x5)

/** 
 * \brief
 * Corrected 1s counter.
 * Sum of corrected 1s since the last PMTICK event.
 *
 * \details 
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_COR_ONES . COR_ONES
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_COR_ONES_COR_ONES(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_COR_ONES_COR_ONES     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_COR_ONES_COR_ONES(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Sum of UNCORR_ERRORS_ROW1
 *
 * \details
 * Summation of number of UNCORR_ERRORS_ROW1 indicated by EFEC 1.7 since
 * last PMTICK event.
 * To preclude overflow, this register will stop summation when this value
 * exceeds X"ffffffe0".
 *
 * Register: \a EFECI7::EFECI7_CONTROL::UNCOR_ERR_ROW1
 *
 * @param target A \a ::vtss_target_EFECI7_e target
 */
#define VTSS_EFECI7_EFECI7_CONTROL_UNCOR_ERR_ROW1(target)  VTSS_IOREG(target,0x6)

/** 
 * \brief
 * Uncorrected row1 error counter.
 * Sum of uncorrected row1 errors since the last PMTICK event.
 *
 * \details 
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_UNCOR_ERR_ROW1 . UNCOR_ERR_ROW1
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_UNCOR_ERR_ROW1_UNCOR_ERR_ROW1(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_UNCOR_ERR_ROW1_UNCOR_ERR_ROW1     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_UNCOR_ERR_ROW1_UNCOR_ERR_ROW1(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Sum of UNCORR_ERRORS_ROW2
 *
 * \details
 * Summation of number of UNCORR_ERRORS_ROW2 indicated by EFEC 1.7 since
 * last PMTICK event.
 * To preclude overflow, this register will stop summation when this value
 * exceeds X"ffffffe0".
 *
 * Register: \a EFECI7::EFECI7_CONTROL::UNCOR_ERR_ROW2
 *
 * @param target A \a ::vtss_target_EFECI7_e target
 */
#define VTSS_EFECI7_EFECI7_CONTROL_UNCOR_ERR_ROW2(target)  VTSS_IOREG(target,0x7)

/** 
 * \brief
 * Uncorrected row2 error counter.
 * Sum of uncorrected row2 errors since the last PMTICK event.
 *
 * \details 
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_UNCOR_ERR_ROW2 . UNCOR_ERR_ROW2
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_UNCOR_ERR_ROW2_UNCOR_ERR_ROW2(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_UNCOR_ERR_ROW2_UNCOR_ERR_ROW2     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_UNCOR_ERR_ROW2_UNCOR_ERR_ROW2(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Sum of UNCORR_ERRORS_ROW3
 *
 * \details
 * Summation of number of UNCORR_ERRORS_ROW3 indicated by EFEC 1.7 since
 * last PMTICK event.
 * To preclude overflow, this register will stop summation when this value
 * exceeds X"ffffffe0".
 *
 * Register: \a EFECI7::EFECI7_CONTROL::UNCOR_ERR_ROW3
 *
 * @param target A \a ::vtss_target_EFECI7_e target
 */
#define VTSS_EFECI7_EFECI7_CONTROL_UNCOR_ERR_ROW3(target)  VTSS_IOREG(target,0x8)

/** 
 * \brief
 * Uncorrected row3 error counter.
 * Sum of uncorrected row3 errors since the last PMTICK event.
 *
 * \details 
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_UNCOR_ERR_ROW3 . UNCOR_ERR_ROW3
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_UNCOR_ERR_ROW3_UNCOR_ERR_ROW3(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_UNCOR_ERR_ROW3_UNCOR_ERR_ROW3     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_UNCOR_ERR_ROW3_UNCOR_ERR_ROW3(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief EFEC I.7 spare register
 *
 * \details
 * Register: \a EFECI7::EFECI7_CONTROL::EFECI7_SPARE
 *
 * @param target A \a ::vtss_target_EFECI7_e target
 */
#define VTSS_EFECI7_EFECI7_CONTROL_EFECI7_SPARE(target)  VTSS_IOREG(target,0x9)

/** 
 * \brief
 * Spare register
 *
 * \details 
 * Field: ::VTSS_EFECI7_EFECI7_CONTROL_EFECI7_SPARE . EFECI7_REG
 */
#define  VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_SPARE_EFECI7_REG(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_SPARE_EFECI7_REG     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_EFECI7_EFECI7_CONTROL_EFECI7_SPARE_EFECI7_REG(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


#endif /* _VTSS_DAYTONA_REGS_EFECI7_H_ */
