#ifndef _VTSS_JAGUAR_REGS_DEV10G_H_
#define _VTSS_JAGUAR_REGS_DEV10G_H_

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
 * Target: \a DEV10G
 *
 * \see vtss_target_DEV10G_e
 *
 * VAUI @ 12Gbps 2 lanes, XAUI @ 10 Gbps, XAUI @ 12 Gbps, VAUI @ 10Gbps 2
 * lanes and SGMII @ 10/100/1000/2500 Mbps
 *
 ***********************************************************************/

/**
 * Register Group: \a DEV10G:PCS1G_CFG_STATUS
 *
 * PCS 1G Configuration Status Registers
 */


/** 
 * \brief PCS1G Configuration
 *
 * \details
 * PCS1G main configuration register
 *
 * Register: \a DEV10G:PCS1G_CFG_STATUS:PCS1G_CFG
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_CFG(target)  VTSS_IOREG(target,0x0)

/** 
 * \brief
 * Set type of link_status indication at CPU-System
 *
 * \details 
 * 0: Sync_status (from PCS synchronization state machine)
 * 1: Bit 15 of PCS1G_ANEG_STATUS.lp_adv_ability (Link up/down)
 *
 * Field: ::VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_CFG . LINK_STATUS_TYPE
 */
#define  VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_CFG_LINK_STATUS_TYPE  VTSS_BIT(4)

/** 
 * \brief
 * PCS enable
 *
 * \details 
 * 0: Disable PCS
 * 1: Enable PCS
 *
 * Field: ::VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_CFG . PCS_ENA
 */
#define  VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_CFG_PCS_ENA  VTSS_BIT(0)


/** 
 * \brief PCS1G Mode Configuration
 *
 * \details
 * PCS1G mode configuration
 *
 * Register: \a DEV10G:PCS1G_CFG_STATUS:PCS1G_MODE_CFG
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_MODE_CFG(target)  VTSS_IOREG(target,0x1)

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
 * Field: ::VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_MODE_CFG . UNIDIR_MODE_ENA
 */
#define  VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_MODE_CFG_UNIDIR_MODE_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Selection of PCS operation
 *
 * \details 
 * 0: PCS is used in SERDES mode
 * 1: PCS is used in SGMII mode. Configuration bit
 * PCS1G_ANEG_CFG.SW_RESOLVE_ENA must be set additionally
 *
 * Field: ::VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_MODE_CFG . SGMII_MODE_ENA
 */
#define  VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_MODE_CFG_SGMII_MODE_ENA  VTSS_BIT(0)


/** 
 * \brief PCS1G Signal Detect Configuration
 *
 * \details
 * PCS1G signal_detect configuration
 *
 * Register: \a DEV10G:PCS1G_CFG_STATUS:PCS1G_SD_CFG
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_SD_CFG(target)  VTSS_IOREG(target,0x2)

/** 
 * \brief
 * Signal detect selection (select input for internal signal_detect line)
 *
 * \details 
 * 0: Select signal_detect line from hardmacro
 * 1: Select external signal_detect line
 *
 * Field: ::VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_SD_CFG . SD_SEL
 */
#define  VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_SD_CFG_SD_SEL  VTSS_BIT(8)

/** 
 * \brief
 * Signal detect polarity: The signal level on signal_detect input pin must
 * be equal to SD_POL to indicate signal detection (SD_ENA must be set)
 *
 * \details 
 * 0: Signal Detect input pin must be '0' to indicate a signal detection
 * 1: Signal Detect input pin must be '1' to indicate a signal detection
 *
 * Field: ::VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_SD_CFG . SD_POL
 */
#define  VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_SD_CFG_SD_POL  VTSS_BIT(4)

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
 * Field: ::VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_SD_CFG . SD_ENA
 */
#define  VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_SD_CFG_SD_ENA  VTSS_BIT(0)


/** 
 * \brief PCS1G Aneg Configuration
 *
 * \details
 * PCS1G Auto-negotiation configuration register
 *
 * Register: \a DEV10G:PCS1G_CFG_STATUS:PCS1G_ANEG_CFG
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG(target)  VTSS_IOREG(target,0x3)

/** 
 * \brief
 * Advertised Ability Register: Holds the capabilities of the device as
 * described IEEE 802.3, Clause 37. If SGMII mode is selected
 * (PCS1G_MODE_CFG.SGMII_MODE_ENA = 1), SW_RESOLVE_ENA must be set.
 *
 * \details 
 * Field: ::VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG . ADV_ABILITY
 */
#define  VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ADV_ABILITY(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ADV_ABILITY     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ADV_ABILITY(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

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
 * Field: ::VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG . SW_RESOLVE_ENA
 */
#define  VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_SW_RESOLVE_ENA  VTSS_BIT(8)

/** 
 * \brief
 * Auto Negotiation Restart
 *
 * \details 
 * 0: No action
 * 1: Restart Auto Negotiation
 *
 * Field: ::VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG . ANEG_RESTART_ONE_SHOT
 */
#define  VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ANEG_RESTART_ONE_SHOT  VTSS_BIT(1)

/** 
 * \brief
 * Auto Negotiation Enable
 *
 * \details 
 * 0: Auto Negotiation Disabled
 * 1: Auto Negotiation Enabled
 *
 * Field: ::VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG . ANEG_ENA
 */
#define  VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ANEG_ENA  VTSS_BIT(0)


/** 
 * \brief PCS1G Aneg Next Page Configuration
 *
 * \details
 * PCS1G Auto-negotiation configuration register for next-page function
 *
 * Register: \a DEV10G:PCS1G_CFG_STATUS:PCS1G_ANEG_NP_CFG
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_NP_CFG(target)  VTSS_IOREG(target,0x4)

/** 
 * \brief
 * Next page register: Holds the next-page information as described in IEEE
 * 802.3, Clause 37
 *
 * \details 
 * Field: ::VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_NP_CFG . NP_TX
 */
#define  VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_NP_CFG_NP_TX(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_NP_CFG_NP_TX     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_NP_CFG_NP_TX(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Next page loaded
 *
 * \details 
 * 0: next page is free and can be loaded
 * 1: next page register has been filled (to be set after np_tx has been
 * filled)
 *
 * Field: ::VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_NP_CFG . NP_LOADED_ONE_SHOT
 */
#define  VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_NP_CFG_NP_LOADED_ONE_SHOT  VTSS_BIT(0)


/** 
 * \brief PCS1G Loopback Configuration
 *
 * \details
 * PCS1G Loop-Back configuration register
 *
 * Register: \a DEV10G:PCS1G_CFG_STATUS:PCS1G_LB_CFG
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_LB_CFG(target)  VTSS_IOREG(target,0x5)

/** 
 * \brief
 * Loops data in PCS (TBI side) from egress direction to ingress direction.
 * The Rx clock is automatically set equal to the Tx clock
 *
 * \details 
 * 0: TBI Loopback Disabled
 * 1:TBI Loopback Enabled
 *
 * Field: ::VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_LB_CFG . TBI_HOST_LB_ENA
 */
#define  VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_LB_CFG_TBI_HOST_LB_ENA  VTSS_BIT(0)


/** 
 * \brief PCS1G ANEG Status Register
 *
 * \details
 * PCS1G Auto-negotiation status register
 *
 * Register: \a DEV10G:PCS1G_CFG_STATUS:PCS1G_ANEG_STATUS
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS(target)  VTSS_IOREG(target,0x8)

/** 
 * \brief
 * Advertised abilities from link partner as described in IEEE 802.3,
 * Clause 37
 *
 * \details 
 * Field: ::VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS . LP_ADV_ABILITY
 */
#define  VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS_LP_ADV_ABILITY(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS_LP_ADV_ABILITY     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS_LP_ADV_ABILITY(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Resolve priority
 *
 * \details 
 * 0: ANEG is in progress
 * 1: ANEG nearly finished - priority can be resolved (via software)
 *
 * Field: ::VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS . PR
 */
#define  VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS_PR  VTSS_BIT(4)

/** 
 * \brief
 * Status indicating whether a new page has been received.
 *
 * \details 
 * 0: No new page received
 * 1: New page received
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS . PAGE_RX_STICKY
 */
#define  VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS_PAGE_RX_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * Auto Negotiation Complete
 *
 * \details 
 * 0: No Auto Negotiation has been completed
 * 1: Indicates that an Auto Negotiation has completed successfully
 *
 * Field: ::VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS . ANEG_COMPLETE
 */
#define  VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS_ANEG_COMPLETE  VTSS_BIT(0)


/** 
 * \brief PCS1G Aneg Next Page Status Register
 *
 * \details
 * PCS1G Auto-negotiation next page status register
 *
 * Register: \a DEV10G:PCS1G_CFG_STATUS:PCS1G_ANEG_NP_STATUS
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_NP_STATUS(target)  VTSS_IOREG(target,0x9)

/** 
 * \brief
 * Next page ability register from link partner as described in IEEE 802.3,
 * Clause 37
 *
 * \details 
 * Field: ::VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_NP_STATUS . LP_NP_RX
 */
#define  VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_NP_STATUS_LP_NP_RX(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_NP_STATUS_LP_NP_RX     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_DEV10G_PCS1G_CFG_STATUS_PCS1G_ANEG_NP_STATUS_LP_NP_RX(x)  VTSS_EXTRACT_BITFIELD(x,16,16)


/** 
 * \brief PCS1G link status
 *
 * \details
 * PCS1G link status register
 *
 * Register: \a DEV10G:PCS1G_CFG_STATUS:PCS1G_LINK_STATUS
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS(target)  VTSS_IOREG(target,0xa)

/** 
 * \brief
 * Indicates whether or not the selected Signal Detect input line is
 * asserted
 *
 * \details 
 * 0: No signal detected
 * 1: Signal detected
 *
 * Field: ::VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS . SIGNAL_DETECT
 */
#define  VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS_SIGNAL_DETECT  VTSS_BIT(8)

/** 
 * \brief
 * Indicates whether the link is up or down. A link is up when ANEG state
 * machine is in state LINK_OK or AN_DISABLE_LINK_OK
 *
 * \details 
 * 0: Link down
 * 1: Link up
 *
 * Field: ::VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS . LINK_STATUS
 */
#define  VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS_LINK_STATUS  VTSS_BIT(4)

/** 
 * \brief
 * Indicates if PCS has successfully synchronized
 *
 * \details 
 * 0: PCS is out of sync
 * 1: PCS has synchronized
 *
 * Field: ::VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS . SYNC_STATUS
 */
#define  VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS_SYNC_STATUS  VTSS_BIT(0)


/** 
 * \brief PCS1G link down counter
 *
 * \details
 * PCS1G link down counter register
 *
 * Register: \a DEV10G:PCS1G_CFG_STATUS:PCS1G_LINK_DOWN_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_LINK_DOWN_CNT(target)  VTSS_IOREG(target,0xb)

/** 
 * \brief
 * Link Down Counter. A counter that counts the number of times a link has
 * been down. The counter does not saturate at 255 and is only cleared when
 * writing 0 to the register
 *
 * \details 
 * Field: ::VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_LINK_DOWN_CNT . LINK_DOWN_CNT
 */
#define  VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_LINK_DOWN_CNT_LINK_DOWN_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_DEV10G_PCS1G_CFG_STATUS_PCS1G_LINK_DOWN_CNT_LINK_DOWN_CNT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_DEV10G_PCS1G_CFG_STATUS_PCS1G_LINK_DOWN_CNT_LINK_DOWN_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief PCS1G sticky register
 *
 * \details
 * PCS1G status register for sticky bits
 *
 * Register: \a DEV10G:PCS1G_CFG_STATUS:PCS1G_STICKY
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_STICKY(target)  VTSS_IOREG(target,0xc)

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
 * Field: ::VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_STICKY . LINK_DOWN_STICKY
 */
#define  VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_STICKY_LINK_DOWN_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Sticky bit indicating if PCS synchronization has been lost
 *
 * \details 
 * 0: Synchronization has not been lost at any time
 * 1: Synchronization has been lost for one or more clock cycles
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_DEV10G_PCS1G_CFG_STATUS_PCS1G_STICKY . OUT_OF_SYNC_STICKY
 */
