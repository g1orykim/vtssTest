#ifndef _VTSS_JAGUAR2_REGS_ANA_CL_H_
#define _VTSS_JAGUAR2_REGS_ANA_CL_H_

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
 * Target: \a ANA_CL
 *
 * Classifier sub block of the Analyzer
 *
 ***********************************************************************/

/**
 * Register Group: \a ANA_CL:PORT
 *
 * Classification and filter configurations per port
 */


/** 
 * \brief Filter configuration
 *
 * \details
 * Configure filtering of frames not matching expected ingress properties
 *
 * Register: \a ANA_CL:PORT:FILTER_CTRL
 *
 * @param gi Replicator: x_ANA_CL_NUM_PORT_CFG (??), 0-56
 */
#define VTSS_ANA_CL_PORT_FILTER_CTRL(gi)     VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x8000,gi,64,0,0)

/** 
 * \brief
 * Discard frames with a multicast SMAC address.
 *
 * \details 
 * 0: Discard frames with multicast SMAC
 * 1: No filter
 *
 * Field: ::VTSS_ANA_CL_PORT_FILTER_CTRL . FILTER_SMAC_MC_DIS
 */
#define  VTSS_F_ANA_CL_PORT_FILTER_CTRL_FILTER_SMAC_MC_DIS  VTSS_BIT(2)

/** 
 * \brief
 * Discard frames with DMAC or SMAC equal to 00-00-00-00-00-00.
 *
 * \details 
 * 0: Discard null MAC frames
 * 1: No filter
 *
 * Field: ::VTSS_ANA_CL_PORT_FILTER_CTRL . FILTER_NULL_MAC_DIS
 */
#define  VTSS_F_ANA_CL_PORT_FILTER_CTRL_FILTER_NULL_MAC_DIS  VTSS_BIT(1)

/** 
 * \brief
 * Enable FCS update of all frames from port.
 *
 * \details 
 * 0: Disable
 * 1: Force FCS update
 *
 * Field: ::VTSS_ANA_CL_PORT_FILTER_CTRL . FORCE_FCS_UPDATE_ENA
 */
#define  VTSS_F_ANA_CL_PORT_FILTER_CTRL_FORCE_FCS_UPDATE_ENA  VTSS_BIT(0)


/** 
 * \brief VLAN acceptance filter
 *
 * \details
 * VLAN_FILTER[0] applies to outer VLAN tag (first tag).
 * VLAN_FILTER[1] applies to middle VLAN tag (second tag).
 * VLAN_FILTER[2] applies to inner VLAN tag (third tag).

 *
 * Register: \a ANA_CL:PORT:VLAN_FILTER_CTRL
 *
 * @param gi Replicator: x_ANA_CL_NUM_PORT_CFG (??), 0-56
 * @param ri Register: VLAN_FILTER_CTRL (??), 0-2
 */
#define VTSS_ANA_CL_PORT_VLAN_FILTER_CTRL(gi,ri)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x8000,gi,64,ri,1)

/** 
 * \brief
 * Discard frame if VLAN_FILTER[n].TAG_REQUIRED_ENA is set and the frame's
 * VLAN tag count is less than n+1:
 * - If VLAN_FILTER[0].TAG_REQUIRED_ENA is set: Discard frame if it is
 * untagged.
 * - If VLAN_FILTER[1].TAG_REQUIRED_ENA is set: Discard frame if it is
 * single tagged or untagged.
 * - If VLAN_FILTER[2].TAG_REQUIRED_ENA is set: Discard frame if it is
 * single tagged, double tagged, or untagged.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_PORT_VLAN_FILTER_CTRL . TAG_REQUIRED_ENA
 */
#define  VTSS_F_ANA_CL_PORT_VLAN_FILTER_CTRL_TAG_REQUIRED_ENA  VTSS_BIT(10)

/** 
 * \brief
 * Discard frame if the investigated VLAN tag is a priority C-tag (VID=0).
 *
 * \details 
 * Field: ::VTSS_ANA_CL_PORT_VLAN_FILTER_CTRL . PRIO_CTAG_DIS
 */
#define  VTSS_F_ANA_CL_PORT_VLAN_FILTER_CTRL_PRIO_CTAG_DIS  VTSS_BIT(9)

/** 
 * \brief
 * Discard frame if the investigated VLAN tag is a C-tag.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_PORT_VLAN_FILTER_CTRL . CTAG_DIS
 */
#define  VTSS_F_ANA_CL_PORT_VLAN_FILTER_CTRL_CTAG_DIS  VTSS_BIT(8)

/** 
 * \brief
 * Discard frame if the investigated VLAN TPDI is an S-tag (TPID=0x88A8)
 * and VID=0.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_PORT_VLAN_FILTER_CTRL . PRIO_STAG_DIS
 */
#define  VTSS_F_ANA_CL_PORT_VLAN_FILTER_CTRL_PRIO_STAG_DIS  VTSS_BIT(7)

/** 
 * \brief
 * Discard frame if the investigated VLAN TPID is
 * VLAN_STAG_CFG[0].STAG_ETYPE_VAL and VID=0.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_PORT_VLAN_FILTER_CTRL . PRIO_CUST1_STAG_DIS
 */
#define  VTSS_F_ANA_CL_PORT_VLAN_FILTER_CTRL_PRIO_CUST1_STAG_DIS  VTSS_BIT(6)

/** 
 * \brief
 * Discard frame if the investigated VLAN TPID is
 * VLAN_STAG_CFG[1].STAG_ETYPE_VAL and VID=0.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_PORT_VLAN_FILTER_CTRL . PRIO_CUST2_STAG_DIS
 */
#define  VTSS_F_ANA_CL_PORT_VLAN_FILTER_CTRL_PRIO_CUST2_STAG_DIS  VTSS_BIT(5)

/** 
 * \brief
 * Discard frame if the investigated VLAN TPID is
 * VLAN_STAG_CFG[2].STAG_ETYPE_VAL and VID=0.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_PORT_VLAN_FILTER_CTRL . PRIO_CUST3_STAG_DIS
 */
#define  VTSS_F_ANA_CL_PORT_VLAN_FILTER_CTRL_PRIO_CUST3_STAG_DIS  VTSS_BIT(4)

/** 
 * \brief
 * Discard frame if the investigated VLAN tag is an S-tag (TPID=0x88A8).
 *
 * \details 
 * Field: ::VTSS_ANA_CL_PORT_VLAN_FILTER_CTRL . STAG_DIS
 */
#define  VTSS_F_ANA_CL_PORT_VLAN_FILTER_CTRL_STAG_DIS  VTSS_BIT(3)

/** 
 * \brief
 * Discard frame if the investigated VLAN TPID is
 * VLAN_STAG_CFG[0].STAG_ETYPE_VAL.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_PORT_VLAN_FILTER_CTRL . CUST1_STAG_DIS
 */
#define  VTSS_F_ANA_CL_PORT_VLAN_FILTER_CTRL_CUST1_STAG_DIS  VTSS_BIT(2)

/** 
 * \brief
 * Discard frame if the investigated VLAN TPID is
 * VLAN_STAG_CFG[1].STAG_ETYPE_VAL.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_PORT_VLAN_FILTER_CTRL . CUST2_STAG_DIS
 */
#define  VTSS_F_ANA_CL_PORT_VLAN_FILTER_CTRL_CUST2_STAG_DIS  VTSS_BIT(1)

/** 
 * \brief
 * Discard frame if the investigated VLAN TPID is
 * VLAN_STAG_CFG[2].STAG_ETYPE_VAL.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_PORT_VLAN_FILTER_CTRL . CUST3_STAG_DIS
 */
#define  VTSS_F_ANA_CL_PORT_VLAN_FILTER_CTRL_CUST3_STAG_DIS  VTSS_BIT(0)


/** 
 * \brief Stacking configuration
 *
 * \details
 * Configure stacking awareness
 *
 * Register: \a ANA_CL:PORT:STACKING_CTRL
 *
 * @param gi Replicator: x_ANA_CL_NUM_PORT_CFG (??), 0-56
 */
#define VTSS_ANA_CL_PORT_STACKING_CTRL(gi)   VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x8000,gi,64,0,4)

/** 
 * \brief
 * Ingress port drop mode configuration. Applicable for front port only.
 *
 * \details 
 * 0 : Disable drop mode for the priority
 * 1 : Enable drop mode for the priority
 *
 * Field: ::VTSS_ANA_CL_PORT_STACKING_CTRL . IGR_DROP_ENA
 */
#define  VTSS_F_ANA_CL_PORT_STACKING_CTRL_IGR_DROP_ENA(x)  VTSS_ENCODE_BITFIELD(x,8,8)
#define  VTSS_M_ANA_CL_PORT_STACKING_CTRL_IGR_DROP_ENA     VTSS_ENCODE_BITMASK(8,8)
#define  VTSS_X_ANA_CL_PORT_STACKING_CTRL_IGR_DROP_ENA(x)  VTSS_EXTRACT_BITFIELD(x,8,8)

/** 
 * \brief
 * If set, and STACKING_AWARE_ENA=1 then any VStaX header in the frame is
 * assumed to contain an ISDX.
 * 
 * Otherwise the VStaX header is assumed to contain an AC.
 * 
 * Related parameters:
 * ANA_AC:PS_COMMON:COMMON_VSTAX_CFG.VSTAX2_MISC_ISDX_ENA
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_PORT_STACKING_CTRL . VSTAX_ISDX_ENA
 */
#define  VTSS_F_ANA_CL_PORT_STACKING_CTRL_VSTAX_ISDX_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Enable usage of stacking information.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_PORT_STACKING_CTRL . STACKING_AWARE_ENA
 */
#define  VTSS_F_ANA_CL_PORT_STACKING_CTRL_STACKING_AWARE_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Enable discard of frames received without a stacking header.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_PORT_STACKING_CTRL . STACKING_NON_HEADER_DISCARD_ENA
 */
#define  VTSS_F_ANA_CL_PORT_STACKING_CTRL_STACKING_NON_HEADER_DISCARD_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable discard of frames received with a stacking header.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_PORT_STACKING_CTRL . STACKING_HEADER_DISCARD_ENA
 */
#define  VTSS_F_ANA_CL_PORT_STACKING_CTRL_STACKING_HEADER_DISCARD_ENA  VTSS_BIT(0)


/** 
 * \brief TPID awareness configuration
 *
 * \details
 * Controls which TPID values are accepted as valid VLAN tags.
 *
 * Register: \a ANA_CL:PORT:VLAN_TPID_CTRL
 *
 * @param gi Replicator: x_ANA_CL_NUM_PORT_CFG (??), 0-56
 */
#define VTSS_ANA_CL_PORT_VLAN_TPID_CTRL(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x8000,gi,64,0,5)

/** 
 * \brief
 * Control which TPID values are accepted as valid VLAN tags.
 * 
 * If an incoming frame contains a TPID for which BASIC_TPID_AWARE_DIS is
 * set, then the TPID is treated as any other unknow protocol and thus no
 * further tags are identified.
 * 
 * Related parameters:
 * ANA_CL::VLAN_STAG_CFG[0-2]
 * ANA_CL:PORT:VLAN_TPID_CTRL.RT_TAG_CTRL
 * ANA_CL:PORT:VLAN_CTRL.VLAN_AWARE_ENA
 *
 * \details 
 * Bit0: First(outermost) TPID = 0x8100.
 * Bit1: First TPID = 0x88A8
 * Bit2: First TPID = ANA_CL::VLAN_STAG_CFG[0]
 * Bit3: First TPID = ANA_CL::VLAN_STAG_CFG[1]
 * Bit4: First TPID = ANA_CL::VLAN_STAG_CFG[2]
 * Bit5: Second TPID = 0x8100.
 * Bit6: Second TPID = 0x88A8
 * Bit7: Second TPID = ANA_CL::VLAN_STAG_CFG[0]
 * Bit8: Second TPID = ANA_CL::VLAN_STAG_CFG[1]
 * Bit9: Second TPID = ANA_CL::VLAN_STAG_CFG[2]
 * Bit10: Third TPID = 0x8100.
 * Bit11: Third TPID = 0x88A8
 * Bit12: Third TPID = ANA_CL::VLAN_STAG_CFG[0]
 * Bit13: Third TPID = ANA_CL::VLAN_STAG_CFG[1]
 * Bit14: Third TPID = ANA_CL::VLAN_STAG_CFG[2]
 *
 * Field: ::VTSS_ANA_CL_PORT_VLAN_TPID_CTRL . BASIC_TPID_AWARE_DIS
 */
#define  VTSS_F_ANA_CL_PORT_VLAN_TPID_CTRL_BASIC_TPID_AWARE_DIS(x)  VTSS_ENCODE_BITFIELD(x,4,15)
#define  VTSS_M_ANA_CL_PORT_VLAN_TPID_CTRL_BASIC_TPID_AWARE_DIS     VTSS_ENCODE_BITMASK(4,15)
#define  VTSS_X_ANA_CL_PORT_VLAN_TPID_CTRL_BASIC_TPID_AWARE_DIS(x)  VTSS_EXTRACT_BITFIELD(x,4,15)

/** 
 * \brief
 * Control the number of VLAN tags, which are accepted for frames to be
 * routed.
 * 
 * Related parameters:
 * ANA_CL:PORT:VLAN_TPID_CTRL.BASIC_TPID_AWARE_DIS
 * ANA_CL:PORT:VLAN_CTRL.VLAN_AWARE_ENA
 * ANA_CL::VLAN_STAG_CFG[0-2]

 *
 * \details 
 * Bit0: Route untagged frames.
 * Bit1: Route with one accepted tag (TPID is accepted by
 * BASIC_TPID_AWARE_DIS)
 * Bit2: Route with two accepted tags (TPID is accepted by
 * BASIC_TPID_AWARE_DIS)
 * Bit3: Route with three accepted tags (TPID is accepted by
 * BASIC_TPID_AWARE_DIS)
 *
 * Field: ::VTSS_ANA_CL_PORT_VLAN_TPID_CTRL . RT_TAG_CTRL
 */
#define  VTSS_F_ANA_CL_PORT_VLAN_TPID_CTRL_RT_TAG_CTRL(x)  VTSS_ENCODE_BITFIELD(x,0,4)
#define  VTSS_M_ANA_CL_PORT_VLAN_TPID_CTRL_RT_TAG_CTRL     VTSS_ENCODE_BITMASK(0,4)
#define  VTSS_X_ANA_CL_PORT_VLAN_TPID_CTRL_RT_TAG_CTRL(x)  VTSS_EXTRACT_BITFIELD(x,0,4)


/** 
 * \brief VLAN configuration
 *
 * \details
 * Port VLAN ID.
 *
 * Register: \a ANA_CL:PORT:VLAN_CTRL
 *
 * @param gi Replicator: x_ANA_CL_NUM_PORT_CFG (??), 0-56
 */
#define VTSS_ANA_CL_PORT_VLAN_CTRL(gi)       VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x8000,gi,64,0,6)

/** 
 * \brief
 * This configuration applies to VLAN tag awareness in the port VOE for
 * frames. Each bit corresponds to one of the known TPIDs.
 * If the incoming frame's outer tag contains a TPID for which
 * PORT_VOE_TPID_AWARE_DIS is set, then the port VOE sees the frame as
 * untagged.
 *
 * \details 
 * Bit0: TPID = 0x8100.
 * Bit1: TPID = 0x88A8
 * Bit2: TPID = ANA_CL:VLAN_STAG_CFG[0]
 * Bit3: TPID = ANA_CL:VLAN_STAG_CFG[1]
 * Bit4: TPID = ANA_CL:VLAN_STAG_CFG[2]
 *
 * Field: ::VTSS_ANA_CL_PORT_VLAN_CTRL . PORT_VOE_TPID_AWARE_DIS
 */
#define  VTSS_F_ANA_CL_PORT_VLAN_CTRL_PORT_VOE_TPID_AWARE_DIS(x)  VTSS_ENCODE_BITFIELD(x,26,5)
#define  VTSS_M_ANA_CL_PORT_VLAN_CTRL_PORT_VOE_TPID_AWARE_DIS     VTSS_ENCODE_BITMASK(26,5)
#define  VTSS_X_ANA_CL_PORT_VLAN_CTRL_PORT_VOE_TPID_AWARE_DIS(x)  VTSS_EXTRACT_BITFIELD(x,26,5)

/** 
 * \brief
 * Default PCP value used by the OAM port VOE. This value is used for port
 * VOE counter updates when no outer Q-Tag is present in a frame.
 *
 * \details 
 * n: OAM default PCP value
 *
 * Field: ::VTSS_ANA_CL_PORT_VLAN_CTRL . PORT_VOE_DEFAULT_PCP
 */
#define  VTSS_F_ANA_CL_PORT_VLAN_CTRL_PORT_VOE_DEFAULT_PCP(x)  VTSS_ENCODE_BITFIELD(x,23,3)
#define  VTSS_M_ANA_CL_PORT_VLAN_CTRL_PORT_VOE_DEFAULT_PCP     VTSS_ENCODE_BITMASK(23,3)
#define  VTSS_X_ANA_CL_PORT_VLAN_CTRL_PORT_VOE_DEFAULT_PCP(x)  VTSS_EXTRACT_BITFIELD(x,23,3)

/** 
 * \brief
 * Default DEI value used by the OAM port VOE. This value is used for port
 * VOE counter updates when no outer Q-Tag is present in a frame.
 *
 * \details 
 * n: OAM default DEI value
 *
 * Field: ::VTSS_ANA_CL_PORT_VLAN_CTRL . PORT_VOE_DEFAULT_DEI
 */
#define  VTSS_F_ANA_CL_PORT_VLAN_CTRL_PORT_VOE_DEFAULT_DEI  VTSS_BIT(22)

/** 
 * \brief
 * If set, the PCP_DEI_TRANS_CFG table is used for VLAN classification.
 * Otherwise, the frame's values are used directly.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_PORT_VLAN_CTRL . VLAN_PCP_DEI_TRANS_ENA
 */
#define  VTSS_F_ANA_CL_PORT_VLAN_CTRL_VLAN_PCP_DEI_TRANS_ENA  VTSS_BIT(21)

/** 
 * \brief
 * Selects the VLAN tag used for basic VLAN and QoS classification.
 *
 * \details 
 * 0: Use first tag (outer-most tag).
 * 1: Use second tag if present, otherwise use first tag.
 *
 * Field: ::VTSS_ANA_CL_PORT_VLAN_CTRL . VLAN_TAG_SEL
 */
#define  VTSS_F_ANA_CL_PORT_VLAN_CTRL_VLAN_TAG_SEL  VTSS_BIT(20)

/** 
 * \brief
 * Enable VLAN awareness for port. If VLAN unaware, the frame's VLAN tags
 * are not used for VLAN classification.
 * 
 * Related parameters:
 * ANA_CL::VLAN_STAG_CFG[0-2]
 * ANA_CL:PORT:VLAN_TPID_CTRL.BASIC_TPID_AWARE_DIS
 * ANA_CL:PORT:VLAN_TPID_CTRL.RT_TAG_CTRL
 *
 * \details 
 * 0: Disable (VLAN unaware)
 * 1: Enable (VLAN aware)
 *
 * Field: ::VTSS_ANA_CL_PORT_VLAN_CTRL . VLAN_AWARE_ENA
 */
#define  VTSS_F_ANA_CL_PORT_VLAN_CTRL_VLAN_AWARE_ENA  VTSS_BIT(19)

/** 
 * \brief
 * Number of tag headers to remove from ingress frame.
 *
 * \details 
 * 0: Keep all tags.
 * 1: Pop up to 1 tag if available.
 * 2: Pop up to 2 tags if available.
 * 3: Reserved.
 *
 * Field: ::VTSS_ANA_CL_PORT_VLAN_CTRL . VLAN_POP_CNT
 */
#define  VTSS_F_ANA_CL_PORT_VLAN_CTRL_VLAN_POP_CNT(x)  VTSS_ENCODE_BITFIELD(x,17,2)
#define  VTSS_M_ANA_CL_PORT_VLAN_CTRL_VLAN_POP_CNT     VTSS_ENCODE_BITMASK(17,2)
#define  VTSS_X_ANA_CL_PORT_VLAN_CTRL_VLAN_POP_CNT(x)  VTSS_EXTRACT_BITFIELD(x,17,2)

/** 
 * \brief
 * Default tag type for untagged frames. Also used if port is VLAN unaware.
 * The tag type is carried with the frame to the rewriter where the tag
 * type can be used when VLAN tagging the frame.
 *
 * \details 
 * 0: Tag type equals C-tag
 * 1: Tag type equals S-tag
 *
 * Field: ::VTSS_ANA_CL_PORT_VLAN_CTRL . PORT_TAG_TYPE
 */
