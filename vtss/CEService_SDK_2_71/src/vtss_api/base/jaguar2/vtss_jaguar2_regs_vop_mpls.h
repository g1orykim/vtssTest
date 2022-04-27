#ifndef _VTSS_JAGUAR2_REGS_VOP_MPLS_H_
#define _VTSS_JAGUAR2_REGS_VOP_MPLS_H_

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
 * Target: \a VOP_MPLS
 *
 * The Vitesse OAM MEP Processor (VOP) implements the HW support for
 * implementing OAM MEPs.
 * 
 * The HW support for implementing an OAM MEP is implemented in a sub
 * block, referred to as:
 * Vitesse OAM Endpoint (VOE).
 * 
 * The VOP includes the following:
 * 
 *  * 1024 Service / Path VOEs
 *  * 53 Port VOEs (52 front ports + NPI)
 *
 ***********************************************************************/

/**
 * Register Group: \a VOP_MPLS:VOE_CONF_MPLS
 *
 * Configuration per Vitesse OAM MEP Endpoints (VOE) for MPLS-TP OAM
 */


/** 
 * \brief Misc. VOE control configuration
 *
 * \details
 * This register includes configuration of misc. VOE control properties.
 *
 * Register: \a VOP_MPLS:VOE_CONF_MPLS:VOE_CTRL_MPLS
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_CONF_MPLS_VOE_CTRL_MPLS(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x10000,gi,16,0,0)

/** 
 * \brief
 * Enables VOE functionality.
 * 
 * When the VOE is not enabled, it will not do any OAM processing or update
 * statistics.
 * 
 * The VOE can be configured while not enabled.
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_VOE_CTRL_MPLS . VOE_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_VOE_CTRL_MPLS_VOE_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Configures VOE for Down-MEP or Up-MEP functionality.
 * 
 * NOTE:
 * Port VOE may NOT be configured for Up-MEP functionality, they only
 * support Down-MEP implementation.
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_VOE_CTRL_MPLS . UPMEP_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_VOE_CTRL_MPLS_UPMEP_ENA  VTSS_BIT(1)

/** 
 * \brief
 * If another VOE is pointing to this VOE as a PATH VOE using the following
 * configuration:
 * 
 *  - VOE_CONF_MPLS:PATH_VOE_MPLS.PATH.VOEID
 *  - VOE_CONF_MPLS:PATH_VOE_MPLS.PATH.VOEID_ENA
 * 
 * this register MUST be set to '1'.
 * 
 * If not this register must be set to '
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_VOE_CTRL_MPLS . VOE_IS_PATH
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_VOE_CTRL_MPLS_VOE_IS_PATH  VTSS_BIT(0)


/** 
 * \brief CPU extraction for the supported MPLS-TP OAM Codepoints.
 *
 * \details
 * Configures CPU copy for the supported MPLS-TP PDU Codepoints.
 * 
 * Configuring a PDU type for CPU extraction, will result in all valid OAM
 * PDUs of this type to extracted to the CPU.
 * 
 * The PDU will be to the extraction queue configured for the Codepoint in
 * the following registers:
 * 
 *  - OAM_MEP::COMMON:CPU_EXTR_MPLS.*
 * 
 * In-valid OAM PDUs are not extracted based on the configuration in this
 * register group. OAM PDUs are considered invalid if they fail either of
 * the following checks:
 * 
 *  * Protocol Specific verification (E.g. BFD RX / TX verification)
 *
 * Register: \a VOP_MPLS:VOE_CONF_MPLS:CPU_COPY_CTRL_MPLS
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_CONF_MPLS_CPU_COPY_CTRL_MPLS(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x10000,gi,16,0,1)

/** 
 * \brief
 * This bit field contains 8 bits each of which represent one of the
 * Generic Codepoints.
 * 
 * If the bit representing a specific generic Codepoint is asserted, all
 * valid PDUs received by the VOE of that type are extracted to the CPU
 * queue configured in the following bit fields:
 * 
 *  - OAM_MEP:COMMON:OAM_GENERIC_CFG.GENERIC_OPCODE_CPU_QU
 *
 * \details 
 * 0: No CPU copy
 * x1x: Copy to CPU
 *
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_CPU_COPY_CTRL_MPLS . GENERIC_COPY_MASK
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_CPU_COPY_CTRL_MPLS_GENERIC_COPY_MASK(x)  VTSS_ENCODE_BITFIELD(x,3,8)
#define  VTSS_M_VOP_MPLS_VOE_CONF_MPLS_CPU_COPY_CTRL_MPLS_GENERIC_COPY_MASK     VTSS_ENCODE_BITMASK(3,8)
#define  VTSS_X_VOP_MPLS_VOE_CONF_MPLS_CPU_COPY_CTRL_MPLS_GENERIC_COPY_MASK(x)  VTSS_EXTRACT_BITFIELD(x,3,8)

/** 
 * \brief
 * Configures whether MPLS-TP OAM PDUs with UNKNOWN codepoint should be
 * extracted to the CPU.
 * 
 * Extracted frames are extracted to the default CPU queue, configured in:
 * 
 *  - CPU_EXTR_CFG.DEF_COPY_QU
 *
 * \details 
 * '0': No CPU copy
 * '1': Copy to CPU
 *
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_CPU_COPY_CTRL_MPLS . UNK_CPT_CPU_COPY_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_CPU_COPY_CTRL_MPLS_UNK_CPT_CPU_COPY_ENA  VTSS_BIT(2)

/** 
 * \brief
 * If asserted all valid BFD CC PDUs received by the VOE are extracted to
 * the CPU.
 * 
 * Extraction queue is determined by : 
 *  * CPU_EXTR_MPLS.BFD_CC_CPU_QU
 *
 * \details 
 * '0': No extraction to CPU
 * '1': Extract valid BFD CC PDUs to CPU
 *
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_CPU_COPY_CTRL_MPLS . BFD_CC_CPU_COPY_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_CPU_COPY_CTRL_MPLS_BFD_CC_CPU_COPY_ENA  VTSS_BIT(1)

/** 
 * \brief
 * If asserted all valid BFD CV PDUs received by the VOE are extracted to
 * the CPU.
 * 
 * Extraction queue is determined by : 
 *  * CPU_EXTR_MPLS.BFD_CV_CPU_QU
 *
 * \details 
 * '0': No extraction to CPU
 * '1': Extract valid BFD CV frames to CPU
 *
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_CPU_COPY_CTRL_MPLS . BFD_CV_CPU_COPY_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_CPU_COPY_CTRL_MPLS_BFD_CV_CPU_COPY_ENA  VTSS_BIT(0)


/** 
 * \brief OAM HW processing control
 *
 * \details
 * Configures per MPLS-TP codepoingif it is processed by VOE hardware.
 * 
 * If a MPLS-TP OAM type is not enabled in this register, the OAM PDU will
 * not be processed by the VOE.
 * 
 * This implies that the OAM PDU is not updated, and that PDU specific
 * registers are not updated.
 * 
 * However, note the following:
 *  * The RX sticky bits will be set for a PDU, even when the HW processing
 * is not enabled.
 *  * OAM PDU can be extracted to the CPU, even when HW processing is not
 * enabled.
 *  * LM counters will be updated.
 *  * SEL / non SEL counters are not updated when the PDU type is not
 * enabled.
 *
 * Register: \a VOP_MPLS:VOE_CONF_MPLS:OAM_HW_CTRL_MPLS
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_CONF_MPLS_OAM_HW_CTRL_MPLS(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x10000,gi,16,0,2)

/** 
 * \brief
 * Enable HW processing of valid BFD CC PDUs received by the VOE in both
 * the TX and the RX direction.
 * 
 * If this is disabled, all BFD CC PDUs no verification of the
 * YourDiscriminator is done of the incoming BFD CC PDUs and all will be
 * processed as belonging to the Coordinated Mode.
 * 
 * I.e. a BFD CC PDU's will never be processed as belonging to the FEIS
 * Session.
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_OAM_HW_CTRL_MPLS . BFD_CC_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_OAM_HW_CTRL_MPLS_BFD_CC_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable HW processing of valid BFD CV PDUs received by the VOE in both
 * the TX and the RX direction.
 * 
 * If this is disabled, all BFD CV PDUs no verification of the
 * YourDiscriminator is done of the incoming BFD CV PDUs and all will be
 * processed as belonging to the Coordinated Mode.
 * 
 * I.e. a BFD CV PDU's will never be processed as belonging to the FEIS
 * Session.
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_OAM_HW_CTRL_MPLS . BFD_CV_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_OAM_HW_CTRL_MPLS_BFD_CV_ENA  VTSS_BIT(0)


/** 
 * \brief Configuration which OAM PDU are to be included in selected Cnt
 *
 * \details
 * The OAM frames handeled by the VOE can be counted separately in RX and
 * TX direction.
 * In each direction there are two counters:
 * 
 * 1) Default OAM counter
 * For debug purposes, all valid PDUs in RX and TX direction are counted in
 * one of two counters. This register determines which are counted as
 * either :
 * 
 * 1) NON Selected OAM counter:
 *  - VOE_STAT_MPLS:RX_CNT_SEL_OAM_MPLS
 *  - VOE_STAT_MPLS:TX_CNT_SEL_OAM_MPLS
 * 
 * 2) Selected OAM counter:
 * This counter counts all the PDU types selected for counting using the
 * OAM_CNT_OAM_CTRL register:
 * 
 *  - VOE_STAT_MPLS:RX_CNT_NON_SEL_OAM_MPLS
 *  - VOE_STAT_MPLS:TX_CNT_NON_SEL_OAM_MPLS
 * 
 * Any OAM PDU is counted in exactly one of the above registers.
 * 
 * I.e. as default all OAM PDUs are not selected, and they are all counted
 * in the default OAM counters : RX / TX_NON_SEL_OAM_MPLS.
 * 
 * Using this register (OAM_CNT_SEL_MPLS), PDUs can be moved to the
 * selected coutners:
 * RX / TX _SEL_OAM_MPLS.
 * 
 * The selection of OAM PDUs for the selected counter is done commonly for
 * the TX and RX direction.
 *
 * Register: \a VOP_MPLS:VOE_CONF_MPLS:OAM_CNT_SEL_MPLS
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_CONF_MPLS_OAM_CNT_SEL_MPLS(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x10000,gi,16,0,3)

/** 
 * \brief
 * Enable / disable that valid OAM PDUs with Generic Codepoint are counted
 * as selected OAM.
 * 
 * This bit field contains a separate bit for each of the possible 8
 * Generic opcodes.
 *
 * \details 
 * x0x: Count as other OAM
 * x1x: Count as selected OAM
 *
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_OAM_CNT_SEL_MPLS . GENERIC_CPT_CNT_SEL_MASK
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_OAM_CNT_SEL_MPLS_GENERIC_CPT_CNT_SEL_MASK(x)  VTSS_ENCODE_BITFIELD(x,3,8)
#define  VTSS_M_VOP_MPLS_VOE_CONF_MPLS_OAM_CNT_SEL_MPLS_GENERIC_CPT_CNT_SEL_MASK     VTSS_ENCODE_BITMASK(3,8)
#define  VTSS_X_VOP_MPLS_VOE_CONF_MPLS_OAM_CNT_SEL_MPLS_GENERIC_CPT_CNT_SEL_MASK(x)  VTSS_EXTRACT_BITFIELD(x,3,8)

/** 
 * \brief
 * MPLS-TP OAM PDUs not recognized as either on of the PDUs with special
 * configuration or as a generic PDU, will be classified as an UNKNOWN PDU.
 * 
 * This register configures whether UNKNOWN PDUs should be counted as
 * selected OAM.
 *
 * \details 
 * '0': Count as other OAM
 * '1': Count as selected OAM
 *
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_OAM_CNT_SEL_MPLS . UNK_CPT_CNT_SEL_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_OAM_CNT_SEL_MPLS_UNK_CPT_CNT_SEL_ENA  VTSS_BIT(2)

/** 
 * \brief
 * This register configures whether valid BFD CC PDUs are counted Selected
 * OAM or NON Selected OAM.
 *
 * \details 
 * '0': Count as other OAM
 * '1': Count as selected OAM
 *
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_OAM_CNT_SEL_MPLS . BFD_CC_CNT_SEL_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_OAM_CNT_SEL_MPLS_BFD_CC_CNT_SEL_ENA  VTSS_BIT(1)

/** 
 * \brief
 * This register configures whether valid BFD CV PDUs are counted Selected
 * OAM or NON Selected OAM.
 *
 * \details 
 * '0': Count as other OAM
 * '1': Count as selected OAM
 *
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_OAM_CNT_SEL_MPLS . BFD_CV_CNT_SEL_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_OAM_CNT_SEL_MPLS_BFD_CV_CNT_SEL_ENA  VTSS_BIT(0)


/** 
 * \brief Configuration of which OAM PDUs should be counted by LM counters.
 *
 * \details
 * Default behavior is that all MPLS-TP OAM PDUs processed by a VOE will
 * not be counted as part of the LM count. 
 * 
 * Using this register (OAM_CNT_DATA_MPLS) it is possible to configure the
 * OAM PDUs separately to be counted as part of the LM count.
 * 
 * Frames are counted separately in the RX and TX direction.
 * 
 * The data counters are located:
 * 
 * Service VOE:
 * ---------------------
 * Egress: REW::VOE_SRV_LM_CNT.SRV_LM_CNT_LSB.SRV_LM_CNT_LSB
 * Ingress: ANA::VOE_SRV_LM_CNT.SRV_LM_CNT_LSB.SRV_LM_CNT_LSB
 * 
 * Port VOE :
 * -------------------
 * Egress: REW::VOE_PORT_LM_CNT:PORT_LM_CNT_LSB.PORT_LM_CNT_LSB
 * Ingress: ANA::VOE_PORT_LM_CNT:PORT_LM_CNT_LSB.PORT_LM_CNT_LSB

 *
 * Register: \a VOP_MPLS:VOE_CONF_MPLS:OAM_CNT_DATA_MPLS
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_CONF_MPLS_OAM_CNT_DATA_MPLS(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x10000,gi,16,0,4)

/** 
 * \brief
 * Enable / disable that valid MPLS-TP OAM PDUs with Generic codepoint are
 * counted by the VOE LM counters.
 * 
 * This bit field contains a separate bit for each of the possible 8
 * Generic Codepoints.
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_OAM_CNT_DATA_MPLS . GENERIC_CPT_CNT_DATA_MASK
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_OAM_CNT_DATA_MPLS_GENERIC_CPT_CNT_DATA_MASK(x)  VTSS_ENCODE_BITFIELD(x,3,8)
#define  VTSS_M_VOP_MPLS_VOE_CONF_MPLS_OAM_CNT_DATA_MPLS_GENERIC_CPT_CNT_DATA_MASK     VTSS_ENCODE_BITMASK(3,8)
#define  VTSS_X_VOP_MPLS_VOE_CONF_MPLS_OAM_CNT_DATA_MPLS_GENERIC_CPT_CNT_DATA_MASK(x)  VTSS_EXTRACT_BITFIELD(x,3,8)

/** 
 * \brief
 * If a PDU is received with an Codepoint which does not match any Specific
 * Codepoint or a Generic OpCode, it will be processed as an UNKNOWN
 * Codepoint.
 * 
 * This bit field configures if OAM frames with UNKOWN Codepoint are
 * counted as data in the LM counters.
 *
 * \details 
 * '0': Do not count as data
 * '1': Count as data
 *
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_OAM_CNT_DATA_MPLS . UNK_CPT_CNT_DATA_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_OAM_CNT_DATA_MPLS_UNK_CPT_CNT_DATA_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Enable / disable count of valid BFD CC OAM PDU as part of the LM
 * counters.
 *
 * \details 
 * '0': Do not count as data
 * '1': Count as data
 *
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_OAM_CNT_DATA_MPLS . BFD_CC_CNT_DATA_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_OAM_CNT_DATA_MPLS_BFD_CC_CNT_DATA_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable / disable count of valid BFD CV OAM PDU as part of the LM
 * counters.
 *
 * \details 
 * '0': Do not count as data
 * '1': Count as data
 *
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_OAM_CNT_DATA_MPLS . BFD_CV_CNT_DATA_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_OAM_CNT_DATA_MPLS_BFD_CV_CNT_DATA_ENA  VTSS_BIT(0)


/** 
 * \brief Path MEP configuration
 *
 * \details
 * This register is used to assign a Path VOE to the current service VOE.
 * 
 * Assigning a Path VOE to a Service VOE to implies that all frames
 * received by this VOE, will also be counted by the Path VOE. The VOE
 * index of the PATH VOE is configured by the following bit field:
 * 
 *  * PATH_VOEID
 * 
 * The path VOE must be enabled by asserting the following field: 
 * 
 *  * PATH_VOE_ENA
 *
 * Register: \a VOP_MPLS:VOE_CONF_MPLS:PATH_VOE_MPLS
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_CONF_MPLS_PATH_VOE_MPLS(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x10000,gi,16,0,5)

/** 
 * \brief
 * Configures if a service VOE is part of a Path VOE.
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_PATH_VOE_MPLS . PATH_VOE_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_PATH_VOE_MPLS_PATH_VOE_ENA  VTSS_BIT(10)

/** 
 * \brief
 * Configures the Path VOE monitoring the PATH which carries traffic of the
 * Service EVC being monitored by the current VOE.
 * 
 * Must be enabled by : PATH_VOE_ENA = 1
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_PATH_VOE_MPLS . PATH_VOEID
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_PATH_VOE_MPLS_PATH_VOEID(x)  VTSS_ENCODE_BITFIELD(x,0,10)
#define  VTSS_M_VOP_MPLS_VOE_CONF_MPLS_PATH_VOE_MPLS_PATH_VOEID     VTSS_ENCODE_BITMASK(0,10)
#define  VTSS_X_VOP_MPLS_VOE_CONF_MPLS_PATH_VOE_MPLS_PATH_VOEID(x)  VTSS_EXTRACT_BITFIELD(x,0,10)


/** 
 * \brief MPLS-TP BFD configuration.
 *
 * \details
 * This register contains misc. bit fields used to figure the BFD session
 * monitored by the VOE.
 *
 * Register: \a VOP_MPLS:VOE_CONF_MPLS:BFD_CONFIG
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x10000,gi,16,0,6)

/** 
 * \brief
 * This bit field which CODEPOINT the VOE will use to detect BFD CC PDUs.
 * 
 * If an MPLS-TP PDU is received with the Codepoint which is not selected,
 * this PDU will be processed as an UNKNOWN Codepoint.
 *
 * \details 
 * The pending on configuration of this bit field, the VOE will expect the
 * following codepoint for BFD CC PDUs:
 * 
 * 0: 0x0007 (RFC_5885)
 * 1: 0x0022 (RFC_6428)
 *
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG . BFD_CC_RFC6428
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG_BFD_CC_RFC6428  VTSS_BIT(22)

/** 
 * \brief
 * Further the configuration of this bit field specifies the LOC timeout
 * counter to be used for LOC scan for this VOE.
 * 
 * Every time the LOC timeout counter indicated by this register causes a
 * scan, the VOE LOC counter is incremented.
 * 
 * The LOC counter is cleared upon reception of a valid BFD CC/CV PDU.
 * 
 * The LOC counter is located in the following bit field:
 * 
 *  - VOE_STAT_MPLS:BFD_STAT.BFD_MISS_CNT

 *
 * \details 
 * 0: Indicates that the LOC counter is not incremented.
 * 1: Indicates that the LOC counter is incremented by LOC timeout counter
 * 0
 * 2: Indicates that the LOC counter is incremented by LOC timeout counter
 * 1
 * 3: Indicates that the LOC counter is incremented by LOC timeout counter
 * 2
 * 4: Indicates that the LOC counter is incremented by LOC timeout counter
 * 3
 * 5: Indicates that the LOC counter is incremented by LOC timeout counter
 * 4
 * 6: Indicates that the LOC counter is incremented by LOC timeout counter
 * 5
 * 7: Indicates that the LOC counter is incremented by LOC timeout counter
 * 6
 *
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG . BFD_SCAN_PERIOD
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG_BFD_SCAN_PERIOD(x)  VTSS_ENCODE_BITFIELD(x,19,3)
#define  VTSS_M_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG_BFD_SCAN_PERIOD     VTSS_ENCODE_BITMASK(19,3)
#define  VTSS_X_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG_BFD_SCAN_PERIOD(x)  VTSS_EXTRACT_BITFIELD(x,19,3)

/** 
 * \brief
 * If enabled the VOE will update the following registers based on the
 * information received in valid BFD CC PDUs:
 * 
 * Coordinated / NEIS:
 *  - VOE_STAT_MPLS:BFD_SRC_INFO.BFD_REMOTE_STATE_SRC
 *  - VOE_STAT_MPLS:BFD_SRC_INFO.BFD_REMOTE_DIAG_SRC
 *  - VOE_STAT_MPLS:BFD_SRC_INFO.BFD_REMOTE_DM_SRC
 * 
 * FEIS session:
 *  - VOE_STAT_MPLS:BFD_SINK_INFO.BFD_REMOTE_STATE_SINK
 *  - VOE_STAT_MPLS:BFD_SINK_INFO.BFD_REMOTE_DIAG_SINK
 *  - VOE_STAT_MPLS:BFD_SINK_INFO.BFD_REMOTE_DM_SINK
 * 
 * The session to which a BFD PDU belongs is determined as part of the BFD
 * RX verification, based on comparing the BFD PDU Your Discriminator to
 * the values configured in the following bit fields:
 * 
 *  - VOE_CONF_MPLS:BFD_CONFIG.BFD_COORDINATED_MODE_ENA
 * 
 * Coordinated / NEIS session:
 *  - VOE_CONF_MPLS:BFD_LOCAL_DISCR_SRC.BFD_LOCAL_DISCR_SRC
 * 
 * FEIS session:
 *  - VOE_CONF_MPLS:BFD_LOCAL_DISCR_SINK.BFD_LOCAL_DISCR_SINK
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG . BFD_RX_SAMPLE_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG_BFD_RX_SAMPLE_ENA  VTSS_BIT(18)

/** 
 * \brief
 * If enabled, the VOE will update the following fields when transmitting
 * BFD CC / CV PDU's
 *  * BFD.STATE
 *  * BFD.DIAG
 *  * BFD.DM
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG . BFD_TX_UPDATE_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG_BFD_TX_UPDATE_ENA  VTSS_BIT(17)

/** 
 * \brief
 * This bit field configures whether the BFD session is running in :
 * 
 * 0: Independent BFD Mode
 * 1: Coordinated BFD Mode
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG . BFD_COORDINATED_MODE_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG_BFD_COORDINATED_MODE_ENA  VTSS_BIT(16)

/** 
 * \brief
 * If this field is asserted all incoming BFD CC PDUs will be verified in
 * HW.
 * If disabled, the BFD_CC PDUs will not be validated.
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG . BFD_RX_VERIFY_CC_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG_BFD_RX_VERIFY_CC_ENA  VTSS_BIT(15)

/** 
 * \brief
 * If this field is asserted all incoming BFD CV PDUs will be verified in
 * HW.
 * If disabled, the BFD_CC PDUs will not be validated.
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG . BFD_RX_VERIFY_CV_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG_BFD_RX_VERIFY_CV_ENA  VTSS_BIT(14)

/** 
 * \brief
 * If not enabled, the VERSION test will not be done as part of the BFD RX
 * verification.
 * 
 * This testing will depend on the setting of the following bit fields:
 * 
 *  - BFD_CC_AUTH_ENA
 *  - BFD_CV_AUTH_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG . BFD_RX_VERIFY_VERSION_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG_BFD_RX_VERIFY_VERSION_ENA  VTSS_BIT(13)

/** 
 * \brief
 * If not enabled, the MIN_LEN test will not be done as part of the BFD RX
 * verification.
 * 
 * This testing will depend on the setting of the following bit fields:
 * 
 *  - BFD_CC_AUTH_ENA
 *  - BFD_CV_AUTH_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG . BFD_RX_VERIFY_MIN_LEN_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG_BFD_RX_VERIFY_MIN_LEN_ENA  VTSS_BIT(12)

/** 
 * \brief
 * When disabled, the Your/My Discriminator fields of incoming BFD CC/CV
 * PDUs are not verified.
 * 
 * The BFD PDU is assumed to  belong to the Coordinated / NEIS session.
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG . BFD_RX_VERIFY_DISCR_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG_BFD_RX_VERIFY_DISCR_ENA  VTSS_BIT(11)

/** 
 * \brief
 * When enabled, the RX verification will verify:
 * 
 * * Detect Mult is non ZERO
 * * Auth Bit matches configured
 * * Demand bit is zero
 * * F and P bits are not set in the same frame.
 * 
 * This testing will depend on the setting of the following bit fields:
 * 
 *  - BFD_CC_AUTH_ENA
 *  - BFD_CV_AUTH_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG . BFD_RX_VERIFY_FLAGS_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG_BFD_RX_VERIFY_FLAGS_ENA  VTSS_BIT(10)

/** 
 * \brief
 * The configuration of this bit field indicates if the incoming BFD CC
 * PDUs are expected to have the AUTH bit set.
 * 
 * The value of the AUTH bit in the incoming BFD PDUs is verified as part
 * of the RX verification  AUTH_MISMATCH test.
 * 
 * If this test fails, the following sticky bit is asserted:
 * 
 *  - VOE_STAT_MPLS:BFD_RX_STICKY.AUTH_MISMATCH_STICKY
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG . BFD_CC_AUTH_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG_BFD_CC_AUTH_ENA  VTSS_BIT(9)

/** 
 * \brief
 * The configuration of this bit field indicates if the incoming BFD CV
 * PDUs are expected to have the AUTH bit set.
 * 
 * The value of the AUTH bit in the incoming BFD PDUs is verified as part
 * of the RX verification  AUTH_MISMATCH test.
 * 
 * If this test fails, the following sticky bit is asserted:
 * 
 *  - VOE_STAT_MPLS:BFD_RX_STICKY.AUTH_MISMATCH_STICKY
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG . BFD_CV_AUTH_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG_BFD_CV_AUTH_ENA  VTSS_BIT(8)

/** 
 * \brief
 * As part of the RX verification of BFD PDUs, the BFD lengh field is
 * compared to the value configured in this bitfield as part of the MAX_LEN
 * test.
 * 
 * If the MAX_LEN test fails, the frame is discarded at the following
 * sticky bit is asserted.
 * 
 *  - VOE_STAT_MPLS:BFD_RX_STICKY.MAX_LEN_ERR_STICKY
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG . BFD_MAX_LEN
 */
