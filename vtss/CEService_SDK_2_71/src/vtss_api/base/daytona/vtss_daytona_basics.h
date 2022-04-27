/*

 Vitesse API software.

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

 $Id$
 $Revision$

*/

#ifndef _VTSS_DAYTONA_BASICS_H_
#define _VTSS_DAYTONA_BASICS_H_

#include "vtss_types.h"
#include "vtss_init_api.h"

#if defined(VTSS_ARCH_DAYTONA)
/* ================================================================= *
 *  Register access
 * ================================================================= */

/* Read register */
#define DAYTONA_RD(p, value)                            \
    {                                                  \
        vtss_rc __rc__ = daytona_pi_rd(vtss_state,0,p,value);   \
        if (__rc__ != VTSS_RC_OK)                          \
            return __rc__;                                 \
    }

#define DAYTONA_RD_D(p, value)  daytona_pi_rd(vtss_state,0,p,value)

/* Write register */
#define DAYTONA_WR(p, value)                            \
    {                                              \
        vtss_rc __rc__ = daytona_pi_wr(vtss_state,0,p,value); \
        if (__rc__ != VTSS_RC_OK)                          \
            return __rc__;                                 \
    }

/* Write register field*/
#define DAYTONA_WRF(p,o,w,value)                            \
    {                                              \
        vtss_rc __rc__ = daytona_wrf(vtss_state,0,p,o,w,value); \
        if (__rc__ != VTSS_RC_OK)                          \
            return __rc__;                                 \
    }

/* Read register field*/
#define DAYTONA_RDF(p,o,w,value)                            \
    {                                              \
        vtss_rc __rc__ = daytona_rdf(vtss_state,0,p,o,w,value); \
        if (__rc__ != VTSS_RC_OK)                          \
            return __rc__;                                 \
    }

#define DAYTONA_WRM(p, value, mask)                            \
    {                                                         \
        vtss_rc __rc__ = daytona_wrm(vtss_state,0, p, value, mask); \
        if (__rc__ != VTSS_RC_OK)                                 \
            return __rc__;                                        \
    }

#define DAYTONA_WR_OR(p, mask) DAYTONA_WRM(p, mask, mask)

/* Poll bit until equal to val */
#define DAYTONA_POLL_FIELD(p, o, w, val)                                \
{                                                                       \
    u32 _x_, count = 0;                                                 \
    do {                                                                \
        DAYTONA_RDF(p, o, w, &_x_);                                     \
        VTSS_MSLEEP(1);                                                 \
        count++;                                                        \
        if (count == 100) {                                             \
            VTSS_E("Timeout occured while polling the Field: %x", p);   \
            return VTSS_RC_ERROR;                                       \
        }                                                               \
    } while (_x_ != val);                                               \
}

