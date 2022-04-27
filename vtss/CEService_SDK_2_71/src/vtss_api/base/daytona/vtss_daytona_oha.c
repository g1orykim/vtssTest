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

// Avoid "vtss_api.h not used in module vtss_daytona_oha.c"
/*lint --e{766} */

#include "vtss_api.h"
#if defined(VTSS_ARCH_DAYTONA) && defined(VTSS_FEATURE_OHA)
#include "../ail/vtss_state.h"
#include "vtss_daytona.h"
#include "vtss_daytona_regs.h"
#include "vtss_daytona_basics.h"
#include "vtss_daytona_oha.h"
#include "vtss_daytona_regs_devcpu_gcb.h"
#include "vtss_daytona_regs_ewis.h"


/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */

static vtss_rc daytona_oha_config_set(vtss_state_t *vtss_state,
                                      const vtss_port_no_t port_no)
{
    daytona_channel_t channel;
    daytona_side_t    side;
    u32 target_wis;

    VTSS_RC(daytona_port_2_channel_side(vtss_state, port_no, &channel, &side));
    VTSS_RC(daytona_port_2_target(vtss_state, port_no, DAYTONA_BLOCK_EWIS, &target_wis));

    if (vtss_state->oha_state[port_no].oha_cfg.tosi_rosi_otu2_1 == VTSS_OHA_TOSI_ROSI_EWIS2) {
        if(side == 0) {
           return VTSS_RC_ERROR;
        }
        /* Set mux for eWIS */
        if (channel == 0) {
            DAYTONA_WRM(VTSS_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0, 
                        VTSS_F_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0_ROSI_TOSI_MUX_0(0),
                        VTSS_M_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0_ROSI_TOSI_MUX_0);
        } else if (channel == 1) {
            DAYTONA_WRM(VTSS_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1, 
                        VTSS_F_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1_ROSI_TOSI_MUX_1(0),
                        VTSS_M_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1_ROSI_TOSI_MUX_1);
        }
        /* Enable eWIS2 ROSI & TOSI */
         DAYTONA_WRM(VTSS_EWIS_TX_WIS_CTRL_TOSI_CTRL(target_wis), 
                     VTSS_F_EWIS_TX_WIS_CTRL_TOSI_CTRL_TOSI_ENA(1),
                     VTSS_M_EWIS_TX_WIS_CTRL_TOSI_CTRL_TOSI_ENA);

         DAYTONA_WRM(VTSS_EWIS_RX_WIS_CTRL_ROSI_CTRL(target_wis), 
                     VTSS_F_EWIS_RX_WIS_CTRL_ROSI_CTRL_ROSI_ENA(1),
                     VTSS_M_EWIS_RX_WIS_CTRL_ROSI_CTRL_ROSI_ENA);

    } else if (vtss_state->oha_state[port_no].oha_cfg.tosi_rosi_otu2_1 == VTSS_OHA_TOSI_ROSI_OTU2_1) {
        /* Set mux for OTN1 */
        if (channel == 0) {
            DAYTONA_WRM(VTSS_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0, 
                        VTSS_F_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0_ROSI_TOSI_MUX_0(1),
                        VTSS_M_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0_ROSI_TOSI_MUX_0);
        } else if (channel == 1) {
            DAYTONA_WRM(VTSS_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1, 
                        VTSS_F_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1_ROSI_TOSI_MUX_1(1),
                        VTSS_M_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1_ROSI_TOSI_MUX_1);
        }
    }

    if (vtss_state->oha_state[port_no].oha_cfg.otu2_1_otu2_2 == VTSS_OHA_OTU2_OTU2_1) {
        /* Set mux for OTN1 */
        if (channel == 0) {
            DAYTONA_WRM(VTSS_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0, 
                        VTSS_F_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0_OTN_OH_MUX_0(1),
                        VTSS_M_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0_OTN_OH_MUX_0);
        } else if (channel == 1) {
            DAYTONA_WRM(VTSS_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1, 
                        VTSS_F_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1_OTN_OH_MUX_1(1),
                        VTSS_M_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1_OTN_OH_MUX_1);
        }
    } else if (vtss_state->oha_state[port_no].oha_cfg.otu2_1_otu2_2 == VTSS_OHA_OTU2_OTU2_2) {
        /* Set mux for OTN1 */
        if (channel == 0) {
            DAYTONA_WRM(VTSS_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0, 
                        VTSS_F_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0_OTN_OH_MUX_0(0),
                        VTSS_M_DEVCPU_GCB_CHIP_MUX_DATA_MUX_0_OTN_OH_MUX_0);
        } else if (channel == 1) {
            DAYTONA_WRM(VTSS_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1, 
                        VTSS_F_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1_OTN_OH_MUX_1(0),
                        VTSS_M_DEVCPU_GCB_CHIP_MUX_DATA_MUX_1_OTN_OH_MUX_1);
        }
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
 * \brief Static Initialization of the configuration data base in the vtss_state.
 **/

static vtss_oha_cfg_t cfg_default = {
    .tosi_rosi_otu2_1  = VTSS_OHA_TOSI_ROSI_EWIS2,
    .otu2_1_otu2_2     = VTSS_OHA_OTU2_OTU2_2,
};

vtss_rc vtss_daytona_oha_restart_conf_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    vtss_rc rc;

    rc = daytona_oha_config_set(vtss_state, port_no);
    VTSS_D("port_no %d, restart conf set, rc = %x", port_no, rc);
    return VTSS_RC_OK;
}

/*
 * \brief Set default oha configuration
 *
 * \param port_no [IN]   Port number
 * 
 * \return Return code.
 **/
vtss_rc vtss_daytona_oha_default_conf_set(vtss_state_t *vtss_state, vtss_port_no_t port_no)
{
    vtss_state->oha_state[port_no].oha_cfg = cfg_default;

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
vtss_rc vtss_daytona_inst_oha_create(vtss_state_t *vtss_state)
{
    vtss_cil_func_t *func = &vtss_state->cil;
    vtss_port_no_t port_no;

    for (port_no = 0 ; port_no < VTSS_PORT_ARRAY_SIZE; port_no++) {
        vtss_state->oha_state[port_no].oha_cfg = cfg_default;
    }

    /* Daytona OHA functions */    
    func->oha_config_set = daytona_oha_config_set;

    return VTSS_RC_OK;
}

#endif /* VTSS_ARCH_DAYTONA */

