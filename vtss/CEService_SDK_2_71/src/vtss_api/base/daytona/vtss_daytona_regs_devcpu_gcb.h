#ifndef _VTSS_DAYTONA_REGS_DEVCPU_GCB_H_
#define _VTSS_DAYTONA_REGS_DEVCPU_GCB_H_

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
 * Target: \a DEVCPU_GCB
 *
 * General configuration block.
 *
 ***********************************************************************/

/**
 * Register Group: \a DEVCPU_GCB::CHIP_REGS
 *
 * Miscellaneous registers
 */


/** 
 * \brief General purpose register 0
 *
 * \details
 * Register: \a DEVCPU_GCB::CHIP_REGS::GENERAL_PURPOSE_0
 */
#define VTSS_DEVCPU_GCB_CHIP_REGS_GENERAL_PURPOSE_0  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x0)

/** 
 * \brief
 * This is a general-purpose register that can be used for testing. The
 * value in this register has no functionality. Values written can be read
 * out.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_GENERAL_PURPOSE_0 . GENERAL_PURPOSE_REG_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_GENERAL_PURPOSE_0_GENERAL_PURPOSE_REG_0(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_GENERAL_PURPOSE_0_GENERAL_PURPOSE_REG_0     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_GENERAL_PURPOSE_0_GENERAL_PURPOSE_REG_0(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief General purpose register 1
 *
 * \details
 * Register: \a DEVCPU_GCB::CHIP_REGS::GENERAL_PURPOSE_1
 */
#define VTSS_DEVCPU_GCB_CHIP_REGS_GENERAL_PURPOSE_1  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x1)

/** 
 * \brief
 * This is a general-purpose register that can be used for testing. The
 * value in this register has no functionality. Values written can be read
 * out.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_GENERAL_PURPOSE_1 . GENERAL_PURPOSE_REG_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_GENERAL_PURPOSE_1_GENERAL_PURPOSE_REG_1(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_GENERAL_PURPOSE_1_GENERAL_PURPOSE_REG_1     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_GENERAL_PURPOSE_1_GENERAL_PURPOSE_REG_1(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief General purpose register 2
 *
 * \details
 * Register: \a DEVCPU_GCB::CHIP_REGS::GENERAL_PURPOSE_2
 */
#define VTSS_DEVCPU_GCB_CHIP_REGS_GENERAL_PURPOSE_2  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x2)

/** 
 * \brief
 * This is a general-purpose register that can be used for testing. The
 * value in this register has no functionality. Values written can be read
 * out.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_GENERAL_PURPOSE_2 . GENERAL_PURPOSE_REG_2
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_GENERAL_PURPOSE_2_GENERAL_PURPOSE_REG_2(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_GENERAL_PURPOSE_2_GENERAL_PURPOSE_REG_2     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_GENERAL_PURPOSE_2_GENERAL_PURPOSE_REG_2(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief General purpose register 3
 *
 * \details
 * Register: \a DEVCPU_GCB::CHIP_REGS::GENERAL_PURPOSE_3
 */
#define VTSS_DEVCPU_GCB_CHIP_REGS_GENERAL_PURPOSE_3  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x3)

/** 
 * \brief
 * This is a general-purpose register that can be used for testing. The
 * value in this register has no functionality. Values written can be read
 * out.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_GENERAL_PURPOSE_3 . GENERAL_PURPOSE_REG_3
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_GENERAL_PURPOSE_3_GENERAL_PURPOSE_REG_3(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_GENERAL_PURPOSE_3_GENERAL_PURPOSE_REG_3     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_GENERAL_PURPOSE_3_GENERAL_PURPOSE_REG_3(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Chip ID
 *
 * \details
 * Register: \a DEVCPU_GCB::CHIP_REGS::CHIP_ID
 */
#define VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_ID    VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x4)

/** 
 * \brief
 * Revision ID
 * Device revision starting from 1
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_ID . REV_ID
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_CHIP_ID_REV_ID(x)  VTSS_ENCODE_BITFIELD(x,28,4)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_CHIP_ID_REV_ID     VTSS_ENCODE_BITMASK(28,4)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_CHIP_ID_REV_ID(x)  VTSS_EXTRACT_BITFIELD(x,28,4)

/** 
 * \brief
 * Part ID
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_ID . PART_ID
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_CHIP_ID_PART_ID(x)  VTSS_ENCODE_BITFIELD(x,12,16)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_CHIP_ID_PART_ID     VTSS_ENCODE_BITMASK(12,16)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_CHIP_ID_PART_ID(x)  VTSS_EXTRACT_BITFIELD(x,12,16)

/** 
 * \brief
 * Manufacturer ID
 * Returns the value 0x74
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_ID . MFG_ID
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_CHIP_ID_MFG_ID(x)  VTSS_ENCODE_BITFIELD(x,1,11)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_CHIP_ID_MFG_ID     VTSS_ENCODE_BITMASK(1,11)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_CHIP_ID_MFG_ID(x)  VTSS_EXTRACT_BITFIELD(x,1,11)

/** 
 * \brief
 * Returns 1
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_ID . ONE
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_CHIP_ID_ONE(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_CHIP_ID_ONE     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_CHIP_ID_ONE(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Chip mode register
 *
 * \details
 * Register: \a DEVCPU_GCB::CHIP_REGS::CHIP_MODE
 */
#define VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_MODE  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x5)

/** 
 * \brief
 * Indicates which of two dies this chip is
 *
 * \details 
 * 0: Die 0, channels 0 & 1
 * 1: Die 1, channels 2 & 3

 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_MODE . DIE_SEL
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_CHIP_MODE_DIE_SEL(x)  VTSS_ENCODE_BITFIELD(x,28,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_CHIP_MODE_DIE_SEL     VTSS_ENCODE_BITMASK(28,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_CHIP_MODE_DIE_SEL(x)  VTSS_EXTRACT_BITFIELD(x,28,1)

/** 
 * \brief
 * Variant select
 * Indicates the variant of the device
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_MODE . VAR_SEL
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_CHIP_MODE_VAR_SEL(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_CHIP_MODE_VAR_SEL     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_CHIP_MODE_VAR_SEL(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief PMTICK control register, channel 0
 *
 * \details
 * Register: \a DEVCPU_GCB::CHIP_REGS::PMTICK_CTL_0
 */
#define VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_0  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x6)

/** 
 * \brief
 * Writing a 1 to this bit forces a PMTICK_0 event. It is automatically
 * cleared to 0.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_0 . PMTICK_FRC_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_0_PMTICK_FRC_0(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_0_PMTICK_FRC_0     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_0_PMTICK_FRC_0(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * When the source of PMTICK_0 is the internal counter, this bit can be
 * used to drive PMTICK_0 to the EXT_PMTICK pin.
 *
 * \details 
 * 0: Disable drive
 * 1: Enable drive

 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_0 . PMTICK_DRV_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_0_PMTICK_DRV_0(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_0_PMTICK_DRV_0     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_0_PMTICK_DRV_0(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Selects the source of the PMTICK_0 pulse to be either from the internal
 * PMTICK_0 counter or from an external pin.
 *
 * \details 
 * 0: Internal counter
 * 1: External pin
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_0 . PMTICK_SRC_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_0_PMTICK_SRC_0(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_0_PMTICK_SRC_0     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_0_PMTICK_SRC_0(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * PMTICK_0 pulse enable.
 *
 * \details 
 * 0: Disabled
 * 1: Enabled
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_0 . PMTICK_ENA_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_0_PMTICK_ENA_0(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_0_PMTICK_ENA_0     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_0_PMTICK_ENA_0(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief PMTICK period register, channel 0
 *
 * \details
 * Register: \a DEVCPU_GCB::CHIP_REGS::PMTICK_PERIOD_0
 */
#define VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_PERIOD_0  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x7)

/** 
 * \brief
 * Configures the period of PMTICK_0 when the internal counter is selected.
 * REFCLK input divided by 1024 is used to clock the internal counter.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_PERIOD_0 . PMTICK_PERIOD_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_PERIOD_0_PMTICK_PERIOD_0(x)  VTSS_ENCODE_BITFIELD(x,0,20)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_PMTICK_PERIOD_0_PMTICK_PERIOD_0     VTSS_ENCODE_BITMASK(0,20)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_PMTICK_PERIOD_0_PMTICK_PERIOD_0(x)  VTSS_EXTRACT_BITFIELD(x,0,20)


/** 
 * \brief PMTICK interrupt register, channel 0
 *
 * \details
 * Register: \a DEVCPU_GCB::CHIP_REGS::PMTICK_INT_0
 */
#define VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_INT_0  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x8)

/** 
 * \brief
 * Indicates whether a PMTICK event has occurred
 *
 * \details 
 * 1: A PMTICK event has occurred
 * 0: No PMTICK event
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_INT_0 . PMTICK_STICKY_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_INT_0_PMTICK_STICKY_0(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_PMTICK_INT_0_PMTICK_STICKY_0     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_PMTICK_INT_0_PMTICK_STICKY_0(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief PMTICK interrupt mask register, channel 0
 *
 * \details
 * Register: \a DEVCPU_GCB::CHIP_REGS::PMTICK_INT_MASK_0
 */
#define VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_INT_MASK_0  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x9)

/** 
 * \brief
 * Interrupt mask bit for PMTICK_STICKY_0
 *
 * \details 
 * 0: Interrupt masked (not enabled)
 * 1: Interrupt not masked (enabled)
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_INT_MASK_0 . PMTICK_INT_MASK_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_INT_MASK_0_PMTICK_INT_MASK_0(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_PMTICK_INT_MASK_0_PMTICK_INT_MASK_0     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_PMTICK_INT_MASK_0_PMTICK_INT_MASK_0(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief PMTICK control register, channel 1
 *
 * \details
 * Register: \a DEVCPU_GCB::CHIP_REGS::PMTICK_CTL_1
 */
#define VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_1  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0xa)

/** 
 * \brief
 * Writing a 1 to this bit forces a PMTICK_1 event. It is automatically
 * cleared to 0.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_1 . PMTICK_FRC_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_1_PMTICK_FRC_1(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_1_PMTICK_FRC_1     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_1_PMTICK_FRC_1(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * When the source of PMTICK_1 is the internal counter, this bit can be
 * used to drive PMTICK_1 to the external PMTICK pin. If both PMTICK_DRV_0
 * and PMTICK_DRV_1 are set to 1, the PMTICK pulse from channel 0 will be
 * selected.
 *
 * \details 
 * 0: Disable drive
 * 1: Enable drive
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_1 . PMTICK_DRV_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_1_PMTICK_DRV_1(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_1_PMTICK_DRV_1     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_1_PMTICK_DRV_1(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Selects the source of the PMTICK_1 pulse to be either from the internal
 * PMTICK_1 counter or from an external pin.
 *
 * \details 
 * 0: Internal counter
 * 1: External pin

 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_1 . PMTICK_SRC_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_1_PMTICK_SRC_1(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_1_PMTICK_SRC_1     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_1_PMTICK_SRC_1(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * PMTICK_1 pulse enable
 *
 * \details 
 * 0: Disabled
 * 1: Enabled
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_1 . PMTICK_ENA_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_1_PMTICK_ENA_1(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_1_PMTICK_ENA_1     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_PMTICK_CTL_1_PMTICK_ENA_1(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief PMTICK period register, channel 1
 *
 * \details
 * Register: \a DEVCPU_GCB::CHIP_REGS::PMTICK_PERIOD_1
 */
#define VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_PERIOD_1  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0xb)

/** 
 * \brief
 * Configures the period of PMTICK_1 when the internal counter is selected.
 * REFCLK input divided by 1024 is used to clock the internal counter.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_PERIOD_1 . PMTICK_PERIOD_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_PERIOD_1_PMTICK_PERIOD_1(x)  VTSS_ENCODE_BITFIELD(x,0,20)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_PMTICK_PERIOD_1_PMTICK_PERIOD_1     VTSS_ENCODE_BITMASK(0,20)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_PMTICK_PERIOD_1_PMTICK_PERIOD_1(x)  VTSS_EXTRACT_BITFIELD(x,0,20)


/** 
 * \brief PMTICK interrupt register, channel 1
 *
 * \details
 * Register: \a DEVCPU_GCB::CHIP_REGS::PMTICK_INT_1
 */
#define VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_INT_1  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0xc)

/** 
 * \brief
 * Indicates whether a PMTICK event has occurred
 *
 * \details 
 * 1: A PMTICK event has occurred
 * 0: No PMTICK event
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_INT_1 . PMTICK_STICKY_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_INT_1_PMTICK_STICKY_1(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_PMTICK_INT_1_PMTICK_STICKY_1     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_PMTICK_INT_1_PMTICK_STICKY_1(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief PMTICK interrupt mask register, channel 1
 *
 * \details
 * Register: \a DEVCPU_GCB::CHIP_REGS::PMTICK_INT_MASK_1
 */
#define VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_INT_MASK_1  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0xd)

