#ifndef _VTSS_VENICE_REGS_VENICE_DEV1_H_
#define _VTSS_VENICE_REGS_VENICE_DEV1_H_

/*

 Vitesse Switch API software.

 Copyright (c) 2002-2013 Vitesse Semiconductor Corporation "Vitesse". All
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

#include "vtss_venice_regs_common.h"

/*********************************************************************** 
 *
 * Target: \a VENICE_DEV1
 *
 * 
 *
 ***********************************************************************/

/**
 * Register Group: \a VENICE_DEV1:DEV1_IEEE_PMA_CONTROL
 *
 * Device 1 IEEE MDIO Configuration and Status Register set
 */


/** 
 * \brief PMA Control Register 1
 *
 * \details
 * PMA Control Register 1
 *
 * Register: \a VENICE_DEV1:DEV1_IEEE_PMA_CONTROL:PMA_CONTROL_1
 */
#define VTSS_VENICE_DEV1_DEV1_IEEE_PMA_CONTROL_PMA_CONTROL_1  VTSS_IOREG(0x01, 0, 0x0000)

/** 
 * \brief
 * MDIO Managable Device (MMD) software reset.	This register resets all
 * portions of the channel on the line side of the failover mux.  Data path
 * logic and configuration registers are reset. This register is
 * self-clearing.
 *
 * \details 
 * 0: Normal operation
 * 1: Reset
 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_CONTROL_PMA_CONTROL_1 . RST
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_CONTROL_PMA_CONTROL_1_RST  VTSS_BIT(15)

/** 
 * \brief
 * Indicates whether the device operates at 10 Gbps and above. 

 *
 * \details 
 * 0: Unspecified
 * 1: Operation at 10 Gbps and above

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_CONTROL_PMA_CONTROL_1 . SPEED_SEL_A
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_CONTROL_PMA_CONTROL_1_SPEED_SEL_A  VTSS_BIT(13)

/** 
 * \brief
 * The channel's data path is placed into low power mode with this
 * register.  The PMA in this channel is also placed into low power mode
 * regardless of the channel cross connect configuration.  The TXEN control
 * (register 1x0009.0) can be transmitted from a GPIO pin to shut off an
 * optics module's TX driver.  The TXEN signal automatically disables the
 * TX driver when the channel is in low power mode.
 *
 * \details 
 * 1: Low Power Mode.
 * 0: Normal Operation.

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_CONTROL_PMA_CONTROL_1 . LOW_PWR_PMA
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_CONTROL_PMA_CONTROL_1_LOW_PWR_PMA  VTSS_BIT(11)

/** 
 * \brief
 * Indicates whether the device operates at 10 Gbps and above. 

 *
 * \details 
 * 0: Unspecified
 * 1: 10 Gbps an above

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_CONTROL_PMA_CONTROL_1 . SPEED_SEL_B
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_CONTROL_PMA_CONTROL_1_SPEED_SEL_B  VTSS_BIT(6)

/** 
 * \brief
 * Device speed selection

 *
 * \details 
 * 1xxx: Reserved
 * x1xx: Reserved
 * xx1x: Reserved
 * 0001: Reserved
 * 0000: 10 Gbps
 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_CONTROL_PMA_CONTROL_1 . SPEED_SEL_C
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_CONTROL_PMA_CONTROL_1_SPEED_SEL_C(x)  VTSS_ENCODE_BITFIELD(x,2,4)
#define  VTSS_M_VENICE_DEV1_DEV1_IEEE_PMA_CONTROL_PMA_CONTROL_1_SPEED_SEL_C     VTSS_ENCODE_BITMASK(2,4)
#define  VTSS_X_VENICE_DEV1_DEV1_IEEE_PMA_CONTROL_PMA_CONTROL_1_SPEED_SEL_C(x)  VTSS_EXTRACT_BITFIELD(x,2,4)

/** 
 * \brief
 * Enable PMA Pad Loopback H5

 *
 * \details 
 * 0: Disable
 * 1: Enable

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_CONTROL_PMA_CONTROL_1 . EN_PAD_LOOP
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_CONTROL_PMA_CONTROL_1_EN_PAD_LOOP  VTSS_BIT(0)

/**
 * Register Group: \a VENICE_DEV1:DEV1_IEEE_PMA_STATUS
 *
 * Device 1 IEEE MDIO Status Register set
 */


/** 
 * \brief PMA Status Register 1
 *
 * \details
 * PMA Status Register 1
 *
 * Register: \a VENICE_DEV1:DEV1_IEEE_PMA_STATUS:PMA_STATUS_1
 */
#define VTSS_VENICE_DEV1_DEV1_IEEE_PMA_STATUS_PMA_STATUS_1  VTSS_IOREG(0x01, 0, 0x0001)

/** 
 * \brief
 * Indicates a fault condition on either the transmit or the receive paths. 
 *
 * \details 
 * 0: Fault condition not detected
 * 1: Fault condition detected

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_STATUS_PMA_STATUS_1 . FAULT
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_STATUS_PMA_STATUS_1_FAULT  VTSS_BIT(7)

/** 
 * \brief
 * Indicates the receive link status
 * This is a sticky bit that latches the low state. The latch-low bit is
 * cleared when the register is read.

 *
 * \details 
 * 0: PMA/PMD receive link down
 * 1: PMA/PMD receive link up

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_STATUS_PMA_STATUS_1 . RECEIVE_LINK_STATUS
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_STATUS_PMA_STATUS_1_RECEIVE_LINK_STATUS  VTSS_BIT(2)

/** 
 * \brief
 * Indicates PMA/PMD supports Low Power Mode
 *
 * \details 
 * 0: PMA/PMD does not support low power mode
 * 1: PMA/PMD supports low power mode

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_STATUS_PMA_STATUS_1 . LOW_POWER_ABILITY
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_STATUS_PMA_STATUS_1_LOW_POWER_ABILITY  VTSS_BIT(1)

/**
 * Register Group: \a VENICE_DEV1:DEV1_IEEE_PMA_DEVICE_ID
 *
 * Device 1 IEEE MDIO Configuration and Status Register set
 */


/** 
 * \brief PMA Device Identifier 1
 *
 * \details
 * PMA Device Identifier 1

 *
 * Register: \a VENICE_DEV1:DEV1_IEEE_PMA_DEVICE_ID:PMA_DEVICE_ID_1
 */
#define VTSS_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_DEVICE_ID_1  VTSS_IOREG(0x01, 0, 0x0002)

/** 
 * \brief
 * Upper 16 bits of a 32-bit unique PMA device identifier. Bits 3-18 of the
 * device manufacturer's OUI.
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_DEVICE_ID_1 . PMA_DEVICE_ID_1
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_DEVICE_ID_1_PMA_DEVICE_ID_1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_DEVICE_ID_1_PMA_DEVICE_ID_1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_DEVICE_ID_1_PMA_DEVICE_ID_1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief PMA Device Identifier 2
 *
 * \details
 * PMA Device Identifier 2

 *
 * Register: \a VENICE_DEV1:DEV1_IEEE_PMA_DEVICE_ID:PMA_DEVICE_ID_2
 */
#define VTSS_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_DEVICE_ID_2  VTSS_IOREG(0x01, 0, 0x0003)

/** 
 * \brief
 * Lower 16 bits of a 32-bit unique PMA device identifier. Bits 19-24 of
 * the device manufacturer's OUI. Six-bit model number, and a four-bit
 * revision number.
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_DEVICE_ID_2 . PMA_DEVICE_ID_2
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_DEVICE_ID_2_PMA_DEVICE_ID_2(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_DEVICE_ID_2_PMA_DEVICE_ID_2     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_DEVICE_ID_2_PMA_DEVICE_ID_2(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief PMA/PMD Speed Ability
 *
 * \details
 * PMA/PMD Speed Ability

 *
 * Register: \a VENICE_DEV1:DEV1_IEEE_PMA_DEVICE_ID:PMA_PMD_SPEED_ABILITY
 */
#define VTSS_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_PMD_SPEED_ABILITY  VTSS_IOREG(0x01, 0, 0x0004)

/** 
 * \brief
 * Indicates PMA/PMD capability to run at 10 Gbps

 *
 * \details 
 * 0: PMA/PMD is not capable of operating at 10 Gbps
 * 1: PMA/PMD is capable of operating at 10 Gbps

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_PMD_SPEED_ABILITY . ETH_10G_CAPABLE
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_PMD_SPEED_ABILITY_ETH_10G_CAPABLE  VTSS_BIT(0)


/** 
 * \brief PMA/PMD Devices In Package 1
 *
 * \details
 * PMA/PMD Devices In Package 1
 *
 * Register: \a VENICE_DEV1:DEV1_IEEE_PMA_DEVICE_ID:PMA_PMD_DEV_IN_PACKAGE_1
 */
#define VTSS_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_PMD_DEV_IN_PACKAGE_1  VTSS_IOREG(0x01, 0, 0x0005)

/** 
 * \brief
 * Indicates if the DTE XS is present

 *
 * \details 
 * 0: DTE XS is not present in package
 * 1: DTE XS is present in package

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_PMD_DEV_IN_PACKAGE_1 . DTE_XS_PRESENT
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_PMD_DEV_IN_PACKAGE_1_DTE_XS_PRESENT  VTSS_BIT(5)

/** 
 * \brief
 * Indicates if the PHY XS is present

 *
 * \details 
 * 0: PHY XS is not present in package
 * 1: PHY iXS is present in package

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_PMD_DEV_IN_PACKAGE_1 . PHY_XS_PRESENT
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_PMD_DEV_IN_PACKAGE_1_PHY_XS_PRESENT  VTSS_BIT(4)

/** 
 * \brief
 * Indicates if the PCS is present

 *
 * \details 
 * 0: PCS is not present in package
 * 1: PCS is present in package

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_PMD_DEV_IN_PACKAGE_1 . PCS_PRESENT
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_PMD_DEV_IN_PACKAGE_1_PCS_PRESENT  VTSS_BIT(3)

/** 
 * \brief
 * Indicates if the WIS is present
 *
 * \details 
 * 0: WIS is not present in package
 * 1: WIS is present in package

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_PMD_DEV_IN_PACKAGE_1 . WIS_PRESENT
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_PMD_DEV_IN_PACKAGE_1_WIS_PRESENT  VTSS_BIT(2)

/** 
 * \brief
 * Indicates if the PMA/PMD is present

 *
 * \details 
 * 0: PMD/PMA is not present in package
 * 1: PMD/PMA is present in package

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_PMD_DEV_IN_PACKAGE_1 . PMD_PMA_PRESENT
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_PMD_DEV_IN_PACKAGE_1_PMD_PMA_PRESENT  VTSS_BIT(1)

/** 
 * \brief
 * Indicates if the clause 22 registers are present
 *
 * \details 
 * 0: Clause 22 registers are not present in package
 * 1: Clause 22 registers are present in package

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_PMD_DEV_IN_PACKAGE_1 . CLAUSE_22_REG_PRESENT
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_PMD_DEV_IN_PACKAGE_1_CLAUSE_22_REG_PRESENT  VTSS_BIT(0)


/** 
 * \brief PMA/PMD Devices In Package 2
 *
 * \details
 * PMA/PMD Devices In Package 2

 *
 * Register: \a VENICE_DEV1:DEV1_IEEE_PMA_DEVICE_ID:PMA_PMD_DEV_IN_PACKAGE_2
 */
#define VTSS_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_PMD_DEV_IN_PACKAGE_2  VTSS_IOREG(0x01, 0, 0x0006)

/** 
 * \brief
 * Indicates if the vendor specific device 2 is present
 *
 * \details 
 * 0: Vendor specific device 2 is not present in package
 * 1: Vendor specific device 2 is present in package

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_PMD_DEV_IN_PACKAGE_2 . VENDOR_SPECIFIC_DEV2_PRESENT
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_PMD_DEV_IN_PACKAGE_2_VENDOR_SPECIFIC_DEV2_PRESENT  VTSS_BIT(15)

/** 
 * \brief
 * Indicates if the vendor specific device 1 is present

 *
 * \details 
 * 0: Vendor specific device 1 is not present in package
 * 1: Vendor specific device 1 is present in package

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_PMD_DEV_IN_PACKAGE_2 . VENDOR_SPECIFIC_DEV1_PRESENT
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_PMD_DEV_IN_PACKAGE_2_VENDOR_SPECIFIC_DEV1_PRESENT  VTSS_BIT(14)


/** 
 * \brief PMA/PMD Control 2
 *
 * \details
 * PMA/PMD Control 2

 *
 * Register: \a VENICE_DEV1:DEV1_IEEE_PMA_DEVICE_ID:PMA_PMD_CONTROL_2
 */
#define VTSS_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_PMD_CONTROL_2  VTSS_IOREG(0x01, 0, 0x0007)

/** 
 * \brief
 * Indicates the PMA type selected
 * WAN mode is enabled when 10GBASE-SW, 10GBASE-LW or 10GBASE-EW is
 * selected.
 *
 * \details 
 * 1111: 10BASE-T (not supported)
 * 1110: 100BASE-TX (not supported)
 * 1101: 1000BASE-KX
 * 1001: 10GBASE-KR
 * 1010: 10GBASE-KX4 (not supported)
 * 1001: 10GBASE-T (not supported)
 * 1000: 10GBASE-LRM
 * 0111: 10GBASE-SR
 * 0110: 10GBASE-LR
 * 0101: 10GBASE-ER
 * 0100: 10GBASE-LX-4
 * 0011: 10GBASE-SW
 * 0010: 10GBASE-LW
 * 0001: 10GBASE-EW
 * 0000: Reserved
 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_PMD_CONTROL_2 . VENDOR_SPECIFIC_DEV2_PRESENT_CTRL
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_PMD_CONTROL_2_VENDOR_SPECIFIC_DEV2_PRESENT_CTRL(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_PMD_CONTROL_2_VENDOR_SPECIFIC_DEV2_PRESENT_CTRL     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_VENICE_DEV1_DEV1_IEEE_PMA_DEVICE_ID_PMA_PMD_CONTROL_2_VENDOR_SPECIFIC_DEV2_PRESENT_CTRL(x)  VTSS_EXTRACT_BITFIELD(x,0,4)

/**
 * Register Group: \a VENICE_DEV1:DEV1_IEEE_PMA_PMD_STATUS
 *
 * Device 1 IEEE MDIO Configuration and Status Register set
 */


/** 
 * \brief PMA/PMD Status 2
 *
 * \details
 * PMA/PMD Status 2

 *
 * Register: \a VENICE_DEV1:DEV1_IEEE_PMA_PMD_STATUS:PMA_PMD_STATUS_2
 */
#define VTSS_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2  VTSS_IOREG(0x01, 0, 0x0008)

/** 
 * \brief
 * Indicates if the PMA device is present
 *
 * \details 
 * 00: Device not Present
 * 01: Reserved
 * 10: Device Present
 * 11: Reserved
 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2 . DEVICE_PRESENT
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2_DEVICE_PRESENT(x)  VTSS_ENCODE_BITFIELD(x,14,2)
#define  VTSS_M_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2_DEVICE_PRESENT     VTSS_ENCODE_BITMASK(14,2)
#define  VTSS_X_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2_DEVICE_PRESENT(x)  VTSS_EXTRACT_BITFIELD(x,14,2)

/** 
 * \brief
 * PMA/PMD transmit path fault detection ability
 *
 * \details 
 * 0: PMA/PMD does not have the ability to detect a fault condition on the
 * transmit path
 * 1: PMA/PMD has the ability to detect a fault condition on the transmit
 * path

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2 . TRANSMIT_FAULT_ABILITY
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2_TRANSMIT_FAULT_ABILITY  VTSS_BIT(13)

/** 
 * \brief
 * PMA/PMD receive path fault detection ability

 *
 * \details 
 * 0: PMA/PMD does not have the ability to detect a fault condition on the
 * receive path
 * 1: PMA/PMD has the ability to detect a fault condition on the receive
 * path

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2 . RECEIVE_FAULT_ABILITY
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2_RECEIVE_FAULT_ABILITY  VTSS_BIT(12)

/** 
 * \brief
 * Indicates a fault condition on the transmit path
 * This is a sticky bit that latches the high state.  The latch-high bit is
 * cleared when the register is read.
 *
 * \details 
 * 0: No fault condition on transmit path
 * 1: Fault condition on transmit path

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2 . TRANSMIT_FAULT
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2_TRANSMIT_FAULT  VTSS_BIT(11)

/** 
 * \brief
 * Indicates a fault condition on the receive path 
 * This is a sticky bit that latches the high state.  The latch-high bit is
 * cleared when the register is read.
 *
 * \details 
 * 0: No fault condition on receive path
 * 1: Fault condition on receive path
 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2 . RECEIVE_FAULT
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2_RECEIVE_FAULT  VTSS_BIT(10)

/** 
 * \brief
 * Disable the PMA/PMD transmit path ability
 *
 * \details 
 * 0: PMD does not have the ability to disable the transmit path
 * 1: PMD has the ability to disable the transmit path

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2 . PMD_TRANSMIT_DISABLE_ABILITY
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2_PMD_TRANSMIT_DISABLE_ABILITY  VTSS_BIT(8)

/** 
 * \brief
 * 10GBASE-SR ability
 *
 * \details 
 * 0: PMA/PMD is not able to perform 10GBASE-SR
 * 1: PMA/PMD is able to perform 10GBASE-SR

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2 . ETH_10GBASE_SR_ABILITY
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2_ETH_10GBASE_SR_ABILITY  VTSS_BIT(7)

/** 
 * \brief
 * 10GBASE-LR ability

 *
 * \details 
 * 0: PMA/PMD is not able to perform 10GBASE-LR
 * 1: PMA/PMD is able to perform 10GBASE-LR

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2 . ETH_10GBASE_LR_ABILITY
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2_ETH_10GBASE_LR_ABILITY  VTSS_BIT(6)

/** 
 * \brief
 * 10GBASE-ER ability

 *
 * \details 
 * 0: PMA/PMD is not able to perform 10GBASE-ER
 * 1: PMA/PMD is able to perform 10GBASE-ER

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2 . ETH_10GBASE_ER_ABILITY
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2_ETH_10GBASE_ER_ABILITY  VTSS_BIT(5)

/** 
 * \brief
 * 10GBASE-LX4 ability

 *
 * \details 
 * 0: PMA/PMD is not able to perform 10GBASE-LX4
 * 1: PMA/PMD is able to perform 10GBASE-LX4

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2 . ETH_10GBASE_LX4_ABILITY
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2_ETH_10GBASE_LX4_ABILITY  VTSS_BIT(4)

/** 
 * \brief
 * 10GBASE-SW ability

 *
 * \details 
 * 0: PMA/PMD is not able to perform 10GBASE-SW
 * 1: PMA/PMD is able to perform 10GBASE-SW

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2 . ETH_10GBASE_SW_ABILITY
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2_ETH_10GBASE_SW_ABILITY  VTSS_BIT(3)

/** 
 * \brief
 * 10GBASE-LW ability

 *
 * \details 
 * 0: PMA/PMD is not able to perform 10GBASE-LW
 * 1: PMA/PMD is able to perform 10GBASE-LW

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2 . ETH_10GBASE_LW_ABILITY
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2_ETH_10GBASE_LW_ABILITY  VTSS_BIT(2)

/** 
 * \brief
 * 10GBASE-EW ability

 *
 * \details 
 * 0: PMA/PMD is not able to perform 10GBASE-EW
 * 1: PMA/PMD is able to perform 10GBASE-EW

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2 . ETH_10GBASE_EW_ABILITY
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2_ETH_10GBASE_EW_ABILITY  VTSS_BIT(1)

/** 
 * \brief
 * Ability to perform a loopback function

 *
 * \details 
 * 0: PMA does not have the ability to perform a loopback function
 * 1: PMA has the ability to perform a loopback function

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2 . PMA_LOOPBACK_ABILITY
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_PMD_STATUS_PMA_PMD_STATUS_2_PMA_LOOPBACK_ABILITY  VTSS_BIT(0)

/**
 * Register Group: \a VENICE_DEV1:DEV1_IEEE_PMD_CONTROL_AND_STATUS
 *
 * Device 1 IEEE MDIO Configuration and Status Register set
 */


/** 
 * \brief PMD Transmit Disable
 *
 * \details
 * PMD Transmit Disable

 *
 * Register: \a VENICE_DEV1:DEV1_IEEE_PMD_CONTROL_AND_STATUS:PMD_TRANSMIT_DISABLE
 */
#define VTSS_VENICE_DEV1_DEV1_IEEE_PMD_CONTROL_AND_STATUS_PMD_TRANSMIT_DISABLE  VTSS_IOREG(0x01, 0, 0x0009)

