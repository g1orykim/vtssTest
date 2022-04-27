#ifndef _VTSS_JAGUAR_REGS_DEVNPI_H_
#define _VTSS_JAGUAR_REGS_DEVNPI_H_

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

#include "vtss_jaguar_regs_common.h"

/*********************************************************************** 
 *
 * Target: \a DEVNPI
 *
 * The packet interface towards an external node processor. The device
 * operates as an interface between the TAXI bus and SGMII macro supporting
 * data rates of 10M, 100M, and 1000Mbps. The device includes PCS, MAC and
 * rate adaption logic.
 * Note: A DEVNPI includes the same MAC and PCS submodules as DEV1G I.e.
 * MAC1G and PCS1G.
 *
 ***********************************************************************/

/**
 * Register Group: \a DEVNPI:DEV_CFG_STATUS
 *
 * DEVNPI Configuration and Status Registers

 */


/** 
 * \brief DEV_RST_CTRL register
 *
 * \details
 * Clock/Reset Control Register
 *
 * Register: \a DEVNPI:DEV_CFG_STATUS:DEV_RST_CTRL
 */
#define VTSS_DEVNPI_DEV_CFG_STATUS_DEV_RST_CTRL  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x0)

/** 
 * \brief
 * This register selects the port data rate. The no clock options is
 * intended to save power for unused ports.
 *
 * \details 
 * 0: 10 Mbps
 * 1: 100 Mbps
 * 2: 1000 Mbps
 * 3: No clock.
 *
 * Field: ::VTSS_DEVNPI_DEV_CFG_STATUS_DEV_RST_CTRL . SPEED_SEL
 */
#define  VTSS_F_DEVNPI_DEV_CFG_STATUS_DEV_RST_CTRL_SPEED_SEL(x)  VTSS_ENCODE_BITFIELD(x,20,2)
#define  VTSS_M_DEVNPI_DEV_CFG_STATUS_DEV_RST_CTRL_SPEED_SEL     VTSS_ENCODE_BITMASK(20,2)
#define  VTSS_X_DEVNPI_DEV_CFG_STATUS_DEV_RST_CTRL_SPEED_SEL(x)  VTSS_EXTRACT_BITFIELD(x,20,2)

/** 
 * \brief
 * Reset the PCS tx clock domain in the DEVNPI. The setup registers in the
 * DEVNPI is not affected by this reset.

 *
 * \details 
 * 0: No reset
 * 1: Reset. Note: PCS_TX_RST is NOT a one-shot operation. The clock domain
 * remains reset until a '0' is written to PCS_TX_RST.
 *
 * Field: ::VTSS_DEVNPI_DEV_CFG_STATUS_DEV_RST_CTRL . PCS_TX_RST
 */
#define  VTSS_F_DEVNPI_DEV_CFG_STATUS_DEV_RST_CTRL_PCS_TX_RST  VTSS_BIT(12)

/** 
 * \brief
 * Reset the PCS rx clock domain in the DEVNPI. The setup registers in the
 * DEVNPI is not affected by this reset.

 *
 * \details 
 * 0: No reset
 * 1: Reset. Note: PCS_RX_RST is NOT a one-shot operation. The clock domain
 * remains reset until a '0' is written to PCS_RX_RST.
 *
 * Field: ::VTSS_DEVNPI_DEV_CFG_STATUS_DEV_RST_CTRL . PCS_RX_RST
 */
#define  VTSS_F_DEVNPI_DEV_CFG_STATUS_DEV_RST_CTRL_PCS_RX_RST  VTSS_BIT(8)

/** 
 * \brief
 * Reset the MAC tx clock domain in the DEVNPI. The setup registers in the
 * DEVNPI is not affected by this reset.

 *
 * \details 
 * 0: No reset
 * 1: Reset. Note: MAC_TX_RST is NOT a one-shot operation. The clock domain
 * remains reset until a '0' is written to MAC_TX_RST.
 *
 * Field: ::VTSS_DEVNPI_DEV_CFG_STATUS_DEV_RST_CTRL . MAC_TX_RST
 */
#define  VTSS_F_DEVNPI_DEV_CFG_STATUS_DEV_RST_CTRL_MAC_TX_RST  VTSS_BIT(4)

/** 
 * \brief
 * Reset the MAC rx clock domain in the DEVNPI. The setup registers in the
 * DEVNPI is not affected by this reset.

 *
 * \details 
 * 0: No reset
 * 1: Reset. Note: MAC_RX_RST is NOT a one-shot operation. The clock domain
 * remains reset until a '0' is written to MAC_RX_RST.
 *
 * Field: ::VTSS_DEVNPI_DEV_CFG_STATUS_DEV_RST_CTRL . MAC_RX_RST
 */
#define  VTSS_F_DEVNPI_DEV_CFG_STATUS_DEV_RST_CTRL_MAC_RX_RST  VTSS_BIT(0)


/** 
 * \brief Loopback Configuration Register
 *
 * \details
 * A number of internal loopback can be enabled in each device by the
 * configuration bits in this register.
 *
 * Register: \a DEVNPI:DEV_CFG_STATUS:DEV_LB_CFG
 */
#define VTSS_DEVNPI_DEV_CFG_STATUS_DEV_LB_CFG  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x2)

/** 
 * \brief
 * This cfg bit is deprecated. If test mode is disabled and a Host Loopback
 * is enabled, the internal Rx clock can be inverted.
 *
 * \details 
 * 0: Clock inversion disabled
 * 1: Clock inversion enabled
 *
 * Field: ::VTSS_DEVNPI_DEV_CFG_STATUS_DEV_LB_CFG . INV_RX_CLOCK
 */
#define  VTSS_F_DEVNPI_DEV_CFG_STATUS_DEV_LB_CFG_INV_RX_CLOCK  VTSS_BIT(12)

/** 
 * \brief
 * Loop back external data from ingress to egress on the Taxi bus.
 *
 * \details 
 * 0: External taxi loop back disabled
 * 1: External taxi loop back enabled
 *
 * Field: ::VTSS_DEVNPI_DEV_CFG_STATUS_DEV_LB_CFG . TAXI_PHY_LB_ENA
 */
#define  VTSS_F_DEVNPI_DEV_CFG_STATUS_DEV_LB_CFG_TAXI_PHY_LB_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Loop back internal data from the egress Taxi bus to the ingress Taxi
 * bus.
 *
 * \details 
 * 0: Loopback from Taxi egress to Taxi ingress is disabled
 * 1: Loopback from Taxi egress to Taxi ingress is enabled
 *
 * Field: ::VTSS_DEVNPI_DEV_CFG_STATUS_DEV_LB_CFG . TAXI_HOST_LB_ENA
 */
#define  VTSS_F_DEVNPI_DEV_CFG_STATUS_DEV_LB_CFG_TAXI_HOST_LB_ENA  VTSS_BIT(0)


/** 
 * \brief Device Debug Config
 *
 * \details
 * Device NPI Debug Configuration Register

 *
 * Register: \a DEVNPI:DEV_CFG_STATUS:DEV_DBG_CFG
 */
#define VTSS_DEVNPI_DEV_CFG_STATUS_DEV_DBG_CFG  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x3)

/** 
 * \brief
 * Holds the ID of the last pre counter that had an overflow. The user has
 * to check the PRE_CNT_OFLW_STICKY up front. See the IS0076 for a mapping
 * of the counter ID to the counter type.
 *
 * \details 
 * 0: Pre counter 0 had an overflow (if sticky bit is set)
 * 1: Pre counter 1 had an overflow
 * ...
 *
 * Field: ::VTSS_DEVNPI_DEV_CFG_STATUS_DEV_DBG_CFG . PRE_CNT_OFLW_ID
 */
#define  VTSS_F_DEVNPI_DEV_CFG_STATUS_DEV_DBG_CFG_PRE_CNT_OFLW_ID(x)  VTSS_ENCODE_BITFIELD(x,24,6)
#define  VTSS_M_DEVNPI_DEV_CFG_STATUS_DEV_DBG_CFG_PRE_CNT_OFLW_ID     VTSS_ENCODE_BITMASK(24,6)
#define  VTSS_X_DEVNPI_DEV_CFG_STATUS_DEV_DBG_CFG_PRE_CNT_OFLW_ID(x)  VTSS_EXTRACT_BITFIELD(x,24,6)

