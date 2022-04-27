#ifndef _VTSS_JAGUAR_REGS_REW_H_
#define _VTSS_JAGUAR_REGS_REW_H_

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
 * Target: \a REW
 *
 * Rewriter
 *
 ***********************************************************************/

/**
 * Register Group: \a REW:COMMON
 *
 * Common configuration and status for all rewriter ports
 */


/** 
 * \brief Common Control configuration
 *
 * \details
 * Register: \a REW:COMMON:COMMON_CTRL
 */
#define VTSS_REW_COMMON_COMMON_CTRL          VTSS_IOREG(VTSS_TO_REW,0xb900)

/** 
 * \brief
 * Setting this bit will force an initialization of the port configuration.
 * The bit will be cleared by HW when the initialization is done.
 *
 * \details 
 * 0: Table is ready
 * 1: Force config initialization
 *
 * Field: ::VTSS_REW_COMMON_COMMON_CTRL . FORCE_PORT_CFG_INIT
 */
#define  VTSS_F_REW_COMMON_COMMON_CTRL_FORCE_PORT_CFG_INIT  VTSS_BIT(1)


/** 
 * \brief Various configuration of service handling in rewriter module 
 *
 * \details
 * Register: \a REW:COMMON:SERVICE_CTRL
 */
#define VTSS_REW_COMMON_SERVICE_CTRL         VTSS_IOREG(VTSS_TO_REW,0xb901)

/** 
 * \brief
 * ETYPE to use for ITAG insertion and recognition
 *
 * \details 
 * Field: ::VTSS_REW_COMMON_SERVICE_CTRL . ITAG_TPI_ETYPE
 */
#define  VTSS_F_REW_COMMON_SERVICE_CTRL_ITAG_TPI_ETYPE(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_REW_COMMON_SERVICE_CTRL_ITAG_TPI_ETYPE     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_REW_COMMON_SERVICE_CTRL_ITAG_TPI_ETYPE(x)  VTSS_EXTRACT_BITFIELD(x,16,16)


/** 
 * \brief Configure mirror probes
 *
 * \details
 * Register: \a REW:COMMON:MIRROR_PROBE_CFG
 *
 * @param ri Replicator: x_NUM_MIRROR_PROBES (??), 0-2
 */
#define VTSS_REW_COMMON_MIRROR_PROBE_CFG(ri)  VTSS_IOREG(VTSS_TO_REW,0xb902 + (ri))

/** 
 * \brief
 * The TX port for the mirror_probe (From where rewrite configuration is
 * taken).
 * 
 * (The configuration of this register must be consistant with the
 * configuration of the mirror registers in the Arbiter.)
 *
 * \details 
 * 0: Port 0
 * 1: Port 1
 * . . .
 * n: Port n
 *
 * Field: ::VTSS_REW_COMMON_MIRROR_PROBE_CFG . MIRROR_TX_PORT
 */
#define  VTSS_F_REW_COMMON_MIRROR_PROBE_CFG_MIRROR_TX_PORT(x)  VTSS_ENCODE_BITFIELD(x,20,8)
#define  VTSS_M_REW_COMMON_MIRROR_PROBE_CFG_MIRROR_TX_PORT     VTSS_ENCODE_BITMASK(20,8)
#define  VTSS_X_REW_COMMON_MIRROR_PROBE_CFG_MIRROR_TX_PORT(x)  VTSS_EXTRACT_BITFIELD(x,20,8)

/** 
 * \brief
 * The user priority in the remote mirror VLAN tag..
 *
 * \details 
 * 0: User priority 0
 * 1: User priority 1
 * . . .
 * 7: User priority 7
 *
 * Field: ::VTSS_REW_COMMON_MIRROR_PROBE_CFG . MIRROR_VLAN_UPRIO
 */
#define  VTSS_F_REW_COMMON_MIRROR_PROBE_CFG_MIRROR_VLAN_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,17,3)
#define  VTSS_M_REW_COMMON_MIRROR_PROBE_CFG_MIRROR_VLAN_UPRIO     VTSS_ENCODE_BITMASK(17,3)
#define  VTSS_X_REW_COMMON_MIRROR_PROBE_CFG_MIRROR_VLAN_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,17,3)

/** 
 * \brief
 * The CFI bit in the remote mirror VLAN tag..
 *
 * \details 
 * 0: CFI bit = 0
 * 1: CFI bit = 1.
 *
 * Field: ::VTSS_REW_COMMON_MIRROR_PROBE_CFG . MIRROR_VLAN_CFI
 */
#define  VTSS_F_REW_COMMON_MIRROR_PROBE_CFG_MIRROR_VLAN_CFI  VTSS_BIT(16)

/** 
 * \brief
 * The VID of the remote mirror VLAN tag..
 *
 * \details 
 * Field: ::VTSS_REW_COMMON_MIRROR_PROBE_CFG . MIRROR_VLAN_VID
 */
#define  VTSS_F_REW_COMMON_MIRROR_PROBE_CFG_MIRROR_VLAN_VID(x)  VTSS_ENCODE_BITFIELD(x,4,12)
#define  VTSS_M_REW_COMMON_MIRROR_PROBE_CFG_MIRROR_VLAN_VID     VTSS_ENCODE_BITMASK(4,12)
#define  VTSS_X_REW_COMMON_MIRROR_PROBE_CFG_MIRROR_VLAN_VID(x)  VTSS_EXTRACT_BITFIELD(x,4,12)

/** 
 * \brief
 * The tag protocol identifier(TPI) for the remote mirror VLAN tag.
 *
 * \details 
 * 0: C-TAG (tag using ethertype 0x8100)
 * 1: S-TAG (tag using ethertype 0x88a8)
 * 2: Custom S-TAG (tag using PORT_TAG_DEFAULT::PORT_TPI_ETYPE)
 * 3: C-TAG if received frame was untagged/C-Tagged, otherwise use custom
 * S-TAG.
 *
 * Field: ::VTSS_REW_COMMON_MIRROR_PROBE_CFG . MIRROR_VLAN_TPI
 */
#define  VTSS_F_REW_COMMON_MIRROR_PROBE_CFG_MIRROR_VLAN_TPI(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_REW_COMMON_MIRROR_PROBE_CFG_MIRROR_VLAN_TPI     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_REW_COMMON_MIRROR_PROBE_CFG_MIRROR_VLAN_TPI(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Enable mirror uprio.
 *
 * \details 
 * 0: Disable MIRROR_VLAN_UPRIO (use mirror traffic uprio).
 * 1: Enable use of MIRROR_VLAN_UPRIO.
 *
 * Field: ::VTSS_REW_COMMON_MIRROR_PROBE_CFG . USE_MIRROR_UPRIO_ENA
 */