/** 
 * \brief
 * Value always 0, writes ignored.

 *
 * \details 
 * 0: Normal operation
 * 1: Transmit disable

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMD_CONTROL_AND_STATUS_PMD_TRANSMIT_DISABLE . PMD_TRANSMIT_DISABLE_3
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMD_CONTROL_AND_STATUS_PMD_TRANSMIT_DISABLE_PMD_TRANSMIT_DISABLE_3  VTSS_BIT(4)

/** 
 * \brief
 * Value always 0, writes ignored.

 *
 * \details 
 * 0: Normal operation
 * 1: Transmit disable

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMD_CONTROL_AND_STATUS_PMD_TRANSMIT_DISABLE . PMD_TRANSMIT_DISABLE_2
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMD_CONTROL_AND_STATUS_PMD_TRANSMIT_DISABLE_PMD_TRANSMIT_DISABLE_2  VTSS_BIT(3)

/** 
 * \brief
 * Value always 0, writes ignored.

 *
 * \details 
 * 0: Normal operation
 * 1: Transmit disable

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMD_CONTROL_AND_STATUS_PMD_TRANSMIT_DISABLE . PMD_TRANSMIT_DISABLE_1
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMD_CONTROL_AND_STATUS_PMD_TRANSMIT_DISABLE_PMD_TRANSMIT_DISABLE_1  VTSS_BIT(2)

/** 
 * \brief
 * Value always 0, writes ignored.

 *
 * \details 
 * 0: Normal operation
 * 1: Transmit disable

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMD_CONTROL_AND_STATUS_PMD_TRANSMIT_DISABLE . PMD_TRANSMIT_DISABLE_0
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMD_CONTROL_AND_STATUS_PMD_TRANSMIT_DISABLE_PMD_TRANSMIT_DISABLE_0  VTSS_BIT(1)

/** 
 * \brief
 * PMD Transmit Disable.  This register bit can be transmitted from a GPIO
 * pin to shut off an optics module's TX driver.  This TXEN signal
 * automatically disables the TX driver when the channel is in low power
 * mode.  The GPIO configuration controls whether the transmitted signal is
 * active high or active low.
 *
 * \details 
 * 0: Transmit enabled
 * 1: Transmit disabled
 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMD_CONTROL_AND_STATUS_PMD_TRANSMIT_DISABLE . GLOBAL_PMD_TRANSMIT_DISABLE
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMD_CONTROL_AND_STATUS_PMD_TRANSMIT_DISABLE_GLOBAL_PMD_TRANSMIT_DISABLE  VTSS_BIT(0)


/** 
 * \brief PMD Receive Signal Detect 
 *
 * \details
 * PMD Receive Signal Detect 
 *
 * Register: \a VENICE_DEV1:DEV1_IEEE_PMD_CONTROL_AND_STATUS:PMD_RECEIVE_SIGNAL_DETECT
 */
#define VTSS_VENICE_DEV1_DEV1_IEEE_PMD_CONTROL_AND_STATUS_PMD_RECEIVE_SIGNAL_DETECT  VTSS_IOREG(0x01, 0, 0x000a)

/** 
 * \brief
 * Do not support this function, value always 0

 *
 * \details 
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMD_CONTROL_AND_STATUS_PMD_RECEIVE_SIGNAL_DETECT . PMD_RECEIVE_SIGNAL_DETECT_3
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMD_CONTROL_AND_STATUS_PMD_RECEIVE_SIGNAL_DETECT_PMD_RECEIVE_SIGNAL_DETECT_3  VTSS_BIT(4)

/** 
 * \brief
 * Do not support this function, value always 0

 *
 * \details 
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMD_CONTROL_AND_STATUS_PMD_RECEIVE_SIGNAL_DETECT . PMD_RECEIVE_SIGNAL_DETECT_2
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMD_CONTROL_AND_STATUS_PMD_RECEIVE_SIGNAL_DETECT_PMD_RECEIVE_SIGNAL_DETECT_2  VTSS_BIT(3)

/** 
 * \brief
 * Do not support this function, value always 0

 *
 * \details 
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMD_CONTROL_AND_STATUS_PMD_RECEIVE_SIGNAL_DETECT . PMD_RECEIVE_SIGNAL_DETECT_1
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMD_CONTROL_AND_STATUS_PMD_RECEIVE_SIGNAL_DETECT_PMD_RECEIVE_SIGNAL_DETECT_1  VTSS_BIT(2)

/** 
 * \brief
 * Do not support this function, value always 0

 *
 * \details 
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMD_CONTROL_AND_STATUS_PMD_RECEIVE_SIGNAL_DETECT . PMD_RECEIVE_SIGNAL_DETECT_0
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMD_CONTROL_AND_STATUS_PMD_RECEIVE_SIGNAL_DETECT_PMD_RECEIVE_SIGNAL_DETECT_0  VTSS_BIT(1)

/** 
 * \brief
 * PMD receiver signal detect

 *
 * \details 
 * 0: Signal not detected by receiver
 * 1: Signal detected by receiver

 *
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMD_CONTROL_AND_STATUS_PMD_RECEIVE_SIGNAL_DETECT . GLOBAL_PMD_RECEIVE_SIGNAL_DETECT
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMD_CONTROL_AND_STATUS_PMD_RECEIVE_SIGNAL_DETECT_GLOBAL_PMD_RECEIVE_SIGNAL_DETECT  VTSS_BIT(0)

/**
 * Register Group: \a VENICE_DEV1:DEV1_IEEE_PMA_PMD_PACKAGE_ID
 *
 * Device 1 IEEE MDIO Configuration and Status Register set
 */


/** 
 * \brief PMA/PMD Package Identifier 1
 *
 * \details
 * PMA/PMD Package Identifier 1

 *
 * Register: \a VENICE_DEV1:DEV1_IEEE_PMA_PMD_PACKAGE_ID:PMA_PMD_PACKAGE_ID_1
 */
#define VTSS_VENICE_DEV1_DEV1_IEEE_PMA_PMD_PACKAGE_ID_PMA_PMD_PACKAGE_ID_1  VTSS_IOREG(0x01, 0, 0x001e)

/** 
 * \brief
 * PMA/PMD package identifier 1

 *
 * \details 
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_PMD_PACKAGE_ID_PMA_PMD_PACKAGE_ID_1 . PMA_PMD_PACKAGE_ID_1
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_PMD_PACKAGE_ID_PMA_PMD_PACKAGE_ID_1_PMA_PMD_PACKAGE_ID_1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_DEV1_IEEE_PMA_PMD_PACKAGE_ID_PMA_PMD_PACKAGE_ID_1_PMA_PMD_PACKAGE_ID_1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_DEV1_IEEE_PMA_PMD_PACKAGE_ID_PMA_PMD_PACKAGE_ID_1_PMA_PMD_PACKAGE_ID_1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief PMA/PMD package identifier 2
 *
 * \details
 * PMA/PMD package identifier 2

 *
 * Register: \a VENICE_DEV1:DEV1_IEEE_PMA_PMD_PACKAGE_ID:PMA_PMD_PACKAGE_ID_2
 */
#define VTSS_VENICE_DEV1_DEV1_IEEE_PMA_PMD_PACKAGE_ID_PMA_PMD_PACKAGE_ID_2  VTSS_IOREG(0x01, 0, 0x001f)

/** 
 * \brief
 * PMA/PMD Package Identifier 2

 *
 * \details 
 * Field: VTSS_VENICE_DEV1_DEV1_IEEE_PMA_PMD_PACKAGE_ID_PMA_PMD_PACKAGE_ID_2 . PMA_PMD_PACKAGE_ID_2
 */
#define  VTSS_F_VENICE_DEV1_DEV1_IEEE_PMA_PMD_PACKAGE_ID_PMA_PMD_PACKAGE_ID_2_PMA_PMD_PACKAGE_ID_2(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_DEV1_IEEE_PMA_PMD_PACKAGE_ID_PMA_PMD_PACKAGE_ID_2_PMA_PMD_PACKAGE_ID_2     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_DEV1_IEEE_PMA_PMD_PACKAGE_ID_PMA_PMD_PACKAGE_ID_2_PMA_PMD_PACKAGE_ID_2(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VENICE_DEV1:KR_FEC_ABILITY
 *
 * KR FEC IEEE ABILITY REGISTER
 */


/** 
 * \brief KR FEC ability
 *
 * \details
 * Register: \a VENICE_DEV1:KR_FEC_ABILITY:KR_FEC_ABILITY
 */
#define VTSS_VENICE_DEV1_KR_FEC_ABILITY_KR_FEC_ABILITY  VTSS_IOREG(0x01, 0, 0x00aa)

/** 
 * \brief
 * FEC error reporting ability
 *
 * \details 
 * 
 * 0: This PHY device is not able to report FEC decoding errors to the PCS
 * layer.
 * 1: This PHY device is able to report FEC decoding errors to the PCS
 * layer.

 *
 * Field: VTSS_VENICE_DEV1_KR_FEC_ABILITY_KR_FEC_ABILITY . FEC_error_indication_ability
 */
#define  VTSS_F_VENICE_DEV1_KR_FEC_ABILITY_KR_FEC_ABILITY_FEC_error_indication_ability  VTSS_BIT(1)

/** 
 * \brief
 * FEC ability
 *
 * \details 
 * 
 * 0: This PHY device does not support FEC.
 * 1: This PHY device supports FEC.

 *
 * Field: VTSS_VENICE_DEV1_KR_FEC_ABILITY_KR_FEC_ABILITY . FEC_ability
 */
#define  VTSS_F_VENICE_DEV1_KR_FEC_ABILITY_KR_FEC_ABILITY_FEC_ability  VTSS_BIT(0)

/**
 * Register Group: \a VENICE_DEV1:KR_FEC_CONTROL_1
 *
 * KR FEC IEEE CONTROL REGISTER
 */


/** 
 * \brief KR FEC control 1
 *
 * \details
 * Register: \a VENICE_DEV1:KR_FEC_CONTROL_1:KR_FEC_control_1
 */
#define VTSS_VENICE_DEV1_KR_FEC_CONTROL_1_KR_FEC_control_1  VTSS_IOREG(0x01, 0, 0x00ab)

/** 
 * \details 
 * 
 * 0: Decoding errors have no effect on PCS sync bits
 * 1: Enable decoder to indicate errors to PCS sync bits

 *
 * Field: VTSS_VENICE_DEV1_KR_FEC_CONTROL_1_KR_FEC_control_1 . FEC_enable_error_indication
 */
#define  VTSS_F_VENICE_DEV1_KR_FEC_CONTROL_1_KR_FEC_control_1_FEC_enable_error_indication  VTSS_BIT(1)

/** 
 * \brief
 * FEC enable
 *
 * \details 
 * 
 * 0: Disable FEC
 * 1: Enable FEC

 *
 * Field: VTSS_VENICE_DEV1_KR_FEC_CONTROL_1_KR_FEC_control_1 . FEC_enable
 */
#define  VTSS_F_VENICE_DEV1_KR_FEC_CONTROL_1_KR_FEC_control_1_FEC_enable  VTSS_BIT(0)

/**
 * Register Group: \a VENICE_DEV1:KR_FEC_STATUS
 *
 * KR FEC IEEE STATUS REGISTER
 */


/** 
 * \brief KR FEC corrected lower
 *
 * \details
 * Register: \a VENICE_DEV1:KR_FEC_STATUS:KR_FEC_corrected_lower
 */
#define VTSS_VENICE_DEV1_KR_FEC_STATUS_KR_FEC_corrected_lower  VTSS_IOREG(0x01, 0, 0x00ac)

/** 
 * \brief
 * The FEC corrected block count is split across two registers, 1x00AC and
 * 1x00AD.  1x00AC contains the least significant 16 bits of the count. 
 * 1x00AD contains the most significant 16 bits of the count.
 * Reading address 1x00AC latches the 16 most significant bits of the
 * counter in 1x00AD for future read out.  The block count register is
 * cleared when 1x00AC is read.
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_KR_FEC_STATUS_KR_FEC_corrected_lower . FEC_corrected_blocks_lower
 */
#define  VTSS_F_VENICE_DEV1_KR_FEC_STATUS_KR_FEC_corrected_lower_FEC_corrected_blocks_lower(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_KR_FEC_STATUS_KR_FEC_corrected_lower_FEC_corrected_blocks_lower     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_KR_FEC_STATUS_KR_FEC_corrected_lower_FEC_corrected_blocks_lower(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief KR FEC corrected upper
 *
 * \details
 * Register: \a VENICE_DEV1:KR_FEC_STATUS:KR_FEC_corrected_upper
 */
#define VTSS_VENICE_DEV1_KR_FEC_STATUS_KR_FEC_corrected_upper  VTSS_IOREG(0x01, 0, 0x00ad)

/** 
 * \brief
 * The FEC corrected block count is split across two registers, 1x00AC and
 * 1x00AD.  1x00AC contains the least significant 16 bits of the count. 
 * 1x00AD contains the most significant 16 bits of the count.
 * Reading address 1x00AC latches the 16 most significant bits of the
 * counter in 1x00AD for future read out.  The block count register is
 * cleared when 1x00AC is read.
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_KR_FEC_STATUS_KR_FEC_corrected_upper . FEC_corrected_blocks_upper
 */
#define  VTSS_F_VENICE_DEV1_KR_FEC_STATUS_KR_FEC_corrected_upper_FEC_corrected_blocks_upper(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_KR_FEC_STATUS_KR_FEC_corrected_upper_FEC_corrected_blocks_upper     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_KR_FEC_STATUS_KR_FEC_corrected_upper_FEC_corrected_blocks_upper(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief KR FEC uncorrected lower
 *
 * \details
 * Register: \a VENICE_DEV1:KR_FEC_STATUS:KR_FEC_uncorrected_lower
 */
#define VTSS_VENICE_DEV1_KR_FEC_STATUS_KR_FEC_uncorrected_lower  VTSS_IOREG(0x01, 0, 0x00ae)

/** 
 * \brief
 * The FEC uncorrectable block count is split across two registers, 1x00AE
 * and 1x00AF.	1x00AE contains the least significant 16 bits of the count.
 *  1x00AF contains the most significant 16 bits of the count.
 * Reading address 1x00AE latches the 16 most significant bits of the
 * counter in 1x00AF for future read out.  The block count register is
 * cleared when 1x00AE is read.
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_KR_FEC_STATUS_KR_FEC_uncorrected_lower . FEC_uncorrected_blocks_lower
 */
#define  VTSS_F_VENICE_DEV1_KR_FEC_STATUS_KR_FEC_uncorrected_lower_FEC_uncorrected_blocks_lower(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_KR_FEC_STATUS_KR_FEC_uncorrected_lower_FEC_uncorrected_blocks_lower     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_KR_FEC_STATUS_KR_FEC_uncorrected_lower_FEC_uncorrected_blocks_lower(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief KR FEC uncorrected upper
 *
 * \details
 * Register: \a VENICE_DEV1:KR_FEC_STATUS:KR_FEC_uncorrected_upper
 */
#define VTSS_VENICE_DEV1_KR_FEC_STATUS_KR_FEC_uncorrected_upper  VTSS_IOREG(0x01, 0, 0x00af)

/** 
 * \brief
 * The FEC uncorrectable block count is split across two registers, 1x00AE
 * and 1x00AF.	1x00AE contains the least significant 16 bits of the count.
 *  1x00AF contains the most significant 16 bits of the count.
 * Reading address 1x00AE latches the 16 most significant bits of the
 * counter in 1x00AF for future read out.  The block count register is
 * cleared when 1x00AE is read.
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_KR_FEC_STATUS_KR_FEC_uncorrected_upper . FEC_uncorrected_blocks_upper
 */
#define  VTSS_F_VENICE_DEV1_KR_FEC_STATUS_KR_FEC_uncorrected_upper_FEC_uncorrected_blocks_upper(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_KR_FEC_STATUS_KR_FEC_uncorrected_upper_FEC_uncorrected_blocks_upper     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_KR_FEC_STATUS_KR_FEC_uncorrected_upper_FEC_uncorrected_blocks_upper(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VENICE_DEV1:KR_FEC_CONTROL_2
 *
 * KR FEC Configuration and Status Vendor Register set
 */


/** 
 * \brief KR_FEC_Control_2
 *
 * \details
 * Register: \a VENICE_DEV1:KR_FEC_CONTROL_2:KR_FEC_Control_2
 */
#define VTSS_VENICE_DEV1_KR_FEC_CONTROL_2_KR_FEC_Control_2  VTSS_IOREG(0x01, 0, 0x8300)

/** 
 * \brief
 * FEC in frame lock indication
 * This is a sticky bit that latches the low state. The latch-low bit is
 * cleared when the register is read.

 *
 * \details 
 * 
 * 0: FEC has not achieved lock
 * 1: FEC has achieved lock

 *
 * Field: VTSS_VENICE_DEV1_KR_FEC_CONTROL_2_KR_FEC_Control_2 . fec_inframe
 */
#define  VTSS_F_VENICE_DEV1_KR_FEC_CONTROL_2_KR_FEC_Control_2_fec_inframe  VTSS_BIT(1)

/** 
 * \brief
 * FEC counters reset
 *
 * \details 
 * 
 * 0: no effect
 * 1: reset FEC counters

 *
 * Field: VTSS_VENICE_DEV1_KR_FEC_CONTROL_2_KR_FEC_Control_2 . fec_rstmon
 */
#define  VTSS_F_VENICE_DEV1_KR_FEC_CONTROL_2_KR_FEC_Control_2_fec_rstmon  VTSS_BIT(0)

/**
 * Register Group: \a VENICE_DEV1:RX_ALARM_Control_Register
 *
 * Not documented
 */


/** 
 * \details
 * Register: \a VENICE_DEV1:RX_ALARM_Control_Register:RX_ALARM_Control_Register
 */
#define VTSS_VENICE_DEV1_RX_ALARM_Control_Register_RX_ALARM_Control_Register  VTSS_IOREG(0x01, 0, 0x9000)

/** 
 * \brief
 * Vendor specific
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: VTSS_VENICE_DEV1_RX_ALARM_Control_Register_RX_ALARM_Control_Register . Vendor_Specific
 */
#define  VTSS_F_VENICE_DEV1_RX_ALARM_Control_Register_RX_ALARM_Control_Register_Vendor_Specific  VTSS_BIT(10)

/** 
 * \brief
 * WIS Local Fault Enable

 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: VTSS_VENICE_DEV1_RX_ALARM_Control_Register_RX_ALARM_Control_Register . WIS_Local_Fault_Enable
 */
#define  VTSS_F_VENICE_DEV1_RX_ALARM_Control_Register_RX_ALARM_Control_Register_WIS_Local_Fault_Enable  VTSS_BIT(9)

/** 
 * \brief
 * Vendor Specific

 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: VTSS_VENICE_DEV1_RX_ALARM_Control_Register_RX_ALARM_Control_Register . Vendor_Specific_idx2
 */
#define  VTSS_F_VENICE_DEV1_RX_ALARM_Control_Register_RX_ALARM_Control_Register_Vendor_Specific_idx2(x)  VTSS_ENCODE_BITFIELD(x,5,4)
#define  VTSS_M_VENICE_DEV1_RX_ALARM_Control_Register_RX_ALARM_Control_Register_Vendor_Specific_idx2     VTSS_ENCODE_BITMASK(5,4)
#define  VTSS_X_VENICE_DEV1_RX_ALARM_Control_Register_RX_ALARM_Control_Register_Vendor_Specific_idx2(x)  VTSS_EXTRACT_BITFIELD(x,5,4)

/** 
 * \brief
 * PMA/PMD Receiver Local Fault Enable

 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: VTSS_VENICE_DEV1_RX_ALARM_Control_Register_RX_ALARM_Control_Register . PMA_PMD_Receiver_Local_Fault_Enable
 */
#define  VTSS_F_VENICE_DEV1_RX_ALARM_Control_Register_RX_ALARM_Control_Register_PMA_PMD_Receiver_Local_Fault_Enable  VTSS_BIT(4)

/** 
 * \brief
 * PCS Receive Local Fault Enable

 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: VTSS_VENICE_DEV1_RX_ALARM_Control_Register_RX_ALARM_Control_Register . PCS_Receive_Local_Fault_Enable
 */
#define  VTSS_F_VENICE_DEV1_RX_ALARM_Control_Register_RX_ALARM_Control_Register_PCS_Receive_Local_Fault_Enable  VTSS_BIT(3)

/** 
 * \brief
 * Vendor Specific

 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: VTSS_VENICE_DEV1_RX_ALARM_Control_Register_RX_ALARM_Control_Register . Vendor_Specific_idx3
 */
#define  VTSS_F_VENICE_DEV1_RX_ALARM_Control_Register_RX_ALARM_Control_Register_Vendor_Specific_idx3(x)  VTSS_ENCODE_BITFIELD(x,1,2)
#define  VTSS_M_VENICE_DEV1_RX_ALARM_Control_Register_RX_ALARM_Control_Register_Vendor_Specific_idx3     VTSS_ENCODE_BITMASK(1,2)
#define  VTSS_X_VENICE_DEV1_RX_ALARM_Control_Register_RX_ALARM_Control_Register_Vendor_Specific_idx3(x)  VTSS_EXTRACT_BITFIELD(x,1,2)

/** 
 * \brief
 * PHY XS Receive Local Fault Enable

 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: VTSS_VENICE_DEV1_RX_ALARM_Control_Register_RX_ALARM_Control_Register . PHY_XS_Receive_Local_Fault_Enable
 */
#define  VTSS_F_VENICE_DEV1_RX_ALARM_Control_Register_RX_ALARM_Control_Register_PHY_XS_Receive_Local_Fault_Enable  VTSS_BIT(0)

/**
 * Register Group: \a VENICE_DEV1:TX_ALARM_Control_Register
 *
 * Not documented
 */


/** 
 * \brief TX_ALARM Control Register
 *
 * \details
 * Register: \a VENICE_DEV1:TX_ALARM_Control_Register:TX_ALARM_Control_Register
 */
#define VTSS_VENICE_DEV1_TX_ALARM_Control_Register_TX_ALARM_Control_Register  VTSS_IOREG(0x01, 0, 0x9001)

/** 
 * \brief
 * Vendor Specific

 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: VTSS_VENICE_DEV1_TX_ALARM_Control_Register_TX_ALARM_Control_Register . Vendor_Specific
 */
