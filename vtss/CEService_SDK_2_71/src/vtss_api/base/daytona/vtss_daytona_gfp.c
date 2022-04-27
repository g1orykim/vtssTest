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

// Avoid "vtss_api.h not used in module vtss_daytona_gfp.c"
/*lint --e{766} */

#include "vtss_api.h"
#if defined(VTSS_ARCH_DAYTONA)
#include "../ail/vtss_state.h"
#include "vtss_daytona.h"
#include "vtss_daytona_regs.h"
#include "vtss_daytona_basics.h"
#include "vtss_daytona_gfp.h"




#define FORCE_CMF_INTERVAL_MAX 10

/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */

static vtss_rc daytona_gfp_single_err_corr_set(vtss_state_t *vtss_state,
                                               const vtss_port_no_t  port_no)
{
    u32 target;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_GFP, &target));
    DAYTONA_WRM(VTSS_GFP_GFPM_RX_GFP_RX_GFP_CONTROL(target), VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_CORE_HDR_ERROR_CORRECT_ENABLE(vtss_state->gfp_state[port_no].err_corr.core_enable), VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_CORE_HDR_ERROR_CORRECT_ENABLE);
    DAYTONA_WRM(VTSS_GFP_GFPM_RX_GFP_RX_GFP_CONTROL(target), VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_PYLD_HDR_ERROR_CORRECT_ENABLE(vtss_state->gfp_state[port_no].err_corr.pay_enable), VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_PYLD_HDR_ERROR_CORRECT_ENABLE);
    DAYTONA_WRM(VTSS_GFP_GFPM_RX_GFP_RX_GFP_CONTROL(target), VTSS_F_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_TRANSP_CRC16_ERROR_CORRECT_ENABLE(vtss_state->gfp_state[port_no].err_corr.trans_enable), VTSS_M_GFP_GFPM_RX_GFP_RX_GFP_CONTROL_TRANSP_CRC16_ERROR_CORRECT_ENABLE);

    return VTSS_RC_OK;
}