#define  VTSS_F_REW_COMMON_MIRROR_PROBE_CFG_USE_MIRROR_UPRIO_ENA  VTSS_BIT(1)

/** 
 * \brief
 * The mirror port mode.
 *
 * \details 
 * 0: Local mirror port.
 * 1: Remote VLAN. A mirror tag will be added as outmost tag.
 *
 * Field: ::VTSS_REW_COMMON_MIRROR_PROBE_CFG . REMOTE_MIRROR_ENA
 */
#define  VTSS_F_REW_COMMON_MIRROR_PROBE_CFG_REMOTE_MIRROR_ENA  VTSS_BIT(0)


/** 
 * \brief Remap table of DSCP values.
 *
 * \details
 * Register: \a REW:COMMON:DSCP_REMAP
 *
 * @param ri Register: DSCP_REMAP (??), 0-63
 */
#define VTSS_REW_COMMON_DSCP_REMAP(ri)       VTSS_IOREG(VTSS_TO_REW,0xb905 + (ri))

/** 
 * \brief
 * Full one to one DSCP remapping table common for all ports.
 *
 * \details 
 * Field: ::VTSS_REW_COMMON_DSCP_REMAP . DSCP_REMAP
 */
#define  VTSS_F_REW_COMMON_DSCP_REMAP_DSCP_REMAP(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_REW_COMMON_DSCP_REMAP_DSCP_REMAP     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_REW_COMMON_DSCP_REMAP_DSCP_REMAP(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief Rewriter router leg configuration.
 *
 * \details
 * Register: \a REW:COMMON:RLEG_CFG_0
 */
#define VTSS_REW_COMMON_RLEG_CFG_0           VTSS_IOREG(VTSS_TO_REW,0xb945)

/** 
 * \brief
 * Router Leg MAC address used for IP routing (lease significant bits).
 * 
 * Must be set identical to ANA_L3::RLEG_MAC_LSB
 *
 * \details 
 * Field: ::VTSS_REW_COMMON_RLEG_CFG_0 . RLEG_MAC_LSB
 */
#define  VTSS_F_REW_COMMON_RLEG_CFG_0_RLEG_MAC_LSB(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_REW_COMMON_RLEG_CFG_0_RLEG_MAC_LSB     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_REW_COMMON_RLEG_CFG_0_RLEG_MAC_LSB(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief Rewriter router leg configuration.
 *
 * \details
 * Register: \a REW:COMMON:RLEG_CFG_1
 */
#define VTSS_REW_COMMON_RLEG_CFG_1           VTSS_IOREG(VTSS_TO_REW,0xb946)

/** 
 * \brief
 * Specifies how Router Leg MAC addresses are specified
 * 
 * Must be set identical to ANA_L3::RLEG_MAC_TYPE_SEL
 *
 * \details 
 * 0: RLEG_MAC := RLEG_MAC_MSB(23:0) & (RLEG_MAC_LSB(23:0) + VMID(7:0) mod
 * 2**24)
 * 1: RLEG_MAC := RLEG_MAC_MSB(23:0) & (RLEG_MAC_LSB(23:0) + VID(11:0) mod
 * 2**24)
 * Others: RLEG_MAC := RLEG_MAC_MSB(23:0) & RLEG_MAC_LSB(23:0)	# common
 * addr for all VLANs
 *
 * Field: ::VTSS_REW_COMMON_RLEG_CFG_1 . RLEG_MAC_TYPE_SEL
 */
#define  VTSS_F_REW_COMMON_RLEG_CFG_1_RLEG_MAC_TYPE_SEL(x)  VTSS_ENCODE_BITFIELD(x,24,2)
#define  VTSS_M_REW_COMMON_RLEG_CFG_1_RLEG_MAC_TYPE_SEL     VTSS_ENCODE_BITMASK(24,2)
#define  VTSS_X_REW_COMMON_RLEG_CFG_1_RLEG_MAC_TYPE_SEL(x)  VTSS_EXTRACT_BITFIELD(x,24,2)

/** 
 * \brief
 * MSB part of SMAC used for IP routing
 * 
 * Must be set identical to ANA_L3::RLEG_MAC_MSB
 *
 * \details 
 * Field: ::VTSS_REW_COMMON_RLEG_CFG_1 . RLEG_MAC_MSB
 */
#define  VTSS_F_REW_COMMON_RLEG_CFG_1_RLEG_MAC_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_REW_COMMON_RLEG_CFG_1_RLEG_MAC_MSB     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_REW_COMMON_RLEG_CFG_1_RLEG_MAC_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief Mac addresses for MacInMac encapsulation.
 *
 * \details
 * Service action selects one of these 32 addresses as backbone SMAC.
 *
 * Register: \a REW:COMMON:ITAG_SMAC_HI
 *
 * @param ri Register: ITAG_SMAC_HI (??), 0-31
 */
#define VTSS_REW_COMMON_ITAG_SMAC_HI(ri)     VTSS_IOREG(VTSS_TO_REW,0xb947 + (ri))

/** 
 * \brief
 * Most significant 16 bits of provider source addresses
 *
 * \details 
 * Field: ::VTSS_REW_COMMON_ITAG_SMAC_HI . ITAG_SMAC_HIGH
 */
#define  VTSS_F_REW_COMMON_ITAG_SMAC_HI_ITAG_SMAC_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_REW_COMMON_ITAG_SMAC_HI_ITAG_SMAC_HIGH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_REW_COMMON_ITAG_SMAC_HI_ITAG_SMAC_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Mac addresses for MacInMac encapsulation.
 *
 * \details
 * Service action selects one of these 32 addresses as backbone SMAC.
 *
 * Register: \a REW:COMMON:ITAG_SMAC_LO
 *
 * @param ri Register: ITAG_SMAC_LO (??), 0-31
 */
#define VTSS_REW_COMMON_ITAG_SMAC_LO(ri)     VTSS_IOREG(VTSS_TO_REW,0xb967 + (ri))


/** 
 * \brief Control register for counter events
 *
 * \details
 * Register: \a REW:COMMON:CNT_CTRL
 */
#define VTSS_REW_COMMON_CNT_CTRL             VTSS_IOREG(VTSS_TO_REW,0xb987)

/** 
 * \brief
 * If 1 events from all ports are counted. If 0 the port is selected by the
 * 'EVENT_CNT_PORT' register
 *
 * \details 
 * 0: Count events from the port selected by 'EVENT_CNT_PORT'
 * 1: Count events from all ports
 *
 * Field: ::VTSS_REW_COMMON_CNT_CTRL . EVENT_CNT_ALL
 */
#define  VTSS_F_REW_COMMON_CNT_CTRL_EVENT_CNT_ALL  VTSS_BIT(31)

/** 
 * \brief
 * Select from which source port events are counted.
 *
 * \details 
 * 'n': Count events from source port n
 *
 * Field: ::VTSS_REW_COMMON_CNT_CTRL . EVENT_CNT_PORT
 */
