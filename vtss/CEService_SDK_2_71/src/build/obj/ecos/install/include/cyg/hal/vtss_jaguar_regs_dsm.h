/*

 Vitesse Switch Software.

==========================================================================

      Vitesse Switch Software

      Register Definitions

==========================================================================
 #####ECOSGPLCOPYRIGHTBEGIN#####
 -------------------------------------------
 This file is part of eCos, the Embedded Configurable Operating System.
 Copyright (C) 1998-2012 Free Software Foundation, Inc.

 eCos is free software; you can redistribute it and/or modify it under
 the terms of the GNU General Public License as published by the Free
 Software Foundation; either version 2 or (at your option) any later
 version.

 eCos is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.

 You should have received a copy of the GNU General Public License
 along with eCos; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

 As a special exception, if other files instantiate templates or use
 macros or inline functions from this file, or you compile this file
 and link it with other works to produce a work based on this file,
 this file does not by itself cause the resulting work to be covered by
 the GNU General Public License. However the source code for this file
 must still be made available in accordance with section (3) of the GNU
 General Public License v2.

 This exception does not invalidate any other reasons why a work based
 on this file might be covered by the GNU General Public License.
 -------------------------------------------
 #####ECOSGPLCOPYRIGHTEND#####
 ==========================================================================
 #####DESCRIPTIONBEGIN####

 Author(s):    lpovlsen
 Contributors: 
 Date:         2011-10-18
 Purpose:      HAL register definitions
 Description:  This file contains register definitions used by the
               HAL.

 ####DESCRIPTIONEND####

*/
#ifndef _VTSS_JAGUAR_REGS_DSM_H_
#define _VTSS_JAGUAR_REGS_DSM_H_

#include "vtss_jaguar_regs_common.h"

/*********************************************************************** 
 *
 * Target: \a DSM
 *
 * Disassembler
 *
 ***********************************************************************/

/**
 * Register Group: \a DSM:CFG
 *
 * Configuration registers
 */


/** 
 * \brief Configuration per device buffer.
 *
 * \details
 * Miscellaneous configurations per device buffer.
 *
 * Register: \a DSM:CFG:BUF_CFG
 *
 * @param ri Replicator: x_DSM_NON_CPU_PORTS (??), 0-31
 */
#define VTSS_DSM_CFG_BUF_CFG(ri)             VTSS_IOREG(VTSS_TO_DSM,0x299 + (ri))

/** 
 * \brief
 * Enable aging of frames stuck in the DSM buffer system for long periods.
 * Aging is done by comparing the age era stamped into the frame with the
 * current age era. This check is only performed at SOF. If the current age
 * era is more than one higher than the era stamped into the frame, then
 * the frame is discarded.
 *
 * \details 
 * '0': Aging disabled.
 * '1': Aging enabled.
 *
 * Field: ::VTSS_DSM_CFG_BUF_CFG . AGING_ENA
 */
#define  VTSS_F_DSM_CFG_BUF_CFG_AGING_ENA     VTSS_BIT(0)


/** 
 * \brief Rate control of dataflow to devices
 *
 * \details
 * Configuration of the data flow rates from the DSM towards the device.
 *
 * Register: \a DSM:CFG:RATE_CTRL
 *
 * @param ri Replicator: x_DSM_NON_CPU_PORTS (??), 0-31
 */
#define VTSS_DSM_CFG_RATE_CTRL(ri)           VTSS_IOREG(VTSS_TO_DSM,0x2b9 + (ri))

/** 
 * \brief
 * Compensation for gap between frames. This value must be set to the
 * minimum gap between frames transmitted on the interface.
 * 
 * If Rate Limit mode Frame Rate Overhead is enabled this bitgroup is used
 * for configuration of txAdditionalFrameOverhead.
 *
 * \details 
 * The value should be set to these values depending on port type and mode.
 * Ethernet: DEV1G and DEV2G5  -> 12
 *		   other			   -> 12 + preamble-size
 * CPU: n/a
 *
 * Field: ::VTSS_DSM_CFG_RATE_CTRL . FRM_GAP_COMP
 */
#define  VTSS_F_DSM_CFG_RATE_CTRL_FRM_GAP_COMP(x)  VTSS_ENCODE_BITFIELD(x,24,8)
#define  VTSS_M_DSM_CFG_RATE_CTRL_FRM_GAP_COMP     VTSS_ENCODE_BITMASK(24,8)
#define  VTSS_X_DSM_CFG_RATE_CTRL_FRM_GAP_COMP(x)  VTSS_EXTRACT_BITFIELD(x,24,8)

/** 
 * \brief
 * The transmission rate for this port when push-back from port is not
 * activated.
 * 
 * The value must be calculated from the formula:
 * 
 * TAXI_RATE_HIGH = CEILING((nominal port_speed + 200 ppm)* 2^11 /
 * (core_freq * bus_width))
 * 
 * Note for port speeds that allow HDX mode the TAXI_RATE_HIGH must be 3
 * times the value calculated according to the above formula to compensate
 * for dropped remainders of collided frames.
 *
 * \details 
 * Field: ::VTSS_DSM_CFG_RATE_CTRL . TAXI_RATE_HIGH
 */
#define  VTSS_F_DSM_CFG_RATE_CTRL_TAXI_RATE_HIGH(x)  VTSS_ENCODE_BITFIELD(x,12,12)
#define  VTSS_M_DSM_CFG_RATE_CTRL_TAXI_RATE_HIGH     VTSS_ENCODE_BITMASK(12,12)
#define  VTSS_X_DSM_CFG_RATE_CTRL_TAXI_RATE_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,12,12)

/** 
 * \brief
 * The transmission rate for this port when push-back from port is
 * activated.
 * 
 * The value must be calculated from the formula:
 * 
 * TAXI_RATE_LOW = FLOOR((nominal port_speed - 200 ppm)* 2^11 / (core_freq
 * * bus_width))
 * 
 * Note: For port speeds that allow HDX mode the TAXI_RATE_LOW must be set
 * to 0 to completely stop traffic in case of backing off.
 *
 * \details 
 * Field: ::VTSS_DSM_CFG_RATE_CTRL . TAXI_RATE_LOW
 */
#define  VTSS_F_DSM_CFG_RATE_CTRL_TAXI_RATE_LOW(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_DSM_CFG_RATE_CTRL_TAXI_RATE_LOW     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_DSM_CFG_RATE_CTRL_TAXI_RATE_LOW(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \brief Rate control watermark of dataflow to devices
 *
 * \details
 * Configuration of the watermark in the rate control for the data flow
 * from the DSM towards the device.
 *
 * Register: \a DSM:CFG:RATE_CTRL_WM
 */
#define VTSS_DSM_CFG_RATE_CTRL_WM            VTSS_IOREG(VTSS_TO_DSM,0x2d9)

/** 
 * \brief
 * This watermark sets the bucket level that has to be reached, for taxi
 * 128 and the device NPI attached to it, at the start of a burst before
 * the rate is controlled.
 *
 * \details 
 * bucket_level < value: Data transmit allowed
 * bucket level >= value: Data transmit controlled
 *
 * Field: ::VTSS_DSM_CFG_RATE_CTRL_WM . TAXI_128_RATE_CTRL_WM_DEVNPI
 */
#define  VTSS_F_DSM_CFG_RATE_CTRL_WM_TAXI_128_RATE_CTRL_WM_DEVNPI(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_DSM_CFG_RATE_CTRL_WM_TAXI_128_RATE_CTRL_WM_DEVNPI     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_DSM_CFG_RATE_CTRL_WM_TAXI_128_RATE_CTRL_WM_DEVNPI(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * This watermark sets the bucket level that has to be reached, for taxi 32
 * and all device 1G attached to it, at the start of a burst before the
 * rate is controlled.
 *
 * \details 
 * bucket_level < value: Data transmit allowed
 * bucket level >= value: Data transmit controlled
 *
 * Field: ::VTSS_DSM_CFG_RATE_CTRL_WM . TAXI_32_RATE_CTRL_WM
 */
#define  VTSS_F_DSM_CFG_RATE_CTRL_WM_TAXI_32_RATE_CTRL_WM(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_DSM_CFG_RATE_CTRL_WM_TAXI_32_RATE_CTRL_WM     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_DSM_CFG_RATE_CTRL_WM_TAXI_32_RATE_CTRL_WM(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Rate control watermark of dataflow to device2G5s
 *
 * \details
 * Configuration of the watermark in the rate control for the data flow
 * from the DSM towards the device.
 *
 * Register: \a DSM:CFG:RATE_CTRL_WM_DEV2G5
 *
 * @param ri Replicator: x_DSM_NUM_OF_DEV2G5_PORTS (??), 0-8
 */
#define VTSS_DSM_CFG_RATE_CTRL_WM_DEV2G5(ri)  VTSS_IOREG(VTSS_TO_DSM,0x2da + (ri))

/** 
 * \brief
 * This watermark sets the bucket level that has to be reached, for taxi
 * 128 and all device 2G5 attached to it, at the start of a burst before
 * the rate is controlled.
 *
 * \details 
 * bucket_level < value: Data transmit allowed
 * bucket level >= value: Data transmit controlled
 *
 * Field: ::VTSS_DSM_CFG_RATE_CTRL_WM_DEV2G5 . TAXI_128_RATE_CTRL_WM_DEV2G5
 */
#define  VTSS_F_DSM_CFG_RATE_CTRL_WM_DEV2G5_TAXI_128_RATE_CTRL_WM_DEV2G5(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_DSM_CFG_RATE_CTRL_WM_DEV2G5_TAXI_128_RATE_CTRL_WM_DEV2G5     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_DSM_CFG_RATE_CTRL_WM_DEV2G5_TAXI_128_RATE_CTRL_WM_DEV2G5(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Rate control watermark of dataflow to device10Gs
 *
 * \details
 * Configuration of the watermark in the rate control for the data flow
 * from the DSM towards the device.
 *
 * Register: \a DSM:CFG:RATE_CTRL_WM_DEV10G
 *
 * @param ri Replicator: x_DSM_NUM_OF_DEV10G_PORTS (??), 0-3
 */
#define VTSS_DSM_CFG_RATE_CTRL_WM_DEV10G(ri)  VTSS_IOREG(VTSS_TO_DSM,0x2e3 + (ri))

/** 
 * \brief
 * This watermark sets the bucket level that has to be reached, for taxi
 * 128 at the respective device 10G attached to it, at the start of a burst
 * before the rate is controlled.
 *
 * \details 
 * bucket_level < value: Data transmit allowed
 * bucket level >= value: Data transmit controlled
 *
 * Field: ::VTSS_DSM_CFG_RATE_CTRL_WM_DEV10G . TAXI_128_RATE_CTRL_WM
 */
#define  VTSS_F_DSM_CFG_RATE_CTRL_WM_DEV10G_TAXI_128_RATE_CTRL_WM(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_DSM_CFG_RATE_CTRL_WM_DEV10G_TAXI_128_RATE_CTRL_WM     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_DSM_CFG_RATE_CTRL_WM_DEV10G_TAXI_128_RATE_CTRL_WM(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Configuration register for IPG shrink mode
 *
 * \details
 * Configuration register for IPG shrink mode
 *
 * Register: \a DSM:CFG:IPG_SHRINK_CFG
 */
#define VTSS_DSM_CFG_IPG_SHRINK_CFG          VTSS_IOREG(VTSS_TO_DSM,0x2e7)