static vtss_rc daytona_gfp_cmf_auto_sf_gen_set(vtss_state_t *vtss_state,
                                               const vtss_port_no_t  port_no)
{
    u32                target, mode;
    vtss_config_mode_t channel_mode;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_GFP, &target));
    VTSS_RC(daytona_port_2_mode(vtss_state, port_no, &channel_mode));
    DAYTONA_RD(VTSS_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL(target), &mode);
    mode = VTSS_X_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_MAPPER_MODE(mode);

    /* LSYNC CSF configuration */
    if(vtss_state->gfp_state[port_no].cmf_auto_sf.lsync_sf == VTSS_GFP_CMF_SF_HW) {
        if(mode == 0) {
            VTSS_E("Invalid LSYNC Configuration in GFP-T Mode \n");
        } else {
            if(channel_mode == VTSS_CONFIG_MODE_TGO_2) { //TBD : Need to all add T-modes which has GFP
                DAYTONA_WRM(VTSS_GFP_GFP_BASE_GFP_CONTROL(target), VTSS_F_GFP_GFP_BASE_GFP_CONTROL_CSF_LSYNC_SEL(3), VTSS_M_GFP_GFP_BASE_GFP_CONTROL_CSF_LSYNC_SEL);
                DAYTONA_WRM(VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL(target), VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_LSYNC_MGMT_INSERT(1), VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_LSYNC_MGMT_INSERT);
            } else {
                DAYTONA_WRM(VTSS_GFP_GFP_BASE_GFP_CONTROL(target), VTSS_F_GFP_GFP_BASE_GFP_CONTROL_CSF_LSYNC_SEL(2), VTSS_M_GFP_GFP_BASE_GFP_CONTROL_CSF_LSYNC_SEL);
                DAYTONA_WRM(VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL(target), VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_LSYNC_MGMT_INSERT(1), VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_LSYNC_MGMT_INSERT);
            }
        }
    } else if(vtss_state->gfp_state[port_no].cmf_auto_sf.lsync_sf == VTSS_GFP_CMF_SF_FORCED_ON) {
        DAYTONA_WRM(VTSS_GFP_GFP_BASE_GFP_CONTROL(target), VTSS_F_GFP_GFP_BASE_GFP_CONTROL_CSF_LSYNC_SEL(0), VTSS_M_GFP_GFP_BASE_GFP_CONTROL_CSF_LSYNC_SEL);
        DAYTONA_WRM(VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL(target), VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_LSYNC_MGMT_INSERT(1), VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_LSYNC_MGMT_INSERT);
        DAYTONA_WRM(VTSS_GFP_GFP_BASE_GFP_CONTROL(target), VTSS_F_GFP_GFP_BASE_GFP_CONTROL_FORCE_CSF_LSYNC(1), VTSS_M_GFP_GFP_BASE_GFP_CONTROL_FORCE_CSF_LSYNC);
    } else if(vtss_state->gfp_state[port_no].cmf_auto_sf.lsync_sf == VTSS_GFP_CMF_SF_FORCED_OFF) {
        DAYTONA_WRM(VTSS_GFP_GFP_BASE_GFP_CONTROL(target), VTSS_F_GFP_GFP_BASE_GFP_CONTROL_CSF_LSYNC_SEL(0), VTSS_M_GFP_GFP_BASE_GFP_CONTROL_CSF_LSYNC_SEL);
        DAYTONA_WRM(VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL(target), VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_LSYNC_MGMT_INSERT(1), VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_LSYNC_MGMT_INSERT);
        DAYTONA_WRM(VTSS_GFP_GFP_BASE_GFP_CONTROL(target), VTSS_F_GFP_GFP_BASE_GFP_CONTROL_FORCE_CSF_LSYNC(0), VTSS_M_GFP_GFP_BASE_GFP_CONTROL_FORCE_CSF_LSYNC);

    } else {
        DAYTONA_WRM(VTSS_GFP_GFP_BASE_GFP_CONTROL(target), VTSS_F_GFP_GFP_BASE_GFP_CONTROL_CSF_LSYNC_SEL(0), VTSS_M_GFP_GFP_BASE_GFP_CONTROL_CSF_LSYNC_SEL);
        DAYTONA_WRM(VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL(target), VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_LSYNC_MGMT_INSERT(0), VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_LSYNC_MGMT_INSERT);
    }
    
    /* LOS CSF Configuration */
    if(vtss_state->gfp_state[port_no].cmf_auto_sf.los_sf == VTSS_GFP_CMF_SF_HW) {
        DAYTONA_WRM(VTSS_GFP_GFP_BASE_GFP_CONTROL(target), VTSS_F_GFP_GFP_BASE_GFP_CONTROL_CSF_LOS_SEL(1), VTSS_M_GFP_GFP_BASE_GFP_CONTROL_CSF_LOS_SEL);
        DAYTONA_WRM(VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL(target), VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_LOS_MGMT_INSERT(1), VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_LOS_MGMT_INSERT);
    } else if(vtss_state->gfp_state[port_no].cmf_auto_sf.los_sf == VTSS_GFP_CMF_SF_FORCED_ON){
        DAYTONA_WRM(VTSS_GFP_GFP_BASE_GFP_CONTROL(target), VTSS_F_GFP_GFP_BASE_GFP_CONTROL_CSF_LOS_SEL(0), VTSS_M_GFP_GFP_BASE_GFP_CONTROL_CSF_LOS_SEL);
        DAYTONA_WRM(VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL(target), VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_LOS_MGMT_INSERT(1), VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_LOS_MGMT_INSERT);
        DAYTONA_WRM(VTSS_GFP_GFP_BASE_GFP_CONTROL(target), VTSS_F_GFP_GFP_BASE_GFP_CONTROL_FORCE_CSF_LOS(1), VTSS_M_GFP_GFP_BASE_GFP_CONTROL_FORCE_CSF_LOS);
    } else if(vtss_state->gfp_state[port_no].cmf_auto_sf.los_sf == VTSS_GFP_CMF_SF_FORCED_OFF) {
        DAYTONA_WRM(VTSS_GFP_GFP_BASE_GFP_CONTROL(target), VTSS_F_GFP_GFP_BASE_GFP_CONTROL_CSF_LOS_SEL(0), VTSS_M_GFP_GFP_BASE_GFP_CONTROL_CSF_LOS_SEL);
        DAYTONA_WRM(VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL(target), VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_LOS_MGMT_INSERT(1), VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_LOS_MGMT_INSERT);
        DAYTONA_WRM(VTSS_GFP_GFP_BASE_GFP_CONTROL(target), VTSS_F_GFP_GFP_BASE_GFP_CONTROL_FORCE_CSF_LOS(0), VTSS_M_GFP_GFP_BASE_GFP_CONTROL_FORCE_CSF_LOS);
    } else {
        DAYTONA_WRM(VTSS_GFP_GFP_BASE_GFP_CONTROL(target), VTSS_F_GFP_GFP_BASE_GFP_CONTROL_CSF_LOS_SEL(0), VTSS_M_GFP_GFP_BASE_GFP_CONTROL_CSF_LOS_SEL);
        DAYTONA_WRM(VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL(target), VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_LOS_MGMT_INSERT(0), VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_LOS_MGMT_INSERT);
    }

    return VTSS_RC_OK;
}


static vtss_rc daytona_gfp_cmf_forced_gen_set(vtss_state_t *vtss_state,
                                              const vtss_port_no_t  port_no)
{
    u32 target;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_GFP, &target));
    if(vtss_state->gfp_state[port_no].cmf_forced.enable) {
        if(vtss_state->gfp_state[port_no].cmf_forced.interval <= FORCE_CMF_INTERVAL_MAX) {
            DAYTONA_WRM(VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL(target),VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_FORCE_MGMT_UPI_VALUE(vtss_state->gfp_state[port_no].cmf_forced.upi), VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_FORCE_MGMT_UPI_VALUE);
            DAYTONA_WRM(VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL(target),VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_MGMT_PTI_VALUE(vtss_state->gfp_state[port_no].cmf_forced.pti), VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_MGMT_PTI_VALUE);
            DAYTONA_WRM(VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_INTERVAL(target), VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_INTERVAL_MGMT_FRAME_INTERVAL_SELECT(vtss_state->gfp_state[port_no].cmf_forced.interval), VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_INTERVAL_MGMT_FRAME_INTERVAL_SELECT);
            DAYTONA_WRM(VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_PKT_LENGTH(target),  VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_PKT_LENGTH_CLIENT_MGMT_PKT_LENGTH(vtss_state->gfp_state[port_no].cmf_forced.frame_len),  VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_PKT_LENGTH_CLIENT_MGMT_PKT_LENGTH);
            DAYTONA_WRM(VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL(target), VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_FORCE_MGMT_INSERT(vtss_state->gfp_state[port_no].cmf_forced.enable), VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_FORCE_MGMT_INSERT);
        } else {
            VTSS_E("Invalid interval passed in CMF Forced Generation Configuration \n");
            return VTSS_RC_ERROR;
        }
    } else {
       DAYTONA_WRM(VTSS_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL(target), VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_FORCE_MGMT_INSERT(vtss_state->gfp_state[port_no].cmf_forced.enable), VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_CLIENT_MGMT_FRAME_CONTROL_FORCE_MGMT_INSERT);
    }
    return VTSS_RC_OK;
}


