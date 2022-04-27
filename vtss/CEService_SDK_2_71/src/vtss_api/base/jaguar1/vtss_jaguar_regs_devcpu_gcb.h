#ifndef _VTSS_JAGUAR_REGS_DEVCPU_GCB_H_
#define _VTSS_JAGUAR_REGS_DEVCPU_GCB_H_

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
 * Target: \a DEVCPU_GCB
 *
 * General configuration block.
 *
 ***********************************************************************/

/**
 * Register Group: \a DEVCPU_GCB:CHIP_REGS
 *
 * General registers
 */


/** 
 * \brief general purpose register
 *
 * \details
 * Register: \a DEVCPU_GCB:CHIP_REGS:GENERAL_PURPOSE
 */
#define VTSS_DEVCPU_GCB_CHIP_REGS_GENERAL_PURPOSE  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x0)


/** 
 * \brief SI registers
 *
 * \details
 * Configuration of serial interface data format. This register modifies
 * how the SI receives and transmits data, when configuring this register
 * first write 0 (to get to a known state), then configure the desired
 * values.
 *
 * Register: \a DEVCPU_GCB:CHIP_REGS:SI
 */
#define VTSS_DEVCPU_GCB_CHIP_REGS_SI         VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x1)

/** 
 * \brief
 * Setup SI to use MSB or LSB first. See datasheet for more information.
 *
 * \details 
 * 0: SI expect/transmit MSB first
 * 1: SI expect/transmit LSB first
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_SI . SI_LSB
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_SI_SI_LSB  VTSS_BIT(5)

/** 
 * \brief
 * Setup SI to use either big or little endian data format. See datasheet
 * for more information.
 *
 * \details 
 * 0: SI uses little endian notation
 * 1: SI uses big endian notation
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_SI . SI_ENDIAN
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_SI_SI_ENDIAN  VTSS_BIT(4)

/** 
 * \brief
 * Configure the number of padding bytes that the SI must insert before
 * transmitting read-data during reading from the device.
 *
 * \details 
 * 0 : don't insert any padding
 * 1 : Insert 1 byte of padding
 * ...
 * 15: Insert 15 bytes of padding
 *
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_SI . SI_WAIT_STATES
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_SI_SI_WAIT_STATES(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_SI_SI_WAIT_STATES     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_SI_SI_WAIT_STATES(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief Chip Id
 *
 * \details
 * Register: \a DEVCPU_GCB:CHIP_REGS:CHIP_ID
 */
#define VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_ID    VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x2)

/** 
 * \brief
 * Revision ID.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_ID . REV_ID
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_CHIP_ID_REV_ID(x)  VTSS_ENCODE_BITFIELD(x,28,4)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_CHIP_ID_REV_ID     VTSS_ENCODE_BITMASK(28,4)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_CHIP_ID_REV_ID(x)  VTSS_EXTRACT_BITFIELD(x,28,4)

/** 
 * \brief
 * Part ID.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_ID . PART_ID
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_CHIP_ID_PART_ID(x)  VTSS_ENCODE_BITFIELD(x,12,16)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_CHIP_ID_PART_ID     VTSS_ENCODE_BITMASK(12,16)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_CHIP_ID_PART_ID(x)  VTSS_EXTRACT_BITFIELD(x,12,16)

/** 
 * \brief
 * Manufacturer's ID.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_ID . MFG_ID
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_CHIP_ID_MFG_ID(x)  VTSS_ENCODE_BITFIELD(x,1,11)
#define  VTSS_M_DEVCPU_GCB_CHIP_REGS_CHIP_ID_MFG_ID     VTSS_ENCODE_BITMASK(1,11)
#define  VTSS_X_DEVCPU_GCB_CHIP_REGS_CHIP_ID_MFG_ID(x)  VTSS_EXTRACT_BITFIELD(x,1,11)

/** 
 * \brief
 * Returns '1'
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_ID . ONE
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_CHIP_ID_ONE  VTSS_BIT(0)


/** 
 * \brief Startup state of the chip
 *
 * \details
 * Register: \a DEVCPU_GCB:CHIP_REGS:HW_STAT
 */
#define VTSS_DEVCPU_GCB_CHIP_REGS_HW_STAT    VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x3)

/** 
 * \brief
 * This field is set if a hardware fail has been detected in any of the
 * memories during startup-initialization of the chip. This field is valid
 * after release of reset.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_CHIP_REGS_HW_STAT . MEM_FAIL
 */
#define  VTSS_F_DEVCPU_GCB_CHIP_REGS_HW_STAT_MEM_FAIL  VTSS_BIT(0)

/**
 * Register Group: \a DEVCPU_GCB:DBG
 *
 * Debug registers
 */

/**
 * Register Group: \a DEVCPU_GCB:SW_REGS
 *
 * Registers for software/software interaction
 */


/** 
 * \brief Semaphore SW interrupt enable
 *
 * \details
 * Register: \a DEVCPU_GCB:SW_REGS:SEMA_INTR_ENA
 */
#define VTSS_DEVCPU_GCB_SW_REGS_SEMA_INTR_ENA  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x5)

/** 
 * \brief
 * This is a bitwise AND of SEMA_FREE and SEMA_INTR_ENA providing an fast
 * access to the cause of an interrupt, given the current mask.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_SW_REGS_SEMA_INTR_ENA . SEMA_INTR_IDENT
 */
#define  VTSS_F_DEVCPU_GCB_SW_REGS_SEMA_INTR_ENA_SEMA_INTR_IDENT(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_DEVCPU_GCB_SW_REGS_SEMA_INTR_ENA_SEMA_INTR_IDENT     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_DEVCPU_GCB_SW_REGS_SEMA_INTR_ENA_SEMA_INTR_IDENT(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Set bits in this register to enable interrupt when the corresponding
 * semaphore is free. In a multi-threaded environment, or with more than
 * one active processor the CPU_SEMA_ENA_SET and CPU_SEMA_ENA_CLR registers
 * can be used for atomic modifications of this register.
 * If interrupt is enabled for a particular semaphore, then software
 * interrupt will be asserted for as long as the semaphore is free (and
 * interrupt is enabled for that semaphore). The lower half of the
 * available semaphores are connected to software Interrupt 0 (SW0), the
 * upper half is connected to software interrupt 1 (SW1).
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_SW_REGS_SEMA_INTR_ENA . SEMA_INTR_ENA
 */
#define  VTSS_F_DEVCPU_GCB_SW_REGS_SEMA_INTR_ENA_SEMA_INTR_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_DEVCPU_GCB_SW_REGS_SEMA_INTR_ENA_SEMA_INTR_ENA     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_DEVCPU_GCB_SW_REGS_SEMA_INTR_ENA_SEMA_INTR_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Clear of semaphore SW interrupt enables
 *
 * \details
 * Register: \a DEVCPU_GCB:SW_REGS:SEMA_INTR_ENA_CLR
 */
#define VTSS_DEVCPU_GCB_SW_REGS_SEMA_INTR_ENA_CLR  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x6)

/** 
 * \brief
 * Set to clear corresponding interrupt enable in SEMA_INTR_ENA.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_SW_REGS_SEMA_INTR_ENA_CLR . SEMA_INTR_ENA_CLR
 */
#define  VTSS_F_DEVCPU_GCB_SW_REGS_SEMA_INTR_ENA_CLR_SEMA_INTR_ENA_CLR(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_DEVCPU_GCB_SW_REGS_SEMA_INTR_ENA_CLR_SEMA_INTR_ENA_CLR     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_DEVCPU_GCB_SW_REGS_SEMA_INTR_ENA_CLR_SEMA_INTR_ENA_CLR(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Masking of semaphore
 *
 * \details
 * Register: \a DEVCPU_GCB:SW_REGS:SEMA_INTR_ENA_SET
 */
#define VTSS_DEVCPU_GCB_SW_REGS_SEMA_INTR_ENA_SET  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x7)

/** 
 * \brief
 * Set to set corresponding interrupt enable in SEMA_INTR_ENA.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_SW_REGS_SEMA_INTR_ENA_SET . SEMA_INTR_ENA_SET
 */
#define  VTSS_F_DEVCPU_GCB_SW_REGS_SEMA_INTR_ENA_SET_SEMA_INTR_ENA_SET(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_DEVCPU_GCB_SW_REGS_SEMA_INTR_ENA_SET_SEMA_INTR_ENA_SET     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_DEVCPU_GCB_SW_REGS_SEMA_INTR_ENA_SET_SEMA_INTR_ENA_SET(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Semaphore register
 *
 * \details
 * Register: \a DEVCPU_GCB:SW_REGS:SEMA
 *
 * @param ri Replicator: x_NUM_SEMA (??), 0-7
 */
#define VTSS_DEVCPU_GCB_SW_REGS_SEMA(ri)     VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x8 + (ri))

/** 
 * \brief
 * General Semaphore.The process to read this field will read a '1' and
 * thus be granted the semaphore. The semaphore is released by the
 * interface by writing a '1' to this field.
 *
 * \details 
 * Read :
 * '0': Semaphore was not granted.
 * '1': Semaphore was granted.
 * 
 * Write : 
 * '0': No action.
 * '1': Release semaphore.
 *
 * Field: ::VTSS_DEVCPU_GCB_SW_REGS_SEMA . SEMA
 */
#define  VTSS_F_DEVCPU_GCB_SW_REGS_SEMA_SEMA  VTSS_BIT(0)


/** 
 * \brief Semaphore status
 *
 * \details
 * Register: \a DEVCPU_GCB:SW_REGS:SEMA_FREE
 */
#define VTSS_DEVCPU_GCB_SW_REGS_SEMA_FREE    VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x10)

/** 
 * \brief
 * Show which semaphores that are currently free.
 *
 * \details 
 * '0' : Corresponding semaphore is taken.
 * '1' : Corresponding semaphore is free.
 *
 * Field: ::VTSS_DEVCPU_GCB_SW_REGS_SEMA_FREE . SEMA_FREE
 */
#define  VTSS_F_DEVCPU_GCB_SW_REGS_SEMA_FREE_SEMA_FREE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_DEVCPU_GCB_SW_REGS_SEMA_FREE_SEMA_FREE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_DEVCPU_GCB_SW_REGS_SEMA_FREE_SEMA_FREE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Manually assert software interrupt
 *
 * \details
 * This register provides a simple interface for interrupting on either
 * software interrupt 0 or 1, without implementing semaphore support. Note:
 * setting this field causes a short pulse on the corresponding interrupt
 * connection, this kind of interrupt cannot be used in combination with
 * the SW1_INTR_CONFIG.SW1_INTR_BYPASS feature.
 *
 * Register: \a DEVCPU_GCB:SW_REGS:SW_INTR
 */
#define VTSS_DEVCPU_GCB_SW_REGS_SW_INTR      VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x11)

/** 
 * \brief
 * Set this field to inject software interrupt 1. This field is
 * automatically cleared after interrupt has been generated.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_SW_REGS_SW_INTR . SW1_INTR
 */
#define  VTSS_F_DEVCPU_GCB_SW_REGS_SW_INTR_SW1_INTR  VTSS_BIT(1)

/** 
 * \brief
 * Set this field to assert software interrupt 0. This field is
 * automatically cleared after interrupt has been generated.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_SW_REGS_SW_INTR . SW0_INTR
 */
#define  VTSS_F_DEVCPU_GCB_SW_REGS_SW_INTR_SW0_INTR  VTSS_BIT(0)


/** 
 * \brief Mailbox register
 *
 * \details
 * Register: \a DEVCPU_GCB:SW_REGS:MAILBOX
 */
#define VTSS_DEVCPU_GCB_SW_REGS_MAILBOX      VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x12)


/** 
 * \brief Mailbox register atomic clear
 *
 * \details
 * Register: \a DEVCPU_GCB:SW_REGS:MAILBOX_CLR
 */
#define VTSS_DEVCPU_GCB_SW_REGS_MAILBOX_CLR  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x13)