/** 
 * \brief
 * Clears TX_MAX_FILL_LVL that holds the maximal reached TX FiFo fill
 * level. This bit is a one shot bit that is clear automatically by
 * hardware.
 *
 * \details 
 * 0: No action
 * 1: Clear TX_MAX_FILL_LVL (Bit is automatically cleared)
 *
 * Field: ::VTSS_DEVNPI_DEV_CFG_STATUS_DEV_DBG_CFG . TX_MAX_FILL_LVL_CLR_ONE_SHOT
 */
#define  VTSS_F_DEVNPI_DEV_CFG_STATUS_DEV_DBG_CFG_TX_MAX_FILL_LVL_CLR_ONE_SHOT  VTSS_BIT(23)

/** 
 * \brief
 * Stores the maximum reach TX FiFo fill level.
 *
 * \details 
 * 0: Max fill level was zero
 * 1: Max fill level was one
 * ...
 *
 * Field: ::VTSS_DEVNPI_DEV_CFG_STATUS_DEV_DBG_CFG . TX_MAX_FILL_LVL
 */
#define  VTSS_F_DEVNPI_DEV_CFG_STATUS_DEV_DBG_CFG_TX_MAX_FILL_LVL(x)  VTSS_ENCODE_BITFIELD(x,16,5)
#define  VTSS_M_DEVNPI_DEV_CFG_STATUS_DEV_DBG_CFG_TX_MAX_FILL_LVL     VTSS_ENCODE_BITMASK(16,5)
#define  VTSS_X_DEVNPI_DEV_CFG_STATUS_DEV_DBG_CFG_TX_MAX_FILL_LVL(x)  VTSS_EXTRACT_BITFIELD(x,16,5)

/** 
 * \brief
 * Determines the required fill level that must be EXCEEDED before the
 * buffer full signal to the DSM is asserted.
 *
 * \details 
 * 0: The 'buffer full' signal is asserted as soon as the Tx FIFO holds any
 * data
 * 1: The 'buffer full' signal is asserted if the Tx FIFO holds more than 1
 * data word
 * 2: The 'buffer full' signal is asserted if the Tx FIFO holds more than 2
 * data word
 * ...
 * 31: The 'buffer full' signal is disabled
 *
 * Field: ::VTSS_DEVNPI_DEV_CFG_STATUS_DEV_DBG_CFG . TX_BUF_HIGH_WM
 */
#define  VTSS_F_DEVNPI_DEV_CFG_STATUS_DEV_DBG_CFG_TX_BUF_HIGH_WM(x)  VTSS_ENCODE_BITFIELD(x,8,5)
#define  VTSS_M_DEVNPI_DEV_CFG_STATUS_DEV_DBG_CFG_TX_BUF_HIGH_WM     VTSS_ENCODE_BITMASK(8,5)
#define  VTSS_X_DEVNPI_DEV_CFG_STATUS_DEV_DBG_CFG_TX_BUF_HIGH_WM(x)  VTSS_EXTRACT_BITFIELD(x,8,5)

/** 
 * \brief
 * Controls the FCS update function of the device.
 *
 * \details 
 * 0: Use information from DSM to control FCS updating
 * 1: Always update FCS
 * 2: Never update FCS
 * 3: Reserved
 *
 * Field: ::VTSS_DEVNPI_DEV_CFG_STATUS_DEV_DBG_CFG . FCS_UPDATE_CFG
 */
#define  VTSS_F_DEVNPI_DEV_CFG_STATUS_DEV_DBG_CFG_FCS_UPDATE_CFG(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_DEVNPI_DEV_CFG_STATUS_DEV_DBG_CFG_FCS_UPDATE_CFG     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_DEVNPI_DEV_CFG_STATUS_DEV_DBG_CFG_FCS_UPDATE_CFG(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Disable of advanced rate control feature.
 *
 * \details 
 * 0: Advanced rate control active
 * 1: Advanced rate control disabled
 *
 * Field: ::VTSS_DEVNPI_DEV_CFG_STATUS_DEV_DBG_CFG . IFG_LEN_DIS
 */
#define  VTSS_F_DEVNPI_DEV_CFG_STATUS_DEV_DBG_CFG_IFG_LEN_DIS  VTSS_BIT(1)

/** 
 * \brief
 * The TX_Size and the TX_Backoff events share the same counters. Per
 * default the counters count TX_SIZE events. BACKOFF_CNT_ENA switches the
 * counter to count backup events instead.
 *
 * \details 
 * 0: Count TX_SIZE events
 * 1: Count TX_BACKOFF events
 *
 * Field: ::VTSS_DEVNPI_DEV_CFG_STATUS_DEV_DBG_CFG . BACKOFF_CNT_ENA
 */
#define  VTSS_F_DEVNPI_DEV_CFG_STATUS_DEV_DBG_CFG_BACKOFF_CNT_ENA  VTSS_BIT(0)


/** 
 * \brief Configuration for the port protectio feature
 *
 * \details
 * This register is used to configure the port protection feature of the
 * device.
 *
 * Register: \a DEVNPI:DEV_CFG_STATUS:DEV_PORT_PROTECT
 */
#define VTSS_DEVNPI_DEV_CFG_STATUS_DEV_PORT_PROTECT  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x4)

/** 
 * \brief
 * Indicates from which port egress data must be copied and transmitted at
 * this Ethernet port. The port from which egress data is copied must
 * always be a port that is closer to the ASM. I.e. DEV_X may be configured
 * to snoop egress data destined for DEV_X+n, where DEV_X+n is closer to
 * the ASM - but NOT vice versa.
 *
 * \details 
 * 0: Reserved
 * 1: Egress data destined for DEV_1 is also transmitted by this device
 * 2: Egress data destined for DEV_2 is also transmitted by this device
 * ...
 *
 * Field: ::VTSS_DEVNPI_DEV_CFG_STATUS_DEV_PORT_PROTECT . PORT_PROTECT_ID
 */
#define  VTSS_F_DEVNPI_DEV_CFG_STATUS_DEV_PORT_PROTECT_PORT_PROTECT_ID(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_DEVNPI_DEV_CFG_STATUS_DEV_PORT_PROTECT_PORT_PROTECT_ID     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_DEVNPI_DEV_CFG_STATUS_DEV_PORT_PROTECT_PORT_PROTECT_ID(x)  VTSS_EXTRACT_BITFIELD(x,4,4)

/** 
 * \brief
 * Enables snooping of egress data from another port. The port from which
 * egress data is copied and transmitted at the Ethernet port is determined
 * by the PORT_PROTECT_ID configuration.
 *
 * \details 
 * 0: Port protection is disabled
 * 1: Port protection is enabled
 *
 * Field: ::VTSS_DEVNPI_DEV_CFG_STATUS_DEV_PORT_PROTECT . PORT_PROTECT_ENA
 */
#define  VTSS_F_DEVNPI_DEV_CFG_STATUS_DEV_PORT_PROTECT_PORT_PROTECT_ENA  VTSS_BIT(0)


/** 
 * \brief Precision Time Protocol Configuration
 *
 * \details
 * Configuration of Support for Precision Time Protocol
 *
 * Register: \a DEVNPI:DEV_CFG_STATUS:DEV_PTP_CFG
 */
#define VTSS_DEVNPI_DEV_CFG_STATUS_DEV_PTP_CFG  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x5)

/** 
 * \brief
 * Enable PTP support according IEEE 1588.
 *
 * \details 
 * 0: PTP support disabled
 * 1: PTP support enabled
 *
 * Field: ::VTSS_DEVNPI_DEV_CFG_STATUS_DEV_PTP_CFG . PTP_ENA
 */
#define  VTSS_F_DEVNPI_DEV_CFG_STATUS_DEV_PTP_CFG_PTP_ENA  VTSS_BIT(4)


/** 
 * \brief PTP Time Stamp Sticky Register
 *
 * \details
 * Sticky bits are set when ever a PTP TX register is updated
 *
 * Register: \a DEVNPI:DEV_CFG_STATUS:DEV_PTP_TX_STICKY
 */
#define VTSS_DEVNPI_DEV_CFG_STATUS_DEV_PTP_TX_STICKY  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x6)

/** 
 * \brief
 * Indicates if a new time stamp was written to related register.
 *
 * \details 
 * 0: Time stamp register unchanged
 * 1: Time stamp register updated with time stamp
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_DEVNPI_DEV_CFG_STATUS_DEV_PTP_TX_STICKY . TX_PTP_TSTAMP_STICKY
 */