/** 
 * \brief
 * Interrupt mask bit for PMTICK_STICKY_1
 *
 * \details 
 * 0: Interrupt masked (not enabled).
 * 1: Interrupt not masked (enabled)
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_PMTICK_INT_MASK_1 . PMTICK_INT_MASK_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_PMTICK_INT_MASK_1_PMTICK_INT_MASK_1(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_PMTICK_INT_MASK_1_PMTICK_INT_MASK_1     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_PMTICK_INT_MASK_1_PMTICK_INT_MASK_1(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/**
 * Register Group: \a DEVCPU_GCB::GPIO
 *
 * General purpose I/O pin registers
 */


/** 
 * \brief GPIO pin direction
 *
 * \details
 * Register: \a DEVCPU_GCB::GPIO::GPIO_OUTPUT_ENA
 */
#define VTSS_DEVCPU_GCB_GPIO_GPIO_OUTPUT_ENA  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x20)

/** 
 * \brief
 * Indicates the direction of the GPIO pins
 *
 * \details 
 * 0: Input
 * 1: Output
 *
 * Field: ::VTSS_DEVCPU_GCB_GPIO_GPIO_OUTPUT_ENA . G_OE
 */
#define  VTSS_F_DEVCPU_GCB_GPIO_GPIO_OUTPUT_ENA_G_OE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DEVCPU_GCB_GPIO_GPIO_OUTPUT_ENA_G_OE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DEVCPU_GCB_GPIO_GPIO_OUTPUT_ENA_G_OE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GPIO data out
 *
 * \details
 * Register: \a DEVCPU_GCB::GPIO::GPIO_O
 */
#define VTSS_DEVCPU_GCB_GPIO_GPIO_O          VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x21)

/** 
 * \brief
 * GPIO output register
 * The following bits can be used for the following purposes:
 * 6: CH0 OTN1 Tx OH port MFAS bit 
 * 7: CH0 OTN1 Rx OH port MFAS bit
 * 14: CH0 OTN1 Tx OH port MFAS bit
 * 15: CH0 OTN1 Rx OH port MFAS bit
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_GPIO_GPIO_O . G_OUT
 */
#define  VTSS_F_DEVCPU_GCB_GPIO_GPIO_O_G_OUT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DEVCPU_GCB_GPIO_GPIO_O_G_OUT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DEVCPU_GCB_GPIO_GPIO_O_G_OUT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GPIO change
 *
 * \details
 * Register: \a DEVCPU_GCB::GPIO::GPIO_DELTA
 */
#define VTSS_DEVCPU_GCB_GPIO_GPIO_DELTA      VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x22)

/** 
 * \brief
 * Indicates whether a GPIO input or output pin has changed state since
 * last clear (writing of 1's) of the delta bit
 *
 * \details 
 * 0: No change
 * 1: Change
 *
 * Field: ::VTSS_DEVCPU_GCB_GPIO_GPIO_DELTA . GPIO_DELTA
 */
#define  VTSS_F_DEVCPU_GCB_GPIO_GPIO_DELTA_GPIO_DELTA(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DEVCPU_GCB_GPIO_GPIO_DELTA_GPIO_DELTA     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DEVCPU_GCB_GPIO_GPIO_DELTA_GPIO_DELTA(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GPIO interrupt mask
 *
 * \details
 * Register: \a DEVCPU_GCB::GPIO::GPIO_INT_MASK
 */
#define VTSS_DEVCPU_GCB_GPIO_GPIO_INT_MASK   VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x23)

/** 
 * \brief
 * Indicates whether a GPIO sticky bit will generate an interrupt
 *
 * \details 
 * 0: Interrupt masked (not enabled)
 * 1: Interrupt not masked (enabled)
 *
 * Field: ::VTSS_DEVCPU_GCB_GPIO_GPIO_INT_MASK . GPIO_INT_MASK
 */
#define  VTSS_F_DEVCPU_GCB_GPIO_GPIO_INT_MASK_GPIO_INT_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DEVCPU_GCB_GPIO_GPIO_INT_MASK_GPIO_INT_MASK     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DEVCPU_GCB_GPIO_GPIO_INT_MASK_GPIO_INT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief GPIO data in
 *
 * \details
 * Register: \a DEVCPU_GCB::GPIO::GPIO_I
 */
#define VTSS_DEVCPU_GCB_GPIO_GPIO_I          VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x24)

/** 
 * \brief
 * GPIO input register
 * The following bits can be used for the following purposes:
 * 3: CXFI0 lane B LOS input 
 * 4: LXFI0 lane B LOS input 
 * 5: CH0 OTN1 Tx OH port mask bit
 * 11: CXFI0 lane B LOS input
 * 12: LXFI0 lane B LOS input
 * 13: CH1 OTN1 Tx OH port mask bit
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_GPIO_GPIO_I . G_IN
 */
#define  VTSS_F_DEVCPU_GCB_GPIO_GPIO_I_G_IN(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DEVCPU_GCB_GPIO_GPIO_I_G_IN     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DEVCPU_GCB_GPIO_GPIO_I_G_IN(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a DEVCPU_GCB::DEVCPU_RST_REGS
 *
 * DFT and reset registers
 */


/** 
 * \brief DFT and reset registers
 *
 * \details
 * Register: \a DEVCPU_GCB::DEVCPU_RST_REGS::DEVCPU_RST_REGS
 */
#define VTSS_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x40)

/** 
 * \brief
 * Reset for channel 1
 * Reset channel logic and restores configuration and counter values to
 * their default state
 *
 * \details 
 * 0: Normal operation
 * 1: Reset the channel
 *
 * Field: ::VTSS_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS . CHANNEL_RST_1
 */
#define  VTSS_F_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_CHANNEL_RST_1(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_CHANNEL_RST_1     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_CHANNEL_RST_1(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Reset for channel 0
 * Reset channel logic and restores configuration and counter values to
 * their default state
 *
 * \details 
 * 0: Normal operation
 * 1: Reset the channel
 *
 * Field: ::VTSS_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS . CHANNEL_RST_0
 */
#define  VTSS_F_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_CHANNEL_RST_0(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_CHANNEL_RST_0     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_CHANNEL_RST_0(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Resets the whole device
 * Returns to 0 automatically when the device reset is complete.
 * Note: The device will not respond with a PI_RDY_N assertion after the
 * write because the parallel configuration interface is being reset along
 * with the rest of the device.
 * This reset completes within 400 ns of being received.
 *
 * \details 
 * 0: No action
 * 1: Reset device
 *
 * Field: ::VTSS_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS . SOFT_CHIP_RST
 */
#define  VTSS_F_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_SOFT_CHIP_RST(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_SOFT_CHIP_RST     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_DEVCPU_GCB_DEVCPU_RST_REGS_DEVCPU_RST_REGS_SOFT_CHIP_RST(x)  VTSS_EXTRACT_BITFIELD(x,1,1)


/** 
 * \brief BIST logic soft reset
 *
 * \details
 * Register: \a DEVCPU_GCB::DEVCPU_RST_REGS::SOFT_DEVCPU_RST
 */
#define VTSS_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_DEVCPU_RST  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x41)

/** 
 * \brief
 * Resets the MIIM logic
 *
 * \details 
 * 0: No action
 * 1: Resets logic
 *
 * Field: ::VTSS_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_DEVCPU_RST . SOFT_MISC_RST
 */
#define  VTSS_F_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_DEVCPU_RST_SOFT_MISC_RST(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_DEVCPU_RST_SOFT_MISC_RST     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_DEVCPU_RST_SOFT_MISC_RST(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/**
 * Register Group: \a DEVCPU_GCB::ALIGN
 *
 * Alignment Block Register Group
 */

/**
 * Register Group: \a DEVCPU_GCB::MIIM
 *
 * MIIM register group
 */


/** 
 * \brief MIIM status
 *
 * \details
 * Register: \a DEVCPU_GCB::MIIM::MIIM_STATUS
 */
#define VTSS_DEVCPU_GCB_MIIM_MIIM_STATUS     VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x80)

/** 
 * \brief
 * Indicates whether a read or write operation via the MIIM interface is in
 * progress.
 *
 * \details 
 * 0: No read or write operation in progress
 * 1: A read or a write operation is in progress
 *
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MIIM_STATUS . MIIM_STAT_BUSY
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MIIM_STATUS_MIIM_STAT_BUSY(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_DEVCPU_GCB_MIIM_MIIM_STATUS_MIIM_STAT_BUSY     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_DEVCPU_GCB_MIIM_MIIM_STATUS_MIIM_STAT_BUSY(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Indicates whether a read operation via the MIIM interface is in
 * progress.
 *
 * \details 
 * 0: Read not in progress
 * 1: Read in progress.
 *
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MIIM_STATUS . MIIM_STAT_PENDING_RD
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MIIM_STATUS_MIIM_STAT_PENDING_RD(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_DEVCPU_GCB_MIIM_MIIM_STATUS_MIIM_STAT_PENDING_RD     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_DEVCPU_GCB_MIIM_MIIM_STATUS_MIIM_STAT_PENDING_RD(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Indicates whether a write operation via the MIIM interface is in
 * progress or not.
 *
 * \details 
 * 0: Write not in progress
 * 1: Write in progress.
 *
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MIIM_STATUS . MIIM_STAT_PENDING_WR
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MIIM_STATUS_MIIM_STAT_PENDING_WR(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_DEVCPU_GCB_MIIM_MIIM_STATUS_MIIM_STAT_PENDING_WR     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_DEVCPU_GCB_MIIM_MIIM_STATUS_MIIM_STAT_PENDING_WR(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief MIIM command
 *
 * \details
 * Register: \a DEVCPU_GCB::MIIM::MIIM_CMD
 */
#define VTSS_DEVCPU_GCB_MIIM_MIIM_CMD        VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x82)

/** 
 * \brief
 * Must be set for starting a new PHY access. This bit is automatically
 * cleared.
 *
 * \details 
 * 0: Write to this register is ignored
 * 1: Write to this register is processed
 *
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MIIM_CMD . MIIM_CMD_VLD
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_VLD(x)  VTSS_ENCODE_BITFIELD(x,31,1)
#define  VTSS_M_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_VLD     VTSS_ENCODE_BITMASK(31,1)
#define  VTSS_X_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_VLD(x)  VTSS_EXTRACT_BITFIELD(x,31,1)

/** 
 * \brief
 * Indicates the addressed PHY number.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MIIM_CMD . MIIM_CMD_PHYAD
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_PHYAD(x)  VTSS_ENCODE_BITFIELD(x,24,5)
#define  VTSS_M_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_PHYAD     VTSS_ENCODE_BITMASK(24,5)
#define  VTSS_X_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_PHYAD(x)  VTSS_EXTRACT_BITFIELD(x,24,5)

/** 
 * \brief
 * Indicates the address of the register within the PHY that will be
 * accessed.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MIIM_CMD . MIIM_CMD_REGAD
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_REGAD(x)  VTSS_ENCODE_BITFIELD(x,19,5)
#define  VTSS_M_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_REGAD     VTSS_ENCODE_BITMASK(19,5)
#define  VTSS_X_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_REGAD(x)  VTSS_EXTRACT_BITFIELD(x,19,5)

/** 
 * \brief
 * Indicates whether automatic scanning of PHY registers is enabled. When
 * enabled, the PHY-number for each automatic read is continuously
 * round-robined from PHY_ADDR_LOW through PHY_ADDR_HIGH. This function is
 * started upon a read operation (ACCESS_TYPE).
 *
 * \details 
 * 0: Disabled
 * 1: Enabled.
 *
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MIIM_CMD . MIIM_CMD_SCAN
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_SCAN(x)  VTSS_ENCODE_BITFIELD(x,18,1)
#define  VTSS_M_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_SCAN     VTSS_ENCODE_BITMASK(18,1)
#define  VTSS_X_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_SCAN(x)  VTSS_EXTRACT_BITFIELD(x,18,1)

/** 
 * \brief
 * Data to be written in the PHY register.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MIIM_CMD . MIIM_CMD_WRDATA
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_WRDATA(x)  VTSS_ENCODE_BITFIELD(x,2,16)
#define  VTSS_M_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_WRDATA     VTSS_ENCODE_BITMASK(2,16)
#define  VTSS_X_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_WRDATA(x)  VTSS_EXTRACT_BITFIELD(x,2,16)

/** 
 * \brief
 * Indicates type of operation.
 *
 * \details 
 * 0: Address (for 802.3 clause 45 access)
 * 1: Write (for 802.3 clause 22 or 45 access)
 * 2: Read increment (for 802.3 clause 45 access)
 *     Read (for 802.3 clause 22 access)
 * 3: Read (for 802.3 clause 45 access)
 *
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MIIM_CMD . MIIM_CMD_OPR_FIELD
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_OPR_FIELD(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_OPR_FIELD     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_DEVCPU_GCB_MIIM_MIIM_CMD_MIIM_CMD_OPR_FIELD(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief MIIM reply data
 *
 * \details
 * Register: \a DEVCPU_GCB::MIIM::MIIM_DATA
 */