#define  VTSS_F_ANA_CL_PORT_VLAN_CTRL_PORT_TAG_TYPE  VTSS_BIT(16)

/** 
 * \brief
 * Default PCP value for the port for untagged frames. Also used if port is
 * VLAN unaware.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_PORT_VLAN_CTRL . PORT_PCP
 */
#define  VTSS_F_ANA_CL_PORT_VLAN_CTRL_PORT_PCP(x)  VTSS_ENCODE_BITFIELD(x,13,3)
#define  VTSS_M_ANA_CL_PORT_VLAN_CTRL_PORT_PCP     VTSS_ENCODE_BITMASK(13,3)
#define  VTSS_X_ANA_CL_PORT_VLAN_CTRL_PORT_PCP(x)  VTSS_EXTRACT_BITFIELD(x,13,3)

/** 
 * \brief
 * Default DEI bit for the port for untagged frames. Also used if port is
 * VLAN unaware.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_PORT_VLAN_CTRL . PORT_DEI
 */
#define  VTSS_F_ANA_CL_PORT_VLAN_CTRL_PORT_DEI  VTSS_BIT(12)

/** 
 * \brief
 * Default VID value for the port for untagged frames. Also used if port is
 * VLAN unaware.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_PORT_VLAN_CTRL . PORT_VID
 */
#define  VTSS_F_ANA_CL_PORT_VLAN_CTRL_PORT_VID(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_ANA_CL_PORT_VLAN_CTRL_PORT_VID     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_ANA_CL_PORT_VLAN_CTRL_PORT_VID(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \brief DEI and PCP translation table
 *
 * \details
 * Translation of frame's DEI and PCP to classified DEI and PCP.
 * Configuration per DEI and PCP.
 * The use of this table is enabled in VLAN_CTRL.VLAN_PCP_DEI_TRANS.
 *
 * Register: \a ANA_CL:PORT:PCP_DEI_TRANS_CFG
 *
 * @param gi Replicator: x_ANA_CL_NUM_PORT_CFG (??), 0-56
 * @param ri Register: PCP_DEI_TRANS_CFG (??), 0-15
 */
#define VTSS_ANA_CL_PORT_PCP_DEI_TRANS_CFG(gi,ri)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x8000,gi,64,ri,7)

/** 
 * \brief
 * Translate VLAN PCP and DEI to a classified PCP: PCP =
 * PCP_DEI_TRANS_CFG[8*DEI + PCP].PCP_TRANS_VAL.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_PORT_PCP_DEI_TRANS_CFG . PCP_TRANS_VAL
 */
#define  VTSS_F_ANA_CL_PORT_PCP_DEI_TRANS_CFG_PCP_TRANS_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_ANA_CL_PORT_PCP_DEI_TRANS_CFG_PCP_TRANS_VAL     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_ANA_CL_PORT_PCP_DEI_TRANS_CFG_PCP_TRANS_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,3)

/** 
 * \brief
 * Translate VLAN PCP and DEI to a classified DEI: DEI =
 * PCP_DEI_TRANS_CFG[8*DEI + PCP].DEI_TRANS_VAL.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_PORT_PCP_DEI_TRANS_CFG . DEI_TRANS_VAL
 */
#define  VTSS_F_ANA_CL_PORT_PCP_DEI_TRANS_CFG_DEI_TRANS_VAL  VTSS_BIT(3)


/** 
 * \brief Port ID data
 *
 * \details
 * Configuration of GLAG and logical port number.
 *
 * Register: \a ANA_CL:PORT:PORT_ID_CFG
 *
 * @param gi Replicator: x_ANA_CL_NUM_PORT_CFG (??), 0-56
 */
#define VTSS_ANA_CL_PORT_PORT_ID_CFG(gi)     VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x8000,gi,64,0,23)

/** 
 * \brief
 * Global Link Aggregation Group (GLAG) number to be used in relation to
 * learning and forwarding.
 *
 * \details 
 * PORT_IS_GLAG_ENA=0:
 * Port does not participate in a GLAG
 * 
 * PORT_IS_GLAG_ENA=1:
 * 0: port is member of GLAG 0
 * 1: port is member of GLAG 1
 * ...
 * N: port is member of GLAG N
 *
 * Field: ::VTSS_ANA_CL_PORT_PORT_ID_CFG . GLAG_NUM
 */
#define  VTSS_F_ANA_CL_PORT_PORT_ID_CFG_GLAG_NUM(x)  VTSS_ENCODE_BITFIELD(x,8,5)
#define  VTSS_M_ANA_CL_PORT_PORT_ID_CFG_GLAG_NUM     VTSS_ENCODE_BITMASK(8,5)
#define  VTSS_X_ANA_CL_PORT_PORT_ID_CFG_GLAG_NUM(x)  VTSS_EXTRACT_BITFIELD(x,8,5)

/** 
 * \brief
 * Port is part of a Global Link Aggregation Gloup (GLAG).
 *
 * \details 
 * 0: Port is not globally link aggregated.
 * 1: Port is part of a GLAG.
 *
 * Field: ::VTSS_ANA_CL_PORT_PORT_ID_CFG . PORT_IS_GLAG_ENA
 */
#define  VTSS_F_ANA_CL_PORT_PORT_ID_CFG_PORT_IS_GLAG_ENA  VTSS_BIT(7)

/** 
 * \brief
 * Logical port number to be used in relation to classification, learning,
 * forwarding and policing.
 *
 * \details 
 * 0: Logical port 0
 * 1: Logical port 1
 * ...
 * n: Logical port n.
 *
 * Field: ::VTSS_ANA_CL_PORT_PORT_ID_CFG . LPORT_NUM
 */
#define  VTSS_F_ANA_CL_PORT_PORT_ID_CFG_LPORT_NUM(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_ANA_CL_PORT_PORT_ID_CFG_LPORT_NUM     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_ANA_CL_PORT_PORT_ID_CFG_LPORT_NUM(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief DEI and PCP mapping table
 *
 * \details
 * Mapping of frame's DEI and PCP to classified QoS class and drop
 * precedence level. Configuration per DEI, PCP.
 *
 * Register: \a ANA_CL:PORT:PCP_DEI_MAP_CFG
 *
 * @param gi Replicator: x_ANA_CL_NUM_PORT_CFG (??), 0-56
 * @param ri Register: PCP_DEI_MAP_CFG (??), 0-15
 */
#define VTSS_ANA_CL_PORT_PCP_DEI_MAP_CFG(gi,ri)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x8000,gi,64,ri,24)

/** 
 * \brief
 * Map VLAN PCP and DEI to a QoS class: QOS class = PCP_DEI_MAP_CFG[8*DEI +
 * PCP].PCP_DEI_QOS_VAL
 *
 * \details 
 * 0: Class 0 (lowest)
 * 1: Class 1
 * ...
 * n: Class n (highest).
 *
 * Field: ::VTSS_ANA_CL_PORT_PCP_DEI_MAP_CFG . PCP_DEI_QOS_VAL
 */
#define  VTSS_F_ANA_CL_PORT_PCP_DEI_MAP_CFG_PCP_DEI_QOS_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_ANA_CL_PORT_PCP_DEI_MAP_CFG_PCP_DEI_QOS_VAL     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_ANA_CL_PORT_PCP_DEI_MAP_CFG_PCP_DEI_QOS_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,3)

/** 
 * \brief
 * Map DEI and PCP to a DP level: DP level = PCP_DEI_MAP_CFG[8*DEI +
 * PCP].PCP_DEI_DP_VAL.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_PORT_PCP_DEI_MAP_CFG . PCP_DEI_DP_VAL
 */
#define  VTSS_F_ANA_CL_PORT_PCP_DEI_MAP_CFG_PCP_DEI_DP_VAL(x)  VTSS_ENCODE_BITFIELD(x,3,2)
#define  VTSS_M_ANA_CL_PORT_PCP_DEI_MAP_CFG_PCP_DEI_DP_VAL     VTSS_ENCODE_BITMASK(3,2)
#define  VTSS_X_ANA_CL_PORT_PCP_DEI_MAP_CFG_PCP_DEI_DP_VAL(x)  VTSS_EXTRACT_BITFIELD(x,3,2)


/** 
 * \brief QoS configuration
 *
 * \details
 * Configuration of basic QoS classification.
 *
 * Register: \a ANA_CL:PORT:QOS_CFG
 *
 * @param gi Replicator: x_ANA_CL_NUM_PORT_CFG (??), 0-56
 */
#define VTSS_ANA_CL_PORT_QOS_CFG(gi)         VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x8000,gi,64,0,40)

/** 
 * \brief
 * Select which DSCP values are updated according to internal QoS class.
 *
 * \details 
 * 0: Rewrite none
 * 1: Rewrite zero
 * 2: Rewrite selected 
 * 3: Rewrite all
 *
 * Field: ::VTSS_ANA_CL_PORT_QOS_CFG . DSCP_REWR_MODE_SEL
 */
#define  VTSS_F_ANA_CL_PORT_QOS_CFG_DSCP_REWR_MODE_SEL(x)  VTSS_ENCODE_BITFIELD(x,12,2)
#define  VTSS_M_ANA_CL_PORT_QOS_CFG_DSCP_REWR_MODE_SEL     VTSS_ENCODE_BITMASK(12,2)
#define  VTSS_X_ANA_CL_PORT_QOS_CFG_DSCP_REWR_MODE_SEL(x)  VTSS_EXTRACT_BITFIELD(x,12,2)

/** 
 * \brief
 * Enable ingress DSCP translation for QoS and DP classification.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_PORT_QOS_CFG . DSCP_TRANSLATE_ENA
 */
#define  VTSS_F_ANA_CL_PORT_QOS_CFG_DSCP_TRANSLATE_ENA  VTSS_BIT(11)

/** 
 * \brief
 * Setting this bit prevents the rewriter from remapping DSCP values for
 * frames from this port.
 *
 * \details 
 * 0: Allow rewriter to remap DSCP field
 * 1: Do not allow rewriter to remap of DSCP field
 *
 * Field: ::VTSS_ANA_CL_PORT_QOS_CFG . DSCP_KEEP_ENA
 */
#define  VTSS_F_ANA_CL_PORT_QOS_CFG_DSCP_KEEP_ENA  VTSS_BIT(10)

/** 
 * \brief
 * Setting this bit prevents the rewriter from making any changes to frames
 * from this port.
 * If a frame is CPU injected, the bit is from the injected IFH and the
 * configuration bit is overruled.
 *
 * \details 
 * 0: Allow rewriter to change the frame
 * 1: Do not allow rewriter to change the frame
 *
 * Field: ::VTSS_ANA_CL_PORT_QOS_CFG . KEEP_ENA
 */
#define  VTSS_F_ANA_CL_PORT_QOS_CFG_KEEP_ENA  VTSS_BIT(9)

/** 
 * \brief
 * Allow DP classification based on PCP and DEI for tagged frames.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_PORT_QOS_CFG . PCP_DEI_DP_ENA
 */
#define  VTSS_F_ANA_CL_PORT_QOS_CFG_PCP_DEI_DP_ENA  VTSS_BIT(8)

/** 
 * \brief
 * Allow QoS classification based on PCP and DEI from tagged frames.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_PORT_QOS_CFG . PCP_DEI_QOS_ENA
 */
#define  VTSS_F_ANA_CL_PORT_QOS_CFG_PCP_DEI_QOS_ENA  VTSS_BIT(7)

/** 
 * \brief
 * Allow DP classification based on DSCP.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_PORT_QOS_CFG . DSCP_DP_ENA
 */
#define  VTSS_F_ANA_CL_PORT_QOS_CFG_DSCP_DP_ENA  VTSS_BIT(6)

/** 
 * \brief
 * Allow QoS classification based on DSCP.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_PORT_QOS_CFG . DSCP_QOS_ENA
 */
#define  VTSS_F_ANA_CL_PORT_QOS_CFG_DSCP_QOS_ENA  VTSS_BIT(5)

/** 
 * \brief
 * Default port DP level.
 *
 * \details 
 * 0: DP 0 (disable)
 * 1: DP 1
 * ...
 * n: DP n (highest drop probability).
 *
 * Field: ::VTSS_ANA_CL_PORT_QOS_CFG . DEFAULT_DP_VAL
 */
#define  VTSS_F_ANA_CL_PORT_QOS_CFG_DEFAULT_DP_VAL(x)  VTSS_ENCODE_BITFIELD(x,3,2)
#define  VTSS_M_ANA_CL_PORT_QOS_CFG_DEFAULT_DP_VAL     VTSS_ENCODE_BITMASK(3,2)
#define  VTSS_X_ANA_CL_PORT_QOS_CFG_DEFAULT_DP_VAL(x)  VTSS_EXTRACT_BITFIELD(x,3,2)

/** 
 * \brief
 * Default port QoS class.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_PORT_QOS_CFG . DEFAULT_QOS_VAL
 */
#define  VTSS_F_ANA_CL_PORT_QOS_CFG_DEFAULT_QOS_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_ANA_CL_PORT_QOS_CFG_DEFAULT_QOS_VAL     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_ANA_CL_PORT_QOS_CFG_DEFAULT_QOS_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief CPU forward control
 *
 * \details
 * Configuration of CPU capturing of control frames.
 *
 * Register: \a ANA_CL:PORT:CAPTURE_CFG
 *
 * @param gi Replicator: x_ANA_CL_NUM_PORT_CFG (??), 0-56
 */
#define VTSS_ANA_CL_PORT_CAPTURE_CFG(gi)     VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x8000,gi,64,0,41)

/** 
 * \brief
 * This configuration applies to the CPU forwarding function of the basic
 * classifier. Each bit corresponds to one of the known TPIDs. If a bit is
 * set, the basic classifier does not CPU forward a frame if the frame's
 * outer VLAN tag contains the corresponding TPID.
 *
 * \details 
 * Bit0: TPID = 0x8100.
 * Bit1: TPID = 0x88A8
 * Bit2: TPID = ANA_CL:VLAN_STAG_CFG[0]
 * Bit3: TPID = ANA_CL:VLAN_STAG_CFG[1]
 * Bit4: TPID = ANA_CL:VLAN_STAG_CFG[2]
 *
 * Field: ::VTSS_ANA_CL_PORT_CAPTURE_CFG . CAPTURE_TPID_AWARE_DIS
 */
#define  VTSS_F_ANA_CL_PORT_CAPTURE_CFG_CAPTURE_TPID_AWARE_DIS(x)  VTSS_ENCODE_BITFIELD(x,7,5)
#define  VTSS_M_ANA_CL_PORT_CAPTURE_CFG_CAPTURE_TPID_AWARE_DIS     VTSS_ENCODE_BITMASK(7,5)
#define  VTSS_X_ANA_CL_PORT_CAPTURE_CFG_CAPTURE_TPID_AWARE_DIS(x)  VTSS_EXTRACT_BITFIELD(x,7,5)

/** 
 * \brief
 * If set, VRAP frames are redirected to the CPU extraction queue given by
 * CPU_PROTO_QU_CFG.CPU_VRAP_QU.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_PORT_CAPTURE_CFG . CPU_VRAP_REDIR_ENA
 */
#define  VTSS_F_ANA_CL_PORT_CAPTURE_CFG_CPU_VRAP_REDIR_ENA  VTSS_BIT(6)

/** 
 * \brief
 * Redirect ICMPv6 frames to the CPU.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_PORT_CAPTURE_CFG . IP6_ICMP_HOP_BY_HOP_REDIR_ENA
 */
#define  VTSS_F_ANA_CL_PORT_CAPTURE_CFG_IP6_ICMP_HOP_BY_HOP_REDIR_ENA  VTSS_BIT(5)

/** 
 * \brief
 * Redirect IPv6 frames with hop by hop options to the CPU.
 *
 * \details 
 * 0: Disable redirection
 * 1: Enable redirection to the CPU queue
 *
 * Field: ::VTSS_ANA_CL_PORT_CAPTURE_CFG . IP6_HOP_BY_HOP_REDIR_ENA
 */
#define  VTSS_F_ANA_CL_PORT_CAPTURE_CFG_IP6_HOP_BY_HOP_REDIR_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Control CPU redirection of MLD packets
 *
 * \details 
 * 0: Disable redirection
 * 1: Enable redirection to CPU queue.
 *
 * Field: ::VTSS_ANA_CL_PORT_CAPTURE_CFG . CPU_MLD_REDIR_ENA
 */
#define  VTSS_F_ANA_CL_PORT_CAPTURE_CFG_CPU_MLD_REDIR_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Control CPU copy of IPv6 Multicast control packets (DIP equals
 * FF02::/16).
 *
 * \details 
 * 0: Disable copy
 * 1: Enable copy to CPU queue
 *
 * Field: ::VTSS_ANA_CL_PORT_CAPTURE_CFG . CPU_IP6_MC_COPY_ENA
 */
#define  VTSS_F_ANA_CL_PORT_CAPTURE_CFG_CPU_IP6_MC_COPY_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Control CPU copy of IPv4 Multicast control packets
 *
 * \details 
 * 0: Disable copy
 * 1: Enable copy to CPU queue
 *
 * Field: ::VTSS_ANA_CL_PORT_CAPTURE_CFG . CPU_IP4_MC_COPY_ENA
 */
#define  VTSS_F_ANA_CL_PORT_CAPTURE_CFG_CPU_IP4_MC_COPY_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Control CPU redirection of IGMP frames.
 *
 * \details 
 * 0: Disable redirection
 * 1: Enable redirection to CPU queue
 *
 * Field: ::VTSS_ANA_CL_PORT_CAPTURE_CFG . CPU_IGMP_REDIR_ENA
 */
#define  VTSS_F_ANA_CL_PORT_CAPTURE_CFG_CPU_IGMP_REDIR_ENA  VTSS_BIT(0)


/** 
 * \brief IEEE802.1ag / ITU-T Y.1731 OAM frame filtering control
 *
 * \details
 * Configuration of CPU capturing of 802.1ag and Y.1731 control frames.
 *
 * Register: \a ANA_CL:PORT:CAPTURE_Y1731_AG_CFG
 *
 * @param gi Replicator: x_ANA_CL_NUM_PORT_CFG (??), 0-56
 */
#define VTSS_ANA_CL_PORT_CAPTURE_Y1731_AG_CFG(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x8000,gi,64,0,42)


/** 
 * \brief GXRP redirection control
 *
 * \details
 * Configuration of CPU capturing of GARP frames.
 *
 * Register: \a ANA_CL:PORT:CAPTURE_GXRP_CFG
 *
 * @param gi Replicator: x_ANA_CL_NUM_PORT_CFG (??), 0-56
 */
#define VTSS_ANA_CL_PORT_CAPTURE_GXRP_CFG(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x8000,gi,64,0,43)


/** 
 * \brief BPDU redirection control
 *
 * \details
 * Configuration of CPU capturing of bridge control frames.
 *
 * Register: \a ANA_CL:PORT:CAPTURE_BPDU_CFG
 *
 * @param gi Replicator: x_ANA_CL_NUM_PORT_CFG (??), 0-56
 */
#define VTSS_ANA_CL_PORT_CAPTURE_BPDU_CFG(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x8000,gi,64,0,44)


/** 
 * \brief VCAP CLM configuration
 *
 * \details
 * Configures which CLM key to use for different frame types, as well as
 * whether classified of frame values are used for DSCP and TCI. Replicated
 * per lookup in CLM.
 *
 * Register: \a ANA_CL:PORT:ADV_CL_CFG
 *
 * @param gi Replicator: x_ANA_CL_NUM_PORT_CFG (??), 0-56
 * @param ri Replicator: x_ANA_CLM_CNT (??), 0-5
 */
#define VTSS_ANA_CL_PORT_ADV_CL_CFG(gi,ri)   VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x8000,gi,64,ri,45)

/** 
 * \brief
 * Use classified VID, DEI and PCP instead of received TCI.
 *
 * \details 
 * 1: Enable
 * 0: Disable
 *
 * Field: ::VTSS_ANA_CL_PORT_ADV_CL_CFG . USE_CL_TCI0_ENA
 */
#define  VTSS_F_ANA_CL_PORT_ADV_CL_CFG_USE_CL_TCI0_ENA  VTSS_BIT(26)

/** 
 * \brief
 * Use classified DSCP instead of received DSCP.
 *
 * \details 
 * 1: Enable
 * 0: Disable
 *
 * Field: ::VTSS_ANA_CL_PORT_ADV_CL_CFG . USE_CL_DSCP_ENA
 */
#define  VTSS_F_ANA_CL_PORT_ADV_CL_CFG_USE_CL_DSCP_ENA  VTSS_BIT(25)