#define  VTSS_F_VENICE_DEV1_TX_ALARM_Control_Register_TX_ALARM_Control_Register_Vendor_Specific(x)  VTSS_ENCODE_BITFIELD(x,5,6)
#define  VTSS_M_VENICE_DEV1_TX_ALARM_Control_Register_TX_ALARM_Control_Register_Vendor_Specific     VTSS_ENCODE_BITMASK(5,6)
#define  VTSS_X_VENICE_DEV1_TX_ALARM_Control_Register_TX_ALARM_Control_Register_Vendor_Specific(x)  VTSS_EXTRACT_BITFIELD(x,5,6)

/** 
 * \brief
 * PMA/PMD Transmitter Local Fault Enable

 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: VTSS_VENICE_DEV1_TX_ALARM_Control_Register_TX_ALARM_Control_Register . PMA_PMD_Transmitter_Local_Fault_Enable
 */
#define  VTSS_F_VENICE_DEV1_TX_ALARM_Control_Register_TX_ALARM_Control_Register_PMA_PMD_Transmitter_Local_Fault_Enable  VTSS_BIT(4)

/** 
 * \brief
 * PCS Transmit Local Fault Enable

 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: VTSS_VENICE_DEV1_TX_ALARM_Control_Register_TX_ALARM_Control_Register . PCS_Transmit_Local_Fault_Enable
 */
#define  VTSS_F_VENICE_DEV1_TX_ALARM_Control_Register_TX_ALARM_Control_Register_PCS_Transmit_Local_Fault_Enable  VTSS_BIT(3)

/** 
 * \brief
 * Vendor Specific

 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: VTSS_VENICE_DEV1_TX_ALARM_Control_Register_TX_ALARM_Control_Register . Vendor_Specific_idx2
 */
#define  VTSS_F_VENICE_DEV1_TX_ALARM_Control_Register_TX_ALARM_Control_Register_Vendor_Specific_idx2(x)  VTSS_ENCODE_BITFIELD(x,1,2)
#define  VTSS_M_VENICE_DEV1_TX_ALARM_Control_Register_TX_ALARM_Control_Register_Vendor_Specific_idx2     VTSS_ENCODE_BITMASK(1,2)
#define  VTSS_X_VENICE_DEV1_TX_ALARM_Control_Register_TX_ALARM_Control_Register_Vendor_Specific_idx2(x)  VTSS_EXTRACT_BITFIELD(x,1,2)

/** 
 * \brief
 * PHY XS Transmit Local Fault Enable

 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: VTSS_VENICE_DEV1_TX_ALARM_Control_Register_TX_ALARM_Control_Register . PHY_XS_Transmit_Local_Fault_Enable
 */
#define  VTSS_F_VENICE_DEV1_TX_ALARM_Control_Register_TX_ALARM_Control_Register_PHY_XS_Transmit_Local_Fault_Enable  VTSS_BIT(0)

/**
 * Register Group: \a VENICE_DEV1:RX_ALARM_Status_Register
 *
 * Not documented
 */


/** 
 * \brief RX_ALARM Status Register
 *
 * \details
 * Register: \a VENICE_DEV1:RX_ALARM_Status_Register:RX_ALARM_Status_Register
 */
#define VTSS_VENICE_DEV1_RX_ALARM_Status_Register_RX_ALARM_Status_Register  VTSS_IOREG(0x01, 0, 0x9003)

/** 
 * \brief
 * For future use. 

 *
 * \details 
 * Field: VTSS_VENICE_DEV1_RX_ALARM_Status_Register_RX_ALARM_Status_Register . Vendor_Specific
 */
#define  VTSS_F_VENICE_DEV1_RX_ALARM_Status_Register_RX_ALARM_Status_Register_Vendor_Specific  VTSS_BIT(10)

/** 
 * \brief
 * WIS Local Fault
 * This is a sticky bit that latches the high state.  The latch-high bit is
 * cleared when the register is read.
 *
 * \details 
 * 0: No WIS Local Fault
 * 1: WIS Local Fault
 *
 * Field: VTSS_VENICE_DEV1_RX_ALARM_Status_Register_RX_ALARM_Status_Register . WIS_Local_Fault
 */
#define  VTSS_F_VENICE_DEV1_RX_ALARM_Status_Register_RX_ALARM_Status_Register_WIS_Local_Fault  VTSS_BIT(9)

/** 
 * \brief
 * For future use. 
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_RX_ALARM_Status_Register_RX_ALARM_Status_Register . Vendor_Specific_idx2
 */
#define  VTSS_F_VENICE_DEV1_RX_ALARM_Status_Register_RX_ALARM_Status_Register_Vendor_Specific_idx2(x)  VTSS_ENCODE_BITFIELD(x,5,4)
#define  VTSS_M_VENICE_DEV1_RX_ALARM_Status_Register_RX_ALARM_Status_Register_Vendor_Specific_idx2     VTSS_ENCODE_BITMASK(5,4)
#define  VTSS_X_VENICE_DEV1_RX_ALARM_Status_Register_RX_ALARM_Status_Register_Vendor_Specific_idx2(x)  VTSS_EXTRACT_BITFIELD(x,5,4)

/** 
 * \brief
 * PMA/PMD Receiver Local Fault
 * This is a sticky bit that latches the high state.  The latch-high bit is
 * cleared when the register is read.
 *
 * \details 
 * 0: No PMA/PMD Receiver Local Fault
 * 1: PMA/PMD Receiver Local Fault
 *
 * Field: VTSS_VENICE_DEV1_RX_ALARM_Status_Register_RX_ALARM_Status_Register . PMA_PMD_Receiver_Local_Fault
 */
#define  VTSS_F_VENICE_DEV1_RX_ALARM_Status_Register_RX_ALARM_Status_Register_PMA_PMD_Receiver_Local_Fault  VTSS_BIT(4)

/** 
 * \brief
 * PCS Receive Local Fault
 * This is a sticky bit that latches the high state.  The latch-high bit is
 * cleared when the register is read.
 *
 * \details 
 * 0: No PCS Receive Local Fault
 * 1: PCS Receive Local Fault
 *
 * Field: VTSS_VENICE_DEV1_RX_ALARM_Status_Register_RX_ALARM_Status_Register . PCS_Receive_Local_Fault
 */
#define  VTSS_F_VENICE_DEV1_RX_ALARM_Status_Register_RX_ALARM_Status_Register_PCS_Receive_Local_Fault  VTSS_BIT(3)

/** 
 * \brief
 * For future use. 
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_RX_ALARM_Status_Register_RX_ALARM_Status_Register . Vendor_Specific_idx3
 */
#define  VTSS_F_VENICE_DEV1_RX_ALARM_Status_Register_RX_ALARM_Status_Register_Vendor_Specific_idx3(x)  VTSS_ENCODE_BITFIELD(x,1,2)
#define  VTSS_M_VENICE_DEV1_RX_ALARM_Status_Register_RX_ALARM_Status_Register_Vendor_Specific_idx3     VTSS_ENCODE_BITMASK(1,2)
#define  VTSS_X_VENICE_DEV1_RX_ALARM_Status_Register_RX_ALARM_Status_Register_Vendor_Specific_idx3(x)  VTSS_EXTRACT_BITFIELD(x,1,2)

/** 
 * \brief
 * PHY XS Receive Local Fault
 * This is a sticky bit that latches the high state.  The latch-high bit is
 * cleared when the register is read.
 *
 * \details 
 * 0: No PHY XS Receive Local Fault
 * 1: PHY XS Receive Local Fault
 *
 * Field: VTSS_VENICE_DEV1_RX_ALARM_Status_Register_RX_ALARM_Status_Register . PHY_XS_Receive_Local_Fault
 */
#define  VTSS_F_VENICE_DEV1_RX_ALARM_Status_Register_RX_ALARM_Status_Register_PHY_XS_Receive_Local_Fault  VTSS_BIT(0)

/**
 * Register Group: \a VENICE_DEV1:TX_ALARM_Status_Register
 *
 * Not documented
 */


/** 
 * \brief TX_ALARM Status Register
 *
 * \details
 * Register: \a VENICE_DEV1:TX_ALARM_Status_Register:TX_ALARM_Status_Register
 */
#define VTSS_VENICE_DEV1_TX_ALARM_Status_Register_TX_ALARM_Status_Register  VTSS_IOREG(0x01, 0, 0x9004)

/** 
 * \brief
 * For future use. 

 *
 * \details 
 * Field: VTSS_VENICE_DEV1_TX_ALARM_Status_Register_TX_ALARM_Status_Register . Vendor_Specific
 */
#define  VTSS_F_VENICE_DEV1_TX_ALARM_Status_Register_TX_ALARM_Status_Register_Vendor_Specific(x)  VTSS_ENCODE_BITFIELD(x,5,6)
#define  VTSS_M_VENICE_DEV1_TX_ALARM_Status_Register_TX_ALARM_Status_Register_Vendor_Specific     VTSS_ENCODE_BITMASK(5,6)
#define  VTSS_X_VENICE_DEV1_TX_ALARM_Status_Register_TX_ALARM_Status_Register_Vendor_Specific(x)  VTSS_EXTRACT_BITFIELD(x,5,6)

/** 
 * \brief
 * PMA/PMD Transmitter Local Fault Enable
 * This is a sticky bit that latches the high state.  The latch-high bit is
 * cleared when the register is read.
 *
 * \details 
 * 0: No PMA/PMD Transmitter Local Fault
 * 1: PMA/PMD Transmitter Local Fault
 *
 * Field: VTSS_VENICE_DEV1_TX_ALARM_Status_Register_TX_ALARM_Status_Register . PMA_PMD_Transmitter_Local_Fault
 */
#define  VTSS_F_VENICE_DEV1_TX_ALARM_Status_Register_TX_ALARM_Status_Register_PMA_PMD_Transmitter_Local_Fault  VTSS_BIT(4)

/** 
 * \brief
 * PCS Transmit Local Fault Enable
 * This is a sticky bit that latches the high state.  The latch-high bit is
 * cleared when the register is read.
 *
 * \details 
 * 0: No PCS Transmit Local Fault
 * 1: PCS Transmit Local Fault
 *
 * Field: VTSS_VENICE_DEV1_TX_ALARM_Status_Register_TX_ALARM_Status_Register . PCS_Transmit_Local_Fault
 */
#define  VTSS_F_VENICE_DEV1_TX_ALARM_Status_Register_TX_ALARM_Status_Register_PCS_Transmit_Local_Fault  VTSS_BIT(3)

/** 
 * \brief
 * For future use. 

 *
 * \details 
 * Field: VTSS_VENICE_DEV1_TX_ALARM_Status_Register_TX_ALARM_Status_Register . Vendor_Specific_idx2
 */
#define  VTSS_F_VENICE_DEV1_TX_ALARM_Status_Register_TX_ALARM_Status_Register_Vendor_Specific_idx2(x)  VTSS_ENCODE_BITFIELD(x,1,2)
#define  VTSS_M_VENICE_DEV1_TX_ALARM_Status_Register_TX_ALARM_Status_Register_Vendor_Specific_idx2     VTSS_ENCODE_BITMASK(1,2)
#define  VTSS_X_VENICE_DEV1_TX_ALARM_Status_Register_TX_ALARM_Status_Register_Vendor_Specific_idx2(x)  VTSS_EXTRACT_BITFIELD(x,1,2)

/** 
 * \brief
 * PHY XS Transmit Local Fault Enable
 * This is a sticky bit that latches the high state.  The latch-high bit is
 * cleared when the register is read.
 *
 * \details 
 * 0: No PHY XS Transmit Local Fault
 * 1: PHY XS Transmit Local Fault
 *
 * Field: VTSS_VENICE_DEV1_TX_ALARM_Status_Register_TX_ALARM_Status_Register . PHY_XS_Transmit_Local_Fault
 */
#define  VTSS_F_VENICE_DEV1_TX_ALARM_Status_Register_TX_ALARM_Status_Register_PHY_XS_Transmit_Local_Fault  VTSS_BIT(0)

/**
 * Register Group: \a VENICE_DEV1:CLOCK_OUTPUT_CONTROL
 *
 * Control Signal Clock Output
 */


/** 
 * \brief RXCKOUT configuration register
 *
 * \details
 * Configuration register for RXCKOUT
 *
 * Register: \a VENICE_DEV1:CLOCK_OUTPUT_CONTROL:RXCK_CFG
 */
#define VTSS_VENICE_DEV1_CLOCK_OUTPUT_CONTROL_RXCK_CFG  VTSS_IOREG(0x01, 0, 0xa000)

/** 
 * \brief
 * Mask RXPLLF_LOCK from affecting the RXCK64 auto-squelch at the RXCKOUT
 * and TXCKOUT pins
 *
 * \details 
 * 0: RXPLLF_LOCK does not affect RXCK64 auto-squelch
 * 1: RXPLLF_LOCK affects RXCK64 auto-squelch
 *
 * Field: VTSS_VENICE_DEV1_CLOCK_OUTPUT_CONTROL_RXCK_CFG . RXCK_RXPLLF_LOCK_MASK
 */
#define  VTSS_F_VENICE_DEV1_CLOCK_OUTPUT_CONTROL_RXCK_CFG_RXCK_RXPLLF_LOCK_MASK  VTSS_BIT(7)

/** 
 * \brief
 * Mask PCS_FAULT from affecting the RXCK64 auto-squelch at the RXCKOUT and
 * TXCKOUT pins
 *
 * \details 
 * 0: PCS_FAULT does not affect RXCK64 auto-squelch
 * 1: PCS_FAULT affects RXCK64 auto-squelch
 *
 * Field: VTSS_VENICE_DEV1_CLOCK_OUTPUT_CONTROL_RXCK_CFG . RXCK_PCS_FAULT_MASK
 */
#define  VTSS_F_VENICE_DEV1_CLOCK_OUTPUT_CONTROL_RXCK_CFG_RXCK_PCS_FAULT_MASK  VTSS_BIT(6)

/** 
 * \brief
 * Mask LOPC from affecting the RXCK64 auto-squelch at the RXCKOUT and
 * TXCKOUT pins
 *
 * \details 
 * 0: LOPC does not affect RXCK64 auto-squelch
 * 1: LOPC affects RXCK64 auto-squelch
 *
 * Field: VTSS_VENICE_DEV1_CLOCK_OUTPUT_CONTROL_RXCK_CFG . RXCK_LOPC_MASK
 */
#define  VTSS_F_VENICE_DEV1_CLOCK_OUTPUT_CONTROL_RXCK_CFG_RXCK_LOPC_MASK  VTSS_BIT(5)

/** 
 * \brief
 * Mask IB_SIGNAL_DETECT from affecting the RXCK64 auto-squelch at the
 * RXCKOUT and TXCKOUT pins
 *
 * \details 
 * 0: IB_SIGNAL_DETECT does not affect RXCK64 auto-squelch
 * 1: IB_SIGNAL_DETECT affects RXCK64 auto-squelch
 *
 * Field: VTSS_VENICE_DEV1_CLOCK_OUTPUT_CONTROL_RXCK_CFG . RXCK_IB_SIGNAL_DETECT_MASK
 */
#define  VTSS_F_VENICE_DEV1_CLOCK_OUTPUT_CONTROL_RXCK_CFG_RXCK_IB_SIGNAL_DETECT_MASK  VTSS_BIT(4)

/** 
 * \brief
 * Enable the RXCKOUT pins
 * RXCKOUT is also affected by TXCKOUT_ENABLE bit and
 * OB_TST_OUT_CFG.OB_CTRL[10]
 * To enable the RXCKOUT pins either RXCKOUT_ENABLE or TXCKOUT_ENABLE must
 * be set to 1 and OB_CTRL[10] must be set to 1
 *
 * \details 
 * 0 = RXCKOUT disable
 * 1 = RXCKOUT enable 
 *
 * Field: VTSS_VENICE_DEV1_CLOCK_OUTPUT_CONTROL_RXCK_CFG . RXCKOUT_ENABLE
 */
#define  VTSS_F_VENICE_DEV1_CLOCK_OUTPUT_CONTROL_RXCK_CFG_RXCKOUT_ENABLE  VTSS_BIT(3)

/** 
 * \brief
 * Configure the RXCKOUT pins
 *
 * \details 
 * 000 = line side rx recovered clock (LAN: 322.26 Mhz, WAN: 311.04 Mhz,
 * 1G: 125 MHz)
 * 001 = line side rx recovered clock div by 2	(LAN: 161.13 MHz, WAN:
 * 155.52 MHz, 1G: 62.5 Mhz)
 * 010 = line side tx clock (LAN: 322.26 Mhz, WAN: 311.04 Mhz, 1G: 125 MHz)
 * 011 = line side tx clock div by 2 (LAN: 161.13 MHz, WAN: 155.52 MHz, 1G:
 * 62.5 Mhz)
 * 100 = line side pll test clock
 * 101 = factory test
 * 110 = host side rx recovered clock (LAN/WAN: 312.5 MHz, 78.125 MHz or
 * 62.5 MHz, 1G: 125 MHz, 31.25 MHz or 25 MHz)
 * 111 = host side pll test clock

 *
 * Field: VTSS_VENICE_DEV1_CLOCK_OUTPUT_CONTROL_RXCK_CFG . RXCKOUT_SEL
 */
#define  VTSS_F_VENICE_DEV1_CLOCK_OUTPUT_CONTROL_RXCK_CFG_RXCKOUT_SEL(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_VENICE_DEV1_CLOCK_OUTPUT_CONTROL_RXCK_CFG_RXCKOUT_SEL     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_VENICE_DEV1_CLOCK_OUTPUT_CONTROL_RXCK_CFG_RXCKOUT_SEL(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief TXCKOUT configuration register
 *
 * \details
 * Configuration register for TXCKOUT
 *
 * Register: \a VENICE_DEV1:CLOCK_OUTPUT_CONTROL:TXCK_CFG
 */
#define VTSS_VENICE_DEV1_CLOCK_OUTPUT_CONTROL_TXCK_CFG  VTSS_IOREG(0x01, 0, 0xa001)

/** 
 * \brief
 * Enable the TXCKOUT pins
 * TXCKOUT is also affected by RXCKOUT_ENABLE bit and
 * OB_TST_OUT_CFG.OB_CTRL[10]
 * To enable the TXCKOUT pins either TXCKOUT_ENABLE or RXCKOUT_ENABLE must
 * be set to 1 and OB_CTRL[10] must be set to 1
 *
 * \details 
 * 0 = TXCKOUT disable
 * 1 = TXCKOUT enable
 *
 * Field: VTSS_VENICE_DEV1_CLOCK_OUTPUT_CONTROL_TXCK_CFG . TXCKOUT_ENABLE
 */
#define  VTSS_F_VENICE_DEV1_CLOCK_OUTPUT_CONTROL_TXCK_CFG_TXCKOUT_ENABLE  VTSS_BIT(3)

/** 
 * \brief
 * Configure the TXCKOUT pins

 *
 * \details 
 * 000 = line side tx clock (LAN: 322.26 Mhz, WAN: 311.04 Mhz, 1G: 125 MHz)
 * 001 = line side tx clock div by 2  (LAN: 161.13 MHz, WAN: 155.52 MHz,
 * 1G: 62.5 Mhz)
 * 010 = line side rx recovered clock (LAN: 322.26 Mhz, WAN: 311.04 Mhz,
 * 1G: 125 MHz)
 * 011 = line side rx recovered clock div by 2 (LAN: 161.13 MHz, WAN:
 * 155.52 MHz, 1G: 62.5 Mhz)
 * 100 = line side pll test clock
 * 101 = factory test
 * 110 = host side rx recovered clock (LAN/WAN: 312.5 MHz, 78.125 MHz or
 * 62.5 MHz, 1G: 125 MHz, 31.25 MHz or 25 MHz)
 * 111 = host side pll test clock

 *
 * Field: VTSS_VENICE_DEV1_CLOCK_OUTPUT_CONTROL_TXCK_CFG . TXCKOUT_SEL
 */
#define  VTSS_F_VENICE_DEV1_CLOCK_OUTPUT_CONTROL_TXCK_CFG_TXCKOUT_SEL(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_VENICE_DEV1_CLOCK_OUTPUT_CONTROL_TXCK_CFG_TXCKOUT_SEL     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_VENICE_DEV1_CLOCK_OUTPUT_CONTROL_TXCK_CFG_TXCKOUT_SEL(x)  VTSS_EXTRACT_BITFIELD(x,0,3)

/**
 * Register Group: \a VENICE_DEV1:DATAPATH_CONTROL
 *
 * Control Signal for Data Path
 */


/** 
 * \brief 10G or 1G mode in datapath
 *
 * \details
 * Determine the datapath mode

 *
 * Register: \a VENICE_DEV1:DATAPATH_CONTROL:DATAPATH_MODE
 */
#define VTSS_VENICE_DEV1_DATAPATH_CONTROL_DATAPATH_MODE  VTSS_IOREG(0x01, 0, 0xa002)

/** 
 * \brief
 * Configure Lineside Serdes to 10G WAN mode with the ROM Engine. This
 * register is self-clearing.
 *
 * \details 
 * 0 = Do Nothing
 * 1 = Configure Lineside Serdes to 10G WAN mode with the ROM Engine

 *
 * Field: VTSS_VENICE_DEV1_DATAPATH_CONTROL_DATAPATH_MODE . USR_10G_WAN
 */
#define  VTSS_F_VENICE_DEV1_DATAPATH_CONTROL_DATAPATH_MODE_USR_10G_WAN  VTSS_BIT(6)

/** 
 * \brief
 * Configure Lineside Serdes to 10G LAN mode with the ROM Engine.This
 * register is self-clearing.
 *
 * \details 
 * 0 = Do Nothing
 * 1 = Configure Lineside Serdes to 10G LAN mode with the ROM Engine

 *
 * Field: VTSS_VENICE_DEV1_DATAPATH_CONTROL_DATAPATH_MODE . USR_10G_LAN
 */