#define  VTSS_F_REW_COMMON_CNT_CTRL_EVENT_CNT_PORT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_REW_COMMON_CNT_CTRL_EVENT_CNT_PORT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_REW_COMMON_CNT_CTRL_EVENT_CNT_PORT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Controls the common sticky event counter.
 *
 * \details
 * Register: \a REW:COMMON:STICKY_EVENT_COUNT
 */
#define VTSS_REW_COMMON_STICKY_EVENT_COUNT   VTSS_IOREG(VTSS_TO_REW,0xb988)


/** 
 * \brief Mask for the STICKY_EVENT_COUNT
 *
 * \details
 * Register: \a REW:COMMON:STICKY_EVENT_CNT_MASK_CFG
 */
#define VTSS_REW_COMMON_STICKY_EVENT_CNT_MASK_CFG  VTSS_IOREG(VTSS_TO_REW,0xb989)

/** 
 * \brief
 * Enable counting of the event
 *
 * \details 
 * 0: Sticky event will not be counted
 * 1: Sticky event will be counted
 *
 * Field: ::VTSS_REW_COMMON_STICKY_EVENT_CNT_MASK_CFG . IFH_BIP8_ERR_STICKY_MASK
 */
#define  VTSS_F_REW_COMMON_STICKY_EVENT_CNT_MASK_CFG_IFH_BIP8_ERR_STICKY_MASK  VTSS_BIT(0)


/** 
 * \brief Rewriter sticky bit register
 *
 * \details
 * Register: \a REW:COMMON:STICKY_EVENT
 */
#define VTSS_REW_COMMON_STICKY_EVENT         VTSS_IOREG(VTSS_TO_REW,0xb98a)

/** 
 * \brief
 * This bit is set if a IFH BIP8 error event occurs (IFH checksum error)
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_REW_COMMON_STICKY_EVENT . IFH_BIP8_ERR_STICKY
 */
#define  VTSS_F_REW_COMMON_STICKY_EVENT_IFH_BIP8_ERR_STICKY  VTSS_BIT(0)


/** 
 * \brief Ram integrity errors 
 *
 * \details
 * Register: \a REW:COMMON:RAM_INTEGRITY_ERR_STICKY
 */
#define VTSS_REW_COMMON_RAM_INTEGRITY_ERR_STICKY  VTSS_IOREG(VTSS_TO_REW,0xb98b)

/** 
 * \brief
 * Sticky bit asserted if SRAM integrity error detected.
 *
 * \details 
 * Bit(0) : Untag RAM
 * Bit(1) : Port Configuration RAM
 * Bit(2) : ES0 Action RAM
 * Bit(4:3) : Stats RAM
 *
 * Field: ::VTSS_REW_COMMON_RAM_INTEGRITY_ERR_STICKY . RAM_INTEGRITY_ERROR_STICKY
 */
#define  VTSS_F_REW_COMMON_RAM_INTEGRITY_ERR_STICKY_RAM_INTEGRITY_ERROR_STICKY(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_REW_COMMON_RAM_INTEGRITY_ERR_STICKY_RAM_INTEGRITY_ERROR_STICKY     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_REW_COMMON_RAM_INTEGRITY_ERR_STICKY_RAM_INTEGRITY_ERROR_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief VSTAX global CPU configuration
 *
 * \details
 * Using ARB:CFG_STATUS:STACK_CFG.CPU_TO_GCPU_MASK selected local CPU
 * queues can be redirected to a GCPU port.
 * 
 * The GCPU port, which such frames are forwarded to, is configured in this
 * register.
 *
 * Register: \a REW:COMMON:VSTAX_GCPU_CFG
 */
#define VTSS_REW_COMMON_VSTAX_GCPU_CFG       VTSS_IOREG(VTSS_TO_REW,0xb98d)

/** 
 * \brief
 * CPU QUEUE to be used as destination queue for global CPU transmission.
 * This is configurable for each possible CPU queue.
 *
 * \details 
 * 0: Remote CPU QUEUE 0
 * 1: Remote CPU QUEUE 1
 * ... 
 * n: Remote CPU QUEUE n.
 *
 * Field: ::VTSS_REW_COMMON_VSTAX_GCPU_CFG . GCPU_UPSPN
 */
#define  VTSS_F_REW_COMMON_VSTAX_GCPU_CFG_GCPU_UPSPN(x)  VTSS_ENCODE_BITFIELD(x,8,5)
#define  VTSS_M_REW_COMMON_VSTAX_GCPU_CFG_GCPU_UPSPN     VTSS_ENCODE_BITMASK(8,5)
#define  VTSS_X_REW_COMMON_VSTAX_GCPU_CFG_GCPU_UPSPN(x)  VTSS_EXTRACT_BITFIELD(x,8,5)

/** 
 * \brief
 * UPSID to be used as destination for global CPU transmission
 *
 * \details 
 * 0: UPSID 0
 * 1: UPSID 1
 * ... 
 * n: UPSID n.
 *
 * Field: ::VTSS_REW_COMMON_VSTAX_GCPU_CFG . GCPU_UPSID
 */
#define  VTSS_F_REW_COMMON_VSTAX_GCPU_CFG_GCPU_UPSID(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_REW_COMMON_VSTAX_GCPU_CFG_GCPU_UPSID     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_REW_COMMON_VSTAX_GCPU_CFG_GCPU_UPSID(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief VSTAX Configuration per logical Stacking port
 *
 * \details
 * Register: \a REW:COMMON:VSTAX_PORT_GRP_CFG
 *
 * @param ri Register: VSTAX_PORT_GRP_CFG (??), 0-1
 */
#define VTSS_REW_COMMON_VSTAX_PORT_GRP_CFG(ri)  VTSS_IOREG(VTSS_TO_REW,0xb98e + (ri))

/** 
 * \brief
 * Link TTL value
 *
 * \details 
 * Field: ::VTSS_REW_COMMON_VSTAX_PORT_GRP_CFG . VSTAX_TTL
 */
#define  VTSS_F_REW_COMMON_VSTAX_PORT_GRP_CFG_VSTAX_TTL(x)  VTSS_ENCODE_BITFIELD(x,4,5)
#define  VTSS_M_REW_COMMON_VSTAX_PORT_GRP_CFG_VSTAX_TTL     VTSS_ENCODE_BITMASK(4,5)
#define  VTSS_X_REW_COMMON_VSTAX_PORT_GRP_CFG_VSTAX_TTL(x)  VTSS_EXTRACT_BITFIELD(x,4,5)

/** 
 * \brief
 * Change priority for learn all frames.
 *
 * \details 
 * 0 : Disable
 * 1: Enable (Learn all are send with highest priority)
 *
 * Field: ::VTSS_REW_COMMON_VSTAX_PORT_GRP_CFG . VSTAX_LRN_ALL_HP_ENA
 */