/** 
 * \brief Mailbox register atomic set
 *
 * \details
 * Register: \a DEVCPU_GCB:SW_REGS:MAILBOX_SET
 */
#define VTSS_DEVCPU_GCB_SW_REGS_MAILBOX_SET  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x14)

/**
 * Register Group: \a DEVCPU_GCB:VCORE_ACCESS
 *
 * Registers for access to the VCore SBA
 */


/** 
 * \brief Control register for VCore accesses
 *
 * \details
 * Register: \a DEVCPU_GCB:VCORE_ACCESS:VA_CTRL
 */
#define VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x15)

/** 
 * \brief
 * This field is set to the value of VA_CTRL:VA_ERR whenever one of the
 * data registers ACC_DATA, ACC_DATA_INCR, or ACC_DATA_RO is read. By
 * reading this field it is possible to determine if the last read-value
 * from one of these registers was erred.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL . VA_ERR_RD
 */
#define  VTSS_F_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL_VA_ERR_RD  VTSS_BIT(3)

/** 
 * \brief
 * This field is set if the access inside the VCore domain was terminated
 * by an error. This situation can occur when accessing an unmapped part of
 * the VCore memory-map or when accessing a target that reports error (e.g.
 * accessing uninitialized DDR2 memory).
 * If an error occurs during reading, the read-data will be 0x80000000. So
 * as an optimization, software only has to check for error if 0x80000000
 * is returned (and in that case VA_ERR_RD should be checked). When writing
 * you should always check if sucessful.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL . VA_ERR
 */
#define  VTSS_F_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL_VA_ERR  VTSS_BIT(2)

/** 
 * \brief
 * This field is set to the value of VA_CTRL:VA_BUSY whenever one of the
 * data registers ACC_DATA, ACC_DATA_INCR, or ACC_DATA_RO is read. By
 * reading this field it is possible to determine if the last read-value
 * from one of these registers was valid.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL . VA_BUSY_RD
 */
#define  VTSS_F_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL_VA_BUSY_RD  VTSS_BIT(1)

/** 
 * \brief
 * This field is set by hardware when an access into VCore domain is
 * started, and cleared when the access is done.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL . VA_BUSY
 */
#define  VTSS_F_DEVCPU_GCB_VCORE_ACCESS_VA_CTRL_VA_BUSY  VTSS_BIT(0)


/** 
 * \brief Address register for VCore accesses
 *
 * \details
 * Register: \a DEVCPU_GCB:VCORE_ACCESS:VA_ADDR
 */
#define VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_ADDR  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x16)


/** 
 * \brief Data register for VCore accesses
 *
 * \details
 * The VA_DATA, VA_DATA_INCR, and VA_DATA_INERT registers are used for
 * indirect access into the VCore domain. The functionality of the
 * VA_DATA_INCR and VA_DATA_INERT registers are similar to this register -
 * but with minor exceptions. These exceptions are fleshed out in the
 * description of the respective registers.
 *
 * Register: \a DEVCPU_GCB:VCORE_ACCESS:VA_DATA
 */
#define VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_DATA  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x17)


/** 
 * \brief Data register for VCore accesses (w. auto increment of address)
 *
 * \details
 * Register: \a DEVCPU_GCB:VCORE_ACCESS:VA_DATA_INCR
 */
#define VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_DATA_INCR  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x18)


/** 
 * \brief Data register for VCore accesses (will not initiate access)
 *
 * \details
 * Register: \a DEVCPU_GCB:VCORE_ACCESS:VA_DATA_INERT
 */
#define VTSS_DEVCPU_GCB_VCORE_ACCESS_VA_DATA_INERT  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x19)

/**
 * Register Group: \a DEVCPU_GCB:GPIO
 *
 * GPIO registers
 */


/** 
 * \brief GPIO output set
 *
 * \details
 * Register: \a DEVCPU_GCB:GPIO:GPIO_OUT_SET
 */
#define VTSS_DEVCPU_GCB_GPIO_GPIO_OUT_SET    VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x1a)

/** 
 * \brief
 * Setting a bit in this field will immediately set the corresponding bit
 * in GPIO_O::G_OUT. Reading this register always return 0.
 *
 * \details 
 * '0': No change
 * '1': Corresponding bit in GPIO_O::OUT is set.
 *
 * Field: ::VTSS_DEVCPU_GCB_GPIO_GPIO_OUT_SET . G_OUT_SET
 */
#define  VTSS_F_DEVCPU_GCB_GPIO_GPIO_OUT_SET_G_OUT_SET(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_DEVCPU_GCB_GPIO_GPIO_OUT_SET_G_OUT_SET     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_DEVCPU_GCB_GPIO_GPIO_OUT_SET_G_OUT_SET(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief GPIO output clear
 *
 * \details
 * Register: \a DEVCPU_GCB:GPIO:GPIO_OUT_CLR
 */
#define VTSS_DEVCPU_GCB_GPIO_GPIO_OUT_CLR    VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x1b)

/** 
 * \brief
 * Setting a bit in this field will immediately clear the corresponding bit
 * in GPIO_O::G_OUT. Reading this register always return 0.
 *
 * \details 
 * '0': No change
 * '1': Corresponding bit in GPIO_O::OUT is cleared.
 *
 * Field: ::VTSS_DEVCPU_GCB_GPIO_GPIO_OUT_CLR . G_OUT_CLR
 */
#define  VTSS_F_DEVCPU_GCB_GPIO_GPIO_OUT_CLR_G_OUT_CLR(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_DEVCPU_GCB_GPIO_GPIO_OUT_CLR_G_OUT_CLR     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_DEVCPU_GCB_GPIO_GPIO_OUT_CLR_G_OUT_CLR(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief GPIO output
 *
 * \details
 * In a multi-threaded software environment using the registers
 * GPIO_OUT_SET and GPIO_OUT_CLR for modifying GPIO values removes the need
 * for software-locked access.
 *
 * Register: \a DEVCPU_GCB:GPIO:GPIO_OUT
 */
#define VTSS_DEVCPU_GCB_GPIO_GPIO_OUT        VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x1c)

/** 
 * \brief
 * Controls the value on the GPIO pins enabled for output (via the GPIO_OE
 * register). This field can be modified directly or by using the
 * GPIO_O_SET and GPIO_O_CLR registers.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_GPIO_GPIO_OUT . G_OUT
 */
#define  VTSS_F_DEVCPU_GCB_GPIO_GPIO_OUT_G_OUT(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_DEVCPU_GCB_GPIO_GPIO_OUT_G_OUT     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_DEVCPU_GCB_GPIO_GPIO_OUT_G_OUT(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief GPIO input
 *
 * \details
 * Register: \a DEVCPU_GCB:GPIO:GPIO_IN
 */
#define VTSS_DEVCPU_GCB_GPIO_GPIO_IN         VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x1d)

/** 
 * \brief
 * GPIO input register. Reflects the current state of the corresponding
 * GPIO pins.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_GPIO_GPIO_IN . G_IN
 */
#define  VTSS_F_DEVCPU_GCB_GPIO_GPIO_IN_G_IN(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_DEVCPU_GCB_GPIO_GPIO_IN_G_IN     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_DEVCPU_GCB_GPIO_GPIO_IN_G_IN(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief GPIO pin direction
 *
 * \details
 * Register: \a DEVCPU_GCB:GPIO:GPIO_OE
 */
#define VTSS_DEVCPU_GCB_GPIO_GPIO_OE         VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x1e)

/** 
 * \brief
 * Configures the direction of the GPIO pins.
 *
 * \details 
 * '0': Input
 * '1': Output
 *
 * Field: ::VTSS_DEVCPU_GCB_GPIO_GPIO_OE . G_OE
 */
#define  VTSS_F_DEVCPU_GCB_GPIO_GPIO_OE_G_OE(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_DEVCPU_GCB_GPIO_GPIO_OE_G_OE     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_DEVCPU_GCB_GPIO_GPIO_OE_G_OE(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief GPIO interrupt
 *
 * \details
 * Register: \a DEVCPU_GCB:GPIO:GPIO_INTR
 */
#define VTSS_DEVCPU_GCB_GPIO_GPIO_INTR       VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x1f)

/** 
 * \brief
 * Indicates whether a GPIO input has changed since last clear.
 *
 * \details 
 * '0': No change
 * '1': GPIO has changed
 *
 * Field: ::VTSS_DEVCPU_GCB_GPIO_GPIO_INTR . G_INTR
 */
#define  VTSS_F_DEVCPU_GCB_GPIO_GPIO_INTR_G_INTR(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_DEVCPU_GCB_GPIO_GPIO_INTR_G_INTR     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_DEVCPU_GCB_GPIO_GPIO_INTR_G_INTR(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief GPIO interrupt enable
 *
 * \details
 * Register: \a DEVCPU_GCB:GPIO:GPIO_INTR_ENA
 */
#define VTSS_DEVCPU_GCB_GPIO_GPIO_INTR_ENA   VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x20)

/** 
 * \brief
 * Enables individual GPIO pins for interrupt.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_GPIO_GPIO_INTR_ENA . G_INTR_ENA
 */
#define  VTSS_F_DEVCPU_GCB_GPIO_GPIO_INTR_ENA_G_INTR_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_DEVCPU_GCB_GPIO_GPIO_INTR_ENA_G_INTR_ENA     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_DEVCPU_GCB_GPIO_GPIO_INTR_ENA_G_INTR_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief GPIO interrupt identity
 *
 * \details
 * Register: \a DEVCPU_GCB:GPIO:GPIO_INTR_IDENT
 */
#define VTSS_DEVCPU_GCB_GPIO_GPIO_INTR_IDENT  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x21)

/** 
 * \brief
 * Shows which GPIO sources that are currently interrupting. This field is
 * the result of an AND-operation between the GPIO_INTR and the
 * GPIO_INTR_ENA registers.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_GPIO_GPIO_INTR_IDENT . G_INTR_IDENT
 */
#define  VTSS_F_DEVCPU_GCB_GPIO_GPIO_INTR_IDENT_G_INTR_IDENT(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_DEVCPU_GCB_GPIO_GPIO_INTR_IDENT_G_INTR_IDENT     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_DEVCPU_GCB_GPIO_GPIO_INTR_IDENT_G_INTR_IDENT(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief GPIO alternate functions
 *
 * \details
 * Register: \a DEVCPU_GCB:GPIO:GPIO_ALT
 *
 * @param ri Register: GPIO_ALT (??), 0-1
 */
#define VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(ri)    VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x22 + (ri))

/** 
 * \brief
 * Configures alternate functions for individual GPIO bits. This field is
 * replicated two times, the functionality of the GPIO is determined by the
 * bit in this field corresponding to the GPIO for BOTH replications.
 * For example, to enable aternate function 1 for GPIO number 3; then bit 3
 * in G_ALT[0] must be set and bit 3 in G_ALT[1] must be cleared.
 * The encoding describes the rsesult of setting bits in both replications
 * of this field per GPIO. That is, the encoding shows the following
 * concatenation "G_ALT[1] & G_ALT[0]" per GPIO.
 *
 * \details 
 * "00": GPIO mode
 * "01": Alternate mode 1
 * "10": Reserved
 * "11": Reserved
 *
 * Field: ::VTSS_DEVCPU_GCB_GPIO_GPIO_ALT . G_ALT
 */
#define  VTSS_F_DEVCPU_GCB_GPIO_GPIO_ALT_G_ALT(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_DEVCPU_GCB_GPIO_GPIO_ALT_G_ALT     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_DEVCPU_GCB_GPIO_GPIO_ALT_G_ALT(x)  VTSS_EXTRACT_BITFIELD(x,0,24)

/**
 * Register Group: \a DEVCPU_GCB:DEVCPU_RST_REGS
 *
 * Control of resets in the cip
 */


/** 
 * \brief Reset part or the whole chip
 *
 * \details
 * Register: \a DEVCPU_GCB:DEVCPU_RST_REGS:SOFT_CHIP_RST
 */
