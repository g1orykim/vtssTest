#ifndef _VTSS_JAGUAR2_REGS_VOP_H_
#define _VTSS_JAGUAR2_REGS_VOP_H_

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
 * Target: \a VOP
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
 * Register Group: \a VOP:COMMON
 *
 * Configuration Registers for OAM VOP
 */


/** 
 * \brief Miscellaneous Vitesse OAM Processor Controls
 *
 * \details
 * This register contains variable settings which are global to all VOEs.
 *
 * Register: \a VOP:COMMON:VOP_CTRL
 */
#define VTSS_VOP_COMMON_VOP_CTRL             VTSS_IOREG(VTSS_TO_VOP,0x10d42)

/** 
 * \brief
 * The CCM PDU does not have a dedicated register for the RxFC_f counter.
 * However there is a reserved field which can be used to hold this value.
 * 
 * Asserting this field enables updating reserved field for RX CCM LM PDU
 * with RxFC_f.
 *
 * \details 
 * 0: Do not update CCM-LM frames with RxFC_f information
 * 1: Update the CCM-LM Reserved field with RxFC_f information
 *
 * Field: ::VTSS_VOP_COMMON_VOP_CTRL . CCM_LM_UPD_RSV_ENA
 */
#define  VTSS_F_VOP_COMMON_VOP_CTRL_CCM_LM_UPD_RSV_ENA  VTSS_BIT(6)

/** 
 * \brief
 * The LMR PDU does not have any fields reserved to hold the RxFC_b counter
 * upon reception of an LMR/SLR frame.
 * 
 * For LMR:
 * ----------------
 * The VOE supports adding the RxFC_b counter following the other three
 * counters in the LMR PDU. This requires extending the first TLV from 12
 * --> 16 bytes.
 * 
 * Asserting this fields enables updating TLV OFFSET to 16 and setting
 * inserting RxFcLocal after TxFcb.
 * Further a new End TLV = 0 is written right after the LMR.RxFCb (at
 * offset = 16)
 * 
 * For SLR:
 * --------------
 * The VOE supports adding the RxFC_b counter following the other three
 * counters in the SLR PDU. This requires extending the first TLV from 16
 * --> 20 bytes.
 * 
 * Asserting this fields enables updating TLV OFFSET to 16 and setting
 * inserting RxFcLocal after TxFcb.
 * Further a new End TLV = 0 is written right after the SLR.RxFCb (at
 * offset = 16)
 * 
 * This solution is a proprietary solution, which allows forwarding the
 * RxFC_b value to an internal or external CPU.
 * 
 * 
 * NOTE this will result in a non standards compliant frame.
 * However, this is the only way to convey the RxFC_b frame count to a CPU.
 *
 * \details 
 * 0: No modifications to LMR upon reception
 * 1: Update LMR frames as described above.
 *
 * Field: ::VTSS_VOP_COMMON_VOP_CTRL . LMR_UPD_RxFcL_ENA
 */
#define  VTSS_F_VOP_COMMON_VOP_CTRL_LMR_UPD_RxFcL_ENA  VTSS_BIT(5)

/** 
 * \brief
 * Enables or disables automated LOC SCAN to be used for Loss Of Continuity
 * (LOC) handling.
 * 
 * When the LOC SCAN is disabled, no check for LOC is carried out.
 * 
 * When the LOC SCAN is enabled, the LOC scan will be performed as
 * configured in register:
 * 
 * OAM_MEP::COMMON:CCM_CTRL.*
 * 
 * When the LOC scan controller is disabled, the LOC scan controller is
 * automatically reset.
 * I.e. all internal counters are set to default values.
 * 
 * NOTE: This does not affect the LOC miss counters in the VOE.
 *
 * \details 
 * Field: ::VTSS_VOP_COMMON_VOP_CTRL . LOC_SCAN_ENA
 */
#define  VTSS_F_VOP_COMMON_VOP_CTRL_LOC_SCAN_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enables or disables the functionality of the VOEs in the VOP.
 * 
 * The VOE and the single VOEs can still be configured even when the VOP is
 * disabled.
 * Further each individual VOE must be enabled to enable its functionality.
 *
 * \details 
 * Field: ::VTSS_VOP_COMMON_VOP_CTRL . VOP_ENA
 */
#define  VTSS_F_VOP_COMMON_VOP_CTRL_VOP_ENA   VTSS_BIT(0)

/** 
 * \brief
 * The VOE will detect changes in the source port for CCM PDUs.
 * 
 * The port on which the last valid PDU was received is saved :
 * 
 *  - VOE_STAT:CCM_RX_LAST.CCM_RX_SRC_PORT
 * 
 * The VOE counts the number of valid CCM PDUs received on the same port as
 * the previous in the following register:
 * 
 *  - VOE_STAT:CCM_STAT.CCM_RX_SRC_PORT_CNT
 * 
 * If the this count reaches the value configured in this field, the
 * following sticky bit is asserted:
 * 
 *  - VOE_STAT:INTR_STICKY.CCM_RX_SRC_PORT_DETECT_STICKY
 * 
 * The sticky bit can optionally generate an interrupt.
 * 
 * Note: that the VOE counts the number of frames matching the previous,
 * hence if this bitfield is programmed to 'X', it will require 'X' + 1
 * consecutive frames on the same port to assert the sticky bit.
 *
 * \details 
 * 0 : illegal value (Will never generate a sticky bit)
 * 1-7 : Assert sticky bit when 1-7 CCM PDUs were received on the same port
 * as the previous.
 *
 * Field: ::VTSS_VOP_COMMON_VOP_CTRL . CCM_RX_SRC_PORT_DETECT_CNT
 */
#define  VTSS_F_VOP_COMMON_VOP_CTRL_CCM_RX_SRC_PORT_DETECT_CNT(x)  VTSS_ENCODE_BITFIELD(x,2,3)
#define  VTSS_M_VOP_COMMON_VOP_CTRL_CCM_RX_SRC_PORT_DETECT_CNT     VTSS_ENCODE_BITMASK(2,3)
#define  VTSS_X_VOP_COMMON_VOP_CTRL_CCM_RX_SRC_PORT_DETECT_CNT(x)  VTSS_EXTRACT_BITFIELD(x,2,3)


/** 
 * \brief Configuring destination for frames extracted to the CPU
 *
 * \details
 * This register configures the destination for OAM frames which are
 * extracted to CPU for various reasons.
 * 
 * An OAM PDU may be extracted to the CPU for various reasons:
 *  - Based on PDU type (See register: VOE_CONF:OAM_CPU_COPY_CTRL.*
 *  - An generan OAM error condition (e.g. DMAC error)
 *  - PDU specific extraction reasons.
 * 
 * Most HW supported OAM PDUs have their own configuration, while some
 * related PDUs share a single configuration.
 * 
 * The configuration for each OAM PDU consists of one bit field which
 * indicate which of the extraction queues the relevant PDUs are extracted:
 * 
 *  *_CPU_QU
 * 
 * OAM PDU types which do not have a specific configuration will use the
 * default configuration:
 * 
 * DEF_COPY_CPU_QU
 * 
 * The details are described for each bit field.
 *
 * Register: \a VOP:COMMON:CPU_EXTR_CFG
 */
#define VTSS_VOP_COMMON_CPU_EXTR_CFG         VTSS_IOREG(VTSS_TO_VOP,0x10d43)

/** 
 * \brief
 * Configures the destination for PDUs extracted to the Default CPU queue.
 * 
 * The default CPU extraction queue is used for extraction of PDUs which do
 * not have a dedicated extraction queue (UNKNOWN OPCODES).

 *
 * \details 
 * [TBU]
 *
 * Field: ::VTSS_VOP_COMMON_CPU_EXTR_CFG . DEF_COPY_QU
 */
#define  VTSS_F_VOP_COMMON_CPU_EXTR_CFG_DEF_COPY_QU(x)  VTSS_ENCODE_BITFIELD(x,15,3)
#define  VTSS_M_VOP_COMMON_CPU_EXTR_CFG_DEF_COPY_QU     VTSS_ENCODE_BITMASK(15,3)
#define  VTSS_X_VOP_COMMON_CPU_EXTR_CFG_DEF_COPY_QU(x)  VTSS_EXTRACT_BITFIELD(x,15,3)

/** 
 * \brief
 * Configures the CPU queue port of the CPU error queue.

 *
 * \details 
 * [TBU]
 *
 * Field: ::VTSS_VOP_COMMON_CPU_EXTR_CFG . CPU_ERR_QU
 */
#define  VTSS_F_VOP_COMMON_CPU_EXTR_CFG_CPU_ERR_QU(x)  VTSS_ENCODE_BITFIELD(x,12,3)
#define  VTSS_M_VOP_COMMON_CPU_EXTR_CFG_CPU_ERR_QU     VTSS_ENCODE_BITMASK(12,3)
#define  VTSS_X_VOP_COMMON_CPU_EXTR_CFG_CPU_ERR_QU(x)  VTSS_EXTRACT_BITFIELD(x,12,3)

/** 
 * \brief
 * Configures the CPU queue to which LMM frames are extracted.
 *
 * \details 
 * Field: ::VTSS_VOP_COMMON_CPU_EXTR_CFG . LMM_CPU_QU
 */
#define  VTSS_F_VOP_COMMON_CPU_EXTR_CFG_LMM_CPU_QU(x)  VTSS_ENCODE_BITFIELD(x,9,3)
#define  VTSS_M_VOP_COMMON_CPU_EXTR_CFG_LMM_CPU_QU     VTSS_ENCODE_BITMASK(9,3)
#define  VTSS_X_VOP_COMMON_CPU_EXTR_CFG_LMM_CPU_QU(x)  VTSS_EXTRACT_BITFIELD(x,9,3)

/** 
 * \brief
 * Configures the CPU queue to which LMR / SLR / 1SL frames are extracted.
 *
 * \details 
 * [TBU]
 *
 * Field: ::VTSS_VOP_COMMON_CPU_EXTR_CFG . LMR_CPU_QU
 */
#define  VTSS_F_VOP_COMMON_CPU_EXTR_CFG_LMR_CPU_QU(x)  VTSS_ENCODE_BITFIELD(x,6,3)
#define  VTSS_M_VOP_COMMON_CPU_EXTR_CFG_LMR_CPU_QU     VTSS_ENCODE_BITMASK(6,3)
#define  VTSS_X_VOP_COMMON_CPU_EXTR_CFG_LMR_CPU_QU(x)  VTSS_EXTRACT_BITFIELD(x,6,3)

/** 
 * \brief
 * Configures the CPU queue to which DMM frames are extracted.

 *
 * \details 
 * Field: ::VTSS_VOP_COMMON_CPU_EXTR_CFG . DMM_CPU_QU
 */
#define  VTSS_F_VOP_COMMON_CPU_EXTR_CFG_DMM_CPU_QU(x)  VTSS_ENCODE_BITFIELD(x,3,3)
#define  VTSS_M_VOP_COMMON_CPU_EXTR_CFG_DMM_CPU_QU     VTSS_ENCODE_BITMASK(3,3)
#define  VTSS_X_VOP_COMMON_CPU_EXTR_CFG_DMM_CPU_QU(x)  VTSS_EXTRACT_BITFIELD(x,3,3)

/** 
 * \brief
 * Configures the CPU queue to which DMR and 1DM frames are extracted.

 *
 * \details 
 * Field: ::VTSS_VOP_COMMON_CPU_EXTR_CFG . DMR_CPU_QU
 */
#define  VTSS_F_VOP_COMMON_CPU_EXTR_CFG_DMR_CPU_QU(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_VOP_COMMON_CPU_EXTR_CFG_DMR_CPU_QU     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_VOP_COMMON_CPU_EXTR_CFG_DMR_CPU_QU(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Configuring destination for frames extracted to the CPU.
 *
 * \details
 * See description for CPU_CFG
 *
 * Register: \a VOP:COMMON:CPU_EXTR_CFG_1
 */
#define VTSS_VOP_COMMON_CPU_EXTR_CFG_1       VTSS_IOREG(VTSS_TO_VOP,0x10d44)

/** 
 * \brief
 * Configures the CPU queue to which CCM frames are extracted.

 *
 * \details 
 * Field: ::VTSS_VOP_COMMON_CPU_EXTR_CFG_1 . CCM_CPU_QU
 */
#define  VTSS_F_VOP_COMMON_CPU_EXTR_CFG_1_CCM_CPU_QU(x)  VTSS_ENCODE_BITFIELD(x,15,3)
#define  VTSS_M_VOP_COMMON_CPU_EXTR_CFG_1_CCM_CPU_QU     VTSS_ENCODE_BITMASK(15,3)
#define  VTSS_X_VOP_COMMON_CPU_EXTR_CFG_1_CCM_CPU_QU(x)  VTSS_EXTRACT_BITFIELD(x,15,3)

/** 
 * \brief
 * Configures the CPU queue to which CCM-LM frames are extracted.

 *
 * \details 
 * Field: ::VTSS_VOP_COMMON_CPU_EXTR_CFG_1 . CCM_LM_CPU_QU
 */
#define  VTSS_F_VOP_COMMON_CPU_EXTR_CFG_1_CCM_LM_CPU_QU(x)  VTSS_ENCODE_BITFIELD(x,12,3)
#define  VTSS_M_VOP_COMMON_CPU_EXTR_CFG_1_CCM_LM_CPU_QU     VTSS_ENCODE_BITMASK(12,3)
#define  VTSS_X_VOP_COMMON_CPU_EXTR_CFG_1_CCM_LM_CPU_QU(x)  VTSS_EXTRACT_BITFIELD(x,12,3)

/** 
 * \brief
 * Configures the CPU queue to which LBM frames are extracted.

 *
 * \details 
 * [TBU]
 *
 * Field: ::VTSS_VOP_COMMON_CPU_EXTR_CFG_1 . LBM_CPU_QU
 */
#define  VTSS_F_VOP_COMMON_CPU_EXTR_CFG_1_LBM_CPU_QU(x)  VTSS_ENCODE_BITFIELD(x,9,3)
#define  VTSS_M_VOP_COMMON_CPU_EXTR_CFG_1_LBM_CPU_QU     VTSS_ENCODE_BITMASK(9,3)
#define  VTSS_X_VOP_COMMON_CPU_EXTR_CFG_1_LBM_CPU_QU(x)  VTSS_EXTRACT_BITFIELD(x,9,3)

/** 
 * \brief
 * Configures the CPU queue to which LBR and SAM_SEQ frames are extracted.

 *
 * \details 
 * Field: ::VTSS_VOP_COMMON_CPU_EXTR_CFG_1 . LBR_CPU_QU
 */
#define  VTSS_F_VOP_COMMON_CPU_EXTR_CFG_1_LBR_CPU_QU(x)  VTSS_ENCODE_BITFIELD(x,6,3)
#define  VTSS_M_VOP_COMMON_CPU_EXTR_CFG_1_LBR_CPU_QU     VTSS_ENCODE_BITMASK(6,3)
#define  VTSS_X_VOP_COMMON_CPU_EXTR_CFG_1_LBR_CPU_QU(x)  VTSS_EXTRACT_BITFIELD(x,6,3)

/** 
 * \brief
 * Configures the CPU queue to which TST frames are extracted.

 *
 * \details 
 * Field: ::VTSS_VOP_COMMON_CPU_EXTR_CFG_1 . TST_CPU_QU
 */
#define  VTSS_F_VOP_COMMON_CPU_EXTR_CFG_1_TST_CPU_QU(x)  VTSS_ENCODE_BITFIELD(x,3,3)
#define  VTSS_M_VOP_COMMON_CPU_EXTR_CFG_1_TST_CPU_QU     VTSS_ENCODE_BITMASK(3,3)
#define  VTSS_X_VOP_COMMON_CPU_EXTR_CFG_1_TST_CPU_QU(x)  VTSS_EXTRACT_BITFIELD(x,3,3)

/** 
 * \brief
 * LTM and LTR PDUs are extracted to the same queue.
 * "LTx" is used to denote both LTM and LTR OAM PDUs.

 *
 * \details 
 * Field: ::VTSS_VOP_COMMON_CPU_EXTR_CFG_1 . LT_CPU_QU
 */
#define  VTSS_F_VOP_COMMON_CPU_EXTR_CFG_1_LT_CPU_QU(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_VOP_COMMON_CPU_EXTR_CFG_1_LT_CPU_QU     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_VOP_COMMON_CPU_EXTR_CFG_1_LT_CPU_QU(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Configuring destination for frames extracted to the CPU
 *
 * \details
 * See description for CPU_CFG
 *
 * Register: \a VOP:COMMON:CPU_EXTR_MPLS
 */
#define VTSS_VOP_COMMON_CPU_EXTR_MPLS        VTSS_IOREG(VTSS_TO_VOP,0x10d45)

/** 
 * \brief
 * Configures the CPU queue / external port to which BFD-CC PDUs are
 * extracted.
 * 
 * Whether BFD-CC frames are extracted to a CPU queue or an external port
 * depends on the configuration of:
 * 
 *  - BFD_CC_EXT_PORT_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_COMMON_CPU_EXTR_MPLS . BFD_CC_CPU_QU
 */
#define  VTSS_F_VOP_COMMON_CPU_EXTR_MPLS_BFD_CC_CPU_QU(x)  VTSS_ENCODE_BITFIELD(x,3,3)
#define  VTSS_M_VOP_COMMON_CPU_EXTR_MPLS_BFD_CC_CPU_QU     VTSS_ENCODE_BITMASK(3,3)
#define  VTSS_X_VOP_COMMON_CPU_EXTR_MPLS_BFD_CC_CPU_QU(x)  VTSS_EXTRACT_BITFIELD(x,3,3)

/** 
 * \brief
 * Configures the CPU queue / external port to which DMR frames are
 * extracted.
 * 
 * Whether DMR frames are extracted to a CPU queue or an external port
 * depends on the configuration of:
 * 
 *  - DMR_EXT_PORT_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_COMMON_CPU_EXTR_MPLS . BFD_CV_CPU_QU
 */
#define  VTSS_F_VOP_COMMON_CPU_EXTR_MPLS_BFD_CV_CPU_QU(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_VOP_COMMON_CPU_EXTR_MPLS_BFD_CV_CPU_QU     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_VOP_COMMON_CPU_EXTR_MPLS_BFD_CV_CPU_QU(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Define valid y.1731 PDU version number.
 *
 * \details
 * This register group allows configuration of which versions of the PDU is
 * valid for each of the Y.1731 PDUs supported by HW processing.
 * 
 * Only the PDUs which are being processed by VOE HW can be verified by the
 * VOE.
 * 
 * For PDUs being forwarded to the CPU the version is assumed to be
 * verified in SW.
 * 
 * The version is configured commonly for Message and Response of the same
 * PDU type.
 * 
 * For each PDU (pair) there are 8 bits, each representing a version
 * number: 0 - 7.
 * 
 * Asserting the bit corresponding to a given version will configure the HW
 * to accept PDUs of this version.
 * 
 * E.g. the following will configure VERSION = 1 as being valid for
 * CCM(-LM) frames.
 *  * CCM_VERSION(1) = 1
 *
 * Register: \a VOP:COMMON:VERSION_CTRL
 */
#define VTSS_VOP_COMMON_VERSION_CTRL         VTSS_IOREG(VTSS_TO_VOP,0x10d46)

/** 
 * \brief
 * Configure which version of the CCM(-LM) PDU will be processed by the VOE
 * HW.
 *
 * \details 
 * CCM_VERSION(x) = '0'; Version X of the CCM(-LM) PDU is invalid.
 * CCM_VERSION(x) = '1'; Version X of the CCM(-LM) PDU is valid.
 *
 * Field: ::VTSS_VOP_COMMON_VERSION_CTRL . CCM_VERSION
 */
#define  VTSS_F_VOP_COMMON_VERSION_CTRL_CCM_VERSION(x)  VTSS_ENCODE_BITFIELD(x,24,8)
#define  VTSS_M_VOP_COMMON_VERSION_CTRL_CCM_VERSION     VTSS_ENCODE_BITMASK(24,8)
#define  VTSS_X_VOP_COMMON_VERSION_CTRL_CCM_VERSION(x)  VTSS_EXTRACT_BITFIELD(x,24,8)

/** 
 * \brief
 * Configure which version of the LMM/LMR PDU will be processed by the VOE
 * HW.
 *
 * \details 
 * LM_VERSION(x) = '0'; Version X of the LMM/LMR PDU is invalid.
 * LM_VERSION(x) = '1'; Version X of the LMM/LMR PDU is valid.
 *
 * Field: ::VTSS_VOP_COMMON_VERSION_CTRL . LM_VERSION
 */
#define  VTSS_F_VOP_COMMON_VERSION_CTRL_LM_VERSION(x)  VTSS_ENCODE_BITFIELD(x,16,8)
#define  VTSS_M_VOP_COMMON_VERSION_CTRL_LM_VERSION     VTSS_ENCODE_BITMASK(16,8)
#define  VTSS_X_VOP_COMMON_VERSION_CTRL_LM_VERSION(x)  VTSS_EXTRACT_BITFIELD(x,16,8)

/** 
 * \brief
 * Configure which version of the DMM/DMR PDU will be processed by the VOE
 * HW.
 *
 * \details 
 * DM_VERSION(x) = '0'; Version X of the DMM/DMR PDU is invalid.
 * DM_VERSION(x) = '1'; Version X of the DMM/DMR PDU is valid.
 *
 * Field: ::VTSS_VOP_COMMON_VERSION_CTRL . DM_VERSION
 */
#define  VTSS_F_VOP_COMMON_VERSION_CTRL_DM_VERSION(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_VOP_COMMON_VERSION_CTRL_DM_VERSION     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_VOP_COMMON_VERSION_CTRL_DM_VERSION(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Configure which version of the 1DM PDU will be processed by the VOE HW.
 *
 * \details 
 * SDM_VERSION(x) = '0'; Version X of the 1DM PDU is invalid.
 * SDM_VERSION(x) = '1'; Version X of the 1DM PDU is valid.
 *
 * Field: ::VTSS_VOP_COMMON_VERSION_CTRL . SDM_VERSION
 */
#define  VTSS_F_VOP_COMMON_VERSION_CTRL_SDM_VERSION(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VOP_COMMON_VERSION_CTRL_SDM_VERSION     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VOP_COMMON_VERSION_CTRL_SDM_VERSION(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \details
 * This register group allows configuration of which versions of the PDU is
 * valid for each of the Y.1731 PDUs supported by HW processing.
 * 
 * Only the PDUs which are being processed by VOE HW can be verified by the
 * VOE.
 * 
 * For PDUs being forwarded to the CPU the version is assumed to be
 * verified in SW.
 * 
 * The version is configured commonly for Message and Response of the same
 * PDU type.
 * 
 * For each PDU (pair) there are 8 bits, each representing a version
 * number: 0 - 7.
 * 
 * Asserting the bit corresponding to a given version will configure the HW
 * to accept PDUs of this version.
 * 
 * E.g. the following will configure VERSION = 1 as being valid for
 * CCM(-LM) frames.
 *  * CCM_VERSION(1) = 1
 *
 * Register: \a VOP:COMMON:VERSION_CTRL_2
 */
#define VTSS_VOP_COMMON_VERSION_CTRL_2       VTSS_IOREG(VTSS_TO_VOP,0x10d47)

/** 
 * \brief
 * Configure which version of the TST PDU will be processed by the VOE HW.
 *
 * \details 
 * TST_VERSION(x) = '0'; Version X of the TST PDU is invalid.
 * TST_VERSION(x) = '1'; Version X of the TST PDU is valid.
 *
 * Field: ::VTSS_VOP_COMMON_VERSION_CTRL_2 . TST_VERSION
 */
#define  VTSS_F_VOP_COMMON_VERSION_CTRL_2_TST_VERSION(x)  VTSS_ENCODE_BITFIELD(x,24,8)
#define  VTSS_M_VOP_COMMON_VERSION_CTRL_2_TST_VERSION     VTSS_ENCODE_BITMASK(24,8)
#define  VTSS_X_VOP_COMMON_VERSION_CTRL_2_TST_VERSION(x)  VTSS_EXTRACT_BITFIELD(x,24,8)

/** 
 * \brief
 * Configure which version of the LBM/LBR PDU will be processed by the VOE
 * HW.
 *
 * \details 
 * LB_VERSION(x) = '0'; Version X of the LBM/LBR PDU is invalid.
 * LB_VERSION(x) = '1'; Version X of the LBM/LBR PDU is valid.
 *
 * Field: ::VTSS_VOP_COMMON_VERSION_CTRL_2 . LB_VERSION
 */
#define  VTSS_F_VOP_COMMON_VERSION_CTRL_2_LB_VERSION(x)  VTSS_ENCODE_BITFIELD(x,16,8)
#define  VTSS_M_VOP_COMMON_VERSION_CTRL_2_LB_VERSION     VTSS_ENCODE_BITMASK(16,8)
#define  VTSS_X_VOP_COMMON_VERSION_CTRL_2_LB_VERSION(x)  VTSS_EXTRACT_BITFIELD(x,16,8)

/** 
 * \brief
 * Configure which version of the SLM/SLR PDU will be processed by the VOE
 * HW.
 *
 * \details 
 * Field: ::VTSS_VOP_COMMON_VERSION_CTRL_2 . SL_VERSION
 */
#define  VTSS_F_VOP_COMMON_VERSION_CTRL_2_SL_VERSION(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_VOP_COMMON_VERSION_CTRL_2_SL_VERSION     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_VOP_COMMON_VERSION_CTRL_2_SL_VERSION(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Configure which version of the 1SL PDU will be processed by the VOE HW.
 *
 * \details 
 * Field: ::VTSS_VOP_COMMON_VERSION_CTRL_2 . SL1_VERSION
 */
#define  VTSS_F_VOP_COMMON_VERSION_CTRL_2_SL1_VERSION(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VOP_COMMON_VERSION_CTRL_2_SL1_VERSION     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VOP_COMMON_VERSION_CTRL_2_SL1_VERSION(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Version control configuration for MPLS
 *
 * \details
 * Register: \a VOP:COMMON:VERSION_CTRL_MPLS
 */
#define VTSS_VOP_COMMON_VERSION_CTRL_MPLS    VTSS_IOREG(VTSS_TO_VOP,0x10d48)

/** 
 * \brief
 * The VOE will optionally validate the version of the incoming BFD frames
 * against the value configured in this register.
 * 
 * If the version in the incoming frame is not as configured, the frame
 * will be discarded.
 * 
 * The RX validation is configured in the following bitfields:
 * 
 *  - VOE_CONF_MPLS:BFD_CONF.BFD_RX_VERIFY_*
 *
 * \details 
 * Field: ::VTSS_VOP_COMMON_VERSION_CTRL_MPLS . BFD_VERSION
 */
#define  VTSS_F_VOP_COMMON_VERSION_CTRL_MPLS_BFD_VERSION(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_VOP_COMMON_VERSION_CTRL_MPLS_BFD_VERSION     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_VOP_COMMON_VERSION_CTRL_MPLS_BFD_VERSION(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Generic OAM PDU Opcodes configuration
 *
 * \details
 * The VOE implements HW support for a number of selected OAM PDUs which
 * all have dedicated control registers.
 * 
 * In addition to the selected OAM PDUs with dedicated control registers,
 * the VOE further supports configuring 8 Generic Opcodes, which can be
 * extracted to CPU or forwarded independently of other OpCodes.
 * 
 * The value of each OpCode is configured globally across all VOEs in the
 * following bit field:
 * 
 *  - GENERIC_OPCODE_VAL
 * 
 * An opcode which is not explicitly supported and which not configured as
 * a Generic OpCode will be treated as an UNKNOWN opcode.
 * 
 * The extraction and forwarding of Generic OpCodes can be configured
 * individually pr. VOE using the following registers:
 * 
 * VOE_CONF:OAM_CPU_COPY_CTRL:GENERIC_COPY_MASK
 * VOE_CONF:CENTRAL_OAM_CTRL:GENERIC_FWD_MASK
 * VOE_CONF:OAM_CNT_OAM_CTRL:GENERIC_OAM_CNT_MASK
 * VOE_CONF:OAM_CNT_DATA_CTRL:GENERIC_DATA_CNT_MASK
 *
 * Register: \a VOP:COMMON:OAM_GENERIC_CFG
 *
 * @param ri Replicator: x_MEP_NUM_GEN_OPCODES (??), 0-7
 */
#define VTSS_VOP_COMMON_OAM_GENERIC_CFG(ri)  VTSS_IOREG(VTSS_TO_VOP,0x10d49 + (ri))

/** 
 * \brief
 * This value configures the Y.1731 OpCode of to be processed as a Generic
 * OpCode corresponding to this generic Index.
 *
 * \details 
 * Field: ::VTSS_VOP_COMMON_OAM_GENERIC_CFG . GENERIC_OPCODE_VAL
 */
#define  VTSS_F_VOP_COMMON_OAM_GENERIC_CFG_GENERIC_OPCODE_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VOP_COMMON_OAM_GENERIC_CFG_GENERIC_OPCODE_VAL     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VOP_COMMON_OAM_GENERIC_CFG_GENERIC_OPCODE_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,8)

/** 
 * \brief
 * Configures the CPU queue to which frames for this generic OpCode are
 * extracted.
 *
 * \details 
 * Field: ::VTSS_VOP_COMMON_OAM_GENERIC_CFG . GENERIC_OPCODE_CPU_QU
 */
#define  VTSS_F_VOP_COMMON_OAM_GENERIC_CFG_GENERIC_OPCODE_CPU_QU(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_VOP_COMMON_OAM_GENERIC_CFG_GENERIC_OPCODE_CPU_QU     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_VOP_COMMON_OAM_GENERIC_CFG_GENERIC_OPCODE_CPU_QU(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * If this bit is asserted, the DMAC check will be disabled for this
 * generic OpCode, regardless of the configuration of the bit field:
 * 
 * VOE_CONF:VOE_CTRL.RX_DMAC_CHK_SEL
 * 
 * This bit is required because some OpCodes (i.e. Ring PDU) will use a
 * DMAC which is different from the configured Multicast and Unicast.
 *
 * \details 
 * 0 : Perform DMAC check according to the value configured in :
 * VOE_CONF:VOE_CTRL.RX_DMAC_CHK_SEL
 * 1: No DMAC for this Generic OpCode.
 *
 * Field: ::VTSS_VOP_COMMON_OAM_GENERIC_CFG . GENERIC_DMAC_CHK_DIS
 */
#define  VTSS_F_VOP_COMMON_OAM_GENERIC_CFG_GENERIC_DMAC_CHK_DIS  VTSS_BIT(17)


