#ifndef _VTSS_JAGUAR2_REGS_DW_APB_SSI_H_
#define _VTSS_JAGUAR2_REGS_DW_APB_SSI_H_

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

#include "vtss_jaguar2_regs_common.h"

/*********************************************************************** 
 *
 * Target: \a DW_APB_SSI
 *
 * 
 *
 ***********************************************************************/

/**
 * Register Group: \a DW_APB_SSI:ssi_memory_map_ssi_address_block
 *
 * Not documented
 */


/** 
 * \details
 * 
 * This register controls the serial data transfer. It is impossible to
 * write to this register when the DW_apb_ssi is enabled. The DW_apb_ssi
 * is enabled and disabled by writing to the SSIENR register.

 *
 * Register: \a DW_APB_SSI:ssi_memory_map_ssi_address_block:CTRLR0
 */
#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0  VTSS_IOREG(VTSS_TO_SSI,0x0)

/** 
 * \brief
 * 
 * Control Frame Size. Selects the length of the control word for the
 * Microwire frame format

 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0 . CFS
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_CFS(x)  VTSS_ENCODE_BITFIELD(x,12,4)
#define  VTSS_M_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_CFS     VTSS_ENCODE_BITMASK(12,4)
#define  VTSS_X_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_CFS(x)  VTSS_EXTRACT_BITFIELD(x,12,4)

/** 
 * \brief
 * 
 * Shift Register Loop. Used for testing purposes only. When internally
 * active, connects the transmit shift register output to the receive
 * shift register input.

 *
 * \details 
 * 
 * 0 - Normal Mode Operation
 * 1 - Test Mode Operation

 *
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0 . SRL
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_SRL  VTSS_BIT(11)

/** 
 * \brief
 * 
 * Selects the mode of transfer for serial communication. This field does
 * not affect the transfer duplicity. Only indicates whether the receive or
 * transmit data are valid. In transmit-only mode, data received from the
 * external device is not valid and is not stored in the receive FIFO
 * memory;
 * it is overwritten on the next transfer. In receive-only mode,
 * transmitted
 * data are not valid. After the first write to the transmit FIFO, the same
 * word is retransmitted for the duration of the transfer. In
 * transmit-and-receive mode, both transmit and receive data are valid.
 * The transfer continues until the transmit FIFO is empty. Data received
 * from the external device are stored into the receive FIFO memory, where
 * it can be accessed by the host processor.

 *
 * \details 
 * 
 * 00 - Transmit and Receive
 * 01 - Transmit Only
 * 10 - Receive Only
 * 11 - Reserved

 *
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0 . TMOD
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_TMOD(x)  VTSS_ENCODE_BITFIELD(x,8,2)
#define  VTSS_M_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_TMOD     VTSS_ENCODE_BITMASK(8,2)
#define  VTSS_X_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_TMOD(x)  VTSS_EXTRACT_BITFIELD(x,8,2)

/** 
 * \brief
 * 
 * Valid when the frame format (FRF) is set to Motorola SPI. Used to select
 * the polarity of the inactive serial clock, which is held inactive when
 * the DW_apb_ssi master is not actively transferring data on the serial
 * bus.

 *
 * \details 
 * 
 * 0 - Inactive state of serial clock is low
 * 1 - Inactive state of serial clock is high

 *
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0 . SCPOL
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_SCPOL  VTSS_BIT(7)

/** 
 * \brief
 * 
 * Valid when the frame format (FRF) is set to Motorola SPI. The serial
 * clock phase selects the relationship of the serial clock with the slave
 * select signal. When SCPH = 0, data are captured on the first edge of
 * the serial clock. When SCPH = 1, the serial clock starts toggling one
 * cycle after the slave select line is activated, and data are captured
 * on the second edge of the serial clock.

 *
 * \details 
 * 
 * 0: Serial clock toggles in middle of first data bit
 * 1: Serial clock toggles at start of first data bit

 *
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0 . SCPH
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_SCPH  VTSS_BIT(6)

/** 
 * \brief
 * 
 * Selects which serial protocol transfers the data.

 *
 * \details 
 * 
 * 00 - Motorola SPI
 * 01 - Texas Instruments SSP
 * 10 - National Semiconductors Microwire
 * 11 - Reserved

 *
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0 . FRF
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_FRF(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_FRF     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_FRF(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * 
 * Selects the data frame length. When the data frame size is programmed
 * to be less than 16 bits, the receive data are automatically
 * right-justified by the receive logic, with the upper bits of the receive
 * FIFO zero-padded. You must right-justify transmit data before writing
 * into the transmit FIFO. The transmit logic ignores the upper unused
 * bits when transmitting the data

 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0 . DFS
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_DFS(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_DFS     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR0_DFS(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \details
 * 
 * Control register 1 controls the end of serial transfers
 * when in receive-only mode. It is impossible to write to this
 * register when the DW_apb_ssi is enabled. The DW_apb_ssi is enabled
 * and disabled by writing to the SSIENR register.

 *
 * Register: \a DW_APB_SSI:ssi_memory_map_ssi_address_block:CTRLR1
 */
