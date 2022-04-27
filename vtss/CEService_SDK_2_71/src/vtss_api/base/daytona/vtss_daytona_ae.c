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

// Avoid "vtss_api.h not used in module vtss_daytona_ae.c"
/*lint --e{766} */

#include "vtss_api.h"
#if defined(VTSS_ARCH_DAYTONA)
#include "../ail/vtss_state.h"
#include "vtss_daytona.h"
#include "vtss_daytona_regs.h"
#include "vtss_daytona_basics.h"
#include "vtss_daytona_ae.h"
#include "vtss_daytona_regs_xfi.h"


#if defined(VTSS_FEATURE_AE)

/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */

static vtss_rc daytona_ae_config_set(const vtss_ae_lane_t lane_no)
{
    u32 target;    
   

    VTSS_RC(daytona_port_2_target(DAYTONA_LANETOPORT(lane_no), DAYTONA_BLOCK_XFI, &target));

    if(lane_no % 2){
        DAYTONA_WRM(VTSS_XFI_AE_REGS_A_REG_MODE_A(target),VTSS_F_XFI_AE_REGS_A_REG_MODE_A_MDIO_OVR(vtss_state->ae_state[lane_no].ae_cfg.MDIO_override),VTSS_M_XFI_AE_REGS_A_REG_MODE_A_MDIO_OVR);
        DAYTONA_WRM(VTSS_XFI_AE_REGS_A_REG_MODE_A(target),VTSS_F_XFI_AE_REGS_A_REG_MODE_A_MWT_DISABLE(vtss_state->ae_state[lane_no].ae_cfg.max_wait_timer_disable),VTSS_M_XFI_AE_REGS_A_REG_MODE_A_MWT_DISABLE);
        DAYTONA_WRM(VTSS_XFI_AE_REGS_A_REG_MODE_A(target),VTSS_F_XFI_AE_REGS_A_REG_MODE_A_MANUAL_RDY(vtss_state->ae_state[lane_no].ae_cfg.max_wait_timer_disable),VTSS_M_XFI_AE_REGS_A_REG_MODE_A_MANUAL_RDY);
        DAYTONA_WRM(VTSS_XFI_AE_REGS_A_REG_MODE_A(target),VTSS_F_XFI_AE_REGS_A_REG_MODE_A_MAN_DEC(vtss_state->ae_state[lane_no].ae_cfg.max_wait_timer_disable),VTSS_M_XFI_AE_REGS_A_REG_MODE_A_MAN_DEC);

        //fsm config
        DAYTONA_WRM(VTSS_XFI_AE_REGS_A_FSM_CTL_A(target),VTSS_F_XFI_AE_REGS_A_FSM_CTL_A_FSM_TR_EN(vtss_state->ae_state[lane_no].ae_cfg.fsm_cfg.training_enable),VTSS_M_XFI_AE_REGS_A_FSM_CTL_A_FSM_TR_EN);
        DAYTONA_WRM(VTSS_XFI_AE_REGS_A_FSM_CTL_A(target),VTSS_F_XFI_AE_REGS_A_FSM_CTL_A_FSM_CU_DIS(vtss_state->ae_state[lane_no].ae_cfg.fsm_cfg.disable_coeff_update_fsm),VTSS_M_XFI_AE_REGS_A_FSM_CTL_A_FSM_CU_DIS);

        //prbs
        DAYTONA_WRM(VTSS_XFI_AE_REGS_A_PRBS_CTL_A(target),VTSS_F_XFI_AE_REGS_A_PRBS_CTL_A_PRBS_GEN_OFF(vtss_state->ae_state[lane_no].ae_cfg.prbs_cfg.gen_off),VTSS_M_XFI_AE_REGS_A_PRBS_CTL_A_PRBS_GEN_OFF);
        DAYTONA_WRM(VTSS_XFI_AE_REGS_A_PRBS_CTL_A(target),VTSS_F_XFI_AE_REGS_A_PRBS_CTL_A_PRBS_CHK_OFF(vtss_state->ae_state[lane_no].ae_cfg.prbs_cfg.checker_off),VTSS_M_XFI_AE_REGS_A_PRBS_CTL_A_PRBS_CHK_OFF);
    }else{
        DAYTONA_WRM(VTSS_XFI_AE_REGS_B_REG_MODE_B(target),VTSS_F_XFI_AE_REGS_B_REG_MODE_B_MDIO_OVR(vtss_state->ae_state[lane_no].ae_cfg.MDIO_override),VTSS_M_XFI_AE_REGS_B_REG_MODE_B_MDIO_OVR);
        DAYTONA_WRM(VTSS_XFI_AE_REGS_B_REG_MODE_B(target),VTSS_F_XFI_AE_REGS_B_REG_MODE_B_MWT_DISABLE(vtss_state->ae_state[lane_no].ae_cfg.max_wait_timer_disable),VTSS_M_XFI_AE_REGS_B_REG_MODE_B_MWT_DISABLE);
        DAYTONA_WRM(VTSS_XFI_AE_REGS_B_REG_MODE_B(target),VTSS_F_XFI_AE_REGS_B_REG_MODE_B_MANUAL_RDY(vtss_state->ae_state[lane_no].ae_cfg.max_wait_timer_disable),VTSS_M_XFI_AE_REGS_B_REG_MODE_B_MANUAL_RDY);
        DAYTONA_WRM(VTSS_XFI_AE_REGS_B_REG_MODE_B(target),VTSS_F_XFI_AE_REGS_B_REG_MODE_B_MAN_DEC(vtss_state->ae_state[lane_no].ae_cfg.max_wait_timer_disable),VTSS_M_XFI_AE_REGS_B_REG_MODE_B_MAN_DEC);

        //fsm config
        DAYTONA_WRM(VTSS_XFI_AE_REGS_B_FSM_CTL_B(target),VTSS_F_XFI_AE_REGS_B_FSM_CTL_B_FSM_TR_EN(vtss_state->ae_state[lane_no].ae_cfg.fsm_cfg.training_enable),VTSS_M_XFI_AE_REGS_B_FSM_CTL_B_FSM_TR_EN);
        DAYTONA_WRM(VTSS_XFI_AE_REGS_B_FSM_CTL_B(target),VTSS_F_XFI_AE_REGS_B_FSM_CTL_B_FSM_CU_DIS(vtss_state->ae_state[lane_no].ae_cfg.fsm_cfg.disable_coeff_update_fsm),VTSS_M_XFI_AE_REGS_B_FSM_CTL_B_FSM_CU_DIS);

        //prbs
        DAYTONA_WRM(VTSS_XFI_AE_REGS_B_PRBS_CTL_B(target),VTSS_F_XFI_AE_REGS_B_PRBS_CTL_B_PRBS_GEN_OFF(vtss_state->ae_state[lane_no].ae_cfg.prbs_cfg.gen_off),VTSS_M_XFI_AE_REGS_B_PRBS_CTL_B_PRBS_GEN_OFF);
        DAYTONA_WRM(VTSS_XFI_AE_REGS_B_PRBS_CTL_B(target),VTSS_F_XFI_AE_REGS_B_PRBS_CTL_B_PRBS_CHK_OFF(vtss_state->ae_state[lane_no].ae_cfg.prbs_cfg.checker_off),VTSS_M_XFI_AE_REGS_B_PRBS_CTL_B_PRBS_CHK_OFF);
    }

    

   return VTSS_RC_OK;
}


