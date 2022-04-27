#ifndef _VTSS_JAGUAR_REGS_MACRO_CTRL_H_
#define _VTSS_JAGUAR_REGS_MACRO_CTRL_H_

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
 * Target: \a MACRO_CTRL
 *
 * Register Collection for Control of Macros (SERDES1G, SERDES6G, LCPLL)
 *
 ***********************************************************************/

/**
 * Register Group: \a MACRO_CTRL:PLL5G_CFG
 *
 * PLL5G Configuration Registers
 */


/** 
 * \brief PLL5G Configuration 0
 *
 * \details
 * Configuration register 0 for PLL5G
 *
 * Register: \a MACRO_CTRL:PLL5G_CFG:PLL5G_CFG0
 */
#define VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0  VTSS_IOREG(VTSS_TO_HSIO,0x0)

/** 
 * \brief
 * setting for core clock divider 0:625MHz, 1:312.5MHz, 2:500MHz,
 * 3:277.77MHz, 4:500MHz, 5:250MHz, 6:416.66MHz, 7:227.27MHz, 8:416.66MHz,
 * 9:208.33MHz, 10:357.14MHz, 11:192.3MHz, 12:357.14MHz, 13:178.57MHz,
 * 14:312.5MHz, 15:166.66,MHz
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0 . CORE_CLK_DIV
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_CORE_CLK_DIV(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_CORE_CLK_DIV     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_CORE_CLK_DIV(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

/** 
 * \brief
 * setting for CPU clock divider same as core clk divider
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0 . CPU_CLK_DIV
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_CPU_CLK_DIV(x)  VTSS_ENCODE_BITFIELD(x,6,6)
#define  VTSS_M_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_CPU_CLK_DIV     VTSS_ENCODE_BITMASK(6,6)
#define  VTSS_X_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_CPU_CLK_DIV(x)  VTSS_EXTRACT_BITFIELD(x,6,6)

/** 
 * \brief
 * enable BIAS circuitry incl. Bandgap, voltage regulators, etc.
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0 . ENA_BIAS
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_ENA_BIAS  VTSS_BIT(12)

/** 
 * \brief
 * enable BIAS for LCPLL VCO output buffer
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0 . ENA_VCO_BUF
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_ENA_VCO_BUF  VTSS_BIT(13)

/** 
 * \brief
 * enable current mode chargepump, normal mode
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0 . ENA_CP1
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_ENA_CP1  VTSS_BIT(14)

/** 
 * \brief
 * enable fine VCO operating point regulator
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0 . ENA_VCO_CONTRH
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_ENA_VCO_CONTRH  VTSS_BIT(15)

/** 
 * \brief
 * setting for chargepump current 0: lowest current, 3: highest current
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0 . SELCPI
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_SELCPI(x)  VTSS_ENCODE_BITFIELD(x,16,2)
#define  VTSS_M_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_SELCPI     VTSS_ENCODE_BITMASK(16,2)
#define  VTSS_X_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_SELCPI(x)  VTSS_EXTRACT_BITFIELD(x,16,2)

/** 
 * \brief
 * setting for filter resistor value 0: biggest resistance, 31: lowest
 * resistance
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0 . LOOP_BW_RES
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_LOOP_BW_RES(x)  VTSS_ENCODE_BITFIELD(x,18,5)
#define  VTSS_M_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_LOOP_BW_RES     VTSS_ENCODE_BITMASK(18,5)
#define  VTSS_X_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_LOOP_BW_RES(x)  VTSS_EXTRACT_BITFIELD(x,18,5)

/** 
 * \brief
 * fine tune of bandgap voltage distribution 0: lowest voltage, 15: highest
 * voltage
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0 . SELBGV820
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_SELBGV820(x)  VTSS_ENCODE_BITFIELD(x,23,4)
#define  VTSS_M_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_SELBGV820     VTSS_ENCODE_BITMASK(23,4)
#define  VTSS_X_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_SELBGV820(x)  VTSS_EXTRACT_BITFIELD(x,23,4)

/** 
 * \brief
 * enable fine locking, last stage in startup locking sequence
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0 . ENA_LOCK_FINE
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_ENA_LOCK_FINE  VTSS_BIT(27)

/** 
 * \brief
 * RCPLL feedback divider setting
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0 . DIV4
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_DIV4  VTSS_BIT(28)

/** 
 * \brief
 * RCPLL enable BIAS for clocktree buffer (active low) 0: enable BIAS, 1:
 * disable BIAS
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0 . ENA_CLKTREE
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_ENA_CLKTREE  VTSS_BIT(29)

/** 
 * \brief
 * RCPLL Global enable for serdes lane.
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0 . ENA_LANE
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_ENA_LANE  VTSS_BIT(30)

/** 
 * \brief
 * RCPLL feedback divider setting
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0 . ENA_ROT
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG0_ENA_ROT  VTSS_BIT(31)


/** 
 * \brief PLL5G Configuration 1
 *
 * \details
 * Configuration register 1 for PLL5G
 *
 * Register: \a MACRO_CTRL:PLL5G_CFG:PLL5G_CFG1
 */
#define VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1  VTSS_IOREG(VTSS_TO_HSIO,0x1)

/** 
 * \brief
 * RCPLL When set to 1 the value at sx_pll_fsm_ctrl_data_I is not taken as
 * reference value for the FSM, but is directly apllied to the PLL as
 * frequency range setting.
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1 . FORCE_SET_ENA
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1_FORCE_SET_ENA  VTSS_BIT(0)

/** 
 * \brief
 * RCPLL Enable for half rate mode
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1 . HALF_RATE
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1_HALF_RATE  VTSS_BIT(1)

/** 
 * \brief
 * RCPLL Enable recalibration of PLL when out of range is detected
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1 . OUT_OF_RANGE_RECAL_ENA
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1_OUT_OF_RANGE_RECAL_ENA  VTSS_BIT(2)

/** 
 * \brief
 * RCPLL Power down for the RX-path
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1 . PWD_RX
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1_PWD_RX  VTSS_BIT(3)

/** 
 * \brief
 * RCPLL Power down for the TX-path
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1 . PWD_TX
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1_PWD_TX  VTSS_BIT(4)

/** 
 * \brief
 * RCPLL Enable for quarter rate mode
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1 . QUARTER_RATE
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1_QUARTER_RATE  VTSS_BIT(5)

/** 
 * \brief
 * RCPLL Control input for startup FSM
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1 . RC_CTRL_DATA
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1_RC_CTRL_DATA(x)  VTSS_ENCODE_BITFIELD(x,6,8)
#define  VTSS_M_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1_RC_CTRL_DATA     VTSS_ENCODE_BITMASK(6,8)
#define  VTSS_X_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1_RC_CTRL_DATA(x)  VTSS_EXTRACT_BITFIELD(x,6,8)

/** 
 * \brief
 * RCPLL Enable for startup FSM
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1 . RC_ENABLE
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1_RC_ENABLE  VTSS_BIT(14)

/** 
 * \brief
 * RCPLL Selects whether (when set to 1) the frequency range setting from
 * the FSM can be read back at sx_pll_rb_data_o or (when cleared to 0) the
 * measured period.
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1 . READBACK_DATA_SEL
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1_READBACK_DATA_SEL  VTSS_BIT(15)

/** 
 * \brief
 * RCPLL feedback divider setting
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1 . ROT_DIR
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1_ROT_DIR  VTSS_BIT(16)

/** 
 * \brief
 * RCPLL feedback divider setting
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1 . ROT_SPEED
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1_ROT_SPEED  VTSS_BIT(17)

/** 
 * \brief
 * enable for direct data mode (ATPG/JTAG) reference clock input buffer and
 * test output buffer
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1 . ENA_DIRECT
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG1_ENA_DIRECT  VTSS_BIT(18)


/** 
 * \brief PLL5G Configuration 2
 *
 * \details
 * Configuration register 2 for PLL5G
 *
 * Register: \a MACRO_CTRL:PLL5G_CFG:PLL5G_CFG2
 */
#define VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG2  VTSS_IOREG(VTSS_TO_HSIO,0x2)


/** 
 * \brief PLL5G Configuration 3
 *
 * \details
 * Configuration register 3 for PLL5G
 *
 * Register: \a MACRO_CTRL:PLL5G_CFG:PLL5G_CFG3
 */
#define VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG3  VTSS_IOREG(VTSS_TO_HSIO,0x3)


/** 
 * \brief PLL5G Configuration 4
 *
 * \details
 * Configuration register 4 for PLL5G
 *
 * Register: \a MACRO_CTRL:PLL5G_CFG:PLL5G_CFG4
 */
#define VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG4  VTSS_IOREG(VTSS_TO_HSIO,0x4)


/** 
 * \brief PLL5G Configuration 5
 *
 * \details
 * Configuration register 5 for PLL5G
 *
 * Register: \a MACRO_CTRL:PLL5G_CFG:PLL5G_CFG5
 */
#define VTSS_MACRO_CTRL_PLL5G_CFG_PLL5G_CFG5  VTSS_IOREG(VTSS_TO_HSIO,0x5)

/**
 * Register Group: \a MACRO_CTRL:PLL5G_STATUS
 *
 * PLL5G Status Registers
 */


/** 
 * \brief PLL5G Status 0
 *
 * \details
 * Status register 0 for the PLL5G
 *
 * Register: \a MACRO_CTRL:PLL5G_STATUS:PLL5G_STATUS0
 */
#define VTSS_MACRO_CTRL_PLL5G_STATUS_PLL5G_STATUS0  VTSS_IOREG(VTSS_TO_HSIO,0x6)

/** 
 * \brief
 * PLL lock status 0: not locked, 1: locked
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_STATUS_PLL5G_STATUS0 . LOCK_STATUS
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_STATUS_PLL5G_STATUS0_LOCK_STATUS  VTSS_BIT(0)

/** 
 * \brief
 * RCPLL Interface to read back internal data of the FSM.
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_STATUS_PLL5G_STATUS0 . READBACK_DATA
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_STATUS_PLL5G_STATUS0_READBACK_DATA(x)  VTSS_ENCODE_BITFIELD(x,1,8)
#define  VTSS_M_MACRO_CTRL_PLL5G_STATUS_PLL5G_STATUS0_READBACK_DATA     VTSS_ENCODE_BITMASK(1,8)
#define  VTSS_X_MACRO_CTRL_PLL5G_STATUS_PLL5G_STATUS0_READBACK_DATA(x)  VTSS_EXTRACT_BITFIELD(x,1,8)

/** 
 * \brief
 * RCPLL Flag that indicates that the calibration procedure has finished.
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_STATUS_PLL5G_STATUS0 . CALIBRATION_DONE
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_STATUS_PLL5G_STATUS0_CALIBRATION_DONE  VTSS_BIT(9)

/** 
 * \brief
 * RCPLL Flag that indicates errors that may occur during the calibration
 * procedure.
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_STATUS_PLL5G_STATUS0 . CALIBRATION_ERR
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_STATUS_PLL5G_STATUS0_CALIBRATION_ERR  VTSS_BIT(10)

/** 
 * \brief
 * RCPLL Flag that indicates a out of range condition while NOT in
 * calibration mode.
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_STATUS_PLL5G_STATUS0 . OUT_OF_RANGE_ERR
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_STATUS_PLL5G_STATUS0_OUT_OF_RANGE_ERR  VTSS_BIT(11)

/** 
 * \brief
 * RCPLL Flag range limiter signaling
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_PLL5G_STATUS_PLL5G_STATUS0 . RANGE_LIM
 */
