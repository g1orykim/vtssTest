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

// Avoid "vtss_api.h not used in module vtss_daytona_otn.c"
/*lint --e{766} */

#include "vtss_api.h"
#if defined(VTSS_ARCH_DAYTONA)
#include "../ail/vtss_state.h"
#include "vtss_daytona.h"
#include "vtss_daytona_regs.h"
#include "vtss_daytona_basics.h"
#include "vtss_daytona_otn.h"

#define TTI_INDEX_START 0    /* TTI Register Index Start */
#define TTI_INDEX_LENGTH 31  /* TTI Register Index Length */ 

#define FTFL_INDEX_START 0    /* FTFL Register Index Start */
#define FTFL_INDEX_LENGTH 127 /* FTFL Register Index Length */


static u32 auto_odu_ais(vtss_state_t *vtss_state,
                        const vtss_port_no_t port_no)
{
    u32 i;

    if (vtss_state->otn_state[port_no].otu_cons.ais)    return(1);
    if (vtss_state->otn_state[port_no].odu_cons.ais)    return(1);
    for (i=0; i<VTSS_OTN_ODUT_LEVEL_NUMBER; ++i)    if (vtss_state->otn_state[port_no].odut_cons[i].ais)    return(1);
    return(0);
}

static vtss_rc  odu_aps_config(vtss_state_t *vtss_state,
                               const vtss_port_no_t port_no, u32 target)
{
    u32 value, i=0;
    u8  aps[VTSS_OTN_ODUT_APS_NUMBER]={0,0,0,0};
    u8  mfas;

    if (vtss_state->otn_state[port_no].odu_aps.enable) {
        memcpy(aps, vtss_state->otn_state[port_no].odu_aps.value, VTSS_OTN_ODUT_APS_NUMBER);
        mfas = 0;
    }
    else {
        for (i=0; i<VTSS_OTN_ODUT_LEVEL_NUMBER; ++i)
            if (vtss_state->otn_state[port_no].odut_aps[i].enable)  {memcpy(aps, vtss_state->otn_state[port_no].odut_aps[i].value, VTSS_OTN_ODUT_APS_NUMBER); break;}
        mfas = i+1;
    }

    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_APSPCC_MFAS_SELECT(mfas) | VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_APSPCC((i<VTSS_OTN_ODUT_LEVEL_NUMBER) ? 1 : 0), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_APSPCC_MFAS_SELECT | VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_APSPCC);

    if (i < VTSS_OTN_ODUT_LEVEL_NUMBER) {
        value = (aps[0] << 8) | aps[1];
        DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_APS_PCC_HIGH_VALUE(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_APS_PCC_HIGH_VALUE_APSPCC_HIGH_VALUE(value), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_APS_PCC_HIGH_VALUE_APSPCC_HIGH_VALUE);
        
        value = (aps[2] << 8) | aps[3];
        DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_APS_PCC_LOW_VALUE(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_APS_PCC_LOW_VALUE_APSPCC_LOW_VALUE(value), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_APS_PCC_LOW_VALUE_APSPCC_LOW_VALUE);
    }

    return VTSS_RC_OK;
}


/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */

/* ================================================================= *
 *  OCH Config
 * ================================================================= */
static vtss_rc daytona_otn_och_fec_set(vtss_state_t *vtss_state,
                                       const vtss_port_no_t port_no, const vtss_otn_och_fec_t   *const cfg)
{
    u32               otn_target, efec4_target, efec7_target;
    daytona_channel_t channel;
    daytona_side_t    side;
    vtss_chip_id_t    chip_id;


    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &otn_target));
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EFECI4, &efec4_target));
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EFECI7, &efec7_target)); 
    VTSS_RC(daytona_port_2_channel_side(vtss_state, port_no, &channel, &side));

    /* I7 Fec cannot be set on Rev A */
    if (cfg->type == VTSS_OTN_OCH_FEC_I7) {
        VTSS_RC(vtss_state->misc.chip_id_get (vtss_state, &chip_id));
        if ((chip_id.part_number == VTSS_TARGET_DAYTONA || chip_id.part_number == VTSS_TARGET_TALLADEGA) && chip_id.revision == 0x1){
            VTSS_E("I7 Fec not supported on Revision A\n");
            return VTSS_RC_ERROR;
        }
    }

    switch (cfg->type) {
        case VTSS_OTN_OCH_FEC_NONE:
            DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL(otn_target), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_FEC_BLANKING, VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_FEC_BLANKING | VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_EXTERNAL_FEC);
            DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL(otn_target), 0x00, VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_FEC_ENABLE | VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_EXTERNAL_FEC);

            DAYTONA_WRM(VTSS_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_CONFIG1_CFG(efec4_target), VTSS_M_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_CONFIG1_CFG_ENCODER_BYPASS, VTSS_M_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_CONFIG1_CFG_ENCODER_BYPASS);
            DAYTONA_WRM(VTSS_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG(efec4_target), 0, VTSS_M_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_SCRAM_EN);
            break;
        case VTSS_OTN_OCH_FEC_RS:
            DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL(otn_target), 0x00, VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_FEC_BLANKING | VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_EXTERNAL_FEC);
            DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL(otn_target), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_FEC_ENABLE, VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_FEC_ENABLE | VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_EXTERNAL_FEC);

            DAYTONA_WRM(VTSS_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_CONFIG1_CFG(efec4_target), VTSS_M_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_CONFIG1_CFG_ENCODER_BYPASS, VTSS_M_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_CONFIG1_CFG_ENCODER_BYPASS);
            DAYTONA_WRM(VTSS_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG(efec4_target), 0, VTSS_M_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_SCRAM_EN);
            break;
        case VTSS_OTN_OCH_FEC_I4:
            if(side == 1) { 
                DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL(otn_target), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_EXTERNAL_FEC, VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_EXTERNAL_FEC | VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_FEC_BLANKING);
                DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL(otn_target), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_FEC_ENABLE | VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_EXTERNAL_FEC, VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_FEC_ENABLE | VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_EXTERNAL_FEC);
                DAYTONA_WRM(VTSS_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG(otn_target), VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_FEC_SEL(0), VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_FEC_SEL);
                DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL(otn_target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_SCRAMBLE_PRE_FEC(0), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_SCRAMBLE_PRE_FEC);
                DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL(otn_target), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_SCRAMBLE_POST_FEC(0), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_SCRAMBLE_POST_FEC);

                DAYTONA_WRM(VTSS_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG(efec4_target), VTSS_F_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_LATENCY(3), VTSS_M_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_LATENCY);
                DAYTONA_WRM(VTSS_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG(efec4_target), VTSS_F_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_DI_BYPASS(0), VTSS_M_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_DI_BYPASS);
                DAYTONA_WRM(VTSS_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG(efec4_target), VTSS_M_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_SCRAM_EN, VTSS_M_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_SCRAM_EN);
                DAYTONA_WRM(VTSS_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG(efec4_target), VTSS_M_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_SW_ENA, VTSS_M_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_SW_ENA);
                DAYTONA_WRM(VTSS_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG(efec4_target), VTSS_F_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_SW_RST(0), VTSS_M_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_SW_RST);
                DAYTONA_WRM(VTSS_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG(efec4_target), VTSS_F_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_PMTICK_EN(1), VTSS_M_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_PMTICK_EN);

                DAYTONA_WRM(VTSS_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_CONFIG1_CFG(efec4_target), 0, VTSS_M_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_CONFIG1_CFG_ENCODER_BYPASS);
                DAYTONA_WRM(VTSS_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_CONFIG1_CFG(efec4_target), VTSS_M_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_CONFIG1_CFG_ENCODER_SW_ENA, VTSS_M_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_CONFIG1_CFG_ENCODER_SW_ENA);
            } else {
                VTSS_E("EFEC I.4 not supported on this port\n");
                return VTSS_RC_ERROR;
            }

            break;
        case VTSS_OTN_OCH_FEC_I7:
            if(side == 1) {
                /* TX_OTN_GLOBAL_CONTROL */
                DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL(otn_target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_EXTERNAL_FEC(1), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_EXTERNAL_FEC);
                DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL(otn_target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_FEC_BLANKING(0), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_FEC_BLANKING);  
                DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL(otn_target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_SCRAMBLE_ENABLE(1), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_SCRAMBLE_ENABLE);
                DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL(otn_target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_SCRAMBLE_PRE_FEC(1), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_SCRAMBLE_PRE_FEC);

                /*RX_OTN_GLOBAL_CONTROL */
                DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL(otn_target), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_EXTERNAL_FEC(1), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_EXTERNAL_FEC);
                DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL(otn_target), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_SCRAMBLE_POST_FEC(1), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_SCRAMBLE_POST_FEC);
                DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL(otn_target), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_FEC_ENABLE(1), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_FEC_ENABLE);
                DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL(otn_target), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_SCRAMBLE_ENABLE(1), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_SCRAMBLE_ENABLE);

                /* WRAPPER_CONTROL */
                DAYTONA_WRM(VTSS_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG(otn_target), VTSS_F_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_FEC_SEL(1), VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_CONTROL_REG_FEC_SEL);

                /* EFECI7 Configuration */
                DAYTONA_WRM(VTSS_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_CONFIG1_CFG(efec4_target), 0, VTSS_M_EFECI4_EFECI4_ENCODER_CFG_EFECI4_ENCODER_CONFIG1_CFG_ENCODER_BYPASS);
                DAYTONA_WRM(VTSS_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG(efec4_target), 0, VTSS_M_EFECI4_EFECI4_DECODER_CFG_EFECI4_DECODER_CONFIG1_CFG_DECODER_SCRAM_EN);
                DAYTONA_WRM(VTSS_EFECI7_EFECI7_CONTROL_EFECI7_MODE(efec7_target), VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_MODE_SW_ENA(TRUE), VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_MODE_SW_ENA);
                DAYTONA_WRM(VTSS_EFECI7_EFECI7_CONTROL_EFECI7_MODE(efec7_target), VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_MODE_SW_RST(FALSE), VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_MODE_SW_RST);
            } else {
                VTSS_E("EFEC I.7 not supported on this port\n");
                return VTSS_RC_ERROR;
            }
            break;
        default: return(VTSS_RC_ERROR);
    }
    vtss_state->otn_state[port_no].och_fec = *cfg;

    /* Set default Ber_threshold for I7 Fec */
    if (cfg->type == VTSS_OTN_OCH_FEC_I7 && side == 1)
        vtss_state->otn_state[port_no].och_fec.ber_threshold = 8356;    
    
    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_och_ber_threshold_set(vtss_state_t *vtss_state,
                                                 const vtss_port_no_t port_no, const vtss_otn_och_fec_t   *const cfg)
{
    u32 otn_target, efeci7_target;
    daytona_channel_t channel;
    daytona_side_t    side;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &otn_target));
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EFECI7, &efeci7_target));
    VTSS_RC(daytona_port_2_channel_side(vtss_state, port_no, &channel, &side));

    if (side == 1) {
        if (cfg->type == VTSS_OTN_OCH_FEC_RS) {
            DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL(otn_target), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_FEC_BER_SELECT(cfg->ber_threshold), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_FEC_BER_SELECT);
        }
        else if (cfg->type == VTSS_OTN_OCH_FEC_I7) {
            DAYTONA_WRM(VTSS_EFECI7_EFECI7_CONTROL_EFECI7_MODE(efeci7_target), VTSS_F_EFECI7_EFECI7_CONTROL_EFECI7_MODE_BER_LIMIT(cfg->ber_threshold), VTSS_M_EFECI7_EFECI7_CONTROL_EFECI7_MODE_BER_LIMIT);
        }

        vtss_state->otn_state[port_no].och_fec = *cfg;
    }

    return VTSS_RC_OK;
}
    
static vtss_rc daytona_otn_och_loopback_set(vtss_state_t *vtss_state,
                                            const vtss_port_no_t port_no)
{
    u32 target;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

        DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_RX_FACILITY_LOOPBACK(vtss_state->otn_state[port_no].och_loop.line_side_line_loopback), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_RX_FACILITY_LOOPBACK);

        DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_TX_TERMINAL_LOOPBACK(vtss_state->otn_state[port_no].och_loop.line_side_equipment_loopback), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_TX_TERMINAL_LOOPBACK);

        DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_RX_TERMINAL_LOOPBACK(vtss_state->otn_state[port_no].och_loop.system_side_line_loopback), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_GLOBAL_CONTROL_RX_TERMINAL_LOOPBACK);

        DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_TX_FACILITY_LOOPBACK(vtss_state->otn_state[port_no].och_loop.system_side_equipment_loopback), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_GLOBAL_CONTROL_TX_FACILITY_LOOPBACK);

    return VTSS_RC_OK;
}


//static vtss_rc daytona_otn_och_fec_ber_threshold_set(vtss_state *vtss_state,

/* ================================================================= *
 *  OTU Config
 * ================================================================= */