#define  VTSS_F_DEV10G_PCS1G_CFG_STATUS_PCS1G_STICKY_OUT_OF_SYNC_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a DEV10G:PCS1G_TSTPAT_CFG_STATUS
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
 * Register: \a DEV10G:PCS1G_TSTPAT_CFG_STATUS:PCS1G_TSTPAT_MODE_CFG
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_MODE_CFG(target)  VTSS_IOREG(target,0x10)

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
 * Field: ::VTSS_DEV10G_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_MODE_CFG . JTP_SEL
 */
#define  VTSS_F_DEV10G_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_MODE_CFG_JTP_SEL(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_DEV10G_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_MODE_CFG_JTP_SEL     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_DEV10G_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_MODE_CFG_JTP_SEL(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief PCS1G TSTPAT STATUS
 *
 * \details
 * PCS1G testpattern status register
 *
 * Register: \a DEV10G:PCS1G_TSTPAT_CFG_STATUS:PCS1G_TSTPAT_STATUS
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_STATUS(target)  VTSS_IOREG(target,0x11)

/** 
 * \brief
 * Jitter Test Pattern Error Counter. Due to re-sync measures it might
 * happen that single errors are not counted (applies for 2.5gpbs mode).
 * The counter saturates at 255 and is only cleared when writing 0 to the
 * register
 *
 * \details 
 * Field: ::VTSS_DEV10G_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_STATUS . JTP_ERR_CNT
 */
#define  VTSS_F_DEV10G_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_STATUS_JTP_ERR_CNT(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_DEV10G_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_STATUS_JTP_ERR_CNT     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_DEV10G_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_STATUS_JTP_ERR_CNT(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Jitter Test Pattern Error
 *
 * \details 
 * 0: Jitter pattern checker has found no error
 * 1: Jitter pattern checker has found an error
 *
 * Field: ::VTSS_DEV10G_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_STATUS . JTP_ERR
 */
#define  VTSS_F_DEV10G_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_STATUS_JTP_ERR  VTSS_BIT(4)

/** 
 * \brief
 * Jitter Test Pattern Lock
 *
 * \details 
 * 0: Jitter pattern checker has not locked
 * 1: Jitter pattern checker has locked
 *
 * Field: ::VTSS_DEV10G_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_STATUS . JTP_LOCK
 */
#define  VTSS_F_DEV10G_PCS1G_TSTPAT_CFG_STATUS_PCS1G_TSTPAT_STATUS_JTP_LOCK  VTSS_BIT(0)

/**
 * Register Group: \a DEV10G:MAC_CFG_STATUS
 *
 * MAC10G Configuration and Status Registers
 */


/** 
 * \brief Mode Configuration Register
 *
 * \details
 * Register: \a DEV10G:MAC_CFG_STATUS:MAC_ENA_CFG
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_MAC_CFG_STATUS_MAC_ENA_CFG(target)  VTSS_IOREG(target,0x12)

/** 
 * \brief
 * Enable Receiver.
 *
 * \details 
 * '0': Disabled
 * '1': Enabled.
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_ENA_CFG . RX_ENA
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_ENA_CFG_RX_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Enable Transmitter.
 *
 * \details 
 * '0': Disabled
 * '1': Enabled.
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_ENA_CFG . TX_ENA
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_ENA_CFG_TX_ENA  VTSS_BIT(0)


/** 
 * \brief Mode Configuration Register
 *
 * \details
 * Register: \a DEV10G:MAC_CFG_STATUS:MAC_MODE_CFG
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_MAC_CFG_STATUS_MAC_MODE_CFG(target)  VTSS_IOREG(target,0x13)

/** 
 * \brief
 * On a given link, Pause Frames can be either "Link Pause frames" or
 * "Tunnel Pause Frames".
 * 
 * "Link Pause Frames" will have standard preamble, while "Tunnel Pause
 * Frames" will have HIH in the preamble.
 * 
 * For "Link Pause Frames" MAC10G should not verify HIH CKSM, while for
 * "Tunnel Pause Frames" MAC10G should verify HIH CKSM.

 *
 * \details 
 * 0: Pause frames are expected to be Link Pause frames - No HIH CKSM
 * verification in MAC10G
 * 1: Pause frames are expected to be Tunnel Pause frames - HIH CKSM is
 * verified in MAC10G
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_MODE_CFG . TUNNEL_PAUSE_FRAMES
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_MODE_CFG_TUNNEL_PAUSE_FRAMES  VTSS_BIT(16)

/** 
 * \brief
 * This register configures the format of the preamble.
 * This supports:
 * 1) Standard preamble = 0xFB555555555555D5
 * 2) Minimum preamble = 0xFB
 * 3) Host Interface Header (HIH) located before the SFD
 * 4) Host Interface Header (HIH) located after the SFD - default CRC
 * 5) Host Interface Header (HIH) located after the SFD - HIH included in
 * CRC
 *
 * \details 
 * 0: Standard Preamble
 * 1: Minimum Preamble = 0xFB
 * 2: HIH before SFD -	default CRC = 0xFB H0 H1 H2 H3 0x55 0x55 0xD5
 * 3: HIH after SFD - default CRC = 0xFB 0x55 0x55 0xD5 H0 H1 H2 H3
 * 4: HIH after SFD - HIH in CRC = 0xFB 0x55 0x55 0xD5 H0 H1 H2 H3
 * 5 - 7: reserved
 * 
 * HIH is 4 bytes: H0 H1 H2 H3
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_MODE_CFG . MAC_PREAMBLE_CFG
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_MODE_CFG_MAC_PREAMBLE_CFG(x)  VTSS_ENCODE_BITFIELD(x,12,3)
#define  VTSS_M_DEV10G_MAC_CFG_STATUS_MAC_MODE_CFG_MAC_PREAMBLE_CFG     VTSS_ENCODE_BITMASK(12,3)
#define  VTSS_X_DEV10G_MAC_CFG_STATUS_MAC_MODE_CFG_MAC_PREAMBLE_CFG(x)  VTSS_EXTRACT_BITFIELD(x,12,3)

/** 
 * \brief
 * Register configures the format of the IPG.
 * 1) Normal IPG : Averaging IPG for average of 12 bytes
 * 2) Shrinked IPG : Minimum IPG of 4 - 7 bytes (Incl. Terminate Character
 * and IDLE BYTEs)
 *
 * \details 
 * '0': Normal IPG.
 * '1': Shrink IPG.
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_MODE_CFG . MAC_IPG_CFG
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_MODE_CFG_MAC_IPG_CFG  VTSS_BIT(8)

/** 
 * \brief
 * When asserted (=1) MAC10G must check HIH checksum and discard frames if
 * HIH checksum is not valid.
 * When de-asserted (=0) MAC10G must disregard the HIH checksum.
 *
 * \details 
 * = 0 ; MAC10G should disregard the HIH CKSM.
 * = 1 ; MAC10G should discard any frame with invalid CKSM
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_MODE_CFG . HIH_CRC_CHECK
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_MODE_CFG_HIH_CRC_CHECK  VTSS_BIT(0)


/** 
 * \brief Max Length Configuration Register
 *
 * \details
 * Register: \a DEV10G:MAC_CFG_STATUS:MAC_MAXLEN_CFG
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_MAC_CFG_STATUS_MAC_MAXLEN_CFG(target)  VTSS_IOREG(target,0x14)

/** 
 * \brief
 * Configures whether the Max Length Check takes the number of Q tags into
 * consideration when assing if a frame is too long.
 * 
 * If asserted, 
 *  - 4 bytes will be added to MAX_LEN for single tagged frame.
 *  - 8 bytes will be added for MAX_LEN double tagged frame.
 *
 * \details 
 * '0' : Only check max frame length against MAX_LEN
 * '1' : Add 4/8 bytes to MAX_LEN when checking for max frame length
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_MAXLEN_CFG . MAX_LEN_TAG_CHK
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_MAXLEN_CFG_MAX_LEN_TAG_CHK  VTSS_BIT(16)

/** 
 * \brief
 * The maximum frame length accepted by the Receive Module. If the length
 * is exceeded, this is indicated in the Statistics Engine (LONG_FRAME).
 * The maximum length is automatically adjusted to accommodate maximum
 * sized frames containing a VLAN tag - given that the MAC is configured to
 * be VLAN aware (default):
 * 
 * MTU size = 10056 Bytes. This includes all encapsulations and TAGs. Not
 * including IFH.
 *  
 * Reason is: QS supports a max of 63 segments. A segment is 160 Bytes. The
 * IFH must be stored in the QS also, so room must be allocated.
 * Thereby:
 * 63 x 160B - 24B (IFH) = 10056 Bytes
 *
 * \details 
 * The maximum allowable size is 10056 Bytes.
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_MAXLEN_CFG . MAX_LEN
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_MAXLEN_CFG_MAX_LEN(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_DEV10G_MAC_CFG_STATUS_MAC_MAXLEN_CFG_MAX_LEN     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_DEV10G_MAC_CFG_STATUS_MAC_MAXLEN_CFG_MAX_LEN(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief VLAN / Service tag configuration register
 *
 * \details
 * Register: \a DEV10G:MAC_CFG_STATUS:MAC_TAGS_CFG
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_MAC_CFG_STATUS_MAC_TAGS_CFG(target)  VTSS_IOREG(target,0x15)

/** 
 * \brief
 * This field defines which value is regarded as a VLAN/Service tag -
 * besides 0x8100. The value is used for all ALL tag positions. I.e. a
 * double tagged frame can have the following tag values:
 * (INNER_TAG, OUTER_TAG):
 * ( 0x8100, 0x8100 )
 * ( 0x8100, TAG_ID ) or
 * ( TAG_ID, TAG_ID )
 * 
 * Single tagged frame can have the following tag values - regardless of
 * the DOUBLE_TAG_ENA setting: 
 * 0x8100 or TAG_ID. TAG_ID should therefore normally be set to 0x8100 if
 * DOUBLE_TAG_ENA = '0'.
 *
 * \details 
 * "0x8100" - Standard Ethernet Bridge ethertype (C-tag)
 * "0x88A8" - Provider Bridge Ethertype  (S-tag)
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_TAGS_CFG . TAG_ID
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_TAGS_CFG_TAG_ID(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_DEV10G_MAC_CFG_STATUS_MAC_TAGS_CFG_TAG_ID     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_DEV10G_MAC_CFG_STATUS_MAC_TAGS_CFG_TAG_ID(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Provider Bridge (double tag) Enable
 * 
 * VLAN_AWR_ENA must be set to '1' when double tag is enabled
 * (DOUBLE_TAG_ENA = '1')
 *
 * \details 
 * '0': The MAC looks only for a single tag (0x8100 or TAG_ID)
 * '1': The MAC looks for two tags according to encoding of TAG_ID
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_TAGS_CFG . DOUBLE_TAG_ENA
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_TAGS_CFG_DOUBLE_TAG_ENA  VTSS_BIT(4)

/** 
 * \brief
 * VLAN Aware: Enables the MAC to work in a VLAN aware environment:
 *
 * \details 
 * '0': VLAN Disabled
 * '1': VLAN Enabled.
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_TAGS_CFG . VLAN_AWR_ENA
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_TAGS_CFG_VLAN_AWR_ENA  VTSS_BIT(0)


/** 
 * \brief Advanced Check Feature Configuration Register
 *
 * \details
 * Register: \a DEV10G:MAC_CFG_STATUS:MAC_ADV_CHK_CFG
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG(target)  VTSS_IOREG(target,0x16)

/** 
 * \brief
 * Extended End Of Packet Check: Specifies the requirement for the Rx
 * column when holding an EOP character.
 *
 * \details 
 * '0': The values of the remaining Rx lanes of a column holding an EOP are
 * ignored. E.g. if lane 1 holds an EOP, the value of lanes 2 and 3 are
 * ignored
 * '1': A received frame is error-marked if an Error character is received
 * in any lane of the column holding the EOP character. E.g. if lane 1
 * holds an EOP, the frame is error-marked if lanes 0, 2, or 3 holds an
 * Error character.
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG . EXT_EOP_CHK_ENA
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_EXT_EOP_CHK_ENA  VTSS_BIT(24)

/** 
 * \brief
 * Extended Start Of Packet Check Enable: Specifies the requirement for the
 * Rx column prior to SOP character.
 *
 * \details 
 * '0': Value of Rx column at the XGMII interface prior to a SOP character
 * is ignored
 * '1': An IDLE column at the XGMII interface must be received prior to a
 * SOP character for the MAC to detect a start of frame.
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG . EXT_SOP_CHK_ENA
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_EXT_SOP_CHK_ENA  VTSS_BIT(20)

/** 
 * \brief
 * Start-of-Frame-Delimiter Check Enable: Specifies the requirement for a
 * successful frame reception.
 * When disabled (='0'), MAC10G will assume that preamble is 8 bytes (incl.
 * SOP & SFD) when SOP is received. No checking of SFD is carried out.
 * When enabled (='1'), MAC10G will search for SFD in lane 3/7 after
 * reception of SOP, before accepting frame data. MAC10G will search for
 * SFD until SFD is found or a control character is encountered.
 *
 * \details 
 * '0': Skip SFD check
 * '1': Strict SFD check enabled, i.e. the SFD must be "D5" for a
 * successful frame reception.
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG . SFD_CHK_ENA
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_SFD_CHK_ENA  VTSS_BIT(16)

/** 
 * \brief
 * Preamble Shrink Check Disable:
 * The MAC can be setup to discard a frame, if a SFD is received in lane 3
 * of the column containing the SOP. If stacking tags must be supported,
 * the MAC must accept any data byte at this position.
 *
 * \details 
 * '0': Frames with a SFD in lane 3 of the column containing the SOP are
 * discarded.
 * '1': Frames with a SFD in lane 3 of the column containing the SOP are
 * NOT discarded.
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG . PRM_SHK_CHK_DIS
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_PRM_SHK_CHK_DIS  VTSS_BIT(12)

/** 
 * \brief
 * Preamble Check Enable: Specifies the preamble requirement for a
 * successful frame reception.
 *
 * \details 
 * '0': Skip preamble check. A SOP control character is sufficient for a
 * successful frame reception. The minimum allowed preamble size is still 8
 * bytes (incl. SOP and SFD) but the preamble bytes between the SOP and SFD
 * can have any data value.
 * '1': Strict preamble check enabled, i.e. the last 6 bytes of a preamble
 * - prior to the SFD - must all be equal to 0x55 for a successful frame
 * reception. For preambles larger than 8 bytes, only the last 6 preamble
 * bytes prior to the SFD are checked when this bit is set to 1.
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG . PRM_CHK_ENA
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_PRM_CHK_ENA  VTSS_BIT(8)

/** 
 * \brief
 * Out-Of-Range Error Check Enable: Determines whether or not a received
 * frame should be discarded if the frame length field is out of range.
 *
 * \details 
 * '0': Out-of-range errors are ignored
 * '1': A frame is discarded if the frame length field value is out of
 * range
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG . OOR_ERR_ENA
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_OOR_ERR_ENA  VTSS_BIT(4)

/** 
 * \brief
 * In-Range Error Check Enable: Determines whether or not a received frame
 * should be discarded if the frame length does not match the frame PDU
 * size:
 *
 * \details 
 * '0':
 * Frames which have a frame length field inconsistent with the actual
 * frame length are not error-marked
 * '1': Frames with inconsistent frame length fields are error marked and
 * will be discarded by the Rx Queue System.
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG . INR_ERR_ENA
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_ADV_CHK_CFG_INR_ERR_ENA  VTSS_BIT(0)


/** 
 * \brief Link Fault Signaling Register
 *
 * \details
 * Register: \a DEV10G:MAC_CFG_STATUS:MAC_LFS_CFG
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_MAC_CFG_STATUS_MAC_LFS_CFG(target)  VTSS_IOREG(target,0x17)

/** 
 * \brief
 * LFS Unidirectional mode enable. Implementation of 802.3 clause 66. When
 * asserted, this enables MAC to transmit data during reception of Local
 * Fault and Remote Fault ordered sets from the PHY. 
 * 
 * When in Unidirectional mode:
 * When receiving LF, frames are transmitted separated by RF ordered sets.
 * When receiving RF, frames are transmitted separated by IDLE symbols
 *
 * \details 
 * '0' : LFS unidirectional mode is disabled
 * '1' : LFS unidirectional mode is enabled
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_LFS_CFG . LFS_UNIDIR_ENA
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_LFS_CFG_LFS_UNIDIR_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Link Fault Signaling Mode Enable: If enabled, the transmitter reacts on
 * received Link Fault indications.
 *
 * \details 
 * 
 * '0': Ignore Link Faults detected by the MAC receiver module
 * '1': React on detected Link Faults and transmit the appropriate Sequence
 * Ordered Set.
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_LFS_CFG . LFS_MODE_ENA
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_LFS_CFG_LFS_MODE_ENA  VTSS_BIT(0)


/** 
 * \brief Miscellaneous Configuration Register
 *
 * \details
 * Register: \a DEV10G:MAC_CFG_STATUS:MAC_LB_CFG
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_MAC_CFG_STATUS_MAC_LB_CFG(target)  VTSS_IOREG(target,0x18)

/** 
 * \brief
 * Enables loopback from egress to ingress in the device. The MAC Rx clock
 * is automatically set equal to the MAC Tx clock when the loop back is
 * enabled.
 *
 * \details 
 * '0': Host loopback disabled at XGMII interface.
 * '1': Host loopback enabled at XGMII interface.
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_LB_CFG . XGMII_HOST_LB_ENA
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_LB_CFG_XGMII_HOST_LB_ENA  VTSS_BIT(4)


/** 
 * \brief Sticky Bit Register
 *
 * \details
 * Clear the sticky bits by writing a '0' in the relevant bitgroups
 * (writing a '1' sets the bit)!.
 *
 * Register: \a DEV10G:MAC_CFG_STATUS:MAC_STICKY
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_MAC_CFG_STATUS_MAC_STICKY(target)  VTSS_IOREG(target,0x1c)

/** 
 * \brief
 * Indicates that an inter packet gap shrink was detected (IPG < 12 bytes).
 *
 * \details 
 * '0': no ipg shrink was detected
 * '1': one or more ipg shrink were detected
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_STICKY . RX_IPG_SHRINK_STICKY
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_STICKY_RX_IPG_SHRINK_STICKY  VTSS_BIT(9)

/** 
 * \brief
 * Indicates that a preamble shrink was detected (preamble < 8 bytes).
 * This sticky bit can only be set when the port is setup in 10 Gbps mode,
 * where frames with e.g. a 4 bytes preamble will be discarded, and it
 * requires that PRM_SHK_CHK_DIS = 0 and SFD_CHK_ENA = 1.
 * In SGMII mode, all preamble sizes down to 3 bytes (including SFD) are
 * accepted and will not cause this sticky bit to be set.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_STICKY . RX_PREAM_SHRINK_STICKY
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_STICKY_RX_PREAM_SHRINK_STICKY  VTSS_BIT(8)

/** 
 * \brief
 * If Preamble Check is enabled and a SOP is received, this bit is set if
 * the following bytes do not match a "5555555555..55D5" pattern.
 * A 12 bytes preamble of "55555555.55555555.555555D5" will not cause this
 * sticky bit to be set.
 * 
 * This sticky bit can only be set when the port is setup in 10 Gbps mode.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_STICKY . RX_PREAM_MISMATCH_STICKY
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_STICKY_RX_PREAM_MISMATCH_STICKY  VTSS_BIT(7)

/** 
 * \brief
 * If a SOP is received and a following control character is received
 * within the preamble, this bit is set. (No data is passed to the host
 * interface of the MAC).
 *
 * \details 
 * '0': <text>
 * '1': <text>
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_STICKY . RX_PREAM_ERR_STICKY
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_STICKY_RX_PREAM_ERR_STICKY  VTSS_BIT(6)

/** 
 * \brief
 * Indicates that a frame was received with a non-standard preamble.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_STICKY . RX_NON_STD_PREAM_STICKY
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_STICKY_RX_NON_STD_PREAM_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * Indicates that a frame with MPLS multicast was received.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_STICKY . RX_MPLS_MC_STICKY
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_STICKY_RX_MPLS_MC_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Indicates that a frame with MPLS unicast was received.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_STICKY . RX_MPLS_UC_STICKY
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_STICKY_RX_MPLS_UC_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * Indicates that a frame was received with a VLAN tag.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_STICKY . RX_TAG_STICKY
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_STICKY_RX_TAG_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Sticky bit indicating that the MAC Transmit FIFO has dropped one or more
 * frames because of underrun.
 *
 * \details 
 * '0': No MAC Tx FIFO underrun has occured
 * '1': One or more MAC Tx FIFO underruns have occured
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_STICKY . TX_UFLW_STICKY
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_STICKY_TX_UFLW_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Indicates that the transmit host initiated abort was executed.
 *
 * \details 
 * '0': no tx frame was aborted
 * '1': one or more tx frames were aborted
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_DEV10G_MAC_CFG_STATUS_MAC_STICKY . TX_ABORT_STICKY
 */
