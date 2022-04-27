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

#define VTSS_TRACE_LAYER VTSS_TRACE_LAYER_CIL 

// Avoid "vtss_api.h not used in module vtss_daytona_wis.c"
/*lint --e{766} */

#include "vtss_api.h"
#if defined(VTSS_ARCH_DAYTONA)
#include "../ail/vtss_state.h"
#include "vtss_daytona.h"
#include "vtss_daytona_regs.h"
#include "vtss_daytona_basics.h"
#include "vtss_daytona_wis.h"
#include "vtss_daytona_reg_init_block_states.h"
#include "vtss_daytona_reg_init_wis.h"


/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */

/**
 * \brief Set wis mode.
 * \param port_no [IN]   Port number.
 * \return Return code.
 *
 * The main operation mode is defined in static configuration, i.e. only non_intrusive mode is dynamic.
 * Change on mode is only possible when the eWIS block is operating in OC192 mode.
 * Daytona register: SW only.
 **/
#define VTSS_EWIS_C2_BYTE_SONET 0x5
#define VTSS_EWIS_C2_BYTE_SDH   0x5
static vtss_rc daytona_ewis_mode_set(vtss_state_t *vtss_state,
                                     const vtss_port_no_t port_no)
{
    u32 target;
    vtss_ewis_mode_t  mode;
    block_wis_mode_t  blk_wis_mode;
    daytona_channel_t channel;
    daytona_side_t    side = 0;
    vtss_rc           rc = VTSS_RC_OK;
    u8                blk_mode;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EWIS, &target));
    do {
        mode = vtss_state->ewis_conf[port_no].ewis_mode;
        /* get the eWIS block mode */
        VTSS_RC(daytona_port_2_channel_side(vtss_state, port_no, &channel, &side));
        VTSS_RC(vtss_daytona_blk_oper_mode_get(vtss_state, port_no, channel, (side ? BLOCK_WIS_2 : BLOCK_WIS_1), &blk_mode));
        blk_wis_mode = (block_wis_mode_t) blk_mode;
        if ((mode == VTSS_WIS_OPERMODE_WIS_MODE) && (blk_wis_mode == BM_WIS_WAN)) {
            /* configure the block in WIS mode */
        } else if ((mode == VTSS_WIS_OPERMODE_STS192) && (blk_wis_mode == BM_WIS_OC192)) {
            /* Configure the mode in SONET mode */
            /* Signal Label configuration */
            DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_C2(target), VTSS_EWIS_C2_BYTE_SONET, VTSS_M_EWIS_TX_BYTE_TX_C2_TX_C2);
            DAYTONA_WRM(VTSS_EWIS_RX_WIS_CTRL_C2_CTRL(target), VTSS_EWIS_C2_BYTE_SONET, VTSS_M_EWIS_RX_WIS_CTRL_C2_CTRL_RX_EXPECTED_C2);

            /* Rx - Pointer Interpreter configuration */
            DAYTONA_WRM(VTSS_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL(target), VTSS_F_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_JUST_MODE(1), VTSS_M_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_JUST_MODE);
            DAYTONA_WRM(VTSS_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL(target), VTSS_F_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_SEC_PTR_MODE(0), VTSS_M_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_SEC_PTR_MODE);
            DAYTONA_WRM(VTSS_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL(target), VTSS_F_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_SS_ENA(0), VTSS_M_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_SS_ENA);
            DAYTONA_WRM(VTSS_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL(target), VTSS_F_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_H1_NORM_PTR_VALID_MODE(1), VTSS_M_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_H1_NORM_PTR_VALID_MODE);

        } else if ((mode == VTSS_WIS_OPERMODE_STM64) && (blk_wis_mode == BM_WIS_OC192)) {
            /* Configure the mode in SDH mode */
            DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_C2(target), VTSS_EWIS_C2_BYTE_SDH, VTSS_M_EWIS_TX_BYTE_TX_C2_TX_C2);
            DAYTONA_WRM(VTSS_EWIS_RX_WIS_CTRL_C2_CTRL(target), VTSS_EWIS_C2_BYTE_SDH, VTSS_M_EWIS_RX_WIS_CTRL_C2_CTRL_RX_EXPECTED_C2);

            /* Rx - Pointer Interpreter configuration */
            DAYTONA_WRM(VTSS_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL(target), VTSS_F_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_JUST_MODE(0), VTSS_M_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_JUST_MODE);
            DAYTONA_WRM(VTSS_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL(target), VTSS_F_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_SEC_PTR_MODE(1), VTSS_M_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_SEC_PTR_MODE);
            DAYTONA_WRM(VTSS_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL(target), VTSS_F_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_SS_ENA(1), VTSS_M_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_SS_ENA);
            DAYTONA_WRM(VTSS_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL(target), VTSS_F_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_H1_NORM_PTR_VALID_MODE(2), VTSS_M_EWIS_RX_WIS_CTRL_PNTR_INTRPRTR_CTRL_RX_H1_NORM_PTR_VALID_MODE);

        } else {
            rc = VTSS_RC_ERROR;
            break;
        }
    } while (0);
    return rc;
}

/**
 * \brief Set consequent actions, i.e. how to handle AIS-L insertion and RDI_L backreporting.
 * \param port_no [IN]   Port number.
 * \return Return code.
 *
 * Daytona registers:
 * fault_on_feplmp;     not supported 
 * fault_on_feaisp;     not supported
 * fault_on_rdil;       not supported
 * fault_on_sef;        not supported
 * fault_on_lof;        not supported
 * fault_on_los;        not supported
 * fault_on_aisl;       not supported
 * fault_on_lcdp;       not supported
 * fault_on_plmp;       not supported
 * fault_on_aisp;       not supported
 * fault_on_lopp;       not supported

 * ais_on_los;          LOS_CTRL::RX_WIS_INS_AISL_ON_LOS
 * ais_on_lof;          LOF_CTRL::RX_WIS_AISL_ON_LOF

 * rdil_on_los;         LOS_CTRL::RX_WIS_INS_RDIL_ON_LOS
 * rdil_on_lof;         LOF_CTRL::RX_WIS_INS_RDIL_ON_LOF
 * rdil_on_lopc;        not supported
 * rdil_on_ais_l;       MISC_LOH_CTRL::RX_WIS_INS_RDIL_ON_AISL
 **/
static vtss_rc daytona_ewis_cons_action_set(vtss_state_t *vtss_state,
                                            const vtss_port_no_t port_no)
{
    u32 target;
    u32 reg, mask;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EWIS, &target));

    reg = 0;
    mask = VTSS_M_EWIS_RX_WIS_CTRL_LOS_CTRL_RX_WIS_INS_AISL_ON_LOS | VTSS_M_EWIS_RX_WIS_CTRL_LOS_CTRL_RX_WIS_INS_RDIL_ON_LOS;

    if (vtss_state->ewis_conf[port_no].section_cons_act.aisl.ais_on_los)    reg |= VTSS_M_EWIS_RX_WIS_CTRL_LOS_CTRL_RX_WIS_INS_AISL_ON_LOS;
    if (vtss_state->ewis_conf[port_no].section_cons_act.rdil.rdil_on_los)   reg |= VTSS_M_EWIS_RX_WIS_CTRL_LOS_CTRL_RX_WIS_INS_RDIL_ON_LOS;

    DAYTONA_WRM(VTSS_EWIS_RX_WIS_CTRL_LOS_CTRL(target), reg, mask);

    reg = 0;
    mask = VTSS_M_EWIS_RX_WIS_CTRL_LOF_CTRL_RX_WIS_AISL_ON_LOF | VTSS_M_EWIS_RX_WIS_CTRL_LOF_CTRL_RX_WIS_INS_RDIL_ON_LOF;

    if (vtss_state->ewis_conf[port_no].section_cons_act.aisl.ais_on_lof)    reg |= VTSS_M_EWIS_RX_WIS_CTRL_LOF_CTRL_RX_WIS_AISL_ON_LOF;
    if (vtss_state->ewis_conf[port_no].section_cons_act.rdil.rdil_on_lof)   reg |= VTSS_M_EWIS_RX_WIS_CTRL_LOF_CTRL_RX_WIS_INS_RDIL_ON_LOF;

    DAYTONA_WRM(VTSS_EWIS_RX_WIS_CTRL_LOF_CTRL(target), reg, mask);

    reg = 0;
    mask = VTSS_M_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_RX_WIS_INS_RDIL_ON_AISL;

    if (vtss_state->ewis_conf[port_no].section_cons_act.rdil.rdil_on_ais_l)   reg |= VTSS_M_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_RX_WIS_INS_RDIL_ON_AISL;

    DAYTONA_WRM(VTSS_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL(target), reg, mask);

    return VTSS_RC_OK;
}