static vtss_rc daytona_otn_otu_tti_set(vtss_state_t *vtss_state,
                                       const vtss_port_no_t port_no)
{
    u32 target;
    u8  i;
    u16 temp_tti = 0;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    /* Set the TX TTI Value */
    if(vtss_state->otn_state[port_no].otu_tti.tx_enable) {
        for(i = TTI_INDEX_START; i <= TTI_INDEX_LENGTH; i++) {
            temp_tti = (((vtss_state->otn_state[port_no].otu_tti.tx_tti[(i<<1) + 1]) << 8) | (vtss_state->otn_state[port_no].otu_tti.tx_tti[(i<<1)]));
            DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_TTI(target,i), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_TTI_SM_TTI_VALUE(temp_tti), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_TTI_SM_TTI_VALUE);
        }
    }
    temp_tti = 0;
    /* Set the Expected TTI value */
    for(i = TTI_INDEX_START; i <= TTI_INDEX_LENGTH; i++) {
        temp_tti = (((vtss_state->otn_state[port_no].otu_tti.exp_tti[(i<<1) + 1]) << 8) | (vtss_state->otn_state[port_no].otu_tti.exp_tti[(i<<1)]));
        DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_EXPECT(target,i), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_EXPECT_SM_TTI_EXPECT_VALUE(temp_tti), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_EXPECT_SM_TTI_EXPECT_VALUE);
    }

    /* Set insertion of the TX TTI */
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_INSERT_SM_TTI(vtss_state->otn_state[port_no].otu_tti.tx_enable), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_INSERT_SM_TTI);

    /* Enable/Disable TIM on mismatch of SAPI/DAPI/OS */
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL_SM_TTI_SAPI_TIM(vtss_state->otn_state[port_no].otu_tti.tim_sapi), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL_SM_TTI_SAPI_TIM);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL_SM_TTI_DAPI_TIM(vtss_state->otn_state[port_no].otu_tti.tim_dapi), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL_SM_TTI_DAPI_TIM);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL_SM_TTI_OS_TIM(vtss_state->otn_state[port_no].otu_tti.tim_os), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_CONTROL_SM_TTI_OS_TIM);

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_otu_acc_tti_get(vtss_state_t *vtss_state,
                                           const vtss_port_no_t port_no,vtss_otn_otu_acc_tti_t *const tti)
{
    u32 target,value,status;
    u8  i;
    u16 temp_tti1 = 0,temp_tti2 = 0;
    
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    for(i = TTI_INDEX_START; i <= TTI_INDEX_LENGTH; i++) {
        DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_EXTRACT(target,i), &value);
        temp_tti1 = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_TTI_EXTRACT_SM_TTI_EXTRACT_VALUE(value);
        temp_tti2 = (temp_tti1 & 0x00ff);
        tti->tti[i<<1] = temp_tti2;
        tti->tti[(i<<1)+1] = ((temp_tti1 & 0xff00) >> 8);
    }
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STATUS(target), &status);
    tti->inconsistent = (status & (VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STATUS_SM_TTI_INCONSISTENT_IRQ)) ? TRUE : FALSE;

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_otu_ais_set(vtss_state_t *vtss_state,
                                       const vtss_port_no_t port_no)
{
    u32 target;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_FORCE_OTUK_AIS(vtss_state->otn_state[port_no].otu_ais.so_enable), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_FORCE_OTUK_AIS);

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_otu_consequent_actions_set(vtss_state_t *vtss_state,
                                                      const vtss_port_no_t port_no)
{
    u32 target;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    /* OTU BDI Consequent Actions */
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BDI_OTUK_AIS(vtss_state->otn_state[port_no].otu_cons.bdi_ais), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BDI_OTUK_AIS);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BDI_LOF(vtss_state->otn_state[port_no].otu_cons.bdi_lof), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BDI_LOF);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BDI_LOM(vtss_state->otn_state[port_no].otu_cons.bdi_lom), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BDI_LOM);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BDI_LOS(vtss_state->otn_state[port_no].otu_cons.bdi_los), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BDI_LOS);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BDI_TIM(vtss_state->otn_state[port_no].otu_cons.bdi_tim), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_SM_BDI_TIM);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_AUTO_SM_BDI(vtss_state->otn_state[port_no].otu_cons.bdi), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_AUTO_SM_BDI);
    /* OTU BEIBIAE Consequent Actions */
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_AUTO_SM_BEIBIAE(vtss_state->otn_state[port_no].otu_cons.beibiae), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_AUTO_SM_BEIBIAE);
    /* OTU AIS Consequent Actions */
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_LOM(vtss_state->otn_state[port_no].otu_cons.ais_lom), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_LOM);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_SM_TIM(vtss_state->otn_state[port_no].otu_cons.ais_tim), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_SM_TIM);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_AUTO_ODU_AIS(auto_odu_ais(vtss_state, port_no)), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_AUTO_ODU_AIS);
   /* OTU IAE Consequent Actions */
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_AUTO_SM_IAE(vtss_state->otn_state[port_no].otu_cons.iae), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_AUTO_SM_IAE);
 
    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_otu_oh_insertion_set(vtss_state_t *vtss_state,
                                                const vtss_port_no_t port_no)
{
    u32                target;
    vtss_rc            rc = VTSS_RC_OK;
    daytona_channel_t  channel;
    daytona_side_t     side;
    
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    VTSS_RC(daytona_port_2_channel_side(vtss_state, port_no, &channel, &side));
    switch(vtss_state->otn_state[port_no].otu_oh.mode) {
        case VTSS_OTN_OTU_OH_INS_NORMAL:
            DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_BLANKING(0), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_BLANKING);
            DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_PORT_INSERT(0), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_PORT_INSERT);
            break;
        case VTSS_OTN_OTU_OH_INS_BLANK:
            DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_BLANKING(1), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_BLANKING);
            DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_PORT_INSERT(0), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_PORT_INSERT);
            break;
        case VTSS_OTN_OTU_OH_INS_PORT:
            if(side == 0) {
                if(!((vtss_state->oha_state[port_no].oha_cfg.tosi_rosi_otu2_1 == VTSS_OHA_TOSI_ROSI_OTU2_1) || (vtss_state->oha_state[port_no].oha_cfg.otu2_1_otu2_2 == VTSS_OHA_OTU2_OTU2_1))) {
                    VTSS_E("MUX Configuration not done for the OTN block on %s side on channel %d, \n", side == 0 ? "Client" : "Line", channel);
                } else {
                    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_BLANKING(0), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_BLANKING);
                    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_PORT_INSERT(1), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_PORT_INSERT);
                }
            } else {
                if(!(vtss_state->oha_state[port_no].oha_cfg.otu2_1_otu2_2 == VTSS_OHA_OTU2_OTU2_2)) {
                    VTSS_E("MUX Configuration not done for the OTN block on %s side on channel %d, \n", side == 0 ? "Client" : "Line", channel);
                } else {
                    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_BLANKING(0), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_BLANKING);
                    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_PORT_INSERT(1), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_PORT_INSERT);
                }
            }
            break;
        default:
            VTSS_E("Invalid OTU OH Insertion Mode");
            rc = VTSS_RC_ERROR;
            break;
    }

    return rc;
}

static vtss_rc daytona_otn_otu_tx_res_set(vtss_state_t *vtss_state,
                                          const vtss_port_no_t port_no)
{
    u32 target;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    if(vtss_state->otn_state[port_no].otu_res.enable) {
        DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_RES_VALUE(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_RES_VALUE_RES_VALUE(vtss_state->otn_state[port_no].otu_res.value), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_RES_VALUE_RES_VALUE);
        DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_INSERT_RES(vtss_state->otn_state[port_no].otu_res.enable), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_INSERT_RES);
    } else {
        DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_INSERT_RES(vtss_state->otn_state[port_no].otu_res.enable), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_INSERT_RES);
    }

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_otu_acc_res_get(vtss_state_t *vtss_state,
                                           const vtss_port_no_t port_no, vtss_otn_otu_acc_res_t   *const res)
{
    u32 target;
    u32 value = 0, status;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_RES_ACCEPT(target), &value)
        res->value = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_RES_ACCEPT_RES_ACCEPT_VALUE(value);
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STATUS(target), &status);
    res->inconsistent = (status & (VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STATUS_RES_INCONSISTENT_IRQ)) ? TRUE : FALSE;

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_otu_tx_smres_set(vtss_state_t *vtss_state,
                                            const vtss_port_no_t port_no)
{
    u32 target;
    
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    if(vtss_state->otn_state[port_no].otu_smres.enable) {
        DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_VALUE(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_VALUE_SM_RES_VALUE((vtss_state->otn_state[port_no].otu_smres.value)&0x03), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_VALUE_SM_RES_VALUE);
        DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_INSERT_SM_RES(vtss_state->otn_state[port_no].otu_smres.enable), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_INSERT_SM_RES);
    } else {
        DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_INSERT_SM_RES(vtss_state->otn_state[port_no].otu_smres.enable), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_SM_CONTROL_INSERT_SM_RES);
    }

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_otu_acc_smres_get(vtss_state_t *vtss_state,
                                             const vtss_port_no_t port_no, vtss_otn_otu_acc_smres_t   *const smres)
{
    u32 target;
    u32 value = 0, status;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL(target), &value);
    smres->value = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIAE_BDI_IAE_RES_CONTROL_SM_RES_ACCEPT_VALUE(value);
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STATUS(target), &status);
    smres->inconsistent = (status & (VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STATUS_SM_RES_INCONSISTENT_IRQ)) ? TRUE : FALSE;

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_otu_tx_gcc0_set(vtss_state_t *vtss_state,
                                           const vtss_port_no_t port_no)
{
    u32 target;
    
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    if(vtss_state->otn_state[port_no].otu_gcc0.enable) {
        DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_GCC0_VALUE(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_GCC0_VALUE_GCC0_VALUE(vtss_state->otn_state[port_no].otu_gcc0.value), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_GCC0_VALUE_GCC0_VALUE);
        DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_INSERT_GCC0(vtss_state->otn_state[port_no].otu_gcc0.enable), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_INSERT_GCC0);
    } else {
        DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_INSERT_GCC0(vtss_state->otn_state[port_no].otu_gcc0.enable), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_INSERT_GCC0);
    }

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_otu_acc_gcc0_get(vtss_state_t *vtss_state,
                                            const vtss_port_no_t port_no, vtss_otn_otu_acc_gcc0_t   *const gcc0)
{
    u32 target;
    u32 value = 0, status;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_GCC0_ACCEPT(target), &value);
    gcc0->value = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_GCC0_ACCEPT_GCC0_ACCEPT_VALUE(value);
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STATUS(target), &status);
    gcc0->inconsistent = (status & (VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STATUS_GCC0_INCONSISTENT_IRQ)) ? TRUE : FALSE;

    return VTSS_RC_OK;
}



/* ================================================================= *
 *  ODU Config
 * ================================================================= */
static vtss_rc daytona_otn_odu_mode_set(vtss_state_t *vtss_state,
                                        const vtss_port_no_t port_no)
{
    u32 target;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_INSERT_PM_BIP((vtss_state->otn_state[port_no].odu_mode.mode == VTSS_OTN_ODU_MODE_TERMINATION) ? 1 : 0), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_INSERT_PM_BIP);

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odu_tti_set(vtss_state_t *vtss_state,
                                       const vtss_port_no_t port_no)
{
    u32 target;
    u8  i;
    u16 temp_tti = 0;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    /* Set OTN ODU TX TTI */
    if(vtss_state->otn_state[port_no].odu_tti.tx_enable) {
        for(i = TTI_INDEX_START; i <= TTI_INDEX_LENGTH; i++) {
            temp_tti = (((vtss_state->otn_state[port_no].odu_tti.tx_tti[(i<<1) + 1]) << 8) | (vtss_state->otn_state[port_no].odu_tti.tx_tti[(i<<1)]));
            DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_TTI(target,i), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_TTI_PM_TTI_VALUE(temp_tti), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_TTI_PM_TTI_VALUE);
        }
    }
    temp_tti = 0;
    /* Set OTN ODU Expected TTI */
    for(i = TTI_INDEX_START; i <= TTI_INDEX_LENGTH; i++) {
        temp_tti = (((vtss_state->otn_state[port_no].odu_tti.exp_tti[(i<<1) + 1]) << 8) | (vtss_state->otn_state[port_no].odu_tti.exp_tti[(i<<1)]));
        DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_TTI_EXPECT(target,i), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_TTI_EXPECT_PM_TTI_EXPECT_VALUE(temp_tti), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_TTI_EXPECT_PM_TTI_EXPECT_VALUE);
    }

    /* Enable/Disable TX TTI, SAPI/DAPI/OS TIM detection*/
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_TTI_SAPI_TIM(vtss_state->otn_state[port_no].odu_tti.tim_sapi), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_TTI_SAPI_TIM);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_TTI_DAPI_TIM(vtss_state->otn_state[port_no].odu_tti.tim_dapi), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_TTI_DAPI_TIM);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_TTI_OS_TIM(vtss_state->otn_state[port_no].odu_tti.tim_os), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_CONTROL_PM_TTI_OS_TIM);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_INSERT_PM_TTI(vtss_state->otn_state[port_no].odu_tti.tx_enable), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_INSERT_PM_TTI);

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odu_ais_set(vtss_state_t *vtss_state,
                                       const vtss_port_no_t port_no)
{
    u32 target;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    /* OTN ODU Force source AIS */
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_FORCE_ODUK_AIS(vtss_state->otn_state[port_no].odu_ais.so_enable), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_FORCE_ODUK_AIS);
    /* OTN ODU Force sink AIS */
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_FORCE_ODU_AIS(vtss_state->otn_state[port_no].odu_ais.si_enable), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_FORCE_ODU_AIS);

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odu_consequent_actions_set(vtss_state_t *vtss_state,
                                                      const vtss_port_no_t port_no)
{
    u32 target;
    
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    /* OTN ODU BDI Consequent Actions */
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_LOS(vtss_state->otn_state[port_no].odu_cons.bdi_los), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_LOS);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_LOF(vtss_state->otn_state[port_no].odu_cons.bdi_lof), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_LOF);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_LOM(vtss_state->otn_state[port_no].odu_cons.bdi_lom), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_LOM);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_OTUK_AIS(vtss_state->otn_state[port_no].odu_cons.bdi_otu_ais), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_OTUK_AIS);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_PM_LCK(vtss_state->otn_state[port_no].odu_cons.bdi_lck), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_PM_LCK);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_PM_OCI(vtss_state->otn_state[port_no].odu_cons.bdi_oci), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_PM_OCI);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_PM_ODUK_AIS(vtss_state->otn_state[port_no].odu_cons.bdi_ais), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_PM_ODUK_AIS);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_PM_TIM(vtss_state->otn_state[port_no].odu_cons.bdi_tim), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_PM_TIM);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_SM_TIM(vtss_state->otn_state[port_no].odu_cons.bdi_otu_tim), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_AUTO_BDI_CONTROL_PM_BDI_SM_TIM);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_AUTO_PM_BDI(vtss_state->otn_state[port_no].odu_cons.bdi), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_AUTO_PM_BDI);

    /* OTN ODU BEI Consequent Actions */
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_AUTO_PM_BEI(vtss_state->otn_state[port_no].odu_cons.bei), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_PM_CONTROL_AUTO_PM_BEI);

    /* OTN ODU AIS Consequent Actions */
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_PT_MM(vtss_state->otn_state[port_no].odu_cons.ais_plm), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_PT_MM);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_PM_TIMOCILCK(vtss_state->otn_state[port_no].odu_cons.ais_timocilck), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_PM_TIMOCILCK);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_AUTO_ODU_AIS(auto_odu_ais(vtss_state, port_no)), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_AUTO_ODU_AIS);

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odu_oh_insertion_set(vtss_state_t *vtss_state,
                                                const vtss_port_no_t port_no)
{
    u32                target;
    vtss_rc            rc = VTSS_RC_OK;
    daytona_channel_t  channel;
    daytona_side_t     side;


    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    VTSS_RC(daytona_port_2_channel_side(vtss_state, port_no, &channel, &side));
    switch(vtss_state->otn_state[port_no].odu_oh.mode) {
        case VTSS_OTN_ODU_OH_INS_NORMAL:
            DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_ODU_BLANKING(0), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_ODU_BLANKING);
            DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_ODU_PORT_INSERT(0), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_ODU_PORT_INSERT);
            break;
        case VTSS_OTN_ODU_OH_INS_BLANK:
            DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_ODU_BLANKING(1), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_ODU_BLANKING);
            DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_ODU_PORT_INSERT(0), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_ODU_PORT_INSERT);
            break;
        case VTSS_OTN_ODU_OH_INS_PORT:
            if(side == 0) {
                if(!((vtss_state->oha_state[port_no].oha_cfg.tosi_rosi_otu2_1 == VTSS_OHA_TOSI_ROSI_OTU2_1) || (vtss_state->oha_state[port_no].oha_cfg.otu2_1_otu2_2 == VTSS_OHA_OTU2_OTU2_1))) {
                    VTSS_E("MUX Configuration not done for the OTN block on %s side on channel %d, \n", side == 0 ? "Client" : "Line", channel);
                } else {
                    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_ODU_BLANKING(0), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_ODU_BLANKING);
                    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_ODU_PORT_INSERT(1), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_ODU_PORT_INSERT);
                }
            } else {
                if(!(vtss_state->oha_state[port_no].oha_cfg.otu2_1_otu2_2 == VTSS_OHA_OTU2_OTU2_2)) {
                    VTSS_E("MUX Configuration not done for the OTN block on %s side on channel %d, \n", side == 0 ? "Client" : "Line", channel);
                } else {
                    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_BLANKING(0), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_BLANKING);
                    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_PORT_INSERT(1), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_PORT_INSERT);
                }
            }
            break;
        default:
            VTSS_E("Invalid ODU OH Insertion Mode");
            rc = VTSS_RC_ERROR;
            break;
    }

    return rc;
}

