#ifndef _VTSS_DAYTONA_REGS_EFECI4_H_
#define _VTSS_DAYTONA_REGS_EFECI4_H_

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

#include "vtss_daytona_regs_common.h"

/*********************************************************************** 
 *
 * Target: \a EFECI4
 *
 * \see vtss_target_EFECI4_e
 *
 * Configuration and status registers for efeci4 module
 *
 ***********************************************************************/

/**
 * Register Group: \a EFECI4::EFECI4_ENCODER_CFG
 *
 * Configuration and status registers for efeci4_encoder module
 */


/** 
 * \brief Configuration settings for efeci4_encoder
 *
 * \details
 * Register: \a EFECI4::EFECI4_ENCODER_CFG::EFECI4_ENCODER_CONFIG1_CFG
 *
 * @param target A \a ::vtss_target_EFECI4_e target
 */
#define VTSS_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_CONFIG1_CFG(target)  VTSS_IOREG(target,0x0)

/** 
 * \brief
 * Encoder switch reset
 *
 * \details 
 * Field: ::VTSS_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_CONFIG1_CFG . ENCODER_SW_RST
 */
#define  VTSS_F_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_CONFIG1_CFG_ENCODER_SW_RST(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_CONFIG1_CFG_ENCODER_SW_RST     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_CONFIG1_CFG_ENCODER_SW_RST(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Encoder switch enable
 *
 * \details 
 * Field: ::VTSS_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_CONFIG1_CFG . ENCODER_SW_ENA
 */
#define  VTSS_F_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_CONFIG1_CFG_ENCODER_SW_ENA(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_CONFIG1_CFG_ENCODER_SW_ENA     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_CONFIG1_CFG_ENCODER_SW_ENA(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Bypass encoder
 *
 * \details 
 * 0: Encoder not bypassed
 * 1: Encoder bypassed
 *
 * Field: ::VTSS_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_CONFIG1_CFG . ENCODER_BYPASS
 */
#define  VTSS_F_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_CONFIG1_CFG_ENCODER_BYPASS(x)  VTSS_ENCODE_BITFIELD(x,1,1)
#define  VTSS_M_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_CONFIG1_CFG_ENCODER_BYPASS     VTSS_ENCODE_BITMASK(1,1)
#define  VTSS_X_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_CONFIG1_CFG_ENCODER_BYPASS(x)  VTSS_EXTRACT_BITFIELD(x,1,1)


/** 
 * \brief Error generator error rate
 *
 * \details
 * Register: \a EFECI4::EFECI4_ENCODER_CFG::EFECI4_ENCODER_ERR_RATE_CFG
 *
 * @param target A \a ::vtss_target_EFECI4_e target
 */
#define VTSS_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_ERR_RATE_CFG(target)  VTSS_IOREG(target,0x2)

/** 
 * \brief
 * Programmable error rate: BER = ERR_RATE/65536
 *
 * \details 
 * Field: ::VTSS_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_ERR_RATE_CFG . ERR_RATE
 */
#define  VTSS_F_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_ERR_RATE_CFG_ERR_RATE(x)  VTSS_ENCODE_BITFIELD(x,0,9)
#define  VTSS_M_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_ERR_RATE_CFG_ERR_RATE     VTSS_ENCODE_BITMASK(0,9)
#define  VTSS_X_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_ERR_RATE_CFG_ERR_RATE(x)  VTSS_EXTRACT_BITFIELD(x,0,9)

/**
 * Register Group: \a EFECI4::EFECI4_ENCODER_CTR
 *
 * Counters for Encoder
 */

/**
 * Register Group: \a EFECI4::EFECI4_ENCODER_ERR_GEN_SEED
 *
 * Random seed value for error generator
 */


/** 
 * \brief Random seed value for error generator
 *
 * \details
 * This is the current random seed for the error generator.
 *
 * Register: \a EFECI4::EFECI4_ENCODER_ERR_GEN_SEED::EFECI4_ERR_GEN_SEED
 *
 * @param target A \a ::vtss_target_EFECI4_e target
 * @param ri Register: EFECI4_ERR_GEN_SEED (??), 0-7
 */