#define VTSS_DEVCPU_GCB_MIIM_MIIM_DATA       VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x83)

/** 
 * \brief
 * Indicates whether a read operation failed or succeeded
 *
 * \details 
 * 0: OK
 * 3: Error
 *
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MIIM_DATA . MIIM_DATA_SUCCESS
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MIIM_DATA_MIIM_DATA_SUCCESS(x)  VTSS_ENCODE_BITFIELD(x,16,2)
#define  VTSS_M_DEVCPU_GCB_MIIM_MIIM_DATA_MIIM_DATA_SUCCESS     VTSS_ENCODE_BITMASK(16,2)
#define  VTSS_X_DEVCPU_GCB_MIIM_MIIM_DATA_MIIM_DATA_SUCCESS(x)  VTSS_EXTRACT_BITFIELD(x,16,2)

/** 
 * \brief
 * Data read from PHY register
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MIIM_DATA . MIIM_DATA_RDDATA
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MIIM_DATA_MIIM_DATA_RDDATA(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DEVCPU_GCB_MIIM_MIIM_DATA_MIIM_DATA_RDDATA     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DEVCPU_GCB_MIIM_MIIM_DATA_MIIM_DATA_RDDATA(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief MIIM configuration
 *
 * \details
 * Register: \a DEVCPU_GCB::MIIM::MIIM_CFG
 */
#define VTSS_DEVCPU_GCB_MIIM_MIIM_CFG        VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x84)

/** 
 * \brief
 * The ST (start-of-frame) field of the MIIM frame format adopts the value
 * of this field.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MIIM_CFG . MIIM_ST_CFG_FIELD
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MIIM_CFG_MIIM_ST_CFG_FIELD(x)  VTSS_ENCODE_BITFIELD(x,8,2)
#define  VTSS_M_DEVCPU_GCB_MIIM_MIIM_CFG_MIIM_ST_CFG_FIELD     VTSS_ENCODE_BITMASK(8,2)
#define  VTSS_X_DEVCPU_GCB_MIIM_MIIM_CFG_MIIM_ST_CFG_FIELD(x)  VTSS_EXTRACT_BITFIELD(x,8,2)

/** 
 * \brief
 * Configures the MIIM clock frequency
 * This is computed as (REFCLK frequency * 2)/(2 * (1+X)), where X is the
 * value written to this register. With a 312 MHz CSR clock, this gives a
 * range 610 KHz to 156 MHz. The device, however, only supports a maximum
 * clock frequency of 12.5 MHz. This corresponds to a value of 12
 * programmed into the register.
 *
 * \details 
 * 0 - 11: Invalid
 * 12 - 255: Valid clock prescale value
 *
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MIIM_CFG . MIIM_CFG_PRESCALE
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MIIM_CFG_MIIM_CFG_PRESCALE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_DEVCPU_GCB_MIIM_MIIM_CFG_MIIM_CFG_PRESCALE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_DEVCPU_GCB_MIIM_MIIM_CFG_MIIM_CFG_PRESCALE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a DEVCPU_GCB::MIIM_READ_SCAN
 *
 * MIIM Read Scan Registers
 */

/**
 * Register Group: \a DEVCPU_GCB::CHIP_MUX
 *
 * Mode mux controls
 */


/** 
 * \brief Channel 0 data muxes 
 *
 * \details
 * Contains the top-level data mux controls for channel 0
 *
 * Register: \a DEVCPU_GCB::CHIP_MUX::DATA_MUX_0
 */
#define VTSS_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0xd0)

/** 
 * \brief
 * ROSI/TOSI mux control for channel 0
 * Selects the source/sink of the ROSI and TOSI pins on the device.
 *
 * \details 
 * 0: Select ROSI/TOSI from eWIS2 as source/sink of ROSI/TOSI pins of
 * channel 0
 * 1: Select OTN OH port from OTN1 as source/sink of ROSI/TOSI pins of
 * channel 0 (uses GPIO(7:5) as well)
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0 . ROSI_TOSI_MUX_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0_ROSI_TOSI_MUX_0(x)  VTSS_ENCODE_BITFIELD(x,24,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0_ROSI_TOSI_MUX_0     VTSS_ENCODE_BITMASK(24,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0_ROSI_TOSI_MUX_0(x)  VTSS_EXTRACT_BITFIELD(x,24,1)

/** 
 * \brief
 * OTN OH port mux control for channel 0
 * Selects the source/sink of the OTN OH port pins on the device.
 *
 * \details 
 * 0: Select OTN OH port from OTN2 as source/sink of OTN OH port pins of
 * channel 0
 * 1: Select OTN OH port from OTN1 as source/sink of OTN OH port pins of
 * channel 0
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0 . OTN_OH_MUX_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0_OTN_OH_MUX_0(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0_OTN_OH_MUX_0     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0_OTN_OH_MUX_0(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Client Rx mux control for channel 0
 * Selects the source of data for client side of channel 0 from either the
 * channel 0 UPI or channel 0 client XFI
 *
 * \details 
 * 0: Select UPI as source of channel 0 client
 * 1: Select client XFI as source of channel 0 client
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0 . CLIENT_RX_MUX_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0_CLIENT_RX_MUX_0(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0_CLIENT_RX_MUX_0     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0_CLIENT_RX_MUX_0(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Protection control mux for channel 0 Tx
 * Selects between channel 0 and channel 1 as data sink
 *
 * \details 
 * 0: Select UPI/client XFI 0 as sink of channel 0 client
 * 1: Select UPI/client XFI 1 as sink of channel 0 client
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0 . PROT_TX_MUX_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0_PROT_TX_MUX_0(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0_PROT_TX_MUX_0     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0_PROT_TX_MUX_0(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Protection control mux for channel 0 Rx
 * Selects between channel 0 and channel 1 as data source
 *
 * \details 
 * 0: Select UPI/client XFI 0 as source of channel 0 client
 * 1: Select UPI/client XFI 1 as source of channel 0 client
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0 . PROT_RX_MUX_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0_PROT_RX_MUX_0(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0_PROT_RX_MUX_0     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0_PROT_RX_MUX_0(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Channel 0 top clock muxes
 *
 * \details
 * Contains the top level clock mux controls for channel 0
 *
 * Register: \a DEVCPU_GCB::CHIP_MUX::TOP_CLK_MUX_0
 */
#define VTSS_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0xd1)

/** 
 * \brief
 * Selects the source of the LXFI reference clock for channel 0
 *
 * \details 
 * 0: LXFI_REFCLK_0 is source
 * 1: Reserved
 * 2: UPI_CXFI_REFCLK_0 is source
 * 3: REFCLK is source
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0 . LXFI_REFCLK_MUX_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0_LXFI_REFCLK_MUX_0(x)  VTSS_ENCODE_BITFIELD(x,28,2)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0_LXFI_REFCLK_MUX_0     VTSS_ENCODE_BITMASK(28,2)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0_LXFI_REFCLK_MUX_0(x)  VTSS_EXTRACT_BITFIELD(x,28,2)

/** 
 * \brief
 * Selects the source of the UPI_CXFI reference clock for channel 0
 *
 * \details 
 * 0: UPI_CXFI_REFCLK_0 is source
 * 1: REFCLK is source
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0 . UPI_CXFI_REFCLK_MUX_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0_UPI_CXFI_REFCLK_MUX_0(x)  VTSS_ENCODE_BITFIELD(x,24,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0_UPI_CXFI_REFCLK_MUX_0     VTSS_ENCODE_BITMASK(24,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0_UPI_CXFI_REFCLK_MUX_0(x)  VTSS_EXTRACT_BITFIELD(x,24,1)

/** 
 * \brief
 * Selects the source of the output LXFI recovered clock for channel 0
 *
 * \details 
 * 0: LXFI lane A is source
 * 1: LXFI lane B is source
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0 . LXFI_RCVDCLK_SEL_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0_LXFI_RCVDCLK_SEL_0(x)  VTSS_ENCODE_BITFIELD(x,22,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0_LXFI_RCVDCLK_SEL_0     VTSS_ENCODE_BITMASK(22,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0_LXFI_RCVDCLK_SEL_0(x)  VTSS_EXTRACT_BITFIELD(x,22,1)

/** 
 * \brief
 * Selects the source of the output CXFI recovered clock for channel 0
 *
 * \details 
 * 0: CXFI lane A is source
 * 1: CXFI lane B is source
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0 . CXFI_RCVDCLK_SEL_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0_CXFI_RCVDCLK_SEL_0(x)  VTSS_ENCODE_BITFIELD(x,21,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0_CXFI_RCVDCLK_SEL_0     VTSS_ENCODE_BITMASK(21,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0_CXFI_RCVDCLK_SEL_0(x)  VTSS_EXTRACT_BITFIELD(x,21,1)

/** 
 * \brief
 * Selects the source of the output UPI_CXFI recovered clock for channel 0
 *
 * \details 
 * 0: CXFI is source, lane is determined by CXFI_RCVDCLK_SEL_0
 * 1: UPI is source
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0 . UPI_CXFI_RCVDCLK_MUX_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0_UPI_CXFI_RCVDCLK_MUX_0(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0_UPI_CXFI_RCVDCLK_MUX_0     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0_UPI_CXFI_RCVDCLK_MUX_0(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \brief
 * Selects the source of the output OTN gapped clock for channel 0
 *
 * \details 
 * 0: OTN1 is source
 * 1: OTN2 is source
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0 . OTN_GAP_CLK_MUX_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0_OTN_GAP_CLK_MUX_0(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0_OTN_GAP_CLK_MUX_0     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0_OTN_GAP_CLK_MUX_0(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Selects the source of the Tx client clock for channel 0
 *
 * \details 
 * 0: UPI 0 is clock source
 * 1: UPI 1 is clock source
 * 2: Client XFI 0 is clock source
 * 3: Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0 . CLIENT_TX_CLK_MUX_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0_CLIENT_TX_CLK_MUX_0(x)  VTSS_ENCODE_BITFIELD(x,8,2)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0_CLIENT_TX_CLK_MUX_0     VTSS_ENCODE_BITMASK(8,2)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0_CLIENT_TX_CLK_MUX_0(x)  VTSS_EXTRACT_BITFIELD(x,8,2)

/** 
 * \brief
 * Selects the source of the Rx client clock for channel 0
 *
 * \details 
 * 0: UPI 0 is clock source
 * 1: UPI 1 is clock source
 * 2: Client XFI 0 is clock source
 * 3: Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0 . CLIENT_RX_CLK_MUX_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0_CLIENT_RX_CLK_MUX_0(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0_CLIENT_RX_CLK_MUX_0     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_0_CLIENT_RX_CLK_MUX_0(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Channel 0 MAC mux controls
 *
 * \details
 * Contains the mux controls for channel 0 MAC and PCS XGMII and RAB clocks
 *
 * Register: \a DEVCPU_GCB::CHIP_MUX::MAC_CLK_MUX_0
 */
#define VTSS_DEVCPU_GCB_CHIP_MUX_MAC_CLK_MUX_0  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0xd3)

/** 
 * \brief
 * Mux control for PCS2 Tx XGMII, MAC2 Tx, RAB Tx, MAC1 Rx, PCS1 Rx XGMII
 * and GFP2 Tx client clock in channel 0
 *
 * \details 
 * 0: Client Rx clock is source
 * 1: Line Tx clock is source
 * 2: Client Rx gapped clock is source
 * 3: Line Tx gapped clock is source
 * 7:4: Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_MAC_CLK_MUX_0 . MAC2_TX_CLK_MUX_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_MAC_CLK_MUX_0_MAC2_TX_CLK_MUX_0(x)  VTSS_ENCODE_BITFIELD(x,24,3)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_MAC_CLK_MUX_0_MAC2_TX_CLK_MUX_0     VTSS_ENCODE_BITMASK(24,3)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_MAC_CLK_MUX_0_MAC2_TX_CLK_MUX_0(x)  VTSS_EXTRACT_BITFIELD(x,24,3)

/** 
 * \brief
 * Mux control for PCS1 Tx XGMII, MAC1 Tx, RAB Rx, MAC2 Rx, PCS2 Rx XGMII
 * and GFP2 Rx client clock in channel 0
 *
 * \details 
 * 0: Client Tx clock is source
 * 1: Line Rx clock is source
 * 2: Client Tx gapped clock is source
 * 3: Line Rx gapped clock is source
 * 7:4: Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_MAC_CLK_MUX_0 . MAC1_TX_CLK_MUX_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_MAC_CLK_MUX_0_MAC1_TX_CLK_MUX_0(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_MAC_CLK_MUX_0_MAC1_TX_CLK_MUX_0     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_MAC_CLK_MUX_0_MAC1_TX_CLK_MUX_0(x)  VTSS_EXTRACT_BITFIELD(x,8,3)


/** 
 * \brief Channel 0 OTN1 mux controls
 *
 * \details
 * Contains the mux controls for channel 0 OTN1 clocks
 *
 * Register: \a DEVCPU_GCB::CHIP_MUX::OTN1_CLK_MUX_0
 */