#define VTSS_DAYTONA_EWIS_TTI_MODE_1_LEN     1
#define VTSS_DAYTONA_EWIS_TTI_MODE_16_LEN    16
#define VTSS_DAYTONA_EWIS_TTI_MODE_64_LEN    64


#define VTSS_DAYTONA_EWIS_TTI_MODE_16_MATCH_BYTE      0x80

#define VTSS_DAYTONA_EWIS_TTI_MODE_64_MATCH_BYTE_0    13
#define VTSS_DAYTONA_EWIS_TTI_MODE_64_MATCH_BYTE_1    10

static vtss_rc daytona_ewis_section_txti_set(vtss_state_t *vtss_state,
                                             const vtss_port_no_t port_no)
{
    u32 target;
    vtss_ewis_tti_t txti;
    uint tti_size;
    u32 tti_trace_len;
    uint i;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EWIS, &target));

    txti = vtss_state->ewis_conf[port_no].section_txti;
    if (txti.mode == TTI_MODE_1) {
        tti_size = VTSS_DAYTONA_EWIS_TTI_MODE_1_LEN;
        tti_trace_len = 2;
    }
    else if (txti.mode == TTI_MODE_16) {
        tti_size = VTSS_DAYTONA_EWIS_TTI_MODE_16_LEN;
        tti_trace_len = 0;
    }
    else if (txti.mode == TTI_MODE_64) {
        tti_size = VTSS_DAYTONA_EWIS_TTI_MODE_64_LEN;
        tti_trace_len = 1;
    }
    else {
        VTSS_E("invalid tti mode");
        return VTSS_RC_ERROR;
    }
    /* configure trace length */
    DAYTONA_WRM(VTSS_EWIS_TX_WIS_CTRL_TX_MISC_CTRL(target),
                VTSS_F_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_J0_TRACE_LENGTH(tti_trace_len),
                VTSS_M_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_J0_TRACE_LENGTH);

    DAYTONA_WRM(VTSS_EWIS_RX_WIS_CTRL_MISC_SOH_CTRL(target),
                VTSS_F_EWIS_RX_WIS_CTRL_MISC_SOH_CTRL_RX_J0_64BYTE_TRACE(tti_trace_len),
                VTSS_M_EWIS_RX_WIS_CTRL_MISC_SOH_CTRL_RX_J0_64BYTE_TRACE);

    /* write tti */
    for (i=0; i < tti_size; i++) {
        DAYTONA_WR(VTSS_EWIS_TX_J0_INS_TX_J0(target,i), txti.tti[i]);
    }
    return VTSS_RC_OK;
}

static vtss_rc daytona_ewis_force_conf_set(vtss_state_t *vtss_state,
                                           const vtss_port_no_t port_no)
{
    u32 target;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EWIS, &target));
    DAYTONA_WRM(VTSS_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL(target), VTSS_F_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_LINE_RX_FORCE_AIS_L(vtss_state->ewis_conf[port_no].force_mode.line_rx_force.force_ais), VTSS_M_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_LINE_RX_FORCE_AIS_L);
    DAYTONA_WRM(VTSS_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL(target), VTSS_F_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_LINE_RX_FORCE_RDI_L(vtss_state->ewis_conf[port_no].force_mode.line_rx_force.force_rdi), VTSS_M_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_LINE_RX_FORCE_RDI_L);
    DAYTONA_WRM(VTSS_EWIS_TX_WIS_CTRL_TX_MISC_CTRL(target), VTSS_F_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_LINE_TX_FORCE_AIS_L(vtss_state->ewis_conf[port_no].force_mode.line_tx_force.force_ais), VTSS_M_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_LINE_TX_FORCE_AIS_L);
    DAYTONA_WRM(VTSS_EWIS_TX_WIS_CTRL_TX_MISC_CTRL(target), VTSS_F_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_LINE_TX_FORCE_RDI_L(vtss_state->ewis_conf[port_no].force_mode.line_tx_force.force_rdi), VTSS_M_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_LINE_TX_FORCE_RDI_L);
   return VTSS_RC_OK;
}

static vtss_rc daytona_ewis_tx_oh_set(vtss_state_t *vtss_state,
                                      const vtss_port_no_t port_no)
{
    u32 target;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EWIS, &target));

    /**< Section Overhead: */
    /**< Section Data Communications Channel(DCC) D1-D3*/
    DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_D1(target) , vtss_state->ewis_conf[port_no].tx_oh.tx_dcc_s[0], VTSS_M_EWIS_TX_BYTE_TX_D1_TX_D1);
    DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_D2(target) , vtss_state->ewis_conf[port_no].tx_oh.tx_dcc_s[1], VTSS_M_EWIS_TX_BYTE_TX_D2_TX_D2);
    DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_D3(target) , vtss_state->ewis_conf[port_no].tx_oh.tx_dcc_s[2], VTSS_M_EWIS_TX_BYTE_TX_D3_TX_D3);

    /**< Orderwire */
    DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_E1(target) , vtss_state->ewis_conf[port_no].tx_oh.tx_e1, VTSS_M_EWIS_TX_BYTE_TX_E1_TX_E1);

    /**< Section User Channel */
    DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_F1(target) , vtss_state->ewis_conf[port_no].tx_oh.tx_f1, VTSS_M_EWIS_TX_BYTE_TX_F1_TX_F1);

    /**< Reserved for Section growth */
    DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_Z0(target) , vtss_state->ewis_conf[port_no].tx_oh.tx_z0, VTSS_M_EWIS_TX_BYTE_TX_Z0_TX_Z0);

    /**< line overhead: */
    /**< Line Data Communications Channel (DCC) D4-D12 */
    DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_D4(target) , vtss_state->ewis_conf[port_no].tx_oh.tx_dcc_l[0], VTSS_M_EWIS_TX_BYTE_TX_D4_TX_D4);
    DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_D5(target) , vtss_state->ewis_conf[port_no].tx_oh.tx_dcc_l[1], VTSS_M_EWIS_TX_BYTE_TX_D5_TX_D5);
    DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_D6(target) , vtss_state->ewis_conf[port_no].tx_oh.tx_dcc_l[2], VTSS_M_EWIS_TX_BYTE_TX_D6_TX_D6);
    DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_D7(target) , vtss_state->ewis_conf[port_no].tx_oh.tx_dcc_l[3], VTSS_M_EWIS_TX_BYTE_TX_D7_TX_D7);
    DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_D8(target) , vtss_state->ewis_conf[port_no].tx_oh.tx_dcc_l[4], VTSS_M_EWIS_TX_BYTE_TX_D8_TX_D8);
    DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_D9(target) , vtss_state->ewis_conf[port_no].tx_oh.tx_dcc_l[5], VTSS_M_EWIS_TX_BYTE_TX_D9_TX_D9);
    DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_D10(target) , vtss_state->ewis_conf[port_no].tx_oh.tx_dcc_l[6], VTSS_M_EWIS_TX_BYTE_TX_D10_TX_D10);
    DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_D11(target) , vtss_state->ewis_conf[port_no].tx_oh.tx_dcc_l[7], VTSS_M_EWIS_TX_BYTE_TX_D11_TX_D11);
    DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_D12(target) , vtss_state->ewis_conf[port_no].tx_oh.tx_dcc_l[8], VTSS_M_EWIS_TX_BYTE_TX_D12_TX_D12);

    /**< Orderwire */
    DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_E2(target) , vtss_state->ewis_conf[port_no].tx_oh.tx_e2, VTSS_M_EWIS_TX_BYTE_TX_E2_TX_E2);

    /**< Automatic protection switch (APS) channel and Line Remote Defect Identifier (RDI-L) */
    DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_K1(target) , (vtss_state->ewis_conf[port_no].tx_oh.tx_k1_k2 & 0xff00) >> 8, VTSS_M_EWIS_TX_BYTE_TX_K1_TX_K1);
    DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_K2(target) , (vtss_state->ewis_conf[port_no].tx_oh.tx_k1_k2 & 0x00ff) , VTSS_M_EWIS_TX_BYTE_TX_K2_TX_K2);

    /**< Synchronization messaging */
    DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_S1(target), vtss_state->ewis_conf[port_no].tx_oh.tx_s1, VTSS_M_EWIS_TX_BYTE_TX_S1_TX_S1);

    /**< Reserved for Line growth  */
    DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_Z1(target) , (vtss_state->ewis_conf[port_no].tx_oh.tx_z1_z2 & 0xff00) >> 8, VTSS_M_EWIS_TX_BYTE_TX_Z1_TX_Z1);
    DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_Z2(target) , (vtss_state->ewis_conf[port_no].tx_oh.tx_z1_z2 & 0x00ff) , VTSS_M_EWIS_TX_BYTE_TX_Z2_TX_Z2);

    /**< path overhead: */
    /**< Transmitted C2 path label */
    DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_C2(target), vtss_state->ewis_conf[port_no].tx_oh.tx_c2, VTSS_M_EWIS_TX_BYTE_TX_C2_TX_C2);
    DAYTONA_WRM(VTSS_EWIS_RX_WIS_CTRL_C2_CTRL(target), vtss_state->ewis_conf[port_no].tx_oh.tx_c2, VTSS_M_EWIS_RX_WIS_CTRL_C2_CTRL_RX_EXPECTED_C2);

    /**< Path User Channel  */
    DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_F2(target), vtss_state->ewis_conf[port_no].tx_oh.tx_f2, VTSS_M_EWIS_TX_BYTE_TX_F2_TX_F2);

    /**< Tandem connection maintenance/Path data channel */
    DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_N1(target) , vtss_state->ewis_conf[port_no].tx_oh.tx_n1, VTSS_M_EWIS_TX_BYTE_TX_N1_TX_N1);

    /**< Reserved for Path growth  */
    DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_Z3(target) , (vtss_state->ewis_conf[port_no].tx_oh.tx_z3_z4 & 0xff00) >> 8, VTSS_M_EWIS_TX_BYTE_TX_Z3_TX_Z3);
    DAYTONA_WRM(VTSS_EWIS_TX_BYTE_TX_Z4(target) , (vtss_state->ewis_conf[port_no].tx_oh.tx_z3_z4 & 0x00ff) , VTSS_M_EWIS_TX_BYTE_TX_Z4_TX_Z4);

    return VTSS_RC_OK;
}