#define  VTSS_F_MACRO_CTRL_PLL5G_STATUS_PLL5G_STATUS0_RANGE_LIM  VTSS_BIT(12)

/**
 * Register Group: \a MACRO_CTRL:RCOMP_CFG
 *
 * RCOMP Configuration Registers
 */

/**
 * Register Group: \a MACRO_CTRL:RCOMP_STATUS
 *
 * RCOMP Status Registers
 */


/** 
 * \brief RCOMP Status
 *
 * \details
 * Status register bits for the RCOMP
 *
 * Register: \a MACRO_CTRL:RCOMP_STATUS:RCOMP_STATUS
 */
#define VTSS_MACRO_CTRL_RCOMP_STATUS_RCOMP_STATUS  VTSS_IOREG(VTSS_TO_HSIO,0x9)

/** 
 * \brief
 * Resistor comparison activity
 *
 * \details 
 * 0: resistor measurement finished or inactive
 * 1: resistor measurement in progress
 *
 * Field: ::VTSS_MACRO_CTRL_RCOMP_STATUS_RCOMP_STATUS . BUSY
 */
#define  VTSS_F_MACRO_CTRL_RCOMP_STATUS_RCOMP_STATUS_BUSY  VTSS_BIT(12)

/** 
 * \brief
 * Alarm signal if rcomp isn't best choice anymore
 *
 * \details 
 * 0: inactive
 * 1: active
 *
 * Field: ::VTSS_MACRO_CTRL_RCOMP_STATUS_RCOMP_STATUS . DELTA_ALERT
 */
#define  VTSS_F_MACRO_CTRL_RCOMP_STATUS_RCOMP_STATUS_DELTA_ALERT  VTSS_BIT(7)

/** 
 * \brief
 * Measured resistor value
 *
 * \details 
 * 0: maximum resistance value
 * 15: minimum resistance value
 *
 * Field: ::VTSS_MACRO_CTRL_RCOMP_STATUS_RCOMP_STATUS . RCOMP
 */
#define  VTSS_F_MACRO_CTRL_RCOMP_STATUS_RCOMP_STATUS_RCOMP(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_MACRO_CTRL_RCOMP_STATUS_RCOMP_STATUS_RCOMP     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_MACRO_CTRL_RCOMP_STATUS_RCOMP_STATUS_RCOMP(x)  VTSS_EXTRACT_BITFIELD(x,0,4)

/**
 * Register Group: \a MACRO_CTRL:SYNC_ETH_CFG
 *
 * SYNC_ETH Configuration Registers
 */


/** 
 * \brief SYNC ETH Configuration 0
 *
 * \details
 * Selection register for SYNC_ETH.
 *
 * Register: \a MACRO_CTRL:SYNC_ETH_CFG:SYNC_ETH_CFG
 */
#define VTSS_MACRO_CTRL_SYNC_ETH_CFG_SYNC_ETH_CFG  VTSS_IOREG(VTSS_TO_HSIO,0xa)

/** 
 * \brief
 * Select recovered clock divider B
 *
 * \details 
 * 0: No clock dividing
 * 1: Divide clock by 5
 * 2: Divide clock by 4
 * 3: Reserved
 *
 * Field: ::VTSS_MACRO_CTRL_SYNC_ETH_CFG_SYNC_ETH_CFG . SEL_RECO_CLK_B
 */
#define  VTSS_F_MACRO_CTRL_SYNC_ETH_CFG_SYNC_ETH_CFG_SEL_RECO_CLK_B(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_MACRO_CTRL_SYNC_ETH_CFG_SYNC_ETH_CFG_SEL_RECO_CLK_B     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_MACRO_CTRL_SYNC_ETH_CFG_SYNC_ETH_CFG_SEL_RECO_CLK_B(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * Select recovered clock divider A
 *
 * \details 
 * 0: No clock dividing
 * 1: Divide clock by 5
 * 2: Divide clock by 4
 * 3: Reserved
 *
 * Field: ::VTSS_MACRO_CTRL_SYNC_ETH_CFG_SYNC_ETH_CFG . SEL_RECO_CLK_A
 */
#define  VTSS_F_MACRO_CTRL_SYNC_ETH_CFG_SYNC_ETH_CFG_SEL_RECO_CLK_A(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_MACRO_CTRL_SYNC_ETH_CFG_SYNC_ETH_CFG_SEL_RECO_CLK_A     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_MACRO_CTRL_SYNC_ETH_CFG_SYNC_ETH_CFG_SEL_RECO_CLK_A(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Enable recovered clock B pad
 *
 * \details 
 * 0: Disable (high-impedance)
 * 1: Enable (output recovered clock)
 *
 * Field: ::VTSS_MACRO_CTRL_SYNC_ETH_CFG_SYNC_ETH_CFG . RECO_CLK_B_ENA
 */
#define  VTSS_F_MACRO_CTRL_SYNC_ETH_CFG_SYNC_ETH_CFG_RECO_CLK_B_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable recovered clock A pad
 *
 * \details 
 * 0: Disable (high-impedance)
 * 1: Enable (output recovered clock)
 *
 * Field: ::VTSS_MACRO_CTRL_SYNC_ETH_CFG_SYNC_ETH_CFG . RECO_CLK_A_ENA
 */
#define  VTSS_F_MACRO_CTRL_SYNC_ETH_CFG_SYNC_ETH_CFG_RECO_CLK_A_ENA  VTSS_BIT(0)

/**
 * Register Group: \a MACRO_CTRL:SERDES1G_ANA_CFG
 *
 * SERDES1G Analog Configuration Registers
 */


/** 
 * \brief SERDES1G Deserializer Cfg
 *
 * \details
 * Configuration register for SERDES1G deserializer
 *
 * Register: \a MACRO_CTRL:SERDES1G_ANA_CFG:SERDES1G_DES_CFG
 */
#define VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG  VTSS_IOREG(VTSS_TO_HSIO,0xb)

/** 
 * \brief
 * Control of phase regulator logic (bit 3 selects input to integrator
 * block - 0: cp/md from DES, 1: cp/md from core)
 *
 * \details 
 * 0: Disabled
 * 1: Enabled with 99 ppm limit
 * 2: Enabled with 202 ppm limit
 * 3: Enabled with 485 ppm limit
 * 4: Enabled if corresponding PCS is in sync with 50 ppm limit
 * 5: Enabled if corresponding PCS is in sync with 99 ppm limit
 * 6: Enabled if corresponding PCS is in sync with 202 ppm limit
 * 7: Enabled if corresponding PCS is in sync with 485 ppm limit
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG . DES_PHS_CTRL
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_PHS_CTRL(x)  VTSS_ENCODE_BITFIELD(x,13,4)
#define  VTSS_M_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_PHS_CTRL     VTSS_ENCODE_BITMASK(13,4)
#define  VTSS_X_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_PHS_CTRL(x)  VTSS_EXTRACT_BITFIELD(x,13,4)

/** 
 * \brief
 * Deserializer phase control, main cp/md select
 *
 * \details 
 * 00: Directly from DES
 * 01: Through hysteresis stage from DES
 * 10: From core
 * 11: Disabled
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG . DES_CPMD_SEL
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_CPMD_SEL(x)  VTSS_ENCODE_BITFIELD(x,11,2)
#define  VTSS_M_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_CPMD_SEL     VTSS_ENCODE_BITMASK(11,2)
#define  VTSS_X_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_CPMD_SEL(x)  VTSS_EXTRACT_BITFIELD(x,11,2)

/** 
 * \brief
 * Des phase control for 180 degrees deadlock block mode of operation
 *
 * \details 
 * 000: Depending on density of input pattern
 * 001: Active until PCS has synchronized
 * 010: Depending on density of input pattern until PCS has synchronized
 * 011: Never
 * 100: Always
 * 111: Debug feature: Add cp/md of DES and cp/md from core
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG . DES_MBTR_CTRL
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_MBTR_CTRL(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_MBTR_CTRL     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_MBTR_CTRL(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * Bandwidth selection. Selects dividing factor for non-hysteresis CP/MD
 * outputs.
 *
 * \details 
 * 0: No division
 * 1: Divide by 2
 * 2: Divide by 4
 * 3: Divide by 8
 * 4: Divide by 16
 * 5: Divide by 32
 * 6: Divide by 64
 * 7: Divide by 128
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG . DES_BW_ANA
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_BW_ANA(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_BW_ANA     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_BW_ANA(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * Bandwidth selection. Selects dividing factor for hysteresis CP/MD
 * outputs.
 *
 * \details 
 * 0: Divide by 2
 * 1: Divide by 4
 * 2: Divide by 8
 * 3: Divide by 16
 * 4: Divide by 32
 * 5: Divide by 64
 * 6: Divide by 128
 * 7: Divide by 256
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG . DES_BW_HYST
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_BW_HYST(x)  VTSS_ENCODE_BITFIELD(x,1,3)
#define  VTSS_M_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_BW_HYST     VTSS_ENCODE_BITMASK(1,3)
#define  VTSS_X_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_DES_CFG_DES_BW_HYST(x)  VTSS_EXTRACT_BITFIELD(x,1,3)


/** 
 * \brief SERDES1G Input Buffer Cfg
 *
 * \details
 * Configuration register for SERDES1G input buffer
 *
 * Register: \a MACRO_CTRL:SERDES1G_ANA_CFG:SERDES1G_IB_CFG
 */
#define VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG  VTSS_IOREG(VTSS_TO_HSIO,0xc)