#define VTSS_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_CHIP_RST  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x24)

/** 
 * \brief
 * Set this field to reset everything but the chip configuration. The reset
 * remains asserted until this field is cleared.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_CHIP_RST . SOFT_NON_CFG_RST
 */
#define  VTSS_F_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_CHIP_RST_SOFT_NON_CFG_RST  VTSS_BIT(1)

/** 
 * \brief
 * Set this field to reset the whole chip. This field is automatically
 * cleared by the reset.
 * Note: It is possible for the VCore to protect itself from soft-reset of
 * the chip, for more info see RESET.CORE_RST_PROTECT inside the VCore
 * register space.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_CHIP_RST . SOFT_CHIP_RST
 */
#define  VTSS_F_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_CHIP_RST_SOFT_CHIP_RST  VTSS_BIT(0)


/** 
 * \brief Soft reset of devcpu.
 *
 * \details
 * Register: \a DEVCPU_GCB:DEVCPU_RST_REGS:SOFT_DEVCPU_RST
 */
#define VTSS_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_DEVCPU_RST  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x25)

/** 
 * \brief
 * Set this field to reset the extraction logic. The reset remains asserted
 * until this field is cleared.
 * Note: Extraction logic is also reset while
 * SOFT_CHIP_RST.SOFT_NON_CFG_RST is set.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_DEVCPU_RST . SOFT_XTR_RST
 */
#define  VTSS_F_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_DEVCPU_RST_SOFT_XTR_RST  VTSS_BIT(1)

/** 
 * \brief
 * Set this field to reset the injection logic. The reset remains asserted
 * until this field is cleared.
 * Note: Injection logic is also reset while SOFT_CHIP_RST.SOFT_NON_CFG_RST
 * is set.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_DEVCPU_RST . SOFT_INJ_RST
 */
#define  VTSS_F_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_DEVCPU_RST_SOFT_INJ_RST  VTSS_BIT(0)

/**
 * Register Group: \a DEVCPU_GCB:ALIGN
 *
 * Configruation of the CSR ring
 */


/** 
 * \brief WD conf
 *
 * \details
 * Register: \a DEVCPU_GCB:ALIGN:WD_SETUP
 */
#define VTSS_DEVCPU_GCB_ALIGN_WD_SETUP       VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x26)

/**
 * Register Group: \a DEVCPU_GCB:MIIM
 *
 * Not documented
 */


/** 
 * \brief MIIM Status
 *
 * \details
 * Register: \a DEVCPU_GCB:MIIM:MII_STATUS
 *
 * @param gi Replicator: x_NUM_OF_MIIM_INTERFACES (??), 0-1
 */
#define VTSS_DEVCPU_GCB_MIIM_MII_STATUS(gi)  VTSS_IOREG_IX(VTSS_TO_DEVCPU_GCB,0x28,gi,9,0,0)

/** 
 * \brief
 * Indicates the current state of the MIIM controller. When read operations
 * are done (no longer busy), then read data is available via the
 * DEVCPU_GCB::MII_DATA register.
 *
 * \details 
 * 0: MIIM controller is in idle state
 * 1: MIIM controller is busy performing MIIM cmd (Either read or read
 * cmd).
 *
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MII_STATUS . MIIM_STAT_BUSY
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MII_STATUS_MIIM_STAT_BUSY  VTSS_BIT(3)

/** 
 * \brief
 * The MIIM controller has a CMD fifo of depth one. When this field is 0,
 * then it is safe to write another MIIM command to the MIIM controller.
 *
 * \details 
 * 0 : Read or write not pending
 * 1 : Read or write pending.
 *
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MII_STATUS . MIIM_STAT_OPR_PEND
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MII_STATUS_MIIM_STAT_OPR_PEND  VTSS_BIT(2)

/** 
 * \brief
 * Indicates whether a read operation via the MIIM interface is in progress
 * or not.
 *
 * \details 
 * 0 : Read not in progress
 * 1 : Read in progress.
 *
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MII_STATUS . MIIM_STAT_PENDING_RD
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MII_STATUS_MIIM_STAT_PENDING_RD  VTSS_BIT(1)

/** 
 * \brief
 * Indicates whether a write operation via the MIIM interface is in
 * progress or not.
 *
 * \details 
 * 0 : Write not in progress
 * 1 : Write in progress.
 *
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MII_STATUS . MIIM_STAT_PENDING_WR
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MII_STATUS_MIIM_STAT_PENDING_WR  VTSS_BIT(0)

/** 
 * \brief
 * Signals if all PHYs have been scanned ( with auto scan ) at least once.
 *
 * \details 
 * 0 : Auto scan has not scanned all PHYs.
 * 1 : Auto scan has scanned all PHY at least once.
 *
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MII_STATUS . MIIM_SCAN_COMPLETE
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MII_STATUS_MIIM_SCAN_COMPLETE  VTSS_BIT(4)


/** 
 * \brief MIIM Command
 *
 * \details
 * Register: \a DEVCPU_GCB:MIIM:MII_CMD
 *
 * @param gi Replicator: x_NUM_OF_MIIM_INTERFACES (??), 0-1
 */
#define VTSS_DEVCPU_GCB_MIIM_MII_CMD(gi)     VTSS_IOREG_IX(VTSS_TO_DEVCPU_GCB,0x28,gi,9,0,2)

/** 
 * \brief
 * Must be set for starting a new PHY access. This bit is automatically
 * cleared.
 *
 * \details 
 * 0 : Write to this register is ignored.
 * 1 : Write to this register is processed.
 *
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MII_CMD . MIIM_CMD_VLD
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_VLD  VTSS_BIT(31)

/** 
 * \brief
 * Indicates the addressed PHY number.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MII_CMD . MIIM_CMD_PHYAD
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_PHYAD(x)  VTSS_ENCODE_BITFIELD(x,25,5)
#define  VTSS_M_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_PHYAD     VTSS_ENCODE_BITMASK(25,5)
#define  VTSS_X_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_PHYAD(x)  VTSS_EXTRACT_BITFIELD(x,25,5)

/** 
 * \brief
 * Indicates the addressed of the register within the PHY that shall be
 * accessed.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MII_CMD . MIIM_CMD_REGAD
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_REGAD(x)  VTSS_ENCODE_BITFIELD(x,20,5)
#define  VTSS_M_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_REGAD     VTSS_ENCODE_BITMASK(20,5)
#define  VTSS_X_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_REGAD(x)  VTSS_EXTRACT_BITFIELD(x,20,5)

/** 
 * \brief
 * Data to be written in the PHY register.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MII_CMD . MIIM_CMD_WRDATA
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_WRDATA(x)  VTSS_ENCODE_BITFIELD(x,4,16)
#define  VTSS_M_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_WRDATA     VTSS_ENCODE_BITMASK(4,16)
#define  VTSS_X_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_WRDATA(x)  VTSS_EXTRACT_BITFIELD(x,4,16)

/** 
 * \brief
 * Select if scanning of the PHY shall be done once, or scanning should be
 * done continuously.
 *
 * \details 
 * 0 : Do continuously PHY scanning 
 * 1 : Stop once all PHY have been scanned.
 *
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MII_CMD . MIIM_CMD_SINGLE_SCAN
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_SINGLE_SCAN  VTSS_BIT(3)

/** 
 * \brief
 * Indicates type of operation.
 *
 * \details 
 * Clause 22:
 * 
 * 01 : Write
 * 10 : Read
 * 
 * Clause 45:
 * 
 * 00 : Address
 * 01 : Write
 * 10 : Read inc.
 * 11 : Read.
 *
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MII_CMD . MIIM_CMD_OPR_FIELD
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_OPR_FIELD(x)  VTSS_ENCODE_BITFIELD(x,1,2)
#define  VTSS_M_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_OPR_FIELD     VTSS_ENCODE_BITMASK(1,2)
#define  VTSS_X_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_OPR_FIELD(x)  VTSS_EXTRACT_BITFIELD(x,1,2)

/** 
 * \brief
 * Indicates whether automatic scanning of PHY registers is enabled. When
 * enabled, the PHY-number for each automatic read is continuously
 * round-robined from PHY_ADDR_LOW through PHY_ADDR_HIGH. This function is
 * started upon a read operation (ACCESS_TYPE).
 * Scan MUST be disabled when doing any configuration of the MIIM
 * controller.
 *
 * \details 
 * 0 : Disabled
 * 1 : Enabled.
 *
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MII_CMD . MIIM_CMD_SCAN
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_SCAN  VTSS_BIT(0)


/** 
 * \brief MIIM Reply Data
 *
 * \details
 * Register: \a DEVCPU_GCB:MIIM:MII_DATA
 *
 * @param gi Replicator: x_NUM_OF_MIIM_INTERFACES (??), 0-1
 */
#define VTSS_DEVCPU_GCB_MIIM_MII_DATA(gi)    VTSS_IOREG_IX(VTSS_TO_DEVCPU_GCB,0x28,gi,9,0,3)

/** 
 * \brief
 * Indicates whether a read operation failed or succeeded.
 *
 * \details 
 * 00 : OK
 * 11 : Error
 *
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MII_DATA . MIIM_DATA_SUCCESS
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MII_DATA_MIIM_DATA_SUCCESS(x)  VTSS_ENCODE_BITFIELD(x,16,2)
#define  VTSS_M_DEVCPU_GCB_MIIM_MII_DATA_MIIM_DATA_SUCCESS     VTSS_ENCODE_BITMASK(16,2)
#define  VTSS_X_DEVCPU_GCB_MIIM_MII_DATA_MIIM_DATA_SUCCESS(x)  VTSS_EXTRACT_BITFIELD(x,16,2)

/** 
 * \brief
 * Data read from PHY register.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MII_DATA . MIIM_DATA_RDDATA
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MII_DATA_MIIM_DATA_RDDATA(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DEVCPU_GCB_MIIM_MII_DATA_MIIM_DATA_RDDATA     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DEVCPU_GCB_MIIM_MII_DATA_MIIM_DATA_RDDATA(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief MIIM Configuration
 *
 * \details
 * Register: \a DEVCPU_GCB:MIIM:MII_CFG
 *
 * @param gi Replicator: x_NUM_OF_MIIM_INTERFACES (??), 0-1
 */
#define VTSS_DEVCPU_GCB_MIIM_MII_CFG(gi)     VTSS_IOREG_IX(VTSS_TO_DEVCPU_GCB,0x28,gi,9,0,4)

/** 
 * \brief
 * Configures the MIIM clock frequency. This is computed as
 * system_clk/(2*(1+X)), where X is the value written to this register.
 * Note : Setting X to 0 is invalid and will result in the same frequency
 * as setting X to 1.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MII_CFG . MIIM_CFG_PRESCALE
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MII_CFG_MIIM_CFG_PRESCALE(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_DEVCPU_GCB_MIIM_MII_CFG_MIIM_CFG_PRESCALE     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_DEVCPU_GCB_MIIM_MII_CFG_MIIM_CFG_PRESCALE(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/** 
 * \brief
 * The ST (start-of-frame) field of the MIIM frame format adopts the value
 * of this field. This must be configured for either clause 22 or 45 MIIM
 * operation.
 *
 * \details 
 * "01": Clause 22
 * "00": Clause 45
 * Other values are reserved.
 *
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MII_CFG . MIIM_ST_CFG_FIELD
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MII_CFG_MIIM_ST_CFG_FIELD(x)  VTSS_ENCODE_BITFIELD(x,9,2)
#define  VTSS_M_DEVCPU_GCB_MIIM_MII_CFG_MIIM_ST_CFG_FIELD     VTSS_ENCODE_BITMASK(9,2)
#define  VTSS_X_DEVCPU_GCB_MIIM_MII_CFG_MIIM_ST_CFG_FIELD(x)  VTSS_EXTRACT_BITFIELD(x,9,2)


/** 
 * \brief MIIM Scan 0
 *
 * \details
 * Register: \a DEVCPU_GCB:MIIM:MII_SCAN_0
 *
 * @param gi Replicator: x_NUM_OF_MIIM_INTERFACES (??), 0-1
 */