static vtss_rc daytona_ewis_tx_oh_passthru_set(vtss_state_t *vtss_state,
                                               const vtss_port_no_t port_no)
{
    u32 target;
    vtss_ewis_tx_oh_passthru_t passthru;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EWIS, &target));
    passthru = vtss_state->ewis_conf[port_no].tx_oh_passthru;

    DAYTONA_WRM(VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU(target) ,
                VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_J0_PASSTHRU((passthru.tx_j0 == FALSE) ? 0 : 1),
                VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_J0_PASSTHRU);

    DAYTONA_WRM(VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU(target) ,
                VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_Z0_PASSTHRU((passthru.tx_z0 == FALSE) ? 0 : 1),
                VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_Z0_PASSTHRU);
   
    DAYTONA_WRM(VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU(target) ,
                VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_B1_PASSTHRU((passthru.tx_b1 == FALSE) ? 0 : 1),
                VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_B1_PASSTHRU);

    DAYTONA_WRM(VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU(target) ,
                VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_E1_PASSTHRU((passthru.tx_e1 == FALSE) ? 0 : 1),
                VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_E1_PASSTHRU);

    DAYTONA_WRM(VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU(target) ,
                VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_F1_PASSTHRU((passthru.tx_f1 == FALSE) ? 0 : 1),
                VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_F1_PASSTHRU);

    DAYTONA_WRM(VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU(target) ,
                VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_SDCC_PASSTHRU((passthru.tx_dcc_s == FALSE) ? 0 : 1),
                VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_SDCC_PASSTHRU);

    DAYTONA_WRM(VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU(target) ,
                VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_SOH_PASSTHRU((passthru.tx_soh == FALSE) ? 0 : 1),
                VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_SOH_PASSTHRU);

    DAYTONA_WRM(VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU(target) ,
                VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_B2_PASSTHRU((passthru.tx_b2 == FALSE) ? 0 : 1),
                VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_B2_PASSTHRU);

    DAYTONA_WRM(VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU(target) ,
                VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_K1_PASSTHRU((passthru.tx_k1 == FALSE) ? 0 : 1),
                VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_K1_PASSTHRU);

    DAYTONA_WRM(VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU(target) ,
                VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_K2_PASSTHRU((passthru.tx_k2 == FALSE) ? 0 : 1),
                VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_K2_PASSTHRU);

    DAYTONA_WRM(VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU(target) ,
                VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_REIL_PASSTHRU((passthru.tx_reil == FALSE) ? 0 : 1),
                VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_REIL_PASSTHRU);

    DAYTONA_WRM(VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU(target) ,
                VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_LDCC_PASSTHRU((passthru.tx_dcc_l == FALSE) ? 0 : 1),
                VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_LDCC_PASSTHRU);

    DAYTONA_WRM(VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU(target) ,
                VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_S1_PASSTHRU((passthru.tx_s1 == FALSE) ? 0 : 1),
                VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_S1_PASSTHRU);

    DAYTONA_WRM(VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU(target) ,
                VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_E2_PASSTHRU((passthru.tx_e2 == FALSE) ? 0 : 1),
                VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_E2_PASSTHRU);

    DAYTONA_WRM(VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU(target) ,
                VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_Z1Z2_PASSTHRU((passthru.tx_z1_z2 == FALSE) ? 0 : 1),
                VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_Z1Z2_PASSTHRU);

    DAYTONA_WRM(VTSS_EWIS_TX_WIS_CTRL_TX_PASSTHRU(target) ,
                VTSS_F_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_LOH_PASSTHRU((passthru.tx_loh == FALSE) ? 0 : 1),
                VTSS_M_EWIS_TX_WIS_CTRL_TX_PASSTHRU_TX_LOH_PASSTHRU);

    return VTSS_RC_OK;

}

static vtss_rc daytona_ewis_exp_sl_set(vtss_state_t *vtss_state,
                                       const vtss_port_no_t port_no)
{
    u32 target;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EWIS, &target));
    DAYTONA_WRM(VTSS_EWIS_RX_WIS_CTRL_C2_CTRL(target), vtss_state->ewis_conf[port_no].exp_sl.exsl, VTSS_M_EWIS_RX_WIS_CTRL_C2_CTRL_RX_EXPECTED_C2);

    return VTSS_RC_OK;
}


/**
 * \brief Set path transmitted Trail Trace Identifier.
 * \param port_no [IN]   Port number.
 * \return Return code.
 **/
static vtss_rc daytona_ewis_path_txti_set(vtss_state_t *vtss_state,
                                          const vtss_port_no_t port_no)
{
    u32 target;
    vtss_ewis_tti_t txti;
    uint tti_size;
    u32 tti_trace_len;
    uint i;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EWIS, &target));
    txti = vtss_state->ewis_conf[port_no].path_txti;
    if (txti.mode == TTI_MODE_1) {
        tti_size = VTSS_DAYTONA_EWIS_TTI_MODE_1_LEN;
        tti_trace_len = 2;
    }
    else if (txti.mode == TTI_MODE_16) {
        tti_size = VTSS_DAYTONA_EWIS_TTI_MODE_16_LEN;
        tti_trace_len = 0;
    }
    else if (txti.mode == TTI_MODE_64) {
        tti_size = VTSS_DAYTONA_EWIS_TTI_MODE_64_LEN;
        tti_trace_len = 1;
    }
    else {
        VTSS_E("invalid tti mode");
        return VTSS_RC_ERROR;
    }
    /* configure trace length */
    DAYTONA_WRM(VTSS_EWIS_TX_WIS_CTRL_TX_MISC_CTRL(target),
                VTSS_F_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_J1_TRACE_LENGTH(tti_trace_len),
                VTSS_M_EWIS_TX_WIS_CTRL_TX_MISC_CTRL_TX_J1_TRACE_LENGTH);

    DAYTONA_WRM(VTSS_EWIS_RX_WIS_CTRL_MISC_POH_CTRL(target),
                VTSS_F_EWIS_RX_WIS_CTRL_MISC_POH_CTRL_RX_J1_64BYTE_TRACE(tti_trace_len),
                VTSS_M_EWIS_RX_WIS_CTRL_MISC_POH_CTRL_RX_J1_64BYTE_TRACE);

    /* write tti */
    for (i=0; i < tti_size; i++) {
        DAYTONA_WR(VTSS_EWIS_TX_J1_INS_TX_J1(target,i), txti.tti[i]);
    }

    return VTSS_RC_OK;
}