static vtss_rc daytona_gfp_cdf_fcs_insert_set(vtss_state_t *vtss_state,
                                              const vtss_port_no_t  port_no)
{
    u32 target;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_GFP, &target));
    DAYTONA_WRM(VTSS_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL(target), VTSS_F_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_INSERT_PFI(vtss_state->gfp_state[port_no].fcs_insert.enable), VTSS_M_GFP_GFPM_TX_GFP_TX_GFP_FRAME_CONTROL_INSERT_PFI);

    return VTSS_RC_OK;
}


static vtss_rc daytona_gfp_cdf_upi_set(vtss_state_t *vtss_state,
                                       const vtss_port_no_t  port_no)
{
    u32 target;
    u32 mode;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_GFP, &target));
    DAYTONA_RD(VTSS_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL(target), &mode);
    mode = VTSS_X_GFP_GFPM_TX_GLOBAL_TX_GLOBAL_CONTROL_MAPPER_MODE(mode);
    if(mode == 2) {             /* GFP-F 10GE mode */         
        DAYTONA_WRM(VTSS_GFP_GFP_BASE_GFP_CLIENT_DATA_UPI(target), VTSS_F_GFP_GFP_BASE_GFP_CLIENT_DATA_UPI_CDF_UPI_VALUE(vtss_state->gfp_state[port_no].cdf_upi.upi), VTSS_M_GFP_GFP_BASE_GFP_CLIENT_DATA_UPI_CDF_UPI_VALUE);
    } else if(mode == 3) {     /* GFP-F 10GE PP/OS mode */
        DAYTONA_WRM(VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPI(target), VTSS_F_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPI_DATA_UPI_VALUE(vtss_state->gfp_state[port_no].cdf_upi.upi), VTSS_M_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPI_DATA_UPI_VALUE);
        DAYTONA_WRM(VTSS_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPI(target), VTSS_F_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPI_ORDERED_SET_UPI_VALUE(vtss_state->gfp_state[port_no].cdf_upi.ordered_upi), VTSS_M_GFP_GFPM_TX_10GE_PPOS_TX_PPOS_UPI_ORDERED_SET_UPI_VALUE);
        /* Setting the RX Expected UPI values */
        DAYTONA_WRM(VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPI(target), VTSS_F_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPI_DATA_UPI_VALUE(vtss_state->gfp_state[port_no].cdf_upi.upi), VTSS_M_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPI_DATA_UPI_VALUE);
        DAYTONA_WRM(VTSS_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPI(target), VTSS_F_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPI_ORDERED_SET_UPI_VALUE(vtss_state->gfp_state[port_no].cdf_upi.ordered_upi), VTSS_M_GFP_GFPM_RX_10GE_PPOS_RX_PPOS_UPI_ORDERED_SET_UPI_VALUE);
    } else { /* GFP-T mode */
        VTSS_D("UPI cannot be set in GFP-T mode.\n");    /* UPI value is fixed to 0x3 for Fiber Channel GFP-T mode */
    }

    return VTSS_RC_OK;
}


static vtss_rc daytona_gfp_8b10b_control_code_set(vtss_state_t *vtss_state,
                                                  const vtss_port_no_t  port_no)
{
    u32 target;
    
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_GFP, &target));
    DAYTONA_WRM(VTSS_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL(target), VTSS_F_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_FORCE_LOS(vtss_state->gfp_state[port_no].control_code.force_los), VTSS_M_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_FORCE_LOS);
    DAYTONA_WRM(VTSS_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL(target), VTSS_F_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_FORCE_LSYNC(vtss_state->gfp_state[port_no].control_code.force_lsync), VTSS_M_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_FORCE_LSYNC);
    DAYTONA_WRM(VTSS_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL(target), VTSS_F_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_K30_7_ERROR_CHARACTER_SELECT(vtss_state->gfp_state[port_no].control_code.err_code), VTSS_M_GFP_GFPM_TX_8B10B_DECODER_TX_8B10B_GLOBAL_CONTROL_K30_7_ERROR_CHARACTER_SELECT);

    return VTSS_RC_OK;
}


static vtss_rc daytona_gfp_cmf_acc_upi_pti_get(vtss_state_t *vtss_state,
                                           const vtss_port_no_t  port_no,
                                           u32 *const upi, u32 *const pti)
{
    u32 target, value;
    *upi = 0;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_GFP, &target));
    DAYTONA_RD(VTSS_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE(target), &value);
    *upi = VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE_CAPTURE_UPI_VALUE(value);
    *pti = VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_MGMT_PTIUPI_CAPTURE_CAPTURE_PTI_VALUE(value);

    return VTSS_RC_OK;
}


static vtss_rc daytona_gfp_cdf_acc_upi_get(vtss_state_t *vtss_state,
                                           const vtss_port_no_t  port_no, u32 *const upi)
{
    u32 target, value;
    *upi = 0;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_GFP, &target));
    DAYTONA_RD(VTSS_GFP_GFP_BASE_GFP_CLIENT_DATA_UPI(target), &value);
    *upi = VTSS_X_GFP_GFP_BASE_GFP_CLIENT_DATA_UPI_MISMATCHED_CDF_UPI(value);

    return VTSS_RC_OK;
}