/** 
 * \brief Generic OAM PDU Opcodes configuration
 *
 * \details
 * The VOE implements HW support for a number of dedicated MPLS-TP
 * codepoints (BFD CC/CV) which all have dedicated control registers.
 * 
 * In addition to the dedicated MPLS-TP Codepoints with dedicated control
 * registers, the VOE further supports configuring 8 Generic Codepoints,
 * which can be extracted to CPU or forwarded independently of other
 * Codepoints.
 * 
 * The Generic Codepoints are configured using this register.
 * 
 * The value of each Codepoint is configured globally across all VOEs in
 * the following bit field:
 * 
 *  - GENERIC_CODEPOINT_VAL
 * 
 * A codepoint which is not explicitly supported and which not configured
 * as a Generic Codepoint will be treated as an UNKNOWN Codepoint.
 * 
 * The extraction and forwarding of Generic Codepoints can be configured
 * individually pr. VOE using the following registers:
 * 
 * VOE_CONF_MPLS:OAM_COPY_CTRL_MPLS:GENERIC_COPY_MASK
 * VOE_CONF_MPLS:OAM_CNT_SEL_MPLS:GENERIC_CPT_CNT_SEL_MASK
 * VOE_CONF_MPLS:OAM_CNT_DATA_CMPLS:GENERIC_CPT_CNT_DATA_MASK
 *
 * Register: \a VOP:COMMON:MPLS_GENERIC_CODEPOINT
 *
 * @param ri Replicator: x_MEP_NUM_GEN_OPCODES (??), 0-7
 */
#define VTSS_VOP_COMMON_MPLS_GENERIC_CODEPOINT(ri)  VTSS_IOREG(VTSS_TO_VOP,0x10d51 + (ri))

/** 
 * \brief
 * This value configures the MPLS-TP Codepoint of to be processed as a
 * Generic OpCode corresponding to this generic Index.
 *
 * \details 
 * Field: ::VTSS_VOP_COMMON_MPLS_GENERIC_CODEPOINT . GENERIC_CODEPOINT_VAL
 */
#define  VTSS_F_VOP_COMMON_MPLS_GENERIC_CODEPOINT_GENERIC_CODEPOINT_VAL(x)  VTSS_ENCODE_BITFIELD(x,11,16)
#define  VTSS_M_VOP_COMMON_MPLS_GENERIC_CODEPOINT_GENERIC_CODEPOINT_VAL     VTSS_ENCODE_BITMASK(11,16)
#define  VTSS_X_VOP_COMMON_MPLS_GENERIC_CODEPOINT_GENERIC_CODEPOINT_VAL(x)  VTSS_EXTRACT_BITFIELD(x,11,16)

/** 
 * \brief
 * Configures CPU queue or external CPU port, according to the
 * configuration of bit field :
 * GENERIC_CODEPOINT_EXT_PORT_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_COMMON_MPLS_GENERIC_CODEPOINT . GENERIC_CODEPOINT_CPU_QU
 */
#define  VTSS_F_VOP_COMMON_MPLS_GENERIC_CODEPOINT_GENERIC_CODEPOINT_CPU_QU(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_VOP_COMMON_MPLS_GENERIC_CODEPOINT_GENERIC_CODEPOINT_CPU_QU     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_VOP_COMMON_MPLS_GENERIC_CODEPOINT_GENERIC_CODEPOINT_CPU_QU(x)  VTSS_EXTRACT_BITFIELD(x,8,3)


/** 
 * \brief Configures the 7 different LOC scan periods
 *
 * \details
 * Independent timers are implemented to be used with the Loss Of
 * Continuity (LOC) SCAN.
 * 
 * This register implements the timout period for every one of the 7 LOC
 * counters.
 * The timeout period is specified in the number of LOC base ticks between
 * every LOC timeout.
 * 
 * For configuration of the LOC base tick, see bit field:
 *  - LOC_CTRL.LOC_BASE_TICK_CNT
 * 
 * The default value for the LOC base tick is 200 ns, which means that the
 * LOC timer counters are incremented every 200 ns.
 * 
 * A LOC miss count scan is initiated at half the configured LOC PERIOD,
 * and a LOC event is generated at the VOE when the CCM_MISS count is 7
 * (for Ethernet).
 * 
 * This effectively implements a LOC event at 3,5 times the configured LOC
 * period.
 * 
 * A VOE can be configured for LOC checking based on one of the 7 timeout
 * counters.
 * When the timeout counter configured for a VOE has timed out 3 times
 * without a CCM frame being received, the LOC interrupt will be asserted.

 *
 * Register: \a VOP:COMMON:LOC_PERIOD_CFG
 *
 * @param ri Register: LOC_PERIOD_CFG (??), 0-6
 */
#define VTSS_VOP_COMMON_LOC_PERIOD_CFG(ri)   VTSS_IOREG(VTSS_TO_VOP,0x10d59 + (ri))


/** 
 * \brief LOC scan configuration
 *
 * \details
 * Configures LOC hardware SCAN

 *
 * Register: \a VOP:COMMON:LOC_CTRL
 */
#define VTSS_VOP_COMMON_LOC_CTRL             VTSS_IOREG(VTSS_TO_VOP,0x10d60)

/** 
 * \brief
 * Specifies the number of system clock cycles for each CCM base time tick.
 * The current system clock of Jaguar2-R is : 4,0 ns (250 MHz).
 * 
 * The default base tick is set to 50 = 200 ns
 * 
 * The base tick, is the event used for incrementing the 7 LOC counters.
 * The time at which each of the LOC timers will timeout is specified in : 
 * 
 *  - COMMON::CCM_PERIOD_VAL
 * 
 * With the current default setting (=TBD), the CCM base time tick will
 * occur every : 198,4 ns.
 * 
 * When a LOC timer expires it causes a LOC Scan event, which will
 * increment the LOC miss counter (VOE_STAT:CCM_STAT.CCM_MISS_CNT) for each
 * VOE assigned to that particular LOC timer.
 * 
 * The LOC counter for each VOE (CCM_MISS) will be cleared each time the
 * VOE receives a valid CCM or CCM-LM frame.
 * 
 * If the CCM_MISS count reaches 7 it will optimally cause an interrupt
 * [TBD]
 *
 * \details 
 * 0: Illegal value
 * 1: One clock between interval increment
 * ...
 * n: n clock between interval increment
 *
 * Field: ::VTSS_VOP_COMMON_LOC_CTRL . LOC_BASE_TICK_CNT
 */
#define  VTSS_F_VOP_COMMON_LOC_CTRL_LOC_BASE_TICK_CNT(x)  VTSS_ENCODE_BITFIELD(x,24,8)
#define  VTSS_M_VOP_COMMON_LOC_CTRL_LOC_BASE_TICK_CNT     VTSS_ENCODE_BITMASK(24,8)
#define  VTSS_X_VOP_COMMON_LOC_CTRL_LOC_BASE_TICK_CNT(x)  VTSS_EXTRACT_BITFIELD(x,24,8)

/** 
 * \brief
 * Specifies the number of clk cycle before another scan entry can be
 * attempted.
 * 
 * This can be used to space the LOC miss scanning of the VOEs.
 * 
 * If an active SCAN is ongoing, the VOE will scan through the VOE_STAT and
 * update the LOC MISS CNT of VOEs which are enabled for LOC detection.
 * The scan will be done in 'idle cycles'.
 * 'Idle cycles' are defined as cycles where the is no frame access and no
 * CSR access to the VOP.
 *
 * \details 
 * 0-1: A scan is attempted at every idle cycle in the VOP
 * 2: A scan is attempted no more often than every 2nd cycle.
 * 3: A scan is attempted no more often than every 3rd cycle.
 * ....
 * MAX_VAL: A scan is attempted no more often than every MAX_VAL'th cycle.
 *
 * Field: ::VTSS_VOP_COMMON_LOC_CTRL . LOC_SPACE_BETWEEN_ENTRY_SCAN
 */
#define  VTSS_F_VOP_COMMON_LOC_CTRL_LOC_SPACE_BETWEEN_ENTRY_SCAN(x)  VTSS_ENCODE_BITFIELD(x,16,3)
#define  VTSS_M_VOP_COMMON_LOC_CTRL_LOC_SPACE_BETWEEN_ENTRY_SCAN     VTSS_ENCODE_BITMASK(16,3)
#define  VTSS_X_VOP_COMMON_LOC_CTRL_LOC_SPACE_BETWEEN_ENTRY_SCAN(x)  VTSS_EXTRACT_BITFIELD(x,16,3)

/** 
 * \brief
 * When a LOC timer expires, the VOE will scan through all VOEs in the VOP
 * and increment the CCM MISS counter of all the VOEs assigned to that LOC
 * timer.
 * 
 * Another way to force a LOC scan is to write a mask to
 * LOC_FORCE_HW_SCAN_ENA. Each of the bits in this register represents a
 * LOC timer.
 * Writing a mask to this register will force a LOC scan as if the LOC
 * timers indicated in the mask had expired.
 * 
 * A forced scan will start as soon as any currently active scan completes
 * Active scan can be stopped by disableing the LOC scan controller.
 *
 * \details 
 * 0: No force
 * x1x: Force one CCM miss count increment for that type
 *
 * Field: ::VTSS_VOP_COMMON_LOC_CTRL . LOC_FORCE_HW_SCAN_ENA
 */
#define  VTSS_F_VOP_COMMON_LOC_CTRL_LOC_FORCE_HW_SCAN_ENA(x)  VTSS_ENCODE_BITFIELD(x,8,7)
#define  VTSS_M_VOP_COMMON_LOC_CTRL_LOC_FORCE_HW_SCAN_ENA     VTSS_ENCODE_BITMASK(8,7)
#define  VTSS_X_VOP_COMMON_LOC_CTRL_LOC_FORCE_HW_SCAN_ENA(x)  VTSS_EXTRACT_BITFIELD(x,8,7)


/** 
 * \brief CCM SCAN Diagnostic
 *
 * \details
 * Bits in this register indicate the current status of the LOC scanning.
 *
 * Register: \a VOP:COMMON:LOC_SCAN_STICKY
 */
#define VTSS_VOP_COMMON_LOC_SCAN_STICKY      VTSS_IOREG(VTSS_TO_VOP,0x10d61)

/** 
 * \brief
 * Reflects the current LOC scan mask.
 * 
 * A bit is asserted for each of the LOC counters currently being
 * increased.
 * 
 * NOTE: This is not a sticky bit.
 *
 * \details 
 * 0: No event has occured
 * 1: CCM Miss count scan ongoing. A bit is asserted for each LOC counter
 * which is updating the LOC MISS CNT.
 *
 * Field: ::VTSS_VOP_COMMON_LOC_SCAN_STICKY . LOC_SCAN_ONGOING_STATUS
 */
#define  VTSS_F_VOP_COMMON_LOC_SCAN_STICKY_LOC_SCAN_ONGOING_STATUS(x)  VTSS_ENCODE_BITFIELD(x,24,7)
#define  VTSS_M_VOP_COMMON_LOC_SCAN_STICKY_LOC_SCAN_ONGOING_STATUS     VTSS_ENCODE_BITMASK(24,7)
#define  VTSS_X_VOP_COMMON_LOC_SCAN_STICKY_LOC_SCAN_ONGOING_STATUS(x)  VTSS_EXTRACT_BITFIELD(x,24,7)

/** 
 * \brief
 * Set when CCM_SCAN completes.
 *
 * \details 
 * 0: No event has occured
 * 1: CCM miss count scan completed
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_VOP_COMMON_LOC_SCAN_STICKY . LOC_SCAN_COMPLETED_STICKY
 */
#define  VTSS_F_VOP_COMMON_LOC_SCAN_STICKY_LOC_SCAN_COMPLETED_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Asserted by VOP when LOC_SCAN starts.
 *
 * \details 
 * 0: No event has occured
 * 1: LOC miss count scan started
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_VOP_COMMON_LOC_SCAN_STICKY . LOC_SCAN_STARTED_STICKY
 */
#define  VTSS_F_VOP_COMMON_LOC_SCAN_STICKY_LOC_SCAN_STARTED_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Set when a LOC scan could not start because a scan is already ongoing. 
 * 
 * This is an indication that a LOC timerout occurred before the previous
 * scan scheduled by the same LOC timer was initiated.
 * 
 * The configured Period time the LOC scan controller must be incremented.
 *
 * \details 
 * 0: No event has occured
 * 1: Scan could not start in time
 * 
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_VOP_COMMON_LOC_SCAN_STICKY . LOC_SCAN_START_DELAYED_STICKY
 */
#define  VTSS_F_VOP_COMMON_LOC_SCAN_STICKY_LOC_SCAN_START_DELAYED_STICKY  VTSS_BIT(0)


/** 
 * \brief OAM_MEP interrupt control.
 *
 * \details
 * This is the combined interrupt output from the VOP.
 * 
 * To determine the VOE source of the interrupt, read register : INTR
 *
 * Register: \a VOP:COMMON:MASTER_INTR_CTRL
 */
#define VTSS_VOP_COMMON_MASTER_INTR_CTRL     VTSS_IOREG(VTSS_TO_VOP,0x10d62)

/** 
 * \brief
 * Status of interrupt to CPU
 * 
 * To enable interrupt of this status set the corresponding *_INTR_ENA
 *
 * \details 
 * 0: VOP Interrupt is de-asserted.
 * 1: VOP Interrupt is asserted.
 *
 * Field: ::VTSS_VOP_COMMON_MASTER_INTR_CTRL . OAM_MEP_INTR
 */
#define  VTSS_F_VOP_COMMON_MASTER_INTR_CTRL_OAM_MEP_INTR  VTSS_BIT(8)

/** 
 * \brief
 * Configures if OAM_MEP_INTR causes CPU interrupts
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_VOP_COMMON_MASTER_INTR_CTRL . OAM_MEP_INTR_ENA
 */
#define  VTSS_F_VOP_COMMON_MASTER_INTR_CTRL_OAM_MEP_INTR_ENA  VTSS_BIT(25)


/** 
 * \brief OAM_MEP VOE interrupt
 *
 * \details
 * This register contains the interrupt for each individual VOE.
 * 
 * The interrupts are numbered according to the VOEs:
 * 
 * 0 - 1023 are the Service / Path VOEs
 * 1024 - 1076 are the Port VOEs
 * 
 * The VOE interrupts are enabled in the following register:
 * 
 *  - VOE_STAT:INTR_ENA
 *
 * Register: \a VOP:COMMON:INTR
 *
 * @param ri Replicator: x_NUM_TOTAL_VOE_DIV32_CEIL (??), 0-33
 */
#define VTSS_VOP_COMMON_INTR(ri)             VTSS_IOREG(VTSS_TO_VOP,0x10d63 + (ri))


/** 
 * \brief OAM MEP Multicast MAC address configuration (LSB)
 *
 * \details
 * Each VOE can be addressed using either a common Multicast MAC address or
 * a VOE specific Unicast MAC address.
 * 
 * This register configures the Multicast Address common to all the VOEs.
 * 
 * The full MAC address is a concatenation of the folliowing registers:
 * 1) COMMON:COMMON_MEP_MC_MAC_LSB
 * 2) COMMON:COMMON_MEP_MC_MAC_MSB
 * 
 * The default value of this register is determined by 802.1ag.
 * 
 * The DMAC check to be performed for each VOE is configured in the
 * following bit field:
 * VOE:BASIC_CTRL.RX_DMAC_CHK_SEL
 * 
 * Note that only the upper 44 bits are matched, since the lower 4 bits of
 * the DMAC address contain the MEG level.
 *
 * Register: \a VOP:COMMON:COMMON_MEP_MC_MAC_LSB
 */
#define VTSS_VOP_COMMON_COMMON_MEP_MC_MAC_LSB  VTSS_IOREG(VTSS_TO_VOP,0x10d85)

/** 
 * \brief
 * See register decription.
 *
 * \details 
 * Field: ::VTSS_VOP_COMMON_COMMON_MEP_MC_MAC_LSB . MEP_MC_MAC_LSB
 */
#define  VTSS_F_VOP_COMMON_COMMON_MEP_MC_MAC_LSB_MEP_MC_MAC_LSB(x)  VTSS_ENCODE_BITFIELD(x,3,28)
#define  VTSS_M_VOP_COMMON_COMMON_MEP_MC_MAC_LSB_MEP_MC_MAC_LSB     VTSS_ENCODE_BITMASK(3,28)
#define  VTSS_X_VOP_COMMON_COMMON_MEP_MC_MAC_LSB_MEP_MC_MAC_LSB(x)  VTSS_EXTRACT_BITFIELD(x,3,28)


/** 
 * \brief OAM MEP Multicast MAC address configuration (MSB)
 *
 * \details
 * Each VOE can be addressed using either a common Multicast MAC address or
 * a VOE specific Unicast MAC address.
 * 
 * This register configures the Multicast Address common to all the VOEs.
 * 
 * The full MAC address is a concatenation of the folliowing registers:
 * 1) COMMON:COMMON_MEP_MC_MAC_LSB
 * 2) COMMON:COMMON_MEP_MC_MAC_MSB
 * 
 * The default value of this register is determined by 802.1ag.
 * 
 * The DMAC check to be performed for each VOE is configured in the
 * following bit field:
 * VOE:BASIC_CTRL.RX_DMAC_CHK_SEL
 * 
 * Note that only the upper 44 bits are matched, since the lower 4 bits of
 * the DMAC address contain the MEG level.
 *
 * Register: \a VOP:COMMON:COMMON_MEP_MC_MAC_MSB
 */
#define VTSS_VOP_COMMON_COMMON_MEP_MC_MAC_MSB  VTSS_IOREG(VTSS_TO_VOP,0x10d86)

/** 
 * \brief
 * See register description.
 *
 * \details 
 * Field: ::VTSS_VOP_COMMON_COMMON_MEP_MC_MAC_MSB . MEP_MC_MAC_MSB
 */
#define  VTSS_F_VOP_COMMON_COMMON_MEP_MC_MAC_MSB_MEP_MC_MAC_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VOP_COMMON_COMMON_MEP_MC_MAC_MSB_MEP_MC_MAC_MSB     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VOP_COMMON_COMMON_MEP_MC_MAC_MSB_MEP_MC_MAC_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VOP:VOE_CONF_REG
 *
 * Not documented
 */


/** 
 * \details
 * Register: \a VOP:VOE_CONF_REG:VOE_MISC_CONFIG
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_CONF_REG_VOE_MISC_CONFIG(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x13800,gi,1,0,0)

/** 
 * \brief
 * The VOE HW will process either Y.1731 or MPLS OAM PDUs depending on the
 * configuration of this register.
 * 
 * To enable a specific VOE for MPLS OAM processing, the corresponding bit
 * in this register must be asserted.
 *
 * \details 
 * '0': VOE is configured to process Y.1731 OAM PDUs
 * '1': VOE is configured to process MPLS-TP OAM PDUs
 *
 * Field: ::VTSS_VOP_VOE_CONF_REG_VOE_MISC_CONFIG . MPLS_OAM_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_REG_VOE_MISC_CONFIG_MPLS_OAM_ENA  VTSS_BIT(2)

/** 
 * \brief
 * If this field is asserted, the VOE will count bytes instead of frames.
 * 
 * This is not 100% supported and tested.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_REG_VOE_MISC_CONFIG . LM_CNT_BYTE
 */
#define  VTSS_F_VOP_VOE_CONF_REG_VOE_MISC_CONFIG_LM_CNT_BYTE  VTSS_BIT(1)

/** 
 * \brief
 * Enable the VOE for Synthetic Loss Measurements.
 * 
 * If enabled, the normal LM counters are used differently than when
 * running standard frame loss measurements.
 * 
 * The RX counters are used to count SLR/SL1 frames received from different
 * Peer MEPs.
 * The TX counters are used to count SLR/SL1 frames transmitted to
 * different Peer MEPs.
 * 
 * Note that there is no counting of data frames or other NON SL OAM PDUs.
 * 
 * Asserting this register will avoid any other VOEs from updating the LM
 * counters of this VOE as part of a hierarchical LM counter update.

 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_REG_VOE_MISC_CONFIG . SL_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_REG_VOE_MISC_CONFIG_SL_ENA  VTSS_BIT(0)

/**
 * Register Group: \a VOP:VOE_CONF
 *
 * Configuration per Vitesse OAM MEP Endpoints (VOE) for Y.1731 OAM
 */


/** 
 * \brief Misc. VOE control configuration
 *
 * \details
 * This register includes configuration of misc. VOE control properties.
 *
 * Register: \a VOP:VOE_CONF:VOE_CTRL
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_CONF_VOE_CTRL(gi)       VTSS_IOREG_IX(VTSS_TO_VOP,0x0,gi,64,0,0)

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
 * 0: Disable VOE
 * 1: Enable VOE for MEP or MIP processing of OAM PDUs.
 *
 * Field: ::VTSS_VOP_VOE_CONF_VOE_CTRL . VOE_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_VOE_CTRL_VOE_ENA  VTSS_BIT(13)

/** 
 * \brief
 * This register configures the MEL value for the VOE.
 * 
 * PDUs passing the VOE with MEL above this MEL_VAL will be treated as
 * data.
 * 
 * PDUs passing the VOEwith MEL below this MEL_VAL will be discarded.
 * 
 * PDUs passing the VOE with MEL equal to this MEL_VAL will optional
 * handled by HW / copied to CPU. 
 * 
 * The processing of OAM PDUs with a MEL value equal to the VOE MEL_VAL
 * depends on the following:
 * 
 *  * If the PDU was inserted by the CPU
 *  * If the PDU type is enabled for processing by the VOE
 *  * Whether the VOE is operating in Centralized OAM Mode
 *  * If the PDU is received on the Active or Passive side of the VOE.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_VOE_CTRL . MEL_VAL
 */
#define  VTSS_F_VOP_VOE_CONF_VOE_CTRL_MEL_VAL(x)  VTSS_ENCODE_BITFIELD(x,10,3)
#define  VTSS_M_VOP_VOE_CONF_VOE_CTRL_MEL_VAL     VTSS_ENCODE_BITMASK(10,3)
#define  VTSS_X_VOP_VOE_CONF_VOE_CTRL_MEL_VAL(x)  VTSS_EXTRACT_BITFIELD(x,10,3)

/** 
 * \brief
 * Configures VOE for Down-MEP or Up-MEP functionality.
 * 
 * NOTE:
 * Port VOE may NOT be configured for Up-MEP functionality, they only
 * support Down-MEP implementation.
 *
 * \details 
 * 0: Configure VOE for Down-MEP functionality.
 * 1: Configure VOE for Up-MEP functionality.
 *
 * Field: ::VTSS_VOP_VOE_CONF_VOE_CTRL . UPMEP_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_VOE_CTRL_UPMEP_ENA  VTSS_BIT(9)

/** 
 * \brief
 * If another VOE is pointing to this VOE as a PATH VOE using the following
 * configuration:
 * 
 *  - VOE_CONF:PATH_VOE_CFG.PATH.VOEID
 *  - VOE_CONF:PATH_VOE_CFG.PATH.VOE_ENA
 * 
 * this register MUST be set to '1'.
 * 
 * If not this register must be set to '0'.
 *
 * \details 
 * '0': This VOE is not configured as PATH VOE in another VOE.
 * '1': This VOE is configured as PATH VOE in another VOE.

 *
 * Field: ::VTSS_VOP_VOE_CONF_VOE_CTRL . VOE_IS_PATH
 */
#define  VTSS_F_VOP_VOE_CONF_VOE_CTRL_VOE_IS_PATH  VTSS_BIT(8)

/** 
 * \brief
 * VOE configured as MEP:
 * ----------------------------------------
 * Configures the DMAC check performed when received from the peer MEP.
 * 
 * This check can be disabled for Generic OpCodes. 
 * See COMMON:OAM_GENERIC_CFG.GENERIC_DMAC_CHK_DIS
 * 
 * In case of MPLS-TP encapsulated frames, the check is performed on the
 * Customer DMAC (Inner DMAC)
 * 
 * If DMAC check fails, the frame will be discarded.
 * 
 * The following sticky bit is asserted when the VOE receives a OAM PDU
 * which fails the DMAC check:
 * 
 * VOE_STAT::OAM_RX_STICKY.DMAC_RX_ERR_STICKY
 * 
 * Note that the RX check configured in this register can be bypassed for
 * GENERIC OPCODES depending on the configuration of the following
 * register:
 * 
 *  - COMMON:OAM_GENERIC_CFG.GENERIC_DMAC_CHK_DIS
 * 
 * VOE configured as MIP:
 * ----------------------------------------
 * When the VOE is configured for MIP operation, the two bits of this bit
 * group have independent functionality.
 * 
 * RX_DMAC_CHK_SEL(0) : 
 * 0: The DMAC of incoming frames is verified against the VOE unicast MAC
 * address.
 * If there is a match, the LBM PDU is returned as a LBR.
 * If there is no match, the MIP will be transparent to the LBM PDU.
 * 
 * 1: No DMAC check is done for LBM frames. All LBM frames are looped.
 * 
 * RX_DMAC_CHK_SEL(1) : 
 * 0: The DMAC of incoming LTM frames is verified against the VOP multicast
 * MAC address. 
 * 
 * 1: No DMAC check is done for LTM frames.
 * 
 * If there is match the LTM can optionally be discarded.
 * If there is no match, the LTM is discarded. It can optionally be
 * extracted to the CPU ERR queue using the DMAC error extraction.
 *
 * \details 
 * VOE configured as MEP:
 * 0: No DMAC check
 * 1: Check DMAC against MEP_UC_MAC_MSB and MEP_UC_MAC_LSB
 * 2: Check DMAC against MEP_MC_MAC_MSB and MEP_MC_MAC_LSB
 * 3: Check DMAC against either MEP_UC_MAC_MSB and MEP_UC_MAC_LSB or
 * MEP_MC_MAC_MSB and MEP_MC_MAC_LSB
 *
 * Field: ::VTSS_VOP_VOE_CONF_VOE_CTRL . RX_DMAC_CHK_SEL
 */
#define  VTSS_F_VOP_VOE_CONF_VOE_CTRL_RX_DMAC_CHK_SEL(x)  VTSS_ENCODE_BITFIELD(x,6,2)
#define  VTSS_M_VOP_VOE_CONF_VOE_CTRL_RX_DMAC_CHK_SEL     VTSS_ENCODE_BITMASK(6,2)
#define  VTSS_X_VOP_VOE_CONF_VOE_CTRL_RX_DMAC_CHK_SEL(x)  VTSS_EXTRACT_BITFIELD(x,6,2)

/** 
 * \brief
 * On reception of an OAM PDU in the RX direction, the VOE will verify that
 * the Y.1731 version number in the OAM PDU header is = 0.
 * 
 * The VERSION is verified for all but GENERIC and UNKNOWN PDUs, since
 * these are not processed in HW.
 * 
 * This bit field can disable the VERSION verification as part of the RX
 * verification.
 * 
 * If the VERSION verification fails, the following sticky bit is asserted:
 * 
 *  - VOE_STAT:OAM_RX_STICKY.PDU_VERSION_RX_ERR_STICKY
 * 
 * OAM PDUs which fail the version check can optionally be extracted using
 * the following configuration bit:
 * 
 *  - VOE_STAT:PDU_EXTRACT.PDU_VERSION_RX_ERR_EXTR
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_VOE_CTRL . VERIFY_VERSION_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_VOE_CTRL_VERIFY_VERSION_ENA  VTSS_BIT(5)

/** 
 * \brief
 * When asserted, the VOE will assume that an external device updates the
 * Timestamp information in Y.1731 Delay Measurement PDUs.
 * Hence the VOE will not update the DM PDUs with TS information.
 * 
 * The VOE will however, still extract and loop DM PDUs according to VOE
 * configuration.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_VOE_CTRL . EXTERN_DM_TSTAMP
 */
#define  VTSS_F_VOP_VOE_CONF_VOE_CTRL_EXTERN_DM_TSTAMP  VTSS_BIT(4)

/** 
 * \brief
 * When asserted, the VOE will block Y.1731 PDUs with MEL higher than the
 * configured MEP MEL in the RX direction.
 * 
 * This can be used to terminate higher MEL frames when the VOE is located
 * at the border of a MEL domain.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_VOE_CTRL . BLOCK_MEL_HIGH_RX
 */
#define  VTSS_F_VOP_VOE_CONF_VOE_CTRL_BLOCK_MEL_HIGH_RX  VTSS_BIT(3)

/** 
 * \brief
 * Asserting this bit will block all RX frames not processed or blocked by
 * MEL filtering in the VOE.
 * 
 * This blocking will not interfer with OAM PDU's processed or MEL filtered
 * by the VOE, but it will block all service frames in the RX direction.
 * 
 * The following frames will be blocked:
 *  * OAM PDU's with MEL_HIGH
 *  * Data frames
 * 
 * Frames discarded by this blocking will counted in the following counter:
 * 
 *  * VOP:VOE_STAT:RX_OAM_DISCARD.RX_OAM_DISCARD_CNT
 * 
 * Frames blocked by this functionality will be counted as part of the LM
 * RX counters.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_VOE_CTRL . BLOCK_DATA_RX
 */
#define  VTSS_F_VOP_VOE_CONF_VOE_CTRL_BLOCK_DATA_RX  VTSS_BIT(2)

/** 
 * \brief
 * Asserting this bit will block all TX frames not blocked by MEL filtering
 * in the VOE. It will not affect PDU's injected into the VOE.
 * 
 * This blocking will not interfer with OAM PDU's processed or MEL filtered
 * by the VOE, but it will block all service frames in the TX direction.
 * 
 * The following frames will be blocked:
 *  * OAM PDU's with MEL_HIGH
 *  * Data frames
 * 
 * Frames discarded by this blocking will counted in the following counter:
 * 
 *  * VOP:VOE_STAT:TX_OAM_DISCARD.TX_OAM_DISCARD_CNT
 * 
 * Frames blocked by this functionality will not be counted as part of the
 * LM TX counters.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_VOE_CTRL . BLOCK_DATA_TX
 */
#define  VTSS_F_VOP_VOE_CONF_VOE_CTRL_BLOCK_DATA_TX  VTSS_BIT(1)

/** 
 * \brief
 * Configure the current VOE to be used as SAT_TEST_VOE.
 * 
 * When the VOE is used as SAT test VOE, the OAM frames being processed by
 * the VOE must be counted in the egress statistics, which is not the
 * default VOE behavior.
 *
 * \details 
 * 0: Configure as standard VOE
 * 1: Configure as SAT test VOE
 *
 * Field: ::VTSS_VOP_VOE_CONF_VOE_CTRL . SAT_TEST_VOE
 */
#define  VTSS_F_VOP_VOE_CONF_VOE_CTRL_SAT_TEST_VOE  VTSS_BIT(0)


/** 
 * \brief SAM per COSID sequence numbering
 *
 * \details
 * The VOP includes 32 counter sets which can be used for SAM per COSID
 * sequence numbering of the following PDU types:
 * 
 *  - CCM
 *  - LBM/LBR
 *  - TST
 * 
 * This register is used to configure the VOE for per COSID sequence
 * numbering by assigning one of the SAM per COSID counter sets to the VOE.
 * 
 * The per COSID sequence numbering is implemented by using the
 * corresponding register in the VOE to count priority 0, while the
 * remaining priorities (1-7) are counted using a dedicated RAM.
 * 
 * The SAM per COSID counters (prio: 1 - 7) are located in:
 * VOP::SAM_COSID_SEQ_CNT
 * 
 * NOTE: The appointed per COSID counter set can be used for either
 * LBM/LBR/CCM or CCM, hence only one of the below registers may be
 * asserted.
 * 
 *  - PER_COSID_LBM
 *  - PER_COSID_CCM
 * 
 * Asserting both is a misconfiguration.
 * 
 * When per COSID sequence numbering is enabled, the VOE will use the
 * counter set configured in :
 * 
 *  - PER_COSID_CNT_SET
 *
 * Register: \a VOP:VOE_CONF:SAM_COSID_SEQ_CFG
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_CONF_SAM_COSID_SEQ_CFG(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x0,gi,64,0,1)

/** 
 * \brief
 * Enable SAM per COSID sequence numbering for the following PDUs
 *  - TST
 *  - LBM/LBR
 *  - non OAM sequence numbering (see VOE_CONF:SAM_NON_OAM_SEQ_CFG.*)
 * 
 * Note that the above PDUs are mutually exclusive.
 * 
 * This bit field MUST not be asserted at the same time as: PER_COSID_CCM
 * 
 * When SAM per COSID sequence numbering is enabled, the VOE will use the
 * SAM counter set configured in :
 * 
 *  - PER_COSID_CNT_SET
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_SAM_COSID_SEQ_CFG . PER_COSID_LBM
 */