/**
 * \brief Get eWIS counters.
 * \param port_no [IN]   Port number.
 * \return Return code.
 **/
static vtss_rc daytona_ewis_counter_get(vtss_state_t *vtss_state,
                                        const vtss_port_no_t port_no,
                                        vtss_ewis_counter_t *const cntrs)
{
    u32 target;
    u32 value;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EWIS, &target));

    /**< Section BIP error count */
    DAYTONA_RD(VTSS_EWIS_RX_WIS_ERR_CNT_RX_B1_ERR_CNT(target), &value);
    cntrs->pn_ebc_s = VTSS_X_EWIS_RX_WIS_ERR_CNT_RX_B1_ERR_CNT_RX_B1_ERR_CNT(value);

    /**< Near end line block (BIP) error count */
    DAYTONA_RD(VTSS_EWIS_RX_WIS_ERR_CNT_RX_B2_ERR_CNT(target), &value);
    cntrs->pn_ebc_l = VTSS_X_EWIS_RX_WIS_ERR_CNT_RX_B2_ERR_CNT_RX_B2_ERR_CNT(value);

    /**< Far end line block (BIP) error count */
    DAYTONA_RD(VTSS_EWIS_RX_WIS_ERR_CNT_RX_REIL_ERR_CNT(target), &value);
    cntrs->pf_ebc_l = VTSS_X_EWIS_RX_WIS_ERR_CNT_RX_REIL_ERR_CNT_RX_REIL_ERR_CNT(value);

    /**< Path block error count */
    DAYTONA_RD(VTSS_EWIS_RX_WIS_ERR_CNT_RX_B3_ERR_CNT(target), &value);
    cntrs->pn_ebc_p = VTSS_X_EWIS_RX_WIS_ERR_CNT_RX_B3_ERR_CNT_RX_B3_ERR_CNT(value);

    /**< Far end path block error count */
    DAYTONA_RD(VTSS_EWIS_RX_WIS_ERR_CNT_RX_REIP_ERR_BLK_CNT(target), &value);
    cntrs->pf_ebc_p = VTSS_X_EWIS_RX_WIS_ERR_CNT_RX_REIP_ERR_BLK_CNT_RX_REIP_ERR_BLK_CNT(value);

    return VTSS_RC_OK;
}

/* ================================================================= *
 *  Test Config/status
 * ================================================================= */


/**
 * \brief Set wis test mode.
 * \param port_no [IN]   Port number.
 * \return Return code.
 *
 *
 * Daytona registers:
    VTSS_WIS_TEST_MODE_DISABLE,         Disable test
    VTSS_WIS_TEST_MODE_SQUARE_WAVE,     Enable squarevave generator, Only valid for test generator
    VTSS_WIS_TEST_MODE_PRBS31,          not supported
    VTSS_WIS_TEST_MODE_MIXED_FREQUENCY, Enable mixed frequency generator / analyzer

 * loopback;                RX_TOP_LEVEL::SYS_LOOP_ENA [DBG]
 * test_pattern_gen;        TX_TST_CTRL::TX_EWIS_TEST_PAT_SEL [DBG], TX_EWIS_TEST_PAT_EN [DBG]
 * test_pattern_ana;        RX_TST_CTRL::RX_MIXFREQ_CHKR_EN [DBG]
 * 
 **/
static vtss_rc daytona_ewis_test_mode_set(vtss_state_t *vtss_state,
                                          const vtss_port_no_t port_no)
{
    /* TBD :: Postpone for Later releases */
    return VTSS_RC_ERROR;
}

/**
 * \brief Set PRBS31 error inject configuration.
 * \param port_no [IN]   Port number.
 * \param inj [IN] Pointer to the error inject configuration.
 * \return Return code.
 *
 * Daytona registers: Not supported (as it is only relevant for prbs31.
 **/
static vtss_rc daytona_ewis_prbs31_err_inj_set (vtss_state_t *vtss_state,
                                                vtss_port_no_t port_no,
                                                const vtss_ewis_prbs31_err_inj_t *const inj)
{
    /* TBD :: Postpone for Later releases */
    return VTSS_RC_ERROR;
}


/**
 * \brief Get wis test counter.
 * \param port_no [IN]   Port number.
 * \return Return code.
 *
 * Daytona registers: Not supported (as it is only relevant for prbs31.
 **/
static vtss_rc daytona_ewis_test_counter_get(vtss_state_t *vtss_state,
                                             const vtss_port_no_t port_no, vtss_ewis_test_status_t *const test_status)
{
    /* TBD :: Postpone for Later releases */
    return VTSS_RC_ERROR;
}



/* ================================================================= *
 *  State Reporting
 * ================================================================= */
/**
 * \brief Get wis Status.
 * \param port_no [IN]   Port number.
 * \param status  [OUT]  wis status.
 * \return Return code.
 *
 **/
static vtss_rc daytona_ewis_status_get(vtss_state_t *vtss_state,
                                       const vtss_port_no_t port_no,
                                       vtss_ewis_status_t *const status)
{
    /* TBD :: Postpone for Later releases */
    return VTSS_RC_ERROR;
}

/**
 * \brief Get wis defects.
 * \param port_no [IN]   Port number.
 * \param def [OUT]      pointer to defect status structure.
 * \return Return code.
 *
 * Daytona registers:
 * dlos_s;        RX_WIS_STATUS::RX_STATUS_LOS
 * doof_s;        RX_WIS_STATUS::RX_STATUS_OOF_ST
 * dlof_s;        RX_WIS_STATUS::RX_STATUS_LOF
 * dais_l;        RX_WIS_STATUS::RX_STATUS_AIS_L
 * drdi_l;        RX_WIS_STATUS::RX_STATUS_RDI_L
 * dais_p;        RX_WIS_STATUS::RX_STATUS_AIS_P
 * dlop_p;        RX_WIS_STATUS::RX_STATUS_LOP
 * duneq_p;       RX_WIS_STATUS::RX_STATUS_UNEQ_P
 * drdi_p;        RX_WIS_STATUS::RX_STATUS_RDIP
 * dlcd_p;        not supported in 8492
 * dplm_p;        RX_WIS_STATUS::RX_STATUS_PLM_P
 * dfais_p;       RX_WIS_STATUS::RX_STATUS_AIS_P_F
 * dfplm_p;       RX_WIS_STATUS::RX_STATUS_PLM_P_F
 * dfuneq_p;      RX_WIS_STATUS::RX_STATUS_UNEQ_P_F
 **/
static vtss_rc daytona_ewis_defects_get(vtss_state_t *vtss_state,
                                        const               vtss_port_no_t port_no,
                                        vtss_ewis_defects_t  *const def)
{
    u32 target;
    u32 status;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EWIS, &target));

    memset(def, 0, sizeof(vtss_ewis_defects_t));

    DAYTONA_RD(VTSS_EWIS_RX_WIS_STATUS_RX_WIS_STATUS(target), &status);

    if (VTSS_X_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_LOS(status))           def->dlos_s = TRUE;
    if (VTSS_X_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_FRMR_OOF_ST(status))          def->doof_s = TRUE;
    if (VTSS_X_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_LOF(status))           def->dlof_s = TRUE;
    if (VTSS_X_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_RDI_L(status))         def->drdi_l = TRUE;
    if (VTSS_X_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_AIS_L(status))         def->dais_l = TRUE;
    if (VTSS_X_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_LOP(status))           def->dlop_p = TRUE;
    if (VTSS_X_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_AIS_P(status))         def->dais_p = TRUE;
    if (VTSS_X_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_PLM_P(status))         def->dplm_p = TRUE;
    if (VTSS_X_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_UNEQ_P(status))        def->duneq_p = TRUE;
    if (VTSS_X_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_RDIP(status))          def->drdi_p = TRUE;
    if (VTSS_X_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_AIS_P_F(status))       def->dfais_p = TRUE;
    if (VTSS_X_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_PLM_P_F(status))       def->dfplm_p = TRUE;
    if (VTSS_X_EWIS_RX_WIS_STATUS_RX_WIS_STATUS_RX_STATUS_UNEQ_P_F(status))      def->dfuneq_p = TRUE;

    return VTSS_RC_OK;
}