#define VTSS_DEVCPU_GCB_MIIM_MII_SCAN_0(gi)  VTSS_IOREG_IX(VTSS_TO_DEVCPU_GCB,0x28,gi,9,0,5)

/** 
 * \brief
 * Indicates the high PHY number to scan during automatic scanning.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MII_SCAN_0 . MIIM_SCAN_PHYADHI
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MII_SCAN_0_MIIM_SCAN_PHYADHI(x)  VTSS_ENCODE_BITFIELD(x,5,5)
#define  VTSS_M_DEVCPU_GCB_MIIM_MII_SCAN_0_MIIM_SCAN_PHYADHI     VTSS_ENCODE_BITMASK(5,5)
#define  VTSS_X_DEVCPU_GCB_MIIM_MII_SCAN_0_MIIM_SCAN_PHYADHI(x)  VTSS_EXTRACT_BITFIELD(x,5,5)

/** 
 * \brief
 * Indicates the low PHY number to scan during automatic scanning.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MII_SCAN_0 . MIIM_SCAN_PHYADLO
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MII_SCAN_0_MIIM_SCAN_PHYADLO(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_DEVCPU_GCB_MIIM_MII_SCAN_0_MIIM_SCAN_PHYADLO     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_DEVCPU_GCB_MIIM_MII_SCAN_0_MIIM_SCAN_PHYADLO(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief MIIM Scan 1
 *
 * \details
 * Register: \a DEVCPU_GCB:MIIM:MII_SCAN_1
 *
 * @param gi Replicator: x_NUM_OF_MIIM_INTERFACES (??), 0-1
 */
#define VTSS_DEVCPU_GCB_MIIM_MII_SCAN_1(gi)  VTSS_IOREG_IX(VTSS_TO_DEVCPU_GCB,0x28,gi,9,0,6)

/** 
 * \brief
 * Indicates the mask for comparing the PHY registers during automatic
 * scan.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MII_SCAN_1 . MIIM_SCAN_MASK
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MII_SCAN_1_MIIM_SCAN_MASK(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_DEVCPU_GCB_MIIM_MII_SCAN_1_MIIM_SCAN_MASK     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_DEVCPU_GCB_MIIM_MII_SCAN_1_MIIM_SCAN_MASK(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Indicates the expected value for comparing the PHY registers during
 * automatic scan.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_MIIM_MII_SCAN_1 . MIIM_SCAN_EXPECT
 */
#define  VTSS_F_DEVCPU_GCB_MIIM_MII_SCAN_1_MIIM_SCAN_EXPECT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DEVCPU_GCB_MIIM_MII_SCAN_1_MIIM_SCAN_EXPECT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DEVCPU_GCB_MIIM_MII_SCAN_1_MIIM_SCAN_EXPECT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief MIIM Results
 *
 * \details
 * Register: \a DEVCPU_GCB:MIIM:MII_SCAN_LAST_RSLTS
 *
 * @param gi Replicator: x_NUM_OF_MIIM_INTERFACES (??), 0-1
 */
#define VTSS_DEVCPU_GCB_MIIM_MII_SCAN_LAST_RSLTS(gi)  VTSS_IOREG_IX(VTSS_TO_DEVCPU_GCB,0x28,gi,9,0,7)


/** 
 * \brief MIIM Results
 *
 * \details
 * Register: \a DEVCPU_GCB:MIIM:MII_SCAN_LAST_RSLTS_VLD
 *
 * @param gi Replicator: x_NUM_OF_MIIM_INTERFACES (??), 0-1
 */
#define VTSS_DEVCPU_GCB_MIIM_MII_SCAN_LAST_RSLTS_VLD(gi)  VTSS_IOREG_IX(VTSS_TO_DEVCPU_GCB,0x28,gi,9,0,8)

/**
 * Register Group: \a DEVCPU_GCB:MIIM_READ_SCAN
 *
 * Not documented
 */


/** 
 * \brief MIIM Results
 *
 * \details
 * Register: \a DEVCPU_GCB:MIIM_READ_SCAN:MII_SCAN_RSLTS_STICKY
 *
 * @param ri Replicator: x_NUM_OF_MIIM_INTERFACES (??), 0-1
 */
#define VTSS_DEVCPU_GCB_MIIM_READ_SCAN_MII_SCAN_RSLTS_STICKY(ri)  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x3a + (ri))

/**
 * Register Group: \a DEVCPU_GCB:VAUI2_MUX
 *
 * Group for the control signals of the vaui2 mux.
 */


/** 
 * \brief DEV10G, DEV2G5 mux configuration
 *
 * \details
 * Register: \a DEVCPU_GCB:VAUI2_MUX:VAUI2_MUX
 */
#define VTSS_DEVCPU_GCB_VAUI2_MUX_VAUI2_MUX  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x3c)

/** 
 * \brief
 * Configures multiplexing of Ethernet configurations.
 *
 * \details 
 * 0: No 2.5G support
 * 1: 4x 2.5G support
 * 7: 8x 2.5G support
 * Other values are reserved.
 *
 * Field: ::VTSS_DEVCPU_GCB_VAUI2_MUX_VAUI2_MUX . MUX_CONF
 */
#define  VTSS_F_DEVCPU_GCB_VAUI2_MUX_VAUI2_MUX_MUX_CONF(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_DEVCPU_GCB_VAUI2_MUX_VAUI2_MUX_MUX_CONF     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_DEVCPU_GCB_VAUI2_MUX_VAUI2_MUX_MUX_CONF(x)  VTSS_EXTRACT_BITFIELD(x,0,3)

/**
 * Register Group: \a DEVCPU_GCB:FEATURES
 *
 * Chip features
 */


/** 
 * \brief Chip features
 *
 * \details
 * This register shows which of the 0x7460 family features that is
 * implemented by this chip.
 *
 * Register: \a DEVCPU_GCB:FEATURES:FEA_STAT
 */
#define VTSS_DEVCPU_GCB_FEATURES_FEA_STAT    VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x3d)

/** 
 * \brief
 * Support for MIPS CPU.
 *
 * \details 
 * 0: Available
 * 1: Unsupported
 *
 * Field: ::VTSS_DEVCPU_GCB_FEATURES_FEA_STAT . FEA_MIPS
 */
#define  VTSS_F_DEVCPU_GCB_FEATURES_FEA_STAT_FEA_MIPS  VTSS_BIT(22)

/** 
 * \brief
 * Support for hardware-based CCM checker.
 *
 * \details 
 * 0: Available
 * 1: Unsupported
 *
 * Field: ::VTSS_DEVCPU_GCB_FEATURES_FEA_STAT . FEA_CCM_CHK
 */
#define  VTSS_F_DEVCPU_GCB_FEATURES_FEA_STAT_FEA_CCM_CHK  VTSS_BIT(21)

/** 
 * \brief
 * Support for MPLS BITW FPGA.
 *
 * \details 
 * 0: Available
 * 1: Unsupported
 *
 * Field: ::VTSS_DEVCPU_GCB_FEATURES_FEA_STAT . FEA_MPLS_BITW
 */
#define  VTSS_F_DEVCPU_GCB_FEATURES_FEA_STAT_FEA_MPLS_BITW  VTSS_BIT(20)

/** 
 * \brief
 * Support for MPLS classification.
 *
 * \details 
 * 0: Available
 * 1: Unsupported
 *
 * Field: ::VTSS_DEVCPU_GCB_FEATURES_FEA_STAT . FEA_MPLS_CLASS
 */
#define  VTSS_F_DEVCPU_GCB_FEATURES_FEA_STAT_FEA_MPLS_CLASS  VTSS_BIT(19)

/** 
 * \brief
 * Supported port-configuration.
 *
 * \details 
 * 0: 24+4 configurations are supported.
 * 1: 12+2 configurations are supported.
 *
 * Field: ::VTSS_DEVCPU_GCB_FEATURES_FEA_STAT . FEA_PORTCFG
 */
#define  VTSS_F_DEVCPU_GCB_FEATURES_FEA_STAT_FEA_PORTCFG  VTSS_BIT(18)

/** 
 * \brief
 * Support for frame switching.
 *
 * \details 
 * 0: Available
 * 1: Unsupported
 *
 * Field: ::VTSS_DEVCPU_GCB_FEATURES_FEA_STAT . FEA_SWITCHING
 */
#define  VTSS_F_DEVCPU_GCB_FEATURES_FEA_STAT_FEA_SWITCHING  VTSS_BIT(17)

/** 
 * \brief
 * Support for service concept.
 *
 * \details 
 * 0: Available
 * 1: Unsupported
 *
 * Field: ::VTSS_DEVCPU_GCB_FEATURES_FEA_STAT . FEA_SERVICES
 */
#define  VTSS_F_DEVCPU_GCB_FEATURES_FEA_STAT_FEA_SERVICES  VTSS_BIT(16)

/** 
 * \brief
 * Support for dual domain processing.
 *
 * \details 
 * 0: Available
 * 1: Unsupported
 *
 * Field: ::VTSS_DEVCPU_GCB_FEATURES_FEA_STAT . FEA_DUAL_DOMAIN
 */
#define  VTSS_F_DEVCPU_GCB_FEATURES_FEA_STAT_FEA_DUAL_DOMAIN  VTSS_BIT(15)

/** 
 * \brief
 * Suport for routing.
 *
 * \details 
 * 0: Available
 * 1: Unsupported
 *
 * Field: ::VTSS_DEVCPU_GCB_FEATURES_FEA_STAT . FEA_ROUTING
 */
#define  VTSS_F_DEVCPU_GCB_FEATURES_FEA_STAT_FEA_ROUTING  VTSS_BIT(14)

/** 
 * \brief
 * Support for IEEE1588.
 *
 * \details 
 * 0: Available
 * 1: Unsupported
 *
 * Field: ::VTSS_DEVCPU_GCB_FEATURES_FEA_STAT . FEA_1588
 */
#define  VTSS_F_DEVCPU_GCB_FEATURES_FEA_STAT_FEA_1588  VTSS_BIT(13)

/** 
 * \brief
 * Supported number of XAUI ports. This is a per port mask, bit 0 in this
 * field corresponds to XAUI port 0, bit 1 to port 1, etc.
 *
 * \details 
 * 0: Available
 * 1: Unsupported
 *
 * Field: ::VTSS_DEVCPU_GCB_FEATURES_FEA_STAT . FEA_XAUI
 */
#define  VTSS_F_DEVCPU_GCB_FEATURES_FEA_STAT_FEA_XAUI(x)  VTSS_ENCODE_BITFIELD(x,9,4)
#define  VTSS_M_DEVCPU_GCB_FEATURES_FEA_STAT_FEA_XAUI     VTSS_ENCODE_BITMASK(9,4)
#define  VTSS_X_DEVCPU_GCB_FEATURES_FEA_STAT_FEA_XAUI(x)  VTSS_EXTRACT_BITFIELD(x,9,4)

/** 
 * \brief
 * Memory size.
 *
 * \details 
 * 0: 32MBit
 * 1: 16MBit
 * 2: 8MBit
 *
 * Field: ::VTSS_DEVCPU_GCB_FEATURES_FEA_STAT . FEA_MEM
 */
#define  VTSS_F_DEVCPU_GCB_FEATURES_FEA_STAT_FEA_MEM(x)  VTSS_ENCODE_BITFIELD(x,7,2)
#define  VTSS_M_DEVCPU_GCB_FEATURES_FEA_STAT_FEA_MEM     VTSS_ENCODE_BITMASK(7,2)
#define  VTSS_X_DEVCPU_GCB_FEATURES_FEA_STAT_FEA_MEM(x)  VTSS_EXTRACT_BITFIELD(x,7,2)