#define VTSS_EFECI4_EFECI4_ENCODER_ERR_GEN_SEED_EFECI4_ERR_GEN_SEED(target,ri)  VTSS_IOREG(target,0x4 + (ri))

/** 
 * \details 
 * Field: ::VTSS_EFECI4_EFECI4_ENCODER_ERR_GEN_SEED_EFECI4_ERR_GEN_SEED . RANDOM_SEED
 */
#define  VTSS_F_EFECI4_EFECI4_ENCODER_ERR_GEN_SEED_EFECI4_ERR_GEN_SEED_RANDOM_SEED(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_EFECI4_EFECI4_ENCODER_ERR_GEN_SEED_EFECI4_ERR_GEN_SEED_RANDOM_SEED     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_EFECI4_EFECI4_ENCODER_ERR_GEN_SEED_EFECI4_ERR_GEN_SEED_RANDOM_SEED(x)  VTSS_EXTRACT_BITFIELD(x,0,32)

/**
 * Register Group: \a EFECI4::EFECI4_DECODER_CFG
 *
 * Configuration and status registers for efeci4_decoder module
 */


/** 
 * \brief Configuration settings for efeci4_decoder
 *
 * \details
 * Register: \a EFECI4::EFECI4_DECODER_CFG::EFECI4_DECODER_CONFIG1_CFG
 *
 * @param target A \a ::vtss_target_EFECI4_e target
 */
#define VTSS_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG(target)  VTSS_IOREG(target,0xc)

/** 
 * \brief
 * Decoder switch reset
 *
 * \details 
 * Field: ::VTSS_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG . DECODER_SW_RST
 */
#define  VTSS_F_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_SW_RST(x)  VTSS_ENCODE_BITFIELD(x,8,1)
#define  VTSS_M_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_SW_RST     VTSS_ENCODE_BITMASK(8,1)
#define  VTSS_X_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_SW_RST(x)  VTSS_EXTRACT_BITFIELD(x,8,1)

/** 
 * \brief
 * Decoder switch enable
 *
 * \details 
 * Field: ::VTSS_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG . DECODER_SW_ENA
 */
#define  VTSS_F_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_SW_ENA(x)  VTSS_ENCODE_BITFIELD(x,7,1)
#define  VTSS_M_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_SW_ENA     VTSS_ENCODE_BITMASK(7,1)
#define  VTSS_X_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_SW_ENA(x)  VTSS_EXTRACT_BITFIELD(x,7,1)

/** 
 * \brief
 * If OTN scrambler is enabled, set this field to 1 to compensate for
 * scrambler's effect on corrected 1s and 0s counts
 *
 * \details 
 * 0: Do not adjust counters for scrambling
 * 1: Adjust counters for scrambling
 *
 * Field: ::VTSS_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG . DECODER_SCRAM_EN
 */
#define  VTSS_F_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_SCRAM_EN(x)  VTSS_ENCODE_BITFIELD(x,6,1)
#define  VTSS_M_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_SCRAM_EN     VTSS_ENCODE_BITMASK(6,1)
#define  VTSS_X_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_SCRAM_EN(x)  VTSS_EXTRACT_BITFIELD(x,6,1)

/** 
 * \brief
 * Setting this self-clearing bit to 1 will load all count holding
 * registers and clear all counters to start another interval
 *
 * \details 
 * Field: ::VTSS_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG . DECODER_LOAD_COUNTERS
 */
#define  VTSS_F_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_LOAD_COUNTERS(x)  VTSS_ENCODE_BITFIELD(x,5,1)
#define  VTSS_M_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_LOAD_COUNTERS     VTSS_ENCODE_BITMASK(5,1)
#define  VTSS_X_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_LOAD_COUNTERS(x)  VTSS_EXTRACT_BITFIELD(x,5,1)