#define  VTSS_F_DEV10G_MAC_CFG_STATUS_MAC_STICKY_TX_ABORT_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a DEV10G:DEV_STATISTICS_32BIT
 *
 * Device Statistics Registers
 */


/** 
 * \brief Rx Symbol Carrier Error Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:RX_SYMBOL_ERR_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_RX_SYMBOL_ERR_CNT(target)  VTSS_IOREG(target,0x1d)


/** 
 * \brief Rx Pause Frame Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:RX_PAUSE_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_RX_PAUSE_CNT(target)  VTSS_IOREG(target,0x1e)


/** 
 * \brief Rx Control Frame Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:RX_UNSUP_OPCODE_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_RX_UNSUP_OPCODE_CNT(target)  VTSS_IOREG(target,0x1f)


/** 
 * \brief Rx Unicast Frame Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:RX_UC_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_RX_UC_CNT(target)  VTSS_IOREG(target,0x20)


/** 
 * \brief Rx Multicast Frame Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:RX_MC_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_RX_MC_CNT(target)  VTSS_IOREG(target,0x21)


/** 
 * \brief Rx Broadcast Frame Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:RX_BC_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_RX_BC_CNT(target)  VTSS_IOREG(target,0x22)


/** 
 * \brief Rx CRC Error Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:RX_CRC_ERR_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_RX_CRC_ERR_CNT(target)  VTSS_IOREG(target,0x23)


/** 
 * \brief Rx Undersize Counter (valid frame format)
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:RX_UNDERSIZE_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_RX_UNDERSIZE_CNT(target)  VTSS_IOREG(target,0x24)


/** 
 * \brief Rx Undersize Counter (CRC error)
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:RX_FRAGMENTS_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_RX_FRAGMENTS_CNT(target)  VTSS_IOREG(target,0x25)


/** 
 * \brief Rx In-range Length Error Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:RX_IN_RANGE_LEN_ERR_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_RX_IN_RANGE_LEN_ERR_CNT(target)  VTSS_IOREG(target,0x26)


/** 
 * \brief Rx Out-Of-Range Length Error Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:RX_OUT_OF_RANGE_LEN_ERR_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_RX_OUT_OF_RANGE_LEN_ERR_CNT(target)  VTSS_IOREG(target,0x27)


/** 
 * \brief Rx Oversize Counter (valid frame format)
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:RX_OVERSIZE_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_RX_OVERSIZE_CNT(target)  VTSS_IOREG(target,0x28)


/** 
 * \brief Rx Jabbers Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:RX_JABBERS_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_RX_JABBERS_CNT(target)  VTSS_IOREG(target,0x29)


/** 
 * \brief Rx 64 Byte Frame Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:RX_SIZE64_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_RX_SIZE64_CNT(target)  VTSS_IOREG(target,0x2a)


/** 
 * \brief Rx 65-127 Byte Frame Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:RX_SIZE65TO127_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_RX_SIZE65TO127_CNT(target)  VTSS_IOREG(target,0x2b)


/** 
 * \brief Rx 128-255 Byte Frame Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:RX_SIZE128TO255_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_RX_SIZE128TO255_CNT(target)  VTSS_IOREG(target,0x2c)


/** 
 * \brief Rx 256-511 Byte Frame Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:RX_SIZE256TO511_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_RX_SIZE256TO511_CNT(target)  VTSS_IOREG(target,0x2d)


/** 
 * \brief Rx 512-1023 Byte Frame Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:RX_SIZE512TO1023_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_RX_SIZE512TO1023_CNT(target)  VTSS_IOREG(target,0x2e)


/** 
 * \brief Rx 1024-1518 Byte Frame Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:RX_SIZE1024TO1518_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_RX_SIZE1024TO1518_CNT(target)  VTSS_IOREG(target,0x2f)


/** 
 * \brief Rx 1519 To Max. Length Byte Frame Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:RX_SIZE1519TOMAX_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_RX_SIZE1519TOMAX_CNT(target)  VTSS_IOREG(target,0x30)


/** 
 * \brief Rx Inter Packet Gap Shrink Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:RX_IPG_SHRINK_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_RX_IPG_SHRINK_CNT(target)  VTSS_IOREG(target,0x31)


/** 
 * \brief Tx Pause Frame Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:TX_PAUSE_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_TX_PAUSE_CNT(target)  VTSS_IOREG(target,0x32)


/** 
 * \brief Tx Unicast Frame Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:TX_UC_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_TX_UC_CNT(target)  VTSS_IOREG(target,0x33)


/** 
 * \brief Tx Multicast Frame Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:TX_MC_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_TX_MC_CNT(target)  VTSS_IOREG(target,0x34)


/** 
 * \brief Tx Broadcast Frame Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:TX_BC_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_TX_BC_CNT(target)  VTSS_IOREG(target,0x35)


/** 
 * \brief Tx 64 Byte Frame Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:TX_SIZE64_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_TX_SIZE64_CNT(target)  VTSS_IOREG(target,0x36)


/** 
 * \brief Tx 65-127 Byte Frame Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:TX_SIZE65TO127_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_TX_SIZE65TO127_CNT(target)  VTSS_IOREG(target,0x37)


/** 
 * \brief Tx 128-255 Byte Frame Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:TX_SIZE128TO255_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_TX_SIZE128TO255_CNT(target)  VTSS_IOREG(target,0x38)


/** 
 * \brief Tx 256-511 Byte Frame Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:TX_SIZE256TO511_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_TX_SIZE256TO511_CNT(target)  VTSS_IOREG(target,0x39)


/** 
 * \brief Tx 512-1023 Byte Frame Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:TX_SIZE512TO1023_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_TX_SIZE512TO1023_CNT(target)  VTSS_IOREG(target,0x3a)


/** 
 * \brief Tx 1024-1518 Byte Frame Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:TX_SIZE1024TO1518_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_TX_SIZE1024TO1518_CNT(target)  VTSS_IOREG(target,0x3b)


/** 
 * \brief Tx 1519 To Max. Length Byte Frame Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:TX_SIZE1519TOMAX_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_TX_SIZE1519TOMAX_CNT(target)  VTSS_IOREG(target,0x3c)


/** 
 * \brief Rx HiH Checksum Error Counter
 *
 * \details
 * If HIH CRC checking is enabled, this counter will count the number of
 * frames discarded due to HIH CRC errors.
 *
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:RX_HIH_CKSM_ERR_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_RX_HIH_CKSM_ERR_CNT(target)  VTSS_IOREG(target,0x3d)


/** 
 * \brief Rx XGMII Protocol Error Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_32BIT:RX_XGMII_PROT_ERR_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_32BIT_RX_XGMII_PROT_ERR_CNT(target)  VTSS_IOREG(target,0x3e)

/**
 * Register Group: \a DEV10G:DEV_STATISTICS_40BIT
 *
 * Device Statistics Registers
 */