#define VTSS_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_0  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0xd6)

/** 
 * \brief
 * Mux control for OTN1 Tx line clock in channel 0
 *
 * \details 
 * 0: Client Tx clock is source
 * 1: Line Rx clock is source
 * 2: Client Tx gapped clock is source
 * 7:3: Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_0 . OTN1_TX_LINE_CLK_MUX_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_0_OTN1_TX_LINE_CLK_MUX_0(x)  VTSS_ENCODE_BITFIELD(x,24,3)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_0_OTN1_TX_LINE_CLK_MUX_0     VTSS_ENCODE_BITMASK(24,3)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_0_OTN1_TX_LINE_CLK_MUX_0(x)  VTSS_EXTRACT_BITFIELD(x,24,3)

/** 
 * \brief
 * Mux control for OTN1 Rx line clock in channel 0
 *
 * \details 
 * 0: Client Rx clock is source
 * 1: Client Rc gapped clock is source
 * 7:2: Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_0 . OTN1_RX_LINE_CLK_MUX_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_0_OTN1_RX_LINE_CLK_MUX_0(x)  VTSS_ENCODE_BITFIELD(x,16,3)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_0_OTN1_RX_LINE_CLK_MUX_0     VTSS_ENCODE_BITMASK(16,3)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_0_OTN1_RX_LINE_CLK_MUX_0(x)  VTSS_EXTRACT_BITFIELD(x,16,3)

/** 
 * \brief
 * Mux control for OTN1 Tx system clock, EWIS1 Tx clock, PCS1 Tx PMA clock
 * and GFP1 Tx line clock in channel 0
 *
 * \details 
 * 0: Client Tx clock is source
 * 1: Line Rx clock is source
 * 2: OTN1 Tx gapped clock is source
 * 3: Line Rx gapped clock is source
 * 7:4: Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_0 . OTN1_TX_SYS_CLK_MUX_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_0_OTN1_TX_SYS_CLK_MUX_0(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_0_OTN1_TX_SYS_CLK_MUX_0     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_0_OTN1_TX_SYS_CLK_MUX_0(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * Mux control for OTN1 Rx system clock, EWIS1 Rx clock, PCS1 Rx PMA clock
 * and GFP1 Rx line clock in channel 0
 *
 * \details 
 * 0: Client Rx clock is source
 * 1: Line Tx clock is source
 * 2: Client Rx gapped clock is source
 * 3: Line Tx gapped clock is source
 * 4: OTN1 Rx gapped clock is source
 * 7:5: Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_0 . OTN1_RX_SYS_CLK_MUX_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_0_OTN1_RX_SYS_CLK_MUX_0(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_0_OTN1_RX_SYS_CLK_MUX_0     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_0_OTN1_RX_SYS_CLK_MUX_0(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Channel 0 OTN2 mux controls
 *
 * \details
 * Contains the mux controls for channel 0 OTN2 clocks
 *
 * Register: \a DEVCPU_GCB::CHIP_MUX::OTN2_CLK_MUX_0
 */
#define VTSS_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_0  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0xd7)

/** 
 * \brief
 * Mux control for OTN2 Tx line clock in channel 0
 *
 * \details 
 * 0: Client Rx clock is source
 * 1: Line Tx clock is source
 * 2: Client Rx gapped clock is source
 * 7:3: Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_0 . OTN2_TX_LINE_CLK_MUX_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_0_OTN2_TX_LINE_CLK_MUX_0(x)  VTSS_ENCODE_BITFIELD(x,24,3)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_0_OTN2_TX_LINE_CLK_MUX_0     VTSS_ENCODE_BITMASK(24,3)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_0_OTN2_TX_LINE_CLK_MUX_0(x)  VTSS_EXTRACT_BITFIELD(x,24,3)

/** 
 * \brief
 * Mux control for OTN2 Rx line clock in channel 0
 *
 * \details 
 * 0: Line Rx clock is source
 * 7:1: Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_0 . OTN2_RX_LINE_CLK_MUX_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_0_OTN2_RX_LINE_CLK_MUX_0(x)  VTSS_ENCODE_BITFIELD(x,16,3)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_0_OTN2_RX_LINE_CLK_MUX_0     VTSS_ENCODE_BITMASK(16,3)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_0_OTN2_RX_LINE_CLK_MUX_0(x)  VTSS_EXTRACT_BITFIELD(x,16,3)

/** 
 * \brief
 * Mux control for OTN2 Tx system clock, EWIS2 Tx clock, PCS2 Tx PMA clock
 * and GFP2 Tx line clock in channel 0
 *
 * \details 
 * 0: Client Rx clock is source
 * 1: Line Tx clock is source
 * 2: Client Rx gapped clock is source
 * 3: OTN2 Tx gapped clock is source
 * 7:4: Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_0 . OTN2_TX_SYS_CLK_MUX_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_0_OTN2_TX_SYS_CLK_MUX_0(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_0_OTN2_TX_SYS_CLK_MUX_0     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_0_OTN2_TX_SYS_CLK_MUX_0(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * Mux control for OTN2 Rx system clock, EWIS2 Rx clock, PCS2 Rx PMA clock
 * and GFP2 Rx line clock in channel 0
 *
 * \details 
 * 0: Client Tx clock is source
 * 1: Line Rx clock is source
 * 2: Client Tx gapped clock is source
 * 3: OTN2 Rx gapped clock is source
 * 7:4: Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_0 . OTN2_RX_SYS_CLK_MUX_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_0_OTN2_RX_SYS_CLK_MUX_0(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_0_OTN2_RX_SYS_CLK_MUX_0     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_0_OTN2_RX_SYS_CLK_MUX_0(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Channel 0 interface mux controls
 *
 * \details
 * Contains the mux controls for channel 0 interface clocks
 *
 * Register: \a DEVCPU_GCB::CHIP_MUX::TX_INTF_CLK_MUX_0
 */
#define VTSS_DEVCPU_GCB_CHIP_MUX_TX_INTF_CLK_MUX_0  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0xd8)

/** 
 * \brief
 * Mux control for line XFI Tx clock in channel 0
 *
 * \details 
 * 0: Line Tx SERDES is source
 * 1: Client Rx is source
 * 2 Client Rx gapped is source
 * 3: MAC2_TX_CLK is source
 * 7:4: Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_TX_INTF_CLK_MUX_0 . LINE_XFI_CLK_MUX_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_TX_INTF_CLK_MUX_0_LINE_XFI_CLK_MUX_0(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_TX_INTF_CLK_MUX_0_LINE_XFI_CLK_MUX_0     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_TX_INTF_CLK_MUX_0_LINE_XFI_CLK_MUX_0(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * Mux control for UPI/Client XFI Tx clock in channel 0
 *
 * \details 
 * 0: UPI/Client XFI Tx SERDES is source
 * 1: Line Rx is source
 * 2: Client Tx gapped is source (UPI only)
 * 3: MAC1_TX_CLK is source (CXFI only)
 * 7:4: Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_TX_INTF_CLK_MUX_0 . UPI_XFI_CLK_MUX_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_TX_INTF_CLK_MUX_0_UPI_XFI_CLK_MUX_0(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_TX_INTF_CLK_MUX_0_UPI_XFI_CLK_MUX_0     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_TX_INTF_CLK_MUX_0_UPI_XFI_CLK_MUX_0(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Channel 1 data muxes 
 *
 * \details
 * Contains the top level data muxes for channel 1
 *
 * Register: \a DEVCPU_GCB::CHIP_MUX::DATA_MUX_1
 */
#define VTSS_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0xd9)

/** 
 * \brief
 * ROSI/TOSI mux control for channel 1
 * Selects the source/sink of the ROSI and TOSI pins on the device
 *
 * \details 
 * 0: Select ROSI/TOSI from eWIS2 as source/sink of ROSI/TOSI pins of
 * channel 1
 * 1: Select OTN OH port from OTN1 as source/sink of ROSI/TOSI pins of
 * channel 1 (uses GPIO(7:5) as well)
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1 . ROSI_TOSI_MUX_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1_ROSI_TOSI_MUX_1(x)  VTSS_ENCODE_BITFIELD(x,24,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1_ROSI_TOSI_MUX_1     VTSS_ENCODE_BITMASK(24,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1_ROSI_TOSI_MUX_1(x)  VTSS_EXTRACT_BITFIELD(x,24,1)

/** 
 * \brief
 * OTN OH port mux control for channel 1
 * Selects the source/sink of the OTN OH port pins on the device
 *
 * \details 
 * 0: Select OTN OH port from OTN2 as source/sink of OTN OH port pins of
 * channel 1
 * 1: Select OTN OH port from OTN1 as source/sink of OTN OH port pins of
 * channel 1
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1 . OTN_OH_MUX_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1_OTN_OH_MUX_1(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1_OTN_OH_MUX_1     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1_OTN_OH_MUX_1(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Client Rx mux control for channel 1
 * Selects the source of data for client side of channel 1 from either the
 * channel 1 UPI or channel 1 client XFI
 *
 * \details 
 * 0: Select UPI as source of channel 1 client
 * 1: Select client XFI as source of channel 1 client
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1 . CLIENT_RX_MUX_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1_CLIENT_RX_MUX_1(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1_CLIENT_RX_MUX_1     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1_CLIENT_RX_MUX_1(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Protection control mux for channel 1 Tx
 * Selects between channel 0 and channel 1 as data sink
 *
 * \details 
 * 0: Select UPI/Client XFI 0 as sink of channel 0 client
 * 1: Select UPI/Client XFI 1 as sink of channel 0 client
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1 . PROT_TX_MUX_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1_PROT_TX_MUX_1(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1_PROT_TX_MUX_1     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1_PROT_TX_MUX_1(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Protection control mux for channel 0 Rx
 * Selects between channel 0 and channel 1 as data source
 *
 * \details 
 * 0: Select UPI/Client XFI 0 as source of channel 0 client
 * 1: Select UPI/Client XFI 1 as source of channel 0 client
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1 . PROT_RX_MUX_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1_PROT_RX_MUX_1(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1_PROT_RX_MUX_1     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1_PROT_RX_MUX_1(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Channel 1 top clock muxes
 *
 * \details
 * Contains the top level clock mux controls for channel 1
 *
 * Register: \a DEVCPU_GCB::CHIP_MUX::TOP_CLK_MUX_1
 */
#define VTSS_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0xda)

/** 
 * \brief
 * Selects the source of the LXFI reference clock for channel 1
 *
 * \details 
 * 0: LXFI_REFCLK_1 is source
 * 1: Reserved
 * 2: UPI_CXFI_REFCLK_1 is source
 * 3: REFCLK is source
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1 . LXFI_REFCLK_MUX_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1_LXFI_REFCLK_MUX_1(x)  VTSS_ENCODE_BITFIELD(x,28,2)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1_LXFI_REFCLK_MUX_1     VTSS_ENCODE_BITMASK(28,2)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1_LXFI_REFCLK_MUX_1(x)  VTSS_EXTRACT_BITFIELD(x,28,2)

/** 
 * \brief
 * Selects the source of the UPI_CXFI reference clock for channel 1
 *
 * \details 
 * 0: UPI_CXFI_REFCLK_1 is source
 * 1: REFCLK is source
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1 . UPI_CXFI_REFCLK_MUX_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1_UPI_CXFI_REFCLK_MUX_1(x)  VTSS_ENCODE_BITFIELD(x,24,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1_UPI_CXFI_REFCLK_MUX_1     VTSS_ENCODE_BITMASK(24,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1_UPI_CXFI_REFCLK_MUX_1(x)  VTSS_EXTRACT_BITFIELD(x,24,1)

/** 
 * \brief
 * Selects the source of the output LXFI recovered clock for channel 1
 *
 * \details 
 * 0: LXFI lane A is source
 * 1: LXFI lane B is source
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1 . LXFI_RCVDCLK_SEL_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1_LXFI_RCVDCLK_SEL_1(x)  VTSS_ENCODE_BITFIELD(x,22,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1_LXFI_RCVDCLK_SEL_1     VTSS_ENCODE_BITMASK(22,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1_LXFI_RCVDCLK_SEL_1(x)  VTSS_EXTRACT_BITFIELD(x,22,1)

/** 
 * \brief
 * Selects the source of the output CXFI recovered clock for channel 1
 *
 * \details 
 * 0: CXFI lane A is source
 * 1: CXFI lane B is source
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1 . CXFI_RCVDCLK_SEL_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1_CXFI_RCVDCLK_SEL_1(x)  VTSS_ENCODE_BITFIELD(x,21,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1_CXFI_RCVDCLK_SEL_1     VTSS_ENCODE_BITMASK(21,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1_CXFI_RCVDCLK_SEL_1(x)  VTSS_EXTRACT_BITFIELD(x,21,1)

/** 
 * \brief
 * Selects the source of the output UPI_CXFI recovered clock for channel 1
 *
 * \details 
 * 0: CXFI is source, lane is determined by CXFI_RCVDCLK_SEL_1
 * 1: UPI is source
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1 . UPI_CXFI_RCVDCLK_MUX_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1_UPI_CXFI_RCVDCLK_MUX_1(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1_UPI_CXFI_RCVDCLK_MUX_1     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1_UPI_CXFI_RCVDCLK_MUX_1(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \brief
 * Selects the source of the output OTN gapped clock for channel 0
 *
 * \details 
 * 0: OTN1 is source
 * 1: OTN2 is source
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1 . OTN_GAP_CLK_MUX_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1_OTN_GAP_CLK_MUX_1(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1_OTN_GAP_CLK_MUX_1     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1_OTN_GAP_CLK_MUX_1(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Selects the source of the Tx client clock for channel 1
 *
 * \details 
 * 0: UPI 0 is clock source
 * 1: UPI 1 is clock source
 * 2: Reserved
 * 3: Client XFI 1 is clock source
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1 . CLIENT_TX_CLK_MUX_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1_CLIENT_TX_CLK_MUX_1(x)  VTSS_ENCODE_BITFIELD(x,8,2)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1_CLIENT_TX_CLK_MUX_1     VTSS_ENCODE_BITMASK(8,2)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1_CLIENT_TX_CLK_MUX_1(x)  VTSS_EXTRACT_BITFIELD(x,8,2)

/** 
 * \brief
 * Selects the source of the Rx client clock for channel 1
 *
 * \details 
 * 0: UPI 0 is clock source
 * 1: UPI 1 is clock source
 * 2: Reserved
 * 3: Client XFI 1 is clock source
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1 . CLIENT_RX_CLK_MUX_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1_CLIENT_RX_CLK_MUX_1(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1_CLIENT_RX_CLK_MUX_1     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_TOP_CLK_MUX_1_CLIENT_RX_CLK_MUX_1(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Channel 1 MAC mux controls
 *
 * \details
 * Contains the mux controls for channel 1 MAC and PCS XGMII and RAB clocks
 *
 * Register: \a DEVCPU_GCB::CHIP_MUX::MAC_CLK_MUX_1
 */