static vtss_rc daytona_ae_coeff_updt_req_set(const vtss_ae_lane_t lane_no, const u32 req)
{
    u32 target;        
    VTSS_RC(daytona_port_2_target(DAYTONA_LANETOPORT(lane_no), DAYTONA_BLOCK_XFI, &target));

    //write the whole thing
    if(lane_no%2){
        DAYTONA_WRF(VTSS_XFI_AE_REGS_A_RX_COEF_REQ_A(target),0,16,(u16)req);
    }else{
        DAYTONA_WRF(VTSS_XFI_AE_REGS_B_RX_COEF_REQ_B(target),0,16,(u16)req);
    }
    return VTSS_RC_OK;
}


static vtss_rc daytona_ae_coeff_updt_stat_get(const vtss_ae_lane_t lane_no,u32  *const stat)
{
    u32 target;        
    VTSS_RC(daytona_port_2_target(DAYTONA_LANETOPORT(lane_no), DAYTONA_BLOCK_XFI, &target));

    //read the whole thing
    if(lane_no%2){
        DAYTONA_RDF(VTSS_XFI_AE_REGS_A_RX_COEF_STAT_A(target),0,16,stat);
    }else{
        DAYTONA_RDF(VTSS_XFI_AE_REGS_B_RX_COEF_STAT_B(target),0,16,stat);
    }
    return VTSS_RC_OK;
}