#define  VTSS_F_VOP_VOE_CONF_SAM_COSID_SEQ_CFG_PER_COSID_LBM  VTSS_BIT(6)

/** 
 * \brief
 * Enable SAM per COSID sequence numbering for the following PDUs
 *  - CCM(-LM)
 * 
 * This bit field MUST not be asserted at the same time as: PER_COSID_LBM
 * 
 * When SAM per COSID sequence numbering is enabled, the VOE will use the
 * SAM counter set configured in :
 * 
 *  - PER_COSID_CNT_SET
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_SAM_COSID_SEQ_CFG . PER_COSID_CCM
 */
#define  VTSS_F_VOP_VOE_CONF_SAM_COSID_SEQ_CFG_PER_COSID_CCM  VTSS_BIT(5)

/** 
 * \brief
 * When per COSID sequence numbering is enabled by asserting one of the
 * following bitfields, this register selectes which of the per COSID
 * counter sets will be used for per COSID statistics.
 * 
 * Per COSID sequence numbering is enabled by asserting one of the
 * following bit fields:
 *  - PER_COSID_LBM
 *  - PER_COSID_CCM

 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_SAM_COSID_SEQ_CFG . PER_COSID_CNT_SET
 */
#define  VTSS_F_VOP_VOE_CONF_SAM_COSID_SEQ_CFG_PER_COSID_CNT_SET(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_VOP_VOE_CONF_SAM_COSID_SEQ_CFG_PER_COSID_CNT_SET     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_VOP_VOE_CONF_SAM_COSID_SEQ_CFG_PER_COSID_CNT_SET(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief Support for SAM sequence numbering of non OAM frames.
 *
 * \details
 * The VOE can be configured to support sequence numbering of non OAM
 * frames.
 * This can be used for testing as specified in SAM or RFC2544 etc.
 * 
 * Note that the configuring support for sequence numbering of non OAM
 * frames excludes the use of the following PDU types for this VOE:
 *  - TST
 *  - LBM
 *  - LBR
 * 
 * This is because the statistics used to support non OAM sequence
 * numbering re-uses the registers otherwise used for processing the above
 * PDUs.
 * 
 * This functionality is referred to as SAM_SEQ.
 *
 * Register: \a VOP:VOE_CONF:SAM_NON_OAM_SEQ_CFG
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_CONF_SAM_NON_OAM_SEQ_CFG(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x0,gi,64,0,2)

/** 
 * \brief
 * Asserting this bit will configure the VOE as initiator of non OAM frames
 * with sequence number.
 * 
 * This must NOT be asserted at the same time as asserting:
 *  - SAM_SEQ_RECV = 1
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_SAM_NON_OAM_SEQ_CFG . SAM_SEQ_INIT
 */
#define  VTSS_F_VOP_VOE_CONF_SAM_NON_OAM_SEQ_CFG_SAM_SEQ_INIT  VTSS_BIT(9)

/** 
 * \brief
 * Asserting this bit will configure the VOE as SAM_SEQ Responder of non
 * OAM frames with sequence number.
 * 
 * This must NOT be asserted at the same time as asserting:
 *  - SAM_SEQ_INIT = 1
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_SAM_NON_OAM_SEQ_CFG . SAM_SEQ_RESP
 */
#define  VTSS_F_VOP_VOE_CONF_SAM_NON_OAM_SEQ_CFG_SAM_SEQ_RESP  VTSS_BIT(8)

/** 
 * \brief
 * This configures the 16-bit offset to the sequence number to be updates
 * with the non OAM frame.
 * 
 * For UDP frames this field MUST be programmed a value no less than 8
 * bytes (register value =4).
 * (To avoid overwriting the UDP protocol information in the frame).
 * 
 * The valid values are:
 * 
 * ETH: 1 - 33
 * IPv4/IPv6: 4 - 33
 * 
 * However, the sequence number in the PDU MUST be located within the first
 * CELL of the frame on the cell bus (JR2 cell size=176 bytes, incl. 28
 * bytes of IFH).
 * 
 * This implies that in case of a long encapsulation (e.g. IPv6 over ETH
 * over MPLS over ETH) there will be an upper limit to the valid value of
 * the offset value.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_SAM_NON_OAM_SEQ_CFG . SAM_SEQ_OFFSET_SEQ_NO
 */
#define  VTSS_F_VOP_VOE_CONF_SAM_NON_OAM_SEQ_CFG_SAM_SEQ_OFFSET_SEQ_NO(x)  VTSS_ENCODE_BITFIELD(x,2,6)
#define  VTSS_M_VOP_VOE_CONF_SAM_NON_OAM_SEQ_CFG_SAM_SEQ_OFFSET_SEQ_NO     VTSS_ENCODE_BITMASK(2,6)
#define  VTSS_X_VOP_VOE_CONF_SAM_NON_OAM_SEQ_CFG_SAM_SEQ_OFFSET_SEQ_NO(x)  VTSS_EXTRACT_BITFIELD(x,2,6)

/** 
 * \brief
 * If the non OAM frames are IP/UDP frames, the VOE must update not only
 * the sequence number but also the UDP checksum correction field, to avoid
 * altering the UDP checksum.
 * 
 * For this to happen, this register must be asserted.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_SAM_NON_OAM_SEQ_CFG . SAM_SEQ_UPD_CHKSUM
 */
#define  VTSS_F_VOP_VOE_CONF_SAM_NON_OAM_SEQ_CFG_SAM_SEQ_UPD_CHKSUM  VTSS_BIT(1)

/** 
 * \brief
 * This bitfield is only used if :
 * 
 *  - SAM_SEQ_INIT = 1
 * 
 * When the Initiator function receives non OAM frames with sequence
 * numbers, it can count either: 
 * 
 * 1) Number of frames received with FORWARD-SEQ-NUMBER-ERROR = 1
 * 2) Number of frames received where the sequence number does not match
 * the sequence number received in the previous frame + 1.
 * 
 * Frames 1) are always counted.
 * 
 * If SAM_SEQ_RX_ERR_CNT_ENA = 1 also frames 2) will be counted.
 * 
 * The configuration of this bit also determines which frames frames will
 * assert the sticky bit: 
 * 
 *  - VOE_STAT:OAM_RX_STICKY.LBR_TRANSID_ERR_STICKY
 * 
 * and be extracted to the CPU based on the following configuration:
 * 
 *  - VOE_STAT:PDU_EXTRACT.SAM_RX_SEQ_ERR_EXTR

 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_SAM_NON_OAM_SEQ_CFG . SAM_SEQ_RX_ERR_CNT_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_SAM_NON_OAM_SEQ_CFG_SAM_SEQ_RX_ERR_CNT_ENA  VTSS_BIT(0)


/** 
 * \brief Path MEP configuration
 *
 * \details
 * This register is used to assign the current service VOE to a Path VOE.
 * 
 * Assigning a Service VOE to a Path VOE implies that all frames received
 * by this VOE, will also be counted by the Path VOE indicated by the
 * following registers depending on the proct status of this Service VOE:
 * 
 *  * PATH_VOEID
 *  * PATH_VOEID_P
 * 
 * The path VOE must be enabled by asserting the following field: 
 * 
 *  * PATH_VOE_ENA
 *
 * Register: \a VOP:VOE_CONF:PATH_VOE_CFG
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_CONF_PATH_VOE_CFG(gi)   VTSS_IOREG_IX(VTSS_TO_VOP,0x0,gi,64,0,3)

/** 
 * \brief
 * Configures if a service VOE is part of a path VOE.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_PATH_VOE_CFG . PATH_VOE_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_PATH_VOE_CFG_PATH_VOE_ENA  VTSS_BIT(10)

/** 
 * \brief
 * Configures the associated WORK Path VOE
 * 
 * Must be enabled by : PATH_VOE_ENA = 1
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_PATH_VOE_CFG . PATH_VOEID
 */
#define  VTSS_F_VOP_VOE_CONF_PATH_VOE_CFG_PATH_VOEID(x)  VTSS_ENCODE_BITFIELD(x,0,10)
#define  VTSS_M_VOP_VOE_CONF_PATH_VOE_CFG_PATH_VOEID     VTSS_ENCODE_BITMASK(0,10)
#define  VTSS_X_VOP_VOE_CONF_PATH_VOE_CFG_PATH_VOEID(x)  VTSS_EXTRACT_BITFIELD(x,0,10)


/** 
 * \brief CPU extraction for the supported OAM PDU opcodes.
 *
 * \details
 * Configures CPU copy for the supported OAM PDU opcodes. This CPU copy
 * functionality operates independent of CENTRAL_OAM_CTRL.
 * 
 * Configuring a PDU type for CPU extraction, will result in all valid OAM
 * PDUs of this type to extracted to the CPU.
 * 
 * In-valid OAM PDUs are not extracted. OAM PDUs are considered invalid if
 * they fail either of the following checks:
 * 
 *  * MEL check (Must match the VOE)
 *  * MAC check
 *  * CCM validation (CCM/CCM-LM frames only)
 *  * LM counters are updated.
 *  * SEL / non SEL counters are not updated when the PDU type is not
 * enabled.
 *
 * Register: \a VOP:VOE_CONF:OAM_CPU_COPY_CTRL
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_CONF_OAM_CPU_COPY_CTRL(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x0,gi,64,0,4)

/** 
 * \brief
 * This bit field contains 8 bits each of which represent one of the
 * Generic Opcodes.
 * 
 * If the bit representing a specific generic opcode is asserted, all valid
 * PDUs received by the VOE of that type are extracted to the CPU queue
 * configured in the following field:
 * 
 *  - OAM_MEP:COMMON:OAM_GENERIC_CFG.GENERIC_OPCODE_CPU_QU
 *
 * \details 
 * 0: No CPU copy
 * x1x: Copy to CPU
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CPU_COPY_CTRL . GENERIC_COPY_MASK
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CPU_COPY_CTRL_GENERIC_COPY_MASK(x)  VTSS_ENCODE_BITFIELD(x,14,8)
#define  VTSS_M_VOP_VOE_CONF_OAM_CPU_COPY_CTRL_GENERIC_COPY_MASK     VTSS_ENCODE_BITMASK(14,8)
#define  VTSS_X_VOP_VOE_CONF_OAM_CPU_COPY_CTRL_GENERIC_COPY_MASK(x)  VTSS_EXTRACT_BITFIELD(x,14,8)

/** 
 * \brief
 * Configures whether OAM PDUs with UNKNOWN opcode should be extracted to
 * the CPU.
 * 
 * Extracted frames are extracted to the default CPU queue, configured in:
 * 
 *  - CPU_EXTR_CFG.DEF_COPY_QU
 *
 * \details 
 * '0': No CPU copy
 * '1': Copy to CPU
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CPU_COPY_CTRL . UNK_OPCODE_CPU_COPY_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CPU_COPY_CTRL_UNK_OPCODE_CPU_COPY_ENA  VTSS_BIT(13)

/** 
 * \brief
 * If asserted all valid LTM PDUs with MEL = VOE_MEL received by the VOE
 * are extracted to the CPU.
 * 
 * Extraction queue is determined by:
 * 
 *  - CPU_EXTR_CFG_1.LT_CPU_QU
 *
 * \details 
 * '0': No extraction to CPU
 * '1': Extract valid LTM frames to CPU
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CPU_COPY_CTRL . LTM_CPU_COPY_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CPU_COPY_CTRL_LTM_CPU_COPY_ENA  VTSS_BIT(12)

/** 
 * \brief
 * If asserted all valid LTR PDUs with MEL = VOE_MEL received by the VOE
 * are extracted to the CPU.
 * 
 * Extraction queue is determined by:
 * 
 *  - CPU_EXTR_CFG_1.LT_CPU_QU
 *
 * \details 
 * '0': No extraction to CPU
 * '1': Extract valid LTR frames to CPU
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CPU_COPY_CTRL . LTR_CPU_COPY_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CPU_COPY_CTRL_LTR_CPU_COPY_ENA  VTSS_BIT(11)

/** 
 * \brief
 * If asserted all valid LMM/SLM PDUs with MEL = VOE_MEL received by the
 * VOE are extracted to the CPU.
 * 
 * Extraction queue is determined by:
 * 
 *  - CPU_EXTR_CFG.LMM_CPU_QU
 *
 * \details 
 * '0': No extraction to CPU
 * '1': Extract valid LMM frames to CPU
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CPU_COPY_CTRL . LMM_CPU_COPY_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CPU_COPY_CTRL_LMM_CPU_COPY_ENA  VTSS_BIT(10)

/** 
 * \brief
 * If asserted all valid LMR / SLR / 1SL  PDUs with MEL = VOE_MEL received
 * by the VOE are extracted to the CPU.
 * 
 * Extraction queue is determined by:
 * 
 *  - CPU_EXTR_CFG.LMR_CPU_QU
 *
 * \details 
 * '0': No extraction to CPU
 * '1': Extract valid LMR frames to CPU
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CPU_COPY_CTRL . LMR_CPU_COPY_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CPU_COPY_CTRL_LMR_CPU_COPY_ENA  VTSS_BIT(9)

/** 
 * \brief
 * If asserted all valid TST PDUs with MEL = VOE_MEL received by the VOE
 * are extracted to the CPU.
 * 
 * Extraction queue is determined by:
 * 
 *  - CPU_EXTR_CFG_1.TST_CPU_QU
 *
 * \details 
 * '0': No extraction to CPU
 * '1': Extract valid TST frames to CPU
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CPU_COPY_CTRL . TST_CPU_COPY_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CPU_COPY_CTRL_TST_CPU_COPY_ENA  VTSS_BIT(8)

/** 
 * \brief
 * If asserted all valid LBM PDUs with MEL = VOE_MEL received by the VOE
 * are extracted to the CPU.
 * 
 * Extraction queue is determined by:
 * 
 *  - CPU_EXTR_CFG_1.LBM_CPU_QU
 *
 * \details 
 * '0': No extraction to CPU
 * '1': Extract valid LBM frames to CPU
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CPU_COPY_CTRL . LBM_CPU_COPY_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CPU_COPY_CTRL_LBM_CPU_COPY_ENA  VTSS_BIT(7)

/** 
 * \brief
 * If asserted all valid LBR PDUs with MEL = VOE_MEL received by the VOE
 * are extracted to the CPU.
 * 
 * Extraction queue is determined by:
 * 
 *  - CPU_EXTR_CFG_1.LBR_CPU_QU
 * 
 * This setting is also used to control extraction of SAM_SEQ frames (see
 * VOE_CONF:SAM_NON_OAM_SEQ_CFG.*)
 *
 * \details 
 * '0': No extraction to CPU
 * '1': Extract valid LBR frames to CPU
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CPU_COPY_CTRL . LBR_CPU_COPY_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CPU_COPY_CTRL_LBR_CPU_COPY_ENA  VTSS_BIT(6)

/** 
 * \brief
 * If asserted all valid DMM PDUs with MEL = VOE_MEL received by the VOE
 * are extracted to the CPU.
 * 
 * Extraction queue is determined by:
 * 
 *  - CPU_EXTR_CFG.DMM_CPU_QU
 *
 * \details 
 * '0': No extraction to CPU
 * '1': Extract valid DMM frames to CPU
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CPU_COPY_CTRL . DMM_CPU_COPY_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CPU_COPY_CTRL_DMM_CPU_COPY_ENA  VTSS_BIT(5)

/** 
 * \brief
 * If asserted all valid DMR PDUs with MEL = VOE_MEL received by the VOE
 * are extracted to the CPU.
 * 
 * Extraction queue is determined by:
 * 
 *  - CPU_EXTR_CFG.DMR_CPU_QU
 *
 * \details 
 * '0': No extraction to CPU
 * '1': Extract valid DMR frames to CPU
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CPU_COPY_CTRL . DMR_CPU_COPY_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CPU_COPY_CTRL_DMR_CPU_COPY_ENA  VTSS_BIT(4)

/** 
 * \brief
 * If asserted all valid 1DM PDUs with MEL = VOE_MEL received by the VOE
 * are extracted to the CPU.
 * 
 * Extraction queue is determined by:
 * 
 *  - CPU_EXTR_CFG.DMR_CPU_QU
 * 
 * (This PDU type reuses the DMR extraction queue)
 *
 * \details 
 * '0': No extraction to CPU
 * '1': Extract valid 1DM frames to CPU
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CPU_COPY_CTRL . SDM_CPU_COPY_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CPU_COPY_CTRL_SDM_CPU_COPY_ENA  VTSS_BIT(3)

/** 
 * \brief
 * If asserted all valid CCM PDUs with MEL = VOE_MEL received by the VOE
 * are extracted to the CPU.
 * 
 * Extraction queue is determined by:
 * 
 *  - CPU_EXTR_CFG_1.CCM_CPU_QU
 *
 * \details 
 * '0': No extraction to CPU
 * '1': Extract valid CCM frames to CPU
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CPU_COPY_CTRL . CCM_CPU_COPY_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CPU_COPY_CTRL_CCM_CPU_COPY_ENA  VTSS_BIT(2)

/** 
 * \brief
 * If asserted all valid CCM_LM PDUs with MEL = VOE_MEL received by the VOE
 * are extracted to the CPU.
 * 
 * Extraction queue is determined by:
 * 
 *  - CPU_EXTR_CFG_1.CCM_LM_CPU_QU
 *
 * \details 
 * '0': No extraction to CPU
 * '1': Extract valid CCM_LM frames to CPU
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CPU_COPY_CTRL . CCM_LM_CPU_COPY_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CPU_COPY_CTRL_CCM_LM_CPU_COPY_ENA  VTSS_BIT(1)

/** 
 * \brief
 * The configuration of this bit field influences the following PDU error
 * verification:
 * 
 * RX:
 * ----------
 *  - RX_MEL_LOW
 *  - DMAC_RX_ERR
 *  - PDU_VERSION_RX_ERR
 * 
 * TX:
 * --------------
 *  - TX_BLOCK_ERR
 * 
 * The checks mentioned above will be done for all PDU types, however the
 * associated sticky bit assertion and extraction of PDUs will depend on
 * the setting of this register.
 * 
 * 0: For the above listed checks, sticky bits are asserted and PDU is
 * extracted, for all PDU types.
 * 
 * 1: For the above listed checks, sticky bits are asserted and PDU is
 * extracted, only for CCM(-LM) PDUs.

 *
 * \details 
 * 0: Assert sticky bit and extract PDU for all PDU types.
 * 1: Assert sticky bit and extract PDU only for CCM(-LM) frames.
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CPU_COPY_CTRL . PDU_ERR_EXTRACT_CCM_ONLY
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CPU_COPY_CTRL_PDU_ERR_EXTRACT_CCM_ONLY  VTSS_BIT(0)


/** 
 * \details
 * Register: \a VOP:VOE_CONF:PDU_VOE_PASS
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_CONF_PDU_VOE_PASS(gi)   VTSS_IOREG_IX(VTSS_TO_VOP,0x0,gi,64,0,5)

/** 
 * \brief
 * Each of the bits in the register represents a Generic OpCode.
 * 
 * See: OAM_MEP::COMMON:OAM_GENERIC_CFG.*
 * 
 * When asserting a bit in the register, the corresponding Generic OpCode
 * will be allowed to pass though VOE, rather than be terminated, when
 * received at the same MEL as is configured for the VOE.
 * 
 * This can be used to allow e.g. Ring PDUs to be copied and to pass
 * transparently through the VOE.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_PDU_VOE_PASS . GENERIC_VOE_PASS_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_PDU_VOE_PASS_GENERIC_VOE_PASS_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VOP_VOE_CONF_PDU_VOE_PASS_GENERIC_VOE_PASS_ENA     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VOP_VOE_CONF_PDU_VOE_PASS_GENERIC_VOE_PASS_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief Configuration which OAM PDU are to be included in selected Cnt
 *
 * \details
 * The OAM frames handeled by the VOE can be counted separately in RX and
 * TX direction.
 * In each direction there are two counters:
 * 
 * 1) Default OAM counter
 * This counter counts all the PDU types which are NOT selected using the
 * OAM_CNT_OAM_CTRL register:
 * 
 *  - RX_OAM_FRM_CNT
 *  - TX_OAM_FRM_CNT
 * 
 * 2) Selected OAM counter:
 * This counter counts all the PDU types selected for counting using the
 * OAM_CNT_OAM_CTRL register:
 * 
 *  - RX_SEL_OAM_CNT
 *  - TX_SEL_OAM_CNT
 * 
 * Any OAM PDU is counted in exactly one of the above registers.
 * 
 * I.e. as default all OAM PDUs are not selected, and they are all counted
 * in the default OAM counters : RX / TX _ OAM_FRM_CNT.
 * 
 * Using this register (OAM_CNT_OAM_CTRL), PDUs can be moved to the
 * selected coutners:
 * RX / TX SEL_OAM_CNT.
 * 
 * The selection of OAM PDUs for the selected counter is done commonly for
 * the TX and RX direction.
 *
 * Register: \a VOP:VOE_CONF:OAM_CNT_OAM_CTRL
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_CONF_OAM_CNT_OAM_CTRL(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x0,gi,64,0,6)

/** 
 * \brief
 * Enable / disable that valid OAM PDUs with Generic OPCODES are counted as
 * selected OAM.
 * 
 * This bit field contains a separate bit for each of the possible 8
 * Generic opcodes.
 *
 * \details 
 * x0x: Count as other OAM
 * x1x: Count as selected OAM
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_OAM_CTRL . GENERIC_OAM_CNT_MASK
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_OAM_CTRL_GENERIC_OAM_CNT_MASK(x)  VTSS_ENCODE_BITFIELD(x,13,8)
#define  VTSS_M_VOP_VOE_CONF_OAM_CNT_OAM_CTRL_GENERIC_OAM_CNT_MASK     VTSS_ENCODE_BITMASK(13,8)
#define  VTSS_X_VOP_VOE_CONF_OAM_CNT_OAM_CTRL_GENERIC_OAM_CNT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,13,8)

/** 
 * \brief
 * OAM PDUs not recognized as either on of the PDUs with special
 * configuration or as a generic PDU, will be classified as an UNKNOWN PDU.
 * 
 * This register configures whether UNKNOWN PDUs should be counted as
 * selected OAM.
 *
 * \details 
 * '0': Count as other OAM
 * '1': Count as selected OAM
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_OAM_CTRL . UNK_OPCODE_OAM_CNT_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_OAM_CTRL_UNK_OPCODE_OAM_CNT_ENA  VTSS_BIT(12)

/** 
 * \brief
 * Enable / disable count of OAM PDU LTM as selected OAM.
 *
 * \details 
 * '0': Count as other OAM
 * '1': Count as selected OAM
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_OAM_CTRL . LTM_OAM_CNT_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_OAM_CTRL_LTM_OAM_CNT_ENA  VTSS_BIT(11)

/** 
 * \brief
 * Enable / disable count of OAM PDU LTR as selected OAM.
 *
 * \details 
 * '0': Count as other OAM
 * '1': Count as selected OAM
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_OAM_CTRL . LTR_OAM_CNT_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_OAM_CTRL_LTR_OAM_CNT_ENA  VTSS_BIT(10)

/** 
 * \brief
 * Enable / disable count of OAM PDU LMM/SLM as selected OAM.
 *
 * \details 
 * '0': Count as other OAM
 * '1': Count as selected OAM
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_OAM_CTRL . LMM_OAM_CNT_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_OAM_CTRL_LMM_OAM_CNT_ENA  VTSS_BIT(9)

/** 
 * \brief
 * Enable / disable count of OAM PDU LMR/SLR/1SL as selected OAM.
 *
 * \details 
 * '0': Count as other OAM
 * '1': Count as selected OAM
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_OAM_CTRL . LMR_OAM_CNT_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_OAM_CTRL_LMR_OAM_CNT_ENA  VTSS_BIT(8)

/** 
 * \brief
 * Enable / disable count of OAM PDU LBM as selected OAM.
 * 
 * This setting is also used to control counting of SAM_SEQ frames (see
 * VOE_CONF:SAM_NON_OAM_SEQ_CFG.*)
 *
 * \details 
 * '0': Count as other OAM
 * '1': Count as selected OAM
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_OAM_CTRL . LBM_OAM_CNT_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_OAM_CTRL_LBM_OAM_CNT_ENA  VTSS_BIT(7)

/** 
 * \brief
 * Enable / disable count of OAM PDU TST as selected OAM.
 *
 * \details 
 * '0': Count as other OAM
 * '1': Count as selected OAM
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_OAM_CTRL . TST_OAM_CNT_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_OAM_CTRL_TST_OAM_CNT_ENA  VTSS_BIT(6)

/** 
 * \brief
 * Enable / disable count of OAM PDU LBR as selected OAM.
 *
 * \details 
 * '0': Count as other OAM
 * '1': Count as selected OAM
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_OAM_CTRL . LBR_OAM_CNT_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_OAM_CTRL_LBR_OAM_CNT_ENA  VTSS_BIT(5)

/** 
 * \brief
 * Enable / disable count of OAM PDU DMM as selected OAM.
 *
 * \details 
 * '0': Count as other OAM
 * '1': Count as selected OAM
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_OAM_CTRL . DMM_OAM_CNT_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_OAM_CTRL_DMM_OAM_CNT_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Enable / disable count of OAM PDU DMR as selected OAM.
 *
 * \details 
 * '0': Count as other OAM
 * '1': Count as selected OAM
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_OAM_CTRL . DMR_OAM_CNT_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_OAM_CTRL_DMR_OAM_CNT_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Enable / disable count of OAM PDU 1DM as selected OAM.
 *
 * \details 
 * '0': Count as other OAM
 * '1': Count as selected OAM
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_OAM_CTRL . SDM_OAM_CNT_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_OAM_CTRL_SDM_OAM_CNT_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Enable / disable count of OAM PDU CCM as selected OAM.
 *
 * \details 
 * '0': Count as other OAM
 * '1': Count as selected OAM
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_OAM_CTRL . CCM_OAM_CNT_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_OAM_CTRL_CCM_OAM_CNT_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable / disable count of OAM PDU CCM with LM content as selected OAM.
 * 
 * For this register to take effect, the HW processing of CCM-LM frames
 * must be enabled:
 * 
 *  * OAM_HW_CTRL.CCM_LM_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_OAM_CTRL . CCM_LM_OAM_CNT_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_OAM_CTRL_CCM_LM_OAM_CNT_ENA  VTSS_BIT(0)


/** 
 * \brief Configuration of which OAM PDUs should be counted by LM counters.
 *
 * \details
 * Default behavior is that all OAM PDUs processed by a VOE (i.e. OAM PDU
 * MEG level matches VOE MEL_VAL) will not be counted as data. This is
 * according to Y.1731
 * 
 * Using this register (OAM_CNT_DATA_CTRL) it is possible to configure the
 * OAM PDUs separately to be counted as data.
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
 * Register: \a VOP:VOE_CONF:OAM_CNT_DATA_CTRL
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_CONF_OAM_CNT_DATA_CTRL(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x0,gi,64,0,7)

/** 
 * \brief
 * Enable / disable that valid OAM PDUs with Generic OPCODES are counted by
 * the VOE LM counters.
 * 
 * This bit field contains a separate bit for each of the possible 8
 * Generic opcodes.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_DATA_CTRL . GENERIC_DATA_CNT_MASK
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_DATA_CTRL_GENERIC_DATA_CNT_MASK(x)  VTSS_ENCODE_BITFIELD(x,12,8)
#define  VTSS_M_VOP_VOE_CONF_OAM_CNT_DATA_CTRL_GENERIC_DATA_CNT_MASK     VTSS_ENCODE_BITMASK(12,8)
#define  VTSS_X_VOP_VOE_CONF_OAM_CNT_DATA_CTRL_GENERIC_DATA_CNT_MASK(x)  VTSS_EXTRACT_BITFIELD(x,12,8)

/** 
 * \brief
 * If a PDU is received with an OpCode which does not match any Specific
 * OpCode or a Generic OpCode, it will be processed as an UNKNOWN OpCode.
 * 
 * This bit field configures if OAM frames with UNKOWN OpCode are counted
 * as data in the LM counters.
 *
 * \details 
 * '0': Do not count as data
 * '1': Count as data
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_DATA_CTRL . UNK_OPCODE_DATA_CNT_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_DATA_CTRL_UNK_OPCODE_DATA_CNT_ENA  VTSS_BIT(11)

/** 
 * \brief
 * Enable / disable count of OAM PDU LTM as data in LM counters.
 *
 * \details 
 * '0': Do not count as data
 * '1': Count as data
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_DATA_CTRL . LTM_DATA_CNT_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_DATA_CTRL_LTM_DATA_CNT_ENA  VTSS_BIT(10)

/** 
 * \brief
 * Enable / disable count of OAM PDU LTR as data in LM counters.
 *
 * \details 
 * '0': Do not count as data
 * '1': Count as data
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_DATA_CTRL . LTR_DATA_CNT_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_DATA_CTRL_LTR_DATA_CNT_ENA  VTSS_BIT(9)

/** 
 * \brief
 * Enable / disable count of OAM PDU LMM as data in LM counters.
 *
 * \details 
 * '0': Do not count as data
 * '1': Count as data
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_DATA_CTRL . LMM_DATA_CNT_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_DATA_CTRL_LMM_DATA_CNT_ENA  VTSS_BIT(8)

/** 
 * \brief
 * Enable / disable count of OAM PDU LMR as data in LM counters.
 *
 * \details 
 * '0': Do not count as data
 * '1': Count as data
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_DATA_CTRL . LMR_DATA_CNT_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_DATA_CTRL_LMR_DATA_CNT_ENA  VTSS_BIT(7)

/** 
 * \brief
 * Enable / disable count of OAM PDU LBM as data in LM counters.
 *
 * \details 
 * '0': Do not count as data
 * '1': Count as data
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_DATA_CTRL . LBM_DATA_CNT_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_DATA_CTRL_LBM_DATA_CNT_ENA  VTSS_BIT(6)

/** 
 * \brief
 * Enable / disable count of OAM PDU TST as data in LM counters.
 *
 * \details 
 * '0': Do not count as data
 * '1': Count as data
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_DATA_CTRL . TST_DATA_CNT_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_DATA_CTRL_TST_DATA_CNT_ENA  VTSS_BIT(5)

/** 
 * \brief
 * Enable / disable count of OAM PDU LBR as data in LM counters.
 * 
 * This setting is also used to control counting SAM_SEQ frames (see
 * VOE_CONF:SAM_NON_OAM_SEQ_CFG.*)
 *
 * \details 
 * '0': Do not count as data
 * '1': Count as data
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_DATA_CTRL . LBR_DATA_CNT_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_DATA_CTRL_LBR_DATA_CNT_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Enable / disable count of OAM PDU DMM as data in LM counters.
 *
 * \details 
 * '0': Do not count as data
 * '1': Count as data
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_DATA_CTRL . DMM_DATA_CNT_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_DATA_CTRL_DMM_DATA_CNT_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Enable / disable count of OAM PDU DMR as data in LM counters.
 *
 * \details 
 * '0': Do not count as data
 * '1': Count as data
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_DATA_CTRL . DMR_DATA_CNT_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_DATA_CTRL_DMR_DATA_CNT_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Enable / disable count of OAM PDU 1DM as data in LM counters.
 *
 * \details 
 * '0': Do not count as data
 * '1': Count as data
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_DATA_CTRL . SDM_DATA_CNT_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_DATA_CTRL_SDM_DATA_CNT_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable / disable count of OAM PDU CCM(-LM) as data in LM counters.
 *
 * \details 
 * '0': Do not count as data
 * '1': Count as data
 *
 * Field: ::VTSS_VOP_VOE_CONF_OAM_CNT_DATA_CTRL . CCM_DATA_CNT_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_CNT_DATA_CTRL_CCM_DATA_CNT_ENA  VTSS_BIT(0)


/** 
 * \brief VOE MAC Unicast address (LSB)
 *
 * \details
 * Configures the VOE Unicast MAC address (LSB).
 * 
 * This address can be checked when frames arrive, depending on the
 * configuration of: 
 * VOE_CONF:VOE_CTRL.RX_DMAC_CHK_SEL
 *
 * Register: \a VOP:VOE_CONF:MEP_UC_MAC_LSB
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_CONF_MEP_UC_MAC_LSB(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x0,gi,64,0,8)


/** 
 * \brief VOE MAC Unicast address (MSB)
 *
 * \details
 * Configures the VOE Unicast MAC address (MSB).
 * 
 * This address can be checked when frames arrive, depending on the
 * configuration of: 
 * VOE_CONF:VOE_CTRL.RX_DMAC_CHK_SEL
 *
 * Register: \a VOP:VOE_CONF:MEP_UC_MAC_MSB
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_CONF_MEP_UC_MAC_MSB(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x0,gi,64,0,9)

/** 
 * \brief
 * See register description.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_MEP_UC_MAC_MSB . MEP_UC_MAC_MSB
 */