/** 
 * \brief
 * Enable for preamble shrink in IPG shrink mode of DEV10G.
 * Preamble shrink is only allowed when IPG shrink mode is enabled.
 * Gap compensation is dependent on frame size.
 * If enabled, it incudes one byte for the preamble. RATE_CTRL,FRM_GAP_COMP
 * is not regarded.
 * If disabled, gap compensation is according to RATE_CTRL,FRM_GAP_COMP.
 *  - If the preamble is forwarded on the taxi bus, RATE_CTRL,FRM_GAP_COMP
 * must be set to 0.
 *  - If the preamble is not forwarded on the taxi bus, it must be
 * compensated and RATE_CTRL::FRM_GAP_COMP must be set to 8.

 *
 * \details 
 * One bit per device.

 *
 * Field: ::VTSS_DSM_CFG_IPG_SHRINK_CFG . IPG_PREAM_SHRINK_ENA
 */
#define  VTSS_F_DSM_CFG_IPG_SHRINK_CFG_IPG_PREAM_SHRINK_ENA(x)  VTSS_ENCODE_BITFIELD(x,16,4)
#define  VTSS_M_DSM_CFG_IPG_SHRINK_CFG_IPG_PREAM_SHRINK_ENA     VTSS_ENCODE_BITMASK(16,4)
#define  VTSS_X_DSM_CFG_IPG_SHRINK_CFG_IPG_PREAM_SHRINK_ENA(x)  VTSS_EXTRACT_BITFIELD(x,16,4)

/** 
 * \brief
 * Enable for IPG shrink mode of DEV10G.
 * In shrink mode frame gap compensation is depending on frame size.

 *
 * \details 
 * One bit per device.

 *
 * Field: ::VTSS_DSM_CFG_IPG_SHRINK_CFG . IPG_SHRINK_ENA
 */
#define  VTSS_F_DSM_CFG_IPG_SHRINK_CFG_IPG_SHRINK_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_DSM_CFG_IPG_SHRINK_CFG_IPG_SHRINK_ENA     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_DSM_CFG_IPG_SHRINK_CFG_IPG_SHRINK_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief Clear a single buffer in the DSM.
 *
 * \details
 * This register controls clearing of buffers in the DSM.
 *
 * Register: \a DSM:CFG:CLR_BUF
 *
 * @param ri Replicator: x_DSM_PORTS_DIV32_CEIL (??), 0-1
 */
#define VTSS_DSM_CFG_CLR_BUF(ri)             VTSS_IOREG(VTSS_TO_DSM,0x2e9 + (ri))


/** 
 * \brief SCH stop fill level
 *
 * \details
 * SCH stop fill level
 *
 * Register: \a DSM:CFG:SCH_STOP_WM_CFG
 *
 * @param ri Replicator: x_DSM_NON_CPU_PORTS (??), 0-31
 */
#define VTSS_DSM_CFG_SCH_STOP_WM_CFG(ri)     VTSS_IOREG(VTSS_TO_DSM,0x2eb + (ri))

/** 
 * \brief
 * DSM buffer fill level at which SCH is stopped to send to this device.
 *
 * \details 
 * The SCH is stopped when the fill level is above the configured value.
 *
 * Field: ::VTSS_DSM_CFG_SCH_STOP_WM_CFG . SCH_STOP_WM
 */
#define  VTSS_F_DSM_CFG_SCH_STOP_WM_CFG_SCH_STOP_WM(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_DSM_CFG_SCH_STOP_WM_CFG_SCH_STOP_WM     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_DSM_CFG_SCH_STOP_WM_CFG_SCH_STOP_WM(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief Pause frame handling in RX direction
 *
 * \details
 * Pause frame handling in RX direction
 *
 * Register: \a DSM:CFG:RX_PAUSE_CFG
 *
 * @param ri Replicator: x_DSM_ETHERNET_PORTS (??), 0-31
 */
#define VTSS_DSM_CFG_RX_PAUSE_CFG(ri)        VTSS_IOREG(VTSS_TO_DSM,0x30b + (ri))

/** 
 * \brief
 * Rx Pause Enable: Enables Flow Control in Rx direction:
 *
 * \details 
 * '0': Flow Control Disabled
 * '1': Flow Control Enabled.
 *
 * Field: ::VTSS_DSM_CFG_RX_PAUSE_CFG . RX_PAUSE_EN
 */
#define  VTSS_F_DSM_CFG_RX_PAUSE_CFG_RX_PAUSE_EN  VTSS_BIT(1)

/** 
 * \brief
 * Configures for each Ethernet port whether flow control is obeyed locally
 * in the DSM or if the flow_control information is sent to the SCH.
 * If not all priorities should obey flow control for this port, then the
 * DSM should not stop the data stream in a flow control case, but let the
 * SCH stop the traffic to avoid head of line blocking.
 * If all priorities should obey flow control for this port, then the DSM
 * should stop the data stream locally to be able to pass 802.3 conformance
 * testing. If the data stream is stopped by the SCH frames in the OQS can
 * not be aged, thus the Allocation Bitmaps in the QS must not recover lost
 * elements.
 *
 * \details 
 * '0': This Ethernet port obeys flow control locally in the DSM. 
 * '1': This Ethernet port sends flow control information to the SCH and
 * does not stop traffic in the DSM.
 *
 * Field: ::VTSS_DSM_CFG_RX_PAUSE_CFG . FC_OBEY_LOCAL
 */
#define  VTSS_F_DSM_CFG_RX_PAUSE_CFG_FC_OBEY_LOCAL  VTSS_BIT(0)


/** 
 * \brief FC configuration for Ethernet ports.
 *
 * \details
 * FC configuration for Ethernet ports.
 *
 * Register: \a DSM:CFG:ETH_FC_GEN
 *
 * @param ri Replicator: x_DSM_ETHERNET_PORTS (??), 0-31
 */
#define VTSS_DSM_CFG_ETH_FC_GEN(ri)          VTSS_IOREG(VTSS_TO_DSM,0x32b + (ri))

/** 
 * \brief
 * Controls the generation of FC based on port level status for out of band
 * XAUI host status interface.
 *
 * \details 
 * '0': FC is not generated based on OOB XAUI host status
 * '1': FC is generated based on OOB XAUI host status
 *
 * Field: ::VTSS_DSM_CFG_ETH_FC_GEN . XAUI_EXT_REACH_FC_GEN
 */
#define  VTSS_F_DSM_CFG_ETH_FC_GEN_XAUI_EXT_REACH_FC_GEN(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_DSM_CFG_ETH_FC_GEN_XAUI_EXT_REACH_FC_GEN     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_DSM_CFG_ETH_FC_GEN_XAUI_EXT_REACH_FC_GEN(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * Controls the generation of FC based on a mixture port and buffer level
 * status from IQS.
 * 
 * FC is generated, if buf_hwm and the port_swm is crossed.
 * FC is deasserted if either the buffer filling level falls below the
 * buf_lwm or SWM_CROSSED status is cleared. (See
 * IQS_SWM_PORT_STATUS::IQS_SWM_PORT_STATUS)
 *
 * \details 
 * '0': FC is not generated based on the mixture of port and buffer level
 * '1': FC is generated based on the mixture of port and buffer level
 *
 * Field: ::VTSS_DSM_CFG_ETH_FC_GEN . ETH_PORT_BUF_FC_GEN
 */
#define  VTSS_F_DSM_CFG_ETH_FC_GEN_ETH_PORT_BUF_FC_GEN  VTSS_BIT(3)

/** 
 * \brief
 * Controls generation of FC based on status from port level policer.
 *
 * \details 
 * '0': FC is not generated based on port level policer status
 * '1': FC is generated based on port level policer status
 *
 * Field: ::VTSS_DSM_CFG_ETH_FC_GEN . ETH_POLICED_PORT_FC_GEN
 */
#define  VTSS_F_DSM_CFG_ETH_FC_GEN_ETH_POLICED_PORT_FC_GEN  VTSS_BIT(2)

/** 
 * \brief
 * Controls generation of FC based on buffer level status from IQS.
 *
 * \details 
 * '0': FC is not generated based on buffer level
 * '1': FC is generated based on buffer level
 *
 * Field: ::VTSS_DSM_CFG_ETH_FC_GEN . ETH_BUF_FC_GEN
 */
#define  VTSS_F_DSM_CFG_ETH_FC_GEN_ETH_BUF_FC_GEN  VTSS_BIT(1)

/** 
 * \brief
 * Controls the generation of FC based on port level status from IQS.
 *
 * \details 
 * '0': FC is not generated based on port level
 * '1': FC is generated based on port level
 *
 * Field: ::VTSS_DSM_CFG_ETH_FC_GEN . ETH_PORT_FC_GEN
 */
#define  VTSS_F_DSM_CFG_ETH_FC_GEN_ETH_PORT_FC_GEN  VTSS_BIT(0)


/** 
 * \brief MAC Configuration Register.
 *
 * \details
 * Contains configuration for flowcontrol and operation in FDX or HDX for
 * Ethernet ports connected to the DSM.
 *
 * Register: \a DSM:CFG:MAC_CFG
 *
 * @param ri Replicator: x_DSM_ETHERNET_PORTS (??), 0-31
 */
#define VTSS_DSM_CFG_MAC_CFG(ri)             VTSS_IOREG(VTSS_TO_DSM,0x34b + (ri))

/** 
 * \brief
 * Tx Pause Value: The pause value inserted in each Pause Control Frame in
 * the Tx direction. It is also used to schedule the transmission of Pause
 * Control Frames when Flow Control is enabled and active. If flow control
 * is enabled, the pause value must be set to a value different from 0,
 * based on the link characteristics.
 *
 * \details 
 * 0x0000: 0 quanta (512-bit times) 
 * 0x0001: 1 quanta
 * ...
 * 0xFFFF: 64K-1 quanta.
 *
 * Field: ::VTSS_DSM_CFG_MAC_CFG . TX_PAUSE_VAL
 */
#define  VTSS_F_DSM_CFG_MAC_CFG_TX_PAUSE_VAL(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_DSM_CFG_MAC_CFG_TX_PAUSE_VAL     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_DSM_CFG_MAC_CFG_TX_PAUSE_VAL(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Enables HDX backpressure instead of FDX FC when FC is generated.
 * Note: 10G and 24G ports can not run HDX, so for those ports this field
 * must always be set to '0'.
 *
 * \details 
 * '0': FDX FC is used.
 * '1': HDX backpreassure is used.
 *
 * Field: ::VTSS_DSM_CFG_MAC_CFG . HDX_BACKPRESSURE
 */
#define  VTSS_F_DSM_CFG_MAC_CFG_HDX_BACKPRESSURE  VTSS_BIT(2)

/** 
 * \brief
 * TX pause zero on de-assert. Determines whether or not a pause control
 * frame with pause value zero is transmitted when flow control is
 * de-asserted.
 *
 * \details 
 * '0': No pause frame with value zero is transmitted.
 * '1': Pause frame with value zero is transmitted when flow control
 * becomes inactive.
 *
 * Field: ::VTSS_DSM_CFG_MAC_CFG . TX_PAUSE_XON_XOFF
 */
#define  VTSS_F_DSM_CFG_MAC_CFG_TX_PAUSE_XON_XOFF  VTSS_BIT(0)


/** 
 * \brief MAC Address Base Configuration Register - bits 47-24.
 *
 * \details
 * MAC base address.
 * Used when generating Pause Control Frames with the specified MAC
 * address.
 *
 * Register: \a DSM:CFG:MAC_ADDR_BASE_HIGH_CFG
 *
 * @param ri Replicator: x_DSM_NUM_PAUSE_FRM_MAC_ADDR (??), 0-31
 */
#define VTSS_DSM_CFG_MAC_ADDR_BASE_HIGH_CFG(ri)  VTSS_IOREG(VTSS_TO_DSM,0x36b + (ri))

