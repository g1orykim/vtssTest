#ifndef _VTSS_DAYTONA_REGS_XFI_H_
#define _VTSS_DAYTONA_REGS_XFI_H_

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
 * Target: \a XFI
 *
 * \see vtss_target_XFI_e
 *
 * Core to HSS data interface
 *
 ***********************************************************************/

/**
 * Register Group: \a XFI::XFI_CONTROL
 *
 * XFI controls
 */


/** 
 * \brief XFI modes
 *
 * \details
 * Control and operating modes of the XFI.
 *
 * Register: \a XFI::XFI_CONTROL::XFI_MODE
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_XFI_CONTROL_XFI_MODE(target)  VTSS_IOREG(target,0x0)

/** 
 * \brief
 * Invert data received from the HSS lane-B Rx data port (rxbd)
 *
 * \details 
 * 0: Non-inverted data from the lane-B Rx data port
 * 1 = Inverted data from the lane-B Rx data port
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_XFI_MODE . RX_INVERTB
 */
#define  VTSS_F_XFI_XFI_CONTROL_XFI_MODE_RX_INVERTB(x)  VTSS_ENCODE_BITFIELD(x,31,1)
#define  VTSS_M_XFI_XFI_CONTROL_XFI_MODE_RX_INVERTB     VTSS_ENCODE_BITMASK(31,1)
#define  VTSS_X_XFI_XFI_CONTROL_XFI_MODE_RX_INVERTB(x)  VTSS_EXTRACT_BITFIELD(x,31,1)

/** 
 * \brief
 * Invert data received from the HSS lane-A Rx data port (rxad)
 *
 * \details 
 * 0: Non-inverted data from the lane-A Rx data port
 * 1: Inverted data from the lane-A Rx data port
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_XFI_MODE . RX_INVERTA
 */
#define  VTSS_F_XFI_XFI_CONTROL_XFI_MODE_RX_INVERTA(x)  VTSS_ENCODE_BITFIELD(x,30,1)
#define  VTSS_M_XFI_XFI_CONTROL_XFI_MODE_RX_INVERTA     VTSS_ENCODE_BITMASK(30,1)
#define  VTSS_X_XFI_XFI_CONTROL_XFI_MODE_RX_INVERTA(x)  VTSS_EXTRACT_BITFIELD(x,30,1)

/** 
 * \brief
 * Invert data driven into the HSS lane-B Tx data port (txbd)
 *
 * \details 
 * 0: Non-inverted data to the lane-B Tx data port
 * 1 = Inverted data to the lane-B Tx data port
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_XFI_MODE . TX_INVERTB
 */
#define  VTSS_F_XFI_XFI_CONTROL_XFI_MODE_TX_INVERTB(x)  VTSS_ENCODE_BITFIELD(x,29,1)
#define  VTSS_M_XFI_XFI_CONTROL_XFI_MODE_TX_INVERTB     VTSS_ENCODE_BITMASK(29,1)
#define  VTSS_X_XFI_XFI_CONTROL_XFI_MODE_TX_INVERTB(x)  VTSS_EXTRACT_BITFIELD(x,29,1)

/** 
 * \brief
 * Invert data driven into the HSS lane-A Tx data port (txad)
 *
 * \details 
 * 0 = Non-inverted data to the lane-A Tx data port
 * 1 = Inverted data to the lane-A Tx data port
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_XFI_MODE . TX_INVERTA
 */
#define  VTSS_F_XFI_XFI_CONTROL_XFI_MODE_TX_INVERTA(x)  VTSS_ENCODE_BITFIELD(x,28,1)
#define  VTSS_M_XFI_XFI_CONTROL_XFI_MODE_TX_INVERTA     VTSS_ENCODE_BITMASK(28,1)
#define  VTSS_X_XFI_XFI_CONTROL_XFI_MODE_TX_INVERTA(x)  VTSS_EXTRACT_BITFIELD(x,28,1)

/** 
 * \brief
 * Select the auto-negotiation macros
 *
 * \details 
 * 0: Bypass the ANEG macros (default)
 * 1: The ANEG macros are serially inserted on the data path to the HSS
 * macros
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_XFI_MODE . SEL_ANEG
 */
#define  VTSS_F_XFI_XFI_CONTROL_XFI_MODE_SEL_ANEG(x)  VTSS_ENCODE_BITFIELD(x,22,1)
#define  VTSS_M_XFI_XFI_CONTROL_XFI_MODE_SEL_ANEG     VTSS_ENCODE_BITMASK(22,1)
#define  VTSS_X_XFI_XFI_CONTROL_XFI_MODE_SEL_ANEG(x)  VTSS_EXTRACT_BITFIELD(x,22,1)

/** 
 * \brief
 * Select the AE macros
 *
 * \details 
 * 0: Bypass the AE macros (default)
 * 1: The AE macros are serially inserted on the data path to the HSS
 * macros
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_XFI_MODE . SEL_AE
 */
#define  VTSS_F_XFI_XFI_CONTROL_XFI_MODE_SEL_AE(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_XFI_XFI_CONTROL_XFI_MODE_SEL_AE     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_XFI_XFI_CONTROL_XFI_MODE_SEL_AE(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \brief
 * Select the pattern generator instead of the SEL_LINELB selected data as
 * the source of the HSS-TXD port.
 *
 * \details 
 * 0: HSS-Tx data is sourced by the core-Tx/line-Rx data (default)
 * 1: HSS-Tx data is sourced pattern generator
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_XFI_MODE . SEL_PG
 */
#define  VTSS_F_XFI_XFI_CONTROL_XFI_MODE_SEL_PG(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_XFI_XFI_CONTROL_XFI_MODE_SEL_PG     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_XFI_XFI_CONTROL_XFI_MODE_SEL_PG(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Select the line Rx data loopback path instead of the core Tx data as the
 * source of the internal Tx data path. 
 * Note: The data selected here is one of two sources feeding the SEL_PG
 * mux (SEL_PG in XFI_MODE).
 *
 * \details 
 * 0: Core Tx data selected (default)
 * 1: Line Rx data (loopback) selected
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_XFI_MODE . SEL_LINELB
 */
#define  VTSS_F_XFI_XFI_CONTROL_XFI_MODE_SEL_LINELB(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_XFI_XFI_CONTROL_XFI_MODE_SEL_LINELB     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_XFI_XFI_CONTROL_XFI_MODE_SEL_LINELB(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Select the lane-B Tx port of the HSS instead of the lane-A Tx port as
 * the target of the Tx data and source of the Tx clock feeding the core.
 * Notes: 
 * 1. The XFI Tx clock output will go inactive for 2-3 Tx clock cycles
 * during a Tx lane source switch (to preclude glitches).
 * 2. The Rx data and clock sources are independently controlled by the
 * SEL_RX_LANEB.
 *
 * \details 
 * 0: Lane-A Tx data and clock selected (default)
 * 1: Lane-B Tx data and clock selected
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_XFI_MODE . SEL_TX_LANEB
 */
#define  VTSS_F_XFI_XFI_CONTROL_XFI_MODE_SEL_TX_LANEB(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_XFI_XFI_CONTROL_XFI_MODE_SEL_TX_LANEB     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_XFI_XFI_CONTROL_XFI_MODE_SEL_TX_LANEB(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Select the lane-B Rx port of the HSS instead of the lane-A Rx port as
 * the source of the Rx data and Rx clock feeding the core.
 * Notes: 
 * 1. The XFI Rx clock output will go inactive for 2-3 Rx clock cycles
 * during an Rx lane source switch (to preclude glitches).
 * 2. The Tx data and clock sources are independently controlled by the
 * SEL_TX_LANEB.
 *
 * \details 
 * 0: Lane-A Rx data and clock selected (default)
 * 1: Lane-B Rx data and clock selected
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_XFI_MODE . SEL_RX_LANEB
 */
#define  VTSS_F_XFI_XFI_CONTROL_XFI_MODE_SEL_RX_LANEB(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_XFI_XFI_CONTROL_XFI_MODE_SEL_RX_LANEB     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_XFI_XFI_CONTROL_XFI_MODE_SEL_RX_LANEB(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * TX_FIFO error mode control
 * When deasserted (default), if single Tx-data word is read outside the
 * 'safe' zone (where a full-cycle of setup and hold are guaranteed), the
 * TX_LOS status will be asserted.
 * Due to synchronization uncertainties it is possible to be 'on-edge' and
 * still make timing, having a whole cycle plus something less than a cycle
 * for setup and hold timing. When this field is asserted, two consecutive
 * read cycles outside the 'safe' zone must occur to generate the TX_LOS
 * status. This allows a known (slew-rate limited) drift rate to dither
 * 'on-edge' out of the 'safe' zone for a cycle and not generate the TX_LOS
 * if read timing returns to the 'safe' zone on the following cycle.
 *
 * \details 
 * 0: A single read outside the 'safe' zone generates a TX_LOS condition
 * (default)
 * 1: Monitor for two consecutive out of 'safe' zone cycles to generate
 * TX_LOS
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_XFI_MODE . TXF_ERR_MODE
 */
#define  VTSS_F_XFI_XFI_CONTROL_XFI_MODE_TXF_ERR_MODE(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_XFI_XFI_CONTROL_XFI_MODE_TXF_ERR_MODE     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_XFI_XFI_CONTROL_XFI_MODE_TXF_ERR_MODE(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Set the data-type/endian mode of the core to HSS interface
 *
 * \details 
 * 2'b00: Core 64/66 data: bit 63 first, followed by the remaining bits
 * leading to bit 0 (default)
 * 2'b01: Core 64/66 data: bit 0 first, followed by the remaining bits
 * leading to bit 063
 * 2'b10: Core 8B/10B data: bit 79 first, followed by the remaining bits
 * leading to bit 0
 * 2'b11: Core 8B/10B data: bit 0 first, followed by the remaining bits
 * leading to bit 79
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_XFI_MODE . ENDIAN
 */
#define  VTSS_F_XFI_XFI_CONTROL_XFI_MODE_ENDIAN(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_XFI_XFI_CONTROL_XFI_MODE_ENDIAN     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_XFI_XFI_CONTROL_XFI_MODE_ENDIAN(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * Automatically assert a hardware TX_RESYNC when TX_FIFO_LOS is detected
 *
 * \details 
 * 0: TX_FIFO will not resync when an LOS event is detected
 * 1: TX_FIFO will resync when an LOS event is detected
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_XFI_MODE . TX_AUTORESYNC
 */
#define  VTSS_F_XFI_XFI_CONTROL_XFI_MODE_TX_AUTORESYNC(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_XFI_CONTROL_XFI_MODE_TX_AUTORESYNC     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_XFI_CONTROL_XFI_MODE_TX_AUTORESYNC(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Resynchronize the core/XFI data phase crossing logic
 * Must always be asserted after SW_RST or SW_ENA
 *
 * \details 
 * 0: No action
 * 1: Set TX_RESYNC, HW will clear this bit when sync has completed
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_XFI_MODE . TX_RESYNC
 */
#define  VTSS_F_XFI_XFI_CONTROL_XFI_MODE_TX_RESYNC(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_XFI_CONTROL_XFI_MODE_TX_RESYNC     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_XFI_CONTROL_XFI_MODE_TX_RESYNC(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * SW_ENA input to Daytona_clk_rst_sync modules (internal Tx-clock, core Tx
 * clock-input, internal Rx-clock)
 *
 * \details 
 * 0: SW_ENA deasserted (default)
 * 1: SW_ENA asserted
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_XFI_MODE . SW_ENA
 */
#define  VTSS_F_XFI_XFI_CONTROL_XFI_MODE_SW_ENA(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_XFI_CONTROL_XFI_MODE_SW_ENA     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_XFI_CONTROL_XFI_MODE_SW_ENA(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * SW_RST input to Daytona_clk_rst_sync modules (internal Tx-clock, core Tx
 * clock-input, internal Rx-clock)
 *
 * \details 
 * 0: SW_RST deasserted
 * 1: SW_RST asserted (default)

 *
 * Field: ::VTSS_XFI_XFI_CONTROL_XFI_MODE . SW_RST
 */
#define  VTSS_F_XFI_XFI_CONTROL_XFI_MODE_SW_RST(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_XFI_CONTROL_XFI_MODE_SW_RST     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_XFI_CONTROL_XFI_MODE_SW_RST(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief XFI interrupt status
 *
 * \details
 * The XFI_INT, its sources, and the XFI_SSF are visible from this
 * register.
 *
 * Register: \a XFI::XFI_CONTROL::XFI_STATUS
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_XFI_CONTROL_XFI_STATUS(target)  VTSS_IOREG(target,0x1)

/** 
 * \brief
 * Status of the XFI_SSF output port
 *
 * \details 
 * 0: XFI_SSF deasserted
 * 1: XFI_SSF asserted
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_XFI_STATUS . SSF_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_XFI_STATUS_SSF_STATUS(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_XFI_XFI_CONTROL_XFI_STATUS_SSF_STATUS     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_XFI_XFI_CONTROL_XFI_STATUS_SSF_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \brief
 * Unmasked status of the ANEG_KR_INT status output
 * When the ANEG_KR_INT_MASK is set, this status is a source of the
 * XFI_INT.
 *
 * \details 
 * 0: The ANEG_KR_INT status is deasserted (low)
 * 1: The ANEG_KR_INT status is asserted (high)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_XFI_STATUS . ANEG_KR_INT_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_XFI_STATUS_ANEG_KR_INT_STATUS(x)  VTSS_ENCODE_BITFIELD(x,18,1)
#define  VTSS_M_XFI_XFI_CONTROL_XFI_STATUS_ANEG_KR_INT_STATUS     VTSS_ENCODE_BITMASK(18,1)
#define  VTSS_X_XFI_XFI_CONTROL_XFI_STATUS_ANEG_KR_INT_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,18,1)

/** 
 * \brief
 * Unmasked status of the ANEG_ACTIVE status output
 * When the ANEG_ACTIVE_MASK is set, this status is a source of the
 * XFI_INT.
 *
 * \details 
 * 0: The ANEG_ACTIVE status is deasserted (low)
 * 1: The ANEG_ACTIVE status is asserted (high)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_XFI_STATUS . ANEG_ACTIVE_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_XFI_STATUS_ANEG_ACTIVE_STATUS(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_XFI_XFI_CONTROL_XFI_STATUS_ANEG_ACTIVE_STATUS     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_XFI_XFI_CONTROL_XFI_STATUS_ANEG_ACTIVE_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Unmasked status of the lane-B AE sticky and mask bits. When the
 * AE_B_MASK is set, this status is a source of the XFI_INT.
 *
 * \details 
 * 0: None of the masked (enabled) lane-B AE sticky bits are set
 * 1: At least one of the masked (enabled) lane-B AE sticky bits are set
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_XFI_STATUS . AE_B_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_XFI_STATUS_AE_B_STATUS(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_XFI_XFI_CONTROL_XFI_STATUS_AE_B_STATUS     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_XFI_XFI_CONTROL_XFI_STATUS_AE_B_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Unmasked status of the lane-A AE sticky and mask bits. When the
 * AE_A_MASK is set, this status is a source of the XFI_INT.
 *
 * \details 
 * 0: None of the masked (enabled) lane-A AE sticky bits are set
 * 1: At least one of the masked (enabled) lane-A AE sticky bits are set
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_XFI_STATUS . AE_A_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_XFI_STATUS_AE_A_STATUS(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_XFI_XFI_CONTROL_XFI_STATUS_AE_A_STATUS     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_XFI_XFI_CONTROL_XFI_STATUS_AE_A_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Unmasked status of the OTN_FRM sticky and mask bits. When the
 * OTN_FRM_MASK is set, this status is a source of the XFI_INT.
 *
 * \details 
 * 0: None of the masked (enabled) OTN_FRM sticky bits are set
 * 1: At least one of the masked (enabled) OTN_FRM sticky bits are set
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_XFI_STATUS . OTN_FRM_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_XFI_STATUS_OTN_FRM_STATUS(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_XFI_CONTROL_XFI_STATUS_OTN_FRM_STATUS     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_XFI_CONTROL_XFI_STATUS_OTN_FRM_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Status of XFI_INT output port
 * XFI_INT = XFI_MASK & (RXINT_STATUS | TXINT_STATUS)
 *
 * \details 
 * 0: XFI_INT deasserted
 * 1: XFI_INT asserted
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_XFI_STATUS . XFIINT_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_XFI_STATUS_XFIINT_STATUS(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_XFI_CONTROL_XFI_STATUS_XFIINT_STATUS     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_XFI_CONTROL_XFI_STATUS_XFIINT_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief SSF INT control/masks and global XFI INT mask
 *
 * \details
 * Control and mask of the asynchronous XFI_SFF output. The global XFI_INT
 * mask located here supersede the individual masks.
 * Note: SSF status is also captured in the HSS_STICKY and HSS_STATUS
 * registers.
 *
 * Register: \a XFI::XFI_CONTROL::INT_CTRL
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_XFI_CONTROL_INT_CTRL(target)  VTSS_IOREG(target,0x2)

/** 
 * \brief
 * Invert polarity of rxb_sigdet_i prior to summing it with rxb_los_i to
 * generate xfi_ssf_o.
 * See SSF_MASK Encoding field.
 *
 * \details 
 * 0: Do not invert the RXB_SIGDET before summing it to generate the
 * XFI_SSF output (default)
 * 1: Invert the RXB_SIGDET before summing it to generate the XFI_SSF
 * output
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_INT_CTRL . RXBSIGDET_INV
 */
#define  VTSS_F_XFI_XFI_CONTROL_INT_CTRL_RXBSIGDET_INV(x)  VTSS_ENCODE_BITFIELD(x,31,1)
#define  VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXBSIGDET_INV     VTSS_ENCODE_BITMASK(31,1)
#define  VTSS_X_XFI_XFI_CONTROL_INT_CTRL_RXBSIGDET_INV(x)  VTSS_EXTRACT_BITFIELD(x,31,1)

/** 
 * \brief
 * Enable rxb_sigdet_i prior to summing it with rxb_los_i to generate
 * xfi_ssf_o.
 * See SSF_MASK Encoding field.
 *
 * \details 
 * 0: Inhibit the RXB_SIGDET (either polarity) from the XFI_SSF output
 * 1: Enable the RXB_SIGDET (either polarity) to the XFI_SSF output
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_INT_CTRL . RXBSIGDET_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_INT_CTRL_RXBSIGDET_MASK(x)  VTSS_ENCODE_BITFIELD(x,30,1)
#define  VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXBSIGDET_MASK     VTSS_ENCODE_BITMASK(30,1)
#define  VTSS_X_XFI_XFI_CONTROL_INT_CTRL_RXBSIGDET_MASK(x)  VTSS_EXTRACT_BITFIELD(x,30,1)

/** 
 * \brief
 * Invert polarity of rxb_los_i prior to summing it with rxb_sigdet_i to
 * generate xfi_ssf_o.
 * See SSF_MASK Encoding field.
 *
 * \details 
 * 0: Do not invert the RXB_LOS before summing it to generate the XFI_SSF
 * output (default)
 * 1: Invert the RXB_LOS before summing it to generate the XFI_SSF output
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_INT_CTRL . RXB_LOS_INV
 */
#define  VTSS_F_XFI_XFI_CONTROL_INT_CTRL_RXB_LOS_INV(x)  VTSS_ENCODE_BITFIELD(x,29,1)
#define  VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXB_LOS_INV     VTSS_ENCODE_BITMASK(29,1)
#define  VTSS_X_XFI_XFI_CONTROL_INT_CTRL_RXB_LOS_INV(x)  VTSS_EXTRACT_BITFIELD(x,29,1)

/** 
 * \brief
 * Enable rxb_los_i prior to summing it with rxb_sigdet_i to generate
 * xfi_ssf_o.
 * See SSF_MASK Encoding field.
 *
 * \details 
 * 0: Inhibit the RXB_LOS (either polarity) from the XFI_SSF output
 * 1: Enable the RXB_LOS (either polarity) to the XFI_SSF output
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_INT_CTRL . RXB_LOS_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_INT_CTRL_RXB_LOS_MASK(x)  VTSS_ENCODE_BITFIELD(x,28,1)
#define  VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXB_LOS_MASK     VTSS_ENCODE_BITMASK(28,1)
#define  VTSS_X_XFI_XFI_CONTROL_INT_CTRL_RXB_LOS_MASK(x)  VTSS_EXTRACT_BITFIELD(x,28,1)

/** 
 * \brief
 * Invert polarity of rxa_sigdet_i prior to summing it with rxa_los_i to
 * generate xfi_ssf_o.
 * See SSF_MASK Encoding field.
 *
 * \details 
 * 0: Do not invert the RXA_SIGDET before summing it to generate the
 * XFI_SSF output (default)
 * 1: Invert the RXA_SIGDET before summing it to generate the XFI_SSF
 * output
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_INT_CTRL . RXASIGDET_INV
 */
#define  VTSS_F_XFI_XFI_CONTROL_INT_CTRL_RXASIGDET_INV(x)  VTSS_ENCODE_BITFIELD(x,27,1)
#define  VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXASIGDET_INV     VTSS_ENCODE_BITMASK(27,1)
#define  VTSS_X_XFI_XFI_CONTROL_INT_CTRL_RXASIGDET_INV(x)  VTSS_EXTRACT_BITFIELD(x,27,1)

/** 
 * \brief
 * Enable rxa_sigdet_i prior to summing it with rxa_los_i to generate
 * xfi_ssf_o.
 * See SSF_MASK Encoding field.
 *
 * \details 
 * 0: Inhibit the RXA_SIGDET (either polarity) from the XFI_SSF output
 * 1: Enable the RXA_SIGDET (either polarity) to the XFI_SSF output
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_INT_CTRL . RXASIGDET_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_INT_CTRL_RXASIGDET_MASK(x)  VTSS_ENCODE_BITFIELD(x,26,1)
#define  VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXASIGDET_MASK     VTSS_ENCODE_BITMASK(26,1)
#define  VTSS_X_XFI_XFI_CONTROL_INT_CTRL_RXASIGDET_MASK(x)  VTSS_EXTRACT_BITFIELD(x,26,1)

/** 
 * \brief
 * Invert polarity of rxa_los_i prior to summing it with rxa_sigdet_i to
 * generate xfi_ssf_o.
 * See SSF_MASK Encoding field.
 *
 * \details 
 * 0: Do not invert the RXA_LOS before summing it to generate the XFI_SSF
 * output (default)
 * 1: Invert the RXA_LOS before summing it to generate the XFI_SSF output
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_INT_CTRL . RXA_LOS_INV
 */
#define  VTSS_F_XFI_XFI_CONTROL_INT_CTRL_RXA_LOS_INV(x)  VTSS_ENCODE_BITFIELD(x,25,1)
#define  VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXA_LOS_INV     VTSS_ENCODE_BITMASK(25,1)
#define  VTSS_X_XFI_XFI_CONTROL_INT_CTRL_RXA_LOS_INV(x)  VTSS_EXTRACT_BITFIELD(x,25,1)

/** 
 * \brief
 * Enable rxa_los_i prior to summing it with rxa_sigdet_i to generate
 * xfi_ssf_o.
 * See SSF_MASK Encoding field.
 *
 * \details 
 * 0: Inhibit the RXA_LOS (either polarity) from the XFI_SSF output
 * 1: Enable the RXA_LOS (either polarity) to the XFI_SSF output
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_INT_CTRL . RXA_LOS_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_INT_CTRL_RXA_LOS_MASK(x)  VTSS_ENCODE_BITFIELD(x,24,1)
#define  VTSS_M_XFI_XFI_CONTROL_INT_CTRL_RXA_LOS_MASK     VTSS_ENCODE_BITMASK(24,1)
#define  VTSS_X_XFI_XFI_CONTROL_INT_CTRL_RXA_LOS_MASK(x)  VTSS_EXTRACT_BITFIELD(x,24,1)

/** 
 * \brief
 * Global mask (enable) the XFI_SSF output.
 * xfi_ssf_o =	SSF_MASK &
 *   (sel_laneb=0) ? ( ( rxa_los_mask & (rxa_los_inv ^ rxa_los_i)
 *   | rxa_sigdet_mask & (rxa_sigdet_inv ^ rxa_sigdet_i)
 *   : ( rxb_los_mask & (rxb_los_inv ^ rxb_los_i)
 *   | rxb_sigdet_mask & (rxb_sigdet_inv ^ rxb_sigdet_i)
 *
 * \details 
 * 0: Inhibit XFI_SSF from being asserted
 * 1: Enable the XFI_SSF to use the RX_LOS and the RX_SIGDET states,
 * polarities, and masks
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_INT_CTRL . SSF_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_INT_CTRL_SSF_MASK(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_XFI_XFI_CONTROL_INT_CTRL_SSF_MASK     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_XFI_XFI_CONTROL_INT_CTRL_SSF_MASK(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \brief
 * Global mask (enable) the ANEG_INT status as a source of the XFI_INT.
 *
 * \details 
 * 0: Disable ANEG_KR_INT status from setting XFI_INT (default)
 * 1: Enable ANEG_KR_INT status to set XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_INT_CTRL . ANEG_KR_INT_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_INT_CTRL_ANEG_KR_INT_MASK(x)  VTSS_ENCODE_BITFIELD(x,18,1)
#define  VTSS_M_XFI_XFI_CONTROL_INT_CTRL_ANEG_KR_INT_MASK     VTSS_ENCODE_BITMASK(18,1)
#define  VTSS_X_XFI_XFI_CONTROL_INT_CTRL_ANEG_KR_INT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,18,1)

/** 
 * \brief
 * Global mask (enable) the ANEG_ACTIVE status as a source of the XFI_INT.
 *
 * \details 
 * 0: Disable ANEG_ACTIVE status from setting XFI_INT (default)
 * 1: Enable ANEG_ACTIVE status to set XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_INT_CTRL . ANEG_ACTIVE_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_INT_CTRL_ANEG_ACTIVE_MASK(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_XFI_XFI_CONTROL_INT_CTRL_ANEG_ACTIVE_MASK     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_XFI_XFI_CONTROL_INT_CTRL_ANEG_ACTIVE_MASK(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Global mask (enable) the lane-B AE interrupt as a source of the XFI_INT.
 *
 * \details 
 * 0: Disable lane-B AE interrupt from setting XFI_INT (default)
 * 1: Enable lane-B AE interrupt to set XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_INT_CTRL . AE_B_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_INT_CTRL_AE_B_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_XFI_XFI_CONTROL_INT_CTRL_AE_B_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_XFI_XFI_CONTROL_INT_CTRL_AE_B_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Global mask (enable) the lane-A AE interrupt as a source of the XFI_INT.
 *
 * \details 
 * 0: Disable lane-A AE interrupt from setting XFI_INT (default)
 * 1: Enable lane-A AE interrupt to set XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_INT_CTRL . AE_A_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_INT_CTRL_AE_A_MASK(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_XFI_XFI_CONTROL_INT_CTRL_AE_A_MASK     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_XFI_XFI_CONTROL_INT_CTRL_AE_A_MASK(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Global mask (enable) the OTN_FRM interrupt as a source of the XFI_INT.
 *
 * \details 
 * 0: Disable OTN_FRM interrupt from setting XFI_INT (default)
 * 1: Enable OTN_FRM interrupt to set XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_INT_CTRL . OTN_FRM_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_INT_CTRL_OTN_FRM_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_XFI_CONTROL_INT_CTRL_OTN_FRM_MASK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_XFI_CONTROL_INT_CTRL_OTN_FRM_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Global mask (enable) the XFI_INT output.
 *
 * \details 
 * 0: Disable XFI_INT output, regardless of the STICKY and corresponding
 * MASK states
 * 1: Enable XFI_INT, using STICKY and corresponding MASK states
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_INT_CTRL . XFI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_INT_CTRL_XFI_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_XFI_CONTROL_INT_CTRL_XFI_MASK     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_XFI_CONTROL_INT_CTRL_XFI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief XFI HSS macro status sticky bits
 *
 * \details
 * Register: \a XFI::XFI_CONTROL::HSS_STICKY
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_XFI_CONTROL_HSS_STICKY(target)  VTSS_IOREG(target,0x3)

/** 
 * \brief
 * XFI RXB_LOS input pin (GPIO bit 4) was detected asserted (since this bit
 * was last cleared)
 *
 * \details 
 * 0: RXB_LOS was not detected asserted (since this bit was last cleared)
 * 1: RXB_LOS was detected asserted (since this bit was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STICKY . RXB_LOS_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STICKY_RXB_LOS_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,31,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXB_LOS_HI_STICKY     VTSS_ENCODE_BITMASK(31,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STICKY_RXB_LOS_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,31,1)

/** 
 * \brief
 * XFI RXB_LOS input pin (GPIO bit 4) was detected deasserted (since this
 * bit was last cleared)
 *
 * \details 
 * 0: RXB_LOS was not detected deasserted (since this bit was last cleared)
 * 1: RXB_LOS was detected deasserted (since this bit was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STICKY . RXB_LOS_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STICKY_RXB_LOS_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,30,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXB_LOS_LO_STICKY     VTSS_ENCODE_BITMASK(30,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STICKY_RXB_LOS_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,30,1)

/** 
 * \brief
 * RXBSIGDET_STATUS in HSS_STATUS was detected asserted (since this bit was
 * last cleared)
 *
 * \details 
 * 0 = RXBSIGDET was not detected asserted (since this bit was last
 * cleared);
 * 1 = RXBSIGDET was detected asserted (since this bit was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STICKY . RXBSIGDET_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STICKY_RXBSIGDET_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,29,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXBSIGDET_HI_STICKY     VTSS_ENCODE_BITMASK(29,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STICKY_RXBSIGDET_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,29,1)

/** 
 * \brief
 * RXBSIGDET_STATUS in HSS_STATUS was detected deasserted (since this bit
 * was last cleared)
 *
 * \details 
 * 0: RXBSIGDET was not detected deasserted (since this bit was last
 * cleared)
 * 1: RXBSIGDET was detected deasserted (since this bit was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STICKY . RXBSIGDET_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STICKY_RXBSIGDET_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,28,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXBSIGDET_LO_STICKY     VTSS_ENCODE_BITMASK(28,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STICKY_RXBSIGDET_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,28,1)

/** 
 * \brief
 * The TX_FIFO was detected asserted (since this bit was last cleared)
 *
 * \details 
 * 0: TX_FIFO_LOS was not detected asserted (since this bit was last
 * cleared)
 * 1: TX_FIFO_LOS was detected asserted (since this bit was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STICKY . TX_FIFO_LOS_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STICKY_TX_FIFO_LOS_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,21,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_TX_FIFO_LOS_HI_STICKY     VTSS_ENCODE_BITMASK(21,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STICKY_TX_FIFO_LOS_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,21,1)

/** 
 * \brief
 * IFPLL_LOCK_STATUS in HSS_STATUS was detected asserted (since this bit
 * was last cleared)
 *
 * \details 
 * 0: IFPLL_LOCK was not detected asserted (since this bit was last
 * cleared)
 * 1: IFPLL_LOCK was detected asserted (since this bit was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STICKY . IFPLL_LOCK_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STICKY_IFPLL_LOCK_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,17,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_IFPLL_LOCK_HI_STICKY     VTSS_ENCODE_BITMASK(17,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STICKY_IFPLL_LOCK_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,17,1)

/** 
 * \brief
 * IFPLL_LOCK_STATUS in HSS_STATUS was detected deasserted (since this bit
 * was last cleared).
 *
 * \details 
 * 0: IFPLL_LOCK was not detected deasserted (since this bit was last
 * cleared)
 * 1: IFPLL_LOCK was detected deasserted (since this bit was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STICKY . IFPLL_LOCK_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STICKY_IFPLL_LOCK_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_IFPLL_LOCK_LO_STICKY     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STICKY_IFPLL_LOCK_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * RXA_LOS_STATUS in HSS_STATUS was detected asserted (since this bit was
 * last cleared).
 *
 * \details 
 * 0: RXA_LOS was not detected asserted (since this bit was last cleared)
 * 1: RXA_LOS was detected asserted (since this bit was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STICKY . RXA_LOS_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STICKY_RXA_LOS_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXA_LOS_HI_STICKY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STICKY_RXA_LOS_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * RXA_LOS_STATUS in HSS_STATUS was detected deasserted (since this bit was
 * last cleared)
 *
 * \details 
 * 0: RXA_LOS was not detected deasserted (since this bit was last cleared)
 * 1: RXA_LOS was detected deasserted (since this bit was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STICKY . RXA_LOS_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STICKY_RXA_LOS_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXA_LOS_LO_STICKY     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STICKY_RXA_LOS_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * RXASIGDET_STATUS in HSS_STATUS was detected asserted (since this bit was
 * last cleared)
 *
 * \details 
 * 0: RXASIGDET was not detected asserted (since this bit was last cleared)
 * 1: RXASIGDET was detected asserted (since this bit was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STICKY . RXASIGDET_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STICKY_RXASIGDET_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXASIGDET_HI_STICKY     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STICKY_RXASIGDET_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * RXASIGDET_STATUS in HSS_STATUS was detected deasserted (since this bit
 * was last cleared)
 *
 * \details 
 * 0: RXASIGDET was not detected deasserted (since this bit was last
 * cleared)
 * 1: RXASIGDET was detected deasserted (since this bit was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STICKY . RXASIGDET_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STICKY_RXASIGDET_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXASIGDET_LO_STICKY     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STICKY_RXASIGDET_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * RXPRBSERR_STATUS in HSS_STATUS was detected asserted (since this bit was
 * last cleared)
 *
 * \details 
 * 0: RXPRBSERR was not detected asserted (since this bit was last cleared)
 * 1: RXPRBSERR was detected asserted (since this bit was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STICKY . RXPRBSERR_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STICKY_RXPRBSERR_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXPRBSERR_HI_STICKY     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STICKY_RXPRBSERR_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * RXPRBSERR_STATUS in HSS_STATUS was detected deasserted (since this bit
 * was last cleared)
 *
 * \details 
 * 0: RXPRBSERR was not detected deasserted (since this bit was last
 * cleared)
 * 1: RXPRBSERR was detected deasserted (since this bit was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STICKY . RXPRBSERR_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STICKY_RXPRBSERR_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXPRBSERR_LO_STICKY     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STICKY_RXPRBSERR_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * RXPRBSSYNC_STATUS in HSS_STATUS was detected asserted (since this bit
 * was last cleared)
 *
 * \details 
 * 0: RXPRBSYNC was not detected asserted (since this bit was last cleared)
 * 1: RXPRBSYNC was detected asserted (since this bit was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STICKY . RXPRBSSYNC_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STICKY_RXPRBSSYNC_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXPRBSSYNC_HI_STICKY     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STICKY_RXPRBSSYNC_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * RXPRBSSYNC_STATUS in HSS_STATUS was detected deasserted (since this bit
 * was last cleared)
 *
 * \details 
 * 0: RXPRBSYNC was not detected asserted (since this bit was last cleared)
 * 1: RXPRBSYNC was detected asserted (since this bit was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STICKY . RXPRBSSYNC_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STICKY_RXPRBSSYNC_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_RXPRBSSYNC_LO_STICKY     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STICKY_RXPRBSSYNC_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * HSSPRTREADY_STATUS in HSS_STATUS was detected asserted (since this bit
 * was last cleared)
 *
 * \details 
 * 0: HSSPRTREADY was not detected asserted (since this bit was last
 * cleared)
 * 1: HSSPRTREADY was detected asserted (since this bit was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STICKY . HSSPRTREADY_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STICKY_HSSPRTREADY_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_HSSPRTREADY_HI_STICKY     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STICKY_HSSPRTREADY_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * HSSPRTREADY_STATUS in HSS_STATUS was detected deasserted (since this bit
 * was last cleared)
 *
 * \details 
 * 0: HSSPRTREADY was not detected deasserted (since this bit was last
 * cleared)
 * 1: HSSPRTREADY was detected deasserted (since this bit was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STICKY . HSSPRTREADY_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STICKY_HSSPRTREADY_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_HSSPRTREADY_LO_STICKY     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STICKY_HSSPRTREADY_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * HSSEYEQUALITY_STATUS in HSS_STATUS was detected asserted (since this bit
 * was last cleared)
 *
 * \details 
 * 0: HSSEYEQUALITY was not detected asserted (since this bit was last
 * cleared)
 * 1: HSSEYEQUALITY was detected asserted (since this bit was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STICKY . HSSEYEQUALITY_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STICKY_HSSEYEQUALITY_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_HSSEYEQUALITY_HI_STICKY     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STICKY_HSSEYEQUALITY_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * HSSEYEQUALITY_STATUS in HSS_STATUS was detected asserted (since this bit
 * was last cleared)
 *
 * \details 
 * 0: HSSEYEQUALITY was not detected deasserted (since this bit was last
 * cleared)
 * 1: HSSEYEQUALITY was detected deasserted (since this bit was last
 * cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STICKY . HSSEYEQUALITY_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STICKY_HSSEYEQUALITY_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_HSSEYEQUALITY_LO_STICKY     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STICKY_HSSEYEQUALITY_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * HSSRESETOUT_STATUS in HSS_STATUS was detected asserted (since this bit
 * was last cleared)
 *
 * \details 
 * 0: HSSRESETOUT was not detected asserted (since this bit was last
 * cleared)
 * 1: HSSRESETOUT was detected asserted (since this bit was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STICKY . HSSRESETOUT_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STICKY_HSSRESETOUT_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_HSSRESETOUT_HI_STICKY     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STICKY_HSSRESETOUT_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * HSSRESETOUT_STATUS in HSS_STATUS was detected deasserted (since this bit
 * was last cleared)
 *
 * \details 
 * 0: HSSRESETOUT was not detected asserted (since this bit was last
 * cleared)
 * 1: HSSRESETOUT was detected asserted (since this bit was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STICKY . HSSRESETOUT_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STICKY_HSSRESETOUT_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_HSSRESETOUT_LO_STICKY     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STICKY_HSSRESETOUT_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * HSSPLLLOCK_STATUS in HSS_STATUS was detected asserted (since this bit
 * was last cleared)
 *
 * \details 
 * 0: HSSPLLLOCK was not detected asserted (since this bit was last
 * cleared)
 * 1: HSSPLLLOCK was detected asserted (since this bit was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STICKY . HSSPLLLOCK_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STICKY_HSSPLLLOCK_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_HSSPLLLOCK_HI_STICKY     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STICKY_HSSPLLLOCK_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * HSSPLLLOCK_STATUS in HSS_STATUS was detected deasserted (since this bit
 * was last cleared)
 *
 * \details 
 * 0: HSSPLLLOCK was not detected deasserted (since this bit was last
 * cleared)
 * 1: HSSPLLLOCK was detected deasserted (since this bit was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STICKY . HSSPLLLOCK_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STICKY_HSSPLLLOCK_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STICKY_HSSPLLLOCK_LO_STICKY     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STICKY_HSSPLLLOCK_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief XFI HSS macro sticky interrupt masks (enables) 
 *
 * \details
 * Register: \a XFI::XFI_CONTROL::HSS_MASK
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_XFI_CONTROL_HSS_MASK(target)  VTSS_IOREG(target,0x4)

/** 
 * \brief
 * Mask (enable) a XFI_INT if RXB_LOS_HI_STICKY is asserted
 *
 * \details 
 * 0: Inhibit RXB_LOS_HI_STICKY from generating a XFI_INT
 * 1: Enable RXB_LOS_HI_STICKY to generate a XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_MASK . RXB_LOS_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_MASK_RXB_LOS_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,31,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_MASK_RXB_LOS_HI_MASK     VTSS_ENCODE_BITMASK(31,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_MASK_RXB_LOS_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,31,1)

/** 
 * \brief
 * Mask (enable) a XFI_INT if RXB_LOS_LO_STICKY is asserted
 *
 * \details 
 * 0: Inhibit RXB_LOS_LO_STICKY from generating a XFI_INT
 * 1: enable RXB_LOS_LO_STICKY to generate a XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_MASK . RXB_LOS_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_MASK_RXB_LOS_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,30,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_MASK_RXB_LOS_LO_MASK     VTSS_ENCODE_BITMASK(30,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_MASK_RXB_LOS_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,30,1)

/** 
 * \brief
 * Mask (enable) a XFI_INT if RXBSIGDET_HI_STICKY is asserted
 *
 * \details 
 * 0: Inhibit RXBSIGDET_HI_STICKY from generating a XFI_INT
 * 1: Enable RXBSIGDET_HI_STICKY to generate a XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_MASK . RXBSIGDET_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_MASK_RXBSIGDET_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,29,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_MASK_RXBSIGDET_HI_MASK     VTSS_ENCODE_BITMASK(29,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_MASK_RXBSIGDET_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,29,1)

/** 
 * \brief
 * Mask (enable) a XFI_INT if RXASIGDET_LO_STICKY is asserted
 *
 * \details 
 * 0: Inhibit RXBSIGDET_LO_STICKY from generating a XFI_INT
 * 1: Enable RXBSIGDET_LO_STICKY to generate a XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_MASK . RXBSIGDET_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_MASK_RXBSIGDET_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,28,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_MASK_RXBSIGDET_LO_MASK     VTSS_ENCODE_BITMASK(28,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_MASK_RXBSIGDET_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,28,1)

/** 
 * \brief
 * Mask (enable) a XFI_INT if IF_PLL_LOCK_HI_STICKY is asserted
 *
 * \details 
 * 0: Inhibit TX_FIFO_LOS_HI_STICKY from generating a XFI_INT
 * 1: Enable TX_FIFO_LOSss_HI_STICKY to generate a XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_MASK . TX_FIFO_LOS_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_MASK_TX_FIFO_LOS_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,21,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_MASK_TX_FIFO_LOS_HI_MASK     VTSS_ENCODE_BITMASK(21,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_MASK_TX_FIFO_LOS_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,21,1)

/** 
 * \brief
 * Mask (enable) a XFI_INT if IF_PLL_LOCK_HI_STICKY is asserted
 *
 * \details 
 * 0: Inhibit IFPLL_LOCK_HI_STICKY from generating a XFI_INT
 * 1: Enable IFPLL_LOCK_HI_STICKY to generate a XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_MASK . IFPLL_LOCK_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_MASK_IFPLL_LOCK_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,17,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_MASK_IFPLL_LOCK_HI_MASK     VTSS_ENCODE_BITMASK(17,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_MASK_IFPLL_LOCK_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,17,1)

/** 
 * \brief
 * Mask (enable) a XFI_INT if IF_PLL_LOCK_LO_STICKY is asserted
 *
 * \details 
 * 0: Inhibit IFPLL_LOCK_LO_STICKY from generating a XFI_INT
 * 1: Enable IFPLL_LOCK_LO_STICKY to generate a XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_MASK . IFPLL_LOCK_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_MASK_IFPLL_LOCK_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_MASK_IFPLL_LOCK_LO_MASK     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_MASK_IFPLL_LOCK_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Mask (enable) a XFI_INT if RXA_LOS_HI_STICKY is asserted
 *
 * \details 
 * 0: Inhibit RXA_LOS_HI_STICKY from generating a XFI_INT
 * 1: Enable RXA_LOS_HI_STICKY to generate a XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_MASK . RXA_LOS_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_MASK_RXA_LOS_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_MASK_RXA_LOS_HI_MASK     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_MASK_RXA_LOS_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Mask (enable) a XFI_INT if RXA_LOS_LO_STICKY is asserted
 *
 * \details 
 * 0: Inhibit RXA_LOS_LO_STICKY from generating a XFI_INT
 * 1: Enable RXA_LOS_LO_STICKY to generate a XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_MASK . RXA_LOS_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_MASK_RXA_LOS_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_MASK_RXA_LOS_LO_MASK     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_MASK_RXA_LOS_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Mask (enable) a XFI_INT if RXASIGDET_HI_STICKY is asserted
 *
 * \details 
 * 0: Inhibit RXASIGDET_HI_STICKY from generating a XFI_INT
 * 1: Enable the RXASIGDET_HI_STICKY to generate a XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_MASK . RXASIGDET_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_MASK_RXASIGDET_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_MASK_RXASIGDET_HI_MASK     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_MASK_RXASIGDET_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Mask (enable) a XFI_INT if RXASIGDET_LO_STICKY is asserted
 *
 * \details 
 * 0: Inhibit RXASIGDET_LO_STICKY from generating a XFI_INT
 * 1: Enable RXASIGDET_LO_STICKY to generate a XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_MASK . RXASIGDET_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_MASK_RXASIGDET_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_MASK_RXASIGDET_LO_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_MASK_RXASIGDET_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Mask (enable) a XFI_INT if RXPRBSERR_HI_STICKY is asserted
 *
 * \details 
 * 0: Inhibit RXAPRBSERR_HI_STICKY from generating a XFI_INT
 * 1: Enable RXAPRBSERR_HI_STICKY to generate a XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_MASK . RXPRBSERR_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_MASK_RXPRBSERR_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_MASK_RXPRBSERR_HI_MASK     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_MASK_RXPRBSERR_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Mask (enable) a XFI_INT if RXPRBSERR_LO_STICKY is asserted
 *
 * \details 
 * 0: Inhibit RXPRBSERR_LO_STICKY from generating a XFI_INT
 * 1: Enable RXPRBSERR_LO_STICKY to generate a XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_MASK . RXPRBSERR_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_MASK_RXPRBSERR_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_MASK_RXPRBSERR_LO_MASK     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_MASK_RXPRBSERR_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Mask (enable) a XFI_INT if RXPRBSYNC_HI_STICKY is asserted
 *
 * \details 
 * 0: Inhibit RXPRBSYNC_HI_STICKY from generating a XFI_INT
 * 1: Enable RXPRBSYNC_HI_STICKY to generate a XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_MASK . RXPRBSSYNC_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_MASK_RXPRBSSYNC_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_MASK_RXPRBSSYNC_HI_MASK     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_MASK_RXPRBSSYNC_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Mask (enable) a XFI_INT if RXPRBSYNC_LO_STICKY is asserted
 *
 * \details 
 * 0: Inhibit RXPRBSYNC_LO_STICKY from generating a XFI_INT
 * 1: Enable RXPRBSYNC_LO_STICKY to generate a XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_MASK . RXPRBSSYNC_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_MASK_RXPRBSSYNC_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_MASK_RXPRBSSYNC_LO_MASK     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_MASK_RXPRBSSYNC_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Mask (enable) a XFI_INT if HSSPRTREADY_HI_STICKY is asserted
 *
 * \details 
 * 0: Inhibit HSSPRTREADY_HI_STICKY from generating a XFI_INT
 * 1: Enable HSSPRTREADY_HI_STICKY to generate a XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_MASK . HSSPRTREADY_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_MASK_HSSPRTREADY_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_MASK_HSSPRTREADY_HI_MASK     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_MASK_HSSPRTREADY_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Mask (enable) a XFI_INT if HSSPRTREADY_LO_STICKY is asserted
 *
 * \details 
 * 0: Inhibit HSSPRTREADY_LO_STICKY from generating a XFI_INT
 * 1: Enable HSSPRTREADY_LO_STICKY to generate a XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_MASK . HSSPRTREADY_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_MASK_HSSPRTREADY_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_MASK_HSSPRTREADY_LO_MASK     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_MASK_HSSPRTREADY_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Mask (enable) a XFI_INT if HSSEYEQUALITY_HI_STICKY is asserted
 *
 * \details 
 * 0: Inhibit HSSEYEQUALITY_HI_STICKY from generating a XFI_INT
 * 1: Enable HSSEYEQUALITY_HI_STICKY to generate a XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_MASK . HSSEYEQUALITY_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_MASK_HSSEYEQUALITY_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_MASK_HSSEYEQUALITY_HI_MASK     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_MASK_HSSEYEQUALITY_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Mask (enable) a XFI_INT if HSSEYEQUALITY_LO_STICKY is asserted
 *
 * \details 
 * 0: Inhibit HSSEYEQUALITY_LO_STICKY from generating a XFI_INT
 * 1: Enable HSSEYEQUALITY_LO_STICKY to generate a XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_MASK . HSSEYEQUALITY_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_MASK_HSSEYEQUALITY_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_MASK_HSSEYEQUALITY_LO_MASK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_MASK_HSSEYEQUALITY_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Mask (enable) a XFI_INT if HSSRESETOUT_HI_STICKY is asserted
 *
 * \details 
 * 0: Inhibit HSSRESETOUT_HI_STICKY from generating a XFI_INT
 * 1: Enable HSSRESETOUT_HI_STICKY to generate a XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_MASK . HSSRESETOUT_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_MASK_HSSRESETOUT_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_MASK_HSSRESETOUT_HI_MASK     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_MASK_HSSRESETOUT_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Mask (enable) a XFI_INT if HSSRESETOUT_LO_STICKY is asserted
 *
 * \details 
 * 0: Inhibit HSSRESETOUT_HI_STICKY from generating a XFI_INT
 * 1: Enable HSSRESETOUT_LO_STICKY to generate a XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_MASK . HSSRESETOUT_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_MASK_HSSRESETOUT_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_MASK_HSSRESETOUT_LO_MASK     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_MASK_HSSRESETOUT_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Mask (enable) a XFI_INT if HSSPLLLOCK_HI_STICKY is asserted
 *
 * \details 
 * 0: Inhibit HSSPLLLOCK_HI_STICKY from generating a XFI_INT
 * 1: Enable HSSPLLLOCK_HI_STICKY to generate a XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_MASK . HSSPLLLOCK_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_MASK_HSSPLLLOCK_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_MASK_HSSPLLLOCK_HI_MASK     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_MASK_HSSPLLLOCK_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Mask (enable) a XFI_INT if HSSPLLLOCK_LO_STICKY is asserted
 *
 * \details 
 * 0: Inhibit HSSRESETOUT_LO_STICKY from generating a XFI_INT
 * 1: Enable HSSRESETOUT_LO_STICKY to generate a XFI_INT
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_MASK . HSSPLLLOCK_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_MASK_HSSPLLLOCK_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_MASK_HSSPLLLOCK_LO_MASK     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_MASK_HSSPLLLOCK_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief XFI HSS macro current status
 *
 * \details
 * Register: \a XFI::XFI_CONTROL::HSS_STATUS
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_XFI_CONTROL_HSS_STATUS(target)  VTSS_IOREG(target,0x5)

/** 
 * \brief
 * Current value of lane B XFI LOS pin (GPIO bit 4) driven by HSS XFP/SFP+
 *
 * \details 
 * 0: Pin is driven low
 * 1: Pin is driven high
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STATUS . RXB_LOS_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STATUS_RXB_LOS_STATUS(x)  VTSS_ENCODE_BITFIELD(x,30,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STATUS_RXB_LOS_STATUS     VTSS_ENCODE_BITMASK(30,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STATUS_RXB_LOS_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,30,1)

/** 
 * \brief
 * Lane-B signal detect indicator
 *
 * \details 
 * 0: No signal
 * 1: Active signal detected on the serial input
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STATUS . RXBSIGDET_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STATUS_RXBSIGDET_STATUS(x)  VTSS_ENCODE_BITFIELD(x,28,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STATUS_RXBSIGDET_STATUS     VTSS_ENCODE_BITMASK(28,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STATUS_RXBSIGDET_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,28,1)

/** 
 * \brief
 * TX_FIFO loss of sync 
 * This can occur if the core clock sourcing the Tx port of the XFI is out
 * of phase with the XFI Tx clock, or if the core clock glitches.
 *
 * \details 
 * 0: TX_FIFO is not out of sync
 * 1: TX_FIFO is out of sync
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STATUS . TX_FIFO_LOS_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STATUS_TX_FIFO_LOS_STATUS(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STATUS_TX_FIFO_LOS_STATUS     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STATUS_TX_FIFO_LOS_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \brief
 * IF PLL locked indicator
 *
 * \details 
 * 0: Unlocked
 * 1: Locked
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STATUS . IFPLL_LOCK_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STATUS_IFPLL_LOCK_STATUS(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STATUS_IFPLL_LOCK_STATUS     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STATUS_IFPLL_LOCK_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Current value of lane-A XFI LOS pin (GPIO bit 12) driven by HSS XFP/SFP+
 *
 * \details 
 * 0: Pin is driven low
 * 1: Pin is driven high
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STATUS . RXA_LOS_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STATUS_RXA_LOS_STATUS(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STATUS_RXA_LOS_STATUS     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STATUS_RXA_LOS_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Lane-A signal detect indicator
 *
 * \details 
 * 0: No signal
 * 1: Active signal detected on the serial input
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STATUS . RXASIGDET_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STATUS_RXASIGDET_STATUS(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STATUS_RXASIGDET_STATUS     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STATUS_RXASIGDET_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Receiver BIST error flag
 * Once RXxPRBSSYNC is achieved, subsequent errors cause this signal to be
 * latched at 1. Passing condition is for RXxPRBSSYNC=1 and RXxPRBSERROR=0.
 * 
 * Note: This input reflects the selected (via SEL_LANEB) lane status.
 *
 * \details 
 * 0: No error
 * 1: Errors detected (latched, requires RXxPRBSRST to clear)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STATUS . RXPRBSERR_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STATUS_RXPRBSERR_STATUS(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STATUS_RXPRBSERR_STATUS     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STATUS_RXPRBSERR_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Receiver BIST sync flag
 * Note: This input reflects the selected (via SEL_LANEB) lane status.
 *
 * \details 
 * 0: BIST pattern checker not in sync
 * 1: BIST pattern checker has achieved sync since last RXxPRBSRST
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STATUS . RXPRBSSYNC_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STATUS_RXPRBSSYNC_STATUS(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STATUS_RXPRBSSYNC_STATUS     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STATUS_RXPRBSSYNC_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Parallel register port ready
 * Signifies that the HSS register port is ready to access Tx and Rx
 * registers (PLL registers can be accessed at any time the HSS PLL is
 * running).
 *
 * \details 
 * 0: Not ready
 * 1: Ready (after reset sequence and offset calibration completed)
 * Note: HSSPRTREADY may take up to 150 us to go to 0 after HSSPDWNPLL in
 * HSS_CONTROLS is set to 1.
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STATUS . HSSPRTREADY_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STATUS_HSSPRTREADY_STATUS(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STATUS_HSSPRTREADY_STATUS     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STATUS_HSSPRTREADY_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Eye qualitys status
 *
 * \details 
 * 0: Inactive, no new status information available for any Rx links in the
 * core
 * 1: Active, new status information is available for at least one Rx in
 * the core. When active, HSS register 0x1e for each Rx link can be read to
 * determine updated status.
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STATUS . HSSEYEQUALITY_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STATUS_HSSEYEQUALITY_STATUS(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STATUS_HSSEYEQUALITY_STATUS     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STATUS_HSSEYEQUALITY_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Reset output
 *
 * \details 
 * 0: Normal operation
 * 1: Reset in progress
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STATUS . HSSRESETOUT_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STATUS_HSSRESETOUT_STATUS(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STATUS_HSSRESETOUT_STATUS     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STATUS_HSSRESETOUT_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * HS PLL locked indicator
 *
 * \details 
 * 0: Unlocked
 * 1: Locked
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_STATUS . HSSPLLLOCK_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_STATUS_HSSPLLLOCK_STATUS(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_STATUS_HSSPLLLOCK_STATUS     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_STATUS_HSSPLLLOCK_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Discrete HSS controls
 *
 * \details
 * All discrete HSS inputs are contained here. Many of these signals have
 * duplicate functionality within the HSS registers.
 *
 * Register: \a XFI::XFI_CONTROL::HSS_CONTROLS
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_XFI_CONTROL_HSS_CONTROLS(target)  VTSS_IOREG(target,0x6)

/** 
 * \brief
 * Early input
 * External early input to internal rotator control logic. Based on the
 * setting of the internal/external early/late selection control in Rx
 * register 0x02, bit 4, each rising edge of this input moves the rotator
 * control logic down by one step. Early info entered this way makes use of
 * the internal filter state machine and flywheel.
 *
 * \details 
 * 0: Normal
 * 1: Step rotator down
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . RXEARLYIN
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_RXEARLYIN(x)  VTSS_ENCODE_BITFIELD(x,31,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_RXEARLYIN     VTSS_ENCODE_BITMASK(31,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_RXEARLYIN(x)  VTSS_EXTRACT_BITFIELD(x,31,1)

/** 
 * \brief
 * Late input
 * External late input to internal rotator control logic. Based on the
 * setting of the internal/external early/late selection control in Rx
 * register 0x02, bit 4, each rising edge of this input moves the rotator
 * control logic up by one step. Late info entered this way makes use of
 * the internal filter state machine and flywheel.
 *
 * \details 
 * 0: Normal
 * 1: Step rotator up
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . RXLATEIN
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_RXLATEIN(x)  VTSS_ENCODE_BITFIELD(x,30,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_RXLATEIN     VTSS_ENCODE_BITMASK(30,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_RXLATEIN(x)  VTSS_EXTRACT_BITFIELD(x,30,1)

/** 
 * \brief
 * Phase down input
 * External adjustment control to retard the phase rotators. Each rising
 * edge of this signal causes one incremental adjustment (retard) of the
 * phase rotators. RXxPHSLOCK must be 1 to enable this.
 *
 * \details 
 * 0: Normal
 * 1: Retard phase rotator 1 increment
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . RXPHSDNIN
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_RXPHSDNIN(x)  VTSS_ENCODE_BITFIELD(x,29,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_RXPHSDNIN     VTSS_ENCODE_BITMASK(29,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_RXPHSDNIN(x)  VTSS_EXTRACT_BITFIELD(x,29,1)

/** 
 * \brief
 * Phase up input
 * External adjustment control to advance the phase rotators. Each rising
 * edge of this signal causes one incremental adjustment (advance) of the
 * phase rotators. RXxPHSLOCK must be 1 to enable this. 
 *
 * \details 
 * 0: Normal
 * 1: Advance phase rotator 1 increment
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . RXPHSUPIN
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_RXPHSUPIN(x)  VTSS_ENCODE_BITFIELD(x,28,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_RXPHSUPIN     VTSS_ENCODE_BITMASK(28,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_RXPHSUPIN(x)  VTSS_EXTRACT_BITFIELD(x,28,1)

/** 
 * \brief
 * Enable external adjustment of phase rotator via RXxPHSDNIN and
 * RXxPHSUPIN input signals.
 *
 * \details 
 * 0: Disabled (normal operation)
 * 1: Enabled (external control of phase rotators)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . RXPHSLOCK
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_RXPHSLOCK(x)  VTSS_ENCODE_BITFIELD(x,27,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_RXPHSLOCK     VTSS_ENCODE_BITMASK(27,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_RXPHSLOCK(x)  VTSS_EXTRACT_BITFIELD(x,27,1)

/** 
 * \brief
 * Receiver BIST force error signal
 * Enables verification of the PRBS error detector.
 *
 * \details 
 * 0: Normal
 * 1: Forces errors in internal wrapback path by changing the wrapback mux
 * selector to the wrap back selection. When in internal wrap-back mode
 * this forces errors by changing the wrapback mux selector from the
 * wrapback path to the receiver serial path RXxPRBSEN internal input
 * receiver BIST enable.
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . RXPRBSFRCERR
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_RXPRBSFRCERR(x)  VTSS_ENCODE_BITFIELD(x,26,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_RXPRBSFRCERR     VTSS_ENCODE_BITMASK(26,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_RXPRBSFRCERR(x)  VTSS_EXTRACT_BITFIELD(x,26,1)

/** 
 * \brief
 * Receiver BIST reset signal
 *
 * \details 
 * 0: Normal
 * 1: Resets RXxPRBSSYNC and RXxPRBSERR latches, and restarts the BIST
 * process
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . RXPRBSRST
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_RXPRBSRST(x)  VTSS_ENCODE_BITFIELD(x,25,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_RXPRBSRST     VTSS_ENCODE_BITMASK(25,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_RXPRBSRST(x)  VTSS_EXTRACT_BITFIELD(x,25,1)

/** 
 * \brief
 * Receiver BIST enable signal
 *
 * \details 
 * 0: Normal (PRBS controlled via registers)
 * 1: Enables internal wrapback test
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . RXPRBSEN
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_RXPRBSEN(x)  VTSS_ENCODE_BITFIELD(x,24,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_RXPRBSEN     VTSS_ENCODE_BITMASK(24,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_RXPRBSEN(x)  VTSS_EXTRACT_BITFIELD(x,24,1)

/** 
 * \brief
 * Transmitter PRBS Reset Tx logic BIST reset signal
 *
 * \details 
 * 0: Normal
 * 1: Resets and restarts the test pattern generator
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . TXPRBSRST
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_TXPRBSRST(x)  VTSS_ENCODE_BITFIELD(x,23,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_TXPRBSRST     VTSS_ENCODE_BITMASK(23,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_TXPRBSRST(x)  VTSS_EXTRACT_BITFIELD(x,23,1)

/** 
 * \brief
 * Transmitter PRBS Enable Tx logic BIST enable signal
 *
 * \details 
 * 0: Normal
 * 1: Enables internal test pattern generator and connects it to the Tx
 * input instead of TXAD
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . TXPRBSEN
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_TXPRBSEN(x)  VTSS_ENCODE_BITFIELD(x,22,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_TXPRBSEN     VTSS_ENCODE_BITMASK(22,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_TXPRBSEN(x)  VTSS_EXTRACT_BITFIELD(x,22,1)

/** 
 * \brief
 * PRBS enable signal
 * This signal provides a single point control over the disabling of
 * loopback tests. This is used during manufacturing test to synchronously
 * halt internal PRBS checking, securing test results prior to entering the
 * scan out phase. This is to prevent corruption of test results when the
 * switch is made from functional mode to scan out mode.
 *
 * \details 
 * 1: Normal operation (default)
 * 0: Gate off the enable to the PRBS checker of all ports (Tx and Rx)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . HSSPRBSEN
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPRBSEN(x)  VTSS_ENCODE_BITFIELD(x,21,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPRBSEN     VTSS_ENCODE_BITMASK(21,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPRBSEN(x)  VTSS_EXTRACT_BITFIELD(x,21,1)

/** 
 * \brief
 * Reference clock divider
 * Control signal to the HS PLL reference clock divider
 *
 * \details 
 * 0: Normal operation
 * 1: Divide reference clock by 2
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . HSSREFDIV
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSREFDIV(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSREFDIV     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_HSSREFDIV(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \brief
 * Low frequency range control
 * Selects the 2/3 rate low frequency range divider on the output of the HS
 * PLL
 *
 * \details 
 * 0: Full frequency clock sent to Tx and Rx links (8.5 Gbps to 11.10 Gbps)
 * 1: Two thirds rate clock sent to Tx and Rx links (5.67 Gbps to 7.50
 * Gbps)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . HSSLOFREQPLL
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSLOFREQPLL(x)  VTSS_ENCODE_BITFIELD(x,19,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSLOFREQPLL     VTSS_ENCODE_BITMASK(19,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_HSSLOFREQPLL(x)  VTSS_EXTRACT_BITFIELD(x,19,1)

/** 
 * \brief
 * Low frequency range control 2
 * Selects the divide-by-3 path in the HS PLL feedback loop instead of a
 * divide-by-2 path. Used in conjunction with HSSDIVSEL and HSSREFDIV in
 * HSS_CONTROLS to set the relationship between the HSSREFCLK input
 * frequency and the HS PLL frequency.
 *
 * \details 
 * 0: Divide by 2
 * 1: Divide by 3
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . HSSLOFREQ2PLL
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSLOFREQ2PLL(x)  VTSS_ENCODE_BITFIELD(x,18,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSLOFREQ2PLL     VTSS_ENCODE_BITMASK(18,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_HSSLOFREQ2PLL(x)  VTSS_EXTRACT_BITFIELD(x,18,1)

/** 
 * \brief
 * HS PLL clock vs. PLLREF ratio selector
 * HSSREFDIV = 0
 *
 * \details 
 * 00: 16x
 * 01: 20x (default)
 * 10: 32x
 * 11: 40x
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . HSSDIVSEL
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL(x)  VTSS_ENCODE_BITFIELD(x,16,2)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL     VTSS_ENCODE_BITMASK(16,2)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_HSSDIVSEL(x)  VTSS_EXTRACT_BITFIELD(x,16,2)

/** 
 * \brief
 * Lane-B out of band signalling
 * Drives transmitter outputs to the DC common mode voltage
 *
 * \details 
 * 0: Normal
 * 1: OBS mode enabled
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . TXBOBS
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_TXBOBS(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_TXBOBS     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_TXBOBS(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Lane-A out of band signalling
 * Drives transmitter outputs to the DC common mode voltage
 *
 * \details 
 * 0: Normal
 * 1: OBS mode enabled
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . TXAOBS
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_TXAOBS(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_TXAOBS     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_TXAOBS(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Receiver AC coupling mode selector
 * Sets the receiver termination voltage
 *
 * \details 
 * 0: DC coupling mode (VTR)
 * 1: AC coupling mode (0.8*VTR)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . HSSRXACMODE
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRXACMODE(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRXACMODE     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRXACMODE(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Transmitter AC coupling mode selector
 * Sets the transmitter internal bias
 *
 * \details 
 * 0: DC coupling mode
 * 1: AC coupling mode
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . HSSTXACMODE
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSTXACMODE(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSTXACMODE     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_HSSTXACMODE(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Lane-B transmitter output disable
 * Disables the transmitter output drivers
 *
 * \details 
 * 0: Disable (transmitter outputs are pulled up to AVTT through internal
 * 50-ohm termination resistors)
 * 1: Normal operation
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . TXBTS
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_TXBTS(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_TXBTS     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_TXBTS(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Lane-A transmitter output disable
 * Disables the transmitter output drivers
 *
 * \details 
 * 0: Disable (transmitter outputs are pulled up to AVTT through internal
 * 50-ohm termination resistors)
 * 1: Normal operation
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . TXATS
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_TXATS(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_TXATS     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_TXATS(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * HS PLL calibration request
 *
 * \details 
 * 0: Normal operation
 * 1: Pulse high for minimum of 8 REFCLK cycles to restart VCO coarse
 * calibration
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . HSSRECCAL
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRECCAL(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRECCAL     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRECCAL(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Reset configuration
 * Used during manufacturing testing
 * RSTCONFIG2=0: Normal operation
 * RSTCONFIG2=1: Reset override used during manufacturing test
 * RSTCONFIG1:0 used to configure the loopback slave (wrapclock) condition
 * of the paired receivers during manufacturing testing
 *
 * \details 
 * 00: Normal operation
 * 01: Wrapback test, RXA, RXC, RXE, and RXG = master
 * 10: Wrapback test, RXB, RXD, RXF, and RXH = master
 * 11: Invalid
 * Receiver sensitivity test = 1xx
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . HSSRSTCONFIG
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRSTCONFIG(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRSTCONFIG     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRSTCONFIG(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * HS PLL power down
 * Setting this bit to 1 initiates a full core power down sequence. All of
 * the Tx and Rx state machines finish their current tasks then the
 * individual ports are disabled before the HS PLL is powered off. This may
 * take up to 150 us after this bit is set to 1.
 *
 * \details 
 * 0: Normal operation
 * 1: Power down the entire core
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . HSSPDWNPLL
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPDWNPLL(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * HS PLL bypass
 *
 * \details 
 * 0: Normal operation
 * 1: The HS PLL clock output is bypassed with the reference clock
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . HSSPLLBYP
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPLLBYP(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPLLBYP     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_HSSPLLBYP(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Resync HSS without reinitializing HSS registers
 * The HSSRESYNC is a one-shot register that will auto clear (through HW)
 * after the input to the HSS becomes asserted.
 * Like the HSSRESET, the HSSRESYNC will cause the HSSRESETOUT to assert,
 * the HSSPRTREADY to deassert, and the HSS clock outputs to temporarily
 * stop and reacquire sync. 
 * Unlike the HSSRESET, the HSSRESYNC will not affect the HSSPLLLOCK or the
 * HSSSIGDET outputs.
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . HSSRESYNC
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRESYNC(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRESYNC     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRESYNC(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Assert the HSSRESET port
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_HSS_CONTROLS . HSSRESET
 */
#define  VTSS_F_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRESET(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRESET     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_XFI_CONTROL_HSS_CONTROLS_HSSRESET(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Rx PRBS error counter
 *
 * \details
 * Counts assertions (leading edges) of the RXPRBSERR input. Hardware
 * automatically asserts the RXPRBSRST output (not visible to CSR) for two
 * cycles to clear the RXPRBSERR and restart the PRBS logic.
 *
 * Register: \a XFI::XFI_CONTROL::RXPRBSERR_CTR
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_XFI_CONTROL_RXPRBSERR_CTR(target)  VTSS_IOREG(target,0x7)

/** 
 * \brief
 * Rx PRBS error counter
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_RXPRBSERR_CTR . RXPRBSERR_CNTR
 */
#define  VTSS_F_XFI_XFI_CONTROL_RXPRBSERR_CTR_RXPRBSERR_CNTR(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_XFI_XFI_CONTROL_RXPRBSERR_CTR_RXPRBSERR_CNTR     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_XFI_XFI_CONTROL_RXPRBSERR_CTR_RXPRBSERR_CNTR(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Pattern checker control
 *
 * \details
 * Register: \a XFI::XFI_CONTROL::TESTPAT_MODE
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_XFI_CONTROL_TESTPAT_MODE(target)  VTSS_IOREG(target,0xa)

/** 
 * \brief
 * When this bit is asserted, values in the TESTPAT_ERRORS and the
 * TESTPAT_CYCS are snapshot at their current values and remain so until
 * this bit is deasserted.
 * Note: The counters continue to accumulate errors and cycles while this
 * bit is set, though the CSR read-values are latched.
 *
 * \details 
 * 0: Counter values are updated at the completion of each pattern-cycle
 * (default)
 * 1: Counters values are snapshot at the assertion of this bit
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_TESTPAT_MODE . TP_SNAPSHOT
 */
#define  VTSS_F_XFI_XFI_CONTROL_TESTPAT_MODE_TP_SNAPSHOT(x)  VTSS_ENCODE_BITFIELD(x,28,1)
#define  VTSS_M_XFI_XFI_CONTROL_TESTPAT_MODE_TP_SNAPSHOT     VTSS_ENCODE_BITMASK(28,1)
#define  VTSS_X_XFI_XFI_CONTROL_TESTPAT_MODE_TP_SNAPSHOT(x)  VTSS_EXTRACT_BITFIELD(x,28,1)

/** 
 * \brief
 * Bit-length of test-pattern (2K max)
 * Note: The bit-length must be an even multiple of the (decoded)
 * word-width (TP_WDWIDTH)
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_TESTPAT_MODE . TP_BITLENGTH
 */
#define  VTSS_F_XFI_XFI_CONTROL_TESTPAT_MODE_TP_BITLENGTH(x)  VTSS_ENCODE_BITFIELD(x,16,12)
#define  VTSS_M_XFI_XFI_CONTROL_TESTPAT_MODE_TP_BITLENGTH     VTSS_ENCODE_BITMASK(16,12)
#define  VTSS_X_XFI_XFI_CONTROL_TESTPAT_MODE_TP_BITLENGTH(x)  VTSS_EXTRACT_BITFIELD(x,16,12)

/** 
 * \brief
 * Encoded wave_gen word width
 *
 * \details 
 * 2'b00: 16-bit words
 * 2'b01: 20-bit words
 * 2'b10: 32-bit words
 * 2'b11: 40-bit words
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_TESTPAT_MODE . TP_WDWIDTH
 */
#define  VTSS_F_XFI_XFI_CONTROL_TESTPAT_MODE_TP_WDWIDTH(x)  VTSS_ENCODE_BITFIELD(x,12,2)
#define  VTSS_M_XFI_XFI_CONTROL_TESTPAT_MODE_TP_WDWIDTH     VTSS_ENCODE_BITMASK(12,2)
#define  VTSS_X_XFI_XFI_CONTROL_TESTPAT_MODE_TP_WDWIDTH(x)  VTSS_EXTRACT_BITFIELD(x,12,2)

/** 
 * \brief
 * Reset the TESTPAT_ERRORS counter
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_TESTPAT_MODE . TPERR_RST
 */
#define  VTSS_F_XFI_XFI_CONTROL_TESTPAT_MODE_TPERR_RST(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_XFI_XFI_CONTROL_TESTPAT_MODE_TPERR_RST     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_XFI_XFI_CONTROL_TESTPAT_MODE_TPERR_RST(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Test pattern checker scan mode
 * When set, if the number of errors (mismatched bits) detected in one
 * complete TP_BITLENGTH scan equals or exceeds the TP_THRESHOLD, the
 * pattern checker will auto-skip one bit position (retarding the check
 * phase by one bit clock) and compare the new pattern to the input. The
 * bit-error counter (TESTPAT_ERRORS) is also reset at the beginning of the
 * new scan cycle. This will continue until the number of errors detected
 * is less than the TP_THRESHOLD at which point the scan mode (this bit)
 * will switch to locked (deasserted). When deasserted, the pattern checker
 * scan mode is disabled.
 *
 * \details 
 * 0: lock pattern detection to its current position
 * 1: Scan mode, skip a bit position at the beginning of a pattern sequence
 * whenever the number of bit errors detected exceed TP_THRESHOLD. This bit
 * is auto-cleared when the number of errors in one cycle is less than or
 * equal to TP_THRESHOLD.
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_TESTPAT_MODE . TPCHK_SCAN
 */
#define  VTSS_F_XFI_XFI_CONTROL_TESTPAT_MODE_TPCHK_SCAN(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_XFI_XFI_CONTROL_TESTPAT_MODE_TPCHK_SCAN     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_XFI_XFI_CONTROL_TESTPAT_MODE_TPCHK_SCAN(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Enable test-pattern checker
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_TESTPAT_MODE . TPCHK_ENB
 */
#define  VTSS_F_XFI_XFI_CONTROL_TESTPAT_MODE_TPCHK_ENB(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_XFI_CONTROL_TESTPAT_MODE_TPCHK_ENB     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_XFI_CONTROL_TESTPAT_MODE_TPCHK_ENB(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Enable test-pattern generator
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_TESTPAT_MODE . TPGEN_ENB
 */
#define  VTSS_F_XFI_XFI_CONTROL_TESTPAT_MODE_TPGEN_ENB(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_XFI_CONTROL_TESTPAT_MODE_TPGEN_ENB     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_XFI_CONTROL_TESTPAT_MODE_TPGEN_ENB(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Pattern checker threshold
 *
 * \details
 * Register: \a XFI::XFI_CONTROL::TESTPAT_THRESHOLD
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_XFI_CONTROL_TESTPAT_THRESHOLD(target)  VTSS_IOREG(target,0xb)

/** 
 * \brief
 * Test pattern threshold
 * Specifies the minimum number of bit errors (miscompares) detected during
 * a scan cycle (see TPCHK_SCAN in TESTPAT_MODE) that will inhibit the
 * Scan_Lock state, causing a new scan cycle with the pattern bit-slipped
 * by one position.
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_TESTPAT_THRESHOLD . TP_THRESHOLD
 */
#define  VTSS_F_XFI_XFI_CONTROL_TESTPAT_THRESHOLD_TP_THRESHOLD(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_XFI_XFI_CONTROL_TESTPAT_THRESHOLD_TP_THRESHOLD     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_XFI_XFI_CONTROL_TESTPAT_THRESHOLD_TP_THRESHOLD(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Pattern checker errors
 *
 * \details
 * Register: \a XFI::XFI_CONTROL::TESTPAT_ERRORS
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_XFI_CONTROL_TESTPAT_ERRORS(target)  VTSS_IOREG(target,0xc)

/** 
 * \brief
 * Bit error counter
 * Total number of bit errors detected since the last TPERR_RST was issued
 * or TPCHK_SCAN was deasserted (lock achieved). The TPCHK_ENB must be
 * asserted for this counter to operate. This value is updated (with the
 * total number of bit errors received during the current test-pattern
 * cycle) at the completion of each complete test-pattern cycle. If this
 * register value reaches full scale (0xffffffff), further counting of
 * errors is disabled until a TPERR_RST is issued.
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_TESTPAT_ERRORS . TP_ERRORS
 */
#define  VTSS_F_XFI_XFI_CONTROL_TESTPAT_ERRORS_TP_ERRORS(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_XFI_XFI_CONTROL_TESTPAT_ERRORS_TP_ERRORS     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_XFI_XFI_CONTROL_TESTPAT_ERRORS_TP_ERRORS(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Pattern checker cycles
 *
 * \details
 * Register: \a XFI::XFI_CONTROL::TESTPAT_CYCS
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_XFI_CONTROL_TESTPAT_CYCS(target)  VTSS_IOREG(target,0xd)

/** 
 * \brief
 * Test pattern cycle counter
 * Total number of the specified TP_BITLENGTH cycles completed since the
 * last TPERR_RST was issued or TPCHK_SCAN was deasserted (lock achieved).
 * The User is expected to know the pattern bit-length and multiply this
 * value (TESTPAT_CYCS) by that value (TP_BITLENGTH) to determine the total
 * number of bits received.
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_TESTPAT_CYCS . TP_CYCLES
 */
#define  VTSS_F_XFI_XFI_CONTROL_TESTPAT_CYCS_TP_CYCLES(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_XFI_XFI_CONTROL_TESTPAT_CYCS_TP_CYCLES     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_XFI_XFI_CONTROL_TESTPAT_CYCS_TP_CYCLES(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Discrete AE macro controls
 *
 * \details
 * Inputs to the two (lane-A and lane-B) AE macros. Signals with a '_A'
 * postfix drive the lane-A macro; signals with a '_B' postfix drive the
 * lane-B macro. Signals associated with 16GFC are not supported and should
 * not be asserted.
 *
 * Register: \a XFI::XFI_CONTROL::AE_CONTROLS
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_XFI_CONTROL_AE_CONTROLS(target)  VTSS_IOREG(target,0xe)

/** 
 * \brief
 * 16GFC speed negotiation state
 * Signal used only if MODE_16GFC=1
 *
 * \details 
 * 0: FC_Port FSM is in RESTART_LINK state (default)
 * 1: FC_Port FSM is in SPEED_NEGOTIATION, TRANSMITTER_TRAINING, or
 * NORMAL_OPERATION state
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_CONTROLS . MR_TR_ENABLE_B
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_CONTROLS_MR_TR_ENABLE_B(x)  VTSS_ENCODE_BITFIELD(x,28,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_CONTROLS_MR_TR_ENABLE_B     VTSS_ENCODE_BITMASK(28,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_CONTROLS_MR_TR_ENABLE_B(x)  VTSS_EXTRACT_BITFIELD(x,28,1)

/** 
 * \brief
 * 16GFC speed negotiation
 * Signal is only used if MODE_16GFC=1
 *
 * \details 
 * 1: TX_TRAINING state is active
 * 0: All other states (default)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_CONTROLS . FC_TX_TRAINING_B
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_CONTROLS_FC_TX_TRAINING_B(x)  VTSS_ENCODE_BITFIELD(x,24,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_CONTROLS_FC_TX_TRAINING_B     VTSS_ENCODE_BITMASK(24,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_CONTROLS_FC_TX_TRAINING_B(x)  VTSS_EXTRACT_BITFIELD(x,24,1)

/** 
 * \brief
 * Selects link application
 * Signal should be constant throughout training process. AEC operation is
 * not defined if signal changes state while training is active.
 *
 * \details 
 * 0: 10GBASE-KR (default)
 * 1: 16GFC (Fibre Channel)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_CONTROLS . MODE_16GFC_B
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_CONTROLS_MODE_16GFC_B(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_CONTROLS_MODE_16GFC_B     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_CONTROLS_MODE_16GFC_B(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \brief
 * Resets the entire lane-B AE macro core when asserted
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_CONTROLS . AE_RESET_B
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_CONTROLS_AE_RESET_B(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_CONTROLS_AE_RESET_B     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_CONTROLS_AE_RESET_B(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * 16GFC speed negotiation state
 * Signal used only if MODE_16GFC=1
 *
 * \details 
 * 0: FC_Port FSM is in RESTART_LINK state (default)
 * 1: FC_Port FSM is in SPEED_NEGOTIATION, TRANSMITTER_TRAINING, or
 * NORMAL_OPERATION state
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_CONTROLS . MR_TR_ENABLE_A
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_CONTROLS_MR_TR_ENABLE_A(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_CONTROLS_MR_TR_ENABLE_A     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_CONTROLS_MR_TR_ENABLE_A(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * 16GFC speed negotiation
 * Signal is only used if MODE_16GFC=1
 *
 * \details 
 * 1: TX_TRAINING state is active
 * 0: All other states (default)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_CONTROLS . FC_TX_TRAINING_A
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_CONTROLS_FC_TX_TRAINING_A(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_CONTROLS_FC_TX_TRAINING_A     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_CONTROLS_FC_TX_TRAINING_A(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Selects link application
 * Signal should be constant throughout training process. AEC operation is
 * not defined if signal changes state while training is active.
 *
 * \details 
 * 0: 10GBASE-KR (default)
 * 1: 16GFC (Fibre Channel)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_CONTROLS . MODE_16GFC_A
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_CONTROLS_MODE_16GFC_A(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_CONTROLS_MODE_16GFC_A     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_CONTROLS_MODE_16GFC_A(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Resets the entire lane-A AE macro core when asserted
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_CONTROLS . AE_RESET_A
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_CONTROLS_AE_RESET_A(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_CONTROLS_AE_RESET_A     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_CONTROLS_AE_RESET_A(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Discrete AE macro controls
 *
 * \details
 * Outputs from the two (lane-A and lane-B) AE macros. Signals with a '_A'
 * postfix are driven by the lane-A macro; signals with a '_B' postfix are
 * driven by the lane-B macro.
 *
 * Register: \a XFI::XFI_CONTROL::AE_STATUS
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_XFI_CONTROL_AE_STATUS(target)  VTSS_IOREG(target,0xf)

/** 
 * \brief
 * Asserted following: 
 * (Training FSM = SEND_TRAINING or TRAIN_LOCAL) AND
 * (REG_MODE:AET_ENABLE=1) AND ((FC_TX_TRAINING=1) OR (MODE_16GFC=0) )
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STATUS . AET_RUN_B_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STATUS_AET_RUN_B_STATUS(x)  VTSS_ENCODE_BITFIELD(x,30,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STATUS_AET_RUN_B_STATUS     VTSS_ENCODE_BITMASK(30,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STATUS_AET_RUN_B_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,30,1)

/** 
 * \brief
 * FSM_FRM_LCK status bit from the FSM_CTL register
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STATUS . AET_FRM_LCK_B_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STATUS_AET_FRM_LCK_B_STATUS(x)  VTSS_ENCODE_BITFIELD(x,28,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STATUS_AET_FRM_LCK_B_STATUS     VTSS_ENCODE_BITMASK(28,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STATUS_AET_FRM_LCK_B_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,28,1)

/** 
 * \brief
 * Asserted when PMA_RX_READY is asserted and FSM_TR_COMP is asserted
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STATUS . PCS_RX_READY_B_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STATUS_PCS_RX_READY_B_STATUS(x)  VTSS_ENCODE_BITFIELD(x,26,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STATUS_PCS_RX_READY_B_STATUS     VTSS_ENCODE_BITMASK(26,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STATUS_PCS_RX_READY_B_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,26,1)

/** 
 * \brief
 * Asserted when PMA_TX_READY is asserted and FSM_TR_COMP is asserted
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STATUS . PCS_TX_READY_B_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STATUS_PCS_TX_READY_B_STATUS(x)  VTSS_ENCODE_BITFIELD(x,24,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STATUS_PCS_TX_READY_B_STATUS     VTSS_ENCODE_BITMASK(24,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STATUS_PCS_TX_READY_B_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,24,1)

/** 
 * \brief
 * Asserted if TFSM_STATE is in SEND_DATA state and register bits are as
 * follows:
 * TXSTAT_FEC=1, RXSTAT_FEC=1, and either TXREQ_FEC or RXREQ_FEC = 1.
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STATUS . FEC_NEGOTIATED_B_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STATUS_FEC_NEGOTIATED_B_STATUS(x)  VTSS_ENCODE_BITFIELD(x,22,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STATUS_FEC_NEGOTIATED_B_STATUS     VTSS_ENCODE_BITMASK(22,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STATUS_FEC_NEGOTIATED_B_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,22,1)

/** 
 * \brief
 * Asserted if TFSM_STATE is in SEND_DATA state and the FSM_TR_FAIL output
 * is not asserted
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STATUS . TRAIN_OK_B_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STATUS_TRAIN_OK_B_STATUS(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STATUS_TRAIN_OK_B_STATUS     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STATUS_TRAIN_OK_B_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \brief
 * Asserted if TFSM_STATE is in TRAINING_FAILURE state
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STATUS . TRAIN_FAIL_B_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STATUS_TRAIN_FAIL_B_STATUS(x)  VTSS_ENCODE_BITFIELD(x,18,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STATUS_TRAIN_FAIL_B_STATUS     VTSS_ENCODE_BITMASK(18,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STATUS_TRAIN_FAIL_B_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,18,1)

/** 
 * \brief
 * Asserted when TFSM_STATE is in the TRAIN_LOCAL state. Microcode must
 * execute the training algorithm for the local node in this state.
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STATUS . TRAIN_LOCAL_B_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STATUS_TRAIN_LOCAL_B_STATUS(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STATUS_TRAIN_LOCAL_B_STATUS     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STATUS_TRAIN_LOCAL_B_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Asserted following: 
 * (Training FSM = SEND_TRAINING or TRAIN_LOCAL) AND
 * (REG_MODE:AET_ENABLE=1) AND ((FC_TX_TRAINING=1) OR (MODE_16GFC=0) )
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STATUS . AET_RUN_A_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STATUS_AET_RUN_A_STATUS(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STATUS_AET_RUN_A_STATUS     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STATUS_AET_RUN_A_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * FSM_FRM_LCK status bit from the FSM_CTL register
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STATUS . AET_FRM_LCK_A_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STATUS_AET_FRM_LCK_A_STATUS(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STATUS_AET_FRM_LCK_A_STATUS     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STATUS_AET_FRM_LCK_A_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Asserted when PMA_RX_READY is asserted and FSM_TR_COMP is asserted
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STATUS . PCS_RX_READY_A_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STATUS_PCS_RX_READY_A_STATUS(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STATUS_PCS_RX_READY_A_STATUS     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STATUS_PCS_RX_READY_A_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Asserted when PMA_TX_READY is asserted and FSM_TR_COMP is asserted
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STATUS . PCS_TX_READY_A_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STATUS_PCS_TX_READY_A_STATUS(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STATUS_PCS_TX_READY_A_STATUS     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STATUS_PCS_TX_READY_A_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Asserted if the training FSM is in SEND_DATA state, and register bits
 * are as follows:
 * TXSTAT_FEC=1, RXSTAT_FEC=1, and either TXREQ_FEC or RXREQ_FEC = 1.
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STATUS . FEC_NEGOTIATED_A_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STATUS_FEC_NEGOTIATED_A_STATUS(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STATUS_FEC_NEGOTIATED_A_STATUS     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STATUS_FEC_NEGOTIATED_A_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Asserted if the training FSM is in the SEND_DATA state and the
 * TRAIN_FAIL output is not asserted
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STATUS . TRAIN_OK_A_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STATUS_TRAIN_OK_A_STATUS(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STATUS_TRAIN_OK_A_STATUS     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STATUS_TRAIN_OK_A_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Asserted if the training FSM is in the TRAIN_FAILURE state
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STATUS . TRAIN_FAIL_A_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STATUS_TRAIN_FAIL_A_STATUS(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STATUS_TRAIN_FAIL_A_STATUS     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STATUS_TRAIN_FAIL_A_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Asserted when the training FSM is in the TRAIN_LOCAL state. Microcode
 * must execute the training algorithm for the local node in this state.
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STATUS . TRAIN_LOCAL_A_STATUS
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STATUS_TRAIN_LOCAL_A_STATUS(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STATUS_TRAIN_LOCAL_A_STATUS     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STATUS_TRAIN_LOCAL_A_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Discrete AE macro control output status sticky bits
 *
 * \details
 * Sticky outputs from the two (lane-A and lane-B) AE macros. Signals with
 * a '_A' postfix are driven by the lane-A macro; signals with a '_B'
 * postfix are driven by the lane-B macro.
 *
 * Register: \a XFI::XFI_CONTROL::AE_STICKY
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_XFI_CONTROL_AE_STICKY(target)  VTSS_IOREG(target,0x10)

/** 
 * \brief
 * Lane-B AE macro assert state
 *
 * \details 
 * 0: Lane-B AE macro was not detected asserted (since this bit was last
 * cleared)
 * 1: Lane-B AE macro was detected asserted (since this bit was last
 * cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . AET_RUN_B_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_AET_RUN_B_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,31,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_AET_RUN_B_HI_STICKY     VTSS_ENCODE_BITMASK(31,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_AET_RUN_B_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,31,1)

/** 
 * \brief
 * Lane-B AE macro deassert state
 *
 * \details 
 * 0: Lane-B AE macro was not detected deasserted (since this bit was last
 * cleared)
 * 1: Lane-B AE macro was detected deasserted (since this bit was last
 * cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . AET_RUN_B_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_AET_RUN_B_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,30,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_AET_RUN_B_LO_STICKY     VTSS_ENCODE_BITMASK(30,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_AET_RUN_B_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,30,1)

/** 
 * \brief
 * Lane-B AE macro frame lock assert state from the FSM_FRM_LCK status bit
 * in the FSM_CTL_B register
 *
 * \details 
 * 0: Lane-B AE macro frame lock was not detected asserted (since this bit
 * was last cleared)
 * 1: Lane-B AE macro frame lock was detected asserted (since this bit was
 * last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . AET_FRM_LCK_B_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_AET_FRM_LCK_B_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,29,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_AET_FRM_LCK_B_HI_STICKY     VTSS_ENCODE_BITMASK(29,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_AET_FRM_LCK_B_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,29,1)

/** 
 * \brief
 * Lane-B AE macro frame lock deassert state from the FSM_FRM_LCK status
 * bit in the FSM_CTL_B register
 *
 * \details 
 * 0: Lane-B AE macro frame lock was not detected deasserted (since this
 * bit was last cleared)
 * 1: Lane-B AE macro frame lock was detected deasserted (since this bit
 * was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . AET_FRM_LCK_B_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_AET_FRM_LCK_B_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,28,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_AET_FRM_LCK_B_LO_STICKY     VTSS_ENCODE_BITMASK(28,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_AET_FRM_LCK_B_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,28,1)

/** 
 * \brief
 * Lane-B PCS receiver ready assert state
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . PCS_RX_READY_B_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_PCS_RX_READY_B_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,27,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_PCS_RX_READY_B_HI_STICKY     VTSS_ENCODE_BITMASK(27,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_PCS_RX_READY_B_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,27,1)

/** 
 * \brief
 * Lane-B PCS receiver ready deassert state
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . PCS_RX_READY_B_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_PCS_RX_READY_B_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,26,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_PCS_RX_READY_B_LO_STICKY     VTSS_ENCODE_BITMASK(26,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_PCS_RX_READY_B_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,26,1)

/** 
 * \brief
 * Lane-B PCS transmitter ready assert state
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . PCS_TX_READY_B_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_PCS_TX_READY_B_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,25,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_PCS_TX_READY_B_HI_STICKY     VTSS_ENCODE_BITMASK(25,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_PCS_TX_READY_B_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,25,1)

/** 
 * \brief
 * Lane-B PCS transmitter ready deassert state
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . PCS_TX_READY_B_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_PCS_TX_READY_B_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,24,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_PCS_TX_READY_B_LO_STICKY     VTSS_ENCODE_BITMASK(24,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_PCS_TX_READY_B_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,24,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . FEC_NEGOTIATED_B_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_FEC_NEGOTIATED_B_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,23,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_FEC_NEGOTIATED_B_HI_STICKY     VTSS_ENCODE_BITMASK(23,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_FEC_NEGOTIATED_B_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,23,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . FEC_NEGOTIATED_B_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_FEC_NEGOTIATED_B_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,22,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_FEC_NEGOTIATED_B_LO_STICKY     VTSS_ENCODE_BITMASK(22,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_FEC_NEGOTIATED_B_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,22,1)

/** 
 * \brief
 * Lane-B training FSM OK (TFSM_STATE:110 and FSM_TR_FAIL:0) assert state
 *
 * \details 
 * 0: Lane-B training FSM state OK was not detected asserted (since this
 * bit was last cleared)
 * 1: Lane-B training FSM state OK was detected asserted (since this bit
 * was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . TRAIN_OK_B_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_TRAIN_OK_B_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,21,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_OK_B_HI_STICKY     VTSS_ENCODE_BITMASK(21,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_TRAIN_OK_B_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,21,1)

/** 
 * \brief
 * Lane-B training FSM OK (TFSM_STATE:110 and FSM_TR_FAIL:0) deassert state
 *
 * \details 
 * 0: Lane-B training FSM state OK was not detected deasserted (since this
 * bit was last cleared)
 * 1: Lane-B training FSM state OK was detected deasserted (since this bit
 * was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . TRAIN_OK_B_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_TRAIN_OK_B_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_OK_B_LO_STICKY     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_TRAIN_OK_B_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \brief
 * Lane-B training FSM failure assert state
 *
 * \details 
 * 0: Lane-B training FSM state failure was not detected asserted (since
 * this bit was last cleared)
 * 1: Lane-B training FSM state failure was detected asserted (since this
 * bit was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . TRAIN_FAIL_B_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_TRAIN_FAIL_B_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,19,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_FAIL_B_HI_STICKY     VTSS_ENCODE_BITMASK(19,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_TRAIN_FAIL_B_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,19,1)

/** 
 * \brief
 * Lane-B training FSM failure deassert state
 *
 * \details 
 * 0: Lane-B training FSM state failure was not detected deasserted (since
 * this bit was last cleared)
 * 1: Lane-B training FSM state failure was detected deasserted (since this
 * bit was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . TRAIN_FAIL_B_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_TRAIN_FAIL_B_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,18,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_FAIL_B_LO_STICKY     VTSS_ENCODE_BITMASK(18,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_TRAIN_FAIL_B_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,18,1)

/** 
 * \brief
 * Lane-B FSM train local (TFSM_STATE:011) assert state
 *
 * \details 
 * 0: Lane-B FSM train local state was not detected asserted (since this
 * bit was last cleared)
 * 1: Lane-B FSM train local state was detected asserted (since this bit
 * was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . TRAIN_LOCAL_B_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_TRAIN_LOCAL_B_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,17,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_LOCAL_B_HI_STICKY     VTSS_ENCODE_BITMASK(17,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_TRAIN_LOCAL_B_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,17,1)

/** 
 * \brief
 * Lane-B FSM train local (TFSM_STATE:011) deassert state
 *
 * \details 
 * 0: Lane-B FSM train local state was not detected deasserted (since this
 * bit was last cleared)
 * 1: Lane-B FSM train local state was detected deasserted (since this bit
 * was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . TRAIN_LOCAL_B_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_TRAIN_LOCAL_B_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_LOCAL_B_LO_STICKY     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_TRAIN_LOCAL_B_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Lane-A AE macro assert state

 *
 * \details 
 * 0: Lane-A AE macro was not detected asserted (since this bit was last
 * cleared)
 * 1: Lane-A AE macro was detected asserted (since this bit was last
 * cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . AET_RUN_A_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_AET_RUN_A_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_AET_RUN_A_HI_STICKY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_AET_RUN_A_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Lane-A AE macro deassert state
 *
 * \details 
 * 0: Lane-A AE macro was not detected deasserted (since this bit was last
 * cleared)
 * 1: Lane-A AE macro was detected deasserted (since this bit was last
 * cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . AET_RUN_A_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_AET_RUN_A_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_AET_RUN_A_LO_STICKY     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_AET_RUN_A_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Lane-A AE macro frame lock assert state from the FSM_FRM_LCK status bit
 * from the FSM_CTL_A register
 *
 * \details 
 * 0: Lane-A AE macro frame lock was not detected asserted (since this bit
 * was last cleared)
 * 1: Lane-A AE macro frame lock was detected asserted (since this bit was
 * last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . AET_FRM_LCK_A_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_AET_FRM_LCK_A_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_AET_FRM_LCK_A_HI_STICKY     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_AET_FRM_LCK_A_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Lane-A AE macro frame lock deassert state from the FSM_FRM_LCK status
 * bit from the FSM_CTL_A register
 *
 * \details 
 * 0: Lane-A AE macro frame lock was not detected deasserted (since this
 * bit was last cleared)
 * 1: Lane-A AE macro frame lock was detected deasserted (since this bit
 * was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . AET_FRM_LCK_A_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_AET_FRM_LCK_A_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_AET_FRM_LCK_A_LO_STICKY     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_AET_FRM_LCK_A_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Lane-A PCS receiver ready assert state
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . PCS_RX_READY_A_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_PCS_RX_READY_A_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_PCS_RX_READY_A_HI_STICKY     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_PCS_RX_READY_A_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Lane-A PCS receiver ready deassert state
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . PCS_RX_READY_A_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_PCS_RX_READY_A_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_PCS_RX_READY_A_LO_STICKY     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_PCS_RX_READY_A_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Lane-A PCS transmitter ready assert state
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . PCS_TX_READY_A_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_PCS_TX_READY_A_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_PCS_TX_READY_A_HI_STICKY     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_PCS_TX_READY_A_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Lane-A PCS transmitter ready deassert state
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . PCS_TX_READY_A_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_PCS_TX_READY_A_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_PCS_TX_READY_A_LO_STICKY     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_PCS_TX_READY_A_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . FEC_NEGOTIATED_A_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_FEC_NEGOTIATED_A_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_FEC_NEGOTIATED_A_HI_STICKY     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_FEC_NEGOTIATED_A_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . FEC_NEGOTIATED_A_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_FEC_NEGOTIATED_A_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_FEC_NEGOTIATED_A_LO_STICKY     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_FEC_NEGOTIATED_A_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Lane-A training FSM OK (TFSM_STATE:110 and FSM_TR_FAIL:0) assert state
 *
 * \details 
 * 0: Lane-A training FSM state OK was not detected asserted (since this
 * bit was last cleared)
 * 1: Lane-A training FSM state OK was detected asserted (since this bit
 * was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . TRAIN_OK_A_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_TRAIN_OK_A_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_OK_A_HI_STICKY     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_TRAIN_OK_A_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Lane-A training FSM OK (TFSM_STATE:110 and FSM_TR_FAIL:0) deassert state
 *
 * \details 
 * 0: Lane-A training FSM state OK was not detected deasserted (since this
 * bit was last cleared)
 * 1: Lane-A training FSM state OK was detected deasserted (since this bit
 * was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . TRAIN_OK_A_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_TRAIN_OK_A_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_OK_A_LO_STICKY     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_TRAIN_OK_A_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Lane-A training FSM failure assert state
 *
 * \details 
 * 0: Lane-A training FSM state failure was not detected asserted (since
 * this bit was last cleared)
 * 1: Lane-A training FSM state failure was detected asserted (since this
 * bit was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . TRAIN_FAIL_A_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_TRAIN_FAIL_A_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_FAIL_A_HI_STICKY     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_TRAIN_FAIL_A_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Lane-A training FSM failure deassert state
 *
 * \details 
 * 0: Lane-A training FSM state failure was not detected deasserted (since
 * this bit was last cleared)
 * 1: Lane-A training FSM state failure was detected deasserted (since this
 * bit was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . TRAIN_FAIL_A_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_TRAIN_FAIL_A_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_FAIL_A_LO_STICKY     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_TRAIN_FAIL_A_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Lane-A FSM train local (TFSM_STATE:011) assert state
 *
 * \details 
 * 0: Lane-A FSM train local state was not detected asserted (since this
 * bit was last cleared)
 * 1: Lane-A FSM train local state was detected asserted (since this bit
 * was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . TRAIN_LOCAL_A_HI_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_TRAIN_LOCAL_A_HI_STICKY(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_LOCAL_A_HI_STICKY     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_TRAIN_LOCAL_A_HI_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Lane-A FSM train local (TFSM_STATE:011) deassert state
 *
 * \details 
 * 0: Lane-A FSM train local state was not detected deasserted (since this
 * bit was last cleared)
 * 1: Lane-A FSM train local state was detected deasserted (since this bit
 * was last cleared)
 *
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_STICKY . TRAIN_LOCAL_A_LO_STICKY
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_STICKY_TRAIN_LOCAL_A_LO_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_STICKY_TRAIN_LOCAL_A_LO_STICKY     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_STICKY_TRAIN_LOCAL_A_LO_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief AE macro sticky interrupt masks (enables) 
 *
 * \details
 * Register: \a XFI::XFI_CONTROL::AE_MASK
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_XFI_CONTROL_AE_MASK(target)  VTSS_IOREG(target,0x11)

/** 
 * \brief
 * Mask (enable) an XFI_INT if Lane-B AE macro is asserted
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . AET_RUN_B_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_AET_RUN_B_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,31,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_AET_RUN_B_HI_MASK     VTSS_ENCODE_BITMASK(31,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_AET_RUN_B_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,31,1)

/** 
 * \brief
 * Mask (enable) an XFI_INT if Lane-B AE macro is deasserted
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . AET_RUN_B_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_AET_RUN_B_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,30,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_AET_RUN_B_LO_MASK     VTSS_ENCODE_BITMASK(30,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_AET_RUN_B_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,30,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . AET_FRM_LCK_B_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_AET_FRM_LCK_B_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,29,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_AET_FRM_LCK_B_HI_MASK     VTSS_ENCODE_BITMASK(29,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_AET_FRM_LCK_B_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,29,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . AET_FRM_LCK_B_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_AET_FRM_LCK_B_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,28,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_AET_FRM_LCK_B_LO_MASK     VTSS_ENCODE_BITMASK(28,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_AET_FRM_LCK_B_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,28,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . PCS_RX_READY_B_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_PCS_RX_READY_B_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,27,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_PCS_RX_READY_B_HI_MASK     VTSS_ENCODE_BITMASK(27,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_PCS_RX_READY_B_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,27,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . PCS_RX_READY_B_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_PCS_RX_READY_B_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,26,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_PCS_RX_READY_B_LO_MASK     VTSS_ENCODE_BITMASK(26,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_PCS_RX_READY_B_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,26,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . PCS_TX_READY_B_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_PCS_TX_READY_B_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,25,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_PCS_TX_READY_B_HI_MASK     VTSS_ENCODE_BITMASK(25,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_PCS_TX_READY_B_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,25,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . PCS_TX_READY_B_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_PCS_TX_READY_B_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,24,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_PCS_TX_READY_B_LO_MASK     VTSS_ENCODE_BITMASK(24,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_PCS_TX_READY_B_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,24,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . FEC_NEGOTIATED_B_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_FEC_NEGOTIATED_B_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,23,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_FEC_NEGOTIATED_B_HI_MASK     VTSS_ENCODE_BITMASK(23,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_FEC_NEGOTIATED_B_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,23,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . FEC_NEGOTIATED_B_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_FEC_NEGOTIATED_B_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,22,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_FEC_NEGOTIATED_B_LO_MASK     VTSS_ENCODE_BITMASK(22,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_FEC_NEGOTIATED_B_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,22,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . TRAIN_OK_B_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_TRAIN_OK_B_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,21,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_TRAIN_OK_B_HI_MASK     VTSS_ENCODE_BITMASK(21,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_TRAIN_OK_B_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,21,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . TRAIN_OK_B_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_TRAIN_OK_B_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,20,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_TRAIN_OK_B_LO_MASK     VTSS_ENCODE_BITMASK(20,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_TRAIN_OK_B_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,20,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . TRAIN_FAIL_B_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_TRAIN_FAIL_B_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,19,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_TRAIN_FAIL_B_HI_MASK     VTSS_ENCODE_BITMASK(19,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_TRAIN_FAIL_B_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,19,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . TRAIN_FAIL_B_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_TRAIN_FAIL_B_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,18,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_TRAIN_FAIL_B_LO_MASK     VTSS_ENCODE_BITMASK(18,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_TRAIN_FAIL_B_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,18,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . TRAIN_LOCAL_B_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_TRAIN_LOCAL_B_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,17,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_TRAIN_LOCAL_B_HI_MASK     VTSS_ENCODE_BITMASK(17,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_TRAIN_LOCAL_B_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,17,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . TRAIN_LOCAL_B_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_TRAIN_LOCAL_B_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_TRAIN_LOCAL_B_LO_MASK     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_TRAIN_LOCAL_B_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . AET_RUN_A_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_AET_RUN_A_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_AET_RUN_A_HI_MASK     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_AET_RUN_A_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . AET_RUN_A_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_AET_RUN_A_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_AET_RUN_A_LO_MASK     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_AET_RUN_A_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . AET_FRM_LCK_A_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_AET_FRM_LCK_A_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_AET_FRM_LCK_A_HI_MASK     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_AET_FRM_LCK_A_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . AET_FRM_LCK_A_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_AET_FRM_LCK_A_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_AET_FRM_LCK_A_LO_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_AET_FRM_LCK_A_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . PCS_RX_READY_A_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_PCS_RX_READY_A_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_PCS_RX_READY_A_HI_MASK     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_PCS_RX_READY_A_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . PCS_RX_READY_A_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_PCS_RX_READY_A_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_PCS_RX_READY_A_LO_MASK     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_PCS_RX_READY_A_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . PCS_TX_READY_A_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_PCS_TX_READY_A_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_PCS_TX_READY_A_HI_MASK     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_PCS_TX_READY_A_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . PCS_TX_READY_A_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_PCS_TX_READY_A_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_PCS_TX_READY_A_LO_MASK     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_PCS_TX_READY_A_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . FEC_NEGOTIATED_A_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_FEC_NEGOTIATED_A_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_FEC_NEGOTIATED_A_HI_MASK     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_FEC_NEGOTIATED_A_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . FEC_NEGOTIATED_A_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_FEC_NEGOTIATED_A_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_FEC_NEGOTIATED_A_LO_MASK     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_FEC_NEGOTIATED_A_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . TRAIN_OK_A_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_TRAIN_OK_A_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_TRAIN_OK_A_HI_MASK     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_TRAIN_OK_A_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . TRAIN_OK_A_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_TRAIN_OK_A_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_TRAIN_OK_A_LO_MASK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_TRAIN_OK_A_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . TRAIN_FAIL_A_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_TRAIN_FAIL_A_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_TRAIN_FAIL_A_HI_MASK     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_TRAIN_FAIL_A_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . TRAIN_FAIL_A_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_TRAIN_FAIL_A_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_TRAIN_FAIL_A_LO_MASK     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_TRAIN_FAIL_A_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . TRAIN_LOCAL_A_HI_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_TRAIN_LOCAL_A_HI_MASK(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_TRAIN_LOCAL_A_HI_MASK     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_TRAIN_LOCAL_A_HI_MASK(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_AE_MASK . TRAIN_LOCAL_A_LO_MASK
 */
#define  VTSS_F_XFI_XFI_CONTROL_AE_MASK_TRAIN_LOCAL_A_LO_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_XFI_CONTROL_AE_MASK_TRAIN_LOCAL_A_LO_MASK     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_XFI_CONTROL_AE_MASK_TRAIN_LOCAL_A_LO_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief XFI spare register
 *
 * \details
 * Register: \a XFI::XFI_CONTROL::XFI_SPARE
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_XFI_CONTROL_XFI_SPARE(target)  VTSS_IOREG(target,0x12)

/** 
 * \brief
 * Spare register
 *
 * \details 
 * Field: ::VTSS_XFI_XFI_CONTROL_XFI_SPARE . XFI_SPARE_REG
 */
#define  VTSS_F_XFI_XFI_CONTROL_XFI_SPARE_XFI_SPARE_REG(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_XFI_XFI_CONTROL_XFI_SPARE_XFI_SPARE_REG     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_XFI_XFI_CONTROL_XFI_SPARE_XFI_SPARE_REG(x)  VTSS_EXTRACT_BITFIELD(x,0,32)

/**
 * Register Group: \a XFI::TEST_PATTERN_BUFFER
 *
 * Test-pattern storage memory
 */


/** 
 * \brief Pattern generation/checker buffer memory
 *
 * \details
 * 64 32-bit words used as the test pattern source-buffer/check-buffer
 *
 * Register: \a XFI::TEST_PATTERN_BUFFER::PATTERN_BUFFER
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: PATTERN_BUFFER (??), 0-63
 */
#define VTSS_XFI_TEST_PATTERN_BUFFER_PATTERN_BUFFER(target,ri)  VTSS_IOREG(target,0x80 + (ri))

/** 
 * \brief
 * Only the number of bits specified by the TP_BITLENGTH are used
 *
 * \details 
 * Field: ::VTSS_XFI_TEST_PATTERN_BUFFER_PATTERN_BUFFER . PATTERN_BUFFER
 */
#define  VTSS_F_XFI_TEST_PATTERN_BUFFER_PATTERN_BUFFER_PATTERN_BUFFER(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_XFI_TEST_PATTERN_BUFFER_PATTERN_BUFFER_PATTERN_BUFFER     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_XFI_TEST_PATTERN_BUFFER_PATTERN_BUFFER_PATTERN_BUFFER(x)  VTSS_EXTRACT_BITFIELD(x,0,32)

/**
 * Register Group: \a XFI::AE_REGS_A
 *
 * Lane-A AE register interface for microcode control of the training process
 */


/** 
 * \brief Receive coefficient update request register
 *
 * \details
 * When adaptive equalization is active, this register controls the bits
 * transmitted in the Coefficient Update field of the Training frame.
 * Microcode is responsible for generating coefficient update commands and
 * writing this register while the Training FSM is in the TRAIN_LOCAL
 * state, as indicated by the FSM_TR_LCL bit in the FSM_CTL register.
 *
 * Register: \a XFI::AE_REGS_A::RX_COEF_REQ_A
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_AE_REGS_A_RX_COEF_REQ_A(target)  VTSS_IOREG(target,0x100)

/** 
 * \brief
 * Reserved. Always write to 0.
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_A_RX_COEF_REQ_A . Reserved_AE014
 */
#define  VTSS_F_XFI_AE_REGS_A_RX_COEF_REQ_A_Reserved_AE014(x)  VTSS_ENCODE_BITFIELD(x,14,2)
#define  VTSS_M_XFI_AE_REGS_A_RX_COEF_REQ_A_Reserved_AE014     VTSS_ENCODE_BITMASK(14,2)
#define  VTSS_X_XFI_AE_REGS_A_RX_COEF_REQ_A_Reserved_AE014(x)  VTSS_EXTRACT_BITFIELD(x,14,2)

/** 
 * \brief
 * Coefficient preset
 * Request link partner update FFE coefficients to preset values
 *
 * \details 
 * 1: Preset coefficients
 * 0: Normal operation
 *
 * Field: ::VTSS_XFI_AE_REGS_A_RX_COEF_REQ_A . RXREQ_CPRE
 */
#define  VTSS_F_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_CPRE(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_CPRE     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_CPRE(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Coefficient initialize
 * Request link partner update FFE coefficients to initial values
 *
 * \details 
 * 1: Initialize coefficients
 * 0: Normal operation
 *
 * Field: ::VTSS_XFI_AE_REGS_A_RX_COEF_REQ_A . RXREQ_CINIT
 */
#define  VTSS_F_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_CINIT(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_CINIT     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_CINIT(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Reserved. Always write to 0.
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_A_RX_COEF_REQ_A . Reserved_AE06
 */
#define  VTSS_F_XFI_AE_REGS_A_RX_COEF_REQ_A_Reserved_AE06(x)  VTSS_ENCODE_BITFIELD(x,6,6)
#define  VTSS_M_XFI_AE_REGS_A_RX_COEF_REQ_A_Reserved_AE06     VTSS_ENCODE_BITMASK(6,6)
#define  VTSS_X_XFI_AE_REGS_A_RX_COEF_REQ_A_Reserved_AE06(x)  VTSS_EXTRACT_BITFIELD(x,6,6)

/** 
 * \brief
 * Coefficient 0 (Z +1) update
 * Request link partner update FFE coefficients per below coding
 *
 * \details 
 * 00: Hold
 * 01: Increment
 * 10: Decrement
 * 11: Reserved
 *
 * Field: ::VTSS_XFI_AE_REGS_A_RX_COEF_REQ_A . RXREQ_C0
 */
#define  VTSS_F_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_C0(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_C0     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_C0(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * Coefficient 1 (Z 0) update
 * Request link partner update FFE coefficients per below coding
 *
 * \details 
 * 00: Hold
 * 01: Increment
 * 10: Decrement
 * 11: Reserved
 *
 * Field: ::VTSS_XFI_AE_REGS_A_RX_COEF_REQ_A . RXREQ_C1
 */
#define  VTSS_F_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_C1(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_C1     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_C1(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Coefficient 2 (Z -1) update
 * Request link partner update FFE coefficients per below coding
 *
 * \details 
 * 00: Hold
 * 01: Increment
 * 10: Decrement
 * 11: Reserved
 *
 * Field: ::VTSS_XFI_AE_REGS_A_RX_COEF_REQ_A . RXREQ_C2
 */
#define  VTSS_F_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_C2(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_C2     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_XFI_AE_REGS_A_RX_COEF_REQ_A_RXREQ_C2(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Receive coefficient status register
 *
 * \details
 * When adaptive equalization is active, this register contains the bits
 * received in the Status Report field of the Training frame sent by the
 * link partner. This status is the link partner's response to coefficient
 * update requests in the RX_COEF_REQ register. Microcode is responsible
 * for monitoring and/or clearing this status while the Training FSM is in
 * the TRAIN_LOCAL state, as indicated by the FSM_TR_LCL bit in the FSM_CTL
 * register. The sticky mode bit in the Register Mode Control register (for
 * diagnostics) affects operation of this register. If STICKY_MODE = 1 then
 * bits of this register must be written to reset them.
 *
 * Register: \a XFI::AE_REGS_A::RX_COEF_STAT_A
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_AE_REGS_A_RX_COEF_STAT_A(target)  VTSS_IOREG(target,0x101)

/** 
 * \brief
 * Receiver ready
 * This bit indicates training status of the link partner
 *
 * \details 
 * 1: Link partner has determined that training is complete and is prepared
 * to receive data
 * 0: Link partner is requesting that training continue
 *
 * Field: ::VTSS_XFI_AE_REGS_A_RX_COEF_STAT_A . RXSTAT_RDY
 */
#define  VTSS_F_XFI_AE_REGS_A_RX_COEF_STAT_A_RXSTAT_RDY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_XFI_AE_REGS_A_RX_COEF_STAT_A_RXSTAT_RDY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_XFI_AE_REGS_A_RX_COEF_STAT_A_RXSTAT_RDY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Reserved. Ignore contents.
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_A_RX_COEF_STAT_A . Reserved_AE16
 */
#define  VTSS_F_XFI_AE_REGS_A_RX_COEF_STAT_A_Reserved_AE16(x)  VTSS_ENCODE_BITFIELD(x,6,9)
#define  VTSS_M_XFI_AE_REGS_A_RX_COEF_STAT_A_Reserved_AE16     VTSS_ENCODE_BITMASK(6,9)
#define  VTSS_X_XFI_AE_REGS_A_RX_COEF_STAT_A_Reserved_AE16(x)  VTSS_EXTRACT_BITFIELD(x,6,9)

/** 
 * \brief
 * Coefficient 0 (Z +1) status 
 * Link partner response to RXREQ_C0 field request
 *
 * \details 
 * 11: Coefficient is set to maximum value
 * 10: Coefficient is set to minimum value
 * 01: Coefficient has been updated
 * 00: Coefficient is not updated
 *
 * Field: ::VTSS_XFI_AE_REGS_A_RX_COEF_STAT_A . RXSTAT_C0
 */
#define  VTSS_F_XFI_AE_REGS_A_RX_COEF_STAT_A_RXSTAT_C0(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_XFI_AE_REGS_A_RX_COEF_STAT_A_RXSTAT_C0     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_XFI_AE_REGS_A_RX_COEF_STAT_A_RXSTAT_C0(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * Coefficient 1 (Z 0) status
 * Link partner response to RXREQ_C1 field request
 *
 * \details 
 * 11: Coefficient is set to maximum value
 * 10: Coefficient is set to minimum value
 * 01: Coefficient has been updated
 * 00: Coefficient is not updated
 *
 * Field: ::VTSS_XFI_AE_REGS_A_RX_COEF_STAT_A . RXSTAT_C1
 */
#define  VTSS_F_XFI_AE_REGS_A_RX_COEF_STAT_A_RXSTAT_C1(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_XFI_AE_REGS_A_RX_COEF_STAT_A_RXSTAT_C1     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_XFI_AE_REGS_A_RX_COEF_STAT_A_RXSTAT_C1(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Coefficient 2 (Z -1) status
 * Link partner response to RXREQ_C2 field request
 *
 * \details 
 * 11: Coefficient is set to maximum value
 * 10: Coefficient is set to minimum value
 * 01: Coefficient has been updated
 * 00: Coefficient is not updated
 *
 * Field: ::VTSS_XFI_AE_REGS_A_RX_COEF_STAT_A . RXSTAT_C2
 */
#define  VTSS_F_XFI_AE_REGS_A_RX_COEF_STAT_A_RXSTAT_C2(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_XFI_AE_REGS_A_RX_COEF_STAT_A_RXSTAT_C2     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_XFI_AE_REGS_A_RX_COEF_STAT_A_RXSTAT_C2(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Transmit coefficient update request register
 *
 * \details
 * When adaptive equalization is active, this register contains the bits
 * received in the Coefficient Update field of the Training frame sent by
 * the link partner. This node processes requests and responds by sending
 * status via the TX_COEF_STAT register. Normally, the Coefficient Update
 * FSM processes requests from the link partner. Alternatively, microcode
 * may be used to process these requests. If FSM_CU_DIS = 1 in the FSM_CTL
 * register, then the AE core will not automatically process these
 * requests. The sticky mode bit in the Register Mode Control register (for
 * diagnostics) affects operation of this register. If STICKY_MODE = 1 then
 * bits of this register must be written to reset them.
 *
 * Register: \a XFI::AE_REGS_A::TX_COEF_REQ_A
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_AE_REGS_A_TX_COEF_REQ_A(target)  VTSS_IOREG(target,0x102)

/** 
 * \brief
 * Reserved. Ignore contents
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_A_TX_COEF_REQ_A . Reserved_AE214
 */
#define  VTSS_F_XFI_AE_REGS_A_TX_COEF_REQ_A_Reserved_AE214(x)  VTSS_ENCODE_BITFIELD(x,14,2)
#define  VTSS_M_XFI_AE_REGS_A_TX_COEF_REQ_A_Reserved_AE214     VTSS_ENCODE_BITMASK(14,2)
#define  VTSS_X_XFI_AE_REGS_A_TX_COEF_REQ_A_Reserved_AE214(x)  VTSS_EXTRACT_BITFIELD(x,14,2)

/** 
 * \brief
 * Coefficient preset
 * Link partner is requesting coefficients be updated to preset values
 *
 * \details 
 * 1: Preset coefficients
 * 0: Normal operation
 *
 * Field: ::VTSS_XFI_AE_REGS_A_TX_COEF_REQ_A . TXREQ_CPRE
 */
#define  VTSS_F_XFI_AE_REGS_A_TX_COEF_REQ_A_TXREQ_CPRE(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_XFI_AE_REGS_A_TX_COEF_REQ_A_TXREQ_CPRE     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_XFI_AE_REGS_A_TX_COEF_REQ_A_TXREQ_CPRE(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Coefficient initialize
 * Link partner is requesting coefficients be updated to initial values
 *
 * \details 
 * 1: Initialize coefficients
 * 0: Normal operation
 *
 * Field: ::VTSS_XFI_AE_REGS_A_TX_COEF_REQ_A . TXREQ_CINIT
 */
#define  VTSS_F_XFI_AE_REGS_A_TX_COEF_REQ_A_TXREQ_CINIT(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_XFI_AE_REGS_A_TX_COEF_REQ_A_TXREQ_CINIT     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_XFI_AE_REGS_A_TX_COEF_REQ_A_TXREQ_CINIT(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Reserved. Ignore contents.
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_A_TX_COEF_REQ_A . Reserved_AE26
 */
#define  VTSS_F_XFI_AE_REGS_A_TX_COEF_REQ_A_Reserved_AE26(x)  VTSS_ENCODE_BITFIELD(x,6,6)
#define  VTSS_M_XFI_AE_REGS_A_TX_COEF_REQ_A_Reserved_AE26     VTSS_ENCODE_BITMASK(6,6)
#define  VTSS_X_XFI_AE_REGS_A_TX_COEF_REQ_A_Reserved_AE26(x)  VTSS_EXTRACT_BITFIELD(x,6,6)

/** 
 * \brief
 * Coefficient 0 (Z +1) update
 * Link partner is requesting coefficients be updated
 *
 * \details 
 * 00: Hold
 * 01: Increment
 * 10: Decrement
 * 11: Reserved
 *
 * Field: ::VTSS_XFI_AE_REGS_A_TX_COEF_REQ_A . TXREQ_C0
 */
#define  VTSS_F_XFI_AE_REGS_A_TX_COEF_REQ_A_TXREQ_C0(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_XFI_AE_REGS_A_TX_COEF_REQ_A_TXREQ_C0     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_XFI_AE_REGS_A_TX_COEF_REQ_A_TXREQ_C0(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * Coefficient 0 (Z +1) update
 * Link partner is requesting coefficients be updated
 *
 * \details 
 * 00: Hold
 * 01: Increment
 * 10: Decrement
 * 11: Reserved
 *
 * Field: ::VTSS_XFI_AE_REGS_A_TX_COEF_REQ_A . TXREQ_C1
 */
#define  VTSS_F_XFI_AE_REGS_A_TX_COEF_REQ_A_TXREQ_C1(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_XFI_AE_REGS_A_TX_COEF_REQ_A_TXREQ_C1     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_XFI_AE_REGS_A_TX_COEF_REQ_A_TXREQ_C1(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Coefficient 0 (Z +1) update
 * Link partner is requesting coefficients be updated
 *
 * \details 
 * 00: Hold
 * 01: Increment
 * 10: Decrement
 * 11: Reserved
 *
 * Field: ::VTSS_XFI_AE_REGS_A_TX_COEF_REQ_A . TXREQ_C2
 */
#define  VTSS_F_XFI_AE_REGS_A_TX_COEF_REQ_A_TXREQ_C2(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_XFI_AE_REGS_A_TX_COEF_REQ_A_TXREQ_C2     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_XFI_AE_REGS_A_TX_COEF_REQ_A_TXREQ_C2(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Transmit coefficient status register
 *
 * \details
 * When adaptive equalization is active, this register controls the bits
 * transmitted in the Status Report field of the Training frame. This
 * status is this node's response to requests from the link partner in the
 * TX_COEF_REQ register. Normally, the Coefficient Update FSM processes
 * requests from the link partner, and dynamically updates the status in
 * this register. Alternatively, microcode may be used to process these
 * requests. If FSM_CU_DIS = 1 in the FSM_CTL register, then the AE core
 * will not automatically process these requests, and microcode can use
 * this register to send status responses. The TXSTAT_RDY bit is updated
 * based on the MC_RX_RDY bit in the FSM_CTL register.
 *
 * Register: \a XFI::AE_REGS_A::TX_COEF_STAT_A
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_AE_REGS_A_TX_COEF_STAT_A(target)  VTSS_IOREG(target,0x103)

/** 
 * \brief
 * Tx status ready
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_A_TX_COEF_STAT_A . TXSTAT_RDY
 */
#define  VTSS_F_XFI_AE_REGS_A_TX_COEF_STAT_A_TXSTAT_RDY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_XFI_AE_REGS_A_TX_COEF_STAT_A_TXSTAT_RDY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_XFI_AE_REGS_A_TX_COEF_STAT_A_TXSTAT_RDY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Reserved. Ignore contents.
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_A_TX_COEF_STAT_A . Reserved_AE36
 */
#define  VTSS_F_XFI_AE_REGS_A_TX_COEF_STAT_A_Reserved_AE36(x)  VTSS_ENCODE_BITFIELD(x,6,9)
#define  VTSS_M_XFI_AE_REGS_A_TX_COEF_STAT_A_Reserved_AE36     VTSS_ENCODE_BITMASK(6,9)
#define  VTSS_X_XFI_AE_REGS_A_TX_COEF_STAT_A_Reserved_AE36(x)  VTSS_EXTRACT_BITFIELD(x,6,9)

/** 
 * \brief
 * Coefficient 0 (Z +1) status
 * This node's response to TXREQ_C0 field request
 * Note: Transmitted TXSTAT_C0 field is generated in hardware if
 * HW_COEF_CTL=1 in the Register Mode Control register
 *
 * \details 
 * 11: Coefficient is set to maximum value
 * 10: Coefficient is set to minimum value
 * 01: Coefficient has been updated
 * 00: Coefficient is not updated
 *
 * Field: ::VTSS_XFI_AE_REGS_A_TX_COEF_STAT_A . TXSTAT_C0
 */
#define  VTSS_F_XFI_AE_REGS_A_TX_COEF_STAT_A_TXSTAT_C0(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_XFI_AE_REGS_A_TX_COEF_STAT_A_TXSTAT_C0     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_XFI_AE_REGS_A_TX_COEF_STAT_A_TXSTAT_C0(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * Coefficient 1 (Z 0) status
 * This node's response to TXREQ_C1 field request
 * Note: Transmitted TXSTAT_C0 field is generated in hardware if
 * HW_COEF_CTL=1 in the Register Mode Control register
 *
 * \details 
 * 11: Coefficient is set to maximum value
 * 10: Coefficient is set to minimum value
 * 01: Coefficient has been updated
 * 00: Coefficient is not updated
 *
 * Field: ::VTSS_XFI_AE_REGS_A_TX_COEF_STAT_A . TXSTAT_C1
 */
#define  VTSS_F_XFI_AE_REGS_A_TX_COEF_STAT_A_TXSTAT_C1(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_XFI_AE_REGS_A_TX_COEF_STAT_A_TXSTAT_C1     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_XFI_AE_REGS_A_TX_COEF_STAT_A_TXSTAT_C1(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Coefficient 2 (Z -1) status
 * This node's response to TXREQ_C2 field request
 * Note: Transmitted TXSTAT_C0 field is generated in hardware if
 * HW_COEF_CTL=1 in the Register Mode Control register
 *
 * \details 
 * 11: Coefficient is set to maximum value
 * 10: Coefficient is set to minimum value
 * 01: Coefficient has been updated
 * 00: Coefficient is not updated
 *
 * Field: ::VTSS_XFI_AE_REGS_A_TX_COEF_STAT_A . TXSTAT_C2
 */
#define  VTSS_F_XFI_AE_REGS_A_TX_COEF_STAT_A_TXSTAT_C2(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_XFI_AE_REGS_A_TX_COEF_STAT_A_TXSTAT_C2     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_XFI_AE_REGS_A_TX_COEF_STAT_A_TXSTAT_C2(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Register mode control register
 *
 * \details
 * This register selects optional features and modes of the core
 *
 * Register: \a XFI::AE_REGS_A::REG_MODE_A
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_AE_REGS_A_REG_MODE_A(target)  VTSS_IOREG(target,0x104)

/** 
 * \brief
 * Reserved. Always write to 0.
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_A_REG_MODE_A . Reserved_AE46
 */
#define  VTSS_F_XFI_AE_REGS_A_REG_MODE_A_Reserved_AE46(x)  VTSS_ENCODE_BITFIELD(x,6,12)
#define  VTSS_M_XFI_AE_REGS_A_REG_MODE_A_Reserved_AE46     VTSS_ENCODE_BITMASK(6,12)
#define  VTSS_X_XFI_AE_REGS_A_REG_MODE_A_Reserved_AE46(x)  VTSS_EXTRACT_BITFIELD(x,6,12)

/** 
 * \brief
 * Manchester decode option
 * This option sets the permissiveness of the decode of the Frame Marker
 * and Manchester bits by the Training Frame Decode logic.
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_A_REG_MODE_A . MAN_DEC
 */
#define  VTSS_F_XFI_AE_REGS_A_REG_MODE_A_MAN_DEC(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_XFI_AE_REGS_A_REG_MODE_A_MAN_DEC     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_XFI_AE_REGS_A_REG_MODE_A_MAN_DEC(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * Manual ready
 * This option determines whether hardware or microcode updates the
 * TXSTAT_RDY bit in the TX_COEF_STAT register
 *
 * \details 
 * 1: TXSTAT_RDY bit is R/W; hardware update of this bit is disabled.
 * 0: TXSTAT_RDY bit value is determined by Training FSM; register write is
 * disabled.
 *
 * Field: ::VTSS_XFI_AE_REGS_A_REG_MODE_A . MANUAL_RDY
 */
#define  VTSS_F_XFI_AE_REGS_A_REG_MODE_A_MANUAL_RDY(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_AE_REGS_A_REG_MODE_A_MANUAL_RDY     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_AE_REGS_A_REG_MODE_A_MANUAL_RDY(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Maximum wait timer disable
 * Allows microcode to disable the max_wait_timer. If this timer is
 * disabled, then the Training FSM time out transition to TRAINING_FAIL
 * state cannot occur.
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_A_REG_MODE_A . MWT_DISABLE
 */
#define  VTSS_F_XFI_AE_REGS_A_REG_MODE_A_MWT_DISABLE(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_AE_REGS_A_REG_MODE_A_MWT_DISABLE     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_AE_REGS_A_REG_MODE_A_MWT_DISABLE(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * MDIO override
 * Allows microcode to disable MDIO control
 *
 * \details 
 * 1: MDIO controls are disabled.
 * 0: Normal operation. (MDIO controls override microcode.)
 *
 * Field: ::VTSS_XFI_AE_REGS_A_REG_MODE_A . MDIO_OVR
 */
#define  VTSS_F_XFI_AE_REGS_A_REG_MODE_A_MDIO_OVR(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_AE_REGS_A_REG_MODE_A_MDIO_OVR     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_AE_REGS_A_REG_MODE_A_MDIO_OVR(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Sticky mode (diagnostics only)
 * Alters operation of the TX_COEF_REQ and RX_COEF_STAT registers. 
 * When Sticky Mode = 0 these registers always contain contents of the most
 * recently received training frame. 
 * When Sticky Mode = 1 register values are OR'd with prior contents and
 * registers must be written to reset. 
 * Note: This may result in incorrect interpretation of some register bit
 * fields and therefore this mode should only be used for diagnostics
 * purposes.
 *
 * \details 
 * 1: Sticky mode enabled
 * 0: Sticky mode disable
 *
 * Field: ::VTSS_XFI_AE_REGS_A_REG_MODE_A . STICKY_MODE
 */
#define  VTSS_F_XFI_AE_REGS_A_REG_MODE_A_STICKY_MODE(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_AE_REGS_A_REG_MODE_A_STICKY_MODE     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_AE_REGS_A_REG_MODE_A_STICKY_MODE(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief PRBS control register
 *
 * \details
 * When adaptive equalization is active, this register controls the PRBS
 * generation in the transmitted training frame and controls the PRBS
 * checker that checks the received training frame. PRBS checker status is
 * also accessed in this register.
 *
 * Register: \a XFI::AE_REGS_A::PRBS_CTL_A
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_AE_REGS_A_PRBS_CTL_A(target)  VTSS_IOREG(target,0x105)

/** 
 * \brief
 * PRBS checker error count
 * PRBS checker status which indicates the number of PRBS errors detected.
 * This field is reset by the 0->1 transition of the PRBS_CHK_RST bit, and
 * increments for each DWORD containing one or more bit errors. If the
 * value reaches 0xFF, the field does not increment past this maximum
 * value.
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_A_PRBS_CTL_A . PRBS_CHK_ERRCNT
 */
#define  VTSS_F_XFI_AE_REGS_A_PRBS_CTL_A_PRBS_CHK_ERRCNT(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_XFI_AE_REGS_A_PRBS_CTL_A_PRBS_CHK_ERRCNT     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_XFI_AE_REGS_A_PRBS_CTL_A_PRBS_CHK_ERRCNT(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * PRBS unsynchronized frame counter
 * Counts the number of training frames for which the PRBS Checker never
 * synchronized to the PRBS data pattern. This counter is reset by
 * RESET_ASYNC, 0->1 transition of the PRBS_CHK_RST, and PRBS_CHK_OFF. Once
 * the maximum counter value is reached, value of 111 is held until counter
 * is reset.
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_A_PRBS_CTL_A . PRBS_SYNCCNT
 */
#define  VTSS_F_XFI_AE_REGS_A_PRBS_CTL_A_PRBS_SYNCCNT(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_XFI_AE_REGS_A_PRBS_CTL_A_PRBS_SYNCCNT     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_XFI_AE_REGS_A_PRBS_CTL_A_PRBS_SYNCCNT(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * PRBS checker sync
 * PRBS checker status that indicates the PRBS checker is initialized and
 * is actively checking PRBS bits in received training frame
 *
 * \details 
 * 1: PRBS checker is synchronized
 * 0: PRBS checker is not synchronized
 *
 * Field: ::VTSS_XFI_AE_REGS_A_PRBS_CTL_A . PRBS_CHK_SYNC
 */
#define  VTSS_F_XFI_AE_REGS_A_PRBS_CTL_A_PRBS_CHK_SYNC(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_AE_REGS_A_PRBS_CTL_A_PRBS_CHK_SYNC     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_AE_REGS_A_PRBS_CTL_A_PRBS_CHK_SYNC(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * PRBS checker reset
 * Controls PRBS checker that checks PRBS bits in received training frame.
 * Reset is generated after 0->1 transition of this bit. Bit must be reset
 * by microcode before another reset can be generated.
 *
 * \details 
 * 1: PRBS checker is reset
 * 0: Normal operation
 *
 * Field: ::VTSS_XFI_AE_REGS_A_PRBS_CTL_A . PRBS_CHK_RST
 */
#define  VTSS_F_XFI_AE_REGS_A_PRBS_CTL_A_PRBS_CHK_RST(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_AE_REGS_A_PRBS_CTL_A_PRBS_CHK_RST     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_AE_REGS_A_PRBS_CTL_A_PRBS_CHK_RST(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * PRBS checker disable
 * Controls PRBS checker that checks PRBS bits in received training frame
 *
 * \details 
 * 1: PRBS checker disabled
 * 0: Normal operation (PRBS checker enabled)
 *
 * Field: ::VTSS_XFI_AE_REGS_A_PRBS_CTL_A . PRBS_CHK_OFF
 */
#define  VTSS_F_XFI_AE_REGS_A_PRBS_CTL_A_PRBS_CHK_OFF(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_AE_REGS_A_PRBS_CTL_A_PRBS_CHK_OFF     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_AE_REGS_A_PRBS_CTL_A_PRBS_CHK_OFF(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * PRBS generator force error
 * Controls PRBS generator that generates PRBS bits in transmitted training
 * frame. Error is generated after 0->1 transition of this bit. Bit must be
 * reset by microcode before another error can be generated.
 *
 * \details 
 * 1: Bit error is inserted in transmitted training frame
 * 0: Normal operation
 *
 * Field: ::VTSS_XFI_AE_REGS_A_PRBS_CTL_A . PRBS_GEN_FRCERR
 */
#define  VTSS_F_XFI_AE_REGS_A_PRBS_CTL_A_PRBS_GEN_FRCERR(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_AE_REGS_A_PRBS_CTL_A_PRBS_GEN_FRCERR     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_AE_REGS_A_PRBS_CTL_A_PRBS_GEN_FRCERR(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * PRBS generator disable
 * Controls PRBS generator that generates PRBS bits in transmitted training
 * frame
 *
 * \details 
 * 1: PRBS generator disabled
 * 0: Normal operation (PRBS generator enabled)
 *
 * Field: ::VTSS_XFI_AE_REGS_A_PRBS_CTL_A . PRBS_GEN_OFF
 */
#define  VTSS_F_XFI_AE_REGS_A_PRBS_CTL_A_PRBS_GEN_OFF(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_AE_REGS_A_PRBS_CTL_A_PRBS_GEN_OFF     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_AE_REGS_A_PRBS_CTL_A_PRBS_GEN_OFF(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Training state machine register
 *
 * \details
 * This register controls the training state machine and provides state
 * machine status information.
 * The MC_RX_RDY bit is used to handshake between the Training FSM and
 * microcode. This bit is reset when the Training FSM is in any state other
 * than TRAIN_LOCAL or TRAIN_REMOTE state. Microcode trains the local node
 * as appropriate while the Training FSM is in the TRAIN_LOCAL state. Upon
 * completion of this training algorithm, microcode writes this register to
 * set the MC_RX_RDY bit, which causes the Training FSM to transition to
 * the TRAIN_REMOTE state.
 *
 * Register: \a XFI::AE_REGS_A::FSM_CTL_A
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_AE_REGS_A_FSM_CTL_A(target)  VTSS_IOREG(target,0x106)

/** 
 * \brief
 * Reserved
 * Always write to 0
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_A_FSM_CTL_A . Reserved_AE615
 */
#define  VTSS_F_XFI_AE_REGS_A_FSM_CTL_A_Reserved_AE615(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_XFI_AE_REGS_A_FSM_CTL_A_Reserved_AE615     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_XFI_AE_REGS_A_FSM_CTL_A_Reserved_AE615(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * FSM train local state
 * Training FSM is in TRAIN_LOCAL state. Microcode must execute the
 * training algorithm in this state.
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_A_FSM_CTL_A . FSM_TR_LCL
 */
#define  VTSS_F_XFI_AE_REGS_A_FSM_CTL_A_FSM_TR_LCL(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_XFI_AE_REGS_A_FSM_CTL_A_FSM_TR_LCL     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_XFI_AE_REGS_A_FSM_CTL_A_FSM_TR_LCL(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Good frame markers
 * FSM count of good frame markers received by Frame Lock state machine. 
 * Note: Upper limit of count range is limited by the IEEE 802.3ap
 * description of this counter.
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_A_FSM_CTL_A . FSM_GDMRK
 */
#define  VTSS_F_XFI_AE_REGS_A_FSM_CTL_A_FSM_GDMRK(x)  VTSS_ENCODE_BITFIELD(x,11,3)
#define  VTSS_M_XFI_AE_REGS_A_FSM_CTL_A_FSM_GDMRK     VTSS_ENCODE_BITMASK(11,3)
#define  VTSS_X_XFI_AE_REGS_A_FSM_CTL_A_FSM_GDMRK(x)  VTSS_EXTRACT_BITFIELD(x,11,3)

/** 
 * \brief
 * Bad frame markers
 * FSM count of bad frame markers received by Frame Lock state machine. 
 * Note: Upper limit of count range is limited by the IEEE 802.3ap
 * description of this counter. (The value of this counter in the AE core
 * is in the range of 0 to 4. Detection of the 5th bad marker causes an FSM
 * state transition, which resets this counter.)
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_A_FSM_CTL_A . FSM_BADMRK
 */
#define  VTSS_F_XFI_AE_REGS_A_FSM_CTL_A_FSM_BADMRK(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_XFI_AE_REGS_A_FSM_CTL_A_FSM_BADMRK     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_XFI_AE_REGS_A_FSM_CTL_A_FSM_BADMRK(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * Training failure
 * Training process failed due to expiration of the max_wait_timer while
 * training was in progress. Bit is reset if training restarts.
 *
 * \details 
 * 1: Training Failed
 * 0: Training did not fail
 *
 * Field: ::VTSS_XFI_AE_REGS_A_FSM_CTL_A . FSM_TR_FAIL
 */
#define  VTSS_F_XFI_AE_REGS_A_FSM_CTL_A_FSM_TR_FAIL(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_XFI_AE_REGS_A_FSM_CTL_A_FSM_TR_FAIL     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_XFI_AE_REGS_A_FSM_CTL_A_FSM_TR_FAIL(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Training active
 * FSM state indicates training protocol is in process
 *
 * \details 
 * 1: Training is in progress
 * 0: Training is not in progress
 *
 * Field: ::VTSS_XFI_AE_REGS_A_FSM_CTL_A . FSM_TR_ACT
 */
#define  VTSS_F_XFI_AE_REGS_A_FSM_CTL_A_FSM_TR_ACT(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_XFI_AE_REGS_A_FSM_CTL_A_FSM_TR_ACT     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_XFI_AE_REGS_A_FSM_CTL_A_FSM_TR_ACT(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Frame lock
 * Receiver acquires training frame delineation and FSM is receiving valid
 * training frames
 *
 * \details 
 * 1: Frame lock achieved
 * 0: Frame lock not achieved
 *
 * Field: ::VTSS_XFI_AE_REGS_A_FSM_CTL_A . FSM_FRM_LCK
 */
#define  VTSS_F_XFI_AE_REGS_A_FSM_CTL_A_FSM_FRM_LCK(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_XFI_AE_REGS_A_FSM_CTL_A_FSM_FRM_LCK     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_XFI_AE_REGS_A_FSM_CTL_A_FSM_FRM_LCK(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Receiver training complete
 * Indicates the remote transmit and local receiver equalizers have been
 * optimized and normal data transmission may commence. Bit is reset if
 * training restarts.
 *
 * \details 
 * 1: Training complete
 * 0: Training not complete
 *
 * Field: ::VTSS_XFI_AE_REGS_A_FSM_CTL_A . FSM_TR_COMP
 */
#define  VTSS_F_XFI_AE_REGS_A_FSM_CTL_A_FSM_TR_COMP(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_AE_REGS_A_FSM_CTL_A_FSM_TR_COMP     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_AE_REGS_A_FSM_CTL_A_FSM_TR_COMP(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Held reset if the Training FSM state is not TRAIN_LOCAL or TRAIN_REMOTE.
 * When training is active, microcode sets this bit when the local Rx is
 * trained.
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_A_FSM_CTL_A . MC_RX_RDY
 */
#define  VTSS_F_XFI_AE_REGS_A_FSM_CTL_A_MC_RX_RDY(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_AE_REGS_A_FSM_CTL_A_MC_RX_RDY     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_AE_REGS_A_FSM_CTL_A_MC_RX_RDY(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Coefficient update FSM disable
 * Disable Coefficient Update FSM and execute coefficient updates using
 * microcode
 *
 * \details 
 * 1: Coefficient Update FSM is disabled
 * 0: Normal operation (Coefficient Update FSM is enabled)
 *
 * Field: ::VTSS_XFI_AE_REGS_A_FSM_CTL_A . FSM_CU_DIS
 */
#define  VTSS_F_XFI_AE_REGS_A_FSM_CTL_A_FSM_CU_DIS(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_AE_REGS_A_FSM_CTL_A_FSM_CU_DIS     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_AE_REGS_A_FSM_CTL_A_FSM_CU_DIS(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Training restart
 * FSM mr_restart_training control. Restart is generated after 0->1
 * transition of this bit. Bit must be reset by microcode before another
 * restart command can be generated. MDIO interface can also trigger a
 * restart unless MDIO_OVR bit is set in REG_MODE register.
 *
 * \details 
 * 1: Reset the training state machine and restart training
 * 0: Normal operation
 *
 * Field: ::VTSS_XFI_AE_REGS_A_FSM_CTL_A . FSM_TR_RST
 */
#define  VTSS_F_XFI_AE_REGS_A_FSM_CTL_A_FSM_TR_RST(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_AE_REGS_A_FSM_CTL_A_FSM_TR_RST     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_AE_REGS_A_FSM_CTL_A_FSM_TR_RST(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Training enable
 * FSM mr_training_enable control. MDIO interface can also enable training
 * unless MDIO_OVR bit is set in REG_MODE register.
 *
 * \details 
 * 1: Force training to be enabled
 * 0: MDIO controls whether training is enabled
 *
 * Field: ::VTSS_XFI_AE_REGS_A_FSM_CTL_A . FSM_TR_EN
 */
#define  VTSS_F_XFI_AE_REGS_A_FSM_CTL_A_FSM_TR_EN(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_AE_REGS_A_FSM_CTL_A_FSM_TR_EN     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_AE_REGS_A_FSM_CTL_A_FSM_TR_EN(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief FSM state register
 *
 * \details
 * This status register provides diagnostic status to the current FSM
 * states
 *
 * Register: \a XFI::AE_REGS_A::FSM_STATE_A
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_AE_REGS_A_FSM_STATE_A(target)  VTSS_IOREG(target,0x107)

/** 
 * \brief
 * Coefficient Update FSM state
 * Current state of the Coefficient Update FSM for the C2 coefficient
 *
 * \details 
 * 001 NOTUPDATED: Idle
 * 010 UPDATEIF: Set Cx_Update to Serdes and wait for response on Cx_ACK
 * 011 WRACK: Forward Serdes response to link partner
 * 100 ACKED: Wait for Serdes to set Cx_ACK = not updated
 * 101 WAIT: Wait for link partner to set TXREQ_Cx request = hold
 * 110 WRUPDATE: Send TXSTAT_Cx = not updated
 *
 * Field: ::VTSS_XFI_AE_REGS_A_FSM_STATE_A . CCFSM_STATE_C2
 */
#define  VTSS_F_XFI_AE_REGS_A_FSM_STATE_A_CCFSM_STATE_C2(x)  VTSS_ENCODE_BITFIELD(x,13,3)
#define  VTSS_M_XFI_AE_REGS_A_FSM_STATE_A_CCFSM_STATE_C2     VTSS_ENCODE_BITMASK(13,3)
#define  VTSS_X_XFI_AE_REGS_A_FSM_STATE_A_CCFSM_STATE_C2(x)  VTSS_EXTRACT_BITFIELD(x,13,3)

/** 
 * \brief
 * Coefficient update FSM state
 * Current state of the Coefficient Update FSM for the C1 coefficient
 *
 * \details 
 * 001 NOTUPDATED: Idle
 * 010 UPDATEIF: Set Cx_Update to Serdes and wait for response on Cx_ACK
 * 011 WRACK: Forward Serdes response to link partner
 * 100 ACKED: Wait for Serdes to set Cx_ACK = not updated
 * 101 WAIT: Wait for link partner to set TXREQ_Cx request = hold
 * 110 WRUPDATE: Send TXSTAT_Cx = not updated
 *
 * Field: ::VTSS_XFI_AE_REGS_A_FSM_STATE_A . CCFSM_STATE_C1
 */
#define  VTSS_F_XFI_AE_REGS_A_FSM_STATE_A_CCFSM_STATE_C1(x)  VTSS_ENCODE_BITFIELD(x,10,3)
#define  VTSS_M_XFI_AE_REGS_A_FSM_STATE_A_CCFSM_STATE_C1     VTSS_ENCODE_BITMASK(10,3)
#define  VTSS_X_XFI_AE_REGS_A_FSM_STATE_A_CCFSM_STATE_C1(x)  VTSS_EXTRACT_BITFIELD(x,10,3)

/** 
 * \brief
 * Coefficient update FSM state
 * Current state of the Coefficient Update FSM for the C0 coefficient
 *
 * \details 
 * 001 NOTUPDATED: Idle
 * 010 UPDATEIF: Set Cx_Update to Serdes and wait for response on Cx_ACK
 * 011 WRACK: Forward Serdes response to link partner
 * 100 ACKED: Wait for Serdes to set Cx_ACK = not updated
 * 101 WAIT: Wait for link partner to set TXREQ_Cx request = hold
 * 110 WRUPDATE: Send TXSTAT_Cx = not updated
 *
 * Field: ::VTSS_XFI_AE_REGS_A_FSM_STATE_A . CCFSM_STATE_C0
 */
#define  VTSS_F_XFI_AE_REGS_A_FSM_STATE_A_CCFSM_STATE_C0(x)  VTSS_ENCODE_BITFIELD(x,7,3)
#define  VTSS_M_XFI_AE_REGS_A_FSM_STATE_A_CCFSM_STATE_C0     VTSS_ENCODE_BITMASK(7,3)
#define  VTSS_X_XFI_AE_REGS_A_FSM_STATE_A_CCFSM_STATE_C0(x)  VTSS_EXTRACT_BITFIELD(x,7,3)

/** 
 * \brief
 * Frame lock FSM state
 * Current state of the Frame Lock FSM
 *
 * \details 
 * 001 IDLE
 * 010 SEEK: Look for frame marker
 * 011 WAIT: Wait 136 cycles
 * 100 CHECK: Check whether marker is valid on current cycle
 * 111 INVALID: State decode is invalid. Try again.
 *
 * Field: ::VTSS_XFI_AE_REGS_A_FSM_STATE_A . FLFSM_STATE
 */
#define  VTSS_F_XFI_AE_REGS_A_FSM_STATE_A_FLFSM_STATE(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_XFI_AE_REGS_A_FSM_STATE_A_FLFSM_STATE     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_XFI_AE_REGS_A_FSM_STATE_A_FLFSM_STATE(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * Reserved
 * Always 0
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_A_FSM_STATE_A . Reserved_AE73
 */
#define  VTSS_F_XFI_AE_REGS_A_FSM_STATE_A_Reserved_AE73(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_AE_REGS_A_FSM_STATE_A_Reserved_AE73     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_AE_REGS_A_FSM_STATE_A_Reserved_AE73(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Training FSM state
 * Current state of the Training FSM
 *
 * \details 
 * 001 INITIALIZE
 * 010 SEND_TRAINING
 * 011 TRAIN_LOCAL
 * 100 TRAIN_REMOTE
 * 101 LINK_READY
 * 110 SEND_DATA
 * 111 TRAINING_FAILURE
 *
 * Field: ::VTSS_XFI_AE_REGS_A_FSM_STATE_A . TFSM_STATE
 */
#define  VTSS_F_XFI_AE_REGS_A_FSM_STATE_A_TFSM_STATE(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_XFI_AE_REGS_A_FSM_STATE_A_TFSM_STATE     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_XFI_AE_REGS_A_FSM_STATE_A_TFSM_STATE(x)  VTSS_EXTRACT_BITFIELD(x,0,3)

/**
 * Register Group: \a XFI::AE_REGS_B
 *
 * Lane-B AE register interface for microcode control of the training process
 */


/** 
 * \brief Receive coefficient update request register
 *
 * \details
 * When adaptive equalization is active, this register controls the bits
 * transmitted in the Coefficient Update field of the Training frame.
 * Microcode is responsible for generating coefficient update commands and
 * writing this register while the Training FSM is in the TRAIN_LOCAL
 * state, as indicated by the FSM_TR_LCL bit in the FSM_CTL register.
 *
 * Register: \a XFI::AE_REGS_B::RX_COEF_REQ_B
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_AE_REGS_B_RX_COEF_REQ_B(target)  VTSS_IOREG(target,0x200)

/** 
 * \brief
 * Reserved
 * Always write to 0
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_B_RX_COEF_REQ_B . Reserved_AE014
 */
#define  VTSS_F_XFI_AE_REGS_B_RX_COEF_REQ_B_Reserved_AE014(x)  VTSS_ENCODE_BITFIELD(x,14,2)
#define  VTSS_M_XFI_AE_REGS_B_RX_COEF_REQ_B_Reserved_AE014     VTSS_ENCODE_BITMASK(14,2)
#define  VTSS_X_XFI_AE_REGS_B_RX_COEF_REQ_B_Reserved_AE014(x)  VTSS_EXTRACT_BITFIELD(x,14,2)

/** 
 * \brief
 * Coefficient preset
 * Request link partner update FFE coefficients to preset values
 *
 * \details 
 * 1: Preset coefficients
 * 0: Normal operation
 *
 * Field: ::VTSS_XFI_AE_REGS_B_RX_COEF_REQ_B . RXREQ_CPRE
 */
#define  VTSS_F_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_CPRE(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_CPRE     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_CPRE(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Coefficient initialize
 * Request link partner update FFE coefficients to initial values
 *
 * \details 
 * 1: Initialize coefficients
 * 0: Normal operation
 *
 * Field: ::VTSS_XFI_AE_REGS_B_RX_COEF_REQ_B . RXREQ_CINIT
 */
#define  VTSS_F_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_CINIT(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_CINIT     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_CINIT(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Reserved
 * Always write to 0
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_B_RX_COEF_REQ_B . Reserved_AE06
 */
#define  VTSS_F_XFI_AE_REGS_B_RX_COEF_REQ_B_Reserved_AE06(x)  VTSS_ENCODE_BITFIELD(x,6,6)
#define  VTSS_M_XFI_AE_REGS_B_RX_COEF_REQ_B_Reserved_AE06     VTSS_ENCODE_BITMASK(6,6)
#define  VTSS_X_XFI_AE_REGS_B_RX_COEF_REQ_B_Reserved_AE06(x)  VTSS_EXTRACT_BITFIELD(x,6,6)

/** 
 * \brief
 * Coefficient 0 (Z +1) update
 * Request link partner update FFE coefficients per below coding
 *
 * \details 
 * 00: Hold
 * 01: Increment
 * 10: Decrement
 * 11: Reserved
 *
 * Field: ::VTSS_XFI_AE_REGS_B_RX_COEF_REQ_B . RXREQ_C0
 */
#define  VTSS_F_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_C0(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_C0     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_C0(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * Coefficient 1 (Z 0) update
 * Request link partner update FFE coefficients per below coding
 *
 * \details 
 * 00: Hold
 * 01: Increment
 * 10: Decrement
 * 11: Reserved
 *
 * Field: ::VTSS_XFI_AE_REGS_B_RX_COEF_REQ_B . RXREQ_C1
 */
#define  VTSS_F_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_C1(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_C1     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_C1(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Coefficient 2 (Z -1) update
 * Request link partner update FFE coefficients per below coding
 *
 * \details 
 * 00: Hold
 * 01: Increment
 * 10: Decrement
 * 11: Reserved
 *
 * Field: ::VTSS_XFI_AE_REGS_B_RX_COEF_REQ_B . RXREQ_C2
 */
#define  VTSS_F_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_C2(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_C2     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_XFI_AE_REGS_B_RX_COEF_REQ_B_RXREQ_C2(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Receive coefficient status register
 *
 * \details
 * When adaptive equalization is active, this register contains the bits
 * received in the Status Report field of the Training frame sent by the
 * link partner. This status is the link partner's response to coefficient
 * update requests in the RX_COEF_REQ register. Microcode is responsible
 * for monitoring and/or clearing this status while the Training FSM is in
 * the TRAIN_LOCAL state, as indicated by the FSM_TR_LCL bit in the FSM_CTL
 * register. The Sticky Mode bit in the Register Mode Control register (for
 * diagnostics) affects operation of this register. If STICKY_MODE= 1 then
 * bits of this register must be written to reset them.
 *
 * Register: \a XFI::AE_REGS_B::RX_COEF_STAT_B
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_AE_REGS_B_RX_COEF_STAT_B(target)  VTSS_IOREG(target,0x201)

/** 
 * \brief
 * Receiver ready
 * This bit indicates training status of the link partner
 *
 * \details 
 * 1: Link partner has determined that training is complete and is prepared
 * to receive data
 * 0: Link partner is requesting that training continue
 *
 * Field: ::VTSS_XFI_AE_REGS_B_RX_COEF_STAT_B . RXSTAT_RDY
 */
#define  VTSS_F_XFI_AE_REGS_B_RX_COEF_STAT_B_RXSTAT_RDY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_XFI_AE_REGS_B_RX_COEF_STAT_B_RXSTAT_RDY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_XFI_AE_REGS_B_RX_COEF_STAT_B_RXSTAT_RDY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Reserved. Ignore contents.
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_B_RX_COEF_STAT_B . Reserved_AE16
 */
#define  VTSS_F_XFI_AE_REGS_B_RX_COEF_STAT_B_Reserved_AE16(x)  VTSS_ENCODE_BITFIELD(x,6,9)
#define  VTSS_M_XFI_AE_REGS_B_RX_COEF_STAT_B_Reserved_AE16     VTSS_ENCODE_BITMASK(6,9)
#define  VTSS_X_XFI_AE_REGS_B_RX_COEF_STAT_B_Reserved_AE16(x)  VTSS_EXTRACT_BITFIELD(x,6,9)

/** 
 * \brief
 * Coefficient 0 (Z +1) status
 * Link partner response to RXREQ_C0 field request
 *
 * \details 
 * 11: Coefficient is set to maximum value
 * 10: Coefficient is set to minimum value
 * 01: Coefficient has been updated
 * 00: Coefficient is not updated
 *
 * Field: ::VTSS_XFI_AE_REGS_B_RX_COEF_STAT_B . RXSTAT_C0
 */
#define  VTSS_F_XFI_AE_REGS_B_RX_COEF_STAT_B_RXSTAT_C0(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_XFI_AE_REGS_B_RX_COEF_STAT_B_RXSTAT_C0     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_XFI_AE_REGS_B_RX_COEF_STAT_B_RXSTAT_C0(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * Coefficient 1 (Z 0) status
 * Link partner response to RXREQ_C1 field request
 *
 * \details 
 * 11: Coefficient is set to maximum value
 * 10: Coefficient is set to minimum value
 * 01: Coefficient has been updated
 * 00: Coefficient is not updated
 *
 * Field: ::VTSS_XFI_AE_REGS_B_RX_COEF_STAT_B . RXSTAT_C1
 */
#define  VTSS_F_XFI_AE_REGS_B_RX_COEF_STAT_B_RXSTAT_C1(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_XFI_AE_REGS_B_RX_COEF_STAT_B_RXSTAT_C1     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_XFI_AE_REGS_B_RX_COEF_STAT_B_RXSTAT_C1(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Coefficient 2 (Z -1) status
 * Link partner response to RXREQ_C2 field request
 *
 * \details 
 * 11: Coefficient is set to maximum value
 * 10: Coefficient is set to minimum value
 * 01: Coefficient has been updated
 * 00: Coefficient is not updated
 *
 * Field: ::VTSS_XFI_AE_REGS_B_RX_COEF_STAT_B . RXSTAT_C2
 */
#define  VTSS_F_XFI_AE_REGS_B_RX_COEF_STAT_B_RXSTAT_C2(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_XFI_AE_REGS_B_RX_COEF_STAT_B_RXSTAT_C2     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_XFI_AE_REGS_B_RX_COEF_STAT_B_RXSTAT_C2(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Transmit coefficient update request register
 *
 * \details
 * When adaptive equalization is active, this register contains the bits
 * received in the Coefficient Update field of the Training frame sent by
 * the link partner. This node processes requests and responds by sending
 * status via the TX_COEF_STAT register. Normally, the Coefficient Update
 * FSM processes requests from the link partner. Alternatively, microcode
 * may be used to process these requests. If FSM_CU_DIS = 1 in the FSM_CTL
 * register, then the AE core will not automatically process these
 * requests. The Sticky Mode bit in the Register Mode Control register (for
 * diagnostics) affects operation of this register. See the description of
 * this bit in the REG_CTL register. If STICKY_MODE = 1 then bits of this
 * register must be written to reset them.
 *
 * Register: \a XFI::AE_REGS_B::TX_COEF_REQ_B
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_AE_REGS_B_TX_COEF_REQ_B(target)  VTSS_IOREG(target,0x202)

/** 
 * \brief
 * Reserved. Ignore contents
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_B_TX_COEF_REQ_B . Reserved_AE214
 */
#define  VTSS_F_XFI_AE_REGS_B_TX_COEF_REQ_B_Reserved_AE214(x)  VTSS_ENCODE_BITFIELD(x,14,2)
#define  VTSS_M_XFI_AE_REGS_B_TX_COEF_REQ_B_Reserved_AE214     VTSS_ENCODE_BITMASK(14,2)
#define  VTSS_X_XFI_AE_REGS_B_TX_COEF_REQ_B_Reserved_AE214(x)  VTSS_EXTRACT_BITFIELD(x,14,2)

/** 
 * \brief
 * Coefficient preset
 * Link partner is requesting coefficients be updated to preset values
 *
 * \details 
 * 1: Preset coefficients
 * 0: Normal operation
 *
 * Field: ::VTSS_XFI_AE_REGS_B_TX_COEF_REQ_B . TXREQ_CPRE
 */
#define  VTSS_F_XFI_AE_REGS_B_TX_COEF_REQ_B_TXREQ_CPRE(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_XFI_AE_REGS_B_TX_COEF_REQ_B_TXREQ_CPRE     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_XFI_AE_REGS_B_TX_COEF_REQ_B_TXREQ_CPRE(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Coefficient initialize
 * Link partner is requesting coefficients be updated to initial values
 *
 * \details 
 * 1: Initialize coefficients
 * 0: Normal operation
 *
 * Field: ::VTSS_XFI_AE_REGS_B_TX_COEF_REQ_B . TXREQ_CINIT
 */
#define  VTSS_F_XFI_AE_REGS_B_TX_COEF_REQ_B_TXREQ_CINIT(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_XFI_AE_REGS_B_TX_COEF_REQ_B_TXREQ_CINIT     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_XFI_AE_REGS_B_TX_COEF_REQ_B_TXREQ_CINIT(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Reserved. Ignore contents.
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_B_TX_COEF_REQ_B . Reserved_AE26
 */
#define  VTSS_F_XFI_AE_REGS_B_TX_COEF_REQ_B_Reserved_AE26(x)  VTSS_ENCODE_BITFIELD(x,6,6)
#define  VTSS_M_XFI_AE_REGS_B_TX_COEF_REQ_B_Reserved_AE26     VTSS_ENCODE_BITMASK(6,6)
#define  VTSS_X_XFI_AE_REGS_B_TX_COEF_REQ_B_Reserved_AE26(x)  VTSS_EXTRACT_BITFIELD(x,6,6)

/** 
 * \brief
 * Coefficient 0 (Z +1) update
 * Link partner is requesting coefficients be updated per below coding
 *
 * \details 
 * 00: Hold
 * 01: Increment
 * 10: Decrement
 * 11: Reserved
 *
 * Field: ::VTSS_XFI_AE_REGS_B_TX_COEF_REQ_B . TXREQ_C0
 */
#define  VTSS_F_XFI_AE_REGS_B_TX_COEF_REQ_B_TXREQ_C0(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_XFI_AE_REGS_B_TX_COEF_REQ_B_TXREQ_C0     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_XFI_AE_REGS_B_TX_COEF_REQ_B_TXREQ_C0(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * Coefficient 0 (Z +1) update
 * Link partner is requesting coefficients be updated per below coding
 *
 * \details 
 * 00: Hold
 * 01: Increment
 * 10: Decrement
 * 11: Reserved
 *
 * Field: ::VTSS_XFI_AE_REGS_B_TX_COEF_REQ_B . TXREQ_C1
 */
#define  VTSS_F_XFI_AE_REGS_B_TX_COEF_REQ_B_TXREQ_C1(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_XFI_AE_REGS_B_TX_COEF_REQ_B_TXREQ_C1     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_XFI_AE_REGS_B_TX_COEF_REQ_B_TXREQ_C1(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Coefficient 0 (Z +1) update
 * Link partner is requesting coefficients be updated per below coding
 *
 * \details 
 * 00: Hold
 * 01: Increment
 * 10: Decrement
 * 11: Reserved
 *
 * Field: ::VTSS_XFI_AE_REGS_B_TX_COEF_REQ_B . TXREQ_C2
 */
#define  VTSS_F_XFI_AE_REGS_B_TX_COEF_REQ_B_TXREQ_C2(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_XFI_AE_REGS_B_TX_COEF_REQ_B_TXREQ_C2     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_XFI_AE_REGS_B_TX_COEF_REQ_B_TXREQ_C2(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Transmit coefficient status register
 *
 * \details
 * When adaptive equalization is active, this register controls the bits
 * transmitted in the Status Report field of the Training frame. This
 * status is this node's response to requests from the link partner in the
 * TX_COEF_REQ register. Normally, the Coefficient Update FSM processes
 * requests from the link partner, and dynamically updates the status in
 * this register. Alternatively, microcode may be used to process these
 * requests. If FSM_CU_DIS = 1 in the FSM_CTL register, then the AE core
 * will not automatically process these requests, and microcode can use
 * this register to send status responses. The TXSTAT_RDY bit is updated
 * based on the MC_RX_RDY bit in the FSM_CTL register.
 *
 * Register: \a XFI::AE_REGS_B::TX_COEF_STAT_B
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_AE_REGS_B_TX_COEF_STAT_B(target)  VTSS_IOREG(target,0x203)

/** 
 * \brief
 * Tx status ready
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_B_TX_COEF_STAT_B . TXSTAT_RDY
 */
#define  VTSS_F_XFI_AE_REGS_B_TX_COEF_STAT_B_TXSTAT_RDY(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_XFI_AE_REGS_B_TX_COEF_STAT_B_TXSTAT_RDY     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_XFI_AE_REGS_B_TX_COEF_STAT_B_TXSTAT_RDY(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Reserved. Ignore contents.
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_B_TX_COEF_STAT_B . Reserved_AE36
 */
#define  VTSS_F_XFI_AE_REGS_B_TX_COEF_STAT_B_Reserved_AE36(x)  VTSS_ENCODE_BITFIELD(x,6,9)
#define  VTSS_M_XFI_AE_REGS_B_TX_COEF_STAT_B_Reserved_AE36     VTSS_ENCODE_BITMASK(6,9)
#define  VTSS_X_XFI_AE_REGS_B_TX_COEF_STAT_B_Reserved_AE36(x)  VTSS_EXTRACT_BITFIELD(x,6,9)

/** 
 * \brief
 * Coefficient 0 (Z +1) status
 * This node's response to TXREQ_C0 field request.
 * Note: Transmitted TXSTAT_C0 field is generated in hardware if
 * HW_COEF_CTL=1 in the Register Mode Control register.
 *
 * \details 
 * 11: Coefficient is set to maximum value
 * 10: Coefficient is set to minimum value
 * 01: Coefficient has been updated
 * 00: Coefficient is not updated
 *
 * Field: ::VTSS_XFI_AE_REGS_B_TX_COEF_STAT_B . TXSTAT_C0
 */
#define  VTSS_F_XFI_AE_REGS_B_TX_COEF_STAT_B_TXSTAT_C0(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_XFI_AE_REGS_B_TX_COEF_STAT_B_TXSTAT_C0     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_XFI_AE_REGS_B_TX_COEF_STAT_B_TXSTAT_C0(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * Coefficient 1 (Z 0) status
 * This node's response to TXREQ_C1 field request
 * Note: Transmitted TXSTAT_C0 field is generated in hardware if
 * HW_COEF_CTL=1 in the Register Mode Control register.
 *
 * \details 
 * 11: Coefficient is set to maximum value
 * 10: Coefficient is set to minimum value
 * 01: Coefficient has been updated
 * 00: Coefficient is not updated
 *
 * Field: ::VTSS_XFI_AE_REGS_B_TX_COEF_STAT_B . TXSTAT_C1
 */
#define  VTSS_F_XFI_AE_REGS_B_TX_COEF_STAT_B_TXSTAT_C1(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_XFI_AE_REGS_B_TX_COEF_STAT_B_TXSTAT_C1     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_XFI_AE_REGS_B_TX_COEF_STAT_B_TXSTAT_C1(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Coefficient 2 (Z -1) status
 * This node's response to TXREQ_C2 field request
 * Note: Transmitted TXSTAT_C0 field is generated in hardware if
 * HW_COEF_CTL=1 in the Register Mode Control register.
 *
 * \details 
 * 11: Coefficient is set to maximum value
 * 10: Coefficient is set to minimum value
 * 01: Coefficient has been updated
 * 00: Coefficient is not updated
 *
 * Field: ::VTSS_XFI_AE_REGS_B_TX_COEF_STAT_B . TXSTAT_C2
 */
#define  VTSS_F_XFI_AE_REGS_B_TX_COEF_STAT_B_TXSTAT_C2(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_XFI_AE_REGS_B_TX_COEF_STAT_B_TXSTAT_C2     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_XFI_AE_REGS_B_TX_COEF_STAT_B_TXSTAT_C2(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Register mode control register
 *
 * \details
 * This register selects optional features and modes of the core.
 *
 * Register: \a XFI::AE_REGS_B::REG_MODE_B
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_AE_REGS_B_REG_MODE_B(target)  VTSS_IOREG(target,0x204)

/** 
 * \brief
 * Reserved
 * Always write to 0
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_B_REG_MODE_B . Reserved_AE46
 */
#define  VTSS_F_XFI_AE_REGS_B_REG_MODE_B_Reserved_AE46(x)  VTSS_ENCODE_BITFIELD(x,6,12)
#define  VTSS_M_XFI_AE_REGS_B_REG_MODE_B_Reserved_AE46     VTSS_ENCODE_BITMASK(6,12)
#define  VTSS_X_XFI_AE_REGS_B_REG_MODE_B_Reserved_AE46(x)  VTSS_EXTRACT_BITFIELD(x,6,12)

/** 
 * \brief
 * Manchester decode option
 * This option sets the permissiveness of the decode of the Frame Marker
 * and Manchester bits by the Training Frame Decode logic.
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_B_REG_MODE_B . MAN_DEC
 */
#define  VTSS_F_XFI_AE_REGS_B_REG_MODE_B_MAN_DEC(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_XFI_AE_REGS_B_REG_MODE_B_MAN_DEC     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_XFI_AE_REGS_B_REG_MODE_B_MAN_DEC(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * Manual ready
 * This option determines whether hardware or microcode updates the
 * TXSTAT_RDY bit in the TX_COEF_STAT register
 *
 * \details 
 * 1: TXSTAT_RDY bit is R/W; hardware update of this bit is disabled.
 * 0: TXSTAT_RDY bit value is determined by Training FSM; register write is
 * disabled.
 *
 * Field: ::VTSS_XFI_AE_REGS_B_REG_MODE_B . MANUAL_RDY
 */
#define  VTSS_F_XFI_AE_REGS_B_REG_MODE_B_MANUAL_RDY(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_AE_REGS_B_REG_MODE_B_MANUAL_RDY     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_AE_REGS_B_REG_MODE_B_MANUAL_RDY(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Maximum wait timer disable
 * Allows microcode to disable the max_wait_timer. If this timer is
 * disabled, then the Training FSM time out transition to TRAINING_FAIL
 * state cannot occur.
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_B_REG_MODE_B . MWT_DISABLE
 */
#define  VTSS_F_XFI_AE_REGS_B_REG_MODE_B_MWT_DISABLE(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_AE_REGS_B_REG_MODE_B_MWT_DISABLE     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_AE_REGS_B_REG_MODE_B_MWT_DISABLE(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * MDIO override
 * Allows microcode to disable MDIO control
 *
 * \details 
 * 1: MDIO controls are disabled.
 * 0: Normal operation. (MDIO controls override microcode.)
 *
 * Field: ::VTSS_XFI_AE_REGS_B_REG_MODE_B . MDIO_OVR
 */
#define  VTSS_F_XFI_AE_REGS_B_REG_MODE_B_MDIO_OVR(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_AE_REGS_B_REG_MODE_B_MDIO_OVR     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_AE_REGS_B_REG_MODE_B_MDIO_OVR(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Sticky mode (diagnostics only)
 * Alters operation of the TX_COEF_REQ and RX_COEF_STAT registers. 
 * When Sticky Mode = 0 these registers always contain contents of the most
 * recently received training frame. 
 * When Sticky Mode = 1 register values are OR'd with prior contents and
 * registers must be written to reset. 
 * Note: This may result in incorrect interpretation of some register bit
 * fields and therefore this mode should only be used for diagnostics
 * purposes.
 *
 * \details 
 * 1: Sticky mode enabled
 * 0: Sticky mode disable
 *
 * Field: ::VTSS_XFI_AE_REGS_B_REG_MODE_B . STICKY_MODE
 */
#define  VTSS_F_XFI_AE_REGS_B_REG_MODE_B_STICKY_MODE(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_AE_REGS_B_REG_MODE_B_STICKY_MODE     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_AE_REGS_B_REG_MODE_B_STICKY_MODE(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief PRBS control register
 *
 * \details
 * When adaptive equalization is active, this register controls the PRBS
 * generation in the transmitted training frame and controls the PRBS
 * checker which checks the received training frame. PRBS checker status is
 * also accessed in this register.
 *
 * Register: \a XFI::AE_REGS_B::PRBS_CTL_B
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_AE_REGS_B_PRBS_CTL_B(target)  VTSS_IOREG(target,0x205)

/** 
 * \brief
 * PRBS checker error count
 * PRBS checker status which indicates the number of PRBS errors detected.
 * This field is reset by the 0->1 transition of the PRBS_CHK_RST bit, and
 * increments for each DWORD containing one or more bit errors. If the
 * value reaches 0xFF, the field does not increment past this maximum
 * value.
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_B_PRBS_CTL_B . PRBS_CHK_ERRCNT
 */
#define  VTSS_F_XFI_AE_REGS_B_PRBS_CTL_B_PRBS_CHK_ERRCNT(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_XFI_AE_REGS_B_PRBS_CTL_B_PRBS_CHK_ERRCNT     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_XFI_AE_REGS_B_PRBS_CTL_B_PRBS_CHK_ERRCNT(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * PRBS unsynchronized frame counter
 * Counts the number of training frames for which the PRBS Checker never
 * synchronized to the PRBS data pattern. This counter is reset by
 * RESET_ASYNC, 0->1 transition of the PRBS_CHK_RST, and PRBS_CHK_OFF. Once
 * the maximum counter value is reached, value of 111 is held until counter
 * is reset.
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_B_PRBS_CTL_B . PRBS_SYNCCNT
 */
#define  VTSS_F_XFI_AE_REGS_B_PRBS_CTL_B_PRBS_SYNCCNT(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_XFI_AE_REGS_B_PRBS_CTL_B_PRBS_SYNCCNT     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_XFI_AE_REGS_B_PRBS_CTL_B_PRBS_SYNCCNT(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * PRBS checker sync
 * PRBS checker status which indicates the PRBS checker is initialized and
 * is actively checking PRBS bits in received training frame
 *
 * \details 
 * 1: PRBS checker is synchronized
 * 0: PRBS checker is not synchronized
 *
 * Field: ::VTSS_XFI_AE_REGS_B_PRBS_CTL_B . PRBS_CHK_SYNC
 */
#define  VTSS_F_XFI_AE_REGS_B_PRBS_CTL_B_PRBS_CHK_SYNC(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_AE_REGS_B_PRBS_CTL_B_PRBS_CHK_SYNC     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_AE_REGS_B_PRBS_CTL_B_PRBS_CHK_SYNC(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * PRBS checker reset
 * Controls PRBS checker that checks PRBS bits in received training frame.
 * Reset is generated after 0->1 transition of this bit. Bit must be reset
 * by microcode before another reset can be generated.
 *
 * \details 
 * 1: PRBS checker is reset
 * 0: Normal operation
 *
 * Field: ::VTSS_XFI_AE_REGS_B_PRBS_CTL_B . PRBS_CHK_RST
 */
#define  VTSS_F_XFI_AE_REGS_B_PRBS_CTL_B_PRBS_CHK_RST(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_AE_REGS_B_PRBS_CTL_B_PRBS_CHK_RST     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_AE_REGS_B_PRBS_CTL_B_PRBS_CHK_RST(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * PRBS checker disable
 * Controls PRBS checker that checks PRBS bits in received training frame
 *
 * \details 
 * 1: PRBS checker disabled
 * 0: Normal operation (PRBS checker enabled)
 *
 * Field: ::VTSS_XFI_AE_REGS_B_PRBS_CTL_B . PRBS_CHK_OFF
 */
#define  VTSS_F_XFI_AE_REGS_B_PRBS_CTL_B_PRBS_CHK_OFF(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_AE_REGS_B_PRBS_CTL_B_PRBS_CHK_OFF     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_AE_REGS_B_PRBS_CTL_B_PRBS_CHK_OFF(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * PRBS generator force error
 * Controls PRBS generator that generates PRBS bits in transmitted training
 * frame. Error is generated after 0->1 transition of this bit. Bit must be
 * reset by microcode before another error can be generated.
 *
 * \details 
 * 1: Bit error is inserted in transmitted training frame
 * 0: Normal operation
 *
 * Field: ::VTSS_XFI_AE_REGS_B_PRBS_CTL_B . PRBS_GEN_FRCERR
 */
#define  VTSS_F_XFI_AE_REGS_B_PRBS_CTL_B_PRBS_GEN_FRCERR(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_AE_REGS_B_PRBS_CTL_B_PRBS_GEN_FRCERR     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_AE_REGS_B_PRBS_CTL_B_PRBS_GEN_FRCERR(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * PRBS generator disable
 * Controls PRBS generator that generates PRBS bits in transmitted training
 * frame
 *
 * \details 
 * 1: PRBS generator disabled
 * 0: Normal operation (PRBS generator enabled)
 *
 * Field: ::VTSS_XFI_AE_REGS_B_PRBS_CTL_B . PRBS_GEN_OFF
 */
#define  VTSS_F_XFI_AE_REGS_B_PRBS_CTL_B_PRBS_GEN_OFF(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_AE_REGS_B_PRBS_CTL_B_PRBS_GEN_OFF     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_AE_REGS_B_PRBS_CTL_B_PRBS_GEN_OFF(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Training state machine register
 *
 * \details
 * This register controls the training state machine and provides state
 * machine status information.
 * The MC_RX_RDY bit is used to handshake between the Training FSM and
 * microcode. This bit is reset when the Training FSM is in any state other
 * than TRAIN_LOCAL or TRAIN_REMOTE state. Microcode trains the local node
 * as appropriate while the Training FSM is in the TRAIN_LOCAL state. Upon
 * completion of this training algorithm, microcode writes this register to
 * set the MC_RX_RDY bit, which causes the Training FSM to transition to
 * the TRAIN_REMOTE state.
 *
 * Register: \a XFI::AE_REGS_B::FSM_CTL_B
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_AE_REGS_B_FSM_CTL_B(target)  VTSS_IOREG(target,0x206)

/** 
 * \brief
 * Reserved
 * Always write to 0
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_B_FSM_CTL_B . Reserved_AE615
 */
#define  VTSS_F_XFI_AE_REGS_B_FSM_CTL_B_Reserved_AE615(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_XFI_AE_REGS_B_FSM_CTL_B_Reserved_AE615     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_XFI_AE_REGS_B_FSM_CTL_B_Reserved_AE615(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * FSM train local state
 * Training FSM is in TRAIN_LOCAL state. Microcode must execute the
 * training algorithm in this state.
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_B_FSM_CTL_B . FSM_TR_LCL
 */
#define  VTSS_F_XFI_AE_REGS_B_FSM_CTL_B_FSM_TR_LCL(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_XFI_AE_REGS_B_FSM_CTL_B_FSM_TR_LCL     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_XFI_AE_REGS_B_FSM_CTL_B_FSM_TR_LCL(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Good frame markers
 * FSM count of good frame markers received by Frame Lock state machine. 
 * Note: Upper limit of count range is limited by the IEEE 802.3ap
 * description of this counter.
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_B_FSM_CTL_B . FSM_GDMRK
 */
#define  VTSS_F_XFI_AE_REGS_B_FSM_CTL_B_FSM_GDMRK(x)  VTSS_ENCODE_BITFIELD(x,11,3)
#define  VTSS_M_XFI_AE_REGS_B_FSM_CTL_B_FSM_GDMRK     VTSS_ENCODE_BITMASK(11,3)
#define  VTSS_X_XFI_AE_REGS_B_FSM_CTL_B_FSM_GDMRK(x)  VTSS_EXTRACT_BITFIELD(x,11,3)

/** 
 * \brief
 * FSM count of bad frame markers received by Frame Lock state machine. 
 * Note: Upper limit of count range is limited by the IEEE 802.3ap
 * description of this counter. (The value of this counter in the AE core
 * is in the range of 0 to 4. Detection of the 5th bad marker causes an FSM
 * state transition, which resets this counter.)
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_B_FSM_CTL_B . FSM_BADMRK
 */
#define  VTSS_F_XFI_AE_REGS_B_FSM_CTL_B_FSM_BADMRK(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_XFI_AE_REGS_B_FSM_CTL_B_FSM_BADMRK     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_XFI_AE_REGS_B_FSM_CTL_B_FSM_BADMRK(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * Training failure
 * Training process failed due to expiration of the max_wait_timer while
 * training was in progress. Bit is reset if training restarts.
 *
 * \details 
 * 1: Training Failed
 * 0: Training did not fail
 *
 * Field: ::VTSS_XFI_AE_REGS_B_FSM_CTL_B . FSM_TR_FAIL
 */
#define  VTSS_F_XFI_AE_REGS_B_FSM_CTL_B_FSM_TR_FAIL(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_XFI_AE_REGS_B_FSM_CTL_B_FSM_TR_FAIL     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_XFI_AE_REGS_B_FSM_CTL_B_FSM_TR_FAIL(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Training active
 * FSM state indicates training protocol is in process
 *
 * \details 
 * 1: Training is in progress
 * 0: Training is not in progress
 *
 * Field: ::VTSS_XFI_AE_REGS_B_FSM_CTL_B . FSM_TR_ACT
 */
#define  VTSS_F_XFI_AE_REGS_B_FSM_CTL_B_FSM_TR_ACT(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_XFI_AE_REGS_B_FSM_CTL_B_FSM_TR_ACT     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_XFI_AE_REGS_B_FSM_CTL_B_FSM_TR_ACT(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Frame lock
 * Receiver acquires training frame delineation and FSM is receiving valid
 * training frames
 *
 * \details 
 * 1: Frame lock achieved
 * 0: Frame lock not achieved
 *
 * Field: ::VTSS_XFI_AE_REGS_B_FSM_CTL_B . FSM_FRM_LCK
 */
#define  VTSS_F_XFI_AE_REGS_B_FSM_CTL_B_FSM_FRM_LCK(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_XFI_AE_REGS_B_FSM_CTL_B_FSM_FRM_LCK     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_XFI_AE_REGS_B_FSM_CTL_B_FSM_FRM_LCK(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Receiver training complete
 * Indicates the remote transmit and local receiver equalizers have been
 * optimized and normal data transmission may commence. Bit is reset if
 * training restarts.
 *
 * \details 
 * 1: Training complete
 * 0: Training not complete
 *
 * Field: ::VTSS_XFI_AE_REGS_B_FSM_CTL_B . FSM_TR_COMP
 */
#define  VTSS_F_XFI_AE_REGS_B_FSM_CTL_B_FSM_TR_COMP(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_AE_REGS_B_FSM_CTL_B_FSM_TR_COMP     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_AE_REGS_B_FSM_CTL_B_FSM_TR_COMP(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Held reset if the Training FSM state is not TRAIN_LOCAL or TRAIN_REMOTE.
 * When training is active, microcode sets this bit when the local Rx is
 * trained.
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_B_FSM_CTL_B . MC_RX_RDY
 */
#define  VTSS_F_XFI_AE_REGS_B_FSM_CTL_B_MC_RX_RDY(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_AE_REGS_B_FSM_CTL_B_MC_RX_RDY     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_AE_REGS_B_FSM_CTL_B_MC_RX_RDY(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Coefficient update FSM disable
 * Disable Coefficient Update FSM and execute coefficient updates using
 * microcode
 *
 * \details 
 * 1: Coefficient Update FSM is disabled
 * 0: Normal operation (Coefficient Update FSM is enabled)
 *
 * Field: ::VTSS_XFI_AE_REGS_B_FSM_CTL_B . FSM_CU_DIS
 */
#define  VTSS_F_XFI_AE_REGS_B_FSM_CTL_B_FSM_CU_DIS(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_AE_REGS_B_FSM_CTL_B_FSM_CU_DIS     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_AE_REGS_B_FSM_CTL_B_FSM_CU_DIS(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Training restart
 * FSM mr_restart_training control. Restart is generated after 0->1
 * transition of this bit. Bit must be reset by microcode before another
 * restart command can be generated. MDIO interface can also trigger a
 * restart unless MDIO_OVR bit is set in REG_MODE register.
 *
 * \details 
 * 1: Reset the training state machine and restart training
 * 0: Normal operation
 *
 * Field: ::VTSS_XFI_AE_REGS_B_FSM_CTL_B . FSM_TR_RST
 */
#define  VTSS_F_XFI_AE_REGS_B_FSM_CTL_B_FSM_TR_RST(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_AE_REGS_B_FSM_CTL_B_FSM_TR_RST     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_AE_REGS_B_FSM_CTL_B_FSM_TR_RST(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Training enable
 * FSM mr_training_enable control. MDIO interface can also enable training
 * unless MDIO_OVR bit is set in REG_MODE register.
 *
 * \details 
 * 1: Force training to be enabled
 * 0: MDIO controls whether training is enabled
 *
 * Field: ::VTSS_XFI_AE_REGS_B_FSM_CTL_B . FSM_TR_EN
 */
#define  VTSS_F_XFI_AE_REGS_B_FSM_CTL_B_FSM_TR_EN(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_AE_REGS_B_FSM_CTL_B_FSM_TR_EN     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_AE_REGS_B_FSM_CTL_B_FSM_TR_EN(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief FSM state register
 *
 * \details
 * This status register provides diagnostic status to the current FSM
 * states.
 *
 * Register: \a XFI::AE_REGS_B::FSM_STATE_B
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_AE_REGS_B_FSM_STATE_B(target)  VTSS_IOREG(target,0x207)

/** 
 * \brief
 * Coefficient update FSM state
 * Current state of the coefficient update FSM for the C2 coefficient
 *
 * \details 
 * 001 NOTUPDATED: Idle
 * 010 UPDATEIF: set Cx_Update to serdes and wait for response on Cx_ACK
 * 011 WRACK: Forward serdes response to link partner
 * 100 ACKED: Wait for serdes to set Cx_ACK = not updated
 * 101 WAIT: Wait for link partner to set TXREQ_Cx request = hold
 * 110 WRUPDATE: Send TXSTAT_Cx = not updated
 *
 * Field: ::VTSS_XFI_AE_REGS_B_FSM_STATE_B . CCFSM_STATE_C2
 */
#define  VTSS_F_XFI_AE_REGS_B_FSM_STATE_B_CCFSM_STATE_C2(x)  VTSS_ENCODE_BITFIELD(x,13,3)
#define  VTSS_M_XFI_AE_REGS_B_FSM_STATE_B_CCFSM_STATE_C2     VTSS_ENCODE_BITMASK(13,3)
#define  VTSS_X_XFI_AE_REGS_B_FSM_STATE_B_CCFSM_STATE_C2(x)  VTSS_EXTRACT_BITFIELD(x,13,3)

/** 
 * \brief
 * Coefficient update FSM state
 * Current state of the coefficient update FSM for the C1 coefficient
 *
 * \details 
 * 001 NOTUPDATED: Idle
 * 010 UPDATEIF: Set Cx_Update to serdes and wait for response on Cx_ACK
 * 011 WRACK: Forward serdes response to link partner
 * 100 ACKED: Wait for serdes to set Cx_ACK = not updated
 * 101 WAIT: Wait for link partner to set TXREQ_Cx request = hold
 * 110 WRUPDATE: Send TXSTAT_Cx = not updated
 *
 * Field: ::VTSS_XFI_AE_REGS_B_FSM_STATE_B . CCFSM_STATE_C1
 */
#define  VTSS_F_XFI_AE_REGS_B_FSM_STATE_B_CCFSM_STATE_C1(x)  VTSS_ENCODE_BITFIELD(x,10,3)
#define  VTSS_M_XFI_AE_REGS_B_FSM_STATE_B_CCFSM_STATE_C1     VTSS_ENCODE_BITMASK(10,3)
#define  VTSS_X_XFI_AE_REGS_B_FSM_STATE_B_CCFSM_STATE_C1(x)  VTSS_EXTRACT_BITFIELD(x,10,3)

/** 
 * \brief
 * Coefficient update FSM state
 * Current state of the coefficient update FSM for the C0 coefficient
 *
 * \details 
 * 001 NOTUPDATED: Idle
 * 010 UPDATEIF: Set Cx_Update to serdes and wait for response on Cx_ACK
 * 011 WRACK: Forward serdes response to link partner
 * 100 ACKED: Wait for serdes to set Cx_ACK = not updated
 * 101 WAIT: Wait for link partner to set TXREQ_Cx request = hold
 * 110 WRUPDATE: Send TXSTAT_Cx = not updated
 *
 * Field: ::VTSS_XFI_AE_REGS_B_FSM_STATE_B . CCFSM_STATE_C0
 */
#define  VTSS_F_XFI_AE_REGS_B_FSM_STATE_B_CCFSM_STATE_C0(x)  VTSS_ENCODE_BITFIELD(x,7,3)
#define  VTSS_M_XFI_AE_REGS_B_FSM_STATE_B_CCFSM_STATE_C0     VTSS_ENCODE_BITMASK(7,3)
#define  VTSS_X_XFI_AE_REGS_B_FSM_STATE_B_CCFSM_STATE_C0(x)  VTSS_EXTRACT_BITFIELD(x,7,3)

/** 
 * \brief
 * Frame lock FSM state
 * Current state of the frame lock FSM
 *
 * \details 
 * 001 IDLE
 * 010 SEEK: Look for frame marker
 * 011 WAIT: Wait 136 cycles
 * 100 CHECK: Check whether marker is valid on current cycle
 * 111 INVALID: State decode is invalid. Try again.
 *
 * Field: ::VTSS_XFI_AE_REGS_B_FSM_STATE_B . FLFSM_STATE
 */
#define  VTSS_F_XFI_AE_REGS_B_FSM_STATE_B_FLFSM_STATE(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_XFI_AE_REGS_B_FSM_STATE_B_FLFSM_STATE     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_XFI_AE_REGS_B_FSM_STATE_B_FLFSM_STATE(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * Reserved
 * Always 0
 *
 * \details 
 * Field: ::VTSS_XFI_AE_REGS_B_FSM_STATE_B . Reserved_AE73
 */
#define  VTSS_F_XFI_AE_REGS_B_FSM_STATE_B_Reserved_AE73(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_AE_REGS_B_FSM_STATE_B_Reserved_AE73     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_AE_REGS_B_FSM_STATE_B_Reserved_AE73(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Training FSM state
 * Current state of the training FSM
 *
 * \details 
 * 001 INITIALIZE
 * 010 SEND_TRAINING
 * 011 TRAIN_LOCAL
 * 100 TRAIN_REMOTE
 * 101 LINK_READY
 * 110 SEND_DATA
 * 111 TRAINING_FAILURE
 *
 * Field: ::VTSS_XFI_AE_REGS_B_FSM_STATE_B . TFSM_STATE
 */
#define  VTSS_F_XFI_AE_REGS_B_FSM_STATE_B_TFSM_STATE(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_XFI_AE_REGS_B_FSM_STATE_B_TFSM_STATE     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_XFI_AE_REGS_B_FSM_STATE_B_TFSM_STATE(x)  VTSS_EXTRACT_BITFIELD(x,0,3)

/**
 * Register Group: \a XFI::HSS11G_HS_PLL
 *
 *  XFI PLL registers
 */


/** 
 * \brief HSS11G HS_PLL VCO coarse calibration 0
 *
 * \details
 * This register is reserved for diagnostic and test purposes only
 *
 * Register: \a XFI::HSS11G_HS_PLL::HSS11G_VCO_COARSE_CALIBRATION_0
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_0(target)  VTSS_IOREG(target,0x400)

/** 
 * \brief
 * Band select out 
 * Data from the coarse calibration band select counter
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_0 . BSELO
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_0_BSELO(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_0_BSELO     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_0_BSELO(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief HSS11G HS_PLL VCO coarse calibration 1
 *
 * \details
 * This register is reserved for diagnostic and test purposes only
 *
 * Register: \a XFI::HSS11G_HS_PLL::HSS11G_VCO_COARSE_CALIBRATION_1
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_1(target)  VTSS_IOREG(target,0x401)

/** 
 * \brief
 * Lock detected
 * Provides a general indication of whether the HS PLL is locked to the
 * incoming REFCLK or not. It is primarily intended as a test and
 * diagnostic tool.
 *
 * \details 
 * 0: HS PLL not locked to incoming REFCLK (default)
 * 1: HS PLL lock detected.
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_1 . LDET
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_1_LDET(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_1_LDET     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_1_LDET(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Coarse calibration error
 *
 * \details 
 * 0: Coarse calibration completed without error (default)
 * 1: Coarse calibration error occurred
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_1 . CCERR
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_1_CCERR(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_1_CCERR     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_1_CCERR(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Coarse calibration complete
 *
 * \details 
 * 0: Coarse calibration not complete (default)
 * 1: Coarse calibration has completed
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_1 . CCCMP
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_1_CCCMP(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_1_CCCMP     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_1_CCCMP(x)  VTSS_EXTRACT_BITFIELD(x,2,1)


/** 
 * \brief HSS11G HS_PLL VCO coarse calibration 2
 *
 * \details
 * This register is reserved for diagnostic and test purposes. (This
 * register can be validated that it is read/write, but it must be set
 * properly and the core must be reset before the transmitter or receiver
 * can be used.)
 *
 * Register: \a XFI::HSS11G_HS_PLL::HSS11G_VCO_COARSE_CALIBRATION_2
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_2(target)  VTSS_IOREG(target,0x402)

/** 
 * \brief
 * Band select in 
 * Coarse calibration data to be loaded into the band select counter. This
 * value is loaded to the HS PLL by the coarse cal load bit (bit 0) of VCO
 * coarse calibration register 4.
 *
 * \details 
 * 0000 (default)
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_2 . BSELI
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_2_BSELI(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_2_BSELI     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_2_BSELI(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief HSS11G HS_PLL VCO coarse calibration 3
 *
 * \details
 * This register is reserved for diagnostic and test purposes. (This
 * register can be validated that it is read /write, but it must be set
 * properly and the core must be reset before the transmitter or receiver
 * can be used.)
 *
 * Register: \a XFI::HSS11G_HS_PLL::HSS11G_VCO_COARSE_CALIBRATION_3
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_3(target)  VTSS_IOREG(target,0x403)

/** 
 * \brief
 * VCO current select
 * This value is loaded to the HS PLL by the coarse cal load bit (bit 0) of
 * VCO coarse calibration register 4.
 *
 * \details 
 * 0: Normal (default)
 * 1: The current to the VCO will be increased
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_3 . VISEL
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_3_VISEL(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_3_VISEL     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_3_VISEL(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Coarse calibration force minimum 
 * VCO will oscillate at the lowest frequency for the selected band. FMAX
 * (bit 2) and CVHOLD (bit 1) must be 0 before asserting this bit. This
 * value is loaded to the HS PLL by the coarse cal load bit (bit 0) of VCO
 * coarse calibration register 4.
 *
 * \details 
 * 0: Normal (default)
 * 1: Force VCO control voltage to minimum
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_3 . FMIN
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_3_FMIN(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_3_FMIN     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_3_FMIN(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Coarse calibration force maximum
 * VCO will oscillate at the highest frequency for the selected band. FMIN
 * (bit 3) and CVHOLD (bit 1) must be 0 before asserting this bit. This
 * value is loaded to the HS PLL by the coarse cal load bit (bit 0) of VCO
 * coarse calibration register 4.
 *
 * \details 
 * 0: Normal (default)
 * 1: Force VCO control voltage to maximum
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_3 . FMAX
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_3_FMAX(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_3_FMAX     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_3_FMAX(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Coarse calibration control voltage hold
 * VCO will oscillate at center frequency of selected band. FMAX (bit 3)
 * and FMIN (bit 2) should be 0 before asserting this bit. This value is
 * loaded to the HS PLL by the coarse cal load bit (bit 0) of VCO coarse
 * calibration register 4.
 *
 * \details 
 * 0: Normal (default)
 * 1: Force VCO control voltage to nominal value
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_3 . CVHOLD
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_3_CVHOLD(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_3_CVHOLD     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_3_CVHOLD(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Temperature compensation disable
 * This value is loaded to the HS PLL by the coarse cal load bit (bit 0) of
 * VCO coarse calibration register 4.
 *
 * \details 
 * 0: Enable temperature compensation (default)
 * 1: Disable temperature compensation
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_3 . TCDIS
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_3_TCDIS(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_3_TCDIS     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_3_TCDIS(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief HSS11G HS_PLL VCO coarse calibration 4
 *
 * \details
 * This register is primarily used for diagnostic and test purposes.
 * However, it can be used to initiate a coarse calibration of the HS PLL
 * after an HSSREFCLK frequency change without resetting the entire core.
 * Contact your Vitesse representative before changing the other bits in
 * this register.
 *
 * Register: \a XFI::HSS11G_HS_PLL::HSS11G_VCO_COARSE_CALIBRATION_4
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_4(target)  VTSS_IOREG(target,0x404)

/** 
 * \brief
 * Coarse calibration method
 *
 * \details 
 * 0: Enable deltaF2 measurement using FMAX and FMIN method. (default)
 * 1: Enable deltaF2 measurement using CVHOLD method
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_4 . CMETH
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_4_CMETH(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_4_CMETH     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_4_CMETH(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Recalibrate pulse high for a minimum of eight REFCLK cycles, then return
 * low to initiate PLL recalibration.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_4 . RECAL
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_4_RECAL(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_4_RECAL     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_4_RECAL(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Coarse calibration load
 *
 * \details 
 * 0: Normal (default)
 * 1: Transfer the 4-bit VCO coarse calibration data (register 0x02, bits
 * 3:0) to the 4-bit calibration counter driving the VCO varactor
 * selection. Also loads VISEL, FMAX, FMIN, and CVHOLD to the VCO.
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_4 . CCLD
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_4_CCLD(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_4_CCLD     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_VCO_COARSE_CALIBRATION_4_CCLD(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief HSS11G HS_PLL port enable 0
 *
 * \details
 * The port enable registers are used to enable and disable clocks and
 * power to the individual ports within the core. All bits exist as
 * read/write although not all are needed for the 2-port full duplex cores
 * (N = 2). The bits reset to a 1 which is the enabled state. Disabling a
 * port may take up to 150 sec to complete after writing a bit to 0. When
 * re-enabling a port (setting a bit to 1), wait at least 200 nsec before
 * accessing registers in that port.
 *
 * Register: \a XFI::HSS11G_HS_PLL::HSS11G_PORT_ENABLE_0
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_HSS11G_HS_PLL_HSS11G_PORT_ENABLE_0(target)  VTSS_IOREG(target,0x406)

/** 
 * \brief
 * RXB port enable
 *
 * \details 
 * 0: Disable port
 * 1: Enable port (default)
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_PORT_ENABLE_0 . RXBEN
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_PORT_ENABLE_0_RXBEN(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_PORT_ENABLE_0_RXBEN     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_PORT_ENABLE_0_RXBEN(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * RXA port enable
 *
 * \details 
 * 0: Disable port
 * 1: Enable port (default)
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_PORT_ENABLE_0 . RXAEN
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_PORT_ENABLE_0_RXAEN(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_PORT_ENABLE_0_RXAEN     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_PORT_ENABLE_0_RXAEN(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * TXB port enable
 *
 * \details 
 * 0: Disable port
 * 1: Enable port (default)
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_PORT_ENABLE_0 . TXBEN
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_PORT_ENABLE_0_TXBEN(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_PORT_ENABLE_0_TXBEN     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_PORT_ENABLE_0_TXBEN(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * TXA port enable
 *
 * \details 
 * 0: Disable port
 * 1: Enable port (default)
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_PORT_ENABLE_0 . TXAEN
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_PORT_ENABLE_0_TXAEN(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_PORT_ENABLE_0_TXAEN     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_PORT_ENABLE_0_TXAEN(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief HSS11G HS_PLL port reset 0
 *
 * \details
 * The port reset registers are used to reset the individual ports within
 * the core. This is also referred to as link reset. All bits exist as
 * read/write although not all are needed for the 2-port full duplex cores
 * (N = 2). The bits reset to 0. Write a bit to 1 for at least 100 nsec,
 * then return it to 0 to cause a reset. The link reset may take up to 100
 * ns after the end of the register write to 0 to take effect on the target
 * link. Do not access other registers in the target link during this time.
 *
 * Register: \a XFI::HSS11G_HS_PLL::HSS11G_PORT_RESET_0
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_HSS11G_HS_PLL_HSS11G_PORT_RESET_0(target)  VTSS_IOREG(target,0x408)

/** 
 * \brief
 * RXB port reset
 *
 * \details 
 * 0: Normal (default)
 * 1: Reset receiver port B
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_PORT_RESET_0 . RXBRST
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_PORT_RESET_0_RXBRST(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_PORT_RESET_0_RXBRST     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_PORT_RESET_0_RXBRST(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * RXA port reset
 *
 * \details 
 * 0: Normal (default)
 * 1: Reset receiver port A
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_PORT_RESET_0 . RXARST
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_PORT_RESET_0_RXARST(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_PORT_RESET_0_RXARST     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_PORT_RESET_0_RXARST(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * TXB port reset
 *
 * \details 
 * 0: Normal (default)
 * 1: Reset transmitter port B
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_PORT_RESET_0 . TXBRST
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_PORT_RESET_0_TXBRST(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_PORT_RESET_0_TXBRST     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_PORT_RESET_0_TXBRST(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * TXA port reset
 *
 * \details 
 * 0: Normal (default)
 * 1: Reset transmitter port A
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_PORT_RESET_0 . TXARST
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_PORT_RESET_0_TXARST(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_PORT_RESET_0_TXARST     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_PORT_RESET_0_TXARST(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief HSS11G HS_PLL charge pump control
 *
 * \details
 * This register is reserved for diagnostic and test purposes. (This
 * register can be validated that it is read/write, but it must be set
 * properly and the core must be reset before the transmitter or receiver
 * can be used.)
 *
 * Register: \a XFI::HSS11G_HS_PLL::HSS11G_CHARGE_PUMP_CONTROL
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_HSS11G_HS_PLL_HSS11G_CHARGE_PUMP_CONTROL(target)  VTSS_IOREG(target,0x40a)

/** 
 * \brief
 * Enable charge pump current selects
 *
 * \details 
 * 0: Normal operation (default)
 * 1: Enable charge pump current selects (CPISEL)
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_CHARGE_PUMP_CONTROL . ENCPIS
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_CHARGE_PUMP_CONTROL_ENCPIS(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_CHARGE_PUMP_CONTROL_ENCPIS     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_CHARGE_PUMP_CONTROL_ENCPIS(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Charge pump current selects 
 * 2-bit charge pump current selects. These values are only active if
 * ENCPIS is set to 1.
 *
 * \details 
 * 00: 60 ua (default)
 * 01: 75 ua
 * 10: 120 ua
 * 11: 150 ua
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_CHARGE_PUMP_CONTROL . CPISEL
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_CHARGE_PUMP_CONTROL_CPISEL(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_CHARGE_PUMP_CONTROL_CPISEL     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_CHARGE_PUMP_CONTROL_CPISEL(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief HSS11G HS_PLL bandgap control
 *
 * \details
 * This register is reserved for diagnostic and test purposes. (This
 * register can be validated that it is read/write, but it must be set
 * properly and the core must be reset before the transmitter or receiver
 * can be used.)
 *
 * Register: \a XFI::HSS11G_HS_PLL::HSS11G_BANDGAP_CONTROL
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_HSS11G_HS_PLL_HSS11G_BANDGAP_CONTROL(target)  VTSS_IOREG(target,0x40b)

/** 
 * \brief
 * Band gap control 
 * 5-bit controls for PLL bandgap reference voltage offset. 
 * Note: Only 1 bit is allowed to be 1.
 *
 * \details 
 * 0 0000: 0 mV
 * 0 0001: 150 mV
 * 0 0010: 200 mV (default)
 * 0 0100: 250 mV
 * 0 1000: 300 mV
 * 1 0000: 350 mV
 * All other bit combinations are not supported
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_BANDGAP_CONTROL . BGCTL
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_BANDGAP_CONTROL_BGCTL(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_BANDGAP_CONTROL_BGCTL     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_BANDGAP_CONTROL_BGCTL(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief HSS11G HS_PLL low frequency override
 *
 * \details
 * This register can be validated that it is read/write, but it must be set
 * properly and the core must be reset before the transmitter or receiver
 * can be used.
 *
 * Register: \a XFI::HSS11G_HS_PLL::HSS11G_LOFREQ_OVERRIDE
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_HSS11G_HS_PLL_HSS11G_LOFREQ_OVERRIDE(target)  VTSS_IOREG(target,0x40c)

/** 
 * \brief
 * Low power configuration bit 2
 * Used to reduce power of analog clock buffers if 2 or 4 port full duplex
 * core
 *
 * \details 
 * 0: Full buffer power (default if N = 8 and RSTCONFIG2 = 0 (default))
 * 1: Low buffer power (default if N = 2 or N = 4 and RSTCONFIG2 = 0
 * (default))
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_LOFREQ_OVERRIDE . LOPWR2
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_LOFREQ_OVERRIDE_LOPWR2(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_LOFREQ_OVERRIDE_LOPWR2     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_LOFREQ_OVERRIDE_LOPWR2(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Low power configuration bit 1
 * Can be used to reduce core power if the operating rate is below 9 Gbps. 
 * Note: This bit only resets to 0 when HSSRSTCONFIG2 = 0 (default)
 *
 * \details 
 * 0: Normal operation (default)
 * 1: Low power mode 1 (for use below 9 Gbps)
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_LOFREQ_OVERRIDE . LOPWR1
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_LOFREQ_OVERRIDE_LOPWR1(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_LOFREQ_OVERRIDE_LOPWR1     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_LOFREQ_OVERRIDE_LOPWR1(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Low frequency override
 * Note: An HSSRESYNC should be issued after changing the frequency range.
 * This bit only resets to 0 when HSSRSTCONFIG2=0 (default).
 *
 * \details 
 * 0: Normal operation (default)
 * 1: Bypass the HSSLOFREQ pin input signal and use the LFSEL (bit 0) value
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_LOFREQ_OVERRIDE . LFREQO
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_LOFREQ_OVERRIDE_LFREQO(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_LOFREQ_OVERRIDE_LFREQO     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_LOFREQ_OVERRIDE_LFREQO(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Low frequency range select
 * Note: An HSSRESYNC should be issued after changing the frequency range.
 * This bit only resets to 0 when HSSRSTCONFIG2=0 (default)
 *
 * \details 
 * 0: Full PLL output rate operation (default)
 * 1: 2/3 PLL output rate operation
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_LOFREQ_OVERRIDE . LFSEL
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_LOFREQ_OVERRIDE_LFSEL(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_LOFREQ_OVERRIDE_LFSEL     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_LOFREQ_OVERRIDE_LFSEL(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief HSS11G HS_PLL voltage boost control
 *
 * \details
 * This register is reserved for diagnostic and test purposes. (This
 * register can be validated that it is read/write, but it must be set
 * properly and the core must be reset before the transmitter or receiver
 * can be used.)
 *
 * Register: \a XFI::HSS11G_HS_PLL::HSS11G_VOLTAGE_BOOST_CONTROL
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_HSS11G_HS_PLL_HSS11G_VOLTAGE_BOOST_CONTROL(target)  VTSS_IOREG(target,0x40e)

/** 
 * \brief
 * Charge pump frequency value
 *
 * \details 
 * 0: Divide REFCLKBST by 2 (default)
 * 1: Divide REFCLKBST by 4
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_VOLTAGE_BOOST_CONTROL . PFVAL
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_VOLTAGE_BOOST_CONTROL_PFVAL(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_VOLTAGE_BOOST_CONTROL_PFVAL     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_VOLTAGE_BOOST_CONTROL_PFVAL(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Charge pump frequency enable
 *
 * \details 
 * 0: Normal (default)
 * 1: Pump frequency (PFVAL) enabled
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_VOLTAGE_BOOST_CONTROL . PFEN
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_VOLTAGE_BOOST_CONTROL_PFEN(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_VOLTAGE_BOOST_CONTROL_PFEN     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_VOLTAGE_BOOST_CONTROL_PFEN(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Voltage boost adjust
 *
 * \details 
 * 0: Nominal (default)
 * 1: +6%
 *
 * Field: ::VTSS_XFI_HSS11G_HS_PLL_HSS11G_VOLTAGE_BOOST_CONTROL . VBADJ
 */
#define  VTSS_F_XFI_HSS11G_HS_PLL_HSS11G_VOLTAGE_BOOST_CONTROL_VBADJ(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_HSS11G_HS_PLL_HSS11G_VOLTAGE_BOOST_CONTROL_VBADJ     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_HSS11G_HS_PLL_HSS11G_VOLTAGE_BOOST_CONTROL_VBADJ(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/**
 * Register Group: \a XFI::HSS11G_TX
 *
 * XFI transmit registers
 */


/** 
 * \brief HSS11G Tx transmit configuration mode
 *
 * \details
 * Changing this register causes the frequency of the TXxDCLK to change.
 * The clock will stop for up to 150 ns, but it will not sliver or glitch.
 *
 * Register: \a XFI::HSS11G_TX::HSS11G_TRANSMIT_CONFIGURATION_MODE
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_TRANSMIT_CONFIGURATION_MODE (??), 0-3
 */
#define VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE(target,ri)  VTSS_IOREG(target,0x500 + (ri))

/** 
 * \brief
 * Data bus width select
 * Note: If word synchronization between ports is required, a RESYNC
 * operation is required after changing the value of this register, but it
 * should not be initiated until 150 ns after this register is changed.
 *
 * \details 
 * 00: 16 bit
 * 01: 20 bit
 * 10: 32 bit (default)
 * 11: 40 bit
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE . BWSEL
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_BWSEL(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Rate select
 * Note: If word synchronization between ports is required, a RESYNC
 * operation is required after changing the value of this register, but it
 * should not be initiated until 150 ns after this register is changed.
 *
 * \details 
 * 00: Full rate (default)
 * 01: Half rate
 * 10: Quarter rate
 * 11: Eighth rate (supported only in HS PLL high range)
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE . RTSEL
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_CONFIGURATION_MODE_RTSEL(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief HSS11G Tx transmit test control
 *
 * \details
 * Register: \a XFI::HSS11G_TX::HSS11G_TRANSMIT_TEST_CONTROL
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_TRANSMIT_TEST_CONTROL (??), 0-3
 */
#define VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL(target,ri)  VTSS_IOREG(target,0x504 + (ri))

/** 
 * \brief
 * Transmitter waveform distortion penalty (PRBS9) pattern
 * Note: This bit overrides the decode of bits 2:0 (TPSEL) and generates a
 * PRBS9+ pattern on the transmitter output under all rates and bus width
 * settings.
 *
 * \details 
 * 0: Normal (default)
 * 1: PRBS9+ pattern generated
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL . TWDP
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL_TWDP(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL_TWDP     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL_TWDP(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Test pattern generator reset
 *
 * \details 
 * 0: Normal (default)
 * 1: Reset applied to test pattern generator. (Hold high for at least two
 * TXxDCLK cycles.)
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL . TPGRST
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL_TPGRST(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL_TPGRST     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL_TPGRST(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Test pattern generator enable
 *
 * \details 
 * 0: Disable generator and select customer parallel data (default)
 * 1: Enable generator and select test pattern data
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL . TPGEN
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL_TPGEN(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL_TPGEN     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL_TPGEN(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Test pattern data select
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
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL . TPSEL
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL_TPSEL(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL_TPSEL     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_TEST_CONTROL_TPSEL(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief HSS11G Tx transmit coefficient control
 *
 * \details
 * Register: \a XFI::HSS11G_TX::HSS11G_TRANSMIT_COEFFICIENT_CONTROL
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_TRANSMIT_COEFFICIENT_CONTROL (??), 0-3
 */
#define VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(target,ri)  VTSS_IOREG(target,0x508 + (ri))

/** 
 * \brief
 * AE invert polarity
 * This bit allows the data out of the transmitter to be inverted by
 * flipping the internal polarity bit. It is provided to correct potential
 * P/N wiring mistakes by the customer.
 * Note: This bit is only meaningful when bit 4 (EQMODE) = 1.
 *
 * \details 
 * 0: No data inversion (default)
 * 1: Data is inverted
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL . AEINVPOL
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_AEINVPOL(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_AEINVPOL     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_AEINVPOL(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * AE source
 * This bit selects the source of adaptive equalization (AE) commands.
 * Note: This bit is only meaningful when bit 4 (EQMODE) = 1.
 *
 * \details 
 * 0: Register access will provide commands (default)
 * 1: Reserved
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL . AESOURCE
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_AESOURCE(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_AESOURCE     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_AESOURCE(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Equalization mode
 * This bit selects the equalization mode that will be used by the
 * transmitter
 *
 * \details 
 * 0: Traditional (default)
 * 1: 802p3ap mode
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL . EQMODE
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_EQMODE(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_EQMODE     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_EQMODE(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Override calculated coefficients
 * This bit when asserted bypasses the coefficient calculation logic and
 * instead applies the transmit IDAC coefficients to the analog inputs. An
 * apply load (bit 0) must be pulsed for the new values to be made
 * available to the analog block.
 * Note: This bit is only meaningful when bit 4 (EQMODE) = 0.
 *
 * \details 
 * 0: Normal (default)
 * 1: Override
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL . OCOEF
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_OCOEF(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_OCOEF     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_OCOEF(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Reset coefficient logic
 * Note: This bit is only meaningful when bit 4 (EQMODE) = 0.
 *
 * \details 
 * 0: Normal (default)
 * 1: Reset
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL . COEFRST
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_COEFRST(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_COEFRST     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_COEFRST(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Reserved
 * This bit should be left at 0
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL . SPEN
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_SPEN(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_SPEN     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_SPEN(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Apply load
 * This bit applies the register-loaded values of coefficients, power,
 * polarity and FFE mode to the coefficient recalculation logic, and
 * presents this new value to the analog circuits. For the new values to be
 * applied, the logic must see a transition from low to high on this
 * signal. It is recommended that, after the bit is set, this bit is
 * returned to the low level.
 * Note: This bit is only active when bit 4 (EQMODE) = 0.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL . ALOAD
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL_ALOAD(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief HSS11G Tx transmit driver mode control
 *
 * \details
 * Register: \a XFI::HSS11G_TX::HSS11G_TRANSMIT_DRIVER_MODE_CONTROL
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_TRANSMIT_DRIVER_MODE_CONTROL (??), 0-3
 */
#define VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL(target,ri)  VTSS_IOREG(target,0x50c + (ri))

/** 
 * \brief
 * Force driver output disable mode state
 *
 * \details 
 * 0: Normal (default)
 * 1: Force driver output off. TXxOP/N will be pulled up to AVTT through
 * internal termination resistors
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL . DRVOFF
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL_DRVOFF(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL_DRVOFF     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL_DRVOFF(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Slow slew control
 * Used to limit the minimum transmitter output rise and fall time. Values
 * are approximate.
 *
 * \details 
 * 000: 31 ps min. (default)
 * 101: 37 ps min
 * 011: 38 ps min
 * 111: 145 ps min
 * All other settings are not supported
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL . SLEW
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL_SLEW(x)  VTSS_ENCODE_BITFIELD(x,2,3)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL_SLEW     VTSS_ENCODE_BITMASK(2,3)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL_SLEW(x)  VTSS_EXTRACT_BITFIELD(x,2,3)

/** 
 * \brief
 * FFE mode select
 * This value becomes effective when the Apply Load bit (bit 0 of the
 * Transmit Coefficient Control register, offset 0x02) is asserted.
 *
 * \details 
 * 00: FFE2 (default)
 * 01: FFE3
 * 10: Reserved
 * 11: Reserved
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL . FFE
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL_FFE(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL_FFE     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL_FFE(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief HSS11G Tx transmit driver override control
 *
 * \details
 * Register: \a XFI::HSS11G_TX::HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL (??), 0-3
 */
#define VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL(target,ri)  VTSS_IOREG(target,0x510 + (ri))

/** 
 * \brief
 * Increment DC compensation reference
 * Note: Do not set both bit 6 and bit 7 to 1. The result is not supported.
 *
 * \details 
 * 0: Normal (default)
 * 1: Increases the DC compensation circuit reference by about 16%
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL . VLINC
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL_VLINC(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL_VLINC     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL_VLINC(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Decrement DC compensation reference
 * Note: Do not set both bit 6 and bit 7 to 1. The result is not supported.
 *
 * \details 
 * 0: Normal (default)
 * 1: Decreases the DC compensation circuit reference by about 50%
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL . VLDEC
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL_VLDEC(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL_VLDEC     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL_VLDEC(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Enable additional low power savings
 * This should only be used when the operating rate is 8.5 Gbps or less
 *
 * \details 
 * 0: Low power mode is disabled (default)
 * 1: Low power mode is enabled
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL . LOPWR
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL_LOPWR(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL_LOPWR     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL_LOPWR(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Enable the bias generator low power mode.
 * Note: It may take up to 500 nanoseconds to enter the bypass mode once
 * this bit is set to 0.
 *
 * \details 
 * 0: Bypass mode (no power savings)
 * 1: Time division multiplexing is enabled (default)
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL . TDMEN
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL_TDMEN(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL_TDMEN     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL_TDMEN(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * DC compensation enable
 *
 * \details 
 * 0: Disable DC compensation
 * 1: Enable DC compensation (default)
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL . DCCEN
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL_DCCEN(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL_DCCEN     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL_DCCEN(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Predriver bias VTT reference select
 *
 * \details 
 * 0: VTT-VDD divider reference (default)
 * 1: VTT-independent high level reference
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL . VHSEL
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL_VHSEL(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL_VHSEL     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL_VHSEL(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Increment IDAC amplitude
 * These register bits can be ignored. They are used for manufacturing test
 * only and should not be changed.
 *
 * \details 
 * 00: Nominal reference current (default)
 * 01: Nominal + 10%
 * 10: Nominal + 10%
 * 11: Nominal + 20%
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL . IDAC
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL_IDAC(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL_IDAC     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_OVERRIDE_CONTROL_IDAC(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief HSS11G Tx transmit TDM biasgen standby timer
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored.
 *
 * Register: \a XFI::HSS11G_TX::HSS11G_TRANSMIT_TDM_BIASGEN_STANDBY_TIMER
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_TRANSMIT_TDM_BIASGEN_STANDBY_TIMER (??), 0-3
 */
#define VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TDM_BIASGEN_STANDBY_TIMER(target,ri)  VTSS_IOREG(target,0x514 + (ri))

/** 
 * \brief
 * TDM biasgen standby timer value
 * This register defines the period of time that the bias generator is in
 * standby mode and in a reduced power state. This timer resolution (LSB)
 * is 256 UI at full rate (approximately 45 ns at 5.67 Gbps or
 * approximately 23.1 ns at 11.10 Gbps).
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TDM_BIASGEN_STANDBY_TIMER . STBY
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TDM_BIASGEN_STANDBY_TIMER_STBY(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TDM_BIASGEN_STANDBY_TIMER_STBY     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_TDM_BIASGEN_STANDBY_TIMER_STBY(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief HSS11G Tx transmit TDM biasgen power on timer
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored.
 *
 * Register: \a XFI::HSS11G_TX::HSS11G_TRANSMIT_TDM_BIASGEN_POWER_ON_TIMER
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_TRANSMIT_TDM_BIASGEN_POWER_ON_TIMER (??), 0-3
 */
#define VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TDM_BIASGEN_POWER_ON_TIMER(target,ri)  VTSS_IOREG(target,0x518 + (ri))

/** 
 * \brief
 * TDM biasgen power on timer value
 * This register defines the period of time that the bias generator is in a
 * powered on mode. This timer resolution (LSB) is 256 UI at full rate
 * (approximately 45 ns at 5.67Gbps or approximately 23.1 ns at 11.10
 * Gbps).
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TDM_BIASGEN_POWER_ON_TIMER . PON
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TDM_BIASGEN_POWER_ON_TIMER_PON(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TDM_BIASGEN_POWER_ON_TIMER_PON     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_TDM_BIASGEN_POWER_ON_TIMER_PON(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief HSS11G Tx transmit tap 0 coefficient
 *
 * \details
 * Register: \a XFI::HSS11G_TX::HSS11G_TRANSMIT_TAP_0_COEFFICIENT
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_TRANSMIT_TAP_0_COEFFICIENT (??), 0-3
 */
#define VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_0_COEFFICIENT(target,ri)  VTSS_IOREG(target,0x520 + (ri))

/** 
 * \brief
 * Next tap 0 coefficient to be applied
 * This register's value is used in the computation of the TAP 0 IDAC value
 * that is sent to the analog block. Value is unsigned magnitude. This
 * value becomes effective when ALOAD (bit 0 of
 * HSS11G_TRANSMIT_COEFFICIENT_CONTROL) offset 0x02, is asserted.
 * Note: This register is only used when in traditional equalization mode
 * (EQMODE = 0).
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_0_COEFFICIENT . NXTT0
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_0_COEFFICIENT_NXTT0(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_0_COEFFICIENT_NXTT0     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_0_COEFFICIENT_NXTT0(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief HSS11G Tx transmit tap 1 coefficient
 *
 * \details
 * Register: \a XFI::HSS11G_TX::HSS11G_TRANSMIT_TAP_1_COEFFICIENT
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_TRANSMIT_TAP_1_COEFFICIENT (??), 0-3
 */
#define VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_1_COEFFICIENT(target,ri)  VTSS_IOREG(target,0x524 + (ri))

/** 
 * \brief
 * Next tap 1 coefficient to be applied
 * This register's value is used in the computation of the TAP 1 IDAC value
 * that is sent to the analog block. Value is unsigned magnitude. This
 * value becomes effective when ALOAD (bit 0 of
 * HSS11G_TRANSMIT_COEFFICIENT_CONTROL) offset 0x02, is asserted.
 * Note: This register is only used when in traditional equalization mode
 * (EQMODE = 0).
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_1_COEFFICIENT . NXTT1
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_1_COEFFICIENT_NXTT1(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_1_COEFFICIENT_NXTT1     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_1_COEFFICIENT_NXTT1(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief HSS11G Tx transmit tap 2 coefficient
 *
 * \details
 * Register: \a XFI::HSS11G_TX::HSS11G_TRANSMIT_TAP_2_COEFFICIENT
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_TRANSMIT_TAP_2_COEFFICIENT (??), 0-3
 */
#define VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_2_COEFFICIENT(target,ri)  VTSS_IOREG(target,0x528 + (ri))

/** 
 * \brief
 * Next tap 2 coefficient to be applied
 * This register's value is used in the computation of the TAP 2 IDAC value
 * that is sent to the analog block. Value is unsigned magnitude. This
 * value becomes effective when ALOAD (bit 0 of
 * HSS11G_TRANSMIT_COEFFICIENT_CONTROL) offset 0x02, is asserted.
 * Note: This register is only used when in traditional equalization mode
 * (EQMODE = 0).
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_2_COEFFICIENT . NXTT2
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_2_COEFFICIENT_NXTT2(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_2_COEFFICIENT_NXTT2     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_2_COEFFICIENT_NXTT2(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief HSS11G Tx transmit power
 *
 * \details
 * Register: \a XFI::HSS11G_TX::HSS11G_TRANSMIT_POWER
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_TRANSMIT_POWER (??), 0-3
 */
#define VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_POWER(target,ri)  VTSS_IOREG(target,0x530 + (ri))

/** 
 * \brief
 * Normalized transmitter power
 * Value is unsigned magnitude (always positive). Valid values are 0x20
 * minimum to 0x7F maximum. This value becomes effective when ALOAD (bit 0
 * of HSS11G_TRANSMIT_COEFFICIENT_CONTROL) offset 0x02, is asserted.
 * Note: This register is only used when in traditional equalization mode
 * (EQMODE = 0).
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_POWER . TXPWR
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_POWER_TXPWR(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_POWER_TXPWR     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_POWER_TXPWR(x)  VTSS_EXTRACT_BITFIELD(x,0,7)


/** 
 * \brief HSS11G Tx transmit polarity
 *
 * \details
 * Register: \a XFI::HSS11G_TX::HSS11G_TRANSMIT_POLARITY
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_TRANSMIT_POLARITY (??), 0-3
 */
#define VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_POLARITY(target,ri)  VTSS_IOREG(target,0x534 + (ri))

/** 
 * \brief
 * Current polarity being applied 
 * Bits 2,1,0 correspond to taps 2,1,0.
 *
 * \details 
 * 0: Negative
 * 1: Positive
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_POLARITY . TXPOL
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_POLARITY_TXPOL(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_POLARITY_TXPOL     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_POLARITY_TXPOL(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * Next polarity value to be applied
 * Bits 2,1,0 correspond to taps 2,1,0.This value becomes effective when
 * ALOAD (bit 0 of HSS11G_TRANSMIT_COEFFICIENT_CONTROL) offset 0x02, is
 * asserted. 
 * Note: These bits are used only when in traditional equalization mode
 * (EQMODE = 0).
 *
 * \details 
 * 0: Negative
 * 1: Positive
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_POLARITY . NTXPOL
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_POLARITY_NTXPOL(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_POLARITY_NTXPOL     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_POLARITY_NTXPOL(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief HSS11G Tx transmit 802p3ap adaptive equalization command
 *
 * \details
 * This register is used to make adjustments to the transmitter compatible
 * with the requirements of IEEE 802p3ap, Clause 72, standard. This
 * register is only active when in 802p3ap equalization mode (EQMODE = 1)
 * and AESOURCE = 0 in HSS11G_TRANSMIT_COEFFICIENT_CONTROL.
 *
 * Register: \a XFI::HSS11G_TX::HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_COMMAND
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_COMMAND (??), 0-3
 */
#define VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_COMMAND(target,ri)  VTSS_IOREG(target,0x538 + (ri))

/** 
 * \brief
 * Initialize coefficients
 * The coefficients are initialized with the values found in the
 * Coefficient Initialization registers (see Section Transmit 802p3ap C0
 * Init Extended Register on page 145, Section Transmit 802p3ap C1 Init
 * Extended Register on page 147, and Section Transmit 802p3ap C2 Init
 * Extended Register on page 149.)
 * Notes:
 * 1. This bit is ignored if bit 13 = 1
 * 2. These bits are used only when in 802p3ap equalization mode (EQMODE =
 * 1) and AESOURCE = 0 in HSS11G_TRANSMIT_COEFFICIENT_CONTROL
 *
 * \details 
 * 0: Normal (default)
 * 1: Ignore bits 11:0 of this register and initialize the coefficients
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_COMMAND . CXINIT
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_COMMAND_CXINIT(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_COMMAND_CXINIT     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_COMMAND_CXINIT(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * C2 Update
 * Update transmitter tap coefficient 2
 * Notes:
 * 1. These bits are ignored if bits 12 or 13 equal 1
 * 2. These bits are used only when in 802p3ap Equalization mode (EQMODE =
 * 1) and AESOURCE = 0
 *
 * \details 
 * 00: Hold (default)
 * 01: Increment
 * 10: Decrement
 * 11: Reserved
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_COMMAND . C2UPDT
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_COMMAND_C2UPDT(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_COMMAND_C2UPDT     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_COMMAND_C2UPDT(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * C1 Update
 * Update transmitter tap coefficient 1
 * Notes:
 * 1. These bits are ignored if bits 12 or 13 equal 1
 * 2. These bits are used only when in 802p3ap Equalization mode (EQMODE =
 * 1) and AESOURCE = 0
 *
 * \details 
 * 00: Hold (default)
 * 01: Increment
 * 10: Decrement
 * 11: Reserved
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_COMMAND . C1UPDT
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_COMMAND_C1UPDT(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_COMMAND_C1UPDT     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_COMMAND_C1UPDT(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * C0 Update: Update transmitter tap coefficient 0 as specified below:
 * Notes:
 * 1. These bits are ignored if bits 12 or 13 equal 1
 * 2. These bits are used only when in 802p3ap Equalization mode (EQMODE =
 * 1) and AESOURCE = 0
 *
 * \details 
 * 00: Hold (default)
 * 01: Increment
 * 10: Decrement
 * 11: Reserved
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_COMMAND . C0UPDT
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_COMMAND_C0UPDT(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_COMMAND_C0UPDT     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_COMMAND_C0UPDT(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief HSS11G Tx transmit 802p3ap adaptive equalization status
 *
 * \details
 * This register is used to read the status of adjustments to the
 * transmitter compatible with the requirements of IEEE 802p3ap, Clause 72,
 * standard. This register is only active when in 802p3ap Equalization mode
 * (EQMODE = 1). It is active whether the AE Source bit (bit 5 of transmit
 * register 0x02) is 0 or 1.
 *
 * Register: \a XFI::HSS11G_TX::HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_STATUS
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_STATUS (??), 0-3
 */
#define VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_STATUS(target,ri)  VTSS_IOREG(target,0x53c + (ri))

/** 
 * \brief
 * C2 Status
 * Status of transmitter tap coefficient 2 as a result of the last
 * coefficient command written to the Transmit 802p3ap Adaptive
 * Equalization Command register.
 * Note: These bits are used only when in 802p3ap Equalization mode (EQMODE
 * = 1).
 *
 * \details 
 * 00: Not updated
 * 01: Updated
 * 10: At Minimum value
 * 11: At Maximum value
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_STATUS . C2STAT
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_STATUS_C2STAT(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_STATUS_C2STAT     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_STATUS_C2STAT(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * C1 Status
 * Status of transmitter tap coefficient 1 as a result of the last
 * coefficient command written to the Transmit 802p3ap Adaptive
 * Equalization Command register.
 * Note: These bits are used only when in 802p3ap Equalization mode (EQMODE
 * = 1).
 *
 * \details 
 * 00: Not updated
 * 01: Updated
 * 10: At Minimum value
 * 11: At Maximum value
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_STATUS . C1STAT
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_STATUS_C1STAT(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_STATUS_C1STAT     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_STATUS_C1STAT(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * C0 Status
 * Status of transmitter tap coefficient 0 as a result of the last
 * coefficient command written to the Transmit 802p3ap Adaptive
 * Equalization Command register.
 * Note: These bits are used only when in 802p3ap Equalization mode (EQMODE
 * = 1).
 *
 * \details 
 * 00: Not updated
 * 01: Updated
 * 10: At Minimum value
 * 11: At Maximum value
 *
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_STATUS . C0STAT
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_STATUS_C0STAT(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_STATUS_C0STAT     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_802P3AP_ADAPTIVE_EQUALIZATION_STATUS_C0STAT(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief HSS11G Tx transmit tap 0 IDAC override
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored.
 *
 * Register: \a XFI::HSS11G_TX::HSS11G_TRANSMIT_TAP_0_IDAC_OVERRIDE
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_TRANSMIT_TAP_0_IDAC_OVERRIDE (??), 0-3
 */
#define VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_0_IDAC_OVERRIDE(target,ri)  VTSS_IOREG(target,0x540 + (ri))

/** 
 * \brief
 * Next tap0 IDAC to be applied
 * This register overrides the calculated Tap0 IDAC value with the contents
 * of this register. This is intended as a manufacturing diagnostic tool
 * only.
 * Notes: This register is only applied if OCOEF = 1 in
 * HSS11G_TRANSMIT_COEFFICIENT_CONTROL. The value of this register may
 * change during normal operation if OCOEF = 0. This register is only used
 * when in traditional equalization mode (EQMODE = 0).
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_0_IDAC_OVERRIDE . NIDAC0
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_0_IDAC_OVERRIDE_NIDAC0(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_0_IDAC_OVERRIDE_NIDAC0     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_0_IDAC_OVERRIDE_NIDAC0(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief HSS11G Tx transmit tap 1 IDAC override
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored.
 *
 * Register: \a XFI::HSS11G_TX::HSS11G_TRANSMIT_TAP_1_IDAC_OVERRIDE
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_TRANSMIT_TAP_1_IDAC_OVERRIDE (??), 0-3
 */
#define VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_1_IDAC_OVERRIDE(target,ri)  VTSS_IOREG(target,0x544 + (ri))

/** 
 * \brief
 * Next tap1 IDAC to be applied
 * This register overrides the calculated Tap1 IDAC value with the contents
 * of this register. This is intended as a manufacturing diagnostic tool
 * only.
 * Notes: This register is only applied if OCOEF = 1 in
 * HSS11G_TRANSMIT_COEFFICIENT_CONTROL. The value of this register may
 * change during normal operation if OCOEF = 0. This register is only used
 * when in traditional equalization mode (EQMODE = 0).
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_1_IDAC_OVERRIDE . NIDAC1
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_1_IDAC_OVERRIDE_NIDAC1(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_1_IDAC_OVERRIDE_NIDAC1     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_1_IDAC_OVERRIDE_NIDAC1(x)  VTSS_EXTRACT_BITFIELD(x,0,7)


/** 
 * \brief HSS11G Tx transmit tap 2 IDAC override
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored.
 *
 * Register: \a XFI::HSS11G_TX::HSS11G_TRANSMIT_TAP_2_IDAC_OVERRIDE
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_TRANSMIT_TAP_2_IDAC_OVERRIDE (??), 0-3
 */
#define VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_2_IDAC_OVERRIDE(target,ri)  VTSS_IOREG(target,0x548 + (ri))

/** 
 * \brief
 * Next tap2 IDAC to be applied
 * This register overrides the calculated Tap2 IDAC value with the contents
 * of this register. This is intended as a manufacturing diagnostic tool
 * only.
 * Notes: This register is only applied if OCOEF = 1 in
 * HSS11G_TRANSMIT_COEFFICIENT_CONTROL. The value of this register may
 * change during normal operation if OCOEF = 0. This register is only used
 * when in traditional equalization mode (EQMODE = 0).
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_2_IDAC_OVERRIDE . NIDAC2
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_2_IDAC_OVERRIDE_NIDAC2(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_2_IDAC_OVERRIDE_NIDAC2     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_TAP_2_IDAC_OVERRIDE_NIDAC2(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief HSS11G Tx transmit power DAC override
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored.
 *
 * Register: \a XFI::HSS11G_TX::HSS11G_TRANSMIT_POWER_DAC_OVERRIDE
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_TRANSMIT_POWER_DAC_OVERRIDE (??), 0-3
 */
#define VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_POWER_DAC_OVERRIDE(target,ri)  VTSS_IOREG(target,0x550 + (ri))

/** 
 * \brief
 * Override PDAC enable
 * This bit forces the Override PDAC setting (bits 4:0) to be applied to
 * internal analog circuits instead of the values normally calculated in
 * both Traditional and 802p3ap equalization modes. This is intended as a
 * manufacturing diagnostic tool only and should not be used by customers
 * unless instructed to do so.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_POWER_DAC_OVERRIDE . OPENX
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_POWER_DAC_OVERRIDE_OPENX(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_POWER_DAC_OVERRIDE_OPENX     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_POWER_DAC_OVERRIDE_OPENX(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Override PDAC value
 * ASIC solutions core databook
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_POWER_DAC_OVERRIDE . OPVAL
 */
#define  VTSS_F_XFI_HSS11G_TX_HSS11G_TRANSMIT_POWER_DAC_OVERRIDE_OPVAL(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_XFI_HSS11G_TX_HSS11G_TRANSMIT_POWER_DAC_OVERRIDE_OPVAL     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_XFI_HSS11G_TX_HSS11G_TRANSMIT_POWER_DAC_OVERRIDE_OPVAL(x)  VTSS_EXTRACT_BITFIELD(x,0,5)

/**
 * Register Group: \a XFI::HSS11G_RX
 *
 * XFI receive registers
 */


/** 
 * \brief HSS11G Rx receiver configuration mode
 *
 * \details
 * Changing bits 0, 1, 2, or 3 will cause the frequency of the RXxDCLK to
 * change. The clock will stop for up to 150 ns, but it will not sliver or
 * glitch.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_RECEIVER_CONFIGURATION_MODE
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_RECEIVER_CONFIGURATION_MODE (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(target,ri)  VTSS_IOREG(target,0x600 + (ri))

/** 
 * \brief
 * Bus width modifier control
 * When bits 3:2 of this register are configured to 16 or 20 bit bus width,
 * and bits 1:0 are configured for 1/8 rate mode (1/4 rate mode if
 * HSSLOFREQPLL= 1), this bit can be used to convert the 16 or 20 bit mode
 * to 8 or 10 bit mode respectively. This bit is ignored when bits 3:0 are
 * configured to other settings. 
 * Note: Changes to this bit are not recognized until changes are made to
 * bit 7 or bits 3:0 of this register. If this configuration is desired in
 * 1/8 rate mode (1/4 rate mode if HSSLOFREQPLL= 1), then this bit should
 * be set before, or at the same time as bits 7 and/or 3:0 are written.
 * This bit can be left at 1 at all times, if desired, because it is
 * ignored in other modes.
 *
 * \details 
 * 0: Normal operation. Bits 3:2 of this register behave as defined.
 * 1: Bus width 8/10 mode. Bits 3:2 of this register are interpreted as
 * follows:
 *  00: 8 bit (instead of 16)
 *  01: 10 bit (instead of 20)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE . BW810
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BW810(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BW810     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BW810(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Auxiliary clock input enable
 * This mode requires the rate selection to be set to 11 (eighth rate
 * mode), and enables 1.250G operation of this link while the PLL is
 * configured to support 10.3125G full-rate operation. This bit can be left
 * at 1 at all times, if desired, because it is ignored in other modes.
 * Notes:
 * 1. This bit must be set to 1 before, or simultaneously with, setting
 * bits 3:0 of this register.
 * 2. Use of spread spectrum clocking is not allowed when the AUXCLK bit is
 * set to 1. 
 *
 * \details 
 * 0: Normal operation (default)
 * 1: 10.3125G/1.250G mode
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE . AUXCLK
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_AUXCLK(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_AUXCLK     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_AUXCLK(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * DFE/non-DFE mode selector
 * Notes: Data errors may occur when switching between modes. LPnon-DFE
 * mode disables internal AMP sample processing, Ap/An/Amin loops, and VGA
 * gain/offset loops (unless FRCH = 1 in HSS11G_DFE_CONTROL). An internal
 * DFE reset operation is automatically generated after changing the value
 * of these bits. Bit combinations of 000, 001, and 010, must only be
 * selected when operating in full-rate or half-rate modes. Selection of
 * these values in quarter-rate or eighth-rate modes will give
 * unpredictable results.
 *
 * \details 
 * 000: DFE5
 * 001: DFE3
 * 010: DFE1
 * 100: Non-DFE (default)
 * 101: LPnon-DFE, use only for data rates of 7.5 Gbps or less
 * Others: Reserved, and default to non-DFE
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE . DMSEL
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_DMSEL(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_DMSEL     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_DMSEL(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * Data bus width select 
 *
 * \details 
 * 00: 16 bit parallel bus (8 bit parallel bus if BW810= 1 and 1/8 rate
 * mode *)
 * 01: 20 bit parallel bus (10 bit parallel bus if BW810= 1 and 1/8 rate
 * mode *)
 * 10: 32 bit parallel bus (default)
 * 11: 40 bit parallel bus
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE . BWSEL
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_BWSEL(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Rate select
 * Notes: An internal LINKRESET operation is automatically generated after
 * changing the value of these bits. A resync operation should not be
 * initiated until 150 nsec after this register is changed. DFE1, DFE3, and
 * DFE5 modes are only supported in full-rate and half-rate modes.
 * Selection of these DFE modes while in quarter-rate or eighth-rate modes
 * will cause unpredictable results. Scopemode is only supported in
 * full-rate and half-rate modes.
 *
 * \details 
 * 00: Full rate (default)
 * 01: Half rate
 * 10: Quarter rate
 * 11: Eighth rate
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE . RTSEL
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_RTSEL(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief HSS11G Rx receiver test control
 *
 * \details
 * Register: \a XFI::HSS11G_RX::HSS11G_RECEIVER_TEST_CONTROL
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_RECEIVER_TEST_CONTROL (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL(target,ri)  VTSS_IOREG(target,0x604 + (ri))

/** 
 * \brief
 * Received clock output enable
 * Enables the differential recovered clock output on RXxRCVC16 T and P
 * pins.
 *
 * \details 
 * 0: Disabled (default)
 * 1: Enabled
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL . RCLKEN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RCLKEN(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Received clock output rate selector
 *
 * \details 
 * 00: C16: 1/16th of the received data rate (default)
 * 01: C32: 1/32nd of the received data rate
 * 10: C64: 1/64th of the received data rate
 * 11: C16: 1/16th of the received data rate
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL . RRATE
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE(x)  VTSS_ENCODE_BITFIELD(x,13,2)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE     VTSS_ENCODE_BITMASK(13,2)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_RRATE(x)  VTSS_EXTRACT_BITFIELD(x,13,2)

/** 
 * \brief
 * PRBS checker error status
 * State of RXxPRBSERR output pin that indicates the PRBS checker status
 *
 * \details 
 * 0: PRBS pattern match or PRBS sync not yet achieved
 * 1: PRBS error detected after PRBS sync has been achieved
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL . LBERROR
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_LBERROR(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_LBERROR     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_LBERROR(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * PRBS checker sync status
 * State of the RXxPRBSSYNC output pin that indicates whether pattern
 * synchronization was achieved
 *
 * \details 
 * 0: PRBS checker not synchronized to incoming data
 * 1: PRBS checker synchronized and locked to incoming PRBS data
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL . LBSYNC
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_LBSYNC(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_LBSYNC     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_LBSYNC(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Wrap clock mode enable 
 * The amplitude phase rotator of this link is used to generate an async
 * input clock to the adjacent Tx/Rx pair. This can be used in combination
 * with FDWRAP (bit 6 of this register) to exercise the adjacent Rx in an
 * async environment. This is intended for manufacturing test, and imposes
 * significant limitations for general use. It can only be used in full and
 * half rate modes, it disables the normal full rate clock to the Tx of
 * this link, and it freezes DFE functionality for this link. When the HS
 * PLL is in 2/3 rate mode, this only works in receiver full-rate mode.
 *
 * \details 
 * 0: Normal operation (default)
 * 1: Loopback slave mode
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL . FDWRAPCLK
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_FDWRAPCLK(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_FDWRAPCLK     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_FDWRAPCLK(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Full duplex wrap enable
 * Notes:
 * 1. This register is ignored if RXPRBSFRCERR = 1 and the primary input to
 * the Rx is selected
 * 2. A DFE reset must be issued after this bit is written to 0
 *
 * \details 
 * 0: Normal operation - Selects the primary input to the Rx and disables
 * the internal Tx to Rx wrap buffer (default)
 * 1: Wrap - Enables the wrap back driver in the Tx to drive the internal
 * wrap path to this Rx
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL . FDWRAP
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_FDWRAP(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_FDWRAP     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_FDWRAP(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * PRBS reset
 * Note: This register is ignored if RXPRBSRST = 1. A reset is applied to
 * the PRBS checker.
 *
 * \details 
 * 0: Normal operation (default)
 * 1: Reset is applied to the PRBS checker. (Hold at 1 for at least two
 * RXxDCLK cycles.)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL . PRST
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_PRST(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_PRST     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_PRST(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * PRBS check enable
 * Ignored if RXPRBSEN = 1 and HSSPRBSEN = 1.
 *
 * \details 
 * 0: Disabled (default)
 * 1: Enabled
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL . PCHKEN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_PCHKEN(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_PCHKEN     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_PCHKEN(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Test pattern select
 * Ignored when RXPRBSEN = 1.
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
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL . PRBSSEL
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_PRBSSEL(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_PRBSSEL     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_TEST_CONTROL_PRBSSEL(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief HSS11G Rx phase rotator control
 *
 * \details
 * Register: \a XFI::HSS11G_RX::HSS11G_PHASE_ROTATOR_CONTROL
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_PHASE_ROTATOR_CONTROL (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL(target,ri)  VTSS_IOREG(target,0x608 + (ri))

/** 
 * \brief
 * Flywheel throttle
 * Limits the range of the internal flywheel when SSCENABLE = 1. It should
 * be set to the closest value exceeding the applied SSC sweep range. All
 * values from 0x0 to 0xF are legal, and set the limit as follows: FW_limit
 * = (7 + 8 x value) x 61 ppm. For example: 0x0 => (7 + 8 x 0) x 61 = 7 x
 * 61 = 427 ppm 0xC => (7 + 8 x 12) x 61 = 15 x 61 = 6283 ppm (default) 0xF
 * => (7 + 8 x15) x 61 = 127 x 61 = 7747 ppm
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL . FTHROT
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_FTHROT(x)  VTSS_ENCODE_BITFIELD(x,12,4)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_FTHROT     VTSS_ENCODE_BITMASK(12,4)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_FTHROT(x)  VTSS_EXTRACT_BITFIELD(x,12,4)

/** 
 * \brief
 * Reset flywheel when throttle limit reached
 *
 * \details 
 * 0: Disable flywheel reset due to throttle limit
 * 1: Enable flywheel reset due to throttle limit. (default)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL . RTHROT
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_RTHROT(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_RTHROT     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_RTHROT(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * CDR Loop filter bandwidth control
 * This register should be left at the default value. To support
 * characterization test, this can be used to adjust the CDR loop filter
 * bandwidth. This value controls the magnitude of phase inc/decs to the
 * phase accumulator of the CDR as follows: 
 * Full and half rate mode: magnitude = 1/2 this value
 * Quarter rate mode: magnitude = this value
 * Eighth rate mode: magnitude = 2x this value
 * All values from 0x2 to 0xF are legal
 * 0x2 is forced for writes of 0x0 and 0x1
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL . FILTCTL
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_FILTCTL(x)  VTSS_ENCODE_BITFIELD(x,7,4)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_FILTCTL     VTSS_ENCODE_BITMASK(7,4)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_FILTCTL(x)  VTSS_EXTRACT_BITFIELD(x,7,4)

/** 
 * \brief
 * Rotator CDR servo select
 *
 * \details 
 * 00: EDGE centered vs. DATA (required for normal operation)
 * 01: AMP centered vs. EDGE using AMP (odd)
 * 10: AMP centered vs. EDGE using AMP (even)
 * 11: DATA centered vs. EDGE
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL . RSRVO
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_RSRVO(x)  VTSS_ENCODE_BITFIELD(x,5,2)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_RSRVO     VTSS_ENCODE_BITMASK(5,2)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_RSRVO(x)  VTSS_EXTRACT_BITFIELD(x,5,2)

/** 
 * \brief
 * External early/late control
 * Used to select the external early/late phase rotator controls
 *
 * \details 
 * 0: Internal only (default)
 * 1: External only
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL . EXTEL
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_EXTEL(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_EXTEL     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_EXTEL(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Reset on stuck
 * 1/4 and 1/8 rate modes only
 *
 * \details 
 * 0: Stuck condition does not reset the flywheel (recommended)
 * 1: Stuck condition resets the flywheel (not recommended for application
 * with more than 100 ppm HSSREFCLK tolerance)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL . RSTONSTUCK
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_RSTONSTUCK(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_RSTONSTUCK     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_RSTONSTUCK(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Freeze flywheel
 * This can be used to prevent periods of inactivity from altering the
 * state of the flywheel
 *
 * \details 
 * 0: Normal (default)
 * 1: Freeze the phase rotator flywheel at its current update rate
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL . FREEZEFW
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_FREEZEFW(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_FREEZEFW     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_FREEZEFW(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Reset flywheel
 *
 * \details 
 * 0: Normal, the flywheel is enabled (default)
 * 1: Assert reset to the phase rotator flywheel (disable the flywheel)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL . RESETFW
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_RESETFW(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_RESETFW     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_RESETFW(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Spread spectrum clocking enable
 * This should not be enabled unless SSC input data is applied or HSSREFCLK
 * is performing SSC. Rx jitter tolerance is improved in non-SSC mode.
 * Note: Use of spread spectrum clocking is not allowed when the AUXCLK bit
 * is set to 1.
 *
 * \details 
 * 0: Spread spectrum clocking support disabled (default)
 * 1: Spread spectrum clocking support enabled
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL . SSCENABLE
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_SSCENABLE(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_SSCENABLE     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_CONTROL_SSCENABLE(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief HSS11G Rx phase rotator offset control
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored. During the reset sequence, the positions of the 3
 * rotators are established such that the EDGE rotator lags the DATA and
 * AMP rotators by 16 steps (nominally 90 degrees of C2 clock). These
 * offsets will change during normal operation when bit 7 (func_Offset90)
 * of register 0x1F (DFE Function control) is set to 1. However, by
 * clearing bit 7 of register offset 0x1F, this register can be used to
 * support lab testing by providing direct control of these offsets. The
 * EDGE rotator is treated as the reference, because it is managed directly
 * by the CDR algorithm.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_PHASE_ROTATOR_OFFSET_CONTROL
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_PHASE_ROTATOR_OFFSET_CONTROL (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL(target,ri)  VTSS_IOREG(target,0x60c + (ri))

/** 
 * \brief
 * Rotator snapshot trigger
 * This write value is not actually stored in a register, so it is not
 * necessary to write it to a 0 before writing the next trigger. Read the
 * captured rotator positions from the HSS11G_PHASE_ROTATOR_POSITION_x
 * registers. Note that these position vectors are continuously updated
 * during normal operation. Set the STNDBY bit in HSS11G_DFE_CONTROLS
 * before attempting manual triggering and readout of these position
 * vectors.
 *
 * \details 
 * 0: Normal operation. (default)
 * 1: Triggers the internal snapshot logic to capture the current positions
 * of the three internal phase rotators
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL . RSNP
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL_RSNP(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL_RSNP     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL_RSNP(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Timescale offset enable
 *
 * \details 
 * 0: Disabled (offset_H1/An not applied to rotator offsets)
 * 1: Enabled, normal operation (default)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL . TSOEN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL_TSOEN(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL_TSOEN     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL_TSOEN(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Offset90 enable
 * Note: This value is only effective in DFE mode. In non-DFE mode phase
 * offsets are never enabled.
 *
 * \details 
 * 0: Disable phase offset adjustments for DATA vs. EDGE and AMP vs EDGE
 * and force each offset back to the nominal 90 degree offset (16 steps, +
 * static offset)
 * 1: Enable all phase offset adjustments (default)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL . OFFEN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL_OFFEN(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL_OFFEN     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL_OFFEN(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Timescale factor
 * Controls a coefficient used in the calculation of timing offset as a
 * function of H1 and AN. This value should not be changed.
 *
 * \details 
 * 00: N=180o, offset*8=128-128*H1/AN
 * 01: N=135o, offset*8=128-96*H1/AN
 * 10: N=158o, offset*8=128-112*H1/AN (default)
 * 11: N=112o, offset*8=128-80*H1/AN
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL . TMSCAL
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL_TMSCAL(x)  VTSS_ENCODE_BITFIELD(x,7,2)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL_TMSCAL     VTSS_ENCODE_BITMASK(7,2)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL_TMSCAL(x)  VTSS_EXTRACT_BITFIELD(x,7,2)

/** 
 * \brief
 * Apply adjustment to selected rotator
 * Bits 5:0 should be written in a previous operation before setting this
 * bit to 1. Read the resulting rotator positions from the
 * HSS11G_PHASE_ROTATOR_POSITION_x registers.
 *
 * \details 
 * 0: Normal (default)
 * 1: Rising edge applies the update
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL . APADJ
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL_APADJ(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL_APADJ     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL_APADJ(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Rotator select
 *
 * \details 
 * 0: Target offset adjustment to DATA rotator relative to EDGE (default)
 * 1: Target offset adjustment to AMP rotator relative to EDGE
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL . RSEL
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL_RSEL(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL_RSEL     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL_RSEL(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Phase offset target
 * The phase rotator selected by bit 5 will be adjusted to lead the EDGE
 * rotator by this amount when bit 6 is written to a 1 (rising edge).
 * Increasing this offset moves the sampling clock earlier in the eye.
 *
 * \details 
 * 0x00: 0 degree phase offset (360 degree=1 UI)
 * 0x01 - 0x1F: 11.25 degree phase offset per step (default = 180 degrees)
 * 0x1F: 348.75 degree offset
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL . PHOFFS
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL_PHOFFS(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL_PHOFFS     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_OFFSET_CONTROL_PHOFFS(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief HSS11G Rx phase rotator position 1
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored. In normal operation it is continuously updated by
 * the DFE algorithms. The value of this register following reset is not
 * defined. To accurately read the value in this register, the DFE logic
 * should be stopped by setting the STNDBY bit in HSS11G_DFE_CONTROLS to 1.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_PHASE_ROTATOR_POSITION_1
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_PHASE_ROTATOR_POSITION_1 (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_POSITION_1(target,ri)  VTSS_IOREG(target,0x610 + (ri))

/** 
 * \brief
 * Rotator0A position
 * Snapshot sample of AMP channel phase rotator position. This is a 6-bit
 * vector that indicates which of the 64 possible positions the phase
 * rotator is in when the trigger signal was received (RSNP bit in
 * HSS11G_PHASE_ROTATOR_OFFSET_CONTROL).
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_POSITION_1 . ROT0A
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_POSITION_1_ROT0A(x)  VTSS_ENCODE_BITFIELD(x,8,6)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_POSITION_1_ROT0A     VTSS_ENCODE_BITMASK(8,6)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_POSITION_1_ROT0A(x)  VTSS_EXTRACT_BITFIELD(x,8,6)

/** 
 * \brief
 * Rotator00 position
 * Snapshot sample of DATA channel phase rotator position. This is a 6-bit
 * vector that indicates which of the 64 possible positions the phase
 * rotator is in when the trigger signal was received (RSNP bit in
 * HSS11G_PHASE_ROTATOR_OFFSET_CONTROL).
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_POSITION_1 . ROT00
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_POSITION_1_ROT00(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_POSITION_1_ROT00     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_POSITION_1_ROT00(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief HSS11G Rx phase rotator position 2
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored. In normal operation it is continuously updated by
 * the DFE algorithms. The value of this register following reset is not
 * defined. To accurately read the values in these registers, stop the DFE
 * logic by setting bit 5 of HSS11G_DFE_CONTROL to 1.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_PHASE_ROTATOR_POSITION_2
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_PHASE_ROTATOR_POSITION_2 (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_POSITION_2(target,ri)  VTSS_IOREG(target,0x614 + (ri))

/** 
 * \brief
 * Rotator90 position
 * Snapshot sample of EDGE channel phase rotator position. This is a 6-bit
 * vector indicating which of the 64 possible positions the phase rotator
 * is in when the trigger signal was received (RSNP bit in
 * HSS11G_PHASE_ROTATOR_OFFSET_CONTROL).
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_POSITION_2 . ROT90
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_POSITION_2_ROT90(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_POSITION_2_ROT90     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_POSITION_2_ROT90(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief HSS11G Rx phase rotator static phase offset
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored. Following reset, an internal state machine executes
 * a sequence that measures the static phase offset between the Data and
 * Amp (even/odd) sampling clocks, as perceived by their respective
 * latches. The results of this sequence are stored here. Typically, these
 * registers should not be changed, but they can be written with override
 * values to support lab test purposes. The value of this register after
 * reset is undefined.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_PHASE_ROTATOR_STATIC_PHASE_OFFSET
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_PHASE_ROTATOR_STATIC_PHASE_OFFSET (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_STATIC_PHASE_OFFSET(target,ri)  VTSS_IOREG(target,0x618 + (ri))

/** 
 * \brief
 * Rotator offset calibration error
 *
 * \details 
 * 0: Calibration OK
 * 1: Rotator offset out of expected range (-16 to +15)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_STATIC_PHASE_OFFSET . RCALER
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_STATIC_PHASE_OFFSET_RCALER(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_STATIC_PHASE_OFFSET_RCALER     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_STATIC_PHASE_OFFSET_RCALER(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Amp vs Edge rotator static offset (odd)
 * This is a signed value (-16 to +15)
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_STATIC_PHASE_OFFSET . RAOOFF
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_STATIC_PHASE_OFFSET_RAOOFF(x)  VTSS_ENCODE_BITFIELD(x,10,5)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_STATIC_PHASE_OFFSET_RAOOFF     VTSS_ENCODE_BITMASK(10,5)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_STATIC_PHASE_OFFSET_RAOOFF(x)  VTSS_EXTRACT_BITFIELD(x,10,5)

/** 
 * \brief
 * Amp vs Edge rotator static offset (even)
 * This is a signed value (-16 to +15)
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_STATIC_PHASE_OFFSET . RAEOFF
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_STATIC_PHASE_OFFSET_RAEOFF(x)  VTSS_ENCODE_BITFIELD(x,5,5)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_STATIC_PHASE_OFFSET_RAEOFF     VTSS_ENCODE_BITMASK(5,5)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_STATIC_PHASE_OFFSET_RAEOFF(x)  VTSS_EXTRACT_BITFIELD(x,5,5)

/** 
 * \brief
 * Data vs Edge rotator static offset
 * This is a signed value (-16 to +15)
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_STATIC_PHASE_OFFSET . RDOFF
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_STATIC_PHASE_OFFSET_RDOFF(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_STATIC_PHASE_OFFSET_RDOFF     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_PHASE_ROTATOR_STATIC_PHASE_OFFSET_RDOFF(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief HSS11G Rx receiver sigdet control
 *
 * \details
 * Register: \a XFI::HSS11G_RX::HSS11G_RECEIVER_SIGDET_CONTROL
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_RECEIVER_SIGDET_CONTROL (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(target,ri)  VTSS_IOREG(target,0x61c + (ri))

/** 
 * \brief
 * Offset sign (1:0) 
 * Sign of the offset on signal detect. Bit 14 is R/W, bit 13 is read only.
 *
 * \details 
 * 01: Positive
 * 10: Negative
 * 00: Zero (When Offset Amplitude=0, sign bits are forced to this value)
 * 11: Illegal (sign is poorly defined)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL . SIGNSD
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SIGNSD(x)  VTSS_ENCODE_BITFIELD(x,13,2)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SIGNSD     VTSS_ENCODE_BITMASK(13,2)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SIGNSD(x)  VTSS_EXTRACT_BITFIELD(x,13,2)

/** 
 * \brief
 * Offset Amplitude (4:0)
 * Unsigned absolute value of Offset Amplitude on SIGDET. This value should
 * not be changed in normal operation. It is set by the internal
 * calibration logic after reset and remains constant after calibration is
 * done unless written here.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL . DACSD
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_DACSD(x)  VTSS_ENCODE_BITFIELD(x,8,5)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_DACSD     VTSS_ENCODE_BITMASK(8,5)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_DACSD(x)  VTSS_EXTRACT_BITFIELD(x,8,5)

/** 
 * \brief
 * Signal detect power down
 *
 * \details 
 * 0: Enable (default)
 * 1: Power down the signal detect circuit
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL . SDPDN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDPDN(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDPDN     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDPDN(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Signal detected
 * In normal operation, bit 9 of the DFE Function Control register is set
 * to 1 resulting in the SIGDET signal being passed through this register
 * to the internal logic. This register can be read to obtain the status of
 * the SIGDET signal, but it cannot be written. However, to support
 * diagnostic tests, bit 9 of the DFE Function Control register can be set
 * to 0, and this register can be written to test its effect on internal
 * logic.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL . SIGDET
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SIGDET(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SIGDET     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SIGDET(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Signal detect level
 * Unsigned value of the comparator threshold used in the SIGDET circuit
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL . SDLVL
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief HSS11G Rx DFE control
 *
 * \details
 * Register: \a XFI::HSS11G_RX::HSS11G_DFE_CONTROL
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_DFE_CONTROL (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_DFE_CONTROL(target,ri)  VTSS_IOREG(target,0x620 + (ri))

/** 
 * \brief
 * DFE sample request completed
 *
 * \details 
 * 0: Inactive or not ready yet (normal)
 * 1: Requested sample snapshot is now valid and available in registers
 * 0x09 and 0x0A and in the serial packet processor interface
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_CONTROL . REQCMP
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_REQCMP(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_REQCMP     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_REQCMP(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * DFE sample request
 * If SPIFMT= 000, then this sample will be taken when the internal
 * sampling mode matches the current SMODE value (receiver register 0x0A,
 * bits 11:8). Otherwise, it will be captured immediately. Snapshot results
 * are available in registers 0x09 and 0x0A. Asserting this bit will cause
 * bit 15 to go low until the request has been fulfilled internally. Bit 15
 * must be at 1 (except for first use after RESET), indicating valid data
 * and readiness for a new sample request, before writing this bit to a 1.
 * If a sample request is made before the previous one has completed, the
 * results of both will be invalid.
 *
 * \details 
 * 0: Inactive (default)
 * 1: A rising edge of this bit triggers an internal state machine to take
 * a snapshot of the internal samples selected by SPIFMT
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_CONTROL . DFEREQ
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_DFEREQ(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_DFEREQ     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_DFEREQ(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Serial packet interface (SPI) enable
 *
 * \details 
 * 0: Disabled. Requests received through the UPTSTART signal are ignored.
 * (default)
 * 1: Enabled. Requests received through the UPTSTART signal are honored
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_CONTROL . SPCEN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_SPCEN(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_SPCEN     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_SPCEN(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * DFE gate enable
 * Notes:
 * 1: It is not necessary to set this for DFE/SPI requests because these
 * samples are only captured when normal operation enables sample flow
 * 2: It is not necessary to set this for scopemode because scopemode
 * directly enables sample flow
 *
 * \details 
 * 0: DFE samples flow only when needed by internal logic (normal operation
 * for power savings) (default)
 * 1: DFE samples always enabled
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_CONTROL . GATEEN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_GATEEN(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_GATEEN     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_GATEEN(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Serial packet format select
 * Selects which data is captured and presented in receiver registers 0x09
 * and 0x0A in response to sample requests initiated by bit 14 of this
 * register. For sample requests initiated by the UPTSTART input signal,
 * all of the formats (0 to 5) are processed in the order shown, and
 * transmitted on the serial packet interface. Register 0x09 will then be
 * restored to a fresh update of the data selected by SPIFMT. See receiver
 * registers 0x09 and 0x0A description for SPIFMT content details.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_CONTROL . SPIFMT
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_SPIFMT(x)  VTSS_ENCODE_BITFIELD(x,9,3)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_SPIFMT     VTSS_ENCODE_BITMASK(9,3)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_SPIFMT(x)  VTSS_EXTRACT_BITFIELD(x,9,3)

/** 
 * \brief
 * DFE power management control
 * Selected circuits (AMP path) are only required when actively training
 * the DFE coefficients. Because DFE logic is shared by two links, only one
 * needs to be enabled at a time. Once trained, these circuits can be
 * disabled to save power. This register defines the criteria for disabling
 * these circuits. 
 *
 * \details 
 * Bit 6
 * 0: AMP path continuously enabled (when otherwise enabled)
 * 1: AMP path disabled during inactive share cycle (default) 
 * Bit 8:7
 * 00: AMP path continuously enabled (when otherwise enabled)
 * 01: AMP path disabled after initial DFE convergence, then re-enabled on
 * 5% duty cycle basis to confirm convergence. It remains enabled until
 * convergence is reacquired (default)
 * 10: AMP path disabled after initial DFE convergence, and re-enabled on
 * 5% duty cycle basis, regardless of convergence
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_CONTROL . DFEPWR
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_DFEPWR(x)  VTSS_ENCODE_BITFIELD(x,6,3)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_DFEPWR     VTSS_ENCODE_BITMASK(6,3)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_DFEPWR(x)  VTSS_EXTRACT_BITFIELD(x,6,3)

/** 
 * \brief
 * DFE standby
 * All internal DFE operations are halted at the next available internal
 * break point. Clocks continue to run, but state machines are held idle.
 * All DFE training (A, H, VGA) is halted and coefficients remain fixed at
 * their present values. 
 * Note: Do not enable DFE Standby until the VGA is locked as indicated by
 * bit 8 of HSS11G_RECEIVER_VGA_CONTROL_3.
 *
 * \details 
 * 0: Normal operation (default)
 * 1: Standby mode
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_CONTROL . STNDBY
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_STNDBY(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_STNDBY     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_STNDBY(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Force H updates
 * Under certain conditions, the DFE logic halts its training process.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_CONTROL . FRCH
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_FRCH(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_FRCH     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_FRCH(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Not random data (dynamic)
 * Proper training of the DFE coefficients requires sufficiently random
 * data flow. To ensure this, a random data detector function is built into
 * the logic. This function continuously monitors data randomness, and
 * dynamically enables/disables DFE training to maintain sufficient random
 * balance of the samples included. This register can be read to obtain the
 * status of this internal signal. Depending on data flow, this signal may
 * assert very frequently under normal operation. 
 * When bit 12 of HSS11G_DFE_FUNCTION_CONTROL is set to 1 to enable the
 * internal random data detector to inhibit internal updates, the NONRND
 * bit can be read to obtain status but it cannot be written to. 
 * When bit 12 of HSS11G_DFE_FUNCTION_CONTROL is set to 0 to support
 * diagnostic tests, the NONRND bit can be modified to test its effect.
 * Note: This function only works in full-rate and half-rate modes. The
 * response of this bit is unreliable in quarter-rate and eighth-rate
 * modes.
 *
 * \details 
 * 0: Data is sufficiently random and updates continue
 * 1: Data is not sufficiently random and updates cease until random again
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_CONTROL . NONRND
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_NONRND(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_NONRND     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_NONRND(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Not random data (filtered) 
 * The non-random function described in bit 3 of this register will
 * typically result in very frequent toggling of bit 3. To provide a more
 * general indication of data randomness, a filtered version of bit 3 is
 * provided here in bit 2. 
 * Note: This function only works in full-rate and half-rate modes. The
 * response of this bit is unreliable in quarter-rate and eighth-rate
 * modes.
 *
 * \details 
 * 0: Data is sufficiently random and updates continue
 * 1: Data is not sufficiently random and updates cease until random again
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_CONTROL . NONRNF
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_NONRNF(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_NONRNF     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_NONRNF(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * DFE fast lock enable
 * Reduced loop filtering during initial lock, then max loop filtering
 * applied. 
 *
 * \details 
 * 0: Disabled. Max loop filtering at all times.
 * 1: Enabled. This mode is required to support initial CDR lock within 1
 * ms following rate changes (default).
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_CONTROL . FSTLCK
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_FSTLCK(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_FSTLCK     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_FSTLCK(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * DFE reset
 * DFE control logic reset.This bit must be cleared back to 0 before DFE
 * and non-DFE function can resume.
 *
 * \details 
 * 0: Normal. (default)
 * 1: Triggers a reset of the DFE logic
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_CONTROL . DFERST
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_DFERST(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_DFERST     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_CONTROL_DFERST(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief HSS11G Rx DFE data and edge sample
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_DFE_DATA_AND_EDGE_SAMPLE
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_DFE_DATA_AND_EDGE_SAMPLE (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_DFE_DATA_AND_EDGE_SAMPLE(target,ri)  VTSS_IOREG(target,0x624 + (ri))

/** 
 * \details 
 * SPIFMT: 0x0: edgeSAMP(7:0)
 * SPIFMT: 0x1: An(7:0)
 * SPIFMT: 0x2: Az(7:0)
 * SPIFMT: 0x3: A010(7:0)
 * SPIFMT: 0x4: offsetH1An(3:0), func_DAC, DAC_H1targetRange(2:0)
 * SPIFMT: 0x5: DFE/non-DFEmode(2:0), OEscan(4:0)
 * SPIFMT: 0x6: Unused (returns 0x00)
 * SPIFMT: 0x7: Unused (returns 0x00)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_DATA_AND_EDGE_SAMPLE . BYTE1
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_DATA_AND_EDGE_SAMPLE_BYTE1(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_DATA_AND_EDGE_SAMPLE_BYTE1     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_DATA_AND_EDGE_SAMPLE_BYTE1(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Notes:
 * 1: SMODE(2:0) is described in bits 11:8 of HSS11G_DFE_AMPLITUDE_SAMPLE
 * 2: Quarter_rate and Eighth_rate will also assert in half-rate and
 * quarter-rate respectively, when HSSLOWFREQPLL= 1.
 *
 * \details 
 * SPIFMT: 0x0: dataSAMP(7:0)
 * SPIFMT: 0x1: Amin(7:0)
 * SPIFMT: 0x2: Quarter_rate, Eighth_rate, H1max
 * SPIFMT: 0x3: A111(7:0)
 * SPIFMT: 0x4: func_DPC, DPC_H1targetRange(2:0), VGAgain(3:0)
 * SPIFMT: 0x5: SMODE(2:0),OLscan(4:0)
 * SPIFMT: 0x6: Unused (returns 0x00)
 * SPIFMT: 0x7: Unused (returns 0x00)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_DATA_AND_EDGE_SAMPLE . BYTE0
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_DATA_AND_EDGE_SAMPLE_BYTE0(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_DATA_AND_EDGE_SAMPLE_BYTE0     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_DATA_AND_EDGE_SAMPLE_BYTE0(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief HSS11G Rx DFE amplitude sample
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_DFE_AMPLITUDE_SAMPLE
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_DFE_AMPLITUDE_SAMPLE (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_DFE_AMPLITUDE_SAMPLE(target,ri)  VTSS_IOREG(target,0x628 + (ri))

/** 
 * \brief
 * Sample_mode
 * Indicates the DFE sampling mode that was active at the time the
 * edgeSAMP/dataSAMP/ampSAMP/ampqSAMP snapshot was taken. This value will
 * increment each time a snapshot request is made, so the next request will
 * return a sample of the next type. These sampling modes are defined as
 * follows: 
 *
 * \details 
 * sample_mode(2): AMPSEL -> 0: even, 1: odd 
 * sample_mode(1): H1mode -> 0: previous bit = 0 (+H1), 1: previous bit = 1
 * (-H1) 
 * sample_mode(0): AMPSEL -> 0: ANmode, 1: APmode
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_AMPLITUDE_SAMPLE . SMODE
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_AMPLITUDE_SAMPLE_SMODE(x)  VTSS_ENCODE_BITFIELD(x,8,4)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_AMPLITUDE_SAMPLE_SMODE     VTSS_ENCODE_BITMASK(8,4)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_AMPLITUDE_SAMPLE_SMODE(x)  VTSS_EXTRACT_BITFIELD(x,8,4)

/** 
 * \brief
 * AMP vs DATA sample correlation
 * Sample correlation varies with DATASYNC, and as the DFE runs normally.
 * This relationship can be one of two possibilities when the sample is
 * captured here. Because this correlation must be understood to process
 * the results, this register indicates the correlation at the time of
 * capture. 
 *
 * \details 
 * 0: ampSAMP_DFE(3, 2, 1): dataSAMP_DFE(6, 4, 2) 
 * 1: ampSAMP_DFE(3, 2, 1): dataSAMP_DFE(7, 5, 3)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_AMPLITUDE_SAMPLE . ADCORR
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_AMPLITUDE_SAMPLE_ADCORR(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_AMPLITUDE_SAMPLE_ADCORR     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_AMPLITUDE_SAMPLE_ADCORR(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * FFETRAINENABLE bit from DFE logic
 * This indicates the quality of the data and edge samples
 *
 * \details 
 * 0: Data not valid for FFE determination
 * 1: Data is valid to be used for FFE determination
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_AMPLITUDE_SAMPLE . TRAINEN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_AMPLITUDE_SAMPLE_TRAINEN(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_AMPLITUDE_SAMPLE_TRAINEN     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_AMPLITUDE_SAMPLE_TRAINEN(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * DFE amplitude sample qualifiers [3:1] 
 * These are qualifiers on the latest amplitude samples captured from the
 * pipeline registers in response to a DFE sample request (DFEREQ in
 * HSS11G_DFE_CONTROL). They indicate which DFE amplitude sample bits are
 * valid.
 *
 * \details 
 * 0: Data in bits 2:0 not valid
 * 1: Data in bits 2:0 is valid
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_AMPLITUDE_SAMPLE . ASAMPQ
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_AMPLITUDE_SAMPLE_ASAMPQ(x)  VTSS_ENCODE_BITFIELD(x,3,3)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_AMPLITUDE_SAMPLE_ASAMPQ     VTSS_ENCODE_BITMASK(3,3)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_AMPLITUDE_SAMPLE_ASAMPQ(x)  VTSS_EXTRACT_BITFIELD(x,3,3)

/** 
 * \brief
 * DFE amplitude samples [3:1] 
 * These are the latest amplitude samples captured from the pipeline
 * registers in response to a DFE sample request (DFEREQ in
 * HSS11G_DFE_CONTROL) or optional serial packet UPTSTART signal.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_AMPLITUDE_SAMPLE . ASAMP
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_AMPLITUDE_SAMPLE_ASAMP(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_AMPLITUDE_SAMPLE_ASAMP     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_AMPLITUDE_SAMPLE_ASAMP(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief HSS11G Rx receiver VGA control 1
 *
 * \details
 * Generally, this register can be ignored. In normal operation, bit 10 of
 * the DFE Function Control register is set to 1 enabling the DFE global
 * offset algorithm to control bits 0:7 of this register. This register can
 * be read to obtain the status of this offset coefficient. However, to
 * support diagnostic and manual override capability, bit 10 of register of
 * the DFE Function Control register can be set to 0 and this register can
 * be written to control the global offset coefficient.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_RECEIVER_VGA_CONTROL_1
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_RECEIVER_VGA_CONTROL_1 (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_1(target,ri)  VTSS_IOREG(target,0x62c + (ri))

/** 
 * \brief
 * POLE(1:0)
 * VGA peaking pole frequency control
 *
 * \details 
 * 0x0: Minimum pole frequency
 * 0x3: Maximum pole frequency (default)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_1 . POLE
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_1_POLE(x)  VTSS_ENCODE_BITFIELD(x,12,2)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_1_POLE     VTSS_ENCODE_BITMASK(12,2)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_1_POLE(x)  VTSS_EXTRACT_BITFIELD(x,12,2)

/** 
 * \brief
 * PEAK(2:0)
 * VGA peaking amplitude control value is decoded to PEAK(7:0) thermometer
 * code and driven to the analog block.
 * Note: This register is updated dynamically when DPC is enabled and
 * cannot be written. DPC must be disabled by setting bit 15 of receiver
 * register 0x1F to 0 to read/write this register reliably.
 *
 * \details 
 * 0x0: Minimum peaking
 * 0x7: Maximum peaking
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_1 . PEAK
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_1_PEAK(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_1_PEAK     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_1_PEAK(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * VGA offset sign
 * Sign of offset on VGA. Bit 7 is R/W, bit 6 is read only.
 * Note: This register is updated dynamically when automatic VGA offset
 * control is enabled and cannot be written. Automatic offset control must
 * be disabled by setting bit 10 of receiver register 0x1F to 0 to
 * read/write this register reliably.
 *
 * \details 
 * 01: Positive offset applied
 * 10: Negative offset applied
 * 00: Zero offset applied (may occur when amplitude = 0)
 * 11: Illegal (sign is poorly defined)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_1 . VOFFSN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_1_VOFFSN(x)  VTSS_ENCODE_BITFIELD(x,6,2)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_1_VOFFSN     VTSS_ENCODE_BITMASK(6,2)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_1_VOFFSN(x)  VTSS_EXTRACT_BITFIELD(x,6,2)

/** 
 * \brief
 * VGA offset amplitude
 * Amplitude of the global offset asserted at the input to the VGA. 
 * Note: This register is updated dynamically when automatic VGA offset
 * control is enabled and cannot be written. Automatic offset control must
 * be disabled by setting bit 10 of receiver register 0x1F to 0 to
 * read/write this register reliably.
 *
 * \details 
 * 000000: Minimum amplitude
 * 111111: Maximum amplitude
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_1 . VOFFA
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_1_VOFFA(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_1_VOFFA     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_1_VOFFA(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief HSS11G Rx receiver VGA control 2
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored. In normal operation, bit 11 of the DFE Function
 * Control register is set to 1 enabling the DFE algorithm to control this
 * register. This register can be read to obtain the status of the VGA gain
 * control vector. However, to support diagnostic and manual override
 * capability, bit 11 of the DFE Function Control register can be set to 0
 * and this register can be written to control the gain setting of the VGA.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_RECEIVER_VGA_CONTROL_2
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_RECEIVER_VGA_CONTROL_2 (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_2(target,ri)  VTSS_IOREG(target,0x630 + (ri))

/** 
 * \brief
 * Short VGA
 * Apply short circuit input to analog front end. For test purposes only.
 * Note: This register is updated dynamically when automatic VGA offset
 * control is enabled and cannot be written. Automatic offset control must
 * be disabled by setting FVGAIN of HSS11G_DFE_FUNCTION_CONTROL to 0 to
 * read/write this register reliably.
 *
 * \details 
 * 0: Normal (default)
 * 1: Shorted VGA output (0 Vppd)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_2 . SHORTV
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_2_SHORTV(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_2_SHORTV     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_2_SHORTV(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * VGA gain
 * These bits are decoded to form the 8-bit thermometer coded bits used by
 * the analog VGA gain circuit. The nominal VGA gain characteristic is
 * roughly given by the formula Gain = 0.5
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_2 . VGAIN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_2_VGAIN(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_2_VGAIN     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_2_VGAIN(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief HSS11G Rx receiver VGA control 3
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored. It is initialized at reset and does not change.
 * Access is provided for those who wish to set up the DFE or AGC for
 * unusual conditions.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_RECEIVER_VGA_CONTROL_3
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_RECEIVER_VGA_CONTROL_3 (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_3(target,ri)  VTSS_IOREG(target,0x634 + (ri))

/** 
 * \brief
 * DFE H bound 1
 * This applies a fixed limit on the sum of all Hs and should only be
 * enabled when the DAC algorithm is disabled (FDAC in
 * HSS11G_DFE_FUNCTION_CONTROL). If the DAC algorithm is enabled, this bit
 * will only restrict H2-H5 when the sum of H's limit is reached.
 *
 * \details 
 * 0: Sum of H s bounding disabled. (default)
 * 1: Enables bounding of the sum of |H1|+|H2|+|H3|+|H4|+|H5| such that
 * this sum cannot exceed |Ap|
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_3 . HBND1
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_3_HBND1(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_3_HBND1     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_3_HBND1(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * DFE H bound 0 
 * This applies a fixed limit on the H1, which can only be applied when the
 * DAC algorithm is disabled (FDAC in HSS11G_DFE_FUNCTION_CONTROL). If the
 * DAC algorithm is enabled, this bit has no effect. H1 cannot exceed 0.5 *
 * |Ap|.
 *
 * \details 
 * 0: H1 bounding disabled (default)
 * 1: Enables bounding of the H1 coefficient
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_3 . HBND0
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_3_HBND0(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_3_HBND0     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_3_HBND0(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * VGA locked
 * This register is set when the VGA achieves lock and is cleared if the
 * calculated gain setting falls outside a 25% band around the currently
 * defined AmaxTarget (bits [6:0] of this register). In such a case, the
 * DFE H updates are frozen, and the VGA logic adjusts the gain until Amax
 * reconverges to within 25% of the AmaxTarget value.The DFE algorithm can
 * resume training the H coefficients. 
 * Note: This register is updated dynamically when automatic VGA gain
 * control is enabled and cannot be written. Automatic gain control must be
 * disabled by setting FVGAIN in HSS11G_DFE_FUNCTION_CONTROL to 0 to
 * read/write this register reliably.
 *
 * \details 
 * 0: VGA has not converged or is attempting to reconverge after a large H
 * coefficient change
 * 1: VGA has converged on its gain setting
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_3 . VLCKD
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_3_VLCKD(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_3_VLCKD     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_3_VLCKD(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * VGA locked first 
 * This register is set when the VGA achieves lock and is cleared only by
 * reset. It indicates lock achieved at least once. In normal operation,
 * FNRND in HSS11G_DFE_FUNCTION_CONTROL is set to 1, enabling this signal
 * to control when the DFE can commence training of H coefficients. This
 * register can be read to obtain the status of this internal signal. To
 * support diagnostic tests, FNRND in HSS11G_DFE_FUNCTION_CONTROL can be
 * set to 0, and this register can be written to 1 or 0 to test its effect.
 * 
 * Note: This register is updated dynamically when automatic VGA gain
 * control is enabled and cannot be written. Automatic gain control must be
 * disabled by setting FVGAIN in HSS11G_DFE_FUNCTION_CONTROL to 0 to
 * read/write this register reliably.
 *
 * \details 
 * 0: VGA has not converged since reset
 * 1: VGA has converged on its initial gain setting. The DFE algorithm can
 * commence training the H coefficients
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_3 . VLCKDF2
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_3_VLCKDF2(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_3_VLCKDF2     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_3_VLCKDF2(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Amax target
 * Controls the target value for the Amax variable. The VGA gain is
 * adjusted until the calculated Amax value stabilizes to within two
 * adjacent settings about this target, at which point the VGA is locked
 * (bits 7 and 8 of this register are set), holding the gain setting fixed.
 * This value is treated as a positive, unsigned vector.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_3 . AMAXT
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_3_AMAXT(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_3_AMAXT     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_3_AMAXT(x)  VTSS_EXTRACT_BITFIELD(x,0,7)


/** 
 * \brief HSS11G Rx DFE D00 and D01 offset
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored. DC calibration is performed once following HSS reset
 * (not Link Reset or DFE Reset). The reset value is a function of offsets
 * in the D00 and D01 data paths. This register can be read to obtain the
 * results of DC calibration. To support diagnostic and manual override
 * capability, this register can be written to control these DC offset
 * calibration coefficients.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_DFE_D00_AND_D01_OFFSET
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_DFE_D00_AND_D01_OFFSET (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_DFE_D00_AND_D01_OFFSET(target,ri)  VTSS_IOREG(target,0x638 + (ri))

/** 
 * \brief
 * Summer D01 offset sign
 * Sign of the offset on D01 summer. Bit 14 is R/W, bit 13 is read only and
 * forced to a legal value when bit 14 is written. 
 *
 * \details 
 * 00: Zero (when amplitude = 0, sign bits are forced to this value)
 * 01: Positive
 * 10: Negative
 * 11: Illegal (sign is poorly defined)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_D00_AND_D01_OFFSET . D01SN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_D00_AND_D01_OFFSET_D01SN(x)  VTSS_ENCODE_BITFIELD(x,13,2)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_D00_AND_D01_OFFSET_D01SN     VTSS_ENCODE_BITMASK(13,2)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_D00_AND_D01_OFFSET_D01SN(x)  VTSS_EXTRACT_BITFIELD(x,13,2)

/** 
 * \brief
 * Summer D01 offset amplitude
 * Unsigned absolute value of offset amplitude on the D01 summer. This
 * value is set by the internal calibration logic after reset and remains
 * constant after calibration is done unless written here.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_D00_AND_D01_OFFSET . D01AMP
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_D00_AND_D01_OFFSET_D01AMP(x)  VTSS_ENCODE_BITFIELD(x,8,5)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_D00_AND_D01_OFFSET_D01AMP     VTSS_ENCODE_BITMASK(8,5)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_D00_AND_D01_OFFSET_D01AMP(x)  VTSS_EXTRACT_BITFIELD(x,8,5)

/** 
 * \brief
 * Summer D00 offset sign 
 * Sign of offset on the D00 summer. Bit 6 is R/W, bit 5 is read only and
 * forced to a legal value when bit 6 is written.
 *
 * \details 
 * 00: Zero (when amplitude = 0, sign bits are forced to this value) 
 * 01: Positive 
 * 10: Negative 
 * 11: Illegal (sign is poorly defined)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_D00_AND_D01_OFFSET . D00SN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_D00_AND_D01_OFFSET_D00SN(x)  VTSS_ENCODE_BITFIELD(x,5,2)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_D00_AND_D01_OFFSET_D00SN     VTSS_ENCODE_BITMASK(5,2)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_D00_AND_D01_OFFSET_D00SN(x)  VTSS_EXTRACT_BITFIELD(x,5,2)

/** 
 * \brief
 * Summer D00 offset amplitude 
 * Unsigned absolute value of offset amplitude on the D00 summer. This
 * value is set by the internal calibration logic after reset and remains
 * constant after calibration is done unless written here.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_D00_AND_D01_OFFSET . D00AMP
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_D00_AND_D01_OFFSET_D00AMP(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_D00_AND_D01_OFFSET_D00AMP     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_D00_AND_D01_OFFSET_D00AMP(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief HSS11G Rx DFE D10 and D11 offset
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored. DC calibration is performed once following HSS reset
 * (not Link Reset or DFE Reset). The reset value is a function of offsets
 * in the D10 and D11 data paths. This register can be read to obtain the
 * results of DC calibration. To support diagnostic and manual override
 * capability, this register can be written to control these DC offset
 * calibration coefficients.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_DFE_D10_AND_D11_OFFSET
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_DFE_D10_AND_D11_OFFSET (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_DFE_D10_AND_D11_OFFSET(target,ri)  VTSS_IOREG(target,0x63c + (ri))

/** 
 * \brief
 * Summer D11 offset sign
 * Sign of offset on the D11 summer. Bit 14 is R/W, bit 13 is read only and
 * forced to a legal value when bit 14 is written.
 *
 * \details 
 * 00: Zero (when amplitude = 0, sign bits are forced to this value)
 * 01: Positive
 * 10: Negative
 * 11: Illegal (sign is poorly defined)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_D10_AND_D11_OFFSET . D11SN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_D10_AND_D11_OFFSET_D11SN(x)  VTSS_ENCODE_BITFIELD(x,13,2)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_D10_AND_D11_OFFSET_D11SN     VTSS_ENCODE_BITMASK(13,2)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_D10_AND_D11_OFFSET_D11SN(x)  VTSS_EXTRACT_BITFIELD(x,13,2)

/** 
 * \brief
 * Summer D11 offset amplitude
 * Unsigned absolute value of offset amplitude on the D11 summer. This
 * value is set by the internal calibration logic after reset and remains
 * constant after calibration is done unless written here.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_D10_AND_D11_OFFSET . D11AMP
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_D10_AND_D11_OFFSET_D11AMP(x)  VTSS_ENCODE_BITFIELD(x,8,5)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_D10_AND_D11_OFFSET_D11AMP     VTSS_ENCODE_BITMASK(8,5)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_D10_AND_D11_OFFSET_D11AMP(x)  VTSS_EXTRACT_BITFIELD(x,8,5)

/** 
 * \brief
 * Summer D10 offset sign
 * Sign of offset on the D10 summer. Bit 6 is R/W, bit 5 is read only and
 * forced to a legal value when bit 6 is written.
 *
 * \details 
 * 00: Zero (when amplitude = 0, sign bits are forced to this value)
 * 01: Positive
 * 10: Negative
 * 11: Illegal (sign is poorly defined)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_D10_AND_D11_OFFSET . D10SN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_D10_AND_D11_OFFSET_D10SN(x)  VTSS_ENCODE_BITFIELD(x,5,2)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_D10_AND_D11_OFFSET_D10SN     VTSS_ENCODE_BITMASK(5,2)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_D10_AND_D11_OFFSET_D10SN(x)  VTSS_EXTRACT_BITFIELD(x,5,2)

/** 
 * \brief
 * Summer D10 offset amplitude
 * Unsigned absolute value of offset amplitude on the D10 summer. This
 * value is set by the internal calibration logic after reset and remains
 * constant after calibration is done unless written here.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_D10_AND_D11_OFFSET . D10AMP
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_D10_AND_D11_OFFSET_D10AMP(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_D10_AND_D11_OFFSET_D10AMP     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_D10_AND_D11_OFFSET_D10AMP(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief HSS11G Rx DFE E0 and E1 offset
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored. DC calibration is performed once following HSS reset
 * (not Link Reset or DFE Reset). The reset value is a function of offsets
 * in the E0 and E1 data paths. This register can be read to obtain the
 * results of DC calibration. To support diagnostic and manual override
 * capability, this register can be written to control these DC offset
 * calibration coefficients.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_DFE_E0_AND_E1_OFFSET
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_DFE_E0_AND_E1_OFFSET (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_DFE_E0_AND_E1_OFFSET(target,ri)  VTSS_IOREG(target,0x640 + (ri))

/** 
 * \brief
 * Summer E1 offset sign
 * Sign of offset on the E1 summer. Bit 14 is R/W, bit 13 is read only and
 * forced to a legal value when bit 14 is written.
 *
 * \details 
 * 00: Zero (when amplitude = 0, sign bits are forced to this value)
 * 01: Positive
 * 10: Negative
 * 11: Illegal (sign is poorly defined)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_E0_AND_E1_OFFSET . E1SN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_E0_AND_E1_OFFSET_E1SN(x)  VTSS_ENCODE_BITFIELD(x,13,2)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_E0_AND_E1_OFFSET_E1SN     VTSS_ENCODE_BITMASK(13,2)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_E0_AND_E1_OFFSET_E1SN(x)  VTSS_EXTRACT_BITFIELD(x,13,2)

/** 
 * \brief
 * Summer E1 offset amplitude
 * Unsigned absolute value of offset amplitude on the E1 summer. This value
 * is set by the internal calibration logic after reset and remains
 * constant after calibration is done unless written here.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_E0_AND_E1_OFFSET . E1AMP
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_E0_AND_E1_OFFSET_E1AMP(x)  VTSS_ENCODE_BITFIELD(x,8,5)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_E0_AND_E1_OFFSET_E1AMP     VTSS_ENCODE_BITMASK(8,5)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_E0_AND_E1_OFFSET_E1AMP(x)  VTSS_EXTRACT_BITFIELD(x,8,5)

/** 
 * \brief
 * Summer E0 offset sign
 * Sign of offset on the E0 summer. Bit 6 is R/W, bit 5 is read only and
 * forced to a legal value when bit 6 is written.
 *
 * \details 
 * 00: Zero (when amplitude = 0, sign bits are forced to this value)
 * 01: Positive
 * 10: Negative
 * 11: Illegal (sign is poorly defined)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_E0_AND_E1_OFFSET . E0SN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_E0_AND_E1_OFFSET_E0SN(x)  VTSS_ENCODE_BITFIELD(x,5,2)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_E0_AND_E1_OFFSET_E0SN     VTSS_ENCODE_BITMASK(5,2)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_E0_AND_E1_OFFSET_E0SN(x)  VTSS_EXTRACT_BITFIELD(x,5,2)

/** 
 * \brief
 * Summer E0 offset amplitude
 * Unsigned absolute value of offset amplitude on the E0 summer. This value
 * is set by the internal calibration logic after reset and remains
 * constant after calibration is done unless written here.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_E0_AND_E1_OFFSET . E0AMP
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_E0_AND_E1_OFFSET_E0AMP(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_E0_AND_E1_OFFSET_E0AMP     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_E0_AND_E1_OFFSET_E0AMP(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief HSS11G Rx receiver DAC-A offset
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored. DC calibration is performed once following HSS reset
 * (not Link Reset or DFE Reset). The reset value is a function of offsets
 * in the A0 and A1 data paths. This register can be read to obtain the
 * results of DC calibration. To support diagnostic and manual override
 * capability, this register can be written to control these DC offset
 * calibration coefficients.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_RECEIVER_DACA_OFFSET
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_RECEIVER_DACA_OFFSET (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_DACA_OFFSET(target,ri)  VTSS_IOREG(target,0x644 + (ri))

/** 
 * \brief
 * DAC-A1 offset (odd)
 * Signed value of offset amplitude on A summer when processing AMP samples
 * associated with ODD data bits. This value is summed with the with Ap,
 * An, Amin values as they are applied to the A summer. This value is set
 * by the internal calibration logic after reset, and remains constant
 * after CalibrationDone unless written here.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_DACA_OFFSET . AOFFO
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_DACA_OFFSET_AOFFO(x)  VTSS_ENCODE_BITFIELD(x,8,6)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_DACA_OFFSET_AOFFO     VTSS_ENCODE_BITMASK(8,6)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_DACA_OFFSET_AOFFO(x)  VTSS_EXTRACT_BITFIELD(x,8,6)

/** 
 * \brief
 * DAC-A0 offset (even)
 * Signed value of offset amplitude on A summer when processing AMP samples
 * associated with EVEN data bits. This value is summed with the with Ap,
 * An, Amin values as they are applied to the A summer. This value is set
 * by the internal calibration logic after reset, and remains constant
 * after CalibrationDone unless written here.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_DACA_OFFSET . AOFFE
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_DACA_OFFSET_AOFFE(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_DACA_OFFSET_AOFFE     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_DACA_OFFSET_AOFFE(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief HSS11G Rx receiver DAC-AP and DAC-AN
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_RECEIVER_DACAP_AND_DACAN
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_RECEIVER_DACAP_AND_DACAN (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_DACAP_AND_DACAN(target,ri)  VTSS_IOREG(target,0x648 + (ri))

/** 
 * \brief
 * DAC-A negative
 * This read-only register contains the portion of the AN accumulator that
 * is applied on the amplitude path DAC when the ANmode is active. This is
 * a twos-complement signed representation that normally contains a
 * positive value. This register is dynamic when DFE mode is active, so the
 * DFE should be frozen before valid readings can be taken.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_DACAP_AND_DACAN . DACAN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_DACAP_AND_DACAN_DACAN(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_DACAP_AND_DACAN_DACAN     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_DACAP_AND_DACAN_DACAN(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * DAC-A positive
 * This read-only register contains the portion of the AP accumulator that
 * is applied on the amplitude path DAC when the APmode is active. This is
 * a twos-complement signed representation that normally contains a
 * negative value. This register is dynamic when DFE mode is active, so the
 * DFE should be frozen before valid readings can be taken.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_DACAP_AND_DACAN . DACAP
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_DACAP_AND_DACAN_DACAP(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_DACAP_AND_DACAN_DACAP     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_DACAP_AND_DACAN_DACAP(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief HSS11G Rx receiver DAC-A min
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_RECEIVER_DACA_MIN
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_RECEIVER_DACA_MIN (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_DACA_MIN(target,ri)  VTSS_IOREG(target,0x64c + (ri))

/** 
 * \brief
 * DAC-Az
 * This read only register contains the portion of the Az accumulator that
 * is applied on the amplitude path DAC when the dynamic data centering
 * (DDC) algorithm is running. This is a twos-complement signed
 * representation, which normally contains a positive value. It represents
 * the amplitude at which the DFE corrected data eye width is measured.
 * This register is dynamic when DDC mode is active, so DDC should be
 * disabled (FDDC in HSS11G_DFE_FUNCTION_CONTROL) before valid readings are
 * taken.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_DACA_MIN . DACAZ
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_DACA_MIN_DACAZ(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_DACA_MIN_DACAZ     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_DACA_MIN_DACAZ(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * DAC-A minimum
 * This read-only register contains the portion of the AMin accumulator
 * that is applied on the amplitude path DAC when the AMinmode is active.
 * This is a twos-complement signed representation that normally contains a
 * positive value. This register is dynamic when DFE mode is active, so DFE
 * should be frozen before valid readings are taken.
 * The value of AMin represents the minimum height of the DFE corrected eye
 * at roughly the 3 sigma point, meaning that approximately 999 out of 1000
 * readings of the DFE eye are above AMin. 
 * To convert the digital value to an analog voltage, multiply by 3.3 mV.
 * (This is a nominal value for most of the range of DAC-A minimum. There
 * can be significant variation due to process and temperature variations
 * and some compression at high values of DAC-A minimum.)
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_DACA_MIN . DACAM
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_DACA_MIN_DACAM(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_DACA_MIN_DACAM     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_DACA_MIN_DACAM(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief HSS11G Rx receiver A-DAC control
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored. In normal operation, bit 0 of the DFE Function
 * Control register is set to a 1. This register can be read to obtain the
 * results of DC calibration. However, to support diagnostic and manual
 * override capability, bit 0 of the DFE Function Control register can get
 * set to 0 and this register can be written to control these DC
 * calibration coefficients.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_RECEIVER_ADAC_CONTROL
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_RECEIVER_ADAC_CONTROL (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_ADAC_CONTROL(target,ri)  VTSS_IOREG(target,0x650 + (ri))

/** 
 * \brief
 * A-DAC sign
 * Bit 8 is R/W, bit 7 is read only and forced to a legal value when bit 8
 * is written.
 *
 * \details 
 * 00: Zero (occurs when amplitude = 0)
 * 01: Positive
 * 10: Negative
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_ADAC_CONTROL . ADSN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_ADAC_CONTROL_ADSN(x)  VTSS_ENCODE_BITFIELD(x,7,2)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_ADAC_CONTROL_ADSN     VTSS_ENCODE_BITMASK(7,2)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_ADAC_CONTROL_ADSN(x)  VTSS_EXTRACT_BITFIELD(x,7,2)

/** 
 * \brief
 * A-DAC magnitude
 * Unsigned absolute value of offset amplitude on A1 summer. This value is
 * the final output value to the analog block. This value is continuously
 * updated by the DFE logic, unless disabled by clearing FADAC in
 * HSS11G_DFE_FUNCTION_CONTROL. This bit must be cleared to enable direct
 * register read/write control over this register. This is intended for
 * test purposes only, and changing it can corrupt internal DFE operation. 
 * Note: When controlled manually, internal A offset calibration values are
 * not applied. If desired, these can be obtained from
 * HSS11G_RECEIVER_DACA_OFFSET and manually added to the desired ADAC value
 * before writing it to this register.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_ADAC_CONTROL . ADMAG
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_ADAC_CONTROL_ADMAG(x)  VTSS_ENCODE_BITFIELD(x,0,7)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_ADAC_CONTROL_ADMAG     VTSS_ENCODE_BITMASK(0,7)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_ADAC_CONTROL_ADMAG(x)  VTSS_EXTRACT_BITFIELD(x,0,7)


/** 
 * \brief HSS11G Rx digital eye control
 *
 * \details
 * This register is primarily used for diagnostic and test purposes.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_DIGITAL_EYE_CONTROL
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_DIGITAL_EYE_CONTROL (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_CONTROL(target,ri)  VTSS_IOREG(target,0x654 + (ri))

/** 
 * \brief
 * Block_DAC_Az
 * Digital eye width scans of large eye openings may require DAC_Az to be
 * greater than 0 to locate eye edges while maintaining internal timing
 * constraints. This bit can be used to force DAC_Az to 0 at all times.
 *
 * \details 
 * 0: Az enabled (default)
 * 1: Az forced to zero
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_CONTROL . BLKAZ
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_CONTROL_BLKAZ(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_CONTROL_BLKAZ     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_CONTROL_BLKAZ(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Eye width
 * Latest available eye width measurement, in units of rotator steps. 
 * Note: To ensure stable read values, the DDC algorithm should be disabled
 * (FDDC in HSS11G_DFE_FUNCTION_CONTROL) before reading this register.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_CONTROL . WIDTH
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_CONTROL_WIDTH(x)  VTSS_ENCODE_BITFIELD(x,10,5)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_CONTROL_WIDTH     VTSS_ENCODE_BITMASK(10,5)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_CONTROL_WIDTH(x)  VTSS_EXTRACT_BITFIELD(x,10,5)

/** 
 * \brief
 * Minimum eye width interrupt threshold
 * This is an unsigned vector value (positive) defining the minimum
 * acceptable eye width as measured by the DDC logic algorithm. If the
 * measured eye width falls below this value the WERRFLG bit will be set in
 * HSS11G_RECEIVER_INTERNAL_STATUS and an EYE-QUALITY interrupt will be
 * triggered.
 *
 * \details 
 * 0x0: 0 threshold (default, no interrupt set)
 * 0x1 - 0xE: 1/16 of range per step
 * 0xF: threshold set at 15/16 of full range
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_CONTROL . MINWDTH
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_CONTROL_MINWDTH(x)  VTSS_ENCODE_BITFIELD(x,5,5)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_CONTROL_MINWDTH     VTSS_ENCODE_BITMASK(5,5)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_CONTROL_MINWDTH(x)  VTSS_EXTRACT_BITFIELD(x,5,5)

/** 
 * \brief
 * Minimum Amin interrupt threshold
 * This is an unsigned vector value (positive) defining the minimum
 * acceptable eye amplitude, as measured by the DFE logic. If the measured
 * Amin value falls below this value the EyeAmplitude Status bit will be
 * set in the status register (reg 0x1E, bit 6) and an EYEQUALITY interrupt
 * will be triggered.
 *
 * \details 
 * 0x0: 0 threshold (default, no interrupt set);
 * 0x1 - 0xE: 1/16 of range per step;
 * 0xF: threshold set at 15/16 of full range
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_CONTROL . MINAMP
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_CONTROL_MINAMP(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_CONTROL_MINAMP     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_CONTROL_MINAMP(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief HSS11G Rx digital eye metrics
 *
 * \details
 * This register is primarily used for diagnostic and test purposes.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_DIGITAL_EYE_METRICS
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_DIGITAL_EYE_METRICS (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_METRICS(target,ri)  VTSS_IOREG(target,0x658 + (ri))

/** 
 * \brief
 * EM_bump32 state ready
 *
 * \details 
 * 0: EM_bump32 state machine busy after being triggered by bit 7 
 * 1: EM_bump32 state machine is idle (ready) (default)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_METRICS . EMBRDY
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_METRICS_EMBRDY(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_METRICS_EMBRDY     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_METRICS_EMBRDY(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * EM_bump32
 * A rising edge of this bit triggers the phase rotator to advance to the
 * next bit position of the pattern. Bit 10 should be 1 before asserting
 * this bit.
 *
 * \details 
 * 0: Normal operation (default)
 * 1: Initiate a single-shot trigger to step 1 UI in pattern
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_METRICS . EMBUMP
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_METRICS_EMBUMP(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_METRICS_EMBUMP     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_METRICS_EMBUMP(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Eye metrics mode 
 * Defines eye metrics operating mode
 * Note: Scopemode is only supported in full-rate and half-rate modes.	    
 *
 * \details 
 * 00: Scopemode (default)
 * 01: Reserved
 * 10: Reserved
 * 11: Reserved
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_METRICS . EMMD
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_METRICS_EMMD(x)  VTSS_ENCODE_BITFIELD(x,5,2)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_METRICS_EMMD     VTSS_ENCODE_BITMASK(5,2)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_METRICS_EMMD(x)  VTSS_EXTRACT_BITFIELD(x,5,2)

/** 
 * \brief
 * Eye metrics pattern select
 * Indicates the length of the repeating test pattern applied to the Rx
 * input during the eye metrics test
 *
 * \details 
 * 0: 128 UI pattern length (default)
 * 1: 127 UI pattern length
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_METRICS . EMPAT
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_METRICS_EMPAT(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_METRICS_EMPAT     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_METRICS_EMPAT(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Eye metrics enable
 * Enables internal eye metrics functions
 *
 * \details 
 * 0: Eye measurement tools disabled (default)
 * 1: Eye measurement tools enabled
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_METRICS . EMEN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_METRICS_EMEN(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_METRICS_EMEN     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DIGITAL_EYE_METRICS_EMEN(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief HSS11G Rx DFE H1
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored. In normal operation, bit 4 of the DFE Function
 * Control register is set to 1, enabling the DFE algorithm to control this
 * register. This register can be read to obtain the status of the DFE H1
 * even and odd coefficients. To support diagnostic and manual override
 * capability, bit 4 of the DFE Function Control register can be set to 0
 * and this register can be written to directly control the DFE H1 even and
 * odd coefficients.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_DFE_H1
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_DFE_H1 (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_DFE_H1(target,ri)  VTSS_IOREG(target,0x65c + (ri))

/** 
 * \brief
 * DFE H1 odd sign
 * This sign represents the asserted sign of H1 odd as it is applied to the
 * A summer and changes under normal operation in DFE mode. The data path
 * implements both +H1 odd and -H1 odd paths (speculative mode), and
 * selects the correct version on a bit-by-bit basis in the analog front
 * end. 
 *
 * \details 
 * 00: Magnitude=0 or non-DFE mode or Testmode
 * 01: Positive
 * 10: Negative
 * 11: Invalid
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_H1 . H1OSN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_H1_H1OSN(x)  VTSS_ENCODE_BITFIELD(x,14,2)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_H1_H1OSN     VTSS_ENCODE_BITMASK(14,2)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_H1_H1OSN(x)  VTSS_EXTRACT_BITFIELD(x,14,2)

/** 
 * \brief
 * DACH1 odd
 * Magnitude of DFE H1 odd
 * Note: These register bits are only valid in DFE1, DFE3, and DFE5 modes,
 * and are disabled in other modes. When disabled, these register bits are
 * forced to 0 and cannot be written.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_H1 . H1OMAG
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_H1_H1OMAG(x)  VTSS_ENCODE_BITFIELD(x,8,6)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_H1_H1OMAG     VTSS_ENCODE_BITMASK(8,6)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_H1_H1OMAG(x)  VTSS_EXTRACT_BITFIELD(x,8,6)

/** 
 * \brief
 * DFE H1 even sign
 * This sign represents the asserted sign of H1 even as it is applied to
 * the A summer and changes under normal operation in DFE mode. The data
 * path implements both +H1 even and -H1 even paths (speculative mode), and
 * selects the correct version on a bit-by-bit basis in the analog front
 * end.
 *
 * \details 
 * 00: Magnitude=0 or non-DFE mode or Testmode
 * 01: Positive
 * 10: Negative
 * 11: Invalid
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_H1 . H1ESN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_H1_H1ESN(x)  VTSS_ENCODE_BITFIELD(x,6,2)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_H1_H1ESN     VTSS_ENCODE_BITMASK(6,2)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_H1_H1ESN(x)  VTSS_EXTRACT_BITFIELD(x,6,2)

/** 
 * \brief
 * DFE H1 even magnitude
 * Magnitude of DFE H1
 * Note: These register bits are only valid in DFE1, DFE3, and DFE5 modes,
 * and are disabled in other modes. When disabled, these register bits are
 * forced to 0 and cannot be written.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_H1 . H1EMAG
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_H1_H1EMAG(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_H1_H1EMAG     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_H1_H1EMAG(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief HSS11G Rx DFE H2
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored. In normal operation, bit 5 of the DFE Function
 * Control register is set to 1, enabling the DFE algorithm to control this
 * register. This register can be read to obtain the status of the DFE H2
 * coefficient. To support diagnostic and manual override capability, bit 5
 * of the DFE Function Control register can be set to 0 and this register
 * can be written to directly control the DFE H2 coefficient.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_DFE_H2
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_DFE_H2 (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_DFE_H2(target,ri)  VTSS_IOREG(target,0x660 + (ri))

/** 
 * \brief
 * DFE H2 odd sign
 * Note: These register bits are only valid in DFE3 and DFE5 modes, and are
 * disabled in other modes. When disabled, these register bits cannot be
 * written.
 *
 * \details 
 * 00: Invalid
 * 01: Positive
 * 10: Negative or test mode
 * 11: Magnitude=0 or H2 disabled
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_H2 . H2OSN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_H2_H2OSN(x)  VTSS_ENCODE_BITFIELD(x,13,2)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_H2_H2OSN     VTSS_ENCODE_BITMASK(13,2)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_H2_H2OSN(x)  VTSS_EXTRACT_BITFIELD(x,13,2)

/** 
 * \brief
 * DFE H2 odd magnitude
 * Magnitude of DFE H2 odd
 * Note: These register bits are only valid in DFE3 and DFE5 modes and are
 * disabled in other modes. When disabled, these register bits are forced
 * to 0 and cannot be written.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_H2 . H2OMAG
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_H2_H2OMAG(x)  VTSS_ENCODE_BITFIELD(x,8,5)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_H2_H2OMAG     VTSS_ENCODE_BITMASK(8,5)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_H2_H2OMAG(x)  VTSS_EXTRACT_BITFIELD(x,8,5)

/** 
 * \brief
 * DFE H2 even sign
 * Note: These register bits are only valid in DFE3 and DFE5 modes, and are
 * disabled in other modes. When disabled, these register bits cannot be
 * written.
 *
 * \details 
 * 00: Invalid
 * 01: Positive
 * 10: Negative or test mode
 * 11: Magnitude=0 or H2 disabled
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_H2 . H2ESN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_H2_H2ESN(x)  VTSS_ENCODE_BITFIELD(x,5,2)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_H2_H2ESN     VTSS_ENCODE_BITMASK(5,2)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_H2_H2ESN(x)  VTSS_EXTRACT_BITFIELD(x,5,2)

/** 
 * \brief
 * DFE H2 even magnitude
 * Magnitude of DFE H2 even
 * Note: These register bits are only valid in DFE3 and DFE5 modes, and are
 * disabled in other modes. When disabled, these register bits are forced
 * to 0 and cannot be written.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_H2 . H2EMAG
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_H2_H2EMAG(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_H2_H2EMAG     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_H2_H2EMAG(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief HSS11G Rx DFE H3
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored. In normal operation, bit 5 of the DFE Function
 * Control register is set to 1, enabling the DFE algorithm to control this
 * register. This register can be read to obtain the status of the DFE H3
 * coefficients. To support diagnostic and manual override capability, bit
 * 5 of the DFE Function Control register can be set to 0 and this register
 * can be written to directly control the DFE H3 coefficients.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_DFE_H3
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_DFE_H3 (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_DFE_H3(target,ri)  VTSS_IOREG(target,0x664 + (ri))

/** 
 * \brief
 * DFE H3 odd sign
 * Note: These register bits are only valid in DFE3 and DFE5 modes, and are
 * disabled in other modes. When disabled, these register bits cannot be
 * written.
 *
 * \details 
 * 00: Invalid
 * 01: Positive
 * 10: Negative or test mode
 * 11: Magnitude=0 or H3 disabled
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_H3 . H3OSN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_H3_H3OSN(x)  VTSS_ENCODE_BITFIELD(x,12,2)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_H3_H3OSN     VTSS_ENCODE_BITMASK(12,2)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_H3_H3OSN(x)  VTSS_EXTRACT_BITFIELD(x,12,2)

/** 
 * \brief
 * DFE H3 odd magnitude
 * Magnitude of DFE H3 odd.
 * Note: These register bits are only valid in DFE3 and DFE5 modes, and are
 * disabled in other modes. When disabled, these register bits are forced
 * to 0 and cannot be written.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_H3 . H3OMAG
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_H3_H3OMAG(x)  VTSS_ENCODE_BITFIELD(x,8,4)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_H3_H3OMAG     VTSS_ENCODE_BITMASK(8,4)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_H3_H3OMAG(x)  VTSS_EXTRACT_BITFIELD(x,8,4)

/** 
 * \brief
 * DFE H3 even sign
 * Note: These register bits are only valid in DFE3 and DFE5 modes, and are
 * disabled in other modes. When disabled, these register bits cannot be
 * written.
 *
 * \details 
 * 00: Invalid
 * 01: Positive
 * 10: Negative or test mode
 * 11: Magnitude=0 or H3 disabled
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_H3 . H3ESN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_H3_H3ESN(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_H3_H3ESN     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_H3_H3ESN(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * DFE H3 even magnitude
 * Magnitude of DFE H3 even
 * Note: These register bits are only valid in DFE3 and DFE5 modes, and are
 * disabled in other modes. When disabled, these register bits are forced
 * to 0 and cannot be written.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_H3 . H3EMAG
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_H3_H3EMAG(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_H3_H3EMAG     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_H3_H3EMAG(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief HSS11G Rx DFE H4
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored. In normal operation, bit 6 of the DFE Function
 * Control register is set to 1, enabling the DFE algorithm to control this
 * register. This register can be read to obtain the status of the DFE H4
 * coefficients. To support diagnostic and manual override capability, bit
 * 6 of the DFE Function Control register can be set to 0 and this register
 * can be written to directly control the DFE H4 coefficients.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_DFE_H4
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_DFE_H4 (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_DFE_H4(target,ri)  VTSS_IOREG(target,0x668 + (ri))

/** 
 * \brief
 * DFE H4 odd sign
 * Note: These register bits are only valid in DFE5 mode and are disabled
 * in other modes. When disabled, these register bits cannot be written.
 *
 * \details 
 * 00: Invalid
 * 01: Positive
 * 10: Negative or test mode
 * 11: Magnitude=0 or H4 disabled
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_H4 . H4OSN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_H4_H4OSN(x)  VTSS_ENCODE_BITFIELD(x,12,2)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_H4_H4OSN     VTSS_ENCODE_BITMASK(12,2)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_H4_H4OSN(x)  VTSS_EXTRACT_BITFIELD(x,12,2)

/** 
 * \brief
 * DFE H4 odd magnitude
 * Magnitude of DFE H4 odd
 * Note: These register bits are only valid in DFE5 mode and are disabled
 * in other modes. When disabled, these register bits are forced to 0 and
 * cannot be written.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_H4 . H4OMAG
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_H4_H4OMAG(x)  VTSS_ENCODE_BITFIELD(x,8,4)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_H4_H4OMAG     VTSS_ENCODE_BITMASK(8,4)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_H4_H4OMAG(x)  VTSS_EXTRACT_BITFIELD(x,8,4)

/** 
 * \brief
 * DFE H4 even sign
 * Note: These register bits are only valid in DFE5 mode and are disabled
 * in other modes. When disabled, these register bits cannot be written.
 *
 * \details 
 * 00: Invalid
 * 01: Positive
 * 10: Negative or test mode
 * 11: Magnitude=0 or H4 disabled
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_H4 . H4ESN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_H4_H4ESN(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_H4_H4ESN     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_H4_H4ESN(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * DFE H4 even magnitude
 * Magnitude of DFE H4 even.
 * Note: These register bits are only valid in DFE5 mode and are disabled
 * in other modes. When disabled, these register bits are forced to 0 and
 * cannot be written.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_H4 . H4EMAG
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_H4_H4EMAG(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_H4_H4EMAG     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_H4_H4EMAG(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief HSS11G RX DFE H5
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored. In normal operation, bit 6 of the DFE Function
 * Control register is set to 1, enabling the DFE algorithm to control this
 * register. This register can be read to obtain the status of the DFE H5
 * coefficients. To support diagnostic and manual override capability, bit
 * 6 of the DFE Function Control register can be set to 0 and this register
 * can be written to directly control the DFE H5 coefficients.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_DFE_H5
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_DFE_H5 (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_DFE_H5(target,ri)  VTSS_IOREG(target,0x66c + (ri))

/** 
 * \brief
 * DFE H5 odd sign
 * Note: These register bits are only valid in DFE5 mode and are disabled
 * in other modes. When disabled, these register bits cannot be written.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_H5 . H5OSN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_H5_H5OSN(x)  VTSS_ENCODE_BITFIELD(x,12,2)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_H5_H5OSN     VTSS_ENCODE_BITMASK(12,2)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_H5_H5OSN(x)  VTSS_EXTRACT_BITFIELD(x,12,2)

/** 
 * \brief
 * DFE H5 odd magnitude
 * Magnitude of DFE H5 odd.
 * Note: These register bits are only valid in DFE5 mode and are disabled
 * in other modes. When disabled, these register bits are forced to 0 and
 * cannot be written.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_H5 . H5OMAG
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_H5_H5OMAG(x)  VTSS_ENCODE_BITFIELD(x,8,4)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_H5_H5OMAG     VTSS_ENCODE_BITMASK(8,4)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_H5_H5OMAG(x)  VTSS_EXTRACT_BITFIELD(x,8,4)

/** 
 * \brief
 * DFE H5 even sign
 * Note: These register bits are only valid in DFE5 mode and are disabled
 * in other modes. When disabled, these register bits cannot be written.
 *
 * \details 
 * 00: Invalid
 * 01: Positive
 * 10: Negative or test mode
 * 11: Magnitude=0 or H5 disabled
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_H5 . H5ESN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_H5_H5ESN(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_H5_H5ESN     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_H5_H5ESN(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * DFE H5 even magnitude
 * Magnitude of DFE H5 even.
 * Note: These register bits are only valid in DFE5 mode and are disabled
 * in other modes. When disabled, these register bits are forced to 0 and
 * cannot be written.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_H5 . H5EMAG
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_H5_H5EMAG(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_H5_H5EMAG     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_H5_H5EMAG(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief HSS11G Rx dynamic amplitude centering and dynamic peaking control
 *
 * \details
 * This dynamic amplitude centering (DAC) and dynamic peaking control (DPC)
 * register is reserved for diagnostic and test purposes. It can usually be
 * ignored.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC(target,ri)  VTSS_IOREG(target,0x670 + (ri))

/** 
 * \brief
 * DPC convergence complete
 * This bit also sets status bit DPCCMP in HSS11G_RECEIVER_INTERNAL_STATUS
 *
 * \details 
 * 1: DPC algorithm has converged, and applied the necessary PEAK level to
 * converge the DAC H1_target equal to the DPC_H1_target
 * 0: DPC incomplete.
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC . DPCCVG
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC_DPCCVG(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC_DPCCVG     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC_DPCCVG(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * DAC convergence complete
 * This bit also sets status bit DACCMP in HSS11G_RECEIVER_INTERNAL_STATUS
 *
 * \details 
 * 1 = DAC has established the H1 target range, and the DFE H1 is converged
 * within range.
 * 0 = DAC incomplete. DFE H1 may not be in optimal range yet.
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC . DACCVG
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC_DACCVG(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC_DACCVG     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC_DACCVG(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Dynamic peaking control H1 target range
 * This specifies the desired target range for H1. When enabled, the
 * dynamic peaking control (FDPC in HSS11G_DFE_FUNCTION_CONTROL) and
 * dynamic amplitude centering (FDAC in HSS11G_DFE_FUNCTION_CONTROL) work
 * together, adjusting the peaking control (HSS11G_RECEIVER_VGA_CONTROL_1
 * bit(10:8)) to drive H1 toward this target. 
 *
 * \details 
 * Target range settings:
 * 000: 0.000 < H1 <= 0.125
 * 001: 0.000 < H1 <= 0.125
 * 010: 0.125 < H1 <= 0.250
 * 011: 0.250 < H1 <= 0.375
 * 100: 0.375 < H1 <= 0.500 (default)
 * 101: 0.500 < H1 <= 0.625
 * 110: 0.625 < H1 <= 0.750
 * 111: 0.750 < H1 <= 0.875
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC . DPCTGT
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC_DPCTGT(x)  VTSS_ENCODE_BITFIELD(x,9,3)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC_DPCTGT     VTSS_ENCODE_BITMASK(9,3)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC_DPCTGT(x)  VTSS_EXTRACT_BITFIELD(x,9,3)

/** 
 * \brief
 * Block_H1TARGET updates control
 *
 * \details 
 * 0: Normal operation (default)
 * 1: Block internal updates to H1_TARGET_RANGE_reg (bits 6:4)	      
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC . BLKH1T
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC_BLKH1T(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC_BLKH1T     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC_BLKH1T(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Block_OAE updates control
 *
 * \details 
 * 0: Normal operation (default)
 * 1: Block internal updates to OAE_C_DFE_reg (bits 3:0)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC . BLKOAE
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC_BLKOAE(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC_BLKOAE     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC_BLKOAE(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * C or C,R,L,D 
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC . H1TGT
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC_H1TGT(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC_H1TGT     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC_H1TGT(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * C or C,R,L,D
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC . OAE
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC_OAE(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC_OAE     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC_OAE(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief HSS11G Rx dynamic data centering (DDC)
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored. It is dynamic in normal operation. FDDC in
 * HSS11G_DFE_FUNCTION_CONTROL should be cleared before this register can
 * be read/written reliably. 
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_DYNAMIC_DATA_CENTERING_DDC
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_DYNAMIC_DATA_CENTERING_DDC (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_DYNAMIC_DATA_CENTERING_DDC(target,ri)  VTSS_IOREG(target,0x674 + (ri))

/** 
 * \brief
 * OLscan(4:0) in the DFE logic
 * This register contains the LateScan offset value determined by the DDC
 * algorithm. This is a positive unsigned vector value representing the
 * rotator step count from data eye center to the right edge of the eye as
 * measured at amplitude level Az (also determined by the DDC algorithm).
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DYNAMIC_DATA_CENTERING_DDC . OLS
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DYNAMIC_DATA_CENTERING_DDC_OLS(x)  VTSS_ENCODE_BITFIELD(x,11,5)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DYNAMIC_DATA_CENTERING_DDC_OLS     VTSS_ENCODE_BITMASK(11,5)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DYNAMIC_DATA_CENTERING_DDC_OLS(x)  VTSS_EXTRACT_BITFIELD(x,11,5)

/** 
 * \brief
 * OEscan(4:0) in the DFE logic
 * This register contains the EarlyScan offset value determined by the DDC
 * algorithm. This is a positive unsigned vector value representing the
 * rotator step count from data eye center to the left edge of the eye as
 * measured at amplitude level Az (also determined by the DDC algorithm).
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DYNAMIC_DATA_CENTERING_DDC . OES
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DYNAMIC_DATA_CENTERING_DDC_OES(x)  VTSS_ENCODE_BITFIELD(x,6,5)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DYNAMIC_DATA_CENTERING_DDC_OES     VTSS_ENCODE_BITMASK(6,5)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DYNAMIC_DATA_CENTERING_DDC_OES(x)  VTSS_EXTRACT_BITFIELD(x,6,5)

/** 
 * \brief
 * Block_ODEC updates control
 *
 * \details 
 * 0: Enabled
 * 1: Block internal updates to DDC_offset_ODEC_reg (default) to prevent
 * the results of DDC from affecting the data path sampling clock offset
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DYNAMIC_DATA_CENTERING_DDC . BLKODEC
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DYNAMIC_DATA_CENTERING_DDC_BLKODEC(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DYNAMIC_DATA_CENTERING_DDC_BLKODEC     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DYNAMIC_DATA_CENTERING_DDC_BLKODEC(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * ODEC_reg(4:0) in the DFE logic
 * This register contains the Data rotator offset compensation value
 * determined by the DDC algorithm. This is a signed vector representing
 * the offset adjustment added to the data rotator relative to its nominal
 * offset (offset_H1/An in DFE mode). It is dynamic in normal operation
 * with FDDC enabled in HSS11G_DFE_FUNCTION_CONTROL and cannot be written.
 * DDC should be disabled before this register can be read/written
 * reliably.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DYNAMIC_DATA_CENTERING_DDC . ODEC
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DYNAMIC_DATA_CENTERING_DDC_ODEC(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DYNAMIC_DATA_CENTERING_DDC_ODEC     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DYNAMIC_DATA_CENTERING_DDC_ODEC(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief HSS11G Rx receiver internal status
 *
 * \details
 * This register is primarily used for diagnostic and test purposes. This
 * register is used to report the status of certain internal operations.
 * HSSEYEQUALITY will be asserted each time any of bits 3 through 9
 * transition from 0 to 1 by internal logic. After receiving this signal,
 * the system logic reads this register to determine the cause and take
 * action. The HSSEYEQUALITY interrupt signal is cleared by any write
 * operation to this register (the data does not matter). 
 * There are separate status registers for each receiver port in a core (up
 * to 8). There is an HSSEYEQUALITY latch associated with each of these
 * registers that gets set to 1 any time bits 3 through 9 of that register
 * transition from a 0 to a 1. The HSSEYEQUALITY Interrupt signal is the
 * logical OR of all of these latches. Therefore, it may be necessary to
 * write to more than one of these Internal Status registers to clear the
 * HSSEYEQUALITY Interrupt signal.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_RECEIVER_INTERNAL_STATUS
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_RECEIVER_INTERNAL_STATUS (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS(target,ri)  VTSS_IOREG(target,0x678 + (ri))

/** 
 * \brief
 * BER6 mode status
 * When FDDC and FBER6 in HSS11G_DFE_FUNCTION_CONTROL are both 1, this bit
 * indicates the current state of the automatically selected DDC bit error
 * confidence level.
 * Note: This bit will not affect the HSSEYEQUALITY output signal.
 *
 * \details 
 * 0: 1E-3 (reset state, and after detection of >1E-3 error bursts in 1E-6
 * mode)
 * 1: 1E-6 (automatically selected after initial 1E-3 edges are located)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS . BER6
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_BER6(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_BER6     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_BER6(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * BER6 valid
 * When the 1E-6 confidence level of the DDC scan is enabled, this bit
 * indicates when the first valid measurement better than 1E-6 has been
 * achieved on both early and late edges.
 * Note: This bit will not affect the HSSEYEQUALITY output signal.
 *
 * \details 
 * 0: Invalid (an error free 1E-6 sample interval has not yet been
 * achieved)
 * 1: Valid (an error free 1E-6 sample interval has been achieved)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS . BER6VAL
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_BER6VAL(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_BER6VAL     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_BER6VAL(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * BER3 valid
 * This bit indicates when the first valid measurement better than 1E- 3
 * has been achieved on both early and late edges.
 * Note: This bit will not affect the HSSEYEQUALITY output signal.
 *
 * \details 
 * 0: Invalid: the 1E-3 eye edges have not yet been located.
 * 1: Valid: the 1E-3 eye edges have been located on both early and late
 * sides of the eye. If FBER6 in HSS11G_DFE_FUNCTION_CONTROL is enabled,
 * then 1E-6 confidence scanning will now begin.
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS . BER3VAL
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_BER3VAL(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_BER3VAL     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_BER3VAL(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * DPC convergence complete
 * This is the latched status of DPC as described in DPCCVG in
 * HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC.
 * This bit can be cleared by writing a 1 to this bit position. It can then
 * be read to confirm clear, or to obtain new interrupt status. The current
 * state of DPC convergence can be read in DPCCVG in
 * HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS . DPCCMP
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_DPCCMP(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_DPCCMP     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_DPCCMP(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * DAC convergence complete
 * This is the latched status of DAC as described in DACCVG in
 * HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC.
 * This bit can be cleared by writing a 1 to this bit position. It can then
 * be read to confirm clear, or to obtain new interrupt status. The current
 * state of DAC convergence can be read in DACCVG in
 * HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC.
 *
 * \details 
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS . DACCMP
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_DACCMP(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_DACCMP     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_DACCMP(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * DDC convergence complete
 * This bit can be cleared by writing a 1 to this bit position. It can then
 * be read to confirm clear, or to obtain new interrupt status. The current
 * state of DDC convergence can be read in bits 15:13 of this register.
 *
 * \details 
 * 0: DDC incomplete. Data sampling offset may not be at the optimal value.
 * 1: DDC has located both early and late edges to the selected confidence
 * level (FBER6 in HSS11G_DFE_FUNCTION_CONTROL), and computed the offset
 * adjustment needed to center the data sampling point within this scan
 * range.
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS . DDCCMP
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_DDCCMP(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_DDCCMP     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_DDCCMP(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Eye amplitude error flag
 * This bit can be cleared by writing a 1 to this bit position. It can then
 * be read to confirm clear, or to obtain new interrupt status. This
 * interrupt can be masked by setting the threshold to 0x00 (default) in
 * MINIAMP in HSS11G_DIGITAL_EYE_CONTROL. 
 *
 * \details 
 * 0: Normal: Measured data eye height at or above interrupt threshold set
 * in HSS11G_DIGITAL_EYE_CONTROL
 * 1: Error: Measured data eye height below interrupt threshold set in
 * HSS11G_DIGITAL_EYE_CONTROL
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS . AERRFLG
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_AERRFLG(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_AERRFLG     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_AERRFLG(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Eye width error flag
 * This bit can be cleared by writing a 1 to this bit position. It can then
 * be read to confirm clear, or to obtain new interrupt status. This
 * interrupt can be masked by setting the threshold to 0x00 (default) in
 * MINWDTH in HSS11G_DIGITAL_EYE_CONTROL    
 *
 * \details 
 * 0: Normal: Measured data eye width at or above interrupt threshold set
 * in HSS11G_DIGITAL_EYE_CONTROL
 * 1 = Error: Measured data eye width below interrupt threshold set in
 * HSS11G_DIGITAL_EYE_CONTROL
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS . WERRFLG
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_WERRFLG(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_WERRFLG     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_WERRFLG(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * DFE training complete
 * This register is set when the DFE logic determines that its H
 * coefficients have converged since the last reset. The DFE process runs
 * continuously and these H values may change over time. This flag will
 * only assert when configured for DFE mode. This flag only indicates
 * initial convergence after reset and does not necessarily indicate the
 * current status.
 *
 * \details 
 * 0: Initial DFE convergence not yet achieved
 * 1: Initial DFE convergence achieved
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS . TRCMP
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_TRCMP(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_TRCMP     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_TRCMP(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * VGA locked first
 * This register is set when the VGA achieves lock and is cleared only by
 * reset. It indicates lock achieved at least once. This bit is similar to
 * VLOCKDF2 in HSS11G_RECEIVER_VGA_CONTROL_3, but is read-only. If
 * configured for DFE mode, the DFE algorithm can commence training the H
 * coefficients. During normal operation the VGA gain may unlock and relock
 * at a different gain setting. This activity is not reflected in the state
 * of this bit. It is latched the first time the VGA gain is locked after a
 * reset.
 *
 * \details 
 * 0: VGA has not converged since reset
 * 1: VGA has converged on its initial gain setting
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS . VLCKF
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_VLCKF(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_VLCKF     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_VLCKF(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Phase rotator offset calibration complete (adjacent link)
 * When this link and the adjacent link both reach this point, they operate
 * independently of each other. 
 *
 * \details 
 * 0: Calibration not completed
 * 1: Phase rotator offset calibration process is completed. The VGA gain
 * and offset calibration begin next.
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS . ROCADJ
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_ROCADJ(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_ROCADJ     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_ROCADJ(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Phase rotator offset calibration complete (this link)
 * When this link and the adjacent link both reach this point, they operate
 * independently of each other. 
 *
 * \details 
 * 0: Calibration not completed
 * 1: Phase rotator offset calibration process is completed. The VGA gain
 * and offset calibration begin next.
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS . ROCCMP
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_ROCCMP(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_ROCCMP     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_ROCCMP(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * DC offset calibration complete
 *
 * \details 
 * 0: Offset calibration not completed and the internal shortAGC signal is
 * asserted, preventing data from entering the Rx
 * 1: Internal DC calibration completed. At this point, the internal
 * shortAGC signal is de-asserted and data can flow into the Rx. The phase
 * rotator calibration can begin.
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS . OCCMP
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_OCCMP(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_OCCMP     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_RECEIVER_INTERNAL_STATUS_OCCMP(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief HSS11G Rx DFE function control
 *
 * \details
 * This register is reserved for diagnostic and test purposes. It can
 * usually be ignored. It allows the turning on and off of various
 * functions. These control bits typically enable internal control of
 * various registers. These registers may still be written by the user, but
 * the values will be quickly overwritten by the internal processes unless
 * these control bits are set correctly.
 *
 * Register: \a XFI::HSS11G_RX::HSS11G_DFE_FUNCTION_CONTROL
 *
 * @param target A \a ::vtss_target_XFI_e target
 * @param ri Register: HSS11G_DFE_FUNCTION_CONTROL (??), 0-3
 */
#define VTSS_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL(target,ri)  VTSS_IOREG(target,0x67c + (ri))

/** 
 * \brief
 * Func_DPC
 * Dynamic peaking control (DPC) enable. HSS11G_RECEIVER_VGA_CONTROL_1 and
 * HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC
 * are affected by this control bit.
 *
 * \details 
 * 0: Disabled (default)
 * 1: Enabled
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL . FDPC
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FDPC(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FDPC     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FDPC(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Func_DAC
 * Dynamic amplitude centering (DAC) enable.
 * HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC
 * and HSS11G_DYNAMIC_DATA_CENTERING_DDC are affected by this control bit.
 *
 * \details 
 * 0: Disabled
 * 1: Enable DAC algorithm (default)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL . FDAC
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FDAC(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FDAC     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FDAC(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * Func_DDC
 * Dynamic data centering (DDC) enable. HSS11G_DYNAMIC_DATA_CENTERING_DDC
 * is affected by this control bit.
 *
 * \details 
 * 0: Disabled (default)
 * 1: Enabled 
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL . FDDC
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FDDC(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FDDC     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FDDC(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Func not random 
 * Registers HSS11G_DFE_CONTROL and HSS11G_RECEIVER_VGA_CONTROL_3 are
 * affected by this control bit.
 *
 * \details 
 * 0: Disable internal logic control of Not Random and VGA Locked First
 * registers
 * 1: Enable internal logic control of Not Random and VGA Locked First
 * registers (default)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL . FNRND
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FNRND(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FNRND     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FNRND(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Func VGA cain 
 * HSS11G_RECEIVER_VGA_CONTROL_2 is affected by this control bit.
 *
 * \details 
 * 0: Disable internal logic control of VGA Gain register
 * 1: Enable internal logic control of VGA Gain register (default)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL . FVGAIN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FVGAIN(x)  VTSS_ENCODE_BITFIELD(x,11,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FVGAIN     VTSS_ENCODE_BITMASK(11,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FVGAIN(x)  VTSS_EXTRACT_BITFIELD(x,11,1)

/** 
 * \brief
 * Func VGA offset 
 * HSS11G_RECEIVER_VGA_CONTROL_1 is affected by this control bit.
 *
 * \details 
 * 0: Disable internal logic control of VGA Offset register
 * 1: Enable internal logic control of VGA Offset register (default)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL . FVOFF
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FVOFF(x)  VTSS_ENCODE_BITFIELD(x,10,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FVOFF     VTSS_ENCODE_BITMASK(10,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FVOFF(x)  VTSS_EXTRACT_BITFIELD(x,10,1)

/** 
 * \brief
 * Func SigDet 
 * HSS11G_RECEIVER_SIGDET_CONTROL is affected by this control bit.
 *
 * \details 
 * 0: Disable internal logic control of SigDet register
 * 1: Enable internal logic control of SigDet register (default)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL . FSDET
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FSDET(x)  VTSS_ENCODE_BITFIELD(x,9,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FSDET     VTSS_ENCODE_BITMASK(9,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FSDET(x)  VTSS_EXTRACT_BITFIELD(x,9,1)

/** 
 * \brief
 * Func_BER6
 * DDC scan error confidence control
 *
 * \details 
 * 0: Force DDC scan error confidence to remain at its initial 1E-3 level
 * 1: Allow confidence level to automatically shift to 1E-6 after locating
 * 1E-3 edge locations. This will cause DDC algorithm to run 16x more
 * often, and will increase the sampling time by 1024x. The status of the
 * 1E-6 confidence level can be read in HSS11G_RECEIVER_INTERNAL STATUS
 * bits 15:14.
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL . FBER6
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FBER6(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FBER6     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FBER6(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Func rotator offset 
 * HSS11G_PHASE_ROTATOR_OFFSET_CONTROL is affected by this control bit.
 *
 * \details 
 * 0: Disable internal logic control of phase rotator offsets
 * 1: Enable internal logic control of phase rotator offsets (default)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL . FROTO
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FROTO(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FROTO     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FROTO(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * Func H4H5 
 * HSS11G_DFE_H4 and HSS11G_DFE_H5 are affected by this control bit.
 *
 * \details 
 * 0: Disable internal logic control of DacH4 and DacH5 coefficients
 * 1: Enable internal logic control of DacH4 and DacH5 coefficients
 * (default)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL . FH4H5
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FH4H5(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FH4H5     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FH4H5(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Func H2H3 
 * HSS11G_DFE_H2 and HSS11G_DFE_H3 are affected by this control bit.
 *
 * \details 
 * 0: Disable internal logic control of DacH2 and DacH3 coefficients
 * 1: Enable internal logic control of DacH2 and DacH3 coefficients
 * (default)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL . FH2H3
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FH2H3(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FH2H3     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FH2H3(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Func H1
 * HSS11G_DFE_H1 is affected by this control bit.
 *
 * \details 
 * 0: Disable internal logic control of DacH1 coefficient
 * 1: Enable internal logic control of DacH1 coefficient (default).
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL . FH1
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FH1(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FH1     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FH1(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Func H1 sign 
 * HSS11G_DFE_H1 is affected by this control bit.
 *
 * \details 
 * 0: Disable H1 feedback on A summer (scope mode only)
 * 1: Enable H1 feedback on A summer (normal operation). (default)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL . FH1SN
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FH1SN(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FH1SN     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FH1SN(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Func not random filter
 *
 * \details 
 * 0: Run DFE training normally, regardless of NONRNF in HSS11G_DFE_CONTROL
 * 1: Disable DFE training when NONRNF= 1 (default) in HSS11G_DFE_CONTROL 
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL . FNRDF
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FNRDF(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FNRDF     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FNRDF(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Func ADAC 
 * HSS11G_RECEIVER_ADAC_CONTROL is affected by this control bit.
 *
 * \details 
 * 0: Disable internal logic control of DACA1 coefficient
 * 1: Enable internal logic control of DACA1 coefficient (default)
 *
 * Field: ::VTSS_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL . FADAC
 */
#define  VTSS_F_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FADAC(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FADAC     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL_FADAC(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/**
 * Register Group: \a XFI::UPI_RX_OTN_FRMR
 *
 * UPI Rx OTN framer configuration and status
 */


/** 
 * \brief UPI Rx OTN framer enable
 *
 * \details
 * UPI Rx OTN framer enable
 *
 * Register: \a XFI::UPI_RX_OTN_FRMR::UPI_RX_OTN_FRMR_ENA
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_ENA(target)  VTSS_IOREG(target,0x700)

/** 
 * \brief
 * Enables the UPI Rx OTN Framer
 *
 * \details 
 * 0: Framer is bypassed (monitoring only)
 * 1: Framer is enabled to be the source of downstream data
 *
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_ENA . FRMR_ENA
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_ENA_FRMR_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_ENA_FRMR_ENA     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_ENA_FRMR_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief UPI Rx OTN frame alignment control
 *
 * \details
 * Frame alignment control
 *
 * Register: \a XFI::UPI_RX_OTN_FRMR::UPI_RX_FA_CTRL1
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL1(target)  VTSS_IOREG(target,0x701)

/** 
 * \brief
 * Forces the OOF state
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL1 . FORCE_OOF
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL1_FORCE_OOF(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL1_FORCE_OOF     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL1_FORCE_OOF(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Forces the LOF state
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL1 . FORCE_LOF
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL1_FORCE_LOF(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL1_FORCE_LOF     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL1_FORCE_LOF(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Number of frames (time in the OOF state) to enter the LOF state. 
 * Default is 3 ms.
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL1 . COUNT_TO_ENTER_LOF
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL1_COUNT_TO_ENTER_LOF(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL1_COUNT_TO_ENTER_LOF     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL1_COUNT_TO_ENTER_LOF(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \brief UPI Rx OTN frame alignment control 2
 *
 * \details
 * Frame alignment control
 *
 * Register: \a XFI::UPI_RX_OTN_FRMR::UPI_RX_FA_CTRL2
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL2(target)  VTSS_IOREG(target,0x702)

/** 
 * \brief
 * Number of consecutive frames to enter the OOF state (from the IF state).
 * 
 * Default is 5 frames.
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL2 . COUNT_TO_ENTER_OOF
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL2_COUNT_TO_ENTER_OOF(x)  VTSS_ENCODE_BITFIELD(x,12,4)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL2_COUNT_TO_ENTER_OOF     VTSS_ENCODE_BITMASK(12,4)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL2_COUNT_TO_ENTER_OOF(x)  VTSS_EXTRACT_BITFIELD(x,12,4)

/** 
 * \brief
 * Number of frames (continuous time in the IF state) to exit the LOF
 * state.
 * Default is 3 ms.
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL2 . COUNT_TO_EXIT_LOF
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL2_COUNT_TO_EXIT_LOF(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL2_COUNT_TO_EXIT_LOF     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_FA_CTRL2_COUNT_TO_EXIT_LOF(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \brief UPI Rx OTN multiframe alignment control
 *
 * \details
 * Multiframe alignment control
 *
 * Register: \a XFI::UPI_RX_OTN_FRMR::UPI_RX_MFA_CTRL1
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL1(target)  VTSS_IOREG(target,0x703)

/** 
 * \brief
 * Forces the OOM state
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL1 . FORCE_OOM
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL1_FORCE_OOM(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL1_FORCE_OOM     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL1_FORCE_OOM(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Forces the LOM state
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL1 . FORCE_LOM
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL1_FORCE_LOM(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL1_FORCE_LOM     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL1_FORCE_LOM(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Number of frames (time in the LOM state) to enter the LOM state.
 * Default is 3 ms.
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL1 . COUNT_TO_ENTER_LOM
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL1_COUNT_TO_ENTER_LOM(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL1_COUNT_TO_ENTER_LOM     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL1_COUNT_TO_ENTER_LOM(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \brief UPI Rx OTN multiframe alignment control 2
 *
 * \details
 * Multiframe alignment control
 *
 * Register: \a XFI::UPI_RX_OTN_FRMR::UPI_RX_MFA_CTRL2
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL2(target)  VTSS_IOREG(target,0x704)

/** 
 * \brief
 * Number of consecutive frames to enter the OOM state (from the IM state).
 * Default is 5 frames.
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL2 . COUNT_TO_ENTER_OOM
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL2_COUNT_TO_ENTER_OOM(x)  VTSS_ENCODE_BITFIELD(x,12,4)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL2_COUNT_TO_ENTER_OOM     VTSS_ENCODE_BITMASK(12,4)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL2_COUNT_TO_ENTER_OOM(x)  VTSS_EXTRACT_BITFIELD(x,12,4)

/** 
 * \brief
 * Number of frames (continuos time in the IM state) to exit the LOM state.
 * Default is 0 ms.
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL2 . COUNT_TO_EXIT_LOM
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL2_COUNT_TO_EXIT_LOM(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL2_COUNT_TO_EXIT_LOM     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_MFA_CTRL2_COUNT_TO_EXIT_LOM(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief Out of frame counter
 *
 * \details
 * Count the number of times the OOF state was entered
 *
 * Register: \a XFI::UPI_RX_OTN_FRMR::UPI_RX_OOF_CNTR
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OOF_CNTR(target)  VTSS_IOREG(target,0x705)

/** 
 * \brief
 * OOF counter value (saturates at the maximum value)
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OOF_CNTR . UPI_RX_OOF_CNTR
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_OOF_CNTR_UPI_RX_OOF_CNTR(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OOF_CNTR_UPI_RX_OOF_CNTR     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_OOF_CNTR_UPI_RX_OOF_CNTR(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Out of multiframe counter
 *
 * \details
 * Count the number of times the OOM state was entered
 *
 * Register: \a XFI::UPI_RX_OTN_FRMR::UPI_RX_OOM_CNTR
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OOM_CNTR(target)  VTSS_IOREG(target,0x706)

/** 
 * \brief
 * OOM counter value (saturates at the maximum value)
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OOM_CNTR . UPI_RX_OOM_CNTR
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_OOM_CNTR_UPI_RX_OOM_CNTR(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OOM_CNTR_UPI_RX_OOM_CNTR     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_OOM_CNTR_UPI_RX_OOM_CNTR(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Status bits
 *
 * \details
 * Bit encoding for each status condition:
 * 0: Status condition not present
 * 1: Status condition present
 *
 * Register: \a XFI::UPI_RX_OTN_FRMR::UPI_RX_OTN_FRMR_STATUS
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS(target)  VTSS_IOREG(target,0x707)

/** 
 * \brief
 * Status bit from the OTN framer indicates the current LOM state
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS . LOM_STATUS
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS_LOM_STATUS(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS_LOM_STATUS     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS_LOM_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Status bit from the OTN framer indicates the current OOM state
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS . OOM_STATUS
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS_OOM_STATUS(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS_OOM_STATUS     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS_OOM_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Status bit from the OTN framer indicates the current LOF state
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS . LOF_STATUS
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS_LOF_STATUS(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS_LOF_STATUS     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS_LOF_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Status bit from the OTN framer indicates the current OOF state
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS . OOF_STATUS
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS_OOF_STATUS(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS_OOF_STATUS     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STATUS_OOF_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Status bits (sticky) for interrupt generation
 *
 * \details
 * Bit encoding for each interrupt source:
 * 0: No sticky condition detected
 * 1: Sticky condition detected
 *
 * Register: \a XFI::UPI_RX_OTN_FRMR::UPI_RX_OTN_FRMR_STICKY
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY(target)  VTSS_IOREG(target,0x708)

/** 
 * \brief
 * Sticky bit from the OTN framer indicates a change in the frame alignment
 * position
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY . FAS_POS_STICKY
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_FAS_POS_STICKY(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_FAS_POS_STICKY     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_FAS_POS_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Sticky bit from the OTN framer indicates a change in LOM state
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY . LOM_STICKY
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_LOM_STICKY(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_LOM_STICKY     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_LOM_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Sticky bit from the OTN framer indicates a change in OOM state
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY . OOM_STICKY
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_OOM_STICKY(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_OOM_STICKY     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_OOM_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Sticky bit from the OTN framer indicates a change in LOF state
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY . LOF_STICKY
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_LOF_STICKY(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_LOF_STICKY     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_LOF_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Sticky bit from the OTN framer indicates a change in OOF state
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY . OOF_STICKY
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_OOF_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_OOF_STICKY     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_STICKY_OOF_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief Mask bits for interrupt generation
 *
 * \details
 * Encoding for each interrupt mask bit:
 * 0: Associated sticky condition not enabled to be a source of interrupt
 * 1: Associated sticky condition is enabled to be a source of interrupt
 *
 * Register: \a XFI::UPI_RX_OTN_FRMR::UPI_RX_OTN_FRMR_MASK
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK(target)  VTSS_IOREG(target,0x709)

/** 
 * \brief
 * Mask sticky bit from the OTN framer that indicates a change in the frame
 * alignment position
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK . FAS_POS_MASK
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_FAS_POS_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_FAS_POS_MASK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_FAS_POS_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Mask sticky bit from the OTN framer that indicates a change in LOM state
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK . LOM_MASK
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_LOM_MASK(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_LOM_MASK     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_LOM_MASK(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Mask sticky bit from the OTN framer that indicates a change in OOM state
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK . OOM_MASK
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_OOM_MASK(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_OOM_MASK     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_OOM_MASK(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Mask sticky bit from the OTN framer that indicates a change in LOF state
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK . LOF_MASK
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_LOF_MASK(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_LOF_MASK     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_LOF_MASK(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Mask sticky bit from the OTN framer that indicates a change in OOF state
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK . OOF_MASK
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_OOF_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_OOF_MASK     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_OTN_FRMR_MASK_OOF_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


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
 * Register: \a XFI::UPI_RX_OTN_FRMR::UPI_RX_AIS_CTRL
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_AIS_CTRL(target)  VTSS_IOREG(target,0x70a)

/** 
 * \brief
 * Force all ones in the ODU frame (excluding FA OH)
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_AIS_CTRL . FORCE_ODU_AIS
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_AIS_CTRL_FORCE_ODU_AIS(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_AIS_CTRL_FORCE_ODU_AIS     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_AIS_CTRL_FORCE_ODU_AIS(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Generate ODU AIS upon LOM
 * (AIS is always generated upon LOF or LOS)
 *
 * \details 
 * Field: ::VTSS_XFI_UPI_RX_OTN_FRMR_UPI_RX_AIS_CTRL . GEN_AIS_UPON_LOM
 */
#define  VTSS_F_XFI_UPI_RX_OTN_FRMR_UPI_RX_AIS_CTRL_GEN_AIS_UPON_LOM(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_UPI_RX_OTN_FRMR_UPI_RX_AIS_CTRL_GEN_AIS_UPON_LOM     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_UPI_RX_OTN_FRMR_UPI_RX_AIS_CTRL_GEN_AIS_UPON_LOM(x)  VTSS_EXTRACT_BITFIELD(x,0,1)

/**
 * Register Group: \a XFI::ANEG_CFG
 *
 * ANEG Configuration Registers
 */


/** 
 * \brief ANEG Configuration
 *
 * \details
 * Auto-negotiation configuration register. Note: Setting one of the
 * parallel detect wait times to 0 disables parallel detect function for
 * that specific mode.
 *
 * Register: \a XFI::ANEG_CFG::ANEG_CFG
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_ANEG_CFG_ANEG_CFG(target)   VTSS_IOREG(target,0x1700)

/** 
 * \brief
 * Disable automatic ANEG OB configuration
 *
 * \details 
 * 0: Allow ANEG block to control OB during auto-negotiation
 * 1: OB settings are not touched by ANEG block
 *
 * Field: ::VTSS_XFI_ANEG_CFG_ANEG_CFG . ANEG_OB_CTRL_DIS
 */
#define  VTSS_F_XFI_ANEG_CFG_ANEG_CFG_ANEG_OB_CTRL_DIS(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_XFI_ANEG_CFG_ANEG_CFG_ANEG_OB_CTRL_DIS     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_XFI_ANEG_CFG_ANEG_CFG_ANEG_OB_CTRL_DIS(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Parallel detect wait time for 10GBase-KR using single lane
 *
 * \details 
 * 0: 0 ms
 * 1: 10 ms
 * 2: 40 ms
 * 3: 505 ms
 *
 * Field: ::VTSS_XFI_ANEG_CFG_ANEG_CFG . PD_TIMER_10GKR
 */
#define  VTSS_F_XFI_ANEG_CFG_ANEG_CFG_PD_TIMER_10GKR(x)  VTSS_ENCODE_BITFIELD(x,8,2)
#define  VTSS_M_XFI_ANEG_CFG_ANEG_CFG_PD_TIMER_10GKR     VTSS_ENCODE_BITMASK(8,2)
#define  VTSS_X_XFI_ANEG_CFG_ANEG_CFG_PD_TIMER_10GKR(x)  VTSS_EXTRACT_BITFIELD(x,8,2)

/** 
 * \brief
 * Restart negotiation process
 *
 * \details 
 * 1: Restart
 *
 * Field: ::VTSS_XFI_ANEG_CFG_ANEG_CFG . RESTART_ANEG_ONE_SHOT
 */
#define  VTSS_F_XFI_ANEG_CFG_ANEG_CFG_RESTART_ANEG_ONE_SHOT(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_ANEG_CFG_ANEG_CFG_RESTART_ANEG_ONE_SHOT     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_ANEG_CFG_ANEG_CFG_RESTART_ANEG_ONE_SHOT(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Auto-negotiation enable
 *
 * \details 
 * 1: Enable
 * 0: Disable
 *
 * Field: ::VTSS_XFI_ANEG_CFG_ANEG_CFG . ANEG_ENA
 */
#define  VTSS_F_XFI_ANEG_CFG_ANEG_CFG_ANEG_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_ANEG_CFG_ANEG_CFG_ANEG_ENA     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_ANEG_CFG_ANEG_CFG_ANEG_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


/** 
 * \brief ANEG Advertised Ability 0
 *
 * \details
 * 48 bits that contain the advertised abilities link code word for
 * auto-negotiation (here: lower 32 bit).
 *
 * Register: \a XFI::ANEG_CFG::ANEG_ADV_ABILITY_0
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0(target)  VTSS_IOREG(target,0x1701)

/** 
 * \brief
 * Reserved for future technology as defined in IEEE 802.3ap clause 73.
 *
 * \details 
 * Field: ::VTSS_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0 . ADV_ABIL_LSB
 */
#define  VTSS_F_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_ADV_ABIL_LSB(x)  VTSS_ENCODE_BITFIELD(x,24,8)
#define  VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_ADV_ABIL_LSB     VTSS_ENCODE_BITMASK(24,8)
#define  VTSS_X_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_ADV_ABIL_LSB(x)  VTSS_EXTRACT_BITFIELD(x,24,8)

/** 
 * \brief
 * Technology Ability to be advertised (here: 10GBase-KR)
 *
 * \details 
 * 0: Do not advertise 10GB-KR capability
 * 1: Advertise 10GB-KR capability
 *
 * Field: ::VTSS_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0 . CAP_10GKR
 */
#define  VTSS_F_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_10GKR(x)  VTSS_ENCODE_BITFIELD(x,23,1)
#define  VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_10GKR     VTSS_ENCODE_BITMASK(23,1)
#define  VTSS_X_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_10GKR(x)  VTSS_EXTRACT_BITFIELD(x,23,1)

/** 
 * \brief
 * Technology Ability to be advertised (here: 10GBase-KX4). Should be left
 * at its default value.
 *
 * \details 
 * 0: Do not advertise 10GB-KX4 capability
 * 1: Advertise 10GB-KX4 capability
 *
 * Field: ::VTSS_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0 . CAP_10GKX4
 */
#define  VTSS_F_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_10GKX4(x)  VTSS_ENCODE_BITFIELD(x,22,1)
#define  VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_10GKX4     VTSS_ENCODE_BITMASK(22,1)
#define  VTSS_X_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_10GKX4(x)  VTSS_EXTRACT_BITFIELD(x,22,1)

/** 
 * \brief
 * Technology Ability to be advertised (here: 1000Base-KX). Should be left
 * at its default value.
 *
 * \details 
 * 0: Do not advertise 1GB-KX capability
 * 1: Advertise 1GB-KX capability
 *
 * Field: ::VTSS_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0 . CAP_1GKX
 */
#define  VTSS_F_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_1GKX(x)  VTSS_ENCODE_BITFIELD(x,21,1)
#define  VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_1GKX     VTSS_ENCODE_BITMASK(21,1)
#define  VTSS_X_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_1GKX(x)  VTSS_EXTRACT_BITFIELD(x,21,1)

/** 
 * \brief
 * Initial value for Transmit-Nonce field
 *
 * \details 
 * 5-bit binary number
 *
 * Field: ::VTSS_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0 . TX_NONCE
 */
#define  VTSS_F_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_TX_NONCE(x)  VTSS_ENCODE_BITFIELD(x,16,5)
#define  VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_TX_NONCE     VTSS_ENCODE_BITMASK(16,5)
#define  VTSS_X_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_TX_NONCE(x)  VTSS_EXTRACT_BITFIELD(x,16,5)

/** 
 * \brief
 * Next page exchange desired
 *
 * \details 
 * 0: Disable NP exchange
 * 1: Enable NP exchange
 *
 * Field: ::VTSS_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0 . NP
 */
#define  VTSS_F_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_NP(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_NP     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_NP(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * RF bit (initial value)
 *
 * \details 
 * 0 = No fault
 * 1 = Fault
 *
 * Field: ::VTSS_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0 . RF
 */
#define  VTSS_F_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_RF(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_RF     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_RF(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Pause field
 *
 * \details 
 * 0 = Pause not supported
 * 1 = Pause supported
 *
 * Field: ::VTSS_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0 . PAUSE
 */
#define  VTSS_F_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_PAUSE(x)  VTSS_ENCODE_BITFIELD(x,10,3)
#define  VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_PAUSE     VTSS_ENCODE_BITMASK(10,3)
#define  VTSS_X_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_PAUSE(x)  VTSS_EXTRACT_BITFIELD(x,10,3)

/** 
 * \brief
 * Reserved for echoed nonce field (must be cleared)
 *
 * \details 
 * binary number
 *
 * Field: ::VTSS_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0 . ECHOED_NONCE
 */
#define  VTSS_F_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_ECHOED_NONCE(x)  VTSS_ENCODE_BITFIELD(x,5,5)
#define  VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_ECHOED_NONCE     VTSS_ENCODE_BITMASK(5,5)
#define  VTSS_X_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_ECHOED_NONCE(x)  VTSS_EXTRACT_BITFIELD(x,5,5)

/** 
 * \brief
 * Selector field (must be 0x1)
 *
 * \details 
 * binary number
 *
 * Field: ::VTSS_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0 . SEL_FIELD
 */
#define  VTSS_F_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_SEL_FIELD(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_SEL_FIELD     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_SEL_FIELD(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief ANEG Advertised Ability 1
 *
 * \details
 * 48 bits that contain the advertised abilities link code word for
 * auto-negotiation (here: upper 16 bit).
 *
 * Register: \a XFI::ANEG_CFG::ANEG_ADV_ABILITY_1
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_ANEG_CFG_ANEG_ADV_ABILITY_1(target)  VTSS_IOREG(target,0x1702)

/** 
 * \brief
 * FEC capability (bit 14: FEC ability, bit 15: FEC requested) - Only used
 * with 10GBase-KR. Should be left at its default value.
 *
 * \details 
 * Bit 14 = 0 : FEC not available
 * Bit 14 = 1 : FEC available
 * Bit 15 = 0 : Do Not Request FEC
 * Bit 15 = 1: Request FEC
 *
 * Field: ::VTSS_XFI_ANEG_CFG_ANEG_ADV_ABILITY_1 . FEC
 */
#define  VTSS_F_XFI_ANEG_CFG_ANEG_ADV_ABILITY_1_FEC(x)  VTSS_ENCODE_BITFIELD(x,14,2)
#define  VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_1_FEC     VTSS_ENCODE_BITMASK(14,2)
#define  VTSS_X_XFI_ANEG_CFG_ANEG_ADV_ABILITY_1_FEC(x)  VTSS_EXTRACT_BITFIELD(x,14,2)

/** 
 * \brief
 * Reserved for future technology as defined in IEEE 802.3ap clause 73.
 *
 * \details 
 * Field: ::VTSS_XFI_ANEG_CFG_ANEG_ADV_ABILITY_1 . ADV_ABIL_MSB
 */
#define  VTSS_F_XFI_ANEG_CFG_ANEG_ADV_ABILITY_1_ADV_ABIL_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,14)
#define  VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_1_ADV_ABIL_MSB     VTSS_ENCODE_BITMASK(0,14)
#define  VTSS_X_XFI_ANEG_CFG_ANEG_ADV_ABILITY_1_ADV_ABIL_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,14)


/** 
 * \brief ANEG Next Page 0
 *
 * \details
 * 48 bits that contain the new next page to transmit during
 * auto-negotiation (here: lower 32 bits).
 *
 * Register: \a XFI::ANEG_CFG::ANEG_NEXT_PAGE_0
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_ANEG_CFG_ANEG_NEXT_PAGE_0(target)  VTSS_IOREG(target,0x1703)

/** 
 * \brief
 * Lower 32 bits of next page link code word
 *
 * \details 
 * Field: ::VTSS_XFI_ANEG_CFG_ANEG_NEXT_PAGE_0 . NP_TX_LSB
 */
#define  VTSS_F_XFI_ANEG_CFG_ANEG_NEXT_PAGE_0_NP_TX_LSB(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_XFI_ANEG_CFG_ANEG_NEXT_PAGE_0_NP_TX_LSB     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_XFI_ANEG_CFG_ANEG_NEXT_PAGE_0_NP_TX_LSB(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief ANEG Next Page 1
 *
 * \details
 * 48 bits that contain the new next page to transmit during
 * auto-negotiation (here: upper 16 bits).
 *
 * Register: \a XFI::ANEG_CFG::ANEG_NEXT_PAGE_1
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_ANEG_CFG_ANEG_NEXT_PAGE_1(target)  VTSS_IOREG(target,0x1704)

/** 
 * \brief
 * Must be set when a new next page is programmed (self-clearing)
 *
 * \details 
 * Write 1: Indicate that a page was programmed.
 *
 * Field: ::VTSS_XFI_ANEG_CFG_ANEG_NEXT_PAGE_1 . NEXT_PAGE_LOADED_ONE_SHOT
 */
#define  VTSS_F_XFI_ANEG_CFG_ANEG_NEXT_PAGE_1_NEXT_PAGE_LOADED_ONE_SHOT(x)  VTSS_ENCODE_BITFIELD(x,31,1)
#define  VTSS_M_XFI_ANEG_CFG_ANEG_NEXT_PAGE_1_NEXT_PAGE_LOADED_ONE_SHOT     VTSS_ENCODE_BITMASK(31,1)
#define  VTSS_X_XFI_ANEG_CFG_ANEG_NEXT_PAGE_1_NEXT_PAGE_LOADED_ONE_SHOT(x)  VTSS_EXTRACT_BITFIELD(x,31,1)

/** 
 * \brief
 * Upper 16 bits of next page link code word
 *
 * \details 
 * Field: ::VTSS_XFI_ANEG_CFG_ANEG_NEXT_PAGE_1 . NP_TX_MSB
 */
#define  VTSS_F_XFI_ANEG_CFG_ANEG_NEXT_PAGE_1_NP_TX_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_XFI_ANEG_CFG_ANEG_NEXT_PAGE_1_NP_TX_MSB     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_XFI_ANEG_CFG_ANEG_NEXT_PAGE_1_NP_TX_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mask bits for interrupts
 *
 * \details
 * The bits in the interrupt mask register are used to enable the
 * associated interrupts. Status is available in ANEG_STATUS. Setting the
 * bit to '1' enables the interrupt.
 *
 * Register: \a XFI::ANEG_CFG::ANEG_MASK
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_ANEG_CFG_ANEG_MASK(target)  VTSS_IOREG(target,0x1705)

/** 
 * \brief
 * Mask bit for INCOMPATIBLE_LINK_STICKY
 *
 * \details 
 * 0 = Interrupt Disabled
 * 1 = Interrupt Enabled
 *
 * Field: ::VTSS_XFI_ANEG_CFG_ANEG_MASK . INCOMPATIBLE_LINK_MASK
 */
#define  VTSS_F_XFI_ANEG_CFG_ANEG_MASK_INCOMPATIBLE_LINK_MASK(x)  VTSS_ENCODE_BITFIELD(x,21,1)
#define  VTSS_M_XFI_ANEG_CFG_ANEG_MASK_INCOMPATIBLE_LINK_MASK     VTSS_ENCODE_BITMASK(21,1)
#define  VTSS_X_XFI_ANEG_CFG_ANEG_MASK_INCOMPATIBLE_LINK_MASK(x)  VTSS_EXTRACT_BITFIELD(x,21,1)

/** 
 * \brief
 * Interrupt mask for PAR_DETECT_FAULT.
 *
 * \details 
 * 0 = Interrupt Disabled
 * 1 = Interrupt Enabled
 *
 * Field: ::VTSS_XFI_ANEG_CFG_ANEG_MASK . PAR_DETECT_FAULT_MASK
 */
#define  VTSS_F_XFI_ANEG_CFG_ANEG_MASK_PAR_DETECT_FAULT_MASK(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_XFI_ANEG_CFG_ANEG_MASK_PAR_DETECT_FAULT_MASK     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_XFI_ANEG_CFG_ANEG_MASK_PAR_DETECT_FAULT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * Interrupt mask for AN_GOOD_CHECK_STICKY
 *
 * \details 
 * 0 = Interrupt Disabled
 * 1 = Interrupt Enabled
 *
 * Field: ::VTSS_XFI_ANEG_CFG_ANEG_MASK . AN_GOOD_CHECK_MASK
 */
#define  VTSS_F_XFI_ANEG_CFG_ANEG_MASK_AN_GOOD_CHECK_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_XFI_ANEG_CFG_ANEG_MASK_AN_GOOD_CHECK_MASK     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_XFI_ANEG_CFG_ANEG_MASK_AN_GOOD_CHECK_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Interrupt mask for PAGE_RX_STICKY.
 *
 * \details 
 * 0 = Interrupt Disabled
 * 1 = Interrupt Enabled
 *
 * Field: ::VTSS_XFI_ANEG_CFG_ANEG_MASK . PAGE_RX_MASK
 */
#define  VTSS_F_XFI_ANEG_CFG_ANEG_MASK_PAGE_RX_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_ANEG_CFG_ANEG_MASK_PAGE_RX_MASK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_ANEG_CFG_ANEG_MASK_PAGE_RX_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Interrupt mask for ANEG_COMPLETE_STICKY.
 *
 * \details 
 * 0 = Interrupt Disabled
 * 1 = Interrupt Enabled
 *
 * Field: ::VTSS_XFI_ANEG_CFG_ANEG_MASK . ANEG_COMPLETE_MASK
 */
#define  VTSS_F_XFI_ANEG_CFG_ANEG_MASK_ANEG_COMPLETE_MASK(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_ANEG_CFG_ANEG_MASK_ANEG_COMPLETE_MASK     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_ANEG_CFG_ANEG_MASK_ANEG_COMPLETE_MASK(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/**
 * Register Group: \a XFI::ANEG_STATUS
 *
 * ANEG Status Registers
 */


/** 
 * \brief ANEG Link Partner Advertised Ability 0
 *
 * \details
 * 48 bits that contain the link partner's advertised abilities / next page
 * information (received link code word, lower 32 bits, received during
 * auto-negotiation). The bit groups are only valid for base pages; for
 * next page data exchange a different bit group coding has to be applied.
 *
 * Register: \a XFI::ANEG_STATUS::ANEG_LP_ADV_ABILITY_0
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0(target)  VTSS_IOREG(target,0x1706)

/** 
 * \brief
 * Bits 31 down to 24 of link code word received from link partner.
 *
 * \details 
 * Field: ::VTSS_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . LP_ADV_ABIL_LSB
 */
#define  VTSS_F_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_LP_ADV_ABIL_LSB(x)  VTSS_ENCODE_BITFIELD(x,24,8)
#define  VTSS_M_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_LP_ADV_ABIL_LSB     VTSS_ENCODE_BITMASK(24,8)
#define  VTSS_X_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_LP_ADV_ABIL_LSB(x)  VTSS_EXTRACT_BITFIELD(x,24,8)

/** 
 * \brief
 * Technology Ability advertised by LP (here: 10GBase-KR)
 *
 * \details 
 * 0: LP is not 10GB-KR capable
 * 1: LP is 10GB-KR capable
 *
 * Field: ::VTSS_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . CAP_10GKR
 */
#define  VTSS_F_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_CAP_10GKR(x)  VTSS_ENCODE_BITFIELD(x,23,1)
#define  VTSS_M_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_CAP_10GKR     VTSS_ENCODE_BITMASK(23,1)
#define  VTSS_X_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_CAP_10GKR(x)  VTSS_EXTRACT_BITFIELD(x,23,1)

/** 
 * \brief
 * Technology Ability advertised by LP (here: 10GBase-KX4)
 *
 * \details 
 * 0: LP is not 10GB-KX4 capable
 * 1: LP is 10GB-KX4 capable
 *
 * Field: ::VTSS_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . CAP_10GKX4
 */
#define  VTSS_F_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_CAP_10GKX4(x)  VTSS_ENCODE_BITFIELD(x,22,1)
#define  VTSS_M_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_CAP_10GKX4     VTSS_ENCODE_BITMASK(22,1)
#define  VTSS_X_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_CAP_10GKX4(x)  VTSS_EXTRACT_BITFIELD(x,22,1)

/** 
 * \brief
 * Technology Ability advertised by LP (here: 1000Base-KX)
 *
 * \details 
 * 0: LP is not 1GB-KX capable
 * 1: LP is 1GB-KX capable
 *
 * Field: ::VTSS_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . CAP_1GKX
 */
#define  VTSS_F_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_CAP_1GKX(x)  VTSS_ENCODE_BITFIELD(x,21,1)
#define  VTSS_M_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_CAP_1GKX     VTSS_ENCODE_BITMASK(21,1)
#define  VTSS_X_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_CAP_1GKX(x)  VTSS_EXTRACT_BITFIELD(x,21,1)

/** 
 * \brief
 * Transmit-Nonce field (received from LinkPartner)
 *
 * \details 
 * 5-bit binary number
 *
 * Field: ::VTSS_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . TX_NONCE
 */
#define  VTSS_F_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_TX_NONCE(x)  VTSS_ENCODE_BITFIELD(x,16,5)
#define  VTSS_M_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_TX_NONCE     VTSS_ENCODE_BITMASK(16,5)
#define  VTSS_X_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_TX_NONCE(x)  VTSS_EXTRACT_BITFIELD(x,16,5)

/** 
 * \brief
 * Next page exchange desired by LP
 *
 * \details 
 * 0: No NP exchange desired
 * 1: NP exchange desired
 *
 * Field: ::VTSS_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . NP
 */
#define  VTSS_F_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_NP(x)  VTSS_ENCODE_BITFIELD(x,15,1)
#define  VTSS_M_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_NP     VTSS_ENCODE_BITMASK(15,1)
#define  VTSS_X_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_NP(x)  VTSS_EXTRACT_BITFIELD(x,15,1)

/** 
 * \brief
 * Acknowledge bit (this bit is automatically overwritten by ANEG)
 *
 * \details 
 * 0 = Link Codeword not received by partner
 * 1 = Link Codeword received by partner
 *
 * Field: ::VTSS_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . ACKN
 */
#define  VTSS_F_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_ACKN(x)  VTSS_ENCODE_BITFIELD(x,14,1)
#define  VTSS_M_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_ACKN     VTSS_ENCODE_BITMASK(14,1)
#define  VTSS_X_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_ACKN(x)  VTSS_EXTRACT_BITFIELD(x,14,1)

/** 
 * \brief
 * RF bit
 *
 * \details 
 * 0 = No fault
 * 1 = Fault
 *
 * Field: ::VTSS_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . RF
 */
#define  VTSS_F_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_RF(x)  VTSS_ENCODE_BITFIELD(x,13,1)
#define  VTSS_M_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_RF     VTSS_ENCODE_BITMASK(13,1)
#define  VTSS_X_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_RF(x)  VTSS_EXTRACT_BITFIELD(x,13,1)

/** 
 * \brief
 * Pause field
 *
 * \details 
 * 0 = Pause not supported
 * 1 = Pause supported
 *
 * Field: ::VTSS_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . PAUSE
 */
#define  VTSS_F_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_PAUSE(x)  VTSS_ENCODE_BITFIELD(x,10,3)
#define  VTSS_M_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_PAUSE     VTSS_ENCODE_BITMASK(10,3)
#define  VTSS_X_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_PAUSE(x)  VTSS_EXTRACT_BITFIELD(x,10,3)

/** 
 * \brief
 * Echoed nonce field
 *
 * \details 
 * binary number
 *
 * Field: ::VTSS_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . ECHOED_NONCE
 */
#define  VTSS_F_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_ECHOED_NONCE(x)  VTSS_ENCODE_BITFIELD(x,5,5)
#define  VTSS_M_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_ECHOED_NONCE     VTSS_ENCODE_BITMASK(5,5)
#define  VTSS_X_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_ECHOED_NONCE(x)  VTSS_EXTRACT_BITFIELD(x,5,5)

/** 
 * \brief
 * Selector field
 *
 * \details 
 * binary number
 *
 * Field: ::VTSS_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . SEL_FIELD
 */
#define  VTSS_F_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_SEL_FIELD(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_SEL_FIELD     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_SEL_FIELD(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief ANEG Link Partner Advertised Ability 1
 *
 * \details
 * 48 bits that contain the link partner's advertised abilities or next
 * page information (received link code word, upper 16 bits, received
 * during auto-negotiation). The bit groups are only valid for base pages;
 * for next page data exchange a different bit group coding has to be
 * applied.
 *
 * Register: \a XFI::ANEG_STATUS::ANEG_LP_ADV_ABILITY_1
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1(target)  VTSS_IOREG(target,0x1707)

/** 
 * \brief
 * FEC capability (bit 14: FEC ability, bit 15: FEC requested) - Only used
 * with 10GBase-KR
 *
 * \details 
 * Bit 14 = 0 : FEC not available
 * Bit 14 = 1 : FEC available
 * Bit 15 = 0 : Do Not Request FEC
 * Bit 15 = 1: Request FEC
 *
 * Field: ::VTSS_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1 . FEC
 */
#define  VTSS_F_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1_FEC(x)  VTSS_ENCODE_BITFIELD(x,14,2)
#define  VTSS_M_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1_FEC     VTSS_ENCODE_BITMASK(14,2)
#define  VTSS_X_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1_FEC(x)  VTSS_EXTRACT_BITFIELD(x,14,2)

/** 
 * \brief
 * Bits 45 down to 32 of link code word received from link partner.
 *
 * \details 
 * Field: ::VTSS_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1 . LP_ADV_ABIL_MSB
 */
#define  VTSS_F_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1_LP_ADV_ABIL_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,14)
#define  VTSS_M_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1_LP_ADV_ABIL_MSB     VTSS_ENCODE_BITMASK(0,14)
#define  VTSS_X_XFI_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1_LP_ADV_ABIL_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,14)


/** 
 * \brief ANEG Status
 *
 * \details
 * Auto negotiation status register
 *
 * Register: \a XFI::ANEG_STATUS::ANEG_STATUS
 *
 * @param target A \a ::vtss_target_XFI_e target
 */
#define VTSS_XFI_ANEG_STATUS_ANEG_STATUS(target)  VTSS_IOREG(target,0x1708)

/** 
 * \brief
 * Sticky bit for the INCOMPATIBLE_LINK status bit. The sticky is set
 * whenever the state of INCOMPATIBLE_LINK is set. Bit is cleared by
 * writing a 1 to this position.
 *
 * \details 
 * 0 = INCOMPATIBLE_LINK is not set (link compatible)
 * 1 = INCOMPATIBLE_LINK has been set
 *
 * Field: ::VTSS_XFI_ANEG_STATUS_ANEG_STATUS . INCOMPATIBLE_LINK_STICKY
 */
#define  VTSS_F_XFI_ANEG_STATUS_ANEG_STATUS_INCOMPATIBLE_LINK_STICKY(x)  VTSS_ENCODE_BITFIELD(x,21,1)
#define  VTSS_M_XFI_ANEG_STATUS_ANEG_STATUS_INCOMPATIBLE_LINK_STICKY     VTSS_ENCODE_BITMASK(21,1)
#define  VTSS_X_XFI_ANEG_STATUS_ANEG_STATUS_INCOMPATIBLE_LINK_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,21,1)

/** 
 * \brief
 * Error condition indicating that no compatible link was found.
 *
 * \details 
 * 0 = Link is compatible
 * 1 = Link is incompatible
 *
 * Field: ::VTSS_XFI_ANEG_STATUS_ANEG_STATUS . INCOMPATIBLE_LINK
 */
#define  VTSS_F_XFI_ANEG_STATUS_ANEG_STATUS_INCOMPATIBLE_LINK(x)  VTSS_ENCODE_BITFIELD(x,17,1)
#define  VTSS_M_XFI_ANEG_STATUS_ANEG_STATUS_INCOMPATIBLE_LINK     VTSS_ENCODE_BITMASK(17,1)
#define  VTSS_X_XFI_ANEG_STATUS_ANEG_STATUS_INCOMPATIBLE_LINK(x)  VTSS_EXTRACT_BITFIELD(x,17,1)

/** 
 * \brief
 * Error condition indicating errors during parallel detection. Bit is
 * cleared by writing a 1 to this position.
 *
 * \details 
 * 0 = No fault detected
 * 1 = Parallel Detection Fault detected
 *
 * Field: ::VTSS_XFI_ANEG_STATUS_ANEG_STATUS . PAR_DETECT_FAULT_STICKY
 */
#define  VTSS_F_XFI_ANEG_STATUS_ANEG_STATUS_PAR_DETECT_FAULT_STICKY(x)  VTSS_ENCODE_BITFIELD(x,16,1)
#define  VTSS_M_XFI_ANEG_STATUS_ANEG_STATUS_PAR_DETECT_FAULT_STICKY     VTSS_ENCODE_BITMASK(16,1)
#define  VTSS_X_XFI_ANEG_STATUS_ANEG_STATUS_PAR_DETECT_FAULT_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,16,1)

/** 
 * \brief
 * This bit is set whenever the Arbitration FSM enters the AN_GOOD_CHECK
 * state and it may also optionally generate an interrupt. This interrupt
 * can be used to start the 10GBase-KR Coefficient Update and Training
 * sequence which should complete within this state.
 *
 * \details 
 * 0 = Arb FSM not in AN_GOOD_CHECK state
 * 1 = Arb FSM entered the AN_GOOD_CHECK state
 *
 * Field: ::VTSS_XFI_ANEG_STATUS_ANEG_STATUS . AN_GOOD_CHECK_STICKY
 */
#define  VTSS_F_XFI_ANEG_STATUS_ANEG_STATUS_AN_GOOD_CHECK_STICKY(x)  VTSS_ENCODE_BITFIELD(x,12,1)
#define  VTSS_M_XFI_ANEG_STATUS_ANEG_STATUS_AN_GOOD_CHECK_STICKY     VTSS_ENCODE_BITMASK(12,1)
#define  VTSS_X_XFI_ANEG_STATUS_ANEG_STATUS_AN_GOOD_CHECK_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,12,1)

/** 
 * \brief
 * Sticky bit set when PAGE_RX is set.
 *
 * \details 
 * 0 = PAGE_RX bit has not been set.
 * 1 = PAGE_RX bit has changed from 0 to 1.
 *
 * Field: ::VTSS_XFI_ANEG_STATUS_ANEG_STATUS . PAGE_RX_STICKY
 */
#define  VTSS_F_XFI_ANEG_STATUS_ANEG_STATUS_PAGE_RX_STICKY(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_XFI_ANEG_STATUS_ANEG_STATUS_PAGE_RX_STICKY     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_XFI_ANEG_STATUS_ANEG_STATUS_PAGE_RX_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Status indicating whether a new page has been received.
 *
 * \details 
 * 0 = No page received
 * 1 = Page received
 *
 * Field: ::VTSS_XFI_ANEG_STATUS_ANEG_STATUS . PAGE_RX
 */
#define  VTSS_F_XFI_ANEG_STATUS_ANEG_STATUS_PAGE_RX(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_XFI_ANEG_STATUS_ANEG_STATUS_PAGE_RX     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_XFI_ANEG_STATUS_ANEG_STATUS_PAGE_RX(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Status indicating whether the link partner supports auto-negotiation.
 *
 * \details 
 * 0 = Link Partner does NOT support ANEG
 * 1 = Link Partner supports ANEG
 *
 * Field: ::VTSS_XFI_ANEG_STATUS_ANEG_STATUS . LP_ANEG_ABLE
 */
#define  VTSS_F_XFI_ANEG_STATUS_ANEG_STATUS_LP_ANEG_ABLE(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_XFI_ANEG_STATUS_ANEG_STATUS_LP_ANEG_ABLE     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_XFI_ANEG_STATUS_ANEG_STATUS_LP_ANEG_ABLE(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Sticky for ANEG_COMPLETE. Bit is set whenever the ANEG_COMPLETE status
 * is set.
 *
 * \details 
 * 0 = ANEG_COMPLETE status is not set.
 * 1 = ANEG_COMPLETE status set to 1.
 *
 * Field: ::VTSS_XFI_ANEG_STATUS_ANEG_STATUS . ANEG_COMPLETE_STICKY
 */
#define  VTSS_F_XFI_ANEG_STATUS_ANEG_STATUS_ANEG_COMPLETE_STICKY(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_XFI_ANEG_STATUS_ANEG_STATUS_ANEG_COMPLETE_STICKY     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_XFI_ANEG_STATUS_ANEG_STATUS_ANEG_COMPLETE_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,1,1)

/** 
 * \brief
 * Status indicating whether auto-negotiation has completed.
 *
 * \details 
 * 0 = Autonegotiation not started or not completed
 * 1 = Autonegotiation complete
 *
 * Field: ::VTSS_XFI_ANEG_STATUS_ANEG_STATUS . ANEG_COMPLETE
 */
#define  VTSS_F_XFI_ANEG_STATUS_ANEG_STATUS_ANEG_COMPLETE(x)  VTSS_ENCODE_BITFIELD(x,0,1)
#define  VTSS_M_XFI_ANEG_STATUS_ANEG_STATUS_ANEG_COMPLETE     VTSS_ENCODE_BITMASK(0,1)
#define  VTSS_X_XFI_ANEG_STATUS_ANEG_STATUS_ANEG_COMPLETE(x)  VTSS_EXTRACT_BITFIELD(x,0,1)


#endif /* _VTSS_DAYTONA_REGS_XFI_H_ */
