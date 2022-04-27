#ifndef _VTSS_JAGUAR_REGS_ANA_CL_2_H_
#define _VTSS_JAGUAR_REGS_ANA_CL_2_H_

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
 * Target: \a ANA_CL_2
 *
 * Classifier sub block of the Analyzer
 *
 ***********************************************************************/

/**
 * Register Group: \a ANA_CL_2:INTEGRETY_IDENT
 *
 * Memory Integrity error information
 */


/** 
 * \brief Memory integrity status registers
 *
 * \details
 * Register: \a ANA_CL_2:INTEGRETY_IDENT:INTEGRETY_IDENT
 */
#define VTSS_ANA_CL_2_INTEGRETY_IDENT_INTEGRETY_IDENT  VTSS_IOREG(VTSS_TO_ANA_CL,0x8c0)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_CL_2_INTEGRETY_IDENT_INTEGRETY_IDENT . MEM_DATA_2_DELAY_ERR
 */
#define  VTSS_F_ANA_CL_2_INTEGRETY_IDENT_INTEGRETY_IDENT_MEM_DATA_2_DELAY_ERR  VTSS_BIT(5)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_CL_2_INTEGRETY_IDENT_INTEGRETY_IDENT . MEM_S2_ERR
 */
#define  VTSS_F_ANA_CL_2_INTEGRETY_IDENT_INTEGRETY_IDENT_MEM_S2_ERR  VTSS_BIT(4)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_CL_2_INTEGRETY_IDENT_INTEGRETY_IDENT . MEM_S1_ERR
 */
#define  VTSS_F_ANA_CL_2_INTEGRETY_IDENT_INTEGRETY_IDENT_MEM_S1_ERR  VTSS_BIT(3)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_CL_2_INTEGRETY_IDENT_INTEGRETY_IDENT . MEM_S0_ERR
 */
#define  VTSS_F_ANA_CL_2_INTEGRETY_IDENT_INTEGRETY_IDENT_MEM_S0_ERR  VTSS_BIT(2)

/** 
 * \brief
 * Shows value of internal RAM integrity check.
 *
 * \details 
 * '0': No RAM integrity check error occurred
 * '1': A RAM integrity check error occurred
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_CL_2_INTEGRETY_IDENT_INTEGRETY_IDENT . MEM_DATA_DELAY_ERR
 */
#define  VTSS_F_ANA_CL_2_INTEGRETY_IDENT_INTEGRETY_IDENT_MEM_DATA_DELAY_ERR  VTSS_BIT(1)

/**
 * Register Group: \a ANA_CL_2:VQ_COUNTER
 *
 * Virtual queue counter
 */


/** 
 * \brief Virtual queue Drop counter register 
 *
 * \details
 * Register: \a ANA_CL_2:VQ_COUNTER:DROP_CNT
 */
#define VTSS_ANA_CL_2_VQ_COUNTER_DROP_CNT    VTSS_IOREG(VTSS_TO_ANA_CL,0x8c1)

/**
 * Register Group: \a ANA_CL_2:PORT
 *
 * Classification and filter configurations per port
 */


/** 
 * \brief Filter Configuration
 *
 * \details
 * Configure filtering of frames not matching expected ingress properties
 *
 * Register: \a ANA_CL_2:PORT:FILTER_CTRL
 *
 * @param gi Replicator: x_ANA_NUM_ALL_PORTS_INC_CPU (??), 0-34
 */
#define VTSS_ANA_CL_2_PORT_FILTER_CTRL(gi)   VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x0,gi,64,0,0)

/** 
 * \brief
 * Redirect ICMPv6 frames to the CPU.
 *
 * \details 
 * 0: Disabled
 * 1: Enabled
 *
 * Field: ::VTSS_ANA_CL_2_PORT_FILTER_CTRL . IP6_ICMP_HOP_BY_HOP_REDIR_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_FILTER_CTRL_IP6_ICMP_HOP_BY_HOP_REDIR_ENA  VTSS_BIT(20)

/** 
 * \brief
 * Redirect IPv6 frames with hop by hop options to the CPU.
 *
 * \details 
 * 0: Disabled redirection
 * 1: Enable redirection to the CPU queue
 *
 * Field: ::VTSS_ANA_CL_2_PORT_FILTER_CTRL . IP6_HOP_BY_HOP_REDIR_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_FILTER_CTRL_IP6_HOP_BY_HOP_REDIR_ENA  VTSS_BIT(19)

/** 
 * \brief
 * Discard frames with a multicast SMAC address.
 *
 * \details 
 * 0: Drop frames with multicast SMAC
 * 1: no filter
 *
 * Field: ::VTSS_ANA_CL_2_PORT_FILTER_CTRL . FILTER_SMAC_MC_DIS
 */
#define  VTSS_F_ANA_CL_2_PORT_FILTER_CTRL_FILTER_SMAC_MC_DIS  VTSS_BIT(18)

/** 
 * \brief
 * Discard frames with DMAC or SMAC equal to 00-00-00-00-00-00
 *
 * \details 
 * 0: Drop Null MAC frames
 * 1: no filter
 *
 * Field: ::VTSS_ANA_CL_2_PORT_FILTER_CTRL . FILTER_NULL_MAC_DIS
 */
#define  VTSS_F_ANA_CL_2_PORT_FILTER_CTRL_FILTER_NULL_MAC_DIS  VTSS_BIT(17)

/** 
 * \brief
 * Discard frame if frame is single tagged.
 *
 * \details 
 * 0: Disable
 * 1: Enable discard
 *
 * Field: ::VTSS_ANA_CL_2_PORT_FILTER_CTRL . SINGLE_TAG_DIS
 */
#define  VTSS_F_ANA_CL_2_PORT_FILTER_CTRL_SINGLE_TAG_DIS  VTSS_BIT(16)

/** 
 * \brief
 * Discard frame if inner vlan tag is C-TAG.
 *
 * \details 
 * 0: Disable
 * 1: Enable discard
 *
 * Field: ::VTSS_ANA_CL_2_PORT_FILTER_CTRL . IN_PRIO_CTAG_DIS
 */
#define  VTSS_F_ANA_CL_2_PORT_FILTER_CTRL_IN_PRIO_CTAG_DIS  VTSS_BIT(15)

/** 
 * \brief
 * Discard frame if inner vlan tag is C-TAG.
 *
 * \details 
 * 0: Disable
 * 1: Enable discard
 *
 * Field: ::VTSS_ANA_CL_2_PORT_FILTER_CTRL . IN_CTAG_DIS
 */
#define  VTSS_F_ANA_CL_2_PORT_FILTER_CTRL_IN_CTAG_DIS  VTSS_BIT(14)

/** 
 * \brief
 * Discard frame if inner vlan tag is I-TAG
 *
 * \details 
 * 0: Disable
 * 1: Enable discard
 *
 * Field: ::VTSS_ANA_CL_2_PORT_FILTER_CTRL . IN_ITAG_DIS
 */
#define  VTSS_F_ANA_CL_2_PORT_FILTER_CTRL_IN_ITAG_DIS  VTSS_BIT(11)

/** 
 * \brief
 * Discard frame if inner vlan tag is priority S-TAG (VID=0)
 *
 * \details 
 * 0: Disable
 * 1: Enable discard
 *
 * Field: ::VTSS_ANA_CL_2_PORT_FILTER_CTRL . IN_PRIO_STAG_DIS
 */
#define  VTSS_F_ANA_CL_2_PORT_FILTER_CTRL_IN_PRIO_STAG_DIS  VTSS_BIT(10)

/** 
 * \brief
 * Discard frames if inner vlan tag is S-TAG
 *
 * \details 
 * 0: Disable
 * 1: Enable discard
 *
 * Field: ::VTSS_ANA_CL_2_PORT_FILTER_CTRL . IN_STAG_DIS
 */
#define  VTSS_F_ANA_CL_2_PORT_FILTER_CTRL_IN_STAG_DIS  VTSS_BIT(9)

/** 
 * \brief
 * Discard untagged frames.
 *
 * \details 
 * 0: Disable
 * 1: Enable discard
 *
 * Field: ::VTSS_ANA_CL_2_PORT_FILTER_CTRL . VLAN_UNTAG_DIS
 */
#define  VTSS_F_ANA_CL_2_PORT_FILTER_CTRL_VLAN_UNTAG_DIS  VTSS_BIT(8)

/** 
 * \brief
 * Discard frames if outer vlan tag is I-TAG
 *
 * \details 
 * 0: Disable
 * 1: Enable discard
 *
 * Field: ::VTSS_ANA_CL_2_PORT_FILTER_CTRL . OUT_ITAG_DIS
 */
#define  VTSS_F_ANA_CL_2_PORT_FILTER_CTRL_OUT_ITAG_DIS  VTSS_BIT(5)

/** 
 * \brief
 * Discard frame if outer vlan tag is priority C-TAG (VID=0).
 *
 * \details 
 * 0: Disable
 * 1: Enable discard
 *
 * Field: ::VTSS_ANA_CL_2_PORT_FILTER_CTRL . OUT_PRIO_CTAG_DIS
 */
#define  VTSS_F_ANA_CL_2_PORT_FILTER_CTRL_OUT_PRIO_CTAG_DIS  VTSS_BIT(4)

/** 
 * \brief
 * Discard frame if outer vlan tag is C-TAG.
 *
 * \details 
 * 0: Disable
 * 1: Enable discard
 *
 * Field: ::VTSS_ANA_CL_2_PORT_FILTER_CTRL . OUT_CTAG_DIS
 */
#define  VTSS_F_ANA_CL_2_PORT_FILTER_CTRL_OUT_CTAG_DIS  VTSS_BIT(3)

/** 
 * \brief
 * Discard frame if outer vlan tag is priority S-TAG (VID=0).
 *
 * \details 
 * 0: Disable
 * 1: Enable discard
 *
 * Field: ::VTSS_ANA_CL_2_PORT_FILTER_CTRL . OUT_PRIO_STAG_DIS
 */
#define  VTSS_F_ANA_CL_2_PORT_FILTER_CTRL_OUT_PRIO_STAG_DIS  VTSS_BIT(2)

/** 
 * \brief
 * Discard frame if outer vlan tag is S-TAG.
 *
 * \details 
 * 0: Disable
 * 1: Enable discard
 *
 * Field: ::VTSS_ANA_CL_2_PORT_FILTER_CTRL . OUT_STAG_DIS
 */
#define  VTSS_F_ANA_CL_2_PORT_FILTER_CTRL_OUT_STAG_DIS  VTSS_BIT(1)


/** 
 * \brief Stacking configuration
 *
 * \details
 * Configure stacking awareness
 *
 * Register: \a ANA_CL_2:PORT:STACKING_CTRL
 *
 * @param gi Replicator: x_ANA_NUM_ALL_PORTS_INC_CPU (??), 0-34
 */
#define VTSS_ANA_CL_2_PORT_STACKING_CTRL(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x0,gi,64,0,1)

/** 
 * \brief
 * Ingress port drop mode configuration. Applicable for front port only.
 *
 * \details 
 * 0 : Disable drop mode for the priority
 * 1 : Enable drop mode for the priority
 *
 * Field: ::VTSS_ANA_CL_2_PORT_STACKING_CTRL . IGR_DROP_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_STACKING_CTRL_IGR_DROP_ENA(x)  VTSS_ENCODE_BITFIELD(x,4,8)
#define  VTSS_M_ANA_CL_2_PORT_STACKING_CTRL_IGR_DROP_ENA     VTSS_ENCODE_BITMASK(4,8)
#define  VTSS_X_ANA_CL_2_PORT_STACKING_CTRL_IGR_DROP_ENA(x)  VTSS_EXTRACT_BITFIELD(x,4,8)

/** 
 * \brief
 * Enable usage of stacking information.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_2_PORT_STACKING_CTRL . STACKING_AWARE_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_STACKING_CTRL_STACKING_AWARE_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Enable discard of frames received without a stacking header.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_2_PORT_STACKING_CTRL . STACKING_NON_HEADER_DISCARD_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_STACKING_CTRL_STACKING_NON_HEADER_DISCARD_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable discard of frames received with a stacking header.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_2_PORT_STACKING_CTRL . STACKING_HEADER_DISCARD_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_STACKING_CTRL_STACKING_HEADER_DISCARD_ENA  VTSS_BIT(0)


/** 
 * \brief PVID
 *
 * \details
 * Port VLAN ID.
 *
 * Register: \a ANA_CL_2:PORT:VLAN_CTRL
 *
 * @param gi Replicator: x_ANA_NUM_ALL_PORTS_INC_CPU (??), 0-34
 */
#define VTSS_ANA_CL_2_PORT_VLAN_CTRL(gi)     VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x0,gi,64,0,2)