#define  VTSS_F_REW_COMMON_VSTAX_PORT_GRP_CFG_VSTAX_LRN_ALL_HP_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Control whether forwarding modes specific to VStaX AF shall be
 * translated to BF forwarding modes.
 * 
 * If set to 0, the following translation will be performed:
 * fwd_logical -> fwd_llookup
 * fwd_mc -> fwd_llookup
 * 
 * If set to 1, no translation will be performed.
 * 
 * Translation is only required for AF switches operating in a BF stack.

 *
 * \details 
 * 0: Perform fwd_mode translation.
 * 1: Do not perform fwd_mode translation.
 *
 * Field: ::VTSS_REW_COMMON_VSTAX_PORT_GRP_CFG . VSTAX_MODE
 */
#define  VTSS_F_REW_COMMON_VSTAX_PORT_GRP_CFG_VSTAX_MODE  VTSS_BIT(0)


/** 
 * \brief Various Host Mode control
 *
 * \details
 * Register: \a REW:COMMON:HM_CTRL
 */
#define VTSS_REW_COMMON_HM_CTRL              VTSS_IOREG(VTSS_TO_REW,0xb990)

/** 
 * \brief
 * Controls how the 192 host mode queues are assigned to the two Host
 * Interfaces operating in host mode (HMDA and HMDB), and how these queues
 * are mapped into Virtual Ports (VPs) and priorities within a Virtual
 * Port.
 * 
 * The configuration of HM_MODE must be consistent with QSIF_HM_CTRL.
 * 
 * The Host Interface device numbers, are configured using ETH_DEV_HMDA
 * (HMDA) and ETH_DEV_HMDB (HMDB).

 *
 * \details 
 * 0: Mode 12x8 - 12 VPs each with 8 priorities on HMDA and HMDB
 * 1: Mode 24x4 - 24 VPs each with 4 priorities on HMDA and HMDB
 * 2: Mode 24x8 - 24 VPs each with 8 priorities on HMDA
 * 3: Mode 48x4 - 48 VPs each with 4 priorities on HMDA
 *
 * Field: ::VTSS_REW_COMMON_HM_CTRL . HM_MODE
 */
#define  VTSS_F_REW_COMMON_HM_CTRL_HM_MODE(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_REW_COMMON_HM_CTRL_HM_MODE     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_REW_COMMON_HM_CTRL_HM_MODE(x)  VTSS_EXTRACT_BITFIELD(x,0,3)

/** 
 * \brief
 * Ethernet device, which is used as Host Mode Device A (HMDA).
 * 
 * This information must be configured consistently among the following
 * registers:
 * ANA_CL::HM_CFG.HMD_PORT
 * ANA_AC::HM_CFG.HMD_PORT
 * ARB::HM_CFG.ETH_DEV_HMDA / ARB::HM_CFG.ETH_DEV_HMDB
 * SCH::QSIF_HM_CTRL.ETH_DEV_HMDA / SCH::QSIF_HM_CTRL.ETH_DEV_HMDB
 * REW::HM_CTRL.ETH_DEV_HMDA / REW::HM_CTRL.ETH_DEV_HMDB
 * 
 * This refers to one of the 10G ports, i.e. port 27-30.
 * 
 * Must be configured during startup, i.e. before the Ethernet port is up.
 *
 * \details 
 * Field: ::VTSS_REW_COMMON_HM_CTRL . ETH_DEV_HMDA
 */
#define  VTSS_F_REW_COMMON_HM_CTRL_ETH_DEV_HMDA(x)  VTSS_ENCODE_BITFIELD(x,8,5)
#define  VTSS_M_REW_COMMON_HM_CTRL_ETH_DEV_HMDA     VTSS_ENCODE_BITMASK(8,5)
#define  VTSS_X_REW_COMMON_HM_CTRL_ETH_DEV_HMDA(x)  VTSS_EXTRACT_BITFIELD(x,8,5)

/** 
 * \brief
 * Ethernet device, which is used as Host Mode Device B (HMDB).
 * 
 * This information must be configured consistently among the following
 * registers:
 * ANA_CL::HM_CFG.HMD_PORT
 * ANA_AC::HM_CFG.HMD_PORT
 * ARB::HM_CFG.ETH_DEV_HMDA / ARB::HM_CFG.ETH_DEV_HMDB
 * SCH::QSIF_HM_CTRL.ETH_DEV_HMDA / SCH::QSIF_HM_CTRL.ETH_DEV_HMDB
 * REW::HM_CTRL.ETH_DEV_HMDA / REW::HM_CTRL.ETH_DEV_HMDB
 * 
 * This refers to one of the 10G ports, i.e. port 27-30.
 * 
 * Must be configured during startup, i.e. before the Ethernet port is up.
 *
 * \details 
 * Field: ::VTSS_REW_COMMON_HM_CTRL . ETH_DEV_HMDB
 */
#define  VTSS_F_REW_COMMON_HM_CTRL_ETH_DEV_HMDB(x)  VTSS_ENCODE_BITFIELD(x,16,5)
#define  VTSS_M_REW_COMMON_HM_CTRL_ETH_DEV_HMDB     VTSS_ENCODE_BITMASK(16,5)
#define  VTSS_X_REW_COMMON_HM_CTRL_ETH_DEV_HMDB(x)  VTSS_EXTRACT_BITFIELD(x,16,5)


/** 
 * \brief Various Host Mode control
 *
 * \details
 * Register: \a REW:COMMON:HIH_CTRL
 *
 * @param ri Register: HIH_CTRL (??), 0-3
 */
#define VTSS_REW_COMMON_HIH_CTRL(ri)         VTSS_IOREG(VTSS_TO_REW,0xb991 + (ri))

/** 
 * \brief
 * Set the format of the Logical Port ID (LPID)
 * 
 * (Value put into the HIH will be determined by HIH_FRM_LPID)
 *
 * \details 
 * 0: Egress Host Port number (According to HM_CTRL::HM_MODE)
 * 1: Ingress physical port number (Used when port is not used as host mode
 * port)
 *
 * Field: ::VTSS_REW_COMMON_HIH_CTRL . HIH_LPID_MODE
 */
#define  VTSS_F_REW_COMMON_HIH_CTRL_HIH_LPID_MODE  VTSS_BIT(6)

/** 
 * \brief
 * Enables prepending of Host Interface Header (HIH) on the port
 *
 * \details 
 * Field: ::VTSS_REW_COMMON_HIH_CTRL . HIH_ENA
 */
#define  VTSS_F_REW_COMMON_HIH_CTRL_HIH_ENA   VTSS_BIT(5)