#define  VTSS_F_VENICE_DEV1_DATAPATH_CONTROL_DATAPATH_MODE_USR_10G_LAN  VTSS_BIT(5)

/** 
 * \brief
 * Configure Lineside Serdes to 1G mode with the ROM Engine.This register
 * is self-clearing.
 *
 * \details 
 * 0 = Do Nothing
 * 1 = Configure Lineside Serdes to 1G mode with the ROM Engine

 *
 * Field: VTSS_VENICE_DEV1_DATAPATH_CONTROL_DATAPATH_MODE . USR_1G
 */
#define  VTSS_F_VENICE_DEV1_DATAPATH_CONTROL_DATAPATH_MODE_USR_1G  VTSS_BIT(4)

/** 
 * \brief
 * In 1G mode of operation, select the lane used for data on the client
 * side interface
 *
 * \details 
 * 0 = lane 0 is used for 1G data
 * 1 = lane 3 is used for 1G data
 *
 * Field: VTSS_VENICE_DEV1_DATAPATH_CONTROL_DATAPATH_MODE . SEL_1G_LANE_3
 */
#define  VTSS_F_VENICE_DEV1_DATAPATH_CONTROL_DATAPATH_MODE_SEL_1G_LANE_3  VTSS_BIT(1)

/** 
 * \brief
 * Configure datapath and host side serdes into 1G mode

 *
 * \details 
 * 0 = 10G LAN or WAN
 * 1 = 1G 
 *
 * Field: VTSS_VENICE_DEV1_DATAPATH_CONTROL_DATAPATH_MODE . ETH_1G_ENA
 */
#define  VTSS_F_VENICE_DEV1_DATAPATH_CONTROL_DATAPATH_MODE_ETH_1G_ENA  VTSS_BIT(0)

/**
 * Register Group: \a VENICE_DEV1:LOOPBACK_CONTROL
 *
 * Control Signal for Loopback
 */


/** 
 * \brief Datapath Loopback Control
 *
 * \details
 * Datapath Loopback Control
 *
 * Register: \a VENICE_DEV1:LOOPBACK_CONTROL:PMA_LOOPBACK_CONTROL
 */
#define VTSS_VENICE_DEV1_LOOPBACK_CONTROL_PMA_LOOPBACK_CONTROL  VTSS_IOREG(0x01, 0, 0xa003)

/** 
 * \brief
 * Loopback L3 Enable
 *
 * \details 
 * 0 = Normal Operation
 * 1 = Enable L3 Loopback 
 *
 * Field: VTSS_VENICE_DEV1_LOOPBACK_CONTROL_PMA_LOOPBACK_CONTROL . L3_CONTROL
 */
#define  VTSS_F_VENICE_DEV1_LOOPBACK_CONTROL_PMA_LOOPBACK_CONTROL_L3_CONTROL  VTSS_BIT(0)

/**
 * Register Group: \a VENICE_DEV1:MAC_ENABLE
 *
 * Enable MAC in the datapath
 */


/** 
 * \brief Enable MAC in the datapath
 *
 * \details
 * Enable MAC in the datapath
 *
 * Register: \a VENICE_DEV1:MAC_ENABLE:MAC_ENA
 */
#define VTSS_VENICE_DEV1_MAC_ENABLE_MAC_ENA  VTSS_IOREG(0x01, 0, 0xa006)

/** 
 * \brief
 * Clock enable for the MACsec logic.  Deasserting this bit when MACsec is
 * disabled and MACs are enabled will save power.  Note:  the CLK_EN
 * register bits within the MACsec register space must be asserted along
 * with this bit when the MACsec logic is to be used.  This bit usage
 * applies to the VSC8490 and VSC8491 products only.
 *
 * \details 
 * 0 = MACsec clock is squelched
 * 1 = MACsec clock is enabled
 *
 * Field: VTSS_VENICE_DEV1_MAC_ENABLE_MAC_ENA . MACSEC_CLK_ENA
 */
#define  VTSS_F_VENICE_DEV1_MAC_ENABLE_MAC_ENA_MACSEC_CLK_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable MAC in the datapath
 *
 * \details 
 * 0 = MAC is not in the datapath
 * 1 = MAC is in the datapath
 *
 * Field: VTSS_VENICE_DEV1_MAC_ENABLE_MAC_ENA . MAC_ENA
 */
#define  VTSS_F_VENICE_DEV1_MAC_ENABLE_MAC_ENA_MAC_ENA  VTSS_BIT(0)

/**
 * Register Group: \a VENICE_DEV1:RCOMP
 *
 * Write RCOMP 4-bit resistor calibration value into SD10G
 */


/** 
 * \brief Write RCOMP 4-bit resistor calibration value into SD10G
 *
 * \details
 * Write RCOMP 4-bit resistor calibration value into SD10G
 *
 * Register: \a VENICE_DEV1:RCOMP:RCOMP
 */
#define VTSS_VENICE_DEV1_RCOMP_RCOMP         VTSS_IOREG(0x01, 0, 0xa007)

/** 
 * \brief
 * Write RCOMP 4-bit resistor calibration value into SD10G  (SC)

 *
 * \details 
 * 0 = Do nothing
 * 1 = Start RCOMP event
 *
 * Field: VTSS_VENICE_DEV1_RCOMP_RCOMP . RCOMP_WRITE
 */
#define  VTSS_F_VENICE_DEV1_RCOMP_RCOMP_RCOMP_WRITE  VTSS_BIT(0)

/**
 * Register Group: \a VENICE_DEV1:OB_TST_OUT_CFG
 *
 * Configuration Registers for Clock Output Buffer
 */


/** 
 * \brief Clock Output Buffer Bias Control
 *
 * \details
 * Clock Output Buffer Bias Control
 *
 * Register: \a VENICE_DEV1:OB_TST_OUT_CFG:OB_BIAS_CTRL
 */
#define VTSS_VENICE_DEV1_OB_TST_OUT_CFG_OB_BIAS_CTRL  VTSS_IOREG(0x01, 0, 0xa00c)

/** 
 * \brief
 * Clock Output Buffer Bias Control
 * 
 * [2:0] = Sets the class AB bias current in the common mode control
 * circuit. 0.5 mA is expected to give sufficient performance and is
 * default. Other settings are for debug. Current range is 0 to 1.75 mA in
 * 0.25mA steps. Default is set to 0 (disable)
 * [3] = enable internal CML to CMOS converter for input to test output
 * path
 * [5:4] = reserved
 * [7:6] = slope/slew rate control, 0: 45ps, 1: 85ps, 2: 105ps, 3: 115ps
 * rise/fall time (all values are typical)

 *
 * \details 
 * Field: VTSS_VENICE_DEV1_OB_TST_OUT_CFG_OB_BIAS_CTRL . OB_BIAS_CTRL
 */
#define  VTSS_F_VENICE_DEV1_OB_TST_OUT_CFG_OB_BIAS_CTRL_OB_BIAS_CTRL(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VENICE_DEV1_OB_TST_OUT_CFG_OB_BIAS_CTRL_OB_BIAS_CTRL     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VENICE_DEV1_OB_TST_OUT_CFG_OB_BIAS_CTRL_OB_BIAS_CTRL(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Clock Output Buffer Control Registers
 *
 * \details
 * Clock Output Buffer Control Registers
 *
 * Register: \a VENICE_DEV1:OB_TST_OUT_CFG:OB_CTRL
 */
#define VTSS_VENICE_DEV1_OB_TST_OUT_CFG_OB_CTRL  VTSS_IOREG(0x01, 0, 0xa00d)

/** 
 * \brief
 * Clock Output Buffer Control Registers
 * 
 * [3:0] = value for resistor calibration (RCOMP), 15: lowest value 0:
 * highest value
 * [7:4] = Adjustment for Common Mode Voltage, 0: off --> results in a
 * value around 500mV, 1: 440mV, 2: 480mV, 3: 460mV, 4: 530mV, 6: 500mV, 8:
 * 570mV, 12: 550mV
 * [8] =  disable VCM control, 1: disable, 0: enable
 * [9] = enable VREG measure, 1: enable, 0: disable
 * [10] = enable output buffer, 1: enable, 0: disable (powerdown)
 * [11] = reserved
 * [15:12] = select output level, 400mVppd (0) to 1100mVppd (15) in 50mVppd
 * steps

 *
 * \details 
 * Field: VTSS_VENICE_DEV1_OB_TST_OUT_CFG_OB_CTRL . OB_CTRL
 */
#define  VTSS_F_VENICE_DEV1_OB_TST_OUT_CFG_OB_CTRL_OB_CTRL(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_OB_TST_OUT_CFG_OB_CTRL_OB_CTRL     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_OB_TST_OUT_CFG_OB_CTRL_OB_CTRL(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VENICE_DEV1:Vendor_Specific_PMA_Control_2
 *
 * Not documented
 */


/** 
 * \brief Vendor Specific PMA Control 2
 *
 * \details
 * Register: \a VENICE_DEV1:Vendor_Specific_PMA_Control_2:Vendor_Specific_PMA_Control_2
 */
#define VTSS_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2  VTSS_IOREG(0x01, 0, 0xa100)

/** 
 * \brief
 * WIS_INTB active edge
 *
 * \details 
 * 0: WIS_INTB is active low
 * 1: WIS_INTB is active high
 *
 * Field: VTSS_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2 . wis_intb_activeh
 */
#define  VTSS_F_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2_wis_intb_activeh  VTSS_BIT(15)

/** 
 * \brief
 * WIS_INTA active edge
 *
 * \details 
 * 0: WIS_INTA is active low
 * 1: WIS_INTA is active high
 *
 * Field: VTSS_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2 . wis_inta_activeh
 */
#define  VTSS_F_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2_wis_inta_activeh  VTSS_BIT(14)

/** 
 * \brief
 * LOS circuitry is driven by a signal detection status signal in the
 * line-side input buffer.  The signal detection alarm driving the LOS
 * curcuitry can be squelched with this register bit.
 *
 * \details 
 * LOS detection is
 * 0: Allowed
 * 1: Suppressed
 *
 * Field: VTSS_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2 . Suppress_LOS_detection
 */
#define  VTSS_F_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2_Suppress_LOS_detection  VTSS_BIT(10)

/** 
 * \brief
 * LOL circuitry is driven by a status signal in the line-side CRU.  The
 * status signal driving the LOL curcuitry can be squelched with this
 * register bit.
 *
 * \details 
 * LOL detection is
 * 0: Allowed
 * 1: Suppressed
 *
 * Field: VTSS_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2 . Suppress_LOL_detection
 */
#define  VTSS_F_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2_Suppress_LOL_detection  VTSS_BIT(9)

/** 
 * \brief
 * TX data activity LED blink time
 *
 * \details 
 * 0: 50ms interval
 * 1: 100ms interval
 *
 * Field: VTSS_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2 . TX_LED_BLINK_TIME
 */
#define  VTSS_F_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2_TX_LED_BLINK_TIME  VTSS_BIT(8)

/** 
 * \brief
 * RX data activity LED blink time
 *
 * \details 
 * 0: 50ms interval
 * 1: 100ms interval
 *
 * Field: VTSS_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2 . RX_LED_BLINK_TIME
 */
#define  VTSS_F_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2_RX_LED_BLINK_TIME  VTSS_BIT(7)

/** 
 * \brief
 * Tx LED mode control
 *
 * \details 
 * 00:	Display Tx link status
 * 01:	Reserved
 * 10:	Display combination of Tx link and Tx data activity status
 * 11:	Reserved
 *
 * Field: VTSS_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2 . TX_LED_MODE
 */
#define  VTSS_F_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2_TX_LED_MODE(x)  VTSS_ENCODE_BITFIELD(x,5,2)
#define  VTSS_M_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2_TX_LED_MODE     VTSS_ENCODE_BITMASK(5,2)
#define  VTSS_X_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2_TX_LED_MODE(x)  VTSS_EXTRACT_BITFIELD(x,5,2)

/** 
 * \brief
 * Rx LED mode control
 *
 * \details 
 * 00:	Display Rx link status
 * 01:	Reserved
 * 10:	Display combination of Rx link and Rx data activity status
 * 11:	Reserved
 *
 * Field: VTSS_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2 . RX_LED_MODE
 */
#define  VTSS_F_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2_RX_LED_MODE(x)  VTSS_ENCODE_BITFIELD(x,3,2)
#define  VTSS_M_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2_RX_LED_MODE     VTSS_ENCODE_BITMASK(3,2)
#define  VTSS_X_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2_RX_LED_MODE(x)  VTSS_EXTRACT_BITFIELD(x,3,2)

/** 
 * \brief
 * System loopback data override
 *
 * \details 
 * 0: Data sent out XFI output matches default.
 * 1: Use 'PMA system loopback data select' to select XFI ouput data.
 *
 * Field: VTSS_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2 . Override_system_loopback_data
 */
#define  VTSS_F_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2_Override_system_loopback_data  VTSS_BIT(2)

/** 
 * \brief
 * When Override system loopback data (bit 2) is set and the data channel
 * is in 10G mode, the data transmitted from TX PMA is determined by these
 * register bits.  

 *
 * \details 
 * 00:	repeating 0x00FF pattern
 * 01: continuously send 0's
 * 10: continuously send 1's
 * 11: data from TX WIS block
 *
 * Field: VTSS_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2 . PMA_system_loopback_data_select
 */
#define  VTSS_F_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2_PMA_system_loopback_data_select(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2_PMA_system_loopback_data_select     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_VENICE_DEV1_Vendor_Specific_PMA_Control_2_Vendor_Specific_PMA_Control_2_PMA_system_loopback_data_select(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

/**
 * Register Group: \a VENICE_DEV1:Vendor_Specific_PMA_Status_2
 *
 * Not documented
 */


/** 
 * \brief Vendor Specific PMA Status 2
 *
 * \details
 * Register: \a VENICE_DEV1:Vendor_Specific_PMA_Status_2:Vendor_Specific_PMA_Status_2
 */
#define VTSS_VENICE_DEV1_Vendor_Specific_PMA_Status_2_Vendor_Specific_PMA_Status_2  VTSS_IOREG(0x01, 0, 0xa101)

/** 
 * \brief
 * Indicates if the device is in WAN mode
 * WAN mode is enable when
 * 2x0007.0 = 1 OR 
 * 3x0007.1:0 = 10 OR 
 * 1x0007.2:0 = 001 OR 
 * 1x0007.2:0 = 010 OR 
 * 1x0007.2:0 = 011

 *
 * \details 
 * 0: Not in Wan Mode
 * 1: WAN Mode
 *
 * Field: VTSS_VENICE_DEV1_Vendor_Specific_PMA_Status_2_Vendor_Specific_PMA_Status_2 . WAN_ENABLED_status
 */
#define  VTSS_F_VENICE_DEV1_Vendor_Specific_PMA_Status_2_Vendor_Specific_PMA_Status_2_WAN_ENABLED_status  VTSS_BIT(3)

/** 
 * \brief
 * WIS_INTA pin status

 *
 * \details 
 * 0: WIS_INTA pin is low
 * 1: WIS_INTA pin is high
 *
 * Field: VTSS_VENICE_DEV1_Vendor_Specific_PMA_Status_2_Vendor_Specific_PMA_Status_2 . WIS_INTA_pin_status
 */
#define  VTSS_F_VENICE_DEV1_Vendor_Specific_PMA_Status_2_Vendor_Specific_PMA_Status_2_WIS_INTA_pin_status  VTSS_BIT(2)

/** 
 * \brief
 * WIS_INTB pin status
 *
 * \details 
 * 0: WIS_INTB pin is low
 * 1: WIS_INTB pin is high
 *
 * Field: VTSS_VENICE_DEV1_Vendor_Specific_PMA_Status_2_Vendor_Specific_PMA_Status_2 . WIS_INTB_pin_status
 */
#define  VTSS_F_VENICE_DEV1_Vendor_Specific_PMA_Status_2_Vendor_Specific_PMA_Status_2_WIS_INTB_pin_status  VTSS_BIT(1)

/** 
 * \brief
 * PMTICK pin status
 *
 * \details 
 * 0: PMTICK pin is low
 * 1: PMTICK pin is high
 *
 * Field: VTSS_VENICE_DEV1_Vendor_Specific_PMA_Status_2_Vendor_Specific_PMA_Status_2 . PMTICK_pin_status
 */
#define  VTSS_F_VENICE_DEV1_Vendor_Specific_PMA_Status_2_Vendor_Specific_PMA_Status_2_PMTICK_pin_status  VTSS_BIT(0)

/**
 * Register Group: \a VENICE_DEV1:Vendor_Specific_LOPC_Status
 *
 * Not documented
 */


/** 
 * \brief Vendor Specific LOPC Status
 *
 * \details
 * Register: \a VENICE_DEV1:Vendor_Specific_LOPC_Status:Vendor_Specific_LOPC_Status
 */
#define VTSS_VENICE_DEV1_Vendor_Specific_LOPC_Status_Vendor_Specific_LOPC_Status  VTSS_IOREG(0x01, 0, 0xa200)

/** 
 * \brief
 * Present state of the LOPC pin which taking into account the lopc invert
 * regbit 1xA201.2
 *
 * \details 
 * 0: LOPC pin is low when lopc_invert (1xA201.2) =0, LOPC is high when
 * lopc_invert (1xA201.2) =1
 * 1: LOPC pin is high when lopc_invert (1xA201.2) =0, LOPC is low when
 * lopc_invert (1xA201.2) =1
 *
 * Field: VTSS_VENICE_DEV1_Vendor_Specific_LOPC_Status_Vendor_Specific_LOPC_Status . Present_state_of_the_LOPC_pin_with_reginv
 */
#define  VTSS_F_VENICE_DEV1_Vendor_Specific_LOPC_Status_Vendor_Specific_LOPC_Status_Present_state_of_the_LOPC_pin_with_reginv  VTSS_BIT(2)

/** 
 * \brief
 * Present state of the LOPC pin which does not take into account the lopc
 * invert regbit 1xA201.2
 *
 * \details 
 * 0: LOPC pin is low
 * 1: LOPC pin is high
 *
 * Field: VTSS_VENICE_DEV1_Vendor_Specific_LOPC_Status_Vendor_Specific_LOPC_Status . Present_state_of_the_LOPC_pin_without_reginv
 */
#define  VTSS_F_VENICE_DEV1_Vendor_Specific_LOPC_Status_Vendor_Specific_LOPC_Status_Present_state_of_the_LOPC_pin_without_reginv  VTSS_BIT(1)

/** 
 * \brief
 * LOPC interupt pending status.  The latch-high bit is cleared when the
 * register is read.
 *
 * \details 
 * 0: An interrupt event has not occurred since the last time this bit was
 * read
 * 1: An interrupt event determined by lopc_intr_mode has occurred
 *
 * Field: VTSS_VENICE_DEV1_Vendor_Specific_LOPC_Status_Vendor_Specific_LOPC_Status . Interrupt_pending_bit
 */
#define  VTSS_F_VENICE_DEV1_Vendor_Specific_LOPC_Status_Vendor_Specific_LOPC_Status_Interrupt_pending_bit  VTSS_BIT(0)

/**
 * Register Group: \a VENICE_DEV1:Vendor_Specific_LOPC_Control
 *
 * Not documented
 */


/** 
 * \brief Vendor Specific LOPC Control
 *
 * \details
 * Register: \a VENICE_DEV1:Vendor_Specific_LOPC_Control:Vendor_Specific_LOPC_Control
 */
#define VTSS_VENICE_DEV1_Vendor_Specific_LOPC_Control_Vendor_Specific_LOPC_Control  VTSS_IOREG(0x01, 0, 0xa201)

/** 
 * \brief
 * LOPC pin polarity
 *
 * \details 
 * 0: The part is in a LOPC alarm state when the LOPC pin is logic low.
 * 1: The part is in a LOPC alarm state when the LOPC pin is logic high.
 *
 * Field: VTSS_VENICE_DEV1_Vendor_Specific_LOPC_Control_Vendor_Specific_LOPC_Control . LOPC_state_inversion_select
 */
#define  VTSS_F_VENICE_DEV1_Vendor_Specific_LOPC_Control_Vendor_Specific_LOPC_Control_LOPC_state_inversion_select  VTSS_BIT(2)

/** 
 * \brief
 * This bit group determines how the LOPC interrupt pending register in
 * 1xA200.0 is asserted
 *
 * \details 
 * 00: interrupt generation is disable
 * 01: lopc_intr_pend is set on a rising edge of the LOPC pin, regardless
 * of the lopc_invert (1xA201.2) setting
 * 10: lopc_intr_pend is set on a falling edge of the LOPC pin, regardless
 * of the lopc_invert (1xA201.2) setting
 * 11: lopc_intr_pend is set on both edges of the LOPC pin
 *
 * Field: VTSS_VENICE_DEV1_Vendor_Specific_LOPC_Control_Vendor_Specific_LOPC_Control . lopc_intr_pend_bit_select
 */
#define  VTSS_F_VENICE_DEV1_Vendor_Specific_LOPC_Control_Vendor_Specific_LOPC_Control_lopc_intr_pend_bit_select(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_VENICE_DEV1_Vendor_Specific_LOPC_Control_Vendor_Specific_LOPC_Control_lopc_intr_pend_bit_select     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_VENICE_DEV1_Vendor_Specific_LOPC_Control_Vendor_Specific_LOPC_Control_lopc_intr_pend_bit_select(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

/**
 * Register Group: \a VENICE_DEV1:BLOCK_LEVEL_RESET
 *
 * Block Level Reset
 */


/** 
 * \brief Block Level Soft Reset1
 *
 * \details
 * Register: \a VENICE_DEV1:BLOCK_LEVEL_RESET:BLOCK_LEVEL_RESET1
 */