/** 
 * \brief
 * Enable interval counters
 *
 * \details 
 * 0: All counters count continuously until saturating or being cleared by
 * software
 * 1: All counters are cleared at the beginning of each programmable
 * interval and counts in the holding registers reflect the count from the
 * previous interval
 *
 * Field: ::VTSS_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG . DECODER_PMTICK_EN
 */
#define  VTSS_F_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_PMTICK_EN(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_PMTICK_EN     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_PMTICK_EN(x)  VTSS_EXTRACT_BITFIELD(x,4,1)

/** 
 * \brief
 * Bypass de-interleaver
 *
 * \details 
 * 0: Output de-interleaved into G.709 format
 * 1: Output not de-interleaved into G.709 format
 *
 * Field: ::VTSS_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG . DECODER_DI_BYPASS
 */
#define  VTSS_F_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_DI_BYPASS(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_DI_BYPASS     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_DI_BYPASS(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/** 
 * \brief
 * Decoder latency setting. Higher latency provides more coding gain.
 *
 * \details 
 * 00: Output bypasses decoder processing completely
 * 01: Output has decoder processing applied at minimum level
 * 10: Output has decoder processing applied at medium level
 * 11: Output has maximum decoder processing applied
 *
 * Field: ::VTSS_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG . DECODER_LATENCY
 */
#define  VTSS_F_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_LATENCY(x)  VTSS_ENCODE_BITFIELD(x,0,2)
#define  VTSS_M_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_LATENCY     VTSS_ENCODE_BITMASK(0,2)
#define  VTSS_X_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_LATENCY(x)  VTSS_EXTRACT_BITFIELD(x,0,2)

/**
 * Register Group: \a EFECI4::EFECI4_DECODER_INT_CTR
 *
 * Not documented
 */

/**
 * Register Group: \a EFECI4::EFECI4_DECODER_CTR
 *
 * Not documented
 */


/** 
 * \brief Total number of corrected 0s
 *
 * \details
 * Register: \a EFECI4::EFECI4_DECODER_CTR::EFECI4_DECODER_CORRECTED0_CNT
 *
 * @param target A \a ::vtss_target_EFECI4_e target
 */
#define VTSS_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_CORRECTED0_CNT(target)  VTSS_IOREG(target,0x1e)

/** 
 * \brief
 * Corrected 0s counter
 *
 * \details 
 * Field: ::VTSS_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_CORRECTED0_CNT . CORRECTED0
 */
#define  VTSS_F_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_CORRECTED0_CNT_CORRECTED0(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_CORRECTED0_CNT_CORRECTED0     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_CORRECTED0_CNT_CORRECTED0(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Total number of corrected 1s
 *
 * \details
 * Register: \a EFECI4::EFECI4_DECODER_CTR::EFECI4_DECODER_CORRECTED1_CNT
 *
 * @param target A \a ::vtss_target_EFECI4_e target
 */
#define VTSS_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_CORRECTED1_CNT(target)  VTSS_IOREG(target,0x1f)

/** 
 * \brief
 * Corrected 1s counter
 *
 * \details 
 * Field: ::VTSS_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_CORRECTED1_CNT . CORRECTED1
 */
#define  VTSS_F_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_CORRECTED1_CNT_CORRECTED1(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_CORRECTED1_CNT_CORRECTED1     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_CORRECTED1_CNT_CORRECTED1(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Total number of RS uncorrectable codewords
 *
 * \details
 * Register: \a EFECI4::EFECI4_DECODER_CTR::EFECI4_DECODER_RS_UCCW_CNT
 *
 * @param target A \a ::vtss_target_EFECI4_e target
 */
#define VTSS_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_RS_UCCW_CNT(target)  VTSS_IOREG(target,0x20)

/** 
 * \brief
 * RS-FEC uncorrected code word error counter
 *
 * \details 
 * Field: ::VTSS_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_RS_UCCW_CNT . RS_UCCW
 */