/** 
 * \brief Rx Byte Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_40BIT:RX_IN_BYTES_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_40BIT_RX_IN_BYTES_CNT(target)  VTSS_IOREG(target,0x3f)


/** 
 * \brief Rx In Bytes Counter - MSB part
 *
 * \details
 * The number of bytes received (good, bad. Framing). 
 * MSB s only)
 *
 * Register: \a DEV10G:DEV_STATISTICS_40BIT:RX_IN_BYTES_MSB_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_40BIT_RX_IN_BYTES_MSB_CNT(target)  VTSS_IOREG(target,0x40)

/** 
 * \brief
 * The number of bytes received (good, bad, and framing) - MSBs only
 *
 * \details 
 * Counter can be written by SW.
 *
 * Field: ::VTSS_DEV10G_DEV_STATISTICS_40BIT_RX_IN_BYTES_MSB_CNT . RX_IN_BYTES_MSB_CNT
 */
#define  VTSS_F_DEV10G_DEV_STATISTICS_40BIT_RX_IN_BYTES_MSB_CNT_RX_IN_BYTES_MSB_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_DEV10G_DEV_STATISTICS_40BIT_RX_IN_BYTES_MSB_CNT_RX_IN_BYTES_MSB_CNT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_DEV10G_DEV_STATISTICS_40BIT_RX_IN_BYTES_MSB_CNT_RX_IN_BYTES_MSB_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Rx OK Byte Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_40BIT:RX_OK_BYTES_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_40BIT_RX_OK_BYTES_CNT(target)  VTSS_IOREG(target,0x41)


/** 
 * \brief Rx Ok Bytes Counter - MSB part
 *
 * \details
 * The number of received bytes in good frames - MSBs only
 *
 * Register: \a DEV10G:DEV_STATISTICS_40BIT:RX_OK_BYTES_MSB_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_40BIT_RX_OK_BYTES_MSB_CNT(target)  VTSS_IOREG(target,0x42)

/** 
 * \brief
 * The numberof received bytes in good frames - MSBs only.
 *
 * \details 
 * Counter can be written by SW.
 *
 * Field: ::VTSS_DEV10G_DEV_STATISTICS_40BIT_RX_OK_BYTES_MSB_CNT . RX_OK_BYTES_MSB_CNT
 */
#define  VTSS_F_DEV10G_DEV_STATISTICS_40BIT_RX_OK_BYTES_MSB_CNT_RX_OK_BYTES_MSB_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_DEV10G_DEV_STATISTICS_40BIT_RX_OK_BYTES_MSB_CNT_RX_OK_BYTES_MSB_CNT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_DEV10G_DEV_STATISTICS_40BIT_RX_OK_BYTES_MSB_CNT_RX_OK_BYTES_MSB_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Rx Bad Byte Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_40BIT:RX_BAD_BYTES_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_40BIT_RX_BAD_BYTES_CNT(target)  VTSS_IOREG(target,0x43)


/** 
 * \brief Rx Bad Bytes Counter - MSB part
 *
 * \details
 * The number of received bytes in bad frames -  MSBs only.
 *
 * Register: \a DEV10G:DEV_STATISTICS_40BIT:RX_BAD_BYTES_MSB_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_40BIT_RX_BAD_BYTES_MSB_CNT(target)  VTSS_IOREG(target,0x44)

/** 
 * \brief
 * The number of received bytes in bad frames - MSBs only.
 *
 * \details 
 * Counter can be written by SW.
 *
 * Field: ::VTSS_DEV10G_DEV_STATISTICS_40BIT_RX_BAD_BYTES_MSB_CNT . RX_BAD_BYTES_MSB_CNT
 */
#define  VTSS_F_DEV10G_DEV_STATISTICS_40BIT_RX_BAD_BYTES_MSB_CNT_RX_BAD_BYTES_MSB_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_DEV10G_DEV_STATISTICS_40BIT_RX_BAD_BYTES_MSB_CNT_RX_BAD_BYTES_MSB_CNT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_DEV10G_DEV_STATISTICS_40BIT_RX_BAD_BYTES_MSB_CNT_RX_BAD_BYTES_MSB_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Tx Byte Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_40BIT:TX_OUT_BYTES_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_40BIT_TX_OUT_BYTES_CNT(target)  VTSS_IOREG(target,0x45)


/** 
 * \brief Tx Out Bytes Counter - MSB part
 *
 * \details
 * The number of bytes transmitted (good, bad, and framing) - MSBs only
 *
 * Register: \a DEV10G:DEV_STATISTICS_40BIT:TX_OUT_BYTES_MSB_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_40BIT_TX_OUT_BYTES_MSB_CNT(target)  VTSS_IOREG(target,0x46)

/** 
 * \brief
 * The number of bytes transmitted (good, bad, framing) - MSBs only.
 *
 * \details 
 * Counter can be written by SW.
 *
 * Field: ::VTSS_DEV10G_DEV_STATISTICS_40BIT_TX_OUT_BYTES_MSB_CNT . TX_OUT_BYTES_MSB_CNT
 */
#define  VTSS_F_DEV10G_DEV_STATISTICS_40BIT_TX_OUT_BYTES_MSB_CNT_TX_OUT_BYTES_MSB_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_DEV10G_DEV_STATISTICS_40BIT_TX_OUT_BYTES_MSB_CNT_TX_OUT_BYTES_MSB_CNT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_DEV10G_DEV_STATISTICS_40BIT_TX_OUT_BYTES_MSB_CNT_TX_OUT_BYTES_MSB_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Tx OK Byte Counter
 *
 * \details
 * Register: \a DEV10G:DEV_STATISTICS_40BIT:TX_OK_BYTES_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_40BIT_TX_OK_BYTES_CNT(target)  VTSS_IOREG(target,0x47)


/** 
 * \brief Tx OK Bytes Counter - MSB part
Tx OK Bytes Counter - MSB
 *
 * \details
 * The number of bytes transmitted - MSBs only.
 *
 * Register: \a DEV10G:DEV_STATISTICS_40BIT:TX_OK_BYTES_MSB_CNT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_STATISTICS_40BIT_TX_OK_BYTES_MSB_CNT(target)  VTSS_IOREG(target,0x48)

/** 
 * \brief
 * The numbe rof transmitted bytes transmitted successfully - MSBs only.
 *
 * \details 
 * Counter can be written by SW.
 *
 * Field: ::VTSS_DEV10G_DEV_STATISTICS_40BIT_TX_OK_BYTES_MSB_CNT . TX_OK_BYTES_MSB_CNT
 */
#define  VTSS_F_DEV10G_DEV_STATISTICS_40BIT_TX_OK_BYTES_MSB_CNT_TX_OK_BYTES_MSB_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_DEV10G_DEV_STATISTICS_40BIT_TX_OK_BYTES_MSB_CNT_TX_OK_BYTES_MSB_CNT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_DEV10G_DEV_STATISTICS_40BIT_TX_OK_BYTES_MSB_CNT_TX_OK_BYTES_MSB_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/**
 * Register Group: \a DEV10G:PCS_XAUI_CONFIGURATION
 *
 * PCS XAUI Configuration Registers
 */


/** 
 * \brief PCS XAUI Configuration
 *
 * \details
 * PCS XAUI Configuration Register
 *
 * Register: \a DEV10G:PCS_XAUI_CONFIGURATION:PCS_XAUI_CFG
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG(target)  VTSS_IOREG(target,0x49)

/** 
 * \brief
 * Testloop, if enabled (XAUI encoded) data are looped from TX path to RX
 * path just before the SERDES
 *
 * \details 
 * 1: Enable loop
 * 0: Disable loop
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG . XAUI_LOOP_ENA
 */
#define  VTSS_F_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_XAUI_LOOP_ENA  VTSS_BIT(20)

/** 
 * \brief
 * Idle sequencing mode (IPG shrink mode support). When active, the first
 * ||I|| after ||T|| will be alternatingly ||K||, ||A|| or ||R|| instead of
 * ||K|| or ||A|| only in normal mode
 *
 * \details 
 * 1: Modified idle sequencing for IPG shrink mode support
 * 0: Normal idle sequencing
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG . IDLE_SEQ_MODE
 */
#define  VTSS_F_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_IDLE_SEQ_MODE  VTSS_BIT(12)

/** 
 * \brief
 * Disable RX Pad/Truncate Mode
 *
 * \details 
 * 0: Normal operation
 * 1: Disable pad/truncate
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG . PT_DIS
 */
#define  VTSS_F_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_PT_DIS  VTSS_BIT(2)

/** 
 * \brief
 * PCS enable
 *
 * \details 
 * 0: Disable PCS
 * 1: Enable PCS
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG . PCS_ENA
 */
#define  VTSS_F_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_CFG_PCS_ENA  VTSS_BIT(0)