/** 
 * \brief
 * MAC Table size.
 *
 * \details 
 * 0/1: 32K
 * 2: 16K
 * 3: 8K
 *
 * Field: ::VTSS_DEVCPU_GCB_FEATURES_FEA_STAT . FEA_MAC_TBL
 */
#define  VTSS_F_DEVCPU_GCB_FEATURES_FEA_STAT_FEA_MAC_TBL(x)  VTSS_ENCODE_BITFIELD(x,5,2)
#define  VTSS_M_DEVCPU_GCB_FEATURES_FEA_STAT_FEA_MAC_TBL     VTSS_ENCODE_BITMASK(5,2)
#define  VTSS_X_DEVCPU_GCB_FEATURES_FEA_STAT_FEA_MAC_TBL(x)  VTSS_EXTRACT_BITFIELD(x,5,2)

/** 
 * \brief
 * Encoded ChipID.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_FEATURES_FEA_STAT . FEA_CHIP_ID
 */
#define  VTSS_F_DEVCPU_GCB_FEATURES_FEA_STAT_FEA_CHIP_ID(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_DEVCPU_GCB_FEATURES_FEA_STAT_FEA_CHIP_ID     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_DEVCPU_GCB_FEATURES_FEA_STAT_FEA_CHIP_ID(x)  VTSS_EXTRACT_BITFIELD(x,0,5)

/**
 * Register Group: \a DEVCPU_GCB:RAM_STAT
 *
 * QS RAM status
 */


/** 
 * \details
 * Register: \a DEVCPU_GCB:RAM_STAT:RAM_INTEGRITY_ERR_STICKY
 */
#define VTSS_DEVCPU_GCB_RAM_STAT_RAM_INTEGRITY_ERR_STICKY  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x3f)

/** 
 * \brief
 * Integrity error for QS_XTR RAM
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DEVCPU_GCB_RAM_STAT_RAM_INTEGRITY_ERR_STICKY . QS_XTR_RAM_INTGR_ERR_STICKY
 */
#define  VTSS_F_DEVCPU_GCB_RAM_STAT_RAM_INTEGRITY_ERR_STICKY_QS_XTR_RAM_INTGR_ERR_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a DEVCPU_GCB:TEMP_SENSOR
 *
 * Temperature sensor control
 */


/** 
 * \brief Control the state of the Temperature Sensor
 *
 * \details
 * Register: \a DEVCPU_GCB:TEMP_SENSOR:TEMP_SENSOR_CTRL
 */
#define VTSS_DEVCPU_GCB_TEMP_SENSOR_TEMP_SENSOR_CTRL  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x40)

/** 
 * \brief
 * Register controls the power down state of the JR temperature sensor
 *
 * \details 
 * '0': Temperature sensor active
 * '1': Temperature sensor in power down
 *
 * Field: ::VTSS_DEVCPU_GCB_TEMP_SENSOR_TEMP_SENSOR_CTRL . TEMP_SENSOR_POWER_DOWN
 */
#define  VTSS_F_DEVCPU_GCB_TEMP_SENSOR_TEMP_SENSOR_CTRL_TEMP_SENSOR_POWER_DOWN  VTSS_BIT(1)

/** 
 * \brief
 * Register control reset state of the Temperature Sensor
 *
 * \details 
 * '0' : Temperature Sensor is in reset state
 * '1': Temperature Sensor is NOT in reset state.
 *
 * Field: ::VTSS_DEVCPU_GCB_TEMP_SENSOR_TEMP_SENSOR_CTRL . TEMP_SENSOR_RESET_N
 */
#define  VTSS_F_DEVCPU_GCB_TEMP_SENSOR_TEMP_SENSOR_CTRL_TEMP_SENSOR_RESET_N  VTSS_BIT(0)


/** 
 * \brief Read Temperature Data from Temperature Sensor
 *
 * \details
 * Register: \a DEVCPU_GCB:TEMP_SENSOR:TEMP_SENSOR_DATA
 */
#define VTSS_DEVCPU_GCB_TEMP_SENSOR_TEMP_SENSOR_DATA  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0x41)

/** 
 * \brief
 * Indicates if valid data has been read from the Temperature sensor.
 * 
 * From temperature sensor is enabled (TEMP_SENSOR_POWER_DOWN = '0' and
 * TEMP_SENSOR_RESET_N	= '1' and TEMP_SENSOR_RUN = '1')  it will take some
 * time for the first valid temperature reading to be performed. During
 * this period TEMP_SENSOR_DATA_VALID will be deasserted.
 * 
 * TEMP_SENSOR_DATA_VALID is asserted as soon as first valid data
 * (TEMP_SENSOR_DATA) has been read from temperature sensor.
 * 
 * Once valid data has been read, data will be updated (and remain valid)
 * until the temperature sensor is either in reset state
 * (TEMP_SENSOR_RESET_N = '0') or in power down state
 * (TEMP_SENSOR_POWER_DOWN = '1') or run is disabled (TEMP_SENSOR_RUN =
 * '0').
 *
 * \details 
 * '0': TEMP_SENSOR_DATA does NOT contain valid temperature data.
 * '1': TEMP_SENSOR_DATA DOES contain valid temperature data.
 *
 * Field: ::VTSS_DEVCPU_GCB_TEMP_SENSOR_TEMP_SENSOR_DATA . TEMP_SENSOR_DATA_VALID
 */
#define  VTSS_F_DEVCPU_GCB_TEMP_SENSOR_TEMP_SENSOR_DATA_TEMP_SENSOR_DATA_VALID  VTSS_BIT(8)

/** 
 * \brief
 * Temperature data readout.
 * 
 * Note temperature is qualified by TEMP_SENSOR_DATA_VALID = '1'
 *
 * \details 
 * TEMP = 135.3 - (0.71*TEMP_SENSOR_DATA)
 *
 * Field: ::VTSS_DEVCPU_GCB_TEMP_SENSOR_TEMP_SENSOR_DATA . TEMP_SENSOR_DATA
 */
#define  VTSS_F_DEVCPU_GCB_TEMP_SENSOR_TEMP_SENSOR_DATA_TEMP_SENSOR_DATA(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_DEVCPU_GCB_TEMP_SENSOR_TEMP_SENSOR_DATA_TEMP_SENSOR_DATA     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_DEVCPU_GCB_TEMP_SENSOR_TEMP_SENSOR_DATA_TEMP_SENSOR_DATA(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a DEVCPU_GCB:SIO_CTRL
 *
 * Serial IO control configuration
 */


/** 
 * \brief Input data registers
 *
 * \details
 * Register: \a DEVCPU_GCB:SIO_CTRL:SIO_INPUT_DATA
 *
 * @param gi Replicator: x_SIO_CTRL_CNT (??), 0-1
 * @param ri Replicator: x_SIO_IOS_PR_PORT (??), 0-3
 */
#define VTSS_DEVCPU_GCB_SIO_CTRL_SIO_INPUT_DATA(gi,ri)  VTSS_IOREG_IX(VTSS_TO_DEVCPU_GCB,0x42,gi,48,ri,0)


/** 
 * \brief Interrupt polarity for each GPIO
 *
 * \details
 * Register: \a DEVCPU_GCB:SIO_CTRL:SIO_INT_POL
 *
 * @param gi Replicator: x_SIO_CTRL_CNT (??), 0-1
 * @param ri Replicator: x_SIO_IOS_PR_PORT (??), 0-3
 */
#define VTSS_DEVCPU_GCB_SIO_CTRL_SIO_INT_POL(gi,ri)  VTSS_IOREG_IX(VTSS_TO_DEVCPU_GCB,0x42,gi,48,ri,4)


/** 
 * \brief Interrupt enable register for each port.
 *
 * \details
 * Register: \a DEVCPU_GCB:SIO_CTRL:SIO_PORT_INT_ENA
 *
 * @param gi Replicator: x_SIO_CTRL_CNT (??), 0-1
 */
#define VTSS_DEVCPU_GCB_SIO_CTRL_SIO_PORT_INT_ENA(gi)  VTSS_IOREG_IX(VTSS_TO_DEVCPU_GCB,0x42,gi,48,0,8)


/** 
 * \brief Configuration of output data values
 *
 * \details
 * Register: \a DEVCPU_GCB:SIO_CTRL:SIO_PORT_CONFIG
 *
 * @param gi Replicator: x_SIO_CTRL_CNT (??), 0-1
 * @param ri Replicator: x_SIO_PORT_CNT (??), 0-31
 */
#define VTSS_DEVCPU_GCB_SIO_CTRL_SIO_PORT_CONFIG(gi,ri)  VTSS_IOREG_IX(VTSS_TO_DEVCPU_GCB,0x42,gi,48,ri,9)

/** 
 * \brief
 * Output source select for the four outputs from each port.
 * The source select is encoded using three bits for each output bit.
 *
 * \details 
 * The placement of the source select bits for each output bit in the
 * register:
 * Output bit 0: (2 down to 0)
 * Output bit 1: (5 down to 3)
 * Output bit 2: (8 down to 6)
 * Output bit 3: (11 down to 9)
 * Source select encoding for each output bit:
 * 0 : Forced '0'
 * 1 : Forced '1'
 * 2 : Blink mode 0
 * 3 : Blink mode 1
 * 4 : Link activity blink mode 0
 * 5 : Link activity blink mode 1
 * 6 : Link activity blink mode 0 inversed polarity
 * 7 : Link activity blink mode 1 inversed polarity
 *
 * Field: ::VTSS_DEVCPU_GCB_SIO_CTRL_SIO_PORT_CONFIG . BIT_SOURCE
 */
#define  VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_PORT_CONFIG_BIT_SOURCE(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_DEVCPU_GCB_SIO_CTRL_SIO_PORT_CONFIG_BIT_SOURCE     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_DEVCPU_GCB_SIO_CTRL_SIO_PORT_CONFIG_BIT_SOURCE(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \brief Port enable register
 *
 * \details
 * Register: \a DEVCPU_GCB:SIO_CTRL:SIO_PORT_ENABLE
 *
 * @param gi Replicator: x_SIO_CTRL_CNT (??), 0-1
 */
#define VTSS_DEVCPU_GCB_SIO_CTRL_SIO_PORT_ENABLE(gi)  VTSS_IOREG_IX(VTSS_TO_DEVCPU_GCB,0x42,gi,48,0,41)


/** 
 * \brief General configuration register
 *
 * \details
 * Register: \a DEVCPU_GCB:SIO_CTRL:SIO_CONFIG
 *
 * @param gi Replicator: x_SIO_CTRL_CNT (??), 0-1
 */
#define VTSS_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG(gi)  VTSS_IOREG_IX(VTSS_TO_DEVCPU_GCB,0x42,gi,48,0,42)

/** 
 * \brief
 * Configuration for blink mode 1.
 * Supports three different blink modes and a "burst toggle" mode in which
 * blink mode 1 will alternate for each burst.
 *
 * \details 
 * 0 : Blink freq appr. 20Hz.
 * 1 : Blink freq appr. 10Hz.
 * 2 : Blink freq  appr. 5Hz.
 * 3 : Burst toggle.
 *
 * Field: ::VTSS_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG . SIO_BMODE_1
 */
#define  VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_BMODE_1(x)  VTSS_ENCODE_BITFIELD(x,19,2)
#define  VTSS_M_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_BMODE_1     VTSS_ENCODE_BITMASK(19,2)
#define  VTSS_X_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_BMODE_1(x)  VTSS_EXTRACT_BITFIELD(x,19,2)

/** 
 * \brief
 * Configuration of blink mode 0. Supports four different blink modes.
 *
 * \details 
 * 0 : Blink freq appr. 20Hz.
 * 1 : Blink freq appr. 10Hz.
 * 2 : Blink freq  appr. 5Hz.
 * 3 : Blink freq  appr. 2.5Hz.
 *
 * Field: ::VTSS_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG . SIO_BMODE_0
 */