#define  VTSS_F_VOP_VOE_CONF_MEP_UC_MAC_MSB_MEP_UC_MAC_MSB(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VOP_VOE_CONF_MEP_UC_MAC_MSB_MEP_UC_MAC_MSB     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VOP_VOE_CONF_MEP_UC_MAC_MSB_MEP_UC_MAC_MSB(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief OAM HW processing control
 *
 * \details
 * Configures per OAM opcode if it is processed by VOE hardware.
 * 
 * If a OAM PDU type is not enabled in this register, the OAM PDU will not
 * be processed by the VOE.
 * 
 * This implies that the OAM PDU is not updated, and that PDU specific
 * registers are not updated.
 * 
 * However, note the following:
 *  * The RX sticky bits will be set for a PDU, even when the HW processing
 * is not enabled.
 *  * OAM PDUs can be extracted to the CPU, even when HW processing is not
 * enabled.
 *  * OAM PDUs can be counted as data, even when HW processing is not
 * enabled.
 *  * The MEL check will be performed and PDUs with LOW MEL will be
 * discarded. They can optionally be extracted to the CPU.
 *
 * Register: \a VOP:VOE_CONF:OAM_HW_CTRL
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_CONF_OAM_HW_CTRL(gi)    VTSS_IOREG_IX(VTSS_TO_VOP,0x0,gi,64,0,10)

/** 
 * \brief
 * Enable HW processing of valid LMM/SLM PDUs received by the VOE in both
 * the TX and the RX direction.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_OAM_HW_CTRL . LMM_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_HW_CTRL_LMM_ENA  VTSS_BIT(11)

/** 
 * \brief
 * Enable HW processing of valid LMR/SLR/1SL PDUs received by the VOE in
 * both the TX and the RX direction.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_OAM_HW_CTRL . LMR_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_HW_CTRL_LMR_ENA  VTSS_BIT(10)

/** 
 * \brief
 * Enable HW processing of valid TST PDUs received by the VOE in both the
 * TX and the RX direction.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_OAM_HW_CTRL . TST_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_HW_CTRL_TST_ENA  VTSS_BIT(9)

/** 
 * \brief
 * Assertion of this bit field requires TST_ENA to be asserted.
 * 
 * If this bit field is asserted, the incoming TST PDUs will be checked for
 * having a TLV of type = "Test TLV" (Type= 32).
 * 
 * If the TLV Test PDU includes a CRC-32 field, the CRC is calculated
 * across the Data Pattern, and the CRC-32 field is verified.
 * 
 * The number of Test TLV with CRC-32 error is counted in :
 * 
 *  - OAM_MEP:VOE_CRC_ERR:LBR_CRC_ERR_CNT.LBR_CRC_ERR_CNT
 * 
 * Note: The Test TLV must be the first TLV in the received TST PDU.
 * In case several Test TLVs are present in the same PDU, only the first is
 * checked for CRC.
 * 
 * Note: This should not be enabled at the same time as: 
 *  - LBR_TLV_CRC_VERIFY_ENA
 * 
 * (The TST and LBM/LBR are expected to mutually exclusive, hence they will
 * use the same TLV_CRC_ERR counter)
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_OAM_HW_CTRL . TST_TLV_CRC_VERIFY_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_HW_CTRL_TST_TLV_CRC_VERIFY_ENA  VTSS_BIT(8)

/** 
 * \brief
 * Enable HW processing of valid LBM PDUs received by the VOE in both the
 * TX and the RX direction.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_OAM_HW_CTRL . LBM_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_HW_CTRL_LBM_ENA  VTSS_BIT(7)

/** 
 * \brief
 * Enable HW processing of valid LBR PDUs received by the VOE in both the
 * TX and the RX direction.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_OAM_HW_CTRL . LBR_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_HW_CTRL_LBR_ENA  VTSS_BIT(6)

/** 
 * \brief
 * Assertion of this bit field requires LBR_ENA to be asserted.
 * 
 * If this bit field is asserted, the incoming LBR PDUs will be checked for
 * having a TLV of type = "Test TLV" (Type= 32).
 * 
 * If the TLV Test PDU includes a CRC-32 field, the CRC is calculated
 * across the Data Pattern, and the CRC-32 field is verified.
 * 
 * The number of Test TLV with CRC-32 error is counted in :
 * 
 *  - OAM_MEP:VOE_CRC_ERR:LBR_CRC_ERR_CNT.LBR_CRC_ERR_CNT
 * 
 * Note: The Test TLV must be the first TLV in the received LBR PDU.
 * In case several Test TLVs are present in the same PDU, only the first is
 * checked for CRC.
 * 
 * Note: This should not be enabled at the same time as: 
 *  - TST_TLV_CRC_VERIFY_ENA
 * 
 * (The TST and LBM/LBR are expected to mutually exclusive, hence they will
 * use the same TLV_CRC_ERR counter)
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_OAM_HW_CTRL . LBR_TLV_CRC_VERIFY_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_HW_CTRL_LBR_TLV_CRC_VERIFY_ENA  VTSS_BIT(5)

/** 
 * \brief
 * Enable HW processing of valid DMM PDUs received by the VOE in both the
 * TX and the RX direction.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_OAM_HW_CTRL . DMM_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_HW_CTRL_DMM_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Enable HW processing of valid DMR PDUs received by the VOE in both the
 * TX and the RX direction.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_OAM_HW_CTRL . DMR_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_HW_CTRL_DMR_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Enable HW processing of valid 1DM PDUs received by the VOE in both the
 * TX and the RX direction.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_OAM_HW_CTRL . SDM_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_HW_CTRL_SDM_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Enable HW processing of valid CCM PDUs received by the VOE in both the
 * TX and the RX direction.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_OAM_HW_CTRL . CCM_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_HW_CTRL_CCM_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable HW processing of valid CCM-LM PDUs received by the VOE in both
 * the TX and the RX direction.
 * 
 * CC-LM is only supported on the priority configured in the following
 * priority:
 * 
 *  - VOE_CONF:CCM_CFG.CCM_PRIO
 * 
 * If an RX CCM PDU is received on another PRIORITY than this, it will
 * update the statistics will be updated as if it was a CCM_LM PDU, but no
 * values are sampled for use in the TX direction:
 * 
 *  - CCM-LM.TX_FC_F
 *  - FC LM Rx counter when the CCM-LM frame was received
 * 
 * These values are sampled for valid CCM-LM PDUs received on the correct
 * priority.
 * 
 * 
 * NOTE:
 *   OAM_HW_CTRL.CCM_ENA must be asserted when asserting CCM_LM_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_OAM_HW_CTRL . CCM_LM_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_OAM_HW_CTRL_CCM_LM_ENA  VTSS_BIT(0)


/** 
 * \brief Enables loopback of OAM Messages to OAM Replies
 *
 * \details
 * Some OAM PDU types can be looped, by returning a Reply in response to a
 * Message.
 * 
 * The looping of these PDUs can be enabled setting the bit fields in this
 * register.
 * 
 * The PDU being looped must be enabled in the following register:
 * 
 *  - VOE_CONF:OAM_HW_CTRL.*
 *
 * Register: \a VOP:VOE_CONF:LOOPBACK_ENA
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_CONF_LOOPBACK_ENA(gi)   VTSS_IOREG_IX(VTSS_TO_VOP,0x0,gi,64,0,11)

/** 
 * \brief
 * This field determines whether incoming LBM frames are looped and
 * transmitted as LBR frames.
 * 
 * If loopback is not enabled frames are discarded.
 * 
 * Incoming LBM frames can optionally be extracted to the CPU, regardless
 * of loopback setting.
 * 
 * This setting is also used to loop SAM_SEQ frames (see
 * VOE_CONF:SAM_NON_OAM_SEQ_CFG.*)
 *
 * \details 
 * '0' : OAM LBM PDU is not looped as LBR frames.
 * '1' : OAM LBM PDU is looped as a LBR frame.
 *
 * Field: ::VTSS_VOP_VOE_CONF_LOOPBACK_ENA . LB_LBM_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_LOOPBACK_ENA_LB_LBM_ENA  VTSS_BIT(2)

/** 
 * \brief
 * This field determines whether incoming LMM/SLM frames are looped and
 * transmitted as LMR/SLR frames.
 * 
 * If loopback is not enabled frames are discarded.
 * 
 * Incoming LMM/SLM frames can optionally be extracted to the CPU,
 * regardless of loopback setting.

 *
 * \details 
 * '0' : OAM LMM/SLM PDU is not looped as LMR frames.
 * '1' : OAM LMM/SLM PDU is looped as a LMR frame.
 *
 * Field: ::VTSS_VOP_VOE_CONF_LOOPBACK_ENA . LB_LMM_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_LOOPBACK_ENA_LB_LMM_ENA  VTSS_BIT(1)

/** 
 * \brief
 * This field determines whether incoming DMM frames are looped and
 * transmitted as DMR frames.
 * 
 * If loopback is not enabled frames are discarded.
 * 
 * Incoming DMM frames can optionally be extracted to the CPU, regardless
 * of loopback setting.
 *
 * \details 
 * '0' : OAM DMM PDU is not looped as DMR frames.
 * '1' : OAM DMM PDU is looped as a DMR frame.
 *
 * Field: ::VTSS_VOP_VOE_CONF_LOOPBACK_ENA . LB_DMM_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_LOOPBACK_ENA_LB_DMM_ENA  VTSS_BIT(0)


/** 
 * \brief OAM Loopback configuration
 *
 * \details
 * Contains configuration for loopbing back frames. I.e. returning OAM
 * replies in response to messages.
 *
 * Register: \a VOP:VOE_CONF:LOOPBACK_CFG
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_CONF_LOOPBACK_CFG(gi)   VTSS_IOREG_IX(VTSS_TO_VOP,0x0,gi,64,0,12)

/** 
 * \brief
 * Used for Down-MEP only - has no effect for Up-MEP.
 * 
 * When a PDU Message is looped to a PDU Response, the ISDX for the PDU
 * Respoinse will be change to the value configured in :
 * 
 *  * ISDX_LB / ISDX_LB_P
 * 
 * When the Response passes through the REW, this value will overwrite the
 * IFH.FWD.ES0_ISDX_KEY_ENA.
 * 
 * Hence using this bit field it is possible to configure whether the PDU
 * Message will use ISDX for the ES0 lookup.
 * 
 * This is relevant for following PDU types:
 *  * LMM --> LMR
 *  * DMM --> DMR
 *  * LBM --> LBR

 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_LOOPBACK_CFG . LB_ES0_ISDX_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_LOOPBACK_CFG_LB_ES0_ISDX_ENA  VTSS_BIT(13)

/** 
 * \brief
 * All valid OAM messages (LBM, DMM, LMM) which are loopedback into OAM
 * replies (LBR, DMR, LMR) will be forwarded in the return direction with
 * the ISDX value configured in this register.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_LOOPBACK_CFG . LB_ISDX
 */
#define  VTSS_F_VOP_VOE_CONF_LOOPBACK_CFG_LB_ISDX(x)  VTSS_ENCODE_BITFIELD(x,1,12)
#define  VTSS_M_VOP_VOE_CONF_LOOPBACK_CFG_LB_ISDX     VTSS_ENCODE_BITMASK(1,12)
#define  VTSS_X_VOP_VOE_CONF_LOOPBACK_CFG_LB_ISDX(x)  VTSS_EXTRACT_BITFIELD(x,1,12)

/** 
 * \brief
 * When OAM PDUs are looped, the dp bits can be cleared or keep their value
 * depending on the setting of this bit.
 * 
 * This only affects frames being looped:
 *  - LBM --> LBR
 *  - LMM --> LMR
 *  - DMM --> DMR
 *
 * \details 
 * 0: DP bits to be looped with the frame.
 * 1: DP bits are cleared when frame is looped.
 *
 * Field: ::VTSS_VOP_VOE_CONF_LOOPBACK_CFG . CLEAR_DP_ON_LOOP
 */
#define  VTSS_F_VOP_VOE_CONF_LOOPBACK_CFG_CLEAR_DP_ON_LOOP  VTSS_BIT(0)


/** 
 * \brief Configures updating sequence numbers / transactions ID (TX)
 *
 * \details
 * The configuration in this register group determines whether the VOE will
 * update the sequence number / transaction ID for valid TX frames.
 *
 * Register: \a VOP:VOE_CONF:TX_TRANSID_UPDATE
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_CONF_TX_TRANSID_UPDATE(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x0,gi,64,0,13)

/** 
 * \brief
 * If asserted, the transaction ID will be updated for valid LBM frames
 * transmitted by this VOE.
 * 
 * This can be used to avoid overwriting the TX ID for externally generated
 * LBM frames.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_TX_TRANSID_UPDATE . LBM_UPDATE_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_TX_TRANSID_UPDATE_LBM_UPDATE_ENA  VTSS_BIT(1)

/** 
 * \brief
 * If asserted, the transaction ID will be updated for valid TST frames
 * transmitted by this VOE.
 * 
 * This can be used to avoid overwriting the TX ID for externally generated
 * TST frames.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_TX_TRANSID_UPDATE . TST_UPDATE_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_TX_TRANSID_UPDATE_TST_UPDATE_ENA  VTSS_BIT(0)


/** 
 * \brief Miscellaneous CCM configuration
 *
 * \details
 * Misc configuration for CCM PDU handling.
 *
 * Register: \a VOP:VOE_CONF:CCM_CFG
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_CONF_CCM_CFG(gi)        VTSS_IOREG_IX(VTSS_TO_VOP,0x0,gi,64,0,14)

/** 
 * \brief
 * The VOE can insert LM information into CCM frames injected by internal
 * or external CPU.
 * 
 * (NOTE: OAM_HW_CTRL.CCM_LM_ENA must be asserted prior to configuring this
 * register.)
 * 
 * The insertion of LM information into CCM frames will occur based on a
 * LOC timeout counter.
 * 
 * This bit field configures which of the 7 LOC timeout counters to use for
 * LM insertion.
 * This configuration will NOT affect the CCM PDU period field.
 * 
 * When the selected LOC timeout counter times out, the following bit field
 * will be asserted by the HW:
 * 
 * VOE_STAT:CCM_STAT.CCM_LM_INSERT_NXT
 * 
 * When the above bit field is asserted, LM information will be inserted
 * into the next CCM frame transmitted by the VOE. Subsequently the
 * bitfield will be de-asserted by HW.
 * 
 * Note that the rate at which LM infornation is inserted is twice the rate
 * indicated by the PERIOD_VAL of the selected timeout counter.
 * 
 * I.e. if the CCM_PERIOD_CFG.CCM_PERIOD_VAL of the selected timeout
 * counter is set to 10 ms, LM information will be inserted every 5 ms.
 *
 * \details 
 * 0: Disable automatic insertion of Loss Measurements in OAM CCM PDU
 * n: Automatic insertion of Loss Measurements in next OAM CCM PDU when the
 * corresponding OAM_MEP:COMMON:CCM_PERIOD_CFG occurs.
 *
 * Field: ::VTSS_VOP_VOE_CONF_CCM_CFG . CCM_LM_PERIOD
 */
#define  VTSS_F_VOP_VOE_CONF_CCM_CFG_CCM_LM_PERIOD(x)  VTSS_ENCODE_BITFIELD(x,10,3)
#define  VTSS_M_VOP_VOE_CONF_CCM_CFG_CCM_LM_PERIOD     VTSS_ENCODE_BITMASK(10,3)
#define  VTSS_X_VOP_VOE_CONF_CCM_CFG_CCM_LM_PERIOD(x)  VTSS_EXTRACT_BITFIELD(x,10,3)

/** 
 * \brief
 * If asserted, the sequence number will be updated for CCM frames
 * transmitted from this VOE.
 * 
 * The CCM sequence number of transmitted CCM(-LM) frames will be
 * overwritten with the value configured in the following bit field:
 * 
 *  - VOE_STAT:CCM_TX_SEQ_CFG.CCM_TX_SEQ
 * 
 * (Note that the above register is always updated +1 when the VOE
 * transmits a valid CCM(-LM) PDU)

 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_CCM_CFG . CCM_SEQ_UPD_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_CCM_CFG_CCM_SEQ_UPD_ENA  VTSS_BIT(9)

/** 
 * \brief
 * If asserted, the sequence number of valid CCM(-LM) frames received by
 * the VOE is validated against the expected value.
 * The expected value is the value of the following bit field + 1:
 * 
 *  - VOE_STAT:CCM_RX_SEQ_CFG.CCM_RX_SEQ
 * 
 * When a valid CCM(-LM) PDU is received by the VOE, the value of the
 * CCM(-LM).sequence_number is stored in the above bit field.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_CCM_CFG . CCM_RX_SEQ_CHK_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_CCM_CFG_CCM_RX_SEQ_CHK_ENA  VTSS_BIT(8)

/** 
 * \brief
 * Configures expected priority of CCM(-LM) frames received by the VOE.
 * 
 * The VOE will verify the priority of valid incoming CCM(-LM) PDUs against
 * this value.
 * 
 * If the validation fails the following sticky bit is asserted:
 * 
 *  - VOE_STAT:CCM_RX_LAST.CCM_PERIOD_ERR
 * 
 * An interrupt can optionally be generated when the state of the CCM prio
 * verification changes :
 * 
 *  - VOE_STAT:INTR_STICKY.CCM_PRIO_RX_ERR_STICKY
 * 
 * Note that a CCM(-LM) frame is valid even when the CCM PRIORITY
 * verifation fails. 
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_CCM_CFG . CCM_PRIO
 */
#define  VTSS_F_VOP_VOE_CONF_CCM_CFG_CCM_PRIO(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_VOP_VOE_CONF_CCM_CFG_CCM_PRIO     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_VOP_VOE_CONF_CCM_CFG_CCM_PRIO(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * Configures expected CCM period.
 * 
 * The PERIOD fields of the valid incoming CCM frames will be checked
 * against this value.
 * 
 * Further the configuration of this bit field specifies the LOC timeout
 * counter to be used for LOC scan for this VOE.
 * 
 * Note that a CCM(-LM) frame is valid even when the CCM PERIOD verifation
 * fails.
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
 * Field: ::VTSS_VOP_VOE_CONF_CCM_CFG . CCM_PERIOD
 */
#define  VTSS_F_VOP_VOE_CONF_CCM_CFG_CCM_PERIOD(x)  VTSS_ENCODE_BITFIELD(x,2,3)
#define  VTSS_M_VOP_VOE_CONF_CCM_CFG_CCM_PERIOD     VTSS_ENCODE_BITMASK(2,3)
#define  VTSS_X_VOP_VOE_CONF_CCM_CFG_CCM_PERIOD(x)  VTSS_EXTRACT_BITFIELD(x,2,3)

/** 
 * \brief
 * Configures if the VOE will validate the MEG ID of valid incoming
 * CCM(-LM) PDUs.
 *
 * \details 
 * 0: No MEGID check
 * 1: Check MEG ID
 *
 * Field: ::VTSS_VOP_VOE_CONF_CCM_CFG . CCM_MEGID_CHK_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_CCM_CFG_CCM_MEGID_CHK_ENA  VTSS_BIT(1)

/** 
 * \brief
 * If this bit is asserted, the value of the MEP ID in incoming CCM frames
 * will be verified against the value programmed in :
 * 
 * VOE_CONF:CCM_MEPID_CFG.CCM_MEPID
 * 
 * Errors in the incomfing MEP ID will be reported in the following error
 * bit:
 * 
 * VOE_CONF:CCM_CFG.CCM_MEPID_ERR
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_CCM_CFG . CCM_MEPID_CHK_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_CCM_CFG_CCM_MEPID_CHK_ENA  VTSS_BIT(0)


/** 
 * \brief Configuration of CCM MEPID
 *
 * \details
 * Configures 16 bit MEP ID value to be verified against incoming CCM
 * frames.
 * 
 * In case MEP ID verification is enabled
 * (VOE_CONF:CCM_CFG.CCM_MEPID_CHK_ENA = 1) the value of the CCM.MEPID
 * field of incoming CCM/CCM_LM frames will be verified against the value
 * configured in this register.
 * 
 * If there is a mismatch, the following bit will be asserted:
 * 
 * VOE_STAT:CCM_STAT.CCM_MEPID_ERR
 * 
 * The above bit can be configured to generate an interrupt.
 *
 * Register: \a VOP:VOE_CONF:CCM_MEPID_CFG
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_CONF_CCM_MEPID_CFG(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x0,gi,64,0,15)

/** 
 * \brief
 * See register description.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_CCM_MEPID_CFG . CCM_MEPID
 */
#define  VTSS_F_VOP_VOE_CONF_CCM_MEPID_CFG_CCM_MEPID(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VOP_VOE_CONF_CCM_MEPID_CFG_CCM_MEPID     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VOP_VOE_CONF_CCM_MEPID_CFG_CCM_MEPID(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Configuration of CCM MEGID
 *
 * \details
 * Configures 48 byte MEG ID (lowest replication index correspond to MSB)
 * to verified in incoming CCM frames.
 * 
 * In case MEG ID verification is enabled
 * (VOE_CONF:CCM_CFG.CCM_MEGID_CHK_ENA = 1) the value of the CCM.MEGID
 * field of incoming CCM/CCM_LM frames will be verified against the value
 * configured in this register.
 * 
 * If there is a mismatch, the following bit will be asserted:
 * 
 * VOE_STAT:CCM_STAT.CCM_MEGID_ERR
 * 
 * The above bit can be configured to generate an interrupt.
 *
 * Register: \a VOP:VOE_CONF:CCM_MEGID_CFG
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 * @param ri Register: CCM_MEGID_CFG (??), 0-11
 */
#define VTSS_VOP_VOE_CONF_CCM_MEGID_CFG(gi,ri)  VTSS_IOREG_IX(VTSS_TO_VOP,0x0,gi,64,ri,16)


/** 
 * \brief Configurations for Synthetic Loss Measurements
 *
 * \details
 * Register: \a VOP:VOE_CONF:SLM_CONFIG
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_CONF_SLM_CONFIG(gi)     VTSS_IOREG_IX(VTSS_TO_VOP,0x0,gi,64,0,28)

/** 
 * \brief
 * The VOE supports only a single priority (COSID) when configured for
 * SynLM.
 * 
 * If the frame priority of TX / RX frames SynLM PDUs processed by the VOE
 * does not match the configured value, the frame is considered to be
 * invalid.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_SLM_CONFIG . SLM_PRIO
 */
#define  VTSS_F_VOP_VOE_CONF_SLM_CONFIG_SLM_PRIO(x)  VTSS_ENCODE_BITFIELD(x,13,3)
#define  VTSS_M_VOP_VOE_CONF_SLM_CONFIG_SLM_PRIO     VTSS_ENCODE_BITMASK(13,3)
#define  VTSS_X_VOP_VOE_CONF_SLM_CONFIG_SLM_PRIO(x)  VTSS_EXTRACT_BITFIELD(x,13,3)

/** 
 * \brief
 * The Initiator MEPID is matched against the SLR.SRC_MEPID of incoming SLR
 * PDUs.
 * 
 * If the value of the incoming SLR.SRC_MEPID does not match this value,
 * the frame is considered to be invalid.
 * 
 * When this checks fail, the following sticky bit is asserted:
 *  - VOE_STAT:OAM_RX_STICKY2.RX_INI_ILLEGAL_MEPID
 * 
 * Note that the upper 3 bits of the MEPID must be zero.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_SLM_CONFIG . SLM_MEPID
 */
#define  VTSS_F_VOP_VOE_CONF_SLM_CONFIG_SLM_MEPID(x)  VTSS_ENCODE_BITFIELD(x,0,13)
#define  VTSS_M_VOP_VOE_CONF_SLM_CONFIG_SLM_MEPID     VTSS_ENCODE_BITMASK(0,13)
#define  VTSS_X_VOP_VOE_CONF_SLM_CONFIG_SLM_MEPID(x)  VTSS_EXTRACT_BITFIELD(x,0,13)


/** 
 * \brief SynLM Initiator Test ID
 *
 * \details
 * A SynLM session is identified by a SynLM Test ID.
 * 
 * The VOE supports a single Test ID for each Initiator function.
 * The Initiator function will validate the Test ID of incoming SLR PDUs.
 * If the Test ID of the incoming SLR PDU doest not match the value
 * configured in this register, the frame will be considered invalid.
 * 
 * The VOE will not verify the Test ID in incoming PDUs when acting as a
 * Remote MEP.
 *
 * Register: \a VOP:VOE_CONF:SLM_TEST_ID
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_CONF_SLM_TEST_ID(gi)    VTSS_IOREG_IX(VTSS_TO_VOP,0x0,gi,64,0,29)


/** 
 * \brief A list of the MEPIDs with which SLM is supported.
 *
 * \details
 * This list can be programmed with up till 8 MEPIDs which identify the
 * peer MEPs which are part of the SynLM session.
 * 
 * When a SynLM PDU is processed by the VOE, the VOE will match the MEPID
 * of the peer MEP (found in the SynLM PDU)
 * 
 * If a match is found, the VOE will use the index of the row which matches
 * the MEPID to identify which peer MEP the frame is sent to / received
 * from.
 * 
 * This index is used when updating the RX / TX LM counters for a VOE which
 * is enabled for SynLM.
 *
 * Register: \a VOP:VOE_CONF:SLM_PEER_LIST
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 * @param ri Replicator: x_SLM_NO_OF_PEERS (??), 0-7
 */
#define VTSS_VOP_VOE_CONF_SLM_PEER_LIST(gi,ri)  VTSS_IOREG_IX(VTSS_TO_VOP,0x0,gi,64,ri,30)

/** 
 * \brief
 * MEPID used to identify the peer MEP from which the SL PDU is received /
 * to which the SL PDU is sent.
 * 
 * Only valid if SLM_PEER_ENA = 1
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_SLM_PEER_LIST . SLM_PEER_MEPID
 */
#define  VTSS_F_VOP_VOE_CONF_SLM_PEER_LIST_SLM_PEER_MEPID(x)  VTSS_ENCODE_BITFIELD(x,1,13)
#define  VTSS_M_VOP_VOE_CONF_SLM_PEER_LIST_SLM_PEER_MEPID     VTSS_ENCODE_BITMASK(1,13)
#define  VTSS_X_VOP_VOE_CONF_SLM_PEER_LIST_SLM_PEER_MEPID(x)  VTSS_EXTRACT_BITFIELD(x,1,13)

/** 
 * \brief
 * If the enabled, SLM_PEER_MEPID contains a valid MEPID
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONF_SLM_PEER_LIST . SLM_PEER_ENA
 */
#define  VTSS_F_VOP_VOE_CONF_SLM_PEER_LIST_SLM_PEER_ENA  VTSS_BIT(0)

/**
 * Register Group: \a VOP:VOE_STAT
 *
 * Per VOE statistics and counters (Y.1731 OAM)
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
 * Selected OAM PDUs are configured in : VOE_CONF:OAM_CNT_OAM_CTRL.*
 *
 * Register: \a VOP:VOE_STAT:RX_SEL_OAM_CNT
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_RX_SEL_OAM_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,0)


/** 
 * \brief Count non-selected OAM PDU received by VOE.
 *
 * \details
 * All PDU types can be configured as either selected or non-selected PDUs.
 * 
 * This register counts the number of valid OAM PDUs configured as
 * non-selected PDU type, received by the VOE.
 * 
 * Selected OAM PDUs are configured in : VOE_CONF:OAM_CNT_OAM_CTRL.*
 *
 * Register: \a VOP:VOE_STAT:RX_OAM_FRM_CNT
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_RX_OAM_FRM_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,1)


/** 
 * \brief Count selected OAM PDUs transmitted by VOE.
 *
 * \details
 * All PDU types can be configured as either selected or non-selected PDUs.
 * 
 * This register counts the number of valid OAM PDUs configured as selected
 * PDU type, transmitted by the VOE.
 * 
 * Selected OAM PDUs are configured in : VOE_CONF:OAM_CNT_OAM_CTRL.*
 *
 * Register: \a VOP:VOE_STAT:TX_SEL_OAM_CNT
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_TX_SEL_OAM_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,2)


/** 
 * \brief Count NON-selected OAM PDUs transmitted by VOE.
 *
 * \details
 * All PDU types can be configured as either selected or non-selected PDUs.
 * 
 * This register counts the number of valid OAM PDUs configured as
 * non-selected PDU type, transmitted by the VOE.
 * 
 * Selected OAM PDUs are configured in : VOE_CONF:OAM_CNT_OAM_CTRL.*
 *
 * Register: \a VOP:VOE_STAT:TX_OAM_FRM_CNT
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_TX_OAM_FRM_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,3)


/** 
 * \brief Number of valid CCM(-LM) PDUs received by VOE.
 *
 * \details
 * Counts the number of valid OAM CCM PDUs received by the VOE.
 * 
 * Valid CCM(-LM) PDUs will clear the CCM_MISS counter.
 * 
 * Counters are updated regardless of the value of :
 * 
 *  * VOE_CONF:OAM_HW_CTRL.CCM_ENA
 *
 * Register: \a VOP:VOE_STAT:CCM_RX_FRM_CNT
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_CCM_RX_FRM_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,4)

/** 
 * \brief
 * Counts the number of valid OAM CCM PDUs received by the VOE.
 * 
 * Valid CCM(-LM) PDUs will clear the CCM_MISS counter.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_CCM_RX_FRM_CNT . CCM_RX_VLD_FC_CNT
 */