#define VTSS_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET1  VTSS_IOREG(0x01, 0, 0xae00)

/** 
 * \brief
 * Reset the I2C(master) used to communicate with an optics module.
 *
 * \details 
 * 0: Normal operation
 * 1: Reset
 *
 * Field: VTSS_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET1 . I2CM_RESET
 */
#define  VTSS_F_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET1_I2CM_RESET  VTSS_BIT(15)

/** 
 * \brief
 * Reset WIS interrupt tree logic
 *
 * \details 
 * 0: Normal operation
 * 1: Reset
 *
 * Field: VTSS_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET1 . WIS_INTR_TREE_RESET
 */
#define  VTSS_F_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET1_WIS_INTR_TREE_RESET  VTSS_BIT(9)

/** 
 * \brief
 * Reset the ingress data path in the host 1G PCS block
 *
 * \details 
 * 0: Normal operation
 * 1: Reset
 *
 * Field: VTSS_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET1 . HOST_1G_PCS_INGR_RESET
 */
#define  VTSS_F_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET1_HOST_1G_PCS_INGR_RESET  VTSS_BIT(7)

/** 
 * \brief
 * Reset FIFO in the ingress data path.  The FIFO is used when the MACs are
 * disabled.
 *
 * \details 
 * 0: Normal operation
 * 1: Reset
 *
 * Field: VTSS_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET1 . FIFO_INGR_RESET
 */
#define  VTSS_F_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET1_FIFO_INGR_RESET  VTSS_BIT(6)

/** 
 * \brief
 * Reset the ingress data path in the host MAC and flow control buffer.
 *
 * \details 
 * 0: Normal operation
 * 1: Reset
 *
 * Field: VTSS_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET1 . HOST_MAC_INGR_RESET
 */
#define  VTSS_F_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET1_HOST_MAC_INGR_RESET  VTSS_BIT(5)

/** 
 * \brief
 * Reset the ingress data path in the line MAC, MACsec (applies to 8488-16)
 * and flow control buffer.
 *
 * \details 
 * 0: Normal operation
 * 1: Reset
 *
 * Field: VTSS_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET1 . LINE_MAC_INGR_RESET
 */
#define  VTSS_F_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET1_LINE_MAC_INGR_RESET  VTSS_BIT(4)

/** 
 * \brief
 * Reset the ingress data path in the 10G PCS and 1588 blocks when the part
 * is operating mode in 10G mode.
 *
 * \details 
 * 0: Normal operation
 * 1: Reset
 *
 * Field: VTSS_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET1 . LINE_10G_PCS_INGR_RESET
 */
#define  VTSS_F_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET1_LINE_10G_PCS_INGR_RESET  VTSS_BIT(3)

/** 
 * \brief
 * Reset the ingress data path in the 1G PCS and 1588 blocks when the part
 * is operating mode in 1G mode.
 *
 * \details 
 * 0: Normal operation
 * 1: Reset
 *
 * Field: VTSS_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET1 . LINE_1G_PCS_INGR_RESET
 */
#define  VTSS_F_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET1_LINE_1G_PCS_INGR_RESET  VTSS_BIT(2)

/** 
 * \brief
 * Reset the ingress data path in the WIS block.
 *
 * \details 
 * 0: Normal operation
 * 1: Reset
 *
 * Field: VTSS_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET1 . WIS_INGR_RESET
 */
#define  VTSS_F_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET1_WIS_INGR_RESET  VTSS_BIT(1)

/** 
 * \brief
 * Reset the ingress data path in the PMA and PMA_INT blocks.
 *
 * \details 
 * 0: Normal operation
 * 1: Reset
 *
 * Field: VTSS_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET1 . PMA_INGR_RESET
 */
#define  VTSS_F_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET1_PMA_INGR_RESET  VTSS_BIT(0)


/** 
 * \brief Block Level Soft Reset2
 *
 * \details
 * Register: \a VENICE_DEV1:BLOCK_LEVEL_RESET:BLOCK_LEVEL_RESET2
 */
#define VTSS_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET2  VTSS_IOREG(0x01, 0, 0xae01)

/** 
 * \brief
 * Reset both the egress and ingress data paths in the HSIO_MACRO_HOST
 * block (client-side serdes)
 *
 * \details 
 * 0: Normal operation
 * 1: Reset
 *
 * Field: VTSS_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET2 . CLIENT_SERDES_RESET
 */
#define  VTSS_F_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET2_CLIENT_SERDES_RESET  VTSS_BIT(10)

/** 
 * \brief
 * Reset the egress data path in the XGXS block
 *
 * \details 
 * 0: Normal operation
 * 1: Reset
 *
 * Field: VTSS_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET2 . XGXS_EGR_RESET
 */
#define  VTSS_F_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET2_XGXS_EGR_RESET  VTSS_BIT(8)

/** 
 * \brief
 * Reset the egress data path in the host 1G PCS block
 *
 * \details 
 * 0: Normal operation
 * 1: Reset
 *
 * Field: VTSS_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET2 . HOST_1G_PCS_EGR_RESET
 */
#define  VTSS_F_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET2_HOST_1G_PCS_EGR_RESET  VTSS_BIT(7)

/** 
 * \brief
 * Reset FIFO in the egress data path.	The FIFO is used when the MACs are
 * disabled.
 *
 * \details 
 * 0: Normal operation
 * 1: Reset
 *
 * Field: VTSS_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET2 . FIFO_EGR_RESET
 */
#define  VTSS_F_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET2_FIFO_EGR_RESET  VTSS_BIT(6)

/** 
 * \brief
 * Reset the egress data path in the host MAC and flow control buffer.
 *
 * \details 
 * 0: Normal operation
 * 1: Reset
 *
 * Field: VTSS_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET2 . HOST_MAC_EGR_RESET
 */
#define  VTSS_F_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET2_HOST_MAC_EGR_RESET  VTSS_BIT(5)

/** 
 * \brief
 * Reset the egress data path in the line MAC, MACsec (applies to 8488-16)
 * and flow control buffer.
 *
 * \details 
 * 0: Normal operation
 * 1: Reset
 *
 * Field: VTSS_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET2 . LINE_MAC_EGR_RESET
 */
#define  VTSS_F_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET2_LINE_MAC_EGR_RESET  VTSS_BIT(4)

/** 
 * \brief
 * Reset the egress data path in the 10G PCS and 1588 blocks when the part
 * is operating mode in 10G mode.
 *
 * \details 
 * 0: Normal operation
 * 1: Reset
 *
 * Field: VTSS_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET2 . LINE_10G_PCS_EGR_RESET
 */
#define  VTSS_F_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET2_LINE_10G_PCS_EGR_RESET  VTSS_BIT(3)

/** 
 * \brief
 * Reset the egress data path in the 1G PCS and 1588 blocks when the part
 * is operating mode in 1G mode.
 *
 * \details 
 * 0: Normal operation
 * 1: Reset
 *
 * Field: VTSS_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET2 . LINE_1G_PCS_EGR_RESET
 */
#define  VTSS_F_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET2_LINE_1G_PCS_EGR_RESET  VTSS_BIT(2)

/** 
 * \brief
 * Reset the egress data path in the WIS block.
 *
 * \details 
 * 0: Normal operation
 * 1: Reset
 *
 * Field: VTSS_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET2 . WIS_EGR_RESET
 */
#define  VTSS_F_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET2_WIS_EGR_RESET  VTSS_BIT(1)

/** 
 * \brief
 * Reset the egress data path in the PMA and PMA_INT blocks.
 *
 * \details 
 * 0: Normal operation
 * 1: Reset
 *
 * Field: VTSS_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET2 . PMA_EGR_RESET
 */
#define  VTSS_F_VENICE_DEV1_BLOCK_LEVEL_RESET_BLOCK_LEVEL_RESET2_PMA_EGR_RESET  VTSS_BIT(0)

/**
 * Register Group: \a VENICE_DEV1:SPARE_RW_REGISTERS
 *
 * Spare R/W Registers
 */


/** 
 * \brief Device1 Spare R/W 0
 *
 * \details
 * Register: \a VENICE_DEV1:SPARE_RW_REGISTERS:DEV1_SPARE_RW0
 */
#define VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW0  VTSS_IOREG(0x01, 0, 0xaef0)

/** 
 * \brief
 * Spare
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW0 . dev1_spare_rw0
 */
#define  VTSS_F_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW0_dev1_spare_rw0(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW0_dev1_spare_rw0     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW0_dev1_spare_rw0(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Device1 Spare R/W 1
 *
 * \details
 * Register: \a VENICE_DEV1:SPARE_RW_REGISTERS:DEV1_SPARE_RW1
 */
#define VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW1  VTSS_IOREG(0x01, 0, 0xaef1)

/** 
 * \brief
 * Spare
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW1 . dev1_spare_rw1
 */
#define  VTSS_F_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW1_dev1_spare_rw1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW1_dev1_spare_rw1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW1_dev1_spare_rw1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Device1 Spare R/W 2
 *
 * \details
 * Register: \a VENICE_DEV1:SPARE_RW_REGISTERS:DEV1_SPARE_RW2
 */
#define VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW2  VTSS_IOREG(0x01, 0, 0xaef2)

/** 
 * \brief
 * Spare
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW2 . dev1_spare_rw2
 */
#define  VTSS_F_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW2_dev1_spare_rw2(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW2_dev1_spare_rw2     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW2_dev1_spare_rw2(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Device1 Spare R/W 3
 *
 * \details
 * Register: \a VENICE_DEV1:SPARE_RW_REGISTERS:DEV1_SPARE_RW3
 */
#define VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW3  VTSS_IOREG(0x01, 0, 0xaef3)

/** 
 * \brief
 * Spare
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW3 . dev1_spare_rw3
 */
#define  VTSS_F_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW3_dev1_spare_rw3(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW3_dev1_spare_rw3     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW3_dev1_spare_rw3(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Device1 Spare R/W 4
 *
 * \details
 * Register: \a VENICE_DEV1:SPARE_RW_REGISTERS:DEV1_SPARE_RW4
 */
#define VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW4  VTSS_IOREG(0x01, 0, 0xaef4)

/** 
 * \brief
 * Spare
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW4 . dev1_spare_rw4
 */
#define  VTSS_F_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW4_dev1_spare_rw4(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW4_dev1_spare_rw4     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW4_dev1_spare_rw4(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Device1 Spare R/W 5
 *
 * \details
 * Register: \a VENICE_DEV1:SPARE_RW_REGISTERS:DEV1_SPARE_RW5
 */
#define VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW5  VTSS_IOREG(0x01, 0, 0xaef5)

/** 
 * \brief
 * Spare
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW5 . dev1_spare_rw5
 */
#define  VTSS_F_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW5_dev1_spare_rw5(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW5_dev1_spare_rw5     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW5_dev1_spare_rw5(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Device1 Spare R/W 6
 *
 * \details
 * Register: \a VENICE_DEV1:SPARE_RW_REGISTERS:DEV1_SPARE_RW6
 */
#define VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW6  VTSS_IOREG(0x01, 0, 0xaef6)

/** 
 * \brief
 * Spare
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW6 . dev1_spare_rw6
 */
#define  VTSS_F_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW6_dev1_spare_rw6(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW6_dev1_spare_rw6     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW6_dev1_spare_rw6(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Device1 Spare R/W 7
 *
 * \details
 * Register: \a VENICE_DEV1:SPARE_RW_REGISTERS:DEV1_SPARE_RW7
 */
#define VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW7  VTSS_IOREG(0x01, 0, 0xaef7)

/** 
 * \brief
 * Spare
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW7 . dev1_spare_rw7
 */
#define  VTSS_F_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW7_dev1_spare_rw7(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW7_dev1_spare_rw7     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW7_dev1_spare_rw7(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Device1 Spare R/W 8
 *
 * \details
 * Register: \a VENICE_DEV1:SPARE_RW_REGISTERS:DEV1_SPARE_RW8
 */
#define VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW8  VTSS_IOREG(0x01, 0, 0xaef8)

/** 
 * \brief
 * Spare
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW8 . dev1_spare_rw8
 */
#define  VTSS_F_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW8_dev1_spare_rw8(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW8_dev1_spare_rw8     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW8_dev1_spare_rw8(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Device1 Spare R/W 9
 *
 * \details
 * Register: \a VENICE_DEV1:SPARE_RW_REGISTERS:DEV1_SPARE_RW9
 */
#define VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW9  VTSS_IOREG(0x01, 0, 0xaef9)

/** 
 * \brief
 * Spare
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW9 . dev1_spare_rw9
 */
#define  VTSS_F_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW9_dev1_spare_rw9(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW9_dev1_spare_rw9     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW9_dev1_spare_rw9(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Device1 Spare R/W 10
 *
 * \details
 * Register: \a VENICE_DEV1:SPARE_RW_REGISTERS:DEV1_SPARE_RW10
 */
#define VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW10  VTSS_IOREG(0x01, 0, 0xaefa)

/** 
 * \brief
 * Spare
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW10 . dev1_spare_rw10
 */
#define  VTSS_F_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW10_dev1_spare_rw10(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW10_dev1_spare_rw10     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW10_dev1_spare_rw10(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Device1 Spare R/W 11
 *
 * \details
 * Register: \a VENICE_DEV1:SPARE_RW_REGISTERS:DEV1_SPARE_RW11
 */
#define VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW11  VTSS_IOREG(0x01, 0, 0xaefb)

/** 
 * \brief
 * Spare
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW11 . dev1_spare_rw11
 */
#define  VTSS_F_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW11_dev1_spare_rw11(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW11_dev1_spare_rw11     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW11_dev1_spare_rw11(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Device1 Spare R/W 12
 *
 * \details
 * Register: \a VENICE_DEV1:SPARE_RW_REGISTERS:DEV1_SPARE_RW12
 */
#define VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW12  VTSS_IOREG(0x01, 0, 0xaefc)

/** 
 * \brief
 * Spare
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW12 . dev1_spare_rw12
 */
#define  VTSS_F_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW12_dev1_spare_rw12(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW12_dev1_spare_rw12     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW12_dev1_spare_rw12(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Device1 Spare R/W 13
 *
 * \details
 * Register: \a VENICE_DEV1:SPARE_RW_REGISTERS:DEV1_SPARE_RW13
 */
#define VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW13  VTSS_IOREG(0x01, 0, 0xaefd)

/** 
 * \brief
 * Spare
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW13 . dev1_spare_rw13
 */
#define  VTSS_F_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW13_dev1_spare_rw13(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW13_dev1_spare_rw13     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW13_dev1_spare_rw13(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Device1 Spare R/W 14
 *
 * \details
 * Register: \a VENICE_DEV1:SPARE_RW_REGISTERS:DEV1_SPARE_RW14
 */
#define VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW14  VTSS_IOREG(0x01, 0, 0xaefe)

/** 
 * \brief
 * Spare
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW14 . dev1_spare_rw14
 */
#define  VTSS_F_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW14_dev1_spare_rw14(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW14_dev1_spare_rw14     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW14_dev1_spare_rw14(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Device1 Spare R/W 15
 *
 * \details
 * Register: \a VENICE_DEV1:SPARE_RW_REGISTERS:DEV1_SPARE_RW15
 */
#define VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW15  VTSS_IOREG(0x01, 0, 0xaeff)

/** 
 * \brief
 * Spare
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW15 . dev1_spare_rw15
 */
#define  VTSS_F_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW15_dev1_spare_rw15(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW15_dev1_spare_rw15     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SPARE_RW_REGISTERS_DEV1_SPARE_RW15_dev1_spare_rw15(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VENICE_DEV1:SD10G65_VSCOPE2
 *
 * SD10G65 VSCOPE Configuration and Status Register set
 */


/** 
 * \brief Vscope main config register A
 *
 * \details
 * Vscope main configuration register A
 *
 * Register: \a VENICE_DEV1:SD10G65_VSCOPE2:VSCOPE_MAIN_CFG_A
 */
#define VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_A  VTSS_IOREG(0x01, 0, 0xb000)

/** 
 * \brief
 * Disables writing of synth_phase_aux in synthesizer
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_A . SYN_PHASE_WR_DIS
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_A_SYN_PHASE_WR_DIS  VTSS_BIT(8)

/** 
 * \brief
 * Disables writing of ib_auxl_offset and ib_auxh_offset in IB
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_A . IB_AUX_OFFS_WR_DIS
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_A_IB_AUX_OFFS_WR_DIS  VTSS_BIT(7)

/** 
 * \brief
 * Disables writing of ib_jumpl_ena and ib_jumph_ena in IB
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_A . IB_JUMP_ENA_WR_DIS
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_A_IB_JUMP_ENA_WR_DIS  VTSS_BIT(6)

/** 
 * \brief
 * Counter output selection
 *
 * \details 
 * 0-3: error counter 0-3
 * 4: hit counter
 * 5: clock counter
 * 6: 8 LSBs of error counter 3-1 and hit counter
 * 7: 8 LSBs of error counter 3-0
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_A . CNT_OUT_SEL
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_A_CNT_OUT_SEL(x)  VTSS_ENCODE_BITFIELD(x,3,3)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_A_CNT_OUT_SEL     VTSS_ENCODE_BITMASK(3,3)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_A_CNT_OUT_SEL(x)  VTSS_EXTRACT_BITFIELD(x,3,3)

/** 
 * \brief
 * Comparator input selection
 *
 * \details 
 * [REF] 0
 * 1: auxL
 * 4
 * 5: auxH
 * 2
 * 7: main; [SUB] 5
 * 7: auxL
 * 0
 * 2: auxH
 * 1
 * 4: main (3
 * 6: reserved)
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_A . COMP_SEL
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_A_COMP_SEL(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_A_COMP_SEL     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_A_COMP_SEL(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Vscope main config register B
 *
 * \details
 * Vscope main configuration register B
 *
 * Register: \a VENICE_DEV1:SD10G65_VSCOPE2:VSCOPE_MAIN_CFG_B
 */
#define VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_B  VTSS_IOREG(0x01, 0, 0xb001)

/** 
 * \brief
 * Select GP reg input
 *
 * \details 
 * 0: rx (main)
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_B . GP_SELECT
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_B_GP_SELECT(x)  VTSS_ENCODE_BITFIELD(x,8,2)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_B_GP_SELECT     VTSS_ENCODE_BITMASK(8,2)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_B_GP_SELECT(x)  VTSS_EXTRACT_BITFIELD(x,8,2)

/** 
 * \brief
 * Allows to freeze the GP register value to assure valid reading
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_B . GP_REG_FREEZE
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_B_GP_REG_FREEZE  VTSS_BIT(7)

/** 
 * \brief
 * Scan limit, selects which counter saturation limits the other counters
 *
 * \details 
 * 0: clock counter
 * 1: hit counter
 * 2: error counters
 * 3: no limit
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_B . SCAN_LIM
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_B_SCAN_LIM(x)  VTSS_ENCODE_BITFIELD(x,5,2)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_B_SCAN_LIM     VTSS_ENCODE_BITMASK(5,2)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_B_SCAN_LIM(x)  VTSS_EXTRACT_BITFIELD(x,5,2)

/** 
 * \brief
 * Preload value for error counter
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_B . PRELOAD_VAL
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_B_PRELOAD_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_B_PRELOAD_VAL     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_B_PRELOAD_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief Vscope main config register C
 *
 * \details
 * Vscope main configuration register C
 *
 * Register: \a VENICE_DEV1:SD10G65_VSCOPE2:VSCOPE_MAIN_CFG_C
 */
#define VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_C  VTSS_IOREG(0x01, 0, 0xb002)

/** 
 * \brief
 * Disable interrupt output
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_C . INTR_DIS
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_C_INTR_DIS  VTSS_BIT(12)

/** 
 * \brief
 * Enable trigger
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_C . TRIG_ENA
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_C_TRIG_ENA  VTSS_BIT(11)

/** 
 * \brief
 * Counter enable (bit 4) implicitly done by reading the counter; unused in
 * hw-scan mode
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_C . QUICK_SCAN
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_C_QUICK_SCAN  VTSS_BIT(10)

/** 
 * \brief
 * Counter period: preload value for clock counter
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_C . COUNT_PER
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_C_COUNT_PER(x)  VTSS_ENCODE_BITFIELD(x,5,5)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_C_COUNT_PER     VTSS_ENCODE_BITMASK(5,5)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_C_COUNT_PER(x)  VTSS_EXTRACT_BITFIELD(x,5,5)

/** 
 * \brief
 * Enable Counting; unused in hw-scan mode
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_C . CNT_ENA
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_C_CNT_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Interface Width
 *
 * \details 
 * 0: 8 bit
 * 1: 10 bit
 * 2: 16 bit
 * 3: 20 bit
 * 4: 32 bit
 * 5: 40 bit
 * others: reserved
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_C . IF_MODE
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_C_IF_MODE(x)  VTSS_ENCODE_BITFIELD(x,1,3)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_C_IF_MODE     VTSS_ENCODE_BITMASK(1,3)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_C_IF_MODE(x)  VTSS_EXTRACT_BITFIELD(x,1,3)

/** 
 * \brief
 * Enable Vscope
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_C . VSCOPE_ENA
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_MAIN_CFG_C_VSCOPE_ENA  VTSS_BIT(0)


/** 
 * \brief Vscope pattern lock config register A
 *
 * \details
 * Vscope pattern lock configuration register A
 *
 * Register: \a VENICE_DEV1:SD10G65_VSCOPE2:VSCOPE_PAT_LOCK_CFG_A
 */
#define VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_PAT_LOCK_CFG_A  VTSS_IOREG(0x01, 0, 0xb003)