static vtss_rc daytona_otn_odu_tx_res_set(vtss_state_t *vtss_state,
                                          const vtss_port_no_t port_no)
{
    u32 target;
    u8 res_low = 0;
    u16 res_high = 0;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    if(vtss_state->otn_state[port_no].odu_res.enable) {
        res_low = ((vtss_state->otn_state[port_no].odu_res.value) & 0xFF);
        res_high = (((vtss_state->otn_state[port_no].odu_res.value) >> 8) & 0xFFFF);
        DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES1_LOW_TCM_ACT_VALUE(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES1_LOW_TCM_ACT_VALUE_RES1_LOW_VALUE(res_low), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES1_LOW_TCM_ACT_VALUE_RES1_LOW_VALUE);
        DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES1_HIGH_VALUE(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES1_HIGH_VALUE_RES1_HIGH_VALUE(res_high), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_RES1_HIGH_VALUE_RES1_HIGH_VALUE);
    }
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_RES1(vtss_state->otn_state[port_no].odu_res.enable), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_RES1);

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odu_tx_exp_set(vtss_state_t *vtss_state,
                                          const vtss_port_no_t port_no)
{
    u32 target;
    u16 exp_value = 0;
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    if(vtss_state->otn_state[port_no].odu_exp.enable){
        exp_value = ((vtss_state->otn_state[port_no].odu_exp.value) & (0xFFFF));
        DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_EXP_VALUE(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_EXP_VALUE_EXP_VALUE(exp_value), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_EXP_VALUE_EXP_VALUE);
    }
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_EXP(vtss_state->otn_state[port_no].odu_exp.enable), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_EXP);

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odu_tx_ftfl_set(vtss_state_t *vtss_state,
                                           const vtss_port_no_t port_no)
{
    u32 target;
    u8 i = 0;
    u16 temp_ftfl = 0;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    if(vtss_state->otn_state[port_no].odu_ftfl.enable) {
        for(i = FTFL_INDEX_START; i<= FTFL_INDEX_LENGTH; i++) {
            temp_ftfl = (((vtss_state->otn_state[port_no].odu_ftfl.value[(i<<1) + 1]) << 8) | (vtss_state->otn_state[port_no].odu_ftfl.value[(i<<1)]));
            DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_FTFL(target,i), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_FTFL_FTFL_VALUE(temp_ftfl), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_FTFL_FTFL_VALUE);
        }
    }
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_FTFL(vtss_state->otn_state[port_no].odu_ftfl.enable), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_CONTROL_INSERT_FTFL);

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odu_tx_aps_set(vtss_state_t *vtss_state,
                                          const vtss_port_no_t port_no)
{
    u32 target;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    return(odu_aps_config(vtss_state, port_no, target));
}

static vtss_rc daytona_otn_odu_pt_set(vtss_state_t *vtss_state,
                                      const vtss_port_no_t port_no)
{
    u32 target;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIXED_PAYLOAD_PAYLOAD_TYPE(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIXED_PAYLOAD_PAYLOAD_TYPE_PAYLOAD_TYPE(vtss_state->otn_state[port_no].odu_pt.pt), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIXED_PAYLOAD_PAYLOAD_TYPE_PAYLOAD_TYPE);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_INSERT_PAYLOAD_TYPE(vtss_state->otn_state[port_no].odu_pt.tx_enable), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_INSERT_PAYLOAD_TYPE);
    /* Setting the Expected PT to Tx PT */
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_CONTROL_PT_EXPECT_VALUE(vtss_state->otn_state[port_no].odu_pt.pt), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_CONTROL_PT_EXPECT_VALUE);

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odu_opu_oh_insertion_set(vtss_state_t *vtss_state,
                                                    const vtss_port_no_t port_no)
{
    u32                target;
    vtss_rc            rc = VTSS_RC_OK;
    daytona_channel_t  channel;
    daytona_side_t     side;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    VTSS_RC(daytona_port_2_channel_side(vtss_state, port_no, &channel, &side));
    switch(vtss_state->otn_state[port_no].odu_opu_oh.mode) {
        case VTSS_OTN_ODU_OPU_OH_INS_NORMAL:
            DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_OPU_BLANKING(0), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_OPU_BLANKING);
            DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_OPU_PORT_INSERT(0), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_OPU_PORT_INSERT);
            break;
        case VTSS_OTN_ODU_OPU_OH_INS_PORT:
            if(side == 0) {
                if(!((vtss_state->oha_state[port_no].oha_cfg.tosi_rosi_otu2_1 == VTSS_OHA_TOSI_ROSI_OTU2_1) || (vtss_state->oha_state[port_no].oha_cfg.otu2_1_otu2_2 == VTSS_OHA_OTU2_OTU2_1))) {
                    VTSS_E("MUX Configuration not done for the OTN block on %s side on channel %d, \n", side == 0 ? "Client" : "Line", channel);
                } else {
                    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_OPU_BLANKING(1), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_OPU_BLANKING);
                    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_OPU_PORT_INSERT(0), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_OPU_PORT_INSERT);
                } 
            } else {
                if(!(vtss_state->oha_state[port_no].oha_cfg.otu2_1_otu2_2 == VTSS_OHA_OTU2_OTU2_2)) {
                    VTSS_E("MUX Configuration not done for the OTN block on %s side on channel %d, \n", side == 0 ? "Client" : "Line", channel);
                } else {
                    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_BLANKING(0), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_BLANKING);
                    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_PORT_INSERT(1), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_FA_OTU_CONTROL_OTU_PORT_INSERT);
                }
            }
            break;
        case VTSS_OTN_ODU_OPU_OH_INS_BLANK:
            DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_OPU_BLANKING(0), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_OPU_BLANKING);
            DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_OPU_PORT_INSERT(1), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_OPU_PORT_INSERT);
            break;
        default:
            VTSS_E("Invalid OPU OH Insertion Mode");
            rc = VTSS_RC_ERROR;
            break;
    }

    return rc;
}

static vtss_rc daytona_otn_odu_opu_test_insertion_set(vtss_state_t *vtss_state,
                                                      const vtss_port_no_t port_no)
{
    u32     target;
    vtss_rc rc = VTSS_RC_OK;


    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    if(vtss_state->otn_state[port_no].odu_opu_test.ins_payload_type){
        switch(vtss_state->otn_state[port_no].odu_opu_test.ins_payload) {
            case VTSS_OTN_ODU_OPU_TEST_INS_FIXED:
                DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_TEST_PAYLOAD_TYPE(0), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_TEST_PAYLOAD_TYPE);
                DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIXED_PAYLOAD_PAYLOAD_TYPE(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIXED_PAYLOAD_PAYLOAD_TYPE_FIXED_PAYLOAD_VALUE(vtss_state->otn_state[port_no].odu_opu_test.payload_value), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_FIXED_PAYLOAD_PAYLOAD_TYPE_FIXED_PAYLOAD_VALUE);
                DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_INSERT_TEST_PAYLOAD(1), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_INSERT_TEST_PAYLOAD);
                break;
            case VTSS_OTN_ODU_OPU_TEST_INS_PN_23:
                DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_TEST_PAYLOAD_TYPE(2),VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_TEST_PAYLOAD_TYPE );
                DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_INSERT_TEST_PAYLOAD(1), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_INSERT_TEST_PAYLOAD);
                break;
            case VTSS_OTN_ODU_OPU_TEST_INS_PN_31:
                DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_TEST_PAYLOAD_TYPE(3),VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_TEST_PAYLOAD_TYPE );
                DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_INSERT_TEST_PAYLOAD(1), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_INSERT_TEST_PAYLOAD);
                break;
            default:
                VTSS_E("Invalid OPU OH Test Payload Insertion Mode");
                rc = VTSS_RC_ERROR;
        }   
    } else {
        DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_INSERT_TEST_PAYLOAD(0), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OPU_CONTROL_INSERT_TEST_PAYLOAD);
    }

    return rc;
}

static vtss_rc daytona_otn_odu_acc_tti_get(vtss_state_t *vtss_state,
                                           const vtss_port_no_t port_no, vtss_otn_odu_acc_tti_t *const tti)
{
    u32 target, value, status;
    u8  i;
    u16 temp_tti1 = 0,temp_tti2 = 0;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    for(i = TTI_INDEX_START; i <= TTI_INDEX_LENGTH; i++) {
        DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_TTI_EXTRACT(target,i), &value);
        temp_tti1 = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_TTI_EXTRACT_PM_TTI_EXTRACT_VALUE(value);
        temp_tti2 = (temp_tti1 & 0x00ff);
        tti->tti[i<<1] = temp_tti2;
        tti->tti[(i<<1)+1] = ((temp_tti1 & 0xff00) >> 8);
    }
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STATUS(target), &status);
    tti->inconsistent = (status & (VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STATUS_PM_TTI_INCONSISTENT_IRQ)) ? TRUE : FALSE;

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odu_acc_res_get(vtss_state_t *vtss_state,
                                           const vtss_port_no_t port_no, vtss_otn_odu_acc_res_t *const res)
{
    u32 target, low_value, high_value, status;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_LOW_TCM_ACT_ACCEPT_VALUE(target), &low_value);
    low_value = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_LOW_TCM_ACT_ACCEPT_VALUE_RES1_LOW_ACCEPT_VALUE(low_value);
    DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_HIGH_ACCEPT(target), &high_value);
    high_value = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_RES1_HIGH_ACCEPT_RES1_HIGH_ACCEPT_VALUE(high_value);
    res->value = ((high_value << 8) | (low_value));
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STATUS(target), &status);
    res->inconsistent = (status & (VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STATUS_RES1_INCONSISTENT_IRQ)) ? TRUE : FALSE;

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odu_acc_exp_get(vtss_state_t *vtss_state,
                                           const vtss_port_no_t port_no, vtss_otn_odu_acc_exp_t *const exp)
{
    u32 target, exp_value, status;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_EXP_ACCEPT(target), &exp_value);
    exp->value = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_EXP_ACCEPT_EXP_ACCEPT_VALUE(exp_value);
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STATUS(target), &status);
    exp->inconsistent = (status & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STATUS_EXP_INCONSISTENT_IRQ) ? TRUE : FALSE;

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odu_acc_ftfl_get(vtss_state_t *vtss_state,
                                            const vtss_port_no_t port_no, vtss_otn_odu_acc_ftfl_t *const ftfl)
{
    u32 target, ftfl_value, status;
    u8 i = 0;
    u16 temp_ftfl1 = 0, temp_ftfl2 = 0;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    for(i = FTFL_INDEX_START; i <= FTFL_INDEX_LENGTH; i++) {
        DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_FTFL_EXTRACT(target,i), &ftfl_value);
        temp_ftfl1 = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_FTFL_EXTRACT_FTFL_EXTRACT_VALUE(ftfl_value);
        temp_ftfl2 = (temp_ftfl1 & 0x00ff);
        ftfl->value[i<<1] = temp_ftfl2;
        ftfl->value[(i<<1)+1] = ((temp_ftfl1 & 0xff00) >> 8);
    }
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STATUS(target), &status);
    ftfl->inconsistent = (status & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STATUS_FTFL_INCONSISTENT_IRQ) ? TRUE : FALSE;

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odu_acc_aps_get(vtss_state_t *vtss_state,
                                           const vtss_port_no_t port_no, vtss_otn_odu_acc_aps_t *const aps)
{
    u32 target, value;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_MFASX_HIGH_ACCEPT(target,0), &value);
    aps->value[0] = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_MFASX_HIGH_ACCEPT_APSPCC_MFASX_HIGH_ACCEPT_VALUE(value) >> 8;
    aps->value[1] = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_MFASX_HIGH_ACCEPT_APSPCC_MFASX_HIGH_ACCEPT_VALUE(value) & 0xFF;

    DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_MFASX_LOW_ACCEPT(target,0), &value);
    aps->value[2] = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_MFASX_LOW_ACCEPT_APSPCC_MFASX_LOW_ACCEPT_VALUE(value) >> 8;
    aps->value[3] = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_MFASX_LOW_ACCEPT_APSPCC_MFASX_LOW_ACCEPT_VALUE(value) & 0xFF;

    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STATUS(target), &value);
    aps->inconsistent = (value & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STATUS_APSPCC_MFAS0_INCONSISTENT_IRQ) ? TRUE : FALSE;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odu_acc_pt_get(vtss_state_t *vtss_state,
                                          const vtss_port_no_t port_no, vtss_otn_odu_acc_pt_t *const pt)
{
    u32 target, value, status;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_FIXED_PYLD_EXPECT_PT_ACCEPT(target), &value);
    pt->pt = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_FIXED_PYLD_EXPECT_PT_ACCEPT_PT_ACCEPT_VALUE(value);
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STATUS(target), &status);
    pt->inconsistent = (status & (VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STATUS_RX_PT_INCONSISTENT_IRQ)) ? TRUE : FALSE;

    return VTSS_RC_OK;
}