#define  VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_BMODE_0(x)  VTSS_ENCODE_BITFIELD(x,17,2)
#define  VTSS_M_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_BMODE_0     VTSS_ENCODE_BITMASK(17,2)
#define  VTSS_X_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_BMODE_0(x)  VTSS_EXTRACT_BITFIELD(x,17,2)

/** 
 * \brief
 * Reset the blink counters. Used to synchronize the blink modes between
 * different chips.
 *
 * \details 
 * 0 : Blink counter is running.
 * 1 : Blink counter is reset until sio_blink_reset is unset again.
 *
 * Field: ::VTSS_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG . SIO_BLINK_RESET
 */
#define  VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_BLINK_RESET  VTSS_BIT(16)

/** 
 * \brief
 * Bit interrupt enable. Enables interrupts for the four gpios in a port.
 * Is applied to all ports.
 *
 * \details 
 * 0: Interrupt is disabled for bit n for all ports.
 * 1: Interrupt is enabled for bit n for all ports.
 *
 * Field: ::VTSS_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG . SIO_INT_ENA
 */
#define  VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_INT_ENA(x)  VTSS_ENCODE_BITFIELD(x,12,4)
#define  VTSS_M_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_INT_ENA     VTSS_ENCODE_BITMASK(12,4)
#define  VTSS_X_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_INT_ENA(x)  VTSS_EXTRACT_BITFIELD(x,12,4)

/** 
 * \brief
 * Configures the length of burst gap in steps of approx. 1 ms. Burst gap
 * can be disabled by setting SIO_CONFIG.SIO_BURST_GAP_DIS.
 *
 * \details 
 * 0: 1.05 ms burst gap.
 * 1: 2.10 ms burst gap.
 * 31: 33.55 ms burst gap.
 *
 * Field: ::VTSS_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG . SIO_BURST_GAP
 */
#define  VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_BURST_GAP(x)  VTSS_ENCODE_BITFIELD(x,7,5)
#define  VTSS_M_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_BURST_GAP     VTSS_ENCODE_BITMASK(7,5)
#define  VTSS_X_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_BURST_GAP(x)  VTSS_EXTRACT_BITFIELD(x,7,5)

/** 
 * \brief
 * Use this to output a single burst. Will be cleared by hardware when the
 * burst has finished.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG . SIO_SINGLE_SHOT
 */
#define  VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_SINGLE_SHOT  VTSS_BIT(6)

/** 
 * \brief
 * Use this to output repeated bursts interleaved with burst gaps. Must be
 * manually reset again to stop output of bursts.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG . SIO_AUTO_REPEAT
 */
#define  VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_AUTO_REPEAT  VTSS_BIT(5)

/** 
 * \brief
 * Polarity of the "Ld" signal
 *
 * \details 
 * 0: load signal is active low
 * 1: load signal is active high
 *
 * Field: ::VTSS_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG . SIO_LD_POLARITY
 */
#define  VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_LD_POLARITY  VTSS_BIT(4)

/** 
 * \brief
 * Number of gpios pr. port.
 *
 * \details 
 * 0: 1 gpio pr. port.
 * 1: 2 gpios pr. port.
 * 2: 3 gpios pr. port.
 * 3: 4 gpios pr. port.  
 *
 * Field: ::VTSS_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG . SIO_PORT_WIDTH
 */
#define  VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_PORT_WIDTH(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_PORT_WIDTH     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_PORT_WIDTH(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Reverse the output bitstream.
 * 
 * The default order of the output bit stream is (displayed in transmitted
 * order):
 * (portN bit3, portN bit2, ...., port0 bit1, port0 bit0)
 * 
 * The reverse order of the output bit stream is (displayed in transmitted
 * order):
 * (port0 bit0, port0 bit1, ...., portN bit2, portN bit3)
 *
 * \details 
 * 0 : Do not reverse.
 * 1 : Reverse.
 *
 * Field: ::VTSS_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG . SIO_REVERSE_OUTPUT
 */
#define  VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_REVERSE_OUTPUT  VTSS_BIT(1)

/** 
 * \brief
 * Reverse the input bitstream.
 * The default order of the input bit stream is (displayed in received
 * order):
 * (port0 bit0, port0 bit1, ...., portN bit2, portN bit3)
 * The reverse order of the input bit stream is (displayed in received
 * order):
 * (portN bit3, portN bit2, ...., port0 bit1, port0 bit0)
 *
 * \details 
 * 0: Do not reverse.
 * 1: Reverse.
 *
 * Field: ::VTSS_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG . SIO_REVERSE_INPUT
 */
#define  VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CONFIG_SIO_REVERSE_INPUT  VTSS_BIT(0)


/** 
 * \brief Configuration of the serial IO clock frequency
 *
 * \details
 * Register: \a DEVCPU_GCB:SIO_CTRL:SIO_CLOCK
 *
 * @param gi Replicator: x_SIO_CTRL_CNT (??), 0-1
 */
#define VTSS_DEVCPU_GCB_SIO_CTRL_SIO_CLOCK(gi)  VTSS_IOREG_IX(VTSS_TO_DEVCPU_GCB,0x42,gi,48,0,43)

/** 
 * \brief
 * SIO controller clock frequency. Divides the 250MHz system clk with value
 * of this field. E.g. the system clk is 250 MHz and this field is set to
 * 10, the output frequency will be 25 MHz.
 *
 * \details 
 * 0 : Disable clock.
 * 1 : Reserved, do not use.
 * Others : Clock divider value.
 *
 * Field: ::VTSS_DEVCPU_GCB_SIO_CTRL_SIO_CLOCK . SIO_CLK_FREQ
 */
#define  VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CLOCK_SIO_CLK_FREQ(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_DEVCPU_GCB_SIO_CTRL_SIO_CLOCK_SIO_CLK_FREQ     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_DEVCPU_GCB_SIO_CTRL_SIO_CLOCK_SIO_CLK_FREQ(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \brief Interrupt register
 *
 * \details
 * Register: \a DEVCPU_GCB:SIO_CTRL:SIO_INT_REG
 *
 * @param gi Replicator: x_SIO_CTRL_CNT (??), 0-1
 * @param ri Replicator: x_SIO_IOS_PR_PORT (??), 0-3
 */
#define VTSS_DEVCPU_GCB_SIO_CTRL_SIO_INT_REG(gi,ri)  VTSS_IOREG_IX(VTSS_TO_DEVCPU_GCB,0x42,gi,48,ri,44)

/**
 * Register Group: \a DEVCPU_GCB:PTP_CFG
 *
 * Configuration registers for PTP
 */


/** 
 * \brief Misc Configuration Register for PTP
 *
 * \details
 * Misc Configuration Register for PTP
 *
 * Register: \a DEVCPU_GCB:PTP_CFG:PTP_MISC_CFG
 */
#define VTSS_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0xa2)

/** 
 * \brief
 * GPIO selection of external sync output.
 *
 * \details 
 * '0': External sync output is mapped to GPIO_TIME_TICK_0
 * '1': External sync output is mapped to GPIO_TIME_TICK_1 - (same as
 * generated clk GPIO)
 *
 * Field: ::VTSS_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG . EXT_SYNC_OUTP_SEL
 */
#define  VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_OUTP_SEL  VTSS_BIT(7)

/** 
 * \brief
 * Inversion of external sync output.
 *
 * \details 
 * '0': External sync output is not inverted
 * '1': External sync output is inverted
 *
 * Field: ::VTSS_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG . EXT_SYNC_OUTP_INV
 */
#define  VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_OUTP_INV  VTSS_BIT(6)

/** 
 * \brief
 * External sync output enable.
 *
 * \details 
 * 0': External sync output is disabled
 * '1': External sync output is enabled
 *
 * Field: ::VTSS_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG . EXT_SYNC_OUTP_ENA
 */
#define  VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_OUTP_ENA  VTSS_BIT(5)

/** 
 * \brief
 * Select of external sync input GPIO.
 *
 * \details 
 * '0': External sync input is GPIO_TIME_TICK_0
 * '1': External sync input is GPIO_TIME_TICK_1
 *
 * Field: ::VTSS_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG . EXT_SYNC_INP_SEL
 */
#define  VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_INP_SEL  VTSS_BIT(4)

/** 
 * \brief
 * Inversion of external sync input.
 *
 * \details 
 * '0': External sync input is not inverted
 * '1': External sync input is inverted
 *
 * Field: ::VTSS_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG . EXT_SYNC_INP_INV
 */
#define  VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_INP_INV  VTSS_BIT(3)

/** 
 * \brief
 * External sync input enable.
 *
 * \details 
 * '0': External sync input is disabled
 * '1': External sync input is enabled
 *
 * Field: ::VTSS_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG . EXT_SYNC_INP_ENA
 */
#define  VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_INP_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Enable synchronization to external sync.
 *
 * \details 
 * '0': Sync on external signal is disabled
 * '1': Sync on external signal is enabled
 *
 * Field: ::VTSS_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG . EXT_SYNC_ENA
 */
#define  VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_EXT_SYNC_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable master counter.
 *
 * \details 
 * 0: Master counter disabled.
 * 1: Master counter enabled.
 *
 * Field: ::VTSS_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG . PTP_ENA
 */
#define  VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_MISC_CFG_PTP_ENA  VTSS_BIT(0)


/** 
 * \brief Configuration register for master counter upper limit
 *
 * \details
 * Configuration register for master counter upper limit
 *
 * Register: \a DEVCPU_GCB:PTP_CFG:PTP_UPPER_LIMIT_CFG
 */
#define VTSS_DEVCPU_GCB_PTP_CFG_PTP_UPPER_LIMIT_CFG  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0xa3)

/** 
 * \brief
 * Counter value where the Master counter should be reset
 *
 * \details 
 * Units is time in clock_ticks.
 * 1 clock tick is 4 ns, if system_clk is set to 250MHz.
 *
 * Field: ::VTSS_DEVCPU_GCB_PTP_CFG_PTP_UPPER_LIMIT_CFG . PTP_UPPER_LIMIT
 */
#define  VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_UPPER_LIMIT_CFG_PTP_UPPER_LIMIT(x)  VTSS_ENCODE_BITFIELD(x,0,28)
#define  VTSS_M_DEVCPU_GCB_PTP_CFG_PTP_UPPER_LIMIT_CFG_PTP_UPPER_LIMIT     VTSS_ENCODE_BITMASK(0,28)
#define  VTSS_X_DEVCPU_GCB_PTP_CFG_PTP_UPPER_LIMIT_CFG_PTP_UPPER_LIMIT(x)  VTSS_EXTRACT_BITFIELD(x,0,28)


/** 
 * \brief Configuration register for master counter upper limit one time adjustment
 *
 * \details
 * Configuration register for master counter upper limit one time
 * adjustment
 *
 * Register: \a DEVCPU_GCB:PTP_CFG:PTP_UPPER_LIMIT_1_TIME_ADJ_CFG
 */
#define VTSS_DEVCPU_GCB_PTP_CFG_PTP_UPPER_LIMIT_1_TIME_ADJ_CFG  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0xa4)

/** 
 * \brief
 * One time enable for PTP_UPPER_LIMIT_1_TIME_ADJ
 *
 * \details 
 * 0: Normal operation
 * 1: Timer is adjusted by usage of PTP_UPPER_LIMIT_1_TIME_ADJ
 * Bit is cleared by HW
 *
 * Field: ::VTSS_DEVCPU_GCB_PTP_CFG_PTP_UPPER_LIMIT_1_TIME_ADJ_CFG . PTP_UPPER_LIMIT_1_TIME_ADJ_SHOT
 */
#define  VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_UPPER_LIMIT_1_TIME_ADJ_CFG_PTP_UPPER_LIMIT_1_TIME_ADJ_SHOT  VTSS_BIT(31)

/** 
 * \brief
 * Counter value where the Master counter should be reset
 *
 * \details 
 * Units is time in clock_ticks.
 * 1 clock tick is 4 ns
 *
 * Field: ::VTSS_DEVCPU_GCB_PTP_CFG_PTP_UPPER_LIMIT_1_TIME_ADJ_CFG . PTP_UPPER_LIMIT_1_TIME_ADJ
 */