#define  VTSS_F_VOP_VOE_STAT_CCM_RX_FRM_CNT_CCM_RX_VLD_FC_CNT(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_VOP_VOE_STAT_CCM_RX_FRM_CNT_CCM_RX_VLD_FC_CNT     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_VOP_VOE_STAT_CCM_RX_FRM_CNT_CCM_RX_VLD_FC_CNT(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Counts number of invalid CCM(-LM) PDUs received by the VOE.
 * 
 * Invalid CCM(-LM) PDUs will not clear the CCM_MISS_CNT.
 * 
 * Invalid CCM(-LM) PDUs are defined as the CCM(-LM) PDUs which do not pass
 * VOE RX validation.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_CCM_RX_FRM_CNT . CCM_RX_INVLD_FC_CNT
 */
#define  VTSS_F_VOP_VOE_STAT_CCM_RX_FRM_CNT_CCM_RX_INVLD_FC_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VOP_VOE_STAT_CCM_RX_FRM_CNT_CCM_RX_INVLD_FC_CNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VOP_VOE_STAT_CCM_RX_FRM_CNT_CCM_RX_INVLD_FC_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Configuration of CCM Sequence number
 *
 * \details
 * Configures CCM Sequence number to be inserted into the next CCM or
 * CCM-LM PDU.
 * 
 * The sequence number of valid CCM(-LM) PDUs is overwritten in the TX
 * direction depending on the configuration of the following bit field:
 * 
 *  - VOE_CONF:CCM_CFG.CCM_SEQ_UPD_ENA
 * 
 * Counters are updated regardless of the value of :
 * 
 *  * VOE_CONF:OAM_HW_CTRL.CCM_ENA
 *
 * Register: \a VOP:VOE_STAT:CCM_TX_SEQ_CFG
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_CCM_TX_SEQ_CFG(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,5)


/** 
 * \brief Last RX CCM Sequence number
 *
 * \details
 * This register contains the RX CCM sequence number of the latest valid
 * CCM(-LM) PDU received  by the VOE.
 * The register is automatically updated upon reception of a valid CCM(-LM)
 * PDU.
 * 
 * If RX CCM sequence number checking is enabled, the expected value of the
 * next CCM(-LM) sequence number is the value contained in this register +
 * 1.
 * 
 * RX CCM sequence number checking is enabled using the following bit
 * field:
 * 
 *  - VOE_CONF:CCM_CFG.CCM_SEQ_UPD_ENA
 *
 * Register: \a VOP:VOE_STAT:CCM_RX_SEQ_CFG
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_CCM_RX_SEQ_CFG(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,6)


/** 
 * \brief Number of valid CCM received with sequence error
 *
 * \details
 * This register contains counters for the following CCM PDU warnings:
 * 
 *  * Sequence Errors
 *  * Period Errors
 * 
 * These counters are only updated when :
 * 
 *  * VOE_CONF:OAM_HW_CTRL.CCM_ENA = 1
 *
 * Register: \a VOP:VOE_STAT:CCM_RX_WARNING
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_CCM_RX_WARNING(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,7)

/** 
 * \brief
 * Number of valid CCM received with sequence number error.
 * 
 * Sequence numbers are counted when the CCM_PDU.SequenceNumber of a valid
 * CCM PDU is not equal to the previously received SequencNumber + 1.
 * 
 * The previously received CCM_PDU.Sequence Number is stored in :
 * 
 *  * VOE_STAT:CCM_RX_SEQ_CFG.CCM_RX_SEQ
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_CCM_RX_WARNING . CCM_RX_SEQNO_ERR_CNT
 */
#define  VTSS_F_VOP_VOE_STAT_CCM_RX_WARNING_CCM_RX_SEQNO_ERR_CNT(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_VOP_VOE_STAT_CCM_RX_WARNING_CCM_RX_SEQNO_ERR_CNT     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_VOP_VOE_STAT_CCM_RX_WARNING_CCM_RX_SEQNO_ERR_CNT(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_CCM_RX_WARNING . CCM_RX_PERIOD_ERR_CNT
 */
#define  VTSS_F_VOP_VOE_STAT_CCM_RX_WARNING_CCM_RX_PERIOD_ERR_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VOP_VOE_STAT_CCM_RX_WARNING_CCM_RX_PERIOD_ERR_CNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VOP_VOE_STAT_CCM_RX_WARNING_CCM_RX_PERIOD_ERR_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a VOP:VOE_STAT:CCM_ERR
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_CCM_ERR(gi)        VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,8)

/** 
 * \brief
 * Number of CCM PDUs received with MEL lower than the MEL configured for
 * the VOE
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_CCM_ERR . CCM_RX_MEL_ERR_CNT
 */
#define  VTSS_F_VOP_VOE_STAT_CCM_ERR_CCM_RX_MEL_ERR_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VOP_VOE_STAT_CCM_ERR_CCM_RX_MEL_ERR_CNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VOP_VOE_STAT_CCM_ERR_CCM_RX_MEL_ERR_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Counts CCM PDUs received with specific CCM errors.
 * 
 * These counters are only updated when :
 * 
 *  * VOE_CONF:OAM_HW_CTRL.CCM_ENA = 1
 *
 * Register: \a VOP:VOE_STAT:CCM_RX_ERR_1
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_CCM_RX_ERR_1(gi)   VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,9)

/** 
 * \brief
 * Number of CCM PDUs received at the correct MEL, but with an incorrect
 * MEGID.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_CCM_RX_ERR_1 . CCM_RX_MEGID_ERR_CNT
 */
#define  VTSS_F_VOP_VOE_STAT_CCM_RX_ERR_1_CCM_RX_MEGID_ERR_CNT(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_VOP_VOE_STAT_CCM_RX_ERR_1_CCM_RX_MEGID_ERR_CNT     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_VOP_VOE_STAT_CCM_RX_ERR_1_CCM_RX_MEGID_ERR_CNT(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Number of CCM PDUs received at the correct MEL, but with an incorrect
 * MEPID.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_CCM_RX_ERR_1 . CCM_RX_MEPID_ERR_CNT
 */
#define  VTSS_F_VOP_VOE_STAT_CCM_RX_ERR_1_CCM_RX_MEPID_ERR_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VOP_VOE_STAT_CCM_RX_ERR_1_CCM_RX_MEPID_ERR_CNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VOP_VOE_STAT_CCM_RX_ERR_1_CCM_RX_MEPID_ERR_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Configuration of LBM transaction ID 
 *
 * \details
 * This register holds the transaction ID to be inserted into the next
 * valid LBM PDU transmitted by the VOE.
 * 
 * LBM TX frames will be updated with this value if updating is enabled:
 * 
 * VOE_CONF:TX_TRANSID_UPDATE.LBM_UPDATE_ENA
 * 
 * When updating a TX LBM frame, the HW will automatically increment the
 * value of this register by +1.
 *
 * Register: \a VOP:VOE_STAT:LBM_TX_TRANSID_CFG
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_LBM_TX_TRANSID_CFG(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,10)


/** 
 * \details
 * Number of LBR PDUs transmitted by the VOE.
 * 
 * This number will be equal to the number of LBM PDUs received by the MEP,
 * assuming that no PDUs were lost during the loop.
 *
 * Register: \a VOP:VOE_STAT:LBR_TX_FRM_CNT
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_LBR_TX_FRM_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,11)


/** 
 * \brief The latest LBR Transaction ID 
 *
 * \details
 * Holds the value of the transaction ID of the last valid expected LBR PDU
 * received by the VOE.
 * 
 * When a valid LBR PDU is received by the VOE, it is validated against the
 * value of this register + 1.
 * 
 * In case the received Transaction ID does not match the expected, it will
 * result in the assertion of the following sticky bit:
 * 
 * VOE_STAT:OAM_RX_STICKY.LBR_TRANSID_ERR_STICKY
 * 
 * Further the following error counter is increased:
 * 
 * VOE_STAT:LBR_RX_TRANSID_ERR_CNT.LBR_RX_TRANSID_ERR_CNT
 * 
 * Register is only updated when: 
 * 
 * * VOE_CONF:OAM_HW_CTRL.LBR_ENA = 1
 * 
 * or
 * 
 * * VOE_CONF:OAM_HW_CTRL.TST_ENA = 1
 *
 * Register: \a VOP:VOE_STAT:LBR_RX_TRANSID_CFG
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_LBR_RX_TRANSID_CFG(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,12)


/** 
 * \brief Count number of received LBR frames
 *
 * \details
 * Counts number of valid LBR PDUs received by the VOE.
 *
 * Register: \a VOP:VOE_STAT:LBR_RX_FRM_CNT
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_LBR_RX_FRM_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,13)


/** 
 * \brief Number of valid LBR frames with transaction ID error.
 *
 * \details
 * Counts number of valid OAM LBR PDUs received by the VOE for which the
 * transaction ID differs from the expected value:
 * 
 * VOE_STAT:LBR_RX_TRANSID_CFG.LBR_RX_TRANSID + 1
 * 
 * When the above counter is increased, the following sticky bit is
 * asserted:
 * 
 * VOE_STAT:OAM_RX_STICKY.LBR_TRANSID_ERR_STICKY
 *
 * Register: \a VOP:VOE_STAT:LBR_RX_TRANSID_ERR_CNT
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_LBR_RX_TRANSID_ERR_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,14)


/** 
 * \brief DM stat counters.
 *
 * \details
 * This register contains counters for the number of valid DM PDUs.
 * 
 * The counters are shared between the following PDU types:
 *  * DMM/DMR
 *  * 1DM
 *
 * Register: \a VOP:VOE_STAT:DM_PDU_CNT
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_DM_PDU_CNT(gi)     VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,15)

/** 
 * \brief
 * Counts the number of valid TX DMM/1DM PDUs transmitted by the VOE
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_DM_PDU_CNT . DMM_TX_PDU_CNT
 */
#define  VTSS_F_VOP_VOE_STAT_DM_PDU_CNT_DMM_TX_PDU_CNT(x)  VTSS_ENCODE_BITFIELD(x,24,8)
#define  VTSS_M_VOP_VOE_STAT_DM_PDU_CNT_DMM_TX_PDU_CNT     VTSS_ENCODE_BITMASK(24,8)
#define  VTSS_X_VOP_VOE_STAT_DM_PDU_CNT_DMM_TX_PDU_CNT(x)  VTSS_EXTRACT_BITFIELD(x,24,8)

/** 
 * \brief
 * Counts the number of valid RX DMR PDUs transmitted by the VOE
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_DM_PDU_CNT . DMM_RX_PDU_CNT
 */
#define  VTSS_F_VOP_VOE_STAT_DM_PDU_CNT_DMM_RX_PDU_CNT(x)  VTSS_ENCODE_BITFIELD(x,16,8)
#define  VTSS_M_VOP_VOE_STAT_DM_PDU_CNT_DMM_RX_PDU_CNT     VTSS_ENCODE_BITMASK(16,8)
#define  VTSS_X_VOP_VOE_STAT_DM_PDU_CNT_DMM_RX_PDU_CNT(x)  VTSS_EXTRACT_BITFIELD(x,16,8)

/** 
 * \brief
 * Counts the number of valid TX DMR PDUs transmitted by the VOE
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_DM_PDU_CNT . DMR_TX_PDU_CNT
 */
#define  VTSS_F_VOP_VOE_STAT_DM_PDU_CNT_DMR_TX_PDU_CNT(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_VOP_VOE_STAT_DM_PDU_CNT_DMR_TX_PDU_CNT     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_VOP_VOE_STAT_DM_PDU_CNT_DMR_TX_PDU_CNT(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Counts the number of valid RX DMR/1DM PDUs received by the VOE.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_DM_PDU_CNT . DMR_RX_PDU_CNT
 */
#define  VTSS_F_VOP_VOE_STAT_DM_PDU_CNT_DMR_RX_PDU_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VOP_VOE_STAT_DM_PDU_CNT_DMR_RX_PDU_CNT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VOP_VOE_STAT_DM_PDU_CNT_DMR_RX_PDU_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief LM stat counters.
 *
 * \details
 * This register contains counters for the number of valid LM PDUs.
 * 
 * The counters are shared between the following PDU types:
 *  * LMM/LMR
 *  * CCM_LM (only counted if CCM_LM_ENA = 1)
 * 
 * Counters are updated regardless of the value of the PDU enable signals:
 *  * VOE_CONF:OAM_HW_ENA.*
 *
 * Register: \a VOP:VOE_STAT:LM_PDU_CNT
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_LM_PDU_CNT(gi)     VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,16)

/** 
 * \brief
 * Counts the number of valid TX LMM / CCM_LM PDUs transmitted by the VOE.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_LM_PDU_CNT . LMM_TX_PDU_CNT
 */
#define  VTSS_F_VOP_VOE_STAT_LM_PDU_CNT_LMM_TX_PDU_CNT(x)  VTSS_ENCODE_BITFIELD(x,24,8)
#define  VTSS_M_VOP_VOE_STAT_LM_PDU_CNT_LMM_TX_PDU_CNT     VTSS_ENCODE_BITMASK(24,8)
#define  VTSS_X_VOP_VOE_STAT_LM_PDU_CNT_LMM_TX_PDU_CNT(x)  VTSS_EXTRACT_BITFIELD(x,24,8)

/** 
 * \brief
 * Counts the number of valid RX LMM PDUs received by the VOE.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_LM_PDU_CNT . LMM_RX_PDU_CNT
 */
#define  VTSS_F_VOP_VOE_STAT_LM_PDU_CNT_LMM_RX_PDU_CNT(x)  VTSS_ENCODE_BITFIELD(x,16,8)
#define  VTSS_M_VOP_VOE_STAT_LM_PDU_CNT_LMM_RX_PDU_CNT     VTSS_ENCODE_BITMASK(16,8)
#define  VTSS_X_VOP_VOE_STAT_LM_PDU_CNT_LMM_RX_PDU_CNT(x)  VTSS_EXTRACT_BITFIELD(x,16,8)

/** 
 * \brief
 * Counts the number of valid TX LMR PDUs transmitted by the VOE.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_LM_PDU_CNT . LMR_TX_PDU_CNT
 */
#define  VTSS_F_VOP_VOE_STAT_LM_PDU_CNT_LMR_TX_PDU_CNT(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_VOP_VOE_STAT_LM_PDU_CNT_LMR_TX_PDU_CNT     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_VOP_VOE_STAT_LM_PDU_CNT_LMR_TX_PDU_CNT(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * Counts the number of valid RX LMR / CCM_LM PDUs received by the VOE.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_LM_PDU_CNT . LMR_RX_PDU_CNT
 */
#define  VTSS_F_VOP_VOE_STAT_LM_PDU_CNT_LMR_RX_PDU_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VOP_VOE_STAT_LM_PDU_CNT_LMR_RX_PDU_CNT     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VOP_VOE_STAT_LM_PDU_CNT_LMR_RX_PDU_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \details
 * The number of frames discarded by the VOE due to :
 * 
 * * MEL filtering
 *
 * Register: \a VOP:VOE_STAT:TX_OAM_DISCARD
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_TX_OAM_DISCARD(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,17)


/** 
 * \details
 * Number of frames discarded by the VOE, due to the following:
 * 
 *  * MEL filtering
 *  * DMAC check
 *  * Version check
 *  * MEL block high
 *  * If SAM_SEQ frames are received on a VOE not enabled for SAM_SEQ
 * processing
 *    (See VOE_CONF:SAM_NON_OAM_SEQ_CFG.*)
 *
 * Register: \a VOP:VOE_STAT:RX_OAM_DISCARD
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_RX_OAM_DISCARD(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,18)


/** 
 * \brief Configures the extraction of PDU errors to the CPU_ERR_QU
 *
 * \details
 * Each of the bit fields in this register represent a specific PDU
 * condition, which will cause the next PDU meeting the condition to be
 * copied to the CPU if the matching bit is asserted.
 * 
 * All the bits - with the exception of "CCM_RX_CCM_NXT_EXTR" - are encoded
 * as either "Extract All" or "Hit Me Once". This depends on the value of
 * bit field: 
 * 
 * EXTRACT_HIT_ME_ONCE
 * 
 * If configured for "Hit Me Once" then the VOE will clear the bit, when a
 * CPU has been received which meets the condition and has been copied to
 * the CPU.
 * 
 * To extract another frame meeting the same condition, the CPU must
 * re-assert the bit.
 * 
 * If not configured for "Hit Me Once" the VOE will not clear the
 * extraction bit and all frames matching the condition will be extracted,
 * until the extraction bit is cleared by SW.
 * 
 * "CCM_RX_CCM_NXT_EXTR" will always be implemented as "Hit Me Once". If
 * all the CCM(-LM) frames are to be extracted, use the PDU specific
 * extract bits:
 * 
 * VOE_CONF:OAM_CPU_COPY_CTRL.*
 *
 * Register: \a VOP:VOE_STAT:PDU_EXTRACT
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_PDU_EXTRACT(gi)    VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,19)

/** 
 * \brief
 * Is used to configure whether the following extract bits are implemented
 * as "Hit Me Once" or "Extract all":
 * 
 *  - CCM_ZERO_PERIOD_RX_ERR_EXTR
 *  - RX_MEL_LOW_ERR_EXTR
 *  - DMAC_RX_ERR_EXTR
 *  - PDU_VERSION_RX_ERR_EXTR
 *  - CCM_MEGID_RX_ERR_EXTR
 *  - CCM_MEPID_RX_ERR_EXTR
 *  - CCM_PERIOD_RX_ERR_EXTR
 *  - CCM_PRIO_RX_ERR_EXTR
 *  - CCM_RX_TLV_NON_ZERO_EXTR
 *  - TX_BLOCK_ERR_EXTR
 *  - SAM_RX_SEQ_ERR_EXTR
 *  - SL_ERR_EXTR
 * 
 * This configuration bit has no effect on the following extraction bit:
 * 
 * "CCM_RX_CCM_NXT_EXTR" 
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_PDU_EXTRACT . EXTRACT_HIT_ME_ONCE
 */
#define  VTSS_F_VOP_VOE_STAT_PDU_EXTRACT_EXTRACT_HIT_ME_ONCE  VTSS_BIT(15)

/** 
 * \brief
 * If asserted, all OAM PDUs which assert the following status bit will be
 * extracted to the CPU:
 * 
 *  - VOE_STAT:CCM_RX_LAST.CCM_ZERO_PERIOD_ERR
 * 
 * PDUs will be extracted to the following CPU queue:
 * 
 *  - OAM_MEP:COMMON:CPU_EXTR_CFG.CPU_ERR_QU.
 * 
 * The extraction will be "Hit Me Once" or "Extract All" depending on the
 * configuration of:
 * 
 *  - VOE_STAT:PDU_EXTRACT.EXTRACT_HIT_ME_ONCE
 *
 * \details 
 * 0: Do not extract CCM PDU failing CCM_ZERO_PERIOD check.
 * 1: Extract CCM PDU failing CCM_ZERO_PERIOD check
 *
 * Field: ::VTSS_VOP_VOE_STAT_PDU_EXTRACT . CCM_ZERO_PERIOD_RX_ERR_EXTR
 */
#define  VTSS_F_VOP_VOE_STAT_PDU_EXTRACT_CCM_ZERO_PERIOD_RX_ERR_EXTR  VTSS_BIT(14)

/** 
 * \brief
 * If asserted, all OAM PDUs which assert the following sticky bit will be
 * extracted to the CPU:
 * 
 *  - VOE_STAT:OAM_RX_STICKY.RX_MEL_LOW_BLOCK_STICKY
 * 
 * PDUs will be extracted to the following CPU queue:
 * 
 *  - OAM_MEP:COMMON:CPU_EXTR_CFG.CPU_ERR_QU.
 * 
 * The extraction will be "Hit Me Once" or "Extract All" depending on the
 * configuration of:
 * 
 *  - VOE_STAT:PDU_EXTRACT.EXTRACT_HIT_ME_ONCE
 *
 * \details 
 * 0: Disable (no CPU hitme once copy)
 * 1: Enable the next CCM_PDU with a second TLV to be send to CPU.
 * 
 * The bit is cleared by HW when a CPU copy has been made.
 *
 * Field: ::VTSS_VOP_VOE_STAT_PDU_EXTRACT . RX_MEL_LOW_ERR_EXTR
 */
#define  VTSS_F_VOP_VOE_STAT_PDU_EXTRACT_RX_MEL_LOW_ERR_EXTR  VTSS_BIT(13)

/** 
 * \brief
 * If asserted, all OAM PDUs which assert the following status bit will be
 * extracted to the CPU:
 * 
 *  - VOE_STAT:OAM_RX_STICKY.RX_MEL_HIGH_BLOCK_STICKY
 * 
 * PDUs will be extracted to the following CPU queue:
 * 
 *  - OAM_MEP:COMMON:CPU_EXTR_CFG.CPU_ERR_QU.
 * 
 * The extraction will be "Hit Me Once" or "Extract All" depending on the
 * configuration of:
 * 
 *  - VOE_STAT:PDU_EXTRACT.EXTRACT_HIT_ME_ONCE
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_PDU_EXTRACT . RX_MEL_HIGH_BLOCK_EXTR
 */
#define  VTSS_F_VOP_VOE_STAT_PDU_EXTRACT_RX_MEL_HIGH_BLOCK_EXTR  VTSS_BIT(12)

/** 
 * \brief
 * If asserted, all OAM PDUs which assert the following sticky bit will be
 * extracted to the CPU:
 * 
 *  - VOE_STAT:OAM_RX_STICKY.DMAC_RX_ERR_STICKY
 * 
 * PDUs will be extracted to the following CPU queue:
 * 
 *  - OAM_MEP:COMMON:CPU_EXTR_CFG.CPU_ERR_QU.
 * 
 * The extraction will be "Hit Me Once" or "Extract All" depending on the
 * configuration of:
 * 
 *  - VOE_STAT:PDU_EXTRACT.EXTRACT_HIT_ME_ONCE
 *
 * \details 
 * 0: Disable (no CPU hitme once copy)
 * 1: Enable the next frame failing the CCM PDU check to be send to CPU.
 * 
 * The bit is cleared by HW when a CPU copy has been made.
 *
 * Field: ::VTSS_VOP_VOE_STAT_PDU_EXTRACT . DMAC_RX_ERR_EXTR
 */
#define  VTSS_F_VOP_VOE_STAT_PDU_EXTRACT_DMAC_RX_ERR_EXTR  VTSS_BIT(11)

/** 
 * \brief
 * If asserted, all OAM PDUs which assert the following sticky bit will be
 * extracted to the CPU:
 * 
 *  - VOE_STAT:OAM_RX_STICKY.PDU_VERSION_RX_ERR_STICKY
 * 
 * PDUs will be extracted to the following CPU queue:
 * 
 *  - OAM_MEP:COMMON:CPU_EXTR_CFG.CPU_ERR_QU.
 * 
 * The extraction will be "Hit Me Once" or "Extract All" depending on the
 * configuration of:
 * 
 *  - VOE_STAT:PDU_EXTRACT.EXTRACT_HIT_ME_ONCE
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_PDU_EXTRACT . PDU_VERSION_RX_ERR_EXTR
 */
#define  VTSS_F_VOP_VOE_STAT_PDU_EXTRACT_PDU_VERSION_RX_ERR_EXTR  VTSS_BIT(10)

/** 
 * \brief
 * If asserted, all OAM PDUs which assert the following status bit will be
 * extracted to the CPU:
 * 
 *  - VOE_STAT:CCM_RX_LAST.CCM_MEGID_ERR
 * 
 * PDUs will be extracted to the following CPU queue:
 * 
 *  - OAM_MEP:COMMON:CPU_EXTR_CFG.CPU_ERR_QU.
 * 
 * The extraction will be "Hit Me Once" or "Extract All" depending on the
 * configuration of:
 * 
 *  - VOE_STAT:PDU_EXTRACT.EXTRACT_HIT_ME_ONCE
 *
 * \details 
 * 0: Disable (no CPU hitme once copy)
 * 1: Enable the next frame to be send to CPU.
 * 
 * The bit is cleared by HW when a CPU copy has been made.
 *
 * Field: ::VTSS_VOP_VOE_STAT_PDU_EXTRACT . CCM_MEGID_RX_ERR_EXTR
 */
#define  VTSS_F_VOP_VOE_STAT_PDU_EXTRACT_CCM_MEGID_RX_ERR_EXTR  VTSS_BIT(9)

/** 
 * \brief
 * If asserted, all OAM PDUs which assert the following status bit will be
 * extracted to the CPU:
 * 
 *  - VOE_STAT:CCM_RX_LAST.CCM_MEPID_ERR
 * 
 * PDUs will be extracted to the following CPU queue:
 * 
 *  - OAM_MEP:COMMON:CPU_EXTR_CFG.CPU_ERR_QU.
 * 
 * The extraction will be "Hit Me Once" or "Extract All" depending on the
 * configuration of:
 * 
 *  - VOE_STAT:PDU_EXTRACT.EXTRACT_HIT_ME_ONCE
 *
 * \details 
 * 0: Disable (no CPU hitme once copy)
 * 1: Enable the next frame failing the MEP direction check to be send to
 * CPU
 * 
 * The bit is cleared by HW when a CPU copy has been made.
 *
 * Field: ::VTSS_VOP_VOE_STAT_PDU_EXTRACT . CCM_MEPID_RX_ERR_EXTR
 */
#define  VTSS_F_VOP_VOE_STAT_PDU_EXTRACT_CCM_MEPID_RX_ERR_EXTR  VTSS_BIT(8)

/** 
 * \brief
 * If asserted, all OAM PDUs which assert the following status bit will be
 * extracted to the CPU:
 * 
 *  - VOE_STAT:CCM_RX_LAST.CCM_PERIOD_ERR
 * 
 * PDUs will be extracted to the following CPU queue:
 * 
 *  - OAM_MEP:COMMON:CPU_EXTR_CFG.CCM_CPU_QU.
 * 
 * The extraction will be "Hit Me Once" or "Extract All" depending on the
 * configuration of:
 * 
 *  - VOE_STAT:PDU_EXTRACT.EXTRACT_HIT_ME_ONCE
 *
 * \details 
 * 0: Disable (no CPU hitme once copy)
 * 1: Enable the next frame failing the MEL check to be send to CPU 
 * 
 * The bit is cleared by HW when a CPU copy has been made.
 *
 * Field: ::VTSS_VOP_VOE_STAT_PDU_EXTRACT . CCM_PERIOD_RX_ERR_EXTR
 */
#define  VTSS_F_VOP_VOE_STAT_PDU_EXTRACT_CCM_PERIOD_RX_ERR_EXTR  VTSS_BIT(7)

/** 
 * \brief
 * If asserted, all OAM PDUs which assert the following status bit will be
 * extracted to the CPU:
 * 
 *  - VOE_STAT:CCM_RX_LAST.CCM_PRIO_ERR
 * 
 * PDUs will be extracted to the following CPU queue:
 * 
 *  - OAM_MEP:COMMON:CPU_EXTR_CFG.CCM_CPU_QU.
 * 
 * The extraction will be "Hit Me Once" or "Extract All" depending on the
 * configuration of:
 * 
 *  - VOE_STAT:PDU_EXTRACT.EXTRACT_HIT_ME_ONCE
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_PDU_EXTRACT . CCM_PRIO_RX_ERR_EXTR
 */
#define  VTSS_F_VOP_VOE_STAT_PDU_EXTRACT_CCM_PRIO_RX_ERR_EXTR  VTSS_BIT(6)

/** 
 * \brief
 * If asserted, all CCM(-LM) PDUs which assert the following sticky bit
 * will be extracted to the CPU:
 * 
 *  - VOE_STAT:OAM_RX_STICKY.CCM_RX_TLV_NON_ZERO_STICKY
 * 
 * PDUs will be extracted to the following CPU queue:
 * 
 * Frames are extracted to one of the following queues depending on whether
 * the PDU is CCM or CCM-LM
 * 
 * OAM_MEP:COMMON:CPU_EXTR_CFG_1.CPU_CCM_QU
 * OAM_MEP:COMMON:CPU_EXTR_CFG_1.CPU_CCM_LM_QU
 * 
 * The extraction will be "Hit Me Once" or "Extract All" depending on the
 * configuration of:
 * 
 *  - VOE_STAT:PDU_EXTRACT.EXTRACT_HIT_ME_ONCE
 * 
 * This bit field allows the next CCM frame with Second TLV to CPU. 
 * (IEEE802.1AG relevant only).
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_PDU_EXTRACT . CCM_RX_TLV_NON_ZERO_EXTR
 */
#define  VTSS_F_VOP_VOE_STAT_PDU_EXTRACT_CCM_RX_TLV_NON_ZERO_EXTR  VTSS_BIT(5)

/** 
 * \brief
 * If asserted the next valid CCM(-LM) frame will be extracted to the CPU.
 * 
 * Frames are extracted to one of the following queues depending on whether
 * the PDU is CCM or CCM-LM
 * 
 * OAM_MEP:COMMON:CPU_EXTR_CFG_1.CPU_CCM_QU
 * OAM_MEP:COMMON:CPU_EXTR_CFG_1.CPU_CCM_LM_QU
 * 
 * This extraction is always implemented as: "Hit Me Once"
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_PDU_EXTRACT . CCM_RX_CCM_NXT_EXTR
 */
#define  VTSS_F_VOP_VOE_STAT_PDU_EXTRACT_CCM_RX_CCM_NXT_EXTR  VTSS_BIT(4)

/** 
 * \brief
 * If asserted by SW, the next valid OAM PDU received of one of the
 * following PDU types will be extracted:
 * 
 *  - TST
 *  - LBR
 *  - NON_OAM_MSG
 * 
 * Only the first PDU is extracted, regardless of which of the three PDU
 * types it is.
 * 
 * When a PDU has been extracted, the bit will be cleared by the VOE.
 * 
 * The PDU is extracted to the queue assigned to the relevant PDU type in
 * the CPU extract queues.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_PDU_EXTRACT . RX_TEST_FRM_NXT_EXTR
 */
#define  VTSS_F_VOP_VOE_STAT_PDU_EXTRACT_RX_TEST_FRM_NXT_EXTR  VTSS_BIT(3)

/** 
 * \brief
 * If asserted, all OAM PDUs which assert the following sticky bit will be
 * extracted to the CPU:
 * 
 *  - VOE_STAT:OAM_TX_STICKY.TX_BLOCK_ERR_STICKY
 * 
 * PDUs will be extracted to the following CPU queue:
 * 
 *  - OAM_MEP:COMMON:CPU_EXTR_CFG.CPU_ERR_QU.
 * 
 * The extraction will be "Hit Me Once" or "Extract All" depending on the
 * configuration of:
 * 
 *  - VOE_STAT:PDU_EXTRACT.EXTRACT_HIT_ME_ONCE
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_PDU_EXTRACT . TX_BLOCK_ERR_EXTR
 */
#define  VTSS_F_VOP_VOE_STAT_PDU_EXTRACT_TX_BLOCK_ERR_EXTR  VTSS_BIT(2)