#define  VTSS_F_DEVNPI_DEV_CFG_STATUS_DEV_PTP_TX_STICKY_TX_PTP_TSTAMP_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_DEVNPI_DEV_CFG_STATUS_DEV_PTP_TX_STICKY_TX_PTP_TSTAMP_STICKY     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_DEVNPI_DEV_CFG_STATUS_DEV_PTP_TX_STICKY_TX_PTP_TSTAMP_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief PTP Time Stamp Register
 *
 * \details
 * Precision Time Protocol - Time Stamp Register
 *
 * Register: \a DEVNPI:DEV_CFG_STATUS:DEV_PTP_TX_TSTAMP
 *
 * @param ri Register: DEV_PTP_TX_TSTAMP (??), 0-2
 */
#define VTSS_DEVNPI_DEV_CFG_STATUS_DEV_PTP_TX_TSTAMP(ri)  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x7 + (ri))

/** 
 * \brief
 * 30 bit unsigned time stamp value (unit: ns)
 *
 * \details 
 * Field: ::VTSS_DEVNPI_DEV_CFG_STATUS_DEV_PTP_TX_TSTAMP . TX_TSTAMP
 */
#define  VTSS_F_DEVNPI_DEV_CFG_STATUS_DEV_PTP_TX_TSTAMP_TX_TSTAMP(x)  VTSS_ENCODE_BITFIELD(x,0,30)
#define  VTSS_M_DEVNPI_DEV_CFG_STATUS_DEV_PTP_TX_TSTAMP_TX_TSTAMP     VTSS_ENCODE_BITMASK(0,30)
#define  VTSS_X_DEVNPI_DEV_CFG_STATUS_DEV_PTP_TX_TSTAMP_TX_TSTAMP(x)  VTSS_EXTRACT_BITFIELD(x,0,30)


/** 
 * \brief PTP Identification Register
 *
 * \details
 * Precision Time Protocol - Frame Identification Register
 *
 * Register: \a DEVNPI:DEV_CFG_STATUS:DEV_PTP_TX_ID
 *
 * @param ri Register: DEV_PTP_TX_ID (??), 0-2
 */
#define VTSS_DEVNPI_DEV_CFG_STATUS_DEV_PTP_TX_ID(ri)  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0xa + (ri))

/**
 * Register Group: \a DEVNPI:MAC_CFG_STATUS
 *
 * MAC1G Configuration and Status Registers
 */


/** 
 * \brief Mode Configuration Register
 *
 * \details
 * Register: \a DEVNPI:MAC_CFG_STATUS:MAC_ENA_CFG
 */
#define VTSS_DEVNPI_MAC_CFG_STATUS_MAC_ENA_CFG  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0xd)

/** 
 * \brief
 * Receiver Module Enable.
 *
 * \details 
 * '0': Receiver Module Disabled
 * '1': Receiver Module Enabled
 *
 * Field: ::VTSS_DEVNPI_MAC_CFG_STATUS_MAC_ENA_CFG . RX_ENA
 */
#define  VTSS_F_DEVNPI_MAC_CFG_STATUS_MAC_ENA_CFG_RX_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Transmitter Module Enable.
 *
 * \details 
 * '0': Transmitter Module Disabled
 * '1': Transmitter Module Enabled
 *
 * Field: ::VTSS_DEVNPI_MAC_CFG_STATUS_MAC_ENA_CFG . TX_ENA
 */
#define  VTSS_F_DEVNPI_MAC_CFG_STATUS_MAC_ENA_CFG_TX_ENA  VTSS_BIT(0)


/** 
 * \brief Mode Configuration Register
 *
 * \details
 * Register: \a DEVNPI:MAC_CFG_STATUS:MAC_MODE_CFG
 */
#define VTSS_DEVNPI_MAC_CFG_STATUS_MAC_MODE_CFG  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0xe)

/** 
 * \brief
 * Enables 1 Gbps mode.
 *
 * \details 
 * '0': 10/100 Mbps mode
 * '1': 1 Gbps mode. Note: FDX MUST be asserted.
 *
 * Field: ::VTSS_DEVNPI_MAC_CFG_STATUS_MAC_MODE_CFG . GIGA_MODE_ENA
 */
#define  VTSS_F_DEVNPI_MAC_CFG_STATUS_MAC_MODE_CFG_GIGA_MODE_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Enables Full Duplex:
 *
 * \details 
 * '0': Half Duplex
 * '1': Full duplex.
 * \nNote: Full duplex MUST be selected if GIGA_MODE is enabled.
 *
 * Field: ::VTSS_DEVNPI_MAC_CFG_STATUS_MAC_MODE_CFG . FDX_ENA
 */
#define  VTSS_F_DEVNPI_MAC_CFG_STATUS_MAC_MODE_CFG_FDX_ENA  VTSS_BIT(0)


/** 
 * \brief Max Length Configuration Register
 *
 * \details
 * Register: \a DEVNPI:MAC_CFG_STATUS:MAC_MAXLEN_CFG
 */
#define VTSS_DEVNPI_MAC_CFG_STATUS_MAC_MAXLEN_CFG  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0xf)

/** 
 * \brief
 * The maximum frame length accepted by the Receive Module of the MAC. If
 * the length is exceeded, this is indicated in the Statistics Engine
 * (RX_OVERSIZE). The maximum length is automatically adjusted to
 * accommodate maximum sized frames containing single/double VLAN tag(s) -
 * given that the MAC is configured to be single/double VLAN aware.
 *
 * \details 
 * Field: ::VTSS_DEVNPI_MAC_CFG_STATUS_MAC_MAXLEN_CFG . MAX_LEN
 */
#define  VTSS_F_DEVNPI_MAC_CFG_STATUS_MAC_MAXLEN_CFG_MAX_LEN(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DEVNPI_MAC_CFG_STATUS_MAC_MAXLEN_CFG_MAX_LEN     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DEVNPI_MAC_CFG_STATUS_MAC_MAXLEN_CFG_MAX_LEN(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief VLAN / Service tag configuration register
 *
 * \details
 * The MAC can be configured to accept 0, 1 and 2 tags and the TAG value
 * can be user-defined.
 *
 * Register: \a DEVNPI:MAC_CFG_STATUS:MAC_TAGS_CFG
 */
#define VTSS_DEVNPI_MAC_CFG_STATUS_MAC_TAGS_CFG  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x10)

/** 
 * \brief
 * This field defines which value is regarded as a VLAN/Service tag -
 * besides 0x8100. The value is used for all ALL tag positions. I.e. a
 * double tagged frame can have the following tag values:
 * (TAG1,TAG2):
 * ( 0x8100, 0x8100 )
 * ( 0x8100, TAG_ID )
 * ( TAG_ID, 0x8100 ) or
 * ( TAG_ID, TAG_ID )
 * 
 * Single tagged frame can have the following tag values - regardless of
 * the PB_ENA setting: 
 * 0x8100 or TAG_ID. TAG_ID should therefore normally be set to 0x8100 if
 * PB_ENA = '0'.
 *
 * \details 
 * '0': The MAC operates in a non S-TAG aware mode.
 * '1': The MAC accepts S-TAGs that are equal to the value given in the
 * TAG_ID configuration value.
 *
 * Field: ::VTSS_DEVNPI_MAC_CFG_STATUS_MAC_TAGS_CFG . TAG_ID
 */
#define  VTSS_F_DEVNPI_MAC_CFG_STATUS_MAC_TAGS_CFG_TAG_ID(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_DEVNPI_MAC_CFG_STATUS_MAC_TAGS_CFG_TAG_ID     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_DEVNPI_MAC_CFG_STATUS_MAC_TAGS_CFG_TAG_ID(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Provider Bridge Enable
 *
 * \details 
 * '0': The MAC operates in a non S-TAG aware mode.
 * '1': The MAC accepts S-TAGs that are equal to the value given in the
 * PB_TAG_VAL configuration value. VLAN_AWR_ENA must be set to '1' if
 * PB_ENA = '1'.
 *
 * Field: ::VTSS_DEVNPI_MAC_CFG_STATUS_MAC_TAGS_CFG . PB_ENA
 */
#define  VTSS_F_DEVNPI_MAC_CFG_STATUS_MAC_TAGS_CFG_PB_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Single VLAN Aware:\nEnables the MAC to work in a single VLAN aware
 * environment.
 *
 * \details 
 * '0': Single VLAN Disabled
 * '1': Single VLAN Enabled.
 *
 * Field: ::VTSS_DEVNPI_MAC_CFG_STATUS_MAC_TAGS_CFG . VLAN_AWR_ENA
 */
#define  VTSS_F_DEVNPI_MAC_CFG_STATUS_MAC_TAGS_CFG_VLAN_AWR_ENA  VTSS_BIT(0)