#define  VTSS_F_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_RS_UCCW_CNT_RS_UCCW(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_RS_UCCW_CNT_RS_UCCW     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_RS_UCCW_CNT_RS_UCCW(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Total number of BCH uncorrectable codewords
 *
 * \details
 * Register: \a EFECI4::EFECI4_DECODER_CTR::EFECI4_DECODER_BCH_UCCW_CNT
 *
 * @param target A \a ::vtss_target_EFECI4_e target
 */
#define VTSS_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_BCH_UCCW_CNT(target)  VTSS_IOREG(target,0x21)

/** 
 * \brief
 * BCH-FEC uncorrected code word error counter
 *
 * \details 
 * Field: ::VTSS_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_BCH_UCCW_CNT . BCH_UCCW
 */
#define  VTSS_F_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_BCH_UCCW_CNT_BCH_UCCW(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_BCH_UCCW_CNT_BCH_UCCW     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_BCH_UCCW_CNT_BCH_UCCW(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Total number of frames with uncorrectable codewords detected by BCH and/or RS block that will not be corrected by ED block
 *
 * \details
 * Register: \a EFECI4::EFECI4_DECODER_CTR::EFECI4_DECODER_ED_UCCW_CNT
 *
 * @param target A \a ::vtss_target_EFECI4_e target
 */
#define VTSS_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_ED_UCCW_CNT(target)  VTSS_IOREG(target,0x22)

/** 
 * \brief
 * Uncorrectable code word error counter
 *
 * \details 
 * Field: ::VTSS_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_ED_UCCW_CNT . ED_UCCW
 */
#define  VTSS_F_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_ED_UCCW_CNT_ED_UCCW(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_ED_UCCW_CNT_ED_UCCW     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_ED_UCCW_CNT_ED_UCCW(x)  VTSS_EXTRACT_BITFIELD(x,0,32)


/** 
 * \brief Total number of frames received by the decoder
 *
 * \details
 * Register: \a EFECI4::EFECI4_DECODER_CTR::EFECI4_DECODER_FRAME_CNT
 *
 * @param target A \a ::vtss_target_EFECI4_e target
 */
#define VTSS_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_FRAME_CNT(target)  VTSS_IOREG(target,0x23)

/** 
 * \brief
 * Decoded frame counter
 *
 * \details 
 * Field: ::VTSS_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_FRAME_CNT . FRAMES
 */
#define  VTSS_F_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_FRAME_CNT_FRAMES(x)  VTSS_ENCODE_BITFIELD(x,0,32)
#define  VTSS_M_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_FRAME_CNT_FRAMES     VTSS_ENCODE_BITMASK(0,32)
#define  VTSS_X_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_FRAME_CNT_FRAMES(x)  VTSS_EXTRACT_BITFIELD(x,0,32)

/** 
 * \brief Interrupt Mask for EFECI4
 *
 * \details
 * Register: \a EFECI4::EFECI4_INTR::EFECI4_INTR_EN
 *
 * @param target A \a ::vtss_target_EFECI4_e target
 */
#define VTSS_EFECI4_EFECI4_INTR_EFECI4_INTR_EN(target)  VTSS_IOREG(target,0x26)

/** 
 * \brief
 * Interrupt mask for ED_UCCW
 *
 * \details 
 * Field: ::VTSS_EFECI4_EFECI4_INTR_EFECI4_INTR_EN . ED_UCCW_MASK 
 */
#define  VTSS_F_EFECI4_EFECI4_INTR_EFECI4_INTR_EN_ED_UCCW_MASK(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_EFECI4_EFECI4_INTR_EFECI4_INTR_EN_ED_UCCW_MASK     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_EFECI4_EFECI4_INTR_EFECI4_INTR_EN_ED_UCCW_MASK(x)  VTSS_EXTRACT_BITFIELD(x,4,1)


/** 
 * \brief
 * Interrupt mask for RS_UCCW
 *
 * \details 
 * Field: ::VTSS_EFECI4_EFECI4_INTR_EFECI4_INTR_EN . RS_UCCW_MASK
 */