/** 
 * \brief
 * Bits 47-24 of MAC base address.
 * Used when generating Pause Control Frames with the specified MAC
 * address.
 *
 * \details 
 * Field: ::VTSS_DSM_CFG_MAC_ADDR_BASE_HIGH_CFG . MAC_ADDR_HIGH
 */
#define  VTSS_F_DSM_CFG_MAC_ADDR_BASE_HIGH_CFG_MAC_ADDR_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_DSM_CFG_MAC_ADDR_BASE_HIGH_CFG_MAC_ADDR_HIGH     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_DSM_CFG_MAC_ADDR_BASE_HIGH_CFG_MAC_ADDR_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief MAC Address Base Configuration Register - bits 23-0.
 *
 * \details
 * MAC base address.
 * Used when generating Pause Control Frames with the specified MAC
 * address.
 *
 * Register: \a DSM:CFG:MAC_ADDR_BASE_LOW_CFG
 *
 * @param ri Replicator: x_DSM_NUM_PAUSE_FRM_MAC_ADDR (??), 0-31
 */
#define VTSS_DSM_CFG_MAC_ADDR_BASE_LOW_CFG(ri)  VTSS_IOREG(VTSS_TO_DSM,0x38b + (ri))

/** 
 * \brief
 * Bits 23-0 of MAC base address.
 * Used when generating Pause Control Frames with the specified MAC
 * address.
 *
 * \details 
 * Field: ::VTSS_DSM_CFG_MAC_ADDR_BASE_LOW_CFG . MAC_ADDR_LOW
 */
#define  VTSS_F_DSM_CFG_MAC_ADDR_BASE_LOW_CFG_MAC_ADDR_LOW(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_DSM_CFG_MAC_ADDR_BASE_LOW_CFG_MAC_ADDR_LOW     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_DSM_CFG_MAC_ADDR_BASE_LOW_CFG_MAC_ADDR_LOW(x)  VTSS_EXTRACT_BITFIELD(x,0,24)

/**
 * Register Group: \a DSM:CFG_STAT
 *
 * Combined configuration and status registers
 */

/**
 * Register Group: \a DSM:STATUS
 *
 * Status registers
 */


/** 
 * \brief Counts number of aged frames.
 *
 * \details
 * This counter holds the number of aged frames from this port.
 *
 * Register: \a DSM:STATUS:AGED_FRMS
 *
 * @param ri Replicator: x_DSM_NUM_AGED_FRM_CNTRS (??), 0-32
 */
#define VTSS_DSM_STATUS_AGED_FRMS(ri)        VTSS_IOREG(VTSS_TO_DSM,0x5e0 + (ri))


/** 
 * \brief Sticky bits for the DSM cell bus interface.
 *
 * \details
 * This register holds sticky bit information for the cell bus interface of
 * the DSM.
 *
 * Register: \a DSM:STATUS:CELL_BUS_STICKY
 */
#define VTSS_DSM_STATUS_CELL_BUS_STICKY      VTSS_IOREG(VTSS_TO_DSM,0x601)

/** 
 * \brief
 * This sticky bit is set if minimum cell spacing for a taxi interface is
 * violated.
 * Note that this may cause frame corruption.
 *
 * \details 
 * '0': No violation detected.
 * '1': Violation detected.
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DSM_STATUS_CELL_BUS_STICKY . CONS_CELL_FOR_SAME_TAXI_STICKY
 */
#define  VTSS_F_DSM_STATUS_CELL_BUS_STICKY_CONS_CELL_FOR_SAME_TAXI_STICKY  VTSS_BIT(2)


/** 
 * \brief Buffer Overflow
 *
 * \details
 * Buffer Overflow
 *
 * Register: \a DSM:STATUS:BUF_OFLW_STICKY
 */
#define VTSS_DSM_STATUS_BUF_OFLW_STICKY      VTSS_IOREG(VTSS_TO_DSM,0x602)


/** 
 * \brief Buffer Underrun
 *
 * \details
 * Buffer Underrun
 *
 * Register: \a DSM:STATUS:BUF_UFLW_STICKY
 */
#define VTSS_DSM_STATUS_BUF_UFLW_STICKY      VTSS_IOREG(VTSS_TO_DSM,0x603)

/**
 * Register Group: \a DSM:CM_CFG
 *
 * VStax2 Congestion Management configuration registers
 */


/** 
 * \brief Mask for generating CMM info
 *
 * \details
 * Mask for generating CMM info
 *
 * Register: \a DSM:CM_CFG:CMM_MASK_CFG
 */
#define VTSS_DSM_CM_CFG_CMM_MASK_CFG         VTSS_IOREG(VTSS_TO_DSM,0x3b6)


/** 
 * \brief Mask for generating CMM info on queue level
 *
 * \details
 * Mask for generating CMM info on queue level
 *
 * Register: \a DSM:CM_CFG:CMM_QU_MASK_CFG
 *
 * @param ri Replicator: x_NUM_FRONT_PORTS_DIV32_CEIL_x_PRIOS (??), 0-7
 */
#define VTSS_DSM_CM_CFG_CMM_QU_MASK_CFG(ri)  VTSS_IOREG(VTSS_TO_DSM,0x3b7 + (ri))


/** 
 * \brief Mask for generating CMM info for host queues
 *
 * \details
 * Mask for generating CMM info for host queues
 *
 * Register: \a DSM:CM_CFG:CMM_HOST_QU_MASK_CFG
 *
 * @param ri Replicator: x_NUM_HOST_QUEUES_DIV32_CEIL (??), 0-5
 */
#define VTSS_DSM_CM_CFG_CMM_HOST_QU_MASK_CFG(ri)  VTSS_IOREG(VTSS_TO_DSM,0x3bf + (ri))


/** 
 * \brief Mask for generating CMM info for logical host ports
 *
 * \details
 * Mask for generating CMM info for logical host ports
 *
 * Register: \a DSM:CM_CFG:CMM_HOST_LPORT_MASK_CFG
 *
 * @param ri Replicator: x_DSM_HOST_LPORTS_DIV32_CEIL (??), 0-1
 */
#define VTSS_DSM_CM_CFG_CMM_HOST_LPORT_MASK_CFG(ri)  VTSS_IOREG(VTSS_TO_DSM,0x3c5 + (ri))


/** 
 * \brief Configuration register for Host mode backpressure to ARB
 *
 * \details
 * Configuration register to define how resource control counters influence
 * backpreasure signal generation towards the ARB.
 *
 * Register: \a DSM:CM_CFG:HOST_PORT_RESRC_CTL_CFG
 */
#define VTSS_DSM_CM_CFG_HOST_PORT_RESRC_CTL_CFG  VTSS_IOREG(VTSS_TO_DSM,0x3c7)

/** 
 * \brief
 * Defines the number of priorities per Lport.
 *
 * \details 
 * 0: 1 x 48 Lports x 4 priorities or 2 x 24 Lports x 4 priorities
 * 1: 1 x 24 Lports x 8 priorities or 2 x 12 Lports x 8 priorities
 *
 * Field: ::VTSS_DSM_CM_CFG_HOST_PORT_RESRC_CTL_CFG . HOST_PORT_PRIO_CFG
 */
#define  VTSS_F_DSM_CM_CFG_HOST_PORT_RESRC_CTL_CFG_HOST_PORT_PRIO_CFG  VTSS_BIT(0)


/** 
 * \brief Register to control forward of CMMs to ARB
 *
 * \details
 * Register to control forward of CMMs to ARB
 *
 * Register: \a DSM:CM_CFG:CMM_TO_ARB_CFG
 */
#define VTSS_DSM_CM_CFG_CMM_TO_ARB_CFG       VTSS_IOREG(VTSS_TO_DSM,0x3c8)

/** 
 * \brief
 * Enable CMM extraction. One bit per extraction source (corresponds to
 * relay buffer number).
 * If none of the sources is enabled, all UPSIDs should be set to inactive
 * which will cause idle CMMs to be forwarded to the ARB in order to clear
 * all CM status.
 *
 * \details 
 * '0': CMM extraction is disabled
 * '1': CMM extraction is enabled
 *
 * Field: ::VTSS_DSM_CM_CFG_CMM_TO_ARB_CFG . CMM_XTRCT_ENA
 */
#define  VTSS_F_DSM_CM_CFG_CMM_TO_ARB_CFG_CMM_XTRCT_ENA(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_DSM_CM_CFG_CMM_TO_ARB_CFG_CMM_XTRCT_ENA     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_DSM_CM_CFG_CMM_TO_ARB_CFG_CMM_XTRCT_ENA(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Defines the mode how own CMM is forwarded to the ARB.
 * Direct mode: CMM is directly send to the ARB.
 * Indirect mode: CMM is send out and expected to be received in a CMEF
 * again before it is forwarded to the ARB.
 * Note: In Indirect mode the UPSID is own UPSID check for received CMMs is
 * disabled.
 *
 * \details 
 * '0': Direct mode
 * '1': Indirect mode
 *
 * Field: ::VTSS_DSM_CM_CFG_CMM_TO_ARB_CFG . CMM_TO_ARB_MODE
 */
#define  VTSS_F_DSM_CM_CFG_CMM_TO_ARB_CFG_CMM_TO_ARB_MODE  VTSS_BIT(1)

/** 
 * \brief
 * Delay CMM to ARB until corresponding CMEF is generated.
 *
 * \details 
 * '0': CMM to ARB is not delayed
 * '1': CMM to ARB is delayed
 *
 * Field: ::VTSS_DSM_CM_CFG_CMM_TO_ARB_CFG . CMM_TO_ARB_DELAY_ENA
 */
#define  VTSS_F_DSM_CM_CFG_CMM_TO_ARB_CFG_CMM_TO_ARB_DELAY_ENA  VTSS_BIT(0)


/** 
 * \brief Register to config max and min interval for CMEF generation.
 *
 * \details
 * Register to config max and min interval for CMEF generation.
 *
 * Register: \a DSM:CM_CFG:CMEF_RATE_CFG
 */
#define VTSS_DSM_CM_CFG_CMEF_RATE_CFG        VTSS_IOREG(VTSS_TO_DSM,0x3c9)

/** 
 * \brief
 * Maximum interval between two CMEFs.
 * Must always be above CMEF_MIN_INTERVAL, if not disabled.
 *
 * \details 
 * Resolution is in 16 clock cycles.
 * T = n * 16 / core clock frequency.
 * A value of 0 disables CMEF generation after max. interval.
 * 
 * Note: This register is intended for static setup. Changes made may not
 * have impact until the next CMEF is generated.
 * 
 * Note: When changing the value from disabled (0) to other, a CMEF will
 * immediately be generated.
 *
 * Field: ::VTSS_DSM_CM_CFG_CMEF_RATE_CFG . CMEF_MAX_INTERVAL
 */
#define  VTSS_F_DSM_CM_CFG_CMEF_RATE_CFG_CMEF_MAX_INTERVAL(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_DSM_CM_CFG_CMEF_RATE_CFG_CMEF_MAX_INTERVAL     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_DSM_CM_CFG_CMEF_RATE_CFG_CMEF_MAX_INTERVAL(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Minimum interval between two CMEFs.
 * Must always be below CMEF_MAX_INTERVAL
 *
 * \details 
 * Resolution is in 16 clock cycles.
 * T = n * 16 / core clock frequency.
 * 
 * Note: This register is intended for static setup. Changes made may not
 * have impact until interval once has expired.
 *
 * Field: ::VTSS_DSM_CM_CFG_CMEF_RATE_CFG . CMEF_MIN_INTERVAL
 */