/** 
 * \brief
 * VCAP CLM key type to be used for IPv4 frames.
 *
 * \details 
 * 0: Follow ETYPE_CLM_KEY_SEL selection
 * 1: MLL
 * 2: SGL_MLBS
 * 3: DBL_MLBS
 * 4: TRI_MLBS
 * 5: TRI_VID
 * 6: LL_FULL
 * 7: NORMAL with SRC information
 * 8: NORMAL with DST information
 * 9: NORMAL_7TUPLE
 * 10 NORMAL_5TUPLE_IP4
 * 11 PURE_5TUPLE_IP4
 * 15: No Lookup
 * other: reserved
 *
 * Field: ::VTSS_ANA_CL_PORT_ADV_CL_CFG . IP4_CLM_KEY_SEL
 */
#define  VTSS_F_ANA_CL_PORT_ADV_CL_CFG_IP4_CLM_KEY_SEL(x)  VTSS_ENCODE_BITFIELD(x,21,4)
#define  VTSS_M_ANA_CL_PORT_ADV_CL_CFG_IP4_CLM_KEY_SEL     VTSS_ENCODE_BITMASK(21,4)
#define  VTSS_X_ANA_CL_PORT_ADV_CL_CFG_IP4_CLM_KEY_SEL(x)  VTSS_EXTRACT_BITFIELD(x,21,4)

/** 
 * \brief
 * VCAP CLM key type to be used for IPv6 frames.
 *
 * \details 
 * 0: Follow ETYPE_CLM_KEY_SEL selection
 * 1: MLL
 * 2: SGL_MLBS
 * 3: DBL_MLBS
 * 4: TRI_MLBS
 * 5: TRI_VID
 * 6: LL_FULL
 * 7: NORMAL with SRC information
 * 8: NORMAL with DST information
 * 9: NORMAL_7TUPLE
 * 10 NORMAL_5TUPLE_IP4
 * 11 PURE_5TUPLE_IP4
 * 15: No Lookup
 * other: reserved
 *
 * Field: ::VTSS_ANA_CL_PORT_ADV_CL_CFG . IP6_CLM_KEY_SEL
 */
#define  VTSS_F_ANA_CL_PORT_ADV_CL_CFG_IP6_CLM_KEY_SEL(x)  VTSS_ENCODE_BITFIELD(x,17,4)
#define  VTSS_M_ANA_CL_PORT_ADV_CL_CFG_IP6_CLM_KEY_SEL     VTSS_ENCODE_BITMASK(17,4)
#define  VTSS_X_ANA_CL_PORT_ADV_CL_CFG_IP6_CLM_KEY_SEL(x)  VTSS_EXTRACT_BITFIELD(x,17,4)

/** 
 * \brief
 * VCAP CLM key type to be used for unicast MPLS frames (EtherType =
 * 0x8847).
 *
 * \details 
 * 0: Follow ETYPE_CLM_KEY_SEL selection
 * 1: MLL
 * 2: SGL_MLBS
 * 3: DBL_MLBS
 * 4: TRI_MLBS
 * 5: TRI_VID
 * 6: LL_FULL
 * 7: NORMAL with SRC information
 * 8: NORMAL with DST information
 * 9: NORMAL_7TUPLE
 * 10 NORMAL_5TUPLE_IP4
 * 11 PURE_5TUPLE_IP4
 * 15: No Lookup
 * other: reserved
 *
 * Field: ::VTSS_ANA_CL_PORT_ADV_CL_CFG . MPLS_UC_CLM_KEY_SEL
 */
#define  VTSS_F_ANA_CL_PORT_ADV_CL_CFG_MPLS_UC_CLM_KEY_SEL(x)  VTSS_ENCODE_BITFIELD(x,13,4)
#define  VTSS_M_ANA_CL_PORT_ADV_CL_CFG_MPLS_UC_CLM_KEY_SEL     VTSS_ENCODE_BITMASK(13,4)
#define  VTSS_X_ANA_CL_PORT_ADV_CL_CFG_MPLS_UC_CLM_KEY_SEL(x)  VTSS_EXTRACT_BITFIELD(x,13,4)

/** 
 * \brief
 * VCAP CLM key type to be used for multicast MPLS frames (EtherType =
 * 0x8847).
 *
 * \details 
 * 0: Follow ETYPE_CLM_KEY_SEL selection
 * 1: MLL
 * 2: SGL_MLBS
 * 3: DBL_MLBS
 * 4: TRI_MLBS
 * 5: TRI_VID
 * 6: LL_FULL
 * 7: NORMAL with SRC information
 * 8: NORMAL with DST information
 * 9: NORMAL_7TUPLE
 * 10 NORMAL_5TUPLE_IP4
 * 11 PURE_5TUPLE_IP4
 * 15: No Lookup
 * other: reserved
 *
 * Field: ::VTSS_ANA_CL_PORT_ADV_CL_CFG . MPLS_MC_CLM_KEY_SEL
 */
#define  VTSS_F_ANA_CL_PORT_ADV_CL_CFG_MPLS_MC_CLM_KEY_SEL(x)  VTSS_ENCODE_BITFIELD(x,9,4)
#define  VTSS_M_ANA_CL_PORT_ADV_CL_CFG_MPLS_MC_CLM_KEY_SEL     VTSS_ENCODE_BITMASK(9,4)
#define  VTSS_X_ANA_CL_PORT_ADV_CL_CFG_MPLS_MC_CLM_KEY_SEL(x)  VTSS_EXTRACT_BITFIELD(x,9,4)

/** 
 * \brief
 * VCAP CLM key type to be used when current protocol layer is MPLS label
 * stack.
 *
 * \details 
 * 0: Follow ETYPE_CLM_KEY_SEL selection
 * 2: SGL_MLBS
 * 3: DBL_MLBS
 * 4: TRI_MLBS
 * 15: No Lookup
 * other: reserved
 *
 * Field: ::VTSS_ANA_CL_PORT_ADV_CL_CFG . MLBS_CLM_KEY_SEL
 */
#define  VTSS_F_ANA_CL_PORT_ADV_CL_CFG_MLBS_CLM_KEY_SEL(x)  VTSS_ENCODE_BITFIELD(x,5,4)
#define  VTSS_M_ANA_CL_PORT_ADV_CL_CFG_MLBS_CLM_KEY_SEL     VTSS_ENCODE_BITMASK(5,4)
#define  VTSS_X_ANA_CL_PORT_ADV_CL_CFG_MLBS_CLM_KEY_SEL(x)  VTSS_EXTRACT_BITFIELD(x,5,4)

/** 
 * \brief
 * VCAP CLM key type to be used for frame types other than MPLS and
 * IPv4/IPv6.
 *
 * \details 
 * 0: No Lookup
 * 1: MLL
 * 2: SGL_MLBS
 * 3: DBL_MLBS
 * 4: TRI_MLBS
 * 5: TRI_VID
 * 6: LL_FULL
 * 7: NORMAL with SRC information
 * 8: NORMAL with DST information
 * 9: NORMAL_7TUPLE
 * 10 NORMAL_5TUPLE_IP4
 * 11 PURE_5TUPLE_IP4
 * 15: No Lookup
 * other: reserved
 *
 * Field: ::VTSS_ANA_CL_PORT_ADV_CL_CFG . ETYPE_CLM_KEY_SEL
 */
#define  VTSS_F_ANA_CL_PORT_ADV_CL_CFG_ETYPE_CLM_KEY_SEL(x)  VTSS_ENCODE_BITFIELD(x,1,4)
#define  VTSS_M_ANA_CL_PORT_ADV_CL_CFG_ETYPE_CLM_KEY_SEL     VTSS_ENCODE_BITMASK(1,4)
#define  VTSS_X_ANA_CL_PORT_ADV_CL_CFG_ETYPE_CLM_KEY_SEL(x)  VTSS_EXTRACT_BITFIELD(x,1,4)

/** 
 * \brief
 * Controls VCAP CLM lookup.
 *
 * \details 
 * 1: Enable
 * 0: Disable
 *
 * Field: ::VTSS_ANA_CL_PORT_ADV_CL_CFG . LOOKUP_ENA
 */
#define  VTSS_F_ANA_CL_PORT_ADV_CL_CFG_LOOKUP_ENA  VTSS_BIT(0)

/**
 * Register Group: \a ANA_CL:COMMON
 *
 * Common configurations for all ports
 */


/** 
 * \brief Configure UPSID when stacking
 *
 * \details
 * Register: \a ANA_CL:COMMON:UPSID_CFG
 */
#define VTSS_ANA_CL_COMMON_UPSID_CFG         VTSS_IOREG(VTSS_TO_ANA_CL,0x7ea0)

/** 
 * \brief
 * Configures own unit port set ID (UPSID) to be used for stacking.
 * 
 * The configured value must be even.
 * 
 * Port numbers below 32 will use the configured (even) UPSID, whereas port
 * numbers >=32 will use the configured UPSID plus 1.
 * 
 * This must be configured consistently across the following registers:
 * ANA_CL::UPSID_CFG.UPSID_NUM
 * ANA_AC::COMMON_VSTAX_CFG.OWN_UPSID
 * ANA_L2::VSTAX_CTRL.OWN_UPSID
 * REW::COMMON_CTRL.OWN_UPSID
 *
 * \details 
 * Field: ::VTSS_ANA_CL_COMMON_UPSID_CFG . UPSID_NUM
 */
#define  VTSS_F_ANA_CL_COMMON_UPSID_CFG_UPSID_NUM(x)  VTSS_ENCODE_BITFIELD(x,0,5)
#define  VTSS_M_ANA_CL_COMMON_UPSID_CFG_UPSID_NUM     VTSS_ENCODE_BITMASK(0,5)
#define  VTSS_X_ANA_CL_COMMON_UPSID_CFG_UPSID_NUM(x)  VTSS_EXTRACT_BITFIELD(x,0,5)


/** 
 * \brief Aggregation calculation
 *
 * \details
 * This register determines which fields contribute to the calculation of
 * the 4-bit aggregation code. The aggregation code is used to select the
 * egress port if multiple ports are aggregated. Fields from the MAC,  IP
 * and UDP/TCP headers can be enabled. The enabled fields are XOR-ed
 * together to generate the final aggregation code.
 *
 * Register: \a ANA_CL:COMMON:AGGR_CFG
 */
#define VTSS_ANA_CL_COMMON_AGGR_CFG          VTSS_IOREG(VTSS_TO_ANA_CL,0x7ea1)

/** 
 * \brief
 * Enable / disable routed frames to update aggregation code.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_COMMON_AGGR_CFG . RT_UPD_VSTAX_AC_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_AGGR_CFG_RT_UPD_VSTAX_AC_ENA  VTSS_BIT(12)

/** 
 * \brief
 * Enable / disable aggregation calculations as in Jaguar1.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_COMMON_AGGR_CFG . SHORT_AGGR_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_AGGR_CFG_SHORT_AGGR_ENA  VTSS_BIT(11)

/** 
 * \brief
 * Allow the isdx value to contribute.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_COMMON_AGGR_CFG . AGGR_ISDX_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_AGGR_CFG_AGGR_ISDX_ENA  VTSS_BIT(10)

/** 
 * \brief
 * Use AC code received in the VSTAX header as aggregation code.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_COMMON_AGGR_CFG . AGGR_USE_VSTAX_AC_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_AGGR_CFG_AGGR_USE_VSTAX_AC_ENA  VTSS_BIT(9)

/** 
 * \brief
 * Allow reversed DMAC address contribution to aggregation code
 * calculation.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_COMMON_AGGR_CFG . AGGR_DMAC_REVERSED_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_AGGR_CFG_AGGR_DMAC_REVERSED_ENA  VTSS_BIT(8)

/** 
 * \brief
 * Enable a randomly generated aggregation code.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_COMMON_AGGR_CFG . AGGR_RND_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_AGGR_CFG_AGGR_RND_ENA  VTSS_BIT(7)

/** 
 * \brief
 * Enable source and destination IPv6es address to contribute to
 * aggregation code calculation.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_COMMON_AGGR_CFG . AGGR_IP6_SIPDIP_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_AGGR_CFG_AGGR_IP6_SIPDIP_ENA  VTSS_BIT(6)

/** 
 * \brief
 * Allow IPv6 flow label to contribute to aggregation code calculation.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_COMMON_AGGR_CFG . AGGR_IP6_FLOW_LBL_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_AGGR_CFG_AGGR_IP6_FLOW_LBL_ENA  VTSS_BIT(5)

/** 
 * \brief
 * Allow IPv6 UDP/TCP destination and source port number to contribute to
 * aggregation code calculation.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_COMMON_AGGR_CFG . AGGR_IP6_TCPUDP_PORT_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_AGGR_CFG_AGGR_IP6_TCPUDP_PORT_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Allow IPv4 UDP/TCP destination and source port number to contribute to
 * aggregation code calculation.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_COMMON_AGGR_CFG . AGGR_IP4_TCPUDP_PORT_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_AGGR_CFG_AGGR_IP4_TCPUDP_PORT_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Allow source and destination IPv4 addresses to contribute to aggregation
 * code calculation.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_COMMON_AGGR_CFG . AGGR_IP4_SIPDIP_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_AGGR_CFG_AGGR_IP4_SIPDIP_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Allow destination MAC address to contribute to aggregation code
 * calculation.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_COMMON_AGGR_CFG . AGGR_DMAC_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_AGGR_CFG_AGGR_DMAC_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Allow source MAC address to contribute to aggregation code calculation.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_COMMON_AGGR_CFG . AGGR_SMAC_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_AGGR_CFG_AGGR_SMAC_ENA  VTSS_BIT(0)


/** 
 * \brief Ethertype used for identifying service tagged frames
 *
 * \details
 * Register: \a ANA_CL:COMMON:VLAN_STAG_CFG
 *
 * @param ri Register: VLAN_STAG_CFG (??), 0-2
 */
#define VTSS_ANA_CL_COMMON_VLAN_STAG_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_CL,0x7ea2 + (ri))

/** 
 * \brief
 * Configurable S-TAG ethertype.
 * 
 * These must be configured identically in REW::TPID_CFG.TPID_VAL.
 * 
 * Related parameters:
 * ANA_CL:PORT:VLAN_TPID_CTRL.BASIC_TPID_AWARE_DIS
 * ANA_CL:PORT:VLAN_TPID_CTRL.RT_TAG_CTRL
 * ANA_CL:PORT:VLAN_CTRL.VLAN_AWARE_ENA
 * REW::TPID_CFG.TPID_VAL
 *
 * \details 
 * Field: ::VTSS_ANA_CL_COMMON_VLAN_STAG_CFG . STAG_ETYPE_VAL
 */
#define  VTSS_F_ANA_CL_COMMON_VLAN_STAG_CFG_STAG_ETYPE_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_ANA_CL_COMMON_VLAN_STAG_CFG_STAG_ETYPE_VAL     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_ANA_CL_COMMON_VLAN_STAG_CFG_STAG_ETYPE_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief CPU extraction queue configuration 
 *
 * \details
 * Register: \a ANA_CL:COMMON:CPU_PROTO_QU_CFG
 */
#define VTSS_ANA_CL_COMMON_CPU_PROTO_QU_CFG  VTSS_IOREG(VTSS_TO_ANA_CL,0x7ea5)

/** 
 * \brief
 * CPU extraction queue used for VRAP frames.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_CL_COMMON_CPU_PROTO_QU_CFG . CPU_VRAP_QU
 */
#define  VTSS_F_ANA_CL_COMMON_CPU_PROTO_QU_CFG_CPU_VRAP_QU(x)  VTSS_ENCODE_BITFIELD(x,16,3)
#define  VTSS_M_ANA_CL_COMMON_CPU_PROTO_QU_CFG_CPU_VRAP_QU     VTSS_ENCODE_BITMASK(16,3)
#define  VTSS_X_ANA_CL_COMMON_CPU_PROTO_QU_CFG_CPU_VRAP_QU(x)  VTSS_EXTRACT_BITFIELD(x,16,3)

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
 * Field: ::VTSS_ANA_CL_COMMON_CPU_PROTO_QU_CFG . CPU_HOP_BY_HOP_ICMP_QU
 */
#define  VTSS_F_ANA_CL_COMMON_CPU_PROTO_QU_CFG_CPU_HOP_BY_HOP_ICMP_QU(x)  VTSS_ENCODE_BITFIELD(x,12,3)
#define  VTSS_M_ANA_CL_COMMON_CPU_PROTO_QU_CFG_CPU_HOP_BY_HOP_ICMP_QU     VTSS_ENCODE_BITMASK(12,3)
#define  VTSS_X_ANA_CL_COMMON_CPU_PROTO_QU_CFG_CPU_HOP_BY_HOP_ICMP_QU(x)  VTSS_EXTRACT_BITFIELD(x,12,3)

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
 * Field: ::VTSS_ANA_CL_COMMON_CPU_PROTO_QU_CFG . CPU_MLD_QU
 */
#define  VTSS_F_ANA_CL_COMMON_CPU_PROTO_QU_CFG_CPU_MLD_QU(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_ANA_CL_COMMON_CPU_PROTO_QU_CFG_CPU_MLD_QU     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_ANA_CL_COMMON_CPU_PROTO_QU_CFG_CPU_MLD_QU(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * CPU queue number for IPv6 multicast control frames. These are identified
 * by a DIP in the range FF02::/16.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_CL_COMMON_CPU_PROTO_QU_CFG . CPU_IP6_MC_CTRL_QU
 */
#define  VTSS_F_ANA_CL_COMMON_CPU_PROTO_QU_CFG_CPU_IP6_MC_CTRL_QU(x)  VTSS_ENCODE_BITFIELD(x,19,3)
#define  VTSS_M_ANA_CL_COMMON_CPU_PROTO_QU_CFG_CPU_IP6_MC_CTRL_QU     VTSS_ENCODE_BITMASK(19,3)
#define  VTSS_X_ANA_CL_COMMON_CPU_PROTO_QU_CFG_CPU_IP6_MC_CTRL_QU(x)  VTSS_EXTRACT_BITFIELD(x,19,3)

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
 * Field: ::VTSS_ANA_CL_COMMON_CPU_PROTO_QU_CFG . CPU_IP4_MC_CTRL_QU
 */
#define  VTSS_F_ANA_CL_COMMON_CPU_PROTO_QU_CFG_CPU_IP4_MC_CTRL_QU(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_ANA_CL_COMMON_CPU_PROTO_QU_CFG_CPU_IP4_MC_CTRL_QU     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_ANA_CL_COMMON_CPU_PROTO_QU_CFG_CPU_IP4_MC_CTRL_QU(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

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
 * Field: ::VTSS_ANA_CL_COMMON_CPU_PROTO_QU_CFG . CPU_IGMP_QU
 */
#define  VTSS_F_ANA_CL_COMMON_CPU_PROTO_QU_CFG_CPU_IGMP_QU(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_ANA_CL_COMMON_CPU_PROTO_QU_CFG_CPU_IGMP_QU     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_ANA_CL_COMMON_CPU_PROTO_QU_CFG_CPU_IGMP_QU(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief CPU extraction queue per address of BPDU, GARP, and CCM frames.
 *
 * \details
 * The register instance number corresponds to the address of the extracted
 * frame. For instance: CPUQ_8021_CFG[4].CPUQ_BPDU_VAL is the CPU
 * extraction queue used for BPDU frames with address 01-80-C2-00-00-04.
 *
 * Register: \a ANA_CL:COMMON:CPU_8021_QU_CFG
 *
 * @param ri Register: CPU_8021_QU_CFG (??), 0-15
 */
#define VTSS_ANA_CL_COMMON_CPU_8021_QU_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_CL,0x7ea6 + (ri))

/** 
 * \brief
 * CPU queue number for 802.1ag and Y.1731 frames. These are identified by
 * a DMAC in the range 01-80-C2-00-00-30 to 01-80-C2-00-00-3F.
 *
 * \details 
 * 0: CPU queue 0
 * 1: CPU queue 1
 * ...
 * n: CPU queue n.
 *
 * Field: ::VTSS_ANA_CL_COMMON_CPU_8021_QU_CFG . CPU_Y1731_AG_QU
 */
#define  VTSS_F_ANA_CL_COMMON_CPU_8021_QU_CFG_CPU_Y1731_AG_QU(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_ANA_CL_COMMON_CPU_8021_QU_CFG_CPU_Y1731_AG_QU     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_ANA_CL_COMMON_CPU_8021_QU_CFG_CPU_Y1731_AG_QU(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

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
 * Field: ::VTSS_ANA_CL_COMMON_CPU_8021_QU_CFG . CPU_GXRP_QU
 */