/** 
 * \brief
 * Configure which value goes into the HIH.LPID field.
 *
 * \details 
 * 0: Set LPID according egress frame (Formatted by HIH_LPID_MODE)
 * 1: Set LPID to fixed default value (HIH_DEF_CFG)
 *
 * Field: ::VTSS_REW_COMMON_HIH_CTRL . HIH_FRM_LPID
 */
#define  VTSS_F_REW_COMMON_HIH_CTRL_HIH_FRM_LPID  VTSS_BIT(3)

/** 
 * \brief
 * Select source of the HIH.CL field.
 *
 * \details 
 * 0: Set HIH.CL to fixed default value = HIH_DEF_CL
 * 1: Set HIH.CL according to the egress frame classification.
 *
 * Field: ::VTSS_REW_COMMON_HIH_CTRL . HIH_FRM_CL
 */
#define  VTSS_F_REW_COMMON_HIH_CTRL_HIH_FRM_CL  VTSS_BIT(2)

/** 
 * \brief
 * Select the source of the HIH.FLAGS field.
 *
 * \details 
 * 0: Set HIH.FLAGS to fixed default value = HIH_DEF_FLAGS
 * 1: Set HIH.FLAGS to frame PCP and DEI bits.
 *
 * Field: ::VTSS_REW_COMMON_HIH_CTRL . HIH_FRM_FLAGS
 */
#define  VTSS_F_REW_COMMON_HIH_CTRL_HIH_FRM_FLAGS  VTSS_BIT(1)

/** 
 * \brief
 * Select the source of the HIH.CKSM field.
 *
 * \details 
 * 0: Set HIH.CKSM to fixed default value (HIH_DEF_CKSM)
 * 1: Calculate HIH.CKSM according to HIH contents.
 *
 * Field: ::VTSS_REW_COMMON_HIH_CTRL . HIH_AUTO_CKSM
 */
#define  VTSS_F_REW_COMMON_HIH_CTRL_HIH_AUTO_CKSM  VTSS_BIT(0)


/** 
 * \brief Default values for transmitted HIHS
 *
 * \details
 * Register: \a REW:COMMON:HIH_DEF_CFG
 *
 * @param ri Register: HIH_DEF_CFG (??), 0-3
 */
#define VTSS_REW_COMMON_HIH_DEF_CFG(ri)      VTSS_IOREG(VTSS_TO_REW,0xb995 + (ri))

/** 
 * \brief
 * Default value for the LPID field in transmitted HIH's.
 *
 * \details 
 * Field: ::VTSS_REW_COMMON_HIH_DEF_CFG . HIH_DEF_LPID
 */
#define  VTSS_F_REW_COMMON_HIH_DEF_CFG_HIH_DEF_LPID(x)  VTSS_ENCODE_BITFIELD(x,24,8)
#define  VTSS_M_REW_COMMON_HIH_DEF_CFG_HIH_DEF_LPID     VTSS_ENCODE_BITMASK(24,8)
#define  VTSS_X_REW_COMMON_HIH_DEF_CFG_HIH_DEF_LPID(x)  VTSS_EXTRACT_BITFIELD(x,24,8)

/** 
 * \brief
 * Default value for the CL field in transmitted HIH's.
 *
 * \details 
 * Field: ::VTSS_REW_COMMON_HIH_DEF_CFG . HIH_DEF_CL
 */
#define  VTSS_F_REW_COMMON_HIH_DEF_CFG_HIH_DEF_CL(x)  VTSS_ENCODE_BITFIELD(x,8,16)
#define  VTSS_M_REW_COMMON_HIH_DEF_CFG_HIH_DEF_CL     VTSS_ENCODE_BITMASK(8,16)
#define  VTSS_X_REW_COMMON_HIH_DEF_CFG_HIH_DEF_CL(x)  VTSS_EXTRACT_BITFIELD(x,8,16)

/** 
 * \brief
 * Default value for the FLAGS field in transmitted HIH's.
 *
 * \details 
 * Field: ::VTSS_REW_COMMON_HIH_DEF_CFG . HIH_DEF_FLAGS
 */
#define  VTSS_F_REW_COMMON_HIH_DEF_CFG_HIH_DEF_FLAGS(x)  VTSS_ENCODE_BITFIELD(x,4,4)
#define  VTSS_M_REW_COMMON_HIH_DEF_CFG_HIH_DEF_FLAGS     VTSS_ENCODE_BITMASK(4,4)
#define  VTSS_X_REW_COMMON_HIH_DEF_CFG_HIH_DEF_FLAGS(x)  VTSS_EXTRACT_BITFIELD(x,4,4)

/** 
 * \brief
 * Default value for the CKSM field in transmitted HIH's.
 *
 * \details 
 * Field: ::VTSS_REW_COMMON_HIH_DEF_CFG . HIH_DEF_CKSM
 */
#define  VTSS_F_REW_COMMON_HIH_DEF_CFG_HIH_DEF_CKSM(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_REW_COMMON_HIH_DEF_CFG_HIH_DEF_CKSM     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_REW_COMMON_HIH_DEF_CFG_HIH_DEF_CKSM(x)  VTSS_EXTRACT_BITFIELD(x,0,4)

/**
 * Register Group: \a REW:PORT
 *
 * Port based configuration and status for rewriter
 */


/** 
 * \brief Configures per port costum values for TAGs
 *
 * \details
 * Register: \a REW:PORT:PORT_TAG_DEFAULT
 *
 * @param gi Replicator: x_REW_FRONT_PORTS_TXM (??), 0-223
 */
#define VTSS_REW_PORT_PORT_TAG_DEFAULT(gi)   VTSS_IOREG_IX(VTSS_TO_REW,0x8000,gi,64,0,0)

/** 
 * \brief
 * Tag Protocol Identifier(TPI).
 *
 * \details 
 * Field: ::VTSS_REW_PORT_PORT_TAG_DEFAULT . PORT_TPI_ETYPE
 */
#define  VTSS_F_REW_PORT_PORT_TAG_DEFAULT_PORT_TPI_ETYPE(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_REW_PORT_PORT_TAG_DEFAULT_PORT_TPI_ETYPE     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_REW_PORT_PORT_TAG_DEFAULT_PORT_TPI_ETYPE(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Priority field in the TCI.
 *
 * \details 
 * Field: ::VTSS_REW_PORT_PORT_TAG_DEFAULT . PORT_TCI_PRIO
 */
#define  VTSS_F_REW_PORT_PORT_TAG_DEFAULT_PORT_TCI_PRIO(x)  VTSS_ENCODE_BITFIELD(x,13,3)
#define  VTSS_M_REW_PORT_PORT_TAG_DEFAULT_PORT_TCI_PRIO     VTSS_ENCODE_BITMASK(13,3)
#define  VTSS_X_REW_PORT_PORT_TAG_DEFAULT_PORT_TCI_PRIO(x)  VTSS_EXTRACT_BITFIELD(x,13,3)