#define  VTSS_F_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG_BFD_MAX_LEN(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG_BFD_MAX_LEN     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VOP_MPLS_VOE_CONF_MPLS_BFD_CONFIG_BFD_MAX_LEN(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief BFD Local Discriminator (BFD_SRC)
 *
 * \details
 * The Discriminator of the Local BFD Source.
 *
 * Register: \a VOP_MPLS:VOE_CONF_MPLS:BFD_LOCAL_DISCR_SRC
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_CONF_MPLS_BFD_LOCAL_DISCR_SRC(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x10000,gi,16,0,7)


/** 
 * \brief BFD Remote Discriminator (BFD_SRC)
 *
 * \details
 * The Discriminator of the remote BFD entity communicating with the Local
 * BFD Source
 *
 * Register: \a VOP_MPLS:VOE_CONF_MPLS:BFD_REMOTE_DISCR_SRC
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_CONF_MPLS_BFD_REMOTE_DISCR_SRC(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x10000,gi,16,0,8)


/** 
 * \brief BFD Local Discriminator (BFD_SINK)
 *
 * \details
 * The Discriminator of the Local BFD Sink.
 *
 * Register: \a VOP_MPLS:VOE_CONF_MPLS:BFD_LOCAL_DISCR_SINK
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_CONF_MPLS_BFD_LOCAL_DISCR_SINK(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x10000,gi,16,0,9)


/** 
 * \brief BFD Remote Discriminator (BFD_SINK)
 *
 * \details
 * The Discriminator of the remote BFD entity communicating with the Local
 * BFD Sink.
 *
 * Register: \a VOP_MPLS:VOE_CONF_MPLS:BFD_REMOTE_DISCR_SINK
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_CONF_MPLS_BFD_REMOTE_DISCR_SINK(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x10000,gi,16,0,10)

/**
 * Register Group: \a VOP_MPLS:VOE_STAT_MPLS
 *
 * Per VOE statistics and counters (MPLS-TP OAM)
 */