#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR1  VTSS_IOREG(VTSS_TO_SSI,0x1)

/** 
 * \brief
 * 
 * When TMOD = 10, this register field sets the number of data frames to
 * be continuously received by the DW_apb_ssi. The DW_apb_ssi continues
 * to receive serial data until the number of data frames received is
 * equal to this register value plus 1, which enables you to receive up
 * to 64 KB of data in a continuous transfer.

 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR1 . NDF
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR1_NDF(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR1_NDF     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DW_APB_SSI_ssi_memory_map_ssi_address_block_CTRLR1_NDF(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief SSI Enable Register
 *
 * \details
 * Register: \a DW_APB_SSI:ssi_memory_map_ssi_address_block:SSIENR
 */
#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_SSIENR  VTSS_IOREG(VTSS_TO_SSI,0x2)

/** 
 * \brief
 * 
 * SSI Enable. Enables and disables all DW_apb_ssi operations. When
 * disabled, all serial transfers are halted immediately. Transmit and
 * receive FIFO buffers are cleared when the device is disabled. It is
 * impossible to program some of the DW_apb_ssi control registers when
 * enabled.

 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_SSIENR . SSI_EN
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_SSIENR_SSI_EN  VTSS_BIT(0)


/** 
 * \brief Microwire Control Register
 *
 * \details
 * 
 * This register controls the direction of the data word for the
 * half-duplex
 * Microwire serial protocol. It is impossible to write to this register
 * when the DW_apb_ssi is enabled. The DW_apb_ssi is enabled and disabled
 * by
 * writing to the SSIENR register.

 *
 * Register: \a DW_APB_SSI:ssi_memory_map_ssi_address_block:MWCR
 */
#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_MWCR  VTSS_IOREG(VTSS_TO_SSI,0x3)

/** 
 * \brief
 * 
 * Used to enable and disable the busy/ready handshaking interface for the
 * Microwire protocol. When enabled, the DW_apb_ssi checks for a ready
 * status
 * from the target slave, after the transfer of the last data/control bit,
 * before clearing the BUSY status in the SR register.

 *
 * \details 
 * 
 * 0: handshaking interface is disabled
 * 1: handshaking interface is enabled

 *
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_MWCR . MHS
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_MWCR_MHS  VTSS_BIT(2)

/** 
 * \brief
 * 
 * Defines the direction of the data word when the Microwire serial
 * protocol
 * is used. When this bit is set to 0, the data word is received by the
 * DW_apb_ssi MacroCell from the external serial device. When this bit is
 * set to 1, the data word is transmitted from the DW_apb_ssi MacroCell to
 * the external serial device.

 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_MWCR . MDD
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_MWCR_MDD  VTSS_BIT(1)

/** 
 * \brief
 * 
 * Defines whether the Microwire transfer is sequential or non-sequential.
 * When sequential mode is used, only one control word is needed to
 * transmit or receive a block of data words. When non-sequential mode is
 * used, there must be a control word for each data word that is
 * transmitted or received.

 *
 * \details 
 * 
 * 0: non-sequential transfer
 * 1: sequential transfer

 *
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_MWCR . MWMOD
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_MWCR_MWMOD  VTSS_BIT(0)


/** 
 * \brief Slave Enable Register
 *
 * \details
 * 
 * The register enables the individual slave select output lines from the
 * DW_apb_ssi master. 16 slave-select output pins are available on the
 * DW_apb_ssi master. You cannot write to this register when DW_apb_ssi is
 * busy and when SSI_EN = 1.

 *
 * Register: \a DW_APB_SSI:ssi_memory_map_ssi_address_block:SER
 */