/** 
 * \brief
 * Switches signal detect circuit into low frequency mode, must be used in
 * fx100 mode
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG . IB_FX100_ENA
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_FX100_ENA  VTSS_BIT(27)

/** 
 * \brief
 * Detect thresholds. Preliminary values:
 *
 * \details 
 * 00: 159-189mVppd
 * 01: 138-164mVppd
 * 10: 109-124mVppd
 * 11: 74-89mVppd
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG . IB_DET_LEV
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_DET_LEV(x)  VTSS_ENCODE_BITFIELD(x,19,3)
#define  VTSS_M_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_DET_LEV     VTSS_ENCODE_BITMASK(19,3)
#define  VTSS_X_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_DET_LEV(x)  VTSS_EXTRACT_BITFIELD(x,19,3)

/** 
 * \brief
 * Input buffer hysteresis levels. Preliminary values:
 *
 * \details 
 * 0: 59-79mV
 * 1: 81-124mV
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG . IB_HYST_LEV
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_HYST_LEV  VTSS_BIT(14)

/** 
 * \brief
 * Enable common mode voltage termination
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG . IB_ENA_CMV_TERM
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_ENA_CMV_TERM  VTSS_BIT(13)

/** 
 * \brief
 * Enable dc-coupling of input signal
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG . IB_ENA_DC_COUPLING
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_ENA_DC_COUPLING  VTSS_BIT(12)

/** 
 * \brief
 * Enable detect level circuit
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG . IB_ENA_DETLEV
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_ENA_DETLEV  VTSS_BIT(11)

/** 
 * \brief
 * Enable hysteresis for input signal
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG . IB_ENA_HYST
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_ENA_HYST  VTSS_BIT(10)

/** 
 * \brief
 * Enable offset compensation of input stage
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG . IB_ENA_OFFSET_COMP
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_ENA_OFFSET_COMP  VTSS_BIT(9)

/** 
 * \brief
 * Selects weighting between AC and DC input path. Preliminary values:
 *
 * \details 
 * 0: Reserved
 * 1: Reserved
 * 2: 0dB
 * 3: 1.5dB
 * 4: 3dB
 * 5: 6dB
 * 6: 9dB
 * 12.5dB
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG . IB_EQ_GAIN
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_EQ_GAIN(x)  VTSS_ENCODE_BITFIELD(x,6,3)
#define  VTSS_M_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_EQ_GAIN     VTSS_ENCODE_BITMASK(6,3)
#define  VTSS_X_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_EQ_GAIN(x)  VTSS_EXTRACT_BITFIELD(x,6,3)

/** 
 * \brief
 * Corner frequencies of AC path. Preliminary values:
 *
 * \details 
 * 0: 1.3GHz
 * 1: 1.5GHz
 * 2: 1.6GHz
 * 3: 1.8GHz
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG . IB_SEL_CORNER_FREQ
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_SEL_CORNER_FREQ(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_SEL_CORNER_FREQ     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_SEL_CORNER_FREQ(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * Resistor control. Value must be taken from RCOMP_STATUS.RCOMP.
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG . IB_RESISTOR_CTRL
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_RESISTOR_CTRL(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_RESISTOR_CTRL     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_IB_CFG_IB_RESISTOR_CTRL(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief SERDES1G Output Buffer Cfg
 *
 * \details
 * Configuration register for SERDES1G output buffer
 *
 * Register: \a MACRO_CTRL:SERDES1G_ANA_CFG:SERDES1G_OB_CFG
 */
#define VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_OB_CFG  VTSS_IOREG(VTSS_TO_HSIO,0xd)

/** 
 * \brief
 * Slope / slew rate control. Preliminary values:
 *
 * \details 
 * 0: 45ps
 * 1: 85ps
 * 2: 105ps
 * 3: 115ps
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_OB_CFG . OB_SLP
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_OB_CFG_OB_SLP(x)  VTSS_ENCODE_BITFIELD(x,17,2)
#define  VTSS_M_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_OB_CFG_OB_SLP     VTSS_ENCODE_BITMASK(17,2)
#define  VTSS_X_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_OB_CFG_OB_SLP(x)  VTSS_EXTRACT_BITFIELD(x,17,2)

/** 
 * \brief
 * Amplitude control. Preliminary values ranging from 0.4Vppd to 1.1Vppd in
 * steps of 50mVppd. 0: 0.4Vppd, 15: 1.1Vppd
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_OB_CFG . OB_AMP_CTRL
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_OB_CFG_OB_AMP_CTRL(x)  VTSS_ENCODE_BITFIELD(x,13,4)
#define  VTSS_M_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_OB_CFG_OB_AMP_CTRL     VTSS_ENCODE_BITMASK(13,4)
#define  VTSS_X_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_OB_CFG_OB_AMP_CTRL(x)  VTSS_EXTRACT_BITFIELD(x,13,4)

/** 
 * \brief
 * Common mode voltage control. Preliminary values:
 *
 * \details 
 * 0: Reserved
 * 1: 440mV
 * 2: 480mV
 * 3: 460mV
 * 4: 530mV
 * 5: 500mV
 * 6: 570mV
 * 7: 550mV
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_OB_CFG . OB_VCM_CTRL
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_OB_CFG_OB_VCM_CTRL(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_OB_CFG_OB_VCM_CTRL     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_OB_CFG_OB_VCM_CTRL(x)  VTSS_EXTRACT_BITFIELD(x,4,4)

/** 
 * \brief
 * Resistor control. Value must be taken from RCOMP_STATUS.RCOMP.
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_OB_CFG . OB_RESISTOR_CTRL
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_OB_CFG_OB_RESISTOR_CTRL(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_OB_CFG_OB_RESISTOR_CTRL     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_OB_CFG_OB_RESISTOR_CTRL(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief SERDES1G Serializer Cfg
 *
 * \details
 * Configuration register for SERDES1G serializer
 *
 * Register: \a MACRO_CTRL:SERDES1G_ANA_CFG:SERDES1G_SER_CFG
 */
#define VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_SER_CFG  VTSS_IOREG(VTSS_TO_HSIO,0xe)

/** 
 * \brief
 * Invert output D0b for idle-mode of OB
 *
 * \details 
 * 0: Non-inverting
 * 1. Inverting
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_SER_CFG . SER_IDLE
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_SER_CFG_SER_IDLE  VTSS_BIT(9)

/** 
 * \brief
 * Invert and delays (one clk cycle) output D1 for de-emphasis of OB
 *
 * \details 
 * 0: Non-inverting and non-delaying
 * 1: Inverting and delaying
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_SER_CFG . SER_DEEMPH
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_SER_CFG_SER_DEEMPH  VTSS_BIT(8)

/** 
 * \brief
 * Enable hysteresis for phase alignment
 *
 * \details 
 * 0: Disable hysteresis
 * 1: Enable hysteresis
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_SER_CFG . SER_ENHYS
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_SER_CFG_SER_ENHYS  VTSS_BIT(3)

/** 
 * \brief
 * Use wider window for phase alignment
 *
 * \details 
 * 0: Use small window for low jitter (100 to 200ps)
 * 1: Use wide window for higher jitter (150 to 300 ps)
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_SER_CFG . SER_BIG_WIN
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_SER_CFG_SER_BIG_WIN  VTSS_BIT(2)

/** 
 * \brief
 * Enable window for phase alignment
 *
 * \details 
 * 0: Disable window
 * 1: Enable window
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_SER_CFG . SER_EN_WIN
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_SER_CFG_SER_EN_WIN  VTSS_BIT(1)

/** 
 * \brief
 * Enable phase alignment
 *
 * \details 
 * 0: Disable phase alignment
 * 1: Enable phase alignment
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_SER_CFG . SER_ENALI
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_SER_CFG_SER_ENALI  VTSS_BIT(0)


/** 
 * \brief SERDES1G Common Cfg
 *
 * \details
 * Configuration register for common SERDES1G functions Note: When enabling
 * the facility loop (ena_floop) also the phase alignment in the serializer
 * has to be enabled and configured adequate.
 *
 * Register: \a MACRO_CTRL:SERDES1G_ANA_CFG:SERDES1G_COMMON_CFG
 */
#define VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG  VTSS_IOREG(VTSS_TO_HSIO,0xf)

/** 
 * \brief
 * System reset (low active)
 *
 * \details 
 * 0: Apply reset (not self-clearing)
 * 1: Reset released
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG . SYS_RST
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_SYS_RST  VTSS_BIT(31)

/** 
 * \brief
 * Enable auto-squelching for sync. ethernet bus B
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG . SE_AUTO_SQUELCH_B_ENA
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_SE_AUTO_SQUELCH_B_ENA  VTSS_BIT(22)

/** 
 * \brief
 * Enable auto-squelching for sync. ethernet bus A
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG . SE_AUTO_SQUELCH_A_ENA
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_SE_AUTO_SQUELCH_A_ENA  VTSS_BIT(21)

/** 
 * \brief
 * Select recovered clock of this lane on sync. ethernet bus B
 *
 * \details 
 * 0: Lane not selected
 * 1: Lane selected
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG . RECO_SEL_B
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_RECO_SEL_B  VTSS_BIT(20)

/** 
 * \brief
 * Select recovered clock of this lane on sync. ethernet bus A
 *
 * \details 
 * 0: Lane not selected
 * 1: Lane selected
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG . RECO_SEL_A
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_RECO_SEL_A  VTSS_BIT(19)

/** 
 * \brief
 * Enable lane
 *
 * \details 
 * 0: Disable lane
 * 1: Enable line
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG . ENA_LANE
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_ENA_LANE  VTSS_BIT(18)

/** 
 * \brief
 * Enable equipment loop
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG . ENA_ELOOP
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_ENA_ELOOP  VTSS_BIT(11)

/** 
 * \brief
 * Enable facility loop
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG . ENA_FLOOP
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG_ENA_FLOOP  VTSS_BIT(10)


/** 
 * \brief SERDES1G Pll Cfg
 *
 * \details
 * Configuration register for SERDES1G RCPLL
 *
 * Register: \a MACRO_CTRL:SERDES1G_ANA_CFG:SERDES1G_PLL_CFG
 */
#define VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_PLL_CFG  VTSS_IOREG(VTSS_TO_HSIO,0x10)

/** 
 * \brief
 * Control data for FSM
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_PLL_CFG . PLL_FSM_CTRL_DATA
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_PLL_CFG_PLL_FSM_CTRL_DATA(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_PLL_CFG_PLL_FSM_CTRL_DATA     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_PLL_CFG_PLL_FSM_CTRL_DATA(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Enable FSM
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_PLL_CFG . PLL_FSM_ENA
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_ANA_CFG_SERDES1G_PLL_CFG_PLL_FSM_ENA  VTSS_BIT(7)

/**
 * Register Group: \a MACRO_CTRL:SERDES1G_ANA_STATUS
 *
 * SERDES1G Analog Status Register
 */


/** 
 * \brief SERDES1G Pll Status
 *
 * \details
 * Status register of SERDES1G RCPLL
 *
 * Register: \a MACRO_CTRL:SERDES1G_ANA_STATUS:SERDES1G_PLL_STATUS
 */
#define VTSS_MACRO_CTRL_SERDES1G_ANA_STATUS_SERDES1G_PLL_STATUS  VTSS_IOREG(VTSS_TO_HSIO,0x11)

/**
 * Register Group: \a MACRO_CTRL:SERDES1G_DIG_CFG
 *
 * SERDES1G Digital Configuration Register
 */


/** 
 * \brief SERDES1G DFT Configuration register 0
 *
 * \details
 * Configuration register 0 for SERDES1G DFT functions
 *
 * Register: \a MACRO_CTRL:SERDES1G_DIG_CFG:SERDES1G_DFT_CFG0
 */
#define VTSS_MACRO_CTRL_SERDES1G_DIG_CFG_SERDES1G_DFT_CFG0  VTSS_IOREG(VTSS_TO_HSIO,0x12)


/** 
 * \brief SERDES1G DFT Configuration register 1
 *
 * \details
 * Configuration register 1 for SERDES1G DFT functions (TX direction)
 *
 * Register: \a MACRO_CTRL:SERDES1G_DIG_CFG:SERDES1G_DFT_CFG1
 */
