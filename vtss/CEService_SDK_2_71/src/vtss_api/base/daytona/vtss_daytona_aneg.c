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

// Avoid "vtss_api.h not used in module vtss_daytona_aneg.c"
/*lint --e{766} */

#include "vtss_api.h"
#if defined(VTSS_ARCH_DAYTONA)
#include "../ail/vtss_state.h"
#include "vtss_daytona.h"
#include "vtss_daytona_regs.h"
#include "vtss_daytona_basics.h"
#include "vtss_daytona_aneg.h"
#include "vtss_daytona_regs_xfi.h"
#include "vtss_daytona_regs_pcs_xaui.h"


#if defined(VTSS_FEATURE_ANEG)

/** \brief Daytona register defines not found in the regular reg. definition file*/
#define     DAYTONA_ANEG_RESET_BIT_PCS_XAUI  (1U << 31)
#define     DAYTONA_ANEG_RESET_BIT_XFI  (1U << 31)


/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */
static vtss_rc daytona_aneg_get_target(const vtss_port_no_t port_no, daytona_block_id_t *const block, u32 *const target);
static vtss_rc daytona_aneg_ability_set(const vtss_port_no_t port_no);
static vtss_rc daytona_aneg_timeout_check(u32 reg_addr,u32 mask,u32 cmp);

static vtss_rc daytona_aneg_config_set(const vtss_port_no_t port_no)
{
    u32 target;
    daytona_block_id_t block;
    u32 reg_val=0;u32 reg_mask=0;
        
    VTSS_RC(daytona_aneg_get_target(port_no,&block,&target));   
   
    if(block == DAYTONA_BLOCK_XFI){
        (vtss_state->aneg_state[port_no].aneg_cfg.ctrl_ob == TRUE) ?   (reg_val |= VTSS_M_XFI_ANEG_CFG_ANEG_CFG_ANEG_OB_CTRL_DIS):reg_val;
        reg_val |= VTSS_F_XFI_ANEG_CFG_ANEG_CFG_PD_TIMER_10GKR(vtss_state->aneg_state[port_no].aneg_cfg.parallel_det_wait_time);
        
        reg_mask |= (VTSS_M_XFI_ANEG_CFG_ANEG_CFG_ANEG_OB_CTRL_DIS|VTSS_M_XFI_ANEG_CFG_ANEG_CFG_PD_TIMER_10GKR);
        
        //ANEG_CFG register
        DAYTONA_WRM(VTSS_XFI_ANEG_CFG_ANEG_CFG(target),reg_val,reg_mask);
        
        VTSS_RC(daytona_aneg_ability_set(port_no));
        
    }else if(block == DAYTONA_BLOCK_PCS_XAUI){
        //PCS XAUI is the target
        (vtss_state->aneg_state[port_no].aneg_cfg.ctrl_ob == TRUE) ?   (reg_val |= VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_CFG_ANEG_OB_CTRL_DIS):reg_val;
        reg_val |= VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_CFG_PD_TIMER_10GKX4(vtss_state->aneg_state[port_no].aneg_cfg.parallel_det_wait_time);
        
        reg_mask = (VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_CFG_ANEG_OB_CTRL_DIS|VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_CFG_PD_TIMER_10GKX4);
        //ANEG_CFG register
        DAYTONA_WRM(VTSS_PCS_XAUI_ANEG_CFG_ANEG_CFG(target),reg_val,reg_mask);
        
        VTSS_RC(daytona_aneg_ability_set(port_no));
    }   
    
   return VTSS_RC_OK;
}