#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_SER  VTSS_IOREG(VTSS_TO_SSI,0x4)

/** 
 * \brief
 * 
 * Each bit in this register corresponds to a slave select line from the 
 * DW_apb_ssi master. When a bit in this register is set, the
 * corresponding slave select line from the master is activated when a
 * serial transfer begins. It should be noted that setting or clearing bits
 * in this register have no effect on the corresponding slave select
 * outputs
 * until a transfer is started. Before beginning a transfer, you should
 * enable the bit in this register that corresponds to the slave device
 * with which the master wants to communicate. When not operating in
 * broadcast mode, only one bit in this field should be set.

 *
 * \details 
 * 
 * 1: Selected
 * 0: Not Selected

 *
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_SER . SER
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_SER_SER(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DW_APB_SSI_ssi_memory_map_ssi_address_block_SER_SER     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DW_APB_SSI_ssi_memory_map_ssi_address_block_SER_SER(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Baud Rate Select
 *
 * \details
 * 
 * The register derives the frequency of the serial clock that regulates
 * the data
 * transfer. The 16-bit field in this register defines the ssi_clk divider
 * value. It is impossible to write to this register when the DW_apb_ssi is
 * enabled. The DW_apb_ssi is enabled and disabled by writing to the SSIENR
 * register.

 *
 * Register: \a DW_APB_SSI:ssi_memory_map_ssi_address_block:BAUDR
 */
#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_BAUDR  VTSS_IOREG(VTSS_TO_SSI,0x5)

/** 
 * \brief
 * 
 * The LSB for this field is always set to 0 and is unaffected by a write
 * operation, which ensures an even value is held in this register. If the
 * value is 0, the serial output clock (sclk_out) is disabled. The
 * frequency
 * of the sclk_out is derived from the following equation:
 *	   Fsclk_out = Fsystem_clk/SCKDV
 * where SCKDV is any even value between 2 and 65534 abd Fsystem_clk is
 * 250MHz.

 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_BAUDR . SCKDV
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_BAUDR_SCKDV(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DW_APB_SSI_ssi_memory_map_ssi_address_block_BAUDR_SCKDV     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DW_APB_SSI_ssi_memory_map_ssi_address_block_BAUDR_SCKDV(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Transmit FIFO Threshold Level
 *
 * \details
 * Register: \a DW_APB_SSI:ssi_memory_map_ssi_address_block:TXFTLR
 */
#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_TXFTLR  VTSS_IOREG(VTSS_TO_SSI,0x6)

/** 
 * \brief
 * 
 * Controls the level of entries (or below) at which the transmit FIFO
 * controller
 * triggers an interrupt. The FIFO depth is 8. If you attempt to set this
 * value greater than or equal to the depth
 * of the FIFO, this field is not written and retains its current value.
 * When
 * the number of transmit FIFO entries is less than or equal to this value,
 * the transmit FIFO empty interrupt is triggered.

 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_TXFTLR . TFT
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_TXFTLR_TFT(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_DW_APB_SSI_ssi_memory_map_ssi_address_block_TXFTLR_TFT     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_DW_APB_SSI_ssi_memory_map_ssi_address_block_TXFTLR_TFT(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Receive FIFO Threshold level
 *
 * \details
 * Register: \a DW_APB_SSI:ssi_memory_map_ssi_address_block:RXFTLR
 */
#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXFTLR  VTSS_IOREG(VTSS_TO_SSI,0x7)

/** 
 * \brief
 * 
 * Controls the level of entries (or above) at which the receive FIFO
 * controller triggers an interrupt. The FIFO depth is 40. This register is
 * sized to the number of address bits
 * needed to access the FIFO. If you attempt to set this value greater
 * than the depth of the FIFO, this field is not written and retains its
 * current value. When the number of receive FIFO entries is greater than
 * or equal to this value + 1, the receive FIFO full interrupt is
 * triggered.

 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXFTLR . RFT
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXFTLR_RFT(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXFTLR_RFT     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXFTLR_RFT(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief Transmit FIFO Level Register
 *
 * \details
 * Register: \a DW_APB_SSI:ssi_memory_map_ssi_address_block:TXFLR
 */