/** 
 * \brief
 * When set, single and double tagged frames are allowed to be 4/8 bytes
 * longer than the MAXLEN configuration.
 *
 * \details 
 * Field: ::VTSS_DEVNPI_MAC_CFG_STATUS_MAC_TAGS_CFG . VLAN_LEN_AWR_ENA
 */
#define  VTSS_F_DEVNPI_MAC_CFG_STATUS_MAC_TAGS_CFG_VLAN_LEN_AWR_ENA  VTSS_BIT(2)


/** 
 * \brief Advanced Check Feature Configuration Register
 *
 * \details
 * Register: \a DEVNPI:MAC_CFG_STATUS:MAC_ADV_CHK_CFG
 */
#define VTSS_DEVNPI_MAC_CFG_STATUS_MAC_ADV_CHK_CFG  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x11)

/** 
 * \brief
 * Length Drop Enable:\nConfigures the Receive Module to drop frames in
 * reference to in-range and out-of-range errors:
 *
 * \details 
 * '0': Length Drop Disabled
 * '1': Length Drop Enabled.
 *
 * Field: ::VTSS_DEVNPI_MAC_CFG_STATUS_MAC_ADV_CHK_CFG . LEN_DROP_ENA
 */
#define  VTSS_F_DEVNPI_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_LEN_DROP_ENA  VTSS_BIT(0)


/** 
 * \brief Inter Frame Gap Configuration Register
 *
 * \details
 * Register: \a DEVNPI:MAC_CFG_STATUS:MAC_IFG_CFG
 */
#define VTSS_DEVNPI_MAC_CFG_STATUS_MAC_IFG_CFG  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x12)

/** 
 * \brief
 * Used to adjust the duration of the inter-frame gap in the Tx direction
 * and must be set according to the speed and duplex settings.
 *
 * \details 
 * 10/100 Mbps, HDX, FDX 0x19, 0x13
 * 1000 Mbps: 0x07.
 *
 * Field: ::VTSS_DEVNPI_MAC_CFG_STATUS_MAC_IFG_CFG . TX_IFG
 */
#define  VTSS_F_DEVNPI_MAC_CFG_STATUS_MAC_IFG_CFG_TX_IFG(x)  VTSS_ENCODE_BITFIELD(x,8,5)
#define  VTSS_M_DEVNPI_MAC_CFG_STATUS_MAC_IFG_CFG_TX_IFG     VTSS_ENCODE_BITMASK(8,5)
#define  VTSS_X_DEVNPI_MAC_CFG_STATUS_MAC_IFG_CFG_TX_IFG(x)  VTSS_EXTRACT_BITFIELD(x,8,5)

/** 
 * \brief
 * Used to adjust the duration of the second part of the inter-frame gap in
 * the Rx direction and must be set according to the speed and duplex
 * settings.
 *
 * \details 
 * 10/100 Mbps, HDX, FDX: 0x8, 0xB
 * 1000 Mbps: 0x1.
 *
 * Field: ::VTSS_DEVNPI_MAC_CFG_STATUS_MAC_IFG_CFG . RX_IFG2
 */
#define  VTSS_F_DEVNPI_MAC_CFG_STATUS_MAC_IFG_CFG_RX_IFG2(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_DEVNPI_MAC_CFG_STATUS_MAC_IFG_CFG_RX_IFG2     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_DEVNPI_MAC_CFG_STATUS_MAC_IFG_CFG_RX_IFG2(x)  VTSS_EXTRACT_BITFIELD(x,4,4)

/** 
 * \brief
 * Used to adjust the duration of the first part of the inter-frame gap in
 * the Rx direction and must be set according to the speed settings.
 *
 * \details 
 * 10/100 Mbps: 0x7
 * 1000 Mbps: 0x5.
 *
 * Field: ::VTSS_DEVNPI_MAC_CFG_STATUS_MAC_IFG_CFG . RX_IFG1
 */
#define  VTSS_F_DEVNPI_MAC_CFG_STATUS_MAC_IFG_CFG_RX_IFG1(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_DEVNPI_MAC_CFG_STATUS_MAC_IFG_CFG_RX_IFG1     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_DEVNPI_MAC_CFG_STATUS_MAC_IFG_CFG_RX_IFG1(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief Half Duplex Configuration Register
 *
 * \details
 * Register: \a DEVNPI:MAC_CFG_STATUS:MAC_HDX_CFG
 */
#define VTSS_DEVNPI_MAC_CFG_STATUS_MAC_HDX_CFG  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x13)

/** 
 * \brief
 * Seed value loaded into the PRBS of the MAC.
 *
 * \details 
 * Used to prevent excessive collision events.
 *
 * Field: ::VTSS_DEVNPI_MAC_CFG_STATUS_MAC_HDX_CFG . SEED
 */
#define  VTSS_F_DEVNPI_MAC_CFG_STATUS_MAC_HDX_CFG_SEED(x)  VTSS_ENCODE_BITFIELD(x,16,8)
#define  VTSS_M_DEVNPI_MAC_CFG_STATUS_MAC_HDX_CFG_SEED     VTSS_ENCODE_BITMASK(16,8)
#define  VTSS_X_DEVNPI_MAC_CFG_STATUS_MAC_HDX_CFG_SEED(x)  VTSS_EXTRACT_BITFIELD(x,16,8)

/** 
 * \brief
 * Load SEED value into PRNG register. A SEED value is loaded into the PRNG
 * register of the MAC, when SEED_LOAD is asserted. After a load, the
 * SEED_LOAD must be deasserted.
 *
 * \details 
 * '0': Do not load SEED value
 * '1': Load SEED value.
 *
 * Field: ::VTSS_DEVNPI_MAC_CFG_STATUS_MAC_HDX_CFG . SEED_LOAD
 */
#define  VTSS_F_DEVNPI_MAC_CFG_STATUS_MAC_HDX_CFG_SEED_LOAD  VTSS_BIT(12)

/** 
 * \brief
 * This bit is used to setup the MAC to retransmit a frame after an early
 * collision even though 16 (or more) early collisions have occurred. This
 * feature violates the IEEE 802.3 standard and should only be used when
 * running in HDX flow control, which is not defined in the IEEE standard
 * anyway.
 *
 * \details 
 * '0': A frame will be discarded and counted as an excessive collision if
 * 16 collisions occur for this frame.
 * '1': The MAC will retransmit a frame after an early collision,
 * regardless of the number of previous early collisions. The backoff
 * sequence will be reset after every 16 collisions.
 *
 * Field: ::VTSS_DEVNPI_MAC_CFG_STATUS_MAC_HDX_CFG . RETRY_AFTER_EXC_COL_ENA
 */
#define  VTSS_F_DEVNPI_MAC_CFG_STATUS_MAC_HDX_CFG_RETRY_AFTER_EXC_COL_ENA  VTSS_BIT(8)

/** 
 * \brief
 * Adjustment of early/late collision boundary:\nThis bitgroup is used to
 * adjust the MAC so that a collision on a shared transmission medium
 * before bit 512 is handled as an early collision, whereas a collision
 * after bit 512 is handled as a late collision, i.e. no retransmission is
 * performed.
 *
 * \details 
 * Field: ::VTSS_DEVNPI_MAC_CFG_STATUS_MAC_HDX_CFG . LATE_COL_POS
 */
#define  VTSS_F_DEVNPI_MAC_CFG_STATUS_MAC_HDX_CFG_LATE_COL_POS(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_DEVNPI_MAC_CFG_STATUS_MAC_HDX_CFG_LATE_COL_POS     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_DEVNPI_MAC_CFG_STATUS_MAC_HDX_CFG_LATE_COL_POS(x)  VTSS_EXTRACT_BITFIELD(x,0,7)


/** 
 * \brief Sticky Bit Register
 *
 * \details
 * Register: \a DEVNPI:MAC_CFG_STATUS:MAC_STICKY
 */
#define VTSS_DEVNPI_MAC_CFG_STATUS_MAC_STICKY  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x14)

/** 
 * \brief
 * Sticky bit indicating that an inter packet gap shrink was detected (IPG
 * < 12 bytes).
 *
 * \details 
 * Field: ::VTSS_DEVNPI_MAC_CFG_STATUS_MAC_STICKY . RX_IPG_SHRINK_STICKY
 */
#define  VTSS_F_DEVNPI_MAC_CFG_STATUS_MAC_STICKY_RX_IPG_SHRINK_STICKY  VTSS_BIT(9)