/** 
 * \brief PCS XAUI Extended Configuration
 *
 * \details
 * Special configuration bit groups for PCS XAUI
 *
 * Register: \a DEV10G:PCS_XAUI_CONFIGURATION:PCS_XAUI_EXT_CFG
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_EXT_CFG(target)  VTSS_IOREG(target,0x4a)

/** 
 * \brief
 * Invert HM-Bus in receive direction, invert all data signals from SERDES
 *
 * \details 
 * 0: Normal operation
 * 1: Invert HM-bus
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_EXT_CFG . RX_INV_HMBUS
 */
#define  VTSS_F_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_EXT_CFG_RX_INV_HMBUS  VTSS_BIT(3)

/** 
 * \brief
 * Flip HM-Bus in receive direction, i.e. map lane 0 on 3, lane 1 on 2,
 * lane 2 on 1 and lane 3 on 0
 *
 * \details 
 * 0: Normal lane order
 * 1: Flipped lane order
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_EXT_CFG . RX_FLIP_HMBUS
 */
#define  VTSS_F_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_EXT_CFG_RX_FLIP_HMBUS  VTSS_BIT(2)

/** 
 * \brief
 * Invert HM-Bus in transmit direction, invert all data signals to SERDES
 *
 * \details 
 * 0: Normal operation
 * 1: Invert HM-bus
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_EXT_CFG . TX_INV_HMBUS
 */
#define  VTSS_F_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_EXT_CFG_TX_INV_HMBUS  VTSS_BIT(1)

/** 
 * \brief
 * Flip HM-Bus in transmit direction, i.e. map lane 0 on 3, lane 1 on 2,
 * lane 2 on 1 and lane 3 on 0
 *
 * \details 
 * 0: Normal lane order
 * 1: Flipped lane order
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_EXT_CFG . TX_FLIP_HMBUS
 */
#define  VTSS_F_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_EXT_CFG_TX_FLIP_HMBUS  VTSS_BIT(0)


/** 
 * \brief PCS XAUI SignalDetect Configuration
 *
 * \details
 * PCS_XAUI signal_detect configuration
 *
 * Register: \a DEV10G:PCS_XAUI_CONFIGURATION:PCS_XAUI_SD_CFG
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG(target)  VTSS_IOREG(target,0x4b)

/** 
 * \brief
 * Signal detect selection (select input for internal signal_detect line)
 *
 * \details 
 * 0: Select signal_detect line from hardmacro
 * 1: Select external signal_detect line
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG . SD_SEL
 */
#define  VTSS_F_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG_SD_SEL  VTSS_BIT(8)

/** 
 * \brief
 * Signal detect polarity: The signal level on signal_detect input pin must
 * be equal to SD_POL to indicate signal detection (SD_ENA must be set)
 *
 * \details 
 * 0: Signal Detect input pin must be '0' to indicate a signal detection
 * 1: Signal Detect input pin must be '1' to indicate a signal detection
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG . SD_POL
 */
#define  VTSS_F_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG_SD_POL  VTSS_BIT(4)

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
 * Field: ::VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG . SD_ENA
 */
#define  VTSS_F_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_SD_CFG_SD_ENA  VTSS_BIT(0)


/** 
 * \brief PCS Transmitter Sequence Configuration Register
 *
 * \details
 * Sequence Transmit Configuration Register
 *
 * Register: \a DEV10G:PCS_XAUI_CONFIGURATION:PCS_XAUI_TX_SEQ_CFG
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_TX_SEQ_CFG(target)  VTSS_IOREG(target,0x4c)

/** 
 * \brief
 * Transmit ||Q|| code (Sequence information, i.e. lower 24 bit of a
 * Sequence)
 *
 * \details 
 * Field: ::VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_TX_SEQ_CFG . TX_Q
 */
#define  VTSS_F_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_TX_SEQ_CFG_TX_Q(x)  VTSS_ENCODE_BITFIELD(x,8,24)
#define  VTSS_M_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_TX_SEQ_CFG_TX_Q     VTSS_ENCODE_BITMASK(8,24)
#define  VTSS_X_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_TX_SEQ_CFG_TX_Q(x)  VTSS_EXTRACT_BITFIELD(x,8,24)

/** 
 * \brief
 * Disable Transmit ||Q|| code replacement
 *
 * \details 
 * 1: Disable
 * 0: Enable
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_TX_SEQ_CFG . TX_Q_DIS
 */
#define  VTSS_F_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_TX_SEQ_CFG_TX_Q_DIS  VTSS_BIT(7)


/** 
 * \brief PCS XAUI Receiver Error Counter Configuration
 *
 * \details
 * Error Counter Configuration Register (if a bit in the mask field is set,
 * the errors of that lane are not counted).
 *
 * Register: \a DEV10G:PCS_XAUI_CONFIGURATION:PCS_XAUI_RX_ERR_CNT_CFG
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG(target)  VTSS_IOREG(target,0x4d)

/** 
 * \brief
 * Disparity error counting mask
 *
 * \details 
 * 0000: Count errors of all lanes
 * 1110: Count error of lane 0 only
 * ...
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG . DERR_MASK
 */
#define  VTSS_F_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG_DERR_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,4)
#define  VTSS_M_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG_DERR_MASK     VTSS_ENCODE_BITMASK(12,4)
#define  VTSS_X_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG_DERR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,4)

/** 
 * \brief
 * Codegroup error counting mask
 *
 * \details 
 * 0000: Count errors of all lanes
 * 1110: Count error of lane 0 only
 * ...
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG . CERR_MASK
 */
#define  VTSS_F_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG_CERR_MASK(x)  VTSS_ENCODE_BITFIELD(x,8,4)
#define  VTSS_M_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG_CERR_MASK     VTSS_ENCODE_BITMASK(8,4)
#define  VTSS_X_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG_CERR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,8,4)

/** 
 * \brief
 * Fifo underflow error counting mask
 *
 * \details 
 * 0000: Count errors of all lanes
 * 1110: Count error of lane 0 only
 * ...
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG . UFERR_MASK
 */
#define  VTSS_F_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG_UFERR_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG_UFERR_MASK     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG_UFERR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,4)

/** 
 * \brief
 * Fifo overflow error counting mask
 *
 * \details 
 * 0000: Count errors of all lanes
 * 1110: Count error of lane 0 only
 * ...
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG . OFERR_MASK
 */
#define  VTSS_F_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG_OFERR_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG_OFERR_MASK     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_RX_ERR_CNT_CFG_OFERR_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief PCS Interleave Mode Configuration
 *
 * \details
 * Configuration register set for Interleave Mode (XAUI via two lanes) -
 * also known as DDR-XAUI
 *
 * Register: \a DEV10G:PCS_XAUI_CONFIGURATION:PCS_XAUI_INTERLEAVE_MODE_CFG
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG(target)  VTSS_IOREG(target,0x4e)

/** 
 * \brief
 * Dual column ||A|| alignment (||A|| are inserted on even columns only)
 *
 * \details 
 * 0: Normal insertion
 * 1: Even column insertion only
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG . DC_A_ALIGN_ENA
 */
#define  VTSS_F_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_DC_A_ALIGN_ENA  VTSS_BIT(17)

/** 
 * \brief
 * Comma synchronization mode
 *
 * \details 
 * 0: Synchronize on any 7-bit comma (XAUI compliant)
 * 1: Synchronize on K28.5 only (non XAUI compliant)
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG . K28_5_SYNC_ENA
 */
#define  VTSS_F_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_K28_5_SYNC_ENA  VTSS_BIT(16)

/** 
 * \brief
 * Comma replacement. In interleave mode (using /K/ byte orderung) one
 * 20-bit word must have only one comma for proper alignment. Misleading
 * commas are replaced by comma_repl in transmit direction and replaced by
 * K28.5-commas again in receive direction. Comma_repl has to be an unused
 * valid special code-group which does not contain a comma, i.e. K28.2,
 * K28.6 or K23.7 are possible replacements.
 *
 * \details 
 * Field: ::VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG . COMMA_REPL
 */
#define  VTSS_F_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_COMMA_REPL(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_COMMA_REPL     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_COMMA_REPL(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Interleave mode selection. In interleave mode XAUI data are sent via two
 * 5Gbps lanes
 *
 * \details 
 * 0: Interleave mode with /K/ comma based byte re-ordering (using comma
 * replacement)
 * 1: Interleave mode with /A/ alignment symbol based byte re-ordering
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG . ILV_MODE
 */
#define  VTSS_F_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE  VTSS_BIT(1)

/** 
 * \brief
 * Interleave mode enable. In interleave mode XAUI data are sent via two
 * 5Gbps lanes
 *
 * \details 
 * 0: Normal XAUI mode
 * 1: Interleave mode
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG . ILV_MODE_ENA
 */
#define  VTSS_F_DEV10G_PCS_XAUI_CONFIGURATION_PCS_XAUI_INTERLEAVE_MODE_CFG_ILV_MODE_ENA  VTSS_BIT(0)

/**
 * Register Group: \a DEV10G:PCS_XAUI_STATUS
 *
 * PCS XAUI Status Registers
 */


/** 
 * \brief PCS Receiver Status Register
 *
 * \details
 * Receive Lane Status Register
 *
 * Register: \a DEV10G:PCS_XAUI_STATUS:PCS_XAUI_RX_STATUS
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_STATUS(target)  VTSS_IOREG(target,0x50)

/** 
 * \brief
 * Current status of selected signal_detect input lines
 *
 * \details 
 * Field: ::VTSS_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_STATUS . SIGNAL_DETECT
 */
#define  VTSS_F_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_STATUS_SIGNAL_DETECT(x)  VTSS_ENCODE_BITFIELD(x,12,4)
#define  VTSS_M_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_STATUS_SIGNAL_DETECT     VTSS_ENCODE_BITMASK(12,4)
#define  VTSS_X_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_STATUS_SIGNAL_DETECT(x)  VTSS_EXTRACT_BITFIELD(x,12,4)

/** 
 * \brief
 * Local Fault status (one or more of sync/align/fifo_of/fifo_uf/8b10b
 * error)
 *
 * \details 
 * 1: A fault occured
 * 0: No fault detected
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_STATUS . LOCAL_FAULT_STICKY
 */
#define  VTSS_F_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_STATUS_LOCAL_FAULT_STICKY(x)  VTSS_ENCODE_BITFIELD(x,8,4)
#define  VTSS_M_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_STATUS_LOCAL_FAULT_STICKY     VTSS_ENCODE_BITMASK(8,4)
#define  VTSS_X_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_STATUS_LOCAL_FAULT_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,8,4)

/** 
 * \brief
 * Status of lane alignment
 *
 * \details 
 * 1: All lanes are aligned
 * 0: No alignment reached
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_STATUS . ALIGNMENT_STATUS
 */
#define  VTSS_F_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_STATUS_ALIGNMENT_STATUS  VTSS_BIT(4)

/** 
 * \brief
 * Status of code group alignment (lane independent)
 *
 * \details 
 * 1111: All lanes in sync
 * 0001: Lane 0 is in sync
 * ...
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_STATUS . SYNC_STATUS
 */
#define  VTSS_F_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_STATUS_SYNC_STATUS(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_STATUS_SYNC_STATUS     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_STATUS_SYNC_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief PCS Receiver Lane Error Status
 *
 * \details
 * Receiver Lane Error Status register
 *
 * Register: \a DEV10G:PCS_XAUI_STATUS:PCS_XAUI_RX_ERROR_STATUS
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_ERROR_STATUS(target)  VTSS_IOREG(target,0x51)

/** 
 * \brief
 * Coding error detected in received 8B/10B encoded data
 *
 * \details 
 * 0: No error found
 * 1: Coding error detected
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_ERROR_STATUS . C8B10B_ERR_STICKY
 */
#define  VTSS_F_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_ERROR_STATUS_C8B10B_ERR_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * Alignment lost in deskew logic
 *
 * \details 
 * 0: No misalignment occured
 * 1: A (temporary) misalignment has been detected
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_ERROR_STATUS . ALIGNMENT_LOST_STICKY
 */
#define  VTSS_F_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_ERROR_STATUS_ALIGNMENT_LOST_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Synchronization lost in lane i (i = 0...3, one bit per lane)
 *
 * \details 
 * 0: No sync lost occured
 * 1: Synchronization lost in lane i (temporarily)
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_ERROR_STATUS . SYNC_LOST_STICKY
 */