#define VTSS_MACRO_CTRL_SERDES1G_DIG_CFG_SERDES1G_DFT_CFG1  VTSS_IOREG(VTSS_TO_HSIO,0x13)


/** 
 * \brief SERDES1G DFT Configuration register 2
 *
 * \details
 * Configuration register 2 for SERDES1G DFT functions (RX direction)
 *
 * Register: \a MACRO_CTRL:SERDES1G_DIG_CFG:SERDES1G_DFT_CFG2
 */
#define VTSS_MACRO_CTRL_SERDES1G_DIG_CFG_SERDES1G_DFT_CFG2  VTSS_IOREG(VTSS_TO_HSIO,0x14)


/** 
 * \brief SERDES1G Test Pattern Configuration
 *
 * \details
 * Test bits (pattern) for SERDES1G lane. These bits are used when
 * Lane_Test_cfg.Test_mode is set to 2 (fixed pattern)
 *
 * Register: \a MACRO_CTRL:SERDES1G_DIG_CFG:SERDES1G_TP_CFG
 */
#define VTSS_MACRO_CTRL_SERDES1G_DIG_CFG_SERDES1G_TP_CFG  VTSS_IOREG(VTSS_TO_HSIO,0x15)


/** 
 * \brief SERDES1G Misc Configuration
 *
 * \details
 * Configuration register for miscellaneous functions
 *
 * Register: \a MACRO_CTRL:SERDES1G_DIG_CFG:SERDES1G_MISC_CFG
 */
#define VTSS_MACRO_CTRL_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG  VTSS_IOREG(VTSS_TO_HSIO,0x16)

/** 
 * \brief
 * Enable deserializer cp/md handling for 100fx mode
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG . DES_100FX_CPMD_ENA
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG_DES_100FX_CPMD_ENA  VTSS_BIT(8)

/** 
 * \brief
 * Enable RX-Low-Power feature (Power control by LPI-FSM in connected PCS)
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG . RX_LPI_MODE_ENA
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG_RX_LPI_MODE_ENA  VTSS_BIT(5)

/** 
 * \brief
 * Enable TX-Low-Power feature (Power control by LPI-FSM in connected PCS)
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG . TX_LPI_MODE_ENA
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG_TX_LPI_MODE_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Enable data inversion received from Deserializer
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG . RX_DATA_INV_ENA
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG_RX_DATA_INV_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Enable data inversion sent to Serializer
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG . TX_DATA_INV_ENA
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG_TX_DATA_INV_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Lane Reset
 *
 * \details 
 * 0: No reset
 * 1: Reset (not self-clearing)
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG . LANE_RST
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG_LANE_RST  VTSS_BIT(0)

/**
 * Register Group: \a MACRO_CTRL:SERDES1G_DIG_STATUS
 *
 * SERDES1G Digital Status Register
 */


/** 
 * \brief SERDES1G DFT Status
 *
 * \details
 * Status register of SERDES1G DFT functions
 *
 * Register: \a MACRO_CTRL:SERDES1G_DIG_STATUS:SERDES1G_DFT_STATUS
 */
#define VTSS_MACRO_CTRL_SERDES1G_DIG_STATUS_SERDES1G_DFT_STATUS  VTSS_IOREG(VTSS_TO_HSIO,0x17)

/** 
 * \brief
 * BIST sync result
 *
 * \details 
 * 0: Synchronization successful
 * 1: Synchronization on BIST data failed
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES1G_DIG_STATUS_SERDES1G_DFT_STATUS . BIST_NOSYNC
 */
#define  VTSS_F_MACRO_CTRL_SERDES1G_DIG_STATUS_SERDES1G_DFT_STATUS_BIST_NOSYNC  VTSS_BIT(2)

/**
 * Register Group: \a MACRO_CTRL:MCB_SERDES1G_CFG
 *
 * MCB SERDES1G Configuration Register
 */


/** 
 * \brief MCB SERDES1G Address Cfg
 *
 * \details
 * Configuration of SERDES1G MCB slaves to be accessed
 *
 * Register: \a MACRO_CTRL:MCB_SERDES1G_CFG:MCB_SERDES1G_ADDR_CFG
 */
#define VTSS_MACRO_CTRL_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG  VTSS_IOREG(VTSS_TO_HSIO,0x18)

/** 
 * \brief
 * Initiate a write access to marked SERDES1G slaves
 *
 * \details 
 * 0: No write operation pending
 * 1: Initiate write to slaves (kept 1 until write operation has finished)
 *
 * Field: ::VTSS_MACRO_CTRL_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG . SERDES1G_WR_ONE_SHOT
 */
#define  VTSS_F_MACRO_CTRL_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG_SERDES1G_WR_ONE_SHOT  VTSS_BIT(31)

/** 
 * \brief
 * Initiate a read access to marked SERDES1G slaves
 *
 * \details 
 * 0: No read operation pending (read op finished after bit has been set)
 * 1: Initiate a read access (kept 1 until read operation has finished)
 *
 * Field: ::VTSS_MACRO_CTRL_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG . SERDES1G_RD_ONE_SHOT
 */
#define  VTSS_F_MACRO_CTRL_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG_SERDES1G_RD_ONE_SHOT  VTSS_BIT(30)

/** 
 * \brief
 * Activation vector for SERDES1G-Slaves, one-hot coded, each bit is
 * related to one macro, e.g. bit 0 enables/disables access to macro No. 0
 *
 * \details 
 * 0: Disable macro access via MCB
 * 1: Enable macro access via MCB
 *
 * Field: ::VTSS_MACRO_CTRL_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG . SERDES1G_ADDR
 */
#define  VTSS_F_MACRO_CTRL_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG_SERDES1G_ADDR(x)  VTSS_ENCODE_BITFIELD(x,0,25)
#define  VTSS_M_MACRO_CTRL_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG_SERDES1G_ADDR     VTSS_ENCODE_BITMASK(0,25)
#define  VTSS_X_MACRO_CTRL_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG_SERDES1G_ADDR(x)  VTSS_EXTRACT_BITFIELD(x,0,25)

/**
 * Register Group: \a MACRO_CTRL:SERDES6G_ANA_CFG
 *
 * SERDES6G Analog Configuration Registers
 */


/** 
 * \brief SERDES6G Deserializer Cfg
 *
 * \details
 * Configuration register for SERDES6G deserializer
 *
 * Register: \a MACRO_CTRL:SERDES6G_ANA_CFG:SERDES6G_DES_CFG
 */
#define VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG  VTSS_IOREG(VTSS_TO_HSIO,0x19)

/** 
 * \brief
 * Control of phase regulator logic (bit 3 selects input to integrator
 * block - 0: cp/md from DES, 1: cp/md from core)
 *
 * \details 
 * 0: Disabled
 * 1: Enabled with 99 ppm limit
 * 2: Enabled with 202 ppm limit
 * 3: Enabled with 485 ppm limit
 * 4: Enabled if corresponding PCS is in sync with 50 ppm limit
 * 5: Enabled if corresponding PCS is in sync with 99 ppm limit
 * 6: Enabled if corresponding PCS is in sync with 202 ppm limit
 * 7: Enabled if corresponding PCS is in sync with 485 ppm limit
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG . DES_PHS_CTRL
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_PHS_CTRL(x)  VTSS_ENCODE_BITFIELD(x,13,4)
#define  VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_PHS_CTRL     VTSS_ENCODE_BITMASK(13,4)
#define  VTSS_X_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_PHS_CTRL(x)  VTSS_EXTRACT_BITFIELD(x,13,4)

/** 
 * \brief
 * Des phase control for 180 degrees deadlock block mode of operation
 *
 * \details 
 * 000: Depending on density of input pattern
 * 001: Active until PCS has synchronized
 * 010: Depending on density of input pattern until PCS has synchronized
 * 011: Never
 * 100: Always
 * 111: Debug feature: Add cp/md of DES and cp/md from core
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG . DES_MBTR_CTRL
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_MBTR_CTRL(x)  VTSS_ENCODE_BITFIELD(x,10,3)
#define  VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_MBTR_CTRL     VTSS_ENCODE_BITMASK(10,3)
#define  VTSS_X_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_MBTR_CTRL(x)  VTSS_EXTRACT_BITFIELD(x,10,3)

/** 
 * \brief
 * DES phase control, main cp/md select
 *
 * \details 
 * 00: Directly from DES
 * 01: Through hysteresis stage from DES
 * 10: From core
 * 11: Disabled
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG . DES_CPMD_SEL
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_CPMD_SEL(x)  VTSS_ENCODE_BITFIELD(x,8,2)
#define  VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_CPMD_SEL     VTSS_ENCODE_BITMASK(8,2)
#define  VTSS_X_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_CPMD_SEL(x)  VTSS_EXTRACT_BITFIELD(x,8,2)

/** 
 * \brief
 * Bandwidth selection. Selects dividing factor for hysteresis CP/MD
 * outputs.
 *
 * \details 
 * 0: Divide by 2
 * 1: Divide by 4
 * 2: Divide by 8
 * 3: Divide by 16
 * 4: Divide by 32
 * 5: Divide by 64
 * 6: Divide by 128
 * 7: Divide by 256
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG . DES_BW_HYST
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_BW_HYST(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_BW_HYST     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_BW_HYST(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * Bandwidth selection. Selects dividing factor for non-hysteresis CP/MD
 * outputs.
 *
 * \details 
 * 0: No division
 * 1: Divide by 2
 * 2: Divide by 4
 * 3: Divide by 8
 * 4: Divide by 16
 * 5: Divide by 32
 * 6: Divide by 64
 * 7: Divide by 128
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG . DES_BW_ANA
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_BW_ANA(x)  VTSS_ENCODE_BITFIELD(x,1,3)
#define  VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_BW_ANA     VTSS_ENCODE_BITMASK(1,3)
#define  VTSS_X_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_DES_CFG_DES_BW_ANA(x)  VTSS_EXTRACT_BITFIELD(x,1,3)


/** 
 * \brief SERDES6G Input Buffer Cfg
 *
 * \details
 * Configuration register 0 for SERDES6G input buffer
 *
 * Register: \a MACRO_CTRL:SERDES6G_ANA_CFG:SERDES6G_IB_CFG
 */
#define VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG  VTSS_IOREG(VTSS_TO_HSIO,0x1a)