/* ================================================================= *
 *  ODUT Config
 * ================================================================= */
static vtss_rc daytona_otn_odut_mode_set(vtss_state_t *vtss_state,
                                         const vtss_port_no_t port_no, const u32 level)
{
    u32 target;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL(target, level), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_INSERT_TCMI_BIP((vtss_state->otn_state[port_no].odut_mode[level].mode == VTSS_OTN_ODUT_MODE_TERMINATION) ? 1 : 0), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_INSERT_TCMI_BIP);

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odut_tti_set(vtss_state_t *vtss_state,
                                        const vtss_port_no_t port_no, const u32 level)
{
    u32 target;
    u8  i;
    u16 temp_tti = 0;
    vtss_otn_odut_tti_t *tti;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    tti = &vtss_state->otn_state[port_no].odut_tti[level];

    /* Set OTN ODUT TX TTI */
    if(tti->tx_enable) {
        for(i = TTI_INDEX_START; i <= TTI_INDEX_LENGTH; i++) {
            temp_tti = (((tti->tx_tti[(i<<1) + 1]) << 8) | (tti->tx_tti[(i<<1)]));
            DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_TCMX_TTI(target,((32*level)+i)), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_TCMX_TTI_TCMI_TTI_VALUE(temp_tti), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_OTU_TCMX_TTI_TCMI_TTI_VALUE);
        }
    }

    /* Set OTN ODUT Expected TTI */
    temp_tti = 0;
    for(i = TTI_INDEX_START; i <= TTI_INDEX_LENGTH; i++) {
        temp_tti = (((tti->exp_tti[(i<<1) + 1]) << 8) | (tti->exp_tti[(i<<1)]));
        DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_TTI_EXPECT(target,((32*level)+i)), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_TTI_EXPECT_TCMI_TTI_EXPECT_VALUE(temp_tti), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_TTI_EXPECT_TCMI_TTI_EXPECT_VALUE);
    }

    /* Enable/Disable TX TTI, SAPI/DAPI/OS TIM detection*/
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL(target, level), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_TTI_SAPI_TIM(tti->tim_sapi), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_TTI_SAPI_TIM);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL(target, level), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_TTI_DAPI_TIM(tti->tim_dapi), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_TTI_DAPI_TIM);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL(target, level), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_TTI_OS_TIM(tti->tim_os), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_TTI_OS_TIM);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL(target, level), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_INSERT_TCMI_TTI(tti->tx_enable), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_INSERT_TCMI_TTI);

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odut_consequent_actions_set(vtss_state_t *vtss_state,
                                                       const vtss_port_no_t port_no, const u32 level)
{
    u32 target, value, i;
    vtss_otn_odut_cons_act_t *cons;
    vtss_otn_odut_cons_act_t bdi_cons;
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    /* OTN ODUT BDI Consequent Actions */
    memset(&bdi_cons, 0, sizeof(bdi_cons));
    for (i=0; i<VTSS_OTN_ODUT_LEVEL_NUMBER; ++i) {
        bdi_cons.bdi = bdi_cons.bdi || vtss_state->otn_state[port_no].odut_cons[i].bdi;
        bdi_cons.bdi_los = bdi_cons.bdi_los || vtss_state->otn_state[port_no].odut_cons[i].bdi_los;
        bdi_cons.bdi_lof = bdi_cons.bdi_lof || vtss_state->otn_state[port_no].odut_cons[i].bdi_lof;
        bdi_cons.bdi_lom = bdi_cons.bdi_lom || vtss_state->otn_state[port_no].odut_cons[i].bdi_lom;
        bdi_cons.bdi_otu_ais = bdi_cons.bdi_otu_ais || vtss_state->otn_state[port_no].odut_cons[i].bdi_otu_ais;
        bdi_cons.bdi_otu_tim = bdi_cons.bdi_otu_tim || vtss_state->otn_state[port_no].odut_cons[i].bdi_otu_tim;
        bdi_cons.bdi_ais = bdi_cons.bdi_ais || vtss_state->otn_state[port_no].odut_cons[i].bdi_ais;
        bdi_cons.bdi_oci = bdi_cons.bdi_oci || vtss_state->otn_state[port_no].odut_cons[i].bdi_oci;
        bdi_cons.bdi_lck = bdi_cons.bdi_lck || vtss_state->otn_state[port_no].odut_cons[i].bdi_lck;
        bdi_cons.bdi_ltc = bdi_cons.bdi_ltc || vtss_state->otn_state[port_no].odut_cons[i].bdi_ltc;
        bdi_cons.bdi_tim = bdi_cons.bdi_tim || vtss_state->otn_state[port_no].odut_cons[i].bdi_tim;
    }

    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_LOS(bdi_cons.bdi_los), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_LOS);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_LOF(bdi_cons.bdi_lof), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_LOF);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_LOM(bdi_cons.bdi_lom), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_LOM);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_OTUK_AIS(bdi_cons.bdi_otu_ais), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_OTUK_AIS);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_TCMI_LCK(bdi_cons.bdi_lck), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_TCMI_LCK);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_TCMI_LTC(bdi_cons.bdi_ltc), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_TCMI_LTC);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_TCMI_OCI(bdi_cons.bdi_oci), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_TCMI_OCI);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_TCMI_ODUK_AIS(bdi_cons.bdi_ais), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_TCMI_ODUK_AIS);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_TCMI_TIM(bdi_cons.bdi_tim), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_TCMI_TIM);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_SM_TIM(bdi_cons.bdi_otu_tim), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMI_AUTO_BDI_CONTROL_TCMI_BDI_SM_TIM);
    cons = &vtss_state->otn_state[port_no].odut_cons[level];
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL(target, level), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_AUTO_TCMI_BDI(cons->bdi), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_AUTO_TCMI_BDI);
    /* OTN ODUT BEI Consequent Actions */
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL(target, level), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_AUTO_TCMI_BEIBIAE(cons->beibiae), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_AUTO_TCMI_BEIBIAE);
    /* OTN ODUT AIS Consequent Actions */
    value = VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_ODU_AIS_TCM1_TIMOCILCK << level;
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL(target), (cons->ais_timocilck ? value : 0), value);
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL(target), VTSS_F_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_AUTO_ODU_AIS(auto_odu_ais(vtss_state, port_no)), VTSS_M_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_MAINTENANCE_CONTROL_AUTO_ODU_AIS);
   /* OTU ODUT IAE Consequent Actions */
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL(target, level), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_AUTO_TCMI_STAT_IAE(vtss_state->otn_state[port_no].odut_cons[level].iae), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_AUTO_TCMI_STAT_IAE);

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odut_bdi_set(vtss_state_t *vtss_state,
                                        const vtss_port_no_t port_no, const u32 level)
{
    u32 target, value;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    value = (vtss_state->otn_state[port_no].odut_bdi[level].mode == VTSS_OTN_ODUT_BDI_UNFORCED) ? 0 : 1;
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL(target, level), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_INSERT_TCMI_BDI(value), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_INSERT_TCMI_BDI);

    value = (vtss_state->otn_state[port_no].odut_bdi[level].mode == VTSS_OTN_ODUT_BDI_FORCED_HIGH) ? 1 : 0;
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_VALUE(target, level), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_VALUE_TCMI_BDI_VALUE(value), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_VALUE_TCMI_BDI_VALUE);

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odut_tx_stat_set(vtss_state_t *vtss_state,
                                            const vtss_port_no_t port_no, const u32 level)
{
    u32 target, value;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    value = vtss_state->otn_state[port_no].odut_stat[level].enable ? 1 : 0;
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL(target, level), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_INSERT_TCMI_STAT(value), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_CONTROL_INSERT_TCMI_STAT);

    value = vtss_state->otn_state[port_no].odut_stat[level].value;
    DAYTONA_WRM(VTSS_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_VALUE(target, level), VTSS_F_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_VALUE_TCMI_STAT_VALUE(value), VTSS_M_OTN_WRAPPER_XCO2_TX_OTN_XCO2_TX_OTN_ODU_TCMX_VALUE_TCMI_STAT_VALUE);

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odut_tx_aps_set(vtss_state_t *vtss_state,
                                           const vtss_port_no_t port_no, const u32 level)
{
    u32 target;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    return(odu_aps_config(vtss_state, port_no, target));
}

static vtss_rc daytona_otn_odut_acc_tti_get(vtss_state_t *vtss_state,
                                            const vtss_port_no_t port_no, const u32 level, vtss_otn_odut_acc_tti_t *const tti)
{
    u32 target, value, status;
    u8  i;
    u16 temp_tti1 = 0,temp_tti2 = 0;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    for(i = TTI_INDEX_START; i <= TTI_INDEX_LENGTH; i++) {
        DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_TTI_EXTRACT(target,((32*level)+i)), &value);
        temp_tti1 = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_TTI_EXTRACT_TCMI_TTI_EXTRACT_VALUE(value);
        temp_tti2 = (temp_tti1 & 0x00ff);
        tti->tti[i<<1] = temp_tti2;
        tti->tti[(i<<1)+1] = ((temp_tti1 & 0xff00) >> 8);
    }
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STATUS(target, level), &status);
    tti->inconsistent = (status & (VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STATUS_RX_TCMI_TTI_INCONSISTENT_IRQ)) ? TRUE : FALSE;

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odut_acc_stat_get(vtss_state_t *vtss_state,
                                             const vtss_port_no_t port_no, const u32 level, vtss_otn_odut_acc_stat_t *const stat)
{
    u32 target, value;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL(target,level), &value);
    stat->value = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_CONTROL_TCMI_STAT_ACCEPT_VALUE(value);

    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STATUS(target, level), &value);
    stat->inconsistent = (value & (VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STATUS_RX_TCMI_STAT_INCONSISTENT_IRQ)) ? TRUE : FALSE;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odut_acc_aps_get(vtss_state_t *vtss_state,
                                            const vtss_port_no_t port_no, const u32 level, vtss_otn_odut_acc_aps_t *const aps)
{
    u32 target, value;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_MFASX_HIGH_ACCEPT(target,(level+1)), &value);
    aps->value[0] = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_MFASX_HIGH_ACCEPT_APSPCC_MFASX_HIGH_ACCEPT_VALUE(value) >> 8;
    aps->value[1] = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_MFASX_HIGH_ACCEPT_APSPCC_MFASX_HIGH_ACCEPT_VALUE(value) & 0xFF;

    DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_MFASX_LOW_ACCEPT(target,(level+1)), &value);
    aps->value[2] = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_MFASX_LOW_ACCEPT_APSPCC_MFASX_LOW_ACCEPT_VALUE(value) >> 8;
    aps->value[3] = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_APS_PCC_MFASX_LOW_ACCEPT_APSPCC_MFASX_LOW_ACCEPT_VALUE(value) & 0xFF;

    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STATUS(target), &value);
    aps->inconsistent = (value & (VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STATUS_APSPCC_MFAS1_INCONSISTENT_IRQ << level)) ? TRUE : FALSE;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    return VTSS_RC_OK;
}



/* ================================================================= *
 *  State Reporting
 * ================================================================= */