/** 
 * \brief
 * Enable extraction of I-TAG information (PCP, DEI, ISID etc).
 *
 * \details 
 * 0: Disable (unaware)
 * 1: Enable (aware).
 *
 * Field: ::VTSS_ANA_CL_2_PORT_VLAN_CTRL . ITAG_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_VLAN_CTRL_ITAG_ENA  VTSS_BIT(27)

/** 
 * \brief
 * Use the PCP and DEI information out of the ITAG.
 *
 * \details 
 * 0: Disable (unaware)
 * 1: Enable (aware).
 *
 * Field: ::VTSS_ANA_CL_2_PORT_VLAN_CTRL . ITAG_PCP_DEI_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_VLAN_CTRL_ITAG_PCP_DEI_ENA  VTSS_BIT(26)

/** 
 * \brief
 * Selects the tag used for QoS classification and user priority remapping.
 *
 * \details 
 * 0: Use outer tag.
 * 1: Use inner tag if present, otherwise use outer tag.
 *
 * Field: ::VTSS_ANA_CL_2_PORT_VLAN_CTRL . VLAN_TAG_SEL
 */
#define  VTSS_F_ANA_CL_2_PORT_VLAN_CTRL_VLAN_TAG_SEL  VTSS_BIT(25)

/** 
 * \brief
 * Maximum number of consecutive qtags.
 *
 * \details 
 * 0: Tags
 * 1: 1 tag
 * 2: 2 tags
 * 3: Don't care
 *
 * Field: ::VTSS_ANA_CL_2_PORT_VLAN_CTRL . MAX_QTAGS
 */
#define  VTSS_F_ANA_CL_2_PORT_VLAN_CTRL_MAX_QTAGS(x)  VTSS_ENCODE_BITFIELD(x,23,2)
#define  VTSS_M_ANA_CL_2_PORT_VLAN_CTRL_MAX_QTAGS     VTSS_ENCODE_BITMASK(23,2)
#define  VTSS_X_ANA_CL_2_PORT_VLAN_CTRL_MAX_QTAGS(x)  VTSS_EXTRACT_BITFIELD(x,23,2)

/** 
 * \brief
 * Maximum number of consecutive qtags permitted identical to the outer tag
 * type (C-TAG and S-TAG).
 *
 * \details 
 * 0: Don't care
 * 1: 1 tag
 * 2: 2 tags
 * 3: reserved
 *
 * Field: ::VTSS_ANA_CL_2_PORT_VLAN_CTRL . MAX_VLAN_TAGS
 */
#define  VTSS_F_ANA_CL_2_PORT_VLAN_CTRL_MAX_VLAN_TAGS(x)  VTSS_ENCODE_BITFIELD(x,21,2)
#define  VTSS_M_ANA_CL_2_PORT_VLAN_CTRL_MAX_VLAN_TAGS     VTSS_ENCODE_BITMASK(21,2)
#define  VTSS_X_ANA_CL_2_PORT_VLAN_CTRL_MAX_VLAN_TAGS(x)  VTSS_EXTRACT_BITFIELD(x,21,2)

/** 
 * \brief
 * Default CFI bit for the port for untagged frames. Also used if port is
 * unaware.
 *
 * \details 
 * 0: CFI bit = 0
 * 1: CFI bit = 1
 *
 * Field: ::VTSS_ANA_CL_2_PORT_VLAN_CTRL . PORT_CFI
 */
#define  VTSS_F_ANA_CL_2_PORT_VLAN_CTRL_PORT_CFI  VTSS_BIT(20)

/** 
 * \brief
 * Default User priority for the port for untagged frames. Also used if
 * port is unaware.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_2_PORT_VLAN_CTRL . PORT_UPRIO
 */
#define  VTSS_F_ANA_CL_2_PORT_VLAN_CTRL_PORT_UPRIO(x)  VTSS_ENCODE_BITFIELD(x,16,3)
#define  VTSS_M_ANA_CL_2_PORT_VLAN_CTRL_PORT_UPRIO     VTSS_ENCODE_BITMASK(16,3)
#define  VTSS_X_ANA_CL_2_PORT_VLAN_CTRL_PORT_UPRIO(x)  VTSS_EXTRACT_BITFIELD(x,16,3)

/** 
 * \brief
 * Default VID value for the port for untagged frames. Also used if port is
 * unaware.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_2_PORT_VLAN_CTRL . PORT_VID
 */
#define  VTSS_F_ANA_CL_2_PORT_VLAN_CTRL_PORT_VID(x)  VTSS_ENCODE_BITFIELD(x,3,12)
#define  VTSS_M_ANA_CL_2_PORT_VLAN_CTRL_PORT_VID     VTSS_ENCODE_BITMASK(3,12)
#define  VTSS_X_ANA_CL_2_PORT_VLAN_CTRL_PORT_VID(x)  VTSS_EXTRACT_BITFIELD(x,3,12)

/** 
 * \brief
 * Default QTAG type for untagged frames. Also used if port is unaware.
 *
 * \details 
 * 0: C-TAG (Ethertype 0x8100)
 * 1: S-TAG (Configurable etype value)
 *
 * Field: ::VTSS_ANA_CL_2_PORT_VLAN_CTRL . PORT_TAG_TYPE
 */
#define  VTSS_F_ANA_CL_2_PORT_VLAN_CTRL_PORT_TAG_TYPE  VTSS_BIT(2)

/** 
 * \brief
 * Sets the port into the BPORT mode.
 *
 * \details 
 * 0: No BPORT
 * 1: BPORT
 *
 * Field: ::VTSS_ANA_CL_2_PORT_VLAN_CTRL . IGR_PORT_TYPE
 */
#define  VTSS_F_ANA_CL_2_PORT_VLAN_CTRL_IGR_PORT_TYPE  VTSS_BIT(1)

/** 
 * \brief
 * Enable VLAN awareness per port. If unaware QTAG frame information will
 * not be used
 *
 * \details 
 * 0: Disable (unaware)
 * 1: Enable (aware).
 *
 * Field: ::VTSS_ANA_CL_2_PORT_VLAN_CTRL . VLAN_AWARE_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_VLAN_CTRL_VLAN_AWARE_ENA  VTSS_BIT(0)


/** 
 * \brief Port ID Data
 *
 * \details
 * Configuration of glag and logical port number
 *
 * Register: \a ANA_CL_2:PORT:PORT_ID_CFG
 *
 * @param gi Replicator: x_ANA_NUM_ALL_PORTS_INC_CPU (??), 0-34
 */
#define VTSS_ANA_CL_2_PORT_PORT_ID_CFG(gi)   VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x0,gi,64,0,3)

/** 
 * \brief
 * Global Link Aggregation Group(GLAG) number to be used in relation to
 * learning and forwarding.
 *
 * \details 
 * PORT_IS_GLAG_ENA=0:
 * Port does not participate in a GLAG
 * PORT_IS_GLAG_ENA=1:
 * 0: port is member of GLAG 0
 * 1: port is member of GLAG 1
 * ...
 * N: port is member of GLAG N
 *
 * Field: ::VTSS_ANA_CL_2_PORT_PORT_ID_CFG . GLAG_NUM
 */
#define  VTSS_F_ANA_CL_2_PORT_PORT_ID_CFG_GLAG_NUM(x)  VTSS_ENCODE_BITFIELD(x,8,5)
#define  VTSS_M_ANA_CL_2_PORT_PORT_ID_CFG_GLAG_NUM     VTSS_ENCODE_BITMASK(8,5)
#define  VTSS_X_ANA_CL_2_PORT_PORT_ID_CFG_GLAG_NUM(x)  VTSS_EXTRACT_BITFIELD(x,8,5)

/** 
 * \brief
 * Port is part of a Global Link Aggregation Gloup ID (GLAG).
 *
 * \details 
 * 0: Port is not global aggregated.
 * 1: Port is part of a GLAG
 *
 * Field: ::VTSS_ANA_CL_2_PORT_PORT_ID_CFG . PORT_IS_GLAG_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_PORT_ID_CFG_PORT_IS_GLAG_ENA  VTSS_BIT(7)

/** 
 * \brief
 * Logical port number number to be used in relation to learning,
 * forwarding and policing.
 *
 * \details 
 * 0: Logical port 0
 * 1: Logical port 1
 * ...
 * n: Logical port n.
 *
 * Field: ::VTSS_ANA_CL_2_PORT_PORT_ID_CFG . LPORT_NUM
 */
#define  VTSS_F_ANA_CL_2_PORT_PORT_ID_CFG_LPORT_NUM(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_ANA_CL_2_PORT_PORT_ID_CFG_LPORT_NUM     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_ANA_CL_2_PORT_PORT_ID_CFG_LPORT_NUM(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief Configuration per cfi, user priority
 *
 * \details
 * Register: \a ANA_CL_2:PORT:UPRIO_MAP_CFG
 *
 * @param gi Replicator: x_ANA_NUM_ALL_PORTS_INC_CPU (??), 0-34
 * @param ri Register: UPRIO_MAP_CFG (??), 0-15
 */
#define VTSS_ANA_CL_2_PORT_UPRIO_MAP_CFG(gi,ri)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x0,gi,64,ri,4)

/** 
 * \brief
 * Map VLAN User priority and CFI to a QOS.
 * 
 * Class = UPRIO_CFI_TRANSLATE_QOS_VAL[CFI + UPRIO]
 *
 * \details 
 * 0: Class 0 (lowest)
 * 1: Class 1
 * ...
 * n: Class n (highest).
 *
 * Field: ::VTSS_ANA_CL_2_PORT_UPRIO_MAP_CFG . UPRIO_CFI_TRANSLATE_QOS_VAL
 */
#define  VTSS_F_ANA_CL_2_PORT_UPRIO_MAP_CFG_UPRIO_CFI_TRANSLATE_QOS_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_ANA_CL_2_PORT_UPRIO_MAP_CFG_UPRIO_CFI_TRANSLATE_QOS_VAL     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_ANA_CL_2_PORT_UPRIO_MAP_CFG_UPRIO_CFI_TRANSLATE_QOS_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief QOS configuration
 *
 * \details
 * Configuration of basic QOS classification
 *
 * Register: \a ANA_CL_2:PORT:QOS_CFG
 *
 * @param gi Replicator: x_ANA_NUM_ALL_PORTS_INC_CPU (??), 0-34
 */
#define VTSS_ANA_CL_2_PORT_QOS_CFG(gi)       VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x0,gi,64,0,20)

/** 
 * \brief
 * Default port DP value.
 *
 * \details 
 * 0: DP 0 (disable)
 * 1: DP 1
 * ...
 * n: DP n (highest drop probability).
 *
 * Field: ::VTSS_ANA_CL_2_PORT_QOS_CFG . DEFAULT_DP_VAL
 */
#define  VTSS_F_ANA_CL_2_PORT_QOS_CFG_DEFAULT_DP_VAL(x)  VTSS_ENCODE_BITFIELD(x,16,2)
#define  VTSS_M_ANA_CL_2_PORT_QOS_CFG_DEFAULT_DP_VAL     VTSS_ENCODE_BITMASK(16,2)
#define  VTSS_X_ANA_CL_2_PORT_QOS_CFG_DEFAULT_DP_VAL(x)  VTSS_EXTRACT_BITFIELD(x,16,2)

/** 
 * \brief
 * Allow DP classification based on BI-TAGGED frame.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_2_PORT_QOS_CFG . ITAG_UPRIO_DP_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_QOS_CFG_ITAG_UPRIO_DP_ENA  VTSS_BIT(15)

/** 
 * \brief
 * Allow QOS classification based on BI-TAG frames
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_2_PORT_QOS_CFG . ITAG_UPRIO_QOS_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_QOS_CFG_ITAG_UPRIO_QOS_ENA  VTSS_BIT(14)

/** 
 * \brief
 * Allow DP classification based on User Priority and CFI from tagged
 * frames.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_2_PORT_QOS_CFG . UPRIO_DP_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_QOS_CFG_UPRIO_DP_ENA  VTSS_BIT(13)

/** 
 * \brief
 * Allow DP classification based on DSCP
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_2_PORT_QOS_CFG . DSCP_DP_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_QOS_CFG_DSCP_DP_ENA  VTSS_BIT(12)

/** 
 * \brief
 * Default port QOS value.
 *
 * \details 
 * 0: Class 0 (lowest)
 * 1: Class 1
 * ...
 * n: Class n (highest).
 *
 * Field: ::VTSS_ANA_CL_2_PORT_QOS_CFG . DEFAULT_QOS_VAL
 */
#define  VTSS_F_ANA_CL_2_PORT_QOS_CFG_DEFAULT_QOS_VAL(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_ANA_CL_2_PORT_QOS_CFG_DEFAULT_QOS_VAL     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_ANA_CL_2_PORT_QOS_CFG_DEFAULT_QOS_VAL(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * Setting this bit will prevent the REW from making any changes to frames
 * from this port.
 * If a frame is cpu injected then the bit will be taken from the injected
 * IFH and the 
 * configuration bit will be overruled.
 *
 * \details 
 * 0: Allow REW to change the frame
 * 1: Don't allow REW to change the frame
 *
 * Field: ::VTSS_ANA_CL_2_PORT_QOS_CFG . KEEP_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_QOS_CFG_KEEP_ENA  VTSS_BIT(7)