/**
 * \brief Get section received (accepted) Trail Trace Identifier.
 * \param port_no [IN]   Port number.
 * \param acti [OUT]     pointer to accepted tti.
 * \return Return code.
 *
 * Daytona registers:
 * RX_BYTE_J0                           up to 64 bytes trace identifier
 * MISC_SOH_CTRL::RX_J0_64BYTE_TRACE    size of trace identifier
 *
 **/
static vtss_rc daytona_ewis_section_acti_get(vtss_state_t *vtss_state,
                                             const vtss_port_no_t port_no,
                                   vtss_ewis_tti_t *const acti)
{
    u32 target;
    u32 value; 
    vtss_ewis_tti_t txti;
    i16 reg_ct = 0,
        reg_len = 0;
    i16 idx_ct = 0,
        arr_idx = 0,
        zero_based_idx = 0;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EWIS, &target));
    memset(&txti, 0, sizeof(vtss_ewis_tti_t));
    memset(acti, 0, sizeof(vtss_ewis_tti_t));
    txti = vtss_state->ewis_conf[port_no].section_txti;
    switch (txti.mode) {
        case TTI_MODE_1:
            reg_len = VTSS_DAYTONA_EWIS_TTI_MODE_1_LEN;
            break;
        case TTI_MODE_16:
            reg_len = VTSS_DAYTONA_EWIS_TTI_MODE_16_LEN;
            break;
        case TTI_MODE_64:
            reg_len = VTSS_DAYTONA_EWIS_TTI_MODE_64_LEN;
            break;
        default :
            break;
    }
    /* Extract the Accepted Bytes from the registers */
    for(reg_ct = 0; reg_ct < reg_len; reg_ct++) {
        DAYTONA_RD(VTSS_EWIS_RX_J0_STATUS_RX_BYTE_J0(target, reg_ct), &value);
        txti.tti[reg_ct] = (u8) VTSS_X_EWIS_RX_J0_STATUS_RX_BYTE_J0_RX_BYTE_J0(value);
    }
    /* Update the acti based on the TTI_MODE */
    /* Update the acti based on the TTI_MODE */
    acti->mode = txti.mode;
    acti->valid = FALSE;
    txti.valid = FALSE;
    switch (txti.mode) {
        case TTI_MODE_1:
            /* Update Only 1 Byte */
            acti->tti[0] = txti.tti[0];
            acti->valid = TRUE;
            break;
        case TTI_MODE_16:
            zero_based_idx = (VTSS_DAYTONA_EWIS_TTI_MODE_16_LEN - 1);
            for (reg_ct = 0; reg_ct < reg_len; reg_ct++) {
                if (txti.tti[reg_ct] & VTSS_DAYTONA_EWIS_TTI_MODE_16_MATCH_BYTE) {
                    txti.tti[reg_ct] ='\0';
                    acti->valid = TRUE;
                    break;
                }
            }
            if (acti->valid == FALSE) {
                break;
            }
            for (idx_ct = reg_ct; idx_ct >= 0; idx_ct--) {
                if (txti.tti[idx_ct] & 0x80) {
                    acti->valid = FALSE;
                    break;
                }
                acti->tti[zero_based_idx - (reg_ct - idx_ct)] = txti.tti[idx_ct];
            }
            for (idx_ct = zero_based_idx; idx_ct > reg_ct; idx_ct-- ) {
                if (txti.tti[idx_ct] & 0x80) {
                    acti->valid = FALSE;
                    break;
                }
                acti->tti[idx_ct - (reg_ct + 1)] = txti.tti[idx_ct];
            }
            break;       
        case TTI_MODE_64:
            zero_based_idx = (VTSS_DAYTONA_EWIS_TTI_MODE_64_LEN - 1);
            for (reg_ct = 0; reg_ct < reg_len; reg_ct++) {
                if ((txti.tti[reg_ct] == VTSS_DAYTONA_EWIS_TTI_MODE_64_MATCH_BYTE_0) &&
                    (txti.tti[reg_ct + 1] == VTSS_DAYTONA_EWIS_TTI_MODE_64_MATCH_BYTE_1)) {
                    txti.valid = TRUE;
                    break;
                }
            }
            if (txti.valid == FALSE) {
                break;
            }
            reg_ct++;
            /* Since the TTi contains \n\r increase 2 bytes */
            for (idx_ct = (reg_ct + 1), arr_idx = zero_based_idx; idx_ct <= zero_based_idx; idx_ct++, arr_idx--) {
                if (txti.tti[idx_ct] & 0x80) {
                    acti->valid = FALSE;
                    break;
                }
                acti->tti[arr_idx] = txti.tti[idx_ct];
            }
            for (idx_ct = 0, arr_idx = reg_ct; idx_ct <= reg_ct; idx_ct++, arr_idx-- ) {
                if (txti.tti[idx_ct] & 0x80) {
                    acti->valid = FALSE;
                    break;
                }
                acti->tti[arr_idx] = txti.tti[idx_ct];
            }
                 
            /* Reverse the Bytes */
            for (idx_ct = 0; idx_ct < VTSS_DAYTONA_EWIS_TTI_MODE_64_LEN; idx_ct++) {
                txti.tti[(zero_based_idx - idx_ct)] = acti->tti[idx_ct];
            }
            *acti = txti;
            break;
        default :
            break;
    }

    return VTSS_RC_OK;
}


/**
 * \brief Get path received (accepted) Trail Trace Identifier.
 * \param port_no [IN]   Port number.
 * \param acti [OUT]     pointer to accepted tti.
 * \return Return code.
 *
 * Daytona registers:
 * RX_BYTE_J1                           up to 64 bytes trace identifier
 * MISC_POH_CTRL::RX_J1_64BYTE_TRACE    size of trace identifier
 *
 **/