/** 
 * \brief
 * Recovery time for AGC of ac path
 *
 * \details 
 * 0: high
 * 7: low
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG . IB_IC_AC
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_IC_AC(x)  VTSS_ENCODE_BITFIELD(x,25,3)
#define  VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_IC_AC     VTSS_ENCODE_BITMASK(25,3)
#define  VTSS_X_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_IC_AC(x)  VTSS_EXTRACT_BITFIELD(x,25,3)

/** 
 * \brief
 * Recovery time for AGC of ac and dc path (also level detection)
 *
 * \details 
 * 0: high
 * 7: low
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG . IB_IC_COM
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_IC_COM(x)  VTSS_ENCODE_BITFIELD(x,22,3)
#define  VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_IC_COM     VTSS_ENCODE_BITMASK(22,3)
#define  VTSS_X_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_IC_COM(x)  VTSS_EXTRACT_BITFIELD(x,22,3)

/** 
 * \brief
 * Recovery time for AGC of dc path
 *
 * \details 
 * 0: high
 * 7: low
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG . IB_IC_DC
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_IC_DC(x)  VTSS_ENCODE_BITFIELD(x,19,3)
#define  VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_IC_DC     VTSS_ENCODE_BITMASK(19,3)
#define  VTSS_X_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_IC_DC(x)  VTSS_EXTRACT_BITFIELD(x,19,3)

/** 
 * \brief
 * Data rate selector (high pass filter)
 *
 * \details 
 * 0: 8G
 * 15: 1G
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG . IB_RF
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_RF(x)  VTSS_ENCODE_BITFIELD(x,14,4)
#define  VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_RF     VTSS_ENCODE_BITMASK(14,4)
#define  VTSS_X_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_RF(x)  VTSS_EXTRACT_BITFIELD(x,14,4)

/** 
 * \brief
 * Loss correction (low pass filter)
 *
 * \details 
 * 0: high frequencies
 * 15: low frequencies
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG . IB_RT
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_RT(x)  VTSS_ENCODE_BITFIELD(x,10,4)
#define  VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_RT     VTSS_ENCODE_BITMASK(10,4)
#define  VTSS_X_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_RT(x)  VTSS_EXTRACT_BITFIELD(x,10,4)

/** 
 * \brief
 * Level detection thresholds. Preliminary values in steps of approx. 8mV:
 * 0: 60mV, 7: 120mV
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG . IB_VBCOM
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_VBCOM(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_VBCOM     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_VBCOM(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * Resistor control. Value must be taken from RCOMP_STATUS.RCOMP.
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG . IB_RESISTOR_CTRL
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_RESISTOR_CTRL(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_RESISTOR_CTRL     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG_IB_RESISTOR_CTRL(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief SERDES6G Input Buffer Cfg1
 *
 * \details
 * Configuration register 1 for SERDES6G input buffer
 *
 * Register: \a MACRO_CTRL:SERDES6G_ANA_CFG:SERDES6G_IB_CFG1
 */
#define VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1  VTSS_IOREG(VTSS_TO_HSIO,0x1b)

/** 
 * \brief
 * Filter settings for auto adjust (adapt for data rate).
 *
 * \details 
 * 0: high frequencies (8G)
 * 15: low frequencies (1G)
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1 . IB_C
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_C(x)  VTSS_ENCODE_BITFIELD(x,8,4)
#define  VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_C     VTSS_ENCODE_BITMASK(8,4)
#define  VTSS_X_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_C(x)  VTSS_EXTRACT_BITFIELD(x,8,4)

/** 
 * \brief
 * Common mode termination
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1 . IB_CTERM_ENA
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_CTERM_ENA  VTSS_BIT(5)

/** 
 * \brief
 * Disables equalization, use for low frequency modes
 *
 * \details 
 * 0: Enable
 * 1: Disable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1 . IB_DIS_EQ
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_DIS_EQ  VTSS_BIT(4)

/** 
 * \brief
 * Auto offset compensation for ac path
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1 . IB_ENA_OFFSAC
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_ENA_OFFSAC  VTSS_BIT(3)

/** 
 * \brief
 * Auto offset compensation for dc path
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1 . IB_ENA_OFFSDC
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_ENA_OFFSDC  VTSS_BIT(2)

/** 
 * \brief
 * Increases timing constant for level detect circuit, must be used in
 * FX100 mode
 *
 * \details 
 * 0: Normal speed
 * 1: Slow speed (oversampling)
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1 . IB_FX100_ENA
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_FX100_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Reset for IB internal state machine
 *
 * \details 
 * 0: Normal operation
 * 1: Apply reset (not self-clearing)
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1 . IB_RST
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1_IB_RST  VTSS_BIT(0)


/** 
 * \brief SERDES6G Output Buffer Cfg
 *
 * \details
 * Configuration register 0 for SERDES6G output buffer
 *
 * Register: \a MACRO_CTRL:SERDES6G_ANA_CFG:SERDES6G_OB_CFG
 */
#define VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG  VTSS_IOREG(VTSS_TO_HSIO,0x1c)

/** 
 * \brief
 * PCIe support
 *
 * \details 
 * 1: idle - force to 0V differential
 * 0: Normal mode
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG . OB_IDLE
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_IDLE  VTSS_BIT(31)

/** 
 * \brief
 * Output buffer supply voltage
 *
 * \details 
 * 1: Set to nominal 1V
 * 0: Set to higher voltage
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG . OB_ENA1V_MODE
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_ENA1V_MODE  VTSS_BIT(30)

/** 
 * \brief
 * Polarity of output signal
 *
 * \details 
 * 0: Normal
 * 1: Inverted
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG . OB_POL
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_POL  VTSS_BIT(29)

/** 
 * \brief
 * Coefficients for 1st Post Cursor (MSB is sign)
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG . OB_POST0
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_POST0(x)  VTSS_ENCODE_BITFIELD(x,23,6)
#define  VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_POST0     VTSS_ENCODE_BITMASK(23,6)
#define  VTSS_X_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_POST0(x)  VTSS_EXTRACT_BITFIELD(x,23,6)

/** 
 * \brief
 * Coefficients for 2nd Post Cursor (MSB is sign)
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG . OB_POST1
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_POST1(x)  VTSS_ENCODE_BITFIELD(x,18,5)
#define  VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_POST1     VTSS_ENCODE_BITMASK(18,5)
#define  VTSS_X_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_POST1(x)  VTSS_EXTRACT_BITFIELD(x,18,5)

/** 
 * \brief
 * Coefficients for Pre Cursor (MSB is sign)
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG . OB_PREC
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_PREC(x)  VTSS_ENCODE_BITFIELD(x,13,5)
#define  VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_PREC     VTSS_ENCODE_BITMASK(13,5)
#define  VTSS_X_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_PREC(x)  VTSS_EXTRACT_BITFIELD(x,13,5)

/** 
 * \brief
 * Half the predriver speed, use for slew rate control
 *
 * \details 
 * 0: Disable - slew rate < 60 ps
 * 1: Enable - slew rate > 60 ps
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG . OB_SR_H
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_SR_H  VTSS_BIT(8)

/** 
 * \brief
 * Resistor control. Value must be taken from RCOMP_STATUS.RCOMP.
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG . OB_RESISTOR_CTRL
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_RESISTOR_CTRL(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_RESISTOR_CTRL     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_RESISTOR_CTRL(x)  VTSS_EXTRACT_BITFIELD(x,4,4)

/** 
 * \brief
 * Driver speed, fine adjustment of slew rate 30-60ps (if OB_SR_H = 0),
 * 60-140ps (if OB_SR_H = 1)
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG . OB_SR
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_SR(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_SR     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG_OB_SR(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief SERDES6G Output Buffer Cfg1
 *
 * \details
 * Configuration register 1 for SERDES6G output buffer
 *
 * Register: \a MACRO_CTRL:SERDES6G_ANA_CFG:SERDES6G_OB_CFG1
 */
#define VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1  VTSS_IOREG(VTSS_TO_HSIO,0x1d)

/** 
 * \brief
 * Output skew, used for skew adjustment in SGMII mode
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1 . OB_ENA_CAS
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1_OB_ENA_CAS(x)  VTSS_ENCODE_BITFIELD(x,6,3)
#define  VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1_OB_ENA_CAS     VTSS_ENCODE_BITMASK(6,3)
#define  VTSS_X_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1_OB_ENA_CAS(x)  VTSS_EXTRACT_BITFIELD(x,6,3)

/** 
 * \brief
 * Level of output amplitude
 *
 * \details 
 * 0: lowest level
 * 63: highest level
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1 . OB_LEV
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1_OB_LEV(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1_OB_LEV     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1_OB_LEV(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief SERDES6G Serializer Cfg
 *
 * \details
 * Configuration register for SERDES6G serializer
 *
 * Register: \a MACRO_CTRL:SERDES6G_ANA_CFG:SERDES6G_SER_CFG
 */
#define VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_SER_CFG  VTSS_IOREG(VTSS_TO_HSIO,0x1e)

/** 
 * \brief
 * Select reference clock source for phase alignment
 *
 * \details 
 * 00: RXCLKP
 * 01: RefClk15MHz
 * 10: RXCLKN
 * 11: ext. ALICLK
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_SER_CFG . SER_ALISEL
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_SER_CFG_SER_ALISEL(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_SER_CFG_SER_ALISEL     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_SER_CFG_SER_ALISEL(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * Enable hysteresis for phase alignment
 *
 * \details 
 * 0: Disable hysteresis
 * 1: Enable hysteresis
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_SER_CFG . SER_ENHYS
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_SER_CFG_SER_ENHYS  VTSS_BIT(3)

/** 
 * \brief
 * Enable window for phase alignment
 *
 * \details 
 * 0: Disable window
 * 1: Enable window
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_SER_CFG . SER_EN_WIN
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_SER_CFG_SER_EN_WIN  VTSS_BIT(1)

/** 
 * \brief
 * Enable phase alignment
 *
 * \details 
 * 0: Disable phase alignment
 * 1: Enable phase alignment
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_SER_CFG . SER_ENALI
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_SER_CFG_SER_ENALI  VTSS_BIT(0)


/** 
 * \brief SERDES6G Common Cfg
 *
 * \details
 * Configuration register for common SERDES6G functions Note: When enabling
 * the facility loop (ena_floop) also the phase alignment in the serializer
 * has to be enabled and configured adequate.
 *
 * Register: \a MACRO_CTRL:SERDES6G_ANA_CFG:SERDES6G_COMMON_CFG
 */
#define VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG  VTSS_IOREG(VTSS_TO_HSIO,0x1f)