static vtss_rc daytona_otn_och_defects_get(vtss_state_t *vtss_state,
                                           const vtss_port_no_t port_no, vtss_otn_och_defects_t *const defects)
{
    u32 target;
    u32 status;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS(target), &status);

    defects->dlos = (status & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS_LOS_IRQ) ? TRUE : FALSE;
    defects->dlof = (status & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS_LOF_IRQ) ? TRUE : FALSE;
    defects->dlom = (status & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS_LOM_IRQ) ? TRUE : FALSE;
    defects->doof = (status & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS_OOF_IRQ) ? TRUE : FALSE;
    defects->doom = (status & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STATUS_OOM_IRQ) ? TRUE : FALSE;

    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STATUS(target), &status);

    defects->dais = (status & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STATUS_OTUK_AIS_IRQ) ? TRUE : FALSE;

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_otu_defects_get(vtss_state_t *vtss_state,
                                           const vtss_port_no_t port_no, vtss_otn_otu_defects_t *const defects)
{
    u32 target;
    u32 status;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STATUS(target), &status);

    defects->dtim =  (status & (VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STATUS_SM_TIM_IRQ | VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STATUS_SM_TTI_INCONSISTENT_IRQ)) ? TRUE : FALSE;
    defects->dbdi =  (status & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STATUS_SM_BDI_IRQ) ? TRUE : FALSE;
    defects->diae =  (status & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STATUS_SM_IAE_IRQ) ? TRUE : FALSE;
    defects->dbiae = (status & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STATUS_SM_BIAE_IRQ) ? TRUE : FALSE;

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odu_defects_get(vtss_state_t *vtss_state,
                                           const vtss_port_no_t port_no, vtss_otn_odu_defects_t *const defects)
{
    u32 target;
    u32 status;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STATUS(target), &status);

    defects->dais = (status & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STATUS_PM_AIS_IRQ);
    defects->dtim = (status & (VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STATUS_PM_TIM_IRQ | VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STATUS_PM_TTI_INCONSISTENT_IRQ)) ? TRUE : FALSE;
    defects->dbdi = (status & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STATUS_PM_BDI_IRQ) ? TRUE : FALSE;
    defects->doci = (status & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STATUS_PM_OCI_IRQ) ? TRUE : FALSE;
    defects->dlck = (status & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STATUS_PM_LCK_IRQ) ? TRUE : FALSE;

    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STATUS(target), &status);

    defects->dplm = (status & (VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STATUS_RX_PLM_IRQ | VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STATUS_RX_PT_INCONSISTENT_IRQ)) ? TRUE : FALSE;

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odut_defects_get(vtss_state_t *vtss_state,
                                            const vtss_port_no_t port_no, const u32 level, vtss_otn_odut_defects_t *const defects)
{
    u32 target;
    u32 status;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STATUS(target, level), &status);

    defects->dais = (status & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STATUS_RX_TCMI_AIS_IRQ);
    defects->dtim = (status & (VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STATUS_RX_TCMI_TIM_IRQ | VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STATUS_RX_TCMI_TTI_INCONSISTENT_IRQ)) ? TRUE : FALSE;
    defects->dbdi = (status & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STATUS_RX_TCMI_BDI_IRQ) ? TRUE : FALSE;
    defects->doci = (status & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STATUS_RX_TCMI_OCI_IRQ) ? TRUE : FALSE;
    defects->dlck = (status & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STATUS_RX_TCMI_LCK_IRQ) ? TRUE : FALSE;
    defects->dltc = (status & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STATUS_RX_TCMI_LTC_IRQ) ? TRUE : FALSE;
    defects->diae = (status & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STATUS_RX_TCMI_IAE_IRQ) ? TRUE : FALSE;
    defects->dbiae = (status & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STATUS_RX_TCMI_BIAE_IRQ) ? TRUE : FALSE;

    return VTSS_RC_OK;
}



/* ================================================================= *
 *  Performance Primitives
 * ================================================================= */
static vtss_rc daytona_otn_och_perf_get(vtss_state_t *vtss_state,
                                        const vtss_port_no_t port_no, vtss_otn_och_perf_t *const perf)
{
    u32 target;
    u32 value = 0;
    
    memset(perf, 0, sizeof(vtss_otn_och_perf_t));

    if (vtss_state->otn_state[port_no].och_fec.type == VTSS_OTN_OCH_FEC_RS) {
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

        DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_CORRECTABLE_SYMBOLS(target), &value);
        perf->pfec_corrected_symbols = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_CORRECTABLE_SYMBOLS_UPPER_FEC_CORRECTABLE_SYMBOLS_COUNTER_VALUE(value) << 16;

        DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_CORRECTABLE_SYMBOLS(target), &value);
        perf->pfec_corrected_symbols |= VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_CORRECTABLE_SYMBOLS_LOWER_FEC_CORRECTABLE_SYMBOLS_COUNTER_VALUE(value);

        DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_CORRECTED_ZEROS(target), &value);
        perf->pfec_corrected_zero = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_CORRECTED_ZEROS_UPPER_FEC_CORRECTABLE_ZEROS_COUNTER_VALUE(value) << 16;

        DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_CORRECTED_ZEROS(target), &value);
        perf->pfec_corrected_zero |= VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_CORRECTED_ZEROS_LOWER_FEC_CORRECTED_ZEROS_COUNTER_VALUE(value);

        DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_CORRECTED_ONES(target), &value);
        perf->pfec_corrected_one = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_CORRECTED_ONES_UPPER_FEC_CORRECTED_ONES_COUNTER_VALUE(value) << 16;

        DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_CORRECTED_ONES(target), &value);
        perf->pfec_corrected_one |= VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_CORRECTED_ONES_LOWER_FEC_CORRECTED_ONES_COUNTER_VALUE(value);

        DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_UNCORRECTABLE_CODEWORDS(target), &value);
        perf->pfec_uncorrectable_codewords = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_UPPER_UNCORRECTABLE_CODEWORDS_UPPER_FEC_UNCORRECTABLE_CODEWORDS_COUNTER_VALUE(value) << 16;

        DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_UNCORRECTABLE_CODEWORDS(target), &value);
        perf->pfec_uncorrectable_codewords |= VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_FEC_LOWER_UNCORRECTABLE_CODEWORDS_LOWER_FEC_UNCORRECTABLE_CODEWORDS_COUNTER_VALUE(value);
    }

    if (vtss_state->otn_state[port_no].och_fec.type == VTSS_OTN_OCH_FEC_I4) {
        VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EFECI4, &target));

        DAYTONA_RD(VTSS_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_CORRECTED0_CNT(target), &value);
        perf->pfec_corrected_zero = VTSS_X_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_CORRECTED0_CNT_CORRECTED0(value);

        DAYTONA_RD(VTSS_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_CORRECTED1_CNT(target), &value);
        perf->pfec_corrected_one = VTSS_X_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_CORRECTED1_CNT_CORRECTED1(value);

        DAYTONA_RD(VTSS_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_RS_UCCW_CNT(target), &value);
        perf->pfec_uncorrectable_codewords = VTSS_X_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_RS_UCCW_CNT_RS_UCCW(value);

        DAYTONA_RD(VTSS_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_BCH_UCCW_CNT(target), &value);
        perf->pfec_uncorrectable_codewords += VTSS_X_EFECI4_EFECI4_DECODER_CTR_EFECI4_DECODER_BCH_UCCW_CNT_BCH_UCCW(value);
    }

    if (vtss_state->otn_state[port_no].och_fec.type == VTSS_OTN_OCH_FEC_I7) {
        VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EFECI7, &target));

        DAYTONA_RD(VTSS_EFECI7_EFECI7_CONTROL_COR_ZEROES(target), &value);
        perf->pfec_corrected_zero = VTSS_X_EFECI7_EFECI7_CONTROL_COR_ZEROES_COR_ZEROES(value);

        DAYTONA_RD(VTSS_EFECI7_EFECI7_CONTROL_COR_ONES(target), &value);
        perf->pfec_corrected_one = VTSS_X_EFECI7_EFECI7_CONTROL_COR_ONES_COR_ONES(value);

        DAYTONA_RD(VTSS_EFECI7_EFECI7_CONTROL_UNCOR_ERR_ROW1(target), &value);
        perf->pfec_uncorrectable_codewords = VTSS_X_EFECI7_EFECI7_CONTROL_UNCOR_ERR_ROW1_UNCOR_ERR_ROW1(value);

        DAYTONA_RD(VTSS_EFECI7_EFECI7_CONTROL_UNCOR_ERR_ROW2(target), &value);
        perf->pfec_uncorrectable_codewords += VTSS_X_EFECI7_EFECI7_CONTROL_UNCOR_ERR_ROW2_UNCOR_ERR_ROW2(value);

        DAYTONA_RD(VTSS_EFECI7_EFECI7_CONTROL_UNCOR_ERR_ROW3(target), &value);
        perf->pfec_uncorrectable_codewords += VTSS_X_EFECI7_EFECI7_CONTROL_UNCOR_ERR_ROW3_UNCOR_ERR_ROW3(value);
    }

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_otu_perf_get(vtss_state_t *vtss_state,
                                        const vtss_port_no_t port_no, vtss_otn_otu_perf_t *const perf)
{
    u32 target;
    u32 value = 0;
    
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    /* BIP Counter */
    DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIP(target), &value);
    perf->pnebc = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BIP_SM_BIP_COUNTER_VALUE(value);

    /* BEI Counter */
    DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BEI(target), &value);
    perf->pfebc = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OTU_SM_BEI_SM_BEI_COUNTER_VALUE(value);

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odu_perf_get(vtss_state_t *vtss_state,
                                        const vtss_port_no_t port_no, vtss_otn_odu_perf_t *const perf)
{
    u32 target;
    u32 value = 0;
    u16 prbs_error = 0;
    
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));
    /* BIP Counter */
    DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_BIP(target), &value);
    perf->pnebc = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_BIP_PM_BIP_COUNTER_VALUE(value);

    /* BEI Counter */
    DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_BEI(target), &value);
    perf->pfebc = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_PM_BEI_PM_BEI_COUNTER_VALUE(value);

    /* NJ Count */
    DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_NJ(target), &value);
    perf->njus = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_NJ_NJ_COUNTER_VALUE(value);

    /* PJ Count */
    DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_PJ(target), &value);
    perf->pjus = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_PJ_PJ_COUNTER_VALUE(value);

    /* Upper PRBS Error Count */
    DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_PRBS_UPPER_ERROR(target), &value);
    prbs_error = ((VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_PRBS_UPPER_ERROR_PRBS_UPPER_ERROR_COUNTER_HIGH_VALUE(value)) << 8);

    /*Lower PRBS Error Count */
    DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_PRBS_LOWER_ERROR(target), &value);
    prbs_error = ((prbs_error) | (VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_OPU_PRBS_LOWER_ERROR_PRBS_LOWER_ERROR_COUNTER_HIGH_VALUE(value)));
    perf->prbs = prbs_error;
    
    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odut_perf_get(vtss_state_t *vtss_state,
                                         const vtss_port_no_t port_no, const u32 level, vtss_otn_odut_perf_t *const perf)
{
    u32 target;
    u32 value;
    
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    /* BIP Counter */
    DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_BIP(target,level), &value);
    perf->pnebc = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_BIP_TCMI_BIP_COUNTER_VALUE(value);

    /* BEI Counter */
    DAYTONA_RD(VTSS_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_BEI(target,level), &value);
    perf->pfebc = VTSS_X_OTN_WRAPPER_XCO2_RX_OTN_XCO2_RX_OTN_ODU_TCMX_BEI_TCMI_BEI_COUNTER_VALUE(value);
    
    return VTSS_RC_OK;
}



/* ================================================================= *
 *  Defects/Events
 * ================================================================= */