/** 
 * \brief
 * Cfi field in the TCI.
 *
 * \details 
 * Field: ::VTSS_REW_PORT_PORT_TAG_DEFAULT . PORT_TCI_CFI
 */
#define  VTSS_F_REW_PORT_PORT_TAG_DEFAULT_PORT_TCI_CFI  VTSS_BIT(12)

/** 
 * \brief
 * VID field in the TCI.
 *
 * \details 
 * Field: ::VTSS_REW_PORT_PORT_TAG_DEFAULT . PORT_TCI_VID
 */
#define  VTSS_F_REW_PORT_PORT_TAG_DEFAULT_PORT_TCI_VID(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_REW_PORT_PORT_TAG_DEFAULT_PORT_TCI_VID     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_REW_PORT_PORT_TAG_DEFAULT_PORT_TCI_VID(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \brief Map drop precedence to drop eligible value
 *
 * \details
 * Drop precedence level for the frame is translated to a drop eligble
 * value 0 or 1, selecting between _DE0 or _DE1 mapping tables, when
 * translating the internal prio to TAG priorities. Mapped drop
 * precedence/internal priority is used when selected by TAG control
 * configuration (see TAG ctrl)
 *
 * Register: \a REW:PORT:PORT_DP_MAP
 *
 * @param gi Replicator: x_REW_FRONT_PORTS_TXM (??), 0-223
 */
#define VTSS_REW_PORT_PORT_DP_MAP(gi)        VTSS_IOREG_IX(VTSS_TO_REW,0x8000,gi,64,0,1)

/** 
 * \brief
 * Drop precedence N uses mapping table given by bit N in this field.
 *
 * \details 
 * Field: ::VTSS_REW_PORT_PORT_DP_MAP . DP
 */
#define  VTSS_F_REW_PORT_PORT_DP_MAP_DP(x)    VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_REW_PORT_PORT_DP_MAP_DP       VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_REW_PORT_PORT_DP_MAP_DP(x)    VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief Map internal priority to tagged priority.
 *
 * \details
 * Register: \a REW:PORT:PCP_MAP_DE0
 *
 * @param gi Replicator: x_REW_FRONT_PORTS_TXM (??), 0-223
 * @param ri Register: PCP_MAP_DE0 (??), 0-7
 */
#define VTSS_REW_PORT_PCP_MAP_DE0(gi,ri)     VTSS_IOREG_IX(VTSS_TO_REW,0x8000,gi,64,ri,2)

/** 
 * \brief
 * Map internal priority to UPRIO/PCP value in tags. This table used for DP
 * values mapped to 0 in PORT_DP_MAP.
 *
 * \details 
 * Field: ::VTSS_REW_PORT_PCP_MAP_DE0 . PCP_DE0
 */
#define  VTSS_F_REW_PORT_PCP_MAP_DE0_PCP_DE0(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_REW_PORT_PCP_MAP_DE0_PCP_DE0     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_REW_PORT_PCP_MAP_DE0_PCP_DE0(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Map internal priority to tagged priority.
 *
 * \details
 * Register: \a REW:PORT:PCP_MAP_DE1
 *
 * @param gi Replicator: x_REW_FRONT_PORTS_TXM (??), 0-223
 * @param ri Register: PCP_MAP_DE1 (??), 0-7
 */
#define VTSS_REW_PORT_PCP_MAP_DE1(gi,ri)     VTSS_IOREG_IX(VTSS_TO_REW,0x8000,gi,64,ri,10)

/** 
 * \brief
 * Map internal priority to UPRIO/PCP value in tags. This table used for DP
 * values mapped to 1 in PORT_DP_MAP.
 *
 * \details 
 * Field: ::VTSS_REW_PORT_PCP_MAP_DE1 . PCP_DE1
 */
#define  VTSS_F_REW_PORT_PCP_MAP_DE1_PCP_DE1(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_REW_PORT_PCP_MAP_DE1_PCP_DE1     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_REW_PORT_PCP_MAP_DE1_PCP_DE1(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Map internal priority to tag value.
 *
 * \details
 * Register: \a REW:PORT:DEI_MAP_DE0
 *
 * @param gi Replicator: x_REW_FRONT_PORTS_TXM (??), 0-223
 * @param ri Register: DEI_MAP_DE0 (??), 0-7
 */
#define VTSS_REW_PORT_DEI_MAP_DE0(gi,ri)     VTSS_IOREG_IX(VTSS_TO_REW,0x8000,gi,64,ri,18)

/** 
 * \brief
 * Map internal priority to CFI/DEI value in tags. This table used for DP
 * values mapped to 0 in PORT_DP_MAP.
 *
 * \details 
 * Field: ::VTSS_REW_PORT_DEI_MAP_DE0 . DEI_DE0
 */
#define  VTSS_F_REW_PORT_DEI_MAP_DE0_DEI_DE0  VTSS_BIT(0)


/** 
 * \brief Map internal priority to tag value.
 *
 * \details
 * Register: \a REW:PORT:DEI_MAP_DE1
 *
 * @param gi Replicator: x_REW_FRONT_PORTS_TXM (??), 0-223
 * @param ri Register: DEI_MAP_DE1 (??), 0-7
 */
#define VTSS_REW_PORT_DEI_MAP_DE1(gi,ri)     VTSS_IOREG_IX(VTSS_TO_REW,0x8000,gi,64,ri,26)

/** 
 * \brief
 * Map internal priority to CFI/DEI value in tags. This table used for DP
 * values mapped to 1 in PORT_DP_MAP.
 *
 * \details 
 * Field: ::VTSS_REW_PORT_DEI_MAP_DE1 . DEI_DE1
 */
#define  VTSS_F_REW_PORT_DEI_MAP_DE1_DEI_DE1  VTSS_BIT(0)


/** 
 * \brief Configure PORT tagging
 *
 * \details
 * Register: \a REW:PORT:TAG_CTRL
 *
 * @param gi Replicator: x_REW_FRONT_PORTS_TXM (??), 0-223
 */
#define VTSS_REW_PORT_TAG_CTRL(gi)           VTSS_IOREG_IX(VTSS_TO_REW,0x8000,gi,64,0,34)

/** 
 * \brief
 * Enable tagging
 *
 * \details 
 * 0: Port tagging disabled
 * 1: Tag all frames, except when VID=PORT_TCI_VID or 0
 * 2: Tag all frames, except when VID=0
 * 3: Tag all frames!
 *
 * Field: ::VTSS_REW_PORT_TAG_CTRL . TAG_MODE
 */
#define  VTSS_F_REW_PORT_TAG_CTRL_TAG_MODE(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_REW_PORT_TAG_CTRL_TAG_MODE     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_REW_PORT_TAG_CTRL_TAG_MODE(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