/** 
 * \brief Count selected OAM PDU received by VOE.
 *
 * \details
 * All PDU types can be configured as either selected or non-selected PDUs.
 * 
 * This register counts the number of valid OAM PDUs configured as selected
 * PDU type, received by the VOE.
 * 
 * Selected OAM PDUs are configured in : VOE:OAM_CNT_OAM_CTRL.*
 *
 * Register: \a VOP_MPLS:VOE_STAT_MPLS:RX_CNT_SEL_OAM_MPLS
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_STAT_MPLS_RX_CNT_SEL_OAM_MPLS(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x0,gi,32,0,0)


/** 
 * \brief Count non-selected OAM PDU received by VOE.
 *
 * \details
 * All PDU types can be configured as either selected or non-selected PDUs.
 * 
 * This register counts the number of valid OAM PDUs configured as
 * non-selected PDU type, received by the VOE.
 * 
 * Selected OAM PDUs are configured in : VOE:OAM_CNT_OAM_CTRL.*
 *
 * Register: \a VOP_MPLS:VOE_STAT_MPLS:RX_CNT_NON_SEL_OAM_MPLS
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_STAT_MPLS_RX_CNT_NON_SEL_OAM_MPLS(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x0,gi,32,0,1)


/** 
 * \brief Count selected OAM PDUs transmitted by VOE.
 *
 * \details
 * All PDU types can be configured as either selected or non-selected PDUs.
 * 
 * This register counts the number of valid OAM PDUs configured as selected
 * PDU type, transmitted by the VOE.
 * 
 * Selected OAM PDUs are configured in : VOE:OAM_CNT_OAM_CTRL.*
 *
 * Register: \a VOP_MPLS:VOE_STAT_MPLS:TX_CNT_SEL_OAM_MPLS
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_STAT_MPLS_TX_CNT_SEL_OAM_MPLS(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x0,gi,32,0,2)


/** 
 * \brief Count NON-selected OAM PDUs transmitted by VOE.
 *
 * \details
 * All PDU types can be configured as either selected or non-selected PDUs.
 * 
 * This register counts the number of valid OAM PDUs configured as
 * non-selected PDU type, transmitted by the VOE.
 * 
 * Selected OAM PDUs are configured in : VOE:OAM_CNT_OAM_CTRL.*
 *
 * Register: \a VOP_MPLS:VOE_STAT_MPLS:TX_CNT_NON_SEL_OAM_MPLS
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_STAT_MPLS_TX_CNT_NON_SEL_OAM_MPLS(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x0,gi,32,0,3)


/** 
 * \brief BFD info related to the Local BFD Source session (BFD_SRC)
 *
 * \details
 * Misc. configuration related to the Local BFD Source (BFD_SRC)
 *
 * Register: \a VOP_MPLS:VOE_STAT_MPLS:BFD_SRC_INFO
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_SRC_INFO(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x0,gi,32,0,4)

/** 
 * \brief
 * BFD State of the Local BFD_SRC.
 * 
 * This register is configured by SW - never updated by VOE.
 * 
 * If configured to do so, the VOE will write this value into all valid TX
 * BFD CC/CV PDUs STATE field transmitted by the BFD_SRC.
 * 
 * The VOE will update the TX PDUs based on the following configuration:
 * 
 *  - VOE_CONF_MPLS:BFD_CONFIG.BFD_TX_UPDATE_ENA
 *
 * \details 
 * 00 - AdminDown
 * 01 - Down
 * 10 - Init
 * 11 - Up
 *
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_SRC_INFO . BFD_LOCAL_STATE_SRC
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_BFD_SRC_INFO_BFD_LOCAL_STATE_SRC(x)  VTSS_ENCODE_BITFIELD(x,28,2)
#define  VTSS_M_VOP_MPLS_VOE_STAT_MPLS_BFD_SRC_INFO_BFD_LOCAL_STATE_SRC     VTSS_ENCODE_BITMASK(28,2)
#define  VTSS_X_VOP_MPLS_VOE_STAT_MPLS_BFD_SRC_INFO_BFD_LOCAL_STATE_SRC(x)  VTSS_EXTRACT_BITFIELD(x,28,2)

/** 
 * \brief
 * BFD State of the remote BFD entity communicating with the Local BFD_SRC.
 * 
 * If configured to do so, the VOE HW will update this bit field with the
 * value of the last valid BFD CC PDU received by the BFD_SRC.
 * 
 * Updating this bit field is enabled by the following configuration:
 * 
 *  - VOE_CONF_MPLS:BFD_CONFIG.BFD_RX_SAMPLE_ENA
 *
 * \details 
 * 00 - AdminDown
 * 01 - Down
 * 10 - Init
 * 11 - Up
 *
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_SRC_INFO . BFD_REMOTE_STATE_SRC
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_BFD_SRC_INFO_BFD_REMOTE_STATE_SRC(x)  VTSS_ENCODE_BITFIELD(x,26,2)
#define  VTSS_M_VOP_MPLS_VOE_STAT_MPLS_BFD_SRC_INFO_BFD_REMOTE_STATE_SRC     VTSS_ENCODE_BITMASK(26,2)
#define  VTSS_X_VOP_MPLS_VOE_STAT_MPLS_BFD_SRC_INFO_BFD_REMOTE_STATE_SRC(x)  VTSS_EXTRACT_BITFIELD(x,26,2)

/** 
 * \brief
 * BFD diagnostic code of the Local BFD_SRC.
 * 
 * This register is configured by SW - never updated by VOE.
 * 
 * If configured to do so, the VOE will write this value into all valid TX
 * BFD CC/CV PDUs DIAG field transmitted by the BFD_SRC.
 * 
 * The VOE will update the TX PDUs based on the following configuration:
 * 
 *  - VOE_CONF_MPLS:BFD_CONFIG.BFD_TX_UPDATE_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_SRC_INFO . BFD_LOCAL_DIAG_SRC
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_BFD_SRC_INFO_BFD_LOCAL_DIAG_SRC(x)  VTSS_ENCODE_BITFIELD(x,21,5)
#define  VTSS_M_VOP_MPLS_VOE_STAT_MPLS_BFD_SRC_INFO_BFD_LOCAL_DIAG_SRC     VTSS_ENCODE_BITMASK(21,5)
#define  VTSS_X_VOP_MPLS_VOE_STAT_MPLS_BFD_SRC_INFO_BFD_LOCAL_DIAG_SRC(x)  VTSS_EXTRACT_BITFIELD(x,21,5)

/** 
 * \brief
 * BFD diagnostic code of the remote BFD entity communicating with the
 * Local BFD_SRC.
 * 
 * If configured to do so, the VOE HW will update this bit field with the
 * DIAG CODE value of the last valid BFD CC PDU received by the BFD_SRC.
 * 
 * Updating this bit field is enabled by the following configuration:
 * 
 *  - VOE_CONF_MPLS:BFD_CONFIG.BFD_RX_SAMPLE_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_SRC_INFO . BFD_REMOTE_DIAG_SRC
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_BFD_SRC_INFO_BFD_REMOTE_DIAG_SRC(x)  VTSS_ENCODE_BITFIELD(x,16,5)
#define  VTSS_M_VOP_MPLS_VOE_STAT_MPLS_BFD_SRC_INFO_BFD_REMOTE_DIAG_SRC     VTSS_ENCODE_BITMASK(16,5)
#define  VTSS_X_VOP_MPLS_VOE_STAT_MPLS_BFD_SRC_INFO_BFD_REMOTE_DIAG_SRC(x)  VTSS_EXTRACT_BITFIELD(x,16,5)

/** 
 * \brief
 * BFD Detect Mult of the Local BFD_SRC.
 * 
 * This register is configured by SW - never updated by VOE.
 * 
 * If configured to do so, the VOE will write this value into all valid TX
 * BFD CC/CV PDUs DETECT MULT field transmitted by the BFD_SRC.
 * 
 * The VOE will update the TX PDUs based on the following configuration:
 * 
 *  - VOE_CONF_MPLS:BFD_CONFIG.BFD_TX_UPDATE_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_SRC_INFO . BFD_LOCAL_DM_SRC
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_BFD_SRC_INFO_BFD_LOCAL_DM_SRC(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_VOP_MPLS_VOE_STAT_MPLS_BFD_SRC_INFO_BFD_LOCAL_DM_SRC     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_VOP_MPLS_VOE_STAT_MPLS_BFD_SRC_INFO_BFD_LOCAL_DM_SRC(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * BFD Detect Mult of the remote BFD entity communicating with the Local
 * BFD_SRC.
 * 
 * If configured to do so, the VOE HW will update this bit field with the
 * Detect Mult value of the last valid BFD CC PDU received by the BFD_SRC.
 * 
 * Updating this bit field is enabled by the following configuration:
 * 
 *  - VOE_CONF_MPLS:BFD_CONFIG.BFD_RX_SAMPLE_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_SRC_INFO . BFD_REMOTE_DM_SRC
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_BFD_SRC_INFO_BFD_REMOTE_DM_SRC(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VOP_MPLS_VOE_STAT_MPLS_BFD_SRC_INFO_BFD_REMOTE_DM_SRC     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VOP_MPLS_VOE_STAT_MPLS_BFD_SRC_INFO_BFD_REMOTE_DM_SRC(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief BFD info related to the Local BFD Sink session (BFD_SINK)
 *
 * \details
 * Misc. configuration related to the Local BFD Source (BFD_SINK)
 *
 * Register: \a VOP_MPLS:VOE_STAT_MPLS:BFD_SINK_INFO
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_SINK_INFO(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x0,gi,32,0,5)

/** 
 * \brief
 * BFD State of the Local BFD_SINK.
 * 
 * This register is configured by SW - never updated by VOE.
 * 
 * If configured to do so, the VOE will write this value into all valid TX
 * BFD CC/CV PDUs STATE field of all PDUs transmitted by the BFD_SINK.
 * 
 * The VOE will update the TX PDUs based on the following configuration:
 * 
 *  - VOE_CONF_MPLS:BFD_CONFIG.BFD_TX_UPDATE_ENA
 *
 * \details 
 * 00 - AdminDown
 * 01 - Down
 * 10 - Init
 * 11 - Up
 *
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_SINK_INFO . BFD_LOCAL_STATE_SINK
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_BFD_SINK_INFO_BFD_LOCAL_STATE_SINK(x)  VTSS_ENCODE_BITFIELD(x,28,2)
#define  VTSS_M_VOP_MPLS_VOE_STAT_MPLS_BFD_SINK_INFO_BFD_LOCAL_STATE_SINK     VTSS_ENCODE_BITMASK(28,2)
#define  VTSS_X_VOP_MPLS_VOE_STAT_MPLS_BFD_SINK_INFO_BFD_LOCAL_STATE_SINK(x)  VTSS_EXTRACT_BITFIELD(x,28,2)

/** 
 * \brief
 * BFD State of the remote BFD entity communicating with the Local BFD_SINK
 * 
 * If configured to do so, the VOE HW will update this bit field with the
 * value of the last valid BFD CC PDU received by the BFD_SINK.
 * 
 * Updating this bit field is enabled by the following configuration:
 * 
 *  - VOE_CONF_MPLS:BFD_CONFIG.BFD_RX_SAMPLE_ENA
 *
 * \details 
 * 00 - AdminDown
 * 01 - Down
 * 10 - Init
 * 11 - Up
 *
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_SINK_INFO . BFD_REMOTE_STATE_SINK
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_BFD_SINK_INFO_BFD_REMOTE_STATE_SINK(x)  VTSS_ENCODE_BITFIELD(x,26,2)
#define  VTSS_M_VOP_MPLS_VOE_STAT_MPLS_BFD_SINK_INFO_BFD_REMOTE_STATE_SINK     VTSS_ENCODE_BITMASK(26,2)
#define  VTSS_X_VOP_MPLS_VOE_STAT_MPLS_BFD_SINK_INFO_BFD_REMOTE_STATE_SINK(x)  VTSS_EXTRACT_BITFIELD(x,26,2)

/** 
 * \brief
 * BFD diagnostic code of the Local BFD_SINK.
 * 
 * This register is configured by SW - never updated by VOE.
 * 
 * If configured to do so, the VOE will write this value into all valid TX
 * BFD CC/CV PDUs DIAG field transmitted by the BFD_SINK.
 * 
 * The VOE will update the TX PDUs based on the following configuration:
 * 
 *  - VOE_CONF_MPLS:BFD_CONFIG.BFD_TX_UPDATE_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_SINK_INFO . BFD_LOCAL_DIAG_SINK
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_BFD_SINK_INFO_BFD_LOCAL_DIAG_SINK(x)  VTSS_ENCODE_BITFIELD(x,21,5)
#define  VTSS_M_VOP_MPLS_VOE_STAT_MPLS_BFD_SINK_INFO_BFD_LOCAL_DIAG_SINK     VTSS_ENCODE_BITMASK(21,5)
#define  VTSS_X_VOP_MPLS_VOE_STAT_MPLS_BFD_SINK_INFO_BFD_LOCAL_DIAG_SINK(x)  VTSS_EXTRACT_BITFIELD(x,21,5)

/** 
 * \brief
 * BFD diagnostic code of the remote BFD entity communicating with the
 * Local BFD_SINK.
 * 
 * If configured to do so, the VOE HW will update this bit field with the
 * DIAG CODE value of the last valid BFD CC PDU received by the BFD_SINK.
 * 
 * Updating this bit field is enabled by the following configuration:
 * 
 *  - VOE_CONF_MPLS:BFD_CONFIG.BFD_RX_SAMPLE_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_SINK_INFO . BFD_REMOTE_DIAG_SINK
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_BFD_SINK_INFO_BFD_REMOTE_DIAG_SINK(x)  VTSS_ENCODE_BITFIELD(x,16,5)
#define  VTSS_M_VOP_MPLS_VOE_STAT_MPLS_BFD_SINK_INFO_BFD_REMOTE_DIAG_SINK     VTSS_ENCODE_BITMASK(16,5)
#define  VTSS_X_VOP_MPLS_VOE_STAT_MPLS_BFD_SINK_INFO_BFD_REMOTE_DIAG_SINK(x)  VTSS_EXTRACT_BITFIELD(x,16,5)

/** 
 * \brief
 * BFD Detect Mult of the Local BFD_SINK.
 * 
 * This register is configured by SW - never updated by VOE.
 * 
 * If configured to do so, the VOE will write this value into all valid TX
 * BFD CC/CV PDUs DETECT MULT field transmitted by the BFD_SINK.
 * 
 * The VOE will update the TX PDUs based on the following configuration:
 * 
 *  - VOE_CONF_MPLS:BFD_CONFIG.BFD_TX_UPDATE_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_SINK_INFO . BFD_LOCAL_DM_SINK
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_BFD_SINK_INFO_BFD_LOCAL_DM_SINK(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_VOP_MPLS_VOE_STAT_MPLS_BFD_SINK_INFO_BFD_LOCAL_DM_SINK     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_VOP_MPLS_VOE_STAT_MPLS_BFD_SINK_INFO_BFD_LOCAL_DM_SINK(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * BFD Detect Mult of the remote BFD entity communicating with the Local
 * BFD_SINK.
 * 
 * If configured to do so, the VOE HW will update this bit field with the
 * Detect Mult value of the last valid BFD CC PDU received by the BFD_SINK.
 * 
 * Updating this bit field is enabled by the following configuration:
 * 
 *  - VOE_CONF_MPLS:BFD_CONFIG.BFD_RX_SAMPLE_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_SINK_INFO . BFD_REMOTE_DM_SINK
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_BFD_SINK_INFO_BFD_REMOTE_DM_SINK(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VOP_MPLS_VOE_STAT_MPLS_BFD_SINK_INFO_BFD_REMOTE_DM_SINK     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VOP_MPLS_VOE_STAT_MPLS_BFD_SINK_INFO_BFD_REMOTE_DM_SINK(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief BFD statistics
 *
 * \details
 * Register contains misc. BFD related statistics.
 *
 * Register: \a VOP_MPLS:VOE_STAT_MPLS:BFD_STAT
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_STAT(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x0,gi,32,0,6)

/** 
 * \brief
 * BFD Loss of Continuity (LOC) Counter.
 * 
 * This counter is incremented by the LOC SCAN timer programmed in the
 * following bit field:
 * 
 *  - VOE_CONF_MPLS:BFD_CONFIG.BFD_SCAN_PERIOD
 * 
 * The LOC counter is cleared every time a valid BFD CC/CV PDU is received.
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_STAT . BFD_MISS_CNT
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_BFD_STAT_BFD_MISS_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,9)
#define  VTSS_M_VOP_MPLS_VOE_STAT_MPLS_BFD_STAT_BFD_MISS_CNT     VTSS_ENCODE_BITMASK(0,9)
#define  VTSS_X_VOP_MPLS_VOE_STAT_MPLS_BFD_STAT_BFD_MISS_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,9)


/** 
 * \details
 * Register: \a VOP_MPLS:VOE_STAT_MPLS:BFD_RX_LAST
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_RX_LAST(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x0,gi,32,0,7)

/** 
 * \brief
 * Contains the result of the current result of the LOC detect test.
 * 
 * When this value is changed (by LOC timer or RX of valid BFD CC/CV) a
 * sticky bit is asserted :
 * 
 *  - VOE_STAT_MPLS:INTR_STICKY_MPLS.LOC_CHANGE_STICKY
 * 
 * An interrupt may optionally be generated:
 * 
 *  - VOE_STAT_MPLS:INTR_ENA_MPLS.BFD_LOC_CHANGE_INT_ENA

 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_RX_LAST . BFD_LOC_DEFECT
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_BFD_RX_LAST_BFD_LOC_DEFECT  VTSS_BIT(0)


/** 
 * \details
 * The sticky bits in this register will cause a VOE interrupt if the
 * interrupt is enabled by the corresponding enable bit field in :
 * 
 *  - VOE_STAT_MPLS:INTR_ENA_MPLS.*
 * 
 * If an interrupt is enabled, the interrupt is asserted when the
 * corresponding sticky bit is asserted.
 *
 * Register: \a VOP_MPLS:VOE_STAT_MPLS:INTR_STICKY_MPLS
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_STAT_MPLS_INTR_STICKY_MPLS(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x0,gi,32,0,8)

/** 
 * \brief
 * This sticky bit is asserted when the VOE Loss of Continuity (LOC) detect
 * state changes.
 * 
 * The following two events will cause the assertion of this sticky bit:
 * ------------------------------------------------------------------------
 * -----------------------------
 *  - The BFD_MISS_CNT is increased to 2 x the session Detect Mult (LOC
 * detection)
 *  - A valid BFD CC/CV PDU is received by a VOE which is in LOC state.

 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_INTR_STICKY_MPLS . BFD_LOC_CHANGE_STICKY
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_INTR_STICKY_MPLS_BFD_LOC_CHANGE_STICKY  VTSS_BIT(10)

/** 
 * \brief
 * When the BFD SOURCE receives a valid BFD CC PDU, this sticky bit is
 * asserted if the 'Detect Mult' value in the RX BFD CC PDU is different
 * from the previous value received.
 * 
 * The previous value is stored in :
 *  - VOE_STAT_MPLS::BFD_SRC_INFO.BFD_REMOTE_DM_SRC
 * 
 * This event will generate an interrupt if so enabled in :
 *  - VOE_STAT_MPLS::INTR_ENA_MPLS.BFD_RX_DM_CHANGE_SRC_INT_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_INTR_STICKY_MPLS . BFD_RX_DM_CHANGE_SRC_STICKY
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_INTR_STICKY_MPLS_BFD_RX_DM_CHANGE_SRC_STICKY  VTSS_BIT(9)

/** 
 * \brief
 * When the BFD SOURCE receives a valid BFD CC PDU, this sticky bit is
 * asserted if the 'State' value in the RX BFD CC PDU is different from the
 * previous value received.
 * 
 * The previous value is stored in :
 *  - VOE_STAT_MPLS::BFD_SRC_INFO.BFD_REMOTE_STATE_SRC
 * 
 * This event will generate an interrupt if so enabled in :
 *  - VOE_STAT_MPLS::INTR_ENA_MPLS.BFD_RX_STATE_CHANGE_SRC_INT_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_INTR_STICKY_MPLS . BFD_RX_STATE_CHANGE_SRC_STICKY
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_INTR_STICKY_MPLS_BFD_RX_STATE_CHANGE_SRC_STICKY  VTSS_BIT(8)

/** 
 * \brief
 * When the BFD SOURCE receives a valid BFD CC PDU, this sticky bit is
 * asserted if the 'Diag' value in the RX BFD CC PDU is different from the
 * previous value received.
 * 
 * The previous value is stored in :
 *  - VOE_STAT_MPLS::BFD_SRC_INFO.BFD_REMOTE_DIAG_SRC
 * 
 * This event will generate an interrupt if so enabled in :
 *  - VOE_STAT_MPLS::INTR_ENA_MPLS.BFD_RX_DIAG_CHANGE_SRC_INT_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_INTR_STICKY_MPLS . BFD_RX_DIAG_CHANGE_SRC_STICKY
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_INTR_STICKY_MPLS_BFD_RX_DIAG_CHANGE_SRC_STICKY  VTSS_BIT(7)

/** 
 * \brief
 * When the BFD SOURCE receives a valid BFD CC PDU, this sticky bit is
 * asserted if the Poll Flag is asserted in the RX BFD CC PDU.
 * 
 * This event will generate an interrupt if so enabled in :
 *  - VOE_STAT_MPLS::INTR_ENA_MPLS.BFD_RX_P_SET_SRC_INT_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_INTR_STICKY_MPLS . BFD_RX_P_SET_SRC_STICKY
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_INTR_STICKY_MPLS_BFD_RX_P_SET_SRC_STICKY  VTSS_BIT(6)

/** 
 * \brief
 * When the BFD SOURCE receives a valid BFD CC PDU, this sticky bit is
 * asserted if the Final Flag is asserted in the RX BFD CC PDU.
 * 
 * This event will generate an interrupt if so enabled in :
 *  - VOE_STAT_MPLS::INTR_ENA_MPLS.BFD_RX_F_SET_SRC_INT_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_INTR_STICKY_MPLS . BFD_RX_F_SET_SRC_STICKY
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_INTR_STICKY_MPLS_BFD_RX_F_SET_SRC_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * When the BFD SINK receives a valid BFD CC PDU, this sticky bit is
 * asserted if the 'Detect Mult' value in the RX BFD CC PDU is different
 * from the previous value received.
 * 
 * The previous value is stored in :
 *  - VOE_STAT_MPLS::BFD_SRC_INFO.BFD_REMOTE_DM_SINK
 * 
 * This event will generate an interrupt if so enabled in :
 *  - VOE_STAT_MPLS::INTR_ENA_MPLS.BFD_RX_DM_CHANGE_SINK_INT_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_INTR_STICKY_MPLS . BFD_RX_DM_CHANGE_SINK_STICKY
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_INTR_STICKY_MPLS_BFD_RX_DM_CHANGE_SINK_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * When the BFD SINK receives a valid BFD CC PDU, this sticky bit is
 * asserted if the 'State' value in the RX BFD CC PDU is different from the
 * previous value received.
 * 
 * The previous value is stored in :
 *  - VOE_STAT_MPLS::BFD_SRC_INFO.BFD_REMOTE_STATE_SINK
 * 
 * This event will generate an interrupt if so enabled in :
 *  - VOE_STAT_MPLS::INTR_ENA_MPLS.BFD_RX_STATE_CHANGE_SINK_INT_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_INTR_STICKY_MPLS . BFD_RX_STATE_CHANGE_SINK_STICKY
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_INTR_STICKY_MPLS_BFD_RX_STATE_CHANGE_SINK_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * When the BFD SINK receives a valid BFD CC PDU, this sticky bit is
 * asserted if the 'Diag' value in the RX BFD CC PDU is different from the
 * previous value received.
 * 
 * The previous value is stored in :
 *  - VOE_STAT_MPLS::BFD_SRC_INFO.BFD_REMOTE_DIAG_SINK
 * 
 * This event will generate an interrupt if so enabled in :
 *  - VOE_STAT_MPLS::INTR_ENA_MPLS.BFD_RX_DIAG_CHANGE_SINK_INT_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_INTR_STICKY_MPLS . BFD_RX_DIAG_CHANGE_SINK_STICKY
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_INTR_STICKY_MPLS_BFD_RX_DIAG_CHANGE_SINK_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * When the BFD SINK receives a valid BFD CC PDU, this sticky bit is
 * asserted if the Poll Flag is asserted in the RX BFD CC PDU.
 * 
 * This event will generate an interrupt if so enabled in :
 *  - VOE_STAT_MPLS::INTR_ENA_MPLS.BFD_RX_P_SET_SINK_INT_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_INTR_STICKY_MPLS . BFD_RX_P_SET_SINK_STICKY
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_INTR_STICKY_MPLS_BFD_RX_P_SET_SINK_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * When the BFD SINK receives a valid BFD CC PDU, this sticky bit is
 * asserted if the Final Flag is asserted in the RX BFD CC PDU.
 * 
 * This event will generate an interrupt if so enabled in :
 *  - VOE_STAT_MPLS::INTR_ENA_MPLS.BFD_RX_F_SET_SINK_INT_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_INTR_STICKY_MPLS . BFD_RX_F_SET_SINK_STICKY
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_INTR_STICKY_MPLS_BFD_RX_F_SET_SINK_STICKY  VTSS_BIT(0)


/** 
 * \details
 * The bitfields in this register are interrupt enable for the
 * corresponding sticky bits in the following register:
 * 
 *  - VOE_STAT_MPLS:INTR_STICKY_MPLS.*
 * 
 * If an interrupt is enabled, the interrupt is asserted when the
 * corresponding sticky bit is asserted.
 *
 * Register: \a VOP_MPLS:VOE_STAT_MPLS:INTR_ENA_MPLS
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_STAT_MPLS_INTR_ENA_MPLS(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x0,gi,32,0,9)

/** 
 * \brief
 * When asserted, the following sticky bit will cause a VOE interrupt:
 * 
 *  - VOE_STAT_MPLS:INTR_STICKY_MPLS.BFD_LOC_CHANGE_STICKY
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_INTR_ENA_MPLS . BFD_LOC_CHANGE_INT_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_INTR_ENA_MPLS_BFD_LOC_CHANGE_INT_ENA  VTSS_BIT(10)

/** 
 * \brief
 * When asserted, the following sticky bit will cause a VOE interrupt:
 * 
 *  - VOE_STAT_MPLS:INTR_STICKY_MPLS.BFD_RX_DM_CHANGE_SRC_STICKY
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_INTR_ENA_MPLS . BFD_RX_DM_CHANGE_SRC_INT_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_INTR_ENA_MPLS_BFD_RX_DM_CHANGE_SRC_INT_ENA  VTSS_BIT(9)

/** 
 * \brief
 * When asserted, the following sticky bit will cause a VOE interrupt:
 * 
 *  - VOE_STAT_MPLS:INTR_STICKY_MPLS.BFD_RX_STATE_CHANGE_SRC_STICKY
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_INTR_ENA_MPLS . BFD_RX_STATE_CHANGE_SRC_INT_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_INTR_ENA_MPLS_BFD_RX_STATE_CHANGE_SRC_INT_ENA  VTSS_BIT(8)

/** 
 * \brief
 * When asserted, the following sticky bit will cause a VOE interrupt:
 * 
 *  - VOE_STAT_MPLS:INTR_STICKY_MPLS.BFD_RX_DIAG_CHANGE_SRC_STICKY
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_INTR_ENA_MPLS . BFD_RX_DIAG_CHANGE_SRC_INT_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_INTR_ENA_MPLS_BFD_RX_DIAG_CHANGE_SRC_INT_ENA  VTSS_BIT(7)

/** 
 * \brief
 * When asserted, the following sticky bit will cause a VOE interrupt:
 * 
 *  - VOE_STAT_MPLS:INTR_STICKY_MPLS.BFD_RX_P_SET_SRC_STICKY
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_INTR_ENA_MPLS . BFD_RX_P_SET_SRC_INT_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_INTR_ENA_MPLS_BFD_RX_P_SET_SRC_INT_ENA  VTSS_BIT(6)

/** 
 * \brief
 * When asserted, the following sticky bit will cause a VOE interrupt:
 * 
 *  - VOE_STAT_MPLS:INTR_STICKY_MPLS.BFD_RX_F_SET_SRC_STICKY
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_INTR_ENA_MPLS . BFD_RX_F_SET_SRC_INT_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_INTR_ENA_MPLS_BFD_RX_F_SET_SRC_INT_ENA  VTSS_BIT(5)

/** 
 * \brief
 * When asserted, the following sticky bit will cause a VOE interrupt:
 * 
 *  - VOE_STAT_MPLS:INTR_STICKY_MPLS.BFD_RX_DM_CHANGE_SINK_STICKY
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_INTR_ENA_MPLS . BFD_RX_DM_CHANGE_SINK_INT_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_INTR_ENA_MPLS_BFD_RX_DM_CHANGE_SINK_INT_ENA  VTSS_BIT(4)

/** 
 * \brief
 * When asserted, the following sticky bit will cause a VOE interrupt:
 * 
 *  - VOE_STAT_MPLS:INTR_STICKY_MPLS.BFD_RX_STATE_CHANGE_SINK_STICKY
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_INTR_ENA_MPLS . BFD_RX_STATE_CHANGE_SINK_INT_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_INTR_ENA_MPLS_BFD_RX_STATE_CHANGE_SINK_INT_ENA  VTSS_BIT(3)

/** 
 * \brief
 * When asserted, the following sticky bit will cause a VOE interrupt:
 * 
 *  - VOE_STAT_MPLS:INTR_STICKY_MPLS.BFD_RX_DIAG_CHANGE_SINK_STICKY
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_INTR_ENA_MPLS . BFD_RX_DIAG_CHANGE_SINK_INT_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_INTR_ENA_MPLS_BFD_RX_DIAG_CHANGE_SINK_INT_ENA  VTSS_BIT(2)

/** 
 * \brief
 * When asserted, the following sticky bit will cause a VOE interrupt:
 * 
 *  - VOE_STAT_MPLS:INTR_STICKY_MPLS.BFD_RX_P_SET_SINK_STICKY
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_INTR_ENA_MPLS . BFD_RX_P_SET_SINK_INT_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_INTR_ENA_MPLS_BFD_RX_P_SET_SINK_INT_ENA  VTSS_BIT(1)

/** 
 * \brief
 * When asserted, the following sticky bit will cause a VOE interrupt:
 * 
 *  - VOE_STAT_MPLS:INTR_STICKY_MPLS.BFD_RX_F_SET_SINK_STICKY
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_INTR_ENA_MPLS . BFD_RX_F_SET_SINK_INT_ENA
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_INTR_ENA_MPLS_BFD_RX_F_SET_SINK_INT_ENA  VTSS_BIT(0)


/** 
 * \brief Rx Sticky bits
 *
 * \details
 * Sticky bits are asserted when a valid MPLS-TP OAM PDU is received by the
 * VOE.
 * 
 * The Codepoint specific RX sticky bits:
 * 
 *  * "xxx"_RX_STICKY
 * 
 * will be asserted even when the PDU is not enabled
 * (VOE_CONF_MPLS.OAM_HW_CTRL_MPLS), to allow detecting PDU types which are
 * not expected.
 * 
 * For the remaining sticky bits the VOE will require the PDU type to be
 * enabled, before asserting the sticky bits.
 *
 * Register: \a VOP_MPLS:VOE_STAT_MPLS:CPT_RX_STICKY_MPLS
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_STAT_MPLS_CPT_RX_STICKY_MPLS(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x0,gi,32,0,10)

/** 
 * \brief
 * Asserted when VOE receives a valid OAM PDU with an Codepoint which is
 * configured as a GENERIC Codepoint in the following register:
 * 
 *  - OAM_MEP:COMMONMPLS_GENERIC_CODEPOINT.*
 * 
 * There is a separate bit to indicate the reception of each of the
 * configured GENERIC OpCodes.
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_CPT_RX_STICKY_MPLS . GENERIC_CPT_RX_STICKY_MASK
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_CPT_RX_STICKY_MPLS_GENERIC_CPT_RX_STICKY_MASK(x)  VTSS_ENCODE_BITFIELD(x,3,8)
#define  VTSS_M_VOP_MPLS_VOE_STAT_MPLS_CPT_RX_STICKY_MPLS_GENERIC_CPT_RX_STICKY_MASK     VTSS_ENCODE_BITMASK(3,8)
#define  VTSS_X_VOP_MPLS_VOE_STAT_MPLS_CPT_RX_STICKY_MPLS_GENERIC_CPT_RX_STICKY_MASK(x)  VTSS_EXTRACT_BITFIELD(x,3,8)

/** 
 * \brief
 * Asserted when VOE receives a valid MPLS-TP OAM PDU with an Codepoint
 * which is detected as UNKNOWN.
 * 
 * UNKNOWN Codepoint is used for OAM PDU types which do not have dedicated
 * HW support and which is not encoded as a GENERIC CODEPOINT.
 *
 * \details 
 * '0': No PDU received
 * '1': PDU with specified OPCODE received
 *
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_CPT_RX_STICKY_MPLS . UNK_CPT_RX_STICKY
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_CPT_RX_STICKY_MPLS_UNK_CPT_RX_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Asserted when VOE receives a valid BFD CC PDU.
 *
 * \details 
 * '0': No Valid BFD CC PDU received
 * '1': VALID BFD CC received
 *
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_CPT_RX_STICKY_MPLS . BFD_CC_RX_STICKY
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_CPT_RX_STICKY_MPLS_BFD_CC_RX_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Asserted when VOE receives a valid BFD CC PDU
 *
 * \details 
 * '0': No Valid BFD CC PDU received
 * '1': VALID BFD CC received
 *
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_CPT_RX_STICKY_MPLS . BFD_CV_RX_STICKY
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_CPT_RX_STICKY_MPLS_BFD_CV_RX_STICKY  VTSS_BIT(0)


/** 
 * \brief Extraction of MPLS PDUs
 *
 * \details
 * This register contains a number of bit fields which are used to
 * configure extraction of certains MPLS PDUs, which match a given
 * criteria.
 * 
 * Note that some of the registers will extract either 'NEXT FRAME only '
 * (Hit ME Once) or 'ALL FRAMES' matching a given criteria, based on the
 * configuration of the following bitfield:
 * 
 *  - EXTRACT_HIT_ME_ONCE
 *
 * Register: \a VOP_MPLS:VOE_STAT_MPLS:PDU_EXTRACT_MPLS
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_STAT_MPLS_PDU_EXTRACT_MPLS(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x0,gi,32,0,11)

/** 
 * \brief
 * Extract the next valid BFD CC PDU received by the VOE.
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_PDU_EXTRACT_MPLS . BFD_CC_RX_NEXT_GOOD_EXTR
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_PDU_EXTRACT_MPLS_BFD_CC_RX_NEXT_GOOD_EXTR  VTSS_BIT(7)

/** 
 * \brief
 * Extract the next valid BFD CV PDU received by the VOE.
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_PDU_EXTRACT_MPLS . BFD_CV_RX_NEXT_GOOD_EXTR
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_PDU_EXTRACT_MPLS_BFD_CV_RX_NEXT_GOOD_EXTR  VTSS_BIT(6)

/** 
 * \brief
 * THe configuration of this bit field determines whether the following
 * configuration bits cause extraction of the Next Frame Only (Hit Me Once)
 * or all the Frames matching a given criteria:
 * 
 *  - BFD_RX_PARAM_CHANGE_EXTR
 *  - BFD_RX_P_SET_EXTR
 *  - BFD_RX_F_SET_EXTR
 *  - BFD_RX_ERR_EXTR
 *  - BFD_TX_ERR_EXTR
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_PDU_EXTRACT_MPLS . EXTRACT_HIT_ME_ONCE
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_PDU_EXTRACT_MPLS_EXTRACT_HIT_ME_ONCE  VTSS_BIT(5)

/** 
 * \brief
 * Will extract the next valid RX BFD_PDU if one of the following
 * paramteres changed since the last BFD_PDU belonging to the same session
 * (Coordinated / NEIS / FEIS):
 * 
 *  * BFD.DM
 *  * BFD.DIAG
 *  * BFD.STATE
 * 
 * Extraction will be done Hit Me Once or all frames depending on the
 * following bit field :
 * EXTRACT_HIT_ME_ONCE
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_PDU_EXTRACT_MPLS . BFD_RX_PARAM_CHANGE_EXTR
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_PDU_EXTRACT_MPLS_BFD_RX_PARAM_CHANGE_EXTR  VTSS_BIT(4)

/** 
 * \brief
 * Asserting this bit will extract frames with the Poll flag set.
 * 
 * Extraction will be done Hit Me Once or all frames depending on the
 * following bit field :
 * EXTRACT_HIT_ME_ONCE
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_PDU_EXTRACT_MPLS . BFD_RX_P_SET_EXTR
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_PDU_EXTRACT_MPLS_BFD_RX_P_SET_EXTR  VTSS_BIT(3)

/** 
 * \brief
 * Asserting this bit will extract frames with the Final flag set.
 * 
 * Extraction will be done Hit Me Once or all frames depending on the
 * following bit field :
 * EXTRACT_HIT_ME_ONCE
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_PDU_EXTRACT_MPLS . BFD_RX_F_SET_EXTR
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_PDU_EXTRACT_MPLS_BFD_RX_F_SET_EXTR  VTSS_BIT(2)

/** 
 * \brief
 * Asserting this bit will extract BFD PDUs which fail the RX verification
 * test.
 * 
 * Extraction will be done Hit Me Once or all frames depending on the
 * following bit field :
 * EXTRACT_HIT_ME_ONCE
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_PDU_EXTRACT_MPLS . BFD_RX_ERR_EXTR
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_PDU_EXTRACT_MPLS_BFD_RX_ERR_EXTR  VTSS_BIT(1)

/** 
 * \brief
 * Asserting this bit will extract BFD PDUs which fail the TX verification
 * test.
 * 
 * Extraction will be done Hit Me Once or all frames depending on the
 * following bit field :
 * EXTRACT_HIT_ME_ONCE
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_PDU_EXTRACT_MPLS . BFD_TX_ERR_EXTR
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_PDU_EXTRACT_MPLS_BFD_TX_ERR_EXTR  VTSS_BIT(0)


/** 
 * \brief BFD RX verification sticky bits.
 *
 * \details
 * This register contains a sticky bit for each of the tests performed as
 * part of the BFD RX PDU validation.
 * 
 * Frames causing the sticky to be asserted will be discarded if not
 * extracted to the CPU.
 * 
 * The extraction of BFD PDUs is configured in the following register:
 * 
 *  - VOE_STAT_MPLS:PDU_EXTRACT_MPLS
 * 
 * Notice that the RX verification depends on the following register:
 * 
 *  - VOE_CONF_MPLS:BFD_CONFIG.BFD_RX_VERIFY_*
 * 
 * PDUs faling the RX verification can optionally be extracted configuring
 * the following bit field:
 * 
 *  - VOE_STAT_MPLS:PDU_EXTRACT_MPLS.BFD_RX_ERR_EXTR
 *
 * Register: \a VOP_MPLS:VOE_STAT_MPLS:BFD_RX_STICKY
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_RX_STICKY(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x0,gi,32,0,12)

/** 
 * \brief
 * Sticky bit is asserted when a frames was discarded because the
 * BFD_PDU.VERSION field does not match the value configured in the
 * following register:
 * 
 * 
 *  - OAM_MEP::COMMON:VERSION_CTRL_MPLS.BFD_VERSION

 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_RX_STICKY . VERSION_ERR_STICKY
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_BFD_RX_STICKY_VERSION_ERR_STICKY  VTSS_BIT(9)

/** 
 * \brief
 * Sticky bit asserted if an incoming BFD frame fails the MIN_LEN test.
 * 
 * MIN_LEN test will discard incoming BFD frames based on the value of the
 * AUTH bit and the LENGTH field in the BFD PDU:
 * 
 * AUTH = 0: Discard PDU if LENGTH < 24 bytes
 * AUTH = 1: Discard PDU if LENGTH < 26 bytes

 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_RX_STICKY . MIN_LEN_ERR_STICKY
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_BFD_RX_STICKY_MIN_LEN_ERR_STICKY  VTSS_BIT(8)

/** 
 * \brief
 * Sticky bit asserted if an incoming BFD frame fails the MAX_LEN test.
 * 
 * The MAX_LEN will discard incoming BFD PDUs if the LENGTH field in the
 * BFD PDU is larger than the configured MAX_LEN:
 * 
 *  - VOE_CONF:BFD_CFG.BFD_MAX_LEN
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_RX_STICKY . MAX_LEN_ERR_STICKY
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_BFD_RX_STICKY_MAX_LEN_ERR_STICKY  VTSS_BIT(7)

/** 
 * \brief
 * Sticky bit asserted if the incoming RX BFD PDU fails the DM_ZERO test:
 * 
 * Incoming RX BFD PDUs are discarded if the Detect Mult field is ZERO.
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_RX_STICKY . DM_ZERO_ERR_STICKY
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_BFD_RX_STICKY_DM_ZERO_ERR_STICKY  VTSS_BIT(6)

/** 
 * \brief
 * Sticky bit asserted if an incoming BFD PDU fails the M_BIT_SET test.
 * 
 * Incoming RX BFD PDUs are discarded if the M bit is set.
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_RX_STICKY . M_BIT_SET_ERR_STICKY
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_BFD_RX_STICKY_M_BIT_SET_ERR_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * Sticky bit asserted if an incoming RX BFD PDU is discarded because it
 * fails the MY_DISCR verification.
 * 
 * The MY_DISCR test will match the MY DISCRIMINATOR field in the RX BFD
 * PDU, based on which session the BFD PDU belongs to. 
 * 
 * Coordinated or NEIS session:
 *  - VOE_CONF_MPLS:BFD_REMOTE_DISCR_SRC.BFD_REMOTE_DISCR_SRC
 * 
 * FEIS session:
 *  - VOE_CONF_MPLS:BFD_REMOTE_DISCR_SINK.BFD_REMOTE_DISCR_SINK
 * 
 * The session is determined based on matching the BFD PDU Your
 * Discriminator against the following values:
 *  - VOE_CONF_MPLS:BFD_LOCAL_DISCR_SRC.BFD_LOCAL_DISCR_SRC
 *  - VOE_CONF_MPLS:BFD_LOCAL_DISCR_SINK.BFD_LOCAL_DISCR_SINK

 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_RX_STICKY . MY_DISCR_ERR_STICKY
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_BFD_RX_STICKY_MY_DISCR_ERR_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Sticky bit asserted if the incoming BFD PDU fails the YOUR_DISCR test.
 * 
 * The YOUR_DISCR test will match the Your Discriminator field of the
 * incoming BFD PDU against the values configured in the following
 * registers:
 * 
 * Coordinated or NEIS session:
 *  - VOE_CONF_MPLS:BFD_LOCAL_DISCR_SRC.BFD_LOCAL_DISCR_SRC
 * 
 * FEIS session:
 *  - VOE_CONF_MPLS:BFD_LOCAL_DISCR_SINK.BFD_LOCAL_DISCR_SINK
 * 
 * The match will depend on the configuration of the following bit field:
 * 
 *  - VOE_CONF_MPLS:BFD_CONFIG.BFD_COORDINATED_MODE_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_RX_STICKY . YOUR_DISCR_ERR_STICKY
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_BFD_RX_STICKY_YOUR_DISCR_ERR_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * Sticky bit asserted if the incoming BFD PDU fails the AUTH_MISMATCH
 * test.
 * 
 * The VOE is configured with the expected value of the incoming BFD PDUs:
 * 
 * BFD CC:
 *  - VOE_CONF_MPLS:BFD_CONFIG.BFD_CC_AUTH_ENA
 * 
 * BFD CV:
 *  - VOE_CONF_MPLS:BFD_CONFIG.BFD_CV_AUTH_ENA
 * 
 * The AUTH_MISMATCH test will fail if the incoming BFD PDUs AUTH bit does
 * not match the configured value.
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_RX_STICKY . AUTH_MISMATCH_ERR_STICKY
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_BFD_RX_STICKY_AUTH_MISMATCH_ERR_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Sticky bit will be asserted if a BFD PDU is discarded because the the
 * D_BIT_SET test fails.
 * 
 * The D_BIT_SET test will fail if a BFD PDU is received with the D bit =
 * 1.
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_RX_STICKY . D_BIT_SET_ERR_STICKY
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_BFD_RX_STICKY_D_BIT_SET_ERR_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Sticky bit will be asserted if a BFD PDU is discarded because the the
 * P_AND_F_BIT_SET test fails.
 * 
 * The P_AND_F_BIT_SET fails if a BFD PDU is received with both the Poll
 * Flag = 1 and the Final flag = 1 
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_RX_STICKY . P_AND_F_BIT_SET_ERR_STICKY
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_BFD_RX_STICKY_P_AND_F_BIT_SET_ERR_STICKY  VTSS_BIT(0)


/** 
 * \brief BFD TX verification sticky bits.
 *
 * \details
 * This register contains sticky bits indicating the different TX errors
 * detecable by the VOE.
 *
 * Register: \a VOP_MPLS:VOE_STAT_MPLS:BFD_TX_STICKY
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_TX_STICKY(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x0,gi,32,0,13)

/** 
 * \brief
 * Sticky bit asserted if a BFD PDU is transmitted on by a VOE and the VOE
 * is configured to update the BFD PDU:
 * 
 *  - VOE_CONF_MPLS:BFD_CONFIG.BFD_TX_UPDATE_ENA
 * 
 * and the TX BFD PDU My Discriminator value does not match any of the
 * configured BFD Discriminator Values:
 * 
 * Coordinated or NEIS session:
 *  - VOE_CONF_MPLS:BFD_LOCAL_DISCR_SRC.BFD_LOCAL_DISCR_SRC
 * 
 * FEIS session:
 *  - VOE_CONF_MPLS:BFD_LOCAL_DISCR_SINK.BFD_LOCAL_DISCR_SINK
 * 
 * The type of sesssion must be configured using the following bit field:
 * 
 *  - VOE_CONF_MPLS:BFD_CONFIG.BFD_COORDINATED_MODE_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_TX_STICKY . TX_MY_DISCR_MISMATCH
 */