/** 
 * \brief
 * System reset (low active)
 *
 * \details 
 * 0: Apply reset (not self-clearing)
 * 1: Reset released
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG . SYS_RST
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_SYS_RST  VTSS_BIT(31)

/** 
 * \brief
 * Enable auto-squelching for sync. ethernet bus B
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG . SE_AUTO_SQUELCH_B_ENA
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_SE_AUTO_SQUELCH_B_ENA  VTSS_BIT(22)

/** 
 * \brief
 * Enable auto-squelching for sync. ethernet bus A
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG . SE_AUTO_SQUELCH_A_ENA
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_SE_AUTO_SQUELCH_A_ENA  VTSS_BIT(21)

/** 
 * \brief
 * Select recovered clock of this lane on sync. ethernet bus B
 *
 * \details 
 * 0: Lane not selected
 * 1: Lane selected
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG . RECO_SEL_B
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_RECO_SEL_B  VTSS_BIT(20)

/** 
 * \brief
 * Select recovered clock of this lane on sync. ethernet bus A
 *
 * \details 
 * 0: Lane not selected
 * 1: Lane selected
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG . RECO_SEL_A
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_RECO_SEL_A  VTSS_BIT(19)

/** 
 * \brief
 * Enable lane
 *
 * \details 
 * 0: Disable lane
 * 1: Enable line
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG . ENA_LANE
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_ENA_LANE  VTSS_BIT(18)

/** 
 * \brief
 * Enable equipment loop
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG . ENA_ELOOP
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_ENA_ELOOP  VTSS_BIT(11)

/** 
 * \brief
 * Enable facility loop
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG . ENA_FLOOP
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_ENA_FLOOP  VTSS_BIT(10)

/** 
 * \brief
 * Enable half rate
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG . HRATE
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_HRATE  VTSS_BIT(7)

/** 
 * \brief
 * Enable quarter rate
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG . QRATE
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_QRATE  VTSS_BIT(6)

/** 
 * \brief
 * Interface mode
 *
 * \details 
 * 0: 8-bit mode
 * 1: 10-bit mode
 * 2: 16-bit mode
 * 3: 20-bit mode
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG . IF_MODE
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_IF_MODE(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_IF_MODE     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG_IF_MODE(x)  VTSS_EXTRACT_BITFIELD(x,4,2)


/** 
 * \brief SERDES6G Pll Cfg
 *
 * \details
 * Configuration register for SERDES6G RCPLL
 *
 * Register: \a MACRO_CTRL:SERDES6G_ANA_CFG:SERDES6G_PLL_CFG
 */
#define VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG  VTSS_IOREG(VTSS_TO_HSIO,0x20)

/** 
 * \brief
 * Enable div4 mode
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG . PLL_DIV4
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_DIV4  VTSS_BIT(20)

/** 
 * \brief
 * Enable rotation
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG . PLL_ENA_ROT
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_ENA_ROT  VTSS_BIT(18)

/** 
 * \brief
 * Control data for FSM
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG . PLL_FSM_CTRL_DATA
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_FSM_CTRL_DATA(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_FSM_CTRL_DATA     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_FSM_CTRL_DATA(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Enable FSM
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG . PLL_FSM_ENA
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_FSM_ENA  VTSS_BIT(7)

/** 
 * \brief
 * Select rotation direction
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG . PLL_ROT_DIR
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_ROT_DIR  VTSS_BIT(2)

/** 
 * \brief
 * Select rotation frequency
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG . PLL_ROT_FRQ
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG_PLL_ROT_FRQ  VTSS_BIT(1)

/**
 * Register Group: \a MACRO_CTRL:SERDES6G_ANA_STATUS
 *
 * SERDES6G Analog Status Register
 */


/** 
 * \brief SERDES6G Pll Status
 *
 * \details
 * Status register of SERDES6G RCPLL
 *
 * Register: \a MACRO_CTRL:SERDES6G_ANA_STATUS:SERDES6G_PLL_STATUS
 */
#define VTSS_MACRO_CTRL_SERDES6G_ANA_STATUS_SERDES6G_PLL_STATUS  VTSS_IOREG(VTSS_TO_HSIO,0x21)

/**
 * Register Group: \a MACRO_CTRL:SERDES6G_DIG_CFG
 *
 * SERDES6G Digital Configuration Registers
 */


/** 
 * \brief SERDES6G Digital Configuration register
 *
 * \details
 * Configuration register for SERDES6G digital functions
 *
 * Register: \a MACRO_CTRL:SERDES6G_DIG_CFG:SERDES6G_DIG_CFG
 */
#define VTSS_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_DIG_CFG  VTSS_IOREG(VTSS_TO_HSIO,0x22)

/** 
 * \brief
 * Signal detect assertion time
 *
 * \details 
 * 0: 0 us
 * 1: 35 us
 * 2: 70 us
 * 3: 105 us
 * 4: 140 us
 * 5..7: reserved
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_DIG_CFG . SIGDET_AST
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_DIG_CFG_SIGDET_AST(x)  VTSS_ENCODE_BITFIELD(x,3,3)
#define  VTSS_M_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_DIG_CFG_SIGDET_AST     VTSS_ENCODE_BITMASK(3,3)
#define  VTSS_X_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_DIG_CFG_SIGDET_AST(x)  VTSS_EXTRACT_BITFIELD(x,3,3)

/** 
 * \brief
 * Signal detect de-assertion time
 *
 * \details 
 * 0: 0 us
 * 1: 250 us
 * 2: 350 us
 * 3: 450 us
 * 4: 550 us
 * 5..7: reserved
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_DIG_CFG . SIGDET_DST
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_DIG_CFG_SIGDET_DST(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_DIG_CFG_SIGDET_DST     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_DIG_CFG_SIGDET_DST(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief SERDES6G DFT Configuration register 0
 *
 * \details
 * Configuration register 0 for SERDES6G DFT functions
 *
 * Register: \a MACRO_CTRL:SERDES6G_DIG_CFG:SERDES6G_DFT_CFG0
 */
#define VTSS_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_DFT_CFG0  VTSS_IOREG(VTSS_TO_HSIO,0x23)


/** 
 * \brief SERDES6G DFT Configuration register 1
 *
 * \details
 * Configuration register 1 for SERDES6G DFT functions (TX direction)
 *
 * Register: \a MACRO_CTRL:SERDES6G_DIG_CFG:SERDES6G_DFT_CFG1
 */
#define VTSS_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_DFT_CFG1  VTSS_IOREG(VTSS_TO_HSIO,0x24)


/** 
 * \brief SERDES6G DFT Configuration register 2
 *
 * \details
 * Configuration register 2 for SERDES6G DFT functions (RX direction)
 *
 * Register: \a MACRO_CTRL:SERDES6G_DIG_CFG:SERDES6G_DFT_CFG2
 */
#define VTSS_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_DFT_CFG2  VTSS_IOREG(VTSS_TO_HSIO,0x25)


/** 
 * \brief SERDES6G Test Pattern Configuration
 *
 * \details
 * Test bits (pattern) for SERDES6G lane. These bits are used when
 * Lane_Test_cfg.Test_mode is set to 2 (fixed pattern)
 *
 * Register: \a MACRO_CTRL:SERDES6G_DIG_CFG:SERDES6G_TP_CFG0
 */
#define VTSS_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_TP_CFG0  VTSS_IOREG(VTSS_TO_HSIO,0x26)


/** 
 * \brief SERDES6G Test Pattern Configuration
 *
 * \details
 * Test bits (pattern) for SERDES6G lane. These bits are used when
 * Lane_Test_cfg.Test_mode is set to 2 (fixed pattern) and
 * Lane_cfg.hr_mode_ena = '0'. In 20 bit modes bits from static_pattern and
 * static_pattern2 are transmitted alternating.
 *
 * Register: \a MACRO_CTRL:SERDES6G_DIG_CFG:SERDES6G_TP_CFG1
 */
#define VTSS_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_TP_CFG1  VTSS_IOREG(VTSS_TO_HSIO,0x27)


/** 
 * \brief SERDES6G Misc Configuration
 *
 * \details
 * Configuration register for miscellaneous functions
 *
 * Register: \a MACRO_CTRL:SERDES6G_DIG_CFG:SERDES6G_MISC_CFG
 */
#define VTSS_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG  VTSS_IOREG(VTSS_TO_HSIO,0x28)

/** 
 * \brief
 * Enable deserializer cp/md handling for 100fx mode
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG . DES_100FX_CPMD_ENA
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_DES_100FX_CPMD_ENA  VTSS_BIT(8)

/** 
 * \brief
 * Enable RX-Low-Power feature (Power control by LPI-FSM in connected PCS)
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG . RX_LPI_MODE_ENA
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_RX_LPI_MODE_ENA  VTSS_BIT(5)

/** 
 * \brief
 * Enable TX-Low-Power feature (Power control by LPI-FSM in connected PCS)
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG . TX_LPI_MODE_ENA
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_TX_LPI_MODE_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Enable data inversion received from Deserializer
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG . RX_DATA_INV_ENA
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_RX_DATA_INV_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Enable data inversion sent to Serializer
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG . TX_DATA_INV_ENA
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_TX_DATA_INV_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Lane Reset
 *
 * \details 
 * 0: No reset
 * 1: Reset (not self-clearing)
 *
 * Field: ::VTSS_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG . LANE_RST
 */
#define  VTSS_F_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG_LANE_RST  VTSS_BIT(0)


/** 
 * \brief SERDES6G OB ANEG Configuration
 *
 * \details
 * Configuration register for ANEG Output Buffer overwrite parameter. The
 * values are used during Backplane Ethernet Auto-Negotiation when the
 * output level of transmitter (OB output) have to be reduced.
 *
 * Register: \a MACRO_CTRL:SERDES6G_DIG_CFG:SERDES6G_OB_ANEG_CFG
 */
#define VTSS_MACRO_CTRL_SERDES6G_DIG_CFG_SERDES6G_OB_ANEG_CFG  VTSS_IOREG(VTSS_TO_HSIO,0x29)

/**
 * Register Group: \a MACRO_CTRL:SERDES6G_DIG_STATUS
 *
 * SERDES6G Digital Status Register
 */


/** 
 * \brief SERDES6G DFT Status
 *
 * \details
 * Status register of SERDES6G DFT functions
 *
 * Register: \a MACRO_CTRL:SERDES6G_DIG_STATUS:SERDES6G_DFT_STATUS
 */
#define VTSS_MACRO_CTRL_SERDES6G_DIG_STATUS_SERDES6G_DFT_STATUS  VTSS_IOREG(VTSS_TO_HSIO,0x2a)

/**
 * Register Group: \a MACRO_CTRL:MCB_SERDES6G_CFG
 *
 * MCB SERDES6G Configuration Register
 */


/** 
 * \brief MCB SERDES6G Address Cfg
 *
 * \details
 * Configuration of SERDES6G MCB Slaves to be accessed
 *
 * Register: \a MACRO_CTRL:MCB_SERDES6G_CFG:MCB_SERDES6G_ADDR_CFG
 */
#define VTSS_MACRO_CTRL_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG  VTSS_IOREG(VTSS_TO_HSIO,0x2b)

/** 
 * \brief
 * Initiate a write access to marked SERDES6G Slaves
 *
 * \details 
 * 0: No write operation pending
 * 1: Initiate write to slaves (kept 1 until write operation has finished)
 *
 * Field: ::VTSS_MACRO_CTRL_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG . SERDES6G_WR_ONE_SHOT
 */
#define  VTSS_F_MACRO_CTRL_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG_SERDES6G_WR_ONE_SHOT  VTSS_BIT(31)

/** 
 * \brief
 * Initiate a read access to marked SERDES6G Slaves
 *
 * \details 
 * 0: No read operation pending (read op finished after bit has been set)
 * 1: Initiate a read access (kept 1 until read operation has finished)
 *
 * Field: ::VTSS_MACRO_CTRL_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG . SERDES6G_RD_ONE_SHOT
 */
#define  VTSS_F_MACRO_CTRL_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG_SERDES6G_RD_ONE_SHOT  VTSS_BIT(30)