#define  VTSS_F_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_ERROR_STATUS_SYNC_LOST_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_ERROR_STATUS_SYNC_LOST_STICKY     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_ERROR_STATUS_SYNC_LOST_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief PCS Receiver Sequence Result Register
 *
 * \details
 * Sequence Receive Status Register
 *
 * Register: \a DEV10G:PCS_XAUI_STATUS:PCS_XAUI_RX_SEQ_REC_STATUS
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_SEQ_REC_STATUS(target)  VTSS_IOREG(target,0x52)

/** 
 * \brief
 * Received ||Q|| code (Sequence information, i.e. lower 24 bit of a
 * Sequence)
 *
 * \details 
 * Field: ::VTSS_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_SEQ_REC_STATUS . RX_Q
 */
#define  VTSS_F_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_SEQ_REC_STATUS_RX_Q(x)  VTSS_ENCODE_BITFIELD(x,8,24)
#define  VTSS_M_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_SEQ_REC_STATUS_RX_Q     VTSS_ENCODE_BITMASK(8,24)
#define  VTSS_X_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_SEQ_REC_STATUS_RX_Q(x)  VTSS_EXTRACT_BITFIELD(x,8,24)

/** 
 * \brief
 * Received ||Q|| code changed
 *
 * \details 
 * 1: New ||Q|| has been received
 * 0: No new ||Q|| since last read
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_SEQ_REC_STATUS . RX_Q_CHANGED_STICKY
 */
#define  VTSS_F_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_SEQ_REC_STATUS_RX_Q_CHANGED_STICKY  VTSS_BIT(0)


/** 
 * \brief PCS RX Fifo Overflow Error and Lane 0 Error Counter
 *
 * \details
 * Receive Fifo Overflow error counter. In test pattern check mode, this
 * counter counts the errors of lane 0. In the latter case the counter is
 * incremented by one whenever at least one out of eighty received bits
 * (eight symbols) is corrupted.
 *
 * Register: \a DEV10G:PCS_XAUI_STATUS:PCS_XAUI_RX_FIFO_OF_ERR_L0_CNT_STATUS
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_FIFO_OF_ERR_L0_CNT_STATUS(target)  VTSS_IOREG(target,0x53)


/** 
 * \brief PCS RX Fifo Underflow Error and Lane 1 Error Counter
 *
 * \details
 * Receive Fifo Underflow error counter. In test pattern check mode, this
 * counter counts the errors of lane 1. In the latter case the counter is
 * incremented by one whenever at least one out of eighty received bits
 * (eight symbols) is corrupted.
 *
 * Register: \a DEV10G:PCS_XAUI_STATUS:PCS_XAUI_RX_FIFO_UF_ERR_L1_CNT_STATUS
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_FIFO_UF_ERR_L1_CNT_STATUS(target)  VTSS_IOREG(target,0x54)


/** 
 * \brief PCS RX 10b8b Disparity Error and Lane 2 Error Counter
 *
 * \details
 * 10b8b Decoder Disparity error counter. In test pattern check mode, this
 * counter counts the errors of lane 2. In the latter case the counter is
 * incremented by one whenever at least one out of eighty received bits
 * (eight symbols) is corrupted.
 *
 * Register: \a DEV10G:PCS_XAUI_STATUS:PCS_XAUI_RX_FIFO_D_ERR_L2_CNT_STATUS
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_FIFO_D_ERR_L2_CNT_STATUS(target)  VTSS_IOREG(target,0x55)


/** 
 * \brief PCS RX 10b8b Codegroup Error and Lane 3 Error Counter
 *
 * \details
 * 10b8b Decoder Codegroup error counter. In test pattern check mode, this
 * counter counts the errors of lane 3. In the latter case the counter is
 * incremented by one whenever at least one out of eighty received bits
 * (eight symbols) is corrupted.
 *
 * Register: \a DEV10G:PCS_XAUI_STATUS:PCS_XAUI_RX_FIFO_CG_ERR_L3_CNT_STATUS
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS_XAUI_STATUS_PCS_XAUI_RX_FIFO_CG_ERR_L3_CNT_STATUS(target)  VTSS_IOREG(target,0x56)

/**
 * Register Group: \a DEV10G:PCS_XAUI_TSTPAT_CONFIGURATION
 *
 * PCS XAUI Testpattern Configuration Registers
 */


/** 
 * \brief PCS XAUI Test Pattern Configuration
 *
 * \details
 * Test Pattern Generator/Checker Control Register
 *
 * Register: \a DEV10G:PCS_XAUI_TSTPAT_CONFIGURATION:PCS_XAUI_TSTPAT_CFG
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG(target)  VTSS_IOREG(target,0x58)

/** 
 * \brief
 * Capture current error counter values
 *
 * \details 
 * 1: Capture
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG . FREEZE_ERR_CNT_ENA
 */
#define  VTSS_F_DEV10G_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG_FREEZE_ERR_CNT_ENA  VTSS_BIT(8)

/** 
 * \brief
 * Enable Test pattern checker
 *
 * \details 
 * 1: Check
 * 0: No checking
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG . VT_CHK_ENA
 */
#define  VTSS_F_DEV10G_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG_VT_CHK_ENA  VTSS_BIT(7)

/** 
 * \brief
 * Check test pattern
 *
 * \details 
 * 000: Idle
 * 011: MFPAT
 * 100: CRPAT
 * 101: CJPAT
 * all others: Idle
 * Note: LFPAT and HFPAT can not be checked since sync is impossible
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG . VT_CHK_SEL
 */
#define  VTSS_F_DEV10G_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG_VT_CHK_SEL(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_DEV10G_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG_VT_CHK_SEL     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_DEV10G_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG_VT_CHK_SEL(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * Enable Test pattern generator
 *
 * \details 
 * 1: Generate test vectors
 * 0: Normal operation
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG . VT_GEN_ENA
 */
#define  VTSS_F_DEV10G_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG_VT_GEN_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Generate test pattern
 *
 * \details 
 * 000: Idle
 * 001: HFPAT
 * 010: LFPAT
 * 011: MFPAT
 * 100: CRPAT
 * 101: CJPAT
 * All others: Idle
 *
 * Field: ::VTSS_DEV10G_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG . VT_GEN_SEL
 */
#define  VTSS_F_DEV10G_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG_VT_GEN_SEL(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_DEV10G_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG_VT_GEN_SEL     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_DEV10G_PCS_XAUI_TSTPAT_CONFIGURATION_PCS_XAUI_TSTPAT_CFG_VT_GEN_SEL(x)  VTSS_EXTRACT_BITFIELD(x,0,3)

/**
 * Register Group: \a DEV10G:PCS_XAUI_TSTPAT_STATUS
 *
 * PCS XAUI Testpattern Status Registers
 */


/** 
 * \brief Test Pattern Receive Sequence Counter
 *
 * \details
 * Random Sequence Master Counter. The counter is used only if frame based
 * pattern are to be checked (CRPAT, CJPAT) and it is incremented by one
 * every 8th received symbol. The counter is started when a start of frame
 * is detected and the counter stops when the last symbol of the
 * (internally stored) reference frame was compared. The idle phase between
 * two frames is not checked.
 *
 * Register: \a DEV10G:PCS_XAUI_TSTPAT_STATUS:PCS_XAUI_TSTPAT_RX_SEQ_CNT_STATUS
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS_XAUI_TSTPAT_STATUS_PCS_XAUI_TSTPAT_RX_SEQ_CNT_STATUS(target)  VTSS_IOREG(target,0x59)


/** 
 * \brief Test Pattern Transmit Sequence Counter
 *
 * \details
 * Jitter Pattern Transmit Counter. The counter counts the number of
 * transmitted frames (only frame based pattern, i.e. CRPAT and CJPAT).
 *
 * Register: \a DEV10G:PCS_XAUI_TSTPAT_STATUS:PCS_XAUI_TSTPAT_TX_SEQ_CNT_STATUS
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS_XAUI_TSTPAT_STATUS_PCS_XAUI_TSTPAT_TX_SEQ_CNT_STATUS(target)  VTSS_IOREG(target,0x5a)

/**
 * Register Group: \a DEV10G:PCS2X6G_CONFIGURATION
 *
 * PCS2x6G Configuration Registers
 */


/** 
 * \brief PCS2x6G Configuration
 *
 * \details
 * Configuration register
 *
 * Register: \a DEV10G:PCS2X6G_CONFIGURATION:PCS2X6G_CFG
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS2X6G_CONFIGURATION_PCS2X6G_CFG(target)  VTSS_IOREG(target,0x5b)

/** 
 * \brief
 * Signal detect selection (select input for internal signal_detect line)
 *
 * \details 
 * 0: Select signal_detect line from hardmacro
 * 1: Select external signal_detect line
 *
 * Field: ::VTSS_DEV10G_PCS2X6G_CONFIGURATION_PCS2X6G_CFG . SD_SEL
 */
#define  VTSS_F_DEV10G_PCS2X6G_CONFIGURATION_PCS2X6G_CFG_SD_SEL  VTSS_BIT(26)

/** 
 * \brief
 * Signal detect polarity: The signal level on signal_detect input pin must
 * be equal to SD_POL to indicate signal detection (SD_ENA must be set)
 *
 * \details 
 * 0: Signal Detect input pin must be '0' to indicate a signal detection
 * 1: Signal Detect input pin must be '1' to indicate a signal detection
 *
 * Field: ::VTSS_DEV10G_PCS2X6G_CONFIGURATION_PCS2X6G_CFG . SD_POL
 */
#define  VTSS_F_DEV10G_PCS2X6G_CONFIGURATION_PCS2X6G_CFG_SD_POL  VTSS_BIT(25)

/** 
 * \brief
 * Signal Detect Enable
 *
 * \details 
 * 0: The Signal Detect input pin is ignored. The PCS2x6G assumes an active
 * Signal Detect at all times
 * 1: The Signal Detect input pin is used to determine if a signal is
 * detected
 *
 * Field: ::VTSS_DEV10G_PCS2X6G_CONFIGURATION_PCS2X6G_CFG . SD_ENA
 */
#define  VTSS_F_DEV10G_PCS2X6G_CONFIGURATION_PCS2X6G_CFG_SD_ENA  VTSS_BIT(24)

/** 
 * \brief
 * Force re-synchronization of receive logic
 *
 * \details 
 * 0: Normal operation
 * 1: Reset Synchronization
 *
 * Field: ::VTSS_DEV10G_PCS2X6G_CONFIGURATION_PCS2X6G_CFG . RESYNC_ENA
 */
#define  VTSS_F_DEV10G_PCS2X6G_CONFIGURATION_PCS2X6G_CFG_RESYNC_ENA  VTSS_BIT(16)

/** 
 * \brief
 * Number of sync headers required for block lock plus one (used in 10Gb-R
 * type SSM)
 *
 * \details 
 * Field: ::VTSS_DEV10G_PCS2X6G_CONFIGURATION_PCS2X6G_CFG . SH_CNT_MAX
 */
#define  VTSS_F_DEV10G_PCS2X6G_CONFIGURATION_PCS2X6G_CFG_SH_CNT_MAX(x)  VTSS_ENCODE_BITFIELD(x,10,6)
#define  VTSS_M_DEV10G_PCS2X6G_CONFIGURATION_PCS2X6G_CFG_SH_CNT_MAX     VTSS_ENCODE_BITMASK(10,6)
#define  VTSS_X_DEV10G_PCS2X6G_CONFIGURATION_PCS2X6G_CFG_SH_CNT_MAX(x)  VTSS_EXTRACT_BITFIELD(x,10,6)

/** 
 * \brief
 * Select type of Synchronization State Machine
 *
 * \details 
 * 0: Use 10Gb-R (Cl. 49) type
 * 1: Use 10Gb-X (Cl. 48) type
 * Others: Reserved
 *
 * Field: ::VTSS_DEV10G_PCS2X6G_CONFIGURATION_PCS2X6G_CFG . SYNC_TYPE_SEL
 */