/** 
 * \brief
 * Sticky bit indicating that a preamble shrink was detected (preamble < 8
 * bytes).
 *
 * \details 
 * '0': no preamble shrink was detected
 * '1': a preamble shrink was detected one or more times
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DEVNPI_MAC_CFG_STATUS_MAC_STICKY . RX_PREAM_SHRINK_STICKY
 */
#define  VTSS_F_DEVNPI_MAC_CFG_STATUS_MAC_STICKY_RX_PREAM_SHRINK_STICKY  VTSS_BIT(8)

/** 
 * \brief
 * Sticky bit indicating that junk was received (bytes not recognized as a
 * frame).
 *
 * \details 
 * '0': no junk was received
 * '1': junk was received one or more times
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DEVNPI_MAC_CFG_STATUS_MAC_STICKY . RX_JUNK_STICKY
 */
#define  VTSS_F_DEVNPI_MAC_CFG_STATUS_MAC_STICKY_RX_JUNK_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * Sticky bit indicating that the transmit MAC asked the host for a frame
 * retransmission.
 *
 * \details 
 * '0': no tx retransmission was initiated
 * '1': one or more tx retransmissions were initiated
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DEVNPI_MAC_CFG_STATUS_MAC_STICKY . TX_RETRANSMIT_STICKY
 */
#define  VTSS_F_DEVNPI_MAC_CFG_STATUS_MAC_STICKY_TX_RETRANSMIT_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Sticky bit indicating that the transmit host issued a jamming signal.
 *
 * \details 
 * '0': the transmit host issued no jamming signal
 * '1': the transmit host issued one or morejamming signals
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DEVNPI_MAC_CFG_STATUS_MAC_STICKY . TX_JAM_STICKY
 */
#define  VTSS_F_DEVNPI_MAC_CFG_STATUS_MAC_STICKY_TX_JAM_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * Sticky bit indicating that the transmit host initiated abort was
 * executed.
 *
 * \details 
 * Field: ::VTSS_DEVNPI_MAC_CFG_STATUS_MAC_STICKY . TX_ABORT_STICKY
 */
#define  VTSS_F_DEVNPI_MAC_CFG_STATUS_MAC_STICKY_TX_ABORT_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a DEVNPI:PCS1G_CFG_STATUS
 *
 * PCS 1G Configuration Status Registers
 */


/** 
 * \brief PCS1G Configuration
 *
 * \details
 * PCS1G main configuration register
 *
 * Register: \a DEVNPI:PCS1G_CFG_STATUS:PCS1G_CFG
 */
#define VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_CFG  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x15)

/** 
 * \brief
 * Set type of link_status indication at CPU-System
 *
 * \details 
 * 0: Sync_status (from PCS synchronization state machine)
 * 1: Bit 15 of PCS1G_ANEG_STATUS.lp_adv_ability (Link up/down)
 *
 * Field: ::VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_CFG . LINK_STATUS_TYPE
 */
#define  VTSS_F_DEVNPI_PCS1G_CFG_STATUS_PCS1G_CFG_LINK_STATUS_TYPE  VTSS_BIT(4)

/** 
 * \brief
 * PCS enable
 *
 * \details 
 * 0: Disable PCS
 * 1: Enable PCS
 *
 * Field: ::VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_CFG . PCS_ENA
 */
#define  VTSS_F_DEVNPI_PCS1G_CFG_STATUS_PCS1G_CFG_PCS_ENA  VTSS_BIT(0)


/** 
 * \brief PCS1G Mode Configuration
 *
 * \details
 * PCS1G mode configuration
 *
 * Register: \a DEVNPI:PCS1G_CFG_STATUS:PCS1G_MODE_CFG
 */
#define VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_MODE_CFG  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x16)

/** 
 * \brief
 * Unidirectional mode enable. Implementation of 802.3, Clause 66. When
 * asserted, this enables MAC to transmit data independent of the state of
 * the receive link.
 *
 * \details 
 * 0: Unidirectional mode disabled
 * 1: Unidirectional mode enabled
 *
 * Field: ::VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_MODE_CFG . UNIDIR_MODE_ENA
 */
#define  VTSS_F_DEVNPI_PCS1G_CFG_STATUS_PCS1G_MODE_CFG_UNIDIR_MODE_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Selection of PCS operation
 *
 * \details 
 * 0: PCS is used in SERDES mode
 * 1: PCS is used in SGMII mode. Configuration bit
 * PCS1G_ANEG_CFG.SW_RESOLVE_ENA must be set additionally
 *
 * Field: ::VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_MODE_CFG . SGMII_MODE_ENA
 */
#define  VTSS_F_DEVNPI_PCS1G_CFG_STATUS_PCS1G_MODE_CFG_SGMII_MODE_ENA  VTSS_BIT(0)


/** 
 * \brief PCS1G Signal Detect Configuration
 *
 * \details
 * PCS1G signal_detect configuration
 *
 * Register: \a DEVNPI:PCS1G_CFG_STATUS:PCS1G_SD_CFG
 */
#define VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_SD_CFG  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x17)

/** 
 * \brief
 * Signal detect selection (select input for internal signal_detect line)
 *
 * \details 
 * 0: Select signal_detect line from hardmacro
 * 1: Select external signal_detect line
 *
 * Field: ::VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_SD_CFG . SD_SEL
 */
#define  VTSS_F_DEVNPI_PCS1G_CFG_STATUS_PCS1G_SD_CFG_SD_SEL  VTSS_BIT(8)

/** 
 * \brief
 * Signal detect polarity: The signal level on signal_detect input pin must
 * be equal to SD_POL to indicate signal detection (SD_ENA must be set)
 *
 * \details 
 * 0: Signal Detect input pin must be '0' to indicate a signal detection
 * 1: Signal Detect input pin must be '1' to indicate a signal detection
 *
 * Field: ::VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_SD_CFG . SD_POL
 */
#define  VTSS_F_DEVNPI_PCS1G_CFG_STATUS_PCS1G_SD_CFG_SD_POL  VTSS_BIT(4)

/** 
 * \brief
 * Signal Detect Enable
 *
 * \details 
 * 0: The Signal Detect input pin is ignored. The PCS assumes an active
 * Signal Detect at all times
 * 1: The Signal Detect input pin is used to determine if a signal is
 * detected
 *
 * Field: ::VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_SD_CFG . SD_ENA
 */
#define  VTSS_F_DEVNPI_PCS1G_CFG_STATUS_PCS1G_SD_CFG_SD_ENA  VTSS_BIT(0)


/** 
 * \brief PCS1G Aneg Configuration
 *
 * \details
 * PCS1G Auto-negotiation configuration register
 *
 * Register: \a DEVNPI:PCS1G_CFG_STATUS:PCS1G_ANEG_CFG
 */
#define VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x18)

/** 
 * \brief
 * Advertised Ability Register: Holds the capabilities of the device as
 * described IEEE 802.3, Clause 37. If SGMII mode is selected
 * (PCS1G_MODE_CFG.SGMII_MODE_ENA = 1), SW_RESOLVE_ENA must be set.
 *
 * \details 
 * Field: ::VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG . ADV_ABILITY
 */
#define  VTSS_F_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ADV_ABILITY(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ADV_ABILITY     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ADV_ABILITY(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Software Resolve Abilities
 *
 * \details 
 * 0: If Auto Negotiation fails (no matching HD or FD capabilities) the
 * link is disabled
 * 1: The result of an Auto Negotiation is ignored - the link can be setup
 * via software. This bit must be set in SGMII mode.
 *
 * Field: ::VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG . SW_RESOLVE_ENA
 */
#define  VTSS_F_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_SW_RESOLVE_ENA  VTSS_BIT(8)

/** 
 * \brief
 * Auto Negotiation Restart
 *
 * \details 
 * 0: No action
 * 1: Restart Auto Negotiation
 *
 * Field: ::VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG . ANEG_RESTART_ONE_SHOT
 */
#define  VTSS_F_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ANEG_RESTART_ONE_SHOT  VTSS_BIT(1)

/** 
 * \brief
 * Auto Negotiation Enable
 *
 * \details 
 * 0: Auto Negotiation Disabled
 * 1: Auto Negotiation Enabled
 *
 * Field: ::VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG . ANEG_ENA
 */
#define  VTSS_F_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ANEG_ENA  VTSS_BIT(0)


/** 
 * \brief PCS1G Aneg Next Page Configuration
 *
 * \details
 * PCS1G Auto-negotiation configuration register for next-page function
 *
 * Register: \a DEVNPI:PCS1G_CFG_STATUS:PCS1G_ANEG_NP_CFG
 */