#define  VTSS_F_DSM_CM_CFG_CMEF_RATE_CFG_CMEF_MIN_INTERVAL(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DSM_CM_CFG_CMEF_RATE_CFG_CMEF_MIN_INTERVAL     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DSM_CM_CFG_CMEF_RATE_CFG_CMEF_MIN_INTERVAL(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief CMEF generation config register
 *
 * \details
 * CMEF generation config register
 * The address of the configuration is based on the following layout:
 *  0: Configuration for physical port number 24
 *  1: Configuration for physical port number 25
 *  2: Configuration for physical port number 26
 *  3: Configuration for physical port number 27
 *  4: Configuration for physical port number 28
 *  5: Configuration for physical port number 29
 *  6: Configuration for physical port number 30
 *
 * Register: \a DSM:CM_CFG:CMEF_GEN_CFG
 *
 * @param ri Replicator: x_NUM_POT_STACK_PORTS (??), 0-6
 */
#define VTSS_DSM_CM_CFG_CMEF_GEN_CFG(ri)     VTSS_IOREG(VTSS_TO_DSM,0x3ca + (ri))

/** 
 * \brief
 * Enable CMEF relay transmit.
 *
 * \details 
 * '0': Transmit disabled
 * '1': Transmit enabled
 *
 * Field: ::VTSS_DSM_CM_CFG_CMEF_GEN_CFG . CMEF_RELAY_TX_ENA
 */
#define  VTSS_F_DSM_CM_CFG_CMEF_GEN_CFG_CMEF_RELAY_TX_ENA  VTSS_BIT(6)

/** 
 * \brief
 * Time To Live
 *
 * \details 
 * Field: ::VTSS_DSM_CM_CFG_CMEF_GEN_CFG . CMEF_TTL
 */
#define  VTSS_F_DSM_CM_CFG_CMEF_GEN_CFG_CMEF_TTL(x)  VTSS_ENCODE_BITFIELD(x,1,5)
#define  VTSS_M_DSM_CM_CFG_CMEF_GEN_CFG_CMEF_TTL     VTSS_ENCODE_BITMASK(1,5)
#define  VTSS_X_DSM_CM_CFG_CMEF_GEN_CFG_CMEF_TTL(x)  VTSS_EXTRACT_BITFIELD(x,1,5)

/** 
 * \brief
 * Enable Generation of CMEFs for resp. Stack port.
 *
 * \details 
 * '0': Generation of CMEFs disabled
 * '1': Generation of CMEFs enabled
 *
 * Field: ::VTSS_DSM_CM_CFG_CMEF_GEN_CFG . CMEF_GEN_ENA
 */
#define  VTSS_F_DSM_CM_CFG_CMEF_GEN_CFG_CMEF_GEN_ENA  VTSS_BIT(0)


/** 
 * \brief CMEF DMAC address lower 24 bits
 *
 * \details
 * CMEF DMAC address lower 24 bits
 *
 * Register: \a DSM:CM_CFG:CMEF_MAC_ADDR_LOW_CFG
 */
#define VTSS_DSM_CM_CFG_CMEF_MAC_ADDR_LOW_CFG  VTSS_IOREG(VTSS_TO_DSM,0x3d1)

/** 
 * \brief
 * CMEF_MAC_ADDR(23 downto 0)
 *
 * \details 
 * Field: ::VTSS_DSM_CM_CFG_CMEF_MAC_ADDR_LOW_CFG . CMEF_MAC_ADDR_LOW
 */
#define  VTSS_F_DSM_CM_CFG_CMEF_MAC_ADDR_LOW_CFG_CMEF_MAC_ADDR_LOW(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_DSM_CM_CFG_CMEF_MAC_ADDR_LOW_CFG_CMEF_MAC_ADDR_LOW     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_DSM_CM_CFG_CMEF_MAC_ADDR_LOW_CFG_CMEF_MAC_ADDR_LOW(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief CMEF DMAC address upper 24 bits
 *
 * \details
 * CMEF DMAC address upper 24 bits
 *
 * Register: \a DSM:CM_CFG:CMEF_MAC_ADDR_HIGH_CFG
 */
#define VTSS_DSM_CM_CFG_CMEF_MAC_ADDR_HIGH_CFG  VTSS_IOREG(VTSS_TO_DSM,0x3d2)

/** 
 * \brief
 * CMEF_MAC_ADDR(47 downto 24)
 *
 * \details 
 * Field: ::VTSS_DSM_CM_CFG_CMEF_MAC_ADDR_HIGH_CFG . CMEF_MAC_ADDR_HIGH
 */
#define  VTSS_F_DSM_CM_CFG_CMEF_MAC_ADDR_HIGH_CFG_CMEF_MAC_ADDR_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_DSM_CM_CFG_CMEF_MAC_ADDR_HIGH_CFG_CMEF_MAC_ADDR_HIGH     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_DSM_CM_CFG_CMEF_MAC_ADDR_HIGH_CFG_CMEF_MAC_ADDR_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief Register to hold the own UPSID
 *
 * \details
 * Register to hold the own UPSID
 *
 * Register: \a DSM:CM_CFG:CMEF_OWN_UPSID_CFG
 */
#define VTSS_DSM_CM_CFG_CMEF_OWN_UPSID_CFG   VTSS_IOREG(VTSS_TO_DSM,0x3d3)

/** 
 * \brief
 * Defines a bit mask for the own UPSID check
 *
 * \details 
 * '0': All bits from UPSID are relevant for own UPSID check
 * '1': Only bits 4 ..1 are relevant for own UPSID check
 * '2': Only bits 4 ..2 are relevant for own UPSID check
 * '3': Only bits 4 ..3 are relevant for own UPSID check
 * '4': Only bit 4 is relevant for own UPSID check
 * '5..7': reserved.
 *
 * Field: ::VTSS_DSM_CM_CFG_CMEF_OWN_UPSID_CFG . HM_OWN_UPSID_MASK
 */
#define  VTSS_F_DSM_CM_CFG_CMEF_OWN_UPSID_CFG_HM_OWN_UPSID_MASK(x)  VTSS_ENCODE_BITFIELD(x,16,3)
#define  VTSS_M_DSM_CM_CFG_CMEF_OWN_UPSID_CFG_HM_OWN_UPSID_MASK     VTSS_ENCODE_BITMASK(16,3)
#define  VTSS_X_DSM_CM_CFG_CMEF_OWN_UPSID_CFG_HM_OWN_UPSID_MASK(x)  VTSS_EXTRACT_BITFIELD(x,16,3)

/** 
 * \brief
 * Own Unit Port Set Identifier
 *
 * \details 
 * Field: ::VTSS_DSM_CM_CFG_CMEF_OWN_UPSID_CFG . CMEF_OWN_UPSID
 */
#define  VTSS_F_DSM_CM_CFG_CMEF_OWN_UPSID_CFG_CMEF_OWN_UPSID(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_DSM_CM_CFG_CMEF_OWN_UPSID_CFG_CMEF_OWN_UPSID     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_DSM_CM_CFG_CMEF_OWN_UPSID_CFG_CMEF_OWN_UPSID(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief Register to define active UPSIDs
 *
 * \details
 * Register to define active UPSIDs
 *
 * Register: \a DSM:CM_CFG:CMEF_UPSID_ACTIVE_CFG
 */
#define VTSS_DSM_CM_CFG_CMEF_UPSID_ACTIVE_CFG  VTSS_IOREG(VTSS_TO_DSM,0x3d4)


/** 
 * \brief Logical port number mapping registers
 *
 * \details
 * Logical port number mapping registers
 *
 * Register: \a DSM:CM_CFG:LPORT_NUM_CFG
 *
 * @param ri Replicator: x_DSM_ETHERNET_PORTS (??), 0-31
 */
#define VTSS_DSM_CM_CFG_LPORT_NUM_CFG(ri)    VTSS_IOREG(VTSS_TO_DSM,0x3d5 + (ri))

/** 
 * \brief
 * Logical port number to be used in relation to generating CM info based
 * on physical port number. This is needed for CMEF generation in
 * conjunction with link aggregation. The logical port number should always
 * be set to the lowest physical port participating in the aggregation
 * group.
 * For ports not aggregated, logical port number and physical port number 
 * must be identical.
 *
 * \details 
 * 0: Logical port 0
 * 1: Logical port 1
 * ...
 * n: Logical port n.
 *
 * Field: ::VTSS_DSM_CM_CFG_LPORT_NUM_CFG . LPORT_NUM
 */
#define  VTSS_F_DSM_CM_CFG_LPORT_NUM_CFG_LPORT_NUM(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_DSM_CM_CFG_LPORT_NUM_CFG_LPORT_NUM     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_DSM_CM_CFG_LPORT_NUM_CFG_LPORT_NUM(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief Shot register for Logical port number mapping table initialization
 *
 * \details
 * Shot register for Logical port number mapping table initialization
 *
 * Register: \a DSM:CM_CFG:LPORT_NUM_INIT
 */
#define VTSS_DSM_CM_CFG_LPORT_NUM_INIT       VTSS_IOREG(VTSS_TO_DSM,0x3f5)

/** 
 * \brief
 * Bitgroup to start (re)initialization of the logical port number mapping
 * table. Table is initialized to have a 1 to 1 mapping.
 *
 * \details 
 * '0': No action
 * '1': Logical Port Number mapping table is initialized (Bit is
 * automatically cleared)
 *
 * Field: ::VTSS_DSM_CM_CFG_LPORT_NUM_INIT . LPORT_NUM_INIT_SHOT
 */
#define  VTSS_F_DSM_CM_CFG_LPORT_NUM_INIT_LPORT_NUM_INIT_SHOT  VTSS_BIT(0)

/**
 * Register Group: \a DSM:CM_STATUS
 *
 * VStax2 Congestion Management status and sticky bit registers
 */


/** 
 * \brief Relay buffer overflow sticky register
 *
 * \details
 * Relay buffer overflow sticky register
 *
 * Register: \a DSM:CM_STATUS:CMEF_RELAY_BUF_OFLW_STICKY
 */
#define VTSS_DSM_CM_STATUS_CMEF_RELAY_BUF_OFLW_STICKY  VTSS_IOREG(VTSS_TO_DSM,0x64f)

/** 
 * \brief
 * There was an overflow in the CMEF relay buffer
 *
 * \details 
 * '0': No overflow
 * '1': Overflow
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DSM_CM_STATUS_CMEF_RELAY_BUF_OFLW_STICKY . CMEF_RELAY_BUF_OFLW_STICKY
 */
#define  VTSS_F_DSM_CM_STATUS_CMEF_RELAY_BUF_OFLW_STICKY_CMEF_RELAY_BUF_OFLW_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_DSM_CM_STATUS_CMEF_RELAY_BUF_OFLW_STICKY_CMEF_RELAY_BUF_OFLW_STICKY     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_DSM_CM_STATUS_CMEF_RELAY_BUF_OFLW_STICKY_CMEF_RELAY_BUF_OFLW_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Counter for extracted CMMs.
 *
 * \details
 * Counter for extracted CMMs.
 *
 * Register: \a DSM:CM_STATUS:CMM_XTRCT_CNT
 *
 * @param ri Replicator: x_NUM_CMEF_RELAY_BUFS (??), 0-1
 */
#define VTSS_DSM_CM_STATUS_CMM_XTRCT_CNT(ri)  VTSS_IOREG(VTSS_TO_DSM,0x650 + (ri))

/** 
 * \brief
 * Counts the number of extracted CMMs presented to the ARB.
 *
 * \details 
 * Counter can be written by SW.
 *
 * Field: ::VTSS_DSM_CM_STATUS_CMM_XTRCT_CNT . CMM_XTRCT_CNT
 */