#define  VTSS_F_DEV10G_PCS2X6G_CONFIGURATION_PCS2X6G_CFG_SYNC_TYPE_SEL(x)  VTSS_ENCODE_BITFIELD(x,8,2)
#define  VTSS_M_DEV10G_PCS2X6G_CONFIGURATION_PCS2X6G_CFG_SYNC_TYPE_SEL     VTSS_ENCODE_BITMASK(8,2)
#define  VTSS_X_DEV10G_PCS2X6G_CONFIGURATION_PCS2X6G_CFG_SYNC_TYPE_SEL(x)  VTSS_EXTRACT_BITFIELD(x,8,2)

/** 
 * \brief
 * PCS2x6G enable
 *
 * \details 
 * 0: Disable PCS
 * 1: Enable PCS
 *
 * Field: ::VTSS_DEV10G_PCS2X6G_CONFIGURATION_PCS2X6G_CFG . PCS_ENA
 */
#define  VTSS_F_DEV10G_PCS2X6G_CONFIGURATION_PCS2X6G_CFG_PCS_ENA  VTSS_BIT(0)

/**
 * Register Group: \a DEV10G:PCS2X6G_STATUS
 *
 * PCS2x6G Status Registers
 */


/** 
 * \brief PCS2x6G Status
 *
 * \details
 * Status of PCS2x6G logic
 *
 * Register: \a DEV10G:PCS2X6G_STATUS:PCS2X6G_STATUS
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS2X6G_STATUS_PCS2X6G_STATUS(target)  VTSS_IOREG(target,0x5c)

/** 
 * \brief
 * Current status of selected signal_detect input lines
 *
 * \details 
 * Field: ::VTSS_DEV10G_PCS2X6G_STATUS_PCS2X6G_STATUS . SIGNAL_DETECT
 */
#define  VTSS_F_DEV10G_PCS2X6G_STATUS_PCS2X6G_STATUS_SIGNAL_DETECT(x)  VTSS_ENCODE_BITFIELD(x,12,4)
#define  VTSS_M_DEV10G_PCS2X6G_STATUS_PCS2X6G_STATUS_SIGNAL_DETECT     VTSS_ENCODE_BITMASK(12,4)
#define  VTSS_X_DEV10G_PCS2X6G_STATUS_PCS2X6G_STATUS_SIGNAL_DETECT(x)  VTSS_EXTRACT_BITFIELD(x,12,4)

/** 
 * \brief
 * Status of deskew logic
 *
 * \details 
 * 0: Lanes not aligned
 * 1: All lanes are aligned
 *
 * Field: ::VTSS_DEV10G_PCS2X6G_STATUS_PCS2X6G_STATUS . RX_ALIGNMENT_STATUS
 */
#define  VTSS_F_DEV10G_PCS2X6G_STATUS_PCS2X6G_STATUS_RX_ALIGNMENT_STATUS  VTSS_BIT(4)

/** 
 * \brief
 * Status of synchronization in lane i (i = 0...3, one bit per lane)
 *
 * \details 
 * 0: Lane i out of sync
 * 1: Lane i is in sync
 *
 * Field: ::VTSS_DEV10G_PCS2X6G_STATUS_PCS2X6G_STATUS . RX_SYNC_STATUS
 */
#define  VTSS_F_DEV10G_PCS2X6G_STATUS_PCS2X6G_STATUS_RX_SYNC_STATUS(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_DEV10G_PCS2X6G_STATUS_PCS2X6G_STATUS_RX_SYNC_STATUS     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_DEV10G_PCS2X6G_STATUS_PCS2X6G_STATUS_RX_SYNC_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief PCS2x6G Error Status
 *
 * \details
 * Error indication of 64B/66B PCS2x6G logic
 *
 * Register: \a DEV10G:PCS2X6G_STATUS:PCS2X6G_ERR_STATUS
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS2X6G_STATUS_PCS2X6G_ERR_STATUS(target)  VTSS_IOREG(target,0x5d)

/** 
 * \brief
 * Coding error detected in xgmii data to be transmitted
 *
 * \details 
 * 0: No error found
 * 1: Coding error detected
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_DEV10G_PCS2X6G_STATUS_PCS2X6G_ERR_STATUS . XGMII_ERR_STICKY
 */
#define  VTSS_F_DEV10G_PCS2X6G_STATUS_PCS2X6G_ERR_STATUS_XGMII_ERR_STICKY  VTSS_BIT(6)

/** 
 * \brief
 * Coding error detected in received 64B/66B encoded data
 *
 * \details 
 * 0: No error found
 * 1: Coding error detected
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_DEV10G_PCS2X6G_STATUS_PCS2X6G_ERR_STATUS . C64B66B_ERR_STICKY
 */
#define  VTSS_F_DEV10G_PCS2X6G_STATUS_PCS2X6G_ERR_STATUS_C64B66B_ERR_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * Alignment lost in deskew logic
 *
 * \details 
 * 0: No misalignment occured
 * 1: A (temporary) misalignment has been detected
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_DEV10G_PCS2X6G_STATUS_PCS2X6G_ERR_STATUS . ALIGNMENT_LOST_STICKY
 */
#define  VTSS_F_DEV10G_PCS2X6G_STATUS_PCS2X6G_ERR_STATUS_ALIGNMENT_LOST_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Synchronization lost in lane i (i = 0...3, one bit per lane)
 *
 * \details 
 * 0: No sync lost occured
 * 1: Synchronization lost in lane i (temporarily)
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_DEV10G_PCS2X6G_STATUS_PCS2X6G_ERR_STATUS . SYNC_LOST_STICKY
 */
#define  VTSS_F_DEV10G_PCS2X6G_STATUS_PCS2X6G_ERR_STATUS_SYNC_LOST_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_DEV10G_PCS2X6G_STATUS_PCS2X6G_ERR_STATUS_SYNC_LOST_STICKY     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_DEV10G_PCS2X6G_STATUS_PCS2X6G_ERR_STATUS_SYNC_LOST_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief PCS2x6G Error Counter
 *
 * \details
 * PCS2x6G error counter
 *
 * Register: \a DEV10G:PCS2X6G_STATUS:PCS2X6G_ERR_CNT_STAT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS2X6G_STATUS_PCS2X6G_ERR_CNT_STAT(target)  VTSS_IOREG(target,0x5e)

/** 
 * \brief
 * Number of errors detected in 64B/66B decoder.
 *
 * \details 
 * Field: ::VTSS_DEV10G_PCS2X6G_STATUS_PCS2X6G_ERR_CNT_STAT . ERR_64B66BDEC
 */
#define  VTSS_F_DEV10G_PCS2X6G_STATUS_PCS2X6G_ERR_CNT_STAT_ERR_64B66BDEC(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_DEV10G_PCS2X6G_STATUS_PCS2X6G_ERR_CNT_STAT_ERR_64B66BDEC     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_DEV10G_PCS2X6G_STATUS_PCS2X6G_ERR_CNT_STAT_ERR_64B66BDEC(x)  VTSS_EXTRACT_BITFIELD(x,0,24)

/**
 * Register Group: \a DEV10G:PCS2X6G_EXT_CONFIGURATION
 *
 * PCS2x6G Extended Configuration Registers
 */


/** 
 * \brief PCS2x6G Extended Configuration
 *
 * \details
 * Special configuration bit groups for PCS2x6G.
 *
 * Register: \a DEV10G:PCS2X6G_EXT_CONFIGURATION:PCS2X6G_EXT_CFG
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_PCS2X6G_EXT_CONFIGURATION_PCS2X6G_EXT_CFG(target)  VTSS_IOREG(target,0x5f)

/** 
 * \brief
 * Invert HM-Bus, invert all data signals from/to SERDES
 *
 * \details 
 * 0: Normal operation
 * 1: Inverted HM-bus
 *
 * Field: ::VTSS_DEV10G_PCS2X6G_EXT_CONFIGURATION_PCS2X6G_EXT_CFG . INVERT_HMBUS
 */
#define  VTSS_F_DEV10G_PCS2X6G_EXT_CONFIGURATION_PCS2X6G_EXT_CFG_INVERT_HMBUS  VTSS_BIT(1)

/** 
 * \brief
 * Flip HM-Bus, i.e. map lane 0 on 1 and lane 1 on 0
 *
 * \details 
 * 0: Normal operation
 * 1: Flipped lane assignment
 *
 * Field: ::VTSS_DEV10G_PCS2X6G_EXT_CONFIGURATION_PCS2X6G_EXT_CFG . FLIP_HMBUS
 */
#define  VTSS_F_DEV10G_PCS2X6G_EXT_CONFIGURATION_PCS2X6G_EXT_CFG_FLIP_HMBUS  VTSS_BIT(0)

/**
 * Register Group: \a DEV10G:DEV_CFG_STATUS
 *
 * Device 10G Configuration and Status Registers
 */