#define  VTSS_F_EFECI4_EFECI4_INTR_EFECI4_INTR_EN_RS_UCCW_MASK(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_EFECI4_EFECI4_INTR_EFECI4_INTR_EN_RS_UCCW_MASK     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_EFECI4_EFECI4_INTR_EFECI4_INTR_EN_RS_UCCW_MASK(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Interrupt mask for BCH_UCCW
 *
 * \details 
 * Field: ::VTSS_EFECI4_EFECI4_INTR_EFECI4_INTR_EN . BCH_UCCW_MASK
 */
#define  VTSS_F_EFECI4_EFECI4_INTR_EFECI4_INTR_EN_BCH_UCCW_MASK(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_EFECI4_EFECI4_INTR_EFECI4_INTR_EN_BCH_UCCW_MASK     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_EFECI4_EFECI4_INTR_EFECI4_INTR_EN_BCH_UCCW_MASK(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/**
 * Register Group: \a EFECI4::EFECI4_INTR::EFECI4_INTR
 *
 */

/** 
 * \brief Interrupt pending for EFECI4
 *
 * \details
 * Register: \a EFECI4::EFECI4_INTR
 *
 * @param target A \a ::vtss_target_EFECI4_e target
 */
#define VTSS_EFECI4_EFECI4_INTR_EFECI4_INTR(target)  VTSS_IOREG(target,0x27)

/** 
 * \brief
 * Interrupt pending for ED_UCCW
 *
 * \details 
 * Field: ::VTSS_EFECI4_EFECI4_INTR_EFECI4_INTR . ED_UCCW_STICKY 
 */
#define  VTSS_F_EFECI4_EFECI4_INTR_EFECI4_INTR_ED_UCCW_STICKY(x)  VTSS_ENCODE_BITFIELD(x,4,1)
#define  VTSS_M_EFECI4_EFECI4_INTR_EFECI4_INTR_ED_UCCW_STICKY     VTSS_ENCODE_BITMASK(4,1)
#define  VTSS_X_EFECI4_EFECI4_INTR_EFECI4_INTR_ED_UCCW_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,4,1)


/** 
 * \brief
 * Interrupt pending for RS_UCCW
 *
 * \details 
 * Field: ::VTSS_EFECI4_EFECI4_INTR_EFECI4_INTR . RS_UCCW_STICKY
 */
#define  VTSS_F_EFECI4_EFECI4_INTR_EFECI4_INTR_RS_UCCW_STICKY(x)  VTSS_ENCODE_BITFIELD(x,3,1)
#define  VTSS_M_EFECI4_EFECI4_INTR_EFECI4_INTR_RS_UCCW_STICKY     VTSS_ENCODE_BITMASK(3,1)
#define  VTSS_X_EFECI4_EFECI4_INTR_EFECI4_INTR_RS_UCCW_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,3,1)

/** 
 * \brief
 * Interrupt pending for BCH_UCCW
 *
 * \details 
 * Field: ::VTSS_EFECI4_EFECI4_INTR_EFECI4_INTR . BCH_UCCW_STICKY
 */
#define  VTSS_F_EFECI4_EFECI4_INTR_EFECI4_INTR_BCH_UCCW_STICKY(x)  VTSS_ENCODE_BITFIELD(x,2,1)
#define  VTSS_M_EFECI4_EFECI4_INTR_EFECI4_INTR_BCH_UCCW_STICKY     VTSS_ENCODE_BITMASK(2,1)
#define  VTSS_X_EFECI4_EFECI4_INTR_EFECI4_INTR_BCH_UCCW_STICKY(x)  VTSS_EXTRACT_BITFIELD(x,2,1)

/**
 * Register Group: \a EFECI4::EFECI4_SPARE
 *
 * Spare registers
 */


#endif /* _VTSS_DAYTONA_REGS_EFECI4_H_ */