/** 
 * \brief
 * Preload value for hit counter
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_PAT_LOCK_CFG_A . PRELOAD_HIT_CNT
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_PAT_LOCK_CFG_A_PRELOAD_HIT_CNT(x)  VTSS_ENCODE_BITFIELD(x,10,5)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_PAT_LOCK_CFG_A_PRELOAD_HIT_CNT     VTSS_ENCODE_BITMASK(10,5)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_PAT_LOCK_CFG_A_PRELOAD_HIT_CNT(x)  VTSS_EXTRACT_BITFIELD(x,10,5)

/** 
 * \brief
 * Don't Care mask: Enable history mask usage.
 *
 * \details 
 * 0: enable history mask bit
 * 1: history mask bit is "don't care"
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_PAT_LOCK_CFG_A . DC_MASK
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_PAT_LOCK_CFG_A_DC_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,10)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_PAT_LOCK_CFG_A_DC_MASK     VTSS_ENCODE_BITMASK(0,10)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_PAT_LOCK_CFG_A_DC_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,10)


/** 
 * \brief Vscope pattern lock config register B
 *
 * \details
 * Vscope pattern lock configuration register B
 *
 * Register: \a VENICE_DEV1:SD10G65_VSCOPE2:VSCOPE_PAT_LOCK_CFG_B
 */
#define VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_PAT_LOCK_CFG_B  VTSS_IOREG(0x01, 0, 0xb004)

/** 
 * \brief
 * History mask: Respective sequence is expected in reference input
 * (comp_sel); if enabled (dc_mask) before hit and error counting is
 * enabled
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_PAT_LOCK_CFG_B . HIST_MASK
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_PAT_LOCK_CFG_B_HIST_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,10)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_PAT_LOCK_CFG_B_HIST_MASK     VTSS_ENCODE_BITMASK(0,10)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_PAT_LOCK_CFG_B_HIST_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,10)


/** 
 * \brief Vscope hw scan config register 1A
 *
 * \details
 * Vscope HW scan configuration register 1A
 *
 * Register: \a VENICE_DEV1:SD10G65_VSCOPE2:VSCOPE_HW_SCAN_CFG_1A
 */
#define VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1A  VTSS_IOREG(0x01, 0, 0xb005)

/** 
 * \brief
 * Invert the jumph_ena and jumpl_ena bit in HW scan mode
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1A . PHASE_JUMP_INV
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1A_PHASE_JUMP_INV  VTSS_BIT(13)

/** 
 * \brief
 * Offset between AuxL amplitude (reference) and AuxH amplitude, signed
 * (2s-complement), +- 1/4 amplitude max.
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1A . AMPL_OFFS_VAL
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1A_AMPL_OFFS_VAL(x)  VTSS_ENCODE_BITFIELD(x,8,5)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1A_AMPL_OFFS_VAL     VTSS_ENCODE_BITMASK(8,5)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1A_AMPL_OFFS_VAL(x)  VTSS_EXTRACT_BITFIELD(x,8,5)

/** 
 * \brief
 * Maximum phase increment value before wrapping
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1A . MAX_PHASE_INCR_VAL
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1A_MAX_PHASE_INCR_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1A_MAX_PHASE_INCR_VAL     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1A_MAX_PHASE_INCR_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Vscope hw scan config register 1B
 *
 * \details
 * Vscope HW scan configuration register 1B
 *
 * Register: \a VENICE_DEV1:SD10G65_VSCOPE2:VSCOPE_HW_SCAN_CFG_1B
 */
#define VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1B  VTSS_IOREG(0x01, 0, 0xb006)

/** 
 * \brief
 * Maximum amplitude increment value before wrapping
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1B . MAX_AMPL_INCR_VAL
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1B_MAX_AMPL_INCR_VAL(x)  VTSS_ENCODE_BITFIELD(x,10,6)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1B_MAX_AMPL_INCR_VAL     VTSS_ENCODE_BITMASK(10,6)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1B_MAX_AMPL_INCR_VAL(x)  VTSS_EXTRACT_BITFIELD(x,10,6)

/** 
 * \brief
 * Phase increment per scan step
 *
 * \details 
 * Increment = phase_incr + 1
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1B . PHASE_INCR
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1B_PHASE_INCR(x)  VTSS_ENCODE_BITFIELD(x,7,3)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1B_PHASE_INCR     VTSS_ENCODE_BITMASK(7,3)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1B_PHASE_INCR(x)  VTSS_EXTRACT_BITFIELD(x,7,3)

/** 
 * \brief
 * Amplitude increment per scan step
 *
 * \details 
 * Increment = ampl_incr + 1
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1B . AMPL_INCR
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1B_AMPL_INCR(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1B_AMPL_INCR     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1B_AMPL_INCR(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * Number of scans per iteration in N-point-scan mode
 *
 * \details 
 * 0: 1
 * 1: 2
 * 2: 4
 * 3: 8
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1B . NUM_SCANS_PER_ITR
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1B_NUM_SCANS_PER_ITR(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1B_NUM_SCANS_PER_ITR     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1B_NUM_SCANS_PER_ITR(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Enables HW scan with N results per scan or fast-scan
 *
 * \details 
 * 0: off
 * 1: N-point scan
 * 2: fast-scan (sq)
 * 3: fast-scan (diag)
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1B . HW_SCAN_ENA
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1B_HW_SCAN_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1B_HW_SCAN_ENA     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_1B_HW_SCAN_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Vscope hw config register 2A
 *
 * \details
 * Vscope HW scan configuration register 2A
 *
 * Register: \a VENICE_DEV1:SD10G65_VSCOPE2:VSCOPE_HW_SCAN_CFG_2A
 */
#define VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_2A  VTSS_IOREG(0x01, 0, 0xb007)

/** 
 * \brief
 * Threshold for error_counter in fast-scan mode
 *
 * \details 
 * N+1
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_2A . FAST_SCAN_THRES
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_2A_FAST_SCAN_THRES(x)  VTSS_ENCODE_BITFIELD(x,13,3)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_2A_FAST_SCAN_THRES     VTSS_ENCODE_BITMASK(13,3)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_2A_FAST_SCAN_THRES(x)  VTSS_EXTRACT_BITFIELD(x,13,3)

/** 
 * \brief
 * Left shift for threshold of error_counter in fast-scan mode
 *
 * \details 
 * threshold = (fast_scan_thres+1) shift_left fs_thres_shift
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_2A . FS_THRES_SHIFT
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_2A_FS_THRES_SHIFT(x)  VTSS_ENCODE_BITFIELD(x,8,5)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_2A_FS_THRES_SHIFT     VTSS_ENCODE_BITMASK(8,5)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_2A_FS_THRES_SHIFT(x)  VTSS_EXTRACT_BITFIELD(x,8,5)

/** 
 * \brief
 * Value at which jumpl_ena and jumph_ena in IB must be toggled
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_2A . PHASE_JUMP_VAL
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_2A_PHASE_JUMP_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_2A_PHASE_JUMP_VAL     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_2A_PHASE_JUMP_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Vscope hw config register 2B
 *
 * \details
 * Vscope HW scan configuration register 2B
 *
 * Register: \a VENICE_DEV1:SD10G65_VSCOPE2:VSCOPE_HW_SCAN_CFG_2B
 */
#define VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_2B  VTSS_IOREG(0x01, 0, 0xb008)

/** 
 * \brief
 * Disable IB amplitude symmetry compensation for AuxH and AuxL
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_2B . AUX_AMPL_SYM_DIS
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_2B_AUX_AMPL_SYM_DIS  VTSS_BIT(15)

/** 
 * \brief
 * Start value for VScope amplitude in N-point-scan mode and fast-scan mode
 * (before IB amplitude symmetry compensation)
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_2B . AMPL_START_VAL
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_2B_AMPL_START_VAL(x)  VTSS_ENCODE_BITFIELD(x,8,6)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_2B_AMPL_START_VAL     VTSS_ENCODE_BITMASK(8,6)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_2B_AMPL_START_VAL(x)  VTSS_EXTRACT_BITFIELD(x,8,6)

/** 
 * \brief
 * Start value for VScope phase in N-point-scan mode and fast-scan mode
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_2B . PHASE_START_VAL
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_2B_PHASE_START_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_2B_PHASE_START_VAL     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_HW_SCAN_CFG_2B_PHASE_START_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Vscope status register
 *
 * \details
 * Vscope status register
 *
 * Register: \a VENICE_DEV1:SD10G65_VSCOPE2:VSCOPE_STAT
 */
#define VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_STAT  VTSS_IOREG(0x01, 0, 0xb009)

/** 
 * \brief
 * 8 MSBs of general purpose register
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_STAT . GP_REG_MSB
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_STAT_GP_REG_MSB(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_STAT_GP_REG_MSB     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_STAT_GP_REG_MSB(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Fast scan mode: Indicator per cursor position whether threshold was
 * reached
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_STAT . FAST_SCAN_HIT
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_STAT_FAST_SCAN_HIT(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_STAT_FAST_SCAN_HIT     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_STAT_FAST_SCAN_HIT(x)  VTSS_EXTRACT_BITFIELD(x,4,4)

/** 
 * \brief
 * Done sticky
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_STAT . DONE_STICKY
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_STAT_DONE_STICKY  VTSS_BIT(0)


/** 
 * \brief Vscope counter register A
 *
 * \details
 * Vscope counter register A
 *
 * Register: \a VENICE_DEV1:SD10G65_VSCOPE2:VSCOPE_CNT_A
 */
#define VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_CNT_A  VTSS_IOREG(0x01, 0, 0xb00a)

/** 
 * \brief
 * Counter value higher 16-bit MSB [31:16]
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_CNT_A . COUNTER_MSB
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_CNT_A_COUNTER_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_CNT_A_COUNTER_MSB     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_CNT_A_COUNTER_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Vscope counter register B
 *
 * \details
 * Vscope counter register B
 *
 * Register: \a VENICE_DEV1:SD10G65_VSCOPE2:VSCOPE_CNT_B
 */
#define VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_CNT_B  VTSS_IOREG(0x01, 0, 0xb00b)

/** 
 * \brief
 * Counter value lower 16-bit LSB [15:0]
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_CNT_B . COUNTER_LSB
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_CNT_B_COUNTER_LSB(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_CNT_B_COUNTER_LSB     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_CNT_B_COUNTER_LSB(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Vscope general purpose register A
 *
 * \details
 * Vscope general purpose  register A
 *
 * Register: \a VENICE_DEV1:SD10G65_VSCOPE2:VSCOPE_DBG_LSB_A
 */
#define VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_DBG_LSB_A  VTSS_IOREG(0x01, 0, 0xb00c)

/** 
 * \brief
 * 16 bit MSB of a 32 bit general purpose register [31:16]
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_DBG_LSB_A . GP_REG_LSB_A
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_DBG_LSB_A_GP_REG_LSB_A(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_DBG_LSB_A_GP_REG_LSB_A     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_DBG_LSB_A_GP_REG_LSB_A(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Vscope general purpose register A
 *
 * \details
 * Vscope general purpose  register B
 *
 * Register: \a VENICE_DEV1:SD10G65_VSCOPE2:VSCOPE_DBG_LSB_B
 */
#define VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_DBG_LSB_B  VTSS_IOREG(0x01, 0, 0xb00d)

/** 
 * \brief
 * 16 bit LSB of a 32 bit general purpose register [15:0]
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_DBG_LSB_B . GP_REG_LSB_B
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_DBG_LSB_B_GP_REG_LSB_B(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_DBG_LSB_B_GP_REG_LSB_B     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SD10G65_VSCOPE2_VSCOPE_DBG_LSB_B_GP_REG_LSB_B(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VENICE_DEV1:SD10G65_DFT
 *
 * SD10G65 DFT Configuration and Status Register set
 */


/** 
 * \brief SD10G65 DFT Main configuration register 1
 *
 * \details
 * Main configuration register 1 for SD10G65 DFT.
 *
 * Register: \a VENICE_DEV1:SD10G65_DFT:DFT_RX_CFG_1
 */
#define VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_1  VTSS_IOREG(0x01, 0, 0xb100)

/** 
 * \brief
 * Enables data through from gearbox to gearbox
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_1 . DIRECT_THROUGH_ENA_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_1_DIRECT_THROUGH_ENA_CFG  VTSS_BIT(10)

/** 
 * \brief
 * Captures data from error counter to allow reading of stable data
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_1 . ERR_CNT_CAPT_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_1_ERR_CNT_CAPT_CFG  VTSS_BIT(9)

/** 
 * \brief
 * Data source selection
 *
 * \details 
 * 0: main path
 * 1: vscope high path
 * 2: vscope low path
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_1 . RX_DATA_SRC_SEL
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_1_RX_DATA_SRC_SEL(x)  VTSS_ENCODE_BITFIELD(x,7,2)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_1_RX_DATA_SRC_SEL     VTSS_ENCODE_BITMASK(7,2)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_1_RX_DATA_SRC_SEL(x)  VTSS_EXTRACT_BITFIELD(x,7,2)

/** 
 * \brief
 * States in which error counting is enabled
 *
 * \details 
 * 3:all but IDLE; 2:check 1:stable+check
 * 0:wait_stable+stable+check
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_1 . BIST_CNT_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_1_BIST_CNT_CFG(x)  VTSS_ENCODE_BITFIELD(x,5,2)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_1_BIST_CNT_CFG     VTSS_ENCODE_BITMASK(5,2)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_1_BIST_CNT_CFG(x)  VTSS_EXTRACT_BITFIELD(x,5,2)

/** 
 * \brief
 * Disable change of stored patterns (e.g. to avoid changes during
 * read-out)
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_1 . FREEZE_PATTERN_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_1_FREEZE_PATTERN_CFG  VTSS_BIT(4)

/** 
 * \brief
 * Selects pattern to check
 *
 * \details 
 * 0: PRBS pattern
 * 1: constant pattern
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_1 . CHK_MODE_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_1_CHK_MODE_CFG  VTSS_BIT(3)

/** 
 * \brief
 * Selects DES interface width
 *
 * \details 
 * 0:8
 * 1:10
 * 2:16
 * 3:20
 * 4:32
 * 5:40 (default)
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_1 . RX_WID_SEL_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_1_RX_WID_SEL_CFG(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_1_RX_WID_SEL_CFG     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_1_RX_WID_SEL_CFG(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief SD10G65 DFT Main configuration register 2
 *
 * \details
 * Main configuration register 2 for SD10G65 DFT.
 *
 * Register: \a VENICE_DEV1:SD10G65_DFT:DFT_RX_CFG_2
 */
#define VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2  VTSS_IOREG(0x01, 0, 0xb101)

/** 
 * \brief
 * Pattern generator: 0:bytes mode; 1:10-bits word mode
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2 . RX_WORD_MODE_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2_RX_WORD_MODE_CFG  VTSS_BIT(14)

/** 
 * \brief
 * Selects PRBS check
 *
 * \details 
 * 0: prbs7
 * 1: prbs15
 * 2: prbs23
 * 3: prbs11
 * 4: prbs31 (default)
 * 5: prbs9
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2 . RX_PRBS_SEL_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2_RX_PRBS_SEL_CFG(x)  VTSS_ENCODE_BITFIELD(x,11,3)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2_RX_PRBS_SEL_CFG     VTSS_ENCODE_BITMASK(11,3)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2_RX_PRBS_SEL_CFG(x)  VTSS_EXTRACT_BITFIELD(x,11,3)

/** 
 * \brief
 * Enables PRBS checker input inversion
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2 . INV_ENA_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2_INV_ENA_CFG  VTSS_BIT(10)

/** 
 * \brief
 * Selects compare mode
 *
 * \details 
 * 0: compare mode possible
 * 1 learn mode is forced
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2 . CMP_MODE_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2_CMP_MODE_CFG  VTSS_BIT(9)

/** 
 * \brief
 * Number of consecutive errors/non-errors before transitioning to
 * respective state
 *
 * \details 
 * value =
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2 . LRN_CNT_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2_LRN_CNT_CFG(x)  VTSS_ENCODE_BITFIELD(x,6,3)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2_LRN_CNT_CFG     VTSS_ENCODE_BITMASK(6,3)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2_LRN_CNT_CFG(x)  VTSS_EXTRACT_BITFIELD(x,6,3)

/** 
 * \brief
 * SW reset of error counter; rising edge activates reset
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2 . CNT_RST
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2_CNT_RST  VTSS_BIT(5)

/** 
 * \brief
 * Selects modes in which error counter is active
 *
 * \details 
 * 0:learn and compare mode
 * 1:transition between modes
 * 2:learn mode
 * 3:compare mode
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2 . CNT_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2_CNT_CFG(x)  VTSS_ENCODE_BITFIELD(x,3,2)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2_CNT_CFG     VTSS_ENCODE_BITMASK(3,2)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2_CNT_CFG(x)  VTSS_EXTRACT_BITFIELD(x,3,2)

/** 
 * \brief
 * BIST mode
 *
 * \details 
 * 0: off
 * 1: BIST
 * 2: BER
 * 3:CONT (infinite mode)
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2 . BIST_MODE_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2_BIST_MODE_CFG(x)  VTSS_ENCODE_BITFIELD(x,1,2)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2_BIST_MODE_CFG     VTSS_ENCODE_BITMASK(1,2)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2_BIST_MODE_CFG(x)  VTSS_EXTRACT_BITFIELD(x,1,2)

/** 
 * \brief
 * Enable RX DFT capability
 *
 * \details 
 * 0: Disable DFT
 * 1: Enable DFT
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2 . DFT_RX_ENA
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_RX_CFG_2_DFT_RX_ENA  VTSS_BIT(0)


/** 
 * \brief SD10G65 DFT pattern mask configuration register 1 
 *
 * \details
 * Configuration register 1 for SD10G65 DFT to mask data bits preventing
 * error counting for these bits.
 *
 * Register: \a VENICE_DEV1:SD10G65_DFT:DFT_RX_MASK_CFG_1
 */
#define VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_MASK_CFG_1  VTSS_IOREG(0x01, 0, 0xb102)

/** 
 * \brief
 * Mask out (active high) errors in 16 bit MSB data bits [31:16]
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_MASK_CFG_1 . LSB_MASK_CFG_1
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_RX_MASK_CFG_1_LSB_MASK_CFG_1(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_RX_MASK_CFG_1_LSB_MASK_CFG_1     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_RX_MASK_CFG_1_LSB_MASK_CFG_1(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief SD10G65 DFT pattern mask configuration register 2
 *
 * \details
 * Configuration register 2 for SD10G65 DFT to mask data bits preventing
 * error counting for these bits.
 *
 * Register: \a VENICE_DEV1:SD10G65_DFT:DFT_RX_MASK_CFG_2
 */
#define VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_MASK_CFG_2  VTSS_IOREG(0x01, 0, 0xb103)

/** 
 * \brief
 * Mask out (active high) errors in 16 LSB data bits [15:0]
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_MASK_CFG_2 . LSB_MASK_CFG_2
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_RX_MASK_CFG_2_LSB_MASK_CFG_2(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_RX_MASK_CFG_2_LSB_MASK_CFG_2     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_RX_MASK_CFG_2_LSB_MASK_CFG_2(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief SD10G65 DFT Pattern checker configuration register 1
 *
 * \details
 * Pattern checker configuration register 1 for SD10G65 DFT.
 *
 * Register: \a VENICE_DEV1:SD10G65_DFT:DFT_RX_PAT_CFG_1
 */
#define VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_PAT_CFG_1  VTSS_IOREG(0x01, 0, 0xb104)

/** 
 * \brief
 * Mask out (active high) errors in 8 MSB data bits
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_PAT_CFG_1 . MSB_MASK_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_RX_PAT_CFG_1_MSB_MASK_CFG(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_RX_PAT_CFG_1_MSB_MASK_CFG     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_RX_PAT_CFG_1_MSB_MASK_CFG(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Pattern read enable
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_PAT_CFG_1 . PAT_READ_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_RX_PAT_CFG_1_PAT_READ_CFG  VTSS_BIT(0)


/** 
 * \brief SD10G65 DFT Pattern checker configuration register 2
 *
 * \details
 * Pattern checker configuration register 2 for SD10G65 DFT.
 *
 * Register: \a VENICE_DEV1:SD10G65_DFT:DFT_RX_PAT_CFG_2
 */
#define VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_PAT_CFG_2  VTSS_IOREG(0x01, 0, 0xb105)

/** 
 * \brief
 * Maximum address in Checker (before continuing with address 0)
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_PAT_CFG_2 . MAX_ADDR_CHK_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_RX_PAT_CFG_2_MAX_ADDR_CHK_CFG(x)  VTSS_ENCODE_BITFIELD(x,8,4)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_RX_PAT_CFG_2_MAX_ADDR_CHK_CFG     VTSS_ENCODE_BITMASK(8,4)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_RX_PAT_CFG_2_MAX_ADDR_CHK_CFG(x)  VTSS_EXTRACT_BITFIELD(x,8,4)

/** 
 * \brief
 * Address to read patterns from used by SW
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_RX_PAT_CFG_2 . READ_ADDR_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_RX_PAT_CFG_2_READ_ADDR_CFG(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_RX_PAT_CFG_2_READ_ADDR_CFG     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_RX_PAT_CFG_2_READ_ADDR_CFG(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief SD10G65 DFT BIST configuration register A
 *
 * \details
 * BIST configuration register A for SD10G65 DFT controlling 'check and
 * wait-stable' mode.
 *
 * Register: \a VENICE_DEV1:SD10G65_DFT:DFT_BIST_CFG0A
 */
#define VTSS_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG0A  VTSS_IOREG(0x01, 0, 0xb106)