#define  VTSS_F_DSM_CM_STATUS_CMM_XTRCT_CNT_CMM_XTRCT_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DSM_CM_STATUS_CMM_XTRCT_CNT_CMM_XTRCT_CNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DSM_CM_STATUS_CMM_XTRCT_CNT_CMM_XTRCT_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Sticky bit register for CMMs failed verify
 *
 * \details
 * Sticky bit register for CMMs failed verify
 *
 * Register: \a DSM:CM_STATUS:CMM_XTRCT_STICKY
 */
#define VTSS_DSM_CM_STATUS_CMM_XTRCT_STICKY  VTSS_IOREG(VTSS_TO_DSM,0x652)

/** 
 * \brief
 * Verification of CMM has failed:
 * Either UPSID was not valid or active
 * (see CMEF_UPSID_ACTVE_CFG::CMEF_UPSID_ACTIVE)
 *  or
 * UPSID is own UPSID and mode was not indirect mode
 * (see CMM_TO_ARB_CFG::CMM_TO_ARB_MODE)
 *
 * \details 
 * '0': Verification of CMMs has not failed.
 * '1': Verification of at least one CMMs has failed.
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DSM_CM_STATUS_CMM_XTRCT_STICKY . CMM_XTRCT_STICKY
 */
#define  VTSS_F_DSM_CM_STATUS_CMM_XTRCT_STICKY_CMM_XTRCT_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_DSM_CM_STATUS_CMM_XTRCT_STICKY_CMM_XTRCT_STICKY     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_DSM_CM_STATUS_CMM_XTRCT_STICKY_CMM_XTRCT_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Sticky bit register for dropped CMEFs
 *
 * \details
 * Sticky bit register for dropped CMEFs
 *
 * Register: \a DSM:CM_STATUS:CMEF_DROP_STICKY
 *
 * @param ri Replicator: x_NUM_CMEF_RELAY_BUFS (??), 0-1
 */
#define VTSS_DSM_CM_STATUS_CMEF_DROP_STICKY(ri)  VTSS_IOREG(VTSS_TO_DSM,0x653 + (ri))

/** 
 * \brief
 * One or more CMEFs that should have been extracted were dropped because
 * it was abort marked.
 *
 * \details 
 * '0': No CMEF was dropped
 * '1': One or more CMEF were dropped
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DSM_CM_STATUS_CMEF_DROP_STICKY . CMEF_EXTRACT_DROP_STICKY
 */
#define  VTSS_F_DSM_CM_STATUS_CMEF_DROP_STICKY_CMEF_EXTRACT_DROP_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * One or more CMEFs that should have been relayed were dropped because it
 * was abort marked.
 *
 * \details 
 * '0': No CMEF was dropped
 * '1': One or more CMEF were dropped
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DSM_CM_STATUS_CMEF_DROP_STICKY . CMEF_RELAY_DROP_STICKY
 */
#define  VTSS_F_DSM_CM_STATUS_CMEF_DROP_STICKY_CMEF_RELAY_DROP_STICKY  VTSS_BIT(0)


/** 
 * \brief Counter that holds the number of relayed CMEFs from this buffer
 *
 * \details
 * Counter that holds the number of relayed CMEFs from this buffer
 *
 * Register: \a DSM:CM_STATUS:CMEF_RELAYED_STATUS
 *
 * @param ri Replicator: x_NUM_CMEF_RELAY_BUFS (??), 0-1
 */
#define VTSS_DSM_CM_STATUS_CMEF_RELAYED_STATUS(ri)  VTSS_IOREG(VTSS_TO_DSM,0x655 + (ri))

/** 
 * \brief
 * Counts the number of relayed and transmitted CMEFs.
 *
 * \details 
 * Counter can be written by SW.
 *
 * Field: ::VTSS_DSM_CM_STATUS_CMEF_RELAYED_STATUS . CMEF_RELAYED_CNT
 */
#define  VTSS_F_DSM_CM_STATUS_CMEF_RELAYED_STATUS_CMEF_RELAYED_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DSM_CM_STATUS_CMEF_RELAYED_STATUS_CMEF_RELAYED_CNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DSM_CM_STATUS_CMEF_RELAYED_STATUS_CMEF_RELAYED_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief This counter holds the number of generated CMEFs from this port
 *
 * \details
 * This counter holds the number of generated CMEFs from this port
 *
 * Register: \a DSM:CM_STATUS:CMEF_GENERATED_STATUS
 *
 * @param ri Replicator: x_NUM_CMEF_RELAY_BUFS (??), 0-1
 */
#define VTSS_DSM_CM_STATUS_CMEF_GENERATED_STATUS(ri)  VTSS_IOREG(VTSS_TO_DSM,0x657 + (ri))

/** 
 * \brief
 * Counts the number of generated CMEFs.
 *
 * \details 
 * Counter can be written by SW.
 *
 * Field: ::VTSS_DSM_CM_STATUS_CMEF_GENERATED_STATUS . CMEF_GENERATED_CNT
 */
#define  VTSS_F_DSM_CM_STATUS_CMEF_GENERATED_STATUS_CMEF_GENERATED_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DSM_CM_STATUS_CMEF_GENERATED_STATUS_CMEF_GENERATED_CNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DSM_CM_STATUS_CMEF_GENERATED_STATUS_CMEF_GENERATED_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Curren CM status.
 *
 * \details
 * Curren CM status.
 *
 * Register: \a DSM:CM_STATUS:CURRENT_CM_STATUS
 *
 * @param ri Replicator: x_NUM_CM_STATUS_INFOS_DIV32_CEIL (??), 0-14
 */
#define VTSS_DSM_CM_STATUS_CURRENT_CM_STATUS(ri)  VTSS_IOREG(VTSS_TO_DSM,0x659 + (ri))


/** 
 * \brief Curren CM status for LPorts.
 *
 * \details
 * Curren CM status for LPorts.
 *
 * Register: \a DSM:CM_STATUS:CURRENT_LPORT_CM_STATUS
 *
 * @param ri Replicator: x_DSM_HOST_LPORTS_DIV32_CEIL (??), 0-1
 */
#define VTSS_DSM_CM_STATUS_CURRENT_LPORT_CM_STATUS(ri)  VTSS_IOREG(VTSS_TO_DSM,0x668 + (ri))

/**
 * Register Group: \a DSM:SP_CFG
 *
 * Super-Priority configuration registers
 */


/** 
 * \brief Configuration register for disable of TTL decrementation
 *
 * \details
 * Configuration register for disable of TTL decrementation
 *
 * Register: \a DSM:SP_CFG:SP_KEEP_TTL_CFG
 */
#define VTSS_DSM_SP_CFG_SP_KEEP_TTL_CFG      VTSS_IOREG(VTSS_TO_DSM,0x67c)

/** 
 * \brief
 * Enable keeping of the SP VStaX2 Header TTL value.
 * Normally the TTL is decremented in the SP relay buffer. with
 * SP_KEEP_TTL_ENA is set, the TTL is not decremented.
 * This must be configured consistently across the following registers:
 * DSM::SP_KEEP_TTL_CFG.SP_KEEP_TTL_ENA
 * ASM::SP_KEEP_TTL_CFG.SP_KEEP_TTL_ENA
 *
 * \details 
 * '0': TTL of SP frames is decremented
 * '1': TTL of SP frames is kept
 *
 * Field: ::VTSS_DSM_SP_CFG_SP_KEEP_TTL_CFG . SP_KEEP_TTL_ENA
 */
#define  VTSS_F_DSM_SP_CFG_SP_KEEP_TTL_CFG_SP_KEEP_TTL_ENA  VTSS_BIT(0)


/** 
 * \brief Super-Priority transmit configuration register
 *
 * \details
 * Super-Priority transmit configuration register
 * The address of the configuration is based on the following layout:
 *  0: Configuration for physical port number 24
 *  1: Configuration for physical port number 25
 *  2: Configuration for physical port number 26
 *  3: Configuration for physical port number 27
 *  4: Configuration for physical port number 28
 *  5: Configuration for physical port number 29
 *  6: Configuration for physical port number 30
 *
 * Register: \a DSM:SP_CFG:SP_TX_CFG
 *
 * @param ri Replicator: x_NUM_POT_STACK_PORTS (??), 0-6
 */
#define VTSS_DSM_SP_CFG_SP_TX_CFG(ri)        VTSS_IOREG(VTSS_TO_DSM,0x67d + (ri))

/** 
 * \brief
 * Select the relay / inject buffer to get data from
 *
 * \details 
 * '0': Select relay/inject buffer 0
 * '1': Select relay/inject buffer 1
 *
 * Field: ::VTSS_DSM_SP_CFG_SP_TX_CFG . SP_TX_SEL
 */
#define  VTSS_F_DSM_SP_CFG_SP_TX_CFG_SP_TX_SEL  VTSS_BIT(1)

/** 
 * \brief
 * Enable transmission of Super-Priority traffic on a stack port. Common
 * enable for relay and injection traffic.
 *
 * \details 
 * '0': Super-Priority traffic disabled 
 * '1': Super-Priority traffic enabled
 *
 * Field: ::VTSS_DSM_SP_CFG_SP_TX_CFG . SP_TX_ENA
 */
#define  VTSS_F_DSM_SP_CFG_SP_TX_CFG_SP_TX_ENA  VTSS_BIT(0)


/** 
 * \brief Super-Priority extract configuration register
 *
 * \details
 * Super-Priority extract configuration register
 *
 * Register: \a DSM:SP_CFG:SP_XTRCT_CFG
 */
#define VTSS_DSM_SP_CFG_SP_XTRCT_CFG         VTSS_IOREG(VTSS_TO_DSM,0x684)

/** 
 * \brief
 * Enable extraction from Super-Priority queue.
 *
 * \details 
 * One bit per extraction buffer
 * '0': Extraction buffer is NOT enabled for frame extraction
 * '1': Extraction buffer is enabled for frame extraction
 *
 * Field: ::VTSS_DSM_SP_CFG_SP_XTRCT_CFG . SP_XTRCT_ENA
 */
#define  VTSS_F_DSM_SP_CFG_SP_XTRCT_CFG_SP_XTRCT_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_DSM_SP_CFG_SP_XTRCT_CFG_SP_XTRCT_ENA     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_DSM_SP_CFG_SP_XTRCT_CFG_SP_XTRCT_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Configuration register for injection on front ports
 *
 * \details
 * Configuration register for injection on front ports
 *
 * Register: \a DSM:SP_CFG:SP_FRONT_PORT_INJ_CFG
 */
#define VTSS_DSM_SP_CFG_SP_FRONT_PORT_INJ_CFG  VTSS_IOREG(VTSS_TO_DSM,0x685)

/** 
 * \brief
 * Select whether Link Level Flow Control is regarded when injecting
 * Super-Priority frames on a front port.
 *
 * \details 
 * '0': LLFC is disregarded
 * '1': LLFC is regarded
 *
 * Field: ::VTSS_DSM_SP_CFG_SP_FRONT_PORT_INJ_CFG . SP_FRONT_PORT_REGARD_LLFC
 */
#define  VTSS_F_DSM_SP_CFG_SP_FRONT_PORT_INJ_CFG_SP_FRONT_PORT_REGARD_LLFC  VTSS_BIT(1)

/** 
 * \brief
 * Enable transmission of Super-Priority frames on front ports. Bit is
 * common for all front ports.
 *
 * \details 
 * '0': Super-Priority frame transmission disabled
 * '1': Super-Priority frame transmission enabled
 *
 * Field: ::VTSS_DSM_SP_CFG_SP_FRONT_PORT_INJ_CFG . SP_FRONT_PORT_TX_ENA
 */