static vtss_rc daytona_ewis_path_acti_get(vtss_state_t *vtss_state,
                                          const vtss_port_no_t port_no,
                                vtss_ewis_tti_t *const acti)
{
    u32 target;
    u32 value;
    vtss_ewis_tti_t txti;
    i16 reg_ct = 0, reg_len = 0;
    i16 idx_ct = 0,
        arr_idx = 0,
        zero_based_idx = 0;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EWIS, &target));
    memset(&txti, 0, sizeof(vtss_ewis_tti_t));
    memset(acti, 0, sizeof(vtss_ewis_tti_t));
    txti = vtss_state->ewis_conf[port_no].path_txti;
    switch (txti.mode) {
        case TTI_MODE_1:
            reg_len = VTSS_DAYTONA_EWIS_TTI_MODE_1_LEN;
            break;
        case TTI_MODE_16:
            reg_len = VTSS_DAYTONA_EWIS_TTI_MODE_16_LEN;
            break;
        case TTI_MODE_64:
            reg_len = VTSS_DAYTONA_EWIS_TTI_MODE_64_LEN;
            break;
        default :
            break;
    }
    /* Extract the Accepted Bytes from the registers */
    for(reg_ct = 0; reg_ct < reg_len; reg_ct++) {
        DAYTONA_RD(VTSS_EWIS_RX_J1_STATUS_RX_BYTE_J1(target, reg_ct), &value);
        txti.tti[reg_ct] = (u8) VTSS_X_EWIS_RX_J1_STATUS_RX_BYTE_J1_RX_BYTE_J1(value);
    }
    /* Update the acti based on the TTI_MODE */
    /* Update the acti based on the TTI_MODE */
    acti->mode = txti.mode;
    acti->valid = FALSE;
    txti.valid = FALSE;
    switch (txti.mode) {
        case TTI_MODE_1:
            /* Update Only 1 Byte */
            acti->tti[0] = txti.tti[0];
            acti->valid = TRUE;
            break;
        case TTI_MODE_16:
            zero_based_idx = (VTSS_DAYTONA_EWIS_TTI_MODE_16_LEN - 1);
            for (reg_ct = 0; reg_ct < reg_len; reg_ct++) {
                if (txti.tti[reg_ct] & VTSS_DAYTONA_EWIS_TTI_MODE_16_MATCH_BYTE) {
                    txti.tti[reg_ct] ='\0';
                    acti->valid = TRUE;
                    break;
                }
            }
            if (acti->valid == FALSE) {
                break;
            }
            for (idx_ct = reg_ct; idx_ct >= 0; idx_ct--) {
                if (txti.tti[idx_ct] & 0x80) {
                    acti->valid = FALSE;
                    break;
                }
                acti->tti[zero_based_idx - (reg_ct - idx_ct)] = txti.tti[idx_ct];
            }
            for (idx_ct = zero_based_idx; idx_ct > reg_ct; idx_ct-- ) {
                if (txti.tti[idx_ct] & 0x80) {
                    acti->valid = FALSE;
                    break;
                }
                acti->tti[idx_ct - (reg_ct + 1)] = txti.tti[idx_ct];
            }
            break;       
        case TTI_MODE_64:
            zero_based_idx = (VTSS_DAYTONA_EWIS_TTI_MODE_64_LEN - 1);
            for (reg_ct = 0; reg_ct < reg_len; reg_ct++) {
                if ((txti.tti[reg_ct] == VTSS_DAYTONA_EWIS_TTI_MODE_64_MATCH_BYTE_0) &&
                    (txti.tti[reg_ct + 1] == VTSS_DAYTONA_EWIS_TTI_MODE_64_MATCH_BYTE_1)) {
                    txti.valid = TRUE;
                    break;
                }
            }
            if (txti.valid == FALSE) {
                break;
            }
            reg_ct++;
            /* Since the TTi contains \n\r increase 2 bytes */
            for (idx_ct = (reg_ct + 1), arr_idx = zero_based_idx; idx_ct <= zero_based_idx; idx_ct++, arr_idx--) {
                if (txti.tti[idx_ct] & 0x80) {
                    acti->valid = FALSE;
                    break;
                }
                acti->tti[arr_idx] = txti.tti[idx_ct];
            }
            for (idx_ct = 0, arr_idx = reg_ct; idx_ct <= reg_ct; idx_ct++, arr_idx-- ) {
                if (txti.tti[idx_ct] & 0x80) {
                    acti->valid = FALSE;
                    break;
                }
                acti->tti[arr_idx] = txti.tti[idx_ct];
            }
                 
            /* Reverse the Bytes */
            for (idx_ct = 0; idx_ct < VTSS_DAYTONA_EWIS_TTI_MODE_64_LEN; idx_ct++) {
                txti.tti[(zero_based_idx - idx_ct)] = acti->tti[idx_ct];
            }
            *acti = txti;
            break;
        default :
            break;
    }

    return VTSS_RC_OK;
}


/* ================================================================= *
 *  Performance Primitives
 * ================================================================= */

/**
 * \brief Get wis performance primitives.
 * \param port_no [IN]   Port number.
 * \param perf [OUT]     pointer to performance primitive structure.
 * \return Return code.
 *
 * Daytona registers:
 * psef;        not implemented in Daytona HW , may be implemented in the SW performance process
 * pn_ebc_s;    RX_B1_ERR_CNT_PERF
 * pn_ds_s;     not implemented in Daytona HW , may be implemented in the SW performance process
 * pn_ebc_l;    RX_B2_ERR_CNT_PERF
 * pn_ds_l;     not implemented in Daytona HW , may be implemented in the SW performance process
 * pf_ebc_l;    RX_REIL_ERR_CNT_PERF
 * pf_ds_l;     not implemented in Daytona HW , may be implemented in the SW performance process
 * pn_ebc_p;    RX_B3_ERR_CNT_PERF
 * pn_ds_p;     not implemented in Daytona HW , may be implemented in the SW performance process
 * pf_ebc_p;    RX_REIP_ERR_CNT_PERF
 * pf_ds_p;     not implemented in Daytona HW , may be implemented in the SW performance process
 *
 **/
static vtss_rc daytona_ewis_perf_get(vtss_state_t *vtss_state,
                                     const vtss_port_no_t port_no,
                           vtss_ewis_perf_t *const perf)
{

    u32 target;
    u32 value;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EWIS, &target));

    /**< Section BIP Performance error count */
    DAYTONA_RD(VTSS_EWIS_RX_WIS_CNT_PERF_RX_B1_ERR_CNT_PERF(target), &value);
    perf->pn_ebc_s = value;

    /**< Near end Line block (BIP) performance error count */
    DAYTONA_RD(VTSS_EWIS_RX_WIS_CNT_PERF_RX_B2_ERR_CNT_PERF(target), &value);
    perf->pn_ebc_l = value;

    /**< Far end Line block (BIP) performance error count */
    DAYTONA_RD(VTSS_EWIS_RX_WIS_CNT_PERF_RX_REIL_ERR_CNT_PERF(target), &value);
    perf->pf_ebc_l = value;

    /**< Near end Path block (BIP) performance error count */
    DAYTONA_RD(VTSS_EWIS_RX_WIS_CNT_PERF_RX_B3_ERR_CNT_PERF(target), &value);
    perf->pn_ebc_p = value;

    /**< Far end Path block (BIP) performance error count */
    DAYTONA_RD(VTSS_EWIS_RX_WIS_CNT_PERF_RX_REIP_ERR_CNT_PERF(target), &value);
    perf->pf_ebc_p = value;

    return VTSS_RC_OK;
}


static vtss_rc daytona_ewis_perf_mode_set(vtss_state_t *vtss_state,
                                          const vtss_port_no_t port_no)
{
    u32 target;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EWIS, &target));
    DAYTONA_WRM(VTSS_EWIS_RX_WIS_CTRL_MISC_SOH_CTRL(target), VTSS_F_EWIS_RX_WIS_CTRL_MISC_SOH_CTRL_B1_PERF_MODE(vtss_state->ewis_conf[port_no].perf_mode.pn_ebc_mode_s), VTSS_M_EWIS_RX_WIS_CTRL_MISC_SOH_CTRL_B1_PERF_MODE_ENA);
    DAYTONA_WRM(VTSS_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL(target), VTSS_F_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_B2_PERF_MODE(vtss_state->ewis_conf[port_no].perf_mode.pn_ebc_mode_l), VTSS_M_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_B2_PERF_MODE_ENA);
    DAYTONA_WRM(VTSS_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL(target), VTSS_F_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_REIL_PERF_MODE(vtss_state->ewis_conf[port_no].perf_mode.pf_ebc_mode_l), VTSS_M_EWIS_RX_WIS_CTRL_MISC_LOH_CTRL_REIL_PERF_MODE_ENA);
    DAYTONA_WRM(VTSS_EWIS_RX_WIS_CTRL_MISC_POH_CTRL(target), VTSS_F_EWIS_RX_WIS_CTRL_MISC_POH_CTRL_B3_PERF_MODE(vtss_state->ewis_conf[port_no].perf_mode.pn_ebc_mode_p), VTSS_M_EWIS_RX_WIS_CTRL_MISC_POH_CTRL_B3_PERF_MODE_ENA);
    DAYTONA_WRM(VTSS_EWIS_RX_WIS_CTRL_MISC_POH_CTRL(target), VTSS_F_EWIS_RX_WIS_CTRL_MISC_POH_CTRL_REIP_PERF_MODE(vtss_state->ewis_conf[port_no].perf_mode.pf_ebc_mode_p), VTSS_M_EWIS_RX_WIS_CTRL_MISC_POH_CTRL_REIP_PERF_MODE_ENA);

    return VTSS_RC_OK;
}

/* ================================================================= *
 *  Defects/Events
 * ================================================================= */