/** 
 * \brief Clock/Reset Control Register
 *
 * \details
 * Register: \a DEV10G:DEV_CFG_STATUS:DEV_RST_CTRL
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_CFG_STATUS_DEV_RST_CTRL(target)  VTSS_IOREG(target,0x60)

/** 
 * \brief
 * Backplane Ethernet: Enable parallel detection mode for autonegotiation
 *
 * \details 
 * '0':  Parallel detection mode disabled
 * '1': Parallel detection mode enabled
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_DEV_RST_CTRL . PARDET_MODE_ENA
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_DEV_RST_CTRL_PARDET_MODE_ENA  VTSS_BIT(23)

/** 
 * \brief
 * This field is used to configure the MAC and PCS Rx/Tx clock frequencies.
 * 
 * Note:
 * LFS signalling must be disabled if SGMII mode is selected.
 *
 * \details 
 * '000':  SGMII 10 Mbps
 * '001':  SGMII 100 Mbps
 * '010':  SGMII 1000 Mbps
 * '011':  SGMII 2500 Mbps
 * '100':  XAUI 10 Gbps or 12Gbps XAUI
 * '101':  VAUI 12 Gbps (2 lanes)
 * '110':  Both MAC and PCS Rx/Tx clocks are disabled
 * '111': Invalid.
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_DEV_RST_CTRL . SPEED_SEL
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_DEV_RST_CTRL_SPEED_SEL(x)  VTSS_ENCODE_BITFIELD(x,20,3)
#define  VTSS_M_DEV10G_DEV_CFG_STATUS_DEV_RST_CTRL_SPEED_SEL     VTSS_ENCODE_BITMASK(20,3)
#define  VTSS_X_DEV10G_DEV_CFG_STATUS_DEV_RST_CTRL_SPEED_SEL(x)  VTSS_EXTRACT_BITFIELD(x,20,3)

/** 
 * \brief
 * Reset PCS Tx clock domains of the device.
 *
 * \details 
 * '0': The PCS Tx clock domain is NOT reset
 * '1': The PCS Tx clock domain is reset 
 * Note: PCS_TX_RST is NOT a one-shot operation. The PCS Tx clock domain
 * remains reset until a '0' is written to PCS_TX_RST.
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_DEV_RST_CTRL . PCS_TX_RST
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_DEV_RST_CTRL_PCS_TX_RST  VTSS_BIT(12)

/** 
 * \brief
 * Reset PCS Rx clock domains of the device.
 *
 * \details 
 * '0': The PCS Rx clock domain is NOT reset
 * '1': The PCS Rx clock domain is reset 
 * Note: PCS_RX_RST is NOT a one-shot operation. The PCS Rx clock domain
 * remains reset until a '0' is written to PCS_RX_RST.
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_DEV_RST_CTRL . PCS_RX_RST
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_DEV_RST_CTRL_PCS_RX_RST  VTSS_BIT(8)

/** 
 * \brief
 * Reset MAC Tx clock domain of device.
 *
 * \details 
 * '0': The MAC Tx clock domain is not reset.
 * '1': The MAC Tx clock domain is reset.
 * 
 * Note: The MAC Tx clock domain remains reset until 0 is written to this
 * register field.
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_DEV_RST_CTRL . MAC_TX_RST
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_DEV_RST_CTRL_MAC_TX_RST  VTSS_BIT(4)

/** 
 * \brief
 * Reset MAC Rx clock domains of the device.
 *
 * \details 
 * '0': The MAC Rx clock domain is NOT reset
 * '1': The MAC Rx clock domain is reset 
 * Note: MAC_RX_RST is NOT a one-shot operation. The MAC Rx clock domain
 * remains reset until a '0' is written to MAC_RX_RST.
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_DEV_RST_CTRL . MAC_RX_RST
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_DEV_RST_CTRL_MAC_RX_RST  VTSS_BIT(0)


/** 
 * \brief Port Proetection Configuration Register
 *
 * \details
 * When port protedction is enabled the device will snoop and transmit data
 * destined for another device 10G identified by port_protect_id.
 *
 * Register: \a DEV10G:DEV_CFG_STATUS:DEV_PORT_PROTECT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_CFG_STATUS_DEV_PORT_PROTECT(target)  VTSS_IOREG(target,0x61)

/** 
 * \brief
 * Indicates from which port egress data must be copied and transmitted at
 * this Ethernet port.
 * The port from which egress data is copied must always be a port that is
 * closer to the ASM. I.e. DEV(X) may be configured to snoop egress data
 * destined for DEV(X+n), where DEV(X+n) is closer to the ASM - but NOT
 * vice versa.
 *
 * \details 
 * 0: Reserved
 * 1: Egress data destined for DEV(1) is also transmitted by this device.
 * 2: Egress data destined for DEV(2) is also transmitted by this device.
 * ..
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_DEV_PORT_PROTECT . PORT_PROTECT_ID
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_DEV_PORT_PROTECT_PORT_PROTECT_ID(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_DEV10G_DEV_CFG_STATUS_DEV_PORT_PROTECT_PORT_PROTECT_ID     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_DEV10G_DEV_CFG_STATUS_DEV_PORT_PROTECT_PORT_PROTECT_ID(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * Enables snooping of egress data from another port. The port from which
 * egress data is copied and transmitted at the Ethernet port is determined
 * by the PORT_PROTECT_ID configuration.
 *
 * \details 
 * '0': Port protection is disabled.
 * '1': Port protection is enabled.
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_DEV_PORT_PROTECT . PORT_PROTECT_ENA
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_DEV_PORT_PROTECT_PORT_PROTECT_ENA  VTSS_BIT(0)


/** 
 * \brief Interrupt Source Register
 *
 * \details
 * Interrupt Source Register
 *
 * Register: \a DEV10G:DEV_CFG_STATUS:INTR
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_CFG_STATUS_INTR(target)  VTSS_IOREG(target,0x65)

/** 
 * \brief
 * PTP egress counter/ID register update
 *
 * \details 
 * 0 = no indication
 * 1 = active indication
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_INTR . PTP_UPDT_INTR
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_INTR_PTP_UPDT_INTR  VTSS_BIT(7)

/** 
 * \brief
 * PCS1G: New Page Event received by ANEG
 *
 * \details 
 * 0 = no indication
 * 1 = active indication
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_INTR . AN_PAGE_RX_INTR
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_INTR_AN_PAGE_RX_INTR  VTSS_BIT(4)

/** 
 * \brief
 * PCS1G only: ANEG - Link status has changed to up
 *
 * \details 
 * 0 = no indication
 * 1 = active indication
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_INTR . AN_LINK_UP_INTR
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_INTR_AN_LINK_UP_INTR  VTSS_BIT(3)

/** 
 * \brief
 * PCS1G only: ANEG - Link status has changed to down
 *
 * \details 
 * 0 = no indication
 * 1 = active indication
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_INTR . AN_LINK_DWN_INTR
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_INTR_AN_LINK_DWN_INTR  VTSS_BIT(2)

/** 
 * \brief
 * Link status is up (source: PCS1G, PCS10G or PCS2x6G depending on current
 * DEV10G mode)
 *
 * \details 
 * 0 = no indication
 * 1 = active indication
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_INTR . LINK_UP_INTR
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_INTR_LINK_UP_INTR  VTSS_BIT(1)

/** 
 * \brief
 * Link status is down (source: PCS1G, PCS10G or PCS2x6G depending on
 * current DEV10G mode)
 *
 * \details 
 * 0 = no indication
 * 1 = active indication
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_INTR . LINK_DWN_INTR
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_INTR_LINK_DWN_INTR  VTSS_BIT(0)


/** 
 * \brief Interrupt Enable Register
 *
 * \details
 * Interrupt Enable Register
 *
 * Register: \a DEV10G:DEV_CFG_STATUS:INTR_ENA
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_CFG_STATUS_INTR_ENA(target)  VTSS_IOREG(target,0x66)

/** 
 * \brief
 * PTP egress counter/ID register update
 *
 * \details 
 * 0 = interrupt inactive
 * 1 = interrupt active
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_INTR_ENA . PTP_UPDT_INTR_ENA
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_INTR_ENA_PTP_UPDT_INTR_ENA  VTSS_BIT(7)

/** 
 * \brief
 * PCS1G: New Page Event received by ANEG
 *
 * \details 
 * 0 = interrupt inactive
 * 1 = interrupt active
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_INTR_ENA . AN_PAGE_RX_INTR_ENA
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_INTR_ENA_AN_PAGE_RX_INTR_ENA  VTSS_BIT(4)

/** 
 * \brief
 * PCS1G only: ANEG - Link status has changed to up
 *
 * \details 
 * 0 = interrupt inactive
 * 1 = interrupt active
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_INTR_ENA . AN_LINK_UP_INTR_ENA
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_INTR_ENA_AN_LINK_UP_INTR_ENA  VTSS_BIT(3)

/** 
 * \brief
 * PCS1G only: ANEG - Link status has changed to down
 *
 * \details 
 * 0 = interrupt inactive
 * 1 = interrupt active
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_INTR_ENA . AN_LINK_DWN_INTR_ENA
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_INTR_ENA_AN_LINK_DWN_INTR_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Link status is up (source: PCS1G, PCS10G or PCS2x6G depending on current
 * DEV10G mode)
 *
 * \details 
 * 0 = interrupt inactive
 * 1 = interrupt active
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_INTR_ENA . LINK_UP_INTR_ENA
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_INTR_ENA_LINK_UP_INTR_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Link status is down (source: PCS1G, PCS10G or PCS2x6G depending on
 * current DEV10G mode)
 *
 * \details 
 * 0 = interrupt inactive
 * 1 = interrupt active
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_INTR_ENA . LINK_DWN_INTR_ENA
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_INTR_ENA_LINK_DWN_INTR_ENA  VTSS_BIT(0)


/** 
 * \brief Interrupt Status Register
 *
 * \details
 * Interrupt Status Register
 *
 * Register: \a DEV10G:DEV_CFG_STATUS:INTR_IDENT
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_CFG_STATUS_INTR_IDENT(target)  VTSS_IOREG(target,0x67)

/** 
 * \brief
 * PTP egress counter/ID register update
 *
 * \details 
 * 0 = no indication
 * 1 = active indication
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_INTR_IDENT . PTP_UPDT_INTR_IDENT
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_INTR_IDENT_PTP_UPDT_INTR_IDENT  VTSS_BIT(7)

/** 
 * \brief
 * PCS1G: New Page Event received by ANEG
 *
 * \details 
 * 0 = no indication
 * 1 = active indication
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_INTR_IDENT . AN_PAGE_RX_INTR_IDENT
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_INTR_IDENT_AN_PAGE_RX_INTR_IDENT  VTSS_BIT(4)

/** 
 * \brief
 * PCS1G only: ANEG - Link status has changed to up
 *
 * \details 
 * 0 = no indication
 * 1 = active indication
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_INTR_IDENT . AN_LINK_UP_INTR_IDENT
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_INTR_IDENT_AN_LINK_UP_INTR_IDENT  VTSS_BIT(3)

/** 
 * \brief
 * PCS1G only: ANEG - Link status has changed to down
 *
 * \details 
 * 0 = no indication
 * 1 = active indication
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_INTR_IDENT . AN_LINK_DWN_INTR_IDENT
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_INTR_IDENT_AN_LINK_DWN_INTR_IDENT  VTSS_BIT(2)

/** 
 * \brief
 * Link status is up (source: PCS1G, PCS10G or PCS2x6G depending on current
 * DEV10G mode)
 *
 * \details 
 * 0 = no indication
 * 1 = active indication
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_INTR_IDENT . LINK_UP_INTR_IDENT
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_INTR_IDENT_LINK_UP_INTR_IDENT  VTSS_BIT(1)

/** 
 * \brief
 * Link status is down (source: PCS1G, PCS10G or PCS2x6G depending on
 * current DEV10G mode)
 *
 * \details 
 * 0 = no indication
 * 1 = active indication
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_INTR_IDENT . LINK_DWN_INTR_IDENT
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_INTR_IDENT_LINK_DWN_INTR_IDENT  VTSS_BIT(0)


/** 
 * \brief Precision Time Protocol - Configuration Register
 *
 * \details
 * Register: \a DEV10G:DEV_CFG_STATUS:DEV_PTP_CFG
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_CFG_STATUS_DEV_PTP_CFG(target)  VTSS_IOREG(target,0x6a)

/** 
 * \brief
 * Enable PTP support according IEEE 1588.
 *
 * \details 
 * '0':  PTP support disabled
 * '1':  PTP support enabled
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_DEV_PTP_CFG . PTP_CFG_ENA
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_DEV_PTP_CFG_PTP_CFG_ENA  VTSS_BIT(4)


/** 
 * \brief IEEE 1588 TX Sticky Bit Register
 *
 * \details
 * Sticky bits are set when ever a PTP TX register is updated
 *
 * Register: \a DEV10G:DEV_CFG_STATUS:DEV_PTP_TX_STICKY
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 */
#define VTSS_DEV10G_DEV_CFG_STATUS_DEV_PTP_TX_STICKY(target)  VTSS_IOREG(target,0x6b)

/** 
 * \brief
 * Indicates if a new time stamp was written to register.
 * Bit is cleared by writing a '1' to this position.
 *
 * \details 
 * '0': Time stamp register unchanged
 * '1': Time stamp register updated with time stamp
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_DEV_PTP_TX_STICKY . TX_PTP_TSTAMP_STICKY
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_DEV_PTP_TX_STICKY_TX_PTP_TSTAMP_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_DEV10G_DEV_CFG_STATUS_DEV_PTP_TX_STICKY_TX_PTP_TSTAMP_STICKY     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_DEV10G_DEV_CFG_STATUS_DEV_PTP_TX_STICKY_TX_PTP_TSTAMP_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Precision Time Protocol - TX Time Stamp Register
 *
 * \details
 * Register: \a DEV10G:DEV_CFG_STATUS:DEV_PTP_TX_TSTAMP
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 * @param ri Replicator: x_NUM_PTP_TIMESTAMPS (??), 0-2
 */
#define VTSS_DEV10G_DEV_CFG_STATUS_DEV_PTP_TX_TSTAMP(target,ri)  VTSS_IOREG(target,0x6c + (ri))

/** 
 * \brief
 * Time stamp value when the selcted frame has left the device.
 *
 * \details 
 * 30 bit unsigned time stamp value (unit: ns)
 *
 * Field: ::VTSS_DEV10G_DEV_CFG_STATUS_DEV_PTP_TX_TSTAMP . TX_TSTAMP
 */
#define  VTSS_F_DEV10G_DEV_CFG_STATUS_DEV_PTP_TX_TSTAMP_TX_TSTAMP(x)  VTSS_ENCODE_BITFIELD(x,0,30)
#define  VTSS_M_DEV10G_DEV_CFG_STATUS_DEV_PTP_TX_TSTAMP_TX_TSTAMP     VTSS_ENCODE_BITMASK(0,30)
#define  VTSS_X_DEV10G_DEV_CFG_STATUS_DEV_PTP_TX_TSTAMP_TX_TSTAMP(x)  VTSS_EXTRACT_BITFIELD(x,0,30)


/** 
 * \brief Precision Time Protocol -Frame ID Register
 *
 * \details
 * Register: \a DEV10G:DEV_CFG_STATUS:DEV_PTP_TX_ID
 *
 * @param target A \a ::vtss_target_DEV10G_e target
 * @param ri Replicator: x_NUM_PTP_TIMESTAMPS (??), 0-2
 */
#define VTSS_DEV10G_DEV_CFG_STATUS_DEV_PTP_TX_ID(target,ri)  VTSS_IOREG(target,0x6f + (ri))


#endif /* _VTSS_JAGUAR_REGS_DEV10G_H_ */