#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_TXFLR  VTSS_IOREG(VTSS_TO_SSI,0x8)

/** 
 * \brief
 * Contains the number of valid data entries in the transmit FIFO.
 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_TXFLR . TXTFL
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_TXFLR_TXTFL(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_DW_APB_SSI_ssi_memory_map_ssi_address_block_TXFLR_TXTFL     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_DW_APB_SSI_ssi_memory_map_ssi_address_block_TXFLR_TXTFL(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Receive FIFO Level Register
 *
 * \details
 * Register: \a DW_APB_SSI:ssi_memory_map_ssi_address_block:RXFLR
 */
#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXFLR  VTSS_IOREG(VTSS_TO_SSI,0x9)

/** 
 * \brief
 * Contains the number of valid data entries in the receive FIFO.
 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXFLR . RXTFL
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXFLR_RXTFL(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXFLR_RXTFL     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXFLR_RXTFL(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief Interrupt Mask Register
 *
 * \details
 * Register: \a DW_APB_SSI:ssi_memory_map_ssi_address_block:IMR
 */
#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_IMR  VTSS_IOREG(VTSS_TO_SSI,0xb)

/** 
 * \brief
 * Multi-Master Contention Interrupt Mask
 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_IMR . MSTIM
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_IMR_MSTIM  VTSS_BIT(5)

/** 
 * \brief
 * Receive FIFO Full Interrupt Mask
 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_IMR . RXFIM
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_IMR_RXFIM  VTSS_BIT(4)

/** 
 * \brief
 * Receive FIFO Overflow Interrupt Mask
 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_IMR . RXOIM
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_IMR_RXOIM  VTSS_BIT(3)

/** 
 * \brief
 * Receive FIFO Underflow Interrupt Mask
 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_IMR . RXUIM
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_IMR_RXUIM  VTSS_BIT(2)

/** 
 * \brief
 * Transmit FIFO Overflow Interrupt Mask
 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_IMR . TXOIM
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_IMR_TXOIM  VTSS_BIT(1)

/** 
 * \brief
 * Transmit FIFO Empty Interrupt Mask
 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_IMR . TXEIM
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_IMR_TXEIM  VTSS_BIT(0)


/** 
 * \brief Interrupt Status Register
 *
 * \details
 * Register: \a DW_APB_SSI:ssi_memory_map_ssi_address_block:ISR
 */
#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_ISR  VTSS_IOREG(VTSS_TO_SSI,0xc)

/** 
 * \brief
 * Multi-Master Contention Interrupt Status
 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_ISR . MSTIS
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_ISR_MSTIS  VTSS_BIT(5)

/** 
 * \brief
 * Receive FIFO Full Interrupt Status
 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_ISR . RXFIS
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_ISR_RXFIS  VTSS_BIT(4)

/** 
 * \brief
 * Receive FIFO Overflow Interrupt Status
 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_ISR . RXOIS
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_ISR_RXOIS  VTSS_BIT(3)

/** 
 * \brief
 * Receive FIFO Underflow Interrupt Status
 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_ISR . RXUIS
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_ISR_RXUIS  VTSS_BIT(2)

/** 
 * \brief
 * Transmit FIFO Overflow Interrupt Status
 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_ISR . TXOIS
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_ISR_TXOIS  VTSS_BIT(1)

/** 
 * \brief
 * Transmit FIFO Empty Interrupt Status
 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_ISR . TXEIS
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_ISR_TXEIS  VTSS_BIT(0)


/** 
 * \brief Raw Interrupt Status Register
 *
 * \details
 * Register: \a DW_APB_SSI:ssi_memory_map_ssi_address_block:RISR
 */
#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_RISR  VTSS_IOREG(VTSS_TO_SSI,0xd)

