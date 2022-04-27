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

// Avoid "vtss_api.h not used in module vtss_ae_api.c"
/*lint --e{766} */

#include "vtss_api.h"
#if defined(VTSS_FEATURE_AE)

#include "vtss_state.h"
#include "vtss_ae_api.h"

static vtss_rc vtss_ae_lane_no_check(vtss_inst_t inst,vtss_ae_lane_t ae_lane_no);

/* ================================================================= *
 *  Defects/Events
 * ================================================================= */

/* ================================================================= *
 *  Dynamic Config
 * ================================================================= */

vtss_rc vtss_ae_init(vtss_inst_t inst,
                          vtss_ae_lane_t ae_lane_no)
{
    vtss_rc rc;
    int i=0;

    VTSS_D("lane_no: %u", ae_lane_no);
    VTSS_ENTER();
    if ((rc = vtss_ae_lane_no_check (inst, ae_lane_no)) == VTSS_RC_OK) { 
        for(i=0;i<5;i++){
            vtss_state->ae_state[ae_lane_no].ae_protocol_data.reg_save[i]=0;
        }
    }    
    VTSS_EXIT();
    return rc;	
}

vtss_rc vtss_ae_set_config(vtss_inst_t inst,
                          vtss_ae_lane_t ae_lane_no,
                          const vtss_ae_cfg_t *const cfg)
{
    vtss_rc rc;

    VTSS_D("lane_no: %u", ae_lane_no);
    VTSS_ENTER();
    if ((rc = vtss_ae_lane_no_check(inst, ae_lane_no)) == VTSS_RC_OK && cfg) {
        vtss_state->ae_state[ae_lane_no].ae_cfg = *cfg;
        rc = VTSS_FUNC_COLD(ae_set_config, ae_lane_no);
    }
    VTSS_EXIT();
    return rc;	
}

vtss_rc vtss_ae_get_config(vtss_inst_t inst,
                                   vtss_ae_lane_t ae_lane_no,
                                   vtss_ae_cfg_t *const cfg)
{
	vtss_rc rc;

    VTSS_D("lane_no: %u", ae_lane_no);
    VTSS_ENTER();
    if ((rc = vtss_ae_lane_no_check(inst, ae_lane_no)) == VTSS_RC_OK && cfg) {
        *cfg = vtss_state->ae_state[ae_lane_no].ae_cfg;
    }
    VTSS_EXIT();
    return rc;
	
}



vtss_rc vtss_ae_set_coeff_update_req(const vtss_inst_t inst,
                          vtss_ae_lane_t ae_lane_no, const u32  req)
{
    vtss_rc rc;

    VTSS_D("lane_no: %u", ae_lane_no);
    VTSS_ENTER();
    if ((rc = vtss_ae_lane_no_check(inst, ae_lane_no)) == VTSS_RC_OK ) {
        rc = VTSS_FUNC_COLD(ae_set_coeff_updt_req, ae_lane_no,req);
    }
    VTSS_EXIT();
    return rc;	
}         
                          


vtss_rc vtss_ae_get_coeff_update_stat(const vtss_inst_t inst,
                          vtss_ae_lane_t ae_lane_no, u32  *const stat)
{
    vtss_rc rc;

    VTSS_D("lane_no: %u", ae_lane_no);
    VTSS_ENTER();
    if ((rc = vtss_ae_lane_no_check(inst, ae_lane_no)) == VTSS_RC_OK && stat) {
        rc = VTSS_FUNC_COLD(ae_get_coeff_stat_req,ae_lane_no,stat);
    }
    VTSS_EXIT();
    return rc;	

}

vtss_rc vtss_ae_get_eye_data(const vtss_inst_t inst,
                          vtss_ae_lane_t ae_lane_no, const vtss_ae_eye_data_t  cmd,vtss_ae_eye_data_val_t *const retval)
{
    vtss_rc rc;

    VTSS_D("lane_no: %u", ae_lane_no);
    VTSS_ENTER();
    if ((rc = vtss_ae_lane_no_check(inst, ae_lane_no)) == VTSS_RC_OK && retval) {
        rc = VTSS_FUNC_COLD(ae_get_eye_data,ae_lane_no,cmd,retval);
    }
    VTSS_EXIT();
    return rc;

}

vtss_rc vtss_ae_init_rx(const vtss_inst_t inst,
                          vtss_ae_lane_t ae_lane_no)
{
    vtss_rc rc;

    VTSS_D("lane_no: %u", ae_lane_no);
    VTSS_ENTER();
    if ((rc = vtss_ae_lane_no_check(inst, ae_lane_no)) == VTSS_RC_OK  ) {
        rc = VTSS_FUNC_COLD(ae_init_rx,ae_lane_no);
    }
    VTSS_EXIT();
    return rc;

}


vtss_rc vtss_ae_init_tx(const vtss_inst_t inst,
                          vtss_ae_lane_t ae_lane_no)
{
    vtss_rc rc;

    VTSS_D("lane_no: %u", ae_lane_no);
    VTSS_ENTER();
    if ((rc = vtss_ae_lane_no_check(inst, ae_lane_no)) == VTSS_RC_OK ) {
        rc = VTSS_FUNC_COLD(ae_init_tx,ae_lane_no);
    }
    VTSS_EXIT();
    return rc;
	

}


vtss_rc vtss_ae_return_rx_cfg(const vtss_inst_t inst,
                          vtss_ae_lane_t ae_lane_no)
{
    vtss_rc rc;

    VTSS_D("lane_no: %u", ae_lane_no);
    VTSS_ENTER();
    if ((rc = vtss_ae_lane_no_check(inst, ae_lane_no)) == VTSS_RC_OK ) {
        rc = VTSS_FUNC_COLD(ae_return_rx_cfg,ae_lane_no);
    }
    VTSS_EXIT();
    return rc;
	

}


/* ================================================================= *
 *  State Reporting
 * ================================================================= */

vtss_rc vtss_ae_events_get(vtss_inst_t inst,
                          vtss_ae_lane_t ae_lane_no,
                          vtss_ae_events_t *const events)
{
	return VTSS_RC_OK;
}

vtss_rc vtss_ae_get_status(vtss_inst_t inst,
                          vtss_ae_lane_t ae_lane_no,
                          vtss_ae_status_t *const status)
{
	return VTSS_RC_OK;
}


/* ================================================================= *
 *  Performance Primitives
 * ================================================================= */



/* ================================================================= *
 *  Internals
 * ================================================================= */
static vtss_rc vtss_ae_lane_no_check(vtss_inst_t inst,vtss_ae_lane_t ae_lane_no)
{
    if(ae_lane_no >= vtss_state->ae_lane_cnt){
        VTSS_E("%s:illegal lane_no: %u",vtss_func,ae_lane_no);
        return VTSS_RC_ERROR;
    }
    return VTSS_RC_OK;
}


#endif /*VTSS_FEATURE_AE */