/** 
 * \brief
 * BIST FSM: threshold to leave DOZE state
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG0A . WAKEUP_DLY_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG0A_WAKEUP_DLY_CFG(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG0A_WAKEUP_DLY_CFG     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG0A_WAKEUP_DLY_CFG(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief SD10G65 DFT BIST configuration register B
 *
 * \details
 * BIST configuration register B for SD10G65 DFT controlling 'check and
 * wait-stable' mode.
 *
 * Register: \a VENICE_DEV1:SD10G65_DFT:DFT_BIST_CFG0B
 */
#define VTSS_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG0B  VTSS_IOREG(0x01, 0, 0xb107)

/** 
 * \brief
 * BIST FSM: threshold to enter FINISHED state
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG0B . MAX_BIST_FRAMES_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG0B_MAX_BIST_FRAMES_CFG(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG0B_MAX_BIST_FRAMES_CFG     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG0B_MAX_BIST_FRAMES_CFG(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief SD10G65 DFT BIST configuration register A
 *
 * \details
 * BIST configuration register A for SD10G65 DFT  controlling 'stable'
 * mode.
 *
 * Register: \a VENICE_DEV1:SD10G65_DFT:DFT_BIST_CFG1A
 */
#define VTSS_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG1A  VTSS_IOREG(0x01, 0, 0xb108)

/** 
 * \brief
 * BIST FSM: threshold to iterate counter for max_stable_attempts
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG1A . MAX_UNSTABLE_CYC_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG1A_MAX_UNSTABLE_CYC_CFG(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG1A_MAX_UNSTABLE_CYC_CFG     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG1A_MAX_UNSTABLE_CYC_CFG(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief SD10G65 DFT BIST configuration register B
 *
 * \details
 * BIST configuration register B for SD10G65 DFT  controlling 'stable'
 * mode.
 *
 * Register: \a VENICE_DEV1:SD10G65_DFT:DFT_BIST_CFG1B
 */
#define VTSS_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG1B  VTSS_IOREG(0x01, 0, 0xb109)

/** 
 * \brief
 * BIST FSM: threshold to enter CHECK state
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG1B . STABLE_THRES_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG1B_STABLE_THRES_CFG(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG1B_STABLE_THRES_CFG     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG1B_STABLE_THRES_CFG(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief SD10G65 DFT BIST configuration register A
 *
 * \details
 * BIST configuration register B for SD10G65 DFT controlling frame length
 * in 'check' mode.
 *
 * Register: \a VENICE_DEV1:SD10G65_DFT:DFT_BIST_CFG2A
 */
#define VTSS_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG2A  VTSS_IOREG(0x01, 0, 0xb10a)

/** 
 * \brief
 * BIST FSM: threshold to iterate counter for max_bist_frames [31:16]
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG2A . FRAME_LEN_CFG_MSB
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG2A_FRAME_LEN_CFG_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG2A_FRAME_LEN_CFG_MSB     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG2A_FRAME_LEN_CFG_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief SD10G65 DFT BIST configuration register B
 *
 * \details
 * BIST configuration register B for SD10G65 DFT controlling frame length
 * in 'check' mode.
 *
 * Register: \a VENICE_DEV1:SD10G65_DFT:DFT_BIST_CFG2B
 */
#define VTSS_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG2B  VTSS_IOREG(0x01, 0, 0xb10b)

/** 
 * \brief
 * BIST FSM: threshold to iterate counter for max_bist_frames [15:0]
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG2B . FRAME_LEN_CFG_LSB
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG2B_FRAME_LEN_CFG_LSB(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG2B_FRAME_LEN_CFG_LSB     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG2B_FRAME_LEN_CFG_LSB(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief SD10G65 DFT BIST configuration register A
 *
 * \details
 * BIST configuration register A for SD10G65 DFT controlling stable
 * attempts in ' wait-stable' mode.
 *
 * Register: \a VENICE_DEV1:SD10G65_DFT:DFT_BIST_CFG3A
 */
#define VTSS_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG3A  VTSS_IOREG(0x01, 0, 0xb10c)

/** 
 * \brief
 * BIST FSM: threshold to enter SYNC_ERR state [31:16]
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG3A . MAX_STABLE_ATTEMPTS_CFG_MSB
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG3A_MAX_STABLE_ATTEMPTS_CFG_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG3A_MAX_STABLE_ATTEMPTS_CFG_MSB     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG3A_MAX_STABLE_ATTEMPTS_CFG_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief SD10G65 DFT BIST configuration register B
 *
 * \details
 * BIST configuration register B for SD10G65 DFT controlling stable
 * attempts in ' wait-stable' mode.
 *
 * Register: \a VENICE_DEV1:SD10G65_DFT:DFT_BIST_CFG3B
 */
#define VTSS_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG3B  VTSS_IOREG(0x01, 0, 0xb10d)

/** 
 * \brief
 * BIST FSM: threshold to enter SYNC_ERR state [15:0]
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG3B . MAX_STABLE_ATTEMPTS_CFG_LSB
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG3B_MAX_STABLE_ATTEMPTS_CFG_LSB(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG3B_MAX_STABLE_ATTEMPTS_CFG_LSB     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_BIST_CFG3B_MAX_STABLE_ATTEMPTS_CFG_LSB(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief SD10G65 DFT error status register 1
 *
 * \details
 * Status register 1 for SD10G65 DFT containing the error counter value
 *
 * Register: \a VENICE_DEV1:SD10G65_DFT:DFT_ERR_STAT_1
 */
#define VTSS_VENICE_DEV1_SD10G65_DFT_DFT_ERR_STAT_1  VTSS_IOREG(0x01, 0, 0xb10e)

/** 
 * \brief
 * Counter output depending on cnt_cfg_i [31:16]
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_ERR_STAT_1 . ERR_CNT_MSB
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_ERR_STAT_1_ERR_CNT_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_ERR_STAT_1_ERR_CNT_MSB     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_ERR_STAT_1_ERR_CNT_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief SD10G65 DFT error status register 2
 *
 * \details
 * Status register B2 for SD10G65 DFT containing the error counter value
 *
 * Register: \a VENICE_DEV1:SD10G65_DFT:DFT_ERR_STAT_2
 */
#define VTSS_VENICE_DEV1_SD10G65_DFT_DFT_ERR_STAT_2  VTSS_IOREG(0x01, 0, 0xb10f)

/** 
 * \brief
 * Counter output depending on cnt_cfg_i [15:0]
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_ERR_STAT_2 . ERR_CNT_LSB
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_ERR_STAT_2_ERR_CNT_LSB(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_ERR_STAT_2_ERR_CNT_LSB     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_ERR_STAT_2_ERR_CNT_LSB(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief SD10G65 DFT PRBS status register 1
 *
 * \details
 * Status register 1 for SD10G65 DFT containing the PRBS data related to
 * 1st sync lost event
 *
 * Register: \a VENICE_DEV1:SD10G65_DFT:DFT_PRBS_STAT_1
 */
#define VTSS_VENICE_DEV1_SD10G65_DFT_DFT_PRBS_STAT_1  VTSS_IOREG(0x01, 0, 0xb110)

/** 
 * \brief
 * PRBS data after first sync lost [31:16]
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_PRBS_STAT_1 . PRBS_DATA_STAT_MSB
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_PRBS_STAT_1_PRBS_DATA_STAT_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_PRBS_STAT_1_PRBS_DATA_STAT_MSB     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_PRBS_STAT_1_PRBS_DATA_STAT_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief SD10G65 DFT PRBS status register 2
 *
 * \details
 * Status register 2 for SD10G65 DFT containing the PRBS data related to
 * 1st sync lost event
 *
 * Register: \a VENICE_DEV1:SD10G65_DFT:DFT_PRBS_STAT_2
 */
#define VTSS_VENICE_DEV1_SD10G65_DFT_DFT_PRBS_STAT_2  VTSS_IOREG(0x01, 0, 0xb111)

/** 
 * \brief
 * PRBS data after first sync lost [15:0]
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_PRBS_STAT_2 . PRBS_DATA_STAT_LSB
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_PRBS_STAT_2_PRBS_DATA_STAT_LSB(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_PRBS_STAT_2_PRBS_DATA_STAT_LSB     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_PRBS_STAT_2_PRBS_DATA_STAT_LSB(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief SD10G65 DFT miscellaneous status register 1
 *
 * \details
 * Status register 1 for SD10G65 DFT
 *
 * Register: \a VENICE_DEV1:SD10G65_DFT:DFT_MAIN_STAT_1
 */
#define VTSS_VENICE_DEV1_SD10G65_DFT_DFT_MAIN_STAT_1  VTSS_IOREG(0x01, 0, 0xb112)

/** 
 * \brief
 * 10 bits data word at address 'read_addr_cfg' used for further
 * observation by SW
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_MAIN_STAT_1 . CMP_DATA_STAT
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_MAIN_STAT_1_CMP_DATA_STAT(x)  VTSS_ENCODE_BITFIELD(x,0,10)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_MAIN_STAT_1_CMP_DATA_STAT     VTSS_ENCODE_BITMASK(0,10)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_MAIN_STAT_1_CMP_DATA_STAT(x)  VTSS_EXTRACT_BITFIELD(x,0,10)


/** 
 * \brief SD10G65 DFT miscellaneous status register 2
 *
 * \details
 * Status register 2 for SD10G65 DFT
 *
 * Register: \a VENICE_DEV1:SD10G65_DFT:DFT_MAIN_STAT_2
 */
#define VTSS_VENICE_DEV1_SD10G65_DFT_DFT_MAIN_STAT_2  VTSS_IOREG(0x01, 0, 0xb113)

/** 
 * \brief
 * Data input unchanged for at least 7 clock cycles (defined by
 * c_STCK_CNT_THRES)
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_MAIN_STAT_2 . STUCK_AT
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_MAIN_STAT_2_STUCK_AT  VTSS_BIT(4)

/** 
 * \brief
 * BIST: no sync found since BIST enabled
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_MAIN_STAT_2 . NO_SYNC
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_MAIN_STAT_2_NO_SYNC  VTSS_BIT(3)

/** 
 * \brief
 * BIST: input data not stable
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_MAIN_STAT_2 . INSTABLE
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_MAIN_STAT_2_INSTABLE  VTSS_BIT(2)

/** 
 * \brief
 * BIST not complete (i.e. not reached stable state or following)
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_MAIN_STAT_2 . INCOMPLETE
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_MAIN_STAT_2_INCOMPLETE  VTSS_BIT(1)

/** 
 * \brief
 * BIST is active (i.e. left DOZE but did not enter a final state)
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_MAIN_STAT_2 . ACTIVE
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_MAIN_STAT_2_ACTIVE  VTSS_BIT(0)


/** 
 * \brief SD10G65 DFT Main configuration register
 *
 * \details
 * Main configuration register for SD10G65 DFT.
 *
 * Register: \a VENICE_DEV1:SD10G65_DFT:DFT_TX_CFG
 */
#define VTSS_VENICE_DEV1_SD10G65_DFT_DFT_TX_CFG  VTSS_IOREG(0x01, 0, 0xb114)

/** 
 * \brief
 * Clears the tx_stuck_at_sticky status bit
 *
 * \details 
 * 0: Keep sticky bit value
 * 1: Clear sticky bit
 * Note: While 1 each write access to any register of this SPI clears the
 * sticky bit.
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_TX_CFG . TX_STUCK_AT_CLR_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_TX_CFG_TX_STUCK_AT_CLR_CFG  VTSS_BIT(13)

/** 
 * \brief
 * Enables (1) reset of PRBS generator in case of unchanged data
 * ('stuck-at') for at least 511 clock cycles. Can be disabled (0) e.g. in
 * scrambler mode to avoid the very rare case that input patterns allow to
 * keep the generator's shift register filled with a constant value.
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_TX_CFG . RST_ON_STUCK_AT_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_TX_CFG_RST_ON_STUCK_AT_CFG  VTSS_BIT(12)

/** 
 * \brief
 * Selects SER interface width
 *
 * \details 
 * 0:8
 * 1:10
 * 2:16
 * 3:20
 * 4:32
 * 5:40 (default)
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_TX_CFG . TX_WID_SEL_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_TX_CFG_TX_WID_SEL_CFG(x)  VTSS_ENCODE_BITFIELD(x,9,3)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_TX_CFG_TX_WID_SEL_CFG     VTSS_ENCODE_BITMASK(9,3)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_TX_CFG_TX_WID_SEL_CFG(x)  VTSS_EXTRACT_BITFIELD(x,9,3)

/** 
 * \brief
 * Selects PRBS generator
 *
 * \details 
 * 0: prbs7
 * 1: prbs15
 * 2: prbs23
 * 3: prbs11
 * 4: prbs31 (default)
 * 5: prbs9
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_TX_CFG . TX_PRBS_SEL_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_TX_CFG_TX_PRBS_SEL_CFG(x)  VTSS_ENCODE_BITFIELD(x,6,3)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_TX_CFG_TX_PRBS_SEL_CFG     VTSS_ENCODE_BITMASK(6,3)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_TX_CFG_TX_PRBS_SEL_CFG(x)  VTSS_EXTRACT_BITFIELD(x,6,3)

/** 
 * \brief
 * Inverts the scrambler output
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_TX_CFG . SCRAM_INV_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_TX_CFG_SCRAM_INV_CFG  VTSS_BIT(5)

/** 
 * \brief
 * Selects PRBS generator input
 *
 * \details 
 * 0:pat-gen
 * 1:core
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_TX_CFG . IPATH_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_TX_CFG_IPATH_CFG  VTSS_BIT(4)

/** 
 * \brief
 * Selects DFT-TX output
 *
 * \details 
 * 0:PRBS/scrambler (default)
 * 1:bypass

 *
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_TX_CFG . OPATH_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_TX_CFG_OPATH_CFG(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_TX_CFG_OPATH_CFG     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_TX_CFG_OPATH_CFG(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Word width of constant pattern generator
 *
 * \details 
 * 0:bytes mode; 1:10-bits word mode
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_TX_CFG . TX_WORD_MODE_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_TX_CFG_TX_WORD_MODE_CFG  VTSS_BIT(1)

/** 
 * \brief
 * Enable TX DFT capability
 *
 * \details 
 * 0: Disable DFT
 * 1: Enable DFT
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_TX_CFG . DFT_TX_ENA
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_TX_CFG_DFT_TX_ENA  VTSS_BIT(0)


/** 
 * \brief SD10G65 DFT TX Constant pattern configuration register 1
 *
 * \details
 * TX Constant MSB pattern configuration register 1 for SD10G65 DFT.
 *
 * Register: \a VENICE_DEV1:SD10G65_DFT:DFT_TX_PAT_CFG_1
 */
#define VTSS_VENICE_DEV1_SD10G65_DFT_DFT_TX_PAT_CFG_1  VTSS_IOREG(0x01, 0, 0xb115)

/** 
 * \brief
 * Constant patterns are valid to store
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_TX_PAT_CFG_1 . PAT_VLD_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_TX_PAT_CFG_1_PAT_VLD_CFG  VTSS_BIT(4)

/** 
 * \brief
 * Maximum address in generator (before continuing with address 0)
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_TX_PAT_CFG_1 . MAX_ADDR_GEN_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_TX_PAT_CFG_1_MAX_ADDR_GEN_CFG(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_TX_PAT_CFG_1_MAX_ADDR_GEN_CFG     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_TX_PAT_CFG_1_MAX_ADDR_GEN_CFG(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief SD10G65 DFT TX Constant pattern configuration register 2
 *
 * \details
 * TX Constant MSB pattern configuration register 2 for SD10G65 DFT.
 *
 * Register: \a VENICE_DEV1:SD10G65_DFT:DFT_TX_PAT_CFG_2
 */
#define VTSS_VENICE_DEV1_SD10G65_DFT_DFT_TX_PAT_CFG_2  VTSS_IOREG(0x01, 0, 0xb116)

/** 
 * \brief
 * Current storage address for patterns in generator
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_TX_PAT_CFG_2 . STORE_ADDR_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_TX_PAT_CFG_2_STORE_ADDR_CFG(x)  VTSS_ENCODE_BITFIELD(x,10,4)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_TX_PAT_CFG_2_STORE_ADDR_CFG     VTSS_ENCODE_BITMASK(10,4)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_TX_PAT_CFG_2_STORE_ADDR_CFG(x)  VTSS_EXTRACT_BITFIELD(x,10,4)

/** 
 * \brief
 * 10 bits word of constant patterns for transmission
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_TX_PAT_CFG_2 . PATTERN_CFG
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_TX_PAT_CFG_2_PATTERN_CFG(x)  VTSS_ENCODE_BITFIELD(x,0,10)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_TX_PAT_CFG_2_PATTERN_CFG     VTSS_ENCODE_BITMASK(0,10)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_TX_PAT_CFG_2_PATTERN_CFG(x)  VTSS_EXTRACT_BITFIELD(x,0,10)


/** 
 * \brief SD10G65 DFT TX constant pattern status register
 *
 * \details
 * Status register for SD10G65 DFT containing the constant patterns used
 * for comparison (last in LEARN mode)
 *
 * Register: \a VENICE_DEV1:SD10G65_DFT:DFT_TX_CMP_DAT_STAT
 */
#define VTSS_VENICE_DEV1_SD10G65_DFT_DFT_TX_CMP_DAT_STAT  VTSS_IOREG(0x01, 0, 0xb117)

/** 
 * \brief
 * Scrambler/PRBS generator output unchanged for at least 511 clock cycles
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_TX_CMP_DAT_STAT . TX_STUCK_AT_STICKY
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_TX_CMP_DAT_STAT_TX_STUCK_AT_STICKY  VTSS_BIT(12)

/** 
 * \brief
 * 10 bits data word at address 'store_addr_cfg' used for further
 * observation by SW
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_SD10G65_DFT_DFT_TX_CMP_DAT_STAT . PAT_STAT
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_DFT_DFT_TX_CMP_DAT_STAT_PAT_STAT(x)  VTSS_ENCODE_BITFIELD(x,0,10)
#define  VTSS_M_VENICE_DEV1_SD10G65_DFT_DFT_TX_CMP_DAT_STAT_PAT_STAT     VTSS_ENCODE_BITMASK(0,10)
#define  VTSS_X_VENICE_DEV1_SD10G65_DFT_DFT_TX_CMP_DAT_STAT_PAT_STAT(x)  VTSS_EXTRACT_BITFIELD(x,0,10)

/**
 * Register Group: \a VENICE_DEV1:ROMENG_1
 *
 * Register Collection for Rom Engine 1
 */


/** 
 * \brief SPI address field of ROM table entry ... replication_count=170
 *
 * \details
 * Register: \a VENICE_DEV1:ROMENG_1:spi_adr
 *
 * @param ri Register: spi_adr, 0-169
 */
#define VTSS_VENICE_DEV1_ROMENG_1_spi_adr(ri)  VTSS_IOREG(0x01, 0, 0xb200 | (ri))

/** 
 * \brief
 * SPI address to write
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_ROMENG_1_spi_adr . spi_adr
 */
#define  VTSS_F_VENICE_DEV1_ROMENG_1_spi_adr_spi_adr(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_VENICE_DEV1_ROMENG_1_spi_adr_spi_adr     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_VENICE_DEV1_ROMENG_1_spi_adr_spi_adr(x)  VTSS_EXTRACT_BITFIELD(x,0,7)


/** 
 * \brief Lower 16 bits of SPI data field of ROM table entry ... replication_count=170
 *
 * \details
 * Register: \a VENICE_DEV1:ROMENG_1:data_lsw
 *
 * @param ri Register: data_lsw, 0-169
 */
#define VTSS_VENICE_DEV1_ROMENG_1_data_lsw(ri)  VTSS_IOREG(0x01, 0, 0xb300 | (ri))

/** 
 * \brief
 * SPI data lsw
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_ROMENG_1_data_lsw . spi_dat_lsw
 */
#define  VTSS_F_VENICE_DEV1_ROMENG_1_data_lsw_spi_dat_lsw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_ROMENG_1_data_lsw_spi_dat_lsw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_ROMENG_1_data_lsw_spi_dat_lsw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Upper 16 bits of SPI data field of ROM table entry ... replication_count=170
 *
 * \details
 * Register: \a VENICE_DEV1:ROMENG_1:data_msw
 *
 * @param ri Register: data_msw, 0-169
 */
#define VTSS_VENICE_DEV1_ROMENG_1_data_msw(ri)  VTSS_IOREG(0x01, 0, 0xb400 | (ri))

/** 
 * \brief
 * SPI data msw
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_ROMENG_1_data_msw . spi_dat_msw
 */
#define  VTSS_F_VENICE_DEV1_ROMENG_1_data_msw_spi_dat_msw(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VENICE_DEV1_ROMENG_1_data_msw_spi_dat_msw     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VENICE_DEV1_ROMENG_1_data_msw_spi_dat_msw(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VENICE_DEV1:ROMENG_2
 *
 * Register Collection for Rom Engine 2
 */


/** 
 * \brief ROM table start/end addresses of Tx 10G setting routine
 *
 * \details
 * Register: \a VENICE_DEV1:ROMENG_2:adr_tx10g
 */
#define VTSS_VENICE_DEV1_ROMENG_2_adr_tx10g  VTSS_IOREG(0x01, 0, 0xb600)

/** 
 * \brief
 * Starting ROM address of Tx 10G routine
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_ROMENG_2_adr_tx10g . adr_tx10g_start
 */
#define  VTSS_F_VENICE_DEV1_ROMENG_2_adr_tx10g_adr_tx10g_start(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_VENICE_DEV1_ROMENG_2_adr_tx10g_adr_tx10g_start     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_VENICE_DEV1_ROMENG_2_adr_tx10g_adr_tx10g_start(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Ending   ROM address of Tx 10G routine
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_ROMENG_2_adr_tx10g . adr_tx10g_end
 */