/** 
 * \brief
 * Activation vector for SERDES6G-Slaves, one-hot coded, each bit is
 * related to one macro, e.g. bit 0 enables/disables access to macro No. 0
 *
 * \details 
 * 0: Disable macro access via MCB
 * 1: Enable macro access via MCB
 *
 * Field: ::VTSS_MACRO_CTRL_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG . SERDES6G_ADDR
 */
#define  VTSS_F_MACRO_CTRL_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG_SERDES6G_ADDR(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_MACRO_CTRL_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG_SERDES6G_ADDR     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_MACRO_CTRL_MCB_SERDES6G_CFG_MCB_SERDES6G_ADDR_CFG_SERDES6G_ADDR(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a MACRO_CTRL:VAUI_CHANNEL_CFG
 *
 * VAUI channel Configuration Registers
 */


/** 
 * \brief Configuration for VAUI channel
 *
 * \details
 * Configuration register for specific vaui channel settings
 *
 * Register: \a MACRO_CTRL:VAUI_CHANNEL_CFG:VAUI_CHANNEL_CFG
 *
 * @param ri Register: VAUI_CHANNEL_CFG (??), 0-3
 */
#define VTSS_MACRO_CTRL_VAUI_CHANNEL_CFG_VAUI_CHANNEL_CFG(ri)  VTSS_IOREG(VTSS_TO_HSIO,0x2c + (ri))

/** 
 * \brief
 * Enable alignment of lane to a common clock (e.g. XAUI mode) and dock
 * lane with ANEG of lane 0 (Master-ANEG)
 *
 * \details 
 * 0: Off
 * 1: On
 *
 * Field: ::VTSS_MACRO_CTRL_VAUI_CHANNEL_CFG_VAUI_CHANNEL_CFG . LANE_SYNC_ENA
 */
#define  VTSS_F_MACRO_CTRL_VAUI_CHANNEL_CFG_VAUI_CHANNEL_CFG_LANE_SYNC_ENA(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_MACRO_CTRL_VAUI_CHANNEL_CFG_VAUI_CHANNEL_CFG_LANE_SYNC_ENA     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_MACRO_CTRL_VAUI_CHANNEL_CFG_VAUI_CHANNEL_CFG_LANE_SYNC_ENA(x)  VTSS_EXTRACT_BITFIELD(x,4,4)

/** 
 * \brief
 * Combine signal_detect information
 *
 * \details 
 * 0: Each lane provides own signal_detect
 * 1: Signal_detect of all four lanes are ANDed
 *
 * Field: ::VTSS_MACRO_CTRL_VAUI_CHANNEL_CFG_VAUI_CHANNEL_CFG . SIGDET_MODE
 */
#define  VTSS_F_MACRO_CTRL_VAUI_CHANNEL_CFG_VAUI_CHANNEL_CFG_SIGDET_MODE  VTSS_BIT(0)

/**
 * Register Group: \a MACRO_CTRL:ANEG_CFG
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
 * Register: \a MACRO_CTRL:ANEG_CFG:ANEG_CFG
 *
 * @param gi Replicator: x_ANEG_INST (??), 0-15
 */
#define VTSS_MACRO_CTRL_ANEG_CFG_ANEG_CFG(gi)  VTSS_IOREG_IX(VTSS_TO_HSIO,0x30,gi,5,0,0)

/** 
 * \brief
 * Disable automatic ANEG OB configuration
 *
 * \details 
 * 0: Allow ANEG block to control OB during auto-negotiation
 * 1: OB settings are not touched by ANEG block
 *
 * Field: ::VTSS_MACRO_CTRL_ANEG_CFG_ANEG_CFG . ANEG_OB_CTRL_DIS
 */
#define  VTSS_F_MACRO_CTRL_ANEG_CFG_ANEG_CFG_ANEG_OB_CTRL_DIS  VTSS_BIT(16)

/** 
 * \brief
 * Parallel detect wait time for 1G using single lane
 *
 * \details 
 * 0: 0 ms
 * 1: 10 ms
 * 2: 20 ms
 * 3: 40 ms
 *
 * Field: ::VTSS_MACRO_CTRL_ANEG_CFG_ANEG_CFG . PD_TIMER_1GKX
 */
#define  VTSS_F_MACRO_CTRL_ANEG_CFG_ANEG_CFG_PD_TIMER_1GKX(x)  VTSS_ENCODE_BITFIELD(x,10,2)
#define  VTSS_M_MACRO_CTRL_ANEG_CFG_ANEG_CFG_PD_TIMER_1GKX     VTSS_ENCODE_BITMASK(10,2)
#define  VTSS_X_MACRO_CTRL_ANEG_CFG_ANEG_CFG_PD_TIMER_1GKX(x)  VTSS_EXTRACT_BITFIELD(x,10,2)

/** 
 * \brief
 * Parallel detect wait time for 2.5G using single lane
 *
 * \details 
 * 0: 0 ms
 * 1: 10 ms
 * 2: 20 ms
 * 3: 40 ms
 *
 * Field: ::VTSS_MACRO_CTRL_ANEG_CFG_ANEG_CFG . PD_TIMER_2G5
 */
#define  VTSS_F_MACRO_CTRL_ANEG_CFG_ANEG_CFG_PD_TIMER_2G5(x)  VTSS_ENCODE_BITFIELD(x,8,2)
#define  VTSS_M_MACRO_CTRL_ANEG_CFG_ANEG_CFG_PD_TIMER_2G5     VTSS_ENCODE_BITMASK(8,2)
#define  VTSS_X_MACRO_CTRL_ANEG_CFG_ANEG_CFG_PD_TIMER_2G5(x)  VTSS_EXTRACT_BITFIELD(x,8,2)

/** 
 * \brief
 * Restart negotiation process
 *
 * \details 
 * 1: Restart
 *
 * Field: ::VTSS_MACRO_CTRL_ANEG_CFG_ANEG_CFG . RESTART_ANEG_ONE_SHOT
 */
#define  VTSS_F_MACRO_CTRL_ANEG_CFG_ANEG_CFG_RESTART_ANEG_ONE_SHOT  VTSS_BIT(1)

/** 
 * \brief
 * Auto-negotiation enable
 *
 * \details 
 * 1: Enable
 * 0: Disable
 *
 * Field: ::VTSS_MACRO_CTRL_ANEG_CFG_ANEG_CFG . ANEG_ENA
 */
#define  VTSS_F_MACRO_CTRL_ANEG_CFG_ANEG_CFG_ANEG_ENA  VTSS_BIT(0)


/** 
 * \brief ANEG Advertised Ability 0
 *
 * \details
 * 48 bits that contain the advertised abilities link code word for
 * auto-negotiation (here: lower 32 bit).
 *
 * Register: \a MACRO_CTRL:ANEG_CFG:ANEG_ADV_ABILITY_0
 *
 * @param gi Replicator: x_ANEG_INST (??), 0-15
 */
#define VTSS_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0(gi)  VTSS_IOREG_IX(VTSS_TO_HSIO,0x30,gi,5,0,1)

/** 
 * \brief
 * Reserved for future technology as defined in IEEE 802.3ap clause 73.
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0 . ADV_ABIL_LSB
 */
#define  VTSS_F_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0_ADV_ABIL_LSB(x)  VTSS_ENCODE_BITFIELD(x,24,8)
#define  VTSS_M_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0_ADV_ABIL_LSB     VTSS_ENCODE_BITMASK(24,8)
#define  VTSS_X_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0_ADV_ABIL_LSB(x)  VTSS_EXTRACT_BITFIELD(x,24,8)

/** 
 * \brief
 * Technology Ability to be advertised (here: 10GBase-KR)
 *
 * \details 
 * 0: Do not advertise 10GB-KR capability
 * 1: Advertise 10GB-KR capability
 *
 * Field: ::VTSS_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0 . CAP_10GKR
 */
#define  VTSS_F_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_10GKR  VTSS_BIT(23)

/** 
 * \brief
 * Technology Ability to be advertised (here: 10GBase-KX4)
 *
 * \details 
 * 0: Do not advertise 10GB-KX4 capability
 * 1: Advertise 10GB-KX4 capability
 *
 * Field: ::VTSS_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0 . CAP_10GKX4
 */
#define  VTSS_F_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_10GKX4  VTSS_BIT(22)

/** 
 * \brief
 * Technology Ability to be advertised (here: 1000Base-KX)
 *
 * \details 
 * 0: Do not advertise 1GB-KX capability
 * 1: Advertise 1GB-KX capability
 *
 * Field: ::VTSS_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0 . CAP_1GKX
 */
#define  VTSS_F_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_1GKX  VTSS_BIT(21)

/** 
 * \brief
 * Initial value for Transmit-Nonce field
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0 . TX_NONCE
 */
#define  VTSS_F_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0_TX_NONCE(x)  VTSS_ENCODE_BITFIELD(x,16,5)
#define  VTSS_M_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0_TX_NONCE     VTSS_ENCODE_BITMASK(16,5)
#define  VTSS_X_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0_TX_NONCE(x)  VTSS_EXTRACT_BITFIELD(x,16,5)

/** 
 * \brief
 * Next page exchange desired
 *
 * \details 
 * 0: Disable NP exchange
 * 1: Enable NP exchange
 *
 * Field: ::VTSS_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0 . NP
 */
#define  VTSS_F_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0_NP  VTSS_BIT(15)

/** 
 * \brief
 * Acknowledge bit (this bit is automatically overwritten by ANEG)
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0 . ACKN
 */
#define  VTSS_F_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0_ACKN  VTSS_BIT(14)

/** 
 * \brief
 * RF bit (initial value)
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0 . RF
 */
#define  VTSS_F_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0_RF  VTSS_BIT(13)

/** 
 * \brief
 * Pause field
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0 . PAUSE
 */
#define  VTSS_F_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0_PAUSE(x)  VTSS_ENCODE_BITFIELD(x,10,3)
#define  VTSS_M_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0_PAUSE     VTSS_ENCODE_BITMASK(10,3)
#define  VTSS_X_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0_PAUSE(x)  VTSS_EXTRACT_BITFIELD(x,10,3)

/** 
 * \brief
 * Reserved for echoed nonce field (must be cleared)
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0 . ECHOED_NONCE
 */
#define  VTSS_F_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0_ECHOED_NONCE(x)  VTSS_ENCODE_BITFIELD(x,5,5)
#define  VTSS_M_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0_ECHOED_NONCE     VTSS_ENCODE_BITMASK(5,5)
#define  VTSS_X_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0_ECHOED_NONCE(x)  VTSS_EXTRACT_BITFIELD(x,5,5)

/** 
 * \brief
 * Selector field (must be 0x1)
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0 . SEL_FIELD
 */
#define  VTSS_F_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0_SEL_FIELD(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0_SEL_FIELD     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_0_SEL_FIELD(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief ANEG Advertised Ability 1
 *
 * \details
 * 48 bits that contain the advertised abilities link code word for
 * auto-negotiation (here: upper 16 bit).
 *
 * Register: \a MACRO_CTRL:ANEG_CFG:ANEG_ADV_ABILITY_1
 *
 * @param gi Replicator: x_ANEG_INST (??), 0-15
 */