/** 
 * \brief
 * Allow QOS classification based on User Priority and CFI from tagged
 * frames.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_2_PORT_QOS_CFG . UPRIO_QOS_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_QOS_CFG_UPRIO_QOS_ENA  VTSS_BIT(5)

/** 
 * \brief
 * Select which DSCP values are updated according to internal QOS
 *
 * \details 
 * 0: Rewrite none
 * 1: Rewrite zero
 * 2: Rewrite selected 
 * 3: Rewrite all
 *
 * Field: ::VTSS_ANA_CL_2_PORT_QOS_CFG . DSCP_REWR_MODE_SEL
 */
#define  VTSS_F_ANA_CL_2_PORT_QOS_CFG_DSCP_REWR_MODE_SEL(x)  VTSS_ENCODE_BITFIELD(x,3,2)
#define  VTSS_M_ANA_CL_2_PORT_QOS_CFG_DSCP_REWR_MODE_SEL     VTSS_ENCODE_BITMASK(3,2)
#define  VTSS_X_ANA_CL_2_PORT_QOS_CFG_DSCP_REWR_MODE_SEL(x)  VTSS_EXTRACT_BITFIELD(x,3,2)

/** 
 * \brief
 * Setting this bit will prevent the REW from remapping DSCP values for
 * frames from this port.
 *
 * \details 
 * 0: Allow REW to remap DSCP field
 * 1: Don't allow REW to remap of DSCP field
 *
 * Field: ::VTSS_ANA_CL_2_PORT_QOS_CFG . DSCP_KEEP_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_QOS_CFG_DSCP_KEEP_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Enable/disable ingress DSCP translation for the QOS and DP
 * calssification.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_2_PORT_QOS_CFG . DSCP_TRANSLATE_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_QOS_CFG_DSCP_TRANSLATE_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Allow QOS classification based on DSCP
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_2_PORT_QOS_CFG . DSCP_QOS_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_QOS_CFG_DSCP_QOS_ENA  VTSS_BIT(0)


/** 
 * \brief CPU Forward Control
 *
 * \details
 * Configuration of CPU capturing of Control frames
 *
 * Register: \a ANA_CL_2:PORT:CAPTURE_CFG
 *
 * @param gi Replicator: x_ANA_NUM_ALL_PORTS_INC_CPU (??), 0-34
 */
#define VTSS_ANA_CL_2_PORT_CAPTURE_CFG(gi)   VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x0,gi,64,0,21)

/** 
 * \brief
 * Control CPU redirection of MLD packets
 *
 * \details 
 * 0: Disable redirection
 * 1: Enable redirection to CPU queue.
 *
 * Field: ::VTSS_ANA_CL_2_PORT_CAPTURE_CFG . CPU_MLD_REDIR_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_CAPTURE_CFG_CPU_MLD_REDIR_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Control CPU copy of IPv4 Multicast control packets
 *
 * \details 
 * 0: Disable copy
 * 1: Enable copy to CPU queue
 *
 * Field: ::VTSS_ANA_CL_2_PORT_CAPTURE_CFG . CPU_IP4_MC_COPY_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_CAPTURE_CFG_CPU_IP4_MC_COPY_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Control CPU redirection of IGMP frames.
 *
 * \details 
 * 0: Disable redirection
 * 1: Enable redirection to CPU queue
 *
 * Field: ::VTSS_ANA_CL_2_PORT_CAPTURE_CFG . CPU_IGMP_REDIR_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_CAPTURE_CFG_CPU_IGMP_REDIR_ENA  VTSS_BIT(0)


/** 
 * \brief IEEE802.1ag / ITU-T Y.1731 OAM frame filtering Control
 *
 * \details
 * Configuration of CPU capturing of Bridge Control frames
 *
 * Register: \a ANA_CL_2:PORT:CAPTURE_Y1731_AG_CFG
 *
 * @param gi Replicator: x_ANA_NUM_ALL_PORTS_INC_CPU (??), 0-34
 */
#define VTSS_ANA_CL_2_PORT_CAPTURE_Y1731_AG_CFG(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x0,gi,64,0,22)


/** 
 * \brief GXRP redirection control
 *
 * \details
 * Register: \a ANA_CL_2:PORT:CAPTURE_GXRP_CFG
 *
 * @param gi Replicator: x_ANA_NUM_ALL_PORTS_INC_CPU (??), 0-34
 */
#define VTSS_ANA_CL_2_PORT_CAPTURE_GXRP_CFG(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x0,gi,64,0,23)


/** 
 * \brief BPDU redirection control
 *
 * \details
 * Register: \a ANA_CL_2:PORT:CAPTURE_BPDU_CFG
 *
 * @param gi Replicator: x_ANA_NUM_ALL_PORTS_INC_CPU (??), 0-34
 */
#define VTSS_ANA_CL_2_PORT_CAPTURE_BPDU_CFG(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x0,gi,64,0,24)


/** 
 * \brief Advanced classification configuration registers
 *
 * \details
 * Configuration of advanced classification.
 * 
 * The following fields (bits 16, 17, and 24) are configuration registers
 * for VCAP_IS1. 
 *   ADV_QOS_CL_ENA
 *   VCAP_TAG_SEL
 *   ADV_USER_CL_ENA
 * The remaining fields are configuration registers for VCAP_IS2.
 * 
 * For the 2-bit fields of this register the following applies:
 * Bit[0]: Relates to first lookup in VCAP_IS2
 * Bit[1]: Relates to second lookup in VCAP_IS2
 *
 * Register: \a ANA_CL_2:PORT:ADV_CL_CFG
 *
 * @param gi Replicator: x_ANA_NUM_ALL_PORTS_INC_CPU (??), 0-34
 */
#define VTSS_ANA_CL_2_PORT_ADV_CL_CFG(gi)    VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x0,gi,64,0,25)

/** 
 * \brief
 * For frames to be routed, enable/disable the use of IRLEG VID and ERLEG
 * VID as VID value in VCAP IS2 lookup (ref. VCAP_IS2:*_ENTRY:VID).
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_2_PORT_ADV_CL_CFG . SEC_ROUTE_HANDLING_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_ADV_CL_CFG_SEC_ROUTE_HANDLING_ENA  VTSS_BIT(25)

/** 
 * \brief
 * Enable/Disable Advanced User classification in VCAP_IS1.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_2_PORT_ADV_CL_CFG . ADV_USER_CL_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_ADV_CL_CFG_ADV_USER_CL_ENA  VTSS_BIT(24)

/** 
 * \brief
 * For MAC-in-MAC encapsulated OAM frames, enable matching against Control
 * Entries of type OAM in VCAP_IS2.
 * 
 * OAM frames are identified by the following EtherType values:
 * 0x8902 - ITU-T Y.1731
 * 0x8809 - Link Level OAM 
 * 0x88EE - MEF-16 (E-LMI)
 * 
 * Up to one VLAN tag before the I-Tag and up to one VLAN tag after the
 * I-Tag is supported.f
 * 
 * Bit[0]: Relates to first lookup in VCAP_IS2
 * Bit[1]: Relates to second lookup in VCAP_IS2

 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_2_PORT_ADV_CL_CFG . SEC_TYPE_ITAGGED_OAM_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_ITAGGED_OAM_ENA(x)  VTSS_ENCODE_BITFIELD(x,22,2)
#define  VTSS_M_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_ITAGGED_OAM_ENA     VTSS_ENCODE_BITMASK(22,2)
#define  VTSS_X_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_ITAGGED_OAM_ENA(x)  VTSS_EXTRACT_BITFIELD(x,22,2)

/** 
 * \brief
 * For OAM frames (with up to two VLAN tags), enable matching against
 * Control Entries of type OAM in VCAP_IS2.
 * 
 * OAM frames are identified by the following EtherType values:
 * 0x8902 - ITU-T Y.1731
 * 0x8809 - Link Level OAM 
 * 0x88EE - MEF-16 (E-LMI)
 * 
 * Enabling for MAC-in-MAC encapsulated OAM frames are covered separately
 * in SEC_TYPE_ITAGGED_OAM_ENA.
 * 
 * Bit[0]: Relates to first lookup in VCAP_IS2
 * Bit[1]: Relates to second lookup in VCAP_IS2

 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_2_PORT_ADV_CL_CFG . SEC_TYPE_OAM_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_OAM_ENA(x)  VTSS_ENCODE_BITFIELD(x,20,2)
#define  VTSS_M_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_OAM_ENA     VTSS_ENCODE_BITMASK(20,2)
#define  VTSS_X_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_OAM_ENA(x)  VTSS_EXTRACT_BITFIELD(x,20,2)

/** 
 * \brief
 * Enable matching against Control Entries of type CUSTOM_0-CUSTOM_7 in
 * VCAP_IS2.
 * 
 * In order for VCAP_IS2 to use a CUSTOM CE type, a VLAN_PAG_ACTION action
 * with CUSTOM_ACE_TYPE_ENA=1 must be hit in VCAP_IS1. The VLAN_PAG_ACTION
 * provides the offsets to the custom data used in the VCAP_IS2 CUSTOM key.
 * 
 * Bit[0]: Relates to first lookup in VCAP_IS2
 * Bit[1]: Relates to second lookup in VCAP_IS2

 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_2_PORT_ADV_CL_CFG . SEC_TYPE_CUSTOM_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_CUSTOM_ENA(x)  VTSS_ENCODE_BITFIELD(x,18,2)
#define  VTSS_M_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_CUSTOM_ENA     VTSS_ENCODE_BITMASK(18,2)
#define  VTSS_X_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_CUSTOM_ENA(x)  VTSS_EXTRACT_BITFIELD(x,18,2)

/** 
 * \brief
 * Selects the QTAG used for advanced VCAP classification in VCAP_IS1.
 *
 * \details 
 * 0: Use outer tag.
 * 1: Use inner tag if present, otherwise use outer tag.
 *
 * Field: ::VTSS_ANA_CL_2_PORT_ADV_CL_CFG . VCAP_TAG_SEL
 */
#define  VTSS_F_ANA_CL_2_PORT_ADV_CL_CFG_VCAP_TAG_SEL  VTSS_BIT(17)