#define  VTSS_F_ANA_CL_COMMON_CPU_8021_QU_CFG_CPU_GXRP_QU(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_ANA_CL_COMMON_CPU_8021_QU_CFG_CPU_GXRP_QU     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_ANA_CL_COMMON_CPU_8021_QU_CFG_CPU_GXRP_QU(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

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
 * Field: ::VTSS_ANA_CL_COMMON_CPU_8021_QU_CFG . CPU_BPDU_QU
 */
#define  VTSS_F_ANA_CL_COMMON_CPU_8021_QU_CFG_CPU_BPDU_QU(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_ANA_CL_COMMON_CPU_8021_QU_CFG_CPU_BPDU_QU     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_ANA_CL_COMMON_CPU_8021_QU_CFG_CPU_BPDU_QU(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief QOS per address of BPDU, GARP, and CCM frames.
 *
 * \details
 * The register instance number corresponds to the address of the extracted
 * frame. For instance: CPUQ_8021_CFG[4].CPUQ_BPDU_VAL is the CPU
 * extraction queue used for BPDU frames with address 01-80-C2-00-00-04.
 *
 * Register: \a ANA_CL:COMMON:CPU_8021_QOS_CFG
 *
 * @param ri Register: CPU_8021_QOS_CFG (??), 0-15
 */
#define VTSS_ANA_CL_COMMON_CPU_8021_QOS_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_CL,0x7eb6 + (ri))

/** 
 * \brief
 * Configures QoS class for frames with DMAC in Y1731_AG protocol range.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_COMMON_CPU_8021_QOS_CFG . Y1731_AG_QOS
 */
#define  VTSS_F_ANA_CL_COMMON_CPU_8021_QOS_CFG_Y1731_AG_QOS(x)  VTSS_ENCODE_BITFIELD(x,8,3)
#define  VTSS_M_ANA_CL_COMMON_CPU_8021_QOS_CFG_Y1731_AG_QOS     VTSS_ENCODE_BITMASK(8,3)
#define  VTSS_X_ANA_CL_COMMON_CPU_8021_QOS_CFG_Y1731_AG_QOS(x)  VTSS_EXTRACT_BITFIELD(x,8,3)

/** 
 * \brief
 * Configures QoS class for frames with DMAC in GXRP range.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_COMMON_CPU_8021_QOS_CFG . GXRP_QOS
 */
#define  VTSS_F_ANA_CL_COMMON_CPU_8021_QOS_CFG_GXRP_QOS(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_ANA_CL_COMMON_CPU_8021_QOS_CFG_GXRP_QOS     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_ANA_CL_COMMON_CPU_8021_QOS_CFG_GXRP_QOS(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

/** 
 * \brief
 * Configures QoS class for frames with DMAC in BPDU range.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_COMMON_CPU_8021_QOS_CFG . BPDU_QOS
 */
#define  VTSS_F_ANA_CL_COMMON_CPU_8021_QOS_CFG_BPDU_QOS(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_ANA_CL_COMMON_CPU_8021_QOS_CFG_BPDU_QOS     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_ANA_CL_COMMON_CPU_8021_QOS_CFG_BPDU_QOS(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief VRAP classifier configuration
 *
 * \details
 * Register: \a ANA_CL:COMMON:VRAP_CFG
 */
#define VTSS_ANA_CL_COMMON_VRAP_CFG          VTSS_IOREG(VTSS_TO_ANA_CL,0x7ec6)

/** 
 * \brief
 * If set, VRAP frames must be single VLAN tagged and the frame's VID must
 * match ANA::VRAP_CFG.VRAP_VID. If cleared, VRAP frames must be untagged.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_COMMON_VRAP_CFG . VRAP_VLAN_AWARE_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_VRAP_CFG_VRAP_VLAN_AWARE_ENA  VTSS_BIT(12)

/** 
 * \brief
 * VID value for VRAP frames.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_COMMON_VRAP_CFG . VRAP_VID
 */
#define  VTSS_F_ANA_CL_COMMON_VRAP_CFG_VRAP_VID(x)  VTSS_ENCODE_BITFIELD(x,0,12)
#define  VTSS_M_ANA_CL_COMMON_VRAP_CFG_VRAP_VID     VTSS_ENCODE_BITMASK(0,12)
#define  VTSS_X_ANA_CL_COMMON_VRAP_CFG_VRAP_VID(x)  VTSS_EXTRACT_BITFIELD(x,0,12)


/** 
 * \brief VRAP data
 *
 * \details
 * Register: \a ANA_CL:COMMON:VRAP_HDR_DATA
 */
#define VTSS_ANA_CL_COMMON_VRAP_HDR_DATA     VTSS_IOREG(VTSS_TO_ANA_CL,0x7ec7)


/** 
 * \brief VRAP mask
 *
 * \details
 * Register: \a ANA_CL:COMMON:VRAP_HDR_MASK
 */
#define VTSS_ANA_CL_COMMON_VRAP_HDR_MASK     VTSS_IOREG(VTSS_TO_ANA_CL,0x7ec8)


/** 
 * \brief Configuration of TCP range generation
 *
 * \details
 * Register: \a ANA_CL:COMMON:ADV_RNG_CTRL
 *
 * @param ri Replicator: x_ACL_NUM_TCP_RANGES (??), 0-7
 */
#define VTSS_ANA_CL_COMMON_ADV_RNG_CTRL(ri)  VTSS_IOREG(VTSS_TO_ANA_CL,0x7ec9 + (ri))

/** 
 * \brief
 * Selected field matched against the range
 *
 * \details 
 * 0: Idle (No match)
 * 1: TCP / UDP dport value is matched against range
 * 2: TCP / UDP sport value is matched against range
 * 3: TCP / UDP dport or sport values are matched against range
 * 4: Selected (ANA_CL::VLAN_CTRL.VLAN_TAG_SEL) received VID value is
 * matched against range
 * 5: Classified DSCP value is matched against range
 * 6: ETYPE value is matched against range.
 *
 * Field: ::VTSS_ANA_CL_COMMON_ADV_RNG_CTRL . RNG_TYPE_SEL
 */
#define  VTSS_F_ANA_CL_COMMON_ADV_RNG_CTRL_RNG_TYPE_SEL(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_ANA_CL_COMMON_ADV_RNG_CTRL_RNG_TYPE_SEL     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_ANA_CL_COMMON_ADV_RNG_CTRL_RNG_TYPE_SEL(x)  VTSS_EXTRACT_BITFIELD(x,0,3)


/** 
 * \brief Configuration of matcher range generation
 *
 * \details
 * Register: \a ANA_CL:COMMON:ADV_RNG_VALUE_CFG
 *
 * @param ri Replicator: x_ACL_NUM_TCP_RANGES (??), 0-7
 */
#define VTSS_ANA_CL_COMMON_ADV_RNG_VALUE_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_CL,0x7ed1 + (ri))

/** 
 * \brief
 * Upper range value
 *
 * \details 
 * Field: ::VTSS_ANA_CL_COMMON_ADV_RNG_VALUE_CFG . RNG_MAX_VALUE
 */
#define  VTSS_F_ANA_CL_COMMON_ADV_RNG_VALUE_CFG_RNG_MAX_VALUE(x)  VTSS_ENCODE_BITFIELD(x,16,16)
#define  VTSS_M_ANA_CL_COMMON_ADV_RNG_VALUE_CFG_RNG_MAX_VALUE     VTSS_ENCODE_BITMASK(16,16)
#define  VTSS_X_ANA_CL_COMMON_ADV_RNG_VALUE_CFG_RNG_MAX_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,16,16)

/** 
 * \brief
 * Lower range value
 *
 * \details 
 * Field: ::VTSS_ANA_CL_COMMON_ADV_RNG_VALUE_CFG . RNG_MIN_VALUE
 */
#define  VTSS_F_ANA_CL_COMMON_ADV_RNG_VALUE_CFG_RNG_MIN_VALUE(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_ANA_CL_COMMON_ADV_RNG_VALUE_CFG_RNG_MIN_VALUE     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_ANA_CL_COMMON_ADV_RNG_VALUE_CFG_RNG_MIN_VALUE(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief Common stack configuration
 *
 * \details
 * Register: \a ANA_CL:COMMON:COMMON_VSTAX_CFG
 */
#define VTSS_ANA_CL_COMMON_COMMON_VSTAX_CFG  VTSS_IOREG(VTSS_TO_ANA_CL,0x7ed9)

/** 
 * \brief
 * Enable / disable getting DSCP from VSTAX MISC field when encoding as AC.
 *
 * \details 
 * 0: Disable 
 * 1: Enable (VSTAX MISC contains DSCP)
 *
 * Field: ::VTSS_ANA_CL_COMMON_COMMON_VSTAX_CFG . VSTAX2_MISC_DSCP_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_COMMON_VSTAX_CFG_VSTAX2_MISC_DSCP_ENA  VTSS_BIT(0)


/** 
 * \brief Miscellaneous CLM configuration.
 *
 * \details
 * Miscellaneous CLM configuration.
 *
 * Register: \a ANA_CL:COMMON:CLM_MISC_CTRL
 */
#define VTSS_ANA_CL_COMMON_CLM_MISC_CTRL     VTSS_IOREG(VTSS_TO_ANA_CL,0x7eda)

/** 
 * \brief
 * Configures discard if too big single rotation seen.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_COMMON_CLM_MISC_CTRL . TOO_BIG_SGL_ROT_DIS
 */
#define  VTSS_F_ANA_CL_COMMON_CLM_MISC_CTRL_TOO_BIG_SGL_ROT_DIS  VTSS_BIT(20)

/** 
 * \brief
 * Force no CLM lookup if frame is discarded or redirected by CLM (this can
 * be overruled with IGR_PORT_CLM_FORCE_ENA).
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_COMMON_CLM_MISC_CTRL . FORCE_NO_CLM_FOR_BASIC_DIS
 */
#define  VTSS_F_ANA_CL_COMMON_CLM_MISC_CTRL_FORCE_NO_CLM_FOR_BASIC_DIS  VTSS_BIT(19)

/** 
 * \brief
 * Selects CLM key when forced key selection is performed via
 * LBK_CLM_FORCE_ENA or IGR_PORT_CLM_FORCE_ENA.
 * 
 * Notice: ISDX is available as G_IDX
 *
 * \details 
 * 0: Use NORMAL key with destination info
 * 1: Use NORMAL key with source info
 * 2: Use NORMAL_7TUPLE key
 * 3: Use NORMAL_5TUPLE_IP4 key
 *
 * Field: ::VTSS_ANA_CL_COMMON_CLM_MISC_CTRL . FORCED_KEY_SEL
 */
#define  VTSS_F_ANA_CL_COMMON_CLM_MISC_CTRL_FORCED_KEY_SEL(x)  VTSS_ENCODE_BITFIELD(x,17,2)
#define  VTSS_M_ANA_CL_COMMON_CLM_MISC_CTRL_FORCED_KEY_SEL     VTSS_ENCODE_BITMASK(17,2)
#define  VTSS_X_ANA_CL_COMMON_CLM_MISC_CTRL_FORCED_KEY_SEL(x)  VTSS_EXTRACT_BITFIELD(x,17,2)

/** 
 * \brief
 * Determine per CLM if lookup of looped frames are required.
 * Key selection is performed via FORCED_KEY_SEL.
 * 
 * Notice: ISDX is available as G_IDX.
 *
 * \details 
 * Bit 0: Force CLM lookup #0 with key selected by FORCED_KEY_SEL
 * Bit 1: Force CLM lookup #1 with key selected by FORCED_KEY_SEL
 * ...
 *
 * Field: ::VTSS_ANA_CL_COMMON_CLM_MISC_CTRL . LBK_CLM_FORCE_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_CLM_MISC_CTRL_LBK_CLM_FORCE_ENA(x)  VTSS_ENCODE_BITFIELD(x,11,6)
#define  VTSS_M_ANA_CL_COMMON_CLM_MISC_CTRL_LBK_CLM_FORCE_ENA     VTSS_ENCODE_BITMASK(11,6)
#define  VTSS_X_ANA_CL_COMMON_CLM_MISC_CTRL_LBK_CLM_FORCE_ENA(x)  VTSS_EXTRACT_BITFIELD(x,11,6)

/** 
 * \brief
 * Determine per CLM if disabled lookup due to pipeline handling should be
 * overule and performed anyway.
 * Key selection is performed via FORCED_KEY_SEL.
 * 
 * Notice: ISDX is available as G_IDX.
 *
 * \details 
 * Bit 0: Force CLM lookup #0 with key selected by FORCED_KEY_SEL
 * Bit 1: Force CLM lookup #1 with key selected by FORCED_KEY_SEL
 * ...
 *
 * Field: ::VTSS_ANA_CL_COMMON_CLM_MISC_CTRL . IGR_PORT_CLM_FORCE_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_CLM_MISC_CTRL_IGR_PORT_CLM_FORCE_ENA(x)  VTSS_ENCODE_BITFIELD(x,5,6)
#define  VTSS_M_ANA_CL_COMMON_CLM_MISC_CTRL_IGR_PORT_CLM_FORCE_ENA     VTSS_ENCODE_BITMASK(5,6)
#define  VTSS_X_ANA_CL_COMMON_CLM_MISC_CTRL_IGR_PORT_CLM_FORCE_ENA(x)  VTSS_EXTRACT_BITFIELD(x,5,6)

/** 
 * \brief
 * Force CLM lookup to use IGR_PORT_MASK_SEL=3 for looped frames instead of
 * IGR_PORT_MASK_SEL=1.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_COMMON_CLM_MISC_CTRL . LBK_IGR_MASK_SEL3_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_CLM_MISC_CTRL_LBK_IGR_MASK_SEL3_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Enable CLM key field IGR_PORT_MASK_SEL=2 for masqueraded frames.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_COMMON_CLM_MISC_CTRL . MASQ_IGR_MASK_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_CLM_MISC_CTRL_MASQ_IGR_MASK_ENA  VTSS_BIT(3)

/** 
 * \brief
 * Enable CLM key field IGR_PORT_MASK_SEL=3 for frames received with VStaX
 * header.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_COMMON_CLM_MISC_CTRL . FP_VS2_IGR_MASK_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_CLM_MISC_CTRL_FP_VS2_IGR_MASK_ENA  VTSS_BIT(2)

/** 
 * \brief
 * Enable CLM key field IGR_PORT_MASK_SEL=3 for frames from VD0 or VD1.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_COMMON_CLM_MISC_CTRL . VD_IGR_MASK_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_CLM_MISC_CTRL_VD_IGR_MASK_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable CLM key field IGR_PORT_MASK_SEL=3 for CPU injected frames.
 *
 * \details 
 * 0: Disable
 * 1: Enable.
 *
 * Field: ::VTSS_ANA_CL_COMMON_CLM_MISC_CTRL . CPU_IGR_MASK_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_CLM_MISC_CTRL_CPU_IGR_MASK_ENA  VTSS_BIT(0)


/** 
 * \brief HMD port configurations
 *
 * \details
 * Register: \a ANA_CL:COMMON:HM_CFG
 *
 * @param ri Replicator: x_FFL_ANA_NUM_HIH_DEVS (??), 0-3
 */
#define VTSS_ANA_CL_COMMON_HM_CFG(ri)        VTSS_IOREG(VTSS_TO_ANA_CL,0x7edb + (ri))

/** 
 * \brief
 * Contains the port number of the used HMD X port
 *
 * \details 
 * Field: ::VTSS_ANA_CL_COMMON_HM_CFG . HMD_PORT
 */
#define  VTSS_F_ANA_CL_COMMON_HM_CFG_HMD_PORT(x)  VTSS_ENCODE_BITFIELD(x,2,6)
#define  VTSS_M_ANA_CL_COMMON_HM_CFG_HMD_PORT     VTSS_ENCODE_BITMASK(2,6)
#define  VTSS_X_ANA_CL_COMMON_HM_CFG_HMD_PORT(x)  VTSS_EXTRACT_BITFIELD(x,2,6)

/** 
 * \brief
 * If set, CLM action MAP_KEY = 2 uses HIH.PCP and HIH.DEI instead of PCP
 * and DEI from the frame's third tag. This enables mapping HIH values to
 * internal values (DP, COS ID).
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_COMMON_HM_CFG . HM_FORCE_MODE_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_HM_CFG_HM_FORCE_MODE_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Enable the configured port as HMD port.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_COMMON_HM_CFG . HMD_PORT_VLD
 */
#define  VTSS_F_ANA_CL_COMMON_HM_CFG_HMD_PORT_VLD  VTSS_BIT(0)


/** 
 * \brief Various configuration per DSCP
 *
 * \details
 * Register: \a ANA_CL:COMMON:DSCP_CFG
 *
 * @param ri Register: DSCP_CFG (??), 0-63
 */
#define VTSS_ANA_CL_COMMON_DSCP_CFG(ri)      VTSS_IOREG(VTSS_TO_ANA_CL,0x7edf + (ri))

/** 
 * \brief
 * Translated DSCP value if translation is enabled for the port
 * (DSCP_TRANSLATE_ENA).
 *
 * \details 
 * Field: ::VTSS_ANA_CL_COMMON_DSCP_CFG . DSCP_TRANSLATE_VAL
 */
#define  VTSS_F_ANA_CL_COMMON_DSCP_CFG_DSCP_TRANSLATE_VAL(x)  VTSS_ENCODE_BITFIELD(x,7,6)
#define  VTSS_M_ANA_CL_COMMON_DSCP_CFG_DSCP_TRANSLATE_VAL     VTSS_ENCODE_BITMASK(7,6)
#define  VTSS_X_ANA_CL_COMMON_DSCP_CFG_DSCP_TRANSLATE_VAL(x)  VTSS_EXTRACT_BITFIELD(x,7,6)

/** 
 * \brief
 * Configure each DSCP value (0-63) to map to a QoS class.
 *
 * \details 
 * 0: Class 0 (lowest)
 * 1: Class 1
 * ...
 * n: Class n (highest).
 *
 * Field: ::VTSS_ANA_CL_COMMON_DSCP_CFG . DSCP_QOS_VAL
 */
#define  VTSS_F_ANA_CL_COMMON_DSCP_CFG_DSCP_QOS_VAL(x)  VTSS_ENCODE_BITFIELD(x,4,3)
#define  VTSS_M_ANA_CL_COMMON_DSCP_CFG_DSCP_QOS_VAL     VTSS_ENCODE_BITMASK(4,3)
#define  VTSS_X_ANA_CL_COMMON_DSCP_CFG_DSCP_QOS_VAL(x)  VTSS_EXTRACT_BITFIELD(x,4,3)

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
 * Field: ::VTSS_ANA_CL_COMMON_DSCP_CFG . DSCP_DP_VAL
 */
#define  VTSS_F_ANA_CL_COMMON_DSCP_CFG_DSCP_DP_VAL(x)  VTSS_ENCODE_BITFIELD(x,2,2)
#define  VTSS_M_ANA_CL_COMMON_DSCP_CFG_DSCP_DP_VAL     VTSS_ENCODE_BITMASK(2,2)
#define  VTSS_X_ANA_CL_COMMON_DSCP_CFG_DSCP_DP_VAL(x)  VTSS_EXTRACT_BITFIELD(x,2,2)

/** 
 * \brief
 * If enabled then the DSCP value will be overwritten with the value
 * corresponding to the internal QoS if DSCP_REWR_MODE_SEL is set to
 * selected.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_COMMON_DSCP_CFG . DSCP_REWR_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_DSCP_CFG_DSCP_REWR_ENA  VTSS_BIT(1)

/** 
 * \brief
 * If enabled then the QoS corresponding to the DSCP value will be used
 * based on DSCP_QOS_ENA.
 *
 * \details 
 * 0: Disable
 * 1: Enable
 *
 * Field: ::VTSS_ANA_CL_COMMON_DSCP_CFG . DSCP_TRUST_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_DSCP_CFG_DSCP_TRUST_ENA  VTSS_BIT(0)


/** 
 * \brief Configuration per QoS class and DP level
 *
 * \details
 * Register: \a ANA_CL:COMMON:QOS_MAP_CFG
 *
 * @param ri Register: QOS_MAP_CFG (??), 0-31
 */
#define VTSS_ANA_CL_COMMON_QOS_MAP_CFG(ri)   VTSS_IOREG(VTSS_TO_ANA_CL,0x7f1f + (ri))

/** 
 * \brief
 * Table values for rewriting DSCP values using QoS class if enabled.
 *
 * \details 
 * DSCP value for QoS class n
 *
 * Field: ::VTSS_ANA_CL_COMMON_QOS_MAP_CFG . DSCP_REWR_VAL
 */