#define VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_NP_CFG  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x19)

/** 
 * \brief
 * Next page register: Holds the next-page information as described in IEEE
 * 802.3, Clause 37
 *
 * \details 
 * Field: ::VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_NP_CFG . NP_TX
 */
#define  VTSS_F_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_NP_CFG_NP_TX(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_NP_CFG_NP_TX     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_NP_CFG_NP_TX(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Next page loaded
 *
 * \details 
 * 0: next page is free and can be loaded
 * 1: next page register has been filled (to be set after np_tx has been
 * filled)
 *
 * Field: ::VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_NP_CFG . NP_LOADED_ONE_SHOT
 */
#define  VTSS_F_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_NP_CFG_NP_LOADED_ONE_SHOT  VTSS_BIT(0)


/** 
 * \brief PCS1G Loopback Configuration
 *
 * \details
 * PCS1G Loop-Back configuration register
 *
 * Register: \a DEVNPI:PCS1G_CFG_STATUS:PCS1G_LB_CFG
 */
#define VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_LB_CFG  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x1a)

/** 
 * \brief
 * Loops data in PCS (TBI side) from egress direction to ingress direction.
 * The Rx clock is automatically set equal to the Tx clock
 *
 * \details 
 * 0: TBI Loopback Disabled
 * 1:TBI Loopback Enabled
 *
 * Field: ::VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_LB_CFG . TBI_HOST_LB_ENA
 */
#define  VTSS_F_DEVNPI_PCS1G_CFG_STATUS_PCS1G_LB_CFG_TBI_HOST_LB_ENA  VTSS_BIT(0)


/** 
 * \brief PCS1G ANEG Status Register
 *
 * \details
 * PCS1G Auto-negotiation status register
 *
 * Register: \a DEVNPI:PCS1G_CFG_STATUS:PCS1G_ANEG_STATUS
 */
#define VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x1d)

/** 
 * \brief
 * Advertised abilities from link partner as described in IEEE 802.3,
 * Clause 37
 *
 * \details 
 * Field: ::VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS . LP_ADV_ABILITY
 */
#define  VTSS_F_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS_LP_ADV_ABILITY(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS_LP_ADV_ABILITY     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS_LP_ADV_ABILITY(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Resolve priority
 *
 * \details 
 * 0: ANEG is in progress
 * 1: ANEG nearly finished - priority can be resolved (via software)
 *
 * Field: ::VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS . PR
 */
#define  VTSS_F_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS_PR  VTSS_BIT(4)

/** 
 * \brief
 * Status indicating whether a new page has been received.
 *
 * \details 
 * 0: No new page received
 * 1: New page received
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS . PAGE_RX_STICKY
 */
#define  VTSS_F_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS_PAGE_RX_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * Auto Negotiation Complete
 *
 * \details 
 * 0: No Auto Negotiation has been completed
 * 1: Indicates that an Auto Negotiation has completed successfully
 *
 * Field: ::VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS . ANEG_COMPLETE
 */
#define  VTSS_F_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS_ANEG_COMPLETE  VTSS_BIT(0)


/** 
 * \brief PCS1G Aneg Next Page Status Register
 *
 * \details
 * PCS1G Auto-negotiation next page status register
 *
 * Register: \a DEVNPI:PCS1G_CFG_STATUS:PCS1G_ANEG_NP_STATUS
 */
#define VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_NP_STATUS  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x1e)

/** 
 * \brief
 * Next page ability register from link partner as described in IEEE 802.3,
 * Clause 37
 *
 * \details 
 * Field: ::VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_NP_STATUS . LP_NP_RX
 */
#define  VTSS_F_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_NP_STATUS_LP_NP_RX(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_NP_STATUS_LP_NP_RX     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_DEVNPI_PCS1G_CFG_STATUS_PCS1G_ANEG_NP_STATUS_LP_NP_RX(x)  VTSS_EXTRACT_BITFIELD(x,16,16)


/** 
 * \brief PCS1G link status
 *
 * \details
 * PCS1G link status register
 *
 * Register: \a DEVNPI:PCS1G_CFG_STATUS:PCS1G_LINK_STATUS
 */
#define VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x1f)

/** 
 * \brief
 * Indicates whether or not the selected Signal Detect input line is
 * asserted
 *
 * \details 
 * 0: No signal detected
 * 1: Signal detected
 *
 * Field: ::VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS . SIGNAL_DETECT
 */
#define  VTSS_F_DEVNPI_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS_SIGNAL_DETECT  VTSS_BIT(8)

/** 
 * \brief
 * Indicates whether the link is up or down. A link is up when ANEG state
 * machine is in state LINK_OK or AN_DISABLE_LINK_OK
 *
 * \details 
 * 0: Link down
 * 1: Link up
 *
 * Field: ::VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS . LINK_STATUS
 */
#define  VTSS_F_DEVNPI_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS_LINK_STATUS  VTSS_BIT(4)

/** 
 * \brief
 * Indicates if PCS has successfully synchronized
 *
 * \details 
 * 0: PCS is out of sync
 * 1: PCS has synchronized
 *
 * Field: ::VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS . SYNC_STATUS
 */
#define  VTSS_F_DEVNPI_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS_SYNC_STATUS  VTSS_BIT(0)


/** 
 * \brief PCS1G link down counter
 *
 * \details
 * PCS1G link down counter register
 *
 * Register: \a DEVNPI:PCS1G_CFG_STATUS:PCS1G_LINK_DOWN_CNT
 */
#define VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_LINK_DOWN_CNT  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x20)

/** 
 * \brief
 * Link Down Counter. A counter that counts the number of times a link has
 * been down. The counter does not saturate at 255 and is only cleared when
 * writing 0 to the register
 *
 * \details 
 * Field: ::VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_LINK_DOWN_CNT . LINK_DOWN_CNT
 */
#define  VTSS_F_DEVNPI_PCS1G_CFG_STATUS_PCS1G_LINK_DOWN_CNT_LINK_DOWN_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_DEVNPI_PCS1G_CFG_STATUS_PCS1G_LINK_DOWN_CNT_LINK_DOWN_CNT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_DEVNPI_PCS1G_CFG_STATUS_PCS1G_LINK_DOWN_CNT_LINK_DOWN_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief PCS1G sticky register
 *
 * \details
 * PCS1G status register for sticky bits
 *
 * Register: \a DEVNPI:PCS1G_CFG_STATUS:PCS1G_STICKY
 */
#define VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_STICKY  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x21)

/** 
 * \brief
 * The sticky bit is set when the link has been down - i.e. if the ANEG
 * state machine has not been in the AN_DISABLE_LINK_OK or LINK_OK state
 * for one or more clock cycles. This occurs if e.g. ANEG is restarted or
 * for example if signal-detect or synchronization has been lost for more
 * than 10 ms (1.6 ms in SGMII mode). By setting the UDLT bit, the required
 * down time can be reduced to 9,77 us (1.56 us)
 *
 * \details 
 * 0: Link is up
 * 1: Link has been down
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_STICKY . LINK_DOWN_STICKY
 */
#define  VTSS_F_DEVNPI_PCS1G_CFG_STATUS_PCS1G_STICKY_LINK_DOWN_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Sticky bit indicating if PCS synchronization has been lost
 *
 * \details 
 * 0: Synchronization has not been lost at any time
 * 1: Synchronization has been lost for one or more clock cycles
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_DEVNPI_PCS1G_CFG_STATUS_PCS1G_STICKY . OUT_OF_SYNC_STICKY
 */
#define  VTSS_F_DEVNPI_PCS1G_CFG_STATUS_PCS1G_STICKY_OUT_OF_SYNC_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a DEVNPI:PCS1G_TSTPAT_CFG_STATUS
 *
 * PCS1G Testpattern Configuration and Status Registers
 */


/** 
 * \brief PCS1G TSTPAT MODE CFG
 *
 * \details
 * PCS1G testpattern mode configuration register (Frame based pattern 4 and
 * 5 might be not available depending on chip type)
 *
 * Register: \a DEVNPI:PCS1G_TSTPAT_CFG_STATUS:PCS1G_TSTPAT_MODE_CFG
 */
#define VTSS_DEVNPI_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_MODE_CFG  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x25)