#define VTSS_DEVCPU_GCB_CHIP_MUX_MAC_CLK_MUX_1  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0xdc)

/** 
 * \brief
 * Mux control for PCS2 Tx XGMII, MAC2 Tx, RAB Tx, MAC1 Rx, PCS1 Rx XGMII
 * and GFP2 Tx client clock in channel 1
 *
 * \details 
 * 0: Client Rx clock is source
 * 1: Line Tx clock is source
 * 2: Client Rx gapped clock is source
 * 3: Line Tx gapped clock is source
 * 7:4: Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_MAC_CLK_MUX_1 . MAC2_TX_CLK_MUX_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_MAC_CLK_MUX_1_MAC2_TX_CLK_MUX_1(x)  VTSS_ENCODE_BITFIELD(x,24,3)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_MAC_CLK_MUX_1_MAC2_TX_CLK_MUX_1     VTSS_ENCODE_BITMASK(24,3)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_MAC_CLK_MUX_1_MAC2_TX_CLK_MUX_1(x)  VTSS_EXTRACT_BITFIELD(x,24,3)

/** 
 * \brief
 * Mux control for PCS1 Tx XGMII, MAC1 Tx, RAB Rx, MAC2 Rx, PCS2 Rx XGMII
 * and GFP2 Rx client clock in channel 1
 *
 * \details 
 * 0: Client Tx clock is source
 * 1: Line Rx clock is source
 * 2: Client Tx gapped clock is source
 * 3: Line Rx gapped clock is source
 * 7:4: Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_MAC_CLK_MUX_1 . MAC1_TX_CLK_MUX_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_MAC_CLK_MUX_1_MAC1_TX_CLK_MUX_1(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_MAC_CLK_MUX_1_MAC1_TX_CLK_MUX_1     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_MAC_CLK_MUX_1_MAC1_TX_CLK_MUX_1(x)  VTSS_EXTRACT_BITFIELD(x,8,3)


/** 
 * \brief Channel 1 OTN1 mux controls
 *
 * \details
 * Contains the mux controls for channel 1 OTN1 clocks
 *
 * Register: \a DEVCPU_GCB::CHIP_MUX::OTN1_CLK_MUX_1
 */
#define VTSS_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_1  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0xdf)

/** 
 * \brief
 * Mux control for OTN1 Tx line clock in channel 1
 *
 * \details 
 * 0: Client Tx clock is source
 * 1: Line Rx clock is source
 * 2: Client Tx gapped clock is source
 * 7:3: Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_1 . OTN1_TX_LINE_CLK_MUX_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_1_OTN1_TX_LINE_CLK_MUX_1(x)  VTSS_ENCODE_BITFIELD(x,24,3)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_1_OTN1_TX_LINE_CLK_MUX_1     VTSS_ENCODE_BITMASK(24,3)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_1_OTN1_TX_LINE_CLK_MUX_1(x)  VTSS_EXTRACT_BITFIELD(x,24,3)

/** 
 * \brief
 * Mux control for OTN1 Rx line clock in channel 1
 *
 * \details 
 * 0: Client Rx clock is source
 * 1: Client Rx gapped clock is source
 * 7:2: Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_1 . OTN1_RX_LINE_CLK_MUX_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_1_OTN1_RX_LINE_CLK_MUX_1(x)  VTSS_ENCODE_BITFIELD(x,16,3)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_1_OTN1_RX_LINE_CLK_MUX_1     VTSS_ENCODE_BITMASK(16,3)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_1_OTN1_RX_LINE_CLK_MUX_1(x)  VTSS_EXTRACT_BITFIELD(x,16,3)

/** 
 * \brief
 * Mux control for OTN1 Tx system clock, EWIS1 Tx clock, PCS1 Tx PMA clock
 * and GFP1 Tx line clock in channel 1
 *
 * \details 
 * 0: Client Tx clock is source
 * 1: Line Rx clock is source
 * 2: Client TX gapped clock is source
 * 3: Line Rx gapped clock is source
 * 7:4: Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_1 . OTN1_TX_SYS_CLK_MUX_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_1_OTN1_TX_SYS_CLK_MUX_1(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_1_OTN1_TX_SYS_CLK_MUX_1     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_1_OTN1_TX_SYS_CLK_MUX_1(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * Mux control for OTN1 Rx system clock, EWIS1 Rx clock, PCS1 Rx PMA clock
 * and GFP1 Rx line clock in channel 1
 *
 * \details 
 * 0: Client Rx clock is source
 * 1: Line Tx clock is source
 * 2: Client Rx gapped clock is source
 * 3: Line Tx gapped clock is source
 * 7:4: Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_1 . OTN1_RX_SYS_CLK_MUX_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_1_OTN1_RX_SYS_CLK_MUX_1(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_1_OTN1_RX_SYS_CLK_MUX_1     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_OTN1_CLK_MUX_1_OTN1_RX_SYS_CLK_MUX_1(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Channel 1 OTN2 mux controls
 *
 * \details
 * Contains the mux controls for channel 1 OTN2 clocks
 *
 * Register: \a DEVCPU_GCB::CHIP_MUX::OTN2_CLK_MUX_1
 */
#define VTSS_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_1  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0xe0)

/** 
 * \brief
 * Mux control for OTN2 Tx line clock in channel 1
 *
 * \details 
 * 0: Client Rx clock is source
 * 1: Line Tx clock is source
 * 2: Client Rx gapped clock is source
 * 7:3: Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_1 . OTN2_TX_LINE_CLK_MUX_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_1_OTN2_TX_LINE_CLK_MUX_1(x)  VTSS_ENCODE_BITFIELD(x,24,3)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_1_OTN2_TX_LINE_CLK_MUX_1     VTSS_ENCODE_BITMASK(24,3)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_1_OTN2_TX_LINE_CLK_MUX_1(x)  VTSS_EXTRACT_BITFIELD(x,24,3)

/** 
 * \brief
 * Mux control for OTN2 Rx line clock in channel 1
 *
 * \details 
 * 0: Client Rx clock is source
 * 7:1: Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_1 . OTN2_RX_LINE_CLK_MUX_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_1_OTN2_RX_LINE_CLK_MUX_1(x)  VTSS_ENCODE_BITFIELD(x,16,3)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_1_OTN2_RX_LINE_CLK_MUX_1     VTSS_ENCODE_BITMASK(16,3)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_1_OTN2_RX_LINE_CLK_MUX_1(x)  VTSS_EXTRACT_BITFIELD(x,16,3)

/** 
 * \brief
 * Mux control for OTN2 Tx system clock, EWIS2 Tx clock, PCS2 Tx PMA clock
 * and GFP2 Tx line clock in channel 1
 *
 * \details 
 * 0: Client Rx clock is source
 * 1: Line Tx clock is source
 * 2: Client Rx gapped clock is source
 * 3: OTN2 Tx gapped clock is source
 * 7:4: Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_1 . OTN2_TX_SYS_CLK_MUX_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_1_OTN2_TX_SYS_CLK_MUX_1(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_1_OTN2_TX_SYS_CLK_MUX_1     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_1_OTN2_TX_SYS_CLK_MUX_1(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * Mux control for OTN2 Rx system clock, EWIS2 Rx clock, PCS2 Rx PMA clock
 * and GFP2 Rx line clock in channel 1
 *
 * \details 
 * 0: Client Tx clock is source
 * 1: Line Rx clock is source
 * 2: Client Tx gapped clock is source
 * 3: OTN2 Rx gapped clock is source
 * 7:4: Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_1 . OTN2_RX_SYS_CLK_MUX_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_1_OTN2_RX_SYS_CLK_MUX_1(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_1_OTN2_RX_SYS_CLK_MUX_1     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_OTN2_CLK_MUX_1_OTN2_RX_SYS_CLK_MUX_1(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Channel 1 interface mux controls
 *
 * \details
 * Contains the mux controls for channel 1 interface clocks
 *
 * Register: \a DEVCPU_GCB::CHIP_MUX::TX_INTF_CLK_MUX_1
 */
#define VTSS_DEVCPU_GCB_CHIP_MUX_TX_INTF_CLK_MUX_1  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0xe1)

/** 
 * \brief
 * Mux control for line XFI Tx clock in channel 1
 *
 * \details 
 * 0: Line Tx SERDES is source
 * 1: Client Rx is source
 * 2: Client Rx gapped is source
 * 3: MAC2_TX_CLK is source
 * 7:4: Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_TX_INTF_CLK_MUX_1 . LINE_XFI_CLK_MUX_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_TX_INTF_CLK_MUX_1_LINE_XFI_CLK_MUX_1(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_TX_INTF_CLK_MUX_1_LINE_XFI_CLK_MUX_1     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_TX_INTF_CLK_MUX_1_LINE_XFI_CLK_MUX_1(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * Mux control for UPI/Client XFI Tx clock in channel 1
 *
 * \details 
 * 0: UPI/Client XFI Tx SERDES is source
 * 1: Line Rx is source
 * 2: Client Tx gapped is source (UPI only)
 * 3: MAC1_TX_CLK is source (CXFI only)
 * 7:4: Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_TX_INTF_CLK_MUX_1 . UPI_XFI_CLK_MUX_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_TX_INTF_CLK_MUX_1_UPI_XFI_CLK_MUX_1(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_TX_INTF_CLK_MUX_1_UPI_XFI_CLK_MUX_1     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_TX_INTF_CLK_MUX_1_UPI_XFI_CLK_MUX_1(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief REFCLK controls
 *
 * \details
 * Contains controls for REFCLKs
 *
 * Register: \a DEVCPU_GCB::CHIP_MUX::REFCLK_IOTYPE
 */
#define VTSS_DEVCPU_GCB_CHIP_MUX_REFCLK_IOTYPE  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0xe2)

/** 
 * \brief
 * Control for REFCLKs
 *
 * \details 
 * 0: REFCLKs are LVDS
 * 1: REFCLKs are LVPECL
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_REFCLK_IOTYPE . REFCLK_IOTYPE
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_REFCLK_IOTYPE_REFCLK_IOTYPE(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_REFCLK_IOTYPE_REFCLK_IOTYPE     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_REFCLK_IOTYPE_REFCLK_IOTYPE(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief GPIO pin mux control for IP1588
 *
 * \details
 * Register: \a DEVCPU_GCB::CHIP_MUX::IP1588_PIN_MUX
 */
#define VTSS_DEVCPU_GCB_CHIP_MUX_IP1588_PIN_MUX  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0xe4)