#define  VTSS_F_ANA_CL_COMMON_QOS_MAP_CFG_DSCP_REWR_VAL(x)  VTSS_ENCODE_BITFIELD(x,4,6)
#define  VTSS_M_ANA_CL_COMMON_QOS_MAP_CFG_DSCP_REWR_VAL     VTSS_ENCODE_BITMASK(4,6)
#define  VTSS_X_ANA_CL_COMMON_QOS_MAP_CFG_DSCP_REWR_VAL(x)  VTSS_EXTRACT_BITFIELD(x,4,6)


/** 
 * \brief Configuration per reserved MPLS label
 *
 * \details
 * This register allows enabling each of the 16 Reserved MPLS Labels (Label
 * Value < 16) for exception handling.
 * If a label is enabled, it will be forwarded to the queue configured in :
 * RSVD_CPU_QUEUE.
 * 
 * The RESERVED LABELS have been assigned the following meaning (RFC 3032).
 * -------------------------------------------------------------------
 * 0: IPv4 Explicit NULL Label
 * 1: Router Alert Label (RAL)
 * 2: IPv6 Explicit NULL Label 
 * 3: Reserved (Implicit NULL Label - Never in stack)
 * 4: (Reserved - Not yet assigned) 
 * 5: (Reserved - Not yet assigned) 
 * 6: (Reserved - Not yet assigned) 
 * 7: (Reserved - Not yet assigned) 
 * 8: (Reserved - Not yet assigned) 
 * 9: (Reserved - Not yet assigned) 
 * 10: (Reserved - Not yet assigned) 
 * 11: (Reserved - Not yet assigned) 
 * 12: (Reserved - Not yet assigned) 
 * 13: Generic Alert Label (GAL)
 * 14: OAM Alert Label (OAL)
 * 15: (Reserved - Not yet assigned) 
 * -------------------------------------------------------------------
 *
 * Register: \a ANA_CL:COMMON:MPLS_RSV_LBL_CFG
 *
 * @param ri Register: MPLS_RSV_LBL_CFG (??), 0-15
 */
#define VTSS_ANA_CL_COMMON_MPLS_RSV_LBL_CFG(ri)  VTSS_IOREG(VTSS_TO_ANA_CL,0x7f3f + (ri))

/** 
 * \brief
 * Enable This Reserved Label for OAM.
 * If enabled for OAM the MPLS frame will be forwarded to the CPU, if this
 * reserved label is found within scope in the Label Stack.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_COMMON_MPLS_RSV_LBL_CFG . RSVD_LBL_REDIR_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_MPLS_RSV_LBL_CFG_RSVD_LBL_REDIR_ENA  VTSS_BIT(4)

/** 
 * \brief
 * If the MPLS frame is forwarded to the CPU with this RESERVED LABEL in
 * the stack, it will be forwarded to the CPU queue configured in this
 * register.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_COMMON_MPLS_RSV_LBL_CFG . RSVD_CPU_QU
 */
#define  VTSS_F_ANA_CL_COMMON_MPLS_RSV_LBL_CFG_RSVD_CPU_QU(x)  VTSS_ENCODE_BITFIELD(x,1,3)
#define  VTSS_M_ANA_CL_COMMON_MPLS_RSV_LBL_CFG_RSVD_CPU_QU     VTSS_ENCODE_BITMASK(1,3)
#define  VTSS_X_ANA_CL_COMMON_MPLS_RSV_LBL_CFG_RSVD_CPU_QU(x)  VTSS_EXTRACT_BITFIELD(x,1,3)

/** 
 * \brief
 * Enable skipping of reserved label during label extract.
 * 
 * In order for a reserved label to be skipped,both
 * MPLS_RSV_LBL_CFG[<label>].RSVD_LBL_SKIP_ENA
 * and 
 * MPLS_MISC_CFG.CLM_RSVD_LBL_SKIP_ENA[<clm idx>]
 * must be set.

 *
 * \details 
 * 0: Allow reserved label to be part of MPLS label keys
 * 0: Reserved label will be skipped when generating MPLS label keys
 *
 * Field: ::VTSS_ANA_CL_COMMON_MPLS_RSV_LBL_CFG . RSVD_LBL_SKIP_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_MPLS_RSV_LBL_CFG_RSVD_LBL_SKIP_ENA  VTSS_BIT(0)


/** 
 * \details
 * Miscellaneous MPLS configuration.
 *
 * Register: \a ANA_CL:COMMON:MPLS_MISC_CFG
 */
#define VTSS_ANA_CL_COMMON_MPLS_MISC_CFG     VTSS_IOREG(VTSS_TO_ANA_CL,0x7f4f)

/** 
 * \brief
 * Enable skipping of reserved label during label extract.
 * 
 * In order for a reserved label to be skipped,either
 * MPLS_RSV_LBL_CFG[<label>].RSVD_LBL_SKIP_ENA
 * or 
 * MPLS_MISC_CFG.CLM_RSVD_LBL_SKIP_ENA[<clm idx>]
 * must be set.

 *
 * \details 
 * Bit 0: Skip reserved label when generating MPLS label keys for CLM
 * lookup #0.
 * Bit 1: Skip reserved label when generating MPLS label keys for CLM
 * lookup #1.
 * ...
 *
 * Field: ::VTSS_ANA_CL_COMMON_MPLS_MISC_CFG . CLM_RSVD_LBL_SKIP_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_MPLS_MISC_CFG_CLM_RSVD_LBL_SKIP_ENA(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_ANA_CL_COMMON_MPLS_MISC_CFG_CLM_RSVD_LBL_SKIP_ENA     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_ANA_CL_COMMON_MPLS_MISC_CFG_CLM_RSVD_LBL_SKIP_ENA(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief Various MPLS configuration
 *
 * \details
 * Register: \a ANA_CL:COMMON:MPLS_CFG
 */
#define VTSS_ANA_CL_COMMON_MPLS_CFG          VTSS_IOREG(VTSS_TO_ANA_CL,0x7f50)

/** 
 * \brief
 * If set only a valid selected TC_VAL via CLM tc_label can be used for
 * looking up in the mapping table.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_COMMON_MPLS_CFG . MPLS_SEL_TC_ONLY_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_MPLS_CFG_MPLS_SEL_TC_ONLY_ENA  VTSS_BIT(19)

/** 
 * \brief
 * CPU copy of frames when IP frame received as LSR POP (CLM FWD_TYPE=3)
 * with (nxt_type_after_offset = CW).
 *
 * \details 
 * Field: ::VTSS_ANA_CL_COMMON_MPLS_CFG . CPU_MPLS_IP_TRAFFIC_QU
 */
#define  VTSS_F_ANA_CL_COMMON_MPLS_CFG_CPU_MPLS_IP_TRAFFIC_QU(x)  VTSS_ENCODE_BITFIELD(x,16,3)
#define  VTSS_M_ANA_CL_COMMON_MPLS_CFG_CPU_MPLS_IP_TRAFFIC_QU     VTSS_ENCODE_BITMASK(16,3)
#define  VTSS_X_ANA_CL_COMMON_MPLS_CFG_CPU_MPLS_IP_TRAFFIC_QU(x)  VTSS_EXTRACT_BITFIELD(x,16,3)

/** 
 * \brief
 * Selects CPU queue when error occured when fwd_type = 3.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_COMMON_MPLS_CFG . CPU_MPLS_OAM_MEP_ERR_QU
 */
#define  VTSS_F_ANA_CL_COMMON_MPLS_CFG_CPU_MPLS_OAM_MEP_ERR_QU(x)  VTSS_ENCODE_BITFIELD(x,13,3)
#define  VTSS_M_ANA_CL_COMMON_MPLS_CFG_CPU_MPLS_OAM_MEP_ERR_QU     VTSS_ENCODE_BITMASK(13,3)
#define  VTSS_X_ANA_CL_COMMON_MPLS_CFG_CPU_MPLS_OAM_MEP_ERR_QU(x)  VTSS_EXTRACT_BITFIELD(x,13,3)

/** 
 * \brief
 * CPU copy of frames when MPLS OAM MEP err occur.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_COMMON_MPLS_CFG . CPU_MPLS_OAM_MEP_ERR_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_MPLS_CFG_CPU_MPLS_OAM_MEP_ERR_ENA  VTSS_BIT(12)

/** 
 * \brief
 * Selects CPU queue when error occured when fwd_type = 3.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_COMMON_MPLS_CFG . CPU_MPLS_POP_ERR_QU
 */
#define  VTSS_F_ANA_CL_COMMON_MPLS_CFG_CPU_MPLS_POP_ERR_QU(x)  VTSS_ENCODE_BITFIELD(x,9,3)
#define  VTSS_M_ANA_CL_COMMON_MPLS_CFG_CPU_MPLS_POP_ERR_QU     VTSS_ENCODE_BITMASK(9,3)
#define  VTSS_X_ANA_CL_COMMON_MPLS_CFG_CPU_MPLS_POP_ERR_QU(x)  VTSS_EXTRACT_BITFIELD(x,9,3)

/** 
 * \brief
 * CPU copy of frames when MPLS POP err occur.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_COMMON_MPLS_CFG . CPU_MPLS_POP_ERR_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_MPLS_CFG_CPU_MPLS_POP_ERR_ENA  VTSS_BIT(8)

/** 
 * \brief
 * Selects CPU queue when error occured when fwd_type = 2.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_COMMON_MPLS_CFG . CPU_MPLS_SWAP_ERR_QU
 */
#define  VTSS_F_ANA_CL_COMMON_MPLS_CFG_CPU_MPLS_SWAP_ERR_QU(x)  VTSS_ENCODE_BITFIELD(x,5,3)
#define  VTSS_M_ANA_CL_COMMON_MPLS_CFG_CPU_MPLS_SWAP_ERR_QU     VTSS_ENCODE_BITMASK(5,3)
#define  VTSS_X_ANA_CL_COMMON_MPLS_CFG_CPU_MPLS_SWAP_ERR_QU(x)  VTSS_EXTRACT_BITFIELD(x,5,3)

/** 
 * \brief
 * CPU copy of frames when MPLS swap err occur.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_COMMON_MPLS_CFG . CPU_MPLS_SWAP_ERR_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_MPLS_CFG_CPU_MPLS_SWAP_ERR_ENA  VTSS_BIT(4)

/** 
 * \brief
 * Selects CPU queue when error occured when fwd_type = PW.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_COMMON_MPLS_CFG . CPU_MPLS_PW_ERR_QU
 */
#define  VTSS_F_ANA_CL_COMMON_MPLS_CFG_CPU_MPLS_PW_ERR_QU(x)  VTSS_ENCODE_BITFIELD(x,1,3)
#define  VTSS_M_ANA_CL_COMMON_MPLS_CFG_CPU_MPLS_PW_ERR_QU     VTSS_ENCODE_BITMASK(1,3)
#define  VTSS_X_ANA_CL_COMMON_MPLS_CFG_CPU_MPLS_PW_ERR_QU(x)  VTSS_EXTRACT_BITFIELD(x,1,3)

/** 
 * \brief
 * CPU copy of frames when termination PW (fwd_type = PW).
 *
 * \details 
 * Field: ::VTSS_ANA_CL_COMMON_MPLS_CFG . CPU_MPLS_PW_ERR_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_MPLS_CFG_CPU_MPLS_PW_ERR_ENA  VTSS_BIT(0)


/** 
 * \brief Various OAM configuration
 *
 * \details
 * Register: \a ANA_CL:COMMON:OAM_CFG
 */
#define VTSS_ANA_CL_COMMON_OAM_CFG           VTSS_IOREG(VTSS_TO_ANA_CL,0x7f51)

/** 
 * \brief
 * Reserved Label used for PW VCCV2 OAM channel.
 * Default is to use (Router Alert Label = 1)
 *
 * \details 
 * Field: ::VTSS_ANA_CL_COMMON_OAM_CFG . VCCV2_LABEL
 */
#define  VTSS_F_ANA_CL_COMMON_OAM_CFG_VCCV2_LABEL(x)  VTSS_ENCODE_BITFIELD(x,1,4)
#define  VTSS_M_ANA_CL_COMMON_OAM_CFG_VCCV2_LABEL     VTSS_ENCODE_BITMASK(1,4)
#define  VTSS_X_ANA_CL_COMMON_OAM_CFG_VCCV2_LABEL(x)  VTSS_EXTRACT_BITFIELD(x,1,4)

/** 
 * \brief
 * Used to enable VCCV2 OAM signalling.
 * If not enabled, VCCV2 signalling can not be configured.
 *
 * \details 
 * 0: Disable VCCV2 signalling
 * 1: Enable VCCV2 signalling
 *
 * Field: ::VTSS_ANA_CL_COMMON_OAM_CFG . VCCV2_ENA
 */
#define  VTSS_F_ANA_CL_COMMON_OAM_CFG_VCCV2_ENA  VTSS_BIT(0)

/**
 * Register Group: \a ANA_CL:MIP_TBL
 *
 * MIP table
 */


/** 
 * \brief MIP configuration
 *
 * \details
 * Register: \a ANA_CL:MIP_TBL:MIP_CFG
 *
 * @param gi Replicator: x_ANA_CL_NUM_MIP_TBL (??), 0-1023
 */
#define VTSS_ANA_CL_MIP_TBL_MIP_CFG(gi)      VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x6000,gi,4,0,0)

/** 
 * \brief
 * MEL value for the MIP.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_MIP_TBL_MIP_CFG . MEL_VAL
 */
#define  VTSS_F_ANA_CL_MIP_TBL_MIP_CFG_MEL_VAL(x)  VTSS_ENCODE_BITFIELD(x,19,3)
#define  VTSS_M_ANA_CL_MIP_TBL_MIP_CFG_MEL_VAL     VTSS_ENCODE_BITMASK(19,3)
#define  VTSS_X_ANA_CL_MIP_TBL_MIP_CFG_MEL_VAL(x)  VTSS_EXTRACT_BITFIELD(x,19,3)

/** 
 * \brief
 * If set, OAM Y.1731 CCM frames with the correct encapsulation and the
 * correct MEL are copied to the CPU.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_MIP_TBL_MIP_CFG . CCM_COPY_ENA
 */
#define  VTSS_F_ANA_CL_MIP_TBL_MIP_CFG_CCM_COPY_ENA  VTSS_BIT(18)

/** 
 * \brief
 * If set, OAM Y.1731 LBM frames with the correct encapsulation, the
 * correct MEL, and the correct destination MAC address are redirected to
 * the CPU.

 *
 * \details 
 * Field: ::VTSS_ANA_CL_MIP_TBL_MIP_CFG . LBM_REDIR_ENA
 */
#define  VTSS_F_ANA_CL_MIP_TBL_MIP_CFG_LBM_REDIR_ENA  VTSS_BIT(17)

/** 
 * \brief
 * If set, OAM Y.1731 LTM frames with the correct encapsulation and the
 * correct MEL are redirected to the CPU.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_MIP_TBL_MIP_CFG . LTM_REDIR_ENA
 */
#define  VTSS_F_ANA_CL_MIP_TBL_MIP_CFG_LTM_REDIR_ENA  VTSS_BIT(16)

/** 
 * \brief
 * Handling of OAM Y.1731 frames with OpCode=RAPS, correct encapsulation,
 * and correct MEL.
 *
 * \details 
 * 
 * 0: No handling
 * 1: Copy to CPU
 * 2: Redirect to CPU
 * 3: Discard
 *
 * Field: ::VTSS_ANA_CL_MIP_TBL_MIP_CFG . RAPS_CFG
 */
#define  VTSS_F_ANA_CL_MIP_TBL_MIP_CFG_RAPS_CFG(x)  VTSS_ENCODE_BITFIELD(x,14,2)
#define  VTSS_M_ANA_CL_MIP_TBL_MIP_CFG_RAPS_CFG     VTSS_ENCODE_BITMASK(14,2)
#define  VTSS_X_ANA_CL_MIP_TBL_MIP_CFG_RAPS_CFG(x)  VTSS_EXTRACT_BITFIELD(x,14,2)

/** 
 * \brief
 * Generic Opcode. See GENERIC_OPCODE_CFG.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_MIP_TBL_MIP_CFG . GENERIC_OPCODE_VAL
 */
#define  VTSS_F_ANA_CL_MIP_TBL_MIP_CFG_GENERIC_OPCODE_VAL(x)  VTSS_ENCODE_BITFIELD(x,6,8)
#define  VTSS_M_ANA_CL_MIP_TBL_MIP_CFG_GENERIC_OPCODE_VAL     VTSS_ENCODE_BITMASK(6,8)
#define  VTSS_X_ANA_CL_MIP_TBL_MIP_CFG_GENERIC_OPCODE_VAL(x)  VTSS_EXTRACT_BITFIELD(x,6,8)

/** 
 * \brief
 * Handling of OAM Y.1731 frames with OpCode=GENERIC_OPCODE_VAL, correct
 * encapsulation, and correct MEL.
 *
 * \details 
 * 
 * 0: No handling
 * 1: Copy to CPU
 * 2: Redirect to CPU
 * 3: Discard
 *
 * Field: ::VTSS_ANA_CL_MIP_TBL_MIP_CFG . GENERIC_OPCODE_CFG
 */
#define  VTSS_F_ANA_CL_MIP_TBL_MIP_CFG_GENERIC_OPCODE_CFG(x)  VTSS_ENCODE_BITFIELD(x,4,2)
#define  VTSS_M_ANA_CL_MIP_TBL_MIP_CFG_GENERIC_OPCODE_CFG     VTSS_ENCODE_BITMASK(4,2)
#define  VTSS_X_ANA_CL_MIP_TBL_MIP_CFG_GENERIC_OPCODE_CFG(x)  VTSS_EXTRACT_BITFIELD(x,4,2)

/** 
 * \brief
 * CPU extraction queue when frame is forwarded to CPU.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_MIP_TBL_MIP_CFG . CPU_MIP_QU
 */
#define  VTSS_F_ANA_CL_MIP_TBL_MIP_CFG_CPU_MIP_QU(x)  VTSS_ENCODE_BITFIELD(x,1,3)
#define  VTSS_M_ANA_CL_MIP_TBL_MIP_CFG_CPU_MIP_QU     VTSS_ENCODE_BITMASK(1,3)
#define  VTSS_X_ANA_CL_MIP_TBL_MIP_CFG_CPU_MIP_QU(x)  VTSS_EXTRACT_BITFIELD(x,1,3)

/** 
 * \brief
 * MIP location.
 *
 * \details 
 * 0: ANA_IN_MIP
 * 1: ANA_OU_MIP
 *
 * Field: ::VTSS_ANA_CL_MIP_TBL_MIP_CFG . PIPELINE_PT
 */
#define  VTSS_F_ANA_CL_MIP_TBL_MIP_CFG_PIPELINE_PT  VTSS_BIT(0)


/** 
 * \brief MAC address - bits 47:32
 *
 * \details
 * Register: \a ANA_CL:MIP_TBL:LBM_MAC_HIGH
 *
 * @param gi Replicator: x_ANA_CL_NUM_MIP_TBL (??), 0-1023
 */
#define VTSS_ANA_CL_MIP_TBL_LBM_MAC_HIGH(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x6000,gi,4,0,1)

/** 
 * \brief
 * Destination MAC address bits 47:32 used for LBM. If LBM_MAC_HIGH = 0 and
 * LBM_MAC_LOW = 0, the MAC address check for LBM frames is disabled.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_MIP_TBL_LBM_MAC_HIGH . LBM_MAC_HIGH
 */
#define  VTSS_F_ANA_CL_MIP_TBL_LBM_MAC_HIGH_LBM_MAC_HIGH(x)  VTSS_ENCODE_BITFIELD(x,0,16)
#define  VTSS_M_ANA_CL_MIP_TBL_LBM_MAC_HIGH_LBM_MAC_HIGH     VTSS_ENCODE_BITMASK(0,16)
#define  VTSS_X_ANA_CL_MIP_TBL_LBM_MAC_HIGH_LBM_MAC_HIGH(x)  VTSS_EXTRACT_BITFIELD(x,0,16)


/** 
 * \brief MAC address configuration - bits 31:0
 *
 * \details
 * Register: \a ANA_CL:MIP_TBL:LBM_MAC_LOW
 *
 * @param gi Replicator: x_ANA_CL_NUM_MIP_TBL (??), 0-1023
 */
#define VTSS_ANA_CL_MIP_TBL_LBM_MAC_LOW(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x6000,gi,4,0,2)

/**
 * Register Group: \a ANA_CL:L2CP_TBL
 *
 * L2CP table
 */


/** 
 * \brief L2CP table entry
 *
 * \details
 * Register: \a ANA_CL:L2CP_TBL:L2CP_ENTRY_CFG
 *
 * @param gi Replicator: x_ANA_CL_NUM_L2CP_TBL_X_ADDR (??), 0-3743
 */