static vtss_rc daytona_gfp_defects_get(vtss_state_t *vtss_state,
                                       const vtss_port_no_t  port_no, vtss_gfp_defects_t *const defects)
{
    u32 target,value;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_GFP, &target));
    memset(defects, 0, sizeof(vtss_gfp_defects_t));
    /* SSF Detection */
    DAYTONA_RD(VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STATUS(target), &value);
    defects->dssf = VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STATUS_SSF_STATUS(value);

    /* CMF Inconsistent Detection */
    DAYTONA_RD(VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STATUS(target), &value);
    defects->dcmf_inconsistent = VTSS_X_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STATUS_CLIENT_MGMT_INCONSISTENT_STATUS(value);

    /* 8B10B Defects Detection */
    DAYTONA_RD(VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STATUS(target), &value);
    defects->dlos = VTSS_X_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STATUS_LOS_STATUS(value);
    DAYTONA_RD(VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STATUS(target), &value);
    defects->dlsync = VTSS_X_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STATUS_LSYNC_STATUS(value);

    return VTSS_RC_OK;
}



/* ================================================================= *
 *  Performance Primitives
 * ================================================================= */
static vtss_rc daytona_gfp_perf_get(vtss_state_t *vtss_state,
                                    const vtss_port_no_t  port_no, vtss_gfp_perf_t *const perf)
{
    u32 target, value, temp_value;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_GFP, &target));
    memset(perf, 0, sizeof(vtss_gfp_perf_t));

    /* Super Blocks Counters */
    DAYTONA_RD(VTSS_GFP_GFPM_RX_GFP_RX_GFP_TRANSP_CRC16_UNCOR_CNT(target), &value);
    perf->psuper_block_uncorrected = VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_TRANSP_CRC16_UNCOR_CNT_TRANSP_CRC16_UNCOR_COUNT(value);
    DAYTONA_RD(VTSS_GFP_GFPM_RX_GFP_RX_GFP_TRANSP_CRC16_COR_CNT(target), &value);
    perf->psuper_block_corrected = VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_TRANSP_CRC16_COR_CNT_TRANSP_CRC16_COR_COUNT(value);
    
    /* Header Counters */
    DAYTONA_RD(VTSS_GFP_GFPM_RX_GFP_RX_GFP_CORE_HDR_COR_CNT(target), &value);
    perf->pcore_header_corrected = VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CORE_HDR_COR_CNT_CORE_HDR_COR_COUNT(value);
    DAYTONA_RD(VTSS_GFP_GFPM_RX_GFP_RX_GFP_PYLD_HDR_COR_CNT(target), &value);
    perf->ppyld_header_corrected = VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_PYLD_HDR_COR_CNT_PYLD_HDR_COR_COUNT(value);
    DAYTONA_RD(VTSS_GFP_GFPM_RX_GFP_RX_GFP_PYLD_HDR_UNCOR_CNT(target), &value);
    perf->ppyld_header_uncorrected = VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_PYLD_HDR_UNCOR_CNT_PYLD_HDR_UNCOR_COUNT(value);

    /* RX Frames Counters */
    DAYTONA_RD(VTSS_GFP_GFPM_RX_GFP_RX_GFP_LOWER_CONTROL_FRAME_CNT(target), &temp_value);
    temp_value = VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_LOWER_CONTROL_FRAME_CNT_LOWER_CONTROL_FRAME_COUNT(temp_value);
    DAYTONA_RD(VTSS_GFP_GFPM_RX_GFP_RX_GFP_UPPER_CONTROL_FRAME_CNT(target), &value);
    value = VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_UPPER_CONTROL_FRAME_CNT_UPPER_CONTROL_FRAME_COUNT(value);
    perf->prx_control_frames = ((value << 16) | (temp_value));
    DAYTONA_RD(VTSS_GFP_GFPM_RX_GFP_RX_GFP_LOWER_CLIENT_MGMT_PKT_CNT(target), &temp_value);
    temp_value = VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_LOWER_CLIENT_MGMT_PKT_CNT_LOWER_CLIENT_MGMT_PKT_COUNT(temp_value);
    DAYTONA_RD(VTSS_GFP_GFPM_RX_GFP_RX_GFP_UPPER_CLIENT_MGMT_PKT_CNT(target), &value);
    value = VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_UPPER_CLIENT_MGMT_PKT_CNT_UPPER_CLIENT_MGMT_PKT_COUNT(value);
    perf->prx_client_management_frames = ((value << 16) | (temp_value));
    DAYTONA_RD(VTSS_GFP_GFPM_RX_GFP_RX_GFP_LOWER_CLIENT_DATA_PKT_CNT(target), &temp_value);
    temp_value = VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_LOWER_CLIENT_DATA_PKT_CNT_LOWER_CLIENT_DATA_FRAME_COUNT(temp_value);
    DAYTONA_RD(VTSS_GFP_GFPM_RX_GFP_RX_GFP_UPPER_CLIENT_DATA_PKT_CNT(target), &value);
    value = VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_UPPER_CLIENT_DATA_PKT_CNT_UPPER_CLIENT_DATA_FRAME_COUNT(value);
    perf->prx_client_data_frames = ((value << 16) | (temp_value));

    /* TX Frames Counters */
    DAYTONA_RD(VTSS_GFP_GFPM_TX_GFP_TX_GFP_LOWER_CONTROL_FRAME_CNT(target), &temp_value);
    temp_value = VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_LOWER_CONTROL_FRAME_CNT_LOWER_CONTROL_FRAME_COUNT(temp_value);
    DAYTONA_RD(VTSS_GFP_GFPM_TX_GFP_TX_GFP_UPPER_CONTROL_FRAME_CNT(target), &value);
    value = VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_UPPER_CONTROL_FRAME_CNT_UPPER_CONTROL_FRAME_COUNT(value);
    perf->ptx_control_frames = ((value << 16) | (temp_value));
    DAYTONA_RD(VTSS_GFP_GFPM_TX_GFP_TX_GFP_LOWER_CLIENT_MGMT_PKT_CNT(target), &temp_value);
    temp_value = VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_LOWER_CLIENT_MGMT_PKT_CNT_LOWER_CLIENT_MGMT_PKT_COUNT(temp_value);
    DAYTONA_RD(VTSS_GFP_GFPM_TX_GFP_TX_GFP_UPPER_CLIENT_MGMT_PKT_CNT(target), &value);
    value = VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_UPPER_CLIENT_MGMT_PKT_CNT_UPPER_CLIENT_MGMT_PKT_COUNT(value);
    perf->ptx_client_management_frames = ((value << 16) | (temp_value));
    DAYTONA_RD(VTSS_GFP_GFPM_TX_GFP_TX_GFP_LOWER_CLIENT_DATA_PKT_CNT(target), &temp_value);
    temp_value = VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_LOWER_CLIENT_DATA_PKT_CNT_LOWER_CLIENT_DATA_FRAME_COUNT(temp_value);
    DAYTONA_RD(VTSS_GFP_GFPM_TX_GFP_TX_GFP_UPPER_CLIENT_DATA_PKT_CNT(target), &value);
    value = VTSS_X_GFP_GFPM_TX_GFP_TX_GFP_UPPER_CLIENT_DATA_PKT_CNT_UPPER_CLIENT_DATA_FRAME_COUNT(value);
    perf->ptx_client_data_frames = ((value << 16) | (temp_value));

    /* Invalid Frames Counter */
    DAYTONA_RD(VTSS_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_INVALID_PKT_CNT(target), &value);
    perf->prx_client_invalid_frames = VTSS_X_GFP_GFPM_RX_GFP_RX_GFP_CLIENT_INVALID_PKT_CNT_CLIENT_INVALID_PKT_COUNT(value);

    return VTSS_RC_OK;
}