static vtss_rc daytona_ae_init_rx(const vtss_ae_lane_t lane_no)
{
    u32 target;        
    VTSS_RC(daytona_port_2_target(DAYTONA_LANETOPORT(lane_no), DAYTONA_BLOCK_XFI, &target));    
    DAYTONA_RDF(VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(target,(lane_no%2)),0,16,&vtss_state->ae_state[lane_no].ae_protocol_data.reg_save[0]);
    DAYTONA_RDF(VTSS_XFI_HSS11G_RX_HSS11G_DFE_CONTROL(target,(lane_no%2)),0,16,&vtss_state->ae_state[lane_no].ae_protocol_data.reg_save[0]);

    DAYTONA_WRF(VTSS_XFI_HSS11G_RX_HSS11G_DFE_FUNCTION_CONTROL(target,(lane_no%2)),0,16,(u16)0x7FFD);
    
    DAYTONA_WRM(VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE(target,(lane_no%2)),VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_DMSEL(vtss_state->ae_state[lane_no].ae_cfg.dfe_mode),VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_CONFIGURATION_MODE_DMSEL);
    DAYTONA_WRM(VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_1(target,(lane_no%2)),VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_1_PEAK(vtss_state->ae_state[lane_no].ae_cfg.peak_amplitude),VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_1_PEAK);

    DAYTONA_WRF(VTSS_XFI_HSS11G_RX_HSS11G_DFE_CONTROL(target,(lane_no%2)),0,16,(u16)0x1010);
    DAYTONA_WRF(VTSS_XFI_HSS11G_RX_HSS11G_DFE_CONTROL(target,(lane_no%2)),0,16,(u16)0x1011);
    DAYTONA_WRF(VTSS_XFI_HSS11G_RX_HSS11G_DFE_CONTROL(target,(lane_no%2)),0,16,(u16)0x1010);

    DAYTONA_WRM(VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(target,(lane_no%2)),VTSS_F_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL(0),VTSS_M_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL_SDLVL);
    

    return VTSS_RC_OK;
}


static vtss_rc daytona_ae_init_tx(const vtss_ae_lane_t lane_no)
{
    u32 target;        
    VTSS_RC(daytona_port_2_target(DAYTONA_LANETOPORT(lane_no), DAYTONA_BLOCK_XFI, &target));
    DAYTONA_WRF(VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_COEFFICIENT_CONTROL(target,(lane_no%2)),0,16,(u16)0x30);
    DAYTONA_WRF(VTSS_XFI_HSS11G_TX_HSS11G_TRANSMIT_DRIVER_MODE_CONTROL(target,(lane_no%2)),0,16,(u16)0x01);
    return VTSS_RC_OK;
}