#define VTSS_ANA_CL_L2CP_TBL_L2CP_ENTRY_CFG(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x7000,gi,1,0,0)

/** 
 * \brief
 * COS ID value for L2CP frame.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_L2CP_TBL_L2CP_ENTRY_CFG . COSID_VAL
 */
#define  VTSS_F_ANA_CL_L2CP_TBL_L2CP_ENTRY_CFG_COSID_VAL(x)  VTSS_ENCODE_BITFIELD(x,6,3)
#define  VTSS_M_ANA_CL_L2CP_TBL_L2CP_ENTRY_CFG_COSID_VAL     VTSS_ENCODE_BITMASK(6,3)
#define  VTSS_X_ANA_CL_L2CP_TBL_L2CP_ENTRY_CFG_COSID_VAL(x)  VTSS_EXTRACT_BITFIELD(x,6,3)

/** 
 * \brief
 * Enable update of cosid.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_L2CP_TBL_L2CP_ENTRY_CFG . COSID_ENA
 */
#define  VTSS_F_ANA_CL_L2CP_TBL_L2CP_ENTRY_CFG_COSID_ENA  VTSS_BIT(5)

/** 
 * \brief
 * CPU forward configuration for L2CP frame.
 *
 * \details 
 * 0: Normal forward
 * 1: Enable redirection to CPU queue
 * 2: Enable copy to CPU queue
 * 3: Discard the frame
 *
 * Field: ::VTSS_ANA_CL_L2CP_TBL_L2CP_ENTRY_CFG . CPU_FWD_CFG
 */
#define  VTSS_F_ANA_CL_L2CP_TBL_L2CP_ENTRY_CFG_CPU_FWD_CFG(x)  VTSS_ENCODE_BITFIELD(x,3,2)
#define  VTSS_M_ANA_CL_L2CP_TBL_L2CP_ENTRY_CFG_CPU_FWD_CFG     VTSS_ENCODE_BITMASK(3,2)
#define  VTSS_X_ANA_CL_L2CP_TBL_L2CP_ENTRY_CFG_CPU_FWD_CFG(x)  VTSS_EXTRACT_BITFIELD(x,3,2)

/** 
 * \brief
 * CPU queue number for L2CP frame copied or redirected to CPU by
 * CPU_FWD_CFG.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_L2CP_TBL_L2CP_ENTRY_CFG . CPU_L2CP_QU
 */
#define  VTSS_F_ANA_CL_L2CP_TBL_L2CP_ENTRY_CFG_CPU_L2CP_QU(x)  VTSS_ENCODE_BITFIELD(x,0,3)
#define  VTSS_M_ANA_CL_L2CP_TBL_L2CP_ENTRY_CFG_CPU_L2CP_QU     VTSS_ENCODE_BITMASK(0,3)
#define  VTSS_X_ANA_CL_L2CP_TBL_L2CP_ENTRY_CFG_CPU_L2CP_QU(x)  VTSS_EXTRACT_BITFIELD(x,0,3)

/**
 * Register Group: \a ANA_CL:MAP_TBL
 *
 * Mapping table
 */


/** 
 * \brief Configures which entry fields to use by the mapping
 *
 * \details
 * Register: \a ANA_CL:MAP_TBL:SET_CTRL
 *
 * @param gi Replicator: x_ANA_CL_NUM_MAP_TBL (??), 0-511
 */
#define VTSS_ANA_CL_MAP_TBL_SET_CTRL(gi)     VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x4000,gi,16,0,0)

/** 
 * \brief
 * If set, PATH_COSID_VAL and PATH_COLOR_VAL is used.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_MAP_TBL_SET_CTRL . PATH_ENA
 */
#define  VTSS_F_ANA_CL_MAP_TBL_SET_CTRL_PATH_ENA  VTSS_BIT(7)

/** 
 * \brief
 * If set, TC_VAL replaces the classified TC bits when looking up in the
 * mapping table.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_MAP_TBL_SET_CTRL . TC_ENA
 */
#define  VTSS_F_ANA_CL_MAP_TBL_SET_CTRL_TC_ENA  VTSS_BIT(6)

/** 
 * \brief
 * If set, DP_VAL replaces the classified DP level when looking up in the
 * mapping table.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_MAP_TBL_SET_CTRL . DP_ENA
 */
#define  VTSS_F_ANA_CL_MAP_TBL_SET_CTRL_DP_ENA  VTSS_BIT(5)

/** 
 * \brief
 * If set, COSID_VAL replaces the classified COS ID when looking up in the
 * mapping table.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_MAP_TBL_SET_CTRL . COSID_ENA
 */
#define  VTSS_F_ANA_CL_MAP_TBL_SET_CTRL_COSID_ENA  VTSS_BIT(4)

/** 
 * \brief
 * If set, QOS_VAL replaces the classified QoS class when looking up in the
 * mapping table.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_MAP_TBL_SET_CTRL . QOS_ENA
 */
#define  VTSS_F_ANA_CL_MAP_TBL_SET_CTRL_QOS_ENA  VTSS_BIT(3)

/** 
 * \brief
 * If set, DEI_VAL replaces the classified DEI value when looking up in the
 * mapping table.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_MAP_TBL_SET_CTRL . DEI_ENA
 */
#define  VTSS_F_ANA_CL_MAP_TBL_SET_CTRL_DEI_ENA  VTSS_BIT(2)

/** 
 * \brief
 * If set, PCP_VAL replaces the classified PCP value when looking up in the
 * mapping table.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_MAP_TBL_SET_CTRL . PCP_ENA
 */
#define  VTSS_F_ANA_CL_MAP_TBL_SET_CTRL_PCP_ENA  VTSS_BIT(1)

/** 
 * \brief
 * If set, DSCP_VAL replaces the classified DSCP value when looking up in
 * the mapping table.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_MAP_TBL_SET_CTRL . DSCP_ENA
 */
#define  VTSS_F_ANA_CL_MAP_TBL_SET_CTRL_DSCP_ENA  VTSS_BIT(0)


/** 
 * \brief Entry in mapping table
 *
 * \details
 * Register: \a ANA_CL:MAP_TBL:MAP_ENTRY
 *
 * @param gi Replicator: x_ANA_CL_NUM_MAP_TBL (??), 0-511
 * @param ri Register: MAP_ENTRY (??), 0-7
 */
#define VTSS_ANA_CL_MAP_TBL_MAP_ENTRY(gi,ri)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x4000,gi,16,ri,1)

/** 
 * \brief
 * Disable forwarding for frames hitting this entry.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_MAP_TBL_MAP_ENTRY . FWD_DIS
 */
#define  VTSS_F_ANA_CL_MAP_TBL_MAP_ENTRY_FWD_DIS  VTSS_BIT(25)

/** 
 * \details 
 * Field: ::VTSS_ANA_CL_MAP_TBL_MAP_ENTRY . PATH_COLOR_VAL
 */
#define  VTSS_F_ANA_CL_MAP_TBL_MAP_ENTRY_PATH_COLOR_VAL  VTSS_BIT(24)

/** 
 * \details 
 * Field: ::VTSS_ANA_CL_MAP_TBL_MAP_ENTRY . PATH_COSID_VAL
 */
#define  VTSS_F_ANA_CL_MAP_TBL_MAP_ENTRY_PATH_COSID_VAL(x)  VTSS_ENCODE_BITFIELD(x,21,3)
#define  VTSS_M_ANA_CL_MAP_TBL_MAP_ENTRY_PATH_COSID_VAL     VTSS_ENCODE_BITMASK(21,3)
#define  VTSS_X_ANA_CL_MAP_TBL_MAP_ENTRY_PATH_COSID_VAL(x)  VTSS_EXTRACT_BITFIELD(x,21,3)

/** 
 * \brief
 * TC bits. The classified TC bits are set to TC_VAL if SET_CTRL.TC_ENA is
 * set.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_MAP_TBL_MAP_ENTRY . TC_VAL
 */
#define  VTSS_F_ANA_CL_MAP_TBL_MAP_ENTRY_TC_VAL(x)  VTSS_ENCODE_BITFIELD(x,18,3)
#define  VTSS_M_ANA_CL_MAP_TBL_MAP_ENTRY_TC_VAL     VTSS_ENCODE_BITMASK(18,3)
#define  VTSS_X_ANA_CL_MAP_TBL_MAP_ENTRY_TC_VAL(x)  VTSS_EXTRACT_BITFIELD(x,18,3)

/** 
 * \brief
 * Drop precedence level. The classified DP level is set to DP_VAL if
 * SET_CTRL.DP_ENA is set.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_MAP_TBL_MAP_ENTRY . DP_VAL
 */
#define  VTSS_F_ANA_CL_MAP_TBL_MAP_ENTRY_DP_VAL(x)  VTSS_ENCODE_BITFIELD(x,16,2)
#define  VTSS_M_ANA_CL_MAP_TBL_MAP_ENTRY_DP_VAL     VTSS_ENCODE_BITMASK(16,2)
#define  VTSS_X_ANA_CL_MAP_TBL_MAP_ENTRY_DP_VAL(x)  VTSS_EXTRACT_BITFIELD(x,16,2)

/** 
 * \brief
 * COS ID value. The classified COS ID is set to COSID_VAL if
 * SET_CTRL.COSID_ENA is set.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_MAP_TBL_MAP_ENTRY . COSID_VAL
 */
#define  VTSS_F_ANA_CL_MAP_TBL_MAP_ENTRY_COSID_VAL(x)  VTSS_ENCODE_BITFIELD(x,13,3)
#define  VTSS_M_ANA_CL_MAP_TBL_MAP_ENTRY_COSID_VAL     VTSS_ENCODE_BITMASK(13,3)
#define  VTSS_X_ANA_CL_MAP_TBL_MAP_ENTRY_COSID_VAL(x)  VTSS_EXTRACT_BITFIELD(x,13,3)

/** 
 * \brief
 * QoS class. The classified QoS class is set to QOS_VAL if
 * SET_CTRL.QOS_ENA is set.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_MAP_TBL_MAP_ENTRY . QOS_VAL
 */
#define  VTSS_F_ANA_CL_MAP_TBL_MAP_ENTRY_QOS_VAL(x)  VTSS_ENCODE_BITFIELD(x,10,3)
#define  VTSS_M_ANA_CL_MAP_TBL_MAP_ENTRY_QOS_VAL     VTSS_ENCODE_BITMASK(10,3)
#define  VTSS_X_ANA_CL_MAP_TBL_MAP_ENTRY_QOS_VAL(x)  VTSS_EXTRACT_BITFIELD(x,10,3)

/** 
 * \brief
 * DEI value. The classified DEI is set to DEI_VAL if SET_CTRL.DEI_ENA is
 * set.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_MAP_TBL_MAP_ENTRY . DEI_VAL
 */
#define  VTSS_F_ANA_CL_MAP_TBL_MAP_ENTRY_DEI_VAL  VTSS_BIT(9)

/** 
 * \brief
 * PCP value. The classified PCP is set to PCP_VAL if SET_CTRL.PCP_ENA is
 * set.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_MAP_TBL_MAP_ENTRY . PCP_VAL
 */
#define  VTSS_F_ANA_CL_MAP_TBL_MAP_ENTRY_PCP_VAL(x)  VTSS_ENCODE_BITFIELD(x,6,3)
#define  VTSS_M_ANA_CL_MAP_TBL_MAP_ENTRY_PCP_VAL     VTSS_ENCODE_BITMASK(6,3)
#define  VTSS_X_ANA_CL_MAP_TBL_MAP_ENTRY_PCP_VAL(x)  VTSS_EXTRACT_BITFIELD(x,6,3)

/** 
 * \brief
 * DSCP value. The classified DSCP is set to DSCP_VAL if SET_CTRL.DSCP_ENA
 * is set.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_MAP_TBL_MAP_ENTRY . DSCP_VAL
 */
#define  VTSS_F_ANA_CL_MAP_TBL_MAP_ENTRY_DSCP_VAL(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_ANA_CL_MAP_TBL_MAP_ENTRY_DSCP_VAL     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_ANA_CL_MAP_TBL_MAP_ENTRY_DSCP_VAL(x)  VTSS_EXTRACT_BITFIELD(x,0,6)

/**
 * Register Group: \a ANA_CL:IPT
 *
 * Protection and configuration table per ISDX
 */


/** 
 * \brief ISDX configuration
 *
 * \details
 * Register: \a ANA_CL:IPT:ISDX_CFG
 *
 * @param gi Replicator: x_ANA_NUM_ISDX (??), 0-4095
 */
#define VTSS_ANA_CL_IPT_ISDX_CFG(gi)         VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x0,gi,4,0,0)

/** 
 * \brief
 * MIP table index. The index is used when enabled by CLM action MIP_SEL.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_IPT_ISDX_CFG . MIP_IDX
 */
#define  VTSS_F_ANA_CL_IPT_ISDX_CFG_MIP_IDX(x)  VTSS_ENCODE_BITFIELD(x,6,10)
#define  VTSS_M_ANA_CL_IPT_ISDX_CFG_MIP_IDX     VTSS_ENCODE_BITMASK(6,10)
#define  VTSS_X_ANA_CL_IPT_ISDX_CFG_MIP_IDX(x)  VTSS_EXTRACT_BITFIELD(x,6,10)

/** 
 * \details 
 * 0: Disable use of L2CP_IDX. Default port-based index used instead.
 * >0: L2CP_IDX selects the L2CP profile to use.
 *
 * Field: ::VTSS_ANA_CL_IPT_ISDX_CFG . L2CP_IDX
 */
#define  VTSS_F_ANA_CL_IPT_ISDX_CFG_L2CP_IDX(x)  VTSS_ENCODE_BITFIELD(x,0,6)
#define  VTSS_M_ANA_CL_IPT_ISDX_CFG_L2CP_IDX     VTSS_ENCODE_BITMASK(0,6)
#define  VTSS_X_ANA_CL_IPT_ISDX_CFG_L2CP_IDX(x)  VTSS_EXTRACT_BITFIELD(x,0,6)


/** 
 * \brief Configuration of Virtual Switching Instance for service
 *
 * \details
 * Register: \a ANA_CL:IPT:VSI_CFG
 *
 * @param gi Replicator: x_ANA_NUM_ISDX (??), 0-4095
 */
#define VTSS_ANA_CL_IPT_VSI_CFG(gi)          VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x0,gi,4,0,1)

/** 
 * \brief
 * Virtual Switching Instance used if VSI_ENA is set. 
 *
 * \details 
 * Field: ::VTSS_ANA_CL_IPT_VSI_CFG . VSI_VAL
 */
#define  VTSS_F_ANA_CL_IPT_VSI_CFG_VSI_VAL(x)  VTSS_ENCODE_BITFIELD(x,1,10)
#define  VTSS_M_ANA_CL_IPT_VSI_CFG_VSI_VAL     VTSS_ENCODE_BITMASK(1,10)
#define  VTSS_X_ANA_CL_IPT_VSI_CFG_VSI_VAL(x)  VTSS_EXTRACT_BITFIELD(x,1,10)

/** 
 * \brief
 * Configures if learning and forwarding is based on VLAN or Virtual
 * Switching Instance
 *
 * \details 
 * 0: Use classified VID for lookup in VLAN table
 * 1: Use VSI for lookup in VLAN table
 *
 * Field: ::VTSS_ANA_CL_IPT_VSI_CFG . VSI_ENA
 */
#define  VTSS_F_ANA_CL_IPT_VSI_CFG_VSI_ENA    VTSS_BIT(0)


/** 
 * \brief Configuration of OAM MEP
 *
 * \details
 * Register: \a ANA_CL:IPT:OAM_MEP_CFG
 *
 * @param gi Replicator: x_ANA_NUM_ISDX (??), 0-4095
 */
#define VTSS_ANA_CL_IPT_OAM_MEP_CFG(gi)      VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x0,gi,4,0,2)

/** 
 * \brief
 * Configures the associated OAM MEP.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_IPT_OAM_MEP_CFG . MEP_IDX
 */
#define  VTSS_F_ANA_CL_IPT_OAM_MEP_CFG_MEP_IDX(x)  VTSS_ENCODE_BITFIELD(x,2,10)
#define  VTSS_M_ANA_CL_IPT_OAM_MEP_CFG_MEP_IDX     VTSS_ENCODE_BITMASK(2,10)
#define  VTSS_X_ANA_CL_IPT_OAM_MEP_CFG_MEP_IDX(x)  VTSS_EXTRACT_BITFIELD(x,2,10)

/** 
 * \brief
 * Use the associated OAM MEP.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_IPT_OAM_MEP_CFG . MEP_IDX_ENA
 */
#define  VTSS_F_ANA_CL_IPT_OAM_MEP_CFG_MEP_IDX_ENA  VTSS_BIT(1)

/** 
 * \brief
 * Force VOE to handle OAM as data.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_IPT_OAM_MEP_CFG . INDEPENDENT_MEL_ENA
 */
#define  VTSS_F_ANA_CL_IPT_OAM_MEP_CFG_INDEPENDENT_MEL_ENA  VTSS_BIT(0)


/** 
 * \brief ISDX protection table
 *
 * \details
 * Register: \a ANA_CL:IPT:IPT
 *
 * @param gi Replicator: x_ANA_NUM_ISDX (??), 0-4095
 */
#define VTSS_ANA_CL_IPT_IPT(gi)              VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x0,gi,4,0,3)

/** 
 * \brief
 * Protection configuration for ISDX.
 *
 * \details 
 * 0: No protection
 * 1: set prot_active to be used by Rewriter if the ISDX's path group uses
 * the protected path.
 * 2: Working path: Discard the frame if the ISDX's path group uses the
 * protected path.
 * 3: Protected path: Discard the frame if the ISDX's path group uses the
 * working path.

 *
 * Field: ::VTSS_ANA_CL_IPT_IPT . IPT_CFG
 */
#define  VTSS_F_ANA_CL_IPT_IPT_IPT_CFG(x)     VTSS_ENCODE_BITFIELD(x,11,2)
#define  VTSS_M_ANA_CL_IPT_IPT_IPT_CFG        VTSS_ENCODE_BITMASK(11,2)
#define  VTSS_X_ANA_CL_IPT_IPT_IPT_CFG(x)     VTSS_EXTRACT_BITFIELD(x,11,2)

/** 
 * \brief
 * Configures the protection pipeline point i.e. the pipeline where IPT_CFG
 * operates.
 *
 * \details 
 * 0: ANA_IPT_PROT
 * 1: ANA_OU_PROT
 * 2: ANA_MID_PROT
 * 3: ANA_IN_PROT
 *
 * Field: ::VTSS_ANA_CL_IPT_IPT . PROT_PIPELINE_PT
 */
#define  VTSS_F_ANA_CL_IPT_IPT_PROT_PIPELINE_PT(x)  VTSS_ENCODE_BITFIELD(x,9,2)
#define  VTSS_M_ANA_CL_IPT_IPT_PROT_PIPELINE_PT     VTSS_ENCODE_BITMASK(9,2)
#define  VTSS_X_ANA_CL_IPT_IPT_PROT_PIPELINE_PT(x)  VTSS_EXTRACT_BITFIELD(x,9,2)

/** 
 * \brief
 * Path protection group. This index points to the path protection entry in
 * the path protection table used for this ISDX. Only indexes 0 through 75
 * are valid.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_IPT_IPT . PPT_IDX
 */
#define  VTSS_F_ANA_CL_IPT_IPT_PPT_IDX(x)     VTSS_ENCODE_BITFIELD(x,0,9)
#define  VTSS_M_ANA_CL_IPT_IPT_PPT_IDX        VTSS_ENCODE_BITMASK(0,9)
#define  VTSS_X_ANA_CL_IPT_IPT_PPT_IDX(x)     VTSS_EXTRACT_BITFIELD(x,0,9)

/**
 * Register Group: \a ANA_CL:PPT
 *
 * Path group protection table per path
 */


/** 
 * \brief Path protection state configuration
 *
 * \details
 * Configures Path protection group state.
 *
 * Register: \a ANA_CL:PPT:PP_CFG
 *
 * @param ri Replicator: x_ANA_NUM_PATHGRP_DIV32_CEIL (??), 0-15
 */
#define VTSS_ANA_CL_PPT_PP_CFG(ri)           VTSS_IOREG(VTSS_TO_ANA_CL,0x7f52 + (ri))

/**
 * Register Group: \a ANA_CL:STICKY
 *
 * Sticky diagnostic status
 */


/** 
 * \brief Sticky bits register
 *
 * \details
 * Register: \a ANA_CL:STICKY:FILTER_STICKY
 */