static vtss_rc daytona_aneg_ability_set(const vtss_port_no_t port_no)
{

    u32 target;
    daytona_block_id_t block;
    u32 reg_val=0;u32 reg_mask=0;
       
    VTSS_RC(daytona_aneg_get_target(port_no,&block,&target));   

    if(block == DAYTONA_BLOCK_XFI){       
        
        (vtss_state->aneg_state[port_no].aneg_ability.rf == TRUE) ? (reg_val |= VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_RF):reg_val;
        (vtss_state->aneg_state[port_no].aneg_ability.np == TRUE) ? (reg_val |= VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_NP):reg_val;
        (vtss_state->aneg_state[port_no].aneg_ability.cap_1gkx == TRUE) ? (reg_val |= VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_1GKX):reg_val;
        (vtss_state->aneg_state[port_no].aneg_ability.cap_10gkx4 == TRUE) ? (reg_val |= VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_10GKX4):reg_val;
        (vtss_state->aneg_state[port_no].aneg_ability.cap_10gkr == TRUE) ? (reg_val |= VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_10GKR):reg_val;
        
        reg_val |= VTSS_F_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_SEL_FIELD(vtss_state->aneg_state[port_no].aneg_ability.sel_field);
        reg_val |= VTSS_F_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_ECHOED_NONCE(vtss_state->aneg_state[port_no].aneg_ability.echoed_nonce);
        reg_val |= VTSS_F_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_PAUSE(vtss_state->aneg_state[port_no].aneg_ability.pause);
        reg_val |= VTSS_F_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_TX_NONCE(vtss_state->aneg_state[port_no].aneg_ability.tx_nonce);
        reg_val |= VTSS_F_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_ADV_ABIL_LSB(vtss_state->aneg_state[port_no].aneg_ability.lsb_reserved);        
        
        reg_mask = (VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_RF|VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_NP|VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_1GKX|VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_10GKX4|
                    VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_10GKR|VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_SEL_FIELD|VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_ECHOED_NONCE|
                    VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_PAUSE|VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_TX_NONCE|VTSS_M_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0_ADV_ABIL_LSB);

        //ADV ABILITY register
        DAYTONA_WRM(VTSS_XFI_ANEG_CFG_ANEG_ADV_ABILITY_0(target),reg_val,reg_mask);
    }else if(block == DAYTONA_BLOCK_PCS_XAUI){
        
        (vtss_state->aneg_state[port_no].aneg_ability.rf == TRUE) ? (reg_val |= VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_RF):reg_val;
        (vtss_state->aneg_state[port_no].aneg_ability.np == TRUE) ? (reg_val |= VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_NP):reg_val;
        (vtss_state->aneg_state[port_no].aneg_ability.cap_1gkx == TRUE) ? (reg_val |= VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_1GKX):reg_val;
        (vtss_state->aneg_state[port_no].aneg_ability.cap_10gkx4 == TRUE) ? (reg_val |= VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_10GKX4):reg_val;
        (vtss_state->aneg_state[port_no].aneg_ability.cap_10gkr == TRUE) ? (reg_val |= VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_10GKR):reg_val;
        
        reg_val |= VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_SEL_FIELD(vtss_state->aneg_state[port_no].aneg_ability.sel_field);
        reg_val |= VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_ECHOED_NONCE(vtss_state->aneg_state[port_no].aneg_ability.echoed_nonce);
        reg_val |= VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_PAUSE(vtss_state->aneg_state[port_no].aneg_ability.pause);
        reg_val |= VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_TX_NONCE(vtss_state->aneg_state[port_no].aneg_ability.tx_nonce);
        reg_val |= VTSS_F_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_ADV_ABIL_LSB(vtss_state->aneg_state[port_no].aneg_ability.lsb_reserved);
        
        reg_mask = (VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_RF|VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_NP|VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_1GKX|VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_10GKX4|
                    VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_CAP_10GKR|VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_SEL_FIELD|VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_ECHOED_NONCE|
                    VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_PAUSE|VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_TX_NONCE|VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0_ADV_ABIL_LSB);

        //ADV ABILITY register
        DAYTONA_WRM(VTSS_PCS_XAUI_ANEG_CFG_ANEG_ADV_ABILITY_0(target),reg_val,reg_mask);       
    }
        return VTSS_RC_OK;
}

static vtss_rc daytona_aneg_restart(const vtss_port_no_t port_no)
{
    u32 target;
    daytona_block_id_t block;    
    u32 reg_addr=0;u32 bitfield=0;
        
    VTSS_RC(daytona_aneg_get_target(port_no,&block,&target));
    if(block == DAYTONA_BLOCK_XFI){
        reg_addr = VTSS_XFI_ANEG_CFG_ANEG_CFG(target);
        bitfield = VTSS_M_XFI_ANEG_CFG_ANEG_CFG_RESTART_ANEG_ONE_SHOT;
    }
    else if(block == DAYTONA_BLOCK_PCS_XAUI){
        reg_addr = VTSS_PCS_XAUI_ANEG_CFG_ANEG_CFG(target);
        bitfield = VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_CFG_RESTART_ANEG_ONE_SHOT;
    }else {  
        VTSS_E("daytona:aneg:invalid block");
        return VTSS_RC_ERROR;
    }

    //check if bit is clear (from prior) before setting.
    VTSS_RC(daytona_aneg_timeout_check(reg_addr,bitfield,0));  
    
    DAYTONA_WRM(reg_addr, bitfield,bitfield);

    //check if bit clears.    
    VTSS_RC(daytona_aneg_timeout_check(reg_addr,bitfield,0));
    return VTSS_RC_OK;
}

static vtss_rc daytona_aneg_reset(const vtss_port_no_t port_no)
{

    u32 target;
    daytona_block_id_t block;
    u32 reg_addr=0;u32 bitfield=0;
        
    VTSS_RC(daytona_aneg_get_target(port_no,&block,&target));
    if(block == DAYTONA_BLOCK_XFI){
        reg_addr = VTSS_XFI_ANEG_CFG_ANEG_CFG(target);        
        bitfield = DAYTONA_ANEG_RESET_BIT_XFI;
    }
    else if(block == DAYTONA_BLOCK_PCS_XAUI){
        reg_addr = VTSS_PCS_XAUI_ANEG_CFG_ANEG_CFG(target);
        bitfield = DAYTONA_ANEG_RESET_BIT_PCS_XAUI;
    }else {  
        VTSS_E("daytona:aneg:invalid block");
        return VTSS_RC_ERROR;
    }

    //check if bit is clear (from prior) before setting.
    VTSS_RC(daytona_aneg_timeout_check(reg_addr,bitfield,0)); 

    DAYTONA_WRM(reg_addr, bitfield,bitfield);

    //check if bit clears.    
     VTSS_RC(daytona_aneg_timeout_check(reg_addr,bitfield,0));
     
    
    return VTSS_RC_OK;
    
}

