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

// Avoid "vtss_api.h not used in module vtss_daytona_sfi4.c"
/*lint --e{766} */

#include "vtss_api.h"
#if defined(VTSS_ARCH_DAYTONA) && defined(VTSS_FEATURE_SFI4)
#include "../vtss_state.h"
#include "vtss_daytona.h"
#include "vtss_daytona_regs.h"
#include "vtss_daytona_basics.h"
#include "vtss_daytona_sfi4.h"
#include "vtss_daytona_regs_sfi4.h"



/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */

static vtss_rc daytona_sfi4_enable(const vtss_port_no_t port_no)
{
    u32 target;u32 reg_val=0;u32 reg_mask=0;
    VTSS_RC(daytona_port_2_target(port_no, DAYTONA_BLOCK_UPI, &target));

    (vtss_state->sfi4_state[port_no].sfi4_cfg.enable == TRUE) ? (reg_val |= VTSS_M_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SFI4_ENA) : (reg_val &= ~VTSS_M_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SFI4_ENA);    
    reg_mask = (VTSS_M_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SFI4_ENA);
    DAYTONA_WRM(VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG(target),reg_val,reg_mask);  
    
    return VTSS_RC_OK;    
}

static vtss_rc daytona_sfi4_loopback(const vtss_port_no_t port_no)
{
    u32 target;u32 reg_val=0;
    VTSS_RC(daytona_port_2_target(port_no, DAYTONA_BLOCK_UPI, &target));

    (vtss_state->sfi4_state[port_no].sfi4_cfg.rx_to_tx_loopback == TRUE) ? reg_val |= VTSS_M_SFI4_SFI4_CONFIGURATION_SFI4_CFG_RX_TO_TX_LOOP_ENA: (reg_val &= ~VTSS_M_SFI4_SFI4_CONFIGURATION_SFI4_CFG_RX_TO_TX_LOOP_ENA);    
    (vtss_state->sfi4_state[port_no].sfi4_cfg.tx_to_rx_loopback == TRUE) ? reg_val |= VTSS_M_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SLOOP_ENA: (reg_val &= ~VTSS_M_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SLOOP_ENA);    
    DAYTONA_WRM(VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG(target),reg_val,VTSS_M_SFI4_SFI4_CONFIGURATION_SFI4_CFG_RX_TO_TX_LOOP_ENA|VTSS_M_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SLOOP_ENA);    
    
    return VTSS_RC_OK;    
}

static vtss_rc daytona_sfi4_config_set(const vtss_port_no_t port_no)
{
    u32 target;    
    u32 reg_val=0;
    u32 reg_mask=0;

    VTSS_RC(daytona_port_2_target(port_no, DAYTONA_BLOCK_UPI, &target));

    //global ctl   
    (vtss_state->sfi4_state[port_no].sfi4_cfg.dual_rate == TRUE) ? (reg_val |= VTSS_M_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SFI_DDR_SEL) : (reg_val &= ~VTSS_M_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SFI_DDR_SEL);    
    reg_mask = (VTSS_M_SFI4_SFI4_CONFIGURATION_SFI4_CFG_SFI_DDR_SEL);
    DAYTONA_WRM(VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG(target),reg_val,reg_mask);  
   
    VTSS_RC(daytona_sfi4_loopback(port_no));

   return VTSS_RC_OK;
}

/*  Currently unused
static vtss_rc daytona_sfi4_force_resync(const vtss_port_no_t port_no)
{
    u32 target;
    VTSS_RC(daytona_port_2_target(port_no, DAYTONA_BLOCK_UPI, &target));
    
    DAYTONA_WRM(VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG(target),VTSS_M_SFI4_SFI4_CONFIGURATION_SFI4_CFG_RESYNC_ENA,VTSS_M_SFI4_SFI4_CONFIGURATION_SFI4_CFG_RESYNC_ENA);
    //need a small delay here.
    DAYTONA_WRM(VTSS_SFI4_SFI4_CONFIGURATION_SFI4_CFG(target),0,VTSS_M_SFI4_SFI4_CONFIGURATION_SFI4_CFG_RESYNC_ENA);
    
    return VTSS_RC_OK;    
}
*/


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
vtss_rc vtss_daytona_inst_sfi4_create(void)
{
    vtss_cil_func_t *func = &vtss_state->cil_func;

    /* Daytona UPI functions */    
    func->sfi4_set_config = daytona_sfi4_config_set;
    func->sfi4_set_enable = daytona_sfi4_enable;    
    func->sfi4_set_loopback = daytona_sfi4_loopback;    
    return VTSS_RC_OK;
}

#endif /* VTSS_ARCH_DAYTONA */