/** 
 * \brief
 * Timestamp FIFO enable
 * Enable timestamp FIFO serial interface on GPIO
 *
 * \details 
 * 0: GPIO pins 0, 8, and 9 are general purpose inputs/outputs
 * 1: GPIO pins 0, 8, and 9 are serial output port for 1588
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_IP1588_PIN_MUX . IP1588_SI_SEL
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_IP1588_PIN_MUX_IP1588_SI_SEL(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_IP1588_PIN_MUX_IP1588_SI_SEL     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_IP1588_PIN_MUX_IP1588_SI_SEL(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Channel 1 1PPS enable
 * Enable channel 1 1PPS on GPIO
 *
 * \details 
 * 0: GPIO pin 10 is a general purpose input/output
 * 1: GPIO pin 10 is the 1PPS output for channel 1
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_IP1588_PIN_MUX . IP1588_1PPS_SEL_1
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_IP1588_PIN_MUX_IP1588_1PPS_SEL_1(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_IP1588_PIN_MUX_IP1588_1PPS_SEL_1     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_IP1588_PIN_MUX_IP1588_1PPS_SEL_1(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Channel 0 1PPS enable
 * Enable channel 0 1PPS on GPIO
 *
 * \details 
 * 0: GPIO pin 2 is a general purpose input/output
 * 1: GPIO pin 2 is the 1PPS output for channel 0
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_MUX_IP1588_PIN_MUX . IP1588_1PPS_SEL_0
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_MUX_IP1588_PIN_MUX_IP1588_1PPS_SEL_0(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_DEVCPU_GCB_CHIP_MUX_IP1588_PIN_MUX_IP1588_1PPS_SEL_0     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_DEVCPU_GCB_CHIP_MUX_IP1588_PIN_MUX_IP1588_1PPS_SEL_0(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/**
 * Register Group: \a DEVCPU_GCB::XFI_IF_PLL_CTL
 *
 * Control for XFI IF PLL
 */


/** 
 * \brief PLL control register for XFI 0
 *
 * \details
 * Register: \a DEVCPU_GCB::XFI_IF_PLL_CTL::XFI_0_PLL_CTL
 */
#define VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x110)

/** 
 * \brief
 * Used to configure the PLL based on the value of M*N1
 *
 * \details 
 * M*N1: C-value
 * 6-23: 0x38
 * 24-40: 0x64
 * 41-75: 0x74
 * all other values are reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL . XFI_0_IF_PLL_CONTROL
 */
#define  VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL_XFI_0_IF_PLL_CONTROL(x)  VTSS_ENCODE_BITFIELD(x,16,8)
#define  VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL_XFI_0_IF_PLL_CONTROL     VTSS_ENCODE_BITMASK(16,8)
#define  VTSS_X_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL_XFI_0_IF_PLL_CONTROL(x)  VTSS_EXTRACT_BITFIELD(x,16,8)

/** 
 * \brief
 * Unused PLL output enable - Do not enable
 *
 * \details 
 * 0: Disable (default)
 * 1: Enable
 *
 * Field: ::VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL . XFI_0_IF_PLL_OUTENDIF_B
 */
#define  VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL_XFI_0_IF_PLL_OUTENDIF_B(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL_XFI_0_IF_PLL_OUTENDIF_B     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL_XFI_0_IF_PLL_OUTENDIF_B(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Unused PLL output enable - Do not enable
 *
 * \details 
 * 0: Disable (default)
 * 1: Enable
 *
 * Field: ::VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL . XFI_0_IF_PLL_OUTENSE_B
 */
#define  VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL_XFI_0_IF_PLL_OUTENSE_B(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL_XFI_0_IF_PLL_OUTENSE_B     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL_XFI_0_IF_PLL_OUTENSE_B(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Differential output enable
 *
 * \details 
 * 0: Disable differential output
 * 1: Enable differential output
 *
 * Field: ::VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL . XFI_0_IF_PLL_OUTENDIF
 */
#define  VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL_XFI_0_IF_PLL_OUTENDIF(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL_XFI_0_IF_PLL_OUTENDIF     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL_XFI_0_IF_PLL_OUTENDIF(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Single ended output control
 *
 * \details 
 * 0: Disable single ended output
 * 1: Enable single ended output
 *
 * Field: ::VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL . XFI_0_IF_PLL_OUTENSE
 */
#define  VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL_XFI_0_IF_PLL_OUTENSE(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL_XFI_0_IF_PLL_OUTENSE     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL_XFI_0_IF_PLL_OUTENSE(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Reset for XFI 0 IF PLL
 *
 * \details 
 * 0: Normal operation
 * 1: Reset the IF PLL
 *
 * Field: ::VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL . XFI_0_IF_PLL_RESET
 */
#define  VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL_XFI_0_IF_PLL_RESET(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL_XFI_0_IF_PLL_RESET     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL_XFI_0_IF_PLL_RESET(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * When asserted, drive PLLOUT with REFCLK.
 *
 * \details 
 * 0: Normal operation
 * 1: Bypass
 *
 * Field: ::VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL . XFI_0_IF_PLL_BYPASS
 */
#define  VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL_XFI_0_IF_PLL_BYPASS(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL_XFI_0_IF_PLL_BYPASS     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_CTL_XFI_0_IF_PLL_BYPASS(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief XFI 0 IF PLL VCO control
 *
 * \details
 * Register: \a DEVCPU_GCB::XFI_IF_PLL_CTL::XFI_0_PLL_VCO_CTL
 */
#define VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_VCO_CTL  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x111)

/** 
 * \brief
 * VCO range selector
 *
 * \details 
 * VCO-range (GHz)   V-value
 * 2.400-2.475		 0xe
 * 2.475-2.575		 0x6
 * 2.575-2.650		 0xa
 * 2.650-2.750		 0x2
 * 2.750-2.900		 0xc
 * 2.900-3.000		 0x4
 * 3.000-3.150		 0x8
 * 3.150-3.300		 0x0
 * All other values are reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_VCO_CTL . XFI_0_IF_PLL_VCO_RANGE
 */
#define  VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_VCO_CTL_XFI_0_IF_PLL_VCO_RANGE(x)  VTSS_ENCODE_BITFIELD(x,28,4)
#define  VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_VCO_CTL_XFI_0_IF_PLL_VCO_RANGE     VTSS_ENCODE_BITMASK(28,4)
#define  VTSS_X_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_VCO_CTL_XFI_0_IF_PLL_VCO_RANGE(x)  VTSS_EXTRACT_BITFIELD(x,28,4)

/** 
 * \brief
 * Divide ratio select
 *
 * \details 
 * Divide-ratio P-Value
 * 1			 0x7
 * 2			 0x0
 * 3			 0x1
 * 4			 0x3
 * 5			 0x6
 * 6			 0x5
 * 7			 0x2
 * 8			 0x4
 *
 * Field: ::VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_VCO_CTL . XFI_0_IF_PLL_P
 */
#define  VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_VCO_CTL_XFI_0_IF_PLL_P(x)  VTSS_ENCODE_BITFIELD(x,24,3)
#define  VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_VCO_CTL_XFI_0_IF_PLL_P     VTSS_ENCODE_BITMASK(24,3)
#define  VTSS_X_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_VCO_CTL_XFI_0_IF_PLL_P(x)  VTSS_EXTRACT_BITFIELD(x,24,3)

/** 
 * \brief
 * Multiply factor select
 *
 * \details 
 * Multiply-factor - M-Value
 * 2- 0x0      3- 0x1	   4-0x2      5-0x5
 * 6- 0xa      7- 0x15	  8-0x2a    9-0x14
 * 10- 0x29  11- 0x13  12-0x26	13-0xc
 * 14- 0x19  15- 0x32  16-0x24	17-0x8
 * 18- 0x11  19- 0x22  20-0x4	 21-0x9
 * 22- 0x12  23- 0x25  24-0xb	 25-0x16
 * 26- 0x2d  27- 0x1b  28-0x36	29-0x2c
 * 30- 0x18  31- 0x31  32-0x23	33-0x7
 * 34- 0xe    35- 0x1d	36-0x3a  37-0x34
 * 38- 0x28  39- 0x10  40-0x21	41-0x3
 * 42- 0x6    43- 0xd	 44-0x1a  45-0x35
 * 46- 0x2b  47- 0x17  48-0x2e	49-0x1c
 * 50- 0x39  51- 0x33  52-0x27	53-0xf
 * 54- 0x1e  55- 0x3d  56-0x3b	57-0x37
 * 58- 0x2f   59- 0x1f	 60-0x3e  61-0x3c
 * 62- 0x38  63- 0x30  64-0x20
 *
 * Field: ::VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_VCO_CTL . XFI_0_IF_PLL_M
 */
#define  VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_VCO_CTL_XFI_0_IF_PLL_M(x)  VTSS_ENCODE_BITFIELD(x,16,6)
#define  VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_VCO_CTL_XFI_0_IF_PLL_M     VTSS_ENCODE_BITMASK(16,6)
#define  VTSS_X_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_VCO_CTL_XFI_0_IF_PLL_M(x)  VTSS_EXTRACT_BITFIELD(x,16,6)

/** 
 * \brief
 * Unused PLL output divider control.
 * Do not modify
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_VCO_CTL . XFI_0_IF_PLL_N3
 */
#define  VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_VCO_CTL_XFI_0_IF_PLL_N3(x)  VTSS_ENCODE_BITFIELD(x,8,4)
#define  VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_VCO_CTL_XFI_0_IF_PLL_N3     VTSS_ENCODE_BITMASK(8,4)
#define  VTSS_X_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_VCO_CTL_XFI_0_IF_PLL_N3(x)  VTSS_EXTRACT_BITFIELD(x,8,4)

/** 
 * \brief
 * N2 divider control
 *
 * \details 
 * Divide-ratio N-value
 * 2			 0x6
 * 3			 0xa
 * 4			 0x2
 * 5			 0xc
 * 6			 0x4
 * 7			 0x8
 * 8			 0x0
 * 9			 0x9
 * 10			0x1
 * All other values are reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_VCO_CTL . XFI_0_IF_PLL_N2
 */
#define  VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_VCO_CTL_XFI_0_IF_PLL_N2(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_VCO_CTL_XFI_0_IF_PLL_N2     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_VCO_CTL_XFI_0_IF_PLL_N2(x)  VTSS_EXTRACT_BITFIELD(x,4,4)

/** 
 * \brief
 * N1 divider control
 *
 * \details 
 * Divide-ratio N-value
 * 2			 0x6
 * 3			 0xa
 * 4			 0x2
 * 5			 0xc
 * 6			 0x4
 * 7			 0x8
 * 8			 0x0
 * 9			 0x9
 * 10			0x1
 * All other values are reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_VCO_CTL . XFI_0_IF_PLL_N1
 */
#define  VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_VCO_CTL_XFI_0_IF_PLL_N1(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_VCO_CTL_XFI_0_IF_PLL_N1     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_0_PLL_VCO_CTL_XFI_0_IF_PLL_N1(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief PLL control register for XFI 1
 *
 * \details
 * Register: \a DEVCPU_GCB::XFI_IF_PLL_CTL::XFI_1_PLL_CTL
 */
#define VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x112)

/** 
 * \brief
 * Used to configure the PLL based on the value of M*N1
 *
 * \details 
 * M*N1   C-value
 * 6-23     0x38
 * 24-40   0x64
 * 41-75   0x74
 * All other values are reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL . XFI_1_IF_PLL_CONTROL
 */
#define  VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL_XFI_1_IF_PLL_CONTROL(x)  VTSS_ENCODE_BITFIELD(x,16,8)
#define  VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL_XFI_1_IF_PLL_CONTROL     VTSS_ENCODE_BITMASK(16,8)
#define  VTSS_X_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL_XFI_1_IF_PLL_CONTROL(x)  VTSS_EXTRACT_BITFIELD(x,16,8)

/** 
 * \brief
 * Unused PLL output enable - Do not enable
 *
 * \details 
 * 0: Disable (default)
 * 1: Enable
 *
 * Field: ::VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL . XFI_1_IF_PLL_OUTENDIF_B
 */
#define  VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL_XFI_1_IF_PLL_OUTENDIF_B(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL_XFI_1_IF_PLL_OUTENDIF_B     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL_XFI_1_IF_PLL_OUTENDIF_B(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Unused PLL output enable - Do not enable
 *
 * \details 
 * 0: Disable (default)
 * 1: Enable
 *
 * Field: ::VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL . XFI_1_IF_PLL_OUTENSE_B
 */
#define  VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL_XFI_1_IF_PLL_OUTENSE_B(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL_XFI_1_IF_PLL_OUTENSE_B     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL_XFI_1_IF_PLL_OUTENSE_B(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Differential output enable
 *
 * \details 
 * 0: Disable differential output
 * 1: Enable differential output
 *
 * Field: ::VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL . XFI_1_IF_PLL_OUTENDIF
 */
#define  VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL_XFI_1_IF_PLL_OUTENDIF(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL_XFI_1_IF_PLL_OUTENDIF     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL_XFI_1_IF_PLL_OUTENDIF(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Single ended output control
 *
 * \details 
 * 0: Disable single ended output
 * 1: Enable single ended output
 *
 * Field: ::VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL . XFI_1_IF_PLL_OUTENSE
 */