static vtss_rc daytona_ewis_event_enable(vtss_state_t *vtss_state,
                                         const vtss_port_no_t      port_no,
                                         const BOOL                enable,
                                         const vtss_ewis_event_t   ev_mask)
{
    u32 target;
    u32 mask;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EWIS, &target));

    mask = 0;

    if (ev_mask & VTSS_EWIS_LOS_EV)        mask |= VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_LOS_MASK;
    if (ev_mask & VTSS_EWIS_OOF_EV)        mask |= VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_OOF_MASK;
    if (ev_mask & VTSS_EWIS_LOF_EV)        mask |= VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_LOF_MASK;
    if (ev_mask & VTSS_EWIS_RDIL_EV)       mask |= VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_RDIL_MASK;
    if (ev_mask & VTSS_EWIS_AISL_EV)       mask |= VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_AISL_MASK;
    if (ev_mask & VTSS_EWIS_S1_EV)         mask |= VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_S1_MASK;
    if (ev_mask & VTSS_EWIS_LOP_EV)        mask |= VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_LOP_MASK;
    if (ev_mask & VTSS_EWIS_AISP_EV)       mask |= VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_AISP_MASK;
    if (ev_mask & VTSS_EWIS_PLMP_EV)       mask |= VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_PLMP_MASK;
    if (ev_mask & VTSS_EWIS_UNEQP_EV)      mask |= VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_UNEQP_MASK;
    if (ev_mask & VTSS_EWIS_RDIP_EV)       mask |= VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_RDIP_MASK;
    if (ev_mask & VTSS_EWIS_FE_AIS_P_EV)   mask |= VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_FE_AIS_P_MASK;
    if (ev_mask & VTSS_EWIS_FEPLMP_EV)     mask |= VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_FEPLMP_MASK;
    if (ev_mask & VTSS_EWIS_FE_UNEQP_EV)   mask |= VTSS_M_EWIS_RX_MASK_RX_STATUS_MASK_INTR_FE_UNEQP_MASK;

    DAYTONA_WRM(VTSS_EWIS_RX_MASK_RX_STATUS_MASK(target), (enable ? mask : 0), mask);

    return VTSS_RC_OK;
}


static vtss_rc daytona_ewis_event_poll(vtss_state_t *vtss_state,
                                       const vtss_port_no_t  port_no,
                                       vtss_ewis_event_t     *const status)
{
    u32 target;
    u32 sticky, mask;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EWIS, &target));

    DAYTONA_RD(VTSS_EWIS_RX_INTRPT_RX_STATUS_INTRPT(target), &sticky);
    DAYTONA_RD(VTSS_EWIS_RX_MASK_RX_STATUS_MASK(target), &mask);
    sticky &= mask;
    DAYTONA_WR(VTSS_EWIS_RX_INTRPT_RX_STATUS_INTRPT(target), sticky);

    *status = 0;

    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_LOS_PEND_STICKY)         *status |= VTSS_EWIS_LOS_EV;
    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_OOF_PEND_STICKY)         *status |= VTSS_EWIS_OOF_EV;
    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_LOF_PEND_STICKY)         *status |= VTSS_EWIS_LOF_EV;
    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_RDIL_PEND_STICKY)        *status |= VTSS_EWIS_RDIL_EV;
    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_AISL_PEND_STICKY)        *status |= VTSS_EWIS_AISL_EV;
    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_S1_PEND_STICKY)          *status |= VTSS_EWIS_S1_EV;
    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_LOP_PEND_STICKY)         *status |= VTSS_EWIS_LOP_EV;
    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_AISP_PEND_STICKY)        *status |= VTSS_EWIS_AISP_EV;
    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_PLMP_PEND_STICKY)        *status |= VTSS_EWIS_PLMP_EV;
    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_UNEQP_PEND_STICKY)       *status |= VTSS_EWIS_UNEQP_EV;
    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_RDIP_PEND_STICKY)        *status |= VTSS_EWIS_RDIP_EV;
    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_FE_AIS_P_PEND_STICKY)    *status |= VTSS_EWIS_FE_AIS_P_EV;
    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_FEPLMP_PEND_STICKY)      *status |= VTSS_EWIS_FEPLMP_EV;
    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_FE_UNEQP_PEND_STICKY)    *status |= VTSS_EWIS_FE_UNEQP_EV;

    return VTSS_RC_OK;
}

static vtss_rc daytona_ewis_event_poll_without_mask(vtss_state_t *vtss_state,
                                                    const vtss_port_no_t  port_no,
                                       vtss_ewis_event_t     *const status)
{
    u32 target;
    u32 sticky;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EWIS, &target));

    DAYTONA_RD(VTSS_EWIS_RX_INTRPT_RX_STATUS_INTRPT(target), &sticky);
    DAYTONA_WR(VTSS_EWIS_RX_INTRPT_RX_STATUS_INTRPT(target), sticky);

    *status = 0;

    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_LOS_PEND_STICKY)         *status |= VTSS_EWIS_LOS_EV;
    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_OOF_PEND_STICKY)         *status |= VTSS_EWIS_OOF_EV;
    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_LOF_PEND_STICKY)         *status |= VTSS_EWIS_LOF_EV;
    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_RDIL_PEND_STICKY)        *status |= VTSS_EWIS_RDIL_EV;
    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_AISL_PEND_STICKY)        *status |= VTSS_EWIS_AISL_EV;
    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_S1_PEND_STICKY)          *status |= VTSS_EWIS_S1_EV;
    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_LOP_PEND_STICKY)         *status |= VTSS_EWIS_LOP_EV;
    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_AISP_PEND_STICKY)        *status |= VTSS_EWIS_AISP_EV;
    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_PLMP_PEND_STICKY)        *status |= VTSS_EWIS_PLMP_EV;
    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_UNEQP_PEND_STICKY)       *status |= VTSS_EWIS_UNEQP_EV;
    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_RDIP_PEND_STICKY)        *status |= VTSS_EWIS_RDIP_EV;
    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_FE_AIS_P_PEND_STICKY)    *status |= VTSS_EWIS_FE_AIS_P_EV;
    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_FEPLMP_PEND_STICKY)      *status |= VTSS_EWIS_FEPLMP_EV;
    if (sticky & VTSS_M_EWIS_RX_INTRPT_RX_STATUS_INTRPT_INTR_FE_UNEQP_PEND_STICKY)    *status |= VTSS_EWIS_FE_UNEQP_EV;

    return VTSS_RC_OK;
}


/* ================================================================= *
 *  Utilities and internal
 * ================================================================= */
vtss_rc vtss_daytona_wis_restart_conf_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    vtss_rc rc = VTSS_RC_OK;

    VTSS_RC(daytona_ewis_cons_action_set(vtss_state, port_no));
    VTSS_RC(daytona_ewis_section_txti_set(vtss_state, port_no));
    VTSS_RC(daytona_ewis_path_txti_set(vtss_state, port_no));
    //VTSS_RC(daytona_ewis_test_mode_set(port_no));/* TBD :: Postpone for Later releases */
    VTSS_RC(daytona_ewis_tx_oh_set(vtss_state, port_no));
    VTSS_RC(daytona_ewis_tx_oh_passthru_set(vtss_state, port_no));
    VTSS_RC(daytona_ewis_exp_sl_set(vtss_state, port_no));
    VTSS_D("port_no %d, restart conf set, rc = %x", port_no, rc);
    return VTSS_RC_OK;
}


static vtss_ewis_cons_act_t cons_act_default = {
    .aisl = {
        .ais_on_los       = TRUE,
        .ais_on_lof       = TRUE,
    },
    .rdil = {
        .rdil_on_los      = FALSE,
        .rdil_on_lof      = FALSE,
        .rdil_on_lopc     = FALSE,
        .rdil_on_ais_l    = FALSE,
    },
    .fault = {
        .fault_on_feplmp = FALSE,
        .fault_on_feaisp = FALSE,
        .fault_on_rdil   = FALSE,
        .fault_on_sef    = FALSE,
        .fault_on_lof    = FALSE,
        .fault_on_los    = FALSE,
        .fault_on_aisl   = FALSE,
        .fault_on_lcdp   = FALSE,
        .fault_on_plmp   = FALSE,
        .fault_on_aisp   = FALSE,
        .fault_on_lopp   = FALSE,
    },
};

static vtss_ewis_tti_t txti_default = {
    .mode = TTI_MODE_16,
    .tti = {
       [15] = 0x89,
    },

};

static  vtss_ewis_line_force_mode_t line_force_default = {
    .force_ais = FALSE,
    .force_rdi = FALSE,
};

static vtss_ewis_line_tx_force_mode_t  line_tx_force_default = {
    .force_ais = FALSE,
    .force_rdi = FALSE,
};