/* ================================================================= *
 *  Events
 * ================================================================= */
static vtss_rc daytona_gfp_event_enable(vtss_state_t *vtss_state,
                                        const vtss_port_no_t  port_no, const BOOL enable, const vtss_gfp_event_t ev_mask)
{
    u32 target;
    u32 mask, mode;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_GFP, &target));

    DAYTONA_RD(VTSS_GFP_GFP_BASE_GFP_CONTROL(target), &mode);    /* Read mode selector */
    mode = VTSS_X_GFP_GFP_BASE_GFP_CONTROL_CLIENT_MODE_SEL(mode);

    mask = 0;

    if ((mode != 0x03) && (ev_mask & VTSS_GFP_SSF_EV))              mask |= VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK_SSF_IRQ_MASK;

    DAYTONA_WRM(VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK(target), (enable ? mask : 0), mask);

    mask = 0;

    if ((mode == 0x03) && (ev_mask & VTSS_GFP_SSF_EV))              mask |= VTSS_M_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_SSF_DETECT_IRQ_MASK;
    if ((mode == 0x03) && (ev_mask & VTSS_GFP_PPOS_CDF_UPM_EV))     mask |= VTSS_M_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK_UPI_VIOLATION_IRQ_MASK;
    
    DAYTONA_WRM(VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK(target), (enable ? mask : 0), mask);

    mask = 0;

    if (ev_mask & VTSS_GFP_8B10B_LOS_EV)            mask |= VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK_LOS_IRQ_MASK;
    if (ev_mask & VTSS_GFP_8B10B_LSYNC_EV)          mask |= VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK_LSYNC_IRQ_MASK;
    if (ev_mask & VTSS_GFP_8B10B_TX_ILLEGAL_EV)     mask |= VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK_INVALID_CHARACTER_IRQ_MASK;
    if (ev_mask & VTSS_GFP_8B10B_TX_DISPARITY_EV)   mask |= VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK_DISPARITY_ERROR_IRQ_MASK;

    DAYTONA_WRM(VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK(target), (enable ? mask : 0), mask);

    mask = 0;

    if (ev_mask & VTSS_GFP_8B10B_ERROR_EV)                  mask |= VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_RX_10B_ERR_DETECT_IRQ_MASK | VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_RX_10B_ERR_REMOVAL_IRQ_MASK;
    if (ev_mask & VTSS_GFP_8B10B_RX_DISPARITY_EV)           mask |= VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_DISPARITY_CONTINUITY_ERROR_IRQ_MASK;
    if (ev_mask & VTSS_GFP_8B10B_RX_ILLEGAL_EV)             mask |= VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_INVALID_K_CHARACTER_IRQ_MASK;
    if ((mode == 0x00) && (ev_mask & VTSS_GFP_CDF_UPM_EV))  mask |= VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK_UPI_MISMATCH_IRQ_MASK;

    DAYTONA_WRM(VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK(target), (enable ? mask : 0), mask);

    mask = 0;

    if (ev_mask & VTSS_GFP_CMF_LOS_SF_EV)           mask |= VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_MGMT_LOS_DETECT_IRQ_MASK;
    if (ev_mask & VTSS_GFP_CMF_LSYNC_SF_EV)         mask |= VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_MGMT_LSYNC_DETECT_IRQ_MASK;
    if (ev_mask & VTSS_GFP_CMF_UNSUPPORTED_EV)      mask |= VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_MGMT_UNSUPPORTED_DETECT_IRQ_MASK;
    if (ev_mask & VTSS_GFP_CMF_INCONSISTENT_EV)     mask |= VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_MGMT_INCONSISTENT_IRQ_MASK;
    if (ev_mask & VTSS_GFP_CMF_ACCEPTED_EV)         mask |= VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK_CLIENT_MGMT_ACCEPT_IRQ_MASK;
    if (ev_mask & VTSS_GFP_FRAME_MAPPED_ETHERNET_EV) mask |= VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_FRAME_MAPPED_ETHERNET_DETECT_IRQ_STICKY;

    DAYTONA_WRM(VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK(target), (enable ? mask : 0), mask);

    mask = 0;

    if ((mode != 0x00) && (ev_mask & VTSS_GFP_CDF_UPM_EV))    mask |= VTSS_M_GFP_GFP_BASE_GFP_UPI_INTERRUPT_MASK_CDF_UPI_MISMATCH_IRQ_MASK;

    DAYTONA_WRM(VTSS_GFP_GFP_BASE_GFP_UPI_INTERRUPT_MASK(target), (enable ? mask : 0), mask);

    return VTSS_RC_OK;
}