#define  VTSS_F_VENICE_DEV1_ROMENG_2_adr_tx10g_adr_tx10g_end(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VENICE_DEV1_ROMENG_2_adr_tx10g_adr_tx10g_end     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VENICE_DEV1_ROMENG_2_adr_tx10g_adr_tx10g_end(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief ROM table start/end addresses of Rx 10G setting routine
 *
 * \details
 * Register: \a VENICE_DEV1:ROMENG_2:adr_rx10g
 */
#define VTSS_VENICE_DEV1_ROMENG_2_adr_rx10g  VTSS_IOREG(0x01, 0, 0xb601)

/** 
 * \brief
 * Starting ROM address of Rx 10G routine
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_ROMENG_2_adr_rx10g . adr_rx10g_start
 */
#define  VTSS_F_VENICE_DEV1_ROMENG_2_adr_rx10g_adr_rx10g_start(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_VENICE_DEV1_ROMENG_2_adr_rx10g_adr_rx10g_start     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_VENICE_DEV1_ROMENG_2_adr_rx10g_adr_rx10g_start(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Ending   ROM address of Rx 10G routine
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_ROMENG_2_adr_rx10g . adr_rx10g_end
 */
#define  VTSS_F_VENICE_DEV1_ROMENG_2_adr_rx10g_adr_rx10g_end(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VENICE_DEV1_ROMENG_2_adr_rx10g_adr_rx10g_end     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VENICE_DEV1_ROMENG_2_adr_rx10g_adr_rx10g_end(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief ROM table start/end addresses of Tx 1G setting routine
 *
 * \details
 * Register: \a VENICE_DEV1:ROMENG_2:adr_tx1g
 */
#define VTSS_VENICE_DEV1_ROMENG_2_adr_tx1g   VTSS_IOREG(0x01, 0, 0xb602)

/** 
 * \brief
 * Starting ROM address of Tx 1G routine
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_ROMENG_2_adr_tx1g . adr_tx1g_start
 */
#define  VTSS_F_VENICE_DEV1_ROMENG_2_adr_tx1g_adr_tx1g_start(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_VENICE_DEV1_ROMENG_2_adr_tx1g_adr_tx1g_start     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_VENICE_DEV1_ROMENG_2_adr_tx1g_adr_tx1g_start(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Ending   ROM address of Tx 1G routine
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_ROMENG_2_adr_tx1g . adr_tx1g_end
 */
#define  VTSS_F_VENICE_DEV1_ROMENG_2_adr_tx1g_adr_tx1g_end(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VENICE_DEV1_ROMENG_2_adr_tx1g_adr_tx1g_end     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VENICE_DEV1_ROMENG_2_adr_tx1g_adr_tx1g_end(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief ROM table start/end addresses of Rx 1G setting routine
 *
 * \details
 * Register: \a VENICE_DEV1:ROMENG_2:adr_rx1g
 */
#define VTSS_VENICE_DEV1_ROMENG_2_adr_rx1g   VTSS_IOREG(0x01, 0, 0xb603)

/** 
 * \brief
 * Starting ROM address of Rx 1G routine
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_ROMENG_2_adr_rx1g . adr_rx1g_start
 */
#define  VTSS_F_VENICE_DEV1_ROMENG_2_adr_rx1g_adr_rx1g_start(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_VENICE_DEV1_ROMENG_2_adr_rx1g_adr_rx1g_start     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_VENICE_DEV1_ROMENG_2_adr_rx1g_adr_rx1g_start(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Ending   ROM address of Rx 1G routine
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_ROMENG_2_adr_rx1g . adr_rx1g_end
 */
#define  VTSS_F_VENICE_DEV1_ROMENG_2_adr_rx1g_adr_rx1g_end(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VENICE_DEV1_ROMENG_2_adr_rx1g_adr_rx1g_end     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VENICE_DEV1_ROMENG_2_adr_rx1g_adr_rx1g_end(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief ROM table start/end addresses of WAN setting routine
 *
 * \details
 * Register: \a VENICE_DEV1:ROMENG_2:adr_wan
 */
#define VTSS_VENICE_DEV1_ROMENG_2_adr_wan    VTSS_IOREG(0x01, 0, 0xb604)

/** 
 * \brief
 * Starting ROM address of WAN routine
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_ROMENG_2_adr_wan . adr_wan_start
 */
#define  VTSS_F_VENICE_DEV1_ROMENG_2_adr_wan_adr_wan_start(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_VENICE_DEV1_ROMENG_2_adr_wan_adr_wan_start     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_VENICE_DEV1_ROMENG_2_adr_wan_adr_wan_start(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Ending   ROM address of WAN routine
 *
 * \details 
 * Field: VTSS_VENICE_DEV1_ROMENG_2_adr_wan . adr_wan_end
 */
#define  VTSS_F_VENICE_DEV1_ROMENG_2_adr_wan_adr_wan_end(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VENICE_DEV1_ROMENG_2_adr_wan_adr_wan_end     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VENICE_DEV1_ROMENG_2_adr_wan_adr_wan_end(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a VENICE_DEV1:ROMENG_STATUS
 *
 * Rom Engine Status
 */


/** 
 * \brief Rom Engine Status
 *
 * \details
 * Rom Engine Status
 *
 * Register: \a VENICE_DEV1:ROMENG_STATUS:ROMENG_STATUS
 */
#define VTSS_VENICE_DEV1_ROMENG_STATUS_ROMENG_STATUS  VTSS_IOREG(0x01, 0, 0xb6ff)

/** 
 * \brief
 * Rom Engine last routine executed

 *
 * \details 
 * 00000: 10G	- configured for 10G mode
 * 00001: TX10G - Tx configured for 10G mode
 * 00010: RX10G - Rx configured for 10G mode
 * 00011: 1G	- configured for 1G mode
 * 00100: TX1G	- Rx configured for 1G mode
 * 00101: RX1G	- Rx configured for 1G mode
 * 00110: 3G	- configured for 3G mode
 * 00111: TX3G	- Rx configured for 3G mode
 * 01000: RX3G	- Rx configured for 3G mode
 * 01001: WAN	- configured for WAN mode
 * 01010: RST	- configured to Reset condition
 * 01011: LBON	- configured for Loopback enabled
 * 01100: LBOFF - configured for Loopback disabled
 * 01101: LPON	- LowPower mode enabled
 * 01110: LPOFF - LowPower mode disabled
 * 01111: RC	- RCOMP routine
 * 10000: LRON	- Lock2Ref enabled
 * 10001: LROFF - Lock2Ref disabled
 * others: invalid
 *
 * Field: VTSS_VENICE_DEV1_ROMENG_STATUS_ROMENG_STATUS . exe_last
 */
#define  VTSS_F_VENICE_DEV1_ROMENG_STATUS_ROMENG_STATUS_exe_last(x)  VTSS_ENCODE_BITFIELD(x,1,5)
#define  VTSS_M_VENICE_DEV1_ROMENG_STATUS_ROMENG_STATUS_exe_last     VTSS_ENCODE_BITMASK(1,5)
#define  VTSS_X_VENICE_DEV1_ROMENG_STATUS_ROMENG_STATUS_exe_last(x)  VTSS_EXTRACT_BITFIELD(x,1,5)

/** 
 * \brief
 * Rom Engine status
 * This is a sticky bit that latches the high state.  The latch-high bit is
 * cleared when the register is read.

 *
 * \details 
 * 0: Rom Engine has not executed a new routine since the last time this
 * bit is read
 * 1: Rom Engine has executed a new routine since the last time this bit is
 * read
 *
 * Field: VTSS_VENICE_DEV1_ROMENG_STATUS_ROMENG_STATUS . exe_done
 */
#define  VTSS_F_VENICE_DEV1_ROMENG_STATUS_ROMENG_STATUS_exe_done  VTSS_BIT(0)

/**
 * Register Group: \a VENICE_DEV1:SD10G65_SYNC_CTRL
 *
 * SYNC_CTRL Configuration and Status Register set
 */


/** 
 * \brief SYNC_CTRL config register
 *
 * \details
 * Sync control configuration register
 *
 * Register: \a VENICE_DEV1:SD10G65_SYNC_CTRL:SYNC_CTRL_CFG
 */
#define VTSS_VENICE_DEV1_SD10G65_SYNC_CTRL_SYNC_CTRL_CFG  VTSS_IOREG(0x01, 0, 0xb700)

/** 
 * \brief
 * Clear sync ctrl status register
 *
 * \details 
 * 0: Idle
 * 1: Clear
 *
 * Field: VTSS_VENICE_DEV1_SD10G65_SYNC_CTRL_SYNC_CTRL_CFG . CLR_SYNC_STAT
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_SYNC_CTRL_SYNC_CTRL_CFG_CLR_SYNC_STAT  VTSS_BIT(4)

/** 
 * \brief
 * Source selection for lane synchronization
 *
 * \details 
 * 0: Select DES_0
 * 1: Select DES_1
 * 2: Select F to Delta F
 * 3: Synchronization Disabled

 *
 * Field: VTSS_VENICE_DEV1_SD10G65_SYNC_CTRL_SYNC_CTRL_CFG . LANE_SYNC_SRC
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_SYNC_CTRL_SYNC_CTRL_CFG_LANE_SYNC_SRC(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_VENICE_DEV1_SD10G65_SYNC_CTRL_SYNC_CTRL_CFG_LANE_SYNC_SRC     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_VENICE_DEV1_SD10G65_SYNC_CTRL_SYNC_CTRL_CFG_LANE_SYNC_SRC(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief SYNC_CTRL status register
 *
 * \details
 * Sync control status register
 *
 * Register: \a VENICE_DEV1:SD10G65_SYNC_CTRL:SYNC_CTRL_STAT
 */
#define VTSS_VENICE_DEV1_SD10G65_SYNC_CTRL_SYNC_CTRL_STAT  VTSS_IOREG(0x01, 0, 0xb701)

/** 
 * \brief
 * Lane synchronization fifo overflow
 *
 * \details 
 * 0: FIFO normal
 * 1: FIFO overflow

 *
 * Field: VTSS_VENICE_DEV1_SD10G65_SYNC_CTRL_SYNC_CTRL_STAT . LANE_SYNC_FIFO_OF
 */
#define  VTSS_F_VENICE_DEV1_SD10G65_SYNC_CTRL_SYNC_CTRL_STAT_LANE_SYNC_FIFO_OF  VTSS_BIT(0)

/**
 * Register Group: \a VENICE_DEV1:SD10G65_RX_RCPLL_BIST
 *
 * SD10G65 RX RCPLL BIST Configuration and Status Register set
 */


/** 
 * \brief SD10G65 RX RCPLL BIST Configuration register 0A
 *
 * \details
 * Configuration register 0A for SD10G65 RX RCPLL BIST.
 *
 * Register: \a VENICE_DEV1:SD10G65_RX_RCPLL_BIST:SD10G65_RX_RCPLL_BIST_CFG0A
 */
#define VTSS_VENICE_DEV1_SD10G65_RX_RCPLL_BIST_SD10G65_RX_RCPLL_BIST_CFG0A  VTSS_IOREG(0x01, 0, 0xb800)


/** 
 * \brief SD10G65 RX RCPLL BIST Configuration register 0B
 *
 * \details
 * Configuration register 0B for SD10G65 RX RCPLL BIST.
 *
 * Register: \a VENICE_DEV1:SD10G65_RX_RCPLL_BIST:SD10G65_RX_RCPLL_BIST_CFG0B
 */
#define VTSS_VENICE_DEV1_SD10G65_RX_RCPLL_BIST_SD10G65_RX_RCPLL_BIST_CFG0B  VTSS_IOREG(0x01, 0, 0xb801)


/** 
 * \brief SD10G65 RX RCPLL BIST Configuration register 1A
 *
 * \details
 * Configuration register 1A for SD10G65 RX RCPLL BIST.
 *
 * Register: \a VENICE_DEV1:SD10G65_RX_RCPLL_BIST:SD10G65_RX_RCPLL_BIST_CFG1A
 */
#define VTSS_VENICE_DEV1_SD10G65_RX_RCPLL_BIST_SD10G65_RX_RCPLL_BIST_CFG1A  VTSS_IOREG(0x01, 0, 0xb802)


/** 
 * \brief SD10G65 RX RCPLL BIST Configuration register 1B
 *
 * \details
 * Configuration register 1B for SD10G65 RX RCPLL BIST.
 *
 * Register: \a VENICE_DEV1:SD10G65_RX_RCPLL_BIST:SD10G65_RX_RCPLL_BIST_CFG1B
 */
#define VTSS_VENICE_DEV1_SD10G65_RX_RCPLL_BIST_SD10G65_RX_RCPLL_BIST_CFG1B  VTSS_IOREG(0x01, 0, 0xb803)


/** 
 * \brief SD10G65 RX RCPLL BIST Configuration register 2
 *
 * \details
 * Configuration register 2 for SD10G65 RX RCPLL BIST.
 *
 * Register: \a VENICE_DEV1:SD10G65_RX_RCPLL_BIST:SD10G65_RX_RCPLL_BIST_CFG2
 */
#define VTSS_VENICE_DEV1_SD10G65_RX_RCPLL_BIST_SD10G65_RX_RCPLL_BIST_CFG2  VTSS_IOREG(0x01, 0, 0xb804)


/** 
 * \brief SD10G65 RX RCPLL BIST Configuration register 3
 *
 * \details
 * Configuration register 3 for SD10G65 RX RCPLL BIST.
 *
 * Register: \a VENICE_DEV1:SD10G65_RX_RCPLL_BIST:SD10G65_RX_RCPLL_BIST_CFG3
 */
#define VTSS_VENICE_DEV1_SD10G65_RX_RCPLL_BIST_SD10G65_RX_RCPLL_BIST_CFG3  VTSS_IOREG(0x01, 0, 0xb805)


/** 
 * \brief SD10G65 RX RCPLL BIST Configuration register 4
 *
 * \details
 * Configuration register 4 for SD10G65 RX RCPLL BIST.
 *
 * Register: \a VENICE_DEV1:SD10G65_RX_RCPLL_BIST:SD10G65_RX_RCPLL_BIST_CFG4
 */
#define VTSS_VENICE_DEV1_SD10G65_RX_RCPLL_BIST_SD10G65_RX_RCPLL_BIST_CFG4  VTSS_IOREG(0x01, 0, 0xb806)


/** 
 * \brief SD10G65 RX RCPLL BIST Status register 0A
 *
 * \details
 * Status register 0A for SD10G65 RX RCPLL BIST.
 *
 * Register: \a VENICE_DEV1:SD10G65_RX_RCPLL_BIST:SD10G65_RX_RCPLL_BIST_STAT0A
 */
#define VTSS_VENICE_DEV1_SD10G65_RX_RCPLL_BIST_SD10G65_RX_RCPLL_BIST_STAT0A  VTSS_IOREG(0x01, 0, 0xb807)


/** 
 * \brief SD10G65 RX RCPLL BIST Status register 0B
 *
 * \details
 * Status register 0B for SD10G65 RX RCPLL BIST.
 *
 * Register: \a VENICE_DEV1:SD10G65_RX_RCPLL_BIST:SD10G65_RX_RCPLL_BIST_STAT0B
 */
#define VTSS_VENICE_DEV1_SD10G65_RX_RCPLL_BIST_SD10G65_RX_RCPLL_BIST_STAT0B  VTSS_IOREG(0x01, 0, 0xb808)


/** 
 * \brief SD10G65 RX RCPLL BIST Status register 1
 *
 * \details
 * Status register 1 for SD10G65 RX RCPLL BIST.
 *
 * Register: \a VENICE_DEV1:SD10G65_RX_RCPLL_BIST:SD10G65_RX_RCPLL_BIST_STAT1
 */
#define VTSS_VENICE_DEV1_SD10G65_RX_RCPLL_BIST_SD10G65_RX_RCPLL_BIST_STAT1  VTSS_IOREG(0x01, 0, 0xb809)

/**
 * Register Group: \a VENICE_DEV1:SD10G65_TX_RCPLL_BIST
 *
 * SD10G65 TX RCPLL BIST Configuration and Status Register set
 */


/** 
 * \brief SD10G65 TX RCPLL BIST Configuration register 0A
 *
 * \details
 * Configuration register 0A for SD10G65 TX RCPLL BIST.
 *
 * Register: \a VENICE_DEV1:SD10G65_TX_RCPLL_BIST:SD10G65_TX_RCPLL_BIST_CFG0A
 */
#define VTSS_VENICE_DEV1_SD10G65_TX_RCPLL_BIST_SD10G65_TX_RCPLL_BIST_CFG0A  VTSS_IOREG(0x01, 0, 0xb810)


/** 
 * \brief SD10G65 TX RCPLL BIST Configuration register 0B
 *
 * \details
 * Configuration register 0B for SD10G65 TX RCPLL BIST.
 *
 * Register: \a VENICE_DEV1:SD10G65_TX_RCPLL_BIST:SD10G65_TX_RCPLL_BIST_CFG0B
 */
#define VTSS_VENICE_DEV1_SD10G65_TX_RCPLL_BIST_SD10G65_TX_RCPLL_BIST_CFG0B  VTSS_IOREG(0x01, 0, 0xb811)


/** 
 * \brief SD10G65 TX RCPLL BIST Configuration register 1A
 *
 * \details
 * Configuration register 1A for SD10G65 TX RCPLL BIST.
 *
 * Register: \a VENICE_DEV1:SD10G65_TX_RCPLL_BIST:SD10G65_TX_RCPLL_BIST_CFG1A
 */
#define VTSS_VENICE_DEV1_SD10G65_TX_RCPLL_BIST_SD10G65_TX_RCPLL_BIST_CFG1A  VTSS_IOREG(0x01, 0, 0xb812)


/** 
 * \brief SD10G65 TX RCPLL BIST Configuration register 1B
 *
 * \details
 * Configuration register 1B for SD10G65 TX RCPLL BIST.
 *
 * Register: \a VENICE_DEV1:SD10G65_TX_RCPLL_BIST:SD10G65_TX_RCPLL_BIST_CFG1B
 */
#define VTSS_VENICE_DEV1_SD10G65_TX_RCPLL_BIST_SD10G65_TX_RCPLL_BIST_CFG1B  VTSS_IOREG(0x01, 0, 0xb813)


/** 
 * \brief SD10G65 TX RCPLL BIST Configuration register 2
 *
 * \details
 * Configuration register 2 for SD10G65 TX RCPLL BIST.
 *
 * Register: \a VENICE_DEV1:SD10G65_TX_RCPLL_BIST:SD10G65_TX_RCPLL_BIST_CFG2
 */
#define VTSS_VENICE_DEV1_SD10G65_TX_RCPLL_BIST_SD10G65_TX_RCPLL_BIST_CFG2  VTSS_IOREG(0x01, 0, 0xb814)


/** 
 * \brief SD10G65 TX RCPLL BIST Configuration register 3
 *
 * \details
 * Configuration register 3 for SD10G65 TX RCPLL BIST.
 *
 * Register: \a VENICE_DEV1:SD10G65_TX_RCPLL_BIST:SD10G65_TX_RCPLL_BIST_CFG3
 */
#define VTSS_VENICE_DEV1_SD10G65_TX_RCPLL_BIST_SD10G65_TX_RCPLL_BIST_CFG3  VTSS_IOREG(0x01, 0, 0xb815)


/** 
 * \brief SD10G65 TX RCPLL BIST Configuration register 4
 *
 * \details
 * Configuration register 4 for SD10G65 TX RCPLL BIST.
 *
 * Register: \a VENICE_DEV1:SD10G65_TX_RCPLL_BIST:SD10G65_TX_RCPLL_BIST_CFG4
 */
#define VTSS_VENICE_DEV1_SD10G65_TX_RCPLL_BIST_SD10G65_TX_RCPLL_BIST_CFG4  VTSS_IOREG(0x01, 0, 0xb816)


/** 
 * \brief SD10G65 TX RCPLL BIST Status register 0A
 *
 * \details
 * Status register 0A for SD10G65 TX RCPLL BIST.
 *
 * Register: \a VENICE_DEV1:SD10G65_TX_RCPLL_BIST:SD10G65_TX_RCPLL_BIST_STAT0A
 */
#define VTSS_VENICE_DEV1_SD10G65_TX_RCPLL_BIST_SD10G65_TX_RCPLL_BIST_STAT0A  VTSS_IOREG(0x01, 0, 0xb817)


/** 
 * \brief SD10G65 TX RCPLL BIST Status register 0B
 *
 * \details
 * Status register 0B for SD10G65 TX RCPLL BIST.
 *
 * Register: \a VENICE_DEV1:SD10G65_TX_RCPLL_BIST:SD10G65_TX_RCPLL_BIST_STAT0B
 */
#define VTSS_VENICE_DEV1_SD10G65_TX_RCPLL_BIST_SD10G65_TX_RCPLL_BIST_STAT0B  VTSS_IOREG(0x01, 0, 0xb818)


/** 
 * \brief SD10G65 TX RCPLL BIST Status register 1
 *
 * \details
 * Status register 1 for SD10G65 TX RCPLL BIST.
 *
 * Register: \a VENICE_DEV1:SD10G65_TX_RCPLL_BIST:SD10G65_TX_RCPLL_BIST_STAT1
 */
#define VTSS_VENICE_DEV1_SD10G65_TX_RCPLL_BIST_SD10G65_TX_RCPLL_BIST_STAT1  VTSS_IOREG(0x01, 0, 0xb819)


#endif /* _VTSS_VENICE_REGS_VENICE_DEV1_H_ */