#define VTSS_ANA_CL_STICKY_FILTER_STICKY     VTSS_IOREG(VTSS_TO_ANA_CL,0x7f62)

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
 * Field: ::VTSS_ANA_CL_STICKY_FILTER_STICKY . STACKING_FILTER_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_FILTER_STICKY_STACKING_FILTER_STICKY  VTSS_BIT(1)

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
 * Field: ::VTSS_ANA_CL_STICKY_FILTER_STICKY . BAD_MACS_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_FILTER_STICKY_BAD_MACS_STICKY  VTSS_BIT(0)


/** 
 * \brief Sticky bits register
 *
 * \details
 * Register: \a ANA_CL:STICKY:VLAN_FILTER_STICKY
 *
 * @param ri Register: VLAN_FILTER_STICKY (??), 0-2
 */
#define VTSS_ANA_CL_STICKY_VLAN_FILTER_STICKY(ri)  VTSS_IOREG(VTSS_TO_ANA_CL,0x7f63 + (ri))

/** 
 * \brief
 * Set if a frame is dropped due required tag was not found.
 *
 * \details 
 * 0: No Event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_STICKY_VLAN_FILTER_STICKY . FILTER_REQUIRED_TAG_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_VLAN_FILTER_STICKY_FILTER_REQUIRED_TAG_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Set if a frame is dropped due to priority C-TAG filtering at the given
 * tag position.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_STICKY_VLAN_FILTER_STICKY . FILTER_PRIO_CTAG_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_VLAN_FILTER_STICKY_FILTER_PRIO_CTAG_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * Set if a frame is dropped due to C-TAG filtering at the given tag
 * position.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_STICKY_VLAN_FILTER_STICKY . FILTER_CTAG_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_VLAN_FILTER_STICKY_FILTER_CTAG_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Set if a frame is dropped due priority S-TAG filtering at the given tag
 * position.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_STICKY_VLAN_FILTER_STICKY . FILTER_PRIO_STAG_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_VLAN_FILTER_STICKY_FILTER_PRIO_STAG_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Set if a frame is dropped due to S-TAG filtering at the given tag
 * position.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_STICKY_VLAN_FILTER_STICKY . FILTER_STAG_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_VLAN_FILTER_STICKY_FILTER_STAG_STICKY  VTSS_BIT(0)


/** 
 * \brief Sticky bits register
 *
 * \details
 * Register: \a ANA_CL:STICKY:CLASS_STICKY
 */
#define VTSS_ANA_CL_STICKY_CLASS_STICKY      VTSS_IOREG(VTSS_TO_ANA_CL,0x7f66)

/** 
 * \brief
 * This sticky bit indicates that the QoS class was used as index into the
 * DSCP_REWR_VAL table to determine the DSCP value.
 *
 * \details 
 * 0: No event
 * 1: QoS class has been used to determine the DSCP
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_STICKY_CLASS_STICKY . DSCP_QOS_REWR_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_CLASS_STICKY_DSCP_QOS_REWR_STICKY  VTSS_BIT(9)

/** 
 * \brief
 * Set when the  DSCP value is based on the DSCP_TRANSLATE_VAL.
 *
 * \details 
 * 0: No event
 * 1: DSCP_TRANSLATE_VAL has been used.
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_STICKY_CLASS_STICKY . DSCP_TRANSLATE_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_CLASS_STICKY_DSCP_TRANSLATE_STICKY  VTSS_BIT(8)

/** 
 * \brief
 * Set when the port VLAN has been used.
 *
 * \details 
 * 0: The event has not occured
 * 1: Port default value has been used for VLAN classification
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_STICKY_CLASS_STICKY . VID_PORT_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_CLASS_STICKY_VID_PORT_STICKY  VTSS_BIT(7)

/** 
 * \brief
 * Set when the VID information from Stacking TAG has been used.
 *
 * \details 
 * 0: The event has not occured
 * 1: Stacking Header VID has been used for VLAN classification
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_STICKY_CLASS_STICKY . VID_STACK_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_CLASS_STICKY_VID_STACK_STICKY  VTSS_BIT(6)

/** 
 * \brief
 * Set when vid from the TAG has been used.
 *
 * \details 
 * 0: The event has not occured
 * 1: TCI VID value has been used for VLAN classification
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_STICKY_CLASS_STICKY . VID_TAG_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_CLASS_STICKY_VID_TAG_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * Set when the classified PCP and DEI value is used to to determeine the
 * QoS class.
 *
 * \details 
 * 0: No event
 * 1: PCP value has been used for QoS classification
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_STICKY_CLASS_STICKY . QOS_PCP_DEI_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_CLASS_STICKY_QOS_PCP_DEI_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Set when the default port QoS has been used.
 *
 * \details 
 * 0: No event
 * 1: Port default has been used for QoS classification
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_STICKY_CLASS_STICKY . QOS_DEFAULT_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_CLASS_STICKY_QOS_DEFAULT_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * Set when the DSCP QoS has been used.
 *
 * \details 
 * 0: No event
 * 1: DSCP QoS class has been used for QoS classification
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_STICKY_CLASS_STICKY . QOS_DSCP_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_CLASS_STICKY_QOS_DSCP_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Set when the stacking QoS has been used.
 *
 * \details 
 * 0: No event
 * 1: Stacking header QoS class has been used for QoS classification
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_STICKY_CLASS_STICKY . QOS_STACK_TAG_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_CLASS_STICKY_QOS_STACK_TAG_STICKY  VTSS_BIT(0)


/** 
 * \brief Sticky bits register
 *
 * \details
 * Register: \a ANA_CL:STICKY:CAT_STICKY
 */
#define VTSS_ANA_CL_STICKY_CAT_STICKY        VTSS_IOREG(VTSS_TO_ANA_CL,0x7f67)

/** 
 * \brief
 * Set if a frame has been received with a TPID disabled for CPU
 * forwarding.
 *
 * \details 
 * 0: The event has not occured
 * 1: The event did occour
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_STICKY_CAT_STICKY . CAPTURE_TPID_DIS_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_CAT_STICKY_CAPTURE_TPID_DIS_STICKY  VTSS_BIT(20)

/** 
 * \brief
 * Indicates that a frame with a VStax2 TTL value of 0 was dropped.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_STICKY_CAT_STICKY . VSTAX2_TTL_ZERO_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_CAT_STICKY_VSTAX2_TTL_ZERO_STICKY  VTSS_BIT(11)

/** 
 * \brief
 * Set if a VRAP frame has been detected.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_STICKY_CAT_STICKY . VRAP_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_CAT_STICKY_VRAP_STICKY  VTSS_BIT(10)

/** 
 * \brief
 * Set if an IGMP frame has been detected.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_STICKY_CAT_STICKY . IGMP_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_CAT_STICKY_IGMP_STICKY  VTSS_BIT(9)

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
 * Field: ::VTSS_ANA_CL_STICKY_CAT_STICKY . AG_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_CAT_STICKY_AG_STICKY  VTSS_BIT(8)

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
 * Field: ::VTSS_ANA_CL_STICKY_CAT_STICKY . BPDU_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_CAT_STICKY_BPDU_STICKY  VTSS_BIT(7)

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
 * Field: ::VTSS_ANA_CL_STICKY_CAT_STICKY . GXRP_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_CAT_STICKY_GXRP_STICKY  VTSS_BIT(6)

/** 
 * \brief
 * Set if an IPv6 Multicast control frame has been detected.
 *   - DMAC = 0x3333xxxxxxxx , Ethernet Type = IPv6, DIP = FF02::/16
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_STICKY_CAT_STICKY . IP6_MC_CTRL_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_CAT_STICKY_IP6_MC_CTRL_STICKY  VTSS_BIT(5)

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
 * Field: ::VTSS_ANA_CL_STICKY_CAT_STICKY . IP4_MC_CTRL_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_CAT_STICKY_IP4_MC_CTRL_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * Set if an MLD frame has been detected.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_STICKY_CAT_STICKY . MLD_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_CAT_STICKY_MLD_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * Indicates that a IPv6 frame with hop by hop options and ICMPv6 was seen.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_STICKY_CAT_STICKY . IP6_ICMP_HOP_BY_HOP_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_CAT_STICKY_IP6_ICMP_HOP_BY_HOP_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Is set if an IPv6 frame with a hop by hop header and ICMPv6 was seen.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_STICKY_CAT_STICKY . IP6_HOP_BY_HOP_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_CAT_STICKY_IP6_HOP_BY_HOP_STICKY  VTSS_BIT(1)


/** 
 * \brief Sticky bits register for events generated by advanced VCAP classification when handling MPLS TP
 *
 * \details
 * Register: \a ANA_CL:STICKY:ADV_CL_MPLS_STICKY
 */
#define VTSS_ANA_CL_STICKY_ADV_CL_MPLS_STICKY  VTSS_IOREG(VTSS_TO_ANA_CL,0x7f68)

/** 
 * \brief
 * This sticky bit signals frame for Segment OAM seen.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_MPLS_STICKY . ADV_CL_MPLS_SEGMENT_OAM_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_MPLS_STICKY_ADV_CL_MPLS_SEGMENT_OAM_STICKY  VTSS_BIT(16)

/** 
 * \brief
 * This sticky bit signals IP frame received as LSR POP (CLM FWD_TYPE=3)
 * with (nxt_type_after_offset = CW) seen.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_MPLS_STICKY . ADV_CL_MPLS_IP_TRAFFIC_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_MPLS_STICKY_ADV_CL_MPLS_IP_TRAFFIC_STICKY  VTSS_BIT(15)

/** 
 * \brief
 * This sticky bit signals frame for PATH OAM MEP seen.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_MPLS_STICKY . ADV_CL_MPLS_MEP_OAM_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_MPLS_STICKY_ADV_CL_MPLS_MEP_OAM_STICKY  VTSS_BIT(14)

/** 
 * \brief
 * This sticky bit signals MPLS POP err when fwd_mode=3.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_MPLS_STICKY . ADV_CL_MPLS_POP_ERR_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_MPLS_STICKY_ADV_CL_MPLS_POP_ERR_STICKY  VTSS_BIT(13)

/** 
 * \brief
 * This sticky bit signals MPLS SWAP err.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_MPLS_STICKY . ADV_CL_MPLS_SWAP_ERR_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_MPLS_STICKY_ADV_CL_MPLS_SWAP_ERR_STICKY  VTSS_BIT(12)

/** 
 * \brief
 * This sticky bit indicate frame redirected to CPU due to reserved label
 * seen with valid labels.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_MPLS_STICKY . ADV_CL_MPLS_RSV_XTR_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_MPLS_STICKY_ADV_CL_MPLS_RSV_XTR_STICKY  VTSS_BIT(11)

/** 
 * \brief
 * This sticky bit signals MPLS PW termination of data with Control Word.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_MPLS_STICKY . ADV_CL_MPLS_TERM_PW_CW_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_MPLS_STICKY_ADV_CL_MPLS_TERM_PW_CW_STICKY  VTSS_BIT(10)

/** 
 * \brief
 * This sticky bit signals MPLS PW termination of data.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_MPLS_STICKY . ADV_CL_MPLS_TERM_PW_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_MPLS_STICKY_ADV_CL_MPLS_TERM_PW_STICKY  VTSS_BIT(9)

/** 
 * \brief
 * This sticky bit signals MPLS PW termination of OAM VCCv1.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_MPLS_STICKY . ADV_CL_MPLS_TERM_PW_VCC1_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_MPLS_STICKY_ADV_CL_MPLS_TERM_PW_VCC1_STICKY  VTSS_BIT(8)

/** 
 * \brief
 * This sticky bit signals MPLS PW termination of OAM VCCv2.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_MPLS_STICKY . ADV_CL_MPLS_TERM_PW_VCC2_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_MPLS_STICKY_ADV_CL_MPLS_TERM_PW_VCC2_STICKY  VTSS_BIT(7)

/** 
 * \brief
 * This sticky bit signals MPLS PW termination of OAM VCCv3.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_MPLS_STICKY . ADV_CL_MPLS_TERM_PW_VCC3_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_MPLS_STICKY_ADV_CL_MPLS_TERM_PW_VCC3_STICKY  VTSS_BIT(6)

/** 
 * \brief
 * This sticky bit signals MPLS PW termination of OAM VCCv4.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_MPLS_STICKY . ADV_CL_MPLS_TERM_PW_VCC4_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_MPLS_STICKY_ADV_CL_MPLS_TERM_PW_VCC4_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * This sticky bit signals MPLS PW termination err (frame is discarded).
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_MPLS_STICKY . ADV_CL_MPLS_TERM_PW_ERR_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_MPLS_STICKY_ADV_CL_MPLS_TERM_PW_ERR_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * This sticky bit signals vld lables set to less than available.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_MPLS_STICKY . ADV_CL_MPLS_TOO_FEW_VLD_LABELS_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_MPLS_STICKY_ADV_CL_MPLS_TOO_FEW_VLD_LABELS_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * This sticky bit signals CPU redirect due to selected TTL value expired.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_MPLS_STICKY . ADV_CL_MPLS_TTL_EXPIRE_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_MPLS_STICKY_ADV_CL_MPLS_TTL_EXPIRE_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * This sticky bit signals TTL value extracted from Label stack.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_MPLS_STICKY . ADV_CL_MPLS_USE_TTL_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_MPLS_STICKY_ADV_CL_MPLS_USE_TTL_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * This sticky bit signals TC value extracted from Label stack.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_MPLS_STICKY . ADV_CL_MPLS_USE_TC_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_MPLS_STICKY_ADV_CL_MPLS_USE_TC_STICKY  VTSS_BIT(0)


/** 
 * \brief Sticky bits register for events generated by advanced VCAP classification
 *
 * \details
 * Register: \a ANA_CL:STICKY:ADV_CL_STICKY
 */
#define VTSS_ANA_CL_STICKY_ADV_CL_STICKY     VTSS_IOREG(VTSS_TO_ANA_CL,0x7f69)

/** 
 * \brief
 * This sticky bit signals that a HIH force mode use of map index was used.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_STICKY . ADV_CL_HIH_FORCE_MODE_USED_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_STICKY_ADV_CL_HIH_FORCE_MODE_USED_STICKY  VTSS_BIT(24)

/** 
 * \brief
 * This sticky bit signals that a HIH lookup key was used.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_STICKY . ADV_CL_HIH_KEY_USED_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_STICKY_ADV_CL_HIH_KEY_USED_STICKY  VTSS_BIT(23)

/** 
 * \brief
 * This sticky bit signals that a frame ways attempted to normalized beyond
 * what is supported.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_STICKY . ADV_CL_MAX_W16_OFFSET_FAIL_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_STICKY_ADV_CL_MAX_W16_OFFSET_FAIL_STICKY  VTSS_BIT(22)

/** 
 * \brief
 * This sticky bit signals was_tagged from the VCAP was used.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_STICKY . ADV_CL_VLAN_WAS_TAGGED_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_STICKY_ADV_CL_VLAN_WAS_TAGGED_STICKY  VTSS_BIT(21)

/** 
 * \brief
 * This sticky bit signals COS ID from the VCAP was used.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_STICKY . ADV_CL_COSID_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_STICKY_ADV_CL_COSID_STICKY  VTSS_BIT(20)

/** 
 * \brief
 * This sticky bit signals DSCP value from the VCAPs was used.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_STICKY . ADV_CL_DSCP_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_STICKY_ADV_CL_DSCP_STICKY  VTSS_BIT(19)

/** 
 * \brief
 * This sticky bit signals QoS class from the VCAP was used.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_STICKY . ADV_CL_QOS_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_STICKY_ADV_CL_QOS_STICKY  VTSS_BIT(18)

/** 
 * \brief
 * This sticky bit signals DP from the VCAP was used.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_STICKY . ADV_CL_DP_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_STICKY_ADV_CL_DP_STICKY  VTSS_BIT(17)

/** 
 * \brief
 * This sticky bit signals DEI from the VCAP was used.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_STICKY . ADV_CL_DEI_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_STICKY_ADV_CL_DEI_STICKY  VTSS_BIT(16)

/** 
 * \brief
 * This sticky bit signals PCP from the VCAP was used.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_STICKY . ADV_CL_PCP_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_STICKY_ADV_CL_PCP_STICKY  VTSS_BIT(15)

/** 
 * \brief
 * This sticky bit signals VID from the VCAP was used.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_STICKY . ADV_CL_VID_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_STICKY_ADV_CL_VID_STICKY  VTSS_BIT(14)

/** 
 * \brief
 * This sticky bit signals VLAN pop count from the VCAP was used.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_STICKY . ADV_CL_VLAN_POPCNT_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_STICKY_ADV_CL_VLAN_POPCNT_STICKY  VTSS_BIT(13)

/** 
 * \brief
 * This sticky bit signals MAP table usage from the VCAP was used.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_STICKY . ADV_CL_MAP_TBL_IDX0_UPDATED_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_STICKY_ADV_CL_MAP_TBL_IDX0_UPDATED_STICKY  VTSS_BIT(12)

/** 
 * \brief
 * This sticky bit signals MAP table usage from the VCAP was used.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_STICKY . ADV_CL_MAP_TBL_IDX1_UPDATED_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_STICKY_ADV_CL_MAP_TBL_IDX1_UPDATED_STICKY  VTSS_BIT(11)

/** 
 * \brief
 * This sticky bit signals ISDX from the VCAP was used.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_STICKY . ADV_CL_ISDX_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_STICKY_ADV_CL_ISDX_STICKY  VTSS_BIT(10)

/** 
 * \brief
 * This sticky bit signals MASQ_INJ was triggered from the VCAP was used.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_STICKY . ADV_CL_MASQ_INJ_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_STICKY_ADV_CL_MASQ_INJ_STICKY  VTSS_BIT(9)

/** 
 * \brief
 * This sticky bit signals Centralized injection was triggered from the
 * VCAP was used.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_STICKY . ADV_CL_CENTRALIZED_INJ_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_STICKY_ADV_CL_CENTRALIZED_INJ_STICKY  VTSS_BIT(8)

/** 
 * \brief
 * This sticky bit signals generic index was changed from the VCAP.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_STICKY . ADV_CL_GENERIC_IDX_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_STICKY_ADV_CL_GENERIC_IDX_STICKY  VTSS_BIT(7)

/** 
 * \brief
 * This sticky bit signals NXT_KEY_TYPE from the VCAP was used.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_STICKY . ADV_CL_NXT_KEY_TYPE_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_STICKY_ADV_CL_NXT_KEY_TYPE_STICKY  VTSS_BIT(6)

/** 
 * \brief
 * This sticky bit signals NXT_NORM_W16_OFFSET from the VCAP was used.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_STICKY . ADV_CL_NXT_NORM_W16_OFFSET_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_STICKY_ADV_CL_NXT_NORM_W16_OFFSET_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * This sticky bit signals NXT_TYPE_AFTER_OFFSET = ETH from the VCAP was
 * used.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_STICKY . ADV_CL_NXT_TYPE_ETH_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_STICKY_ADV_CL_NXT_TYPE_ETH_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * This sticky bit signals NXT_TYPE_AFTER_OFFSET = MPLS from the VCAP was
 * used.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_STICKY . ADV_CL_NXT_TYPE_MPLS_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_STICKY_ADV_CL_NXT_TYPE_MPLS_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * This sticky bit signals NXT_TYPE_AFTER_OFFSET = CW from the VCAP was
 * used.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_STICKY . ADV_CL_NXT_TYPE_CW_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_STICKY_ADV_CL_NXT_TYPE_CW_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * This sticky bit signals NXT_NORMALIZE from the VCAP was used.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_STICKY . ADV_CL_NXT_NORMALIZE_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_STICKY_ADV_CL_NXT_NORMALIZE_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * This sticky bit signals NXT_OFFSET_FROM_TYPE > 63 from the VCAP was
 * attempted.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_ADV_CL_STICKY . ADV_CL_NXT_OFFSET_TOO_BIG_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_ADV_CL_STICKY_ADV_CL_NXT_OFFSET_TOO_BIG_STICKY  VTSS_BIT(0)


/** 
 * \brief Sticky bits register for events generated by MIP operation
 *
 * \details
 * Register: \a ANA_CL:STICKY:MIP_STICKY
 */
#define VTSS_ANA_CL_STICKY_MIP_STICKY        VTSS_IOREG(VTSS_TO_ANA_CL,0x7f6a)