static vtss_rc daytona_ae_return_rx_cfg(const vtss_ae_lane_t lane_no)
{
    u32 target;        
    VTSS_RC(daytona_port_2_target(DAYTONA_LANETOPORT(lane_no), DAYTONA_BLOCK_XFI, &target));
    DAYTONA_WRF(VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_SIGDET_CONTROL(target,(lane_no%2)),0,16,vtss_state->ae_state[lane_no].ae_protocol_data.reg_save[0]);
    DAYTONA_WRF(VTSS_XFI_HSS11G_RX_HSS11G_DFE_CONTROL(target,(lane_no%2)),0,16,vtss_state->ae_state[lane_no].ae_protocol_data.reg_save[0]);
    return VTSS_RC_OK;

}

static vtss_rc daytona_ae_get_eye_data(const vtss_ae_lane_t lane_no,const vtss_ae_eye_data_t  cmd,vtss_ae_eye_data_val_t *const retval)
{
    u32 target;    
    VTSS_RC(daytona_port_2_target(DAYTONA_LANETOPORT(lane_no), DAYTONA_BLOCK_XFI, &target));

    switch(cmd){
    case AMIN:
        DAYTONA_RDF(VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_DACA_MIN(target,(lane_no%2)),0,8,retval);
        break;
    case GAIN:
        DAYTONA_RDF(VTSS_XFI_HSS11G_RX_HSS11G_RECEIVER_VGA_CONTROL_2(target,(lane_no%2)),0,4,retval);
        break;
    case H1T:
        DAYTONA_RDF(VTSS_XFI_HSS11G_RX_HSS11G_DYNAMIC_AMPLITUDE_CENTERING_DAC_AND_DYNAMIC_PEAKING_CONTROL_DPC(target,(lane_no%2)),4,3,retval);
        break;
    case DATA_EDGE:
        DAYTONA_RDF(VTSS_XFI_HSS11G_RX_HSS11G_DFE_DATA_AND_EDGE_SAMPLE(target,(lane_no%2)),0,16,retval);
        break;
    case DFE_SAMPLE:
        DAYTONA_WRF(VTSS_XFI_HSS11G_RX_HSS11G_DFE_CONTROL(target,(lane_no%2)),0,16,(u16)0x1010);
        DAYTONA_WRF(VTSS_XFI_HSS11G_RX_HSS11G_DFE_CONTROL(target,(lane_no%2)),0,16,(u16)0x5010);
        DAYTONA_RDF(VTSS_XFI_HSS11G_RX_HSS11G_DFE_CONTROL(target,(lane_no%2)),15,1,retval);
        break;

    default:
        return VTSS_RC_ERROR;
    }

    return VTSS_RC_OK;

}




/* ================================================================= *
 *  State Reporting
 * ================================================================= */

/* ================================================================= *
 *  Performance Primitives
 * ================================================================= */



/* ================================================================= *
 *  Utilities and internal
 * ================================================================= */


/**
 * \brief Create instance (set up function pointers).
 *
 * \param inst [IN]      Target instance reference.
 * \param port_map [IN]  Port map array.
 *
 * \return Return code.
 **/
vtss_rc vtss_daytona_inst_ae_create(void)
{
    vtss_cil_func_t *func = &vtss_state->cil_func;

    /* Daytona AE functions */    
    func->ae_set_config = daytona_ae_config_set;
    func->ae_set_coeff_updt_req = daytona_ae_coeff_updt_req_set;
    func->ae_get_coeff_stat_req = daytona_ae_coeff_updt_stat_get;
    func->ae_get_eye_data = daytona_ae_get_eye_data;
    func->ae_init_rx = daytona_ae_init_rx;
    func->ae_init_tx = daytona_ae_init_tx;
    func->ae_return_rx_cfg = daytona_ae_return_rx_cfg;
   
    return VTSS_RC_OK;
}

#endif /* VTSS_FEATURE_AE */
#endif /* VTSS_ARCH_DAYTONA */