static vtss_rc daytona_gfp_event_poll(vtss_state_t *vtss_state,
                                      const vtss_port_no_t  port_no, vtss_gfp_event_t *const status)
{
    u32 target;
    u32 new_sticky, sticky, mask, mode;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_GFP, &target));

    DAYTONA_RD(VTSS_GFP_GFP_BASE_GFP_CONTROL(target), &mode);    /* Read mode selector */
    mode = VTSS_X_GFP_GFP_BASE_GFP_CONTROL_CLIENT_MODE_SEL(mode);

    *status = 0;

    DAYTONA_RD(VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY(target), &sticky); /* Read sticky */
    DAYTONA_RD(VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_MASK(target), &mask);     /* Read mask */
    new_sticky  = sticky & mask;                                /* Newly detected sticky */
    DAYTONA_WR(VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY(target), new_sticky); /* Clear sticky */

    /* Event on new sticky set */
    if ((mode != 0x03) && (new_sticky & VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_SSF_IRQ_STICKY))   *status |= VTSS_GFP_SSF_EV;

    DAYTONA_RD(VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY(target), &sticky); /* Read sticky */
    DAYTONA_RD(VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_MASK(target), &mask);     /* Read mask */
    new_sticky  = sticky & mask;                                /* Newly detected sticky */
    DAYTONA_WR(VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY(target), new_sticky); /* Clear sticky */

    /* Event on new sticky set */
    if ((mode == 0x03) && (new_sticky & VTSS_M_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_SSF_DETECT_IRQ_STICKY))   *status |= VTSS_GFP_SSF_EV;
    if ((mode == 0x03) && (new_sticky & VTSS_M_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_UPI_VIOLATION_IRQ_STICKY)) *status |= VTSS_GFP_PPOS_CDF_UPM_EV; 

    DAYTONA_RD(VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY(target), &sticky); /* Read sticky */
    DAYTONA_RD(VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_MASK(target), &mask);     /* Read mask */
    new_sticky  = sticky & mask;                                /* Newly detected sticky */
    DAYTONA_WR(VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY(target), new_sticky); /* Clear sticky */

    /* Event on new sticky set */
    if (new_sticky & VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_LOS_IRQ_STICKY)                 *status |= VTSS_GFP_8B10B_LOS_EV;
    if (new_sticky & VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_LSYNC_IRQ_STICKY)               *status |= VTSS_GFP_8B10B_LSYNC_EV;
    if (new_sticky & VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_INVALID_CHARACTER_IRQ_STICKY)   *status |= VTSS_GFP_8B10B_TX_ILLEGAL_EV;
    if (new_sticky & VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_DISPARITY_ERROR_IRQ_STICKY)     *status |= VTSS_GFP_8B10B_TX_DISPARITY_EV;

    DAYTONA_RD(VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY(target), &sticky); /* Read sticky */
    DAYTONA_RD(VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_MASK(target), &mask);     /* Read mask */
    new_sticky  = sticky & mask;                                /* Newly detected sticky */
    DAYTONA_WR(VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY(target), new_sticky); /* Clear sticky */

    /* Event on new sticky set */
    if (new_sticky & (VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_RX_10B_ERR_DETECT_IRQ_STICKY | VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_RX_10B_ERR_REMOVAL_IRQ_STICKY))   *status |= VTSS_GFP_8B10B_ERROR_EV;
    if (new_sticky & VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_DISPARITY_CONTINUITY_ERROR_IRQ_STICKY)        *status |= VTSS_GFP_8B10B_RX_DISPARITY_EV;
    if (new_sticky & VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_INVALID_K_CHARACTER_IRQ_STICKY)               *status |= VTSS_GFP_8B10B_RX_ILLEGAL_EV;
    if ((mode == 0x00) && (new_sticky & VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_UPI_MISMATCH_IRQ_STICKY))  *status |= VTSS_GFP_CDF_UPM_EV;

    DAYTONA_RD(VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY(target), &sticky); /* Read sticky */
    DAYTONA_RD(VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_MASK(target), &mask);     /* Read mask */
    new_sticky  = sticky & mask;                                /* Newly detected sticky */
    DAYTONA_WR(VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY(target), new_sticky); /* Clear sticky */

    /* Event on new sticky set */
    if (new_sticky & VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_LOS_DETECT_IRQ_STICKY)           *status |= VTSS_GFP_CMF_LOS_SF_EV;
    if (new_sticky & VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_LSYNC_DETECT_IRQ_STICKY)         *status |= VTSS_GFP_CMF_LSYNC_SF_EV;
    if (new_sticky & VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_UNSUPPORTED_DETECT_IRQ_STICKY)   *status |= VTSS_GFP_CMF_UNSUPPORTED_EV;
    if (new_sticky & VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_INCONSISTENT_IRQ_STICKY)   *status |= VTSS_GFP_CMF_INCONSISTENT_EV;
    if (new_sticky & VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_ACCEPT_IRQ_STICKY)         *status |= VTSS_GFP_CMF_ACCEPTED_EV;
    if (new_sticky & VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_FRAME_MAPPED_ETHERNET_DETECT_IRQ_STICKY)  *status |= VTSS_GFP_FRAME_MAPPED_ETHERNET_EV;

    DAYTONA_RD(VTSS_GFP_GFP_BASE_GFP_UPI_INTERRUPT_STICKY(target), &sticky); /* Read sticky */
    DAYTONA_RD(VTSS_GFP_GFP_BASE_GFP_UPI_INTERRUPT_MASK(target), &mask);     /* Read mask */
    new_sticky  = sticky & mask;                                /* Newly detected sticky */
    DAYTONA_WR(VTSS_GFP_GFP_BASE_GFP_UPI_INTERRUPT_STICKY(target), new_sticky); /* Clear sticky */

    /* Event on new sticky set */
    if ((mode != 0x00) && (new_sticky & VTSS_M_GFP_GFP_BASE_GFP_UPI_INTERRUPT_STICKY_CDF_UPI_MISMATCH_IRQ_STICKY))     *status |= VTSS_GFP_CDF_UPM_EV;

    return VTSS_RC_OK;
}