/** 
 * \brief
 * Extract non OAM sequence numbered frames in the SAM_SEQ Initiator.
 * 
 * Out of sequence non OAM frames is further indicated by asserting the
 * following sticky bit:
 * 
 *  - LBR_TRANSID_ERR_STICKY
 * 
 * The frames which are extracted due to this bit will depend on the config
 * following configuration:
 * 
 *  - VOE_CONF:SAM_NON_OAM_SEQ.SAM_SEQ_RX_ERR_CNT_ENA
 * 
 * PDUs will be extracted to the following CPU queue:
 * 
 *  - OAM_MEP:COMMON:CPU_EXTR_CFG_1.LBR_CPU_QU
 * 
 * The extraction will be "Hit Me Once" or "Extract All" depending on the
 * configuration of:
 * 
 *  - VOE_STAT:PDU_EXTRACT.EXTRACT_HIT_ME_ONCE
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_PDU_EXTRACT . SAM_RX_SEQ_ERR_EXTR
 */
#define  VTSS_F_VOP_VOE_STAT_PDU_EXTRACT_SAM_RX_SEQ_ERR_EXTR  VTSS_BIT(1)

/** 
 * \brief
 * If asserted the VOE will extract frames asserting one of the following
 * sticky bits:
 * 
 *  - OAM_RX_STICKY2.RX_SLM_TESTID_ERR_STICKY
 *  - OAM_RX_STICKY2.RX_SLM_PRIO_ERR_STICKY
 *  - OAM_RX_STICKY2.RX_SLM_MEPID_ERR_STICKY
 *  - OAM_RX_STICKY2.RX_INI_ILLEGAL_MEPID
 *  - OAM_TX_STICKY.TX_SLM_PRIO_ERR_STICKY
 * 
 * The extraction will be "Hit Me Once" or "Extract All" depending on the
 * configuration of:
 * 
 *  - VOE_STAT:PDU_EXTRACT.EXTRACT_HIT_ME_ONCE
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_PDU_EXTRACT . SL_ERR_EXTR
 */
#define  VTSS_F_VOP_VOE_STAT_PDU_EXTRACT_SL_ERR_EXTR  VTSS_BIT(0)


/** 
 * \brief Extraction of SynLM RX PDUs
 *
 * \details
 * Register: \a VOP:VOE_STAT:SYNLM_EXTRACT
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_SYNLM_EXTRACT(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,20)

/** 
 * \brief
 * If asserted, the frame extraction due to a match in the following
 * register :
 * 
 *  - VOE_STAT:SYNLM_EXTRACT.EXTRACT_PEER_RX
 * 
 * will be done "Hit Me Once"
 * 
 * If not asserted all frames matching the above will be extracted to CPU.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_SYNLM_EXTRACT . EXTRACT_HMO
 */
#define  VTSS_F_VOP_VOE_STAT_SYNLM_EXTRACT_EXTRACT_HMO  VTSS_BIT(8)

/** 
 * \brief
 * When the Initiator MEP enabled for SynLM processes RX SLR / 1SL PDUs,
 * they are extracted if so configured in this register.
 * 
 * Configuring this register allows for extracting only RX frames from
 * selected SynLM peer MEPs.
 * 
 * The register width is 8. Each bit represents a SynLM peer MEP.
 * 
 * The SynLM peer MEP associated with an RX PDU is determined from matching
 * the MEPID of the RX SynLM PDU against the configured list of SynLM peer
 * MEPIDs:
 * 
 *  - VOE_CONF:SLM_PEER_LIST(x).SL_PEER_MEPID
 * 
 * If the bit (in the current register) representing the SynLM peer index
 * of the RX frame, the frame is extracted to the CPU.
 * 
 * The extraction will be done either extract ALL or extract "Hit Me Once"
 * depending on the value of the the following bit field:
 * 
 *  - VOE_STAT:SYNLM_EXTRACT.EXTRACT_HMO
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_SYNLM_EXTRACT . EXTRACT_PEER_RX
 */
#define  VTSS_F_VOP_VOE_STAT_SYNLM_EXTRACT_EXTRACT_PEER_RX(x)  VTSS_ENCODE_BITFIELD(x,0,8)
#define  VTSS_M_VOP_VOE_STAT_SYNLM_EXTRACT_EXTRACT_PEER_RX     VTSS_ENCODE_BITMASK(0,8)
#define  VTSS_X_VOP_VOE_STAT_SYNLM_EXTRACT_EXTRACT_PEER_RX(x)  VTSS_EXTRACT_BITFIELD(x,0,8)


/** 
 * \brief TX sticky bits
 *
 * \details
 * Sticky bits asserted
 *
 * Register: \a VOP:VOE_STAT:OAM_TX_STICKY
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_OAM_TX_STICKY(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,21)

/** 
 * \brief
 * This register is asserted if a frame is blocked in the TX direction due
 * to a MEL value which is too low.
 * 
 * A low MEL value will result as one of two conditions:
 * 1) Injection TX PDU
 * If an OAM PDU is injected by the CPU in the TX direction with a MEL
 * different from the MEL configured for the VOE.
 * 
 * 2) Forwarding TX PDU
 * If a frame from a front port (non CPU inject) is forwarded in the TX
 * direction with a MEL which is equal to or lower that the MEL configured
 * for the VOE.
 * 
 * The MEL for the VOE is configured in the following bit field:
 * 
 *  - VOE_CONF:VOE_CTRL.MEL_VAL
 * 
 * The assertion of this Sticky bit depends on the setting of the following
 * bit field:
 * 
 *  - VOE_CONF:OAM_CPU_COPY_CTRL.PDU_ERR_EXTRACT_CCM_ONLY

 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_OAM_TX_STICKY . TX_BLOCK_ERR_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_TX_STICKY_TX_BLOCK_ERR_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * If a SynLM PDU is transmitted with a priority (COSID) which is different
 * from the SynLM PRIO configured for the VOE, 
 * 
 *  - VOE_CONF:SLM_CONFIG.SLM_PRIO
 * 
 * the frame will be marked as invalid and this sticky bit is asserted.
 * 
 * Frames which fail the tx prio test can optionally be extracted to the
 * CPU error queue:
 *  - VOE_STAT:PDU_EXTRACT.SL_ERR_EXTR

 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_OAM_TX_STICKY . TX_SLM_PRIO_ERR_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_TX_STICKY_TX_SLM_PRIO_ERR_STICKY  VTSS_BIT(0)


/** 
 * \brief Rx Sticky bits
 *
 * \details
 * Sticky bits are asserted when a valid OAM PDU is received by the VOE.
 * 
 * The PDU specific RX sticky bits:
 * 
 *  * "xxx"_RX_STICKY
 * 
 * will be asserted even when the PDU is not enabled
 * (VOE_CONF.OAM_HW_CTRL), to allow detecting PDU types which are not
 * expected.
 * 
 * For the remaining sticky bits the VOE will require the PDU type to be
 * enabled, before asserting the sticky bits.
 * 
 * The OAM PDU is considered valid when :
 *  * PDU MEL is equal to the VOE MEL.
 *  * DMAC is verified according to VOE configuration
 * (VOE_CONF:VOE_CTRL.RX_DMAC_CHK_SEL)
 * 
 * For CCM(-LM) frames the following checks are performed to validate the
 * PDU:
 *  * Verify MEGID
 *  * Verify MEPID
 *  * Verify Period is NON Zero
 *
 * Register: \a VOP:VOE_STAT:OAM_RX_STICKY
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_OAM_RX_STICKY(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,22)

/** 
 * \brief
 * Asserted when VOE receives a valid OAM PDU with an OpCode which is
 * configured as a GENERIC OPCODE in the following register:
 * 
 *  - OAM_MEP:COMMON:OAM_GENERIC_CFG.*
 * 
 * There is a separate bit to indicate the reception of each of the
 * configured GENERIC OpCodes.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY . GENERIC_RX_STICKY_MASK
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY_GENERIC_RX_STICKY_MASK(x)  VTSS_ENCODE_BITFIELD(x,24,8)
#define  VTSS_M_VOP_VOE_STAT_OAM_RX_STICKY_GENERIC_RX_STICKY_MASK     VTSS_ENCODE_BITMASK(24,8)
#define  VTSS_X_VOP_VOE_STAT_OAM_RX_STICKY_GENERIC_RX_STICKY_MASK(x)  VTSS_EXTRACT_BITFIELD(x,24,8)

/** 
 * \brief
 * Asserted when VOE receives a valid OAM PDU with an OpCode which is
 * detected as UNKNOWN.
 * 
 * UNKNOWN Opcode is used for OAM PDU types which do not have dedicated HW
 * support and which is not encoded as a GENERIC PDU.
 *
 * \details 
 * '0': No PDU received
 * '1': PDU with specified OPCODE received
 *
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY . UNK_OPCODE_RX_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY_UNK_OPCODE_RX_STICKY  VTSS_BIT(23)

/** 
 * \brief
 * Asserted when VOE receives a valid LTM PDU.
 *
 * \details 
 * '0': No Valid LTM PDU received
 * '1': VALID LTM PDU received
 *
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY . LTM_RX_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY_LTM_RX_STICKY  VTSS_BIT(22)

/** 
 * \brief
 * Asserted when VOE receives a valid LTR PDU
 *
 * \details 
 * '0': No Valid LTR PDU received
 * '1': VALID LTR PDU received
 *
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY . LTR_RX_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY_LTR_RX_STICKY  VTSS_BIT(21)

/** 
 * \brief
 * Asserted when VOE receives a valid LMM PDU
 *
 * \details 
 * '0': No Valid LMM PDU received
 * '1': VALID LMM PDU received
 *
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY . LMM_RX_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY_LMM_RX_STICKY  VTSS_BIT(20)

/** 
 * \brief
 * Asserted when VOE receives a valid LMR PDU
 *
 * \details 
 * '0': No Valid LMR PDU received
 * '1': VALID LMR PDU received
 *
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY . LMR_RX_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY_LMR_RX_STICKY  VTSS_BIT(19)

/** 
 * \brief
 * Asserted when VOE receives a valid TST PDU
 *
 * \details 
 * '0': No Valid TST PDU received
 * '1': VALID TST PDU received
 *
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY . TST_RX_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY_TST_RX_STICKY  VTSS_BIT(18)

/** 
 * \brief
 * Asserted when VOE receives a valid LBM PDU
 *
 * \details 
 * '0': No Valid LBM PDU received
 * '1': VALID LBM PDU received
 *
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY . LBM_RX_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY_LBM_RX_STICKY  VTSS_BIT(17)

/** 
 * \brief
 * Asserted when VOE receives a valid LBR PDU
 *
 * \details 
 * '0': No Valid LBR PDU received
 * '1': VALID LBR PDU received
 *
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY . LBR_RX_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY_LBR_RX_STICKY  VTSS_BIT(16)

/** 
 * \brief
 * Asserted when VOE receives a valid DMM PDU
 *
 * \details 
 * '0': No Valid DMM PDU received
 * '1': VALID DMM PDU received
 *
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY . DMM_RX_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY_DMM_RX_STICKY  VTSS_BIT(15)

/** 
 * \brief
 * Asserted when VOE receives a valid DMR PDU
 *
 * \details 
 * '0': No Valid DMR PDU received
 * '1': VALID DMR PDU received
 *
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY . DMR_RX_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY_DMR_RX_STICKY  VTSS_BIT(14)

/** 
 * \brief
 * Asserted when VOE receives a valid 1DM PDU
 *
 * \details 
 * '0': No Valid 1DM PDU received
 * '1': VALID 1DM PDU received
 *
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY . SDM_RX_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY_SDM_RX_STICKY  VTSS_BIT(13)

/** 
 * \brief
 * Asserted when VOE receives a valid CCM PDU.
 *
 * \details 
 * '0': No Valid CCM PDU received
 * '1': VALID CCM PDU received
 *
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY . CCM_RX_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY_CCM_RX_STICKY  VTSS_BIT(12)

/** 
 * \brief
 * Asserted when VOE receives a valid CCM-LM PDU
 *
 * \details 
 * '0': No Valid CCM-LM PDU received
 * '1': VALID CCM-LM PDU received
 *
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY . CCM_LM_RX_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY_CCM_LM_RX_STICKY  VTSS_BIT(11)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY . SLM_RX_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY_SLM_RX_STICKY  VTSS_BIT(10)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY . SLR_RX_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY_SLR_RX_STICKY  VTSS_BIT(9)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY . SL1_RX_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY_SL1_RX_STICKY  VTSS_BIT(8)

/** 
 * \brief
 * Sticky bit is asserted when the VOE receives a non OAM frame with
 * sequence number.
 * 
 * This requires that the VOE is configured as either SAM_SEQ Initiator: 
 * 
 *  - SAM_NON_OAM_SEQ_CFG.SAM_SEQ_INIT
 * 
 * or as SAM_SEQ Responder:
 * 
 *  - SAM_NON_OAM_SEQ_CFG.SAM_SEQ_RECV
 * 
 * When configured for "SAM_SEQ Initiator" this bit indicates reception of
 * NON_OAM_RSP.
 * When configured for "SAM_SEQ Responder" this bit indicates reception of
 * NON_OAM_MSG.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY . NON_OAM_SEQ_RX_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY_NON_OAM_SEQ_RX_STICKY  VTSS_BIT(7)

/** 
 * \brief
 * Standard LBM/LBR:
 * --------------------------------
 * Valid LBR frame was received with TRANSACTION ID which did not match the
 * expected value of: 
 * 
 *  - VOE_STAT:LBR_RX_TRANSID_CFG.LBR_RX_TRANSID + 1
 * 
 * SAM_SEQ (Non OAM sequence numbering)
 * --------------------------------
 * The assertion of this bit will depend on the config following
 * configuration:
 * 
 *  - VOE_CONF:SAM_NON_OAM_SEQ.SAM_SEQ_RX_ERR_CNT_ENA
 *
 * \details 
 * 0: No LBR with sequence error was received VOE.
 * 1: LBR with sequence error was received by VOE.
 *
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY . LBR_TRANSID_ERR_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY_LBR_TRANSID_ERR_STICKY  VTSS_BIT(6)

/** 
 * \brief
 * The VOE discarded an Rx OAM PDU with correct MEL, which failed the DMAC
 * check configured in :
 * 
 *  - VOE:BASIC_CTRL.RX_DMAC_CHK_SEL
 * 
 * OAM PDUs failing DMAC check can optionally be extracted using:
 *  - VOE_STAT:PDU_EXTRACT.DMAC_RX_ERR_EXTR
 * 
 * Frames failing the DMAC validation can optionally be extracted to the
 * CPU error queue:
 * 
 * OAM_MEP:COMMON:CPU_EXTR_CFG.CPU_ERR_QU
 * 
 * It is configurable if the DMAC check is performed for all PDU types or
 * only for CCM(-LM) PDUs using the following bit field:
 * 
 *  - VOE_CONF:OAM_CPU_COPY_CTRL.PDU_ERR_CHECK_CCM_ONLY
 *
 * \details 
 * 0: No event
 * 1: MAC addr err occured
 *
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY . DMAC_RX_ERR_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY_DMAC_RX_ERR_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * Indicates that a OAM PDU was received by the VOE with a Y.1731 version
 * code differemt from 0, which is the only version currently supported.
 * 
 * Frames with version error are discarded and can optionally be extracted
 * to the CPU using :
 * 
 *  - VOE_STAT:PDU_EXTRACT.PDU_VERSION_RX_ERR_EXTR
 * 
 * Frames extracted to the CPU due to Y.1731 version error are extracted to
 * the CPU error queue:
 * 
 * OAM_MEP:COMMON:CPU_EXTR_CFG.CPU_ERR_QU
 * 
 * It is configurable if the VERSION check is performed for all PDU types
 * or only for CCM(-LM) PDUs using the following bit field:
 * 
 *  - VOE_CONF:OAM_CPU_COPY_CTRL.PDU_ERR_CHECK_CCM_ONLY
 *
 * \details 
 * 0: No PDU with VERSION error received by VOE.
 * 1: PDU with VERSION error received by VOE.

 *
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY . PDU_VERSION_RX_ERR_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY_PDU_VERSION_RX_ERR_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Indicates that an OAM PDU was received by the VOE with a TLV at position
 * 70 which is NON ZERO, indicating that there is a TLV following the
 * CCM(-LM) PDU.
 * 
 * This is a valid option according to 802.1ag.
 * 
 * The additional TLV following the CCM PDU can not be processed by HW, but
 * frames including such a TLV can be extracted to the CPU, using the
 * following register:
 * 
 *  - VOE_STAT:PDU_EXTRACT.CCM_RX_TLV_NON_ZERO_EXTR
 * 
 * Frames are extracted to one of the following queues depending on whether
 * the PDU is CCM or CCM-LM
 * 
 * OAM_MEP:COMMON:CPU_EXTR_CFG_1.CPU_CCM_QU
 * OAM_MEP:COMMON:CPU_EXTR_CFG_1.CPU_CCM_LM_QU

 *
 * \details 
 * 0: No CCM PDU with NON ZERO TLV at position 70 was received by the VOE.
 * 0: CCM PDU with NON ZERO TLV at position 70 was received by the VOE.
 *
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY . CCM_RX_TLV_NON_ZERO_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY_CCM_RX_TLV_NON_ZERO_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * Valid CCM(-LM) PDU with sequence error received by VOE.
 * 
 * Only updated when updating the CCM sequence number is enable:
 * 
 *  - VOE_CONF:CCM_CFG.CCM_SEQ_UPD_ENA
 * 
 * Only updated when :
 * 
 *  * VOE_CONF:OAM_HW_CTRL.CCM_ENA = 1

 *
 * \details 
 * 0: No errors detected in the incoming CCM Seq Number.
 * 1: Detected error in the incoming CCM Seq Number.
 *
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY . CCM_RX_SEQ_ERR_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY_CCM_RX_SEQ_ERR_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * An OAM PDU was discarded because it was received by the VOE with MEL
 * which is lower than the MEL value configure for the MEL.
 * 
 * It is configurable if the RX MEL LOW check is performed for all PDU
 * types or only for CCM(-LM) PDUs using the following bit field:
 * 
 *  - VOE_CONF:OAM_CPU_COPY_CTRL.PDU_ERR_EXTRACT_CCM_ONLY

 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY . RX_MEL_LOW_BLOCK_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY_RX_MEL_LOW_BLOCK_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * This sticky bit is asserted when a frame is discarded because the
 * following option is enabled:
 * 
 * VOE_CONF:VOE_CTRL.BLOCK_MEL_HIGH_RX
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY . RX_MEL_HIGH_BLOCK_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY_RX_MEL_HIGH_BLOCK_STICKY  VTSS_BIT(0)


/** 
 * \brief Rx Sticky bits - continued
 *
 * \details
 * Register: \a VOP:VOE_STAT:OAM_RX_STICKY2
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_OAM_RX_STICKY2(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,23)

/** 
 * \brief
 * The VOE received a SLR frame with a TEST ID which is different from the
 * value programmed for the VOE in the following register:
 * 
 *  - VOE_CONF:SLM_CONFIG.SLM_TEST_ID
 * 
 * If the SLR.TEST_ID does not match the above value, this sticky bit is
 * asserted and the frame is considered invalid.
 * 
 * Frames which fail the TEST_ID check can optionally be extracted to the
 * CPU error queue: 
 *  - VOE_STAT:PDU_EXTRACT.SL_ERR_EXTR
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY2 . RX_SLM_TESTID_ERR_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY2_RX_SLM_TESTID_ERR_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * The VOE received a SLM/SLR/1SL frame with a priority which is different
 * from the value programmed for the VOE in the following register:
 * 
 *  - VOE_CONF:SLM_CONFIG.SLM_PRIO
 * 
 * Frames which fail the SynLM PRIO check can optionally be extracted to
 * the CPU error queue: 
 *  - VOE_STAT:PDU_EXTRACT.SL_ERR_EXTR
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY2 . RX_SLM_PRIO_ERR_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY2_RX_SLM_PRIO_ERR_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * When SynLM frames are received from a SynLM peer MEP, the VOE will
 * verify the MEPID of the incoming PDU against the list of valid peer
 * MEPIDs programmed in the VOE:
 * 
 *  - VOE_CONF:SLM_PEER_LIST.SLM_PEER_MEPID
 * 
 * If no match is found, this sticky bit is asserted and the frame is
 * considered invalid.
 * 
 * Whether the VOE matches the SRC_MEPID or the RCV_MEPID depends on the
 * PDU type and the direction of the PDU (RX/TX).
 * 
 * Frames which fail the SynLM peer MEPID check can optionally be extracted
 * to the CPU error queue: 
 *  - VOE_STAT:PDU_EXTRACT.SL_ERR_EXTR
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY2 . RX_SLM_MEPID_ERR_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY2_RX_SLM_MEPID_ERR_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * When an SLR PDU is received by an Initiator MEP, the VOE will verify
 * that the SLR.SRC_MEPID of the incoming frame matches the Initiators own
 * MEPID:
 * 
 *  - VOE_CONF:SLM_CONFIG.SLM_MEPID
 * 
 * If the SLR.SRC_MEPID does not match the above value, this sticky bit is
 * asserted and the frame is considered invalid.
 * 
 * Frames which fail the SynLM Initiator MEP MEPID check can optionally be
 * extracted to the CPU error queue: 
 *  - VOE_STAT:PDU_EXTRACT.SL_ERR_EXTR
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_OAM_RX_STICKY2 . RX_INI_ILLEGAL_MEPID_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_OAM_RX_STICKY2_RX_INI_ILLEGAL_MEPID_STICKY  VTSS_BIT(0)


/** 
 * \brief Misc. CCM statistics
 *
 * \details
 * This bit fields in this register contain misc. CCM(-LM) statistics.
 *
 * Register: \a VOP:VOE_STAT:CCM_STAT
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_CCM_STAT(gi)       VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,24)

/** 
 * \brief
 * If this bitfield is asserted, CCM_LM info will be inserted into next CCM
 * frame transmitted by the VOE.
 * 
 * (NOTE: OAM_HW_CTRL.CCM_LM_ENA must be asserted prior to configuring this
 * register.)
 * 
 * Upon transmission of CCM-LM information, the bit is cleared by HW.
 * 
 * This bitfield is automatically asserted by HW every time CCM_LM_PERIOD
 * has occured.
 * 
 * Can also be forced by SW writing a '1' to this register.
 * 
 * This function can be used to implement Dual Ended LM flow, by inserting
 * LM information into the CCM frames with a give period, determinde by the
 * setting in:
 * 
 * VOE_CONF:CCM_CFG.CCM_LM_PERIOD
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_CCM_STAT . CCM_LM_INSERT_NXT
 */
#define  VTSS_F_VOP_VOE_STAT_CCM_STAT_CCM_LM_INSERT_NXT  VTSS_BIT(7)

/** 
 * \brief
 * This configures the value to be inserted in the RDI field in the next
 * Valid CCM(-LM) PDU being transmitted by this VOE.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_CCM_STAT . CCM_TX_RDI
 */
#define  VTSS_F_VOP_VOE_STAT_CCM_STAT_CCM_TX_RDI  VTSS_BIT(6)

/** 
 * \brief
 * Specifies the number of CCM half periods has passed without CCM mesages
 * received from the peer MEP.
 * 
 * Cleared by HW when the VOE receives a valid CCM PDU from the peer
 * associated with MEP.
 * 
 * It will be periodically incremented by the LOC timer configured by the
 * following register:
 * 
 * OAM_MEP:VOE_CONF:CCM_CFG.CCM_PERIOD
 * 
 * When the CCM_MISS counter is incremented to 7, an interrupt is generated
 * if so configured in the following bit field:
 * 
 * OAM_MEP:VOE_STAT:INTR_ENA.CCM_LOC_INTR_ENA
 *
 * \details 
 * n<7: No Loss of continuity
 * n==7: Loss of continuity
 *
 * Field: ::VTSS_VOP_VOE_STAT_CCM_STAT . CCM_MISS_CNT
 */
#define  VTSS_F_VOP_VOE_STAT_CCM_STAT_CCM_MISS_CNT(x)  VTSS_ENCODE_BITFIELD(x,3,3)
#define  VTSS_M_VOP_VOE_STAT_CCM_STAT_CCM_MISS_CNT     VTSS_ENCODE_BITMASK(3,3)
#define  VTSS_X_VOP_VOE_STAT_CCM_STAT_CCM_MISS_CNT(x)  VTSS_EXTRACT_BITFIELD(x,3,3)

/** 
 * \brief
 * The VOE will count the number of consecutive CCM(-LM) PDUs received on a
 * front port.
 * 
 * This is used for implementing E-line services over networks using
 * Ethernet Ring protection.
 * 
 * The number of consecutive frames recieved on the same port (not counting
 * the first one) are counted by this counter (CCM_RX_SRC_PORT_CNT).
 * 
 * This front port on which the last valid CCM(-LM) PDU was received is
 * stored in:
 * 
 *  - VOE_STAT:CCM_RX_LAST.CCM_RX_SRC_PORT
 * 
 * When the CPU flushes the MAC table, it can instruct the VOE to detect
 * when CCM(-LM) PDUs are received a number of times on the same front
 * port.
 * 
 * When a number of consecutive CCM(-LM) PDUs have been received on the
 * same front port, the following sticky bit is asserted:
 * 
 *  - VOE_STAT:INTR_STICKY.CCM_RX_SRC_PORT_DETECT_STICKY
 * 
 * This can optionally generate an interrupt.
 * 
 * The number of consecutive CCM(-LM) PDUs which can must be received on
 * the same front port before asserting the sticky bit is configured in the
 * following bitfield:
 * 
 *  - VOP:COMMON:VOP_CTRL.CCM_RX_SRC_PORT_DETECT_CNT
 * 
 * This field is updated by the VOE, and should never be altered by the CPU
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_CCM_STAT . CCM_RX_SRC_PORT_CNT
 */
#define  VTSS_F_VOP_VOE_STAT_CCM_STAT_CCM_RX_SRC_PORT_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_VOP_VOE_STAT_CCM_STAT_CCM_RX_SRC_PORT_CNT     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_VOP_VOE_STAT_CCM_STAT_CCM_RX_SRC_PORT_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Misc. CCM(-LM) statistics for Tx and Rx frames.
 *
 * \details
 * This register contains a number of status bits.
 * 
 * Each status bit indicates the latest result of a specific check
 * performed on RX / TX PDUs.
 * 
 * Each of the status bits in this register corresponds to a sticky bit in
 * the following register:
 * 
 *  - VOE_STAT:INTR_STICKY.*
 * 
 * When ever a status bit this register (CCM_RX_LAST) changes its value,
 * the corresponding sticky bit is asserted.
 * 
 * The bits in this register are status bits updated by the VOE, and the
 * values should never be altered by the CPU, because they must represent
 * the state of the latest valid CCM PDU.
 *
 * Register: \a VOP:VOE_STAT:CCM_RX_LAST
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_CCM_RX_LAST(gi)    VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,25)

/** 
 * \brief
 * If the value of the PERIOD field in a CCM(-LM) payload received is ZERO,
 * the CCM(-LM) frame is invalid.
 * 
 * The value of the period field received with all otherwise valid CCM(-LM)
 *  frames, is checked for a non ZERO value.
 * 
 * This register holds the value of the latest check performed by the VOE.
 * 
 * If the result of the ZERO period check changes, the corresponding sticky
 * bit is asserted: 
 * 
 *  - VOE_STAT:STICKY.CCM_ZERO_PERIOD_STICKY
 * 
 * This allows interrupt generation when result of the incoming
 * CCM_ZERO_PERIOD check changes.
 * 
 * This check will be done for CCM(-LM) frames only.
 * 
 * CCM PDUs received by the VOE, which fail the CCM_ZERO_PERIOD
 * verification can optionally be extracted to the CPU using the following
 * bit field:
 * 
 *  - VOE_STAT:PDU_EXTRACT.CCM_ZERO_PERIOD_ERR_EXTR
 * 
 * Extracted CCM PDUs are extracted to the following CPU queue:
 * 
 *  - OAM_MEP:COMMON:CPU_EXTR_CFG.CPU_ERR_QU
 *
 * \details 
 * 0: No Zero period err
 * 1: Zero period err detected
 *
 * Field: ::VTSS_VOP_VOE_STAT_CCM_RX_LAST . CCM_ZERO_PERIOD_ERR
 */
#define  VTSS_F_VOP_VOE_STAT_CCM_RX_LAST_CCM_ZERO_PERIOD_ERR  VTSS_BIT(5)

/** 
 * \brief
 * If the MEL value of an RX OAM PDU is lower than the MEL configured for
 * the VOE, this is an RX_MEL_LOW error.
 * 
 * This register holds the result of the latest RX_MEL_LOW check performed
 * by the VOE.
 * 
 * The PDUs verified for correct MEL level can be configured using the
 * below register:
 *  
 *  - VOE_CONF:OAM_CPU_COPY_CTRL.PDU_ERR_CHECK_CCM_ONLY
 * 
 * If the RX_MEL_LOW verification fails, the following sticky bit is
 * asserted:
 * 
 *  - VOE_STAT:STICKY.RX_MEL_LOW_ERR_STICKY
 * 
 * This allows interrupt generation when the result of the incoming
 * RX_MEL_LOW check changes.
 * 
 * OAM PDUs received by the VOE, which fail the RX_MEL_LOW verification can
 * optionally be extracted to the CPU using the following bit field:
 * 
 *  - VOE_STAT:PDU_EXTRACT.RX_MEL_LOW_ERR_EXTR
 * 
 * Extracted CCM PDUs are extracted to the following CPU queue:
 * 
 *  - OAM_MEP:COMMON:CPU_EXTR_CFG.CPU_ERR_QU
 *
 * \details 
 * 0: No MEL err
 * 1: MEL err detected
 *
 * Field: ::VTSS_VOP_VOE_STAT_CCM_RX_LAST . CCM_RX_MEL_LOW_ERR
 */
#define  VTSS_F_VOP_VOE_STAT_CCM_RX_LAST_CCM_RX_MEL_LOW_ERR  VTSS_BIT(0)

/** 
 * \brief
 * If the value of the MEGID field in a CCM(-LM) payload received is
 * different from the MEGID value configured for the VOE, the CCM(-LM)
 * frame is invalid.
 * 
 * The MEG ID is configured in :
 * 
 *  - VOE_CONF:CCM_MEGID_CFG.*
 * 
 * This register holds the value of the latest MEG ID check performed by
 * the VOE.
 * 
 * If the result of the MEG ID check changes, the corresponding sticky bit
 * is asserted: 
 * 
 *  - VOE_STAT:STICKY.CCM_MEGID_RX_ERR_STICKY
 * 
 * This allows interrupt generation when result of the incoming CCM_MEGID
 * check changes.
 * 
 * This check will be done for CCM(-LM) frames only.
 * 
 * MEGID checking is enabled by :
 * 
 * VOE:CCM_CFG.CCM_MEGID_CHK_ENA = 1
 * 
 * CCM PDUs received by the VOE, which fail the MEG ID verification can
 * optionally be extracted to the CPU using the following bit field:
 * 
 *  - VOE_STAT:PDU_EXTRACT.CCM_MEGID_RX_ERR_EXTR
 * 
 * Extracted CCM PDUs are extracted to the following CPU queue:
 * 
 *  - OAM_MEP:COMMON:CPU_EXTR_CFG.CPU_ERR_QU
 *
 * \details 
 * 0: No MEGID error detected by VOE.
 * 1: MEGID error detected by VOE.
 *
 * Field: ::VTSS_VOP_VOE_STAT_CCM_RX_LAST . CCM_MEGID_ERR
 */