/** 
 * \brief
 * This sticky bit signals MIP operation failed due to MEL check.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_MIP_STICKY . MIP_MEL_CHK_FAIL_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_MIP_STICKY_MIP_MEL_CHK_FAIL_STICKY  VTSS_BIT(6)

/** 
 * \brief
 * This sticky bit signals MIP LBM operation failed due to faild MAC
 * address check.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_MIP_STICKY . MIP_LBM_DA_CHK_FAIL_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_MIP_STICKY_MIP_LBM_DA_CHK_FAIL_STICKY  VTSS_BIT(5)

/** 
 * \brief
 * This sticky bit signals Generic MIP operation.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_MIP_STICKY . MIP_GENERIC_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_MIP_STICKY_MIP_GENERIC_STICKY  VTSS_BIT(4)

/** 
 * \brief
 * This sticky bit signals  RAPS MIP operation.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_MIP_STICKY . MIP_RAPS_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_MIP_STICKY_MIP_RAPS_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * This sticky bit signals LTM redir MIP operation.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_MIP_STICKY . MIP_LTM_REDIR_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_MIP_STICKY_MIP_LTM_REDIR_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * This sticky bit signals LBM redir MIP operation.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_MIP_STICKY . MIP_LBM_REDIR_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_MIP_STICKY_MIP_LBM_REDIR_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * This sticky bit signals CCM copy MIP operation.
 *
 * \details 
 * Field: ::VTSS_ANA_CL_STICKY_MIP_STICKY . MIP_CCM_COPY_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_MIP_STICKY_MIP_CCM_COPY_STICKY  VTSS_BIT(0)


/** 
 * \brief Sticky bits register
 *
 * \details
 * Register: \a ANA_CL:STICKY:IP_HDR_CHK_STICKY
 */
#define VTSS_ANA_CL_STICKY_IP_HDR_CHK_STICKY  VTSS_IOREG(VTSS_TO_ANA_CL,0x7f6b)

/** 
 * \brief
 * Set if an IP packet with options is found.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_STICKY_IP_HDR_CHK_STICKY . IP_OPTIONS_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_IP_HDR_CHK_STICKY_IP_OPTIONS_STICKY  VTSS_BIT(3)

/** 
 * \brief
 * Set if an IP fragmented frame is found.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_STICKY_IP_HDR_CHK_STICKY . IP4_FRAGMENT_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_IP_HDR_CHK_STICKY_IP4_FRAGMENT_STICKY  VTSS_BIT(2)

/** 
 * \brief
 * Set if IP total length is less that IP header length.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_STICKY_IP_HDR_CHK_STICKY . IP4_LEN_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_IP_HDR_CHK_STICKY_IP4_LEN_STICKY  VTSS_BIT(1)

/** 
 * \brief
 * Set if an IP checksum error is found.
 *
 * \details 
 * 0: No event
 * 1: Event
 * Bit is cleared by writing a 1 to this position.
 *
 * Field: ::VTSS_ANA_CL_STICKY_IP_HDR_CHK_STICKY . IP4_CHKSUM_STICKY
 */
#define  VTSS_F_ANA_CL_STICKY_IP_HDR_CHK_STICKY_IP4_CHKSUM_STICKY  VTSS_BIT(0)

/**
 * Register Group: \a ANA_CL:STICKY_MASK
 *
 * Sticky diagnostic global port counter event configuration
 */


/** 
 * \brief Sticky counter mask register
 *
 * \details
 * Register: \a ANA_CL:STICKY_MASK:FILTER_STICKY_MASK
 *
 * @param gi Replicator: x_ANA_NUM_CONCURRENT_CNT (??), 0-3
 */
#define VTSS_ANA_CL_STICKY_MASK_FILTER_STICKY_MASK(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x7f6c,gi,10,0,0)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_FILTER_STICKY_MASK . STACKING_FILTER_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_FILTER_STICKY_MASK_STACKING_FILTER_STICKY_MASK  VTSS_BIT(1)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_FILTER_STICKY_MASK . BAD_MACS_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_FILTER_STICKY_MASK_BAD_MACS_STICKY_MASK  VTSS_BIT(0)


/** 
 * \brief Sticky counter mask register
 *
 * \details
 * Register: \a ANA_CL:STICKY_MASK:VLAN_FILTER_STICKY_MASK
 *
 * @param gi Replicator: x_ANA_NUM_CONCURRENT_CNT (??), 0-3
 * @param ri Register: VLAN_FILTER_STICKY_MASK (??), 0-2
 */
#define VTSS_ANA_CL_STICKY_MASK_VLAN_FILTER_STICKY_MASK(gi,ri)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x7f6c,gi,10,ri,1)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_VLAN_FILTER_STICKY_MASK . FILTER_REQUIRED_TAG_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_VLAN_FILTER_STICKY_MASK_FILTER_REQUIRED_TAG_STICKY_MASK  VTSS_BIT(4)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_VLAN_FILTER_STICKY_MASK . FILTER_PRIO_CTAG_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_VLAN_FILTER_STICKY_MASK_FILTER_PRIO_CTAG_STICKY_MASK  VTSS_BIT(3)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_VLAN_FILTER_STICKY_MASK . FILTER_CTAG_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_VLAN_FILTER_STICKY_MASK_FILTER_CTAG_STICKY_MASK  VTSS_BIT(2)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enabale event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_VLAN_FILTER_STICKY_MASK . FILTER_PRIO_STAG_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_VLAN_FILTER_STICKY_MASK_FILTER_PRIO_STAG_STICKY_MASK  VTSS_BIT(1)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enabale event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_VLAN_FILTER_STICKY_MASK . FILTER_STAG_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_VLAN_FILTER_STICKY_MASK_FILTER_STAG_STICKY_MASK  VTSS_BIT(0)


/** 
 * \brief Sticky counter mask register
 *
 * \details
 * Register: \a ANA_CL:STICKY_MASK:CLASS_STICKY_MASK
 *
 * @param gi Replicator: x_ANA_NUM_CONCURRENT_CNT (??), 0-3
 */
#define VTSS_ANA_CL_STICKY_MASK_CLASS_STICKY_MASK(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x7f6c,gi,10,0,4)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_CLASS_STICKY_MASK . DSCP_QOS_REWR_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_CLASS_STICKY_MASK_DSCP_QOS_REWR_STICKY_MASK  VTSS_BIT(9)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_CLASS_STICKY_MASK . DSCP_TRANSLATE_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_CLASS_STICKY_MASK_DSCP_TRANSLATE_STICKY_MASK  VTSS_BIT(8)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_CLASS_STICKY_MASK . VID_PORT_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_CLASS_STICKY_MASK_VID_PORT_STICKY_MASK  VTSS_BIT(7)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_CLASS_STICKY_MASK . VID_STACK_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_CLASS_STICKY_MASK_VID_STACK_STICKY_MASK  VTSS_BIT(6)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_CLASS_STICKY_MASK . VID_TAG_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_CLASS_STICKY_MASK_VID_TAG_STICKY_MASK  VTSS_BIT(5)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_CLASS_STICKY_MASK . QOS_PCP_DEI_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_CLASS_STICKY_MASK_QOS_PCP_DEI_STICKY_MASK  VTSS_BIT(4)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_CLASS_STICKY_MASK . QOS_DEFAULT_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_CLASS_STICKY_MASK_QOS_DEFAULT_STICKY_MASK  VTSS_BIT(3)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_CLASS_STICKY_MASK . QOS_DSCP_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_CLASS_STICKY_MASK_QOS_DSCP_STICKY_MASK  VTSS_BIT(1)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_CLASS_STICKY_MASK . QOS_STACK_TAG_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_CLASS_STICKY_MASK_QOS_STACK_TAG_STICKY_MASK  VTSS_BIT(0)


/** 
 * \brief Sticky counter mask register
 *
 * \details
 * Register: \a ANA_CL:STICKY_MASK:CAT_STICKY_MASK
 *
 * @param gi Replicator: x_ANA_NUM_CONCURRENT_CNT (??), 0-3
 */
#define VTSS_ANA_CL_STICKY_MASK_CAT_STICKY_MASK(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x7f6c,gi,10,0,5)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_CAT_STICKY_MASK . CAPTURE_TPID_DIS_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_CAT_STICKY_MASK_CAPTURE_TPID_DIS_STICKY_MASK  VTSS_BIT(20)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_CAT_STICKY_MASK . VSTAX2_TTL_ZERO_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_CAT_STICKY_MASK_VSTAX2_TTL_ZERO_STICKY_MASK  VTSS_BIT(11)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_CAT_STICKY_MASK . VRAP_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_CAT_STICKY_MASK_VRAP_STICKY_MASK  VTSS_BIT(10)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_CAT_STICKY_MASK . IGMP_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_CAT_STICKY_MASK_IGMP_STICKY_MASK  VTSS_BIT(9)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_CAT_STICKY_MASK . AG_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_CAT_STICKY_MASK_AG_STICKY_MASK  VTSS_BIT(8)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_CAT_STICKY_MASK . BPDU_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_CAT_STICKY_MASK_BPDU_STICKY_MASK  VTSS_BIT(7)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_CAT_STICKY_MASK . GXRP_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_CAT_STICKY_MASK_GXRP_STICKY_MASK  VTSS_BIT(6)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_CAT_STICKY_MASK . IP6_MC_CTRL_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_CAT_STICKY_MASK_IP6_MC_CTRL_STICKY_MASK  VTSS_BIT(5)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_CAT_STICKY_MASK . IP4_MC_CTRL_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_CAT_STICKY_MASK_IP4_MC_CTRL_STICKY_MASK  VTSS_BIT(4)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_CAT_STICKY_MASK . MLD_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_CAT_STICKY_MASK_MLD_STICKY_MASK  VTSS_BIT(3)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_CAT_STICKY_MASK . IP6_ICMP_HOP_BY_HOP_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_CAT_STICKY_MASK_IP6_ICMP_HOP_BY_HOP_STICKY_MASK  VTSS_BIT(2)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_CAT_STICKY_MASK . IP6_HOP_BY_HOP_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_CAT_STICKY_MASK_IP6_HOP_BY_HOP_STICKY_MASK  VTSS_BIT(1)


/** 
 * \brief Sticky counter mask register
 *
 * \details
 * Register: \a ANA_CL:STICKY_MASK:ADV_CL_MPLS_STICKY_MASK
 *
 * @param gi Replicator: x_ANA_NUM_CONCURRENT_CNT (??), 0-3
 */
#define VTSS_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x7f6c,gi,10,0,6)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK . ADV_CL_MPLS_SEGMENT_OAM_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK_ADV_CL_MPLS_SEGMENT_OAM_STICKY_MASK  VTSS_BIT(16)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK . ADV_CL_MPLS_IP_TRAFFIC_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK_ADV_CL_MPLS_IP_TRAFFIC_STICKY_MASK  VTSS_BIT(15)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK . ADV_CL_MPLS_MEP_OAM_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK_ADV_CL_MPLS_MEP_OAM_STICKY_MASK  VTSS_BIT(14)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK . ADV_CL_MPLS_POP_ERR_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK_ADV_CL_MPLS_POP_ERR_STICKY_MASK  VTSS_BIT(13)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK . ADV_CL_MPLS_SWAP_ERR_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK_ADV_CL_MPLS_SWAP_ERR_STICKY_MASK  VTSS_BIT(12)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK . ADV_CL_MPLS_RSV_XTR_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK_ADV_CL_MPLS_RSV_XTR_STICKY_MASK  VTSS_BIT(11)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK . ADV_CL_MPLS_TERM_PW_CW_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK_ADV_CL_MPLS_TERM_PW_CW_STICKY_MASK  VTSS_BIT(10)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK . ADV_CL_MPLS_TERM_PW_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK_ADV_CL_MPLS_TERM_PW_STICKY_MASK  VTSS_BIT(9)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK . ADV_CL_MPLS_TERM_PW_VCC1_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK_ADV_CL_MPLS_TERM_PW_VCC1_STICKY_MASK  VTSS_BIT(8)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK . ADV_CL_MPLS_TERM_PW_VCC2_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK_ADV_CL_MPLS_TERM_PW_VCC2_STICKY_MASK  VTSS_BIT(7)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK . ADV_CL_MPLS_TERM_PW_VCC3_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK_ADV_CL_MPLS_TERM_PW_VCC3_STICKY_MASK  VTSS_BIT(6)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK . ADV_CL_MPLS_TERM_PW_VCC4_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK_ADV_CL_MPLS_TERM_PW_VCC4_STICKY_MASK  VTSS_BIT(5)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK . ADV_CL_MPLS_TERM_PW_ERR_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK_ADV_CL_MPLS_TERM_PW_ERR_STICKY_MASK  VTSS_BIT(4)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK . ADV_CL_MPLS_TOO_FEW_VLD_LABELS_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK_ADV_CL_MPLS_TOO_FEW_VLD_LABELS_STICKY_MASK  VTSS_BIT(3)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK . ADV_CL_MPLS_TTL_EXPIRE_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK_ADV_CL_MPLS_TTL_EXPIRE_STICKY_MASK  VTSS_BIT(2)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK . ADV_CL_MPLS_USE_TTL_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK_ADV_CL_MPLS_USE_TTL_STICKY_MASK  VTSS_BIT(1)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK . ADV_CL_MPLS_USE_TC_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_MPLS_STICKY_MASK_ADV_CL_MPLS_USE_TC_STICKY_MASK  VTSS_BIT(0)


/** 
 * \brief Sticky counter mask register
 *
 * \details
 * Register: \a ANA_CL:STICKY_MASK:ADV_CL_STICKY_MASK
 *
 * @param gi Replicator: x_ANA_NUM_CONCURRENT_CNT (??), 0-3
 */
#define VTSS_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x7f6c,gi,10,0,7)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK . ADV_CL_HIH_FORCE_MODE_USED_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK_ADV_CL_HIH_FORCE_MODE_USED_STICKY_MASK  VTSS_BIT(24)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK . ADV_CL_HIH_KEY_USED_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK_ADV_CL_HIH_KEY_USED_STICKY_MASK  VTSS_BIT(23)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK . ADV_CL_MAX_W16_OFFSET_FAIL_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK_ADV_CL_MAX_W16_OFFSET_FAIL_STICKY_MASK  VTSS_BIT(22)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK . ADV_CL_VLAN_WAS_TAGGED_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK_ADV_CL_VLAN_WAS_TAGGED_STICKY_MASK  VTSS_BIT(21)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK . ADV_CL_COSID_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK_ADV_CL_COSID_STICKY_MASK  VTSS_BIT(20)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK . ADV_CL_DSCP_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK_ADV_CL_DSCP_STICKY_MASK  VTSS_BIT(19)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK . ADV_CL_QOS_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK_ADV_CL_QOS_STICKY_MASK  VTSS_BIT(18)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK . ADV_CL_DP_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK_ADV_CL_DP_STICKY_MASK  VTSS_BIT(17)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK . ADV_CL_DEI_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK_ADV_CL_DEI_STICKY_MASK  VTSS_BIT(16)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK . ADV_CL_PCP_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK_ADV_CL_PCP_STICKY_MASK  VTSS_BIT(15)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK . ADV_CL_VID_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK_ADV_CL_VID_STICKY_MASK  VTSS_BIT(14)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK . ADV_CL_VLAN_POPCNT_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK_ADV_CL_VLAN_POPCNT_STICKY_MASK  VTSS_BIT(13)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK . ADV_CL_MAP_TBL_IDX0_UPDATED_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK_ADV_CL_MAP_TBL_IDX0_UPDATED_STICKY_MASK  VTSS_BIT(12)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK . ADV_CL_MAP_TBL_IDX1_UPDATED_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK_ADV_CL_MAP_TBL_IDX1_UPDATED_STICKY_MASK  VTSS_BIT(11)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK . ADV_CL_ISDX_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK_ADV_CL_ISDX_STICKY_MASK  VTSS_BIT(10)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK . ADV_CL_MASQ_INJ_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK_ADV_CL_MASQ_INJ_STICKY_MASK  VTSS_BIT(9)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK . ADV_CL_CENTRALIZED_INJ_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK_ADV_CL_CENTRALIZED_INJ_STICKY_MASK  VTSS_BIT(8)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK . ADV_CL_GENERIC_IDX_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK_ADV_CL_GENERIC_IDX_STICKY_MASK  VTSS_BIT(7)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK . ADV_CL_NXT_KEY_TYPE_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK_ADV_CL_NXT_KEY_TYPE_STICKY_MASK  VTSS_BIT(6)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK . ADV_CL_NXT_NORM_W16_OFFSET_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK_ADV_CL_NXT_NORM_W16_OFFSET_STICKY_MASK  VTSS_BIT(5)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK . ADV_CL_NXT_TYPE_ETH_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK_ADV_CL_NXT_TYPE_ETH_STICKY_MASK  VTSS_BIT(4)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK . ADV_CL_NXT_TYPE_MPLS_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK_ADV_CL_NXT_TYPE_MPLS_STICKY_MASK  VTSS_BIT(3)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK . ADV_CL_NXT_TYPE_CW_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK_ADV_CL_NXT_TYPE_CW_STICKY_MASK  VTSS_BIT(2)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK . ADV_CL_NXT_NORMALIZE_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK_ADV_CL_NXT_NORMALIZE_STICKY_MASK  VTSS_BIT(1)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK . ADV_CL_NXT_OFFSET_TOO_BIG_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_ADV_CL_STICKY_MASK_ADV_CL_NXT_OFFSET_TOO_BIG_STICKY_MASK  VTSS_BIT(0)


/** 
 * \brief Sticky counter mask register
 *
 * \details
 * Register: \a ANA_CL:STICKY_MASK:MIP_STICKY_MASK
 *
 * @param gi Replicator: x_ANA_NUM_CONCURRENT_CNT (??), 0-3
 */
#define VTSS_ANA_CL_STICKY_MASK_MIP_STICKY_MASK(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x7f6c,gi,10,0,8)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_MIP_STICKY_MASK . MIP_MEL_CHK_FAIL_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_MIP_STICKY_MASK_MIP_MEL_CHK_FAIL_STICKY_MASK  VTSS_BIT(6)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_MIP_STICKY_MASK . MIP_LBM_DA_CHK_FAIL_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_MIP_STICKY_MASK_MIP_LBM_DA_CHK_FAIL_STICKY_MASK  VTSS_BIT(5)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_MIP_STICKY_MASK . MIP_GENERIC_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_MIP_STICKY_MASK_MIP_GENERIC_STICKY_MASK  VTSS_BIT(4)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_MIP_STICKY_MASK . MIP_RAPS_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_MIP_STICKY_MASK_MIP_RAPS_STICKY_MASK  VTSS_BIT(3)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_MIP_STICKY_MASK . MIP_LTM_REDIR_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_MIP_STICKY_MASK_MIP_LTM_REDIR_STICKY_MASK  VTSS_BIT(2)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_MIP_STICKY_MASK . MIP_LBM_REDIR_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_MIP_STICKY_MASK_MIP_LBM_REDIR_STICKY_MASK  VTSS_BIT(1)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_MIP_STICKY_MASK . MIP_CCM_COPY_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_MIP_STICKY_MASK_MIP_CCM_COPY_STICKY_MASK  VTSS_BIT(0)


/** 
 * \brief Sticky counter mask register
 *
 * \details
 * Register: \a ANA_CL:STICKY_MASK:IP_HDR_CHK_STICKY_MASK
 *
 * @param gi Replicator: x_ANA_NUM_CONCURRENT_CNT (??), 0-3
 */
#define VTSS_ANA_CL_STICKY_MASK_IP_HDR_CHK_STICKY_MASK(gi)  VTSS_IOREG_IX(VTSS_TO_ANA_CL,0x7f6c,gi,10,0,9)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_IP_HDR_CHK_STICKY_MASK . IP_OPTIONS_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_IP_HDR_CHK_STICKY_MASK_IP_OPTIONS_STICKY_MASK  VTSS_BIT(3)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_IP_HDR_CHK_STICKY_MASK . IP4_FRAGMENT_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_IP_HDR_CHK_STICKY_MASK_IP4_FRAGMENT_STICKY_MASK  VTSS_BIT(2)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_IP_HDR_CHK_STICKY_MASK . IP4_LEN_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_IP_HDR_CHK_STICKY_MASK_IP4_LEN_STICKY_MASK  VTSS_BIT(1)

/** 
 * \brief
 * Mask count of sticky event.
 *
 * \details 
 * 0: Disable event count
 * 1: Enable event count
 *
 * Field: ::VTSS_ANA_CL_STICKY_MASK_IP_HDR_CHK_STICKY_MASK . IP4_CHKSUM_STICKY_MASK
 */
#define  VTSS_F_ANA_CL_STICKY_MASK_IP_HDR_CHK_STICKY_MASK_IP4_CHKSUM_STICKY_MASK  VTSS_BIT(0)


#endif /* _VTSS_JAGUAR2_REGS_ANA_CL_H_ */