static vtss_rc daytona_gfp_event_poll_without_mask(vtss_state_t *vtss_state,
                                                   const vtss_port_no_t  port_no, vtss_gfp_event_t *const status)
{
    u32 target;
    u32 sticky, mode;

    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_GFP, &target));

    DAYTONA_RD(VTSS_GFP_GFP_BASE_GFP_CONTROL(target), &mode);    /* Read mode selector */
    mode = VTSS_X_GFP_GFP_BASE_GFP_CONTROL_CLIENT_MODE_SEL(mode);

    *status = 0;

    DAYTONA_RD(VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY(target), &sticky); /* Read sticky */
    DAYTONA_WR(VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY(target), sticky); /* Clear sticky */

    /* Event on new sticky set */
    if ((mode != 0x03) && (sticky & VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_INTERRUPT_STICKY_SSF_IRQ_STICKY))   *status |= VTSS_GFP_SSF_EV;

    DAYTONA_RD(VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY(target), &sticky); /* Read sticky */
    DAYTONA_WR(VTSS_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY(target), sticky); /* Clear sticky */

    /* Event on new sticky set */
    if ((mode == 0x03) && (sticky & VTSS_M_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_SSF_DETECT_IRQ_STICKY))   *status |= VTSS_GFP_SSF_EV;
    if ((mode == 0x03) && (sticky & VTSS_M_GFP_GFPM_RX_PPOS_ISR_RX_PPOS_INTERRUPT_STICKY_UPI_VIOLATION_IRQ_STICKY)) *status |= VTSS_GFP_PPOS_CDF_UPM_EV;

    DAYTONA_RD(VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY(target), &sticky); /* Read sticky */
    DAYTONA_WR(VTSS_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY(target), sticky); /* Clear sticky */

    /* Event on new sticky set */
    if (sticky & VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_LOS_IRQ_STICKY)                 *status |= VTSS_GFP_8B10B_LOS_EV;
    if (sticky & VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_LSYNC_IRQ_STICKY)               *status |= VTSS_GFP_8B10B_LSYNC_EV;
    if (sticky & VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_INVALID_CHARACTER_IRQ_STICKY)   *status |= VTSS_GFP_8B10B_TX_ILLEGAL_EV;
    if (sticky & VTSS_M_GFP_GFPM_TX_8B10B_ISR_TX_8B10B_INTERRUPT_STICKY_DISPARITY_ERROR_IRQ_STICKY)     *status |= VTSS_GFP_8B10B_TX_DISPARITY_EV;

    DAYTONA_RD(VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY(target), &sticky); /* Read sticky */
    DAYTONA_WR(VTSS_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY(target), sticky); /* Clear sticky */
    /* Event on new sticky set */
    if (sticky & (VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_RX_10B_ERR_DETECT_IRQ_STICKY | VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_RX_10B_ERR_REMOVAL_IRQ_STICKY))   *status |= VTSS_GFP_8B10B_ERROR_EV;
    if (sticky & VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_DISPARITY_CONTINUITY_ERROR_IRQ_STICKY)        *status |= VTSS_GFP_8B10B_RX_DISPARITY_EV;
    if (sticky & VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_INVALID_K_CHARACTER_IRQ_STICKY)               *status |= VTSS_GFP_8B10B_RX_ILLEGAL_EV;
    if ((mode == 0x00) && (sticky & VTSS_M_GFP_GFPM_RX_8B10B_ISR_RX_8B10B_INTERRUPT_STICKY_UPI_MISMATCH_IRQ_STICKY))  *status |= VTSS_GFP_CDF_UPM_EV;

    DAYTONA_RD(VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY(target), &sticky); /* Read sticky */
    DAYTONA_WR(VTSS_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY(target), sticky); /* Clear sticky */

    /* Event on new sticky set */
    if (sticky & VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_LOS_DETECT_IRQ_STICKY)           *status |= VTSS_GFP_CMF_LOS_SF_EV;
    if (sticky & VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_LSYNC_DETECT_IRQ_STICKY)         *status |= VTSS_GFP_CMF_LSYNC_SF_EV;
    if (sticky & VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_UNSUPPORTED_DETECT_IRQ_STICKY)   *status |= VTSS_GFP_CMF_UNSUPPORTED_EV;
    if (sticky & VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_INCONSISTENT_IRQ_STICKY)   *status |= VTSS_GFP_CMF_INCONSISTENT_EV;
    if (sticky & VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_MGMT_ACCEPT_IRQ_STICKY)         *status |= VTSS_GFP_CMF_ACCEPTED_EV;
    if (sticky & VTSS_M_GFP_GFPM_RX_GFP_ISR_RX_GFP_CLIENT_INTERRUPT_STICKY_CLIENT_FRAME_MAPPED_ETHERNET_DETECT_IRQ_STICKY)  *status |= VTSS_GFP_FRAME_MAPPED_ETHERNET_EV;

    DAYTONA_RD(VTSS_GFP_GFP_BASE_GFP_UPI_INTERRUPT_STICKY(target), &sticky); /* Read sticky */
    DAYTONA_WR(VTSS_GFP_GFP_BASE_GFP_UPI_INTERRUPT_STICKY(target), sticky); /* Clear sticky */

    /* Event on new sticky set */
    if ((mode != 0x00) && (sticky & VTSS_M_GFP_GFP_BASE_GFP_UPI_INTERRUPT_STICKY_CDF_UPI_MISMATCH_IRQ_STICKY))     *status |= VTSS_GFP_CDF_UPM_EV;

    return VTSS_RC_OK;
}