/** 
 * \brief
 * Select ETYPE for port tagging
 *
 * \details 
 * 0: Use 8100
 * 1: Use 88A8
 * 2: Use default from PORT_TPI_ETYPE
 * 3: If ingress ETYPE is C-TAG use 0x8100 otherwise use PORT_TPI_ETYPE, 
 *
 * Field: ::VTSS_REW_PORT_TAG_CTRL . TPI_SEL
 */
#define  VTSS_F_REW_PORT_TAG_CTRL_TPI_SEL(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_REW_PORT_TAG_CTRL_TPI_SEL     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_REW_PORT_TAG_CTRL_TPI_SEL(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Select VID in port tag
 *
 * \details 
 * 0: Ignore ES0 result. Always use CL-IVID. (Except if forwarding B-frame
 * to B-port, in this case use CL-BVID)
 * 1: Use ES0 only if it is a MacInMac action. Otherwise use CL-IVID.
 * (Except if forwarding B-frame to B-port, in this case use CL-BVID)
 * 2: Use ES0 action VID selection. (If no ES0 hit use CL-IVID)
 * 3: Reserved.
 *
 * Field: ::VTSS_REW_PORT_TAG_CTRL . VID_SEL
 */
#define  VTSS_F_REW_PORT_TAG_CTRL_VID_SEL(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_REW_PORT_TAG_CTRL_VID_SEL     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_REW_PORT_TAG_CTRL_VID_SEL(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * Select PCP/DEI fields in port tag
 *
 * \details 
 * 0: Use classified PCP/DEI values
 * 1: Use PCP/DEI values from ES0 action (If no ES0 HIT use classified
 * PCP/DEI)
 * 2: Use PCP/DEI from the default port settings in PORT_TAG_DEFAULT
 * 3: Use Mapped versions of classified QOS class and DP level
 *
 * Field: ::VTSS_REW_PORT_TAG_CTRL . QOS_SEL
 */
#define  VTSS_F_REW_PORT_TAG_CTRL_QOS_SEL(x)  VTSS_ENCODE_BITFIELD(x,6,2)
#define  VTSS_M_REW_PORT_TAG_CTRL_QOS_SEL     VTSS_ENCODE_BITMASK(6,2)
#define  VTSS_X_REW_PORT_TAG_CTRL_QOS_SEL(x)  VTSS_EXTRACT_BITFIELD(x,6,2)

/** 
 * \brief
 * If set, this port is a Backbone port
 *
 * \details 
 * Field: ::VTSS_REW_PORT_TAG_CTRL . IS_BPORT
 */
#define  VTSS_F_REW_PORT_TAG_CTRL_IS_BPORT    VTSS_BIT(8)


/** 
 * \brief Enable DSCP updates on the port.
 *
 * \details
 * Register: \a REW:PORT:DSCP_MAP
 *
 * @param gi Replicator: x_REW_FRONT_PORTS_TXM (??), 0-223
 */
#define VTSS_REW_PORT_DSCP_MAP(gi)           VTSS_IOREG_IX(VTSS_TO_REW,0x8000,gi,64,0,35)

/** 
 * \brief
 * Update DSCP with value from Analyzer, if allowed by analyzer.
 * (IFH.QOS.UPDATE_DSCP)
 *
 * \details 
 * 0 : No update
 * 1 : Allow update of DSCP
 *
 * Field: ::VTSS_REW_PORT_DSCP_MAP . DSCP_UPDATE_ENA
 */
#define  VTSS_F_REW_PORT_DSCP_MAP_DSCP_UPDATE_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Enable use of a shared DSCP remap table (DSCP_REMAP).
 * 
 * Map DSCP value resulting from DSCP_UPDATE_ENA and IFH.QOS.UPDATE_DSCP
 * using the common mapping table (DSCP_REMAP).
 *
 * \details 
 * 0 : No remapping.
 * 1 : Remap DSCP using DSCP remap table (DSCP_REMAP)
 *
 * Field: ::VTSS_REW_PORT_DSCP_MAP . DSCP_REMAP_ENA
 */
#define  VTSS_F_REW_PORT_DSCP_MAP_DSCP_REMAP_ENA  VTSS_BIT(2)

/**
 * Register Group: \a REW:PHYSPORT
 *
 * Port based configuration and status for rewriter
 */


/** 
 * \brief Controls stacking operation on port
 *
 * \details
 * Register: \a REW:PHYSPORT:VSTAX_CTRL
 *
 * @param gi Replicator: x_REW_PHYS_PORTS_TXM (??), 0-31
 */
#define VTSS_REW_PHYSPORT_VSTAX_CTRL(gi)     VTSS_IOREG_IX(VTSS_TO_REW,0xb999,gi,2,0,0)

/** 
 * \brief
 * Select logical stacking port (or stacking port group) membership
 *
 * \details 
 * 0: Stack A
 * 1: Stack B
 *
 * Field: ::VTSS_REW_PHYSPORT_VSTAX_CTRL . VSTAX_STACK_GRP_SEL
 */
#define  VTSS_F_REW_PHYSPORT_VSTAX_CTRL_VSTAX_STACK_GRP_SEL  VTSS_BIT(1)

/** 
 * \brief
 * Enable insertion of stacking header in frame
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_REW_PHYSPORT_VSTAX_CTRL . VSTAX_HDR_ENA
 */
#define  VTSS_F_REW_PHYSPORT_VSTAX_CTRL_VSTAX_HDR_ENA  VTSS_BIT(0)


/** 
 * \brief Controls frame drop of frames with invalid IFH checksum
 *
 * \details
 * Register: \a REW:PHYSPORT:PORT_CTRL
 *
 * @param gi Replicator: x_REW_PHYS_PORTS_TXM (??), 0-31
 */
#define VTSS_REW_PHYSPORT_PORT_CTRL(gi)      VTSS_IOREG_IX(VTSS_TO_REW,0xb999,gi,2,0,1)

/** 
 * \brief
 * Configure the rewriter IFH mode for the port.
 *
 * \details 
 * 0 : Normal mode. Frames are updated and IFH removed
 * 1: Keep IFH without modifications. Frames are not updated. IFH is kept
 * 2: Keep IFH and move DMAC and SMAC in front of IFH. Further Rewriter
 * adds a TAG with     ETYPE=8880 (Vitesse ETYPE). 
 *     Frames are updated with recalculated FCS. IFH is kept.
 * 3: reserved
 *
 * Field: ::VTSS_REW_PHYSPORT_PORT_CTRL . KEEP_IFH_SEL
 */
#define  VTSS_F_REW_PHYSPORT_PORT_CTRL_KEEP_IFH_SEL(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_REW_PHYSPORT_PORT_CTRL_KEEP_IFH_SEL     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_REW_PHYSPORT_PORT_CTRL_KEEP_IFH_SEL(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