static vtss_rc daytona_otn_och_event_enable(vtss_state_t *vtss_state,
                                            const vtss_port_no_t port_no, const BOOL enable, const vtss_otn_och_event_t ev_mask)
{
    u32 target;
    u32 mask;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    /* OCH FEC */
    mask = 0;

    if (ev_mask & VTSS_OTN_OCH_LOS_EV)         mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_LOS_IRQ_MASK;
    if (ev_mask & VTSS_OTN_OCH_LOF_EV)         mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_LOF_IRQ_MASK;
    if (ev_mask & VTSS_OTN_OCH_LOM_EV)         mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_LOM_IRQ_MASK;
    if (ev_mask & VTSS_OTN_OCH_BER_EXCD_EV)    mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_FEC_BER_EXCEEDED_IRQ_MASK;
    if (ev_mask & VTSS_OTN_OCH_UNCRCT_CODE_EV) mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK_FEC_UNCORRECTED_CODEWORD_IRQ_MASK;

    DAYTONA_WRM(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK(target), (enable ? mask : 0), mask);

    /* OCH OTU */
    mask = 0;

    if (ev_mask & VTSS_OTN_OCH_AIS_EV)     mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_OTUK_AIS_IRQ_MASK;

    DAYTONA_WRM(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK(target), (enable ? mask : 0), mask);

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_och_event_poll(vtss_state_t *vtss_state,
                                          const vtss_port_no_t port_no, vtss_otn_och_event_t *const status)
{
    u32 target;
    u32 new_sticky, sticky, mask, och;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    /* OCH FEC */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY(target), &sticky); /* Read sticky */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_MASK(target), &mask);     /* Read mask */
    new_sticky  = sticky & mask;                                /* Newly detected sticky */
    DAYTONA_WR(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY(target), new_sticky); /* Clear sticky */

    *status = 0;

    /* Event on new sticky set */
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_LOS_IRQ_STICKY)   *status |= VTSS_OTN_OCH_LOS_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_LOF_IRQ_STICKY)   *status |= VTSS_OTN_OCH_LOF_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_LOM_IRQ_STICKY)   *status |= VTSS_OTN_OCH_LOM_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_FEC_BER_EXCEEDED_IRQ_STICKY)
        *status |= VTSS_OTN_OCH_BER_EXCD_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_FEC_UNCORRECTED_CODEWORD_IRQ_STICKY)
        *status |= VTSS_OTN_OCH_UNCRCT_CODE_EV;

    /* OCH OTU */
    och = VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_OTUK_AIS_IRQ_MASK;         /* Only this IRQ is relevent for OCH */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY(target), &sticky);    /* Read sticky */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK(target), &mask);        /* Read mask */
    new_sticky  = sticky & mask & och;                                                          /* Newly detected sticky */
    DAYTONA_WR(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY(target), new_sticky); /* Clear sticky */

    /* Event on new sticky set */
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_OTUK_AIS_IRQ_MASK)   *status |= VTSS_OTN_OCH_AIS_EV;

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_och_event_poll_without_mask(vtss_state_t *vtss_state,
                                                       const vtss_port_no_t port_no, vtss_otn_och_event_t *const status)
{
    u32 target;
    u32 new_sticky, sticky, och;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    /* OCH FEC */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY(target), &sticky); /* Read sticky */
    DAYTONA_WR(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY(target), sticky); /* Clear sticky */

    /* OCH OTU */
    och = VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_OTUK_AIS_IRQ_MASK;         /* Only this IRQ is relevent for OCH */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY(target), &sticky);    /* Read sticky */
    new_sticky  = sticky & och;                                                          /* Newly detected sticky */
    DAYTONA_WR(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY(target), new_sticky); /* Clear sticky */

    /* Event on new sticky set */
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_OTUK_AIS_IRQ_MASK)   *status |= VTSS_OTN_OCH_AIS_EV;

    *status = 0;

    /* Event on new sticky set */
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_LOS_IRQ_STICKY)   *status |= VTSS_OTN_OCH_LOS_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_LOF_IRQ_STICKY)   *status |= VTSS_OTN_OCH_LOF_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_LOM_IRQ_STICKY)   *status |= VTSS_OTN_OCH_LOM_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_FEC_BER_EXCEEDED_IRQ_STICKY)
        *status |= VTSS_OTN_OCH_BER_EXCD_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_FA_FEC_INTERRUPT_STICKY_FEC_UNCORRECTED_CODEWORD_IRQ_STICKY)
        *status |= VTSS_OTN_OCH_UNCRCT_CODE_EV;

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_otu_event_enable(vtss_state_t *vtss_state,
                                            const vtss_port_no_t port_no, const BOOL enable, const vtss_otn_otu_event_t ev_mask)
{
    u32 target;
    u32 mask;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    mask = 0;

    if (ev_mask & VTSS_OTN_OTU_TIM_EV)              mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_TIM_IRQ_MASK;
    if (ev_mask & VTSS_OTN_OTU_ACC_TTI_EV)          mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_TTI_ACCEPT_IRQ_MASK;
    if (ev_mask & VTSS_OTN_OTU_ICONSIST_TTI_EV)     mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_TTI_INCONSISTENT_IRQ_MASK;
    if (ev_mask & VTSS_OTN_OTU_ACC_RES_EV)          mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_RES_ACCEPT_IRQ_MASK;
    if (ev_mask & VTSS_OTN_OTU_INCONSIST_RES_EV)    mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_RES_INCONSISTENT_IRQ_MASK;
    if (ev_mask & VTSS_OTN_OTU_ACC_SMRES_EV)        mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_RES_ACCEPT_IRQ_MASK;
    if (ev_mask & VTSS_OTN_OTU_INCONSIST_SMRES_EV)  mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_RES_INCONSISTENT_IRQ_MASK;
    if (ev_mask & VTSS_OTN_OTU_ACC_GCC0_EV)         mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_GCC0_ACCEPT_IRQ_MASK;
    if (ev_mask & VTSS_OTN_OTU_INCONSIST_GCC0_EV)   mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_GCC0_INCONSISTENT_IRQ_MASK;
    if (ev_mask & VTSS_OTN_OTU_BDI_EV)              mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_BDI_IRQ_MASK;
    if (ev_mask & VTSS_OTN_OTU_IAE_EV)              mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_IAE_IRQ_MASK;
    if (ev_mask & VTSS_OTN_OTU_BIAE_EV)             mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_BIAE_IRQ_MASK;

    DAYTONA_WRM(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK(target), (enable ? mask : 0), mask);

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_otu_event_poll(vtss_state_t *vtss_state,
                                          const vtss_port_no_t port_no, vtss_otn_otu_event_t *const status)
{
    u32 target;
    u32 new_sticky, sticky, mask, otu;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    otu = ~VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_OTUK_AIS_IRQ_MASK;        /* Only this IRQ is relevent for OTU */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY(target), &sticky);    /* Read sticky */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK(target), &mask);        /* Read mask */
    new_sticky  = sticky & mask & otu;                                                          /* Newly detected sticky */
    DAYTONA_WR(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY(target), new_sticky); /* Clear sticky */

    *status = 0;

    /* Event on new sticky set */
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_TIM_IRQ_MASK)                *status |= VTSS_OTN_OTU_TIM_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_TTI_ACCEPT_IRQ_MASK)         *status |= VTSS_OTN_OTU_ACC_TTI_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_TTI_INCONSISTENT_IRQ_MASK)   *status |= VTSS_OTN_OTU_ICONSIST_TTI_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_RES_ACCEPT_IRQ_MASK)            *status |= VTSS_OTN_OTU_ACC_RES_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_RES_INCONSISTENT_IRQ_MASK)      *status |= VTSS_OTN_OTU_INCONSIST_RES_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_RES_ACCEPT_IRQ_MASK)         *status |= VTSS_OTN_OTU_ACC_SMRES_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_RES_INCONSISTENT_IRQ_MASK)   *status |= VTSS_OTN_OTU_INCONSIST_SMRES_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_GCC0_ACCEPT_IRQ_MASK)           *status |= VTSS_OTN_OTU_ACC_GCC0_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_GCC0_INCONSISTENT_IRQ_MASK)     *status |= VTSS_OTN_OTU_INCONSIST_GCC0_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_BDI_IRQ_MASK)                *status |= VTSS_OTN_OTU_BDI_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_IAE_IRQ_MASK)                *status |= VTSS_OTN_OTU_IAE_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_BIAE_IRQ_MASK)               *status |= VTSS_OTN_OTU_BIAE_EV;

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_otu_event_poll_without_mask(vtss_state_t *vtss_state,
                                                       const vtss_port_no_t port_no, vtss_otn_otu_event_t *const status)
{
    u32 target;
    u32 new_sticky, sticky, otu;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    otu = ~VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_OTUK_AIS_IRQ_MASK;        /* Only this IRQ is relevent for OTU */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY(target), &sticky);    /* Read sticky */
    new_sticky  = sticky & otu;                                                                 /* Updated sticky */
    DAYTONA_WR(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_STICKY(target), new_sticky); /* Clear sticky */

    *status = 0;

    /* Event on new sticky set */
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_TIM_IRQ_MASK)                *status |= VTSS_OTN_OTU_TIM_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_TTI_ACCEPT_IRQ_MASK)         *status |= VTSS_OTN_OTU_ACC_TTI_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_TTI_INCONSISTENT_IRQ_MASK)   *status |= VTSS_OTN_OTU_ICONSIST_TTI_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_RES_ACCEPT_IRQ_MASK)            *status |= VTSS_OTN_OTU_ACC_RES_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_RES_INCONSISTENT_IRQ_MASK)      *status |= VTSS_OTN_OTU_INCONSIST_RES_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_RES_ACCEPT_IRQ_MASK)         *status |= VTSS_OTN_OTU_ACC_SMRES_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_RES_INCONSISTENT_IRQ_MASK)   *status |= VTSS_OTN_OTU_INCONSIST_SMRES_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_GCC0_ACCEPT_IRQ_MASK)           *status |= VTSS_OTN_OTU_ACC_GCC0_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_GCC0_INCONSISTENT_IRQ_MASK)     *status |= VTSS_OTN_OTU_INCONSIST_GCC0_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_BDI_IRQ_MASK)                *status |= VTSS_OTN_OTU_BDI_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_IAE_IRQ_MASK)                *status |= VTSS_OTN_OTU_IAE_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OTU_INTERRUPT_MASK_SM_BIAE_IRQ_MASK)               *status |= VTSS_OTN_OTU_BIAE_EV;

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odu_event_enable(vtss_state_t *vtss_state,
                                            const vtss_port_no_t port_no, const BOOL enable, const vtss_otn_odu_event_t ev_mask)
{
    u32 target;
    u32 mask;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    /* ODU ODU */
    mask = 0;

    if (ev_mask & VTSS_OTN_ODU_ACC_EXP_EV)          mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_EXP_ACCEPT_IRQ_MASK;
    if (ev_mask & VTSS_OTN_ODU_ICONSIST_EXP_EV)     mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_EXP_INCONSISTENT_IRQ_MASK;
    if (ev_mask & VTSS_OTN_ODU_ACC_RES_EV)          mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_RES1_ACCEPT_IRQ_MASK;
    if (ev_mask & VTSS_OTN_ODU_ICONSIST_RES_EV)     mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_RES1_INCONSISTENT_IRQ_MASK;
    if (ev_mask & VTSS_OTN_ODU_ACC_FTFL_EV)         mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_FTFL_ACCEPT_IRQ_MASK;
    if (ev_mask & VTSS_OTN_ODU_ICONSIST_FTFL_EV)    mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_FTFL_INCONSISTENT_IRQ_MASK;

    DAYTONA_WRM(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK(target), (enable ? mask : 0), mask);

    /* ODU PM */
    mask = 0;

    if (ev_mask & VTSS_OTN_ODU_TIM_EV)              mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_TIM_IRQ_MASK;
    if (ev_mask & VTSS_OTN_ODU_ACC_TTI_EV)          mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_TTI_ACCEPT_IRQ_MASK;
    if (ev_mask & VTSS_OTN_ODU_ICONSIST_TTI_EV)     mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_TTI_INCONSISTENT_IRQ_MASK;
    if (ev_mask & VTSS_OTN_ODU_BDI_EV)              mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_BDI_IRQ_MASK;
    if (ev_mask & VTSS_OTN_ODU_OCI_EV)              mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_OCI_IRQ_MASK;
    if (ev_mask & VTSS_OTN_ODU_LCK_EV)              mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_LCK_IRQ_MASK;
    if (ev_mask & VTSS_OTN_ODU_AIS_EV)              mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_AIS_IRQ_MASK;

    DAYTONA_WRM(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK(target), (enable ? mask : 0), mask);

    /* ODU APS */
    mask = 0;

    if (ev_mask & VTSS_OTN_ODU_ACC_APS_EV)          mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS0_ACCEPT_IRQ_MASK;
    if (ev_mask & VTSS_OTN_ODU_ICONSIST_APS_EV)     mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS0_INCONSISTENT_IRQ_MASK;

    DAYTONA_WRM(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK(target), (enable ? mask : 0), mask);

    /* ODU OPU */
    mask = 0;

    if (ev_mask & VTSS_OTN_ODU_PLM_EV)              mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_PLM_IRQ_MASK;
    if (ev_mask & VTSS_OTN_ODU_ACC_PT_EV)           mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_PT_ACCEPT_IRQ_MASK;
    if (ev_mask & VTSS_OTN_ODU_ICONSIST_PT_EV)      mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_PT_INCONSISTENT_IRQ_MASK;

    DAYTONA_WRM(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK(target), (enable ? mask : 0), mask);

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odu_event_poll(vtss_state_t *vtss_state,
                                          const vtss_port_no_t port_no, vtss_otn_odu_event_t *const status)
{
    u32 target;
    u32 new_sticky, sticky, mask, odu;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    /* ODU ODU */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY(target), &sticky);    /* Read sticky */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK(target), &mask);        /* Read mask */
    new_sticky  = sticky & mask;                                                                /* Newly detected sticky */
    DAYTONA_WR(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY(target), new_sticky); /* Clear sticky */

    *status = 0;

    /* Event on new sticky set */
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_EXP_ACCEPT_IRQ_MASK)         *status |= VTSS_OTN_ODU_ACC_EXP_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_EXP_INCONSISTENT_IRQ_MASK)   *status |= VTSS_OTN_ODU_ICONSIST_EXP_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_RES1_ACCEPT_IRQ_MASK)        *status |= VTSS_OTN_ODU_ACC_RES_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_RES1_INCONSISTENT_IRQ_MASK)  *status |= VTSS_OTN_ODU_ICONSIST_RES_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_FTFL_ACCEPT_IRQ_MASK)        *status |= VTSS_OTN_ODU_ACC_FTFL_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_FTFL_INCONSISTENT_IRQ_MASK)  *status |= VTSS_OTN_ODU_ICONSIST_FTFL_EV;

    /* ODU PM */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY(target), &sticky);    /* Read sticky */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK(target), &mask);        /* Read mask */
    new_sticky  = sticky & mask;                                                                   /* Newly detected sticky */
    DAYTONA_WR(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY(target), new_sticky); /* Clear sticky */

    /* Event on new sticky set */
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_TIM_IRQ_MASK)               *status |= VTSS_OTN_ODU_TIM_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_TTI_ACCEPT_IRQ_MASK)        *status |= VTSS_OTN_ODU_ACC_TTI_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_TTI_INCONSISTENT_IRQ_MASK)  *status |= VTSS_OTN_ODU_ICONSIST_TTI_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_BDI_IRQ_MASK)               *status |= VTSS_OTN_ODU_BDI_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_OCI_IRQ_MASK)               *status |= VTSS_OTN_ODU_OCI_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_LCK_IRQ_MASK)               *status |= VTSS_OTN_ODU_LCK_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_AIS_IRQ_MASK)               *status |= VTSS_OTN_ODU_AIS_EV;

    /* ODU APS */
    odu = VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS0_ACCEPT_IRQ_MASK | VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS0_INCONSISTENT_IRQ_MASK;    /* Only this IRQ is relevent for ODU */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY(target), &sticky);    /* Read sticky */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK(target), &mask);        /* Read mask */
    new_sticky  = sticky & mask & odu;                                                            /* Newly detected sticky */
    DAYTONA_WR(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY(target), new_sticky); /* Clear sticky */

    /* Event on new sticky set */
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS0_ACCEPT_IRQ_MASK)         *status |= VTSS_OTN_ODU_ACC_APS_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS0_INCONSISTENT_IRQ_MASK)   *status |= VTSS_OTN_ODU_ICONSIST_APS_EV;

    /* ODU OPU */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY(target), &sticky);    /* Read sticky */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK(target), &mask);        /* Read mask */
    new_sticky  = sticky & mask;                                                                /* Newly detected sticky */
    DAYTONA_WR(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY(target), new_sticky); /* Clear sticky */

    /* Event on new sticky set */
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_PLM_IRQ_MASK)               *status |= VTSS_OTN_ODU_PLM_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_PT_ACCEPT_IRQ_MASK)         *status |= VTSS_OTN_ODU_ACC_PT_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_PT_INCONSISTENT_IRQ_MASK)   *status |= VTSS_OTN_ODU_ICONSIST_PT_EV;

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odu_event_poll_without_mask(vtss_state_t *vtss_state,
                                                       const vtss_port_no_t port_no, vtss_otn_odu_event_t *const status)
{
    u32 target;
    u32 new_sticky, sticky, odu;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    /* ODU ODU */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY(target), &sticky);    /* Read sticky */
    DAYTONA_WR(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_STICKY(target), sticky); /* Clear sticky */

    *status = 0;

    /* Event on new sticky set */
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_EXP_ACCEPT_IRQ_MASK)         *status |= VTSS_OTN_ODU_ACC_EXP_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_EXP_INCONSISTENT_IRQ_MASK)   *status |= VTSS_OTN_ODU_ICONSIST_EXP_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_RES1_ACCEPT_IRQ_MASK)        *status |= VTSS_OTN_ODU_ACC_RES_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_RES1_INCONSISTENT_IRQ_MASK)  *status |= VTSS_OTN_ODU_ICONSIST_RES_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_FTFL_ACCEPT_IRQ_MASK)        *status |= VTSS_OTN_ODU_ACC_FTFL_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_INTERRUPT_MASK_FTFL_INCONSISTENT_IRQ_MASK)  *status |= VTSS_OTN_ODU_ICONSIST_FTFL_EV;

    /* ODU PM */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY(target), &sticky);    /* Read sticky */
    DAYTONA_WR(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_STICKY(target), sticky); /* Clear sticky */

    /* Event on new sticky set */
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_TIM_IRQ_MASK)               *status |= VTSS_OTN_ODU_TIM_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_TTI_ACCEPT_IRQ_MASK)        *status |= VTSS_OTN_ODU_ACC_TTI_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_TTI_INCONSISTENT_IRQ_MASK)  *status |= VTSS_OTN_ODU_ICONSIST_TTI_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_BDI_IRQ_MASK)               *status |= VTSS_OTN_ODU_BDI_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_OCI_IRQ_MASK)               *status |= VTSS_OTN_ODU_OCI_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_LCK_IRQ_MASK)               *status |= VTSS_OTN_ODU_LCK_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_PM_INTERRUPT_MASK_PM_AIS_IRQ_MASK)               *status |= VTSS_OTN_ODU_AIS_EV;

    /* ODU APS */
    odu = VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS0_ACCEPT_IRQ_MASK | VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS0_INCONSISTENT_IRQ_MASK;    /* Only this IRQ is relevent for ODU */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY(target), &sticky);    /* Read sticky */
    new_sticky  = sticky & odu;                                                            /* Newly detected sticky */
    DAYTONA_WR(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY(target), new_sticky); /* Clear sticky */

    /* Event on new sticky set */
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS0_ACCEPT_IRQ_MASK)         *status |= VTSS_OTN_ODU_ACC_APS_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS0_INCONSISTENT_IRQ_MASK)   *status |= VTSS_OTN_ODU_ICONSIST_APS_EV;

    /* ODU OPU */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY(target), &sticky);    /* Read sticky */
    DAYTONA_WR(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_STICKY(target), sticky); /* Clear sticky */

    /* Event on new sticky set */
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_PLM_IRQ_MASK)               *status |= VTSS_OTN_ODU_PLM_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_PT_ACCEPT_IRQ_MASK)         *status |= VTSS_OTN_ODU_ACC_PT_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_OPU_INTERRUPT_MASK_RX_PT_INCONSISTENT_IRQ_MASK)   *status |= VTSS_OTN_ODU_ICONSIST_PT_EV;

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odut_event_enable(vtss_state_t *vtss_state,
                                             const vtss_port_no_t port_no, const u32 level, const BOOL enable, const vtss_otn_odut_event_t ev_mask)
{
    u32 target;
    u32 mask;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    mask = 0;

    if (ev_mask & VTSS_OTN_ODUT_TIM_EV)              mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_TIM_IRQ_MASK;
    if (ev_mask & VTSS_OTN_ODUT_ACC_TTI_EV)          mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_TTI_ACCEPT_IRQ_MASK;
    if (ev_mask & VTSS_OTN_ODUT_ICONSIST_TTI_EV)     mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_TTI_INCONSISTENT_IRQ_MASK;
    if (ev_mask & VTSS_OTN_ODUT_ACC_STAT_EV)         mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_STAT_ACCEPT_IRQ_MASK;
    if (ev_mask & VTSS_OTN_ODUT_ICONSIST_STAT_EV)    mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_STAT_INCONSISTENT_IRQ_MASK;
    if (ev_mask & VTSS_OTN_ODUT_BDI_EV)              mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_BDI_IRQ_MASK;
    if (ev_mask & VTSS_OTN_ODUT_OCI_EV)              mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_OCI_IRQ_MASK;
    if (ev_mask & VTSS_OTN_ODUT_LCK_EV)              mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_LCK_IRQ_MASK;
    if (ev_mask & VTSS_OTN_ODUT_AIS_EV)              mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_AIS_IRQ_MASK;
    if (ev_mask & VTSS_OTN_ODUT_LTC_EV)              mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_LTC_IRQ_MASK;
    if (ev_mask & VTSS_OTN_ODUT_IAE_EV)              mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_IAE_IRQ_MASK;
    if (ev_mask & VTSS_OTN_ODUT_BIAE_EV)             mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_BIAE_IRQ_MASK;

    DAYTONA_WRM(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK(target, level), (enable ? mask : 0), mask);

    /* ODU APS */
    mask = 0;

    if (ev_mask & VTSS_OTN_ODUT_ACC_APS_EV)          mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS1_ACCEPT_IRQ_MASK << level;
    if (ev_mask & VTSS_OTN_ODUT_ICONSIST_APS_EV)     mask |= VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS1_INCONSISTENT_IRQ_MASK << level;

    DAYTONA_WRM(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK(target), (enable ? mask : 0), mask);

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odut_event_poll(vtss_state_t *vtss_state,
                                           const vtss_port_no_t port_no, const u32 level, vtss_otn_odut_event_t *const status)
{
    u32 target;
    u32 new_sticky, sticky, mask, odut;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    /* ODU PM */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY(target, level), &sticky);    /* Read sticky */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK(target, level), &mask);        /* Read mask */
    new_sticky  = sticky & mask;                                                                     /* Newly detected sticky */
    DAYTONA_WR(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY(target, level), new_sticky); /* Clear sticky */

    *status = 0;

    /* Event on new sticky set */
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_TIM_IRQ_MASK)                *status |= VTSS_OTN_ODUT_TIM_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_TTI_ACCEPT_IRQ_MASK)         *status |= VTSS_OTN_ODUT_ACC_TTI_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_TTI_INCONSISTENT_IRQ_MASK)   *status |= VTSS_OTN_ODUT_ICONSIST_TTI_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_STAT_ACCEPT_IRQ_MASK)        *status |= VTSS_OTN_ODUT_ACC_STAT_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_STAT_INCONSISTENT_IRQ_MASK)  *status |= VTSS_OTN_ODUT_ICONSIST_STAT_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_BDI_IRQ_MASK)                *status |= VTSS_OTN_ODUT_BDI_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_OCI_IRQ_MASK)                *status |= VTSS_OTN_ODUT_OCI_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_LCK_IRQ_MASK)                *status |= VTSS_OTN_ODUT_LCK_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_AIS_IRQ_MASK)                *status |= VTSS_OTN_ODUT_AIS_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_LTC_IRQ_MASK)                *status |= VTSS_OTN_ODUT_LTC_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_IAE_IRQ_MASK)                *status |= VTSS_OTN_ODUT_IAE_EV;
    if (new_sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_BIAE_IRQ_MASK)               *status |= VTSS_OTN_ODUT_BIAE_EV;

    /* ODUT APS */
    odut = (VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS1_INCONSISTENT_IRQ_MASK | VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS1_ACCEPT_IRQ_MASK) << level;    /* Only this IRQ is relevent for ODUT */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY(target), &sticky);    /* Read sticky */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK(target), &mask);        /* Read mask */
    new_sticky  = sticky & mask & odut;                                                           /* Newly detected sticky */
    DAYTONA_WR(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY(target), new_sticky); /* Clear sticky */

    /* Event on new sticky set */
    if (new_sticky & (VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS1_ACCEPT_IRQ_MASK << level))         *status |= VTSS_OTN_ODUT_ACC_APS_EV;
    if (new_sticky & (VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS1_INCONSISTENT_IRQ_MASK << level))   *status |= VTSS_OTN_ODUT_ICONSIST_APS_EV;

    return VTSS_RC_OK;
}