#define  VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_UPPER_LIMIT_1_TIME_ADJ_CFG_PTP_UPPER_LIMIT_1_TIME_ADJ(x)  VTSS_ENCODE_BITFIELD(x,0,28)
#define  VTSS_M_DEVCPU_GCB_PTP_CFG_PTP_UPPER_LIMIT_1_TIME_ADJ_CFG_PTP_UPPER_LIMIT_1_TIME_ADJ     VTSS_ENCODE_BITMASK(0,28)
#define  VTSS_X_DEVCPU_GCB_PTP_CFG_PTP_UPPER_LIMIT_1_TIME_ADJ_CFG_PTP_UPPER_LIMIT_1_TIME_ADJ(x)  VTSS_EXTRACT_BITFIELD(x,0,28)


/** 
 * \brief Sync Interrupt enable register
 *
 * \details
 * Sync Interrupt enable register
 *
 * Register: \a DEVCPU_GCB:PTP_CFG:PTP_SYNC_INTR_ENA_CFG
 */
#define VTSS_DEVCPU_GCB_PTP_CFG_PTP_SYNC_INTR_ENA_CFG  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0xa5)

/** 
 * \brief
 * Interrupt mask. Masks interrupt generation when a synchronization pulse
 * is received on external sync input pin.
 *
 * \details 
 * '0': Interrupt is not generated
 * '1': Interrupt is generated
 *
 * Field: ::VTSS_DEVCPU_GCB_PTP_CFG_PTP_SYNC_INTR_ENA_CFG . EXT_SYNC_CURRENT_TIME_ENA
 */
#define  VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_SYNC_INTR_ENA_CFG_EXT_SYNC_CURRENT_TIME_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Interrupt mask. Masks interrupt generation when Master Timer generates a
 * synchronization pulse.
 *
 * \details 
 * '0': Interrupt is not generated
 * '1': Interrupt is generated
 *
 * Field: ::VTSS_DEVCPU_GCB_PTP_CFG_PTP_SYNC_INTR_ENA_CFG . SYNC_STAT_ENA
 */
#define  VTSS_F_DEVCPU_GCB_PTP_CFG_PTP_SYNC_INTR_ENA_CFG_SYNC_STAT_ENA  VTSS_BIT(0)


/** 
 * \brief Generated external clock high period configuration register
 *
 * \details
 * Generated external clock high period configuration register
 *
 * Register: \a DEVCPU_GCB:PTP_CFG:GEN_EXT_CLK_HIGH_PERIOD_CFG
 */
#define VTSS_DEVCPU_GCB_PTP_CFG_GEN_EXT_CLK_HIGH_PERIOD_CFG  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0xa6)

/** 
 * \brief
 * High period for generated external clock in system clock cycles.
 *
 * \details 
 * N: External clock signal is high for (N+1)*system_clk cycles.
 * E.g. N=999, system clock = 250 MHz which means 4 ns clk period.
 * High Phase is 4 us.
 *
 * Field: ::VTSS_DEVCPU_GCB_PTP_CFG_GEN_EXT_CLK_HIGH_PERIOD_CFG . GEN_EXT_CLK_HIGH_PERIOD
 */
#define  VTSS_F_DEVCPU_GCB_PTP_CFG_GEN_EXT_CLK_HIGH_PERIOD_CFG_GEN_EXT_CLK_HIGH_PERIOD(x)  VTSS_ENCODE_BITFIELD(x,0,28)
#define  VTSS_M_DEVCPU_GCB_PTP_CFG_GEN_EXT_CLK_HIGH_PERIOD_CFG_GEN_EXT_CLK_HIGH_PERIOD     VTSS_ENCODE_BITMASK(0,28)
#define  VTSS_X_DEVCPU_GCB_PTP_CFG_GEN_EXT_CLK_HIGH_PERIOD_CFG_GEN_EXT_CLK_HIGH_PERIOD(x)  VTSS_EXTRACT_BITFIELD(x,0,28)


/** 
 * \brief Generated external clock low period configuration register
 *
 * \details
 * Generated external clock low period configuration register
 *
 * Register: \a DEVCPU_GCB:PTP_CFG:GEN_EXT_CLK_LOW_PERIOD_CFG
 */
#define VTSS_DEVCPU_GCB_PTP_CFG_GEN_EXT_CLK_LOW_PERIOD_CFG  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0xa7)

/** 
 * \brief
 * Low period for generated external clock in system clock cycles.
 *
 * \details 
 * N: External clock signal is low for (N+1)*system_clk cycles.
 * E.g. N=999, system clock = 250 MHz which means 4 ns clk period.
 * Low Phase is 4 us.
 *
 * Field: ::VTSS_DEVCPU_GCB_PTP_CFG_GEN_EXT_CLK_LOW_PERIOD_CFG . GEN_EXT_CLK_LOW_PERIOD
 */
#define  VTSS_F_DEVCPU_GCB_PTP_CFG_GEN_EXT_CLK_LOW_PERIOD_CFG_GEN_EXT_CLK_LOW_PERIOD(x)  VTSS_ENCODE_BITFIELD(x,0,28)
#define  VTSS_M_DEVCPU_GCB_PTP_CFG_GEN_EXT_CLK_LOW_PERIOD_CFG_GEN_EXT_CLK_LOW_PERIOD     VTSS_ENCODE_BITMASK(0,28)
#define  VTSS_X_DEVCPU_GCB_PTP_CFG_GEN_EXT_CLK_LOW_PERIOD_CFG_GEN_EXT_CLK_LOW_PERIOD(x)  VTSS_EXTRACT_BITFIELD(x,0,28)


/** 
 * \brief Configuration register for sync. of external clock to internal master sync.
 *
 * \details
 * Configuration register for synchronization of external clock to internal
 * master sync.
 *
 * Register: \a DEVCPU_GCB:PTP_CFG:GEN_EXT_CLK_CFG
 */
#define VTSS_DEVCPU_GCB_PTP_CFG_GEN_EXT_CLK_CFG  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0xa8)

/** 
 * \brief
 * Enable sync of generated external clock to PTP sync master.
 *
 * \details 
 * 0: Synchronization is disabled
 * 1: Synchronization is enabled
 *
 * Field: ::VTSS_DEVCPU_GCB_PTP_CFG_GEN_EXT_CLK_CFG . GEN_EXT_CLK_SYNC_ENA
 */
#define  VTSS_F_DEVCPU_GCB_PTP_CFG_GEN_EXT_CLK_CFG_GEN_EXT_CLK_SYNC_ENA  VTSS_BIT(2)

/** 
 * \brief
 * External clock frequency adjustment enable.
 *
 * \details 
 * 0: Adjustment Disabled
 * 1: Adjustment Enabled
 *
 * Field: ::VTSS_DEVCPU_GCB_PTP_CFG_GEN_EXT_CLK_CFG . GEN_EXT_CLK_ADJ_ENA
 */
#define  VTSS_F_DEVCPU_GCB_PTP_CFG_GEN_EXT_CLK_CFG_GEN_EXT_CLK_ADJ_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable generated external clock.
 *
 * \details 
 * 0: Generated external clock disabled.
 * 1: Generated external clock enabled
 *
 * Field: ::VTSS_DEVCPU_GCB_PTP_CFG_GEN_EXT_CLK_CFG . GEN_EXT_CLK_ENA
 */
#define  VTSS_F_DEVCPU_GCB_PTP_CFG_GEN_EXT_CLK_CFG_GEN_EXT_CLK_ENA  VTSS_BIT(0)


/** 
 * \brief Configuration register for generated clock frequency adjustment
 *
 * \details
 * Configuration register for generated clock frequency adjustment
 *
 * Register: \a DEVCPU_GCB:PTP_CFG:CLK_ADJ_CFG
 */
#define VTSS_DEVCPU_GCB_PTP_CFG_CLK_ADJ_CFG  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0xa9)

/** 
 * \brief
 * Clock frequency adjustment direction.
 *
 * \details 
 * 0: Positive adjustment. Every N cycles a 1 is added to the counter. =>
 * clock period is decrease, clock frequency is increased.
 * 1: Negative adjustment. Every N cycles a 1 is subtracted from the
 * counter. => clock period is increase, clock frequency is decreased.
 *
 * Field: ::VTSS_DEVCPU_GCB_PTP_CFG_CLK_ADJ_CFG . CLK_ADJ_DIR
 */
#define  VTSS_F_DEVCPU_GCB_PTP_CFG_CLK_ADJ_CFG_CLK_ADJ_DIR  VTSS_BIT(31)

/** 
 * \brief
 * Clock frequency adjust enable.
 *
 * \details 
 * 0: Adjustment Disabled
 * 1: Adjustment Enabled
 *
 * Field: ::VTSS_DEVCPU_GCB_PTP_CFG_CLK_ADJ_CFG . CLK_ADJ_ENA
 */
#define  VTSS_F_DEVCPU_GCB_PTP_CFG_CLK_ADJ_CFG_CLK_ADJ_ENA  VTSS_BIT(30)

/** 
 * \brief
 * Defines when the updated adjustment value and direction takes effect.
 *
 * \details 
 * 0: updated values take immediate effect.
 * 1: updated values take effect after the next sync pulse.
 *
 * Field: ::VTSS_DEVCPU_GCB_PTP_CFG_CLK_ADJ_CFG . CLK_ADJ_UPD
 */
#define  VTSS_F_DEVCPU_GCB_PTP_CFG_CLK_ADJ_CFG_CLK_ADJ_UPD  VTSS_BIT(29)

/** 
 * \brief
 * Clock frequency adjust.
 *
 * \details 
 * N: Number of clock cycles minus 1 after which the counter for the clock
 * must be adjusted.
 * E.g. setting CLK_ADJ to 1 would adjust the clock counter every 2nd clock
 * cycle.
 *
 * Field: ::VTSS_DEVCPU_GCB_PTP_CFG_CLK_ADJ_CFG . CLK_ADJ
 */
#define  VTSS_F_DEVCPU_GCB_PTP_CFG_CLK_ADJ_CFG_CLK_ADJ(x)  VTSS_ENCODE_BITFIELD(x,0,28)
#define  VTSS_M_DEVCPU_GCB_PTP_CFG_CLK_ADJ_CFG_CLK_ADJ     VTSS_ENCODE_BITMASK(0,28)
#define  VTSS_X_DEVCPU_GCB_PTP_CFG_CLK_ADJ_CFG_CLK_ADJ(x)  VTSS_EXTRACT_BITFIELD(x,0,28)

/**
 * Register Group: \a DEVCPU_GCB:PTP_STAT
 *
 * Status registers for PTP
 */


/** 
 * \brief Current PTP master timer value
 *
 * \details
 * Current PTP master timer value
 *
 * Register: \a DEVCPU_GCB:PTP_STAT:PTP_CURRENT_TIME_STAT
 */
#define VTSS_DEVCPU_GCB_PTP_STAT_PTP_CURRENT_TIME_STAT  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0xaa)

/** 
 * \brief
 * Current master counter value. Unit is 4 ns.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_PTP_STAT_PTP_CURRENT_TIME_STAT . PTP_CURRENT_TIME
 */
#define  VTSS_F_DEVCPU_GCB_PTP_STAT_PTP_CURRENT_TIME_STAT_PTP_CURRENT_TIME(x)  VTSS_ENCODE_BITFIELD(x,0,28)
#define  VTSS_M_DEVCPU_GCB_PTP_STAT_PTP_CURRENT_TIME_STAT_PTP_CURRENT_TIME     VTSS_ENCODE_BITMASK(0,28)
#define  VTSS_X_DEVCPU_GCB_PTP_STAT_PTP_CURRENT_TIME_STAT_PTP_CURRENT_TIME(x)  VTSS_EXTRACT_BITFIELD(x,0,28)