/* ================================================================= *
 *  Test/Debug Config
 * ================================================================= */


/* ================================================================= *
 *  Utilities and internal
 * ================================================================= */
/*
 * \brief Reset Gfp configuration on a port
 *
 * \param port_no [IN] Port number
 *
 * \return Return code
 */
vtss_rc vtss_daytona_gfp_restart_conf_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    vtss_rc rc;

    rc = daytona_gfp_single_err_corr_set(vtss_state, port_no);
    rc = daytona_gfp_cmf_auto_sf_gen_set(vtss_state, port_no);
    rc = daytona_gfp_cmf_forced_gen_set(vtss_state, port_no);
    rc = daytona_gfp_cdf_fcs_insert_set(vtss_state, port_no);
    rc = daytona_gfp_cdf_upi_set(vtss_state, port_no);
    rc = daytona_gfp_8b10b_control_code_set(vtss_state, port_no);
    VTSS_D("port_no %d, gfp restart conf set, rx %x", port_no, rc);
    return VTSS_RC_OK;
}

/*
 * \brief static initialization of default configuration
 */
static vtss_gfp_err_corr_t err_corr_cfg_def = {
    .core_enable = FALSE,
    .pay_enable  = FALSE,
    .trans_enable= FALSE,
};
static vtss_gfp_cdf_fcs_insert_t fcs_insert_cfg_def = {
    .enable = FALSE,
};
static vtss_gfp_cdf_upi_t cdf_upi_cfg_def = {
    .upi         = 0,
    .ordered_upi = 0,         
};
static vtss_gfp_cmf_auto_sf_t cmf_auto_sf_cfg_def = {
    .los_sf   = VTSS_GFP_CMF_SF_NONE,
    .lsync_sf = VTSS_GFP_CMF_SF_NONE,
};
static vtss_gfp_cmf_forced_t cmf_forced_cfg_def = {
    .enable   = FALSE,
    .interval = 0,
    .upi      = 0,
    .pti      = 0,
};
static vtss_gfp_8b10b_control_code_t control_code_cfg_def = {
    .force_los   = FALSE,
    .force_lsync = FALSE,
    .err_code    = VTSS_GFP_10B_ERR_ERROR_CODE,
};

/*
 * \brief Set default values in GFP state
 */
vtss_rc vtss_daytona_gfp_default_conf_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    vtss_state->gfp_state[port_no].err_corr = err_corr_cfg_def;
    vtss_state->gfp_state[port_no].fcs_insert = fcs_insert_cfg_def;
    vtss_state->gfp_state[port_no].cdf_upi = cdf_upi_cfg_def;
    vtss_state->gfp_state[port_no].cmf_auto_sf = cmf_auto_sf_cfg_def;
    vtss_state->gfp_state[port_no].cmf_forced = cmf_forced_cfg_def;
    vtss_state->gfp_state[port_no].control_code = control_code_cfg_def;
     
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
vtss_rc vtss_daytona_inst_gfp_create(vtss_state_t *vtss_state)
{
    vtss_cil_func_t *func = &vtss_state->cil;

    /* Daytona GFP functions */
    func->gfp_single_err_corr_set = daytona_gfp_single_err_corr_set;
    func->gfp_cmf_auto_sf_gen_set = daytona_gfp_cmf_auto_sf_gen_set;
    func->gfp_cmf_forced_gen_set = daytona_gfp_cmf_forced_gen_set;
    func->gfp_cdf_fcs_insert_set = daytona_gfp_cdf_fcs_insert_set;
    func->gfp_cdf_upi_set = daytona_gfp_cdf_upi_set;
    func->gfp_8b10b_control_code_set = daytona_gfp_8b10b_control_code_set;
    func->gfp_cmf_acc_upi_pti_get = daytona_gfp_cmf_acc_upi_pti_get;
    func->gfp_cdf_acc_upi_get = daytona_gfp_cdf_acc_upi_get;
    func->gfp_defects_get = daytona_gfp_defects_get;
    func->gfp_perf_get = daytona_gfp_perf_get;
    func->gfp_event_enable = daytona_gfp_event_enable;
    func->gfp_event_poll = daytona_gfp_event_poll;
    func->gfp_event_poll_without_mask = daytona_gfp_event_poll_without_mask;

    return VTSS_RC_OK;
}

#endif /* VTSS_ARCH_DAYTONA */