static vtss_rc daytona_otn_odut_event_poll_without_mask(vtss_state_t *vtss_state,
                                                        const vtss_port_no_t port_no, const u32 level, vtss_otn_odut_event_t *const status)
{
    u32 target;
    u32 new_sticky, sticky, odut;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_OTN_WRAPPER, &target));

    /* ODU PM */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY(target, level), &sticky);    /* Read sticky */
    DAYTONA_WR(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_STICKY(target, level), sticky); /* Clear sticky */

    *status = 0;

    /* Event on new sticky set */
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_TIM_IRQ_MASK)                *status |= VTSS_OTN_ODUT_TIM_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_TTI_ACCEPT_IRQ_MASK)         *status |= VTSS_OTN_ODUT_ACC_TTI_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_TTI_INCONSISTENT_IRQ_MASK)   *status |= VTSS_OTN_ODUT_ICONSIST_TTI_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_STAT_ACCEPT_IRQ_MASK)        *status |= VTSS_OTN_ODUT_ACC_STAT_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_STAT_INCONSISTENT_IRQ_MASK)  *status |= VTSS_OTN_ODUT_ICONSIST_STAT_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_BDI_IRQ_MASK)                *status |= VTSS_OTN_ODUT_BDI_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_OCI_IRQ_MASK)                *status |= VTSS_OTN_ODUT_OCI_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_LCK_IRQ_MASK)                *status |= VTSS_OTN_ODUT_LCK_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_AIS_IRQ_MASK)                *status |= VTSS_OTN_ODUT_AIS_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_LTC_IRQ_MASK)                *status |= VTSS_OTN_ODUT_LTC_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_IAE_IRQ_MASK)                *status |= VTSS_OTN_ODUT_IAE_EV;
    if (sticky & VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_TCMX_INTERRUPT_MASK_RX_TCMI_BIAE_IRQ_MASK)               *status |= VTSS_OTN_ODUT_BIAE_EV;

    /* ODUT APS */
    odut = (VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS1_INCONSISTENT_IRQ_MASK | VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS1_ACCEPT_IRQ_MASK) << level;    /* Only this IRQ is relevent for ODUT */
    DAYTONA_RD(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY(target), &sticky);    /* Read sticky */
    new_sticky  = sticky & odut;                                                                  /* Updated sticky */
    DAYTONA_WR(VTSS_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_STICKY(target), new_sticky); /* Clear sticky */

    /* Event on new sticky set */
    if (new_sticky & (VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS1_ACCEPT_IRQ_MASK << level))         *status |= VTSS_OTN_ODUT_ACC_APS_EV;
    if (new_sticky & (VTSS_M_OTN_WRAPPER_WRAPPER_GROUP_RX_OTN_ODU_APS_PCC_IRQ_MASK_APSPCC_MFAS1_INCONSISTENT_IRQ_MASK << level))   *status |= VTSS_OTN_ODUT_ICONSIST_APS_EV;

    return VTSS_RC_OK;
}

/* ================================================================= *
 *  Test/Debug Config
 * ================================================================= */


/* ================================================================= *
 *  Utilities and internal
 * ================================================================= */
/*
 * Restart otn configuration on a port
 */
vtss_rc vtss_daytona_otn_restart_conf_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    u32 level;
    vtss_rc rc;

    rc = daytona_otn_och_loopback_set(vtss_state, port_no);
    rc = daytona_otn_och_fec_set(vtss_state, port_no, &(vtss_state->otn_state[port_no].och_fec));
    rc = daytona_otn_otu_tti_set(vtss_state, port_no);
    rc = daytona_otn_otu_ais_set(vtss_state, port_no);
    rc = daytona_otn_otu_consequent_actions_set(vtss_state, port_no);
    rc = daytona_otn_otu_oh_insertion_set(vtss_state, port_no);
    rc = daytona_otn_otu_tx_res_set(vtss_state, port_no);
    rc = daytona_otn_otu_tx_smres_set(vtss_state, port_no);
    rc = daytona_otn_otu_tx_gcc0_set(vtss_state, port_no);
    rc = daytona_otn_odu_mode_set(vtss_state, port_no);
    rc = daytona_otn_odu_tti_set(vtss_state, port_no);
    rc = daytona_otn_odu_ais_set(vtss_state, port_no);
    rc = daytona_otn_odu_consequent_actions_set(vtss_state, port_no);
    rc = daytona_otn_odu_oh_insertion_set(vtss_state, port_no);
    rc = daytona_otn_odu_tx_res_set(vtss_state, port_no);
    rc = daytona_otn_odu_tx_exp_set(vtss_state, port_no);
    rc = daytona_otn_odu_tx_ftfl_set(vtss_state, port_no);
    rc = daytona_otn_odu_tx_aps_set(vtss_state, port_no);
    rc = daytona_otn_odu_pt_set(vtss_state, port_no);
    rc = daytona_otn_odu_opu_oh_insertion_set(vtss_state, port_no);
    rc = daytona_otn_odu_opu_test_insertion_set(vtss_state, port_no);
    for (level=0; level<VTSS_OTN_ODUT_LEVEL_NUMBER; ++level) {
        rc = daytona_otn_odut_mode_set(vtss_state, port_no, level);
        rc = daytona_otn_odut_tti_set(vtss_state, port_no, level);
        rc = daytona_otn_odut_consequent_actions_set(vtss_state, port_no, level);
        rc = daytona_otn_odut_tx_stat_set(vtss_state, port_no, level);
        rc = daytona_otn_odut_tx_aps_set(vtss_state, port_no, level);
    }
    VTSS_D("port_no %d, otn restart conf set, rx %x", port_no, rc);

    return VTSS_RC_OK;
}

/*
 * Static initialization of default Otn configuration
 */