#define  VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL_XFI_1_IF_PLL_OUTENSE(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL_XFI_1_IF_PLL_OUTENSE     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL_XFI_1_IF_PLL_OUTENSE(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Reset for XFI 0 IF PLL
 *
 * \details 
 * 0: Normal operation
 * 1: Reset the IF PLL
 *
 * Field: ::VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL . XFI_1_IF_PLL_RESET
 */
#define  VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL_XFI_1_IF_PLL_RESET(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL_XFI_1_IF_PLL_RESET     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL_XFI_1_IF_PLL_RESET(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * When asserted, drive PLLOUT with REFCLK
 *
 * \details 
 * 0: Normal operation
 * 1: Bypass
 *
 * Field: ::VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL . XFI_1_IF_PLL_BYPASS
 */
#define  VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL_XFI_1_IF_PLL_BYPASS(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL_XFI_1_IF_PLL_BYPASS     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_CTL_XFI_1_IF_PLL_BYPASS(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief XFI 1 IF PLL VCO control
 *
 * \details
 * Register: \a DEVCPU_GCB::XFI_IF_PLL_CTL::XFI_1_PLL_VCO_CTL
 */
#define VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_VCO_CTL  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x113)

/** 
 * \brief
 * VCO range selector
 *
 * \details 
 * VCO-range (GHz)   V-value
 * 2.400-2.475		 0xe
 * 2.475-2.575		 0x6
 * 2.575-2.650		 0xa
 * 2.650-2.750		 0x2
 * 2.750-2.900		 0xc
 * 2.900-3.000		 0x4
 * 3.000-3.150		 0x8
 * 3.150-3.300		 0x0
 * All other values are reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_VCO_CTL . XFI_1_IF_PLL_VCO_RANGE
 */
#define  VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_VCO_CTL_XFI_1_IF_PLL_VCO_RANGE(x)  VTSS_ENCODE_BITFIELD(x,28,4)
#define  VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_VCO_CTL_XFI_1_IF_PLL_VCO_RANGE     VTSS_ENCODE_BITMASK(28,4)
#define  VTSS_X_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_VCO_CTL_XFI_1_IF_PLL_VCO_RANGE(x)  VTSS_EXTRACT_BITFIELD(x,28,4)

/** 
 * \brief
 * Divide ratio select
 *
 * \details 
 * Divide-ratio P-value
 * 1			 0x7
 * 2			 0x0
 * 3			 0x1
 * 4			 0x3
 * 5			 0x6
 * 6			 0x5
 * 7			 0x2
 * 8			 0x4
 *
 * Field: ::VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_VCO_CTL . XFI_1_IF_PLL_P
 */
#define  VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_VCO_CTL_XFI_1_IF_PLL_P(x)  VTSS_ENCODE_BITFIELD(x,24,3)
#define  VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_VCO_CTL_XFI_1_IF_PLL_P     VTSS_ENCODE_BITMASK(24,3)
#define  VTSS_X_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_VCO_CTL_XFI_1_IF_PLL_P(x)  VTSS_EXTRACT_BITFIELD(x,24,3)

/** 
 * \brief
 * Multiply factor select
 *
 * \details 
 * Multiply-factor - M-value
 * 2   - 0x0;	 3   - 0x1;    4   - 0x2;    5	 - 0x5; 
 * 6   - 0xa;	 7   - 0x15;  8   - 0x2a;  9   - 0x14;
 * 10 - 0x29;  11 - 0x13;  12 - 0x26;  13 - 0xc;
 * 14 - 0x19;  15 - 0x32;  16 - 0x24;  17 - 0x8;
 * 18 - 0x11;  19 - 0x22;  20 - 0x4;	21 - 0x9;
 * 22 - 0x12;  23 - 0x25;  24 - 0xb;	25 - 0x16;
 * 26 - 0x2d;  27 - 0x1b;  28 - 0x36;  29 - 0x2c;
 * 30 - 0x18;  31 - 0x31;  32 - 0x23;  33 - 0x7;
 * 34 - 0xe;	35 - 0x1d;  36 - 0x3a;	37 - 0x34;
 * 38 - 0x28;  39 - 0x10;  40 - 0x21;  41 - 0x3;
 * 42 - 0x6;	43 - 0xd;    44 - 0x1a;  45 - 0x35;
 * 46 - 0x2b;  47 - 0x17;  48 - 0x2e;  49 - 0x1c;
 * 50 - 0x39;  51 - 0x33;  52 - 0x27;  53 - 0xf;
 * 54 - 0x1e;  55 - 0x3d;  56 - 0x3b;  57 - 0x37;
 * 58 - 0x2f;	59 - 0x1f;   60 - 0x3e;  61 - 0x3c;
 * 62 - 0x38;  63 - 0x30;  64 - 0x20
 *
 * Field: ::VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_VCO_CTL . XFI_1_IF_PLL_M
 */
#define  VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_VCO_CTL_XFI_1_IF_PLL_M(x)  VTSS_ENCODE_BITFIELD(x,16,6)
#define  VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_VCO_CTL_XFI_1_IF_PLL_M     VTSS_ENCODE_BITMASK(16,6)
#define  VTSS_X_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_VCO_CTL_XFI_1_IF_PLL_M(x)  VTSS_EXTRACT_BITFIELD(x,16,6)

/** 
 * \brief
 * Unused PLL output divider control
 * Do not modify
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_VCO_CTL . XFI_1_IF_PLL_N3
 */
#define  VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_VCO_CTL_XFI_1_IF_PLL_N3(x)  VTSS_ENCODE_BITFIELD(x,8,4)
#define  VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_VCO_CTL_XFI_1_IF_PLL_N3     VTSS_ENCODE_BITMASK(8,4)
#define  VTSS_X_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_VCO_CTL_XFI_1_IF_PLL_N3(x)  VTSS_EXTRACT_BITFIELD(x,8,4)

/** 
 * \brief
 * N2 divider control.
 *
 * \details 
 * Divide-ratio N-value
 * 2			 0x6
 * 3			 0xa
 * 4			 0x2
 * 5			 0xc
 * 6			 0x4
 * 7			 0x8
 * 8			 0x0
 * 9			 0x9
 * 10			0x1
 * All other values are reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_VCO_CTL . XFI_1_IF_PLL_N2
 */
#define  VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_VCO_CTL_XFI_1_IF_PLL_N2(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_VCO_CTL_XFI_1_IF_PLL_N2     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_VCO_CTL_XFI_1_IF_PLL_N2(x)  VTSS_EXTRACT_BITFIELD(x,4,4)

/** 
 * \brief
 * N1 divider control.
 *
 * \details 
 * Divide-ratio N-value
 * 2			 0x6
 * 3			 0xa
 * 4			 0x2
 * 5			 0xc
 * 6			 0x4
 * 7			 0x8
 * 8			 0x0
 * 9			 0x9
 * 10			0x1
 * All other values are reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_VCO_CTL . XFI_1_IF_PLL_N1
 */
#define  VTSS_F_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_VCO_CTL_XFI_1_IF_PLL_N1(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_VCO_CTL_XFI_1_IF_PLL_N1     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_DEVCPU_GCB_XFI_IF_PLL_CTL_XFI_1_PLL_VCO_CTL_XFI_1_IF_PLL_N1(x)  VTSS_EXTRACT_BITFIELD(x,0,4)

/**
 * Register Group: \a DEVCPU_GCB::XAUI_IF_PLL_CTL
 *
 * Control for XAUI IF PLL
 */


/** 
 * \brief PLL control register for XAUI 0
 *
 * \details
 * Register: \a DEVCPU_GCB::XAUI_IF_PLL_CTL::XAUI_0_PLL_CTL
 */
#define VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x114)

/** 
 * \brief
 * Used to configure the PLL based on the value of M*N1
 *
 * \details 
 * M*N1   C-value
 * 6-23     0x38
 * 24-40   0x64
 * 41-75   0x74
 * All other values are reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL . XAUI_0_IF_PLL_CONTROL
 */
#define  VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL_XAUI_0_IF_PLL_CONTROL(x)  VTSS_ENCODE_BITFIELD(x,16,8)
#define  VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL_XAUI_0_IF_PLL_CONTROL     VTSS_ENCODE_BITMASK(16,8)
#define  VTSS_X_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL_XAUI_0_IF_PLL_CONTROL(x)  VTSS_EXTRACT_BITFIELD(x,16,8)

/** 
 * \brief
 * Unused PLL output enable - Do not enable
 *
 * \details 
 * 0: Disable (default)
 * 1: Enable
 *
 * Field: ::VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL . XAUI_0_IF_PLL_OUTENDIF_B
 */
#define  VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL_XAUI_0_IF_PLL_OUTENDIF_B(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL_XAUI_0_IF_PLL_OUTENDIF_B     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL_XAUI_0_IF_PLL_OUTENDIF_B(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Unused PLL output enable - Do not enable
 *
 * \details 
 * 0: Disable (default)
 * 1: Enable
 *
 * Field: ::VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL . XAUI_0_IF_PLL_OUTENSE_B
 */
#define  VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL_XAUI_0_IF_PLL_OUTENSE_B(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL_XAUI_0_IF_PLL_OUTENSE_B     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL_XAUI_0_IF_PLL_OUTENSE_B(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Differential output enable
 *
 * \details 
 * 0: Disable differential output
 * 1: Enable differential output
 *
 * Field: ::VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL . XAUI_0_IF_PLL_OUTENDIF
 */
#define  VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL_XAUI_0_IF_PLL_OUTENDIF(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL_XAUI_0_IF_PLL_OUTENDIF     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL_XAUI_0_IF_PLL_OUTENDIF(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Single ended vs differential output control
 *
 * \details 
 * 0: Disable single ended output
 * 1: Enable single ended output
 *
 * Field: ::VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL . XAUI_0_IF_PLL_OUTENSE
 */
#define  VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL_XAUI_0_IF_PLL_OUTENSE(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL_XAUI_0_IF_PLL_OUTENSE     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL_XAUI_0_IF_PLL_OUTENSE(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Reset for XAUI 0 IF PLL
 *
 * \details 
 * 0: Normal operation
 * 1: Reset the IF PLL
 *
 * Field: ::VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL . XAUI_0_IF_PLL_RESET
 */
#define  VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL_XAUI_0_IF_PLL_RESET(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL_XAUI_0_IF_PLL_RESET     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL_XAUI_0_IF_PLL_RESET(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * When asserted, drive PLLOUT with REFCLK
 *
 * \details 
 * 0: Normal operation
 * 1: Bypass
 *
 * Field: ::VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL . XAUI_0_IF_PLL_BYPASS
 */
#define  VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL_XAUI_0_IF_PLL_BYPASS(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL_XAUI_0_IF_PLL_BYPASS     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_CTL_XAUI_0_IF_PLL_BYPASS(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief XAUI 0 IF PLL VCO control
 *
 * \details
 * Register: \a DEVCPU_GCB::XAUI_IF_PLL_CTL::XAUI_0_PLL_VCO_CTL
 */
#define VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_VCO_CTL  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x115)

/** 
 * \brief
 * VCO range selector
 *
 * \details 
 * VCO-range (GHz)   V-value
 * 2.400-2.475		 0xe
 * 2.475-2.575		 0x6
 * 2.575-2.650		 0xa
 * 2.650-2.750		 0x2
 * 2.750-2.900		 0xc
 * 2.900-3.000		 0x4
 * 3.000-3.150		 0x8
 * 3.150-3.300		 0x0
 * All other values are reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_VCO_CTL . XAUI_0_IF_PLL_VCO_RANGE
 */
#define  VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_VCO_CTL_XAUI_0_IF_PLL_VCO_RANGE(x)  VTSS_ENCODE_BITFIELD(x,28,4)
#define  VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_VCO_CTL_XAUI_0_IF_PLL_VCO_RANGE     VTSS_ENCODE_BITMASK(28,4)
#define  VTSS_X_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_VCO_CTL_XAUI_0_IF_PLL_VCO_RANGE(x)  VTSS_EXTRACT_BITFIELD(x,28,4)

/** 
 * \brief
 * Divide ratio select
 *
 * \details 
 * Divide-ratio P-value
 * 1			 0x7
 * 2			 0x0
 * 3			 0x1
 * 4			 0x3
 * 5			 0x6
 * 6			 0x5
 * 7			 0x2
 * 8			 0x4
 *
 * Field: ::VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_VCO_CTL . XAUI_0_IF_PLL_P
 */
#define  VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_VCO_CTL_XAUI_0_IF_PLL_P(x)  VTSS_ENCODE_BITFIELD(x,24,3)
#define  VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_VCO_CTL_XAUI_0_IF_PLL_P     VTSS_ENCODE_BITMASK(24,3)
#define  VTSS_X_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_VCO_CTL_XAUI_0_IF_PLL_P(x)  VTSS_EXTRACT_BITFIELD(x,24,3)