/** 
 * \brief
 * Multi-Master Contention Raw Interrupt Status
 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_RISR . MSTIR
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_RISR_MSTIR  VTSS_BIT(5)

/** 
 * \brief
 * Receive FIFO Full Raw Interrupt Status
 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_RISR . RXFIR
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_RISR_RXFIR  VTSS_BIT(4)

/** 
 * \brief
 * Receive FIFO Overflow Raw Interrupt Status
 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_RISR . RXOIR
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_RISR_RXOIR  VTSS_BIT(3)

/** 
 * \brief
 * Receive FIFO Underflow Raw Interrupt Status
 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_RISR . RXUIR
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_RISR_RXUIR  VTSS_BIT(2)

/** 
 * \brief
 * Transmit FIFO Overflow Raw Interrupt Status
 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_RISR . TXOIR
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_RISR_TXOIR  VTSS_BIT(1)

/** 
 * \brief
 * Transmit FIFO Empty Raw Interrupt Status
 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_RISR . TXEIR
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_RISR_TXEIR  VTSS_BIT(0)


/** 
 * \details
 * Register: \a DW_APB_SSI:ssi_memory_map_ssi_address_block:TXOICR
 */
#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_TXOICR  VTSS_IOREG(VTSS_TO_SSI,0xe)

/** 
 * \brief
 * Clear Transmit FIFO Overflow Interrupt. A read from this
 * register clears the interrupt.
 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_TXOICR . TXOICR
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_TXOICR_TXOICR  VTSS_BIT(0)


/** 
 * \details
 * Register: \a DW_APB_SSI:ssi_memory_map_ssi_address_block:RXOICR
 */
#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXOICR  VTSS_IOREG(VTSS_TO_SSI,0xf)

/** 
 * \brief
 * Clear Receive FIFO Overflow Interrupt. A read from this
 * register clears the interrupt.
 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXOICR . RXOICR
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXOICR_RXOICR  VTSS_BIT(0)


/** 
 * \details
 * Register: \a DW_APB_SSI:ssi_memory_map_ssi_address_block:RXUICR
 */
#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXUICR  VTSS_IOREG(VTSS_TO_SSI,0x10)

/** 
 * \brief
 * Clear Receive FIFO Underflow Interrupt. A read from this
 * register clears the interrupt.
 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXUICR . RXUICR
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_RXUICR_RXUICR  VTSS_BIT(0)


/** 
 * \details
 * Register: \a DW_APB_SSI:ssi_memory_map_ssi_address_block:MSTICR
 */
#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_MSTICR  VTSS_IOREG(VTSS_TO_SSI,0x11)

/** 
 * \brief
 * Clear Multi-Master Contention Interrupt. A read from this
 * register clears the interrupt.
 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_MSTICR . MSTICR
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_MSTICR_MSTICR  VTSS_BIT(0)


/** 
 * \brief Clear Interrupts
 *
 * \details
 * Register: \a DW_APB_SSI:ssi_memory_map_ssi_address_block:ICR
 */
#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_ICR  VTSS_IOREG(VTSS_TO_SSI,0x12)

/** 
 * \brief
 * This register is set if any of the interrupts below are active. A read
 * clears the ssi_txo_intr, ssi_rxu_intr, ssi_rxo_intr, and the
 * ssi_mst_intr interrupts.
 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_ICR . ICR
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_ICR_ICR  VTSS_BIT(0)


/** 
 * \details
 * 
 * The DW_apb_ssi data register is a 16-bit read/write buffer for the
 * transmit/receive FIFOs. When the register is read, data in the receive
 * FIFO buffer is accessed. When it is written to, data are moved into the
 * transmit FIFO buffer; a write can occur only when SSI_EN = 1. FIFOs are
 * reset when SSI_EN = 0.

 *
 * Register: \a DW_APB_SSI:ssi_memory_map_ssi_address_block:DR
 *
 * @param ri Register: DR (??), 0-35
 */
#define VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_DR(ri)  VTSS_IOREG(VTSS_TO_SSI,0x18 + (ri))

/** 
 * \brief
 * When writing to this register, you must right-justify the data. Read
 * data are automatically right-justified.
 * Read = Receive FIFO buffer
 * Write = Transmit FIFO buffer
 *
 * \details 
 * Field: ::VTSS_DW_APB_SSI_ssi_memory_map_ssi_address_block_DR . dr
 */
#define  VTSS_F_DW_APB_SSI_ssi_memory_map_ssi_address_block_DR_dr(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DW_APB_SSI_ssi_memory_map_ssi_address_block_DR_dr     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DW_APB_SSI_ssi_memory_map_ssi_address_block_DR_dr(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


#endif /* _VTSS_JAGUAR2_REGS_DW_APB_SSI_H_ */
