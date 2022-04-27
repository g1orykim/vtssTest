#ifndef _VTSS_JAGUAR2_REGS_ANA_AC_OAM_MOD_H_
#define _VTSS_JAGUAR2_REGS_ANA_AC_OAM_MOD_H_

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
 * Target: \a ANA_AC_OAM_MOD
 *
 * This block - Vitesse OAM MEP Processor (VOP) - implements the HW support
 * for implementing OAM MEP.
 * 
 * The HW support for implementing an OAM MEP is implemented in a sub
 * block, known as:
 * Vitesse OAM Endpoint (VOE).
 * 
 * The VOP includes the following:
 * 
 *  * 1024 Service / Path VOEs
 *  * 53 Port VOEs (52 front ports + NPI)
 *
 ***********************************************************************/

/**
 * Register Group: \a ANA_AC_OAM_MOD:VOE_SRV_LM_CNT
 *
 * OAM Service LM counters pr. priority
 */


/** 
 * \brief Service LM counters pr. priority
 *
 * \details
 * Implements the OAM VOE LM counters.
 * 
 * Depending on whether the OAM_PDU_MOD block is instantiated in the REW or
 * the ANA these counters will be either egress (REW) or ingress (ANA)
 * counters.
 * 
 * Y.1731 LM counters count frames and are 32 bit wide.
 *
 * Register: \a ANA_AC_OAM_MOD:VOE_SRV_LM_CNT:SRV_LM_CNT_LSB
 *
 * @param gi Replicator: x_NUM_VOE_x_NUM_PRIO (??), 0-8191
 */
#define VTSS_ANA_AC_OAM_MOD_VOE_SRV_LM_CNT_SRV_LM_CNT_LSB(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_AC_OAM_MOD,0x0,gi,1,0,0)

/**
 * Register Group: \a ANA_AC_OAM_MOD:VOE_PORT_LM_CNT
 *
 * OAM LM port counters pr. priority
 */


/** 
 * \brief Count selected OAM PDU received by MEP Counter
 *
 * \details
 * Implements the OAM VOE LM port counters.
 * 
 * Depending on whether the OAM_PDU_MOD block is instantiated in the REW or
 * the ANA these counters will be either egress (REW) or ingress (ANA)
 * counters.
 * 
 * Y.1731 LM counters count frames and are 32 bit wide.
 *
 * Register: \a ANA_AC_OAM_MOD:VOE_PORT_LM_CNT:PORT_LM_CNT_LSB
 *
 * @param gi Replicator: x_NUM_FRONT_PORTS_x_NUM_PRIO (??), 0-423
 */
#define VTSS_ANA_AC_OAM_MOD_VOE_PORT_LM_CNT_PORT_LM_CNT_LSB(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_AC_OAM_MOD,0x2400,gi,1,0,0)

/**
 * Register Group: \a ANA_AC_OAM_MOD:OAM_PDU_MOD_CONT
 *
 * VOE additional information
 */


/** 
 * \details
 * Register: \a ANA_AC_OAM_MOD:OAM_PDU_MOD_CONT:TEMP_CNT_REG
 *
 * @param gi Replicator: x_NUM_2xFRONT_PORTS (??), 0-109
 */
#define VTSS_ANA_AC_OAM_MOD_OAM_PDU_MOD_CONT_TEMP_CNT_REG(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_AC_OAM_MOD,0x2000,gi,8,0,0)

/** 
 * \details 
 * Field: ::VTSS_ANA_AC_OAM_MOD_OAM_PDU_MOD_CONT_TEMP_CNT_REG . TEMP_CNT_VAL
 */
#define  VTSS_F_ANA_AC_OAM_MOD_OAM_PDU_MOD_CONT_TEMP_CNT_REG_TEMP_CNT_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_ANA_AC_OAM_MOD_OAM_PDU_MOD_CONT_TEMP_CNT_REG_TEMP_CNT_VAL     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_ANA_AC_OAM_MOD_OAM_PDU_MOD_CONT_TEMP_CNT_REG_TEMP_CNT_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \details
 * Register: \a ANA_AC_OAM_MOD:OAM_PDU_MOD_CONT:LM_CNT_FRAME
 *
 * @param gi Replicator: x_NUM_2xFRONT_PORTS (??), 0-109
 */
#define VTSS_ANA_AC_OAM_MOD_OAM_PDU_MOD_CONT_LM_CNT_FRAME(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_AC_OAM_MOD,0x2000,gi,8,0,1)

/** 
 * \brief
 * Determines if the current frame should be counted by the Service LM
 * counter, based on the color / mapping and possibly being killed in the
 * ingress DLB.
 *
 * \details 
 * Field: ::VTSS_ANA_AC_OAM_MOD_OAM_PDU_MOD_CONT_LM_CNT_FRAME . SRV_CNT_FRM
 */
#define  VTSS_F_ANA_AC_OAM_MOD_OAM_PDU_MOD_CONT_LM_CNT_FRAME_SRV_CNT_FRM  VTSS_BIT(1)

/** 
 * \brief
 * Determines if the current frame should be counted by the Path LM
 * counter, based on the color / mapping and possibly being killed in the
 * ingress DLB.
 *
 * \details 
 * Field: ::VTSS_ANA_AC_OAM_MOD_OAM_PDU_MOD_CONT_LM_CNT_FRAME . PATH_CNT_FRM
 */