#define  VTSS_F_DSM_SP_CFG_SP_FRONT_PORT_INJ_CFG_SP_FRONT_PORT_TX_ENA  VTSS_BIT(0)


/** 
 * \brief Configuration Register for Super-Priority Injection FC watermark
 *
 * \details
 * Configuration Register for Super-Priority Injection FC watermark
 *
 * Register: \a DSM:SP_CFG:SP_INJ_CFG
 */
#define VTSS_DSM_SP_CFG_SP_INJ_CFG           VTSS_IOREG(VTSS_TO_DSM,0x687)

/** 
 * \brief
 * If the number of words in the injection buffer exceeds the WM, FC is
 * generated towards DEVCPU.
 *
 * \details 
 * Number of words in the injection buffer. One word has 16 bytes.
 *
 * Field: ::VTSS_DSM_SP_CFG_SP_INJ_CFG . SP_INJ_WM
 */
#define  VTSS_F_DSM_SP_CFG_SP_INJ_CFG_SP_INJ_WM(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_DSM_SP_CFG_SP_INJ_CFG_SP_INJ_WM     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_DSM_SP_CFG_SP_INJ_CFG_SP_INJ_WM(x)  VTSS_EXTRACT_BITFIELD(x,0,7)

/**
 * Register Group: \a DSM:SP_STATUS
 *
 * Super-Priority status and sticky bit registers
 */


/** 
 * \brief Super-Priority Front Port Injection Status register
 *
 * \details
 * Super-Priority Front Port Injection Status register
 *
 * Register: \a DSM:SP_STATUS:SP_FRONT_PORT_INJ_STAT
 */
#define VTSS_DSM_SP_STATUS_SP_FRONT_PORT_INJ_STAT  VTSS_IOREG(VTSS_TO_DSM,0x3f7)

/** 
 * \brief
 * A destination error for Super-Priority injection on a front port has
 * occurred.
 * Either  IFH -> Dest_Routing -> uc_inject_dst was out of range
 * or	     IFH -> Inject_fwd -> mode was wrong.
 *
 * \details 
 * '0': No Super-Priority injection error has occurred.
 * '1': Super-Priority injection error has occurred.
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DSM_SP_STATUS_SP_FRONT_PORT_INJ_STAT . SP_INJ_DST_ERR_STICKY
 */
#define  VTSS_F_DSM_SP_STATUS_SP_FRONT_PORT_INJ_STAT_SP_INJ_DST_ERR_STICKY  VTSS_BIT(0)


/** 
 * \brief Super-Priority frame counter
 *
 * \details
 * Super-Priority frame counter
 *
 * Register: \a DSM:SP_STATUS:SP_FRAME_CNT
 *
 * @param ri Replicator: x_NUM_SP_BUFS (??), 0-6
 */
#define VTSS_DSM_SP_STATUS_SP_FRAME_CNT(ri)  VTSS_IOREG(VTSS_TO_DSM,0x3f8 + (ri))

/** 
 * \brief
 * Counter for frames received to / transmitted from a Super-Priority
 * queue.
 * Good and bad frames are counted.
 * Frame counter is incremented at EOF.
 *
 * \details 
 * Counter can be written by SW.
 *
 * Field: ::VTSS_DSM_SP_STATUS_SP_FRAME_CNT . SP_FRAME_CNT
 */
#define  VTSS_F_DSM_SP_STATUS_SP_FRAME_CNT_SP_FRAME_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DSM_SP_STATUS_SP_FRAME_CNT_SP_FRAME_CNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DSM_SP_STATUS_SP_FRAME_CNT_SP_FRAME_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a DSM:RATE_LIMIT_CFG
 *
 * Configuration registers for rate limit modes
 */


/** 
 * \brief Configuration register for rate limit modes
 *
 * \details
 * Configuration register for rate limit modes
 *
 * Register: \a DSM:RATE_LIMIT_CFG:TX_RATE_LIMIT_MODE
 *
 * @param ri Replicator: x_DSM_NON_CPU_PORTS (??), 0-31
 */
#define VTSS_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE(ri)  VTSS_IOREG(VTSS_TO_DSM,0x688 + (ri))

/** 
 * \brief
 * Scale the IPG calculated by txRateLimitFrameOverhead and/or
 * txRateLimitPayloadRate by a power of 2 to enable shaping down to lower
 * BW.
 * 
 * Note: For txRateLimitPayloadRate only the additional overhead excluding
 * the standard 12 byte IPG is scaled whereas for txRateLimitFrameOverhead
 * the complete value is scaled.
 *
 * \details 
 * 0: Don't scale
 * 1: Multiply by 2
 * 2: Multiply by 4
 * (...)
 * 10: Multiply by 1024
 * 11-15: Reserved
 *
 * Field: ::VTSS_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE . IPG_SCALE_VAL
 */
#define  VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_IPG_SCALE_VAL(x)  VTSS_ENCODE_BITFIELD(x,16,4)
#define  VTSS_M_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_IPG_SCALE_VAL     VTSS_ENCODE_BITMASK(16,4)
#define  VTSS_X_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_IPG_SCALE_VAL(x)  VTSS_EXTRACT_BITFIELD(x,16,4)

/** 
 * \brief
 * Defines if what is configured as header size in
 * TX_RATE_LIMIT_HDR_SIZE::TX_RATE_LIMIT_HDR_CFG is subtracted form the
 * payload in txRateLimitPayloadRate and txRateLimitFrameRate mode.
 *
 * \details 
 * 0: Nothing is subtracted from payload
 * 1: TX_RATE_LIMIT_HDR_SIZE::TX_RATE_LIMIT_HDR_CFG is subtracted from
 * payload
 *
 * Field: ::VTSS_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE . PAYLOAD_CFG
 */
#define  VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_PAYLOAD_CFG  VTSS_BIT(9)

/** 
 * \brief
 * This bitgroup defines whether the preamble is counted as payload in
 * txRateLimitPayloadRate and txRateLimitFrameRate mode.
 *
 * \details 
 * '0': Preamble is not counted as payload
 * '1': Preamble is counted as payload
 *
 * Field: ::VTSS_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE . PAYLOAD_PREAM_CFG
 */
#define  VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_PAYLOAD_PREAM_CFG  VTSS_BIT(8)

/** 
 * \brief
 * Enable for accumulated rate limit mode.
 * Instead of taking the maximum  IPG of txRateLimitFrameOverhead,
 * txRateLimitPayloadRate and txRateLimitFrameRate. The resulting IPG is
 * the sum of txRateLimitFrameOverhead and txRateLimitPayloadRate.
 * txRateLimitFrameRate must be disabled.
 *
 * \details 
 * '0': Accumulated rate limit mode is disabled
 * '1': Accumulated rate limit mode is enabled
 *
 * Field: ::VTSS_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE . TX_RATE_LIMIT_ACCUM_MODE_ENA
 */
#define  VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_TX_RATE_LIMIT_ACCUM_MODE_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Enable txRateLimitFrameRate mode.
 * IPG is extended so that frame_size + IPG result in the value configured
 * in RATE_CTRL::TX_FRAME_RATE_START
 *
 * \details 
 * '0': txRateLimitFrameRate disabled
 * '1': txRateLimitFrameRate enabled
 *
 * Field: ::VTSS_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE . TX_RATE_LIMIT_FRAME_RATE_ENA
 */
#define  VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_TX_RATE_LIMIT_FRAME_RATE_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Enable txRateLimitPayloadRate mode.
 * IPG is increased relative to payload size.
 *
 * \details 
 * '0': txRateLimitPayloadRate disabled
 * '1': txRateLimitPayloadRate enable
 *
 * Field: ::VTSS_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE . TX_RATE_LIMIT_PAYLOAD_RATE_ENA
 */
#define  VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_TX_RATE_LIMIT_PAYLOAD_RATE_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable txRateLimitFrameOverhead mode.
 * IPG is increased by a fixed value specified in 
 * RATE_CTRL::FRM_GAP_COMP
 *
 * \details 
 * '0': txRateLimitFrameOverhead disabled
 * '1': txRateLimitFrameOverhead enabled
 *
 * Field: ::VTSS_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE . TX_RATE_LIMIT_FRAME_OVERHEAD_ENA
 */
#define  VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_MODE_TX_RATE_LIMIT_FRAME_OVERHEAD_ENA  VTSS_BIT(0)


/** 
 * \brief Configuration register for txFineIpgStretchRatio
 *
 * \details
 * Configuration register for txFineIpgStretchRatio
 *
 * Register: \a DSM:RATE_LIMIT_CFG:TX_IPG_STRETCH_RATIO_CFG
 *
 * @param ri Replicator: x_DSM_NON_CPU_PORTS (??), 0-31
 */
#define VTSS_DSM_RATE_LIMIT_CFG_TX_IPG_STRETCH_RATIO_CFG(ri)  VTSS_IOREG(VTSS_TO_DSM,0x6a8 + (ri))

/** 
 * \brief
 * Determines the extent of the interPacketGap extension, when
 * txRateLimitPayloadRate mode is enabled. It may be considered as the
 * number of bits in a packet that would require 32 octets of
 * interPacketGap extension. Note that each interPacketGap is increased by
 * an integer number of octets, the average increase is ( (frame length in
 * bits) * 32) / txFineIpgStretchRatio.
 * 
 * Note: fractions left at frame border will be carried on to the next
 * frame.
 * 
 * Example:
 * Packet length = 64 bytes + 8 bytes preamble 
 * value = 2048
 * IPG increase = 9 bytes
 * 
 * Maximum IPG increase is 255 bytes.
 *
 * \details 
 * IPG increase = frame length in bits * 32 / value.
 * 
 * values below 1152 and above 518143 are reserved for arithmetic
 * processing  reasons.
 *
 * Field: ::VTSS_DSM_RATE_LIMIT_CFG_TX_IPG_STRETCH_RATIO_CFG . TX_FINE_IPG_STRETCH_RATIO
 */
#define  VTSS_F_DSM_RATE_LIMIT_CFG_TX_IPG_STRETCH_RATIO_CFG_TX_FINE_IPG_STRETCH_RATIO(x)  VTSS_ENCODE_BITFIELD(x,0,19)
#define  VTSS_M_DSM_RATE_LIMIT_CFG_TX_IPG_STRETCH_RATIO_CFG_TX_FINE_IPG_STRETCH_RATIO     VTSS_ENCODE_BITMASK(0,19)
#define  VTSS_X_DSM_RATE_LIMIT_CFG_TX_IPG_STRETCH_RATIO_CFG_TX_FINE_IPG_STRETCH_RATIO(x)  VTSS_EXTRACT_BITFIELD(x,0,19)


/** 
 * \brief Configuration register for txFrameRateStart
 *
 * \details
 * Configuration register for txFrameRateStart
 *
 * Register: \a DSM:RATE_LIMIT_CFG:TX_FRAME_RATE_START_CFG
 *
 * @param ri Replicator: x_DSM_NON_CPU_PORTS (??), 0-31
 */
#define VTSS_DSM_RATE_LIMIT_CFG_TX_FRAME_RATE_START_CFG(ri)  VTSS_IOREG(VTSS_TO_DSM,0x6c8 + (ri))

/** 
 * \brief
 * Value loaded into the txFrameRateTimer at the start of each packet, when
 * TX_RATE_LIMIT_FRAME_RATE mode is enabled.
 * Timer is decremented with each byte transmitted.
 * IPG is set to timer value at EOF.
 * Note: If resulting IPG is smaller than the minimum IPG of 12 bytes, it
 * will be set to 12 bytes.
 *
 * \details 
 * Timer start value in bytes.
 * To take effect, value must be above 84. (64byte min frame size, 8 byte
 * preamble, 12 byte minimum IFG).
 *
 * Field: ::VTSS_DSM_RATE_LIMIT_CFG_TX_FRAME_RATE_START_CFG . TX_FRAME_RATE_START
 */