/** 
 * \brief
 * Enable/Disable Advanced QOS classification in VCAP_IS1.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_2_PORT_ADV_CL_CFG . ADV_QOS_CL_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_ADV_CL_CFG_ADV_QOS_CL_ENA  VTSS_BIT(16)

/** 
 * \brief
 * For LLC SNAP frames, enable matching against Control Entries of type
 * MAC_SNAP in VCAP_IS2.
 * 
 * LLC SNAP frames are identified by
 * * EtherType < 0x0600
 * * DSAP = 0xAA
 * * SSAP = 0xAA
 * * CTRL = 0x03
 * 
 * Bit[0]: Relates to first lookup in VCAP_IS2
 * Bit[1]: Relates to second lookup in VCAP_IS2

 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_2_PORT_ADV_CL_CFG . SEC_TYPE_MAC_SNAP_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_MAC_SNAP_ENA(x)  VTSS_ENCODE_BITFIELD(x,14,2)
#define  VTSS_M_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_MAC_SNAP_ENA     VTSS_ENCODE_BITMASK(14,2)
#define  VTSS_X_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_MAC_SNAP_ENA(x)  VTSS_EXTRACT_BITFIELD(x,14,2)

/** 
 * \brief
 * For IPv4 frames, enable matching against Control Entries of type
 * IP_TCP_UDP in VCAP_IS2.
 * 
 * Bit[0]: Relates to first lookup in VCAP_IS2
 * Bit[1]: Relates to second lookup in VCAP_IS2

 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_2_PORT_ADV_CL_CFG . SEC_TYPE_IP_TCPUDP_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_IP_TCPUDP_ENA(x)  VTSS_ENCODE_BITFIELD(x,12,2)
#define  VTSS_M_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_IP_TCPUDP_ENA     VTSS_ENCODE_BITMASK(12,2)
#define  VTSS_X_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_IP_TCPUDP_ENA(x)  VTSS_EXTRACT_BITFIELD(x,12,2)

/** 
 * \brief
 * For IPv6 frames, enable matching against Control Entries of type
 * IP_TCP_UDP and IP_OTHER in VCAP_IS2.
 * 
 * The SIP and DIP fields of IP_TCP_UDP and IP_OTHER Control Entries are
 * used to match against IPv6 SIP, bit 63:0.
 * 
 * Bit[0]: Relates to first lookup in VCAP_IS2
 * Bit[1]: Relates to second lookup in VCAP_IS2

 *
 * \details 
 * 0: Disable the classification.
 * 1: Enable the classification.
 *
 * Field: ::VTSS_ANA_CL_2_PORT_ADV_CL_CFG . SEC_TYPE_IP6_TCPUDP_OTHER_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_IP6_TCPUDP_OTHER_ENA(x)  VTSS_ENCODE_BITFIELD(x,10,2)
#define  VTSS_M_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_IP6_TCPUDP_OTHER_ENA     VTSS_ENCODE_BITMASK(10,2)
#define  VTSS_X_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_IP6_TCPUDP_OTHER_ENA(x)  VTSS_EXTRACT_BITFIELD(x,10,2)

/** 
 * \brief
 * For LLC frames, enable matching against Control Entries of type MAC_LLC
 * in VCAP_IS2.
 * 
 * LLC frames are identified as frames, which have EtherType < 0x0600.
 * 
 * Bit[0]: Relates to first lookup in VCAP_IS2
 * Bit[1]: Relates to second lookup in VCAP_IS2

 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_2_PORT_ADV_CL_CFG . SEC_TYPE_MAC_LLC_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_MAC_LLC_ENA(x)  VTSS_ENCODE_BITFIELD(x,8,2)
#define  VTSS_M_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_MAC_LLC_ENA     VTSS_ENCODE_BITMASK(8,2)
#define  VTSS_X_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_MAC_LLC_ENA(x)  VTSS_EXTRACT_BITFIELD(x,8,2)

/** 
 * \brief
 * For ARP frames (EtherType 0x0806), enable matching against Control
 * Entries of type ARP in VCAP_IS2.
 * 
 * Bit[0]: Relates to first lookup in VCAP_IS2
 * Bit[1]: Relates to second lookup in VCAP_IS2

 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_2_PORT_ADV_CL_CFG . SEC_TYPE_ARP_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_ARP_ENA(x)  VTSS_ENCODE_BITFIELD(x,6,2)
#define  VTSS_M_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_ARP_ENA     VTSS_ENCODE_BITMASK(6,2)
#define  VTSS_X_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_ARP_ENA(x)  VTSS_EXTRACT_BITFIELD(x,6,2)

/** 
 * \brief
 * For IPv4 frames, enable matching against Control Entries of type
 * IP_OTHER in VCAP_IS2.
 * 
 * Bit[0]: Relates to first lookup in VCAP_IS2
 * Bit[1]: Relates to second lookup in VCAP_IS2

 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_2_PORT_ADV_CL_CFG . SEC_TYPE_IP_OTHER_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_IP_OTHER_ENA(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_IP_OTHER_ENA     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_IP_OTHER_ENA(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * For IPv6 frames, enable matching against Control Entries of type IP6_STD
 * in VCAP_IS2.
 * 
 * Bit[0]: Relates to first lookup in VCAP_IS2
 * Bit[1]: Relates to second lookup in VCAP_IS2

 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_2_PORT_ADV_CL_CFG . SEC_TYPE_IP6_STD_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_IP6_STD_ENA(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_IP6_STD_ENA     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_ANA_CL_2_PORT_ADV_CL_CFG_SEC_TYPE_IP6_STD_ENA(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * Enable/disable VCAP IS2 lookup.
 * 
 * Bit[0]: Relates to first lookup in VCAP_IS2
 * Bit[1]: Relates to second lookup in VCAP_IS2

 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_2_PORT_ADV_CL_CFG . SEC_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_ADV_CL_CFG_SEC_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_ANA_CL_2_PORT_ADV_CL_CFG_SEC_ENA     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_ANA_CL_2_PORT_ADV_CL_CFG_SEC_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief DP configurations
 *
 * \details
 * This register contains the settings for the DP configuration.
 *
 * Register: \a ANA_CL_2:PORT:DP_CONFIG
 *
 * @param gi Replicator: x_ANA_NUM_ALL_PORTS_INC_CPU (??), 0-34
 * @param ri Register: DP_CONFIG (??), 0-15
 */
#define VTSS_ANA_CL_2_PORT_DP_CONFIG(gi,ri)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x0,gi,64,ri,26)

/** 
 * \brief
 * Map VLAN CFI and User priority to a DP value
 *
 * \details 
 * 0: DP 0
 * 1: DP 1
 * ...
 * n: DP n (highest drop propability).
 *
 * Field: ::VTSS_ANA_CL_2_PORT_DP_CONFIG . UPRIO_CFI_DP_VAL
 */
#define  VTSS_F_ANA_CL_2_PORT_DP_CONFIG_UPRIO_CFI_DP_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_ANA_CL_2_PORT_DP_CONFIG_UPRIO_CFI_DP_VAL     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_ANA_CL_2_PORT_DP_CONFIG_UPRIO_CFI_DP_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief Contains the PBB configuration bits
 *
 * \details
 * Register: \a ANA_CL_2:PORT:IS0_CFG_1
 *
 * @param gi Replicator: x_ANA_NUM_ALL_PORTS_INC_CPU (??), 0-34
 */
#define VTSS_ANA_CL_2_PORT_IS0_CFG_1(gi)     VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x0,gi,64,0,42)

/** 
 * \brief
 * Congfigures if the SMAC or DMAC will be used for the MAC_ADDR entry into
 * the S0.
 *
 * \details 
 * '0': SMAC
 * '1': DMAC
 *
 * Field: ::VTSS_ANA_CL_2_PORT_IS0_CFG_1 . MAC_SEL
 */
#define  VTSS_F_ANA_CL_2_PORT_IS0_CFG_1_MAC_SEL  VTSS_BIT(2)

/** 
 * \brief
 * Enables key based lookup inside of the S0.
 * If it is disabled then a index based lookup inside of the S0 at the
 * default port position will take place.
 *
 * \details 
 * '1': Enabled
 * '0': Disabled
 *
 * Field: ::VTSS_ANA_CL_2_PORT_IS0_CFG_1 . IS0_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_IS0_CFG_1_IS0_ENA  VTSS_BIT(0)


/** 
 * \brief Contains the I-TAG and MPLS settings
 *
 * \details
 * Register: \a ANA_CL_2:PORT:IS0_CFG_2
 *
 * @param gi Replicator: x_ANA_NUM_ALL_PORTS_INC_CPU (??), 0-34
 */
#define VTSS_ANA_CL_2_PORT_IS0_CFG_2(gi)     VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x0,gi,64,0,43)

/** 
 * \brief
 * Enable I-TAG awareness per port.
 *
 * \details 
 * 0: Disable (unaware)
 * 1: Enable (aware).
 *
 * Field: ::VTSS_ANA_CL_2_PORT_IS0_CFG_2 . ISID_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_IS0_CFG_2_ISID_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Enable MPLS awareness per port.
 *
 * \details 
 * 0: Disable (unaware)
 * 1: Enable (aware).
 *
 * Field: ::VTSS_ANA_CL_2_PORT_IS0_CFG_2 . MPLS_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_IS0_CFG_2_MPLS_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable MAC lookup inside of the S0.
 *
 * \details 
 * 0: Disable (unaware)
 * 1: Enable (aware).
 *
 * Field: ::VTSS_ANA_CL_2_PORT_IS0_CFG_2 . MAC_ENA
 */
#define  VTSS_F_ANA_CL_2_PORT_IS0_CFG_2_MAC_ENA  VTSS_BIT(0)

/**
 * Register Group: \a ANA_CL_2:COMMON
 *
 * Common configurations for all ports
 */


/** 
 * \brief Configuration of the IFH chksum checker
 *
 * \details
 * Register: \a ANA_CL_2:COMMON:IFH_CHKSUM_CFG
 *
 * @param ri Replicator: x_ANA_NUM_ALL_PORTS_DIV32_CEIL (??), 0-1
 */
#define VTSS_ANA_CL_2_COMMON_IFH_CHKSUM_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_CL,0x8c2 + (ri))

/** 
 * \brief
 * Enable IFH checksum check. If enabled the IFH checksum will be checked
 * at reception.
 *
 * \details 
 * 0: Disabled
 * 1: Enabled discard of frames with IFH error
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_IFH_CHKSUM_CFG . IFH_CHKSUM_CHK_ENA
 */
#define  VTSS_F_ANA_CL_2_COMMON_IFH_CHKSUM_CFG_IFH_CHKSUM_CHK_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,18)
#define  VTSS_M_ANA_CL_2_COMMON_IFH_CHKSUM_CFG_IFH_CHKSUM_CHK_ENA     VTSS_ENCODE_BITMASK(0,18)
#define  VTSS_X_ANA_CL_2_COMMON_IFH_CHKSUM_CFG_IFH_CHKSUM_CHK_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,18)


/** 
 * \brief Configure UPSID when stacking
 *
 * \details
 * Register: \a ANA_CL_2:COMMON:UPSID_CFG
 */
#define VTSS_ANA_CL_2_COMMON_UPSID_CFG       VTSS_IOREG(VTSS_TO_ANA_CL,0x8c4)

/** 
 * \brief
 * Unit port set ID to be used for stacking.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_2_COMMON_UPSID_CFG . UPSID_NUM
 */
#define  VTSS_F_ANA_CL_2_COMMON_UPSID_CFG_UPSID_NUM(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_ANA_CL_2_COMMON_UPSID_CFG_UPSID_NUM     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_ANA_CL_2_COMMON_UPSID_CFG_UPSID_NUM(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief Common port control
 *
 * \details
 * Register: \a ANA_CL_2:COMMON:COMMON_CTRL
 */
#define VTSS_ANA_CL_2_COMMON_COMMON_CTRL     VTSS_IOREG(VTSS_TO_ANA_CL,0x8c5)

/** 
 * \brief
 * Initialize the port configuration table. The bit is cleared by HW when
 * Init is complete
 *
 * \details 
 * 0: Idle (no clear)
 * 1: Initialize table
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_COMMON_CTRL . TABLE_INIT_SHOT
 */
#define  VTSS_F_ANA_CL_2_COMMON_COMMON_CTRL_TABLE_INIT_SHOT  VTSS_BIT(0)


/** 
 * \brief Various configuration per DSCP
 *
 * \details
 * Register: \a ANA_CL_2:COMMON:DSCP_CFG
 *
 * @param ri Replicator: x_ANA_NUM_DSCP (??), 0-63
 */
#define VTSS_ANA_CL_2_COMMON_DSCP_CFG(ri)    VTSS_IOREG(VTSS_TO_ANA_CL,0x8c6 + (ri))

/** 
 * \brief
 * Translated DSCP value if translation is enabled for the port
 * (DSCP_TRANSLATE_ENA).
 *
 * \details 
 * Field: ::VTSS_ANA_CL_2_COMMON_DSCP_CFG . DSCP_TRANSLATE_VAL
 */
#define  VTSS_F_ANA_CL_2_COMMON_DSCP_CFG_DSCP_TRANSLATE_VAL(x)  VTSS_ENCODE_BITFIELD(x,8,6)
#define  VTSS_M_ANA_CL_2_COMMON_DSCP_CFG_DSCP_TRANSLATE_VAL     VTSS_ENCODE_BITMASK(8,6)
#define  VTSS_X_ANA_CL_2_COMMON_DSCP_CFG_DSCP_TRANSLATE_VAL(x)  VTSS_EXTRACT_BITFIELD(x,8,6)

/** 
 * \brief
 * Configure each DSCP value (0-63) to map to a QOS class.
 *
 * \details 
 * 0: Class 0 (lowest)
 * 1: Class 1
 * ...
 * n: Class n (highest).
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_DSCP_CFG . DSCP_QOS_VAL
 */
#define  VTSS_F_ANA_CL_2_COMMON_DSCP_CFG_DSCP_QOS_VAL(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_ANA_CL_2_COMMON_DSCP_CFG_DSCP_QOS_VAL     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_ANA_CL_2_COMMON_DSCP_CFG_DSCP_QOS_VAL(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * Configure each DSCP value (0-63) to map to a RED class.
 *
 * \details 
 * 0: DP 0
 * 1: DP 1
 * ...
 * n: DP n (highest drop probability).
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_DSCP_CFG . DSCP_DP_VAL
 */
#define  VTSS_F_ANA_CL_2_COMMON_DSCP_CFG_DSCP_DP_VAL(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_ANA_CL_2_COMMON_DSCP_CFG_DSCP_DP_VAL     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_ANA_CL_2_COMMON_DSCP_CFG_DSCP_DP_VAL(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * If enabled then the DSCP value will be overwritten with the value
 * corresponding to the internal QOS if DSCP_REWR_MODE_SEL is set to
 * selected.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_DSCP_CFG . DSCP_REWR_ENA
 */
#define  VTSS_F_ANA_CL_2_COMMON_DSCP_CFG_DSCP_REWR_ENA  VTSS_BIT(1)

/** 
 * \brief
 * If enabled then the QOS corresponding to the DSCP value will be used
 * based on DSCP_QOS_ENA.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_DSCP_CFG . DSCP_TRUST_ENA
 */