static vtss_ewis_path_force_mode_t path_force_default = {
    .force_uneq = FALSE,
    .force_rdi  = FALSE,
};

static vtss_ewis_tx_oh_passthru_t  tx_oh_passthru_default = {
    /**< Section Overhead: */
    .tx_j0     = FALSE,
    .tx_z0     = FALSE,
    .tx_b1     = FALSE,
    .tx_e1     = FALSE,
    .tx_f1     = FALSE,
    .tx_dcc_s  = FALSE,
    .tx_soh    = FALSE,

    /**< line overhead: */
    .tx_b2     = FALSE,
    .tx_k1     = FALSE,
    .tx_k2     = FALSE,
    .tx_reil   = FALSE,
    .tx_dcc_l  = FALSE,
    .tx_s1     = FALSE,
    .tx_e2     = FALSE,
    .tx_z1_z2  = FALSE,
    .tx_loh    = FALSE,
};

static vtss_ewis_tx_oh_t tx_oh_default = {
    /**< Section Overhead: */
    .tx_dcc_s = {
        [0] = 0x00,
        [1] = 0x00,
        [2] = 0x00,
    },
    .tx_e1 = 0x00,
    .tx_f1 = 0x00,
    .tx_z0 = 0xCC,
    /**< line overhead: */
    .tx_dcc_l = {
        [0] = 0x00,
        [1] = 0x00,
        [2] = 0x00,
        [3] = 0x00,
        [4] = 0x00,
        [5] = 0x00,
        [6] = 0x00,
        [7] = 0x00,
        [8] = 0x00,
    },
    .tx_e2 = 0x00,
    .tx_k1_k2 = 0x0000,
    .tx_s1 = 0x00,
    .tx_z1_z2 = 0x0000,
    /**< path overhead: */
    .tx_c2 = 0x1a,
    .tx_f2 = 0x00,
    .tx_n1 = 0x00,
    .tx_z3_z4 = 0x0000,
} ;

static vtss_ewis_test_conf_t test_conf_default = {
    .loopback         = FALSE,
    .test_pattern_gen = VTSS_WIS_TEST_MODE_DISABLE,
    .test_pattern_ana = VTSS_WIS_TEST_MODE_DISABLE,
};

static vtss_ewis_counter_threshold_t counter_threshold_default = {
    .n_ebc_thr_s  = 0x00,
    .n_ebc_thr_l  = 0x00,
    .f_ebc_thr_l  = 0x00,
    .n_ebc_thr_p  = 0x00,
    .f_ebc_thr_p  = 0x00,
};

/**
 * \brief Set default values
 * 
 * \param port_no [IN]   Port number
 *
 * \param Return code.
 **/
vtss_rc vtss_daytona_wis_default_conf_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{  
    int cnt;

    vtss_state->ewis_conf[port_no].ewis_mode  = VTSS_WIS_OPERMODE_WIS_MODE;
    vtss_state->ewis_conf[port_no].section_cons_act = cons_act_default;
    for (cnt = 0; cnt < 64; cnt++)
        vtss_state->ewis_conf[port_no].section_txti.tti[cnt] = 0;
    vtss_state->ewis_conf[port_no].section_txti     = txti_default;
    vtss_state->ewis_conf[port_no].force_mode.line_rx_force  = line_force_default;
    vtss_state->ewis_conf[port_no].force_mode.line_tx_force  = line_tx_force_default;
    vtss_state->ewis_conf[port_no].force_mode.path_force     = path_force_default;
    for (cnt = 0; cnt < 64; cnt++)
        vtss_state->ewis_conf[port_no].path_txti.tti[cnt] = 0;
    vtss_state->ewis_conf[port_no].path_txti        = txti_default;
    /* TBD :: Update the correct tx_oh */
    vtss_state->ewis_conf[port_no].tx_oh            = tx_oh_default;
    vtss_state->ewis_conf[port_no].tx_oh_passthru   = tx_oh_passthru_default;
    vtss_state->ewis_conf[port_no].exp_sl.exsl      = VTSS_EWIS_RX_WIS_CTRL_C2_CTRL_RX_EXPECTED_C2_WAN;
    vtss_state->ewis_conf[port_no].test_conf        = test_conf_default;
    /* Not supported for Daytona*/
    vtss_state->ewis_conf[port_no].ewis_cntr_thresh_conf = counter_threshold_default;
    vtss_state->ewis_conf[port_no].ewis_init_done = TRUE;

    return VTSS_RC_OK;
}


/**
 * \brief Create instance (set up function pointers).
 *
 * \param inst [IN]      Target instance reference.
 * \param port_map [IN]  Port map array.
 *
 * \return Return code.
 **/
vtss_rc vtss_daytona_inst_wis_create(vtss_state_t *vtss_state)
{
    vtss_port_no_t port_no;
    vtss_cil_func_t *func = &vtss_state->cil;

    /* Daytona eWIS functions */
    func->ewis_events_conf_set              = daytona_ewis_event_enable;
    func->ewis_events_poll                  = daytona_ewis_event_poll;
    func->ewis_events_poll_without_mask     = daytona_ewis_event_poll_without_mask;
    func->ewis_events_force                 = NULL;
    func->ewis_force_conf_set               = daytona_ewis_force_conf_set;
    func->ewis_tx_oh_set                    = daytona_ewis_tx_oh_set;
    func->ewis_tx_oh_passthru_set           = daytona_ewis_tx_oh_passthru_set;
    func->ewis_exp_sl_set                   = daytona_ewis_exp_sl_set;
    func->ewis_mode_conf_set                = daytona_ewis_mode_set;
    func->ewis_reset                        = NULL;
    func->ewis_cons_action_set              = daytona_ewis_cons_action_set;
    func->ewis_section_txti_set             = daytona_ewis_section_txti_set;
    func->ewis_path_txti_set                = daytona_ewis_path_txti_set;
    func->ewis_test_mode_set                = daytona_ewis_test_mode_set;
    func->ewis_prbs31_err_inj_set           = daytona_ewis_prbs31_err_inj_set;
    func->ewis_test_status_get              = daytona_ewis_test_counter_get;
    func->ewis_defects_get                  = daytona_ewis_defects_get;
    func->ewis_status_get                   = daytona_ewis_status_get;
    func->ewis_section_acti_get             = daytona_ewis_section_acti_get;
    func->ewis_path_acti_get                = daytona_ewis_path_acti_get;
    func->ewis_perf_get                     = daytona_ewis_perf_get;
    func->ewis_counter_get                  = daytona_ewis_counter_get;
    func->ewis_counter_threshold_set        = NULL;
    func->ewis_perf_mode_set                = daytona_ewis_perf_mode_set; 
//    func->ewis_nim_set              = daytona_ewis_nim_set;


    for (port_no = 0 ; port_no < VTSS_PORT_ARRAY_SIZE; port_no++) {
        vtss_state->ewis_conf[port_no].ewis_mode  = VTSS_WIS_OPERMODE_WIS_MODE;
		vtss_state->ewis_conf[port_no].section_cons_act = cons_act_default;
        vtss_state->ewis_conf[port_no].section_txti     = txti_default;
        vtss_state->ewis_conf[port_no].force_mode.line_rx_force  = line_force_default;
        vtss_state->ewis_conf[port_no].force_mode.line_tx_force  = line_tx_force_default;
        vtss_state->ewis_conf[port_no].force_mode.path_force     = path_force_default;
        vtss_state->ewis_conf[port_no].path_txti        = txti_default;
        /* TBD :: Update the correct tx_oh */
        vtss_state->ewis_conf[port_no].tx_oh            = tx_oh_default;
        vtss_state->ewis_conf[port_no].tx_oh_passthru   = tx_oh_passthru_default;
        vtss_state->ewis_conf[port_no].exp_sl.exsl      = VTSS_EWIS_RX_WIS_CTRL_C2_CTRL_RX_EXPECTED_C2_WAN;
        vtss_state->ewis_conf[port_no].test_conf        = test_conf_default;
        /* Not supported for Daytona*/
        vtss_state->ewis_conf[port_no].ewis_cntr_thresh_conf = counter_threshold_default;
        vtss_state->ewis_conf[port_no].ewis_init_done = TRUE;
    }

    return VTSS_RC_OK;
}
#endif /* VTSS_ARCH_DAYTONA */