#define  VTSS_F_DSM_RATE_LIMIT_CFG_TX_FRAME_RATE_START_CFG_TX_FRAME_RATE_START(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DSM_RATE_LIMIT_CFG_TX_FRAME_RATE_START_CFG_TX_FRAME_RATE_START     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DSM_RATE_LIMIT_CFG_TX_FRAME_RATE_START_CFG_TX_FRAME_RATE_START(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Header size configuration register for txRateLimitPayloadRate mode
 *
 * \details
 * Header size configuration register for txRateLimitPayloadRate mode
 *
 * Register: \a DSM:RATE_LIMIT_CFG:TX_RATE_LIMIT_HDR_CFG
 */
#define VTSS_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_HDR_CFG  VTSS_IOREG(VTSS_TO_DSM,0x6e8)

/** 
 * \brief
 * Defines how much of the frame is seen as header and not counted as
 * payload in txRateLimitPayloadRate and txRateLimitFrameRate mode when
 * TX_RATE_LIMIT_MODE::PAYLOAD_CFG is set to 1.
 *
 * \details 
 * 0: 32 bytes are counted as header
 * 1-31: 1-31 bytes are counted as header
 *
 * Field: ::VTSS_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_HDR_CFG . TX_RATE_LIMIT_HDR_SIZE
 */
#define  VTSS_F_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_HDR_CFG_TX_RATE_LIMIT_HDR_SIZE(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_HDR_CFG_TX_RATE_LIMIT_HDR_SIZE     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_DSM_RATE_LIMIT_CFG_TX_RATE_LIMIT_HDR_CFG_TX_RATE_LIMIT_HDR_SIZE(x)  VTSS_EXTRACT_BITFIELD(x,0,5)

/**
 * Register Group: \a DSM:RATE_LIMIT_STATUS
 *
 * Status registers for rate limit modes
 */


/** 
 * \brief Sticky bit register for rate limit modes
 *
 * \details
 * Sticky bit register for rate limit modes
 *
 * Register: \a DSM:RATE_LIMIT_STATUS:TX_RATE_LIMIT_STICKY
 */
#define VTSS_DSM_RATE_LIMIT_STATUS_TX_RATE_LIMIT_STICKY  VTSS_IOREG(VTSS_TO_DSM,0x3ff)

/**
 * Register Group: \a DSM:HOST_IF_CFG
 *
 * Registers required for Host Interface configuration
 */


/** 
 * \brief Host Interface Header configuration register
 *
 * \details
 * Host Interface Header configuration register
 *
 * Register: \a DSM:HOST_IF_CFG:HIH_CFG
 *
 * @param ri Replicator: x_NUM_POT_HOST_IFS (??), 0-3
 */
#define VTSS_DSM_HOST_IF_CFG_HIH_CFG(ri)     VTSS_IOREG(VTSS_TO_DSM,0x6e9 + (ri))

/** 
 * \brief
 * Enables HIH-mode for ports that can be operated in host mode.
 *
 * \details 
 * 0: No Preamble/HIH header is added
 * 1: Preamble/HIH header is added
 *
 * Field: ::VTSS_DSM_HOST_IF_CFG_HIH_CFG . HIH_ENA
 */
#define  VTSS_F_DSM_HOST_IF_CFG_HIH_CFG_HIH_ENA  VTSS_BIT(0)


/** 
 * \brief Configutarion register for ITAG - TPID field
 *
 * \details
 * Configutarion register for ITAG - TPID field to be inserted in
 * compressed mode
 *
 * Register: \a DSM:HOST_IF_CFG:HIH_ITAG_TPID_CFG
 */
#define VTSS_DSM_HOST_IF_CFG_HIH_ITAG_TPID_CFG  VTSS_IOREG(VTSS_TO_DSM,0x6ed)

/** 
 * \brief
 * Value of the I-TAG TPI field that will be inserted in HIH compressed
 * mode
 *
 * \details 
 * Field: ::VTSS_DSM_HOST_IF_CFG_HIH_ITAG_TPID_CFG . HIH_ITAG_TPID_VAL
 */
#define  VTSS_F_DSM_HOST_IF_CFG_HIH_ITAG_TPID_CFG_HIH_ITAG_TPID_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DSM_HOST_IF_CFG_HIH_ITAG_TPID_CFG_HIH_ITAG_TPID_VAL     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DSM_HOST_IF_CFG_HIH_ITAG_TPID_CFG_HIH_ITAG_TPID_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a DSM:HOST_IF_IBS_CFG
 *
 * Host Interface Inbound Status Configuration Registers
 */


/** 
 * \brief Host Interface Inbound Status Configuration Register
 *
 * \details
 * Host Interface Inbound Status Configuration Register
 *
 * Register: \a DSM:HOST_IF_IBS_CFG:XAUI_IBS_CFG
 *
 * @param ri Replicator: x_NUM_HOST_IFS (??), 0-1
 */
#define VTSS_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG(ri)  VTSS_IOREG(VTSS_TO_DSM,0x6ee + (ri))

/** 
 * \brief
 * Software reset for Inbound Status Channel
 *
 * \details 
 * '0': Reset inactive
 * '1': Reset active
 *
 * Field: ::VTSS_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG . CONF_IBS_RST
 */
#define  VTSS_F_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG_CONF_IBS_RST  VTSS_BIT(28)

/** 
 * \brief
 * Select clock for inbound status path
 *
 * \details 
 * 0: system_clk / 2
 * 1: system_clk / 4
 * 2: recovered outbound status clock
 * 3: No clock, power down
 *
 * Field: ::VTSS_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG . CONF_IBS_CLK_SEL
 */
#define  VTSS_F_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG_CONF_IBS_CLK_SEL(x)  VTSS_ENCODE_BITFIELD(x,26,2)
#define  VTSS_M_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG_CONF_IBS_CLK_SEL     VTSS_ENCODE_BITMASK(26,2)
#define  VTSS_X_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG_CONF_IBS_CLK_SEL(x)  VTSS_EXTRACT_BITFIELD(x,26,2)

/** 
 * \brief
 * Inbound status clock shift.
 *
 * \details 
 * '0': Normal status clock.
 * '1': 180 degree (inverted) status clock.
 *
 * Field: ::VTSS_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG . CONF_IBS_CLK_SHIFT
 */
#define  VTSS_F_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG_CONF_IBS_CLK_SHIFT  VTSS_BIT(25)

/** 
 * \brief
 * Force IDLE (All "11" values) on Inbound Status Channel.
 *
 * \details 
 * 0: Normal operation.
 * 1: Force IDLE.
 *
 * Field: ::VTSS_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG . CONF_IBS_FORCE_IDLE
 */
#define  VTSS_F_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG_CONF_IBS_FORCE_IDLE  VTSS_BIT(24)

/** 
 * \brief
 * Number of times calendar sequence is repeated between DIP-2/Sync words.
 *
 * \details 
 * "0": Calendar sequence is not repeated (sent only once).
 * [...]
 * "15": Calendar sequence is repeated 15 times. I.e. 16 calendar sequences
 * are sent out, before DIP-2.
 *
 * Field: ::VTSS_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG . CONF_IBS_CAL_M
 */
#define  VTSS_F_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG_CONF_IBS_CAL_M(x)  VTSS_ENCODE_BITFIELD(x,16,4)
#define  VTSS_M_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG_CONF_IBS_CAL_M     VTSS_ENCODE_BITMASK(16,4)
#define  VTSS_X_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG_CONF_IBS_CAL_M(x)  VTSS_EXTRACT_BITFIELD(x,16,4)

/** 
 * \brief
 * Calendar length, value gives the number of the last valid calendar
 * entry.
 *
 * \details 
 * "0":reserved.
 * "1": Only calendar entry 0 is valid
 * "2": Only calendar entries 0 & 1 are valid
 * [...]
 * "320": Calendar entries 0 to 319 are valid.
 *
 * Field: ::VTSS_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG . CONF_IBS_CAL_LEN
 */
#define  VTSS_F_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG_CONF_IBS_CAL_LEN(x)  VTSS_ENCODE_BITFIELD(x,0,9)
#define  VTSS_M_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG_CONF_IBS_CAL_LEN     VTSS_ENCODE_BITMASK(0,9)
#define  VTSS_X_DSM_HOST_IF_IBS_CFG_XAUI_IBS_CFG_CONF_IBS_CAL_LEN(x)  VTSS_EXTRACT_BITFIELD(x,0,9)

/**
 * Register Group: \a DSM:RAM_INTEGRITY
 *
 * RAM integrity errors
 */


/** 
 * \brief RAM integrity errors sticky register
 *
 * \details
 * RAM integrity errors sticky register
 *
 * Register: \a DSM:RAM_INTEGRITY:RAM_INTEGRITY_ERR_STICKY
 */
#define VTSS_DSM_RAM_INTEGRITY_RAM_INTEGRITY_ERR_STICKY  VTSS_IOREG(VTSS_TO_DSM,0x6f2)

/** 
 * \brief
 * Integrity error for Outbound Status calendar RAMs.
 *
 * \details 
 * One bit per status channel.
 * 
 * '0': No RAM integrity check error occurred
 * '1': RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DSM_RAM_INTEGRITY_RAM_INTEGRITY_ERR_STICKY . OBS_CAL_INTEG_ERR_STICKY
 */
#define  VTSS_F_DSM_RAM_INTEGRITY_RAM_INTEGRITY_ERR_STICKY_OBS_CAL_INTEG_ERR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,28,2)
#define  VTSS_M_DSM_RAM_INTEGRITY_RAM_INTEGRITY_ERR_STICKY_OBS_CAL_INTEG_ERR_STICKY     VTSS_ENCODE_BITMASK(28,2)
#define  VTSS_X_DSM_RAM_INTEGRITY_RAM_INTEGRITY_ERR_STICKY_OBS_CAL_INTEG_ERR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,28,2)

/** 
 * \brief
 * Integrity error for Inbound Status calendar RAMs.
 *
 * \details 
 * One bit per status channel.
 * 
 * '0': No RAM integrity check error occurred
 * '1': RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DSM_RAM_INTEGRITY_RAM_INTEGRITY_ERR_STICKY . IBS_CAL_INTEG_ERR_STICKY
 */
#define  VTSS_F_DSM_RAM_INTEGRITY_RAM_INTEGRITY_ERR_STICKY_IBS_CAL_INTEG_ERR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,24,2)
#define  VTSS_M_DSM_RAM_INTEGRITY_RAM_INTEGRITY_ERR_STICKY_IBS_CAL_INTEG_ERR_STICKY     VTSS_ENCODE_BITMASK(24,2)
#define  VTSS_X_DSM_RAM_INTEGRITY_RAM_INTEGRITY_ERR_STICKY_IBS_CAL_INTEG_ERR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,24,2)

/** 
 * \brief
 * Integrity error for Super-Priority buffer RAMs.
 *
 * \details 
 * Bits 0-1: SP relay buffer
 * Bist 2-3: SP extract buffer
 * Bits 4-5: SP inject buffer stack ports
 * Bit	6: SP inject buffer front port
 * 
 * '0': No RAM integrity check error occurred
 * '1': RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DSM_RAM_INTEGRITY_RAM_INTEGRITY_ERR_STICKY . SP_BUF_INTEG_ERR_STICKY
 */