static vtss_otn_och_loop_t och_loop_cfg_def = {
    .line_side_equipment_loopback   = FALSE,
    .line_side_line_loopback        = FALSE,
    .system_side_line_loopback      = FALSE,
    .system_side_equipment_loopback = FALSE,
};
static vtss_otn_och_fec_t och_fec_cfg_def = {
    .type = VTSS_OTN_OCH_FEC_NONE,
};
static vtss_otn_otu_tti_t otu_tti_cfg_def = {
    .tim_sapi  = FALSE,
    .tim_dapi  = FALSE,
    .tim_os    = FALSE,
    .tx_enable = FALSE,
};
static vtss_otn_otu_ais_t otu_ais_cfg_def = {
    .so_enable = FALSE,
};
static vtss_otn_otu_cons_act_t otu_cons_cfg_def = {
    .beibiae = FALSE,
    .bdi     = FALSE,
    .ais     = FALSE,
    .iae     = FALSE,
    .bdi_los = FALSE,
    .bdi_lof = FALSE,
    .bdi_lom = FALSE,
    .bdi_ais = FALSE,
    .bdi_tim = FALSE,
    .ais_lom = FALSE,
    .ais_tim = FALSE,
};
static vtss_otn_otu_oh_ins_t otu_oh_cfg_def = {
    .mode = VTSS_OTN_OTU_OH_INS_NORMAL,
};
static vtss_otn_otu_tx_res_t otu_res_cfg_def = {
    .enable = FALSE,
    .value  = 0,
};
static vtss_otn_otu_tx_smres_t otu_smres_cfg_def = {
    .enable = FALSE,
    .value  = 0,
};
static vtss_otn_otu_tx_gcc0_t otu_gcc0_cfg_def = {
    .enable = FALSE,
    .value  = 0,
};
static vtss_otn_odu_mode_t odu_mode_cfg_def = {
    .mode = VTSS_OTN_ODU_MODE_TERMINATION,
};
static vtss_otn_odu_tti_t odu_tti_cfg_def = {
    .tim_sapi  = FALSE,
    .tim_dapi  = FALSE,
    .tim_os    = FALSE,
    .tx_enable = FALSE,
};
static vtss_otn_odu_ais_t odu_ais_cfg_def = {
    .so_enable = FALSE,
    .si_enable = FALSE,
};
static vtss_otn_odu_cons_act_t odu_cons_cfg_def = {
    .bei           = FALSE,
    .bdi           = FALSE,
    .ais           = FALSE, 
    .bdi_los       = FALSE,
    .bdi_lof       = FALSE,
    .bdi_lom       = FALSE,
    .bdi_otu_ais   = FALSE,
    .bdi_otu_tim   = FALSE,
    .bdi_ais       = FALSE,
    .bdi_oci       = FALSE, 
    .bdi_lck       = FALSE,
    .bdi_tim       = FALSE,
    .ais_timocilck = FALSE,
    .ais_plm       = FALSE,
};
static vtss_otn_odu_oh_ins_t odu_oh_cfg_def = {
    .mode = VTSS_OTN_ODU_OH_INS_NORMAL,
};
static vtss_otn_odu_tx_res_t odu_res_cfg_def = {
    .enable = FALSE,
    .value  = 0,
};
static vtss_otn_odu_tx_exp_t odu_exp_cfg_def = {
    .enable = FALSE,
    .value  = 0,
};
static vtss_otn_odu_tx_ftfl_t odu_ftfl_cfg_def = {
    .enable = FALSE,
};
static vtss_otn_odu_tx_aps_t odu_aps_cfg_def = {
    .enable = FALSE,
};
static vtss_otn_odu_pt_t odu_pt_cfg_def = {
    .plm = FALSE,
    .tx_enable = FALSE,
    .pt  = 0,
};
static vtss_otn_odu_opu_oh_ins_t odu_opu_oh_cfg_def = {
    .mode = VTSS_OTN_ODU_OPU_OH_INS_NORMAL,
};
static vtss_otn_odu_opu_test_ins_t odu_opu_test_cfg_def = {
    .payload_value    = 0,
    .ins_payload      = VTSS_OTN_ODU_OPU_TEST_INS_NONE,
    .ins_payload_type = FALSE,       
};
static vtss_otn_odut_mode_t odut_mode_cfg_def = {
    .mode = VTSS_OTN_ODUT_MODE_TERMINATION,
};
static vtss_otn_odut_tti_t odut_tti_cfg_def = {
    .tim_sapi  = FALSE,
    .tim_dapi  = FALSE,
    .tim_os    = FALSE,
    .tx_enable = FALSE,
}; 
static vtss_otn_odut_cons_act_t odut_cons_cfg_def = {
    .beibiae       = FALSE,
    .bdi           = FALSE,
    .ais           = FALSE, 
    .iae           = FALSE,
    .bdi_los       = FALSE,
    .bdi_lof       = FALSE,
    .bdi_lom       = FALSE,
    .bdi_otu_ais   = FALSE,
    .bdi_otu_tim   = FALSE,
    .bdi_ais       = FALSE,
    .bdi_oci       = FALSE, 
    .bdi_lck       = FALSE,
    .bdi_ltc       = FALSE,
    .bdi_tim       = FALSE,
    .ais_timocilck = FALSE,
};
static vtss_otn_odut_bdi_t odut_bdi_cfg_def = {
    .mode = VTSS_OTN_ODUT_BDI_UNFORCED,
};
static vtss_otn_odut_tx_stat_t odut_stat_cfg_def = {
    .enable = FALSE,
    .value  = 0,
};
static vtss_otn_odut_tx_aps_t odut_aps_cfg_def = {
    .enable = FALSE,
};      
/*
 * Set default otn configuration
 */
vtss_rc vtss_daytona_otn_default_conf_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    int cnt = 0, cnt2 = 0;

    vtss_state->otn_state[port_no].och_loop = och_loop_cfg_def;
    vtss_state->otn_state[port_no].och_fec  = och_fec_cfg_def;
    vtss_state->otn_state[port_no].otu_tti  = otu_tti_cfg_def;
    for(cnt = 0; cnt < VTSS_OTN_OTU_TTI_NUMBER; cnt++)
    {
        vtss_state->otn_state[port_no].otu_tti.tx_tti[cnt] = 0;
        vtss_state->otn_state[port_no].otu_tti.exp_tti[cnt] = 0;
    } 
        
    vtss_state->otn_state[port_no].otu_ais  = otu_ais_cfg_def;
    vtss_state->otn_state[port_no].otu_cons = otu_cons_cfg_def;
    vtss_state->otn_state[port_no].otu_oh   = otu_oh_cfg_def;
    vtss_state->otn_state[port_no].otu_res  = otu_res_cfg_def;
    vtss_state->otn_state[port_no].otu_smres = otu_smres_cfg_def;
    vtss_state->otn_state[port_no].otu_gcc0 = otu_gcc0_cfg_def;
    vtss_state->otn_state[port_no].odu_mode = odu_mode_cfg_def;
    vtss_state->otn_state[port_no].odu_tti  = odu_tti_cfg_def;
    for(cnt = 0; cnt < VTSS_OTN_OTU_TTI_NUMBER; cnt++)
    {
        vtss_state->otn_state[port_no].odu_tti.tx_tti[cnt] = 0;
        vtss_state->otn_state[port_no].odu_tti.exp_tti[cnt] = 0;
    } 
    vtss_state->otn_state[port_no].odu_ais  = odu_ais_cfg_def;
    vtss_state->otn_state[port_no].odu_cons = odu_cons_cfg_def;
    vtss_state->otn_state[port_no].odu_oh   = odu_oh_cfg_def;
    vtss_state->otn_state[port_no].odu_res  = odu_res_cfg_def;
    vtss_state->otn_state[port_no].odu_exp  = odu_exp_cfg_def;
    vtss_state->otn_state[port_no].odu_ftfl = odu_ftfl_cfg_def;
    for(cnt = 0; cnt < 256; cnt++)
        vtss_state->otn_state[port_no].odu_ftfl.value[cnt] = 0;
    vtss_state->otn_state[port_no].odu_aps  = odu_aps_cfg_def;
    for(cnt = 0; cnt < VTSS_OTN_ODU_APS_NUMBER; cnt++)
        vtss_state->otn_state[port_no].odu_aps.value[cnt] = 0;
    vtss_state->otn_state[port_no].odu_pt   = odu_pt_cfg_def;
    vtss_state->otn_state[port_no].odu_opu_oh = odu_opu_oh_cfg_def;
    vtss_state->otn_state[port_no].odu_opu_test = odu_opu_test_cfg_def;
    for (cnt = 0; cnt < VTSS_OTN_ODUT_LEVEL_NUMBER; cnt++)
    {
        vtss_state->otn_state[port_no].odut_mode[cnt] = odut_mode_cfg_def;
        vtss_state->otn_state[port_no].odut_tti[cnt] = odut_tti_cfg_def;
        for(cnt2 = 0; cnt2 < VTSS_OTN_ODUT_TTI_NUMBER; cnt2++)
        {
            vtss_state->otn_state[port_no].odut_tti[cnt].tx_tti[cnt2] = 0;
            vtss_state->otn_state[port_no].odut_tti[cnt].exp_tti[cnt2] = 0;
        } 
        vtss_state->otn_state[port_no].odut_cons[cnt] = odut_cons_cfg_def;
        vtss_state->otn_state[port_no].odut_bdi[cnt] = odut_bdi_cfg_def;
        vtss_state->otn_state[port_no].odut_stat[cnt] = odut_stat_cfg_def;
        vtss_state->otn_state[port_no].odut_aps[cnt] = odut_aps_cfg_def;
        for (cnt2 = 0; cnt2 < VTSS_OTN_ODUT_APS_NUMBER; cnt2++)
            vtss_state->otn_state[port_no].odut_aps[cnt].value[cnt2] = 0;
    } 

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
vtss_rc vtss_daytona_inst_otn_create(vtss_state_t *vtss_state)
{
    vtss_cil_func_t *func = &vtss_state->cil;

    /* Daytona OTN functions */
    func->otn_och_fec_set = daytona_otn_och_fec_set;
    func->otn_och_ber_threshold_set = daytona_otn_och_ber_threshold_set;
    func->otn_och_loopback_set = daytona_otn_och_loopback_set;
    func->otn_otu_tti_set = daytona_otn_otu_tti_set;
    func->otn_otu_ais_set = daytona_otn_otu_ais_set;
    func->otn_otu_consequent_actions_set = daytona_otn_otu_consequent_actions_set;
    func->otn_otu_oh_insertion_set = daytona_otn_otu_oh_insertion_set;
    func->otn_otu_tx_res_set = daytona_otn_otu_tx_res_set;
    func->otn_otu_tx_smres_set = daytona_otn_otu_tx_smres_set;
    func->otn_otu_tx_gcc0_set = daytona_otn_otu_tx_gcc0_set;
    func->otn_odu_mode_set = daytona_otn_odu_mode_set;
    func->otn_odu_tti_set = daytona_otn_odu_tti_set;
    func->otn_odu_ais_set = daytona_otn_odu_ais_set;
    func->otn_odu_consequent_actions_set = daytona_otn_odu_consequent_actions_set;
    func->otn_odu_oh_insertion_set = daytona_otn_odu_oh_insertion_set;
    func->otn_odu_tx_res_set = daytona_otn_odu_tx_res_set;
    func->otn_odu_tx_exp_set = daytona_otn_odu_tx_exp_set;
    func->otn_odu_tx_ftfl_set = daytona_otn_odu_tx_ftfl_set;
    func->otn_odu_tx_aps_set = daytona_otn_odu_tx_aps_set;
    func->otn_odu_pt_set = daytona_otn_odu_pt_set;
    func->otn_odu_opu_oh_insertion_set = daytona_otn_odu_opu_oh_insertion_set;
    func->otn_odu_opu_test_insertion_set = daytona_otn_odu_opu_test_insertion_set;
    func->otn_odut_mode_set = daytona_otn_odut_mode_set;
    func->otn_odut_tti_set = daytona_otn_odut_tti_set;
    func->otn_odut_consequent_actions_set = daytona_otn_odut_consequent_actions_set;
    func->otn_odut_bdi_set = daytona_otn_odut_bdi_set;
    func->otn_odut_tx_stat_set = daytona_otn_odut_tx_stat_set;
    func->otn_odut_tx_aps_set = daytona_otn_odut_tx_aps_set;

    func->otn_otu_acc_tti_get = daytona_otn_otu_acc_tti_get;
    func->otn_otu_acc_res_get = daytona_otn_otu_acc_res_get;
    func->otn_otu_acc_smres_get = daytona_otn_otu_acc_smres_get;
    func->otn_otu_acc_gcc0_get = daytona_otn_otu_acc_gcc0_get;
    func->otn_odu_acc_tti_get = daytona_otn_odu_acc_tti_get;
    func->otn_odu_acc_res_get = daytona_otn_odu_acc_res_get;
    func->otn_odu_acc_exp_get = daytona_otn_odu_acc_exp_get;
    func->otn_odu_acc_ftfl_get = daytona_otn_odu_acc_ftfl_get;
    func->otn_odu_acc_aps_get = daytona_otn_odu_acc_aps_get;
    func->otn_odu_acc_pt_get = daytona_otn_odu_acc_pt_get;
    func->otn_odut_acc_tti_get = daytona_otn_odut_acc_tti_get;
    func->otn_odut_acc_stat_get = daytona_otn_odut_acc_stat_get;
    func->otn_odut_acc_aps_get = daytona_otn_odut_acc_aps_get;

    func->otn_och_defects_get = daytona_otn_och_defects_get;
    func->otn_otu_defects_get = daytona_otn_otu_defects_get;
    func->otn_odu_defects_get = daytona_otn_odu_defects_get;
    func->otn_odut_defects_get = daytona_otn_odut_defects_get;

    func->otn_och_perf_get = daytona_otn_och_perf_get;
    func->otn_otu_perf_get = daytona_otn_otu_perf_get;
    func->otn_odu_perf_get = daytona_otn_odu_perf_get;
    func->otn_odut_perf_get = daytona_otn_odut_perf_get;

    func->otn_och_event_enable = daytona_otn_och_event_enable;
    func->otn_och_event_poll = daytona_otn_och_event_poll;
    func->otn_och_event_poll_without_mask = daytona_otn_och_event_poll_without_mask;
    func->otn_otu_event_enable = daytona_otn_otu_event_enable;
    func->otn_otu_event_poll = daytona_otn_otu_event_poll;
    func->otn_otu_event_poll_without_mask = daytona_otn_otu_event_poll_without_mask;
    func->otn_odu_event_enable = daytona_otn_odu_event_enable;
    func->otn_odu_event_poll = daytona_otn_odu_event_poll;
    func->otn_odu_event_poll_without_mask = daytona_otn_odu_event_poll_without_mask;
    func->otn_odut_event_enable = daytona_otn_odut_event_enable;
    func->otn_odut_event_poll = daytona_otn_odut_event_poll;
    func->otn_odut_event_poll_without_mask = daytona_otn_odut_event_poll_without_mask;

    return VTSS_RC_OK;
}

#endif /* VTSS_ARCH_DAYTONA */