/** 
 * \brief
 * Jitter Test Pattern Select: Enables and selects the jitter test pattern
 * to be transmitted. The jitter test patterns are according to the IEEE
 * 802.3, Annex 36A
 *
 * \details 
 * 0: Disable transmission of test patterns
 * 1: High frequency test pattern - repeated transmission of D21.5 code
 * group
 * 2: Low frequency test pattern - repeated transmission of K28.7 code
 * group
 * 3: Mixed frequency test pattern - repeated transmission of K28.5 code
 * group
 * 4: Long continuous random test pattern (packet length is 1524 bytes)
 * 5: Short continuous random test pattern (packet length is 360 bytes)
 *
 * Field: ::VTSS_DEVNPI_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_MODE_CFG . JTP_SEL
 */
#define  VTSS_F_DEVNPI_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_MODE_CFG_JTP_SEL(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_DEVNPI_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_MODE_CFG_JTP_SEL     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_DEVNPI_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_MODE_CFG_JTP_SEL(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief PCS1G TSTPAT STATUS
 *
 * \details
 * PCS1G testpattern status register
 *
 * Register: \a DEVNPI:PCS1G_TSTPAT_CFG_STATUS:PCS1G_TSTPAT_STATUS
 */
#define VTSS_DEVNPI_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_STATUS  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x26)

/** 
 * \brief
 * Jitter Test Pattern Error Counter. Due to re-sync measures it might
 * happen that single errors are not counted (applies for 2.5gpbs mode).
 * The counter saturates at 255 and is only cleared when writing 0 to the
 * register
 *
 * \details 
 * Field: ::VTSS_DEVNPI_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_STATUS . JTP_ERR_CNT
 */
#define  VTSS_F_DEVNPI_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_STATUS_JTP_ERR_CNT(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_DEVNPI_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_STATUS_JTP_ERR_CNT     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_DEVNPI_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_STATUS_JTP_ERR_CNT(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Jitter Test Pattern Error
 *
 * \details 
 * 0: Jitter pattern checker has found no error
 * 1: Jitter pattern checker has found an error
 *
 * Field: ::VTSS_DEVNPI_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_STATUS . JTP_ERR
 */
#define  VTSS_F_DEVNPI_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_STATUS_JTP_ERR  VTSS_BIT(4)

/** 
 * \brief
 * Jitter Test Pattern Lock
 *
 * \details 
 * 0: Jitter pattern checker has not locked
 * 1: Jitter pattern checker has locked
 *
 * Field: ::VTSS_DEVNPI_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_STATUS . JTP_LOCK
 */
#define  VTSS_F_DEVNPI_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_STATUS_JTP_LOCK  VTSS_BIT(0)

/**
 * Register Group: \a DEVNPI:PCS_FX100_CONFIGURATION
 *
 * PCS FX100 Configuration Registers
 */


/** 
 * \brief PCS 100Base FX Configuration
 *
 * \details
 * Configuration bit groups for 100Base-FX PCS
 *
 * Register: \a DEVNPI:PCS_FX100_CONFIGURATION:PCS_FX100_CFG
 */
#define VTSS_DEVNPI_PCS_FX100_CONFIGURATION_PCS_FX100_CFG  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x27)

/** 
 * \brief
 * Signal detect selection (select input for internal signal_detect line)
 *
 * \details 
 * 0: Select signal_detect line from hardmacro
 * 1: Select external signal_detect line
 *
 * Field: ::VTSS_DEVNPI_PCS_FX100_CONFIGURATION_PCS_FX100_CFG . SD_SEL
 */
#define  VTSS_F_DEVNPI_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_SD_SEL  VTSS_BIT(26)

/** 
 * \brief
 * Signal Detect Enable
 *
 * \details 
 * 0: The Signal Detect input pin is ignored. The PCS assumes an active
 * Signal Detect at all times
 * 1: The Signal Detect input pin is used to determine if a signal is
 * detected
 *
 * Field: ::VTSS_DEVNPI_PCS_FX100_CONFIGURATION_PCS_FX100_CFG . SD_ENA
 */
#define  VTSS_F_DEVNPI_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_SD_ENA  VTSS_BIT(24)

/** 
 * \brief
 * Link hysteresis timer configuration. The hysteresis time lasts
 * [linkhysttimer] * 65536 ns + 2320 ns. If linkhysttime is set to 5, the
 * hysteresis lasts the minimum time of 330 us as specified in IEEE802.3 -
 * 24.3.3.4.
 *
 * \details 
 * Field: ::VTSS_DEVNPI_PCS_FX100_CONFIGURATION_PCS_FX100_CFG . LINKHYSTTIMER
 */
#define  VTSS_F_DEVNPI_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_LINKHYSTTIMER(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_DEVNPI_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_LINKHYSTTIMER     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_DEVNPI_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_LINKHYSTTIMER(x)  VTSS_EXTRACT_BITFIELD(x,4,4)

/** 
 * \brief
 * Unidirectional mode enable. Implementation 0f 802.3 clause 66. When
 * asserted, this enables MAC to transmit data independent of the state of
 * the receive link.
 *
 * \details 
 * 0: Unidirectional mode disabled
 * 1: Unidirectional mode enabled
 *
 * Field: ::VTSS_DEVNPI_PCS_FX100_CONFIGURATION_PCS_FX100_CFG . UNIDIR_MODE_ENA
 */
#define  VTSS_F_DEVNPI_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_UNIDIR_MODE_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Far-End Fault (FEF) detection enable
 *
 * \details 
 * 0: Disable FEF detection
 * 1 Enable FEF detection
 *
 * Field: ::VTSS_DEVNPI_PCS_FX100_CONFIGURATION_PCS_FX100_CFG . FEFCHK_ENA
 */
#define  VTSS_F_DEVNPI_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_FEFCHK_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Far-End Fault (FEF) generation enable
 *
 * \details 
 * 0: Disable FEF generation
 * 1 Enable FEF generation
 *
 * Field: ::VTSS_DEVNPI_PCS_FX100_CONFIGURATION_PCS_FX100_CFG . FEFGEN_ENA
 */
#define  VTSS_F_DEVNPI_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_FEFGEN_ENA  VTSS_BIT(1)

/** 
 * \brief
 * PCS enable
 *
 * \details 
 * 0: Disable PCS
 * 1: Enable PCS
 *
 * Field: ::VTSS_DEVNPI_PCS_FX100_CONFIGURATION_PCS_FX100_CFG . PCS_ENA
 */
#define  VTSS_F_DEVNPI_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_PCS_ENA  VTSS_BIT(0)

/**
 * Register Group: \a DEVNPI:PCS_FX100_STATUS
 *
 * PCS FX100 Status Registers
 */


/** 
 * \brief PCS 100Base FX Status
 *
 * \details
 * Status bit groups for 100Base-FX PCS. Note: If sigdet_cfg != "00" is
 * selected status signal "signal_detect" shows the internal signal_detect
 * value is gated with the status of rx toggle-rate control circuitry.
 *
 * Register: \a DEVNPI:PCS_FX100_STATUS:PCS_FX100_STATUS
 */
#define VTSS_DEVNPI_PCS_FX100_STATUS_PCS_FX100_STATUS  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x28)

/** 
 * \brief
 * PCS error has occurred
 *
 * \details 
 * 1: RX_ER was high while RX_DV active
 * 0: No RX_ER indication found while RX_DV active
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_DEVNPI_PCS_FX100_STATUS_PCS_FX100_STATUS . PCS_ERROR_STICKY
 */
#define  VTSS_F_DEVNPI_PCS_FX100_STATUS_PCS_FX100_STATUS_PCS_ERROR_STICKY  VTSS_BIT(7)

/** 
 * \brief
 * Far-end Fault state has occurred
 *
 * \details 
 * 1: A Far-End Fault has been detected
 * 0: No Far-End Fault occurred
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_DEVNPI_PCS_FX100_STATUS_PCS_FX100_STATUS . FEF_FOUND_STICKY
 */
#define  VTSS_F_DEVNPI_PCS_FX100_STATUS_PCS_FX100_STATUS_FEF_FOUND_STICKY  VTSS_BIT(6)

/** 
 * \brief
 * Stream Start Delimiter error occurred
 *
 * \details 
 * 1: A Start-of-Stream Delimiter error has been detected
 * 0: No SSD error occurred
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_DEVNPI_PCS_FX100_STATUS_PCS_FX100_STATUS . SSD_ERROR_STICKY
 */
#define  VTSS_F_DEVNPI_PCS_FX100_STATUS_PCS_FX100_STATUS_SSD_ERROR_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * Synchronization lost
 *
 * \details 
 * 1: Synchronization lost
 * 0: No sync lost occurred
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_DEVNPI_PCS_FX100_STATUS_PCS_FX100_STATUS . SYNC_LOST_STICKY
 */