#define VTSS_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_1(gi)  VTSS_IOREG_IX(VTSS_TO_HSIO,0x30,gi,5,0,2)

/** 
 * \brief
 * FEC capability (bit 14: FEC ability, bit 15: FEC requested) - Only used
 * with 10GBase-KR
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_1 . FEC
 */
#define  VTSS_F_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_1_FEC(x)  VTSS_ENCODE_BITFIELD(x,14,2)
#define  VTSS_M_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_1_FEC     VTSS_ENCODE_BITMASK(14,2)
#define  VTSS_X_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_1_FEC(x)  VTSS_EXTRACT_BITFIELD(x,14,2)

/** 
 * \brief
 * Reserved for future technology as defined in IEEE 802.3ap clause 73.
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_1 . ADV_ABIL_MSB
 */
#define  VTSS_F_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_1_ADV_ABIL_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,14)
#define  VTSS_M_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_1_ADV_ABIL_MSB     VTSS_ENCODE_BITMASK(0,14)
#define  VTSS_X_MACRO_CTRL_ANEG_CFG_ANEG_ADV_ABILITY_1_ADV_ABIL_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,14)


/** 
 * \brief ANEG Next Page 0
 *
 * \details
 * 48 bits that contain the new next page to transmit during
 * auto-negotiation (here: lower 32 bits).
 *
 * Register: \a MACRO_CTRL:ANEG_CFG:ANEG_NEXT_PAGE_0
 *
 * @param gi Replicator: x_ANEG_INST (??), 0-15
 */
#define VTSS_MACRO_CTRL_ANEG_CFG_ANEG_NEXT_PAGE_0(gi)  VTSS_IOREG_IX(VTSS_TO_HSIO,0x30,gi,5,0,3)


/** 
 * \brief ANEG Next Page 1
 *
 * \details
 * 48 bits that contain the new next page to transmit during
 * auto-negotiation (here: upper 16 bits).
 *
 * Register: \a MACRO_CTRL:ANEG_CFG:ANEG_NEXT_PAGE_1
 *
 * @param gi Replicator: x_ANEG_INST (??), 0-15
 */
#define VTSS_MACRO_CTRL_ANEG_CFG_ANEG_NEXT_PAGE_1(gi)  VTSS_IOREG_IX(VTSS_TO_HSIO,0x30,gi,5,0,4)

/** 
 * \brief
 * Must be set when a new next page is programmed (self-clearing)
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_ANEG_CFG_ANEG_NEXT_PAGE_1 . NEXT_PAGE_LOADED_ONE_SHOT
 */
#define  VTSS_F_MACRO_CTRL_ANEG_CFG_ANEG_NEXT_PAGE_1_NEXT_PAGE_LOADED_ONE_SHOT  VTSS_BIT(31)

/** 
 * \brief
 * Upper 16 bits of next page link code word
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_ANEG_CFG_ANEG_NEXT_PAGE_1 . NP_TX_MSB
 */
#define  VTSS_F_MACRO_CTRL_ANEG_CFG_ANEG_NEXT_PAGE_1_NP_TX_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_MACRO_CTRL_ANEG_CFG_ANEG_NEXT_PAGE_1_NP_TX_MSB     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_MACRO_CTRL_ANEG_CFG_ANEG_NEXT_PAGE_1_NP_TX_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a MACRO_CTRL:ANEG_STATUS
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
 * Register: \a MACRO_CTRL:ANEG_STATUS:ANEG_LP_ADV_ABILITY_0
 *
 * @param gi Replicator: x_ANEG_INST (??), 0-15
 */
#define VTSS_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0(gi)  VTSS_IOREG_IX(VTSS_TO_HSIO,0x80,gi,3,0,0)

/** 
 * \brief
 * Bits 31 down to 24 of link code word received from link partner.
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . LP_ADV_ABIL_LSB
 */
#define  VTSS_F_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_LP_ADV_ABIL_LSB(x)  VTSS_ENCODE_BITFIELD(x,24,8)
#define  VTSS_M_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_LP_ADV_ABIL_LSB     VTSS_ENCODE_BITMASK(24,8)
#define  VTSS_X_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_LP_ADV_ABIL_LSB(x)  VTSS_EXTRACT_BITFIELD(x,24,8)

/** 
 * \brief
 * Technology Ability advertised by LP (here: 10GBase-KR)
 *
 * \details 
 * 0: LP is not 10GB-KR capable
 * 1: LP is 10GB-KR capable
 *
 * Field: ::VTSS_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . CAP_10GKR
 */
#define  VTSS_F_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_CAP_10GKR  VTSS_BIT(23)

/** 
 * \brief
 * Technology Ability advertised by LP (here: 10GBase-KX4)
 *
 * \details 
 * 0: LP is not 10GB-KX4 capable
 * 1: LP is 10GB-KX4 capable
 *
 * Field: ::VTSS_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . CAP_10GKX4
 */
#define  VTSS_F_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_CAP_10GKX4  VTSS_BIT(22)

/** 
 * \brief
 * Technology Ability advertised by LP (here: 1000Base-KX)
 *
 * \details 
 * 0: LP is not 1GB-KX capable
 * 1: LP is 1GB-KX capable
 *
 * Field: ::VTSS_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . CAP_1GKX
 */
#define  VTSS_F_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_CAP_1GKX  VTSS_BIT(21)

/** 
 * \brief
 * Transmit-Nonce field (received from LinkPartner)
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . TX_NONCE
 */
#define  VTSS_F_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_TX_NONCE(x)  VTSS_ENCODE_BITFIELD(x,16,5)
#define  VTSS_M_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_TX_NONCE     VTSS_ENCODE_BITMASK(16,5)
#define  VTSS_X_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_TX_NONCE(x)  VTSS_EXTRACT_BITFIELD(x,16,5)

/** 
 * \brief
 * Next page exchange desired by LP
 *
 * \details 
 * 0: No NP exchange desired
 * 1: NP exchange desired
 *
 * Field: ::VTSS_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . NP
 */
#define  VTSS_F_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_NP  VTSS_BIT(15)

/** 
 * \brief
 * Acknowledge bit (this bit is automatically overwritten by ANEG)
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . ACKN
 */
#define  VTSS_F_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_ACKN  VTSS_BIT(14)

/** 
 * \brief
 * RF bit
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . RF
 */
#define  VTSS_F_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_RF  VTSS_BIT(13)

/** 
 * \brief
 * Pause field
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . PAUSE
 */
#define  VTSS_F_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_PAUSE(x)  VTSS_ENCODE_BITFIELD(x,10,3)
#define  VTSS_M_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_PAUSE     VTSS_ENCODE_BITMASK(10,3)
#define  VTSS_X_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_PAUSE(x)  VTSS_EXTRACT_BITFIELD(x,10,3)

/** 
 * \brief
 * Echoed nonce field
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . ECHOED_NONCE
 */
#define  VTSS_F_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_ECHOED_NONCE(x)  VTSS_ENCODE_BITFIELD(x,5,5)
#define  VTSS_M_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_ECHOED_NONCE     VTSS_ENCODE_BITMASK(5,5)
#define  VTSS_X_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_ECHOED_NONCE(x)  VTSS_EXTRACT_BITFIELD(x,5,5)

/** 
 * \brief
 * Selector field
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0 . SEL_FIELD
 */
#define  VTSS_F_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_SEL_FIELD(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_SEL_FIELD     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_0_SEL_FIELD(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


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
 * Register: \a MACRO_CTRL:ANEG_STATUS:ANEG_LP_ADV_ABILITY_1
 *
 * @param gi Replicator: x_ANEG_INST (??), 0-15
 */
#define VTSS_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1(gi)  VTSS_IOREG_IX(VTSS_TO_HSIO,0x80,gi,3,0,1)

/** 
 * \brief
 * FEC capability (bit 14: FEC ability, bit 15: FEC requested) - Only used
 * with 10GBase-KR
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1 . FEC
 */
#define  VTSS_F_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1_FEC(x)  VTSS_ENCODE_BITFIELD(x,14,2)
#define  VTSS_M_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1_FEC     VTSS_ENCODE_BITMASK(14,2)
#define  VTSS_X_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1_FEC(x)  VTSS_EXTRACT_BITFIELD(x,14,2)

/** 
 * \brief
 * Bits 45 down to 32 of link code word received from link partner.
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1 . LP_ADV_ABIL_MSB
 */
#define  VTSS_F_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1_LP_ADV_ABIL_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,14)
#define  VTSS_M_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1_LP_ADV_ABIL_MSB     VTSS_ENCODE_BITMASK(0,14)
#define  VTSS_X_MACRO_CTRL_ANEG_STATUS_ANEG_LP_ADV_ABILITY_1_LP_ADV_ABIL_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,14)


/** 
 * \brief ANEG Status
 *
 * \details
 * Auto negotiation status register
 *
 * Register: \a MACRO_CTRL:ANEG_STATUS:ANEG_STATUS
 *
 * @param gi Replicator: x_ANEG_INST (??), 0-15
 */
#define VTSS_MACRO_CTRL_ANEG_STATUS_ANEG_STATUS(gi)  VTSS_IOREG_IX(VTSS_TO_HSIO,0x80,gi,3,0,2)

/** 
 * \brief
 * Error condition indicating that no compatible link was found.
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_ANEG_STATUS_ANEG_STATUS . INCOMPATIBLE_LINK
 */
#define  VTSS_F_MACRO_CTRL_ANEG_STATUS_ANEG_STATUS_INCOMPATIBLE_LINK  VTSS_BIT(17)

/** 
 * \brief
 * Error condition indicating errors during parallel detection.
 *
 * \details 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_MACRO_CTRL_ANEG_STATUS_ANEG_STATUS . PAR_DETECT_FAULT_STICKY
 */
#define  VTSS_F_MACRO_CTRL_ANEG_STATUS_ANEG_STATUS_PAR_DETECT_FAULT_STICKY  VTSS_BIT(16)

/** 
 * \brief
 * Status indicating whether a new page has been received.
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_ANEG_STATUS_ANEG_STATUS . PAGE_RX
 */
#define  VTSS_F_MACRO_CTRL_ANEG_STATUS_ANEG_STATUS_PAGE_RX  VTSS_BIT(3)

/** 
 * \brief
 * Status indicating whether the link partner supports auto-negotiation.
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_ANEG_STATUS_ANEG_STATUS . LP_ANEG_ABLE
 */
#define  VTSS_F_MACRO_CTRL_ANEG_STATUS_ANEG_STATUS_LP_ANEG_ABLE  VTSS_BIT(1)

/** 
 * \brief
 * Status indicating whether auto-negotiation has completed.
 *
 * \details 
 * Field: ::VTSS_MACRO_CTRL_ANEG_STATUS_ANEG_STATUS . ANEG_COMPLETE
 */
#define  VTSS_F_MACRO_CTRL_ANEG_STATUS_ANEG_STATUS_ANEG_COMPLETE  VTSS_BIT(0)


#endif /* _VTSS_JAGUAR_REGS_MACRO_CTRL_H_ */