#define  VTSS_F_ANA_CL_2_COMMON_DSCP_CFG_DSCP_TRUST_ENA  VTSS_BIT(0)


/** 
 * \brief Configuration per QOS class
 *
 * \details
 * Register: \a ANA_CL_2:COMMON:QOS_MAP_CFG
 *
 * @param ri Replicator: x_ANA_NUM_QOS (??), 0-7
 */
#define VTSS_ANA_CL_2_COMMON_QOS_MAP_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_CL,0x906 + (ri))

/** 
 * \brief
 * Table values for rewriting DSCP values using QOS if enabled.
 *
 * \details 
 * DSCP value for QOS n
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_QOS_MAP_CFG . DSCP_REWR_VAL
 */
#define  VTSS_F_ANA_CL_2_COMMON_QOS_MAP_CFG_DSCP_REWR_VAL(x)  VTSS_ENCODE_BITFIELD(x,4,6)
#define  VTSS_M_ANA_CL_2_COMMON_QOS_MAP_CFG_DSCP_REWR_VAL     VTSS_ENCODE_BITMASK(4,6)
#define  VTSS_X_ANA_CL_2_COMMON_QOS_MAP_CFG_DSCP_REWR_VAL(x)  VTSS_EXTRACT_BITFIELD(x,4,6)


/** 
 * \brief Aggregation Calculation
 *
 * \details
 * The Aggregation Calculation register determines which fields contribute
 * to the calculation of the 4-bit aggregation code. This code is used to
 * select the egress port if multiple ports are aggregated. Fields from the
 * MAC,  IP and UDP/TCP headers can be enabled. The enabled fields are
 * XOR-ed together to generate the final aggregation code.
 *
 * Register: \a ANA_CL_2:COMMON:AGGR_CFG
 */
#define VTSS_ANA_CL_2_COMMON_AGGR_CFG        VTSS_IOREG(VTSS_TO_ANA_CL,0x90e)

/** 
 * \brief
 * Allow the ISID to contribute.
 *
 * \details 
 * '0': Disable
 * '1': Enable
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_AGGR_CFG . AGGR_ISID_ENA
 */
#define  VTSS_F_ANA_CL_2_COMMON_AGGR_CFG_AGGR_ISID_ENA  VTSS_BIT(12)

/** 
 * \brief
 * Allow the physical source port number to contribute.
 *
 * \details 
 * '0': Disable
 * '1': Enable
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_AGGR_CFG . AGGR_PHYS_SRC_ENA
 */
#define  VTSS_F_ANA_CL_2_COMMON_AGGR_CFG_AGGR_PHYS_SRC_ENA  VTSS_BIT(11)

/** 
 * \brief
 * Allow the isdx value to contribute.
 *
 * \details 
 * '0': Disable
 * '1': Enable
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_AGGR_CFG . AGGR_ISDX_ENA
 */
#define  VTSS_F_ANA_CL_2_COMMON_AGGR_CFG_AGGR_ISDX_ENA  VTSS_BIT(10)

/** 
 * \brief
 * Use AC code received in the VSTAX header as aggregation code.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_AGGR_CFG . AGGR_USE_VSTAX_AC_ENA
 */
#define  VTSS_F_ANA_CL_2_COMMON_AGGR_CFG_AGGR_USE_VSTAX_AC_ENA  VTSS_BIT(8)

/** 
 * \brief
 * Allow DMAC address contribution to be reversed.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_AGGR_CFG . AGGR_DMAC_REVERSED_ENA
 */
#define  VTSS_F_ANA_CL_2_COMMON_AGGR_CFG_AGGR_DMAC_REVERSED_ENA  VTSS_BIT(7)

/** 
 * \brief
 * Allow the IPv6 flow label to contribute.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_AGGR_CFG . AGGR_IP6_FLOW_LBL_ENA
 */
#define  VTSS_F_ANA_CL_2_COMMON_AGGR_CFG_AGGR_IP6_FLOW_LBL_ENA  VTSS_BIT(5)

/** 
 * \brief
 * Allow bit 0-7 of UDP/TCP destination and source port number to
 * contribute.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_AGGR_CFG . AGGR_IP6_TCPUDP_PORT_ENA
 */
#define  VTSS_F_ANA_CL_2_COMMON_AGGR_CFG_AGGR_IP6_TCPUDP_PORT_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Allow bit 0-7 of UDP/TCP destination and source port number to
 * contribute.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_AGGR_CFG . AGGR_IP4_TCPUDP_PORT_ENA
 */
#define  VTSS_F_ANA_CL_2_COMMON_AGGR_CFG_AGGR_IP4_TCPUDP_PORT_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Allow bit 0-7 of the source and destination IP address to contribute.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_AGGR_CFG . AGGR_IP4_SIPDIP_ENA
 */
#define  VTSS_F_ANA_CL_2_COMMON_AGGR_CFG_AGGR_IP4_SIPDIP_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Allow bit 0-11 of the destination MAC address to contribute.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_AGGR_CFG . AGGR_DMAC_ENA
 */
#define  VTSS_F_ANA_CL_2_COMMON_AGGR_CFG_AGGR_DMAC_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Allow bit 0-11 of the source MAC address to contribute.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_AGGR_CFG . AGGR_SMAC_ENA
 */
#define  VTSS_F_ANA_CL_2_COMMON_AGGR_CFG_AGGR_SMAC_ENA  VTSS_BIT(0)


/** 
 * \brief Ethertype used for identifying service tagged frames
 *
 * \details
 * Register: \a ANA_CL_2:COMMON:VLAN_STAG_CFG
 */
#define VTSS_ANA_CL_2_COMMON_VLAN_STAG_CFG   VTSS_IOREG(VTSS_TO_ANA_CL,0x90f)

/** 
 * \brief
 * Configurable S-TAG ethertype
 *
 * \details 
 * 16 bit S-TAG ethertype
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_VLAN_STAG_CFG . STAG_ETYPE_VAL
 */
#define  VTSS_F_ANA_CL_2_COMMON_VLAN_STAG_CFG_STAG_ETYPE_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_ANA_CL_2_COMMON_VLAN_STAG_CFG_STAG_ETYPE_VAL     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_ANA_CL_2_COMMON_VLAN_STAG_CFG_STAG_ETYPE_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief CPU queue configuration 
 *
 * \details
 * Register: \a ANA_CL_2:COMMON:CPU_BPDU_QU_CFG
 *
 * @param ri Replicator: x_ANA_NUM_BPDU_ADDR (??), 0-15
 */
#define VTSS_ANA_CL_2_COMMON_CPU_BPDU_QU_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_CL,0x910 + (ri))

/** 
 * \brief
 * CPU queue number for BPDU frames. These are identified by a DMAC in the
 * range 01-80-C2-00-00-30 to 01-80-C2-00-00-3F.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_CPU_BPDU_QU_CFG . CPU_AG_QU
 */
#define  VTSS_F_ANA_CL_2_COMMON_CPU_BPDU_QU_CFG_CPU_AG_QU(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_ANA_CL_2_COMMON_CPU_BPDU_QU_CFG_CPU_AG_QU     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_ANA_CL_2_COMMON_CPU_BPDU_QU_CFG_CPU_AG_QU(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * CPU queue number for GXRP frames. These are identified by a DMAC in the
 * range 01-80-C2-00-00-20 to 01-80-C2-00-00-2F.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_CPU_BPDU_QU_CFG . CPU_GXRP_QU
 */
#define  VTSS_F_ANA_CL_2_COMMON_CPU_BPDU_QU_CFG_CPU_GXRP_QU(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_ANA_CL_2_COMMON_CPU_BPDU_QU_CFG_CPU_GXRP_QU     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_ANA_CL_2_COMMON_CPU_BPDU_QU_CFG_CPU_GXRP_QU(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * CPU queue number for BPDU frames. These are identified by a DMAC in the
 * range 01-80-C2-00-00-00 to 01-80-C2-00-00-0F.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_CPU_BPDU_QU_CFG . CPU_BPDU_QU
 */
#define  VTSS_F_ANA_CL_2_COMMON_CPU_BPDU_QU_CFG_CPU_BPDU_QU(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_ANA_CL_2_COMMON_CPU_BPDU_QU_CFG_CPU_BPDU_QU     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_ANA_CL_2_COMMON_CPU_BPDU_QU_CFG_CPU_BPDU_QU(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief CPU queue configuration 
 *
 * \details
 * Register: \a ANA_CL_2:COMMON:CPU_PROTO_QU_CFG
 */
#define VTSS_ANA_CL_2_COMMON_CPU_PROTO_QU_CFG  VTSS_IOREG(VTSS_TO_ANA_CL,0x920)

/** 
 * \brief
 * CPU queue number for IPv6 Hop by hop and ICMP frames.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_CPU_PROTO_QU_CFG . CPU_HOP_BY_HOP_ICMP_QU
 */
#define  VTSS_F_ANA_CL_2_COMMON_CPU_PROTO_QU_CFG_CPU_HOP_BY_HOP_ICMP_QU(x)  VTSS_ENCODE_BITFIELD(x,12,3)
#define  VTSS_M_ANA_CL_2_COMMON_CPU_PROTO_QU_CFG_CPU_HOP_BY_HOP_ICMP_QU     VTSS_ENCODE_BITMASK(12,3)
#define  VTSS_X_ANA_CL_2_COMMON_CPU_PROTO_QU_CFG_CPU_HOP_BY_HOP_ICMP_QU(x)  VTSS_EXTRACT_BITFIELD(x,12,3)

/** 
 * \brief
 * CPU queue number for MLD frames.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_CPU_PROTO_QU_CFG . CPU_MLD_QU
 */
#define  VTSS_F_ANA_CL_2_COMMON_CPU_PROTO_QU_CFG_CPU_MLD_QU(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_ANA_CL_2_COMMON_CPU_PROTO_QU_CFG_CPU_MLD_QU     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_ANA_CL_2_COMMON_CPU_PROTO_QU_CFG_CPU_MLD_QU(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * CPU queue number for IPv4 multicast control frames. These are identified
 * by a DIP in the range 224.0.0.0-224.0.0.255.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_CPU_PROTO_QU_CFG . CPU_IP4_MC_CTRL_QU
 */
#define  VTSS_F_ANA_CL_2_COMMON_CPU_PROTO_QU_CFG_CPU_IP4_MC_CTRL_QU(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_ANA_CL_2_COMMON_CPU_PROTO_QU_CFG_CPU_IP4_MC_CTRL_QU     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_ANA_CL_2_COMMON_CPU_PROTO_QU_CFG_CPU_IP4_MC_CTRL_QU(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * CPU queue number for IGMP frames.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_CPU_PROTO_QU_CFG . CPU_IGMP_QU
 */
#define  VTSS_F_ANA_CL_2_COMMON_CPU_PROTO_QU_CFG_CPU_IGMP_QU(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_ANA_CL_2_COMMON_CPU_PROTO_QU_CFG_CPU_IGMP_QU     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_ANA_CL_2_COMMON_CPU_PROTO_QU_CFG_CPU_IGMP_QU(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Configuration of TCP range generation
 *
 * \details
 * Register: \a ANA_CL_2:COMMON:ADV_RNG_CTRL
 *
 * @param ri Replicator: x_ACL_NUM_TCP_RANGES (??), 0-7
 */
#define VTSS_ANA_CL_2_COMMON_ADV_RNG_CTRL(ri)  VTSS_IOREG(VTSS_TO_ANA_CL,0x921 + (ri))

/** 
 * \brief
 * Selected field matched against the range
 *
 * \details 
 * 0: Idle (No match)
 * 1: TCP / UDP dport value is matched against range
 * 2: TCP / UDP sport value is matched against range
 * 3: TCP / UDP dport or sport values are matched against range
 * 4: Selected received VID value is matched against range
 * 5: DSCP value is matched against range
 * 6: Selected value (=frame[RNG_OFFSET_POS:RNG_OFFSET_POS+1] is matched
 * against range
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_ADV_RNG_CTRL . RNG_TYPE_SEL
 */
#define  VTSS_F_ANA_CL_2_COMMON_ADV_RNG_CTRL_RNG_TYPE_SEL(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_ANA_CL_2_COMMON_ADV_RNG_CTRL_RNG_TYPE_SEL     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_ANA_CL_2_COMMON_ADV_RNG_CTRL_RNG_TYPE_SEL(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Configuration of  matcher range generation
 *
 * \details
 * Register: \a ANA_CL_2:COMMON:ADV_RNG_VALUE_CFG
 *
 * @param ri Replicator: x_ACL_NUM_TCP_RANGES (??), 0-7
 */
#define VTSS_ANA_CL_2_COMMON_ADV_RNG_VALUE_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_CL,0x929 + (ri))

/** 
 * \brief
 * Upper range value
 *
 * \details 
 * Field: ::VTSS_ANA_CL_2_COMMON_ADV_RNG_VALUE_CFG . RNG_MAX_VALUE
 */