/** 
 * \brief
 * Multiply factor select
 *
 * \details 
 * Multiply-factor - M-Value
 * 2- 0x0      3- 0x1	   4-0x2      5-0x5
 * 6- 0xa      7- 0x15	  8-0x2a    9-0x14
 * 10- 0x29  11- 0x13  12-0x26	13-0xc
 * 14- 0x19  15- 0x32  16-0x24	17-0x8
 * 18- 0x11  19- 0x22  20-0x4	 21-0x9
 * 22- 0x12  23- 0x25  24-0xb	 25-0x16
 * 26- 0x2d  27- 0x1b  28-0x36	29-0x2c
 * 30- 0x18  31- 0x31  32-0x23	33-0x7
 * 34- 0xe    35- 0x1d	36-0x3a  37-0x34
 * 38- 0x28  39- 0x10  40-0x21	41-0x3
 * 42- 0x6    43- 0xd	 44-0x1a  45-0x35
 * 46- 0x2b  47- 0x17  48-0x2e	49-0x1c
 * 50- 0x39  51- 0x33  52-0x27	53-0xf
 * 54- 0x1e  55- 0x3d  56-0x3b	57-0x37
 * 58- 0x2f   59- 0x1f	 60-0x3e  61-0x3c
 * 62- 0x38  63- 0x30  64-0x20
 *
 * Field: ::VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_VCO_CTL . XAUI_0_IF_PLL_M
 */
#define  VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_VCO_CTL_XAUI_0_IF_PLL_M(x)  VTSS_ENCODE_BITFIELD(x,16,6)
#define  VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_VCO_CTL_XAUI_0_IF_PLL_M     VTSS_ENCODE_BITMASK(16,6)
#define  VTSS_X_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_VCO_CTL_XAUI_0_IF_PLL_M(x)  VTSS_EXTRACT_BITFIELD(x,16,6)

/** 
 * \brief
 * Unused PLL output divider control
 * Do not modify
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_VCO_CTL . XAUI_0_IF_PLL_N3
 */
#define  VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_VCO_CTL_XAUI_0_IF_PLL_N3(x)  VTSS_ENCODE_BITFIELD(x,8,4)
#define  VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_VCO_CTL_XAUI_0_IF_PLL_N3     VTSS_ENCODE_BITMASK(8,4)
#define  VTSS_X_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_VCO_CTL_XAUI_0_IF_PLL_N3(x)  VTSS_EXTRACT_BITFIELD(x,8,4)

/** 
 * \brief
 * N2 divider control
 *
 * \details 
 * Divide-ratio N-value
 * 2			 0x6
 * 3			 0xa
 * 4			 0x2
 * 5			 0xc
 * 6			 0x4
 * 7			 0x8
 * 8			 0x0
 * 9			 0x9
 * 10			0x1
 * All other values are reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_VCO_CTL . XAUI_0_IF_PLL_N2
 */
#define  VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_VCO_CTL_XAUI_0_IF_PLL_N2(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_VCO_CTL_XAUI_0_IF_PLL_N2     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_VCO_CTL_XAUI_0_IF_PLL_N2(x)  VTSS_EXTRACT_BITFIELD(x,4,4)

/** 
 * \brief
 * N1 divider control
 *
 * \details 
 * Divide-ratio N-value
 * 2			 0x6
 * 3			 0xa
 * 4			 0x2
 * 5			 0xc
 * 6			 0x4
 * 7			 0x8
 * 8			 0x0
 * 9			 0x9
 * 10			0x1
 * All other values are reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_VCO_CTL . XAUI_0_IF_PLL_N1
 */
#define  VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_VCO_CTL_XAUI_0_IF_PLL_N1(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_VCO_CTL_XAUI_0_IF_PLL_N1     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_0_PLL_VCO_CTL_XAUI_0_IF_PLL_N1(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief PLL control register for XAUI 1
 *
 * \details
 * Register: \a DEVCPU_GCB::XAUI_IF_PLL_CTL::XAUI_1_PLL_CTL
 */
#define VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x116)

/** 
 * \brief
 * Used to configure the PLL based on the value of M*N1
 *
 * \details 
 * M*N1   C-value
 * 6-23     0x38
 * 24-40   0x64
 * 41-75   0x74
 * All other values are reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL . XAUI_1_IF_PLL_CONTROL
 */
#define  VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL_XAUI_1_IF_PLL_CONTROL(x)  VTSS_ENCODE_BITFIELD(x,16,8)
#define  VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL_XAUI_1_IF_PLL_CONTROL     VTSS_ENCODE_BITMASK(16,8)
#define  VTSS_X_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL_XAUI_1_IF_PLL_CONTROL(x)  VTSS_EXTRACT_BITFIELD(x,16,8)

/** 
 * \brief
 * Unused PLL output enable - Do not enable
 *
 * \details 
 * 0: Disable (default)
 * 1: Enable
 *
 * Field: ::VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL . XAUI_1_IF_PLL_OUTENDIF_B
 */
#define  VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL_XAUI_1_IF_PLL_OUTENDIF_B(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL_XAUI_1_IF_PLL_OUTENDIF_B     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL_XAUI_1_IF_PLL_OUTENDIF_B(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Unused PLL output enable - Do not enable
 *
 * \details 
 * 0: Disable (default)
 * 1: Enable
 *
 * Field: ::VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL . XAUI_1_IF_PLL_OUTENSE_B
 */
#define  VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL_XAUI_1_IF_PLL_OUTENSE_B(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL_XAUI_1_IF_PLL_OUTENSE_B     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL_XAUI_1_IF_PLL_OUTENSE_B(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Differential output enable
 *
 * \details 
 * 0: Disable differential output
 * 1: Enable differential output
 *
 * Field: ::VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL . XAUI_1_IF_PLL_OUTENDIF
 */
#define  VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL_XAUI_1_IF_PLL_OUTENDIF(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL_XAUI_1_IF_PLL_OUTENDIF     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL_XAUI_1_IF_PLL_OUTENDIF(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Single ended vs differential output control
 *
 * \details 
 * 0: Disable single ended output
 * 1: Enable single ended output
 *
 * Field: ::VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL . XAUI_1_IF_PLL_OUTENSE
 */
#define  VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL_XAUI_1_IF_PLL_OUTENSE(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL_XAUI_1_IF_PLL_OUTENSE     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL_XAUI_1_IF_PLL_OUTENSE(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Reset for XAUI 0 IF PLL
 *
 * \details 
 * 0: Normal operation
 * 1: Reset the IF PLL
 *
 * Field: ::VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL . XAUI_1_IF_PLL_RESET
 */
#define  VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL_XAUI_1_IF_PLL_RESET(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL_XAUI_1_IF_PLL_RESET     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL_XAUI_1_IF_PLL_RESET(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * When asserted, drive PLLOUT with REFCLK.
 *
 * \details 
 * 0: Normal operation
 * 1: Bypass
 *
 * Field: ::VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL . XAUI_1_IF_PLL_BYPASS
 */
#define  VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL_XAUI_1_IF_PLL_BYPASS(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL_XAUI_1_IF_PLL_BYPASS     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_CTL_XAUI_1_IF_PLL_BYPASS(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief XAUI 1 IF PLL VCO control
 *
 * \details
 * Register: \a DEVCPU_GCB::XAUI_IF_PLL_CTL::XAUI_1_PLL_VCO_CTL
 */
#define VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_VCO_CTL  VTSS_IOREG(VTSS_TO_CFG_IF_GCB,0x117)

/** 
 * \brief
 * VCO range selector
 *
 * \details 
 * VCO-range (GHz)   V-value
 * 2.400-2.475		 0xe
 * 2.475-2.575		 0x6
 * 2.575-2.650		 0xa
 * 2.650-2.750		 0x2
 * 2.750-2.900		 0xc
 * 2.900-3.000		 0x4
 * 3.000-3.150		 0x8
 * 3.150-3.300		 0x0
 * Other values are reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_VCO_CTL . XAUI_1_IF_PLL_VCO_RANGE
 */
#define  VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_VCO_CTL_XAUI_1_IF_PLL_VCO_RANGE(x)  VTSS_ENCODE_BITFIELD(x,28,4)
#define  VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_VCO_CTL_XAUI_1_IF_PLL_VCO_RANGE     VTSS_ENCODE_BITMASK(28,4)
#define  VTSS_X_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_VCO_CTL_XAUI_1_IF_PLL_VCO_RANGE(x)  VTSS_EXTRACT_BITFIELD(x,28,4)

/** 
 * \brief
 * Divide ratio select
 *
 * \details 
 * Divide-ratio P-value
 * 1			 0x7
 * 2			 0x0
 * 3			 0x1
 * 4			 0x3
 * 5			 0x6
 * 6			 0x5
 * 7			 0x2
 * 8			 0x4
 *
 * Field: ::VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_VCO_CTL . XAUI_1_IF_PLL_P
 */
#define  VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_VCO_CTL_XAUI_1_IF_PLL_P(x)  VTSS_ENCODE_BITFIELD(x,24,3)
#define  VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_VCO_CTL_XAUI_1_IF_PLL_P     VTSS_ENCODE_BITMASK(24,3)
#define  VTSS_X_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_VCO_CTL_XAUI_1_IF_PLL_P(x)  VTSS_EXTRACT_BITFIELD(x,24,3)

/** 
 * \brief
 * Multiply factor select
 *
 * \details 
 * Multiply-factor - M-Value
 * 2- 0x0      3- 0x1	   4-0x2      5-0x5
 * 6- 0xa      7- 0x15	  8-0x2a    9-0x14
 * 10- 0x29  11- 0x13  12-0x26	13-0xc
 * 14- 0x19  15- 0x32  16-0x24	17-0x8
 * 18- 0x11  19- 0x22  20-0x4	 21-0x9
 * 22- 0x12  23- 0x25  24-0xb	 25-0x16
 * 26- 0x2d  27- 0x1b  28-0x36	29-0x2c
 * 30- 0x18  31- 0x31  32-0x23	33-0x7
 * 34- 0xe    35- 0x1d	36-0x3a  37-0x34
 * 38- 0x28  39- 0x10  40-0x21	41-0x3
 * 42- 0x6    43- 0xd	 44-0x1a  45-0x35
 * 46- 0x2b  47- 0x17  48-0x2e	49-0x1c
 * 50- 0x39  51- 0x33  52-0x27	53-0xf
 * 54- 0x1e  55- 0x3d  56-0x3b	57-0x37
 * 58- 0x2f   59- 0x1f	 60-0x3e  61-0x3c
 * 62- 0x38  63- 0x30  64-0x20
 *
 * Field: ::VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_VCO_CTL . XAUI_1_IF_PLL_M
 */
#define  VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_VCO_CTL_XAUI_1_IF_PLL_M(x)  VTSS_ENCODE_BITFIELD(x,16,6)
#define  VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_VCO_CTL_XAUI_1_IF_PLL_M     VTSS_ENCODE_BITMASK(16,6)
#define  VTSS_X_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_VCO_CTL_XAUI_1_IF_PLL_M(x)  VTSS_EXTRACT_BITFIELD(x,16,6)

/** 
 * \brief
 * Unused PLL output divider control
 * Do not modify
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_VCO_CTL . XAUI_1_IF_PLL_N3
 */
#define  VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_VCO_CTL_XAUI_1_IF_PLL_N3(x)  VTSS_ENCODE_BITFIELD(x,8,4)
#define  VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_VCO_CTL_XAUI_1_IF_PLL_N3     VTSS_ENCODE_BITMASK(8,4)
#define  VTSS_X_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_VCO_CTL_XAUI_1_IF_PLL_N3(x)  VTSS_EXTRACT_BITFIELD(x,8,4)

/** 
 * \brief
 * N2 divider control
 *
 * \details 
 * Divide-ratio N-value
 * 2			 0x6
 * 3			 0xa
 * 4			 0x2
 * 5			 0xc
 * 6			 0x4
 * 7			 0x8
 * 8			 0x0
 * 9			 0x9
 * 10			0x1
 * All other values are reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_VCO_CTL . XAUI_1_IF_PLL_N2
 */
#define  VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_VCO_CTL_XAUI_1_IF_PLL_N2(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_VCO_CTL_XAUI_1_IF_PLL_N2     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_VCO_CTL_XAUI_1_IF_PLL_N2(x)  VTSS_EXTRACT_BITFIELD(x,4,4)

/** 
 * \brief
 * N1 divider control
 *
 * \details 
 * Divide-ratio N-value
 * 2			 0x6
 * 3			 0xa
 * 4			 0x2
 * 5			 0xc
 * 6			 0x4
 * 7			 0x8
 * 8			 0x0
 * 9			 0x9
 * 10			0x1
 * All other values are reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_VCO_CTL . XAUI_1_IF_PLL_N1
 */
#define  VTSS_F_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_VCO_CTL_XAUI_1_IF_PLL_N1(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_VCO_CTL_XAUI_1_IF_PLL_N1     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_DEVCPU_GCB_XAUI_IF_PLL_CTL_XAUI_1_PLL_VCO_CTL_XAUI_1_IF_PLL_N1(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


#endif /* _VTSS_DAYTONA_REGS_DEVCPU_GCB_H_ */