#if 0
/* tbd if needed */
/* Read register field */
#define DAYTONA_RDF(tgt, addr, fld, port, value) \
    { \
        vtss_rc __rc__; \
        if ((__rc__ = daytona_rdf(VTSS_TGT_##tgt, VTSS_ADDR_##tgt##_##addr, VTSS_OFF_##tgt##_##addr##_##fld, VTSS_LEN_##tgt##_##addr##_##fld, port, value))<VTSS_RC_OK) \
            return __rc__; \
    }

/* Write register field */
#define DAYTONA_WRF(tgt, addr, fld, port, value) \
    { \
        vtss_rc __rc__; \
        if ((__rc__ = daytona_wrf(VTSS_TGT_##tgt, VTSS_ADDR_##tgt##_##addr, VTSS_OFF_##tgt##_##addr##_##fld, VTSS_LEN_##tgt##_##addr##_##fld, port, value))<VTSS_RC_OK) \
            return __rc__; \
    }
#endif

#define VTSS_DAYTONA_RC(expr) { vtss_rc __rc__ = (expr); if (__rc__ < VTSS_RC_OK) {VTSS_E("Line %d, Error return %x", __LINE__, __rc__); return __rc__; }}

/* ================================================================= *
 *  Block operation modes defined according to
 *  DADS00 Top level Design Specification section 4.11
 * ================================================================= */

 /** \brief GENERAL Operation mode pr. block */
typedef enum  {
    DAYTONA_BLOCK_DISABLED = 0,
    DAYTONA_BLOCK_BYPASSED,
    DAYTONA_BLOCK_PASS_THROUGH,
    DAYTONA_BLOCK_NON_INTRUSIVE,
    DAYTONA_BLOCK_TERMINATING,
} daytona_block_mode_t;

/** \brief OTN Operation mode pr. block */
typedef enum  {
    DAYTONA_OTN_DISABLED = 0,
    DAYTONA_OTN_BYPASSED,
    DAYTONA_OTN_PASS_THROUGH,
    DAYTONA_OTN_MAP_DEMAP,
    DAYTONA_OTN_STREAMING,
} daytona_otn_block_mode_t;

/** \brief UPI Operation mode pr. block */
typedef enum  {
    DAYTONA_UPI_DISABLED = 0,
    DAYTONA_UPI_BYPASSED,
    DAYTONA_UPI_XAUI,
    DAYTONA_UPI_TFI_5,
    DAYTONA_UPI_SFI_4_2,
} daytona_upi_block_mode_t;

/** \brief WIS Operation mode pr. block */
typedef enum  {
    DAYTONA_WIS_DISABLED = 0,
    DAYTONA_WIS_BYPASSED,
    DAYTONA_WIS_PASS_THROUGH,
    DAYTONA_WIS_NON_INT_OC_192,
    DAYTONA_WIS_NON_INT_WAN,
    DAYTONA_WIS_TERM_OC_192,
    DAYTONA_WIS_TERM_WAN,
} daytona_wis_block_mode_t;

/** \brief RAB Operation mode pr. block */
typedef enum  {
    DAYTONA_RAB_DISABLED = 0,
    DAYTONA_RAB_BYPASSED,
    DAYTONA_RAB_MAC,
    DAYTONA_RAB_GFP,
} daytona_rab_block_mode_t;

/** \brief GFP Operation mode pr. block */
typedef enum  {
    DAYTONA_GFP_DISABLED = 0,
    DAYTONA_GFP_BYPASSED,
    DAYTONA_GFP_GFP_F_7041,
    DAYTONA_GFP_GFP_F_43,
    DAYTONA_GFP_GFP_T,
} daytona_gfp_block_mode_t;

/** \brief Functional block classes */
typedef enum  {
    DAYTONA_BLOCK_DEVCPU_ORG,
    DAYTONA_BLOCK_FAST_REGS,
    DAYTONA_BLOCK_DEVCPU_GCB,
    DAYTONA_BLOCK_UPI,
    DAYTONA_BLOCK_XFI,
    DAYTONA_BLOCK_TFI_5,
    DAYTONA_BLOCK_SFI4,
    DAYTONA_BLOCK_PCS_XAUI,
    DAYTONA_BLOCK_OTN_WRAPPER,
    DAYTONA_BLOCK_EWIS,
    DAYTONA_BLOCK_PCS_10GBASE_R,
    DAYTONA_BLOCK_GFP,
    DAYTONA_BLOCK_MAC10G,
    DAYTONA_BLOCK_RAB,
    DAYTONA_BLOCK_EFECI4,
    DAYTONA_BLOCK_EFECI7,
    DAYTONA_BLOCK_IP_1588,
    DAYTONA_BLOCK_IP_1588_ANA0_INGR,
    DAYTONA_BLOCK_IP_1588_ANA0_EGR,
    DAYTONA_BLOCK_IP_1588_ANA1_INGR,
    DAYTONA_BLOCK_IP_1588_ANA1_EGR,
    DAYTONA_BLOCK_IP_1588_ANA2_INGR,
    DAYTONA_BLOCK_IP_1588_ANA2_EGR,
} daytona_block_id_t;

typedef u32 daytona_channel_t; /* 0..1 */
typedef u32 daytona_side_t; /* 0 = Client side, 1 = Line side */
#define CLIENT_SIDE 0
#define LINE_SIDE 1

/**
 * \brief Get target id from {channel, side, blockid}
 *
 * \param channel [IN]  Chip channel no [0..1].
 * \param side [IN]     Channel side (CLIENT_SIDE = 0, LINE_SIDE = 1.
 * \param fb [IN]       Functional block id.
 * \param target [OUT]  Pointer to where the target id is returned.
 *
 * \return Return code.
 **/
vtss_rc daytona_target(daytona_channel_t channel, daytona_side_t side, daytona_block_id_t fb, u32 *target);

/**
 * \brief Get channel mode from port_no
 *
 * \param port [IN]   Port no [0..3].
 * \param mode [OUT]  Channel mode corresponding to the channel port belongs to.
 *
 * \return Return code.
 **/
vtss_rc daytona_port_2_mode(vtss_state_t *vtss_state,
                            vtss_port_no_t port, vtss_config_mode_t *mode);

/**
 * \brief Get target id from {port, blockid}
 *
 * \param port [IN]     Port no [0..3].
 * \param fb [IN]       Functional block id.
 * \param target [OUT]  Pointer to where the target id is returned.
 *
 * \return Return code.
 **/
vtss_rc daytona_port_2_target(vtss_state_t *vtss_state,
                              vtss_port_no_t port, daytona_block_id_t fb, u32 *target);

/**
 * \brief Get channel and side id from port_no.
 *
 * \param port [IN]     Port no [0..3].
 * \param channel [OUT] channel number of the corresponding port.
 * \param side    [OUT] Side of the corresponding channel.
 *
 * \return Return code.
 **/
vtss_rc daytona_port_2_channel_side(vtss_state_t *vtss_state,
                                    vtss_port_no_t port, daytona_channel_t *channel, daytona_side_t *side);

/**
 * \brief Read target register via PI
 *
 * \param chip_no [IN]  Chip no in a multi chip application.
 * \param addr [IN]     Target address (register offset from chip base address).
 * \param value [OUT]   Pointer to where the register value is returned.
 *
 * \return Return code.
 **/
vtss_rc daytona_pi_rd(vtss_state_t *vtss_state,
                      vtss_chip_no_t chip_no, u32 addr, u32 *value);

/**
 * \brief Write target register via PI (direct or indirect register access)
 *
 * \param chip_no [IN]  Chip no in a multi chip application.
 * \param addr [IN]     Target address (register offset from chip base address).
 * \param value [IN]    The register value.
 *
 * \return Return code.
 **/
vtss_rc daytona_pi_wr(vtss_state_t *vtss_state,
                      vtss_chip_no_t chip_no, u32 addr, const u32 value);

/**
 * \brief Read register field
 *
 * \param chip_no [IN]      Chip no in a multi chip application.
 * \param reg [IN]          Target address (register offset from chip base address).
 * \param bit_offset [IN]   Bit offset in the register
 * \param bit_width [IN]    Number of bits in the field.
 * \param value [OUT]       Pointer to where the register field value is returned.
 *
 * \return Return code.
 **/
vtss_rc daytona_rdf(vtss_state_t *vtss_state,
                    vtss_chip_no_t chip_no, u32 reg, u32 bit_offset, u32 bit_width, u32 *value);

/**
 * \brief Write register field
 *
 * \param chip_no [IN]      Chip no in a multi chip application.
 * \param reg [IN]          Target address (register offset from chip base address).
 * \param bit_offset [IN]   Bit offset in the register
 * \param bit_width [IN]    Number of bits in the field.
 * \param value [IN]        The register field value.
 *
 * \return Return code.
 **/
vtss_rc daytona_wrf(vtss_state_t *vtss_state,
                    vtss_chip_no_t chip_no, u32 reg, u32 bit_offset, u32 bit_width, u32 value);

/*  */
/**
 * \brief Write register mask
 *
 * \param chip_no [IN]      Chip no in a multi chip application.
 * \param reg [IN]          Target address (register offset from chip base address).
 * \param value [IN]        The register value.
 * \param mask [IN]         bit mask, which bits are affected by the function.
 *
 * \return Return code.
 **/
vtss_rc daytona_wrm(vtss_state_t *vtss_state,
                    vtss_chip_no_t chip_no, u32 reg, u32 value, u32 mask);

#endif /* VTSS_ARCH_DAYTONA */
#endif /* _VTSS_DAYTONA_BASICS_H_ */