#define  VTSS_F_ANA_CL_2_COMMON_ADV_RNG_VALUE_CFG_RNG_MAX_VALUE(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_ANA_CL_2_COMMON_ADV_RNG_VALUE_CFG_RNG_MAX_VALUE     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_ANA_CL_2_COMMON_ADV_RNG_VALUE_CFG_RNG_MAX_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Lower range value
 *
 * \details 
 * Field: ::VTSS_ANA_CL_2_COMMON_ADV_RNG_VALUE_CFG . RNG_MIN_VALUE
 */
#define  VTSS_F_ANA_CL_2_COMMON_ADV_RNG_VALUE_CFG_RNG_MIN_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_ANA_CL_2_COMMON_ADV_RNG_VALUE_CFG_RNG_MIN_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_ANA_CL_2_COMMON_ADV_RNG_VALUE_CFG_RNG_MIN_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Configuration of TCP range generation
 *
 * \details
 * Register: \a ANA_CL_2:COMMON:ADV_RNG_OFFSET_CFG
 */
#define VTSS_ANA_CL_2_COMMON_ADV_RNG_OFFSET_CFG  VTSS_IOREG(VTSS_TO_ANA_CL,0x931)

/** 
 * \brief
 * 16 bit Offset position of selectable range matcher input counting after
 * the	SMAC
 *
 * \details 
 * 0: frame byte 0 and 1
 * 1: frame byte 2 and 3
 * ...
 * n: frame byte 2n and 2n+1
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_ADV_RNG_OFFSET_CFG . RNG_OFFSET_POS
 */
#define  VTSS_F_ANA_CL_2_COMMON_ADV_RNG_OFFSET_CFG_RNG_OFFSET_POS(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_ANA_CL_2_COMMON_ADV_RNG_OFFSET_CFG_RNG_OFFSET_POS     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_ANA_CL_2_COMMON_ADV_RNG_OFFSET_CFG_RNG_OFFSET_POS(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief Configuration for the virtual queue
 *
 * \details
 * Register: \a ANA_CL_2:COMMON:VQ_CFG
 */
#define VTSS_ANA_CL_2_COMMON_VQ_CFG          VTSS_IOREG(VTSS_TO_ANA_CL,0x932)

/** 
 * \brief
 * Disable ageing inside of the virtual queue.
 *
 * \details 
 * 0: Ageing
 * 1: Ageing disabled
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_VQ_CFG . VQ_AGE_DIS
 */
#define  VTSS_F_ANA_CL_2_COMMON_VQ_CFG_VQ_AGE_DIS  VTSS_BIT(0)


/** 
 * \brief Contains the one shot bitgroups for the VQ
 *
 * \details
 * Register: \a ANA_CL_2:COMMON:VQ_ONE_SHOT
 */
#define VTSS_ANA_CL_2_COMMON_VQ_ONE_SHOT     VTSS_IOREG(VTSS_TO_ANA_CL,0x933)

/** 
 * \brief
 * One shot to initialise the VQ
 *
 * \details 
 * 0: No action
 * 1: Flush the Buffer of the VQ
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_VQ_ONE_SHOT . VQ_BUFFER_FLUSH
 */
#define  VTSS_F_ANA_CL_2_COMMON_VQ_ONE_SHOT_VQ_BUFFER_FLUSH  VTSS_BIT(0)


/** 
 * \brief VStaX  configruations
 *
 * \details
 * Register: \a ANA_CL_2:COMMON:IFH_CFG
 */
#define VTSS_ANA_CL_2_COMMON_IFH_CFG         VTSS_IOREG(VTSS_TO_ANA_CL,0x934)

/** 
 * \brief
 * Enable the ISDX lookup from the VStaX2 header inside of the s0 action
 * list.
 *
 * \details 
 * 0: Disabled
 * 1: Enabled
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_IFH_CFG . VSTAX_ISDX_ENA
 */
#define  VTSS_F_ANA_CL_2_COMMON_IFH_CFG_VSTAX_ISDX_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable the usage of the PCP and DEI field from the VStax2 header inside
 * of the s0.action request
 * for the frames which have the configured chip UPSID.
 * If this is disabled all VStax2 PCP and DEI values will be used
 * regardless of the UPSID.
 *
 * \details 
 * 0: Disabled
 * 1: Enabled
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_IFH_CFG . VSTAX_OWN_ONLY_ENA
 */
#define  VTSS_F_ANA_CL_2_COMMON_IFH_CFG_VSTAX_OWN_ONLY_ENA  VTSS_BIT(0)


/** 
 * \brief STAG type ctrl
 *
 * \details
 * Register: \a ANA_CL_2:COMMON:CUSTOM_STAG_ETYPE_CTRL
 *
 * @param ri Replicator: x_ANA_PORTS_DIV32_CEIL (??), 0-1
 */
#define VTSS_ANA_CL_2_COMMON_CUSTOM_STAG_ETYPE_CTRL(ri)  VTSS_IOREG(VTSS_TO_ANA_CL,0x935 + (ri))


/** 
 * \brief HMD port configurations
 *
 * \details
 * Register: \a ANA_CL_2:COMMON:HM_CFG
 *
 * @param ri Replicator: x_FFL_ANA_NUM_HIH_DEVS (??), 0-3
 */
#define VTSS_ANA_CL_2_COMMON_HM_CFG(ri)      VTSS_IOREG(VTSS_TO_ANA_CL,0x937 + (ri))

/** 
 * \brief
 * Contains the port number of the used HDMX port
 *
 * \details 
 * Field: ::VTSS_ANA_CL_2_COMMON_HM_CFG . HMD_PORT
 */
#define  VTSS_F_ANA_CL_2_COMMON_HM_CFG_HMD_PORT(x)  VTSS_ENCODE_BITFIELD(x,7,6)
#define  VTSS_M_ANA_CL_2_COMMON_HM_CFG_HMD_PORT     VTSS_ENCODE_BITMASK(7,6)
#define  VTSS_X_ANA_CL_2_COMMON_HM_CFG_HMD_PORT(x)  VTSS_EXTRACT_BITFIELD(x,7,6)

/** 
 * \brief
 * Enable the usage of the MPLS_BITW_MODE_VAL
 *
 * \details 
 * '0': Disabled
 * '1': Enabled
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_HM_CFG . MPLS_BITW_MODE_ENA
 */
#define  VTSS_F_ANA_CL_2_COMMON_HM_CFG_MPLS_BITW_MODE_ENA  VTSS_BIT(6)

/** 
 * \brief
 * Enable the usage of the HM_FORCE_MODE_VAL
 *
 * \details 
 * '0': Disabled
 * '1': Enabled
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_HM_CFG . HM_FORCE_MODE_ENA
 */
#define  VTSS_F_ANA_CL_2_COMMON_HM_CFG_HM_FORCE_MODE_ENA  VTSS_BIT(5)

/** 
 * \brief
 * This value is used as s0.pcp_dei_ena for this port.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_HM_CFG . HM_QOS_ENA
 */
#define  VTSS_F_ANA_CL_2_COMMON_HM_CFG_HM_QOS_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Enabel the configured port as HMD port
 *
 * \details 
 * '0': Disabled
 * '1': Enabled
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_HM_CFG . HMD_PORT_VLD
 */
#define  VTSS_F_ANA_CL_2_COMMON_HM_CFG_HMD_PORT_VLD  VTSS_BIT(3)

/** 
 * \brief
 * Configuration of the HMD mode.
 *
 * \details 
 * "00" : VIGR_PORT = HIH.LPID, PCP = HIH.PCP, DEI = HIH.DEI, Lookup at
 * port_default position.
 * "01" : VIGR_PORT = HIH.LPID, Other key value from frame data.
 * "10" :  VIGR_PORT = HIH.LPID, PCP = HIH.PCP, DEI = HIH.DEI,	Other key
 * value from frame data.
 * "11" : Use the HIH.CL[13:2] as direkt address for the action lookup.
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_HM_CFG . HM_FORCE_MODE_VAL
 */
#define  VTSS_F_ANA_CL_2_COMMON_HM_CFG_HM_FORCE_MODE_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_ANA_CL_2_COMMON_HM_CFG_HM_FORCE_MODE_VAL     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_ANA_CL_2_COMMON_HM_CFG_HM_FORCE_MODE_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


/** 
 * \brief BPDU/BPDU_AG/GARP QOS configuration
 *
 * \details
 * Configuration of QOS value for the various control frame types
 *
 * Register: \a ANA_CL_2:COMMON:BPDU_AG_GARP_QOS
 *
 * @param ri Register: BPDU_AG_GARP_QOS (??), 0-15
 */
#define VTSS_ANA_CL_2_COMMON_BPDU_AG_GARP_QOS(ri)  VTSS_IOREG(VTSS_TO_ANA_CL,0x93b + (ri))

/** 
 * \brief
 * Configures QOS for address in GARP range
 *
 * \details 
 * QOS value
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_BPDU_AG_GARP_QOS . GARP_QOS
 */
#define  VTSS_F_ANA_CL_2_COMMON_BPDU_AG_GARP_QOS_GARP_QOS(x)  VTSS_ENCODE_BITFIELD(x,6,3)
#define  VTSS_M_ANA_CL_2_COMMON_BPDU_AG_GARP_QOS_GARP_QOS     VTSS_ENCODE_BITMASK(6,3)
#define  VTSS_X_ANA_CL_2_COMMON_BPDU_AG_GARP_QOS_GARP_QOS(x)  VTSS_EXTRACT_BITFIELD(x,6,3)

/** 
 * \brief
 * Configures QOS for address in BPDU_AG range
 *
 * \details 
 * QOS value
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_BPDU_AG_GARP_QOS . BPDU_AG_QOS
 */
#define  VTSS_F_ANA_CL_2_COMMON_BPDU_AG_GARP_QOS_BPDU_AG_QOS(x)  VTSS_ENCODE_BITFIELD(x,3,3)
#define  VTSS_M_ANA_CL_2_COMMON_BPDU_AG_GARP_QOS_BPDU_AG_QOS     VTSS_ENCODE_BITMASK(3,3)
#define  VTSS_X_ANA_CL_2_COMMON_BPDU_AG_GARP_QOS_BPDU_AG_QOS(x)  VTSS_EXTRACT_BITFIELD(x,3,3)

/** 
 * \brief
 * Configures QOS for address in BPDU range
 *
 * \details 
 * QOS value
 *
 * Field: ::VTSS_ANA_CL_2_COMMON_BPDU_AG_GARP_QOS . BPDU_QOS
 */
#define  VTSS_F_ANA_CL_2_COMMON_BPDU_AG_GARP_QOS_BPDU_QOS(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_ANA_CL_2_COMMON_BPDU_AG_GARP_QOS_BPDU_QOS     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_ANA_CL_2_COMMON_BPDU_AG_GARP_QOS_BPDU_QOS(x)  VTSS_EXTRACT_BITFIELD(x,0,3)

/**
 * Register Group: \a ANA_CL_2:STICKY
 *
 * Sticky diagnostic status
 */


/** 
 * \brief Sticky bits register
 *
 * \details
 * Register: \a ANA_CL_2:STICKY:FILTER_STICKY
 */
#define VTSS_ANA_CL_2_STICKY_FILTER_STICKY   VTSS_IOREG(VTSS_TO_ANA_CL,0x94b)