#define  VTSS_F_DEVNPI_PCS_FX100_STATUS_PCS_FX100_STATUS_SYNC_LOST_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Current status of Far-end Fault detection state
 *
 * \details 
 * 1: Link currently in fault state
 * 0: Link is in normal state
 *
 * Field: ::VTSS_DEVNPI_PCS_FX100_STATUS_PCS_FX100_STATUS . FEF_STATUS
 */
#define  VTSS_F_DEVNPI_PCS_FX100_STATUS_PCS_FX100_STATUS_FEF_STATUS  VTSS_BIT(2)

/** 
 * \brief
 * Current status of selected signal_detect input line
 *
 * \details 
 * 1: Proper signal detected
 * 0: No proper signal found
 *
 * Field: ::VTSS_DEVNPI_PCS_FX100_STATUS_PCS_FX100_STATUS . SIGNAL_DETECT
 */
#define  VTSS_F_DEVNPI_PCS_FX100_STATUS_PCS_FX100_STATUS_SIGNAL_DETECT  VTSS_BIT(1)

/** 
 * \brief
 * Status of synchronization
 *
 * \details 
 * 1: Link established
 * 0: No link found
 *
 * Field: ::VTSS_DEVNPI_PCS_FX100_STATUS_PCS_FX100_STATUS . SYNC_STATUS
 */
#define  VTSS_F_DEVNPI_PCS_FX100_STATUS_PCS_FX100_STATUS_SYNC_STATUS  VTSS_BIT(0)

/**
 * Register Group: \a DEVNPI:DEVNPI_INTR_CFG_STATUS
 *
 * Device NPI Interrupt Configuration and Status Registers

 */


/** 
 * \brief DEVNPI Intr Configuration

 *
 * \details
 * Interrupt Mask (Enable) register
 *
 * Register: \a DEVNPI:DEVNPI_INTR_CFG_STATUS:DEVNPI_INTR_CFG
 */
#define VTSS_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_CFG  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x29)

/** 
 * \brief
 * Far-end-fault indication found interrupt enable (only PCS_fx100)
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_CFG . FEF_FOUND_INTR_ENA
 */
#define  VTSS_F_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_CFG_FEF_FOUND_INTR_ENA  VTSS_BIT(8)

/** 
 * \brief
 * PTP Time-stamp/frame-ID update interrupt enable
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_CFG . PTP_UPDATE_INTR_ENA
 */
#define  VTSS_F_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_CFG_PTP_UPDATE_INTR_ENA  VTSS_BIT(7)

/** 
 * \brief
 * ANEG next page receive interrupt enable (only PCS1g)
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_CFG . AN_PAGE_RX_INTR_ENA
 */
#define  VTSS_F_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_CFG_AN_PAGE_RX_INTR_ENA  VTSS_BIT(4)

/** 
 * \brief
 * ANEG Link up interrupt enable (only PCS1g)
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_CFG . AN_LINK_UP_INTR_ENA
 */
#define  VTSS_F_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_CFG_AN_LINK_UP_INTR_ENA  VTSS_BIT(3)

/** 
 * \brief
 * ANEG Link down interrupt enable (only PCS1g)
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_CFG . AN_LINK_DOWN_INTR_ENA
 */
#define  VTSS_F_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_CFG_AN_LINK_DOWN_INTR_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Link up interrupt enable
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_CFG . LINK_UP_INTR_ENA
 */
#define  VTSS_F_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_CFG_LINK_UP_INTR_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Link down interrupt enable
 *
 * \details 
 * 0: Interrupt disabled
 * 1: Interrupt enabled
 *
 * Field: ::VTSS_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_CFG . LINK_DOWN_INTR_ENA
 */
#define  VTSS_F_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_CFG_LINK_DOWN_INTR_ENA  VTSS_BIT(0)


/** 
 * \brief DEVNPI Interrupt Reg

 *
 * \details
 * Interrupt register
 *
 * Register: \a DEVNPI:DEVNPI_INTR_CFG_STATUS:DEVNPI_INTR
 */
#define VTSS_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x2a)

/** 
 * \brief
 * Far-end-fault indication found interrupt (only PCS_fx100)
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt occured
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR . FEF_FOUND_INTR_STICKY
 */
#define  VTSS_F_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_FEF_FOUND_INTR_STICKY  VTSS_BIT(8)

/** 
 * \brief
 * PTP Time-stamp/frame-ID update interrupt
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt occured
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR . PTP_UPDATE_INTR_STICKY
 */
#define  VTSS_F_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_PTP_UPDATE_INTR_STICKY  VTSS_BIT(7)

/** 
 * \brief
 * ANEG next page receive interrupt (only PCS1g)
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt occured
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR . AN_PAGE_RX_INTR_STICKY
 */
#define  VTSS_F_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_AN_PAGE_RX_INTR_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * ANEG Link up interrupt (only PCS1g)
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt occured
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR . AN_LINK_UP_INTR_STICKY
 */
#define  VTSS_F_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_AN_LINK_UP_INTR_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * ANEG Link down interrupt (only PCS1g)
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt occured
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR . AN_LINK_DOWN_INTR_STICKY
 */
#define  VTSS_F_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_AN_LINK_DOWN_INTR_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Link up interrupt
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt occured
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR . LINK_UP_INTR_STICKY
 */
#define  VTSS_F_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_LINK_UP_INTR_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Link down interrupt
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt occured
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR . LINK_DOWN_INTR_STICKY
 */
#define  VTSS_F_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_LINK_DOWN_INTR_STICKY  VTSS_BIT(0)


/** 
 * \brief DEVNPI Interrupt Ident

 *
 * \details
 * Pending Interrupts (only enabled interrupts are visible)
 *
 * Register: \a DEVNPI:DEVNPI_INTR_CFG_STATUS:DEVNPI_INTR_IDENT
 */
#define VTSS_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_IDENT  VTSS_IOREG(VTSS_TO_DEVRGMII_0,0x2b)

/** 
 * \brief
 * Far-end-fault indication found interrupt (only PCS_fx100)
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt pending
 *
 * Field: ::VTSS_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_IDENT . FEF_FOUND_INTR_IDENT
 */
#define  VTSS_F_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_IDENT_FEF_FOUND_INTR_IDENT  VTSS_BIT(8)

/** 
 * \brief
 * PTP Time-stamp/frame-ID update interrupt
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt pending
 *
 * Field: ::VTSS_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_IDENT . PTP_UPDATE_INTR_IDENT
 */
#define  VTSS_F_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_IDENT_PTP_UPDATE_INTR_IDENT  VTSS_BIT(7)

/** 
 * \brief
 * ANEG next page receive interrupt (only PCS1g)
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt pending
 *
 * Field: ::VTSS_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_IDENT . AN_PAGE_RX_INTR_IDENT
 */
#define  VTSS_F_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_IDENT_AN_PAGE_RX_INTR_IDENT  VTSS_BIT(4)

/** 
 * \brief
 * ANEG Link up interrupt (only PCS1g)
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt pending
 *
 * Field: ::VTSS_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_IDENT . AN_LINK_UP_INTR_IDENT
 */
#define  VTSS_F_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_IDENT_AN_LINK_UP_INTR_IDENT  VTSS_BIT(3)

/** 
 * \brief
 * ANEG Link down interrupt (only PCS1g)
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt pending
 *
 * Field: ::VTSS_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_IDENT . AN_LINK_DOWN_INTR_IDENT
 */
#define  VTSS_F_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_IDENT_AN_LINK_DOWN_INTR_IDENT  VTSS_BIT(2)

/** 
 * \brief
 * Link up interrupt
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt pending
 *
 * Field: ::VTSS_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_IDENT . LINK_UP_INTR_IDENT
 */
#define  VTSS_F_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_IDENT_LINK_UP_INTR_IDENT  VTSS_BIT(1)

/** 
 * \brief
 * Link down interrupt
 *
 * \details 
 * 0: No interrupt
 * 1: Interrupt pending
 *
 * Field: ::VTSS_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_IDENT . LINK_DOWN_INTR_IDENT
 */
#define  VTSS_F_DEVNPI_DEVNPI_INTR_CFG_STATUS_DEVNPI_INTR_IDENT_LINK_DOWN_INTR_IDENT  VTSS_BIT(0)


#endif /* _VTSS_JAGUAR_REGS_DEVNPI_H_ */