#define  VTSS_F_ANA_AC_OAM_MOD_OAM_PDU_MOD_CONT_LM_CNT_FRAME_PATH_CNT_FRM  VTSS_BIT(0)


/** 
 * \details
 * Register: \a ANA_AC_OAM_MOD:OAM_PDU_MOD_CONT:CCM_LM_INFO_REG
 *
 * @param gi Replicator: x_NUM_2xFRONT_PORTS (??), 0-109
 */
#define VTSS_ANA_AC_OAM_MOD_OAM_PDU_MOD_CONT_CCM_LM_INFO_REG(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_AC_OAM_MOD,0x2000,gi,8,0,2)

/** 
 * \brief
 * Indicates whether this entry in the RAM contains valid CCM-LM sample
 * values.
 *
 * \details 
 * Field: ::VTSS_ANA_AC_OAM_MOD_OAM_PDU_MOD_CONT_CCM_LM_INFO_REG . CCM_LM_INFO_VLD
 */
#define  VTSS_F_ANA_AC_OAM_MOD_OAM_PDU_MOD_CONT_CCM_LM_INFO_REG_CCM_LM_INFO_VLD  VTSS_BIT(11)

/** 
 * \brief
 * The number of the VOE for which the LM information was received.
 *
 * \details 
 * Field: ::VTSS_ANA_AC_OAM_MOD_OAM_PDU_MOD_CONT_CCM_LM_INFO_REG . CCM_LM_VOE_IDX
 */
#define  VTSS_F_ANA_AC_OAM_MOD_OAM_PDU_MOD_CONT_CCM_LM_INFO_REG_CCM_LM_VOE_IDX(x)  VTSS_ENCODE_BITFIELD(x,0,11)
#define  VTSS_M_ANA_AC_OAM_MOD_OAM_PDU_MOD_CONT_CCM_LM_INFO_REG_CCM_LM_VOE_IDX     VTSS_ENCODE_BITMASK(0,11)
#define  VTSS_X_ANA_AC_OAM_MOD_OAM_PDU_MOD_CONT_CCM_LM_INFO_REG_CCM_LM_VOE_IDX(x)  VTSS_EXTRACT_BITFIELD(x,0,11)


/** 
 * \details
 * Contains the sampled value of CCM_LM.tx_fc_b from the last valid CCM_LM
 * frame.
 *
 * Register: \a ANA_AC_OAM_MOD:OAM_PDU_MOD_CONT:CCM_LM_TX_B_REG
 *
 * @param gi Replicator: x_NUM_2xFRONT_PORTS (??), 0-109
 */
#define VTSS_ANA_AC_OAM_MOD_OAM_PDU_MOD_CONT_CCM_LM_TX_B_REG(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_AC_OAM_MOD,0x2000,gi,8,0,3)


/** 
 * \details
 * Contains the sampled value of CCM_LM.rx_fc_b from the last valid CCM_LM
 * frame.
 *
 * Register: \a ANA_AC_OAM_MOD:OAM_PDU_MOD_CONT:CCM_LM_RX_B_REG
 *
 * @param gi Replicator: x_NUM_2xFRONT_PORTS (??), 0-109
 */
#define VTSS_ANA_AC_OAM_MOD_OAM_PDU_MOD_CONT_CCM_LM_RX_B_REG(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_AC_OAM_MOD,0x2000,gi,8,0,4)

/**
 * Register Group: \a ANA_AC_OAM_MOD:PDU_MOD_CFG
 *
 * OAM_PDU_MOD misc. configuration
 */


/** 
 * \details
 * Jaguar2-R supports three PTP time domains.
 * 
 * The Delay Measurements (Y.1731) uses the PTP timing for updating the DM
 * PDUs.
 * 
 * For this to work, each port on the switch must be configured for which
 * PTP timedomain to use.
 * 
 * This is done by configuring this register.
 * 
 * The settings must be the same in the REW and the ANA_AC instatiation of
 * the OAM_PDU_MOD block.
 *
 * Register: \a ANA_AC_OAM_MOD:PDU_MOD_CFG:DM_PTP_DOMAIN_CFG
 *
 * @param ri Replicator: x_NUM_FRONT_PORTS (??), 0-52
 */
#define VTSS_ANA_AC_OAM_MOD_PDU_MOD_CFG_DM_PTP_DOMAIN_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_AC_OAM_MOD,0x2370 + (ri))

/** 
 * \details 
 * Field: ::VTSS_ANA_AC_OAM_MOD_PDU_MOD_CFG_DM_PTP_DOMAIN_CFG . PTP_DOMAIN
 */
#define  VTSS_F_ANA_AC_OAM_MOD_PDU_MOD_CFG_DM_PTP_DOMAIN_CFG_PTP_DOMAIN(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_ANA_AC_OAM_MOD_PDU_MOD_CFG_DM_PTP_DOMAIN_CFG_PTP_DOMAIN     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_ANA_AC_OAM_MOD_PDU_MOD_CFG_DM_PTP_DOMAIN_CFG_PTP_DOMAIN(x)  VTSS_EXTRACT_BITFIELD(x,0,2)


#endif /* _VTSS_JAGUAR2_REGS_ANA_AC_OAM_MOD_H_ */