#define  VTSS_F_VOP_VOE_STAT_CCM_RX_LAST_CCM_MEGID_ERR  VTSS_BIT(1)

/** 
 * \brief
 * If the value of the MEPID field in a CCM(-LM) payload received is
 * different from the MEPID value configured for the VOE, the CCM(-LM)
 * frame is invalid.
 * 
 * The MEP ID is configured in :
 * 
 *  - VOE_CONF:CCM_MEPID_CFG.*
 * 
 * This register holds the value of the latest MEP ID check performed by
 * the VOE.
 * 
 * If the result of the MEP ID check changes, the corresponding sticky bit
 * is asserted: 
 * 
 *  - VOE_STAT:STICKY.CCM_MEPID_RX_ERR_STICKY
 * 
 * This allows interrupt generation when result of the incoming CCM_MEPID
 * check changes.
 * 
 * This check will be done for CCM(-LM) frames only.
 * 
 * MEP ID check is enabled by setting: 
 * 
 * VOE:CCM_CFG.CCM_MEPID_CHK_ENA = 1
 * 
 * CCM PDUs received by the VOE, which fail the MEP ID verification can
 * optionally be extracted to the CPU using the following bit field:
 * 
 *  - VOE_STAT:PDU_EXTRACT.CCM_MEPID_RX_ERR_EXTR
 * 
 * Extracted CCM PDUs are extracted to the following CPU queue:
 * 
 *  - OAM_MEP:COMMON:CPU_EXTR_CFG.CPU_ERR_QU
 *
 * \details 
 * 0: No MEPID err
 * 1: MEPID err detected
 *
 * Field: ::VTSS_VOP_VOE_STAT_CCM_RX_LAST . CCM_MEPID_ERR
 */
#define  VTSS_F_VOP_VOE_STAT_CCM_RX_LAST_CCM_MEPID_ERR  VTSS_BIT(2)

/** 
 * \brief
 * For all valid CCM(-LM) frames received by the VOE, the value of the
 * CCM(-LM) PERIOD field is validated against the CCM_PERIOD configured for
 * the VOE:
 * 
 *  - VOE_CONF:CCM_CFG.CCM_PERIOD
 * 
 * This bit fields reflects the result of this check for the last CCM(-LM)
 * frame received by the VOE.
 * 
 * If the value of the CCM_PERIOD check changes, the corresponding STICKY
 * bit is asserted:
 * 
 *  - VOE_STAT:STICKY.CCM_PERIOD_RX_ERR_STICKY
 * 
 * This allows interrupt generation when result of the incoming CCM_PERIOD
 * check changes.
 * 
 * CCM PDUs received by the VOE, which fail the CCM PERIOD verification can
 * optionally be extracted to the CPU using the following bit field:
 * 
 *  - VOE_STAT:PDU_EXTRACT.CCM_PERIOD_RX_ERR_EXTR
 * 
 * Extracted CCM PDUs are extracted to the following CPU queue:
 * 
 *  - OAM_MEP:COMMON:CPU_EXTR_CFG.CPU_ERR_QU
 *
 * \details 
 * 0: No non-ZERO period error detected in the last valid CCM(-LM) frame
 * received
 * 1: Detected non-ZERO period error in the last valid CCM(-LM) frame
 * received
 *
 * Field: ::VTSS_VOP_VOE_STAT_CCM_RX_LAST . CCM_PERIOD_ERR
 */
#define  VTSS_F_VOP_VOE_STAT_CCM_RX_LAST_CCM_PERIOD_ERR  VTSS_BIT(7)

/** 
 * \brief
 * For all valid CCM(-LM) frames received by the VOE, the value of the
 * CCM(-LM) frame priority is validated against the CCM_PRIO configured for
 * the VOE:
 * 
 *  - VOE_CONF:CCM_CFG.CCM_PRIO
 * 
 * This bit fields reflects the result of this check for the last CCM(-LM)
 * frame received by the VOE.
 * 
 * If the value of the CCM_PRIO check changes, the corresponding STICKY bit
 * is asserted:
 * 
 *  - VOE_STAT:STICKY.CCM_PRIO_STICKY
 * 
 * CCM PDUs received by the VOE, which fail the CCM PRIORITY verification
 * can optionally be extracted to the CPU using the following bit field:
 * 
 *  - VOE_STAT:PDU_EXTRACT.CCM_PRIO_RX_ERR_EXTR
 * 
 * Extracted CCM PDUs are extracted to the following CPU queue:
 * 
 *  - OAM_MEP:COMMON:CPU_EXTR_CFG.CPU_ERR_QU
 *
 * \details 
 * 0: No priority error in the last valid CCM(-LM) frame received by this
 * VOE.
 * 1: Priority err detected in the last valid CCM(-LM) frame received by
 * this VOE.
 *
 * Field: ::VTSS_VOP_VOE_STAT_CCM_RX_LAST . CCM_PRIO_ERR
 */
#define  VTSS_F_VOP_VOE_STAT_CCM_RX_LAST_CCM_PRIO_ERR  VTSS_BIT(6)

/** 
 * \brief
 * Status of current Loss Of Continuity interrupt.
 *
 * \details 
 * 0: No loss of continuity
 * 1: Loss of continuity defect persist
 *
 * Field: ::VTSS_VOP_VOE_STAT_CCM_RX_LAST . CCM_LOC_DEFECT
 */
#define  VTSS_F_VOP_VOE_STAT_CCM_RX_LAST_CCM_LOC_DEFECT  VTSS_BIT(3)

/** 
 * \brief
 * The value of RDI bit received with the last valid CCM(-LM) frame.
 * 
 * When this status changes, the following sticky bit will be asserted:
 * 
 *  - VOE_STAT:STICKY.CCM_RX_RDI_STICKY
 * 
 * This allows interrupt generation when the incoming RDI stat changes.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_CCM_RX_LAST . CCM_RX_RDI
 */
#define  VTSS_F_VOP_VOE_STAT_CCM_RX_LAST_CCM_RX_RDI  VTSS_BIT(4)

/** 
 * \brief
 * The source port on which the last CCM(-LM) PDU was received.
 * 
 * This is used for implementing E-line services over networks using
 * Ethernet Ring protection.
 * 
 * When the CPU flushes the MAC table, it can instruct the VOE to detect
 * when CCM(-LM) PDUs are received a number of times on the same front
 * port.
 * 
 * This register contains the latest front port on which a valid CCM(-LM)
 * PDU was received.
 * The number of consecutive frames recieved on the same port (not counting
 * the first one) are counted by the following counter:
 * 
 *  - VOE_STAT:CCM_STAT.CCM_RX_SRC_PORT_CNT
 * 
 * When a number of consecutive CCM(-LM) PDUs have been received on the
 * same front port, the following sticky bit is asserted:
 * 
 *  - VOE_STAT:INTR_STICKY.CCM_RX_SRC_PORT_DETECT_STICKY
 * 
 * This can optionally generate an interrupt.
 * 
 * The number of consecutive CCM(-LM) PDUs which can must be received on
 * the same front port before asserting the sticky bit is configured in the
 * following bitfield:
 * 
 *  - VOP:COMMON:VOP_CTRL.CCM_RX_SRC_PORT_DETECT_CNT
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_CCM_RX_LAST . CCM_RX_SRC_PORT
 */
#define  VTSS_F_VOP_VOE_STAT_CCM_RX_LAST_CCM_RX_SRC_PORT(x)  VTSS_ENCODE_BITFIELD(x,8,6)
#define  VTSS_M_VOP_VOE_STAT_CCM_RX_LAST_CCM_RX_SRC_PORT     VTSS_ENCODE_BITMASK(8,6)
#define  VTSS_X_VOP_VOE_STAT_CCM_RX_LAST_CCM_RX_SRC_PORT(x)  VTSS_EXTRACT_BITFIELD(x,8,6)


/** 
 * \brief CCM(-LM) sticky status indications
 *
 * \details
 * Each of the sticky bits in this register is closely related to a
 * specific status bit in the following register:
 * 
 *  - VOE_STAT:CCM_RX_LAST.*
 * 
 * The above register contains a number of status bits. Each status bit
 * indicates the latest result of a specific check performed on incoming
 * PDUs.
 * 
 * If the value of one of these status bits change, the corresponding
 * sticky bit in this register is asserted.
 * 
 * Hence if the RDI bit of the incoming CCM frames change from '1' --> '0',
 * this will cause the following changes:
 * 
 *  - VOE_STAT:CCM_RX_LAST.CCM_RX_RDI is modified: '1' --> '0'
 *  - VOE_STAT:INTR_STICKY.CCM_RX_RDI_STICKY is asserted.
 * 
 * Each of the sticky bits in this register can optionally be configured to
 * generate an interrupt, using the following register:
 * 
 *  - VOE_CONF:INTR_ENA.*
 * 
 * Sticky bits is only asserted when :
 *  * VOE_CONF:OAM_HW_CTRL.CCM_ENA = 1

 *
 * Register: \a VOP:VOE_STAT:INTR_STICKY
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_INTR_STICKY(gi)    VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,26)

/** 
 * \brief
 * Indicate change in the status of the ZERO period check of incoming
 * CCM(-LM) frames. The value of the incoming CCM(-LM) frame is  checked
 * against the value contained in:
 * 
 *  - CCM_STAT::CCM_ZERO_PERIOD_ERR
 * 
 * The sticky bit can optionally be configured to generate an interrupt:
 * 
 *  - VOE_CONF:INTR_ENA.CCM_ZERO_PERIOD_INTR_ENA
 * 
 * (According to G.8021 CCM(-LM) frames with PERIOD = 0 is an illegal
 * value.)The sticky bit can optionally be configured to generate an
 * interrupt:
 *
 * \details 
 * 0: No event
 * 1: Changed non zero Period occured
 *
 * Field: ::VTSS_VOP_VOE_STAT_INTR_STICKY . CCM_ZERO_PERIOD_RX_ERR_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_INTR_STICKY_CCM_ZERO_PERIOD_RX_ERR_STICKY  VTSS_BIT(7)

/** 
 * \brief
 * Indicate change in the status of the RW_MEL_LOW check of incoming
 * CCM(-LM) frames. 
 * 
 * The value of the incoming CCM(-LM) frame is	checked against the value
 * contained in:
 * 
 *  - CCM_RX_LAST::OAM_RX_MEL_LOW_ERR
 * 
 * The sticky bit can optionally be configured to generate an interrupt:
 * 
 *  - VOE_CONF:INTR_ENA.RX_MEL_LOW_INTR_ENA

 *
 * \details 
 * 0: No event
 * 1: Changed CCM Priority occured
 *
 * Field: ::VTSS_VOP_VOE_STAT_INTR_STICKY . CCM_RX_MEL_LOW_ERR_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_INTR_STICKY_CCM_RX_MEL_LOW_ERR_STICKY  VTSS_BIT(6)

/** 
 * \brief
 * Indicate change in the status of the MEG-ID check of incoming CCM(-LM)
 * frames. The value of the incoming CCM(-LM) frame is	checked against the
 * value contained in:
 * 
 *  - CCM_RX_LAST:CCM_MEGID_ERR
 * 
 * The sticky bit can optionally be configured to generate an interrupt:
 * 
 *  - VOE_CONF:INTR_ENA.CCM_MEGID_INTR_ENA

 *
 * \details 
 * 0: No event
 * 1: Changed Zero Period occured
 *
 * Field: ::VTSS_VOP_VOE_STAT_INTR_STICKY . CCM_MEGID_RX_ERR_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_INTR_STICKY_CCM_MEGID_RX_ERR_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * Indicate change in the status of the MEP-ID check of incoming CCM(-LM)
 * frames. The value of the incoming CCM(-LM) frame is	checked against the
 * value contained in:
 * 
 *  - CCM_RX_LAST:CCM_MEPID_ERR
 * 
 * The sticky bit can optionally be configured to generate an interrupt:
 * 
 *  - VOE_CONF:INTR_ENA.CCM_MEPID_INTR_ENA
 * 
 * Sticky bit is only asserted when :
 *  * VOE_CONF:OAM_HW_CTRL.CCM_ENA = 1
 *
 * \details 
 * 0: No event
 * 1: Changed in RDI occured
 *
 * Field: ::VTSS_VOP_VOE_STAT_INTR_STICKY . CCM_MEPID_RX_ERR_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_INTR_STICKY_CCM_MEPID_RX_ERR_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Indicate change in the status of the PERIOD check of incoming CCM(-LM)
 * frames. The value of the incoming CCM(-LM) frame is	checked against the
 * value contained in:
 * 
 *  - CCM_RX_LAST::CCM_PERIOD_ERR
 * 
 * The sticky bit can optionally be configured to generate an interrupt:
 * 
 *  - VOE_CONF:INTR_ENA.CCM_PERIOD_INTR_ENA

 *
 * \details 
 * 0: LOC has not changed state
 * 1: Changed LOC state (LOC is indicated based on whether CCM_MISS_CNT is
 * less than 7)
 *
 * Field: ::VTSS_VOP_VOE_STAT_INTR_STICKY . CCM_PERIOD_RX_ERR_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_INTR_STICKY_CCM_PERIOD_RX_ERR_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * Indicate change in the status of the PRIORITY check of incoming CCM(-LM)
 * frames. The value of the incoming CCM(-LM) frame is	checked against the
 * value contained in:
 * 
 *  - CCM_RX_LAST::CCM_PRIO_ERR
 * 
 * The sticky bit can optionally be configured to generate an interrupt:
 * 
 *  - VOE_CONF:INTR_ENA.CCM_PRIO_INTR_ENA
 *
 * \details 
 * 0: No event
 * 1: PDU with changed MEPID seen
 *
 * Field: ::VTSS_VOP_VOE_STAT_INTR_STICKY . CCM_PRIO_RX_ERR_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_INTR_STICKY_CCM_PRIO_RX_ERR_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Indicates when the Loss Of Continuity (LOC) status of this VOE changes.
 * 
 * The current LOC status is contained in :
 * 
 *  - VOE_STAT:CCM_RX_LAST:CCM_LOC_DEFECT
 * 
 * The sticky bit can optionally be configured to generate an interrupt:
 * 
 *  - VOE_CONF:INTR_ENA.CCM_LOC_ENA

 *
 * \details 
 * 0: No event
 * 1: PDU with changed MEGID seen
 *
 * Field: ::VTSS_VOP_VOE_STAT_INTR_STICKY . CCM_LOC_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_INTR_STICKY_CCM_LOC_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Indicate change in the status of the RDI check of incoming CCM(-LM)
 * frames. The value of the incoming CCM(-LM) frame is	checked against the
 * value contained in:
 * 
 *  - CCM_RX_LAST::CCM_RX_RDI
 * 
 * The sticky bit can optionally be configured to generate an interrupt:
 * 
 *  - VOE_CONF:INTR_ENA.CCM_RX_RDI_INTR_ENA
 *
 * \details 
 * 0: No event
 * 1: PDU with unexpected MEL seen
 *
 * Field: ::VTSS_VOP_VOE_STAT_INTR_STICKY . CCM_RX_RDI_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_INTR_STICKY_CCM_RX_RDI_STICKY  VTSS_BIT(0)

/** 
 * \brief
 * The number of succesive OAM PDUs received on the same port are counted
 * in the following register:
 * 
 *  - VOE_STAT:CCM_STAT.CCM_RX_SRC_PORT_CNT
 * 
 * When this value reaches the value configured in :
 * 
 *  - OAM_MEP:COMMON:VOP_CTRL.CCM_RX_SRC_PORT_DETECT_CNT
 * 
 * This sticky bit is asserted.
 * 
 * This sticky bit can optionally generate an interrupt depending on the
 * following bit field:
 * 
 *  - VOE_STAT:CCM_RX_SRC_PORT_DETECT_INTR_ENA
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_INTR_STICKY . CCM_RX_SRC_PORT_DETECT_STICKY
 */
#define  VTSS_F_VOP_VOE_STAT_INTR_STICKY_CCM_RX_SRC_PORT_DETECT_STICKY  VTSS_BIT(8)


/** 
 * \brief Enable VOE interrupt sources
 *
 * \details
 * This register contains a bitfield for each of the interrupt sources
 * defined for the VOE.
 * This allows enabling the interrupts independently.
 * 
 * Status of interrupt sources can be found in OAM_MEP:VOE_STAT:INTR_STICKY
 * 
 * Current status of the VOE interrupt can be found in OAM_MEP:COMMON:INTR
 *
 * Register: \a VOP:VOE_STAT:INTR_ENA
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_INTR_ENA(gi)       VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,27)

/** 
 * \brief
 * Enables interrupt generation when the following sticky bit is asserted:
 * 
 *  - VOE_STAT:INTR_STICKY.CCM_ZERO_PERIOD_RX_ERR_STICKY
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_VOP_VOE_STAT_INTR_ENA . CCM_ZERO_PERIOD_INTR_ENA
 */
#define  VTSS_F_VOP_VOE_STAT_INTR_ENA_CCM_ZERO_PERIOD_INTR_ENA  VTSS_BIT(5)

/** 
 * \brief
 * Enables interrupt generation when the following sticky bit is asserted:
 * 
 *  - VOE_STAT:INTR_STICKY.RX_MEL_LOW_ERR_STICKY
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_VOP_VOE_STAT_INTR_ENA . CCM_RX_MEL_LOW_INTR_ENA
 */
#define  VTSS_F_VOP_VOE_STAT_INTR_ENA_CCM_RX_MEL_LOW_INTR_ENA  VTSS_BIT(0)

/** 
 * \brief
 * Enables interrupt generation when the following sticky bit is asserted:
 * 
 *  - VOE_STAT:INTR_STICKY.CCM_MEGID_RX_ERR_STICKY
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_VOP_VOE_STAT_INTR_ENA . CCM_MEGID_INTR_ENA
 */
#define  VTSS_F_VOP_VOE_STAT_INTR_ENA_CCM_MEGID_INTR_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enables interrupt generation when the following sticky bit is asserted:
 * 
 *  - VOE_STAT:INTR_STICKY.CCM_MEPID_RX_ERR_STICKY
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_VOP_VOE_STAT_INTR_ENA . CCM_MEPID_INTR_ENA
 */
#define  VTSS_F_VOP_VOE_STAT_INTR_ENA_CCM_MEPID_INTR_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Enables interrupt generation when the following sticky bit is asserted:
 * 
 *  - VOE_STAT:INTR_STICKY.CCM_PERIOD_RX_ERR_STICKY
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_VOP_VOE_STAT_INTR_ENA . CCM_PERIOD_INTR_ENA
 */
#define  VTSS_F_VOP_VOE_STAT_INTR_ENA_CCM_PERIOD_INTR_ENA  VTSS_BIT(7)

/** 
 * \brief
 * Enables interrupt generation when the following sticky bit is asserted:
 * 
 *  - VOE_STAT:INTR_STICKY.CCM_PRIO_RX_ERR_STICKY
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_VOP_VOE_STAT_INTR_ENA . CCM_PRIO_INTR_ENA
 */
#define  VTSS_F_VOP_VOE_STAT_INTR_ENA_CCM_PRIO_INTR_ENA  VTSS_BIT(6)

/** 
 * \brief
 * Enables interrupt generation when the following sticky bit is asserted:
 * 
 *  - VOE_STAT:INTR_STICKY.CCM_LOC_STICKY
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_VOP_VOE_STAT_INTR_ENA . CCM_LOC_INTR_ENA
 */
#define  VTSS_F_VOP_VOE_STAT_INTR_ENA_CCM_LOC_INTR_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Enables interrupt generation when the following sticky bit is asserted:
 * 
 *  - VOE_STAT:INTR_STICKY.CCM_RX_RDI_STICKY
 *
 * \details 
 * 0: Disable interrupt
 * 1: Enable interrupt
 *
 * Field: ::VTSS_VOP_VOE_STAT_INTR_ENA . CCM_RX_RDI_INTR_ENA
 */
#define  VTSS_F_VOP_VOE_STAT_INTR_ENA_CCM_RX_RDI_INTR_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Enables interrupt generation when the following sticky bit is asserted:
 * 
 *  - VOE_STAT:INTR_STICKY.CCM_RX_SRC_PORT_DETECT_STICKY
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_STAT_INTR_ENA . CCM_RX_SRC_PORT_DETECT_INTR_ENA
 */
#define  VTSS_F_VOP_VOE_STAT_INTR_ENA_CCM_RX_SRC_PORT_DETECT_INTR_ENA  VTSS_BIT(8)


/** 
 * \brief SynLM Tx frame counter.
 *
 * \details
 * TX counter for counting Initiator MEP TX SynLM PDUs (SLM / SL1).
 * 
 * The counter value is written into the the following fields of TX SynLM
 * PDUs:
 *   - SLM.tx_fcf
 *   - 1SL.tx_fcf
 * 
 * The counter is increased (+1) after being written to the TX PDU.
 * 
 * Note:
 * To send TxFCf = 1 in the first TX SynLM PDU, this register must be
 * initialized to 1.
 *
 * Register: \a VOP:VOE_STAT:SLM_TX_FRM_CNT
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_STAT_SLM_TX_FRM_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x20000,gi,32,0,28)

/**
 * Register Group: \a VOP:VOE_CCM_LM
 *
 * VOE CCM-LM samples
 */


/** 
 * \brief [MCC_DEBUG]
 *
 * \details
 * [MCC_DEBUG]
 *
 * Register: \a VOP:VOE_CCM_LM:CCM_TX_FCB_CFG
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_CCM_LM_CCM_TX_FCB_CFG(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x11000,gi,2,0,0)


/** 
 * \brief [MCC_DEBUG]
 *
 * \details
 * [MCC_DEBUG]
 *
 * Register: \a VOP:VOE_CCM_LM:CCM_RX_FCB_CFG
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_CCM_LM_CCM_RX_FCB_CFG(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x11000,gi,2,0,1)

/**
 * Register Group: \a VOP:VOE_CONTEXT_REW
 *
 * [MCC_DEBUG] Contains the context for the VOE if in REW.
 */


/** 
 * \brief [MCC_DEBUG] Context for ports on the REW interface
 *
 * \details
 * [MCC_DEBUG] TX LM frame counters  by VOE.
 *
 * Register: \a VOP:VOE_CONTEXT_REW:CT_OAM_INFO_REW
 *
 * @param gi Replicator: x_NUM_FRONT_PORTS (??), 0-52
 */
#define VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x11900,gi,4,0,0)

/** 
 * \brief
 * [MCC_DEBUG]
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW . CT_ENTRY_VALID_REW
 */
#define  VTSS_F_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW_CT_ENTRY_VALID_REW  VTSS_BIT(30)

/** 
 * \brief
 * [MCC_DEBUG]
 *
 * \details 
 * 0: OAM Frame is TX
 * 1: OAM Frame is RX
 *
 * Field: ::VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW . CT_LOOKUP_TYPE_REW
 */
#define  VTSS_F_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW_CT_LOOKUP_TYPE_REW(x)  VTSS_ENCODE_BITFIELD(x,26,4)
#define  VTSS_M_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW_CT_LOOKUP_TYPE_REW     VTSS_ENCODE_BITMASK(26,4)
#define  VTSS_X_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW_CT_LOOKUP_TYPE_REW(x)  VTSS_EXTRACT_BITFIELD(x,26,4)

/** 
 * \brief
 * [MCC_DEBUG] OAM type currently being processed
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW . CT_OAM_TYPE_REW
 */
#define  VTSS_F_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW_CT_OAM_TYPE_REW(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW_CT_OAM_TYPE_REW     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW_CT_OAM_TYPE_REW(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

/** 
 * \brief
 * [MCC_DEBUG] OAM PDU currently being processed
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW . CT_OAM_PDU_REW
 */
#define  VTSS_F_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW_CT_OAM_PDU_REW(x)  VTSS_ENCODE_BITFIELD(x,21,5)
#define  VTSS_M_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW_CT_OAM_PDU_REW     VTSS_ENCODE_BITMASK(21,5)
#define  VTSS_X_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW_CT_OAM_PDU_REW(x)  VTSS_EXTRACT_BITFIELD(x,21,5)

/** 
 * \brief
 * Generic index from if the OpCode is generic.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW . CT_OAM_GEN_IDX_REW
 */
#define  VTSS_F_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW_CT_OAM_GEN_IDX_REW(x)  VTSS_ENCODE_BITFIELD(x,18,3)
#define  VTSS_M_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW_CT_OAM_GEN_IDX_REW     VTSS_ENCODE_BITMASK(18,3)
#define  VTSS_X_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW_CT_OAM_GEN_IDX_REW(x)  VTSS_EXTRACT_BITFIELD(x,18,3)

/** 
 * \brief
 * [MCC_DEBUG] Source port.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW . CT_SRC_PORT_REW
 */
#define  VTSS_F_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW_CT_SRC_PORT_REW(x)  VTSS_ENCODE_BITFIELD(x,12,6)
#define  VTSS_M_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW_CT_SRC_PORT_REW     VTSS_ENCODE_BITMASK(12,6)
#define  VTSS_X_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW_CT_SRC_PORT_REW(x)  VTSS_EXTRACT_BITFIELD(x,12,6)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW . CT_CHK_SEQ_REW
 */
#define  VTSS_F_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW_CT_CHK_SEQ_REW  VTSS_BIT(11)

/** 
 * \brief
 * [MCC_DEBUG]
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW . CT_UPD_SEQ_REW
 */
#define  VTSS_F_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW_CT_UPD_SEQ_REW  VTSS_BIT(10)

/** 
 * \brief
 * Determines if the PDU is to be counted as Selected OAM or NON Selected
 * OAM.
 *
 * \details 
 * 0: Count as NON Selected OAM
 * 1: Count as Selected OAM
 *
 * Field: ::VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW . CT_SEL_OAM_REW
 */
#define  VTSS_F_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW_CT_SEL_OAM_REW  VTSS_BIT(9)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW . CT_CCM_LM_AS_SEL_REW
 */
#define  VTSS_F_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW_CT_CCM_LM_AS_SEL_REW  VTSS_BIT(8)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW . CT_BLOCK_DATA_REW
 */
#define  VTSS_F_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW_CT_BLOCK_DATA_REW  VTSS_BIT(7)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW . CT_FRAME_PRIO_REW
 */
#define  VTSS_F_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW_CT_FRAME_PRIO_REW(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW_CT_FRAME_PRIO_REW     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW_CT_FRAME_PRIO_REW(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW . CT_NON_OAM_ERR_CNT_REW
 */
#define  VTSS_F_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW_CT_NON_OAM_ERR_CNT_REW  VTSS_BIT(3)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW . CT_NON_OAM_FWD_ERR_REW
 */
#define  VTSS_F_VOP_VOE_CONTEXT_REW_CT_OAM_INFO_REW_CT_NON_OAM_FWD_ERR_REW  VTSS_BIT(2)


/** 
 * \brief [MCC_DEBUG]
 *
 * \details
 * [MCC_DEBUG]
 *
 * Register: \a VOP:VOE_CONTEXT_REW:CT_OAM_STICKY_REW
 *
 * @param gi Replicator: x_NUM_FRONT_PORTS (??), 0-52
 */
#define VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x11900,gi,4,0,1)

/** 
 * \brief
 * [MCC_DEBUG]
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW . CT_MEL_HIGH_REW
 */
#define  VTSS_F_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW_CT_MEL_HIGH_REW  VTSS_BIT(22)

/** 
 * \brief
 * [MCC_DEBUG] PDU was correctly validaded by the VOE and is ready to be
 * processed.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW . CT_VALID_PDU_REW
 */
#define  VTSS_F_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW_CT_VALID_PDU_REW  VTSS_BIT(21)

/** 
 * \brief
 * [MCC_DEBUG] PDU was correctly validaded by the VOE and is ready to be
 * processed.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW . CT_PDU_HW_ENA_REW
 */
#define  VTSS_F_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW_CT_PDU_HW_ENA_REW  VTSS_BIT(20)

/** 
 * \brief
 * [MCC_DEBUG] PDU was correctly validaded by the VOE and is ready to be
 * processed.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW . CT_CCM_PERIOD_ERR_REW
 */
#define  VTSS_F_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW_CT_CCM_PERIOD_ERR_REW  VTSS_BIT(19)

/** 
 * \brief
 * [MCC_DEBUG] PDU was correctly validaded by the VOE and is ready to be
 * processed.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW . CT_CCM_PRIO_ERR_REW
 */
#define  VTSS_F_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW_CT_CCM_PRIO_ERR_REW  VTSS_BIT(18)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW . CT_CCM_NONZERO_ENDTLV_REW
 */
#define  VTSS_F_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW_CT_CCM_NONZERO_ENDTLV_REW  VTSS_BIT(17)

/** 
 * \brief
 * [MCC_DEBUG]
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW . CT_EXTRACT_CAUSE_REW
 */
#define  VTSS_F_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW_CT_EXTRACT_CAUSE_REW(x)  VTSS_ENCODE_BITFIELD(x,13,4)
#define  VTSS_M_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW_CT_EXTRACT_CAUSE_REW     VTSS_ENCODE_BITMASK(13,4)
#define  VTSS_X_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW_CT_EXTRACT_CAUSE_REW(x)  VTSS_EXTRACT_BITFIELD(x,13,4)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW . CT_EXTRACT_QU_REW
 */
#define  VTSS_F_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW_CT_EXTRACT_QU_REW(x)  VTSS_ENCODE_BITFIELD(x,10,3)
#define  VTSS_M_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW_CT_EXTRACT_QU_REW     VTSS_ENCODE_BITMASK(10,3)
#define  VTSS_X_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW_CT_EXTRACT_QU_REW(x)  VTSS_EXTRACT_BITFIELD(x,10,3)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW . CT_SAM_SEQ_LBM_REW
 */
#define  VTSS_F_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW_CT_SAM_SEQ_LBM_REW  VTSS_BIT(9)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW . CT_SAM_SEQ_CCM_REW
 */
#define  VTSS_F_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW_CT_SAM_SEQ_CCM_REW  VTSS_BIT(8)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW . CT_SAM_SEQ_IDX_REW
 */
#define  VTSS_F_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW_CT_SAM_SEQ_IDX_REW(x)  VTSS_ENCODE_BITFIELD(x,3,5)
#define  VTSS_M_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW_CT_SAM_SEQ_IDX_REW     VTSS_ENCODE_BITMASK(3,5)
#define  VTSS_X_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW_CT_SAM_SEQ_IDX_REW(x)  VTSS_EXTRACT_BITFIELD(x,3,5)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW . CT_SYMLM_PEER_IDX_REW
 */
#define  VTSS_F_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW_CT_SYMLM_PEER_IDX_REW(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW_CT_SYMLM_PEER_IDX_REW     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_VOP_VOE_CONTEXT_REW_CT_OAM_STICKY_REW_CT_SYMLM_PEER_IDX_REW(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \details
 * Register: \a VOP:VOE_CONTEXT_REW:CT_OAM_DATA_REW
 *
 * @param gi Replicator: x_NUM_FRONT_PORTS (??), 0-52
 */
#define VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_DATA_REW(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x11900,gi,4,0,2)