/** 
 * \brief
 * Set if a frame has been filtered due to number of expected tag exceeded.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_FILTER_STICKY . FILTER_MAX_QTAG_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_FILTER_STICKY_FILTER_MAX_QTAG_STICKY  VTSS_BIT(20)

/** 
 * \brief
 * Set if a frame has been filtered due to wrong stacking information (e.g.
 * not expected stacking tag etc.).
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_FILTER_STICKY . STACKING_FILTER_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_FILTER_STICKY_STACKING_FILTER_STICKY  VTSS_BIT(19)

/** 
 * \brief
 * Set if a frame has been aborted due to IFH Checksum err.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_FILTER_STICKY . IFH_CHK_FILTER_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_FILTER_STICKY_IFH_CHK_FILTER_STICKY  VTSS_BIT(18)

/** 
 * \brief
 * Set if a frame is dropped due to zero MAC addresses filtering or
 * multicast source MAC address filtering.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_FILTER_STICKY . BAD_MACS_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_FILTER_STICKY_BAD_MACS_STICKY  VTSS_BIT(17)

/** 
 * \brief
 * Set if a frame is dropped due to single tag filtering.
 *
 * \details 
 * '0': No Event
 * '1': Event
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_FILTER_STICKY . FILTER_SINGLE_TAG_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_FILTER_STICKY_FILTER_SINGLE_TAG_STICKY  VTSS_BIT(16)

/** 
 * \brief
 * Set if a frame is dropped due to inner priority C-TAG filtering.
 *
 * \details 
 * '0': No event
 * '1': Event
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_FILTER_STICKY . FILTER_IN_PRIO_CTAG_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_FILTER_STICKY_FILTER_IN_PRIO_CTAG_STICKY  VTSS_BIT(15)

/** 
 * \brief
 * Set if a frame is dropped due to inner C-TAG filtering.
 *
 * \details 
 * '0': <text>
 * '1': <text>
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_FILTER_STICKY . FILTER_IN_CTAG_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_FILTER_STICKY_FILTER_IN_CTAG_STICKY  VTSS_BIT(14)

/** 
 * \brief
 * Set if a frame is dropped due to inner I-TAG filtering.
 *
 * \details 
 * '0': No event
 * '1': Event
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_FILTER_STICKY . FILTER_IN_ITAG_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_FILTER_STICKY_FILTER_IN_ITAG_STICKY  VTSS_BIT(11)

/** 
 * \brief
 * Set if a frame is dropped due inner priority S-TAG filtering.
 *
 * \details 
 * '0': No event
 * '1': Event
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_FILTER_STICKY . FILTER_IN_PRIO_STAG_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_FILTER_STICKY_FILTER_IN_PRIO_STAG_STICKY  VTSS_BIT(10)

/** 
 * \brief
 * Set if a frame is dropped due to inner S-TAG filtering.
 *
 * \details 
 * '0': No event
 * '1': Event
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_FILTER_STICKY . FILTER_IN_STAG_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_FILTER_STICKY_FILTER_IN_STAG_STICKY  VTSS_BIT(9)

/** 
 * \brief
 * Set when frame is dropped due to untagged frame filtering.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_FILTER_STICKY . FILTER_UNTAGGED_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_FILTER_STICKY_FILTER_UNTAGGED_STICKY  VTSS_BIT(8)

/** 
 * \brief
 * Set if a frame is dropped due to outer I-TAG filtering.
 *
 * \details 
 * '0': No event
 * '1': Event
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_FILTER_STICKY . FILTER_OUT_ITAG_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_FILTER_STICKY_FILTER_OUT_ITAG_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * Set when frame is dropped due to outer priority C-TAG filtering.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_FILTER_STICKY . FILTER_OUT_PRIO_CTAG_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_FILTER_STICKY_FILTER_OUT_PRIO_CTAG_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Set when frame is dropped due to outer C-TAG filtering.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_FILTER_STICKY . FILTER_OUT_CTAG_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_FILTER_STICKY_FILTER_OUT_CTAG_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * Set if a frame is dropped due to outer priority S-TAG filtering.
 *
 * \details 
 * '0': No event seen
 * '1': Event 
 * Bit is cleared by writing a '1' to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_FILTER_STICKY . FILTER_OUT_PRIO_STAG_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_FILTER_STICKY_FILTER_OUT_PRIO_STAG_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Set when frame is dropped due to outer S-TAG filtering.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_FILTER_STICKY . FILTER_OUT_STAG_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_FILTER_STICKY_FILTER_OUT_STAG_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Set if a frame has been filtered due to number tags exceeded the
 * expected number of identical tags.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_FILTER_STICKY . FILTER_MAX_TAG_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_FILTER_STICKY_FILTER_MAX_TAG_STICKY  VTSS_BIT(0)


/** 
 * \brief Sticky bits register
 *
 * \details
 * Register: \a ANA_CL_2:STICKY:CLASS_STICKY
 */
#define VTSS_ANA_CL_2_STICKY_CLASS_STICKY    VTSS_IOREG(VTSS_TO_ANA_CL,0x94c)

/** 
 * \brief
 * Set when S0 BVID was used as BVID
 *
 * \details 
 * 0: The event has not occured
 * 1: The event did occour
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CLASS_STICKY . CLASS_S0_BVID_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CLASS_STICKY_CLASS_S0_BVID_STICKY  VTSS_BIT(25)

/** 
 * \brief
 * This sticky bit indicates that the DSCP value from the S1 was used.
 *
 * \details 
 * 0: No event
 * 1: DSCP from the S1 action was used
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CLASS_STICKY . S1_DSCP_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CLASS_STICKY_S1_DSCP_STICKY  VTSS_BIT(24)

/** 
 * \brief
 * Set when the frame DSCP value is used.
 *
 * \details 
 * 0: No event
 * 1: Frame DSCP value has beeen used.
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CLASS_STICKY . FRAME_DSCP_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CLASS_STICKY_FRAME_DSCP_STICKY  VTSS_BIT(23)

/** 
 * \brief
 * Set when the  DSCP value is based on the DSCP_TRANSLATE_VAL.
 *
 * \details 
 * 0: No event
 * 1: DSCP_TRANSLATE_VAL has been used.
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CLASS_STICKY . DSCP_TRANSLATE_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CLASS_STICKY_DSCP_TRANSLATE_STICKY  VTSS_BIT(22)

/** 
 * \brief
 * This sticky bit indicates that the PCP and DEI value from the S0 action
 * was used as lookup into the UPRIO_CFI_QOS_VAL to determine the QOS
 * value.
 *
 * \details 
 * 0: No event
 * 1: S0 PCP and DEI  has been used to determine the QOS value
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CLASS_STICKY . S0_PCP_DEI_QOS_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CLASS_STICKY_S0_PCP_DEI_QOS_STICKY  VTSS_BIT(21)

/** 
 * \brief
 * This sticky bit indicates that the QOS value from the S1 was used.
 *
 * \details 
 * 0: No event
 * 1: QOS value from the S1 was used
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CLASS_STICKY . S1_QOS_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CLASS_STICKY_S1_QOS_STICKY  VTSS_BIT(20)

/** 
 * \brief
 * Set when classified PCP and DEI is based on the S1_action result.
 *
 * \details 
 * 0: The event has not occured
 * 1: The event did occour
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CLASS_STICKY . CLASS_S1_PCPDEI_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CLASS_STICKY_CLASS_S1_PCPDEI_STICKY  VTSS_BIT(19)

/** 
 * \brief
 * Set when VID is changed base on the s1 result
 *
 * \details 
 * 0: The event has not occured
 * 1: The event did occour
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CLASS_STICKY . CLASS_S1_VID_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CLASS_STICKY_CLASS_S1_VID_STICKY  VTSS_BIT(18)

/** 
 * \brief
 * Set when classified PCP and DEI is based on the S0_action result.
 *
 * \details 
 * 0: The event has not occured
 * 1: The event did occour
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CLASS_STICKY . CLASS_S0_PCPDEI_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CLASS_STICKY_CLASS_S0_PCPDEI_STICKY  VTSS_BIT(17)

/** 
 * \brief
 * Set when classified VID is based on the S0_action result.
 *
 * \details 
 * 0: The event has not occured
 * 1: The event did occour
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CLASS_STICKY . CLASS_S0_VID_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CLASS_STICKY_CLASS_S0_VID_STICKY  VTSS_BIT(16)

/** 
 * \brief
 * Set when classified VID was used as BVID
 *
 * \details 
 * 0: The event has not occured
 * 1: The event did occour
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CLASS_STICKY . CLASS_BVID_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CLASS_STICKY_CLASS_BVID_STICKY  VTSS_BIT(15)

/** 
 * \brief
 * Set when a default port lookup was used.
 *
 * \details 
 * 0: The event has not occured
 * 1: The event did occour
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CLASS_STICKY . DEFAULT_PORT_S0_LOOKUP_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CLASS_STICKY_DEFAULT_PORT_S0_LOOKUP_STICKY  VTSS_BIT(14)

/** 
 * \brief
 * Set when HM_CL_MODE_01 was used.
 *
 * \details 
 * 0: The event has not occured
 * 1: The event did occour
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CLASS_STICKY . HM_CL_MODE_01_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CLASS_STICKY_HM_CL_MODE_01_STICKY  VTSS_BIT(12)

/** 
 * \brief
 * Set when HM_CL_MODE 10 was used.
 *
 * \details 
 * 0: The event has not occured
 * 1: The event did occour
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CLASS_STICKY . HM_CL_MODE_10_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CLASS_STICKY_HM_CL_MODE_10_STICKY  VTSS_BIT(11)

/** 
 * \brief
 * Set when HM_CL_MODE_11 was used.
 *
 * \details 
 * 0: The event has not occured
 * 1: The event did occour
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CLASS_STICKY . HM_CL_MODE_11_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CLASS_STICKY_HM_CL_MODE_11_STICKY  VTSS_BIT(10)

/** 
 * \brief
 * Set when HM_CL_MODE_00 was used.
 *
 * \details 
 * 0: The event has not occured
 * 1: The event did occour
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CLASS_STICKY . HM_CL_MODE_00_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CLASS_STICKY_HM_CL_MODE_00_STICKY  VTSS_BIT(9)

/** 
 * \brief
 * Set when the port VLAN has been used.
 *
 * \details 
 * 0: The event has not occured
 * 1: Port default value has been used for VLAN classification
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CLASS_STICKY . VID_PORT_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CLASS_STICKY_VID_PORT_STICKY  VTSS_BIT(7)

/** 
 * \brief
 * Set when the VID information from Stacking TAG has been used.
 *
 * \details 
 * 0: The event has not occured
 * 1: Stacking Header VID has been used for VLAN classification
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CLASS_STICKY . VID_STACK_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CLASS_STICKY_VID_STACK_STICKY  VTSS_BIT(6)

/** 
 * \brief
 * Set when vid from the TAG has been used.
 *
 * \details 
 * 0: The event has not occured
 * 1: TCI VID value has been used for VLAN classification
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CLASS_STICKY . VID_TAG_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CLASS_STICKY_VID_TAG_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * Set when the classified UPRIO and CFI value is used to to determeine the
 * QOS value.
 *
 * \details 
 * 0: No event
 * 1: USER priority QOS value has been used for QOS classification
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CLASS_STICKY . QOS_UPRIO_CFI_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CLASS_STICKY_QOS_UPRIO_CFI_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Set when the default port QOS has been used.
 *
 * \details 
 * 0: No event
 * 1: Port default has been used for QOS classification
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CLASS_STICKY . QOS_DEFAULT_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CLASS_STICKY_QOS_DEFAULT_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * This sticky bit indicates that the QOS value was used as index into the
 * DSCP_REWR_VAL table to determine the DSCP value.
 *
 * \details 
 * 0: No event
 * 1: QOS value has been used to determine the DSCP
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CLASS_STICKY . DSCP_QOS_REWR_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CLASS_STICKY_DSCP_QOS_REWR_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Set when the DSCP QOS has been used.
 *
 * \details 
 * 0: No event
 * 1: DSCP QOS value has been used for QOS classification
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CLASS_STICKY . QOS_DSCP_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CLASS_STICKY_QOS_DSCP_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Set when the stacking QOS has been used.
 *
 * \details 
 * 0: No event
 * 1: Stacking header QOS value has been used for QOS classification
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CLASS_STICKY . QOS_STACK_TAG_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CLASS_STICKY_QOS_STACK_TAG_STICKY  VTSS_BIT(0)


/** 
 * \brief Sticky bits register
 *
 * \details
 * Register: \a ANA_CL_2:STICKY:CAT_STICKY
 */
#define VTSS_ANA_CL_2_STICKY_CAT_STICKY      VTSS_IOREG(VTSS_TO_ANA_CL,0x94d)

/** 
 * \brief
 * Indicates that a frame with a VStax2 TTL value of 0 was dropped.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CAT_STICKY . VSTAX2_TTL_ZERO_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CAT_STICKY_VSTAX2_TTL_ZERO_STICKY  VTSS_BIT(9)

/** 
 * \brief
 * Set if an IGMP frame has been detected.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CAT_STICKY . IGMP_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CAT_STICKY_IGMP_STICKY  VTSS_BIT(8)

/** 
 * \brief
 * This bit is set if a 802.1ag control frame  (DMAC in the range
 * 01-80-C2-00-00-30 to 01-80-C2-00-00-3F) has been detected.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CAT_STICKY . AG_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CAT_STICKY_AG_STICKY  VTSS_BIT(7)

/** 
 * \brief
 * This bit is set if a BPDU control frame  (DMAC in the range
 * 01-80-C2-00-00-00 to 01-80-C2-00-00-0F) has been detected.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CAT_STICKY . BPDU_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CAT_STICKY_BPDU_STICKY  VTSS_BIT(6)

/** 
 * \brief
 * This bit is set if a GxRP frame ( DMAC in the range 01-80-C2-00-00-20 to
 * 01-80-C2-00-00-2F) has been detected.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CAT_STICKY . GXRP_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CAT_STICKY_GXRP_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * Set if an IP Multicast control frame has been detected.
 *   - DMAC = 0x01005Exxxxxx , Ethernet Type = IP, IP Protocol != IGMP, DIP
 * = 224.0.0.x
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CAT_STICKY . IP4_MC_CTRL_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CAT_STICKY_IP4_MC_CTRL_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Set if an MLD frame has been detected.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CAT_STICKY . MLD_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CAT_STICKY_MLD_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * Indicates that a IPv6 frame with hop by hop options and ICMPv6 was seen.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CAT_STICKY . IP6_ICMP_HOP_BY_HOP_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CAT_STICKY_IP6_ICMP_HOP_BY_HOP_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Is set if an IPv6 frame with a hop by hop header and ICMPv6 was seen.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_CAT_STICKY . IP6_HOP_BY_HOP_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_CAT_STICKY_IP6_HOP_BY_HOP_STICKY  VTSS_BIT(1)


/** 
 * \brief Sticky bits register
 *
 * \details
 * Register: \a ANA_CL_2:STICKY:IP_HDR_CHK_STICKY
 */
