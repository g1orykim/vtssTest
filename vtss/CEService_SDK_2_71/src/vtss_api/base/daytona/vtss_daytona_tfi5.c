/*

 Vitesse API software.

 Copyright (c) 2002-2009 Vitesse Semiconductor Corporation "Vitesse". All
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

// Avoid "vtss_api.h not used in module vtss_daytona_tfi5.c"
/*lint --e{766} */

#include "vtss_api.h"
#if defined(VTSS_ARCH_DAYTONA) && defined(VTSS_FEATURE_TFI5)
#include "../vtss_state.h"
#include "vtss_daytona.h"
#include "vtss_daytona_regs.h"
#include "vtss_daytona_basics.h"
#include "vtss_daytona_tfi5.h"
#include "vtss_daytona_regs_tfi_5.h"



/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */

static vtss_rc daytona_tfi5_config_set(const vtss_port_no_t port_no)
{
    u32 target;    
    u32 reg_val=0;
    u32 reg_mask=0;

    VTSS_RC(daytona_port_2_target(port_no, DAYTONA_BLOCK_UPI, &target));

    //global ctl   
    (vtss_state->tfi5_state[port_no].tfi5_cfg.dual_rate == TRUE) ? (reg_val |= VTSS_M_TFI_5_GLOBAL_GLOBAL_CONTROL_RTFI_EN) : (reg_val &= ~VTSS_M_TFI_5_GLOBAL_GLOBAL_CONTROL_RTFI_EN);
    (vtss_state->tfi5_state[port_no].tfi5_cfg.rx_to_tx_loopback == TRUE) ? (reg_val |= VTSS_M_TFI_5_GLOBAL_GLOBAL_CONTROL_LOOP_EN) : (reg_val &= ~VTSS_M_TFI_5_GLOBAL_GLOBAL_CONTROL_LOOP_EN);
    reg_mask = (VTSS_M_TFI_5_GLOBAL_GLOBAL_CONTROL_RTFI_EN|VTSS_M_TFI_5_GLOBAL_GLOBAL_CONTROL_LOOP_EN);
    DAYTONA_WRM(VTSS_TFI_5_GLOBAL_GLOBAL_CONTROL(target),reg_val,reg_mask);

    //tx ctl    
    (vtss_state->tfi5_state[port_no].tfi5_cfg.tx_config.b1_passthru) ? reg_val |= VTSS_M_TFI_5_TX_TX_CONTROL_B1_CALC_INH: (reg_val &= ~VTSS_M_TFI_5_TX_TX_CONTROL_B1_CALC_INH);
    (vtss_state->tfi5_state[port_no].tfi5_cfg.tx_config.fixed_j0z0) ? reg_val |= VTSS_M_TFI_5_TX_TX_CONTROL_J0_Z0_FILL_ENABLE: (reg_val &= ~VTSS_M_TFI_5_TX_TX_CONTROL_J0_Z0_FILL_ENABLE);
    reg_mask = (VTSS_M_TFI_5_TX_TX_CONTROL_B1_CALC_INH|VTSS_M_TFI_5_TX_TX_CONTROL_J0_Z0_FILL_ENABLE);
    DAYTONA_WRM(VTSS_TFI_5_TX_TX_CONTROL(target),reg_val,reg_mask);  
    
    //rx ctl
    //nothing yet

   return VTSS_RC_OK;
}

static vtss_rc daytona_tfi5_reset(const vtss_port_no_t port_no,BOOL enable)
{
    //no reset
    return VTSS_RC_OK;
}

static vtss_rc daytona_tfi5_enable(const vtss_port_no_t port_no)
{
    u32 target;
    VTSS_RC(daytona_port_2_target(port_no, DAYTONA_BLOCK_UPI, &target));

    
    
    return VTSS_RC_OK;    
}

static vtss_rc daytona_tfi5_loopback(const vtss_port_no_t port_no)
{
    u32 target;u32 reg_val=0;
    VTSS_RC(daytona_port_2_target(port_no, DAYTONA_BLOCK_UPI, &target));

    (vtss_state->tfi5_state[port_no].tfi5_cfg.rx_to_tx_loopback == TRUE) ? reg_val |= VTSS_M_TFI_5_GLOBAL_GLOBAL_CONTROL_LOOP_EN: (reg_val &= ~VTSS_M_TFI_5_GLOBAL_GLOBAL_CONTROL_LOOP_EN);    
    DAYTONA_WRM(VTSS_TFI_5_GLOBAL_GLOBAL_CONTROL(target),reg_val,VTSS_M_TFI_5_GLOBAL_GLOBAL_CONTROL_LOOP_EN);    
    
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
vtss_rc vtss_daytona_inst_tfi5_create(void)
{
    vtss_cil_func_t *func = &vtss_state->cil_func;

    /* Daytona UPI functions */    
    func->tfi5_set_config = daytona_tfi5_config_set;
    func->tfi5_set_enable = daytona_tfi5_enable;
    func->tfi5_reset = daytona_tfi5_reset;    
    func->tfi5_set_loopback = daytona_tfi5_loopback;    
    return VTSS_RC_OK;
}


#if 0
/**
 * \brief Error insert
 *
 * \param inst [IN]      Target instance reference.
 * \param port_no [IN]  Port map array.
 * \param enable [IN]  enable/disable
 * \param err [IN]  err type 
 *
 * \return Return code.
 **/
static vtss_rc vtss_daytona_tfi5_error_insert(const vtss_port_no_t port_no,BOOL enable,
                          vtss_tfi5_err_ins_t err)
{
    u32 target; u32 reg_val=0;
    VTSS_RC(daytona_port_2_target(port_no, DAYTONA_BLOCK_UPI, &target));

    switch(err){
    case tfi5_ais:
        (enable == TRUE) ? reg_val |= VTSS_M_TFI_5_TX_TX_CONTROL_AIS_INS:  (reg_val &= ~VTSS_M_TFI_5_TX_TX_CONTROL_AIS_INS);    
        DAYTONA_WRM(VTSS_TFI_5_TX_TX_CONTROL(target),reg_val,VTSS_M_TFI_5_TX_TX_CONTROL_AIS_INS);    
        break;

    case tfi5_b1:
        (enable == TRUE) ? reg_val |= VTSS_M_TFI_5_TX_TX_CONTROL_B1_ERR_ALL: (reg_val &= ~VTSS_M_TFI_5_TX_TX_CONTROL_B1_ERR_ALL);    
        DAYTONA_WRM(VTSS_TFI_5_TX_TX_CONTROL(target),reg_val,VTSS_M_TFI_5_TX_TX_CONTROL_B1_ERR_ALL);    
        break;

    case tfi5_a1a2:
        (enable == TRUE) ? reg_val |= VTSS_M_TFI_5_TX_TX_CONTROL_A1A2_ERR_ALL: (reg_val &= ~VTSS_M_TFI_5_TX_TX_CONTROL_A1A2_ERR_ALL);    
        DAYTONA_WRM(VTSS_TFI_5_TX_TX_CONTROL(target),reg_val,VTSS_M_TFI_5_TX_TX_CONTROL_A1A2_ERR_ALL);    
        break;

    default:
        return VTSS_RC_ERROR;
    }
    
    
    return VTSS_RC_OK;    
}
#endif /* 0 */
#endif /* VTSS_ARCH_DAYTONA */