#define  VTSS_F_VOP_MPLS_VOE_STAT_MPLS_BFD_TX_STICKY_TX_MY_DISCR_MISMATCH  VTSS_BIT(0)


/** 
 * \details
 * The number of BFD CC PDUs transmitted by the VOE
 *
 * Register: \a VOP_MPLS:VOE_STAT_MPLS:BFD_CC_TX_CNT_REG
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_CC_TX_CNT_REG(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x0,gi,32,0,14)


/** 
 * \details
 * The number of BFD CV PDUs transmitted by the VOE
 *
 * Register: \a VOP_MPLS:VOE_STAT_MPLS:BFD_CV_TX_CNT_REG
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_CV_TX_CNT_REG(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x0,gi,32,0,15)


/** 
 * \details
 * The number of valid BFD CC PDUs received by the VOE
 *
 * Register: \a VOP_MPLS:VOE_STAT_MPLS:BFD_CC_RX_VLD_CNT_REG
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_CC_RX_VLD_CNT_REG(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x0,gi,32,0,16)


/** 
 * \details
 * The number of valid BFD CV PDUs received by the VOE
 *
 * Register: \a VOP_MPLS:VOE_STAT_MPLS:BFD_CV_RX_VLD_CNT_REG
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_CV_RX_VLD_CNT_REG(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x0,gi,32,0,17)


/** 
 * \details
 * The number of in-valid BFD CC PDUs received by the VOE. Invalid PDUs are
 * RX BFD CC which are discarded due to failing the BFD RX verification.
 *
 * Register: \a VOP_MPLS:VOE_STAT_MPLS:BFD_CC_RX_INVLD_CNT_REG
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_CC_RX_INVLD_CNT_REG(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x0,gi,32,0,18)


/** 
 * \details
 * The number of in-valid BFD CV PDUs received by the VOE. Invalid PDUs are
 * RX BFD CC which are discarded due to failing the BFD RX verification.
 *
 * Register: \a VOP_MPLS:VOE_STAT_MPLS:BFD_CV_RX_INVLD_CNT_REG
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_MPLS_VOE_STAT_MPLS_BFD_CV_RX_INVLD_CNT_REG(gi)  VTSS_IOREG_IX(VTSS_TO_VOP_MPLS,0x0,gi,32,0,19)


#endif /* _VTSS_JAGUAR2_REGS_VOP_MPLS_H_ */