#define VTSS_ANA_CL_2_STICKY_IP_HDR_CHK_STICKY  VTSS_IOREG(VTSS_TO_ANA_CL,0x94e)

/** 
 * \brief
 * Set if an IP packet with options is found.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_IP_HDR_CHK_STICKY . IP_OPTIONS_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_IP_HDR_CHK_STICKY_IP_OPTIONS_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * Set if an IP fragmented frame is found.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_IP_HDR_CHK_STICKY . IP4_FRAGMENT_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_IP_HDR_CHK_STICKY_IP4_FRAGMENT_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Set if IP total length is less that IP header length.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_IP_HDR_CHK_STICKY . IP4_LEN_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_IP_HDR_CHK_STICKY_IP4_LEN_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Set if an IP checksum error is found.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_IP_HDR_CHK_STICKY . IP4_CHKSUM_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_IP_HDR_CHK_STICKY_IP4_CHKSUM_STICKY  VTSS_BIT(0)


/** 
 * \brief Sticky bits register
 *
 * \details
 * Register: \a ANA_CL_2:STICKY:VQ_STICKY_REG
 */
#define VTSS_ANA_CL_2_STICKY_VQ_STICKY_REG   VTSS_IOREG(VTSS_TO_ANA_CL,0x94f)

/** 
 * \brief
 * Indicates that VQ traffic was seen
 *
 * \details 
 * 0: No VC traffic seen
 * 1: VC traffic seen
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_VQ_STICKY_REG . VQ_TRAFFIC_SEEN_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_VQ_STICKY_REG_VQ_TRAFFIC_SEEN_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Indicates that a overlow of the VC did take place.
 *
 * \details 
 * 0: No overlow
 * 1: Overlow seen
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_VQ_STICKY_REG . VQ_OFLW_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_VQ_STICKY_REG_VQ_OFLW_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Aging inside of the VQ did take place.
 *
 * \details 
 * 0: No ageing
 * 1: Ageing done
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_VQ_STICKY_REG . VQ_AGE_STICKY
 */
#define  VTSS_F_ANA_CL_2_STICKY_VQ_STICKY_REG_VQ_AGE_STICKY  VTSS_BIT(0)


/** 
 * \brief Sticky bits register
 *
 * \details
 * Register: \a ANA_CL_2:STICKY:MISC_CONF_STICKY
 */
#define VTSS_ANA_CL_2_STICKY_MISC_CONF_STICKY  VTSS_IOREG(VTSS_TO_ANA_CL,0x950)

/** 
 * \brief
 * Indicates that a the masquaraded port number of a cpu injected frame 
 * was out of range. 
 * If this bit is set then the the frame will be handled as inject_normal.
 *
 * \details 
 * 0: Masquraded port number was not out of range
 * 1:  Masquraded port number was not of range
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MISC_CONF_STICKY . MASQ_PORT_NUM_OUT_OF_RANGE
 */
#define  VTSS_F_ANA_CL_2_STICKY_MISC_CONF_STICKY_MASQ_PORT_NUM_OUT_OF_RANGE  VTSS_BIT(1)

/**
 * Register Group: \a ANA_CL_2:STICKY_MASK
 *
 * Sticky diagnostic global port counter event configuration
 */


/** 
 * \brief Sticky counter mask register
 *
 * \details
 * Register: \a ANA_CL_2:STICKY_MASK:FILTER_STICKY_MASK
 *
 * @param gi Replicator: x_ANA_NUM_CONCURRENT_CNT (??), 0-3
 */
#define VTSS_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x951,gi,4,0,0)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK . FILTER_MAX_QTAG_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK_FILTER_MAX_QTAG_STICKY_MASK  VTSS_BIT(22)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK . IFH_CHK_FILTER_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK_IFH_CHK_FILTER_STICKY_MASK  VTSS_BIT(21)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK . STACKING_FILTER_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK_STACKING_FILTER_STICKY_MASK  VTSS_BIT(20)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK . BAD_MACS_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK_BAD_MACS_STICKY_MASK  VTSS_BIT(17)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK . FILTER_SINGLE_TAG_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK_FILTER_SINGLE_TAG_STICKY_MASK  VTSS_BIT(16)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * '0': Disable event count
 * '1': Enabale event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK . FILTER_IN_ITAG_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK_FILTER_IN_ITAG_STICKY_MASK  VTSS_BIT(13)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK . FILTER_IN_PRIO_CTAG_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK_FILTER_IN_PRIO_CTAG_STICKY_MASK  VTSS_BIT(12)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * '0': Disable event count
 * '1': Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK . FILTER_IN_CTAG_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK_FILTER_IN_CTAG_STICKY_MASK  VTSS_BIT(11)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * '0': Disable event count
 * '1': Enabale event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK . FILTER_IN_PRIO_STAG_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK_FILTER_IN_PRIO_STAG_STICKY_MASK  VTSS_BIT(10)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * '0': Disable event count
 * '1': Enabale event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK . FILTER_IN_STAG_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK_FILTER_IN_STAG_STICKY_MASK  VTSS_BIT(9)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK . FILTER_UNTAGGED_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK_FILTER_UNTAGGED_STICKY_MASK  VTSS_BIT(8)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * '0': Disable event count
 * '1': Enabale event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK . FILTER_OUT_ITAG_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK_FILTER_OUT_ITAG_STICKY_MASK  VTSS_BIT(5)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK . FILTER_OUT_PRIO_CTAG_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK_FILTER_OUT_PRIO_CTAG_STICKY_MASK  VTSS_BIT(4)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK . FILTER_OUT_CTAG_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK_FILTER_OUT_CTAG_STICKY_MASK  VTSS_BIT(3)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK . FILTER_OUT_PRIO_STAG_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK_FILTER_OUT_PRIO_STAG_STICKY_MASK  VTSS_BIT(2)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK . FILTER_OUT_STAG_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK_FILTER_OUT_STAG_STICKY_MASK  VTSS_BIT(1)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK . FILTER_MAX_TAG_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_FILTER_STICKY_MASK_FILTER_MAX_TAG_STICKY_MASK  VTSS_BIT(0)


/** 
 * \brief Sticky counter mask register
 *
 * \details
 * Register: \a ANA_CL_2:STICKY_MASK:CLASS_STICKY_MASK
 *
 * @param gi Replicator: x_ANA_NUM_CONCURRENT_CNT (??), 0-3
 */
#define VTSS_ANA_CL_2_STICKY_MASK_CLASS_STICKY_MASK(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x951,gi,4,0,1)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CLASS_STICKY_MASK . VID_STACK_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CLASS_STICKY_MASK_VID_STACK_STICKY_MASK  VTSS_BIT(6)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CLASS_STICKY_MASK . VID_PORT_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CLASS_STICKY_MASK_VID_PORT_STICKY_MASK  VTSS_BIT(5)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CLASS_STICKY_MASK . VID_TAG_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CLASS_STICKY_MASK_VID_TAG_STICKY_MASK  VTSS_BIT(4)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CLASS_STICKY_MASK . QOS_DEFAULT_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CLASS_STICKY_MASK_QOS_DEFAULT_STICKY_MASK  VTSS_BIT(3)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CLASS_STICKY_MASK . QOS_UPRIO_CFI_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CLASS_STICKY_MASK_QOS_UPRIO_CFI_STICKY_MASK  VTSS_BIT(2)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CLASS_STICKY_MASK . QOS_DSCP_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CLASS_STICKY_MASK_QOS_DSCP_STICKY_MASK  VTSS_BIT(1)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CLASS_STICKY_MASK . QOS_STACK_TAG_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CLASS_STICKY_MASK_QOS_STACK_TAG_STICKY_MASK  VTSS_BIT(0)


/** 
 * \brief Sticky counter mask register
 *
 * \details
 * Register: \a ANA_CL_2:STICKY_MASK:CAT_STICKY_MASK
 *
 * @param gi Replicator: x_ANA_NUM_CONCURRENT_CNT (??), 0-3
 */
#define VTSS_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x951,gi,4,0,2)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK . CLASS_S0_BVID_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK_CLASS_S0_BVID_STICKY_MASK  VTSS_BIT(27)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK . AG_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK_AG_STICKY_MASK  VTSS_BIT(26)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK . CLASS_S0_VID_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK_CLASS_S0_VID_STICKY_MASK  VTSS_BIT(25)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK . HM_CL_MODE_00_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK_HM_CL_MODE_00_STICKY_MASK  VTSS_BIT(24)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK . CLASS_S1_PCPDEI_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK_CLASS_S1_PCPDEI_STICKY_MASK  VTSS_BIT(23)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK . S0_PCP_DEI_QOS_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK_S0_PCP_DEI_QOS_STICKY_MASK  VTSS_BIT(22)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK . HM_CL_MODE_10_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK_HM_CL_MODE_10_STICKY_MASK  VTSS_BIT(19)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK . CLASS_S1_VID_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK_CLASS_S1_VID_STICKY_MASK  VTSS_BIT(18)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK . DSCP_TRANSLATE_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK_DSCP_TRANSLATE_STICKY_MASK  VTSS_BIT(17)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK . CLASS_S0_PCPDEI_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK_CLASS_S0_PCPDEI_STICKY_MASK  VTSS_BIT(16)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK . BPDU_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK_BPDU_STICKY_MASK  VTSS_BIT(15)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK . DSCP_QOS_REWR_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK_DSCP_QOS_REWR_STICKY_MASK  VTSS_BIT(14)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK . S1_DSCP_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK_S1_DSCP_STICKY_MASK  VTSS_BIT(12)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK . CLASS_BVID_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK_CLASS_BVID_STICKY_MASK  VTSS_BIT(11)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK . HM_CL_MODE_01_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK_HM_CL_MODE_01_STICKY_MASK  VTSS_BIT(10)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK . IGMP_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK_IGMP_STICKY_MASK  VTSS_BIT(9)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK . VSTAX2_TTL_ZERO_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK_VSTAX2_TTL_ZERO_STICKY_MASK  VTSS_BIT(8)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK . HM_CL_MODE_11_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK_HM_CL_MODE_11_STICKY_MASK  VTSS_BIT(7)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK . S1_QOS_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK_S1_QOS_STICKY_MASK  VTSS_BIT(6)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK . GXRP_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK_GXRP_STICKY_MASK  VTSS_BIT(5)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK . IP4_MC_CTRL_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK_IP4_MC_CTRL_STICKY_MASK  VTSS_BIT(4)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK . MLD_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK_MLD_STICKY_MASK  VTSS_BIT(3)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK . IP6_ICMP_HOP_BY_HOP_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK_IP6_ICMP_HOP_BY_HOP_STICKY_MASK  VTSS_BIT(2)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK . IP6_HOP_BY_HOP_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_CAT_STICKY_MASK_IP6_HOP_BY_HOP_STICKY_MASK  VTSS_BIT(1)


/** 
 * \brief Sticky counter mask register
 *
 * \details
 * Register: \a ANA_CL_2:STICKY_MASK:IP_HDR_CHK_STICKY_MASK
 *
 * @param gi Replicator: x_ANA_NUM_CONCURRENT_CNT (??), 0-3
 */
#define VTSS_ANA_CL_2_STICKY_MASK_IP_HDR_CHK_STICKY_MASK(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x951,gi,4,0,3)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_IP_HDR_CHK_STICKY_MASK . IP_OPTIONS_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_IP_HDR_CHK_STICKY_MASK_IP_OPTIONS_STICKY_MASK  VTSS_BIT(3)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_IP_HDR_CHK_STICKY_MASK . IP4_FRAGMENT_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_IP_HDR_CHK_STICKY_MASK_IP4_FRAGMENT_STICKY_MASK  VTSS_BIT(2)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_IP_HDR_CHK_STICKY_MASK . IP4_LEN_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_IP_HDR_CHK_STICKY_MASK_IP4_LEN_STICKY_MASK  VTSS_BIT(1)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_2_STICKY_MASK_IP_HDR_CHK_STICKY_MASK . IP4_CHKSUM_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_2_STICKY_MASK_IP_HDR_CHK_STICKY_MASK_IP4_CHKSUM_STICKY_MASK  VTSS_BIT(0)


#endif /* _VTSS_JAGUAR_REGS_ANA_CL_2_H_ */