#define  VTSS_F_DSM_RAM_INTEGRITY_RAM_INTEGRITY_ERR_STICKY_SP_BUF_INTEG_ERR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,16,7)
#define  VTSS_M_DSM_RAM_INTEGRITY_RAM_INTEGRITY_ERR_STICKY_SP_BUF_INTEG_ERR_STICKY     VTSS_ENCODE_BITMASK(16,7)
#define  VTSS_X_DSM_RAM_INTEGRITY_RAM_INTEGRITY_ERR_STICKY_SP_BUF_INTEG_ERR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,16,7)

/** 
 * \brief
 * Integrity error for align buffer RAMs.
 *
 * \details 
 * One bit per taxi ring.
 * 
 * '0': No RAM integrity check error occurred
 * '1': RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DSM_RAM_INTEGRITY_RAM_INTEGRITY_ERR_STICKY . ALIGN_BUF_INTEG_ERR_STICKY
 */
#define  VTSS_F_DSM_RAM_INTEGRITY_RAM_INTEGRITY_ERR_STICKY_ALIGN_BUF_INTEG_ERR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_DSM_RAM_INTEGRITY_RAM_INTEGRITY_ERR_STICKY_ALIGN_BUF_INTEG_ERR_STICKY     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_DSM_RAM_INTEGRITY_RAM_INTEGRITY_ERR_STICKY_ALIGN_BUF_INTEG_ERR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

/**
 * Register Group: \a DSM:HOST_IF_CONF_IBS_CAL
 *
 * Host Interface Inbound Status Combined Configuration and Status Registers per Status channel
 */


/** 
 * \brief Host Interface Inbound Status Calendar Table
 *
 * \details
 * Host Interface Inbound Status Calendar Table
 *
 * Register: \a DSM:HOST_IF_CONF_IBS_CAL:XAUI_IBS_CAL
 *
 * @param gi Replicator: x_HOST_IF_IB_STAT_MAX_CAL_LEN (??), 0-319
 * @param ri Replicator: x_NUM_HOST_IFS (??), 0-1
 */
#define VTSS_DSM_HOST_IF_CONF_IBS_CAL_XAUI_IBS_CAL(gi,ri)  VTSS_IOREG_IX(VTSS_TO_DSM,0x0,gi,2,ri,0)

/** 
 * \brief
 * Calendar table entries consisting of Logical Port/Queue Numbers.
 * 
 * Note1: To access this registers it is required to enable the respective
 * clock and release the reset.
 * Note2: For reading this register the SW has to wait until HW is not
 * using the RAM where the values are stored. For large numbers of
 * calendar_len*calendar_m (approximately >190), the SW might receive a
 * time out when trying o read. This can be avoided by setting
 * calendar_len*calendar_m to a smaller value when reading. Write is not an
 * issue.
 *
 * \details 
 *     0..   31: CAPTURE_UPSID:0  Ethernet Ports, Port Level - Ports 0..31 
 *   32..   63: CAPTURE_UPSID:1  Ethernet Ports, Queue Level -	Ports
 * 0..31, Prio 0
 *   64..   95: CAPTURE_UPSID:2  Ethernet Ports, Queue Level -	Ports
 * 0..31, Prio 1
 * (...)
 * Nx32..Nx32+31: CAPTURE_UPSID:N  Ethernet Ports, Queue Level -  Ports
 * 0..31, Prio N-1
 *  
 * 510: constantly starving
 * 511: constantly satisfied
 *
 * Field: ::VTSS_DSM_HOST_IF_CONF_IBS_CAL_XAUI_IBS_CAL . CONF_IBS_CAL_VAL
 */
#define  VTSS_F_DSM_HOST_IF_CONF_IBS_CAL_XAUI_IBS_CAL_CONF_IBS_CAL_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,9)
#define  VTSS_M_DSM_HOST_IF_CONF_IBS_CAL_XAUI_IBS_CAL_CONF_IBS_CAL_VAL     VTSS_ENCODE_BITMASK(0,9)
#define  VTSS_X_DSM_HOST_IF_CONF_IBS_CAL_XAUI_IBS_CAL_CONF_IBS_CAL_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,9)

/**
 * Register Group: \a DSM:HOST_IF_IBS_COMMON_CFG
 *
 * Host Interface Inbound Status Common Configuration Registers
 */


/** 
 * \brief Register holds the upsid to capture status from.
 *
 * \details
 * Register holds the upsid to capture status from.
 *
 * Register: \a DSM:HOST_IF_IBS_COMMON_CFG:CAPTURE_UPSID_CFG
 *
 * @param ri Replicator: x_HOST_IF_IB_STAT_MAX_CAL_LEN_DIV32 (??), 0-9
 */
#define VTSS_DSM_HOST_IF_IBS_COMMON_CFG_CAPTURE_UPSID_CFG(ri)  VTSS_IOREG(VTSS_TO_DSM,0x6f3 + (ri))

/** 
 * \brief
 * Defines which UPSIDS are captures in the OOB status channel.
 * One UPSID per 32 bit word
 *
 * \details 
 * UPSID to capture
 *
 * Field: ::VTSS_DSM_HOST_IF_IBS_COMMON_CFG_CAPTURE_UPSID_CFG . CAPTURE_UPSID
 */
#define  VTSS_F_DSM_HOST_IF_IBS_COMMON_CFG_CAPTURE_UPSID_CFG_CAPTURE_UPSID(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_DSM_HOST_IF_IBS_COMMON_CFG_CAPTURE_UPSID_CFG_CAPTURE_UPSID     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_DSM_HOST_IF_IBS_COMMON_CFG_CAPTURE_UPSID_CFG_CAPTURE_UPSID(x)  VTSS_EXTRACT_BITFIELD(x,0,5)

/**
 * Register Group: \a DSM:HOST_IF_IBS_COMMON_CFG_STAT
 *
 * Host Interface Inbound Status Combined Configuration and Status Registers
 */

/**
 * Register Group: \a DSM:HOST_IF_OBS_CFG
 *
 * Host Interface Outbound Status Configuration Registers
 */


/** 
 * \brief Host Interface Outbound Status Configuration Register
 *
 * \details
 * Host Interface Outbound Status Configuration Register
 *
 * Register: \a DSM:HOST_IF_OBS_CFG:XAUI_OBS_CFG
 *
 * @param ri Replicator: x_NUM_HOST_IFS (??), 0-1
 */
#define VTSS_DSM_HOST_IF_OBS_CFG_XAUI_OBS_CFG(ri)  VTSS_IOREG(VTSS_TO_DSM,0x6fe + (ri))

/** 
 * \brief
 * Software reset for Outbound Status Channel
 *
 * \details 
 * '0': Reset inactive
 * '1': Reset active
 *
 * Field: ::VTSS_DSM_HOST_IF_OBS_CFG_XAUI_OBS_CFG . CONF_OBS_RST
 */
#define  VTSS_F_DSM_HOST_IF_OBS_CFG_XAUI_OBS_CFG_CONF_OBS_RST  VTSS_BIT(28)

/** 
 * \brief
 * Outbound status clock shift.
 *
 * \details 
 * '0': Normal status clock.
 * '1': 180 degree (inverted) status clock.
 *
 * Field: ::VTSS_DSM_HOST_IF_OBS_CFG_XAUI_OBS_CFG . CONF_OBS_CLK_SHIFT
 */
#define  VTSS_F_DSM_HOST_IF_OBS_CFG_XAUI_OBS_CFG_CONF_OBS_CLK_SHIFT  VTSS_BIT(20)

/** 
 * \brief
 * Number of times calendar sequence is repeated between DIP-2/Sync words.
 *
 * \details 
 * "0": Calendar sequence is not repeated (sent only once).
 * 
 * [...]
 * 
 * "15": Calendar sequence is repeated 15 times. I.e. 16 calendar sequences
 * are sent out, before DIP-2.
 *
 * Field: ::VTSS_DSM_HOST_IF_OBS_CFG_XAUI_OBS_CFG . CONF_OBS_CAL_M
 */
#define  VTSS_F_DSM_HOST_IF_OBS_CFG_XAUI_OBS_CFG_CONF_OBS_CAL_M(x)  VTSS_ENCODE_BITFIELD(x,16,4)
#define  VTSS_M_DSM_HOST_IF_OBS_CFG_XAUI_OBS_CFG_CONF_OBS_CAL_M     VTSS_ENCODE_BITMASK(16,4)
#define  VTSS_X_DSM_HOST_IF_OBS_CFG_XAUI_OBS_CFG_CONF_OBS_CAL_M(x)  VTSS_EXTRACT_BITFIELD(x,16,4)

/** 
 * \brief
 * Calendar length, value gives the number of the last valid calendar entry
 *
 * \details 
 * "0": reserved
 * "1": Only calendar entry 0 is valid
 * "2": Only calendar entries 0 & 1 are valid
 * [...]
 * "240": Calendar entries 0 to 239 are valid.
 *
 * Field: ::VTSS_DSM_HOST_IF_OBS_CFG_XAUI_OBS_CFG . CONF_OBS_CAL_LEN
 */
#define  VTSS_F_DSM_HOST_IF_OBS_CFG_XAUI_OBS_CFG_CONF_OBS_CAL_LEN(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_DSM_HOST_IF_OBS_CFG_XAUI_OBS_CFG_CONF_OBS_CAL_LEN     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_DSM_HOST_IF_OBS_CFG_XAUI_OBS_CFG_CONF_OBS_CAL_LEN(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a DSM:HOST_IF_CONF_OBS_CAL
 *
 * Configuration Registers for OBS Calendar Table
 */


/** 
 * \brief Host Interface Outbound Status Calendar Table
 *
 * \details
 * Host Interface Outbound Status Calendar Table
 *
 * Register: \a DSM:HOST_IF_CONF_OBS_CAL:XAUI_OBS_CAL
 *
 * @param gi Replicator: x_HOST_IF_OB_STAT_MAX_CAL_LEN (??), 0-239
 * @param ri Replicator: x_NUM_HOST_IFS (??), 0-1
 */
#define VTSS_DSM_HOST_IF_CONF_OBS_CAL_XAUI_OBS_CAL(gi,ri)  VTSS_IOREG_IX(VTSS_TO_DSM,0x400,gi,2,ri,0)

/** 
 * \brief
 * Calendar table entries consisting of Logical Queue/Port Numbers
 *
 * \details 
 * Two Ranges:
 * 0..191: Queue Range, Host Queues 0..191
 * 192..239: Port range, Host Ports 0..47.
 *		   In Extended Reach configuration Ports 0..31 (224..139
 * reserved)
 *
 * Field: ::VTSS_DSM_HOST_IF_CONF_OBS_CAL_XAUI_OBS_CAL . CONF_OBS_CAL_VAL
 */
#define  VTSS_F_DSM_HOST_IF_CONF_OBS_CAL_XAUI_OBS_CAL_CONF_OBS_CAL_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_DSM_HOST_IF_CONF_OBS_CAL_XAUI_OBS_CAL_CONF_OBS_CAL_VAL     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_DSM_HOST_IF_CONF_OBS_CAL_XAUI_OBS_CAL_CONF_OBS_CAL_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a DSM:HOST_IF_OBS_STATUS
 *
 * Host Interface Outbound Status Status Registers
 */


/** 
 * \brief Host Interface Outbound DIP-2 Error Counter
 *
 * \details
 * Host Interface Outbound DIP-2 Error Counter
 *
 * Register: \a DSM:HOST_IF_OBS_STATUS:XAUI_OBS_DIP2_ERR_CNT
 *
 * @param ri Replicator: x_NUM_HOST_IFS (??), 0-1
 */
#define VTSS_DSM_HOST_IF_OBS_STATUS_XAUI_OBS_DIP2_ERR_CNT(ri)  VTSS_IOREG(VTSS_TO_DSM,0x704 + (ri))


#endif /* _VTSS_JAGUAR_REGS_DSM_H_ */