/**
 * Register Group: \a REW:VMID
 *
 * Egress Mapped VLAN (EVMID) configuration
 */


/** 
 * \brief Rewriter router leg control
 *
 * \details
 * Register: \a REW:VMID:RLEG_CTRL
 *
 * @param gi Replicator: x_REW_NUM_VMID (??), 0-255
 */
#define VTSS_REW_VMID_RLEG_CTRL(gi)          VTSS_IOREG_IX(VTSS_TO_REW,0xb800,gi,1,0,0)

/** 
 * \brief
 * VID value assigned to this router leg
 *
 * \details 
 * Field: ::VTSS_REW_VMID_RLEG_CTRL . RLEG_EVID
 */
#define  VTSS_F_REW_VMID_RLEG_CTRL_RLEG_EVID(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_REW_VMID_RLEG_CTRL_RLEG_EVID     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_REW_VMID_RLEG_CTRL_RLEG_EVID(x)  VTSS_EXTRACT_BITFIELD(x,0,12)

/**
 * Register Group: \a REW:STAT_GLOBAL_CFG_ESDX
 *
 * Global configuration.
 */


/** 
 * \brief Global configuration register
 *
 * \details
 * Global configuration register
 *
 * Register: \a REW:STAT_GLOBAL_CFG_ESDX:STAT_GLOBAL_CFG
 *
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-3
 */
#define VTSS_REW_STAT_GLOBAL_CFG_ESDX_STAT_GLOBAL_CFG(ri)  VTSS_IOREG(VTSS_TO_REW,0xb9d9 + (ri))

/** 
 * \brief
 * This field is to configure the counters of all flows to count the byte
 * number or the frame number.
 *
 * \details 
 * '0': The frame number is counted.
 * '1': The byte number is counted.
 *
 * Field: ::VTSS_REW_STAT_GLOBAL_CFG_ESDX_STAT_GLOBAL_CFG . GLOBAL_CFG_CNT_BYTE
 */
#define  VTSS_F_REW_STAT_GLOBAL_CFG_ESDX_STAT_GLOBAL_CFG_GLOBAL_CFG_CNT_BYTE  VTSS_BIT(0)


/** 
 * \brief Event mask for counter.
 *
 * \details
 * Event mask for counter of all flows.
 *
 * Register: \a REW:STAT_GLOBAL_CFG_ESDX:STAT_GLOBAL_EVENT_MASK
 *
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-3
 */
#define VTSS_REW_STAT_GLOBAL_CFG_ESDX_STAT_GLOBAL_EVENT_MASK(ri)  VTSS_IOREG(VTSS_TO_REW,0xb9dd + (ri))

/** 
 * \brief
 * Define per counter what frame properties should be counted. A counter
 * will be updated if the corresponding frame type is set in its counter
 * mask.
 *  The following frame propeties can be enabled:
 *  0. Count frames with Drop Precedence level 0
 *   1. Count frames with Drop Precedence level 1
 *   2. Count frames with Drop Precedence level 2
 *   3. Count frames with Drop Precedence level 3
 *   4. Count multicast frames
 *
 * \details 
 * If an asserted event is masked out in the GLOBAL_EVENT_MASK, the
 * incoming frame should not be counted for all flows.
 *
 * Field: ::VTSS_REW_STAT_GLOBAL_CFG_ESDX_STAT_GLOBAL_EVENT_MASK . GLOBAL_EVENT_MASK
 */
#define  VTSS_F_REW_STAT_GLOBAL_CFG_ESDX_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_REW_STAT_GLOBAL_CFG_ESDX_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_REW_STAT_GLOBAL_CFG_ESDX_STAT_GLOBAL_EVENT_MASK_GLOBAL_EVENT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief Reset all statistic counter.
 *
 * \details
 * Writing '1' into this register will lead to the reset of STAT.
 *
 * Register: \a REW:STAT_GLOBAL_CFG_ESDX:STAT_RESET
 */
#define VTSS_REW_STAT_GLOBAL_CFG_ESDX_STAT_RESET  VTSS_IOREG(VTSS_TO_REW,0xb9e1)

/** 
 * \brief
 * Writing '1' into this bit will lead to the reset of STAT (counter and
 * configuration). This bit is auto-cleared after the reset is done.
 *
 * \details 
 * '0': No action
 * '1': to reset STAT (Bit is automatically cleared)
 *
 * Field: ::VTSS_REW_STAT_GLOBAL_CFG_ESDX_STAT_RESET . RESET
 */
#define  VTSS_F_REW_STAT_GLOBAL_CFG_ESDX_STAT_RESET_RESET  VTSS_BIT(0)

/**
 * Register Group: \a REW:STAT_CNT_CFG_ESDX
 *
 * Individual configuration.
 */


/** 
 * \brief Least significant 32 bits.
 *
 * \details
 * This register contains the least significant 32 bits of a counter.
 *
 * Register: \a REW:STAT_CNT_CFG_ESDX:STAT_LSB_CNT
 *
 * @param gi Replicator: x_STAT_NUM_FLW_ID (??), 0-4095
 * @param ri Replicator: x_STAT_CNT_PER_FLW_ID (??), 0-3
 */
#define VTSS_REW_STAT_CNT_CFG_ESDX_STAT_LSB_CNT(gi,ri)  VTSS_IOREG_IX(VTSS_TO_REW,0x0,gi,8,ri,0)


/** 
 * \brief Most significant 8 bits.
 *
 * \details
 * This register contains the most significant 8 bits of a counter.
 *
 * Register: \a REW:STAT_CNT_CFG_ESDX:STAT_MSB_CNT
 *
 * @param gi Replicator: x_STAT_NUM_FLW_ID (??), 0-4095
 * @param ri Replicator: x_STAT_CNT_MSB_PER_FLW_ID (??), 0-1
 */
#define VTSS_REW_STAT_CNT_CFG_ESDX_STAT_MSB_CNT(gi,ri)  VTSS_IOREG_IX(VTSS_TO_REW,0x0,gi,8,ri,4)

/** 
 * \brief
 * The field stores the value in the counters of a flow from bit 32 to the
 * most significant bit.
 *
 * \details 
 * Field: ::VTSS_REW_STAT_CNT_CFG_ESDX_STAT_MSB_CNT . MSB_CNT
 */
#define  VTSS_F_REW_STAT_CNT_CFG_ESDX_STAT_MSB_CNT_MSB_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_REW_STAT_CNT_CFG_ESDX_STAT_MSB_CNT_MSB_CNT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_REW_STAT_CNT_CFG_ESDX_STAT_MSB_CNT_MSB_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


#endif /* _VTSS_JAGUAR_REGS_REW_H_ */