static vtss_rc daytona_aneg_enable(const vtss_port_no_t port_no)
{
    u32 target;
    daytona_block_id_t block;
    u32 reg_addr=0;u32 bitfield=0;
        
    VTSS_RC(daytona_aneg_get_target(port_no,&block,&target));
    if(block == DAYTONA_BLOCK_XFI){
        reg_addr = VTSS_XFI_ANEG_CFG_ANEG_CFG(target);
        bitfield = VTSS_M_XFI_ANEG_CFG_ANEG_CFG_ANEG_ENA;
    }
    else if(block == DAYTONA_BLOCK_PCS_XAUI){
        reg_addr = VTSS_PCS_XAUI_ANEG_CFG_ANEG_CFG(target);
        bitfield = VTSS_M_PCS_XAUI_ANEG_CFG_ANEG_CFG_ANEG_ENA;
    }else {  
        VTSS_E("daytona:aneg:invalid block");
        return VTSS_RC_ERROR;
    }       
    

    if(vtss_state->aneg_state[port_no].aneg_cfg.aneg_enable == TRUE){   
        DAYTONA_WRM(reg_addr,bitfield,bitfield);
    }else{       
        DAYTONA_WRM(reg_addr,~bitfield,bitfield);
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
 * \brief Timeout check
 *
 * \param inst [IN]      Target instance reference. 
 *
 * \return Return code.
 **/
static vtss_rc daytona_aneg_timeout_check(u32 reg_addr,u32 mask,u32 cmp)
{
    int timeout_counter=0;
    u32 reg_val=0;

    for(timeout_counter=5000;timeout_counter;timeout_counter--){
        DAYTONA_RD(reg_addr, &reg_val);        
        if((reg_val & mask) == cmp)
                break;        
    }
    if(timeout_counter == 0){
        VTSS_E("daytona:aneg:timeout");
        return VTSS_RC_ERROR;        
    }

    return VTSS_RC_OK;
}

/**
 * \brief Get UPI/XFI in use.
 *
 * \param inst [IN]      Target instance reference. 
 *
 * \return Return code.
 **/
static vtss_rc daytona_aneg_is_UPI_XFI(const vtss_port_no_t port_no,BOOL *UPI,BOOL *XFI)
{    
    if(!UPI || !XFI){
        return VTSS_RC_ERROR;
    }          
    
    //if(vtss_state->upi_state[port_no].upi_cfg.enable == FALSE){
    //    if( (vtss_state->xfi_state[port_no].xfi_cfg.bypass_aneg == FALSE) &&
    //        (vtss_state->xfi_state[port_no].xfi_cfg.sel_lane_b == FALSE) &&
    //        (vtss_state->xfi_state[port_no].xfi_cfg.enable == TRUE) )
    //        {
    //            *XFI = TRUE;
    //            *UPI =FALSE;
    //        }else{
    //            *XFI = FALSE;
    //            *UPI = FALSE;                
    //        }
    //}else{
    *UPI = TRUE;
    *XFI = FALSE;
    //}

    return VTSS_RC_OK;
}

/**
 * \brief Get UPI/XFI in use.
 *
 * \param inst [IN]      Target instance reference. 
 *
 * \return Return code.
 **/
static vtss_rc daytona_aneg_get_target(const vtss_port_no_t port_no, daytona_block_id_t *const block, u32 *const target)
{    
    BOOL UPI, XFI;
    
    if(!block || !target){
        return VTSS_RC_ERROR;
    }    
    
    if(daytona_aneg_is_UPI_XFI(port_no,&UPI,&XFI) == VTSS_RC_OK){
        if(UPI==TRUE)
            *block = DAYTONA_BLOCK_PCS_XAUI;
        else if(XFI==TRUE)
            *block = DAYTONA_BLOCK_XFI;
        else 
            return VTSS_RC_ERROR;
    }    
    
    VTSS_RC(daytona_port_2_target(port_no, *block, target));
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
vtss_rc vtss_daytona_inst_aneg_create(void)
{
    vtss_cil_func_t *func = &vtss_state->cil_func;

    /* Daytona ANEG functions */    
    func->aneg_set_config = daytona_aneg_config_set;
    func->aneg_set_ability = daytona_aneg_ability_set;
    func->aneg_set_enable = daytona_aneg_enable;
    func->aneg_restart = daytona_aneg_restart;
    func->aneg_reset = daytona_aneg_reset;   

    return VTSS_RC_OK;
}

#endif /* VTSS_FEATURE_ANEG */
#endif /* VTSS_ARCH_DAYTONA */