/** 
 * \details
 * Register: \a VOP:VOE_CONTEXT_REW:CT_OAM_DATA1_REW
 *
 * @param gi Replicator: x_NUM_FRONT_PORTS (??), 0-52
 */
#define VTSS_VOP_VOE_CONTEXT_REW_CT_OAM_DATA1_REW(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x11900,gi,4,0,3)

/**
 * Register Group: \a VOP:VOE_CONTEXT_ANA
 *
 * [MCC_DEBUG] Contains the context for the VOE if in REW.
 */


/** 
 * \brief [MCC_DEBUG] Context for ports on the ANA interface
 *
 * \details
 * [MCC_DEBUG] TX LM frame counters  by VOE.
 *
 * Register: \a VOP:VOE_CONTEXT_ANA:CT_OAM_INFO_ANA
 *
 * @param gi Replicator: x_NUM_2xFRONT_PORTS (??), 0-105
 */
#define VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x10e00,gi,4,0,0)

/** 
 * \brief
 * [MCC_DEBUG]
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA . CT_ENTRY_VALID_ANA
 */
#define  VTSS_F_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA_CT_ENTRY_VALID_ANA  VTSS_BIT(30)

/** 
 * \brief
 * [MCC_DEBUG]
 *
 * \details 
 * 0: OAM Frame is TX
 * 1: OAM Frame is RX
 *
 * Field: ::VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA . CT_LOOKUP_TYPE_ANA
 */
#define  VTSS_F_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA_CT_LOOKUP_TYPE_ANA(x)  VTSS_ENCODE_BITFIELD(x,26,4)
#define  VTSS_M_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA_CT_LOOKUP_TYPE_ANA     VTSS_ENCODE_BITMASK(26,4)
#define  VTSS_X_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA_CT_LOOKUP_TYPE_ANA(x)  VTSS_EXTRACT_BITFIELD(x,26,4)

/** 
 * \brief
 * [MCC_DEBUG] OAM type currently being processed
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA . CT_OAM_TYPE_ANA
 */
#define  VTSS_F_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA_CT_OAM_TYPE_ANA(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA_CT_OAM_TYPE_ANA     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA_CT_OAM_TYPE_ANA(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

/** 
 * \brief
 * [MCC_DEBUG] OAM PDU currently being processed
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA . CT_OAM_PDU_ANA
 */
#define  VTSS_F_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA_CT_OAM_PDU_ANA(x)  VTSS_ENCODE_BITFIELD(x,21,5)
#define  VTSS_M_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA_CT_OAM_PDU_ANA     VTSS_ENCODE_BITMASK(21,5)
#define  VTSS_X_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA_CT_OAM_PDU_ANA(x)  VTSS_EXTRACT_BITFIELD(x,21,5)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA . CT_OAM_GEN_IDX_ANA
 */
#define  VTSS_F_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA_CT_OAM_GEN_IDX_ANA(x)  VTSS_ENCODE_BITFIELD(x,18,3)
#define  VTSS_M_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA_CT_OAM_GEN_IDX_ANA     VTSS_ENCODE_BITMASK(18,3)
#define  VTSS_X_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA_CT_OAM_GEN_IDX_ANA(x)  VTSS_EXTRACT_BITFIELD(x,18,3)

/** 
 * \brief
 * [MCC_DEBUG] Source port.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA . CT_SRC_PORT_ANA
 */
#define  VTSS_F_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA_CT_SRC_PORT_ANA(x)  VTSS_ENCODE_BITFIELD(x,12,6)
#define  VTSS_M_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA_CT_SRC_PORT_ANA     VTSS_ENCODE_BITMASK(12,6)
#define  VTSS_X_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA_CT_SRC_PORT_ANA(x)  VTSS_EXTRACT_BITFIELD(x,12,6)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA . CT_CHK_SEQ_ANA
 */
#define  VTSS_F_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA_CT_CHK_SEQ_ANA  VTSS_BIT(11)

/** 
 * \brief
 * [MCC_DEBUG]
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA . CT_UPD_SEQ_ANA
 */
#define  VTSS_F_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA_CT_UPD_SEQ_ANA  VTSS_BIT(10)

/** 
 * \brief
 * Determines if the PDU is to be counted as Selected OAM or NON Selected
 * OAM.
 *
 * \details 
 * 0: Count as NON Selected OAM
 * 1: Count as Selected OAM
 *
 * Field: ::VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA . CT_SEL_OAM_ANA
 */
#define  VTSS_F_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA_CT_SEL_OAM_ANA  VTSS_BIT(9)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA . CT_CCM_LM_AS_SEL_ANA
 */
#define  VTSS_F_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA_CT_CCM_LM_AS_SEL_ANA  VTSS_BIT(8)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA . CT_BLOCK_DATA_ANA
 */
#define  VTSS_F_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA_CT_BLOCK_DATA_ANA  VTSS_BIT(7)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA . CT_FRAME_PRIO_ANA
 */
#define  VTSS_F_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA_CT_FRAME_PRIO_ANA(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA_CT_FRAME_PRIO_ANA     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA_CT_FRAME_PRIO_ANA(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA . CT_NON_OAM_ERR_CNT_ANA
 */
#define  VTSS_F_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA_CT_NON_OAM_ERR_CNT_ANA  VTSS_BIT(3)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA . CT_NON_OAM_FWD_ERR_ANA
 */
#define  VTSS_F_VOP_VOE_CONTEXT_ANA_CT_OAM_INFO_ANA_CT_NON_OAM_FWD_ERR_ANA  VTSS_BIT(2)


/** 
 * \brief [MCC_DEBUG]
 *
 * \details
 * [MCC_DEBUG]
 *
 * Register: \a VOP:VOE_CONTEXT_ANA:CT_OAM_STICKY_ANA
 *
 * @param gi Replicator: x_NUM_2xFRONT_PORTS (??), 0-105
 */
#define VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x10e00,gi,4,0,1)

/** 
 * \brief
 * [MCC_DEBUG]
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA . CT_MEL_HIGH_ANA
 */
#define  VTSS_F_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA_CT_MEL_HIGH_ANA  VTSS_BIT(22)

/** 
 * \brief
 * [MCC_DEBUG] PDU was correctly validaded by the VOE and is ready to be
 * processed.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA . CT_VALID_PDU_ANA
 */
#define  VTSS_F_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA_CT_VALID_PDU_ANA  VTSS_BIT(21)

/** 
 * \brief
 * [MCC_DEBUG] PDU was correctly validaded by the VOE and is ready to be
 * processed.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA . CT_PDU_HW_ENA_ANA
 */
#define  VTSS_F_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA_CT_PDU_HW_ENA_ANA  VTSS_BIT(20)

/** 
 * \brief
 * [MCC_DEBUG] PDU was correctly validaded by the VOE and is ready to be
 * processed.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA . CT_CCM_PERIOD_ERR_ANA
 */
#define  VTSS_F_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA_CT_CCM_PERIOD_ERR_ANA  VTSS_BIT(19)

/** 
 * \brief
 * [MCC_DEBUG] PDU was correctly validaded by the VOE and is ready to be
 * processed.
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA . CT_CCM_PRIO_ERR_ANA
 */
#define  VTSS_F_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA_CT_CCM_PRIO_ERR_ANA  VTSS_BIT(18)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA . CT_CCM_NONZERO_ENDTLV_ANA
 */
#define  VTSS_F_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA_CT_CCM_NONZERO_ENDTLV_ANA  VTSS_BIT(17)

/** 
 * \brief
 * [MCC_DEBUG]
 *
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA . CT_EXTRACT_CAUSE_ANA
 */
#define  VTSS_F_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA_CT_EXTRACT_CAUSE_ANA(x)  VTSS_ENCODE_BITFIELD(x,13,4)
#define  VTSS_M_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA_CT_EXTRACT_CAUSE_ANA     VTSS_ENCODE_BITMASK(13,4)
#define  VTSS_X_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA_CT_EXTRACT_CAUSE_ANA(x)  VTSS_EXTRACT_BITFIELD(x,13,4)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA . CT_EXTRACT_QU_ANA
 */
#define  VTSS_F_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA_CT_EXTRACT_QU_ANA(x)  VTSS_ENCODE_BITFIELD(x,10,3)
#define  VTSS_M_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA_CT_EXTRACT_QU_ANA     VTSS_ENCODE_BITMASK(10,3)
#define  VTSS_X_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA_CT_EXTRACT_QU_ANA(x)  VTSS_EXTRACT_BITFIELD(x,10,3)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA . CT_SAM_SEQ_LBM_ANA
 */
#define  VTSS_F_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA_CT_SAM_SEQ_LBM_ANA  VTSS_BIT(9)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA . CT_SAM_SEQ_CCM_ANA
 */
#define  VTSS_F_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA_CT_SAM_SEQ_CCM_ANA  VTSS_BIT(8)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA . CT_SAM_SEQ_IDX_ANA
 */
#define  VTSS_F_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA_CT_SAM_SEQ_IDX_ANA(x)  VTSS_ENCODE_BITFIELD(x,3,5)
#define  VTSS_M_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA_CT_SAM_SEQ_IDX_ANA     VTSS_ENCODE_BITMASK(3,5)
#define  VTSS_X_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA_CT_SAM_SEQ_IDX_ANA(x)  VTSS_EXTRACT_BITFIELD(x,3,5)

/** 
 * \details 
 * Field: ::VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA . CT_SYMLM_PEER_IDX_ANA
 */
#define  VTSS_F_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA_CT_SYMLM_PEER_IDX_ANA(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA_CT_SYMLM_PEER_IDX_ANA     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_VOP_VOE_CONTEXT_ANA_CT_OAM_STICKY_ANA_CT_SYMLM_PEER_IDX_ANA(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \details
 * Register: \a VOP:VOE_CONTEXT_ANA:CT_OAM_DATA_ANA
 *
 * @param gi Replicator: x_NUM_2xFRONT_PORTS (??), 0-105
 */
#define VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_DATA_ANA(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x10e00,gi,4,0,2)


/** 
 * \details
 * Register: \a VOP:VOE_CONTEXT_ANA:CT_OAM_DATA1_ANA
 *
 * @param gi Replicator: x_NUM_2xFRONT_PORTS (??), 0-105
 */
#define VTSS_VOP_VOE_CONTEXT_ANA_CT_OAM_DATA1_ANA(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x10e00,gi,4,0,3)

/**
 * Register Group: \a VOP:VOE_CRC_ERR
 *
 * Count the number of CRC errors in RX LBR TLVs
 */


/** 
 * \brief Count the number of LBR and TST CRC errors received.
 *
 * \details
 * The VOE can verify the CRC-32 of Test TLVs in incoming LBR and TST PDUs.
 * 
 * This functionality is enabled using one of the following bit fields:
 * 
 *  - OAM_MEP:VOE_CTRL:OAM_HW_CTRL.LBR_TLV_CRC_VERIFY_ENA
 *  - OAM_MEP:VOE_CTRL:OAM_HW_CTRL.TST_TLV_CRC_VERIFY_ENA
 * 
 * When enabled the VOE will examine the TLV field of valid LBR and TST
 * PDUs in the RX direction.
 * If the first TLV following the LBR or TST PDU is a Test TLV including a
 * CRC-32 across the Data Pattern, the VOE will calculate the CRC across
 * the Data Pattern and verify the CRC-32.
 * 
 * This register will count the number of CRC errors received by the VOE.
 *
 * Register: \a VOP:VOE_CRC_ERR:LBR_CRC_ERR_CNT
 *
 * @param gi Replicator: x_NUM_TOTAL_VOE (??), 0-1076
 */
#define VTSS_VOP_VOE_CRC_ERR_LBR_CRC_ERR_CNT(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x14000,gi,1,0,0)

/**
 * Register Group: \a VOP:ANA_COSID_MAP_CONF
 *
 * COSID / Color config in ANA
 */


/** 
 * \brief COSID mapping table
 *
 * \details
 * COSID mapping table used for mapping the selected COSID values.
 * 
 * A single mapping table is available for each of the Service/Path VOEs.
 *
 * Register: \a VOP:ANA_COSID_MAP_CONF:COSID_MAP_TABLE_ANA
 *
 * @param gi Replicator: x_NUM_VOE (??), 0-1023
 */
#define VTSS_VOP_ANA_COSID_MAP_CONF_COSID_MAP_TABLE_ANA(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x12000,gi,2,0,0)

/** 
 * \brief
 * The table is used to map the choosen COSID in the ANA.
 * 
 * bit(2:0) will be used to map COSID = 0
 * bit(5:3) will be used to map COSID = 1
 * 
 * ...
 * bit(23:21) will be used to map COSID = 7

 *
 * \details 
 * Field: ::VTSS_VOP_ANA_COSID_MAP_CONF_COSID_MAP_TABLE_ANA . COSID_MAP_TABLE_ANA
 */
#define  VTSS_F_VOP_ANA_COSID_MAP_CONF_COSID_MAP_TABLE_ANA_COSID_MAP_TABLE_ANA(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_VOP_ANA_COSID_MAP_CONF_COSID_MAP_TABLE_ANA_COSID_MAP_TABLE_ANA     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_VOP_ANA_COSID_MAP_CONF_COSID_MAP_TABLE_ANA_COSID_MAP_TABLE_ANA(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief COSID / Color control signals
 *
 * \details
 * The bit fields in this register determines the source of the COSID
 * mapping / COLOR of frames not destined in the VOE.
 *
 * Register: \a VOP:ANA_COSID_MAP_CONF:COSID_MAP_CFG_ANA
 *
 * @param gi Replicator: x_NUM_VOE (??), 0-1023
 */
#define VTSS_VOP_ANA_COSID_MAP_CONF_COSID_MAP_CFG_ANA(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x12000,gi,2,0,1)

/** 
 * \brief
 * Selects the source of the COSID mapping.
 *
 * \details 
 * "00" : ifh.cosid
 * "01" : ifh.tc
 * "10" : ifh_iprio
 * "11" : CLM.path_cosid (do not use for Up-MEP)
 *
 * Field: ::VTSS_VOP_ANA_COSID_MAP_CONF_COSID_MAP_CFG_ANA . COSID_SRC_SEL_ANA
 */
#define  VTSS_F_VOP_ANA_COSID_MAP_CONF_COSID_MAP_CFG_ANA_COSID_SRC_SEL_ANA(x)  VTSS_ENCODE_BITFIELD(x,3,2)
#define  VTSS_M_VOP_ANA_COSID_MAP_CONF_COSID_MAP_CFG_ANA_COSID_SRC_SEL_ANA     VTSS_ENCODE_BITMASK(3,2)
#define  VTSS_X_VOP_ANA_COSID_MAP_CONF_COSID_MAP_CFG_ANA_COSID_SRC_SEL_ANA(x)  VTSS_EXTRACT_BITFIELD(x,3,2)

/** 
 * \brief
 * Determines which internal signal carries color for the current VOE.
 * Used only in ANA
 *
 * \details 
 * "00" : ifh.dp_color
 * "01" : ifh.cl_dei
 * "10" : CLM.path_color (do not use for Up-MEP)
 * "11" : reserved for future use (do not use)
 *
 * Field: ::VTSS_VOP_ANA_COSID_MAP_CONF_COSID_MAP_CFG_ANA . COLOR_SRC_SEL_ANA
 */
#define  VTSS_F_VOP_ANA_COSID_MAP_CONF_COSID_MAP_CFG_ANA_COLOR_SRC_SEL_ANA(x)  VTSS_ENCODE_BITFIELD(x,1,2)
#define  VTSS_M_VOP_ANA_COSID_MAP_CONF_COSID_MAP_CFG_ANA_COLOR_SRC_SEL_ANA     VTSS_ENCODE_BITMASK(1,2)
#define  VTSS_X_VOP_ANA_COSID_MAP_CONF_COSID_MAP_CFG_ANA_COLOR_SRC_SEL_ANA(x)  VTSS_EXTRACT_BITFIELD(x,1,2)

/** 
 * \brief
 * Determines if the VOE LM counters counts all frames or only GREEN
 * frames.
 *
 * \details 
 * '0' : do not include yellow frames in the LM count.
 * '1' : include yellow frames in the LM count.
 *
 * Field: ::VTSS_VOP_ANA_COSID_MAP_CONF_COSID_MAP_CFG_ANA . CNT_YELLOW_ANA
 */
#define  VTSS_F_VOP_ANA_COSID_MAP_CONF_COSID_MAP_CFG_ANA_CNT_YELLOW_ANA  VTSS_BIT(0)

/**
 * Register Group: \a VOP:REW_COSID_MAP_CONF
 *
 * COSID / Color config in REW
 */


/** 
 * \brief COSID mapping table
 *
 * \details
 * COSID mapping table used for mapping the selected COSID values.
 * 
 * A single mapping table is available for each of the Service/Path VOEs.
 *
 * Register: \a VOP:REW_COSID_MAP_CONF:COSID_MAP_TABLE_REW
 *
 * @param gi Replicator: x_NUM_VOE (??), 0-1023
 */
#define VTSS_VOP_REW_COSID_MAP_CONF_COSID_MAP_TABLE_REW(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x12800,gi,2,0,0)

/** 
 * \brief
 * The table is used to map the choosen COSID in the REW.
 * 
 * bit(2:0) will be used to map COSID = 0
 * bit(5:3) will be used to map COSID = 1
 * 
 * ...
 * bit(23:21) will be used to map COSID = 7
 *
 * \details 
 * Field: ::VTSS_VOP_REW_COSID_MAP_CONF_COSID_MAP_TABLE_REW . COSID_MAP_TABLE_REW
 */
#define  VTSS_F_VOP_REW_COSID_MAP_CONF_COSID_MAP_TABLE_REW_COSID_MAP_TABLE_REW(x)  VTSS_ENCODE_BITFIELD(x,0,24)
#define  VTSS_M_VOP_REW_COSID_MAP_CONF_COSID_MAP_TABLE_REW_COSID_MAP_TABLE_REW     VTSS_ENCODE_BITMASK(0,24)
#define  VTSS_X_VOP_REW_COSID_MAP_CONF_COSID_MAP_TABLE_REW_COSID_MAP_TABLE_REW(x)  VTSS_EXTRACT_BITFIELD(x,0,24)


/** 
 * \brief COSID / Color control signals
 *
 * \details
 * The bit fields in this register determines the source of the COSID
 * mapping / COLOR of frames not destined in the VOE.
 *
 * Register: \a VOP:REW_COSID_MAP_CONF:COSID_MAP_CFG_REW
 *
 * @param gi Replicator: x_NUM_VOE (??), 0-1023
 */
#define VTSS_VOP_REW_COSID_MAP_CONF_COSID_MAP_CFG_REW(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x12800,gi,2,0,1)

/** 
 * \brief
 * Selects the source of the COSID mapping.
 *
 * \details 
 * "00" : ifh.cosid
 * "01" : ifh.tc
 * "10" : ifh_iprio
 * "11" : ES0.path_cosid
 *
 * Field: ::VTSS_VOP_REW_COSID_MAP_CONF_COSID_MAP_CFG_REW . COSID_SRC_SEL_REW
 */
#define  VTSS_F_VOP_REW_COSID_MAP_CONF_COSID_MAP_CFG_REW_COSID_SRC_SEL_REW(x)  VTSS_ENCODE_BITFIELD(x,3,2)
#define  VTSS_M_VOP_REW_COSID_MAP_CONF_COSID_MAP_CFG_REW_COSID_SRC_SEL_REW     VTSS_ENCODE_BITMASK(3,2)
#define  VTSS_X_VOP_REW_COSID_MAP_CONF_COSID_MAP_CFG_REW_COSID_SRC_SEL_REW(x)  VTSS_EXTRACT_BITFIELD(x,3,2)

/** 
 * \brief
 * Determines which internal signal carries color for the current VOE.
 *
 * \details 
 * "00" : ifh.dp_color
 * "01" : ifh.cl_dei
 * "10" : ES0.path_color
 * "11" : reserved for future use (do not use)
 *
 * Field: ::VTSS_VOP_REW_COSID_MAP_CONF_COSID_MAP_CFG_REW . COLOR_SRC_SEL_REW
 */
#define  VTSS_F_VOP_REW_COSID_MAP_CONF_COSID_MAP_CFG_REW_COLOR_SRC_SEL_REW(x)  VTSS_ENCODE_BITFIELD(x,1,2)
#define  VTSS_M_VOP_REW_COSID_MAP_CONF_COSID_MAP_CFG_REW_COLOR_SRC_SEL_REW     VTSS_ENCODE_BITMASK(1,2)
#define  VTSS_X_VOP_REW_COSID_MAP_CONF_COSID_MAP_CFG_REW_COLOR_SRC_SEL_REW(x)  VTSS_EXTRACT_BITFIELD(x,1,2)

/** 
 * \brief
 * Determines if the VOE LM counters counts all frames or only GREEN
 * frames.
 *
 * \details 
 * '0' : do not include yellow frames in the LM count.
 * '1' : include yellow frames in the LM count.
 *
 * Field: ::VTSS_VOP_REW_COSID_MAP_CONF_COSID_MAP_CFG_REW . CNT_YELLOW_REW
 */
#define  VTSS_F_VOP_REW_COSID_MAP_CONF_COSID_MAP_CFG_REW_CNT_YELLOW_REW  VTSS_BIT(0)

/**
 * Register Group: \a VOP:PORT_COSID_MAP_CONF
 *
 * Not documented
 */


/** 
 * \brief LSB of RX Port VOE mapping table (ANA).
 *
 * \details
 * This register contains the lower 32 bits of the Port VOE RX (ANA) COSID
 * mapping table.
 * 
 * This mapping in this register is used when Port DEI = 0
 *
 * Register: \a VOP:PORT_COSID_MAP_CONF:PORT_RX_COSID_MAP
 *
 * @param gi Replicator: x_NUM_FRONT_PORTS (??), 0-52
 */
#define VTSS_VOP_PORT_COSID_MAP_CONF_PORT_RX_COSID_MAP(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x11a00,gi,4,0,0)


/** 
 * \brief MSB of RX Port VOE mapping table (ANA).
 *
 * \details
 * This register contains the upper 32 bits of the Port VOE RX (ANA) COSID
 * mapping table.
 * 
 * This mapping in this register is used when Port DEI = 1
 *
 * Register: \a VOP:PORT_COSID_MAP_CONF:PORT_RX_COSID_MAP1
 *
 * @param gi Replicator: x_NUM_FRONT_PORTS (??), 0-52
 */
#define VTSS_VOP_PORT_COSID_MAP_CONF_PORT_RX_COSID_MAP1(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x11a00,gi,4,0,1)


/** 
 * \brief LSB of TX Port VOE mapping table (REW).
 *
 * \details
 * This register contains the lower 32 bits of the Port VOE TX (REW) COSID
 * mapping table.
 * 
 * This mapping in this register is used when Port DEI = 0
 *
 * Register: \a VOP:PORT_COSID_MAP_CONF:PORT_TX_COSID_MAP
 *
 * @param gi Replicator: x_NUM_FRONT_PORTS (??), 0-52
 */
#define VTSS_VOP_PORT_COSID_MAP_CONF_PORT_TX_COSID_MAP(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x11a00,gi,4,0,2)


/** 
 * \brief MSB of TX Port VOE mapping table (REW).
 *
 * \details
 * This register contains the upper 32 bits of the Port VOE TX (REW) COSID
 * mapping table.
 * 
 * This mapping in this register is used when Port DEI = 1
 *
 * Register: \a VOP:PORT_COSID_MAP_CONF:PORT_TX_COSID_MAP1
 *
 * @param gi Replicator: x_NUM_FRONT_PORTS (??), 0-52
 */
#define VTSS_VOP_PORT_COSID_MAP_CONF_PORT_TX_COSID_MAP1(gi)  VTSS_IOREG_IX(VTSS_TO_VOP,0x11a00,gi,4,0,3)

/**
 * Register Group: \a VOP:SAM_COSID_SEQ_CNT
 *
 * SAM per COSID sequence counters
 */


/** 
 * \brief LBM/CCM Tx frames: Priority 1
 *
 * \details
 * When a VOE is assigned a SAM per COSID counter set, this register
 * countes TX frames on priority 1 as follows:
 * 
 * CCM:
 *  - VOE_STAT:CCM_TX_SEQ_CFG.CCM_TX_SEQ
 * 
 * LBM:
 *  - VOE_STAT:LBM_TX_TRANSID_CFG.LBM_TX_TRANSID
 * 
 * TST:
 *  - VOE_STAT:LBM_TX_TRANSID_CFG.LBM_TX_TRANSID
 *
 * Register: \a VOP:SAM_COSID_SEQ_CNT:SAM_LBM_TX_TRANSID
 *
 * @param gi Replicator: x_SAM_SEQ_SETS (??), 0-31
 * @param ri Register: SAM_LBM_TX_TRANSID (??), 0-6
 */
#define VTSS_VOP_SAM_COSID_SEQ_CNT_SAM_LBM_TX_TRANSID(gi,ri)  VTSS_IOREG_IX(VTSS_TO_VOP,0x13000,gi,64,ri,0)


/** 
 * \details
 * Register: \a VOP:SAM_COSID_SEQ_CNT:SAM_LBR_TX_FRM_CNT
 *
 * @param gi Replicator: x_SAM_SEQ_SETS (??), 0-31
 * @param ri Register: SAM_LBR_TX_FRM_CNT (??), 0-6
 */
#define VTSS_VOP_SAM_COSID_SEQ_CNT_SAM_LBR_TX_FRM_CNT(gi,ri)  VTSS_IOREG_IX(VTSS_TO_VOP,0x13000,gi,64,ri,7)


/** 
 * \details
 * Register: \a VOP:SAM_COSID_SEQ_CNT:SAM_LBR_RX_FRM_CNT
 *
 * @param gi Replicator: x_SAM_SEQ_SETS (??), 0-31
 * @param ri Register: SAM_LBR_RX_FRM_CNT (??), 0-6
 */
#define VTSS_VOP_SAM_COSID_SEQ_CNT_SAM_LBR_RX_FRM_CNT(gi,ri)  VTSS_IOREG_IX(VTSS_TO_VOP,0x13000,gi,64,ri,14)


/** 
 * \details
 * Register: \a VOP:SAM_COSID_SEQ_CNT:SAM_LBR_RX_TRANSID
 *
 * @param gi Replicator: x_SAM_SEQ_SETS (??), 0-31
 * @param ri Register: SAM_LBR_RX_TRANSID (??), 0-6
 */
#define VTSS_VOP_SAM_COSID_SEQ_CNT_SAM_LBR_RX_TRANSID(gi,ri)  VTSS_IOREG_IX(VTSS_TO_VOP,0x13000,gi,64,ri,21)


/** 
 * \details
 * Register: \a VOP:SAM_COSID_SEQ_CNT:SAM_LBR_RX_TRANSID_ERR_CNT
 *
 * @param gi Replicator: x_SAM_SEQ_SETS (??), 0-31
 * @param ri Register: SAM_LBR_RX_TRANSID_ERR_CNT (??), 0-6
 */
#define VTSS_VOP_SAM_COSID_SEQ_CNT_SAM_LBR_RX_TRANSID_ERR_CNT(gi,ri)  VTSS_IOREG_IX(VTSS_TO_VOP,0x13000,gi,64,ri,28)

/** 
 * \details 
 * Field: ::VTSS_VOP_SAM_COSID_SEQ_CNT_SAM_LBR_RX_TRANSID_ERR_CNT . SAM_LBR_RX_TRANSID_ERR_CNT
 */
#define  VTSS_F_VOP_SAM_COSID_SEQ_CNT_SAM_LBR_RX_TRANSID_ERR_CNT_SAM_LBR_RX_TRANSID_ERR_CNT(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_VOP_SAM_COSID_SEQ_CNT_SAM_LBR_RX_TRANSID_ERR_CNT_SAM_LBR_RX_TRANSID_ERR_CNT     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_VOP_SAM_COSID_SEQ_CNT_SAM_LBR_RX_TRANSID_ERR_CNT_SAM_LBR_RX_TRANSID_ERR_CNT(x)  VTSS_EXTRACT_BITFIELD(x,0,16)

/**
 * Register Group: \a VOP:RAM_CTRL
 *
 * Access core memory
 */


/** 
 * \brief Core reset control
 *
 * \details
 * Controls reset and initialization of the switching core. Proper startup
 * sequence is:
 * - Enable memories
 * - Initialize memories
 * - Enable core
 *
 * Register: \a VOP:RAM_CTRL:RAM_INIT
 */
#define VTSS_VOP_RAM_CTRL_RAM_INIT           VTSS_IOREG(VTSS_TO_VOP,0x10d87)

/** 
 * \brief
 * Initialize core memories. Field is automatically cleared when operation
 * is complete ( approx. 40 us).
 *
 * \details 
 * Field: ::VTSS_VOP_RAM_CTRL_RAM_INIT . RAM_INIT
 */
#define  VTSS_F_VOP_RAM_CTRL_RAM_INIT_RAM_INIT  VTSS_BIT(1)

/** 
 * \brief
 * Core memory controllers are enabled when this field is set.
 *
 * \details 
 * Field: ::VTSS_VOP_RAM_CTRL_RAM_INIT . RAM_ENA
 */
#define  VTSS_F_VOP_RAM_CTRL_RAM_INIT_RAM_ENA  VTSS_BIT(0)

/**
 * Register Group: \a VOP:COREMEM
 *
 * Access core memory
 */


/** 
 * \brief Address selection
 *
 * \details
 * Register: \a VOP:COREMEM:CM_ADDR
 */
#define VTSS_VOP_COREMEM_CM_ADDR             VTSS_IOREG(VTSS_TO_VOP,0x10d40)

/** 
 * \brief
 * Please refer to cmid.xls in the AS1000, misc_docs folder.
 *
 * \details 
 * Field: ::VTSS_VOP_COREMEM_CM_ADDR . CM_ID
 */
#define  VTSS_F_VOP_COREMEM_CM_ADDR_CM_ID(x)  VTSS_ENCODE_BITFIELD(x,22,8)
#define  VTSS_M_VOP_COREMEM_CM_ADDR_CM_ID     VTSS_ENCODE_BITMASK(22,8)
#define  VTSS_X_VOP_COREMEM_CM_ADDR_CM_ID(x)  VTSS_EXTRACT_BITFIELD(x,22,8)

/** 
 * \brief
 * Address selection within selected core memory (CMID register). Address
 * is automatically advanced at every data access.
 *
 * \details 
 * Field: ::VTSS_VOP_COREMEM_CM_ADDR . CM_ADDR
 */
#define  VTSS_F_VOP_COREMEM_CM_ADDR_CM_ADDR(x)  VTSS_ENCODE_BITFIELD(x,0,22)
#define  VTSS_M_VOP_COREMEM_CM_ADDR_CM_ADDR     VTSS_ENCODE_BITMASK(0,22)
#define  VTSS_X_VOP_COREMEM_CM_ADDR_CM_ADDR(x)  VTSS_EXTRACT_BITFIELD(x,0,22)


/** 
 * \brief Data register for core memory access.
 *
 * \details
 * Register: \a VOP:COREMEM:CM_DATA
 */
#define VTSS_VOP_COREMEM_CM_DATA             VTSS_IOREG(VTSS_TO_VOP,0x10d41)


#endif /* _VTSS_JAGUAR2_REGS_VOP_H_ */