/** 
 * \brief External sync current time status register
 *
 * \details
 * External sync current time status register
 *
 * Register: \a DEVCPU_GCB:PTP_STAT:EXT_SYNC_CURRENT_TIME_STAT
 */
#define VTSS_DEVCPU_GCB_PTP_STAT_EXT_SYNC_CURRENT_TIME_STAT  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0xab)

/** 
 * \brief
 * Snapshot of current time, when a rising edge was seen in on the external
 * sync input.
 * Note: A new value is only captured when the associated sticky bit is not
 * set.
 *
 * \details 
 * Current time in clock_ticks when the rising edge on the external sync
 * input was seen.
 * Note: This has to be adjusted by 3 clock ticks for synchronizing the
 * signal to core clock.
 *
 * Field: ::VTSS_DEVCPU_GCB_PTP_STAT_EXT_SYNC_CURRENT_TIME_STAT . EXT_SYNC_CURRENT_TIME
 */
#define  VTSS_F_DEVCPU_GCB_PTP_STAT_EXT_SYNC_CURRENT_TIME_STAT_EXT_SYNC_CURRENT_TIME(x)  VTSS_ENCODE_BITFIELD(x,0,28)
#define  VTSS_M_DEVCPU_GCB_PTP_STAT_EXT_SYNC_CURRENT_TIME_STAT_EXT_SYNC_CURRENT_TIME     VTSS_ENCODE_BITMASK(0,28)
#define  VTSS_X_DEVCPU_GCB_PTP_STAT_EXT_SYNC_CURRENT_TIME_STAT_EXT_SYNC_CURRENT_TIME(x)  VTSS_EXTRACT_BITFIELD(x,0,28)


/** 
 * \brief Stick register for external sync current time status
 *
 * \details
 * Stick register for external sync current time status
 *
 * Register: \a DEVCPU_GCB:PTP_STAT:PTP_EVT_STAT
 */
#define VTSS_DEVCPU_GCB_PTP_STAT_PTP_EVT_STAT  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0xac)

/** 
 * \brief
 * Identifies if the adjust value update has already happened in case the
 * adjustment is only allowed to take place at sync.
 * If update is allowed to take place immediately the sticky bit is unused.
 *
 * \details 
 * 0: updated has not yet happened
 * 1: updated has happened
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DEVCPU_GCB_PTP_STAT_PTP_EVT_STAT . CLK_ADJ_UPD_STICKY
 */
#define  VTSS_F_DEVCPU_GCB_PTP_STAT_PTP_EVT_STAT_CLK_ADJ_UPD_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Sticky bit that indicates a synchronization pulse has been captured on
 * external sync input pin.
 *
 * \details 
 * '0': No Timestamp has been captured
 * '1': New Timestamp has been captured
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DEVCPU_GCB_PTP_STAT_PTP_EVT_STAT . EXT_SYNC_CURRENT_TIME_STICKY
 */
#define  VTSS_F_DEVCPU_GCB_PTP_STAT_PTP_EVT_STAT_EXT_SYNC_CURRENT_TIME_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Master timer has generated a synchronization pulse to the Slave Timers.
 *
 * \details 
 * '0': No master timer wrap happened.
 * '1': Master timer wrap happened.
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DEVCPU_GCB_PTP_STAT_PTP_EVT_STAT . SYNC_STAT
 */
#define  VTSS_F_DEVCPU_GCB_PTP_STAT_PTP_EVT_STAT_SYNC_STAT  VTSS_BIT(0)

/**
 * Register Group: \a DEVCPU_GCB:MEMITGR
 *
 * Memory integrity monitor
 */


/** 
 * \brief Monitor control
 *
 * \details
 * Register: \a DEVCPU_GCB:MEMITGR:MEMITGR_CTRL
 */
#define VTSS_DEVCPU_GCB_MEMITGR_MEMITGR_CTRL  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0xad)

/** 
 * \brief
 * Setting this field transitions the integrity monitor between operating
 * modes. Transitioning between modes takes time, this field remains set
 * until the new mode is reached. During this time the monitor also reports
 * busy (MEMITGR_MODE.MODE_BUSY is set).
 * From IDLE (MEMITGR_MODE.MODE_IDLE is set) the monitor can transition
 * into either DETECT or LISTEN mode, the DETECT mode is entered if a
 * memory reports an indication - the LISTEN mode is entered if no
 * indications are reported. The first time after reset the monitor will
 * not detect indications, that is; it will transition directly from IDLE
 * to LISTEN mode.
 * From DETECT (MEMITGR_MODE.MODE_DETECT is set) the monitor can transition
 * into either DETECT or LISTEN mode, the DETECT mode is entered if more
 * indications are reported - the LISTEN mode is entered if no more
 * indications are reported.
 * From LISTEN (MEMITGR_MODE.MODE_LISTEN is set) the monitor can transition
 * into IDLE mode.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_MEMITGR_MEMITGR_CTRL . ACTIVATE
 */
#define  VTSS_F_DEVCPU_GCB_MEMITGR_MEMITGR_CTRL_ACTIVATE  VTSS_BIT(0)


/** 
 * \brief Monitor status
 *
 * \details
 * Register: \a DEVCPU_GCB:MEMITGR:MEMITGR_STAT
 */
#define VTSS_DEVCPU_GCB_MEMITGR_MEMITGR_STAT  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0xae)

/** 
 * \brief
 * If this field is set then there is an indication from one of the
 * memories that needs to be analyzed. An indication is either a parity
 * detection or an error correction.
 * This field is only set when the monitor is in LISTEN mode
 * (MEMITGR_MODE.MODE_LISTEN is set), in all other states (including BUSY)
 * this field returns 0.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_MEMITGR_MEMITGR_STAT . INDICATION
 */
#define  VTSS_F_DEVCPU_GCB_MEMITGR_MEMITGR_STAT_INDICATION  VTSS_BIT(4)

/** 
 * \brief
 * This field is set when the monitor is in LISTEN mode, during listen mode
 * the monitor continually check for parity/correction indications from the
 * memories.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_MEMITGR_MEMITGR_STAT . MODE_LISTEN
 */
#define  VTSS_F_DEVCPU_GCB_MEMITGR_MEMITGR_STAT_MODE_LISTEN  VTSS_BIT(3)

/** 
 * \brief
 * This field is set when the monitor is in DETECT mode, during detect mode
 * the MEMITGR_INFO register contains valid information about one
 * indication.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_MEMITGR_MEMITGR_STAT . MODE_DETECT
 */
#define  VTSS_F_DEVCPU_GCB_MEMITGR_MEMITGR_STAT_MODE_DETECT  VTSS_BIT(2)

/** 
 * \brief
 * This field is set when the monitor is in IDLE mode.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_MEMITGR_MEMITGR_STAT . MODE_IDLE
 */
#define  VTSS_F_DEVCPU_GCB_MEMITGR_MEMITGR_STAT_MODE_IDLE  VTSS_BIT(1)

/** 
 * \brief
 * The busy signal is a copy of the MEMITGR_CTRL.ACTIVATE field, see
 * description of that field for more information about the different
 * states/modes of the monitor.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_MEMITGR_MEMITGR_STAT . MODE_BUSY
 */
#define  VTSS_F_DEVCPU_GCB_MEMITGR_MEMITGR_STAT_MODE_BUSY  VTSS_BIT(0)


/** 
 * \brief Memory indication
 *
 * \details
 * This field is only valid when the monitor is in the DETECT
 * (MEMITGR_MODE.MODE_DETECT is set) mode.
 *
 * Register: \a DEVCPU_GCB:MEMITGR:MEMITGR_INFO
 */
#define VTSS_DEVCPU_GCB_MEMITGR_MEMITGR_INFO  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0xaf)

/** 
 * \brief
 * This field is set if the monitor has detected a parity indication (or an
 * unrecoverable correction).
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_MEMITGR_MEMITGR_INFO . MEM_ERR
 */
#define  VTSS_F_DEVCPU_GCB_MEMITGR_MEMITGR_INFO_MEM_ERR  VTSS_BIT(31)

/** 
 * \brief
 * This field is set if the monitor has detected a correction.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_MEMITGR_MEMITGR_INFO . MEM_COR
 */
#define  VTSS_F_DEVCPU_GCB_MEMITGR_MEMITGR_INFO_MEM_COR  VTSS_BIT(30)

/** 
 * \brief
 * This field is set if the monitor has detected a parity indication (or an
 * unrecoverable correction) for which the address has not been recorded.
 * If MEMITGR_INFO.MEM_ERR is set then there has been more than one
 * indication, then only the address of the newest indication has been
 * kept. 
 * If MEMITGR_INFO.MEM_ERR is cleared then an indication has occurred for
 * which the address could not be stored, this is a very rare situation
 * that can only happen if an indication is detected just as the memory is
 * talking to the monitor.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_MEMITGR_MEMITGR_INFO . MEM_ERR_OVF
 */
#define  VTSS_F_DEVCPU_GCB_MEMITGR_MEMITGR_INFO_MEM_ERR_OVF  VTSS_BIT(29)

/** 
 * \brief
 * This field is set if the monitor has correction indication for which the
 * address has not been recorded.
 * If MEMITGR_INFO.MEM_ERR is set then there has also been a parity
 * indication (or an unrecoverable correction) which takes priority over
 * correction indications.
 * If MEMITGR_INFO.MEM_ERR is cleared and MEMITGR_INFO.MEM_COR is set then
 * there has been more than one correction indication, then only the
 * address of the newest correction indication has been kept.
 * If MEMITGR_INFO.MEM_ERR and MEMITGR_INFO.MEM_COR is both cleared then a
 * correction indication has occurred for which the address could not be
 * stored, this is a very rare situaltion that can only happen if an
 * indication is detected just as the memory is talking to the monitor.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_MEMITGR_MEMITGR_INFO . MEM_COR_OVF
 */
#define  VTSS_F_DEVCPU_GCB_MEMITGR_MEMITGR_INFO_MEM_COR_OVF  VTSS_BIT(28)

/** 
 * \brief
 * This field is valid only when MEMITGR.MEM_ERR or MEMITGR.MEM_COR is set.
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_MEMITGR_MEMITGR_INFO . MEM_ADDR
 */
#define  VTSS_F_DEVCPU_GCB_MEMITGR_MEMITGR_INFO_MEM_ADDR(x)  VTSS_ENCODE_BITFIELD(x,0,28)
#define  VTSS_M_DEVCPU_GCB_MEMITGR_MEMITGR_INFO_MEM_ADDR     VTSS_ENCODE_BITMASK(0,28)
#define  VTSS_X_DEVCPU_GCB_MEMITGR_MEMITGR_INFO_MEM_ADDR(x)  VTSS_EXTRACT_BITFIELD(x,0,28)


/** 
 * \brief Memory index
 *
 * \details
 * This field is only valid when the monitor is in the DETECT
 * (MEMITGR_MODE.MODE_DETECT is set) mode.
 *
 * Register: \a DEVCPU_GCB:MEMITGR:MEMITGR_IDX
 */
#define VTSS_DEVCPU_GCB_MEMITGR_MEMITGR_IDX  VTSS_IOREG(VTSS_TO_DEVCPU_GCB,0xb0)

/** 
 * \brief
 * This field contains a unique index for the memory for which info is
 * currently provided in MEMITGR_MEMINFO. Indexes are counted from 1 (not
 * 0).
 *
 * \details 
 * Field: ::VTSS_DEVCPU_GCB_MEMITGR_MEMITGR_IDX . MEM_IDX
 */
#define  VTSS_F_DEVCPU_GCB_MEMITGR_MEMITGR_IDX_MEM_IDX(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DEVCPU_GCB_MEMITGR_MEMITGR_IDX_MEM_IDX     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DEVCPU_GCB_MEMITGR_MEMITGR_IDX_MEM_IDX(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


#endif /* _VTSS_JAGUAR_REGS_DEVCPU_GCB_H_ */
